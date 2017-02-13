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

#ifndef RIB_COMMON_H_
#define RIB_COMMON_H_

#include <cstdint>
#include <utility>

namespace flexran {

  namespace rib {

    typedef uint16_t frame_t;
    typedef uint16_t subframe_t;

    typedef uint32_t rnti_t;
    
    static int const MAX_NUM_HARQ = 8;
    static int const MAX_NUM_TB = 2;
    static int const MAX_NUM_CC = 2;
    static int const MAX_NUM_LC = 11;
    static int const MAX_NUM_UE = 1024;
    
    static int const N_RBG_MAX = 25; // for 20MHz channel BW
    
#define TBStable_rowCnt 27

#define MAX_SUPPORTED_BW 16

#define CQI_VALUE_RANGE 16
    
    extern const int cqi_to_mcs[16]; 

    extern const unsigned int TBStable[TBStable_rowCnt][110];

    extern const uint8_t cqi2fmt0_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];

    extern const uint8_t cqi2fmt1x_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];

    extern const uint8_t cqi2fmt2x_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];
    
    frame_t get_frame(uint32_t sfn_sf);
    
    subframe_t get_subframe(uint32_t sfn_sf);
    
    uint16_t get_sfn_sf(frame_t frame, subframe_t subframe);
    
    std::pair<frame_t, subframe_t> get_frame_subframe(uint32_t sfn_sf);

  }
  
}
#endif
