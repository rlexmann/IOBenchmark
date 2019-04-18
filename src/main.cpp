#include <chrono>
#include <iostream>
#include <thread>

#include <ioBenchmark.hpp>

int
main() {
   IOBenchmark iob;
   iob.start("");
   while (iob.isRunning())
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
   iob.abort();

   std::cout << "Press any key..." << std::endl;
   std::string s;
   std::getline(std::cin, s);

   return 0;
}