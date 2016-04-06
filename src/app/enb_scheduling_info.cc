#include "enb_scheduling_info.h"
#include "remote_scheduler_primitives.h"

std::shared_ptr<progran::app::scheduler::ue_scheduling_info>
progran::app::scheduler::enb_scheduling_info::get_ue_scheduling_info(progran::rib::rnti_t rnti) {
  auto it = scheduling_info_.find(rnti);
  if (it != scheduling_info_.end()) {
    return it->second;
  } else {
    return std::shared_ptr<ue_scheduling_info>(nullptr);
  }
}

void progran::app::scheduler::enb_scheduling_info::create_ue_scheduling_info(progran::rib::rnti_t rnti) {
  scheduling_info_.insert(std::pair<progran::rib::rnti_t,
			  std::shared_ptr<ue_scheduling_info>> (rnti,
								std::shared_ptr<ue_scheduling_info>(new ue_scheduling_info(rnti))));
}


void progran::app::scheduler::enb_scheduling_info::increase_num_pdcch_symbols(const protocol::prp_cell_config& cell_config,
									     progran::rib::subframe_t subframe) {
  int cell_id = cell_config.cell_id();
  uint32_t prev_nCCE_max = get_nCCE_max(num_pdcch_symbols_[cell_id],
					cell_config,
					subframe);
  num_pdcch_symbols_[cell_id]++;
  uint32_t nCCE_max = get_nCCE_max(num_pdcch_symbols_[cell_id],
				   cell_config,
				   subframe);
  nCCE_rem_[cell_id] = nCCE_max - (prev_nCCE_max - nCCE_rem_[cell_id]); 
				   
}

void progran::app::scheduler::enb_scheduling_info::start_new_scheduling_round(progran::rib::subframe_t subframe,
									     const protocol::prp_cell_config& cell_config) {
  
  int cell_id = cell_config.cell_id();
  
  std::fill(vrb_map_[cell_id], vrb_map_[cell_id] + rib::N_RBG_MAX, 0);
  num_pdcch_symbols_[cell_id] = 0;
  
  nCCE_rem_[cell_id] = get_nCCE_max(num_pdcch_symbols_[cell_id],
				    cell_config,
				    subframe);
  
  int n_rb_dl = cell_config.dl_bandwidth();
  
  // Check if we have other things to schedule as well (OAI specific for the moment)
  if ((subframe == 0) || (subframe == 5)) {
    // We will need at least one symbol for the scheduling
    num_pdcch_symbols_[cell_id] = 3;
    // Set the first 4 RBs for RA
    vrb_map_[cell_id][0] = 1;
    vrb_map_[cell_id][1] = 1;
    vrb_map_[cell_id][2] = 1;
    vrb_map_[cell_id][3] = 1;
  }

  // Check if we have an SI transmission as well
  if (subframe == 5) {
    int index = 0;
    switch(n_rb_dl) {
    case 6:
      index = 0;
      break;
    case 15:
      index = 6;
      break;
    case 25:
      index = 11;
      break;
    case 50:
      index = 23;
      break;
    case 100:
      index = 48;
      break;
    default:
      index = 0;
      break;
    }
    vrb_map_[cell_id][index] = 1;
    vrb_map_[cell_id][index+1] = 1;
    vrb_map_[cell_id][index+2] = 1;
    vrb_map_[cell_id][index+3] = 1;
  }
}
