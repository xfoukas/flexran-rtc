#ifndef RT_WRAPPER_H_
#define RT_WRAPPER_H_

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <syscall.h>
#include <math.h>

namespace progran {

  namespace core {

    namespace rt {
    
      void set_latency_target(void);
      
      typedef long long int rtime;
    
//#define rt_printk printf
    
      rtime rt_get_time_ns (void);

      int rt_sleep_ns (rtime x);
    
      void check_clock(void);

#ifdef LOWLATENCY

#define gettid() syscall(__NR_gettid)
    
#define SCHED_DEADLINE  6

    /* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr   314
#define __NR_sched_getattr   315
#endif

#ifdef __i386__
#define __NR_sched_setattr   351
#define __NR_sched_getattr   352
#endif

      struct sched_attr {
	__u32 size;
	
	__u32 sched_policy;
	__u64 sched_flags;
	
	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;
	
	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;
      
	/* SCHED_DEADLINE (nsec) */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
      };
    
      int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
    
      int sched_getattr(pid_t pid,struct sched_attr *attr,unsigned int size, unsigned int flags);

#endif
      
    }
    
  }

}

#endif
