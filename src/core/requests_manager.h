#ifndef REQUESTS_MANAGER_H_
#define REQUESTS_MANAGER_H_

#include "async_xface.h"
#include "flexran.pb.h"

namespace flexran {

  namespace core {

    class requests_manager {

    public:
    requests_manager(flexran::network::async_xface& xface) : net_xface_(xface) {}
      
      void send_message(int agent_id, const protocol::flexran_message& msg) const;
      
    private:
      
      flexran::network::async_xface& net_xface_;
      
    };

  }

}

#endif
