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

#include "rib.h"

void flexran::rib::Rib::add_pending_agent(int agent_id) {
  pending_agents_.insert(agent_id);
}

void flexran::rib::Rib::remove_pending_agent(int agent_id) {
  pending_agents_.erase(agent_id);
}

bool flexran::rib::Rib::agent_is_pending(int agent_id) {
  auto search = pending_agents_.find(agent_id);
  return search != pending_agents_.end();
}

void flexran::rib::Rib::new_eNB_config_entry(int agent_id) {
  eNB_configs_.insert(std::pair<int,
		      std::shared_ptr<enb_rib_info>>(agent_id,
						     std::shared_ptr<enb_rib_info>(new enb_rib_info(agent_id))));
}

bool flexran::rib::Rib::has_eNB_config_entry(int agent_id) {
  auto it = eNB_configs_.find(agent_id);
  return it != eNB_configs_.end();
}

void flexran::rib::Rib::eNB_config_update(int agent_id,
					  const protocol::flex_enb_config_reply& enb_config_update) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_eNB_config(enb_config_update);
  }
}

void flexran::rib::Rib::ue_config_update(int agent_id,
					 const protocol::flex_ue_config_reply& ue_config_update) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_UE_config(ue_config_update);
  }
}

void flexran::rib::Rib::ue_config_update(int agent_id,
					 const protocol::flex_ue_state_change& ue_state_change) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_UE_config(ue_state_change);
  }
}

void flexran::rib::Rib::lc_config_update(int agent_id,
					 const protocol::flex_lc_config_reply& lc_config_update) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_LC_config(lc_config_update);
  }
}

void flexran::rib::Rib::update_liveness(int agent_id) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_liveness();
  }
}

void flexran::rib::Rib::set_subframe_updates(int agent_id,
			       const protocol::flex_sf_trigger& sf_trigger_msg) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_subframe(sf_trigger_msg);
  }
}

void flexran::rib::Rib::mac_stats_update(int agent_id,
		      const protocol::flex_stats_reply& mac_stats_update) {
  auto it = eNB_configs_.find(agent_id);

  if (it == eNB_configs_.end()) {
    return;
  } else {
    it->second->update_mac_stats(mac_stats_update);
  }
}

std::set<int> flexran::rib::Rib::get_available_agents() const {
  std::set<int> agents;
  for (auto it : eNB_configs_) {
    agents.insert(it.first);
  }
  return agents;
}

std::shared_ptr<const flexran::rib::enb_rib_info> flexran::rib::Rib::get_agent(int agent_id) const {
  auto it = eNB_configs_.find(agent_id);
  if (it != eNB_configs_.end()) {
    return it->second;
  }
  return std::shared_ptr<enb_rib_info>(nullptr);
}

void flexran::rib::Rib::dump_mac_stats() const {
  for (auto enb_config : eNB_configs_) {
    enb_config.second->dump_mac_stats();
  }
}

std::string flexran::rib::Rib::dump_mac_stats_to_string() const {

  std::string str;
  
  for (auto enb_config : eNB_configs_) {
    str += enb_config.second->dump_mac_stats_to_string();
    str += "\n";
  }

  return str;
}

void flexran::rib::Rib::dump_enb_configurations() const {
  for (auto eNB_config : eNB_configs_) {
    eNB_config.second->dump_configs();
  }
}

std::string flexran::rib::Rib::dump_enb_configurations_to_string() const {

  std::string str;

  for (auto eNB_config : eNB_configs_) {
    str += eNB_config.second->dump_configs_to_string();
    str += "\n";
  }

  return str;
}


