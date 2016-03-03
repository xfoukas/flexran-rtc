#ifndef REMOTE_SCHEDULER_PRIMITIVES_H_
#define REMOTE_SCHEDULER_PRIMITIVES_H_

#include "progran.pb.h"
#include "rib_common.h"

#include "periodic_component.h"
#include "enb_scheduling_info.h"
#include "ue_scheduling_info.h"

bool needs_scheduling(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			frame_t curr_frame, subframe_t curr_subframe);

bool CCE_allocation_infeasible(std::shared_ptr<enb_scheduling_info>& enb_sched_info,
			       const protocol::prp_cell_config& cell_config,
			       const protocol::prp_ue_config& ue_config,
			       uint8_t aggregation,
			       subframe_t subframe);

uint16_t get_min_rb_unit(const protocol::prp_cell_config& cell_config);

uint16_t get_nb_rbg(const protocol::prp_cell_config& cell_config);

uint32_t get_TBS_DL(uint8_t mcs, uint16_t nb_rb);

unsigned char get_I_TBS(unsigned char I_MCS);

uint8_t get_mi(const protocol::prp_cell_config& cell_config, subframe_t subframe);

uint16_t get_nCCE_max(uint8_t num_pdcch_symbols,
		 const protocol::prp_cell_config& cell_config,
		 subframe_t subframe);

uint16_t get_nquad(uint8_t num_pdcch_symbols,
	      const protocol::prp_cell_config& cell_config,
	      uint8_t mi);

int get_nCCE_offset(const uint8_t aggregation,
		    const int nCCE,
		    const int common_dci,
		    const rnti_t rnti,
		    const subframe_t subframe);

uint32_t allocate_prbs_sub(int nb_rb, uint8_t *rballoc, const protocol::prp_cell_config& cell_config);

#endif
