#ifndef REMOTE_SCHEDULER_EICIC_H_
#define REMOTE_SCHEDULER_EICIC_H_

#include <map>

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

namespace flexran {

  namespace app {

    namespace scheduler {

      class remote_scheduler_eicic : public periodic_component {
  
      public:
	
      remote_scheduler_eicic(const rib::Rib& rib, const core::requests_manager& rm)
	: periodic_component(rib, rm) {}
	
	void run_periodic_task();
	
	static int32_t tpc_accumulated;
	
      private:
	
	::std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
	
	::std::map<int, ::std::shared_ptr<enb_scheduling_info>> scheduling_info_;
	
	// Set these values internally for now
	
	const int schedule_ahead = 1;
	
	int abs_[10] = {0, 1, 0, 1, 0, 0, 0, 1, 0, 1};

	const int macro_agent_id_ = 0;

      };

    }

  }

}
#endif
