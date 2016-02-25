#ifndef TAGGED_MESSAGE_H_
#define TAGGED_MESSAGE_H_

#include <cstring>
#include <cstdlib>

class tagged_message {

  enum { max_msg_size = 2048 };
  
 public:
 tagged_message(char * msg, std::size_t size, int tag):
  size_(size), tag_(tag) {
    std::memcpy(p_msg_, msg, size);
  }

 tagged_message(std::size_t size, int tag):
  size_(size), tag_(tag) {}
  
  tagged_message(const tagged_message& m) {
    tag_ = m.getTag();
    size_ = m.getSize();
    std::memcpy(p_msg_, m.getMessageContents(), size_);
  }

  tagged_message(tagged_message&& other) {
    tag_ = other.getTag();
    size_ = other.getSize();
    std::memcpy(p_msg_, other.getMessageContents(), size_);
  }
  
  int getTag() const { return tag_; }

  int getSize() const { return size_; }

  char * getMessageArray() {return p_msg_;}
  
  const char* getMessageContents() const { return p_msg_; }

  tagged_message& operator=(tagged_message&& other) {
    tag_ = other.getTag();
    size_ = other.getSize();
    std::memcpy(p_msg_, other.getMessageContents(), size_);
  }
  
private:
  
  int tag_;
  std::size_t size_;
  char p_msg_[max_msg_size];
};

#endif
