#include "rt_wrapper.h"
#include <unistd.h> 
#include <cstring>

static int latency_target_fd = -1;

static int32_t latency_target_value = 0;
/* Latency trick - taken from cyclictest.c
 * if the file /dev/cpu_dma_latency exists,
 * open it and write a zero into it. This will tell
 * the power management system not to transition to
 * a high cstate (in fact, the system acts like idle=poll)
 * When the fd to /dev/cpu_dma_latency is closed, the behavior
 * goes back to the system default.
 *
 * Documentation/power/pm_qos_interface.txt
 */
void flexran::core::rt::set_latency_target(void) {
  struct stat s;
  int ret;

  if (stat("/dev/cpu_dma_latency", &s) == 0) {
    latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);

    if (latency_target_fd == -1)
      return;

    ret = write(latency_target_fd, &latency_target_value, 4);

    if (ret == 0) {
      //printf("# error setting cpu_dma_latency to %d!: %s\n", latency_target_value, strerror(errno));
      close(latency_target_fd);
      return;
    }

    //rt_printk("# /dev/cpu_dma_latency set to %dus\n", latency_target_value);
  }
}


struct timespec interval, next, now, res;
clockid_t clock_id = CLOCK_MONOTONIC; //other options are CLOCK_MONOTONIC, CLOCK_REALTIME, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID
flexran::core::rt::rtime flexran::core::rt::rt_get_time_ns (void) {
  clock_gettime(clock_id, &now);
  return(now.tv_sec*1e9+now.tv_nsec);
}

int flexran::core::rt::rt_sleep_ns (rtime x) {
  int ret;
  clock_gettime(clock_id, &now);
  interval.tv_sec = x/((rtime)1000000000);
  interval.tv_nsec = x%((rtime)1000000000);
  //rt_printk("sleeping for %d sec and %d ns\n",interval.tv_sec,interval.tv_nsec);
  next = now;
  next.tv_sec += interval.tv_sec;
  next.tv_nsec += interval.tv_nsec;

  if (next.tv_nsec>=1000000000) {
    next.tv_nsec -= 1000000000;
    next.tv_sec++;
  }

  ret = clock_nanosleep(clock_id, TIMER_ABSTIME, &next, NULL);

  return(ret);
}

void flexran::core::rt::check_clock(void) {
  if (clock_getres(clock_id, &res)) {
    //printf("clock_getres failed");
  } else {
    //printf("reported resolution = %llu ns\n", (long long int) ((int) 1e9 * res.tv_sec) + (long long int) res.tv_nsec);
  }
}


#ifdef LOWLATENCY
int flexran::core::rt::sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags) {
  return syscall(__NR_sched_setattr, pid, attr, flags);
}


int flexran::core::rt::sched_getattr(pid_t pid,struct sched_attr *attr,unsigned int size, unsigned int flags) {

  return syscall(__NR_sched_getattr, pid, attr, size, flags);
}


#endif
