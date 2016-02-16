#include "enb_rib_info.h"

void enb_rib_info::update_eNB_config(const protocol::prp_enb_config_reply& enb_config_update) {
  eNB_config_.MergeFrom(enb_config_update);
}

void enb_rib_info::update_UE_config(const protocol::prp_ue_config_reply& ue_config_update) {
  ue_config_.MergeFrom(ue_config_update);
}

void enb_rib_info::update_LC_config(const protocol::prp_lc_config_reply& lc_config_update) {
  lc_config_.MergeFrom(lc_config_update);
}
