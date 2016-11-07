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

#include <string>
#include <fstream>
#include <streambuf>

#include "delegation_manager.h"

// Example app for using control delegation
void flexran::app::management::delegation_manager::run_periodic_task() {
  

  ::std::set<int> agent_ids = ::std::move(rib_.get_available_agents());

  for (const auto& agent_id : agent_ids) {
    
    ::std::shared_ptr<const rib::enb_rib_info> agent_config = rib_.get_agent(agent_id);
    rib::subframe_t current_subframe = agent_config->get_current_subframe();
    rib::frame_t current_frame = agent_config->get_current_frame();

    int time = (current_frame * 10) + current_subframe;

    if (time == 2000) {
      if (delegation_steps_[0] == false) {
	//Push remote scheduler code
	push_code(agent_id, "schedule_ue_spec_remote", "../tests/delegation_control/libremote_sched.so");
	delegation_steps_[0] = true;
      }
    } else if (time == 2500) {
       if (delegation_steps_[1] == false) {
	 // Push local scheduler code
	 push_code(agent_id, "schedule_ue_spec_default", "../tests/delegation_control/libdefault_sched.so"); 
	 delegation_steps_[1] = true;
      }

    } else if (time == 4000) {
       if (delegation_steps_[2] == false) {
	 // Load remote scheduler and change its default parameters
	 reconfigure_agent(agent_id, "../tests/delegation_control/remote_policy.yaml");
	delegation_steps_[2] = true;
      }
      
    } else if (time == 6000) {
       if (delegation_steps_[3] == false) {
	 // Change remote scheduler parameters
	 reconfigure_agent(agent_id, "../tests/delegation_control/remote_policy2.yaml");
	 delegation_steps_[3] = true;
      }
    } else if (time == 8000) {
       if (delegation_steps_[4] == false) {
	 // Load local scheduler
	 reconfigure_agent(agent_id, "../tests/delegation_control/local_policy.yaml");
	 delegation_steps_[4] = true;
       } 
    } else if (time == 10000) {
       if (delegation_steps_[5] == false) {
	 // Change local scheduler parameters
	 reconfigure_agent(agent_id, "../tests/delegation_control/local_policy2.yaml");
	 delegation_steps_[5] = true;
       }
    }    
  }
}

void flexran::app::management::delegation_manager::reconfigure_agent(int agent_id, std::string policy_name) {
  std::ifstream policy_file(policy_name);
  std::string str_policy;

  policy_file.seekg(0, std::ios::end);
  str_policy.reserve(policy_file.tellg());
  policy_file.seekg(0, std::ios::beg);

  str_policy.assign((std::istreambuf_iterator<char>(policy_file)),
		    std::istreambuf_iterator<char>());

  protocol::flexran_message config_message;
  // Create control delegation message header
  protocol::flex_header *config_header(new protocol::flex_header);
  config_header->set_type(protocol::FLPT_RECONFIGURE_AGENT);
  config_header->set_version(0);
  config_header->set_xid(0);
  
  protocol::flex_agent_reconfiguration *agent_reconfiguration_msg(new protocol::flex_agent_reconfiguration);
  agent_reconfiguration_msg->set_allocated_header(config_header);

  agent_reconfiguration_msg->set_policy(str_policy);

  config_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  config_message.set_allocated_agent_reconfiguration_msg(agent_reconfiguration_msg);
  req_manager_.send_message(agent_id, config_message);
}

void flexran::app::management::delegation_manager::push_code(int agent_id, std::string function_name, std::string lib_name) {
  protocol::flexran_message d_message;
  // Create control delegation message header
  protocol::flex_header *delegation_header(new protocol::flex_header);
  delegation_header->set_type(protocol::FLPT_DELEGATE_CONTROL);
  delegation_header->set_version(0);
  delegation_header->set_xid(0);
  
  protocol::flex_control_delegation *control_delegation_msg(new protocol::flex_control_delegation);
  control_delegation_msg->set_allocated_header(delegation_header);
  control_delegation_msg->set_delegation_type(protocol::FLCDT_MAC_DL_UE_SCHEDULER);
  
  ::std::ifstream fin(lib_name, std::ios::in | std::ios::binary);
  fin.seekg( 0, std::ios::end );  
  size_t len = fin.tellg();
  char *ret = new char[len];  
  fin.seekg(0, std::ios::beg);   
  fin.read(ret, len);  
  fin.close();
  std::string test(ret, len);
  control_delegation_msg->set_payload(ret, len);
  control_delegation_msg->set_name(function_name);
  // Create and send the flexran message
  d_message.set_msg_dir(protocol::INITIATING_MESSAGE);
  d_message.set_allocated_control_delegation_msg(control_delegation_msg);
  req_manager_.send_message(agent_id, d_message);
}
