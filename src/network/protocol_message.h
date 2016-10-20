#ifndef PROTOCOL_MESSAGE_H_
#define PROTOCOL_MESSAGE_H_

#include "flexran.pb.h"

namespace flexran {

  namespace network {
    
    class protocol_message {
      
    public:
      enum { header_length = 4 };
      enum { max_body_length = 240000 };
      
    protocol_message()
      : body_length_(0) {}
      
      const char* data() const {
	return (char *) data_;
      }
      
      char* data() {
	return (char *) data_;
      }
      
      uint32_t length() const {
	return header_length + body_length_;
      }
      
      const char* body() const {
	return (char *) data_ + header_length;
      }
      
      char* body() {
	return (char *) data_ + header_length;
      }
      
      std::size_t body_length() const {
	return body_length_;
      }
      
      void body_length(std::size_t new_length);
      
      void set_message(const char * buf, std::size_t size);
      
      bool decode_header();
      
      void encode_header();
      
    private:
      unsigned char data_[header_length + max_body_length];
      uint32_t body_length_;
      
    };
    
  }
}

#endif
