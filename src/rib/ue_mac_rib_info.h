#ifndef UE_MAC_RIB_INFO_H_
#define UE_MAC_RIB_INFO_H_

#include <cstdint>

#include "rib_common.h"
#include "progran.pb.h"

class ue_mac_rib_info {

public:

 ue_mac_rib_info(rnti_t rnti)
   : rnti_(rnti), harq_stats_{{{0}}}, uplink_reception_stats_{0}, ul_reception_data_{{0}} {}

  void update_dl_sf_info(const protocol::prp_dl_info& dl_info);

  void update_ul_sf_info(const protocol::prp_ul_info& ul_info);

  void update_mac_stats_report(const protocol::prp_ue_stats_report& stats_report);
  
private:

  rnti_t rnti_;
  
  protocol::prp_ue_stats_report mac_stats_report_;
 
  // SF info
  uint8_t harq_stats_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
  uint8_t uplink_reception_stats_[MAX_NUM_CC];
  uint8_t ul_reception_data_[MAX_NUM_CC][MAX_NUM_LC];
  uint8_t tpc_;
  
};

#endif
