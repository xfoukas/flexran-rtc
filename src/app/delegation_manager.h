#ifndef DELEGATION_MANAGER_H_
#define DELEGATION_MANAGER_H_

#include "periodic_component.h"
#include "rib_common.h"

namespace progran {
  
  namespace app {

    namespace management {

      class delegation_manager : public periodic_component {
	
      public:

	delegation_manager(const rib::Rib& rib, const core::requests_manager& rm)
	  : periodic_component(rib, rm), delegation_steps_{false} {}

	void run_periodic_task();

	void push_code(int agent_id, std::string function_name, std::string lib_name);

	void reconfigure_agent(int agent_id, std::string policy_name);
	
      private:
	bool delegation_steps_[6];
      };

    }
  }
}

#endif
