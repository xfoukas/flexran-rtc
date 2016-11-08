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
#include <iostream>

#include <pthread.h>
#include <sched.h>
#include <linux/sched.h>

#include "rt_wrapper.h"

#include "async_xface.h"
#include "flexran.pb.h"
#include "rib_updater.h"
#include "rib.h"
#include "task_manager.h"
#include "stats_manager.h"
#include "remote_scheduler.h"
#include "remote_scheduler_delegation.h"
#include "remote_scheduler_eicic.h"
#include "flexible_scheduler.h"
#include "delegation_manager.h"
#include "requests_manager.h"

// Fort RESTful northbound API
#ifdef REST_NORTHBOUND

#include "call_manager.h"
#include "flexible_sched_calls.h"
#include "stats_manager_calls.h"

#endif

#include <pistache/endpoint.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  std::shared_ptr<flexran::app::scheduler::flexible_scheduler> flex_sched_app;
  
  flexran::network::async_xface net_xface(2210);
  
  // Create the rib
  flexran::rib::Rib rib;

  // Create the rib update manager
  flexran::rib::rib_updater r_updater(net_xface, rib);

  // Create the task manager
  flexran::core::task_manager tm(r_updater);

  // Create the requests manager
  flexran::core::requests_manager rm(net_xface);

  // Register any applications that we might want to execute in the controller
  // Stats manager
  std::shared_ptr<flexran::app::component> stats_app(new flexran::app::stats::stats_manager(rib, rm));
  tm.register_app(stats_app);

  // Flexible scheduler
  std::shared_ptr<flexran::app::component> flex_sched(new flexran::app::scheduler::flexible_scheduler(rib, rm));
  tm.register_app(flex_sched);

  /* More examples of developed applications are available in the commented section.
     WARNING: Some of them might still contain bugs or might be from previous versions of the controller. */
  
  // Remote scheduler
  //std::shared_ptr<flexran::app::component> remote_sched(new flexran::app::scheduler::remote_scheduler(rib, rm));
  //tm.register_app(remote_sched);

  // eICIC remote scheduler
  //std::shared_ptr<flexran::app::component> remote_sched_eicic(new flexran::app::scheduler::remote_scheduler_eicic(rib, rm));
  //tm.register_app(remote_sched_eicic);

  // Remote scheduler with delegation (TEST purposes)
  // std::shared_ptr<flexran::app::component> remote_sched(new flexran::app::scheduler::remote_scheduler_delegation(rib, rm));
  // tm.register_app(remote_sched);
  
  // Delegation manager (TEST purposes)
  //std::shared_ptr<flexran::app::component> delegation_manager(new flexran::app::management::delegation_manager(rib, rm));
  //tm.register_app(delegation_manager);

  // Start the network thread
  std::thread networkThread(&flexran::network::async_xface::execute_task, &net_xface);

  // Start the task manager thread
  std::thread task_manager_thread(&flexran::core::task_manager::execute_task, &tm);

#ifdef REST_NORTHBOUND
  
  // Initialize the northbound API

  // Set the port and the IP to listen for REST calls and initialize the call manager
  Net::Port port(9999);
  Net::Address addr(Net::Ipv4::any(), port);
  flexran::north_api::manager::call_manager north_api(addr);

  // Register API calls for the developed applications
  flexran::north_api::flexible_sched_calls scheduler_calls(std::dynamic_pointer_cast<flexran::app::scheduler::flexible_scheduler>(flex_sched));

  flexran::north_api::stats_manager_calls stats_calls(std::dynamic_pointer_cast<flexran::app::stats::stats_manager>(stats_app));
  
  north_api.register_calls(scheduler_calls);
  north_api.register_calls(stats_calls);

  // Start the call manager
  north_api.init(1);
  north_api.start();
  
  north_api.shutdown();

#else
  if (task_manager_thread.joinable())
    task_manager_thread.join();
  
  if (networkThread.joinable())
    networkThread.join();

#endif  
  return 0;
}
