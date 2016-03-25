#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "rt_task.h"
#include "rib_updater.h"
#include "rt_wrapper.h"
#include "component.h"

#include <linux/types.h>
#include <vector>
#include <memory>

namespace progran {

  namespace core {

    class task_manager : public rt::rt_task {
    public:
      
    task_manager(progran::rib::rib_updater& r_updater)
      : r_updater_(r_updater), rt_task(Policy::FIFO) {}
      
      void manage_rt_tasks();
      
      void register_app(const std::shared_ptr<progran::app::component>& app);
      
    private:
      
      void run();
  
      progran::rib::rib_updater& r_updater_;
      
      std::vector<std::shared_ptr<progran::app::component>> apps_;
    };
  }
}

#endif
