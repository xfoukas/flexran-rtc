#ifndef REMOTE_SCHEDULER_H_
#define REMOTE_SCHEDULER_H_

#include <map>

#include "periodic_component.h"

class remote_scheduler : public periodic_component {

 public:

  remote_scheduler(const Rib& rib, const requests_manager& rm)
    : periodic_component(rib, rm) {}

  void run_periodic_task();

 private:

  std::map<int, int> scheduling_info;
    
};

#endif
