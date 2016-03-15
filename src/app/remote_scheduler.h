#ifndef REMOTE_SCHEDULER_H_
#define REMOTE_SCHEDULER_H_

#include <map>

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

class remote_scheduler : public periodic_component {

 public:

  remote_scheduler(const Rib& rib, const requests_manager& rm)
    : periodic_component(rib, rm) {}

  void run_periodic_task();

  static int32_t tpc_accumulated;
  
 private:

  std::shared_ptr<enb_scheduling_info> get_scheduling_info(int agent_id);
  
  std::map<int, std::shared_ptr<enb_scheduling_info>> scheduling_info_;
  
  // Set these values internally for now

  const int schedule_ahead = 1;

};

#endif
