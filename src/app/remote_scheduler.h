#ifndef REMOTE_SCHEDULER_H_
#define REMOTE_SCHEDULER_H_

#include <map>

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

class remote_scheduler : public periodic_component {

 public:

  remote_scheduler(const Rib& rib, const requests_manager& rm)
    : periodic_component(rib, rm) {}

  void run_periodic_task();

 private:

  bool needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			frame_t curr_frame, subframe_t curr_subframe);
  
  std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
  
  std::map<int, std::shared_ptr<enb_scheduling_info>> scheduling_info_;
  
  // Set these values internally for now


  const int schedule_ahead = 4;
    
};

void run_dlsch_scheduler_preprocessor(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config_reply& ue_configs,
				      std::shared_ptr<const enb_rib_info> agent_config,
				      std::shared_ptr<enb_scheduling_info> sched_info);

void assign_rbs_required(std::shared_ptr<ue_scheduling_info> ue_sched_info,
			 std::shared_ptr<const ue_mac_rib_info> ue_mac_info,
			 const protocol::prp_cell_config& cell_config,
			 const protocol::prp_ue_config& ue_config);

void perform_pre_processor_allocation(const protocol::prp_cell_config& cell_config,
				      const protocol::prp_ue_config& ue_config,
				      std::shared_ptr<enb_scheduling_info> sched_info,
				      std::shared_ptr<ue_scheduling_info> ue_sched_info,
				      int transmission_mode,
				      unsigned char rballoc_sub[N_RBG_MAX]);


uint16_t get_min_rb_unit(const protocol::prp_cell_config& cell_config);

uint16_t get_nb_rbg(const protocol::prp_cell_config& cell_config);

uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);

unsigned char get_I_TBS(unsigned char I_MCS);

#endif
