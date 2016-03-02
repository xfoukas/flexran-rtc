#include "enb_scheduling_info.h"

std::shared_ptr<ue_scheduling_info> enb_scheduling_info::get_ue_scheduling_info(rnti_t rnti) {
  auto it = scheduling_info_.find(rnti);
  if (it != scheduling_info_.end()) {
    return it->second;
  } else {
    return std::shared_ptr<ue_scheduling_info>(nullptr);
  }
}

void enb_scheduling_info::create_ue_scheduling_info(rnti_t rnti) {
  scheduling_info_.insert(std::pair<rnti_t,
			  std::shared_ptr<ue_scheduling_info>> (rnti,
								std::shared_ptr<ue_scheduling_info>(new ue_scheduling_info(rnti))));
}


void enb_scheduling_info::start_new_scheduling_round(subframe_t subframe, const protocol::prp_cell_config& cell_config) {

  int cell_id = cell_config.cell_id();
  
  std::fill(pre_nb_rbs_available_, pre_nb_rbs_available_ + MAX_NUM_CC, 0);
  std::fill(vrb_map_[cell_id], vrb_map_[cell_id] + N_RBG_MAX, 0);

  int n_rb_dl = cell_config.dl_bandwidth();
  
  // Check if we have other things to schedule as well (OAI specific for the moment)
  if ((subframe == 0) || (subframe == 5)) {
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
