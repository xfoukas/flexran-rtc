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

#include "rib_common.h"

flexran::rib::frame_t flexran::rib::get_frame(uint32_t sfn_sf) {
  uint16_t frame_mask = ((1<<12) - 1);
  return (sfn_sf & (frame_mask << 4)) >> 4;
}

flexran::rib::subframe_t flexran::rib::get_subframe(uint32_t sfn_sf) {
  uint16_t sf_mask = ((1<<4) - 1);
  return (sfn_sf & sf_mask);
}

std::pair<flexran::rib::frame_t, flexran::rib::subframe_t> flexran::rib::get_frame_subframe(uint32_t sfn_sf) {
  uint32_t frame_mask =  ((1<<12) - 1);
  frame_t frame = (sfn_sf & (frame_mask << 4)) >> 4;
  uint32_t sf_mask = ((1<<4) - 1);
  subframe_t subframe =  (sfn_sf & sf_mask);
  return std::pair<frame_t, subframe_t>(frame, subframe);
}

uint16_t flexran::rib::get_sfn_sf(frame_t frame, subframe_t subframe) {
  uint16_t sfn_sf, frame_mask, sf_mask;

  frame_mask = ((1<<12) - 1);
  sf_mask = ((1<<4) -1);
  sfn_sf = (subframe & sf_mask) | ((frame & frame_mask) << 4);
  
  return sfn_sf;
}
