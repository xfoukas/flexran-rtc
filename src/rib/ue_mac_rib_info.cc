#include "ue_mac_rib_info.h"

void ue_mac_rib_info::update_dl_sf_info(const protocol::prp_dl_info& dl_info) {
  uint8_t CC_id = dl_info.serv_cell_index();
  uint8_t harq_id = dl_info.harq_process_id();

  for (int i = 0; i < dl_info.harq_status_size(); i++) {
    harq_stats_[CC_id][harq_id][i] = dl_info.harq_status(i);
  }
}

void ue_mac_rib_info::update_ul_sf_info(const protocol::prp_ul_info& ul_info) {
  tpc_ = ul_info.tpc();
  uint8_t CC_id = ul_info.serv_cell_index();
  uplink_reception_stats_[CC_id] = ul_info.reception_status();
  for (int i = 0; i < ul_info.ul_reception_size(); i++) {
    ul_reception_data_[CC_id][i] = ul_info.ul_reception(i);
  }
}
