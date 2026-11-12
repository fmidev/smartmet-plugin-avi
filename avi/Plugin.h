// ======================================================================
/*!
 * \brief SmartMet AVIDB plugin interface
 */
// ======================================================================

#pragma once

#include "Config.h"
#include <spine/SmartMetPlugin.h>
#include <memory>
#include <string>

namespace SmartMet::Engine::Authentication
{
class Engine;
}  // namespace SmartMet::Engine::Authentication
namespace SmartMet::Engine::Avi
{
class Engine;
}  // namespace SmartMet::Engine::Avi
namespace SmartMet::Spine::HTTP
{
class Request;
class Response;
}  // namespace SmartMet::Spine::HTTP
namespace SmartMet::Spine
{
class Reactor;
}  // namespace SmartMet::Spine

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
  ~Plugin() override = default;

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
  std::shared_ptr<SmartMet::Engine::Avi::Engine> itsAviEngine;
  std::shared_ptr<SmartMet::Engine::Authentication::Engine> itsAuthEngine;
};

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet
