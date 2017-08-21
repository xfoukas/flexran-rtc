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

#ifndef UE_SCHEDULING_INFO_H_
#define UE_SCHEDULING_INFO_H_

#include "rib_common.h"
#include "ue_mac_rib_info.h"

#include <memory>

namespace flexran {

  namespace app {

    namespace scheduler {

      class ue_scheduling_info {

      public:
	
      ue_scheduling_info(rib::rnti_t rnti)
	: ta_timer_(20), rnti_(rnti), ndi_{{0}}, tpc_{{0}}, nb_scheduled_rbs_{{0}}, mcs_{{0}}, rballoc_sub_scheduled_{{{0}}},
						dl_pow_off_{2}, nb_rbs_required_{0}, harq_round_{{{0}}}, rballoc_sub_{{0}},
						    nb_rbs_required_remaining_{0}, nb_rbs_required_remaining_1_{0},
						    pre_nb_rbs_available_{0}, pucch_tpc_tx_frame_(0), pucch_tpc_tx_subframe_(0),
										high_priority_(true) {}

       void decr_ta_timer() { ta_timer_--; }

       int get_ta_timer() const { return ta_timer_; }
       
       void set_ta_timer(int val) { ta_timer_ = val; }

       void start_new_scheduling_round();
       
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

       void set_nb_rbs_required_remaining1(uint16_t cell_id, uint16_t nb_rbs) {
	 nb_rbs_required_remaining_1_[cell_id] = nb_rbs;
       }
       
       uint16_t get_nb_rbs_required_remaining1(uint16_t cell_id) const {
	 return nb_rbs_required_remaining_1_[cell_id];
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
       
       uint8_t get_ndi(uint16_t cell_id, uint8_t harq_pid) const { return ndi_[cell_id][harq_pid];}
       
       void toggle_ndi(uint16_t cell_id, uint8_t harq_pid) {
	 ndi_[cell_id][harq_pid] = 1 - ndi_[cell_id][harq_pid];
       }

       uint8_t get_tpc(uint16_t cell_id, uint8_t harq_pid) const { return tpc_[cell_id][harq_pid];}
       
       void set_tpc(uint16_t cell_id, uint8_t harq_pid, uint8_t tpc) {
	 tpc_[cell_id][harq_pid] = tpc;
       }
       
       void set_dl_power_offset(uint16_t cell_id, int val) {
	 dl_pow_off_[cell_id] = val;
       }
       
       uint16_t get_dl_power_offset(uint16_t cell_id) {
	 return dl_pow_off_[cell_id];
       }
       
       void set_pucch_tpc_tx_frame(rib::frame_t frame) {
	 pucch_tpc_tx_frame_ = frame;
       }
       
       rib::frame_t get_pucch_tpc_tx_frame() const {
	 return pucch_tpc_tx_frame_;
       }
       
       void set_pucch_tpc_tx_subframe(rib::subframe_t subframe) {
	 pucch_tpc_tx_subframe_ = subframe;
       }
       
       bool is_high_priority() const { return high_priority_; }

       void is_high_priority(bool flag) { high_priority_ = flag; }

       flexran::rib::subframe_t get_pucch_tpc_tx_subframe() const {
	 return pucch_tpc_tx_subframe_;
       }
       
      private:
       
       int ta_timer_;
       
       rib::rnti_t rnti_;
       uint8_t ndi_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ];
       uint8_t tpc_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ];
       
       uint16_t nb_scheduled_rbs_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ];
       uint16_t mcs_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ];
       uint8_t rballoc_sub_scheduled_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ][rib::N_RBG_MAX];
       uint16_t dl_pow_off_[rib::MAX_NUM_CC];
       uint16_t nb_rbs_required_[rib::MAX_NUM_CC];
       uint8_t harq_round_[rib::MAX_NUM_CC][rib::MAX_NUM_HARQ][rib::MAX_NUM_TB];
       
       //bool harq_uptodate_[MAX_NUM_CC][MAX_NUM_HARQ][MAX_NUM_TB];
       //uint8_t current_harq_;
       
       uint8_t rballoc_sub_[rib::MAX_NUM_CC][rib::N_RBG_MAX];
       uint16_t nb_rbs_required_remaining_[rib::MAX_NUM_CC];
       uint16_t nb_rbs_required_remaining_1_[rib::MAX_NUM_CC];
       uint16_t pre_nb_rbs_available_[rib::MAX_NUM_CC];
       
       rib::frame_t pucch_tpc_tx_frame_;
       rib::subframe_t pucch_tpc_tx_subframe_;
      
       bool high_priority_;
       //uint8_t current_harq_pid_;
       
      };

    }

  }

}
#endif
