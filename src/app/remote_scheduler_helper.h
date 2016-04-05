#ifndef REMOTE_SCHEDULER_HELPER_H_
#define REMOTE_SCHEDULER_HELPER_H_

#include "progran.pb.h"
#include "rib_common.h"

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

namespace progran {

  namespace app {

    namespace scheduler {

      void run_dlsch_scheduler_preprocessor(const protocol::prp_cell_config& cell_config,
					    const protocol::prp_ue_config_reply& ue_configs,
					    const protocol::prp_lc_config_reply& lc_configs,
					    ::std::shared_ptr<const rib::enb_rib_info> agent_config,
					    ::std::shared_ptr<enb_scheduling_info> sched_info,
					    rib::subframe_t subframe);
      
      void assign_rbs_required(::std::shared_ptr<ue_scheduling_info> ue_sched_info,
			       ::std::shared_ptr<const rib::ue_mac_rib_info> ue_mac_info,
			       const protocol::prp_cell_config& cell_config,
			       const protocol::prp_ue_config& ue_config,
			       const protocol::prp_lc_ue_config& lc_ue_config);

      void perform_pre_processor_allocation(const protocol::prp_cell_config& cell_config,
					    const protocol::prp_ue_config& ue_config,
					    std::shared_ptr<enb_scheduling_info> sched_info,
					    std::shared_ptr<ue_scheduling_info> ue_sched_info,
					    int transmission_mode);

    }

  }

}

#endif
