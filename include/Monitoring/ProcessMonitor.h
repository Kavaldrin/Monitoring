// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file ProcessMonitor.h
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#ifndef ALICEO2_MONITORING_CORE_PROCESSMONITOR_H
#define ALICEO2_MONITORING_CORE_PROCESSMONITOR_H

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sys/resource.h>

#include "Monitoring/Metric.h"

namespace o2
{
/// ALICE O2 Monitoring system
namespace monitoring
{

/// Monitors current process and/or other processes running at the same machien
class ProcessMonitor
{
  friend class Monitoring;

 public:
  /// Prepares externam software commands (ps)
  ProcessMonitor();

  /// Default destructor
  ~ProcessMonitor() = default;

  /// Return performance metrics
  Metric getPerformanceMetrics();

 private:
  /// PIDs that are monitored
  unsigned int mPid;

  /// Total memory size
  unsigned int mTotalMemory;

  /// Retrievs total memory size from /proc/meminfo
  void setTotalMemory();

  /// 'getrusage' values from last execution
  struct rusage mPreviousGetrUsage;

  /// Timestamp when process monitoring was executed last time
  std::chrono::high_resolution_clock::time_point mTimeLastRun;

  /// Retrieves memory usage (%)
  double getMemoryUsage();

  /// Retrieves CPU usage (%) and number of context switches during the interval
  Metric getCpuAndContexts();
};

} // namespace monitoring
} // namespace o2

#endif // ALICEO2_MONITORING_CORE_PROCESSMONITOR_H
