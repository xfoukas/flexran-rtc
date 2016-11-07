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

#include <iostream>

#include "ue_mac_rib_info.h"

void flexran::rib::ue_mac_rib_info::update_dl_sf_info(const protocol::flex_dl_info& dl_info) {
  uint8_t CC_id = dl_info.serv_cell_index();
  uint8_t harq_id = dl_info.harq_process_id();

  currently_active_harq_[CC_id] = harq_id;
  
  for (int i = 0; i < dl_info.harq_status_size(); i++) {
    harq_stats_[CC_id][harq_id][i] = dl_info.harq_status(i);
  }
}

void flexran::rib::ue_mac_rib_info::update_ul_sf_info(const protocol::flex_ul_info& ul_info) {
  tpc_ = ul_info.tpc();
  uint8_t CC_id = ul_info.serv_cell_index();
  uplink_reception_stats_[CC_id] = ul_info.reception_status();
  for (int i = 0; i < ul_info.ul_reception_size(); i++) {
    ul_reception_data_[CC_id][i] = ul_info.ul_reception(i);
  }
}

void flexran::rib::ue_mac_rib_info::update_mac_stats_report(const protocol::flex_ue_stats_report& stats_report) {
  // Check the flags of the incoming report and copy only those elements that have been updated
  uint32_t flags = stats_report.flags();
  
  if (protocol::FLUST_BSR & flags) {
    mac_stats_report_.clear_bsr();
    for (int i = 0; i < stats_report.bsr_size(); i++) {
      mac_stats_report_.add_bsr(stats_report.bsr(i));
    }
  }
  if (protocol::FLUST_PRH & flags) {
    mac_stats_report_.set_phr(stats_report.phr());
  }
  if (protocol::FLUST_RLC_BS & flags) {
    mac_stats_report_.mutable_rlc_report()->CopyFrom(stats_report.rlc_report());

    //if (stats_report.rlc_report_size() == mac_stats_report_.rlc_report_size()) {
    //  for (int i = 0; i < mac_stats_report_.rlc_report_size(); i++) {
    //	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
    //      }
    //    } else {
    //      mac_stats_report_.clear_rlc_report();
    //      for (int i = 0; i < stats_report.rlc_report_size(); i++) {
    //	mac_stats_report_.add_rlc_report();
    //	mac_stats_report_.mutable_rlc_report(i)->CopyFrom(stats_report.rlc_report(i));
    //      }
    //    }
  }
  if (protocol::FLUST_MAC_CE_BS & flags) {
    mac_stats_report_.set_pending_mac_ces(stats_report.pending_mac_ces());
  }
  if (protocol::FLUST_DL_CQI & flags) {
    mac_stats_report_.mutable_dl_cqi_report()->CopyFrom(stats_report.dl_cqi_report());
  }
  if (protocol::FLUST_PBS & flags) {
    mac_stats_report_.mutable_pbr()->CopyFrom(stats_report.pbr());
  }
  if (protocol::FLUST_UL_CQI & flags) {
    mac_stats_report_.mutable_ul_cqi_report()->CopyFrom(stats_report.ul_cqi_report());
  }
}

void flexran::rib::ue_mac_rib_info::dump_stats() const {
  std::cout << "Rnti: " << rnti_ << std::endl;
  std::cout << mac_stats_report_.DebugString() << std::endl;
  std::cout << "Harq status" << std::endl;
  for (int i = 0; i < 8; i++) {
    std::cout << "  |  " << i;
  }
  std::cout << "   |   " << std::endl;
  std::cout << " ";
  for (int i = 0; i < 8; i++) {
    if (harq_stats_[0][i][0] == protocol::FLHS_ACK) {
      std::cout << " | " << "ACK";
    } else {
      std::cout << " | " << "NACK";
    }
  }
  std::cout << "  |" << std::endl;

}
