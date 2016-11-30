// ======================================================================

#include "Config.h"
#include <spine/Exception.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <stdexcept>

using namespace std;

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
// ----------------------------------------------------------------------
/*!
 * \brief The only permitted constructor requires a configfile
 */
// ----------------------------------------------------------------------

Config::Config(const string& theConfigFileName)
    : ConfigBase(theConfigFileName), itsTableFormatterOptions()
{
  try
  {
    const auto& theConfig = get_config();

    // Max # of stations allowed in message query; if missing or < 0, engine rules; if 0, unlimited

    if (theConfig.exists("message.maxstations"))
      theConfig.lookupValue("message.maxstations", itsMaxMessageStations);
    else
      itsMaxMessageStations = -1;

    // Max # of rows fetched by message query; if missing or < 0, engine rules; if 0, unlimited

    if (theConfig.exists("message.maxrows"))
      theConfig.lookupValue("message.maxrows", itsMaxMessages);
    else
      itsMaxMessages = -1;

    // Max message time range length in days; if < 0, use default; if 0, unlimited

    if (theConfig.exists("message.maxrangedays"))
      theConfig.lookupValue("message.maxrangedays", itsMaxMessageTimeRangeDays);
    else
      itsMaxMessageTimeRangeDays = -1;

    if (itsMaxMessageTimeRangeDays < 0)
      itsMaxMessageTimeRangeDays = maxMessageTimeRangeDaysDefault;

    // Allow multiple location options ?

    if (theConfig.exists("multiplelocationoptions"))
      theConfig.lookupValue("multiplelocationoptions", itsAllowMultipleLocationOptions);
    else
      itsAllowMultipleLocationOptions = false;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
