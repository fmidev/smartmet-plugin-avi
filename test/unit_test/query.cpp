#define BOOST_TEST_MODULE "QueryClassModule"

#include "Query.h"

#include <boost/test/included/unit_test.hpp>
#include <smartmet/engines/avi/Engine.h>
#include <spine/HTTP.h>
#include <spine/Reactor.h>
#include <typeinfo>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
Engine::Authentication::Engine* authEngine;

BOOST_AUTO_TEST_CASE(query_config_constructor)
{
  const std::string filename = "cnf/aviplugin.conf";
  Config config(filename);
  BOOST_CHECK_EQUAL(config.get_file_name(), filename);
}

BOOST_AUTO_TEST_CASE(query_authengine_singleton,
                     *boost::unit_test::depends_on("query_config_constructor"))
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

BOOST_AUTO_TEST_CASE(query_constructor, *boost::unit_test::depends_on("query_authengine_singleton"))
{
  BOOST_CHECK(authEngine != nullptr);

  const unsigned int unsignedIntVariable = 0;
  const std::string stringVariable;
  const std::string filename = "cnf/aviplugin.conf";
  const Engine::Avi::QueryOptions queryOptionsVariable;

  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");
  Query query(request, authEngine, config);

  BOOST_CHECK(typeid(query.itsPrecision) == typeid(unsignedIntVariable));
  BOOST_CHECK(typeid(query.itsQueryOptions) == typeid(queryOptionsVariable));
  BOOST_CHECK(typeid(query.itsFormat) == typeid(stringVariable));
  BOOST_CHECK_EQUAL(query.itsPrecision, 6);
  BOOST_CHECK_EQUAL(query.itsFormat, "ascii");

  request.removeParameter("param");
  BOOST_CHECK_THROW({ Query query2(request, authEngine, config); }, Spine::Exception);
}

BOOST_AUTO_TEST_CASE(query_constructor_allowMultipleLocationOptions_enabled,
                     *boost::unit_test::depends_on("query_constructor"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");
  request.addParameter("place", "Göteborg");
  Query query1(request, authEngine, config);

  request.addParameter("icao", "EFHK");
  Query query2(request, authEngine, config);
}

BOOST_AUTO_TEST_CASE(query_constructor_allowMultipleLocationOptions_disabled,
                     *boost::unit_test::depends_on("query_constructor"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin-with-authentication.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");
  request.addParameter("place", "Göteborg");
  Query query1(request, authEngine, config);

  // The query does not work with multiple location options.
  request.addParameter("icao", "EFHK");
  BOOST_CHECK_THROW({ Query query2(request, authEngine, config); }, Spine::Exception);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_places,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");
  request.addParameter("place", " Göteborg");
  request.addParameter("place", "Helsinki ");
  request.addParameter("place", ",Turku");
  request.addParameter("places", " Tampere , Oulu ");

  Query query1(request, authEngine, config);
  request.removeParameter("place");
  request.removeParameter("places");

  // Exception: Empty value for option 'places' at position 1; ',Kuopio,Rovaniemi'
  request.addParameter("places", ",Kuopio,Rovaniemi");
  BOOST_CHECK_THROW({ Query query2(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("places");

  // Exception: Empty value for option 'places' at position 3; 'Jyväskylä,Joensuu,'
  request.addParameter("places", "Jyväskylä,Joensuu,");
  BOOST_CHECK_THROW({ Query query3(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("places");
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_bbox,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("bbox", "25,60,26,61");
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  // One bbox by using integers
  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsBBoxes.size(), 1);
  request.removeParameter("bbox");

  // One bbox by using floating point mumbers
  request.addParameter("bbox", "25.11 ,60.1, 26.0,61.1");
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsBBoxes.size(), 1);
  request.removeParameter("bbox");

  // Exception: 4 values required for option 'bbox'; '25,60,26'
  request.addParameter("bbox", "25,60,26");
  BOOST_CHECK_THROW({ Query query4(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("bbox");

  // Exception: 4 values required for option 'bbox'; '25,60,26,61,27'
  request.addParameter("bbox", "25,60,26,61,27");
  BOOST_CHECK_THROW({ Query query5(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("bbox");

  // Two bboxes
  request.addParameter("bbox", "25,60,26,61");
  request.addParameter("bbox", "19,60,20,61");
  Query query6(request, authEngine, config);
  BOOST_CHECK_EQUAL(query6.itsQueryOptions.itsLocationOptions.itsBBoxes.size(), 2);
  request.removeParameter("bbox");
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_lonlat,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("lonlat", "25,60");
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  // One lonlat by using integers
  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  request.removeParameter("lonlat");

  // One lonlat by using floating point numbers
  request.addParameter("lonlat", "25.12,60.1");
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25.12);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60.1);
  request.removeParameter("lonlat");

  // Exception: 2 values required for option 'lonlat'; ',25,60'
  request.addParameter("lonlat", ",25,60");
  BOOST_CHECK_THROW({ Query query4(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("lonlat");

  // Exception: 2 values required for option 'lonlat'; '25,60,'
  request.addParameter("lonlat", "25,60,");
  BOOST_CHECK_THROW({ Query query5(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("lonlat");

  // Exception: 2 values required for option 'lonlat'; '25,60,'
  request.addParameter("lonlat", "25,60,");
  BOOST_CHECK_THROW({ Query query6(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("lonlat");

  // Two separate lonlat query parameters
  request.addParameter("lonlat", "25,60");
  request.addParameter("lonlat", "20.12,60.1");
  Query query7(request, authEngine, config);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 20.12);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 60.1);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_latlon,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("latlon", "60,25");
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  // One latlon by using integers
  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  request.removeParameter("latlon");

  // One latlon by using floating point numbers
  request.addParameter("latlon", "60.1,25.12345");
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25.12345);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60.1);
  request.removeParameter("latlon");

  // Exception: Value in range [-90,90] expected for option 'latlon' at position 1; '91.0,25'
  request.addParameter("latlon", "91.0,25");
  BOOST_CHECK_THROW({ Query query4(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("latlon");

  // Exception: Value in range [-90,90] expected for option 'latlon' at position 1; '-91.0,25'
  request.addParameter("latlon", "-91.0,25");
  BOOST_CHECK_THROW({ Query query5(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("latlon");

  // Exception: Value in range [-180,180] expected for option 'latlon' at position 2; '60,181'
  request.addParameter("latlon", "60,181");
  BOOST_CHECK_THROW({ Query query6(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("latlon");

  // Exception: Value in range [-180,180] expected for option 'latlon' at position 2; '60,-181'
  request.addParameter("latlon", "60,-181");
  BOOST_CHECK_THROW({ Query query7(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("latlon");

  request.addParameter("latlon", "60,25");
  request.addParameter("latlon", "60.1,20.12");
  Query query8(request, authEngine, config);
  request.removeParameter("latlon");
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 20.12);
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 60.1);
}

BOOST_AUTO_TEST_CASE(query_constructor_parseLocationOptions_lonlats,
                     *boost::unit_test::depends_on("query_constructor_parseLocationOptions_lonlat"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("lonlats", "25,60");
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  // One lonlats by using integers
  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  request.removeParameter("lonlats");

  // Exception: Even number of values required for option 'lonlats'; '25,60,21'
  request.addParameter("lonlats", "25,60,21");
  BOOST_CHECK_THROW({ Query query3(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("lonlats");

  // Two LonLat pairs in a lonlats query paramer.
  request.addParameter("lonlats", "25,60,21,61");
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 21);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 61);
  request.removeParameter("lonlats");

  // Two LonLat pairs in separate lonlats query paramers.
  request.addParameter("lonlats", "25,60");
  request.addParameter("lonlats", "21,61");
  Query query5(request, authEngine, config);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 21);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 61);
  request.removeParameter("lonlats");
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
