// ======================================================================

#pragma once

#include <spine/ConfigBase.h>
#include <spine/TableFormatterOptions.h>
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
  int getMaxMessageStations() const { return itsMaxMessageStations; }
  int getMaxMessageRows() const { return itsMaxMessages; }
  int getMaxMessageTimeRangeDays() const { return itsMaxMessageTimeRangeDays; }
  bool allowMultipleLocationOptions() const { return itsAllowMultipleLocationOptions; }
 private:
  TableFormatterOptions itsTableFormatterOptions;

  int itsMaxMessageStations;  // if config value not given or < 0, engine rules; if 0, unlimited
  int itsMaxMessages;         // if config value not given or < 0, engine rules; if 0, unlimited
  int itsMaxMessageTimeRangeDays;  // if config value not given or < 0, defaults to
                                   // maxMessageTimeRangeDaysDefault; if 0, unlimited

  bool itsAllowMultipleLocationOptions;
};  // class Config

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
