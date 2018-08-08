#define BOOST_TEST_MODULE "QueryLimitsClassModule"

#include "QueryLimits.h"

#include <boost/test/included/unit_test.hpp>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
BOOST_AUTO_TEST_CASE(querylimits_constructor_with_default_inputs)
{
  const int intVariable = -1;
  const bool boolVariable = false;
  QueryLimits ql;
  BOOST_CHECK(typeid(ql.getMaxMessageStations()) == typeid(intVariable));
  BOOST_CHECK(typeid(ql.getMaxMessageRows()) == typeid(intVariable));
  BOOST_CHECK(typeid(ql.getMaxMessageTimeRangeDays()) == typeid(intVariable));
  BOOST_CHECK(typeid(ql.getAllowMultipleLocationOptions()) == typeid(boolVariable));

  BOOST_CHECK_EQUAL(ql.getMaxMessageStations(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageRows(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageTimeRangeDays(), intVariable);
  BOOST_CHECK_EQUAL(ql.getAllowMultipleLocationOptions(), boolVariable);
}
BOOST_AUTO_TEST_CASE(querylimits_constructor_with_inputs)
{
  const int intVariable = 1;
  const bool boolVariable = true;
  QueryLimits ql(intVariable, intVariable, intVariable, boolVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageStations(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageRows(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageTimeRangeDays(), intVariable);
  BOOST_CHECK_EQUAL(ql.getAllowMultipleLocationOptions(), boolVariable);
}
BOOST_AUTO_TEST_CASE(querylimits_change_default_values,
                     *boost::unit_test::depends_on("querylimits_constructor_with_default_inputs"))
{
  const int intVariable = 1;
  const bool boolVariable = true;
  QueryLimits ql;
  ql.setMaxMessageStations(intVariable);
  ql.setMaxMessageRows(intVariable);
  ql.setMaxMessageTimeRangeDays(intVariable);
  ql.setAllowMultipleLocationOptions(boolVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageStations(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageRows(), intVariable);
  BOOST_CHECK_EQUAL(ql.getMaxMessageTimeRangeDays(), intVariable);
  BOOST_CHECK_EQUAL(ql.getAllowMultipleLocationOptions(), boolVariable);
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
