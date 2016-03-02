#ifndef UE_SCHEDULING_INFO_H_
#define UE_SCHEDULING_INFO_H_

#include "rib_common.h"

class ue_scheduling_info {

 public:

 ue_scheduling_info(rnti_t rnti)
   : ta_timer_(20), nb_scheduled_rbs_{0},
    dl_pow_off_{0}, nb_rbs_required_{0},
		     rnti_(rnti), rballoc_sub_{{0}},
		       nb_rbs_required_remaining_{0},
			 current_harq_pid_(0), harq_round_{{{0}}} {}

  void decr_ta_timer() { ta_timer_--; }

  int get_ta_timer() const { return ta_timer_; }

  void start_new_scheduling_round();

  int get_active_harq_pid() const { return current_harq_pid_; }

  int get_harq_round(uint16_t cell_id, int harq_pid) const;

  void set_harq_round(uint16_t cell_id, int harq_pid, int round) {
    harq_round_[cell_id][harq_pid][0] = round;
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

  void set_nb_scheduled_rbs(uint16_t cell_id, uint16_t nb_rbs) {
    nb_scheduled_rbs_[cell_id] = nb_rbs;
  }

  uint16_t get_nb_scheduled_rbs(uint16_t cell_id) {
    return nb_scheduled_rbs_[cell_id];
  }

  uint16_t get_pre_nb_rbs_available(uint16_t cell_id) const { return pre_nb_rbs_available_[cell_id]; }

  void set_pre_nb_rbs_available(uint16_t cell_id, uint16_t nb_rbs) {
    pre_nb_rbs_available_[cell_id] = nb_rbs;
  }
  
 private:

  int ta_timer_;

  rnti_t rnti_;

  uint16_t nb_scheduled_rbs_[MAX_NUM_CC];
  uint16_t dl_pow_off_[MAX_NUM_CC];
  uint16_t nb_rbs_required_[MAX_NUM_CC];
  uint8_t harq_round_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
  uint8_t rballoc_sub_[MAX_NUM_CC][N_RBG_MAX];
  uint16_t nb_rbs_required_remaining_[MAX_NUM_CC];
  uint16_t pre_nb_rbs_available_[MAX_NUM_CC] = {0};
  
  int current_harq_pid_;
  
};

#endif
