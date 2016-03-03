#include <algorithm>

#include "ue_scheduling_info.h"

void ue_scheduling_info::start_new_scheduling_round() {
  current_harq_pid_++;
  // std::copy(pre_nb_rbs_required_, pre_nb_rbs_required_ + MAX_NUM_CC, nb_rbs_required_);
  std::fill( &rballoc_sub_[0][0], &rballoc_sub_[0][0] + sizeof(rballoc_sub_) /* / sizeof(flags[0][0]) */, 0 );
  std::fill(nb_rbs_required_remaining_, nb_rbs_required_remaining_ + MAX_NUM_CC, 0);
  std::fill(pre_nb_rbs_available_, pre_nb_rbs_available_ + MAX_NUM_CC, 0);
  std::fill(&rballoc_sub_scheduled_[0][0][0], &rballoc_sub_scheduled_[0][0][0] + sizeof(rballoc_sub_scheduled_), 0);  
}

int ue_scheduling_info::get_harq_round(uint16_t cell_id, int harq_pid) const {
   if (cell_id < MAX_NUM_CC && harq_pid < MAX_NUM_HARQ) {
     return harq_round_[cell_id][harq_pid][0];
   }
   return -1;
}

