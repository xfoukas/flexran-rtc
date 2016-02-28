#include "rt_task.h"
#include <iostream>

rt_task::rt_task(Policy pol, sched_time runtime, sched_time deadline, sched_time period) {
#ifdef LOWLATENCY
  set_scheduling_policy(pol, runtime, deadline, period);
#endif
}

#ifdef LOWLATENCY

void rt_task::set_scheduling_policy(Policy pol, sched_time runtime, sched_time deadline, sched_time period) {

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

void rt_task::execute_task() {
#ifdef LOWLATENCY
  
  if (sched_setattr(0, &attr_, 0) < 0 ) {
    std::cout << "sched_setattr failed" << std::endl;
  }

#endif

  run();
}
