#include <boost/bind.hpp>

#include "async_xface.h"
#include "rt_wrapper.h"

void progran::network::async_xface::run() {
  establish_xface();
}

void progran::network::async_xface::establish_xface() {
  manager_.reset(new connection_manager(io_service, endpoint_, *this));
  work_ptr_.reset(new boost::asio::io_service::work(io_service));
  io_service.run();
}

void progran::network::async_xface::forward_message(tagged_message *msg) {
  in_queue_.push(msg);
}

bool progran::network::async_xface::get_msg_from_network(std::shared_ptr<tagged_message>& msg) {
  return in_queue_.consume_one([&] (tagged_message *tm) {
      std::shared_ptr<tagged_message> p(std::move(tm));
      msg = p;});
}

bool progran::network::async_xface::send_msg(const protocol::progran_message& msg, int agent_tag) const {
  tagged_message *tm =  new tagged_message(msg.ByteSize(), agent_tag);
  msg.SerializeToArray(tm->getMessageArray(), msg.ByteSize());
  if (out_queue_.push(tm)) {
    io_service.post(boost::bind(&async_xface::forward_msg_to_agent, self_));
    return true;
  } else {
    return false;
  }
}

void progran::network::async_xface::forward_msg_to_agent() {
  tagged_message *msg;
  out_queue_.pop(msg);
  std::shared_ptr<tagged_message> message(msg);
  manager_->send_msg_to_agent(message);
}


void progran::network::async_xface::initialize_connection(int session_id) {
  tagged_message *th = new tagged_message(0, session_id);
  in_queue_.push(th);
}
