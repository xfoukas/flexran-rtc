#include "agent_session.h"
#include "tagged_message.h"

void agent_session::start() {
  do_read_header();
}

void agent_session::deliver(const tagged_message& msg) {
  bool write_in_progress = !write_queue_.empty();
  protocol_message data;
  std::size_t size = msg.getSize();
  data.set_message(msg.getMessageContents(), size);
  write_queue_.push_back(data);
  if (!write_in_progress) {
      do_write();
  }
}

void agent_session::do_read_header() {
  boost::asio::async_read(socket_,
			  boost::asio::buffer(read_msg_.data(), protocol_message::header_length),
			  [this](boost::system::error_code ec, std::size_t /*length*/) {
			    if (!ec && read_msg_.decode_header()) {
			      do_read_body();
			    }
			    else {
			      socket_.close();
			    }
			  });
}

void agent_session::do_read_body() {
  boost::asio::async_read(socket_,
			  boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			  [this](boost::system::error_code ec, std::size_t /*length*/) {
			    if (!ec) {
			      tagged_message *th = new tagged_message(read_msg_.body(),
								    read_msg_.body_length(),
								    session_id_);
			      xface_.forward_message(th);
			      do_read_header();
			    } else {
			      socket_.close();
			    }
			  });
}

void agent_session::do_write() {
  auto self(shared_from_this());

  boost::asio::async_write(socket_,
			   boost::asio::buffer(write_queue_.front().data(),
					       write_queue_.front().length()),
			   [this, self](boost::system::error_code ec, std::size_t ) {
    if (!ec) {
      write_queue_.pop_front();
      if (!write_queue_.empty()) {
	do_write();
      }
    } else {
      manager_.close_connection(session_id_);
    }
			   });
}
