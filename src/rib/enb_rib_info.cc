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

#include "enb_rib_info.h"

flexran::rib::enb_rib_info::enb_rib_info(int agent_id)
  : agent_id_(agent_id) {
  last_checked = clock();
}

void flexran::rib::enb_rib_info::update_eNB_config(const protocol::flex_enb_config_reply& enb_config_update) {
  eNB_config_.CopyFrom(enb_config_update);
  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(const protocol::flex_ue_config_reply& ue_config_update) {
  ue_config_.CopyFrom(ue_config_update);
  rnti_t rnti;
  // Check if UE exists and if not create a ue_mac_rib_info entry
  for (int i = 0; i < ue_config_update.ue_config_size(); i++) {
    rnti = ue_config_update.ue_config(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      ue_mac_info_.insert(std::pair<int,
			  std::shared_ptr<ue_mac_rib_info>>(rnti,
							    std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    }
  }
  update_liveness();
}

void flexran::rib::enb_rib_info::update_UE_config(const protocol::flex_ue_state_change& ue_state_change) {
  rnti_t rnti;  
  if (ue_state_change.type() == protocol::FLUESC_ACTIVATED) {
    protocol::flex_ue_config *c = ue_config_.add_ue_config();
    c->CopyFrom(ue_state_change.config());
    rnti = ue_state_change.config().rnti();
    ue_mac_info_.insert(std::pair<int,
			std::shared_ptr<ue_mac_rib_info>>(rnti,
							  std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    return;
  }
  for (int i = 0; i < ue_config_.ue_config_size(); i++) {
    rnti = ue_config_.ue_config(i).rnti();
    if (rnti == ue_state_change.config().rnti()) {
      // Check if this was updated or removed
      if (ue_state_change.type() == protocol::FLUESC_DEACTIVATED) {
	ue_config_.mutable_ue_config()->DeleteSubrange(i, i+1);
	// Erase mac info
	ue_mac_info_.erase(rnti);
	// Erase lc info as well
	for (int j = 0; j < lc_config_.lc_ue_config_size(); j++) {
	  if (rnti == lc_config_.lc_ue_config(j).rnti()) {
	    lc_config_.mutable_lc_ue_config()->DeleteSubrange(j, j+i);
	  }
	}
	return;
      } else if (ue_state_change.type() == protocol::FLUESC_UPDATED) {
	ue_config_.mutable_ue_config(i)->CopyFrom(ue_state_change.config());
      }
    }
  }
}

void flexran::rib::enb_rib_info::update_LC_config(const protocol::flex_lc_config_reply& lc_config_update) {
  lc_config_.CopyFrom(lc_config_update);
  update_liveness();
}

void flexran::rib::enb_rib_info::update_subframe(const protocol::flex_sf_trigger& sf_trigger) {
  rnti_t rnti;
  uint16_t sfn_sf = sf_trigger.sfn_sf();
  current_frame_ = get_frame(sfn_sf);
  current_subframe_ = get_subframe(sfn_sf);

  // Update dl_sf_info
  for (int i = 0; i < sf_trigger.dl_info_size(); i++) {
    rnti = sf_trigger.dl_info(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
    } else {
      it->second->update_dl_sf_info(sf_trigger.dl_info(i));
    }
  }

  // Update ul_sf_info
  for (int i = 0; i < sf_trigger.ul_info_size(); i++){ 
    rnti = sf_trigger.ul_info(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
    } else {
      it->second->update_ul_sf_info(sf_trigger.ul_info(i));
    }
  }
  update_liveness();
}

void flexran::rib::enb_rib_info::update_mac_stats(const protocol::flex_stats_reply& mac_stats) {
  rnti_t rnti;
  // First make the UE updates
  for (int i = 0; i < mac_stats.ue_report_size(); i++) {
    rnti = mac_stats.ue_report(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
      //      ue_mac_info_.insert(std::pair<int,
      //std::shared_ptr<ue_mac_rib_info>>(rnti,
      //							    std::shared_ptr<ue_mac_rib_info>(new ue_mac_rib_info(rnti))));
    } else {
      it->second->update_mac_stats_report(mac_stats.ue_report(i));
    }
  }
  // Then work on the Cell updates
  uint32_t cell_id;
  for (int i = 0; i < mac_stats.cell_report_size(); i++) {
    cell_mac_info_[i].update_cell_stats_report(mac_stats.cell_report(i));
  }
}

std::shared_ptr<flexran::rib::ue_mac_rib_info> flexran::rib::enb_rib_info::get_ue_mac_info(rnti_t rnti) {
  auto it = ue_mac_info_.find(rnti);
  if (it != ue_mac_info_.end()) {
    return it->second;
  }
  return std::shared_ptr<ue_mac_rib_info>(nullptr);
}

bool flexran::rib::enb_rib_info::need_to_query() {
  return ((clock() - last_checked) > time_to_query); 
}

void flexran::rib::enb_rib_info::update_liveness() {
  last_checked = clock();
}

void flexran::rib::enb_rib_info::dump_mac_stats() const {
  std::cout << "UE MAC stats for agent " << agent_id_ << std::endl;
  for (auto ue_stats : ue_mac_info_) {
    ue_stats.second->dump_stats();
  }
}

std::string flexran::rib::enb_rib_info::dump_mac_stats_to_string() const {
  std::string str;

  str += "UE MAC stats for agent ";
  str += agent_id_;
  str += "\n";
  for (auto ue_stats : ue_mac_info_) {
    str += ue_stats.second->dump_stats_to_string();
    str += "\n";
  }

  return str;
}

void flexran::rib::enb_rib_info::dump_configs() const {
  std::cout << eNB_config_.DebugString() << std::endl;
  std::cout << ue_config_.DebugString() << std::endl;
  std::cout << lc_config_.DebugString() << std::endl;
}

std::string flexran::rib::enb_rib_info::dump_configs_to_string() const {
  std::string str;
  str += eNB_config_.DebugString();
  str += "\n";
  str += ue_config_.DebugString();
  str += "\n";
  str += lc_config_.DebugString();
  str += "\n";

  return str;
}
