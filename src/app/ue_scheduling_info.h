#ifndef UE_SCHEDULING_INFO_H_
#define UE_SCHEDULING_INFO_H_

#include "rib_common.h"
#include "ue_mac_rib_info.h"

#include <memory>

class ue_scheduling_info {

 public:

 ue_scheduling_info(rnti_t rnti)
   : ta_timer_(20), nb_scheduled_rbs_{0},
    dl_pow_off_{2}, nb_rbs_required_{0}, ndi_{{0}},
		     rnti_(rnti), rballoc_sub_{{0}},
		       nb_rbs_required_remaining_{0},
			 harq_round_{{{0}}},
			   pucch_tpc_tx_frame_(0), pucch_tpc_tx_subframe_(0) {}

  void decr_ta_timer() { ta_timer_--; }

  int get_ta_timer() const { return ta_timer_; }

  void set_ta_timer(int val) { ta_timer_ = val; }

  void start_new_scheduling_round(uint16_t cell_id, std::shared_ptr<const ue_mac_rib_info> ue_mac_info);

   //uint8_t get_active_harq_pid() const { return current_harq_; }

  int get_harq_round(uint16_t cell_id, int harq_pid) const;

  void set_harq_round(uint16_t cell_id, int harq_pid, int round) {
    harq_round_[cell_id][harq_pid][0] = round;
    //harq_uptodate_[cell_id][harq_pid][0] = false;
  }

  void set_nb_rbs_required(uint16_t cell_id, uint16_t nb_rbs) { nb_rbs_required_[cell_id] = nb_rbs; }

  uint16_t get_nb_rbs_required(uint16_t cell_id) const { return nb_rbs_required_[cell_id]; }

  void set_nb_rbs_required_remaining(uint16_t cell_id, uint16_t nb_rbs) {
    nb_rbs_required_remaining_[cell_id] = nb_rbs;
  }

  uint16_t get_nb_rbs_required_remaining(uint16_t cell_id) const {
    return nb_rbs_required_remaining_[cell_id];
  }

  void set_rballoc_sub(uint16_t cell_id, int rbg, int val) {
    rballoc_sub_[cell_id][rbg] = val;
  }

  uint8_t get_rballoc_sub(uint16_t cell_id, int rbg) const {
    return rballoc_sub_[cell_id][rbg];
  }

  void set_nb_scheduled_rbs(uint16_t cell_id, uint8_t harq_pid, uint16_t nb_rbs) {
    nb_scheduled_rbs_[cell_id][harq_pid] = nb_rbs;
  }

  uint16_t get_nb_scheduled_rbs(uint16_t cell_id, uint8_t harq_pid) {
    return nb_scheduled_rbs_[cell_id][harq_pid];
  }

  uint16_t get_pre_nb_rbs_available(uint16_t cell_id) const { return pre_nb_rbs_available_[cell_id]; }

  void set_pre_nb_rbs_available(uint16_t cell_id, uint16_t nb_rbs) {
    pre_nb_rbs_available_[cell_id] = nb_rbs;
  }

  void set_rballoc_sub_scheduled(uint16_t cell_id, uint8_t harq_pid, int rbg, int val) {
    rballoc_sub_scheduled_[cell_id][harq_pid][rbg] = val;
  }

  uint8_t get_rballoc_sub_scheduled(uint16_t cell_id, uint8_t harq_pid, int rbg) {
    return rballoc_sub_scheduled_[cell_id][harq_pid][rbg];
  }

  uint8_t* get_rballoc_sub_scheduled(uint16_t cell_id, uint8_t harq_pid) {
    return rballoc_sub_scheduled_[cell_id][harq_pid];
  }

  void set_mcs(uint16_t cell_id, uint8_t harq_pid, int mcs) {
    mcs_[cell_id][harq_pid] = mcs;
  }

  int get_mcs(uint16_t cell_id, uint8_t harq_pid) {
    return mcs_[cell_id][harq_pid];
  }

  int get_ndi(uint16_t cell_id, uint8_t harq_pid) const { return ndi_[cell_id][harq_pid];}

  void toggle_ndi(uint16_t cell_id, uint8_t harq_pid) {
    ndi_[cell_id][harq_pid] = 1 - ndi_[cell_id][harq_pid];
  }

  void set_dl_power_offset(uint16_t cell_id, int val) {
    dl_pow_off_[cell_id] = val;
  }

  uint16_t get_dl_power_offset(uint16_t cell_id) {
    return dl_pow_off_[cell_id];
  }

  void set_pucch_tpc_tx_frame(frame_t frame) {
    pucch_tpc_tx_frame_ = frame;
  }

  frame_t get_pucch_tpc_tx_frame() const {
    return pucch_tpc_tx_frame_;
  }
  
  void set_pucch_tpc_tx_subframe(subframe_t subframe) {
    pucch_tpc_tx_subframe_ = subframe;
  }

  subframe_t get_pucch_tpc_tx_subframe() const {
    return pucch_tpc_tx_subframe_;
  }
  
 private:

  int ta_timer_;

  rnti_t rnti_;
  int ndi_[MAX_NUM_CC][MAX_NUM_HARQ];
  
  uint16_t nb_scheduled_rbs_[MAX_NUM_CC][MAX_NUM_HARQ];
  uint16_t mcs_[MAX_NUM_CC][MAX_NUM_HARQ];
  uint8_t rballoc_sub_scheduled_[MAX_NUM_CC][MAX_NUM_HARQ][N_RBG_MAX];
  uint16_t dl_pow_off_[MAX_NUM_CC];
  uint16_t nb_rbs_required_[MAX_NUM_CC];
  uint8_t harq_round_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];

  //bool harq_uptodate_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
  //uint8_t current_harq_;
  
  uint8_t rballoc_sub_[MAX_NUM_CC][N_RBG_MAX];
  uint16_t nb_rbs_required_remaining_[MAX_NUM_CC];
  uint16_t pre_nb_rbs_available_[MAX_NUM_CC] = {0};

  frame_t pucch_tpc_tx_frame_;
  subframe_t pucch_tpc_tx_subframe_;
  
  //uint8_t current_harq_pid_;
  
};

#endif
