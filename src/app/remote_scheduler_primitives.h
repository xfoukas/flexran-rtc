#ifndef REMOTE_SCHEDULER_PRIMITIVES_H_
#define REMOTE_SCHEDULER_PRIMITIVES_H_

#include "progran.pb.h"
#include "rib_common.h"

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

bool needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			frame_t curr_frame, subframe_t curr_subframe);

uint16_t get_min_rb_unit(const protocol::prp_cell_config& cell_config);

uint16_t get_nb_rbg(const protocol::prp_cell_config& cell_config);

uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);

unsigned char get_I_TBS(unsigned char I_MCS);

#endif
