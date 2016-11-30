// ======================================================================

#pragma once

#include "Config.h"

#include <spine/HTTP.h>
#include <spine/Parameter.h>
#include <engines/avi/Engine.h>

#include <list>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
// ----------------------------------------------------------------------
/*!
 * \brief Parsing and storage for request parameters
 */
// ----------------------------------------------------------------------

class Query
{
 public:
  Query(const SmartMet::Spine::HTTP::Request& request, const std::unique_ptr<Config>& config);
  Query() = delete;

  SmartMet::Engine::Avi::QueryOptions itsQueryOptions;
  std::string itsFormat;
  unsigned int itsPrecision;

 private:
  void checkIfMultipleLocationOptionsAllowed(bool allowMultipleLocationOptions);

  void parseMessageTypeOption(const SmartMet::Spine::HTTP::Request& theRequest);
  void parseParamOption(const SmartMet::Spine::HTTP::Request& theRequest);
  void parseLocationOptions(const SmartMet::Spine::HTTP::Request& theRequest,
                            bool allowMultipleLocationOptions);
  void parseTimeOptions(const SmartMet::Spine::HTTP::Request& theRequest, int maxTimeRangeDays);
};

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
