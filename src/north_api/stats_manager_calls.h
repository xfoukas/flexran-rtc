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

#ifndef STATS_MANAGER_CALLS_H_
#define STATS_MANAGER_CALLS_H_

#include <pistache/http.h>

#include "app_calls.h"
#include "stats_manager.h"

namespace flexran {

  namespace north_api {

    class stats_manager_calls : public app_calls {

    public:

      stats_manager_calls(std::shared_ptr<flexran::app::stats::stats_manager> stats)
	: stats_app(stats)
      { }
      
      void register_calls(Pistache::Rest::Router& router);

      void obtain_stats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    private:

      std::shared_ptr<flexran::app::stats::stats_manager> stats_app;

    };
  }
}


#endif /* FLEXIBLE_SCHED_CALLS_H_ */
