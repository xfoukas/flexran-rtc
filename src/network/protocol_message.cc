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

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "protocol_message.h"

void flexran::network::protocol_message::body_length(std::size_t new_length) {
  body_length_ = new_length;
  if (body_length_ > max_body_length)
    body_length_ = max_body_length;
}

void flexran::network::protocol_message::set_message(const char * buf, std::size_t size) {
  body_length(size);
  encode_header();
  std::memcpy(data_ + header_length, buf, body_length_);
}

bool flexran::network::protocol_message::decode_header() {
  body_length_ = (data_[0] << 24) |
    (data_[1] << 16) |
    (data_[2] << 8)  |
    data_[3];
  
  if (body_length_ > max_body_length) {
    body_length_ = 0;
    return false;
  }
  return true;
}

void flexran::network::protocol_message::encode_header() {
  data_[0] = (body_length_ >> 24) & 255;
  data_[1] = (body_length_ >> 16) & 255;
  data_[2] = (body_length_ >> 8) & 255;
  data_[3] = body_length_ & 255;
}
