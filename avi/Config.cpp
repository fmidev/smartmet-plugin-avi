// ======================================================================

#include "Config.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <engines/authentication/Engine.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <spine/Exceptions.h>
#include <stdexcept>

using namespace std;

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{

Config::~Config() = default;

namespace
{
// An optional integer limit, -1 if not set
int optional_limit(const libconfig::Config &theConfig, const char *theName)
{
  int value = 0;

  if (theConfig.exists(theName))
    theConfig.lookupValue(theName, value);
  else
    value = -1;
  return value;
}
}  // namespace

// ----------------------------------------------------------------------
/*!
 * \brief The only permitted constructor requires a configfile
 */
// ----------------------------------------------------------------------

Config::Config(const string &theConfigFileName) : ConfigBase(theConfigFileName)
{
  try
  {
    const auto &theConfig = get_config();

    // Default query limitations applied when apikey groups are disabled or request apikey
    // is not available or it's unknown (has no group membership).
    //
    // Max # of stations allowed in message query; if missing or < 0, engine rules; if 0, unlimited

    int maxMessageStations = optional_limit(theConfig, "message.maxstations");

    // Max # of rows fetched by message query; if missing or < 0, engine rules; if 0, unlimited

    int maxMessages = optional_limit(theConfig, "message.maxrows");

    // Max message time range length in days; if < 0, use default; if 0, unlimited

    int maxMessageTimeRangeDays = optional_limit(theConfig, "message.maxrangedays");

    if (maxMessageTimeRangeDays < 0)
      maxMessageTimeRangeDays = maxMessageTimeRangeDaysDefault;

    // Allow multiple location options ?

    bool allowMultipleLocationOptions = false;

    if (theConfig.exists("multiplelocationoptions"))
      theConfig.lookupValue("multiplelocationoptions", allowMultipleLocationOptions);
    else
      allowMultipleLocationOptions = false;

    // Query limitations for apikey groups (groups are implemented as token values for
    // service 'avi' in authentication database). Apikey's group membership is checked
    // in alphabetical group name (token value) order until first (if any) membership
    // is found.
    //
    // Default query limitations are applied for group's missing settings.
    //
    // Group limitations are disabled (the default limitations are used) if
    // apikey.disabled is set to true

    QueryLimits defaultLimits(
        maxMessageStations, maxMessages, maxMessageTimeRangeDays, allowMultipleLocationOptions);
    const char *optDisabled = "apikey.disabled";
    const char *optGroups = "apikey.groups";
    bool disabled = false;

    if (theConfig.exists(optDisabled))
      theConfig.lookupValue(optDisabled, disabled);

    itsUseAuthEngine = !disabled;

    if (itsUseAuthEngine && theConfig.exists(optGroups))
    {
      libconfig::Setting &groups = theConfig.lookup(optGroups);

      if (!groups.isList())
        throw Fmi::Exception(BCP,
                             "Groups must be stored in list of groups delimited by {}: line " +
                                 Fmi::to_string(groups.getSourceLine()));

      for (int i = 0; i < groups.getLength(); i++)
        parseGroup(groups[i], defaultLimits);
    }

    // Authentication engine needs not to be loaded if there's no apikey groups

    itsUseAuthEngine &= (!itsQueryLimits.empty());

    // Store the default limits as the last map entry

    string defaultGroup(!itsQueryLimits.empty() ? itsQueryLimits.crbegin()->first + "Z"
                                                : "default");
    itsQueryLimits[defaultGroup] = defaultLimits;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse the query limits of an apikey group
 */
// ----------------------------------------------------------------------

void Config::parseGroup(libconfig::Setting &group, const QueryLimits &defaultLimits)
{
  if (!group.isGroup())
    throw Fmi::Exception(BCP,
                         "Groups must be stored in list of groups delimited by {}: line " +
                             Fmi::to_string(group.getSourceLine()));

  QueryLimits groupLimits(defaultLimits);
  string groupName;

  for (int j = 0; j < group.getLength(); j++)
  {
    try
    {
      string paramName = group[j].getName();

      if (paramName == "name")
      {
        string value = group[j];
        groupName = value;

        if (itsQueryLimits.find(groupName) != itsQueryLimits.end())
          throw Fmi::Exception(BCP, string("Duplicate group name"));
      }
      else if ((paramName == "maxstations") || (paramName == "maxrows") ||
               (paramName == "maxrangedays"))
      {
        int value = group[j];

        if (paramName == "maxstations")
          groupLimits.setMaxMessageStations(value);
        else if (paramName == "maxrows")
          groupLimits.setMaxMessageRows(value);
        else
          groupLimits.setMaxMessageTimeRangeDays(
              (value >= 0) ? value : maxMessageTimeRangeDaysDefault);
      }
      else if (paramName == "multiplelocationoptions")
      {
        bool allowMultipleLocationOptions = group[j];
        groupLimits.setAllowMultipleLocationOptions(allowMultipleLocationOptions);
      }
      else
        throw Fmi::Exception(BCP, string("Unknown variable '") + paramName);
    }
    catch (...)
    {
      Spine::Exceptions::handle("AVI plugin");
    }
  }

  if (groupName.empty())
    throw Fmi::Exception(BCP,
                         string("Group name missing for group definition starting at line ") +
                             Fmi::to_string(group.getSourceLine()));

  itsQueryLimits[groupName] = groupLimits;
}

const QueryLimits &Config::getQueryLimits(
    const SmartMet::Engine::Authentication::Engine *authEngine, const std::string &apiKey) const
{
  // The default limits are stored as the last map entry

  auto it = itsQueryLimits.end();

  if (authEngine && (!apiKey.empty()) && (itsQueryLimits.size() > 1))
  {
    for (it = itsQueryLimits.begin(); (it != itsQueryLimits.end()); it++)
      if (authEngine->authorize(apiKey, it->first, "avi", true))
        break;
  }

  return (it != itsQueryLimits.end()) ? it->second : itsQueryLimits.crbegin()->second;
}

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
