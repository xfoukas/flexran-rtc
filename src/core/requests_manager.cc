#include "requests_manager.h"

void requests_manager::send_message(int agent_id, const protocol::progran_message& msg) const {
  net_xface_.send_msg(msg, agent_id); 
}
