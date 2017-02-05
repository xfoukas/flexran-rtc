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

#ifndef RIB_UPDATER_H_
#define RIB_UPDATER_H_

#include "async_xface.h"
#include "rib.h"
#include "flexran.pb.h"
#include "rt_task.h"

namespace flexran {

  namespace rib {

    class rib_updater : public flexran::core::rt::rt_task {

    public:
    rib_updater(flexran::network::async_xface& xface, Rib& storage)
      : rt_task(Policy::DEADLINE, 2 * 100 * 1000, 2 * 100 * 1000, 1000 * 1000),
	net_xface_(xface), rib_(storage), messages_to_check_(350) {} 
      
    rib_updater(flexran::network::async_xface& xface, Rib& storage, int n_msg_check)
      : rt_task(Policy::DEADLINE, 2 * 100 * 1000, 2 * 100 * 1000, 1000 * 1000),
	net_xface_(xface), rib_(storage), messages_to_check_(n_msg_check) {}
      
      void run();
      
      void update_rib();
      
      // Incoming message handlers
      void handle_message(int agent_id,
			  const protocol::flex_hello& hello_msg,
			  protocol::flexran_direction dir);
      void handle_message(int agent_id,
			  const protocol::flex_echo_request& echo_request_msg);
      void handle_message(int agent_id,
			  const protocol::flex_echo_reply& echo_reply_msg);
      void handle_message(int agent_id,
			  const protocol::flex_sf_trigger& sf_trigger_msg);
      void handle_message(int agent_id,
			  const protocol::flex_enb_config_reply& enb_config_reply_msg);
      void handle_message(int agent_id,
			  const protocol::flex_ue_config_reply& ue_config_reply_msg);
      void handle_message(int agent_id,
			  const protocol::flex_lc_config_reply& lc_config_reply_msg);
      void handle_message(int agent_id,
			  const protocol::flex_stats_reply& mac_stats_reply);
      void handle_message(int agent_id,
			  const protocol::flex_ue_state_change& ue_state_change_msg);
      
      
    private:
      
      flexran::network::async_xface& net_xface_;
      Rib& rib_;
      
      // Max number of messages to check during a single update period
      std::atomic<int> messages_to_check_;
      
    };

  }

}

#endif
