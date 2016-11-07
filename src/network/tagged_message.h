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

#ifndef TAGGED_MESSAGE_H_
#define TAGGED_MESSAGE_H_

#include <cstring>
#include <cstdlib>

namespace flexran {

  namespace network {

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

  }
  
}

#endif
