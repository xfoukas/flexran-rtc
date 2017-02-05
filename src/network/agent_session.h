/* The MIT License (MIT)

   Copyright (c) 2016 Xenofon Foukas

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

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
      : socket_(std::move(socket)), session_id_(session_id), manager_(manager), xface_(xface) {
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
