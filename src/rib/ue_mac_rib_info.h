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

#ifndef UE_MAC_RIB_INFO_H_
#define UE_MAC_RIB_INFO_H_

#include <cstdint>

#include "rib_common.h"
#include "flexran.pb.h"

namespace flexran {

  namespace rib {

    class ue_mac_rib_info {
      
    public:
      
    ue_mac_rib_info(rnti_t rnti)
      : rnti_(rnti), harq_stats_{{{protocol::FLHS_ACK}}},
	uplink_reception_stats_{0}, ul_reception_data_{{0}} {

	  for (int i = 0; i < MAX_NUM_CC; i++) {
	    for (int j = 0; j < MAX_NUM_HARQ; j++) {
	      for (int k = 0; k < MAX_NUM_TB; k++) {
		active_harq_[i][j][k] = true;
	      }
	    }
	  }
	}

     void update_dl_sf_info(const protocol::flex_dl_info& dl_info);

     void update_ul_sf_info(const protocol::flex_ul_info& ul_info);

     void update_mac_stats_report(const protocol::flex_ue_stats_report& stats_report);
     
     void dump_stats() const;

     std::string dump_stats_to_string() const;
     
     protocol::flex_ue_stats_report& get_mac_stats_report() { return mac_stats_report_; }
     
     uint8_t get_harq_stats(uint16_t cell_id, int harq_pid) {
       return harq_stats_[cell_id][harq_pid][0];
     }
     
     int get_next_available_harq(uint16_t cell_id) const {
       for (int i = 0; i < MAX_NUM_HARQ; i++) {
	 if (active_harq_[cell_id][i][0] == true) {
	   return i;
	 }
       }
       return -1;
     }

     bool has_available_harq(uint16_t cell_id) const {
       for (int i = 0; i < MAX_NUM_HARQ; i++) {
	 if (active_harq_[cell_id][i][0] == true) {
	   return true;
	 }
       }
       return false;
     }
     
     void harq_scheduled(uint16_t cell_id, uint8_t harq_pid) {
       active_harq_[cell_id][harq_pid][0] = false;
       active_harq_[cell_id][harq_pid][1] = false;
     }
     
    private:
     
     rnti_t rnti_;
     
     protocol::flex_ue_stats_report mac_stats_report_;
     
     // SF info
     uint8_t harq_stats_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
     bool active_harq_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
     uint8_t uplink_reception_stats_[MAX_NUM_CC];
     uint8_t ul_reception_data_[MAX_NUM_CC][MAX_NUM_LC];
     uint8_t tpc_;
    };

  }

}

#endif
