/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <thread>
#include <unistd.h>
#include <iostream>

#include "task_manager.h"

flexran::core::task_manager::task_manager(flexran::rib::rib_updater& r_updater)
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

void flexran::core::task_manager::run() {
  manage_rt_tasks();
}

void flexran::core::task_manager::manage_rt_tasks() {
  std::thread running_apps[100];
  
  while (true) {
    // First run the RIB updater for 0.2 ms and wait to finish
    std::thread rib_updater_thread(&flexran::rib::rib_updater::execute_task, &r_updater_);
    if (rib_updater_thread.joinable()) {
      rib_updater_thread.join();
    }

    // Then spawn any registered application and wait for them to finish
    int i = 0;
    for(auto app : apps_) {
      running_apps[i] = std::thread(&flexran::app::component::execute_task, app);
      i++;
    }
    for (int j = 0; j < i; j++) {
      running_apps[j].join();
    }
    wait_for_cycle();
  }
}

// Warning: Not thread safe for the moment
void flexran::core::task_manager::register_app(const std::shared_ptr<flexran::app::component>& app) {
  apps_.emplace_back(app);
}

void flexran::core::task_manager::wait_for_cycle() {
  uint64_t exp;
  ssize_t res;

  if (sfd > 0) {
    res = read(sfd, &exp, sizeof(exp));

    if ((res < 0) || (res != sizeof(exp))) {
      std::cout << "Failed in task manager timer wait" << std::endl;
    }
  }
}
