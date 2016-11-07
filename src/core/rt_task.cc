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

#include "rt_task.h"
#include <iostream>

flexran::core::rt::rt_task::rt_task(Policy pol, sched_time runtime, sched_time deadline, sched_time period) {
#ifdef LOWLATENCY
  set_scheduling_policy(pol, runtime, deadline, period);
#endif
}

#ifdef LOWLATENCY

void flexran::core::rt::rt_task::set_scheduling_policy(Policy pol, sched_time runtime, sched_time deadline, sched_time period) {

  __u32 sched_policy;

  if(pol == Policy::RR) {
    sched_policy = SCHED_RR;
    attr_.sched_priority = sched_get_priority_max(SCHED_RR);
  } else if (pol == Policy::FIFO) {
    sched_policy = SCHED_FIFO;
    attr_.sched_priority = sched_get_priority_max(SCHED_FIFO);
  } else {
    sched_policy = SCHED_DEADLINE;
    attr_.sched_priority = 0;
  }
  
  /* TODO: Must add checks for validity of parameters */
  attr_.size = sizeof(attr_);
  attr_.sched_flags = 0;
  attr_.sched_nice = 0;

  attr_.sched_policy = sched_policy;
  attr_.sched_runtime  = runtime;
  attr_.sched_deadline = deadline;
  attr_.sched_period   = period;
}

#endif

void flexran::core::rt::rt_task::execute_task() {
#ifdef LOWLATENCY
  
  if (sched_setattr(0, &attr_, 0) < 0 ) {
    std::cout << "sched_setattr failed" << std::endl;
  }

#endif

  run();
}
