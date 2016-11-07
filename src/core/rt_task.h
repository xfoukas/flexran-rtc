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
