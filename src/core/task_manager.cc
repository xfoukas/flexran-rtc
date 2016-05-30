#include <thread>
#include <unistd.h>
#include <iostream>

#include "task_manager.h"

progran::core::task_manager::task_manager(progran::rib::rib_updater& r_updater)
  : r_updater_(r_updater), rt_task(Policy::FIFO) {
  struct itimerspec its;
  
  sfd = timerfd_create(CLOCK_MONOTONIC, 0);
  
  /* Start the timer */
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1000 * 1000;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  
  if (timerfd_settime(sfd, TFD_TIMER_ABSTIME, &its, NULL) == -1) {
    std::cout << "Failed to set timer for task manager" << std::endl;
  }
}

void progran::core::task_manager::run() {
  manage_rt_tasks();
}

void progran::core::task_manager::manage_rt_tasks() {
  std::thread running_apps[100];
  
  while (true) {
    // First run the RIB updater for 0.2 ms and wait to finish
    std::thread rib_updater_thread(&progran::rib::rib_updater::execute_task, &r_updater_);
    if (rib_updater_thread.joinable()) {
      rib_updater_thread.join();
    }

    // Then spawn any registered application and wait for them to finish
    int i = 0;
    for(auto app : apps_) {
      running_apps[i] = std::thread(&progran::app::component::execute_task, app);
      i++;
    }
    for (int j = 0; j < i; j++) {
      running_apps[j].join();
    }
    wait_for_cycle();
  }
}

// Warning: Not thread safe for the moment
void progran::core::task_manager::register_app(const std::shared_ptr<progran::app::component>& app) {
  apps_.emplace_back(app);
}

void progran::core::task_manager::wait_for_cycle() {
  uint64_t exp;
  ssize_t res;

  if (sfd > 0) {
    res = read(sfd, &exp, sizeof(exp));

    if ((res < 0) || (res != sizeof(exp))) {
      std::cout << "Failed in task manager timer wait" << std::endl;
    }
  }
}
