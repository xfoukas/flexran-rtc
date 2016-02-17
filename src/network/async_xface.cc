#include <boost/bind.hpp>

#include "async_xface.h"
#include "rt_wrapper.h"

void async_xface::run() {
  establish_xface();
}

void async_xface::establish_xface() {
  manager_.reset(new connection_manager(io_service, endpoint_, *this));
  
  io_service.run();
}

void async_xface::forward_message(tagged_message *msg) {
  in_queue_.push(msg);
}

bool async_xface::get_msg_from_network(tagged_message **msg) {
  return in_queue_.pop(*msg);
}

bool async_xface::send_msg(protocol::progran_message& msg, int agent_tag) {
  tagged_message *tm =  new tagged_message(msg.ByteSize(), agent_tag);
  msg.SerializeToArray(tm->getMessageArray(), msg.ByteSize());
  if (out_queue_.push(tm)) {
    io_service.post(boost::bind(&async_xface::forward_msg_to_agent, this));
    return true;
  } else {
    return false;
  }
}

void async_xface::forward_msg_to_agent() {
  tagged_message *msg;
  out_queue_.pop(msg);
  std::shared_ptr<tagged_message> message(msg);
  manager_->send_msg_to_agent(message);
}


void async_xface::initialize_connection(int session_id) {
  tagged_message *th = new tagged_message(0, session_id);
  in_queue_.push(th);
}
