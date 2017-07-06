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

#include <pistache/http.h>

#include "flexible_sched_calls.h"

void flexran::north_api::flexible_sched_calls::register_calls(Pistache::Rest::Router& router) {

  Pistache::Rest::Routes::Post(router, "/dl_sched/:sched_type", Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::change_scheduler, this));

  Pistache::Rest::Routes::Post(router, "/rrm/:policyname", Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::apply_policy, this));

  Pistache::Rest::Routes::Post(router, "/rrm_confif/", Pistache::Rest::Routes::bind(&flexran::north_api::flexible_sched_calls::apply_policy_string, this));
}

void flexran::north_api::flexible_sched_calls::change_scheduler(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto sched_type = request.param(":sched_type").as<int>();
  
  if (sched_type == 0) { // Local scheduler
    sched_app->enable_central_scheduling(false);
    response.send(Pistache::Http::Code::Ok, "Loaded Local Scheduler");
  } else if (sched_type == 1) { //Remote scheduler 
    sched_app->enable_central_scheduling(true);
    response.send(Pistache::Http::Code::Ok, "Loaded Remote Scheduler");
  } else { // Scheduler type not supported
    response.send(Pistache::Http::Code::Not_Found, "Scheduler type does not exist");
  }
  
}

void flexran::north_api::flexible_sched_calls::apply_policy(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto policy_name = request.param(":policyname").as<std::string>();
  std::string resp;
  if (policy_name != "") { // Local scheduler
    if (sched_app->apply_agent_rrm_policy(policy_name)) {
      response.send(Pistache::Http::Code::Ok, "Set the policy to the agent\n");
    } else {
      response.send(Pistache::Http::Code::Method_Not_Allowed, "Agent-side scheduling is currently inactive\n");
    }
  } else { // Scheduler policy not set
    response.send(Pistache::Http::Code::Not_Found, "Policy not set\n");
  }
  
}

void flexran::north_api::flexible_sched_calls::apply_policy_string(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {

  auto policy = request.body();

  if (policy.length() != 0) {
    if (sched_app->apply_agent_rrm_policy_string(policy)) {
      response.send(Pistache::Http::Code::Ok, "Set the policy to the agent\n");
    } else {
      response.send(Pistache::Http::Code::Method_Not_Allowed, "Agent-side scheduling is currently inactive. Cannot set policy\n");
    }
  } else {
    response.send(Pistache::Http::Code::Not_Found, "No policy defined in request\n");
  }
}
