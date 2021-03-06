// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <chrono>
#include <vector>

#define BOOST_TEST_MODULE Test Monitoring Colletor
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include "../include/Monitoring/MonitoringFactory.h"

namespace o2
{
namespace monitoring
{
namespace Test
{

using Monitoring = o2::monitoring::MonitoringFactory;

BOOST_AUTO_TEST_CASE(createMonitoring)
{
  auto monitoring = Monitoring::Get("stdout://");

  int intMetric = 10;
  std::string stringMetric("monitoringString");
  double doubleMetric = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

  monitoring->addGlobalTag("name", "Readout");
  monitoring->addGlobalTag(tags::Key::Name, tags::Value::Readout);

  monitoring->send({intMetric, "myCrazyMetricI"});
  monitoring->send({stringMetric, "myCrazyMetricS"});
  monitoring->send({doubleMetric, "myCrazyMetricD"});
}

BOOST_AUTO_TEST_CASE(buffering)
{
  auto monitoring = Monitoring::Get("stdout://,influxdb-udp://localhost:1234");
  monitoring->enableBuffering(10);
  for (int i = 0; i < 25; i++) {
    monitoring->send({10, "myMetricInt"});
  }
  monitoring->flushBuffer();
}

BOOST_AUTO_TEST_CASE(testSymbols)
{
  BOOST_WARN_MESSAGE(!BOOST_IS_DEFINED(O2_MONITORING_WITH_APPMON), "ApMon Backend disabled");
  BOOST_WARN_MESSAGE(BOOST_IS_DEFINED(O2_MONITORING_OS_LINUX), "Linux OS detected");
  BOOST_WARN_MESSAGE(BOOST_IS_DEFINED(O2_MONITORING_OS_MAC), "Mac OS detected");
}

} // namespace Test
} // namespace monitoring
} // namespace o2
