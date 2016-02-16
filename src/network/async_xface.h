#ifndef ASYNC_XFACE_H_
#define ASYNC_XFACE_H_

#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

#include "tagged_message.h"
#include "connection_manager.h"

class connection_manager;
class async_xface {
 public:
 async_xface(int port):port_(port), endpoint_(boost::asio::ip::tcp::v4(), port) { }
  
  void establish_xface();

  void forward_message(tagged_message *msg);

  bool get_msg_from_network(tagged_message **msg);

  bool send_msg(protocol::progran_message& msg, int agent_tag);

  void forward_msg_to_agent();

  void initialize_connection(int session_id);
  
 private:

  boost::asio::io_service io_service;

  boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> in_queue_{10000};
  boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> out_queue_{10000};

  boost::asio::ip::tcp::endpoint endpoint_;

  std::unique_ptr<connection_manager> manager_;
  
  int port_;
  
};

#endif
