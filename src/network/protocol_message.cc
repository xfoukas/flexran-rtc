#include <cstring>
#include <cstdlib>
#include <iostream>

#include "protocol_message.h"

void protocol_message::body_length(std::size_t new_length) {
  body_length_ = new_length;
  if (body_length_ > max_body_length)
    body_length_ = max_body_length;
}

void protocol_message::set_message(const char * buf, std::size_t size) {
  body_length(size);
  encode_header();
  std::memcpy(data_ + header_length, buf, body_length_);
}

bool protocol_message::decode_header() {
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

void protocol_message::encode_header() {
  data_[0] = (body_length_ >> 24) & 255;
  data_[1] = (body_length_ >> 16) & 255;
  data_[2] = (body_length_ >> 8) & 255;
  data_[3] = body_length_ & 255;
}
