#ifndef RIB_COMMON_H_
#define RIB_COMMON_H_

#include <cstdint>
#include <utility>

typedef uint16_t frame_t;
typedef uint16_t subframe_t;

typedef uint32_t rnti_t;

static int const MAX_NUM_HARQ = 8;
static int const MAX_NUM_TB = 2;
static int const MAX_NUM_CC = 2;
static int const MAX_NUM_LC = 11;

frame_t get_frame(uint32_t sfn_sf);

subframe_t get_subframe(uint32_t sfn_sf);

std::pair<frame_t, subframe_t> get_frame_subframe(uint32_t sfn_sf);

#endif
