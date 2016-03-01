#ifndef ENB_RIB_INFO_H_
#define ENB_RIB_INFO_H_

#include <ctime>
#include <map>
#include <memory>

#include "progran.pb.h"
#include "rib_common.h"
#include "ue_mac_rib_info.h"
#include "cell_mac_rib_info.h"

class enb_rib_info {
 public:
  enb_rib_info(int agent_id);

  void update_eNB_config(const protocol::prp_enb_config_reply& enb_config_update);

  void update_UE_config(const protocol::prp_ue_config_reply& ue_config_update);

  void update_UE_config(const protocol::prp_ue_state_change& ue_state_change);

  void update_LC_config(const protocol::prp_lc_config_reply& lc_config_update);

  void update_liveness();

  void update_subframe(const protocol::prp_sf_trigger& sf_trigger);

  void update_mac_stats(const protocol::prp_stats_reply& mac_stats);
  
  bool need_to_query();

  void dump_mac_stats() const;

  void dump_configs() const;

  frame_t get_current_frame() const { return current_frame_; }

  subframe_t get_current_subframe() const { return current_subframe_; }

  const protocol::prp_enb_config_reply& get_enb_config() const { return eNB_config_;}

  const protocol::prp_ue_config_reply& get_ue_configs() const {return ue_config_;}

  const protocol::prp_lc_config_reply& get_lc_configs() const {return lc_config_;}

  std::shared_ptr<const ue_mac_rib_info> get_ue_mac_info(rnti_t rnti) const;
  
 private:
  int agent_id_;

  clock_t last_checked;
  const clock_t time_to_query = 500;

  frame_t current_frame_;
  subframe_t current_subframe_;
  
  // eNB config structure
  protocol::prp_enb_config_reply eNB_config_;
  // UE config structure
  protocol::prp_ue_config_reply ue_config_;
  // LC config structure
  protocol::prp_lc_config_reply lc_config_;

  std::map<rnti_t, std::shared_ptr<ue_mac_rib_info>> ue_mac_info_;

  cell_mac_rib_info cell_mac_info_[MAX_NUM_CC];
  
  
};

#endif
