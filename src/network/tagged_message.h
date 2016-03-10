#ifndef TAGGED_MESSAGE_H_
#define TAGGED_MESSAGE_H_

#include <cstring>
#include <cstdlib>

class tagged_message {

  enum { max_normal_msg_size = 2048 };
  
 public:

  tagged_message(char *msg, std::size_t size, int tag);

  tagged_message(std::size_t size, int tag);

  tagged_message(const tagged_message& m);

  tagged_message(tagged_message&& other);

  tagged_message& operator=(tagged_message&& other);
  
  int getTag() const { return tag_; }

  int getSize() const { return size_; }

  char * getMessageArray() {return msg_contents_;}
  
  const char* getMessageContents() const { return msg_contents_; }

  ~tagged_message();
  
private:
  
  int tag_;
  std::size_t size_;
  char p_msg_[max_normal_msg_size];
  char *msg_contents_;
  bool dynamic_alloc_;
};

#endif
