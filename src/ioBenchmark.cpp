// STL includes
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

// non-lib includes
#include <ioBenchmark.hpp>

void
IOBenchmark::start(const std::string& pathStr) {
   abort(); // abort running benchmarking (if any)

   if (m_results.cend() != m_results.find(pathStr))
   { // directory was already tested, no need to repeat
      m_currentPath = pathStr;
      return;
   }

   m_abort = false;
   m_running = true;
   m_worker = std::thread(IOBenchmarkProc, this, pathStr);
}

void
IOBenchmark::abort() {
   m_abort = true; // IOBenchmarkProc should always terminate after this
   if (m_worker.joinable())
      m_worker.join();
}

void
IOBenchmarkProc(IOBenchmark* const iob, const std::string path) {
   using namespace std::chrono;

   auto result = IOBenchmark::Result();
   iob->setResult(path, result);

   const size_t bytes{ 1024 * 1024 * 256 }; // 256MB
   const size_t chunkBytes{ 1024 * 1024 * 64 }; // 64MB
   assert(0 == bytes % chunkBytes);
   const size_t chunks{ bytes / chunkBytes };
   const size_t iterations{ 25 };
   const auto maxDuration =
     milliseconds(10000); // 5 seconds - maximal duration of benchmarking

   // generate data for writing
   std::vector<char> data(bytes / sizeof(char));
   std::iota(data.begin(), data.end(), 0);
   const size_t chunkSize = data.size() / chunks;

   high_resolution_clock::time_point start, end;
   auto writeTime = end - start;
   size_t megabytesWritten{ 0 };
   std::string filename{ path + std::string("benchmark.tmp") };
   std::fstream fs;

   for (size_t i = 0; i < iterations; ++i)
   {

      start = high_resolution_clock::now();
      //fs.open(filename, std::ios::out | std::ios::binary);
      FILE* file = fopen(filename.c_str(), "wb");
      setvbuf(file, nullptr, _IONBF, 0);
      if (nullptr == file) // directory is not writeable
      {
         result.rating = 0.0;
         iob->setResult(path, result);
         break;
      }

      // write test file
      for (size_t c = 0; c < chunks; ++c)
      {
         //fs.write(data.data() + c * chunkSize, chunkSize);
         fwrite(data.data() + c * chunkSize, sizeof(char), chunkSize, file);
         megabytesWritten += chunkBytes / (1024 * 1024);
         if (iob->m_abort)
            break;
      }

      //fs.close()
      fclose(file);
      end = high_resolution_clock::now();

      if (iob->m_abort)
         break;

      // update result
      writeTime += (end - start);
      result.writeSpeed = (static_cast<double>(megabytesWritten) /
                           duration_cast<milliseconds>(writeTime).count()) *
                          1e3;
      result.rating = IOBenchmark::calcRating(result.writeSpeed);
      std::cout << result.writeSpeed << "MB/s" << std::endl;
      iob->setResult(path, result);

      if (iob->m_abort || writeTime > maxDuration)
         break;
   }
   remove(filename.c_str());

   iob->m_running = false;
}

void
IOBenchmark::setResult(const std::string& path, const Result& result) {
   m_mutex.lock();
   m_currentPath = path;
   m_results[path] = result;
   m_mutex.unlock();
}

bool IOBenchmark::getResult(const std::string& path, Result& result) const
{
   bool success{ false };
   m_mutex.lock();
   const auto it = m_results.find(m_currentPath);
   if (it != m_results.cend())
   {
      result = it->second;
      success = true;
   }
   else
   {
      success = false;
   }
   m_mutex.unlock();
   return success;
}

const double
IOBenchmark::calcRating(const double writeSpeed) {
   /*
   Input is expected to be write speed in MB/s.
   Typically we get:
      ~100MB/s with an HDD,
      ~500MB/s with an SSD,
      ~1000MB/s with an SSD using some software tricks.
   We establish a linear rating scale as follows:
      0.0 - non-writable drive (writeSpeed <= 0.0),
      0.5 - OUR defined minimum recommended write speed: 450MB/s
      1.0 - hard limit of the scale
   */
   return (writeSpeed > 0.0) ? std::min(1.0, writeSpeed / 900) : 0.0;
}
