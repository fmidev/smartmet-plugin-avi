#include "Plugin.h"
#include <spine/PluginTest.h>

using namespace std;

void prelude(SmartMet::Spine::Reactor &reactor)
{
  auto handlers = reactor.getURIMap();
  while (handlers.find("/avi") == handlers.end())
  {
    sleep(1);
    handlers = reactor.getURIMap();
  }

  cout << endl << "Testing avi plugin" << endl << "====================" << endl;
}

int main()
{
  SmartMet::Spine::Options options;
  options.quiet = true;
  options.defaultlogging = false;
  options.configfile = "cnf/reactor.conf";

  return SmartMet::Spine::PluginTest::test(options, prelude, false, 10);
}
