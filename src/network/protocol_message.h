#ifndef PROTOCOL_MESSAGE_H_
#define PROTOCOL_MESSAGE_H_

#include "progran.pb.h"

class protocol_message {

 public:
  enum { header_length = 4 };
  enum { max_body_length = 2048 };

 protocol_message()
   : body_length_(0) {}

  const char* data() const {
    return data_;
  }

  char* data() {
    return data_;
  }

  std::size_t length() const {
    return header_length + body_length_;
  }

  const char* body() const {
    return data_ + header_length;
  }

  char* body() {
    return data_ + header_length;
  }
  
  std::size_t body_length() const {
    return body_length_;
  }

  void body_length(std::size_t new_length);

  void set_message(const char * buf, std::size_t size);

  bool decode_header();

  void encode_header();

 private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
  
};

#endif
