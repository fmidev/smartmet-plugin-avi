// ======================================================================

#pragma once

#include <spine/ConfigBase.h>
#include <spine/TableFormatterOptions.h>
#include <QueryLimits.h>
#include <engines/authentication/Engine.h>
#include <boost/noncopyable.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
using namespace SmartMet::Spine;
const int maxMessageTimeRangeDaysDefault = 31;

class Config : public ConfigBase, private boost::noncopyable
{
 public:
  Config(const std::string &theConfigFileName);
  Config() = delete;
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

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
