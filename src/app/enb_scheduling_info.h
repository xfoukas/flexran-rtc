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

  uint8_t* get_vrb_map(uint16_t cell_id) {
    return vrb_map_[cell_id];
  }

  uint8_t get_num_pdcch_symbols(uint16_t cell_id) const { return num_pdcch_symbols_[cell_id]; }

  void increase_num_pdcch_symbols(uint16_t cell_id) { num_pdcch_symbols_[cell_id]++;}

  void start_new_scheduling_round(subframe_t subframe, const protocol::prp_cell_config& cell_config);
  
  std::shared_ptr<ue_scheduling_info> get_ue_scheduling_info(rnti_t rnti);
  
 private:

  frame_t last_checked_frame_;
  subframe_t last_checked_subframe_;

  std::map<rnti_t, std::shared_ptr<ue_scheduling_info>> scheduling_info_;

  uint8_t vrb_map_[MAX_NUM_CC][N_RBG_MAX] = {{0}};
  uint8_t num_pdcch_symbols_[MAX_NUM_CC] = {0};
};

#endif
