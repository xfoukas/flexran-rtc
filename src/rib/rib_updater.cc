/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <iostream>

#include "rib_updater.h"
#include "tagged_message.h"
#include "rt_wrapper.h"

#include "rt_controller_common.h"

#include "flexran_log.h"

void flexran::rib::rib_updater::run() {
  update_rib();
}

void flexran::rib::rib_updater::update_rib() {  
  int rem_msgs = messages_to_check_;
  protocol::flexran_message in_message;
  std::shared_ptr<flexran::network::tagged_message> tm;
  while(net_xface_.get_msg_from_network(tm) && (rem_msgs > 0)) {
    /* TODO: update the RIB based on what you see */
    if (tm->getSize() == 0) { // New connection. update the pending eNBs list
      LOG4CXX_INFO(flexran::core::rib_logger, "A new agent connection was established");
      rib_.add_pending_agent(tm->getTag());
      protocol::flex_header *header(new protocol::flex_header);
      header->set_type(protocol::FLPT_HELLO);
      header->set_version(0);
      header->set_xid(0);
	
      protocol::flex_hello *hello_msg(new protocol::flex_hello);
      hello_msg->set_allocated_header(header);
	
      protocol::flexran_message out_message;
      out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
      out_message.set_allocated_hello_msg(hello_msg);
      net_xface_.send_msg(out_message, tm->getTag());
    } else { // Message from existing connection. Just update the proper rib entries
      // Deserialize the message
      in_message.ParseFromArray(tm->getMessageContents(), tm->getSize());
      // Update the RIB based on the message type
      if(in_message.has_hello_msg()) {
	LOG4CXX_INFO(flexran::core::rib_logger, "Received a hello msg");
	handle_message(tm->getTag(), in_message.hello_msg(), in_message.msg_dir());
	LOG4CXX_INFO(flexran::core::rib_logger, "Handled the hello msg");
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
	LOG4CXX_INFO(flexran::core::rib_logger, "Got an eNB config reply msg");
	handle_message(tm->getTag(), in_message.enb_config_reply_msg());
      } else if(in_message.has_ue_config_reply_msg()) {
	LOG4CXX_INFO(flexran::core::rib_logger, "Got a UE config reply msg");
	handle_message(tm->getTag(), in_message.ue_config_reply_msg());
      } else if(in_message.has_lc_config_reply_msg()) {
	LOG4CXX_INFO(flexran::core::rib_logger, "Got an LC config reply msg");
        handle_message(tm->getTag(), in_message.lc_config_reply_msg());
      } else if(in_message.has_ue_state_change_msg()) {
	LOG4CXX_INFO(flexran::core::rib_logger, "Seems like a UE state changed");
	handle_message(tm->getTag(), in_message.ue_state_change_msg());
      }
    }
    rem_msgs--;
  }
}

// Handle hello message
void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_hello& hello_msg,
					       protocol::flexran_direction dir) {

  _unused(hello_msg);
  
  if (dir == protocol::SUCCESSFUL_OUTCOME) {
    // Agent is alive. Request info about its configuration
    // eNB config first
    protocol::flex_header *header1(new protocol::flex_header);
    header1->set_type(protocol::FLPT_GET_ENB_CONFIG_REQUEST);
    header1->set_version(0);
    header1->set_xid(0);

    protocol::flex_enb_config_request *enb_config_request_msg(new protocol::flex_enb_config_request);
    enb_config_request_msg->set_allocated_header(header1);

    protocol::flexran_message out_message1;
    out_message1.set_msg_dir(protocol::INITIATING_MESSAGE);
    out_message1.set_allocated_enb_config_request_msg(enb_config_request_msg);
    net_xface_.send_msg(out_message1, agent_id);
  
    // UE config second
    protocol::flexran_message out_message2;
    out_message2.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header2(new protocol::flex_header);
    header2->set_type(protocol::FLPT_GET_UE_CONFIG_REQUEST);
    header2->set_xid(1);
    protocol::flex_ue_config_request *ue_config_request_msg(new protocol::flex_ue_config_request);
    ue_config_request_msg->set_allocated_header(header2);
    out_message2.set_allocated_ue_config_request_msg(ue_config_request_msg);
    net_xface_.send_msg(out_message2, agent_id);

    // LC config third
    protocol::flexran_message out_message3;
    out_message3.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header3(new protocol::flex_header);
    header3->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
    header3->set_xid(2);
    protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
    lc_config_request_msg->set_allocated_header(header3);
    out_message3.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message3, agent_id);
  } // Hello should originate from controller - Ignore in all other cases
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_echo_request& echo_request_msg) {

  // Need to send an echo reply
  protocol::flex_header *header(new protocol::flex_header);
  header->set_type(protocol::FLPT_ECHO_REPLY);
  header->set_version(0);
  header->set_xid(echo_request_msg.header().xid());

  protocol::flex_echo_reply *echo_reply_msg(new protocol::flex_echo_reply);
  echo_reply_msg->set_allocated_header(header);

  protocol::flexran_message out_message;
  out_message.set_msg_dir(protocol::SUCCESSFUL_OUTCOME);
  out_message.set_allocated_echo_reply_msg(echo_reply_msg);
  net_xface_.send_msg(out_message, agent_id);
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_echo_reply& echo_reply_msg) {
  _unused(echo_reply_msg);  

  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.update_liveness(agent_id);
  } else {
    /* TODO: Should probably do some error handling */
  }
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_sf_trigger& sf_trigger_msg) {
  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.set_subframe_updates(agent_id, sf_trigger_msg);
  } else {
    /* TODO: Should probably do some error handling */
  }
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_enb_config_reply& enb_config_reply_msg) {
  if (rib_.agent_is_pending(agent_id)) {
    // Must create a new eNB_config entry
    rib_.new_eNB_config_entry(agent_id);
    rib_.remove_pending_agent(agent_id);
    rib_.eNB_config_update(agent_id, enb_config_reply_msg);
  }// If agent was not pending we should ignore this message. Only for initialization
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_ue_config_reply& ue_config_reply_msg) {
  if (rib_.has_eNB_config_entry(agent_id)) {
    rib_.ue_config_update(agent_id, ue_config_reply_msg);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_lc_config_reply& lc_config_reply_msg) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    rib_.lc_config_update(agent_id, lc_config_reply_msg);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_stats_reply& mac_stats_reply) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    rib_.mac_stats_update(agent_id, mac_stats_reply);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}

void flexran::rib::rib_updater::handle_message(int agent_id,
					       const protocol::flex_ue_state_change& ue_state_change_msg) {
  if(rib_.has_eNB_config_entry(agent_id)) {
    /* TODO add the handler for the update of the state */
    rib_.ue_config_update(agent_id, ue_state_change_msg);
    protocol::flexran_message out_message;
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    protocol::flex_header *header(new protocol::flex_header);
    header->set_type(protocol::FLPT_GET_LC_CONFIG_REQUEST);
    header->set_xid(2);
    protocol::flex_lc_config_request *lc_config_request_msg(new protocol::flex_lc_config_request);
    lc_config_request_msg->set_allocated_header(header);
    out_message.set_allocated_lc_config_request_msg(lc_config_request_msg);
    net_xface_.send_msg(out_message, agent_id);
  } else {
    /* TODO: We did not receive the eNB config message for some reason, need to request it again */
  }
}
