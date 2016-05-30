#include <iostream>

#include "remote_scheduler.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "progran.pb.h"
#include "rib_common.h"
#include "cell_mac_rib_info.h"

int32_t progran::app::scheduler::remote_scheduler::tpc_accumulated = 0;

void progran::app::scheduler::remote_scheduler::run_periodic_task() {

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
  
  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

  for (const auto& agent_id : agent_ids) {

    protocol::progran_message out_message;
    
    ::std::shared_ptr<const rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
    const protocol::prp_enb_config_reply& enb_config = agent_config->get_enb_config();
    const protocol::prp_ue_config_reply& ue_configs = agent_config->get_ue_configs();
    const protocol::prp_lc_config_reply& lc_configs = agent_config->get_lc_configs();

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

    // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
    if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
      continue;
    }
    target_subframe = (current_subframe + schedule_ahead) % 10;
    if (target_subframe < current_subframe) {
      target_frame = (current_frame + 1) % 1024;
    } else {
      target_frame = current_frame;
    }
    int additional_frames = schedule_ahead / 10;
    target_frame = (target_frame + additional_frames) % 1024;

    //if ((target_subframe != 1) && (target_subframe != 4) && (target_subframe != 6) && (target_subframe != 8)) {
    //  continue;
    //}
    

    if ((target_subframe  == 0) || (target_subframe == 5)) {
      continue;
    }
    //std::cout << "Scheduling for frame " << target_frame << " and subframe " << target_subframe << std::endl;

    // Create dl_mac_config message header
    protocol::prp_header *header(new protocol::prp_header);
    header->set_type(protocol::PRPT_DL_MAC_CONFIG);
    header->set_version(0);
    header->set_xid(0);

    // Create dl_mac_config message
    protocol::prp_dl_mac_config *dl_mac_config_msg(new protocol::prp_dl_mac_config);
    dl_mac_config_msg->set_allocated_header(header);
    dl_mac_config_msg->set_sfn_sf(rib::get_sfn_sf(target_frame, target_subframe));
    
    aggregation = 2;
    
    // Go through the cell configs and set the variables
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::prp_cell_config cell_config = enb_config.cell_config(i);
      total_nb_available_rb[i] = cell_config.dl_bandwidth();
      //Remove the RBs used by common channels
      //TODO: For now we will do this manually based on OAI config and scheduling sf. Important to fix it later.
      // Assume an FDD scheduler
      // switch(target_subframe) {
      // case 0:
      // 	total_nb_available_rb[i] -= 4;
      // 	break;
      // case 5:
      // 	total_nb_available_rb[i] -= 8;
      // 	break;
      // }
      enb_sched_info->start_new_scheduling_round(target_subframe, cell_config);

      // Run the preprocessor to make initial allocation of RBs to UEs (Need to do this over all scheduling_info of eNB)
      run_dlsch_scheduler_preprocessor(cell_config, ue_configs, lc_configs, agent_config, enb_sched_info, target_subframe);
    }

    // Go through the cells and schedule the UEs of this cell
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::prp_cell_config cell_config = enb_config.cell_config(i);
      int cell_id = cell_config.cell_id();
      

      for (int UE_id = 0; UE_id < ue_configs.ue_config_size(); UE_id++) {
	const protocol::prp_ue_config ue_config = ue_configs.ue_config(UE_id);
	if (ue_config.pcell_carrier_index() == cell_id) {

	  // Get the MAC stats for this UE
	  ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(ue_config.rnti());
	  
	  // Get the scheduling info
	  ::std::shared_ptr<ue_scheduling_info> ue_sched_info = enb_sched_info->get_ue_scheduling_info(ue_config.rnti());
	  // if (ue_sched_info) {
	  //   ue_sched_info->start_new_scheduling_round();
	  // } else { // we need to create the scheduling info
	  //   enb_sched_info->create_ue_scheduling_info(ue_config.rnti());
	  //   ue_sched_info = enb_sched_info->get_ue_scheduling_info(ue_config.rnti());
	  // }
	  // Check if we have stats for this UE
	  if (!ue_mac_info) {
	    continue;
	  }
	  
	  const protocol::prp_ue_stats_report& mac_report = ue_mac_info->get_mac_stats_report();

	  // Schedule this UE
	  // Check if the preprocessor allocated rbs for this and if
	  // CCE allocation is feasible
	  if ((ue_sched_info->get_pre_nb_rbs_available(cell_id) == 0) ||
	      CCE_allocation_infeasible(enb_sched_info, cell_config, ue_config, aggregation, target_subframe)) {
	    continue;
	  }


	  nb_available_rb = ue_sched_info->get_pre_nb_rbs_available(cell_id);
	  harq_pid = ue_mac_info->get_currently_active_harq(cell_id);
	  //harq_pid = ue_sched_info->get_active_harq_pid();
	  
	  //std::cout << "The current harq_pid is " << (int) harq_pid << std::endl;

	  round = ue_sched_info->get_harq_round(cell_id, harq_pid);
	  
	  sdu_length_total = 0;

	  for (int j = 0; j < mac_report.dl_cqi_report().csi_report_size(); j++) {
	    if (cell_config.cell_id() == mac_report.dl_cqi_report().csi_report(j).serv_cell_index()) {
	      mcs = rib::cqi_to_mcs[mac_report.dl_cqi_report().csi_report(j).p10csi().wb_cqi()];
	      break;
	    }
	  }

	  mcs = ::std::min(mcs, target_dl_mcs_);

	  // Create a dl_data message
	  protocol::prp_dl_data *dl_data = dl_mac_config_msg->add_dl_ue_data();

	  int status = ue_mac_info->get_harq_stats(cell_id, harq_pid);
	  if (status == protocol::PRHS_NACK) {
	    // Use the MCS that was previously assigned to this HARQ process
	    mcs = ue_sched_info->get_mcs(cell_id, harq_pid);
	    nb_rb = ue_sched_info->get_nb_rbs_required(cell_id);

	    dci_tbs = get_TBS_DL(mcs, nb_rb);

	    if (nb_rb <= nb_available_rb) {
	      if (nb_rb == nb_available_rb) {
		//Set the already allocated subband allocation
		for (int j = 0; j < get_nb_rbg(cell_config); j++) {
		  ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							   harq_pid,
							   j,
							   ue_sched_info->get_rballoc_sub(cell_id, j));
		}
	      } else {
		nb_rb_tmp = nb_rb;
		int j = 0;

		while ((nb_rb_tmp > 0) && (j < get_nb_rbg(cell_config))) {
		  if (ue_sched_info->get_rballoc_sub(cell_id, j) == 1) {
		    ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							     harq_pid,
							     j,
							     ue_sched_info->get_rballoc_sub(cell_id, j));
		    if ((j == get_nb_rbg(cell_config) - 1) &&
			((cell_config.dl_bandwidth() == 25) ||
			 (cell_config.dl_bandwidth() == 50))) {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config) + 1;
		    } else {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config);
		    }
		  }
		  j++;
		}
	      }

	      nb_available_rb -= nb_rb;

	      // TODO: Set this statically for now
	      aggregation = 2;
	      ndi = ue_sched_info->get_ndi(cell_id, harq_pid);
	      tpc = 1;
	      ue_has_transmission = true;
	    } else {
	      // Do not schedule. The retransmission takes more resources than what we have
	      ue_has_transmission = false;
	    }
	  } else { /* This is potentially a new SDU opportunity */	    
	    TBS = get_TBS_DL(mcs, nb_available_rb);
	    dci_tbs = TBS;
	    
	    if (ue_sched_info->get_ta_timer() == 0) {
	      // Check if we need to update
	      ue_sched_info->set_ta_timer(20);
	      if (mac_report.pending_mac_ces() & protocol::PRPCET_TA) {
		ta_len = 2;
	      } else {
		ta_len = 0;
	      }
	    } else {
	      ue_sched_info->decr_ta_timer();
	      ta_len = 0;
	    }

	    if (ta_len > 0) {
	      ce_flags |= protocol::PRPCET_TA;
	    }

	    header_len_dcch = 2; // 2 bytes DCCH SDU subheader
	    
	    // Loop through the UE logical channels
	    for (int j = 1; j < mac_report.rlc_report_size() + 1; j++) {
	      header_len += 3;
	      const protocol::prp_rlc_bsr& rlc_report = mac_report.rlc_report(j-1);

	      if (dci_tbs - ta_len - header_len > 0) {
		//std::cout << "Need to request " << rlc_report.tx_queue_size() << " from channel " << j << std::endl;
		if (rlc_report.tx_queue_size() > 0) {
		  data_to_request = ::std::min(dci_tbs - ta_len - header_len, rlc_report.tx_queue_size());
		  if (data_to_request < 128) { // The header will be one byte less
		    header_len--;
		  }
		  if (j == 1) {
		    data_to_request++; // It is not correct but fixes some RLC bug for DCCH
		  }
		  
		  protocol::prp_rlc_pdu *rlc_pdu = dl_data->add_rlc_pdu();
		  protocol::prp_rlc_pdu_tb *tb1 = rlc_pdu->add_rlc_pdu_tb();
		  protocol::prp_rlc_pdu_tb *tb2 = rlc_pdu->add_rlc_pdu_tb();
		  tb1->set_logical_channel_id(rlc_report.lc_id());
		  tb2->set_logical_channel_id(rlc_report.lc_id());
		  tb1->set_size(data_to_request);
		  tb2->set_size(data_to_request);
		  //Set this to the max value that we might request
		  sdu_length_total = data_to_request;
		} else {
		  header_len -= 3;
		} //End tx_queue_size == 0
	      } // end of if dci_tbs - ta_len - header_len > 0
	      
	    } // End of iterating the logical channels

	    // There is a payload
	    if ( dl_data->rlc_pdu_size() > 0) {
	      // Now compute the number of required RBs for total sdu length
	      // Assume RAH format 2
	      //Adjust header lengths
	      header_len_tmp = header_len;

	      if (header_len == 2 || header_len == 3) { // Only one SDU, remove length field
		header_len = 1;
	      } else { //Remove length field from the last SDU
		header_len--;
	      }

	      mcs_tmp = mcs;

	      if (mcs_tmp == 0) {
		nb_rb = 4; // don't let the TBS get too small
	      } else {
		nb_rb = get_min_rb_unit(cell_config);
	      }

	      TBS = get_TBS_DL(mcs_tmp, nb_rb);

	      while (TBS < (sdu_length_total + header_len + ta_len)) {
		nb_rb += get_min_rb_unit(cell_config);
		if (nb_rb > nb_available_rb) { // If we've gone beyond the maximum number of RBs
		  TBS = get_TBS_DL(mcs_tmp, nb_available_rb);
		  nb_rb = nb_available_rb;
		  break;
		}
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      if (nb_rb == ue_sched_info->get_pre_nb_rbs_available(cell_id)) {
		// We have the exact number of RBs required. Just fill the rballoc subband
		for (int j = 0; j < get_nb_rbg(cell_config); j++) {
		  ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							   harq_pid,
							   j,
							   ue_sched_info->get_rballoc_sub(cell_id, j));
		}
	      } else {
		nb_rb_tmp = nb_rb;
		int j = 0;
		while ((nb_rb_tmp > 0) && (j < get_nb_rbg(cell_config))) {
		  if (ue_sched_info->get_rballoc_sub(cell_id, j) == 1) {
		    ue_sched_info->set_rballoc_sub_scheduled(cell_id,
							     harq_pid,
							     j,
							     1);
		    if ((j == get_nb_rbg(cell_config) - 1) &&
			((cell_config.dl_bandwidth() == 25) ||
			 (cell_config.dl_bandwidth() == 50))) {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config) + 1;
		    } else {
		      nb_rb_tmp = nb_rb_tmp - get_min_rb_unit(cell_config);
		    }
		  }
		  j++;
		}
	      }

	      // decrease MCS until TBS falls below required length
	      while ((TBS > (sdu_length_total + header_len + ta_len)) && (mcs_tmp > 0)) {
		mcs_tmp--;
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      // If we have decreased too much we don't have enough RBs, increase MCs
	      while ((TBS < (sdu_length_total + header_len + ta_len)) && (mcs_tmp < 28)) {
		     // (((ue_sched_info->get_dl_power_offset(cell_id) > 0) && (mcs_tmp < 28)) ||
		     // ((ue_sched_info->get_dl_power_offset(cell_id) == 0) && (mcs_tmp <= 15)))) {
		mcs_tmp++;
		TBS = get_TBS_DL(mcs_tmp, nb_rb);
	      }

	      dci_tbs = TBS;
	      mcs = mcs_tmp;
	      
	      //	      std::cout << "Decided MCS, nb_rb and TBS are " << mcs << " " << nb_rb << " " << dci_tbs << std::endl;
	      // Update the mcs used for this harq process
	      ue_sched_info->set_mcs(cell_id, harq_pid, mcs);

	      ue_sched_info->set_nb_scheduled_rbs(cell_id, harq_pid, nb_rb);

	      // do PUCCH power control
	      // This is the normalized RX power
	      const rib::cell_mac_rib_info& cell_rib_info = agent_config->get_cell_mac_rib_info(cell_id);
	      const protocol::prp_cell_stats_report& cell_report = cell_rib_info.get_cell_stats_report();

	      int16_t normalized_rx_power;
	      bool has_normalized_rx_power = false;
	      
	      for (int k = 0; k < mac_report.ul_cqi_report().pucch_dbm_size(); k++) {
		if (mac_report.ul_cqi_report().pucch_dbm(k).serv_cell_index() == cell_id) {
		  if (mac_report.ul_cqi_report().pucch_dbm(k).has_p0_pucch_dbm()) {
		    normalized_rx_power = mac_report.ul_cqi_report().pucch_dbm(k).p0_pucch_dbm();
		    has_normalized_rx_power = true;
		    break;
		  }
		}
	      }
	      
	      int16_t target_rx_power = cell_report.noise_inter_report().p0_nominal_pucch() + 10;

	      int32_t framex10psubframe = ue_sched_info->get_pucch_tpc_tx_frame()*10 + ue_sched_info->get_pucch_tpc_tx_subframe();

	      if (((framex10psubframe+10) <= (target_frame*10 + target_subframe)) || // normal case
		  ((framex10psubframe > (target_frame*10 + target_subframe)) && (((10240 - framex10psubframe + target_frame*10+target_subframe) >= 10 )))) {// Frame wrap-around

		if (has_normalized_rx_power) {
		  ue_sched_info->set_pucch_tpc_tx_frame(target_frame);
		  ue_sched_info->set_pucch_tpc_tx_subframe(target_subframe);
		  if (normalized_rx_power > (target_rx_power+1)) {
		    tpc = 0; //-1
		    tpc_accumulated--;
		  } else if (normalized_rx_power < (target_rx_power - 1)) {
		    tpc = 2; //+1
		    tpc_accumulated++;
		  } else {
		    tpc = 1; //0
		  }
		} else {// Po_PUCCH has been updated
		  tpc = 1;
		} // time to do TPC update
	      } else {
		tpc = 1; //0
	      }
	      ue_sched_info->toggle_ndi(cell_id, harq_pid);
	      ndi = ue_sched_info->get_ndi(cell_id, harq_pid);
	      //std::cout << "Toggling NDI for harq " << (int) harq_pid << std::endl;
	      ue_has_transmission = true;
	    } else { // There is no data to transmit, so don't schedule
	      ue_has_transmission = false;
	    }
	  }
	  
	  // If we had a new transmission or retransmission
	  if (ue_has_transmission) {
	    // After this point all UEs will be scheduled
	    dl_data->set_rnti(ue_config.rnti());
	    dl_data->set_serv_cell_index(cell_id);

	    // Add the control element flags to the progran message
	    dl_data->add_ce_bitmap(ce_flags);
	    dl_data->add_ce_bitmap(ce_flags);

	    protocol::prp_dl_dci *dl_dci(new protocol::prp_dl_dci);
	    dl_data->set_allocated_dl_dci(dl_dci);
	    
	    dl_dci->set_rnti(ue_config.rnti());
	    dl_dci->set_harq_process(harq_pid);

	    // TODO: Currently set to static value. Need to create a function to obtain this
	    aggregation = 2;
	    dl_dci->set_aggr_level(aggregation);
	    
	    enb_sched_info->assign_CCE(cell_id, 1<<aggregation);
	    
	    //ue_sched_info->is_high_priority(false);

	    //std::cout << "Rballoc for UE " << ue_config.rnti() << std::endl;
	    //std::cout << "Allocated " << nb_rb << " RBs " << std::endl;
	    //std::cout << "Harq pid/round: " << (int) harq_pid << "/" << (int) round << std::endl;
	    //std::cout << "Ndi: " << (int) ndi << std::endl;
	    //std::cout << "TPC: " << (int) tpc << std::endl;
	    //std::cout << "MCS: " << (int) mcs << std::endl;
	    //std::cout << "TBS: " << (int) dci_tbs << std::endl;
	    //std::cout << "Frame: " << (int) target_frame << " Subframe: " << (int) target_subframe << std::endl;
	    //std::cout << "Test: " << (int) get_TBS_DL(23, 2) << std::endl;
	    
	    //for (int j = 0; j <  get_nb_rbg(cell_config); j++) {
	    //  std::cout << (int) ue_sched_info->get_rballoc_sub_scheduled(cell_id, harq_pid, j);
	    //}
	    //std::cout << std::endl;

	    switch(ue_config.transmission_mode()) {
	    case 1:
	    case 2:
	    default:
	      dl_dci->set_res_alloc(0);
	      dl_dci->set_vrb_format(protocol::PRVRBF_LOCALIZED);
	      dl_dci->set_format(protocol::PRDCIF_1);
	      dl_dci->set_rb_shift(0);
	      dl_dci->add_ndi(ndi);
	      dl_dci->add_rv(round);
	      dl_dci->set_tpc(tpc);
	      dl_dci->add_mcs(mcs);
	      dl_dci->add_tbs_size(dci_tbs);
	      dl_dci->set_rb_bitmap(allocate_prbs_sub(nb_rb,
						      ue_sched_info->get_rballoc_sub_scheduled(cell_id,harq_pid),
						      cell_config));
	    }
	  } else {
	    dl_mac_config_msg->mutable_dl_ue_data()->RemoveLast();
	  }
	}
      }
    }
    // Done with scheduling of eNB UEs. Set the last scheduled frame and subframe
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);

    // Create and send the progran message
    out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
    out_message.set_allocated_dl_mac_config_msg(dl_mac_config_msg);
    if (dl_mac_config_msg->dl_ue_data_size() > 0) {
      // std::cout << "Scheduled " << dl_mac_config_msg->dl_ue_data_size() << " UEs in this round\n" << std::endl;
      req_manager_.send_message(agent_id, out_message);

    }
  }
}

std::shared_ptr<progran::app::scheduler::enb_scheduling_info>
progran::app::scheduler::remote_scheduler::get_scheduling_info(int agent_id) {
  auto it = scheduling_info_.find(agent_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return ::std::shared_ptr<enb_scheduling_info>(nullptr);
}
