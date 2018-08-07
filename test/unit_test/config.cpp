#define BOOST_TEST_MODULE "ConfigClassModule"

#include "Config.h"

#include <boost/test/included/unit_test.hpp>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
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
}
BOOST_AUTO_TEST_CASE(config_accessor_tableFormatterOptions,
                     *boost::unit_test::depends_on("config_constructor_with_valid_file_exist"))
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  const TableFormatterOptions & tfo = config.tableFormatterOptions();
  BOOST_CHECK_EQUAL(tfo.formatType(), "");
}
BOOST_AUTO_TEST_CASE(config_accessor_getQueryLimits,
                     *boost::unit_test::depends_on("config_constructor_with_valid_file_exist"))
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  const QueryLimits & gql = config.getQueryLimits(nullptr, "");
  BOOST_CHECK_EQUAL(gql.getMaxMessageStations(), 0);
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
