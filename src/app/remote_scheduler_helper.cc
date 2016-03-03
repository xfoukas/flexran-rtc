#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"

void run_dlsch_scheduler_preprocessor(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config_reply& ue_configs,
				      std::shared_ptr<const enb_rib_info> agent_config,
				      std::shared_ptr<enb_scheduling_info> sched_info) {
  
  uint16_t min_rb_unit;
  uint16_t nb_rbs_required[MAX_NUM_UE];
  int total_ue_count;
  uint16_t average_rbs_per_user = 0;
  int transmission_mode;

  min_rb_unit = get_min_rb_unit(cell_config);

  // Find the active UEs for this cell
  for (int i = 0; i < ue_configs.ue_config_size(); i++) {
    const protocol::prp_ue_config ue_config = ue_configs.ue_config(i);
    int cell_id = cell_config.cell_id();
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
    int cell_id = cell_config.cell_id();
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
	ue_sched_info->set_nb_rbs_required(cell_id, ue_sched_info->get_nb_scheduled_rbs(cell_id, harq_pid));
      } else {
	ue_sched_info->set_nb_scheduled_rbs(cell_id, harq_pid, 0);
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
      int cell_id = cell_config.cell_id();
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
	int cell_id = cell_config.cell_id();
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
					   transmission_mode);
	}
      }
    }
 }
}

void perform_pre_processor_allocation(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config& ue_config,
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
	ue_sched_info->set_rballoc_sub(cell_id, i, 1);
	int nb_rem = ue_sched_info->get_nb_rbs_required_remaining(cell_id) - min_rb_unit + 1;
	ue_sched_info->set_nb_rbs_required_remaining(cell_id, nb_rem);
	nb_rem = ue_sched_info->get_pre_nb_rbs_available(cell_id) + min_rb_unit - 1;
	ue_sched_info->set_pre_nb_rbs_available(cell_id, nb_rem);
      } else {
	if (ue_sched_info->get_nb_rbs_required_remaining(cell_id) >= min_rb_unit) {
	  rballoc_sub[i] = 1;
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
