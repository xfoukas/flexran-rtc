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


void enb_scheduling_info::start_new_scheduling_round() {
  std::fill(pre_nb_rbs_available_, pre_nb_rbs_available_ + MAX_NUM_CC, 0);
}
