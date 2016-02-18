#include "enb_rib_info.h"

enb_rib_info::enb_rib_info(int agent_id)
  : agent_id_(agent_id) {
  last_checked = clock();
}

void enb_rib_info::update_eNB_config(const protocol::prp_enb_config_reply& enb_config_update) {
  eNB_config_.MergeFrom(enb_config_update);
  update_liveness();
}

void enb_rib_info::update_UE_config(const protocol::prp_ue_config_reply& ue_config_update) {
  ue_config_.MergeFrom(ue_config_update);
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

void enb_rib_info::update_LC_config(const protocol::prp_lc_config_reply& lc_config_update) {
  lc_config_.MergeFrom(lc_config_update);
  update_liveness();
}

void enb_rib_info::update_subframe(const protocol::prp_sf_trigger& sf_trigger) {
  rnti_t rnti;
  uint32_t sfn_sf = sf_trigger.sfn_sf();
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
  for (int i = 0; i < sf_trigger.ul_info_size(); i++) {
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

void enb_rib_info::update_mac_stats(const protocol::prp_stats_reply& mac_stats) {
  rnti_t rnti;
  // First make the UE updates
  for (int i = 0; i < mac_stats.ue_report_size(); i++) {
    rnti = mac_stats.ue_report(i).rnti();
    auto it = ue_mac_info_.find(rnti);
    if (it == ue_mac_info_.end()) {
      /* TODO: For some reason we have no such entry. This shouldn't happen */
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

bool enb_rib_info::need_to_query() {
  return ((clock() - last_checked) > time_to_query); 
}

void enb_rib_info::update_liveness() {
  last_checked = clock();
}
