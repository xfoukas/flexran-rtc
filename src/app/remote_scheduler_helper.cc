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

#include "remote_scheduler_helper.h"
#include "remote_scheduler.h"
#include "remote_scheduler_primitives.h"

void flexran::app::scheduler::run_dlsch_scheduler_preprocessor(const protocol::flex_cell_config& cell_config,
							       const protocol::flex_ue_config_reply& ue_configs,
							       const protocol::flex_lc_config_reply& lc_configs,
							       std::shared_ptr<const flexran::rib::enb_rib_info> agent_config,
							       std::shared_ptr<enb_scheduling_info> sched_info,
							       flexran::rib::subframe_t subframe) {

  uint16_t total_nb_available_rb;
  uint16_t min_rb_unit;
  uint16_t nb_rbs_required[rib::MAX_NUM_UE];
  int total_ue_count;
  uint16_t average_rbs_per_user = 0;
  int transmission_mode;


  total_nb_available_rb = cell_config.dl_bandwidth();
  //Remove the RBs used by common channels
  //TODO: For now we will do this manually based on OAI config and scheduling sf number. Important to fix it later.
  // Assume an FDD scheduler
  if (subframe == 0) {
    total_nb_available_rb -= 4;
  } else if (subframe == 5) {
    total_nb_available_rb -=8;
  }
  
  min_rb_unit = get_min_rb_unit(cell_config);

  // Find the active UEs for this cell
  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
      
      // Check to see if there is a scheduling configuration created for this UE and if not create it
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      if (!ue_sched_info) {
	sched_info->create_ue_scheduling_info(ue_config.rnti());
	ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      } else {
	ue_sched_info->start_new_scheduling_round(cell_id, ue_mac_info); // reset scheduling-related values 
      }
      
      const protocol::flex_lc_ue_config *lc_conf = nullptr;

      for (int k = 0; k < lc_configs.lc_ue_config_size(); k++) {
	lc_conf = &(lc_configs.lc_ue_config(k));
	if (lc_conf->rnti() == ue_config.rnti()) {
	  break;
	}
      } 
      
      if (lc_conf != nullptr) {
	//Calculate the number of RBs required by each UE based on their logical channels' buffer status
	assign_rbs_required(ue_sched_info, ue_mac_info, cell_config, ue_config, *lc_conf);
      }
    }
  }

  //std::cout << "Got here" << std::endl;
  //TODO: Sort the scheduled UEs based on their buffer size. For now apply unfair scheduling. FIFO priority
  total_ue_count = 0;

  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());

      // Get the scheduling info
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

      // Get the active HARQ process
      int harq_pid = ue_mac_info->get_currently_active_harq(cell_id);
      //int harq_pid = ue_sched_info->get_active_harq_pid();

      int round = ue_sched_info->get_harq_round(cell_id, harq_pid);
      // Check if round needs to be increased
      int status = ue_mac_info->get_harq_stats(cell_id, harq_pid);
      if (status == protocol::FLHS_ACK) {
	round = 0;
      } else {
	round++;
	round = round % 4;
      }
      ue_sched_info->set_harq_round(cell_id, harq_pid, round);

      if (status == protocol::FLHS_NACK) {
	ue_sched_info->set_nb_rbs_required(cell_id, ue_sched_info->get_nb_scheduled_rbs(cell_id, harq_pid));
      } else {
	ue_sched_info->set_nb_scheduled_rbs(cell_id, harq_pid, 0);
      }

      if (ue_sched_info->get_nb_rbs_required(cell_id) > 0) {
	total_ue_count = total_ue_count + 1;
      }

      
      
      if (total_ue_count == 0) {
	average_rbs_per_user = 0;
      } else if ((get_min_rb_unit(cell_config) * total_ue_count) <= total_nb_available_rb) {
	average_rbs_per_user = total_nb_available_rb/total_ue_count;
      } else {
	average_rbs_per_user = get_min_rb_unit(cell_config);
      }
    }
  }

  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.cell_id();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
      
      // Get the scheduling info
      ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

      if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0 ) {
	continue;
      }
  
      if (ue_sched_info->is_high_priority()) {
	//std::cout << "Is high-priority UE: " << ue_config.rnti() << std::endl;
	ue_sched_info->set_nb_rbs_required_remaining1(cell_id, ue_sched_info->get_nb_rbs_required(cell_id));
      } else {
	 ue_sched_info->set_nb_rbs_required_remaining1(cell_id, ::std::min(average_rbs_per_user,
									   ue_sched_info->get_nb_rbs_required(cell_id)));
      }
    }
  }

 for (int r1 = 0; r1 < 2; r1++) {
    // Go through each of the UEs that need to be scheduled
    for (int i = 0; i < ue_configs.ue_config_size(); i++) {
      const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
      int cell_id = cell_config.cell_id();
      // If this UE is assigned to this cell
      if (ue_config.pcell_carrier_index() == cell_id) {
	// Get the MAC stats for this UE
	::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	
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
      bool scheduled_hp = false;

      for (int i = 0; i < ue_configs.ue_config_size(); i++) {
	const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
	int cell_id = cell_config.cell_id();
	// If this UE is assigned to this cell
	if (ue_config.pcell_carrier_index() == cell_id) {
	  // Get the MAC stats for this UE
	  ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  ::std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
	  transmission_mode = ue_config.transmission_mode();
	  
	  if (!ue_sched_info->is_high_priority()) {
	    continue;
	  }

	  if (ue_sched_info->get_nb_rbs_required(cell_id) <= 0) {
	    continue;
	  }

	  scheduled_hp = true;
	  //std::cout << "Need to schedule a high priority UE:" << ue_config.rnti() << std::endl;
	  //std::cout << "Was allocated " << ue_sched_info->get_nb_rbs_required(cell_id) << " rbs" << std::endl;

	  perform_pre_processor_allocation(cell_config,
					   ue_config,
					   sched_info,
					   ue_sched_info,
					   transmission_mode);
	}
      }
      
      if (!scheduled_hp) {
      
	for (int i = 0; i < ue_configs.ue_config_size(); i++) {
	  const protocol::flex_ue_config ue_config = ue_configs.ue_config(i);
	  int cell_id = cell_config.cell_id();
	  // If this UE is assigned to this cell
	  if (ue_config.pcell_carrier_index() == cell_id) {
	    // Get the MAC stats for this UE
	    ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	    
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
					     ue_config,
					     sched_info,
					     ue_sched_info,
					     transmission_mode);
	  }
	}
      }
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

void flexran::app::scheduler::perform_pre_processor_allocation(const protocol::flex_cell_config& cell_config,
							       const protocol::flex_ue_config& ue_config,
							       std::shared_ptr<enb_scheduling_info> sched_info,
							       std::shared_ptr<ue_scheduling_info> ue_sched_info,
							       int transmission_mode) {

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

void flexran::app::scheduler::assign_rbs_required(::std::shared_ptr<ue_scheduling_info> ue_sched_info,
						  ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info,
						  const protocol::flex_cell_config& cell_config,
						  const protocol::flex_ue_config& ue_config,
						  const protocol::flex_lc_ue_config& lc_ue_config) {
  uint16_t TBS = 0;
  
  //std::cout << "Doing this for UE: " << ue_config.rnti() << std::endl;

  //Compute the mcs of the UE for this cell
  int mcs = 0;
  const protocol::flex_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();
  for (int i = 0; i < mac_report.dl_cqi_report().csi_report_size(); i++) {
    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(i).serv_cell_index()) {
      mcs = rib::cqi_to_mcs[mac_report.dl_cqi_report().csi_report(i).p10csi().wb_cqi()];
      mcs = std::min(mcs, target_dl_mcs_);
      break;
    }
  }

  int total_buffer_bytes = 0;
  // Go through the logical channels of the UE and find how many bytes need to be transmitted in total
  for (int i = 0; i < mac_report.rlc_report_size(); i++) {
    //std::cout << "We have an RLC report for this" <<  std::endl;
   
    if (lc_ue_config.lc_config_size() > 2) {
      if (ue_sched_info->is_high_priority()) {
	//std::cout << "No longer high priority" << std::endl;
      }
      ue_sched_info->is_high_priority(false);
    }

    
    // Workaround for RRC_CONNECTED
    if ((mac_report.rlc_report(i).lc_id() == 1)) {
      if (mac_report.rlc_report(i).tx_queue_size() > 0) {
	ue_sched_info->is_high_priority(true);
      }
    }

    total_buffer_bytes += mac_report.rlc_report(i).tx_queue_size();
  }

  if (total_buffer_bytes > 0) {
    //std::cout << "Have something to transmit" << std::endl;
    if (mcs  == 0) {
      ue_sched_info->set_nb_rbs_required(cell_config.cell_id(), 4);
    } else {
      ue_sched_info->set_nb_rbs_required(cell_config.cell_id(), get_min_rb_unit(cell_config));
    }

    TBS = get_TBS_DL(mcs, ue_sched_info->get_nb_rbs_required(cell_config.cell_id()));
    
    //std::cout << "MCS is " << (int) mcs << std::endl;
    

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
