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

#ifndef ASYNC_XFACE_H_
#define ASYNC_XFACE_H_

#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

#include "tagged_message.h"
#include "connection_manager.h"
#include "rt_task.h"

namespace flexran {

  namespace network {
    
    class connection_manager;
    class async_xface : public flexran::core::rt::rt_task {
    public:
    async_xface(int port): rt_task(Policy::FIFO), endpoint_(boost::asio::ip::tcp::v4(), port), port_(port)  {}
      
      void run();
      
      void establish_xface();
      
      void forward_message(tagged_message *msg);
      
      bool get_msg_from_network(std::shared_ptr<tagged_message>& msg);
      
      bool send_msg(const protocol::flexran_message& msg, int agent_tag) const;
      
      void forward_msg_to_agent();

      void initialize_connection(int session_id);
      
    private:
      
      mutable boost::asio::io_service io_service;
      std::unique_ptr<boost::asio::io_service::work> work_ptr_;

      mutable boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> in_queue_{10000};
      mutable boost::lockfree::queue<tagged_message *, boost::lockfree::fixed_sized<true>> out_queue_{10000};

      mutable boost::asio::ip::tcp::endpoint endpoint_;

      std::unique_ptr<connection_manager> manager_;
  
      int port_;

      mutable async_xface* self_ = this;   
    };

  }

}

#endif
