#include <thread>

#include "async_xface.h"

int main(int argc, char *argv[]) {

  async_xface net_xface(2210);

  std::thread networkThread(&async_xface::establish_xface, &net_xface);
  if (networkThread.joinable())
    networkThread.join();
  
  return 0;
  
}
