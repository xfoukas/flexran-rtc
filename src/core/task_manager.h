#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "rt_task.h"
#include "rib_updater.h"
#include "rt_wrapper.h"
#include "component.h"

#include <linux/types.h>
#include <vector>
#include <memory>

#include <sys/timerfd.h>

namespace flexran {

  namespace core {

    class task_manager : public rt::rt_task {
    public:

      task_manager(flexran::rib::rib_updater& r_updater);

      void manage_rt_tasks();
      
      void register_app(const std::shared_ptr<flexran::app::component>& app);
      
    private:
      
      void run();
  
      void wait_for_cycle();
      
      flexran::rib::rib_updater& r_updater_;
      
      std::vector<std::shared_ptr<flexran::app::component>> apps_;

      int sfd;

    };
  }
}

#endif
