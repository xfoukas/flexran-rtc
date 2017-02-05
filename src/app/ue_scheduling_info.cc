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

#include <algorithm>
#include <iostream>

#include "ue_scheduling_info.h"

void flexran::app::scheduler::ue_scheduling_info::start_new_scheduling_round() {
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

