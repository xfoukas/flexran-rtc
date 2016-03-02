#include "remote_scheduler_primitives.h"

bool needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
					frame_t curr_frame, subframe_t curr_subframe) {
  frame_t last_checked_frame = enb_sched_info->get_last_checked_frame();
  subframe_t last_checked_subframe = enb_sched_info->get_last_checked_subframe();

  return !(last_checked_frame == curr_frame && last_checked_subframe == curr_subframe);
}

uint16_t get_min_rb_unit(const protocol::prp_cell_config& cell_config) {
  int min_rb_unit = 0;
  switch (cell_config.dl_bandwidth()) {
  case 6:
    min_rb_unit = 1;
    break;
  case 25:
    min_rb_unit = 2;
    break;
  case 50:
    min_rb_unit = 3;
    break;
  case 100:
    min_rb_unit = 4;
    break;
  default:
    min_rb_unit = 2;
    break;
  }
  return min_rb_unit;
}

uint16_t get_nb_rbg(const protocol::prp_cell_config& cell_config) {
  uint16_t nb_rbg;
  // Get the number of dl RBs
  switch(cell_config.dl_bandwidth()) {
  case 100:
    nb_rbg = 25;
    break;
  case 75:
    nb_rbg = 19;
    break;
  case 50:
    nb_rbg = 17;
    break;
  case 25:
    nb_rbg = 13;
    break;
  case 15:
    nb_rbg = 8;
    break;
  case 6:
    nb_rbg = 6;
    break;
  default:
    nb_rbg = 0;
    break;
  }
  return nb_rbg;
}

uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb) {

  uint32_t TBS;

  if ((nb_rb > 0) && (mcs < 29)) {
#ifdef TBS_FIX
    TBS = 3*TBStable[get_I_TBS(mcs)][nb_rb-1]/4;
    TBS = TBS>>3;
#else
    TBS = TBStable[get_I_TBS(mcs)][nb_rb-1];
    TBS = TBS>>3;
#endif
    return(TBS);
  } else {
    return(uint32_t)0;
  }
}

unsigned char get_I_TBS(unsigned char I_MCS) {

  if (I_MCS < 10)
    return(I_MCS);
  else if (I_MCS == 10)
    return(9);
  else if (I_MCS < 17)
    return(I_MCS-1);
  else if (I_MCS == 17)
    return(15);
  else return(I_MCS-2);

}
