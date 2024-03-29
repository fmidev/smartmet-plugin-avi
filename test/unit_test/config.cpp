#define BOOST_TEST_MODULE "ConfigClassModule"

#include "Config.h"

#include <boost/test/included/unit_test.hpp>
#include <spine/Reactor.h>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
Engine::Authentication::Engine* authEngine;

BOOST_AUTO_TEST_CASE(config_constructor_with_file_not_exist)
{
  const std::string filename = "cnf/notexist.conf";
  BOOST_CHECK_THROW({ Config config(filename); }, Spine::Exception);
}
BOOST_AUTO_TEST_CASE(config_constructor_with_valid_file_exist)
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  BOOST_CHECK_EQUAL(config.get_file_name(), filename);
}
BOOST_AUTO_TEST_CASE(config_accessor_useAuthentication,
                     *boost::unit_test::depends_on("config_constructor_with_valid_file_exist"))
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  const bool boolVariable = true;
  BOOST_CHECK_EQUAL(config.useAuthentication(), not boolVariable);

  const std::string filename2 = "cnf/aviplugin-with-authentication.conf";
  Config config2(filename2);
  BOOST_CHECK_EQUAL(config2.useAuthentication(), boolVariable);
}
BOOST_AUTO_TEST_CASE(config_accessor_tableFormatterOptions,
                     *boost::unit_test::depends_on("config_constructor_with_valid_file_exist"))
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  const TableFormatterOptions& tfo = config.tableFormatterOptions();
  BOOST_CHECK_EQUAL(tfo.formatType(), "");
}
BOOST_AUTO_TEST_CASE(config_accessor_getQueryLimits,
                     *boost::unit_test::depends_on("config_constructor_with_valid_file_exist"))
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  const QueryLimits& gql = config.getQueryLimits(nullptr, "");
  BOOST_CHECK_EQUAL(gql.getMaxMessageStations(), 0);
  BOOST_CHECK_EQUAL(gql.getMaxMessageRows(), 0);
  BOOST_CHECK_EQUAL(gql.getMaxMessageTimeRangeDays(), 31);
  BOOST_CHECK_EQUAL(gql.getAllowMultipleLocationOptions(), true);
}

BOOST_AUTO_TEST_CASE(config_authengine_singleton,
                     *boost::unit_test::depends_on("config_accessor_useAuthentication"))
{
  Spine::Options opts;
  opts.defaultlogging = false;
  opts.configfile = "cnf/reactor-with-authentication.conf";
  opts.parseConfig();
  SmartMet::Spine::Reactor reactor(opts);
  authEngine = nullptr;
  authEngine = reinterpret_cast<Engine::Authentication::Engine*>(
      reactor.getSingleton("Authentication", nullptr));
  BOOST_CHECK(authEngine != nullptr);
}

BOOST_AUTO_TEST_CASE(config_authengine_querylimits,
                     *boost::unit_test::depends_on("config_authengine_singleton"))
{
  BOOST_CHECK(authEngine != nullptr);
  const std::string filename2 = "cnf/aviplugin-with-authentication.conf";
  Config config2(filename2);
  const QueryLimits& gql = config2.getQueryLimits(authEngine, "testkey");
  BOOST_CHECK_EQUAL(gql.getMaxMessageStations(), 0);
  BOOST_CHECK_EQUAL(gql.getMaxMessageRows(), 1);
  BOOST_CHECK_EQUAL(gql.getMaxMessageTimeRangeDays(), 1);
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
