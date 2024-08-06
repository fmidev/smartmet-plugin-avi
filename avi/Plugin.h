// ======================================================================
/*!
 * \brief SmartMet AVIDB plugin interface
 */
// ======================================================================

#pragma once

#include "Config.h"
#include <memory>
#include <engines/authentication/Engine.h>
#include <engines/avi/Engine.h>
#include <spine/HTTP.h>
#include <spine/Reactor.h>
#include <spine/SmartMetPlugin.h>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
class Plugin : public SmartMetPlugin
{
 public:
  Plugin() = delete;
  Plugin(const Plugin &other) = delete;
  Plugin &operator=(const Plugin &other) = delete;
  Plugin(Reactor *theReactor, const char *theConfigFileName);

  const std::string &getPluginName() const override;
  int getRequiredAPIVersion() const override;
  bool queryIsFast(const SmartMet::Spine::HTTP::Request &theRequest) const override;

 protected:
  void init() override;
  void shutdown() override;
  void requestHandler(SmartMet::Spine::Reactor &theReactor,
                      const SmartMet::Spine::HTTP::Request &theRequest,
                      SmartMet::Spine::HTTP::Response &theResponse) override;

 private:
  void query(const SmartMet::Spine::HTTP::Request &theRequest,
             SmartMet::Spine::HTTP::Response &theResponse);

  const std::string itsModuleName;
  const std::string itsConfigFileName;
  std::unique_ptr<Config> itsConfig;

  SmartMet::Spine::Reactor *itsReactor = nullptr;
  SmartMet::Engine::Avi::Engine *itsAviEngine = nullptr;
  SmartMet::Engine::Authentication::Engine *itsAuthEngine = nullptr;
};

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
