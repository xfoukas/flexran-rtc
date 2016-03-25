#ifndef ENB_SCHEDULING_INFO_H_
#define ENB_SCHEDULING_INFO_H_

#include <map>
#include <memory>

#include "ue_scheduling_info.h"
#include "rib_common.h"
#include "progran.pb.h"

namespace progran {

  namespace app {

    namespace scheduler {

      class enb_scheduling_info {
	
      public:
	
	rib::frame_t get_last_checked_frame() const { return last_checked_frame_; }
	
	rib::subframe_t get_last_checked_subframe() const { return last_checked_subframe_; }
	
	void set_last_checked_frame(rib::frame_t frame) { last_checked_frame_ = frame; }
	
	void set_last_checked_subframe(rib::subframe_t subframe) { last_checked_subframe_ = subframe; }
	
	void create_ue_scheduling_info(rib::rnti_t rnti);
	
	uint8_t* get_vrb_map(uint16_t cell_id) {
	  return vrb_map_[cell_id];
	}
	
	uint8_t get_num_pdcch_symbols(uint16_t cell_id) const { return num_pdcch_symbols_[cell_id]; }
	
	void increase_num_pdcch_symbols(uint16_t cell_id) { num_pdcch_symbols_[cell_id]++;}
	
	void start_new_scheduling_round(rib::subframe_t subframe,
					const protocol::prp_cell_config& cell_config);
	
	::std::shared_ptr<ue_scheduling_info> get_ue_scheduling_info(rib::rnti_t rnti);
	
      private:
	
	rib::frame_t last_checked_frame_;
	rib::subframe_t last_checked_subframe_;
	
	::std::map<rib::rnti_t, ::std::shared_ptr<ue_scheduling_info>> scheduling_info_;
	
	uint8_t vrb_map_[rib::MAX_NUM_CC][rib::N_RBG_MAX] = {{0}};
	uint8_t num_pdcch_symbols_[rib::MAX_NUM_CC] = {0};
      };

    }

  }

}
      
#endif
