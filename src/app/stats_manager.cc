#include "stats_manager.h"
#include "progran.pb.h"

void stats_manager::run_periodic_task() {

  // Simply request stats for any registered eNB and print them
  std::set<int> current_agents = rib_.get_available_agents();

  for (auto agent_id : current_agents) {
    auto it = agent_list_.find(agent_id);
    if (it == agent_list_.end()) {
      agent_list_.insert(agent_id);
      // Make a new stats request for the newly added agents
      protocol::prp_header *header(new protocol::prp_header);
      header->set_type(protocol::PRPT_STATS_REQUEST);
      header->set_version(0);
      // We need to store the xid for keeping context info
      header->set_xid(0);

      protocol::prp_complete_stats_request *complete_stats_request(new protocol::prp_complete_stats_request);
      complete_stats_request->set_report_frequency(protocol::PRSRF_CONTINUOUS);
      complete_stats_request->set_sf(2);
      int ue_flags = 0;
      ue_flags |= protocol::PRUST_PRH;                                                                            
      ue_flags |= protocol::PRUST_DL_CQI;
      ue_flags |= protocol::PRUST_RLC_BS;
      ue_flags |= protocol::PRUST_MAC_CE_BS;
      complete_stats_request->set_ue_report_flags(ue_flags);
      int cell_flags = 0;
      cell_flags |= protocol::PRCST_NOISE_INTERFERENCE;
      complete_stats_request->set_cell_report_flags(cell_flags);
      protocol::prp_stats_request *stats_request_msg(new protocol::prp_stats_request);
      stats_request_msg->set_allocated_header(header);
      stats_request_msg->set_type(protocol::PRST_COMPLETE_STATS);
      stats_request_msg->set_allocated_complete_stats_request(complete_stats_request);
      protocol::progran_message msg;
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
