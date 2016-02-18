#include "cell_mac_rib_info.h"

void cell_mac_rib_info::update_cell_stats_report(const protocol::prp_cell_stats_report& stats_report) {
  uint32_t flags = stats_report.flags();

  // Check the fields that need to be updated
  if (flags & protocol::PRCST_NOISE_INTERFERENCE) {
    cell_stats_report_.mutable_noise_inter_report()->CopyFrom(stats_report.noise_inter_report());
  }
}
