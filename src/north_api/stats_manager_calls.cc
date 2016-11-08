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

#include <pistache/http.h>
#include <string>

#include "stats_manager_calls.h"

void flexran::north_api::stats_manager_calls::register_calls(Net::Rest::Router& router) {

  Net::Rest::Routes::Get(router, "/stats_manager/:stats_type", Net::Rest::Routes::bind(&flexran::north_api::stats_manager_calls::obtain_stats, this));
  
}

void flexran::north_api::stats_manager_calls::obtain_stats(const Net::Rest::Request& request, Net::Http::ResponseWriter response) {

  auto stats_type = request.param(":stats_type").as<std::string>();
  
  std::string resp;
  
  if (stats_type.compare("all") == 0) {
    resp = stats_app->all_stats_to_string();
    response.send(Net::Http::Code::Ok, resp);
  } else if (stats_type.compare("enb_config") == 0) {
    resp = stats_app->enb_config_to_string();
    response.send(Net::Http::Code::Ok, resp);
  } else if (stats_type.compare("mac_stats") == 0) {
    resp = stats_app->mac_config_to_string();
    response.send(Net::Http::Code::Ok, resp);
  } else {
    response.send(Net::Http::Code::Not_Found, "Statistics type does not exist");
  }
}
