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

#ifndef REMOTE_SCHEDULER_PRIMITIVES_H_
#define REMOTE_SCHEDULER_PRIMITIVES_H_

#include "flexran.pb.h"
#include "rib_common.h"

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"


namespace flexran {

  namespace app {

    namespace scheduler {

#ifndef ABSTRACT_SIMULATION
      const static int target_dl_mcs_ = 28;
#else
      const static int target_dl_mcs_ = 15;
#endif

      
      bool needs_scheduling(::std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			    rib::frame_t curr_frame,
			    rib::subframe_t curr_subframe);
      
      bool CCE_allocation_infeasible(::std::shared_ptr<enb_scheduling_info>& enb_sched_info,
				     const protocol::flex_cell_config& cell_config,
				     const protocol::flex_ue_config& ue_config,
				     uint8_t aggregation,
				     rib::subframe_t subframe);
      
      uint16_t get_min_rb_unit(const protocol::flex_cell_config& cell_config);
      
      uint16_t get_nb_rbg(const protocol::flex_cell_config& cell_config);
      
      uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);
      
      unsigned char get_I_TBS(unsigned char I_MCS);
      
      uint8_t get_mi(const protocol::flex_cell_config& cell_config,
		     rib::subframe_t subframe);
      
      uint16_t get_nCCE_max(uint8_t num_pdcch_symbols,
			    const protocol::flex_cell_config& cell_config,
			    rib::subframe_t subframe);
      
      uint16_t get_nquad(uint8_t num_pdcch_symbols,
			 const protocol::flex_cell_config& cell_config,
			 uint8_t mi);
      
      int get_nCCE_offset(const uint8_t aggregation,
			  const int nCCE,
			  const int common_dci,
			  const rib::rnti_t rnti,
			  const rib::subframe_t subframe);
      
      uint8_t get_phich_resource(const protocol::flex_cell_config& cell_config);

      uint32_t allocate_prbs_sub(int nb_rb,
				 uint8_t *rballoc,
				 const protocol::flex_cell_config& cell_config);

    }

  }

}

#endif
