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

#include "remote_scheduler_eicic.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "flexran.pb.h"
#include "rib_common.h"
#include "cell_mac_rib_info.h"

int32_t flexran::app::scheduler::remote_scheduler_eicic::tpc_accumulated = 0;

void flexran::app::scheduler::remote_scheduler_eicic::run_periodic_task() {

  rib::frame_t target_frame;
  rib::subframe_t target_subframe;
  
  unsigned char aggregation;
  uint16_t total_nb_available_rb[rib::MAX_NUM_CC];

  uint16_t nb_available_rb, nb_rb, nb_rb_tmp, TBS, sdu_length_total = 0;
  uint8_t harq_pid, round, ta_len = 0;

  uint32_t dci_tbs;
  int mcs, ndi, tpc = 1, mcs_tmp;
  uint32_t ce_flags = 0;
  uint32_t data_to_request;

  uint8_t header_len_dcch = 0, header_len_dcch_tmp = 0, header_len_dtch = 0, header_len_dtch_tmp = 0;
  uint8_t header_len = 0, header_len_tmp = 0;

  bool ue_has_transmission = false;

  bool schedule_macro = true;
  
  bool schedule_flag = false;

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

  // Check if scheduling needs to be performed and who needs to be scheduled (macro or pico cells)
  for (const auto& agent_id : agent_ids) {
    ::std::shared_ptr<const rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
    const protocol::flex_enb_config_reply& enb_config = agent_config->get_enb_config();
    const protocol::flex_ue_config_reply& ue_configs = agent_config->get_ue_configs();
    const protocol::flex_lc_config_reply& lc_configs = agent_config->get_lc_configs();

    rib::frame_t current_frame = agent_config->get_current_frame();
    rib::subframe_t current_subframe = agent_config->get_current_subframe();

    // Check if scheduling context for this eNB is already present and if not create it
    ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
    if (enb_sched_info) {
      // Nothing to do if this exists
    } else { // eNB sched info was not found for this agent
      ::std::cout << "Config was not found. Creating" << ::std::endl;
      scheduling_info_.insert(::std::pair<int,
			      ::std::shared_ptr<enb_scheduling_info>>(agent_id,
								      ::std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
      enb_sched_info = get_scheduling_info(agent_id);
    }
    
    //std::cout << "Checking id " << agent_id << std::endl;
    //std::cout << "Current subframe " << current_subframe << std::endl;

    // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
    if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
      continue;
    }
    
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);

    target_subframe = (current_subframe + schedule_ahead) % 10;
    if (target_subframe < current_subframe) {
      target_frame = (current_frame + 1) % 1024;
    } else {
      target_frame = current_frame;
    }
    int additional_frames = schedule_ahead / 10;
    target_frame = (current_frame + additional_frames) % 1024;
    
    if (abs_[target_subframe] != 0) {
      schedule_flag = true;
    }
    
    // Only need to check if scheduling needs to be performed if this is a pico cell
    if (agent_id == macro_agent_id_) {
      continue;
    }
   
    // Go through the cells and schedule the UEs of this cell
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::flex_cell_config cell_config = enb_config.cell_config(i);
      int cell_id = cell_config.cell_id();

      for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
	const protocol::flex_ue_config ue_config = ue_configs.ue_config(UE_id);

	if (ue_config.pcell_carrier_index() == cell_id) {
	  
	  // Get the MAC stats for this UE
	  ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	
	  //std::cout << "Got the MAC stats of the UE with rnti: " << ue_config.rnti() << std::endl;

	  if (!ue_mac_info) {
	    continue;
	  }
	  
	  const protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();
	  
	  for (int j = 0; j < mac_report.rlc_report_size(); j++) {
	    // If there is something to transmit in one of the pico cells, set the macro cell scheduling flag to false
	    if (mac_report.rlc_report(j).tx_queue_size() > 0) {
	      //std::cout << "We need to schedule the pico cell" << target_subframe << std::endl;
	      schedule_macro = false;
	    }
	  }
	}
      }
    }
  }

  if (!schedule_flag) {
    return;
  }

  for (const auto& agent_id : agent_ids) {
    
    // Check the agent id and the macro cell schedule flag and choose if this agent will be scheduled or not
    if (((agent_id == macro_agent_id_) && (!schedule_macro)) ||
	((agent_id != macro_agent_id_) && (schedule_macro))) {
      continue;
    } else { //Simply send an empty dl_mac_config message to notify the permission to schedule
      protocol::flexran_message out_message;

      // Create dl_mac_config message header
      protocol::flex_header *header(new protocol::flex_header);
      header->set_type(protocol::FLPT_DL_MAC_CONFIG);
      header->set_version(0);
      header->set_xid(0);
    
      ::std::shared_ptr<const rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
      const protocol::flex_enb_config_reply& enb_config = agent_config->get_enb_config();
      const protocol::flex_ue_config_reply& ue_configs = agent_config->get_ue_configs();
      const protocol::flex_lc_config_reply& lc_configs = agent_config->get_lc_configs();
      
      rib::frame_t current_frame = agent_config->get_current_frame();
      rib::subframe_t current_subframe = agent_config->get_current_subframe();
      
      // Check if scheduling context for this eNB is already present and if not create it
      ::std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
      if (enb_sched_info) {
	// Nothing to do if this exists
      } else { // eNB sched info was not found for this agent
	::std::cout << "Config was not found. Creating" << ::std::endl;
	scheduling_info_.insert(::std::pair<int,
				::std::shared_ptr<enb_scheduling_info>>(agent_id,
									::std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
	enb_sched_info = get_scheduling_info(agent_id);
      }
      
      target_subframe = (current_subframe + schedule_ahead) % 10;
      if (target_subframe < current_subframe) {
	target_frame = (current_frame + 1) % 1024;
      } else {
	target_frame = current_frame;
      }
      int additional_frames = schedule_ahead / 10;
      target_frame = (current_frame + additional_frames) % 1024;
      
      // Create dl_mac_config message
      protocol::flex_dl_mac_config *dl_mac_config_msg(new protocol::flex_dl_mac_config);
      dl_mac_config_msg->set_allocated_header(header);
      dl_mac_config_msg->set_sfn_sf(rib::get_sfn_sf(target_frame, target_subframe));
      
      // Create and send the flexran message
      out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
      out_message.set_allocated_dl_mac_config_msg(dl_mac_config_msg);
      req_manager_.send_message(agent_id, out_message);
      
    }  
  }
}   

std::shared_ptr<flexran::app::scheduler::enb_scheduling_info>
flexran::app::scheduler::remote_scheduler_eicic::get_scheduling_info(int agent_id) {
  auto it = scheduling_info_.find(agent_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return ::std::shared_ptr<enb_scheduling_info>(nullptr);
}
