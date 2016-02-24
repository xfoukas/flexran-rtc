#ifndef RIB_H_
#define RIB_H_

#include <map>
#include <set>

#include "enb_rib_info.h"
#include <memory>
#include <set>

class Rib {
 public:

  // Pending agent methods
  void add_pending_agent(int agent_id);
  void remove_pending_agent(int agent_id);
  bool agent_is_pending(int agent_id);

  // eNB config management
  void new_eNB_config_entry(int agent_id);
  bool has_eNB_config_entry(int agent_id);
  void eNB_config_update(int agent_id,
			 const protocol::prp_enb_config_reply& enb_config_update);
  void ue_config_update(int agent_id,
			const protocol::prp_ue_config_reply& ue_config_update);
  void ue_config_update(int agent_id,
			     const protocol::prp_ue_state_change& ue_state_change);
  void lc_config_update(int agent_id,
			const protocol::prp_lc_config_reply& lc_config_update);
  
  void update_liveness(int agent_id);

  void set_subframe_updates(int agent_id,
			    const protocol::prp_sf_trigger& sf_trigger_msg);

  void mac_stats_update(int agent_id,
			const protocol::prp_stats_reply& mac_stats_update);

  std::set<int> get_available_agents() const;

  void dump_mac_stats() const;

  void dump_enb_configurations() const;
  
 private:

  std::map<int, std::shared_ptr<enb_rib_info>> eNB_configs_;
  std::set<int> pending_agents_;
  
};

#endif
