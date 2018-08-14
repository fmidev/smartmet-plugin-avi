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

BOOST_AUTO_TEST_CASE(query_constructor_parseLocationOptions_latlons,
                     *boost::unit_test::depends_on("query_constructor_parseLocationOptions_latlon"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("latlons", "60,25");
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  request.removeParameter("latlons");

  // Exception: Even number of values required for option 'latlons'; '60,25,61'
  request.addParameter("latlons", "60,25,61");
  BOOST_CHECK_THROW({ Query query3(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("latlons");

  // Two LatLon pairs in a latlons query parameter
  request.addParameter("latlons", "60,25,61,21");
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 21);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 61);
  request.removeParameter("latlons");

  // Two LatLon pairs in separate latlons query parameters
  request.addParameter("latlons", "60,25");
  request.addParameter("latlons", "21,61");
  Query query5(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLon, 25);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.front().itsLat, 60);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLon, 21);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsLonLats.back().itsLat, 61);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_wkt,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "12abcDE#)\{}+";
  const std::string stringVariable2 = "POINT(25 60)";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option maxdistance is required with latlon/lonlat, bbox and wkt options
  request.addParameter("wkt", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);

  // One wkt
  request.addParameter("maxdistance", "1000.0");
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.front(),
                    stringVariable1);
  request.removeParameter("wkt");

  request.addParameter("wkt", stringVariable1);
  request.addParameter("wkt", stringVariable2);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.front(),
                    stringVariable1);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.back(),
                    stringVariable2);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_icao,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "12abcDE#)\{}+";
  const std::string stringVariable2 = "EFRO";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // One icao code with invalid value
  request.addParameter("icao", stringVariable1);
  Query query1(request, authEngine, config);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsIcaos.size(), 1);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsIcaos.front(), stringVariable1);
  request.removeParameter("icao");

  // Two icao codes
  request.addParameter("icao", stringVariable1);
  request.addParameter("icao", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.size(), 2);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.front(), stringVariable1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.back(), stringVariable2);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_icaos,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "12abcDE#)\{}+";
  const std::string stringVariable2 = "EFRO";
  const std::string stringVariable3 = stringVariable1 + "," + stringVariable2;
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // One icao code with invalid value
  request.addParameter("icaos", stringVariable1);
  Query query1(request, authEngine, config);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsIcaos.size(), 1);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsIcaos.front(), stringVariable1);
  request.removeParameter("icaos");

  // Two comma separated icao codes in a string.
  request.addParameter("icaos", stringVariable3);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.size(), 2);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.front(), stringVariable1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsIcaos.back(), stringVariable2);
  request.removeParameter("icaos");

  // Two icao codes in separated strings.
  request.addParameter("icaos", stringVariable1);
  request.addParameter("icaos", stringVariable2);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsIcaos.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsIcaos.front(), stringVariable1);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsIcaos.back(), stringVariable2);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_country,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "12abcDE#)\{}+";
  const std::string stringVariable2 = "SE";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // One country with invalid value
  request.addParameter("country", stringVariable1);
  Query query1(request, authEngine, config);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsCountries.size(), 1);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsCountries.front(),
                    stringVariable1);
  request.removeParameter("country");

  // One country with invalid value
  request.addParameter("country", stringVariable1);
  request.addParameter("country", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.size(), 2);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.front(),
                    stringVariable1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.back(), stringVariable2);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_countries,
    *boost::unit_test::depends_on("query_constructor_parseLocationOptions_country"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "12abcDE#)\{}+";
  const std::string stringVariable2 = "SE";
  const std::string stringVariable3 = stringVariable1 + "," + stringVariable2;
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // One country with invalid value
  request.addParameter("countries", stringVariable1);
  Query query1(request, authEngine, config);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsCountries.size(), 1);
  BOOST_CHECK_EQUAL(query1.itsQueryOptions.itsLocationOptions.itsCountries.front(),
                    stringVariable1);
  request.removeParameter("countries");

  // Two comma separated country codes in a countries variable
  request.addParameter("countries", stringVariable3);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.size(), 2);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.front(),
                    stringVariable1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsCountries.back(), stringVariable2);
  request.removeParameter("countries");

  // Two country codes in separated countries variables
  request.addParameter("countries", stringVariable1);
  request.addParameter("countries", stringVariable2);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsCountries.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsCountries.front(),
                    stringVariable1);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsCountries.back(), stringVariable2);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_statioid,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "ab";
  const std::string stringVariable2 = "91";
  const std::string stringVariable3 = "92";
  const int intVariable2 = 91;
  const int intVariable3 = 92;
  const int intVariable4 = std::numeric_limits<int>::lowest();
  const int intVariable5 = std::numeric_limits<int>::max();
  const std::string stringVariable4 = std::to_string(intVariable4);
  const std::string stringVariable5 = std::to_string(intVariable5);
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option 'stationid' is empty
  request.addParameter("stationid", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("stationid");

  // One valid stationid
  request.addParameter("stationid", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable2);
  request.removeParameter("stationid");

  // Two valid stationid values
  request.addParameter("stationid", stringVariable2);
  request.addParameter("stationid", stringVariable3);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.back(), intVariable3);
  request.removeParameter("stationid");

  // Lowest and maximum value of integer.
  request.addParameter("stationid", stringVariable4);
  request.addParameter("stationid", stringVariable5);
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable4);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.back(), intVariable5);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_statioids,
    *boost::unit_test::depends_on("query_constructor_parseLocationOptions_statioid"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "ab";
  const std::string stringVariable2 = "91";
  const std::string stringVariable3 = "92";
  const std::string stringVariable4 = stringVariable2 + "," + stringVariable3;
  const int intVariable2 = 91;
  const int intVariable3 = 92;
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option 'stationids' is empty
  request.addParameter("stationids", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("stationids");

  // One stationid in stationids variable
  request.addParameter("stationids", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 1);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable2);
  request.removeParameter("stationids");

  // Two comma separated statioid values in a stationids parameter
  request.addParameter("stationids", stringVariable4);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsLocationOptions.itsStationIds.back(), intVariable3);
  request.removeParameter("stationids");

  // Two stationid values in separate stationids parameters
  request.addParameter("stationids", stringVariable2);
  request.addParameter("stationids", stringVariable3);
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.front(), intVariable2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsLocationOptions.itsStationIds.back(), intVariable3);
}

BOOST_AUTO_TEST_CASE(
    query_constructor_parseLocationOptions_numberofstations,
    *boost::unit_test::depends_on("query_constructor_allowMultipleLocationOptions_enabled"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "ab";
  const std::string stringVariable2 = "91";
  const unsigned int unsignedIntVariable2 = 91;
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: [Invalid argument] Fmi::stoul failed to convert 'ab' to unsigned long
  request.addParameter("numberofstations", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("numberofstations");

  request.addParameter("numberofstations", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsLocationOptions.itsNumberOfNearestStations,
                    unsignedIntVariable2);
}

BOOST_AUTO_TEST_CASE(query_constructor_parseMessageTypeOption_messagetype,
                     *boost::unit_test::depends_on("query_constructor"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "";
  const std::string stringVariable2 = "a";
  const std::string stringVariable3 = "METAR";
  const std::string stringVariable4 = stringVariable2 + "," + stringVariable3;
  ;
  const std::string stringVariable5 = "A";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Option 'messagetype' is empty
  request.addParameter("messagetype", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("messagetype");

  // One messagetype
  request.addParameter("messagetype", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsMessageTypes.size(), 1);
  request.removeParameter("messagetype");

  // Two messagetype values in separate parameters
  request.addParameter("messagetype", stringVariable2);
  request.addParameter("messagetype", stringVariable3);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsMessageTypes.size(), 2);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsMessageTypes.front(), stringVariable5);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsMessageTypes.back(), stringVariable3);
  request.removeParameter("messagetype");

  // Two comma separated messagetype values in a messagetype parameter
  request.addParameter("messagetype", stringVariable4);
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsMessageTypes.size(), 2);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsMessageTypes.front(), stringVariable5);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsMessageTypes.back(), stringVariable3);
}

BOOST_AUTO_TEST_CASE(query_constructor_parseTimeOptions_starttime_endtime,
                     *boost::unit_test::depends_on("query_constructor"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "a";
  const std::string stringVariable2 = "2010-10-10T10:10:20Z";
  const std::string stringVariable3 = "2010-12-10T10:10:20Z";
  const std::string stringVariable4 = "timestamptz '20101010T101020Z'";
  const std::string stringVariable5 = "timestamptz '20121010T101020Z'";
  const std::string stringVariable6 = "20101010T101020Z";
  const std::string stringVariable7 = "20101010T101020";
  const std::string stringVariable8 = "20101010101020";

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: 'starttime' and 'endtime' options must be given simultaneously
  request.addParameter("starttime", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("starttime");

  // Exception: 'starttime' and 'endtime' options must be given simultaneously
  request.addParameter("endtime", stringVariable1);
  BOOST_CHECK_THROW({ Query query2(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("endtime");

  // Exception  [Runtime error] Unknown time string 'a'
  request.addParameter("starttime", stringVariable1);
  request.addParameter("endtime", stringVariable1);
  BOOST_CHECK_THROW({ Query query3(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  // Same starttime and endtime
  request.addParameter("starttime", stringVariable2);
  request.addParameter("endtime", stringVariable2);
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsTimeOptions.itsStartTime, stringVariable4);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsTimeOptions.itsEndTime, stringVariable4);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  request.addParameter("starttime", stringVariable6);
  request.addParameter("endtime", stringVariable6);
  Query query5(request, authEngine, config);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsTimeOptions.itsStartTime, stringVariable4);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsTimeOptions.itsEndTime, stringVariable4);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  request.addParameter("starttime", stringVariable7);
  request.addParameter("endtime", stringVariable7);
  Query query6(request, authEngine, config);
  BOOST_CHECK_EQUAL(query6.itsQueryOptions.itsTimeOptions.itsStartTime, stringVariable4);
  BOOST_CHECK_EQUAL(query6.itsQueryOptions.itsTimeOptions.itsEndTime, stringVariable4);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  request.addParameter("starttime", stringVariable8);
  request.addParameter("endtime", stringVariable8);
  Query query7(request, authEngine, config);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsTimeOptions.itsStartTime, stringVariable4);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsTimeOptions.itsEndTime, stringVariable4);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  // Exception: 'starttime' must be earlier than 'endtime'
  request.addParameter("starttime", stringVariable3);
  request.addParameter("endtime", stringVariable2);
  BOOST_CHECK_THROW({ Query query8(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("starttime");
  request.removeParameter("endtime");

  // Exception: Time range too long, maximum is 31 days
  request.addParameter("starttime", stringVariable2);
  request.addParameter("endtime", stringVariable3);
  BOOST_CHECK_THROW({ Query query9(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("starttime");
  request.removeParameter("endtime");
}

BOOST_AUTO_TEST_CASE(query_constructor_parseTimeOptions_time,
                     *boost::unit_test::depends_on("query_constructor"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "a";
  const std::string stringVariable2 = "2010-10-10T10:10:20Z";
  const std::string stringVariable3 = "20101010T101020Z";
  const std::string stringVariable4 = "20101010T101020";
  const std::string stringVariable5 = "20101010101020";
  const std::string stringVariable6 = "1286705420";
  const std::string stringVariable7 = "2010-10-10 10:10:20";
  const std::string stringVariable8 = "timestamptz '20101010T101020Z'";
  const std::string stringVariable9 = "current_timestamp";

  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: [Runtime error] Unknown time string 'a'
  request.addParameter("time", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("time");

  // ISO extended format
  request.addParameter("time", stringVariable2);
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  // ISO format with Z
  request.addParameter("time", stringVariable3);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  // ISO format without Z
  request.addParameter("time", stringVariable4);
  Query query4(request, authEngine, config);
  BOOST_CHECK_EQUAL(query4.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  // Timestamp
  request.addParameter("time", stringVariable5);
  Query query5(request, authEngine, config);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  // Unixtime
  request.addParameter("time", stringVariable6);
  Query query6(request, authEngine, config);
  BOOST_CHECK_EQUAL(query6.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  request.addParameter("time", stringVariable7);
  Query query7(request, authEngine, config);
  BOOST_CHECK_EQUAL(query7.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable8);
  request.removeParameter("time");

  // Default time value
  Query query8(request, authEngine, config);
  BOOST_CHECK_EQUAL(query8.itsQueryOptions.itsTimeOptions.itsObservationTime, stringVariable9);
}

BOOST_AUTO_TEST_CASE(query_constructor_parseTimeOptions_timeformat,
                     *boost::unit_test::depends_on("query_constructor_parseTimeOptions_time"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "a";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Unknown 'timeformat', use 'iso', 'timestamp', 'sql', 'xml' or 'epoch'
  request.addParameter("timeformat", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("timeformat");

  // Supported timeformat values.
  for (auto value : {"iso", "timestamp", "sql", "xml", "epoch"})
  {
    request.addParameter("timeformat", value);
    Query query2(request, authEngine, config);
    request.removeParameter("timeformat");
  }
}

BOOST_AUTO_TEST_CASE(
    query_constructor_option_validity,
    *boost::unit_test::depends_on("query_constructor_parseTimeOptions_starttime_endtime"))
{
  BOOST_CHECK(authEngine != nullptr);

  const std::string stringVariable1 = "a";
  const std::string stringVariable2 = "accepted";
  const std::string stringVariable3 = "rejected";
  const std::string stringVariable4 = "2010-10-10T10:10:20Z";
  const std::string filename = "cnf/aviplugin.conf";
  std::unique_ptr<Config> config(new Config(filename));
  Spine::HTTP::Request request;
  request.addParameter("param", "value");

  // Exception: Unknown 'validity', use 'accepted' or 'rejected'
  request.addParameter("validity", stringVariable1);
  BOOST_CHECK_THROW({ Query query1(request, authEngine, config); }, Spine::Exception);
  request.removeParameter("validity");

  // Accepted case as default
  Query query2(request, authEngine, config);
  BOOST_CHECK_EQUAL(query2.itsQueryOptions.itsValidity, SmartMet::Engine::Avi::Accepted);

  // Set accepted validity
  request.addParameter("validity", stringVariable2);
  Query query3(request, authEngine, config);
  BOOST_CHECK_EQUAL(query3.itsQueryOptions.itsValidity, SmartMet::Engine::Avi::Accepted);
  request.removeParameter("validity");

  // Exception: Time range must be used to query rejected messages
  request.addParameter("validity", stringVariable3);
  BOOST_CHECK_THROW({ Query query4(request, authEngine, config); }, Spine::Exception);

  // Rejected case
  request.addParameter("starttime", stringVariable4);
  request.addParameter("endtime", stringVariable4);
  Query query5(request, authEngine, config);
  BOOST_CHECK_EQUAL(query5.itsQueryOptions.itsValidity, Engine::Avi::Rejected);
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
