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

#ifndef DELEGATION_MANAGER_H_
#define DELEGATION_MANAGER_H_

#include "periodic_component.h"
#include "rib_common.h"

namespace flexran {
  
  namespace app {

    namespace management {

      class delegation_manager : public periodic_component {
	
      public:

	delegation_manager(rib::Rib& rib, const core::requests_manager& rm)
	  : periodic_component(rib, rm), delegation_steps_{false} {}

	void run_periodic_task();

	void push_code(int agent_id, std::string function_name, std::string lib_name);

	void reconfigure_agent(int agent_id, std::string policy_name);
	
      private:
	bool delegation_steps_[6];
      };

    }
  }
}

#endif
