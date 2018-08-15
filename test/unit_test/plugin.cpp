#define BOOST_TEST_MODULE "PluginClassModule"

#include "Plugin.h"

#include <boost/test/included/unit_test.hpp>
#include <spine/Reactor.h>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
BOOST_AUTO_TEST_CASE(query_aviplugin_constructor)
{
  Spine::Options opts;
  opts.defaultlogging = false;
  opts.configfile = "cnf/reactor-with-authentication.conf";
  opts.parseConfig();
  std::unique_ptr<Spine::Reactor> reactor;
  reactor.reset(new Spine::Reactor(opts));

  Plugin aviPlugin(reactor.get(), "cnf/aviplugin.conf");

  BOOST_CHECK_EQUAL(aviPlugin.getPluginName(), "Avi");
}
}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
