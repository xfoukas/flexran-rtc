#ifndef ASYNC_XFACE_H_
#define ASYNC_XFACE_H_

#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

#include "tagged_message.h"
#include "connection_manager.h"
#include "rt_task.h"

namespace progran {

  namespace network {
    
    class connection_manager;
    class async_xface : public progran::core::rt::rt_task {
    public:
    async_xface(int port):port_(port), endpoint_(boost::asio::ip::tcp::v4(), port), rt_task(Policy::FIFO) {}
      
      void run();
      
      void establish_xface();
      
      void forward_message(tagged_message *msg);
      
      bool get_msg_from_network(std::shared_ptr<tagged_message>& msg);
      
      bool send_msg(const protocol::progran_message& msg, int agent_tag) const;
      
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
