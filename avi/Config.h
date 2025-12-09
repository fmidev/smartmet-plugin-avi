// ======================================================================

#pragma once

#include <engines/authentication/Engine.h>
#include <spine/ConfigBase.h>
#include <spine/TableFormatterOptions.h>
#include <QueryLimits.h>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
using namespace SmartMet::Spine;
const int maxMessageTimeRangeDaysDefault = 31;

class Config : public ConfigBase
{
 public:
  ~Config() override;
  Config() = delete;
  Config(const std::string &theConfigFileName);

  const TableFormatterOptions &tableFormatterOptions() const { return itsTableFormatterOptions; }
  const QueryLimits &getQueryLimits(const SmartMet::Engine::Authentication::Engine *authEngine,
                                    const std::string &apiKey) const;
  bool useAuthentication() const { return itsUseAuthEngine; }

 private:
  TableFormatterOptions itsTableFormatterOptions;
  bool itsUseAuthEngine;
  std::map<std::string, QueryLimits> itsQueryLimits;
};  // class Config

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
