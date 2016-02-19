#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "rt_task.h"
#include "rib.h"
#include "requests_manager.h"

class component : public rt_task {
 public:
  
 component(const Rib& rib, const requests_manager& rm)
   : rib_(rib), req_manager_(rm), rt_task(Policy::DEADLINE,
					  8 * 100 * 1000,
					  8 * 100 * 1000,
					  1000 * 1000) {}

  virtual void run_app() = 0;

 protected:
  const Rib& rib_;
  const requests_manager& req_manager_;
  
 private:

  void run() { run_app(); }

};

#endif
