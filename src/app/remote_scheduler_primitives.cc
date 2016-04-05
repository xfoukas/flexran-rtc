#include "remote_scheduler_primitives.h"

bool progran::app::scheduler::needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
					       progran::rib::frame_t curr_frame,
					       progran::rib::subframe_t curr_subframe) {
  progran::rib::frame_t last_checked_frame = enb_sched_info->get_last_checked_frame();
  progran::rib::subframe_t last_checked_subframe = enb_sched_info->get_last_checked_subframe();
      
  return !(last_checked_frame == curr_frame && last_checked_subframe == curr_subframe);
}

uint16_t progran::app::scheduler::get_min_rb_unit(const protocol::prp_cell_config& cell_config) {
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

uint16_t progran::app::scheduler::get_nb_rbg(const protocol::prp_cell_config& cell_config) {
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

uint32_t progran::app::scheduler::get_TBS_DL(uint8_t mcs, uint16_t nb_rb) {

  uint32_t TBS;

  if ((nb_rb > 0) && (mcs < 29)) {
#ifdef TBS_FIX
    TBS = 3*progran::rib::TBStable[get_I_TBS(mcs)][nb_rb-1]/4;
    TBS = TBS>>3;
#else
    TBS = progran::rib::TBStable[get_I_TBS(mcs)][nb_rb-1];
    TBS = TBS>>3;
#endif
    return(TBS);
  } else {
    return(uint32_t)0;
  }
}

unsigned char progran::app::scheduler::get_I_TBS(unsigned char I_MCS) {

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

bool progran::app::scheduler::CCE_allocation_infeasible(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
							const protocol::prp_cell_config& cell_config,
							const protocol::prp_ue_config& ue_config,
							uint8_t aggregation,
							progran::rib::subframe_t subframe) {
  int allocation_is_feasible = 1;
  int nCCE_rem, nCCE_max;
  uint16_t cell_id = cell_config.cell_id();
  int common_dci = 0;
  // if ((subframe == 0) || (subframe == 5)) {
  //   common_dci = 1;
  // }
  

    
  //nCCE_max = get_nCCE_max(enb_sched_info->get_num_pdcch_symbols(cell_id),
  //			 cell_config,
  //			 subframe);
  nCCE_rem = enb_sched_info->get_nCCE_rem(cell_id);
  while (allocation_is_feasible == 1) {
    if ((1<<aggregation) > nCCE_rem) {
      if (enb_sched_info->get_num_pdcch_symbols(cell_id) == 3) {
	allocation_is_feasible = 0;
      } else {
	enb_sched_info->increase_num_pdcch_symbols(cell_config,
						   subframe);
	nCCE_rem = enb_sched_info->get_nCCE_rem(cell_id);
	//nCCE_max = get_nCCE_max(enb_sched_info->get_num_pdcch_symbols(cell_id),
	//			cell_config,
	//			subframe);
      }
      continue;
    } else {
      // Since assignment is feasible, get the index
      nCCE_max = get_nCCE_max(enb_sched_info->get_num_pdcch_symbols(cell_id),
			      cell_config,
			      subframe);
      if (get_nCCE_offset(1<<aggregation,
			  nCCE_max,
			  common_dci,
			  ue_config.rnti(),
			  subframe) >= 0) {
	//enb_sched_info->assign_CCE(cell_id, 1<<aggregation);
	return false;
      } else {
	if (enb_sched_info->get_num_pdcch_symbols(cell_id) == 3) {
	  allocation_is_feasible = 0;
	} else {
	  enb_sched_info->increase_num_pdcch_symbols(cell_config,
						     subframe);
	  nCCE_max = get_nCCE_max(enb_sched_info->get_num_pdcch_symbols(cell_id),
				  cell_config,
				  subframe);
	}
	continue;
      }
    }
  }
  if (allocation_is_feasible == 1) {
    enb_sched_info->assign_CCE(cell_id, 1<<aggregation);
    return false;
  }
  
  return true;
}

int progran::app::scheduler::get_nCCE_offset(const uint8_t aggregation,
					     const int nCCE,
					     const int common_dci,
					     const progran::rib::rnti_t rnti,
					     const progran::rib::subframe_t subframe) {
  int search_space_free, nb_candidates = 0;
  unsigned int Yk;

  /* TODO */
  // This should test a CCE allocation in correlation with the the other UEs and
  // common channels dcis
  
  if (common_dci == 1) {
    // TODO: For common search space
  } else { // For a UE DCI
    Yk = (unsigned int)rnti;

    for (int i = 0; i <= subframe; i++) {
      Yk = (Yk*39827)%65537;
    }

    Yk = Yk % (nCCE/aggregation);

    switch (aggregation) {
    case 1:
    case 2:
      nb_candidates = 6;
      break;
    case 4:
    case 8:
      nb_candidates = 2;
      break;
    default:
      break;
    }

    for (int m = 0; m < nb_candidates; m++) {
      search_space_free = 1;

      for (int l = 0; l < aggregation; l++) {

      }
      if (search_space_free == 1) {
	return (((Yk+m) % (nCCE/aggregation))*aggregation);
      }
    }

    return -1;
  }
  return -1;
}


uint16_t progran::app::scheduler::get_nCCE_max(uint8_t num_pdcch_symbols,
					       const protocol::prp_cell_config& cell_config,
					       progran::rib::subframe_t subframe) {
  uint8_t mi = get_mi(cell_config, subframe);
  return (get_nquad(num_pdcch_symbols, cell_config, mi)/9);
}

uint8_t progran::app::scheduler::get_phich_resource(const protocol::prp_cell_config& cell_config) {
  switch (cell_config.phich_resource()) {
    case protocol::PRPR_ONE_SIXTH:
      return 1;
      break;
    case protocol::PRPR_HALF:
      return 3;
      break;
    case protocol::PRPR_ONE:
      return 6;
      break;
    case protocol::PRPR_TWO:
      return 12;
      break;
    default:
      return 0;
  }
}

uint16_t progran::app::scheduler::get_nquad(uint8_t num_pdcch_symbols,
					    const protocol::prp_cell_config& cell_config,
					    uint8_t mi) {

  uint16_t n_reg = 0;
  uint8_t phich_resource = get_phich_resource(cell_config);

  uint8_t n_group_PHICH = (phich_resource * cell_config.dl_bandwidth())/48;

  if (((phich_resource * cell_config.dl_bandwidth())%48) > 0) {
    n_group_PHICH++;
  }

  if (cell_config.dl_cyclic_prefix_length() == protocol::PRDCPL_EXTENDED) {
    n_group_PHICH<<=1;
  }

  n_group_PHICH *= mi;

  if ((num_pdcch_symbols > 0) && (num_pdcch_symbols<4)) {
    switch(cell_config.dl_bandwidth()) {
    case 6:
      n_reg = 12 + (num_pdcch_symbols - 1) * 18;
      break;
    case 25:
      n_reg = 50 + (num_pdcch_symbols - 1) * 75;
      break;
    case 50:
      n_reg = 100 + (num_pdcch_symbols - 1) * 150;
      break;
    case 100:
      n_reg = 200 + (num_pdcch_symbols - 1) * 300;
      break;
    default:
      return 0;
    }
  }

  return (n_reg - 4 - (3 * n_group_PHICH));
}

uint8_t progran::app::scheduler::get_mi(const protocol::prp_cell_config& cell_config,
					progran::rib::subframe_t subframe) {

  if (cell_config.duplex_mode() == protocol::PRDM_FDD) {
    return 1;
  }

  /* TODO implement for TDD */
}

uint32_t progran::app::scheduler::allocate_prbs_sub(int nb_rb,
						    uint8_t *rballoc,
						    const protocol::prp_cell_config& cell_config) {
  int check=0;//check1=0,check2=0;
  uint32_t rballoc_dci=0;
  //uint8_t number_of_subbands=13;

  while((nb_rb >0) && (check < get_nb_rbg(cell_config))) {
    if(rballoc[check] == 1) {
      rballoc_dci |= (1<<((get_nb_rbg(cell_config)-1)-check));

      switch (cell_config.dl_bandwidth()) {
      case 6:
        nb_rb--;
        break;

      case 25:
        if (check == (get_nb_rbg(cell_config) - 1)) {
          nb_rb--;
        } else {
          nb_rb-=2;
        }

        break;

      case 50:
        if (check == (get_nb_rbg(cell_config) - 1)) {
          nb_rb-=2;
        } else {
          nb_rb-=3;
        }

        break;

      case 100:
        nb_rb-=4;
        break;
      }
    }
    check = check+1;
  }
  return (rballoc_dci);
}
