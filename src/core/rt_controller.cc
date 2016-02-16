#include <thread>
#include <iostream>

#include "async_xface.h"
#include "progran.pb.h"
#include "rib_updater.h"
#include "rib.h"

int main(int argc, char *argv[]) {

  async_xface net_xface(2210);

  // Create the rib
  Rib rib;

  // Create the rib update manager
  rib_updater r_updater(net_xface, rib);

  std::thread networkThread(&async_xface::establish_xface, &net_xface);

  while(true) {

    std::shared_ptr<std::thread> rib_updater_thread(new std::thread(&rib_updater::update_rib, &r_updater));
    if (rib_updater_thread->joinable()) {
      rib_updater_thread->join();
    }
  }
   
  if (networkThread.joinable())
    networkThread.join();
  
  return 0;
}
