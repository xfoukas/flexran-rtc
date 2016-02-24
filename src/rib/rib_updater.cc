#include <iostream>

#include "rib_updater.h"
#include "tagged_message.h"
#include "rt_wrapper.h"

void rib_updater::run() {
  update_rib();
}

void rib_updater::update_rib() {  
  int rem_msgs = messages_to_check_;
  tagged_message *tm;
  protocol::progran_message in_message;
  while(net_xface_.get_msg_from_network(&tm) && (rem_msgs > 0)) {
    /* TODO: update the RIB based on what you see */
    if (tm->getSize() == 0) { // New connection. update the pending eNBs list
      std::cout << "Seems that a new connection was established" << std::endl;
      rib_.add_pending_agent(tm->getTag());
      protocol::prp_header *header(new protocol::prp_header);
      header->set_type(protocol::PRPT_HELLO);
      header->set_version(0);
      header->set_xid(0);
	
      protocol::prp_hello *hello_msg(new protocol::prp_hello);
      hello_msg->set_allocated_header(header);
	
      protocol::progran_message out_message;
      out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
      out_message.set_allocated_hello_msg(hello_msg);
      net_xface_.send_msg(out_message, tm->getTag());
    } else { // Message from existing connection. Just update the proper rib entries
      // Deserialize the message
      in_message.ParseFromArray(tm->getMessageContents(), tm->getSize());
      // Update the RIB based on the message type
      if(in_message.has_hello_msg()) {
	std::cout << "Time to handle a hello msg" << std::endl;
	handle_message(tm->getTag(), in_message.hello_msg(), in_message.msg_dir());
	std::cout << "Handled it" << std::endl;
      } else if(in_message.has_echo_request_msg()) {
	handle_message(tm->getTag(), in_message.echo_request_msg());
      } else if(in_message.has_echo_reply_msg()) {
	handle_message(tm->getTag(), in_message.echo_reply_msg());
      } else if(in_message.has_stats_reply_msg()) {
	handle_message(tm->getTag(), in_message.stats_reply_msg());
      } else if(in_message.has_sf_trigger_msg()) {
	//std::cout<<"Got a sf trigger msg" << std::endl;
	handle_message(tm->getTag(), in_message.sf_trigger_msg());
      } else if(in_message.has_ul_sr_info_msg()) {
	/* TODO: Need to implement to enable UL scheduling */
      } else if(in_message.has_enb_config_reply_msg()) {
	std::cout<<"Got an eNB config reply msg" << std::endl;
	handle_message(tm->getTag(), in_message.enb_config_reply_msg());
      } else if(in_message.has_ue_config_reply_msg()) {
	std::cout<<"Got a UE config reply msg" << std::endl;
	handle_message(tm->getTag(), in_message.ue_config_reply_msg());
      } else if(in_message.has_lc_config_reply_msg()) {
	std::cout<<"Got an LC config reply msg" << std::endl;
        handle_message(tm->getTag(), in_message.lc_config_reply_msg());
      } else if(in_message.has_ue_state_change_msg()) {
	std::cout << "Seems like a UE state changed" << std::endl;
	handle_message(tm->getTag(), in_message.ue_state_change_msg());
      }
    }
    rem_msgs--;
    delete tm;
  }
}

// Handle hello message
void rib_updater::handle_message(int agent_id,
				 const protocol::prp_hello& hello_msg,
				 protocol::progran_direction dir) {
  if (dir == protocol::SUCCESSFUL_OUTCOME) {
    // Agent is alive. Request info about its configuration
    // eNB config first
    protocol::prp_header *header1(new protocol::prp_header);
    header1->set_type(protocol::PRPT_GET_ENB_CONFIG_REQUEST);
    header1->set_version(0);
    header1->set_xid(0);

    protocol::prp_enb_config_request *enb_config_request_msg(new protocol::prp_enb_config_request);
    enb_config_request_msg->set_allocated_header(header1);

    protocol::progran_message out_message1;
    out_message1.set_msg_dir(protocol::INITIATING_MESSAGE);
    out_message1.set_allocated_enb_config_request_msg(enb_config_request_msg);
    net_xface_.send_msg(out_message1, agent_id);
  
    // UE config second
    protocol::progran_message out_message2;
    out_message2.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::prp_header *header2(new protocol::prp_header);
    header2->set_type(protocol::PRPT_GET_UE_CONFIG_REQUEST);
    header2->set_xid(1);
    protocol::prp_ue_config_request *ue_config_request_msg(new protocol::prp_ue_config_request);
    ue_config_request_msg->set_allocated_header(header2);
    out_message2.set_allocated_ue_config_request_msg(ue_config_request_msg);
    net_xface_.send_msg(out_message2, agent_id);

    // LC config third
    protocol::progran_message out_message3;
    out_message3.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::prp_header *header3(new protocol::prp_header);
    header3->set_type(protocol::PRPT_GET_LC_CONFIG_REQUEST);
    header3->set_xid(2);
    protocol::prp_lc_config_request *lc_config_request_msg(new protocol::prp_lc_config_request);
    lc_config_request_msg->set_allocated_header(header3);
    out_message3.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message3, agent_id);
  } // Hello should originate from controller - Ignore in all other cases
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_echo_request& echo_request_msg) {
  // Need to send an echo reply
  protocol::prp_header *header(new protocol::prp_header);
  header->set_type(protocol::PRPT_ECHO_REPLY);
  header->set_version(0);
  header->set_xid(echo_request_msg.header().xid());

  protocol::prp_echo_reply *echo_reply_msg(new protocol::prp_echo_reply);
  echo_reply_msg->set_allocated_header(header);

  protocol::progran_message out_message;
  out_message.set_msg_dir(protocol::SUCCESSFUL_OUTCOME);
  out_message.set_allocated_echo_reply_msg(echo_reply_msg);
  net_xface_.send_msg(out_message, agent_id);
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_echo_reply& echo_reply_msg) {
  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.update_liveness(agent_id);
  } else {
    /* TODO: Should probably do some error handling */
  }
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_sf_trigger& sf_trigger_msg) {
  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.set_subframe_updates(agent_id, sf_trigger_msg);
  } else {
    /* TODO: Should probably do some error handling */
  }
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_enb_config_reply& enb_config_reply_msg) {
  if (rib_.agent_is_pending(agent_id)) {
    // Must create a new eNB_config entry
    rib_.new_eNB_config_entry(agent_id);
    rib_.remove_pending_agent(agent_id);
    rib_.eNB_config_update(agent_id, enb_config_reply_msg);
  }// If agent was not pending we should ignore this message. Only for initialization
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_ue_config_reply& ue_config_reply_msg) {
  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.ue_config_update(agent_id, ue_config_reply_msg);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_lc_config_reply& lc_config_reply_msg) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    rib_.lc_config_update(agent_id, lc_config_reply_msg);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void rib_updater::handle_message(int agent_id,
		    const protocol::prp_stats_reply& mac_stats_reply) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    rib_.mac_stats_update(agent_id, mac_stats_reply);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void rib_updater::handle_message(int agent_id,
				 const protocol::prp_ue_state_change& ue_state_change_msg) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    /* TODO add the handler for the update of the state */
    rib_.ue_config_update(agent_id, ue_state_change_msg);
    protocol::progran_message out_message;
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::prp_header *header(new protocol::prp_header);
    header->set_type(protocol::PRPT_GET_LC_CONFIG_REQUEST);
    header->set_xid(2);
    protocol::prp_lc_config_request *lc_config_request_msg(new protocol::prp_lc_config_request);
    lc_config_request_msg->set_allocated_header(header);
    out_message.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message, agent_id);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}
