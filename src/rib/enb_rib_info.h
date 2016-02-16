#ifndef ENB_RIB_INFO_H_
#define ENB_RIB_INFO_H_

#include "progran.pb.h"

class enb_rib_info {
 public:
  enb_rib_info(int agent_id)
    : agent_id_(agent_id) {}

  void update_eNB_config(const protocol::prp_enb_config_reply& enb_config_update);

  void update_UE_config(const protocol::prp_ue_config_reply& ue_config_update);

  void update_LC_config(const protocol::prp_lc_config_reply& lc_config_update);
  
 private:
  int agent_id_;

  // eNB config structure
  protocol::prp_enb_config_reply eNB_config_;
  // UE config structure
  protocol::prp_ue_config_reply ue_config_;
  // LC config structure
  protocol::prp_lc_config_reply lc_config_;
  
};

#endif
