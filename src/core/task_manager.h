#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include "rt_task.h"
#include "rib_updater.h"
#include "rt_wrapper.h"
#include <linux/types.h>

class task_manager : public rt_task {
 public:
  
  task_manager(rib_updater& r_updater)
    : r_updater_(r_updater), rt_task(Policy::FIFO) {}

  void run();
  
  void manage_rt_tasks();

 private:
  rib_updater& r_updater_;
};


#endif
