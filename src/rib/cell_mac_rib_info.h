#ifndef CELL_MAC_RIB_INFO_H_
#define CELL_MAC_RIB_INFO_H_

#include <cstdint>

#include "progran.pb.h"

class cell_mac_rib_info {
 public:

  void update_cell_stats_report(const protocol::prp_cell_stats_report& stats_report);
  
 private:
  protocol::prp_cell_stats_report cell_stats_report_;

};

#endif
