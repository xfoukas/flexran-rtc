#include "remote_scheduler.h"
#include "progran.pb.h"
#include "rib_common.h"

#include <iostream>


void remote_scheduler::run_periodic_task() {

  frame_t target_frame;
  subframe_t target_subframe;
  
  unsigned char aggregation;
  uint16_t min_rb_unit[MAX_NUM_CC];
  uint16_t total_nb_available_rb[MAX_NUM_CC];
  int N_RBG[MAX_NUM_CC];

  std::set<int> agent_ids = std::move(rib_.get_available_agents());

  for (const auto& agent_id : agent_ids) {
    std::shared_ptr<const enb_rib_info> agent_config = rib_.get_agent(agent_id);
    const protocol::prp_enb_config_reply& enb_config = agent_config->get_enb_config();
    const protocol::prp_ue_config_reply& ue_configs = agent_config->get_ue_configs();
    const protocol::prp_lc_config_reply& lc_configs = agent_config->get_lc_configs();

    frame_t current_frame = agent_config->get_current_frame();
    subframe_t current_subframe = agent_config->get_current_subframe();
    
    // Check if scheduling context for this eNB is already present and if not create it
    std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
    if (enb_sched_info) {
     // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
      if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
	continue;
      }
      enb_sched_info->start_new_scheduling_round();
      target_subframe = (current_subframe + schedule_ahead) % 10;
      if (target_subframe < current_subframe) {
	target_frame = (target_frame + 1) % 1024;
      }
    } else { // eNB sched info was not found for this agent
      std::cout << "Config was not found. Creating" << std::endl;
      scheduling_info_.insert(std::pair<int,
			      std::shared_ptr<enb_scheduling_info>>(agent_id,
								    std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
      enb_sched_info = get_scheduling_info(agent_id);
    }

    aggregation = 1;
    
    // Go through the cell configs and set the variables
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::prp_cell_config cell_config = enb_config.cell_config(i);
      min_rb_unit[i] = get_min_rb_unit(cell_config);
      total_nb_available_rb[i] = cell_config.dl_bandwidth();
      //Remove the RBs used by common channels
      //TODO: For now we will do this manually based on OAI config and scheduling sf. Important to fix it later.
      // Assume an FDD scheduler
      switch(target_subframe) {
      case 0:
      case 5:
	total_nb_available_rb[i] -= 4;
	break;
      }
      // Get the number of resource block groups used for this cell configuration
      N_RBG[i] = get_nb_rbg(cell_config);

      // Run the preprocessor to make initial allocation of RBs to UEs (Need to do this over all scheduling_info of eNB)
      run_dlsch_scheduler_preprocessor(cell_config, ue_configs, agent_config, enb_sched_info);
    }

    /*TODO*/
    
    // Go through the UEs of this eNB and create a scheduling decision
    for (int i = 0; i < ue_configs.ue_config_size(); i++) {
      const protocol::prp_ue_config& config = ue_configs.ue_config(i);
      rnti_t rnti = config.rnti();

      // Get the MAC stats for the UE under study
      std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(rnti);

      
      
    }
    // Done with scheduling of eNB UEs. Set the last scheduled frame and subframe
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);
  }
}

void run_dlsch_scheduler_preprocessor(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config_reply& ue_configs,
				      std::shared_ptr<const enb_rib_info> agent_config,
				      std::shared_ptr<enb_scheduling_info> sched_info) {
  
  uint16_t min_rb_unit;
  uint16_t nb_rbs_required[MAX_NUM_UE];
  int total_ue_count;
  uint16_t average_rbs_per_user = 0;
  unsigned char rballoc_sub[N_RBG_MAX] = {0};
  int transmission_mode;

  min_rb_unit = get_min_rb_unit(cell_config);

  // Find the active UEs for this cell
  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::prp_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.carrier_index();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
      
      // Check to see if there is a scheduling configuration created for this UE and if not create it
      std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      if (!ue_sched_info) {
	sched_info->create_ue_scheduling_info(ue_config.rnti());
	ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
      } else {
	ue_sched_info->start_new_scheduling_round(); // reset scheduling-related values 
      }
      //Calculate the number of RBs required by each UE based on their logical channels' buffer status
      assign_rbs_required(ue_sched_info, ue_mac_info, cell_config, ue_config);
    }
  }

  //std::cout << "Got here" << std::endl;
  //TODO: Sort the scheduled UEs based on their buffer size. For now apply unfair scheduling. FIFO priority
  total_ue_count = 0;

  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::prp_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.carrier_index();
    // If this UE is assigned to this cell
    if (ue_config.pcell_carrier_index() == cell_id) {
      // Get the MAC stats for this UE
      std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());

      // Get the scheduling info
      std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

      // Get the active HARQ process
      int harq_pid = ue_sched_info->get_active_harq_pid();
      int round = ue_sched_info->get_harq_round(cell_id, harq_pid);
      // Check if round needs to be increased
      int status = ue_mac_info->get_harq_stats(cell_id, harq_pid);
      if (status == protocol::PRHS_ACK) {
	round = 0;
      } else {
	round++;
	round = round%3;
      }
      ue_sched_info->set_harq_round(cell_id, harq_pid, round);

      if (round > 0) {
	ue_sched_info->set_nb_rbs_required(cell_id, ue_sched_info->get_nb_scheduled_rbs(cell_id));
      } else {
	ue_sched_info->set_nb_scheduled_rbs(cell_id, 0);
      }

      if (ue_sched_info->get_nb_rbs_required(cell_id) > 0) {
	total_ue_count = total_ue_count + 1;
      }

      if (total_ue_count == 0) {
	average_rbs_per_user = 0;
      } else if ((get_min_rb_unit(cell_config) * total_ue_count) <= cell_config.dl_bandwidth()) {
	average_rbs_per_user = cell_config.dl_bandwidth()/total_ue_count;
      } else {
	average_rbs_per_user = get_min_rb_unit(cell_config);
      }
    }
  }

 for (int r1 = 0; r1 < 2; r1++) {
    // Go through each of the UEs that need to be scheduled
    for (int i = 0; i < ue_configs.ue_config_size(); i++) {
      const protocol::prp_ue_config ue_config = ue_configs.ue_config(i);
      int cell_id = cell_config.carrier_index();
      // If this UE is assigned to this cell
      if (ue_config.pcell_carrier_index() == cell_id) {
	// Get the MAC stats for this UE
	std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	
	// Get the scheduling info
	std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());

	if (r1 == 0) {
	  ue_sched_info->set_nb_rbs_required_remaining(cell_id, std::min(average_rbs_per_user,
									 ue_sched_info->get_nb_rbs_required(cell_id)));
	} else {
	  uint16_t nb_rem = ue_sched_info->get_nb_rbs_required(cell_id) - std::min(average_rbs_per_user,
										   ue_sched_info->get_nb_rbs_required(cell_id)) + ue_sched_info->get_nb_rbs_required_remaining(cell_id);
	    ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	}
      }
    }

    if (total_ue_count > 0) {
      // Go through all the UEs and allocate the resources in sched info
      for (int i = 0; i < ue_configs.ue_config_size(); i++) {
	const protocol::prp_ue_config ue_config = ue_configs.ue_config(i);
	int cell_id = cell_config.carrier_index();
	// If this UE is assigned to this cell
	if (ue_config.pcell_carrier_index() == cell_id) {
	  // Get the MAC stats for this UE
	  std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  std::shared_ptr<ue_scheduling_info> ue_sched_info = sched_info->get_ue_scheduling_info(ue_config.rnti());
	  transmission_mode = ue_config.transmission_mode();
	  
	  perform_pre_processor_allocation(cell_config,
					   ue_config,
					   sched_info,
					   ue_sched_info,
					   transmission_mode,
					   rballoc_sub);
	}
      }
    }
 }
  
}

void perform_pre_processor_allocation(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config& ue_config,
				      std::shared_ptr<enb_scheduling_info> sched_info,
				      std::shared_ptr<ue_scheduling_info> ue_sched_info,
				      int transmission_mode,
				      unsigned char rballoc_sub[N_RBG_MAX]) {

  int n_rbg = get_nb_rbg(cell_config);
  int min_rb_unit = get_min_rb_unit(cell_config);
  uint16_t cell_id = cell_config.cell_id();
  for (int i = 0; i < n_rbg; i++) {
    if ((rballoc_sub[i] == 0) &&
	(ue_sched_info->get_rballoc_sub(cell_id, i) == 0) &&
	(ue_sched_info->get_nb_rbs_required_remaining(cell_id) > 0) &&
	(sched_info->get_pre_nb_rbs_available(cell_id) <  ue_sched_info->get_nb_rbs_required(cell_id))) {
	  
      //TODO: No TM5 for now
      if ((i == n_rbg - 1) && ((cell_config.dl_bandwidth() == 25) || (cell_config.dl_bandwidth() == 50))) {
	rballoc_sub[i] = 1;
	ue_sched_info->set_rballoc_sub(cell_id, i, 1);
	int nb_rem = ue_sched_info->get_nb_rbs_required_remaining(cell_id) - min_rb_unit + 1;
	ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	nb_rem = sched_info->get_pre_nb_rbs_available(cell_id) + min_rb_unit - 1;
	sched_info->set_pre_nb_rbs_available(cell_id, nb_rem);
      } else {
	if (ue_sched_info->get_nb_rbs_required_remaining(cell_id) >= min_rb_unit) {
	  rballoc_sub[i] = 1;
	  ue_sched_info->set_rballoc_sub(cell_id, i, 1);
	  int nb_rem = ue_sched_info->get_nb_rbs_required_remaining(cell_id) - min_rb_unit;
	  ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	  nb_rem = sched_info->get_pre_nb_rbs_available(cell_id) + min_rb_unit;
	  sched_info->set_pre_nb_rbs_available(cell_id, nb_rem);
	}
      }
	  
    }
	
  }
  
}

void assign_rbs_required(std::shared_ptr<ue_scheduling_info> ue_sched_info,
			 std::shared_ptr<const ue_mac_rib_info> ue_mac_info,
			 const protocol::prp_cell_config& cell_config,
			 const protocol::prp_ue_config& ue_config) {
  uint16_t TBS = 0;
  
  //Compute the mcs of the UE for this cell
  int mcs = 0;
  const protocol::prp_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();
  for (int i = 0; i < mac_report.dl_cqi_report().csi_report_size(); i++) {
    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(i).serv_cell_index()) {
      mcs = cqi_to_mcs[mac_report.dl_cqi_report().csi_report(i).p10csi().wb_cqi()];
      break;
    }
  }

  int total_buffer_bytes = 0;
  // Go through the logical channels of the UE and find how many bytes need to be transmitted in total
  for (int i = 0; i < mac_report.rlc_report_size(); i++) {
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
  }
}

bool remote_scheduler::needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
					frame_t curr_frame, subframe_t curr_subframe) {
  frame_t last_checked_frame = enb_sched_info->get_last_checked_frame();
  subframe_t last_checked_subframe = enb_sched_info->get_last_checked_subframe();

  return !(last_checked_frame == curr_frame && last_checked_subframe == curr_subframe);
}

std::shared_ptr<enb_scheduling_info> remote_scheduler::get_scheduling_info(int agent_id) {
  auto it = scheduling_info_.find(agent_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return std::shared_ptr<enb_scheduling_info>(nullptr);
}

uint16_t get_min_rb_unit(const protocol::prp_cell_config& cell_config) {
  int min_rb_unit = 0;
  switch (cell_config.dl_bandwidth()) {
  case 6:
    min_rb_unit = 1;
    break;
  case 25:
    min_rb_unit = 2;
    break;
  case 50:
    min_rb_unit = 3;
    break;
  case 100:
    min_rb_unit = 4;
    break;
  default:
    min_rb_unit = 2;
    break;
  }
  return min_rb_unit;
}

uint16_t get_nb_rbg(const protocol::prp_cell_config& cell_config) {
  uint16_t nb_rbg;
  // Get the number of dl RBs
  switch(cell_config.dl_bandwidth()) {
  case 100:
    nb_rbg = 25;
    break;
  case 75:
    nb_rbg = 19;
    break;
  case 50:
    nb_rbg = 17;
    break;
  case 25:
    nb_rbg = 13;
    break;
  case 15:
    nb_rbg = 8;
    break;
  case 6:
    nb_rbg = 6;
    break;
  default:
    nb_rbg = 0;
    break;
  }
  return nb_rbg;
}

uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb) {

  uint32_t TBS;

  if ((nb_rb > 0) && (mcs < 29)) {
#ifdef TBS_FIX
    TBS = 3*TBStable[get_I_TBS(mcs)][nb_rb-1]/4;
    TBS = TBS>>3;
#else
    TBS = TBStable[get_I_TBS(mcs)][nb_rb-1];
    TBS = TBS>>3;
#endif
    return(TBS);
  } else {
    return(uint32_t)0;
  }
}

unsigned char get_I_TBS(unsigned char I_MCS) {

  if (I_MCS < 10)
    return(I_MCS);
  else if (I_MCS == 10)
    return(9);
  else if (I_MCS < 17)
    return(I_MCS-1);
  else if (I_MCS == 17)
    return(15);
  else return(I_MCS-2);

}
