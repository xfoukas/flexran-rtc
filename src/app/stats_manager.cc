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

#include <iostream>

#include "stats_manager.h"
#include "flexran.pb.h"

void flexran::app::stats::stats_manager::run_periodic_task() {

  // Simply request stats for any registered eNB and print them
  std::set<int> current_agents = rib_.get_available_agents();

  for (auto agent_id : current_agents) {
    auto it = agent_list_.find(agent_id);
    if (it == agent_list_.end()) {
      agent_list_.insert(agent_id);
      // Make a new stats request for the newly added agents
      protocol::flex_header *header(new protocol::flex_header);
      header->set_type(protocol::FLPT_STATS_REQUEST);
      header->set_version(0);
      // We need to store the xid for keeping context info
      header->set_xid(0);

      protocol::flex_complete_stats_request *complete_stats_request(new protocol::flex_complete_stats_request);
      complete_stats_request->set_report_frequency(protocol::FLSRF_CONTINUOUS);
      complete_stats_request->set_sf(2);
      int ue_flags = 0;
      ue_flags |= protocol::FLUST_PRH;                                                                            
      ue_flags |= protocol::FLUST_DL_CQI;
      ue_flags |= protocol::FLUST_RLC_BS;
      ue_flags |= protocol::FLUST_MAC_CE_BS;
      ue_flags |= protocol::FLUST_UL_CQI;
      complete_stats_request->set_ue_report_flags(ue_flags);
      int cell_flags = 0;
      cell_flags |= protocol::FLCST_NOISE_INTERFERENCE;
      complete_stats_request->set_cell_report_flags(cell_flags);
      protocol::flex_stats_request *stats_request_msg(new protocol::flex_stats_request);
      stats_request_msg->set_allocated_header(header);
      stats_request_msg->set_type(protocol::FLST_COMPLETE_STATS);
      stats_request_msg->set_allocated_complete_stats_request(complete_stats_request);
      protocol::flexran_message msg;
      msg.set_msg_dir(protocol::INITIATING_MESSAGE);
      msg.set_allocated_stats_request_msg(stats_request_msg);
      req_manager_.send_message(agent_id, msg);
      std::cout << "Time to make a new request for stats" << std::endl;
    }
  }
  
  times_executed_++;

  if (times_executed_ == 100000) {
    // Dump all the stats
    std::cout << "***************" << std::endl;
    std::cout << "Configurations" << std::endl;
    std::cout << "***************" << std::endl;
    rib_.dump_enb_configurations();
    std::cout << "***************" << std::endl;
    std::cout << "MAC stats" << std::endl;
    std::cout << "***************" << std::endl;
    rib_.dump_mac_stats();
    times_executed_ = 0;
  }
}
