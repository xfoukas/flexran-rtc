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
	uplink_reception_stats_{0}, ul_reception_data_{{0}},
				  currently_active_harq_{0} {}

     void update_dl_sf_info(const protocol::flex_dl_info& dl_info);

     void update_ul_sf_info(const protocol::flex_ul_info& ul_info);

     void update_mac_stats_report(const protocol::flex_ue_stats_report& stats_report);
     
     void dump_stats() const;
     
     const protocol::flex_ue_stats_report& get_mac_stats_report() const { return mac_stats_report_; }
     
     uint8_t get_harq_stats(uint16_t cell_id, int harq_pid) const {
       return harq_stats_[cell_id][harq_pid][0];
     }
     
     uint8_t get_currently_active_harq(uint16_t cell_id) const {
       return currently_active_harq_[cell_id];
     }
     
    private:
     
     rnti_t rnti_;
     
     protocol::flex_ue_stats_report mac_stats_report_;
     
     // SF info
     uint8_t harq_stats_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
     uint8_t currently_active_harq_[MAX_NUM_CC];
     uint8_t uplink_reception_stats_[MAX_NUM_CC];
     uint8_t ul_reception_data_[MAX_NUM_CC][MAX_NUM_LC];
     uint8_t tpc_;
     
    };

  }

}

#endif
