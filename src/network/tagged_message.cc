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

#include "tagged_message.h"

flexran::network::tagged_message::tagged_message(char * msg, std::size_t size, int tag):
  size_(size), tag_(tag) {
  if (size <= max_normal_msg_size) {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  } else {
    msg_contents_ = new char[size];
    dynamic_alloc_ = true;
  }
  std::memcpy(msg_contents_, msg, size);
}

flexran::network::tagged_message::tagged_message(std::size_t size, int tag):
  size_(size), tag_(tag) {
  if (size > max_normal_msg_size) {
    msg_contents_ = new char[size];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
}
  
flexran::network::tagged_message::tagged_message(const tagged_message& m) {
  tag_ = m.getTag();
  size_ = m.getSize();
  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, m.getMessageContents(), size_);
}

flexran::network::tagged_message::tagged_message(tagged_message&& other) {
  tag_ = other.getTag();
  size_ = other.getSize();

  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, other.getMessageContents(), size_);
}
  
flexran::network::tagged_message& flexran::network::tagged_message::operator=(flexran::network::tagged_message&& other) {
  if (dynamic_alloc_) {
    delete [] msg_contents_;
  }

  tag_ = other.getTag();
  size_ = other.getSize();
  if (size_ > max_normal_msg_size) {
    msg_contents_ = new char[size_];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
  std::memcpy(msg_contents_, other.getMessageContents(), size_);
  return *this;
}

flexran::network::tagged_message::~tagged_message() {
  if (dynamic_alloc_) {
    delete [] msg_contents_;
  }
}
