#ifndef CELL_MAC_RIB_INFO_H_
#define CELL_MAC_RIB_INFO_H_

#include <cstdint>

#include "flexran.pb.h"

namespace flexran {

  namespace rib {

    class cell_mac_rib_info {
    public:

      void update_cell_stats_report(const protocol::flex_cell_stats_report& stats_report);

      const protocol::flex_cell_stats_report& get_cell_stats_report() const {
	return cell_stats_report_;
      }
  
    private:
      protocol::flex_cell_stats_report cell_stats_report_;

    };

  }

}

#endif
