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

#include "enb_scheduling_info.h"
#include "remote_scheduler_primitives.h"

std::shared_ptr<flexran::app::scheduler::ue_scheduling_info>
flexran::app::scheduler::enb_scheduling_info::get_ue_scheduling_info(flexran::rib::rnti_t rnti) {
  auto it = scheduling_info_.find(rnti);
  if (it != scheduling_info_.end()) {
    return it->second;
  } else {
    return std::shared_ptr<ue_scheduling_info>(nullptr);
  }
}

void flexran::app::scheduler::enb_scheduling_info::create_ue_scheduling_info(flexran::rib::rnti_t rnti) {
  scheduling_info_.insert(std::pair<flexran::rib::rnti_t,
			  std::shared_ptr<ue_scheduling_info>> (rnti,
								std::shared_ptr<ue_scheduling_info>(new ue_scheduling_info(rnti))));
}


void flexran::app::scheduler::enb_scheduling_info::increase_num_pdcch_symbols(const protocol::flex_cell_config& cell_config,
									     flexran::rib::subframe_t subframe) {
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

void flexran::app::scheduler::enb_scheduling_info::start_new_scheduling_round(flexran::rib::subframe_t subframe,
									     const protocol::flex_cell_config& cell_config) {
  
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
