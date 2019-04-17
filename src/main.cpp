#include <chrono>
#include <iostream>
#include <thread>

#include <ioBenchmark.hpp>

int
main() {
   IOBenchmark iob;
   iob.start("");
   while (iob.isRunning())
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   std::cout << iob.getTempDirWriteSpeed() << "MB/s" << std::endl;

   return 0;
}