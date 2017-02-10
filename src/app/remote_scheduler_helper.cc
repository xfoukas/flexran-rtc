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

#include <algorithm>

#include "remote_scheduler_helper.h"
#include "remote_scheduler.h"
#include "remote_scheduler_primitives.h"

void flexran::app::scheduler::remote_scheduler_helper::run_dlsch_scheduler_preprocessor(protocol::flex_cell_config& cell_config,
											protocol::flex_ue_config_reply& ue_configs,
											protocol::flex_lc_config_reply& lc_configs,
											std::shared_ptr<flexran::rib::enb_rib_info> agent_config,
											std::shared_ptr<enb_scheduling_info> sched_info,
											flexran::rib::frame_t frame,
											flexran::rib::subframe_t subframe) {

  uint16_t total_nb_available_rb;
  int total_ue_count;
  uint16_t average_rbs_per_user = 0;
  int transmission_mode;

  _unused(frame);

  total_nb_available_rb = cell_config.dl_bandwidth();
  //Remove the RBs used by common channels
  //TODO: For now we will do this manually based on OAI config and scheduling sf number. Important to fix it later.
  // Assume an FDD scheduler
  if (subframe == 0) {
    total_nb_available_rb -= 4;
  } else if (subframe == 5) {
    total_nb_available_rb -=8;
  }

  // Find the active UEs for this cell
  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    uint16_t cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
      
      // Check to see if there is a scheduling configuration created for this UE and if not create it
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      if (!ue_sched_info) {
	sched_info->create_ue_scheduling_info(ue_config.rnti());
	ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      } else {
	ue_sched_info->start_new_scheduling_round(); // reset scheduling-related values 
      }
     

      for (int k = 0; k < lc_configs.lc_ue_config_size(); k++) {
	const protocol::flex_lc_ue_config *lc_conf = &(lc_configs.lc_ue_config(k));
	if (lc_conf->rnti() == ue_config.rnti()) {
	  assign_rbs_required(ue_sched_info, ue_mac_info, cell_config, *lc_conf);
	  break;
	}
      }       
    }
  }

  
  total_ue_count = 0;

  //Sort the scheduled UEs based on their buffer size
  const std::shared_ptr<std::vector<int>> sorted_ues = sort_UEs(cell_config, ue_configs, agent_config);

  for (int& i : *sorted_ues) {
    protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    uint16_t cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());

      // Get the scheduling info
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

      if (!ue_mac_info->has_available_harq(cell_id)) {
	continue;
      }
      
      // Get the active HARQ process
      int harq_pid = ue_mac_info->get_next_available_harq(cell_id);
      int round = ue_mac_info->get_harq_stats(cell_id, harq_pid);
      ue_sched_info->set_harq_round(cell_id, harq_pid, round);

      if (round > 0) {
	ue_sched_info->set_nb_rbs_required(cell_id, ue_sched_info->get_nb_scheduled_rbs(cell_id, harq_pid));
      } else {
	ue_sched_info->set_nb_scheduled_rbs(cell_id, harq_pid, 0);
      }

      if (ue_sched_info->get_nb_rbs_required(cell_id) > 0) {
	total_ue_count = total_ue_count + 1;
      }
    }
  }

  if (total_ue_count == 0) {
    average_rbs_per_user = 0;
  } else if ((get_min_rb_unit(cell_config) * total_ue_count) <= total_nb_available_rb) {
    average_rbs_per_user = total_nb_available_rb/total_ue_count;
  } else {
    average_rbs_per_user = get_min_rb_unit(cell_config);
  }
  
  for (int& i : *sorted_ues) {
    protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    uint16_t cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
      
      // Get the scheduling info
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

      if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0 ) {
	continue;
      }
  
      if (ue_sched_info->is_high_priority()) {
	ue_sched_info->set_nb_rbs_required_remaining1(cell_id, ue_sched_info->get_nb_rbs_required(cell_id));
      } else {
	 ue_sched_info->set_nb_rbs_required_remaining1(cell_id, ::std::min(average_rbs_per_user,
									   ue_sched_info->get_nb_rbs_required(cell_id)));
      }
    }
  }

 for (int r1 = 0; r1 < 2; r1++) {
    // Go through each of the UEs that need to be scheduled
   for (int& i : *sorted_ues) {
      protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
      uint16_t cell_id = cell_config.cell_id();
      // If this UE is assigned to this cell
      if (ue_config.pcell_carrier_index() == cell_id) {
	// Get the MAC stats for this UE
	::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	
	// Get the scheduling info
	::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
	
	if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0) {
	  continue;
	}

	if (r1 == 0) {
	  ue_sched_info->set_nb_rbs_required_remaining(cell_id, ue_sched_info->get_nb_rbs_required_remaining1(cell_id));
	} else {
	  uint16_t nb_rem = ue_sched_info->get_nb_rbs_required(cell_id) - ue_sched_info->get_nb_rbs_required_remaining1(cell_id) + ue_sched_info->get_nb_rbs_required_remaining(cell_id);
	  ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	}
      }
   }

    if (total_ue_count > 0) {
      // Go through all the UEs and allocate the resources in sched info first to high priority and then to rest

      for (int& i : *sorted_ues) {
	protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
	uint16_t cell_id = cell_config.cell_id();
	// If this UE is assigned to this cell
	if (ue_config.pcell_carrier_index() == cell_id) {
	  // Get the MAC stats for this UE
	  ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
	  transmission_mode = ue_config.transmission_mode();
	  
	  if (!ue_sched_info->is_high_priority()) {
	    continue;
	  }
	  
	  if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0) {
	    continue;
	  }

	  perform_pre_processor_allocation(cell_config,
					   sched_info,
					   ue_sched_info,
					   transmission_mode);

	}
      }

      // Schedule the UEs that are not high priority

      for (int& i : *sorted_ues) {
	//	for (int i = 0; i < ue_configs.ue_config_size(); i++) {
	protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
	uint16_t cell_id = cell_config.cell_id();
	// If this UE is assigned to this cell
	if (ue_config.pcell_carrier_index() == cell_id) {
	  // Get the MAC stats for this UE
	  ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
	  transmission_mode = ue_config.transmission_mode();
	  
	  if (ue_sched_info->is_high_priority()) {
	    continue;
	  }
	  
	  if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0) {
	    continue;
	  }
	  
	  perform_pre_processor_allocation(cell_config,
					   sched_info,
					   ue_sched_info,
					   transmission_mode);

	}
      }
      // }
    }
 }
 // for (int i = 0; i < ue_configs.ue_config_size(); i++) {
 //   const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
 //   int cell_id = cell_config.cell_id();
 //   // If this UE is assigned to this cell
 //   if (ue_config.pcell_carrier_index() == cell_id) {
 //     // Get the MAC stats for this UE
 //     ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
     
 //     // Get the scheduling info
 //     ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
 //     std::cout << "Rballoc for UE " << i << std::endl;
 //     std::cout << "Allocated " << ue_sched_info->get_pre_nb_rbs_available(cell_id) << " RBs " << std::endl;
 //     for (int j = 0; j <  get_nb_rbg(cell_config); j++) {
 //       std::cout << (int) ue_sched_info->get_rballoc_sub(cell_id, j);
 //     }
 //     std::cout << std::endl;
 //   }
 // }
}

void flexran::app::scheduler::remote_scheduler_helper::perform_pre_processor_allocation(protocol::flex_cell_config& cell_config,
											std::shared_ptr<enb_scheduling_info> sched_info,
											std::shared_ptr<ue_scheduling_info> ue_sched_info,
											int transmission_mode) {

  _unused(transmission_mode);
  
  int n_rbg = get_nb_rbg(cell_config);
  int min_rb_unit = get_min_rb_unit(cell_config);
  uint16_t cell_id = cell_config.cell_id();
  uint8_t* rballoc_sub = sched_info->get_vrb_map(cell_id);
  for (int i = 0; i < n_rbg; i++) {
    if ((rballoc_sub[i] == 0) &&
	(ue_sched_info->get_rballoc_sub(cell_id, i) == 0) &&
	(ue_sched_info->get_nb_rbs_required_remaining(cell_id) > 0) &&
	(ue_sched_info->get_pre_nb_rbs_available(cell_id) <  ue_sched_info->get_nb_rbs_required(cell_id))) {
	  
      //TODO: No TM5 for now
      if ((i == n_rbg - 1) && ((cell_config.dl_bandwidth() == 25) || (cell_config.dl_bandwidth() == 50))) {
	rballoc_sub[i] = 1;
	sched_info->toggle_vrb_map(cell_id, i);
	ue_sched_info->set_rballoc_sub(cell_id, i, 1);
	int nb_rem = ue_sched_info->get_nb_rbs_required_remaining(cell_id) - min_rb_unit + 1;
	ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	nb_rem = ue_sched_info->get_pre_nb_rbs_available(cell_id) + min_rb_unit - 1;
	ue_sched_info->set_pre_nb_rbs_available(cell_id, nb_rem);
      } else {
	if (ue_sched_info->get_nb_rbs_required_remaining(cell_id) >= min_rb_unit) {
	  rballoc_sub[i] = 1;
	  sched_info->toggle_vrb_map(cell_id, i);
	  ue_sched_info->set_rballoc_sub(cell_id, i, 1);
	  int nb_rem = ue_sched_info->get_nb_rbs_required_remaining(cell_id) - min_rb_unit;
	  ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	  nb_rem = ue_sched_info->get_pre_nb_rbs_available(cell_id) + min_rb_unit;
	  ue_sched_info->set_pre_nb_rbs_available(cell_id, nb_rem);
	}
      }
	  
    }
	
  }
  
}

void flexran::app::scheduler::remote_scheduler_helper::assign_rbs_required(::std::shared_ptr<ue_scheduling_info> ue_sched_info,
									   ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info,
									   protocol::flex_cell_config& cell_config,
									   const protocol::flex_lc_ue_config& lc_ue_config) {
  uint16_t TBS = 0;
  
  //Compute the mcs of the UE for this cell
  int mcs = 0;
  protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();
  for (int i = 0; i < mac_report.dl_cqi_report().csi_report_size(); i++) {
    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(i).serv_cell_index()) {
      mcs = rib::cqi_to_mcs[mac_report.dl_cqi_report().csi_report(i).p10csi().wb_cqi()];
      break;
    }
  }

  int total_buffer_bytes = 0;
  // Go through the logical channels of the UE and find how many bytes need to be transmitted in total
  for (int i = 0; i < mac_report.rlc_report_size(); i++) {
   
    if (lc_ue_config.lc_config_size() > 2) {
      ue_sched_info->is_high_priority(false);
    } else {
      ue_sched_info->is_high_priority(true); 
    }
      
    // Workaround for RRC_CONNECTED
    //    if ((mac_report.rlc_report(i).lc_id() == 1)) {
    //  if (mac_report.rlc_report(i).tx_queue_size() > 0) {
    //	ue_sched_info->is_high_priority(true);
    //  }
    //}

    total_buffer_bytes += mac_report.rlc_report(i).tx_queue_size();
  
  }

  if (total_buffer_bytes > 0) {
    if (mcs  == 0) {
      ue_sched_info->set_nb_rbs_required(cell_config.cell_id(), 4);
    } else {
      ue_sched_info->set_nb_rbs_required(cell_config.cell_id(), get_min_rb_unit(cell_config));
    }

    TBS = get_TBS_DL(mcs, ue_sched_info->get_nb_rbs_required(cell_config.cell_id()));
    
    // Calculate the required number of RBs for the UE
    while (TBS < total_buffer_bytes) {
      ue_sched_info->set_nb_rbs_required(cell_config.cell_id(),
					 ue_sched_info->get_nb_rbs_required(cell_config.cell_id()) + get_min_rb_unit(cell_config));

      if (ue_sched_info->get_nb_rbs_required(cell_config.cell_id()) >  cell_config.dl_bandwidth()) {
	TBS = get_TBS_DL(mcs, cell_config.dl_bandwidth());
	ue_sched_info->set_nb_rbs_required(cell_config.cell_id(), cell_config.dl_bandwidth());
	break;
      }
      TBS = get_TBS_DL(mcs, ue_sched_info->get_nb_rbs_required(cell_config.cell_id()));
    }
    //std::cout << "RNTI " << ue_config.rnti() << ": For this MCS, " << ue_sched_info->get_nb_rbs_required(cell_config.cell_id()) << " rbs are required and the TBS is " << (int) TBS <<  std::endl;
  }
}

const std::shared_ptr<std::vector<int>> flexran::app::scheduler::remote_scheduler_helper::sort_UEs(const protocol::flex_cell_config& cell_config,
											     const protocol::flex_ue_config_reply& ue_configs,
											     const ::std::shared_ptr<rib::enb_rib_info> agent_config) {

  int num_ues;

  ue_stats ue_list[flexran::rib::MAX_NUM_UE] = {};
  
  //Find how many UEs we actually have
  num_ues = ue_configs.ue_config_size();

  //Fill in the stats for the available UEs
  for (int i = 0; i < num_ues; i++) {
    const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    ue_list[i].ue_id = i;

    uint16_t cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());

      // Find if the UE has a harq available. If not, there is no point in continuing
      if (!ue_mac_info->has_available_harq(cell_id)) {
	continue;
      }

      // Get the active HARQ process
      int harq_pid = ue_mac_info->get_next_available_harq(cell_id);
      int round = ue_mac_info->get_harq_stats(cell_id, harq_pid);

      // Set the harq_round
      ue_list[i].harq_round = round;

      // Check MAC-related stats (cqi, rlc etc)
      protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();

      // Fill in the cqi
      for (int j = 0; j < mac_report.dl_cqi_report().csi_report_size(); j++) {
	if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(j).serv_cell_index()) {
	  ue_list[i].cqi = mac_report.dl_cqi_report().csi_report(j).p10csi().wb_cqi();
	  break;
	}
      }

      // Go through the logical channels of the UE and find how many bytes need to be transmitted in total
      // and for SRB1 and 2 and fill the corresponding fields of the ue_stats array
      for (int j = 0; j < mac_report.rlc_report_size(); j++) {

	// If we have data for SRB1 or 2
	if ((mac_report.rlc_report(j).lc_id() == 1) || (mac_report.rlc_report(j).lc_id() == 2)) {
	  ue_list[i].bytes_in_ccch += mac_report.rlc_report(j).tx_queue_size();
	}

	// Fill in the total bytes that the UE needs to transfer
	if (mac_report.rlc_report(j).has_tx_queue_size()) {
	    ue_list[i].total_bytes_in_buffers += mac_report.rlc_report(j).tx_queue_size();
	}
	// Fill in the maximum hol delay for any lc_id
	if (mac_report.rlc_report(j).has_tx_queue_hol_delay()) {
	  ue_list[i].hol_delay = std::max(ue_list[i].hol_delay, (int) mac_report.rlc_report(j).tx_queue_hol_delay());
	}
      } 
    }
  }

  // We now have all the required data. Let's sort the array
  std::sort(ue_list, ue_list+num_ues, &compare_stats);

  //we now need to return the sorted ids of the UEs
  auto sorted_ue_ids = std::make_shared<std::vector<int>>();

  for (int i = 0; i < num_ues; i++) {
    sorted_ue_ids->push_back(ue_list[i].ue_id);
  }
  
  return sorted_ue_ids;
  
}

bool flexran::app::scheduler::remote_scheduler_helper::compare_stats(const ue_stats& a,
								     const ue_stats& b) {

  if (a.harq_round > b.harq_round) return true;
  if (a.harq_round < b.harq_round) return false;

  if (a.bytes_in_ccch > b.bytes_in_ccch) return true;
  if (a.bytes_in_ccch < b.bytes_in_ccch) return false;

  if (a.hol_delay > b.hol_delay) return true;
  if (a.hol_delay < b.hol_delay) return false;

  if (a.total_bytes_in_buffers > b.total_bytes_in_buffers) return true;
  if (a.total_bytes_in_buffers < b.total_bytes_in_buffers) return false;

  if (a.cqi > b.cqi) return true;
  if (a.cqi < b.cqi) return false;
  
  return true;
}
