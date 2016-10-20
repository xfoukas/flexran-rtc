#include <algorithm>
#include <iostream>

#include "ue_scheduling_info.h"

void flexran::app::scheduler::ue_scheduling_info::start_new_scheduling_round(uint16_t cell_id, std::shared_ptr<const flexran::rib::ue_mac_rib_info> ue_mac_info) {
  // std::copy(pre_nb_rbs_required_, pre_nb_rbs_required_ + MAX_NUM_CC, nb_rbs_required_);
  // int initial_harq = current_harq_;
  // // Find a harq that is up to date
  // harq_uptodate_[cell_id][ue_mac_info->get_currently_active_harq(cell_id)][0] = true;
  // do {
  //   current_harq_++;
  //   current_harq_ = current_harq_ % 8;
  //   // No harq is available at the moment
  //   //if (initial_harq == current_harq_) {
  //   //  std::cout << "Should not have got in here" << std::endl;
  //   //  current_harq_ = 100;
  //   //  break;
  //   //}
  // } while (!harq_uptodate_[cell_id][current_harq_][0]);
  
  std::fill( &rballoc_sub_[0][0], &rballoc_sub_[0][0] + sizeof(rballoc_sub_) /* / sizeof(flags[0][0]) */, 0 );
  std::fill(nb_rbs_required_, nb_rbs_required_ + rib::MAX_NUM_CC, 0);
  std::fill(nb_rbs_required_remaining_, nb_rbs_required_remaining_ + rib::MAX_NUM_CC, 0);
  std::fill(nb_rbs_required_remaining_1_, nb_rbs_required_remaining_1_ + rib::MAX_NUM_CC, 0);
  std::fill(pre_nb_rbs_available_, pre_nb_rbs_available_ + rib::MAX_NUM_CC, 0);
  std::fill(&rballoc_sub_scheduled_[0][0][0], &rballoc_sub_scheduled_[0][0][0] + sizeof(rballoc_sub_scheduled_), 0);  
}

int flexran::app::scheduler::ue_scheduling_info::get_harq_round(uint16_t cell_id, int harq_pid) const {
  if (cell_id < rib::MAX_NUM_CC && harq_pid < rib::MAX_NUM_HARQ) {
     return harq_round_[cell_id][harq_pid][0];
   }
   return -1;
}

