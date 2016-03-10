#include "tagged_message.h"

tagged_message::tagged_message(char * msg, std::size_t size, int tag):
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

tagged_message::tagged_message(std::size_t size, int tag):
  size_(size), tag_(tag) {
  if (size > max_normal_msg_size) {
    msg_contents_ = new char[size];
    dynamic_alloc_ = true;
  } else {
    msg_contents_ = p_msg_;
    dynamic_alloc_ = false;
  }
}
  
tagged_message::tagged_message(const tagged_message& m) {
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

tagged_message::tagged_message(tagged_message&& other) {
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
  
tagged_message& tagged_message::operator=(tagged_message&& other) {
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

tagged_message::~tagged_message() {
  if (dynamic_alloc_) {
    delete [] msg_contents_;
  }
}
