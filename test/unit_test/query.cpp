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
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
