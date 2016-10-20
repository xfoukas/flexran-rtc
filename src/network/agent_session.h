#ifndef AGENT_SESSION_H_
#define AGENT_SESSION_H_

#include <deque>
#include <boost/asio.hpp>

#include "flexran.pb.h"
#include "connection_manager.h"
#include "protocol_message.h"
#include "async_xface.h"

namespace flexran {

  namespace network {
  
    typedef std::deque<protocol_message> flexran_protocol_queue;

    class async_xface;
    class connection_manager;
    class agent_session :
      public std::enable_shared_from_this<agent_session> {
      
    public:
    agent_session(boost::asio::ip::tcp::socket socket,
		  connection_manager& manager,
		  async_xface& xface,
		  int session_id)
      : socket_(std::move(socket)), manager_(manager), xface_(xface), session_id_(session_id) {
	socket_.set_option(boost::asio::ip::tcp::no_delay(true));
      }
      
      void start();

      void deliver(std::shared_ptr<tagged_message> msg);
      
    private:
      
      void do_read_header();
      void do_read_body();
      void do_write();
      
      boost::asio::ip::tcp::socket socket_;
      flexran_protocol_queue write_queue_;
      
      
      protocol_message read_msg_;
      int session_id_;
      connection_manager& manager_;
      async_xface& xface_;
    };
  }
}
#endif
