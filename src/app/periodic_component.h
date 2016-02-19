#ifndef PERIODIC_COMPONENT_H_
#define PERIODIC_COMPONENT_H_

#include "component.h"

class periodic_component : public component {

 public:

 periodic_component(const Rib& rib, const requests_manager& rm)
   : component(rib, rm) {}
  
  virtual void run_periodic_task() = 0;

  void run_app() { run_periodic_task(); }
  
};

#endif
