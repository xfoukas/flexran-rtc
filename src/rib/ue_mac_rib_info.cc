#include <iostream>

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

void ue_mac_rib_info::update_mac_stats_report(const protocol::prp_ue_stats_report& stats_report) {
  // Check the flags of the incoming report and copy only those elements that have been updated
  uint32_t flags = stats_report.flags();
  
  if (protocol::PRUST_BSR & flags) {
    mac_stats_report_.clear_bsr();
    for (int i = 0; i < stats_report.bsr_size(); i++) {
      mac_stats_report_.add_bsr(stats_report.bsr(i));
    }
  }
  if (protocol::PRUST_PRH & flags) {
    mac_stats_report_.set_phr(stats_report.phr());
  }
  if (protocol::PRUST_RLC_BS & flags) {
    if (stats_report.rlc_report_size() == mac_stats_report_.rlc_report_size()) {
      for (int i = 0; i < mac_stats_report_.rlc_report_size(); i++) {
	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
      }
    } else {
      mac_stats_report_.clear_rlc_report();
      for (int i = 0; i < stats_report.rlc_report_size(); i++) {
	mac_stats_report_.add_rlc_report();
	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
      }
    }
  }
  if (protocol::PRUST_MAC_CE_BS & flags) {
    mac_stats_report_.set_pending_mac_ces(stats_report.pending_mac_ces());
  }
  if (protocol::PRUST_DL_CQI & flags) {
    mac_stats_report_.mutable_dl_cqi_report()->CopyFrom(stats_report.dl_cqi_report());
  }
  if (protocol::PRUST_PBS & flags) {
    mac_stats_report_.mutable_pbr()->CopyFrom(stats_report.pbr());
  }
  if (protocol::PRUST_UL_CQI & flags) {
    mac_stats_report_.mutable_ul_cqi_report()->CopyFrom(stats_report.ul_cqi_report());
  }
}

void ue_mac_rib_info::dump_stats() const {
  std::cout << "Rnti: " << rnti_ << std::endl;
  std::cout << mac_stats_report_.DebugString() << std::endl;
}
