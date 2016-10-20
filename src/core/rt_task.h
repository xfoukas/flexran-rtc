#ifndef RT_TASK_H_
#define RT_TASK_H_

#include <linux/types.h>
#include "rt_wrapper.h"

namespace flexran {

  namespace core {

    namespace rt {
      typedef __u64 sched_time;

      class rt_task {
	
      public:
	
#ifdef LOWLATENCY
	
	enum class Policy {RR = SCHED_RR, DEADLINE = SCHED_DEADLINE, FIFO = SCHED_FIFO};
	
#else

	enum class Policy {NORMAL = 0, RR = 1, DEADLINE = 2, FIFO = 3};
	
#endif
  
	rt_task(Policy pol, sched_time runtime = 0, sched_time deadline = 0, sched_time period = 0);
	
	void execute_task();
	
      private:
	
	virtual void run() = 0; 
	
#ifdef LOWLATENCY
  
	void set_scheduling_policy(Policy pol, sched_time runtime, sched_time deadline, sched_time period);
  
	struct sched_attr attr_;

#endif
	
      };
      
    }
    
  }

}
#endif
