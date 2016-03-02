#include "remote_scheduler.h"
#include "remote_scheduler_helper.h"
#include "remote_scheduler_primitives.h"
#include "progran.pb.h"
#include "rib_common.h"

#include <iostream>


void remote_scheduler::run_periodic_task() {

  frame_t target_frame;
  subframe_t target_subframe;
  
  unsigned char aggregation;
  uint16_t min_rb_unit[MAX_NUM_CC];
  uint16_t total_nb_available_rb[MAX_NUM_CC];
  int N_RBG[MAX_NUM_CC];

  std::set<int> agent_ids = std::move(rib_.get_available_agents());

  for (const auto& agent_id : agent_ids) {
    std::shared_ptr<const enb_rib_info> agent_config = rib_.get_agent(agent_id);
    const protocol::prp_enb_config_reply& enb_config = agent_config->get_enb_config();
    const protocol::prp_ue_config_reply& ue_configs = agent_config->get_ue_configs();
    const protocol::prp_lc_config_reply& lc_configs = agent_config->get_lc_configs();

    frame_t current_frame = agent_config->get_current_frame();
    subframe_t current_subframe = agent_config->get_current_subframe();
    
    // Check if scheduling context for this eNB is already present and if not create it
    std::shared_ptr<enb_scheduling_info> enb_sched_info = get_scheduling_info(agent_id);
    if (enb_sched_info) {
     // Check if we have already run the scheduler for this particular time slot and if yes go to next eNB
      if (!needs_scheduling(enb_sched_info, current_frame, current_subframe)) {
	continue;
      }
      enb_sched_info->start_new_scheduling_round();
      target_subframe = (current_subframe + schedule_ahead) % 10;
      if (target_subframe < current_subframe) {
	target_frame = (target_frame + 1) % 1024;
      }
    } else { // eNB sched info was not found for this agent
      std::cout << "Config was not found. Creating" << std::endl;
      scheduling_info_.insert(std::pair<int,
			      std::shared_ptr<enb_scheduling_info>>(agent_id,
								    std::shared_ptr<enb_scheduling_info>(new enb_scheduling_info)));
      enb_sched_info = get_scheduling_info(agent_id);
    }

    aggregation = 1;
    
    // Go through the cell configs and set the variables
    for (int i = 0; i < enb_config.cell_config_size(); i++) {
      const protocol::prp_cell_config cell_config = enb_config.cell_config(i);
      min_rb_unit[i] = get_min_rb_unit(cell_config);
      total_nb_available_rb[i] = cell_config.dl_bandwidth();
      //Remove the RBs used by common channels
      //TODO: For now we will do this manually based on OAI config and scheduling sf. Important to fix it later.
      // Assume an FDD scheduler
      switch(target_subframe) {
      case 0:
      case 5:
	total_nb_available_rb[i] -= 4;
	break;
      }
      // Get the number of resource block groups used for this cell configuration
      N_RBG[i] = get_nb_rbg(cell_config);

      // Run the preprocessor to make initial allocation of RBs to UEs (Need to do this over all scheduling_info of eNB)
      run_dlsch_scheduler_preprocessor(cell_config, ue_configs, agent_config, enb_sched_info);
    }

    /*TODO*/
    
    // Go through the UEs of this eNB and create a scheduling decision
    for (int i = 0; i < ue_configs.ue_config_size(); i++) {
      const protocol::prp_ue_config& config = ue_configs.ue_config(i);
      rnti_t rnti = config.rnti();

      // Get the MAC stats for the UE under study
      std::shared_ptr<const ue_mac_rib_info> ue_mac_info = agent_config->get_ue_mac_info(rnti);

      
      
    }
    // Done with scheduling of eNB UEs. Set the last scheduled frame and subframe
    enb_sched_info->set_last_checked_frame(current_frame);
    enb_sched_info->set_last_checked_subframe(current_subframe);
  }
}

std::shared_ptr<enb_scheduling_info> remote_scheduler::get_scheduling_info(int agent_id) {
  auto it = scheduling_info_.find(agent_id);
  if (it != scheduling_info_.end()) {
    return it->second;
  }
  return std::shared_ptr<enb_scheduling_info>(nullptr);
}
