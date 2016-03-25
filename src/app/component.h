#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "rt_task.h"
#include "rib.h"
#include "requests_manager.h"

namespace progran {

  namespace app {

    class component : public core::rt::rt_task {
    public:
  
    component(const rib::Rib& rib, const core::requests_manager& rm)
      : rib_(rib), req_manager_(rm), rt_task(Policy::DEADLINE,
					     8 * 100 * 1000,
					     8 * 100 * 1000,
					     1000 * 1000) {}

      virtual void run_app() = 0;

    protected:
      const rib::Rib& rib_;
      const core::requests_manager& req_manager_;
  
    private:
      
      void run() { run_app(); }
      
    };

  }

}

#endif
