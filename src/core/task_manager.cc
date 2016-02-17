#include <thread>

#include "task_manager.h"

void task_manager::run() {
  manage_rt_tasks();
}

void task_manager::manage_rt_tasks() {
  
  while (true) {
    // First run the RIB updater for 0.2 ms and wait to finish
    std::thread rib_updater_thread(&rib_updater::execute_task, &r_updater_);
    if (rib_updater_thread.joinable()) {
      rib_updater_thread.join();
    }

    // Then spawn any registered application
  }
}


