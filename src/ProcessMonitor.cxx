///
/// \file ProcessMonitor.cxx
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "Monitoring/ProcessMonitor.h"
#include "Exceptions/MonitoringInternalException.h"
#include "MonLogger.h"
#include <boost/algorithm/string/classification.hpp> 
#include <chrono>
#include <sstream>
#include <cmath>
#include <fstream>
#include <utility>

namespace o2
{
/// ALICE O2 Monitoring system
namespace monitoring
{

ProcessMonitor::ProcessMonitor()
{
  mPid = static_cast<unsigned int>(::getpid());
  getrusage(RUSAGE_SELF, &mPreviousGetrUsage);
  mTimeLastRun = std::chrono::high_resolution_clock::now();
#ifdef _OS_LINUX
  setTotalMemory();
#endif
}

void ProcessMonitor::setTotalMemory()
{
  std::ifstream memInfo("/proc/meminfo");
  std::string totalString;
  std::getline(memInfo, totalString);
  std::istringstream iss(totalString);
  std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>{}};
  mTotalMemory = std::stoi(tokens[1]);
}

Metric ProcessMonitor::getMemoryUsage()
{
  std::ifstream statusStream("/proc/self/status");
  std::string rssString;
  rssString.reserve(50);

  // Scan for VmRSS
  for(int i = 0; i < 18; i++) {
       std::getline(statusStream, rssString);
  }
  std::istringstream iss(rssString);
  std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>{}};
  return Metric{(std::stod(tokens[1])*100)/mTotalMemory, "memoryUsagePercentage"};
}

std::vector<Metric> ProcessMonitor::getCpuAndContexts() {
  std::vector<Metric> metrics;
  struct rusage currentUsage;
  getrusage(RUSAGE_SELF, &currentUsage);
  auto timeNow = std::chrono::high_resolution_clock::now();
  double timePassed = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - mTimeLastRun).count();
  if (timePassed < 950) { // do not run too often
    throw MonitoringInternalException("Process Monitor getrusage", "Do not invoke more often then 1ms");
  }
  double fractionCpuUsed = (
      currentUsage.ru_utime.tv_sec*1000000.0 + currentUsage.ru_utime.tv_usec - (mPreviousGetrUsage.ru_utime.tv_sec*1000000.0 + mPreviousGetrUsage.ru_utime.tv_usec)
    + currentUsage.ru_stime.tv_sec*1000000.0 + currentUsage.ru_stime.tv_usec - (mPreviousGetrUsage.ru_stime.tv_sec*1000000.0 + mPreviousGetrUsage.ru_stime.tv_usec)
  ) / timePassed;

  metrics.emplace_back(Metric{
    static_cast<double>(std::round(fractionCpuUsed * 100.0 * 100.0 ) / 100.0), "cpuUsedPercentage"
  });
  metrics.emplace_back(Metric{
    static_cast<uint64_t>(currentUsage.ru_nivcsw - mPreviousGetrUsage.ru_nivcsw), "involuntaryContextSwitches"
  });

  mTimeLastRun = timeNow;
  mPreviousGetrUsage = currentUsage;
  return metrics;
}

std::vector<Metric> ProcessMonitor::getNetworkUsage()
{
  std::vector<Metric> metrics;
  std::ifstream infile("/proc/" + std::to_string(mPid) + "/net/dev");
  std::string line;
  std::getline(infile, line);
  std::getline(infile, line);
  while (std::getline(infile, line)) {
    if (line.find("virbr") != std::string::npos) continue;
    if (line.find("lo") != std::string::npos) continue;
    std::istringstream iss(line);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>{}};
    metrics.emplace_back(Metric{
      static_cast<uint64_t>(std::stoull(tokens[1])),
      "bytesReceived"}.addTags({{"if", tokens[0]}})
    );
    metrics.emplace_back(Metric{
      static_cast<uint64_t>(std::stoull(tokens[9])),
      "bytesTransmitted"}.addTags({{"if", tokens[0]}})
    );
  }
return metrics;
}

} // namespace monitoring
} // namespace o2
