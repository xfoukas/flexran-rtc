#include "requests_manager.h"

void flexran::core::requests_manager::send_message(int agent_id, const protocol::flexran_message& msg) const {
  net_xface_.send_msg(msg, agent_id); 
}
