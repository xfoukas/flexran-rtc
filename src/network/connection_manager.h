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

#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <boost/asio.hpp>
#include <unordered_map>

#include "agent_session.h"
#include "async_xface.h"

namespace flexran {

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
