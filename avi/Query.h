// ======================================================================

#pragma once

#include <engines/avi/Engine.h>
#include <memory>
#include <string>

namespace SmartMet::Engine::Authentication
{
class Engine;
}  // namespace SmartMet::Engine::Authentication
namespace SmartMet::Plugin::Avi
{
class Config;
}  // namespace SmartMet::Plugin::Avi
namespace SmartMet::Spine::HTTP
{
class Request;
}  // namespace SmartMet::Spine::HTTP

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
  Query(const SmartMet::Spine::HTTP::Request &request,
        const SmartMet::Engine::Authentication::Engine *authEngine,
        const std::unique_ptr<Config> &config);
  Query() = delete;

  SmartMet::Engine::Avi::QueryOptions itsQueryOptions;
  std::string itsFormat;
  unsigned int itsPrecision;

 private:
  void checkIfMultipleLocationOptionsAllowed(bool allowMultipleLocationOptions) const;

  void parseMessageTypeOption(const SmartMet::Spine::HTTP::Request &theRequest);
  void parseParamOption(const SmartMet::Spine::HTTP::Request &theRequest);
  void parsePlaceOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                         bool allowMultipleLocationOptions);
  void parseBBoxOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                        bool allowMultipleLocationOptions);
  void parseLonLatOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                          bool allowMultipleLocationOptions);
  void parseLonLatListOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                              bool allowMultipleLocationOptions);
  void parseWKTOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                       bool allowMultipleLocationOptions);
  void parseIcaoOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                        bool allowMultipleLocationOptions);
  void parseCountryOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                           bool allowMultipleLocationOptions);
  void parseStationIdOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                             bool allowMultipleLocationOptions);
  void parseMaxDistance(const SmartMet::Spine::HTTP::Request &theRequest);
  void parseLocationOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                            bool allowMultipleLocationOptions);
  void parseTimeOptions(const SmartMet::Spine::HTTP::Request &theRequest, int maxTimeRangeInDays);
};

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
