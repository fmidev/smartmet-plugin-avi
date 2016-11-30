// ======================================================================
/*!
 * \brief SmartMet AVIDB plugin interface
 */
// ======================================================================

#pragma once

#include "Config.h"

#include <spine/SmartMetPlugin.h>
#include <spine/Reactor.h>
#include <spine/HTTP.h>
#include <engines/avi/Engine.h>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
class Plugin : public SmartMetPlugin, private boost::noncopyable
{
 public:
  Plugin(Reactor *theReactor, const char *theConfigFileName);
  virtual ~Plugin();

  const std::string &getPluginName() const;
  int getRequiredAPIVersion() const;
  bool queryIsFast(const SmartMet::Spine::HTTP::Request &theRequest) const;

 protected:
  void init();
  void shutdown();
  void requestHandler(SmartMet::Spine::Reactor &theReactor,
                      const SmartMet::Spine::HTTP::Request &theRequest,
                      SmartMet::Spine::HTTP::Response &theResponse);

 private:
  void query(const SmartMet::Spine::HTTP::Request &theRequest,
             SmartMet::Spine::HTTP::Response &theResponse);

  const std::string itsModuleName;
  const std::string itsConfigFileName;
  std::unique_ptr<Config> itsConfig;

  SmartMet::Spine::Reactor *itsReactor;
  SmartMet::Engine::Avi::Engine *itsAviEngine;
};

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
