#include "connection_manager.h"

connection_manager::connection_manager(boost::asio::io_service& io_service,
				       const boost::asio::ip::tcp::endpoint& endpoint,
				       async_xface& xface)
  : acceptor_(io_service, endpoint), socket_(io_service), xface_(xface), next_id_(0) {

  do_accept();

}

void connection_manager::send_msg_to_agent(std::shared_ptr<tagged_message> msg) {
  sessions_[msg->getTag()]->deliver(msg);
}

void connection_manager::do_accept() {

  acceptor_.async_accept(socket_,
			 [this](boost::system::error_code ec) {
      if (!ec) {
	sessions_[next_id_] = std::make_shared<agent_session>(std::move(socket_), *this, xface_, next_id_);
	sessions_[next_id_]->start();
	xface_.initialize_connection(next_id_);
	next_id_++;
      }
      
      do_accept();
			 });
}

void connection_manager::close_connection(int session_id) {
  sessions_.erase(session_id);
}
