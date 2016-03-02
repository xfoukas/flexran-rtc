#ifndef ENB_SCHEDULING_INFO_H_
#define ENB_SCHEDULING_INFO_H_

#include <map>
#include <memory>

#include "ue_scheduling_info.h"
#include "rib_common.h"
#include "progran.pb.h"


class enb_scheduling_info {

 public:

  frame_t get_last_checked_frame() const { return last_checked_frame_; }

  subframe_t get_last_checked_subframe() const { return last_checked_subframe_; }

  void set_last_checked_frame(frame_t frame) { last_checked_frame_ = frame; }

  void set_last_checked_subframe(subframe_t subframe) { last_checked_subframe_ = subframe; }

  void create_ue_scheduling_info(rnti_t rnti);

  uint16_t get_pre_nb_rbs_available(uint16_t cell_id) const { return pre_nb_rbs_available_[cell_id]; }

  void set_pre_nb_rbs_available(uint16_t cell_id, uint16_t nb_rbs) {
    pre_nb_rbs_available_[cell_id] = nb_rbs;
  }

  uint8_t* get_vrb_map(uint16_t cell_id) {
    return vrb_map_[cell_id];
  }

  void start_new_scheduling_round(subframe_t subframe, const protocol::prp_cell_config& cell_config);
  
  std::shared_ptr<ue_scheduling_info> get_ue_scheduling_info(rnti_t rnti);
  
 private:

  frame_t last_checked_frame_;
  subframe_t last_checked_subframe_;

  std::map<rnti_t, std::shared_ptr<ue_scheduling_info>> scheduling_info_;

  uint16_t pre_nb_rbs_available_[MAX_NUM_CC] = {0};
  uint8_t vrb_map_[MAX_NUM_CC][N_RBG_MAX] = {{0}};
  
};

#endif
