#ifndef REQUESTS_MANAGER_H_
#define REQUESTS_MANAGER_H_

#include "async_xface.h"
#include "progran.pb.h"

namespace progran {

  namespace core {

    class requests_manager {

    public:
    requests_manager(progran::network::async_xface& xface) : net_xface_(xface) {}
      
      void send_message(int agent_id, const protocol::progran_message& msg) const;
      
    private:
      
      progran::network::async_xface& net_xface_;
      
    };

  }

}

#endif
