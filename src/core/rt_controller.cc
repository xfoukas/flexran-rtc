#include <thread>
#include <iostream>

#include "async_xface.h"
#include "progran.pb.h"

int main(int argc, char *argv[]) {

  async_xface net_xface(2210);

  std::thread networkThread(&async_xface::establish_xface, &net_xface);

  while(true) {
    tagged_message *msg;
    // Check if there is a message coming from a controller
    if (net_xface.get_msg_from_network(&msg)) {
      if (msg->getSize() == 0) {  //New connection, send a hello msg
	std::cout << "Oooh, got something with no length! Need to initiate a new connection" << std::endl;
	protocol::prp_header *header(new protocol::prp_header);
	header->set_type(protocol::PRPT_HELLO);
	header->set_version(0);
	header->set_xid(0);
	
	protocol::prp_hello *hello_msg(new protocol::prp_hello);
	hello_msg->set_allocated_header(header);
	
	protocol::progran_message out_message;
	out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
	out_message.set_allocated_hello_msg(hello_msg);
	net_xface.send_msg(out_message, msg->getTag());
	delete msg;
      } else { // Send a stats request
	protocol::progran_message in_message;
	in_message.ParseFromArray(msg->getMessageContents(), msg->getSize());
	std::cout << in_message.DebugString() << std::endl;
	if (in_message.has_hello_msg()) {
	  std::cout << "This was indeed a hello message" << std::endl;
	  std::cout << "Time to request some statistics" << std::endl;

	  protocol::prp_header *header(new protocol::prp_header);
	  header->set_type(protocol::PRPT_STATS_REQUEST);
	  header->set_version(0);
	  header->set_xid(0);

	  protocol::prp_complete_stats_request *req(new protocol::prp_complete_stats_request);
	  req->set_sf(100);
	  req->set_report_frequency(protocol::PRSRF_PERIODICAL);

	  int ue_flags = 0;
	  ue_flags |= protocol::PRUST_PRH;
	  ue_flags |= protocol::PRUST_DL_CQI;
	  ue_flags |= protocol::PRUST_RLC_BS;
	  ue_flags |= protocol::PRUST_MAC_CE_BS;
	  req->set_ue_report_flags(ue_flags);

	  int cell_flags = 0;
	  cell_flags |= protocol::PRCST_NOISE_INTERFERENCE;
	  req->set_cell_report_flags(cell_flags);
	  
	  protocol::prp_stats_request *stats_request_msg(new protocol::prp_stats_request);
	  stats_request_msg->set_type(protocol::PRST_COMPLETE_STATS);
	  stats_request_msg->set_allocated_header(header);
	  stats_request_msg->set_allocated_complete_stats_request(req);
	  
	  protocol::progran_message out_message;
	  out_message.set_msg_dir(protocol::INITIATING_MESSAGE);
	  out_message.set_allocated_stats_request_msg(stats_request_msg);
	  net_xface.send_msg(out_message, msg->getTag());
	}
	delete msg;
      }
    }
  }
  
  if (networkThread.joinable())
    networkThread.join();
  
  return 0;
  
}
