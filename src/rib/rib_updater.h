#ifndef RIB_UPDATER_H_
#define RIB_UPDATER_H_

#include "async_xface.h"
#include "progran.pb.h"
#include "rib.h"
#include "progran.pb.h"
#include "rt_task.h"

class rib_updater : public rt_task {

 public:
 rib_updater(async_xface& xface, Rib& storage)
   : net_xface_(xface), rib_(storage),  messages_to_check_(50), rt_task(Policy::DEADLINE,
									2 * 100 * 1000,
									2 * 100 * 1000,
									1000 * 1000) {}

 rib_updater(async_xface& xface, Rib& storage, int n_msg_check)
   : net_xface_(xface), rib_(storage), messages_to_check_(n_msg_check), rt_task(Policy::DEADLINE,
										2 * 100 * 1000,
										2 * 100 * 1000,
										1000* 1000) {}

  void run();
  
  void update_rib();

  // Incoming message handlers
  void handle_message(int agent_id,
		      const protocol::prp_hello& hello_msg,
		      protocol::progran_direction dir);
  void handle_message(int agent_id,
		      const protocol::prp_echo_request& echo_request_msg);
  void handle_message(int agent_id,
		      const protocol::prp_echo_reply& echo_reply_msg);
  void handle_message(int agent_id,
		      const protocol::prp_sf_trigger& sf_trigger_msg);
  void handle_message(int agent_id,
		      const protocol::prp_enb_config_reply& enb_config_reply_msg);
  void handle_message(int agent_id,
		      const protocol::prp_ue_config_reply& ue_config_reply_msg);
  void handle_message(int agent_id,
		      const protocol::prp_lc_config_reply& lc_config_reply_msg);
  
  
 private:
  
  async_xface& net_xface_;
  Rib& rib_;
	      
  // Max number of messages to check during a single update period
  std::atomic<int> messages_to_check_;
  
};

#endif
