#include <thread>

#include "task_manager.h"

void task_manager::run() {
  manage_rt_tasks();
}

void task_manager::manage_rt_tasks() {
  std::thread running_apps[100];
  
  while (true) {
    // First run the RIB updater for 0.2 ms and wait to finish
    std::thread rib_updater_thread(&rib_updater::execute_task, &r_updater_);
    if (rib_updater_thread.joinable()) {
      rib_updater_thread.join();
    }

    // Then spawn any registered application and wait for them to finish
    int i = 0;
    for(auto app : apps_) {
      running_apps[i] = std::thread(&component::execute_task, app);
      i++;
    }
    for (int j = 0; j < i; j++) {
      running_apps[j].join();
    }
  }
}

// Warning: Not thread safe for the moment
void task_manager::register_app(const std::shared_ptr<component>& app) {
  apps_.emplace_back(app);
}


