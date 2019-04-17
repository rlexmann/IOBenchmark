#pragma once

// STL includes
#include <map>
#include <mutex>
#include <string>
#include <thread>

class IOBenchmark {
 public: //---------------------------------------------------------------------
   // constructor
   IOBenchmark()
     : m_abort{ false } {}
   // destructor
   ~IOBenchmark() { abort(); }

   // result storage
   struct Result {
      double writeSpeed{ 0.0 };
      double rating{ -1.0 };
   };
   std::map<std::string, Result> m_results; // <path: result> pairs

   // control methods
   void start(const std::string& pathStr);
   void abort();

   // query methods
   const double getTempDirRating() const {
      const auto it = m_results.find(m_currentPath);
      return (it != m_results.cend()) ? it->second.rating : -1.0;
   }
   const double getTempDirWriteSpeed() const {
      const auto it = m_results.find(m_currentPath);
      return (it != m_results.cend()) ? it->second.writeSpeed : 0.0;
   }
   const bool isRunning() const { return m_running; }

 private: //--------------------------------------------------------------------
   std::string m_currentPath; // last evaluated directory
   bool m_abort{ false };     // used to abort IOBenchmarkProc in worker thread
   bool m_running{ false };
   std::thread m_worker;
   std::mutex m_mutex;

   // actual benchmarking procedure
   friend void IOBenchmarkProc(IOBenchmark* const db, const std::string path);
   // result updater
   void setResult(const std::string& path, const Result& result);
   // rating computation
   static const double calcRating(const double writeSpeed);

   // disable copy constructor and copy assignement operator
   // (declared private without implementation)
   // this is just a precaution
   IOBenchmark(const IOBenchmark&);
   IOBenchmark& operator=(const IOBenchmark&);
};

void
IOBenchmarkProc(IOBenchmark* const iob, const std::string path);