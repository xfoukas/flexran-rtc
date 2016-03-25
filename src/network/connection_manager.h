#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <boost/asio.hpp>
#include <unordered_map>

#include "agent_session.h"
#include "async_xface.h"

namespace progran {

  namespace network {

    class async_xface;
    class agent_session;
    class connection_manager:
      public std::enable_shared_from_this<connection_manager> {
      
    public:
      connection_manager(boost::asio::io_service & io_service,
			 const boost::asio::ip::tcp::endpoint& endpoint,
			 async_xface& xface);
      
      void close_connection(int session_id);
      
      void send_msg_to_agent(std::shared_ptr<tagged_message> msg);
      
    private:
      
      void do_accept();
      
      boost::asio::ip::tcp::acceptor acceptor_;
      boost::asio::ip::tcp::socket socket_;
      
      std::unordered_map<int, std::shared_ptr<agent_session>> sessions_;
      int next_id_;
      async_xface& xface_;
    };

  }
  
}   

#endif
