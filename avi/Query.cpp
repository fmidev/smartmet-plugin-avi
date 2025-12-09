// ======================================================================

#include "Query.h"
#include <boost/algorithm/string/trim.hpp>
#include <macgyver/DateTime.h>
#include <macgyver/DistanceParser.h>
#include <macgyver/Exception.h>
#include <macgyver/StringConversion.h>
#include <spine/Convenience.h>
#include <spine/FmiApiKey.h>

using namespace std;
using namespace boost::algorithm;

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
// ----------------------------------------------------------------------
/*!
 * \brief Return list of pairs of values from comma separated string
 */
// ----------------------------------------------------------------------

namespace
{

template <typename T>
T toType(const string & /* s */)
{
  throw Fmi::Exception(BCP, "internal: unsupported conversion");
}

template <>
SmartMet::Engine::Avi::StationIdType toType<SmartMet::Engine::Avi::StationIdType>(const string &s)
{
  return Fmi::stol(s);
}

template <>
double toType<double>(const string &s)
{
  return Fmi::stod(s);
}

template <>
string toType<string>(const string &s)
{
  return s;
}

template <typename T>
std::optional<list<pair<T, T>>> listOfPairs(const string &commaSeparatedStr,
                                            const char *optionName,
                                            size_t nPairs,
                                            bool latlonPairs = true)
{
  try
  {
    string valueStr(trim_copy(commaSeparatedStr));

    if (valueStr.empty())
      return std::optional<list<pair<T, T>>>();

    std::vector<std::string> flds;
    boost::split(flds, valueStr, boost::is_any_of(","));
    size_t nValues = 2 * nPairs;

    // Given number of value pairs (e.g. 2 for bbox) required ?

    if ((nValues > 0) && (flds.size() != nValues))
      throw Fmi::Exception(BCP,
                           Fmi::to_string(nValues) + string(" values required for option '") +
                               optionName + "'; '" + commaSeparatedStr + "'");

    if ((nValues == 0) && ((flds.size() % 2) != 0))
      throw Fmi::Exception(BCP,
                           string("Even number of values required for option '") + optionName +
                               "'; '" + commaSeparatedStr + "'");

    size_t n;

    for (n = 0, nValues = flds.size(); (n < nValues); n++)
    {
      boost::trim(flds[n]);

      if (flds[n].empty())
        throw Fmi::Exception(BCP,
                             string("Empty value for option '") + optionName + "' at position " +
                                 Fmi::to_string(n + 1) + "; '" + commaSeparatedStr + "'");
    }

    list<pair<T, T>> valueList;

    for (n = 0; (n < nValues);)
    {
      try
      {
        auto value1 = toType<T>(flds[n]);
        n++;
        auto value2 = toType<T>(flds[n]);
        n++;

        if (latlonPairs)
        {
          // Order lon,lat expected
          //
          if ((value1 < -180) || (value1 > 180))
            throw Fmi::Exception(BCP,
                                 string("Value in range [-180,180] expected for option '") +
                                     optionName + "' at position " + Fmi::to_string(n - 1) + "; '" +
                                     commaSeparatedStr + "'");
          if ((value2 < -90) || (value2 > 90))
            throw Fmi::Exception(BCP,
                                 string("Value in range [-90,90] expected for option '") +
                                     optionName + "' at position " + Fmi::to_string(n) + "; '" +
                                     commaSeparatedStr + "'");
        }

        valueList.push_back(make_pair<T, T>((T)value1, (T)value2));
      }
      catch (...)
      {
        throw Fmi::Exception(BCP,
                             string("Invalid value for option '") + optionName + "' at position " +
                                 Fmi::to_string(n + 1) + "; '" + commaSeparatedStr + "'");
      }
    }
    return std::optional<list<pair<T, T>>>(valueList);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return list of values from comma separated string
 */
// ----------------------------------------------------------------------

template <typename T>
bool validValue(const T &value, double minValue, double maxValue)
{
  return ((value >= minValue) && (value <= maxValue));
}

template <>
bool validValue<string>(const string & /*value */, double /*minValue*/, double /*maxValue*/)
{
  return true;
}

template <typename T>
std::optional<list<T>> listOfValues(const string &commaSeparatedStr,
                                    const char *optionName,
                                    size_t nValues = 0,
                                    bool even = true,
                                    bool swap = false,
                                    bool latlonValues = true)
{
  try
  {
    string valueStr(trim_copy(commaSeparatedStr));

    if (valueStr.empty())
      return std::optional<list<T>>();

    std::vector<std::string> flds;
    boost::split(flds, valueStr, boost::is_any_of(","));

    // Given or even number of values required ?

    if (nValues > 0)
    {
      if (nValues != flds.size())
        throw Fmi::Exception(BCP,
                             Fmi::to_string(nValues) + string(" values required for option '") +
                                 optionName + "'; '" + commaSeparatedStr + "'");
    }
    else
    {
      nValues = flds.size();

      if (even && ((nValues % 2) != 0))
        throw Fmi::Exception(BCP,
                             string("Even number of values required for option '") + optionName +
                                 "'; '" + commaSeparatedStr + "'");
    }

    size_t n;
    size_t np;

    for (n = 0; (n < nValues); n++)
    {
      boost::trim(flds[n]);

      if (flds[n].empty())
        throw Fmi::Exception(BCP,
                             string("Empty value for option '") + optionName + "' at position " +
                                 Fmi::to_string(n + 1) + "; '" + commaSeparatedStr + "'");
    }

    list<T> valueList;

    for (n = 0, np = 0; (n < nValues); n++)
    {
      // Swap the elements (e.g. convert latlons to lonlats) if requested
      //
      size_t nn = (swap ? (((n % 2) != 0) ? np : n + 1) : n);
      bool castOk = false;

      try
      {
        castOk = false;
        T value = toType<T>(flds[nn]);
        castOk = true;

        if (latlonValues)
        {
          // Order lon,lat expected
          //
          if ((n % 2) == 0)
          {
            if (!validValue<T>(value, -180, 180))
              throw Fmi::Exception(BCP,
                                   string("Value in range [-180,180] expected for option '") +
                                       optionName + "' at position " + Fmi::to_string(nn + 1) +
                                       "; '" + commaSeparatedStr + "'");
          }
          else
          {
            if (!validValue<T>(value, -90, 90))
              throw Fmi::Exception(BCP,
                                   string("Value in range [-90,90] expected for option '") +
                                       optionName + "' at position " + Fmi::to_string(nn + 1) +
                                       "; '" + commaSeparatedStr + "'");
          }
        }

        valueList.push_back(value);
      }
      catch (...)
      {
        if (castOk)
          throw Fmi::Exception::Trace(BCP,
                                      string("Invalid value for option '") + optionName +
                                          "' at position " + Fmi::to_string(nn + 1) + "; '" +
                                          commaSeparatedStr + "'");
      }

      np = n;
    }

    return std::optional<list<T>>(valueList);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse 'message type' query option.
 *
 * 		  The types given are validated by aviengine when executing message query.
 * 		  Default is all types
 */
// ----------------------------------------------------------------------

string errMsgOptionIsEmpty(const char *optionName)
{
  try
  {
    return string("Option '") + optionName + "' is empty";
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace

void Query::parseMessageTypeOption(const SmartMet::Spine::HTTP::Request &theRequest)
{
  try
  {
    const char *optionName;

    auto messagetypes = theRequest.getParameterList(optionName = "messagetype");
    if (!messagetypes.empty())
    {
      for (const string &messagetype : messagetypes)
      {
        auto listOfMessageTypes =
            listOfValues<string>(messagetype, optionName, 0, false, false, false);

        if ((!listOfMessageTypes) || listOfMessageTypes->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (const string &m : *listOfMessageTypes)
          itsQueryOptions.itsMessageTypes.push_back(Fmi::ascii_toupper_copy(m));
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse 'param' query option.
 *
 * 		  The parameters given are validated by aviengine when executing query.
 */
// ----------------------------------------------------------------------

void Query::parseParamOption(const SmartMet::Spine::HTTP::Request &theRequest)
{
  try
  {
    const char *optionName;

    auto params = theRequest.getParameterList(optionName = "param");
    if (!params.empty())
    {
      for (const string &param : params)
      {
        auto listOfParams = listOfValues<string>(param, optionName, 0, false, false, false);

        if ((!listOfParams) || listOfParams->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (const string &p : *listOfParams)
          itsQueryOptions.itsParameters.push_back(Fmi::ascii_tolower_copy(p));
      }

      return;
    }

    throw Fmi::Exception(BCP, "Option 'param' must be provided");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse location/station related query options.
 *
 * 	  Note: Avi engine supports querying with multiple location options, but depending
 * 	  on configuration, we may allow only one location option to be given.
 *
 *	  The engine checks geometry type of given single wkt, and if the type is linestring,
 *	  the route order (station distance from route segment starting point) is used;
 *	  for other wkt types and for other types of location options, icao ordering is used.
 */
// ----------------------------------------------------------------------

void Query::checkIfMultipleLocationOptionsAllowed(bool allowMultipleLocationOptions) const
{
  try
  {
    if (!allowMultipleLocationOptions)
    {
      if (!itsQueryOptions.itsLocationOptions.itsLonLats.empty() ||
          !itsQueryOptions.itsLocationOptions.itsStationIds.empty() ||
          !itsQueryOptions.itsLocationOptions.itsIcaos.empty() ||
          !itsQueryOptions.itsLocationOptions.itsPlaces.empty() ||
          !itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.empty() ||
          !itsQueryOptions.itsLocationOptions.itsBBoxes.empty())
        throw Fmi::Exception(
            BCP,
            "Only one location option ('place', 'places', 'bbox', 'lonlat', 'latlon', 'lonlats', "
            "'latlons', 'wkt', 'icao', 'icaos', 'stationid') allowed");
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void Query::parseLocationOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                                 bool allowMultipleLocationOptions)
{
  try
  {
    // Named locations (station names)
    //
    // place=place1&place=place2&place=...
    // places=place1,place2,...

    const char *optionName;

    auto places = theRequest.getParameterList(optionName = "place");
    if (!places.empty())
    {
      for (const string &place : places)
      {
        if (place.empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsPlaces.push_back(place);
      }
    }

    places = theRequest.getParameterList(optionName = "places");
    if (!places.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &place : places)
      {
        auto listOfPlaces = listOfValues<string>(place, optionName, 0, false, false, false);

        if ((!listOfPlaces) || listOfPlaces->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (const string &p : *listOfPlaces)
          itsQueryOptions.itsLocationOptions.itsPlaces.push_back(p);
      }
    }

    // Bounding boxes
    //
    // bbox=bllon,bllat,trlon,trlat&bbox=...

    auto bboxes = theRequest.getParameterList(optionName = "bbox");
    if (!bboxes.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &bbox : bboxes)
      {
        auto listOfTwoLonLatPairs = listOfPairs<double>(bbox, optionName, 2);

        if ((!listOfTwoLonLatPairs) || listOfTwoLonLatPairs->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        auto west = listOfTwoLonLatPairs->front().first;
        auto south = listOfTwoLonLatPairs->front().second;
        auto east = listOfTwoLonLatPairs->back().first;
        auto north = listOfTwoLonLatPairs->back().second;

        itsQueryOptions.itsLocationOptions.itsBBoxes.push_back(
            SmartMet::Engine::Avi::BBox(west, east, south, north));
      }
    }

    // Latlon points
    //
    // lonlat=lon1,lat1&lonlat=lon2,lat2&lonlat=...
    // latlon=lat1,lon1&latlon=lat2,lon2&latlon=...
    // lonlats=lon1,lat1,lon2,lat2,...&lonlats=...
    // latlons=lat1,lon1,lat2,lon2,...&latlons=...

    auto lonlats = theRequest.getParameterList(optionName = "lonlat");
    if (!lonlats.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &lonlat : lonlats)
      {
        auto listOfTwoValues = listOfValues<double>(lonlat, optionName, 2);

        if ((!listOfTwoValues) || listOfTwoValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsLonLats.push_back(
            SmartMet::Engine::Avi::LonLat(listOfTwoValues->front(), listOfTwoValues->back()));
      }
    }

    auto latlons = theRequest.getParameterList(optionName = "latlon");
    if (!latlons.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &latlon : latlons)
      {
        auto listOfTwoValues = listOfValues<double>(latlon, optionName, 2, true, true);

        if ((!listOfTwoValues) || listOfTwoValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsLonLats.push_back(
            SmartMet::Engine::Avi::LonLat(listOfTwoValues->front(), listOfTwoValues->back()));
      }
    }

    lonlats = theRequest.getParameterList(optionName = "lonlats");
    if (!lonlats.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &lonlat : lonlats)
      {
        auto listOfEvenNValues = listOfValues<double>(lonlat, optionName);

        if ((!listOfEvenNValues) || listOfEvenNValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (auto it = listOfEvenNValues->begin(); (it != listOfEvenNValues->end());)
        {
          auto lon = *it;
          it++;
          auto lat = *it;
          it++;

          itsQueryOptions.itsLocationOptions.itsLonLats.push_back(
              SmartMet::Engine::Avi::LonLat(lon, lat));
        }
      }
    }

    latlons = theRequest.getParameterList(optionName = "latlons");
    if (!latlons.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &latlon : latlons)
      {
        auto listOfEvenNValues = listOfValues<double>(latlon, optionName, 0, true, true);

        if ((!listOfEvenNValues) || listOfEvenNValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (auto it = listOfEvenNValues->begin(); (it != listOfEvenNValues->end());)
        {
          auto lon = *it;
          it++;
          auto lat = *it;
          it++;

          itsQueryOptions.itsLocationOptions.itsLonLats.push_back(
              SmartMet::Engine::Avi::LonLat(lon, lat));
        }
      }
    }

    // WKT's

    auto wkts = theRequest.getParameterList(optionName = "wkt");
    if (!wkts.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &wkt : wkts)
      {
        if (boost::trim_copy(wkt).empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.push_back(wkt);
      }
    }

    // Icao codes

    auto icaos = theRequest.getParameterList(optionName = "icao");
    if (!icaos.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &icao : icaos)
      {
        if (icao.empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsIcaos.push_back(icao);
      }
    }

    icaos = theRequest.getParameterList(optionName = "icaos");
    if (!icaos.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &icao : icaos)
      {
        auto listOfIcaos = listOfValues<string>(icao, optionName, 0, false, false, false);

        if ((!listOfIcaos) || listOfIcaos->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (const string &p : *listOfIcaos)
          itsQueryOptions.itsLocationOptions.itsIcaos.push_back(p);
      }
    }

    // Country codes

    auto countries = theRequest.getParameterList(optionName = "country");
    if (!countries.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &country : countries)
      {
        if (country.empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        itsQueryOptions.itsLocationOptions.itsCountries.push_back(country);
      }
    }

    countries = theRequest.getParameterList(optionName = "countries");
    if (!countries.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &country : countries)
      {
        auto listOfCountries = listOfValues<string>(country, optionName, 0, false, false, false);

        if ((!listOfCountries) || listOfCountries->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (const string &c : *listOfCountries)
          itsQueryOptions.itsLocationOptions.itsCountries.push_back(c);
      }
    }

    // Station id's

    auto stationids = theRequest.getParameterList(optionName = "stationid");
    if (!stationids.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &stationid : stationids)
      {
        auto listOfNValues = listOfValues<SmartMet::Engine::Avi::StationIdType>(
            stationid, optionName, 0, false, false, false);

        if ((!listOfNValues) || listOfNValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (auto id : *listOfNValues)
          itsQueryOptions.itsLocationOptions.itsStationIds.push_back(id);
      }
    }

    stationids = theRequest.getParameterList(optionName = "stationids");
    if (!stationids.empty())
    {
      checkIfMultipleLocationOptionsAllowed(allowMultipleLocationOptions);

      for (const string &stationid : stationids)
      {
        auto listOfNValues = listOfValues<SmartMet::Engine::Avi::StationIdType>(
            stationid, optionName, 0, false, false, false);

        if ((!listOfNValues) || listOfNValues->empty())
          throw Fmi::Exception(BCP, errMsgOptionIsEmpty(optionName));

        for (auto id : *listOfNValues)
          itsQueryOptions.itsLocationOptions.itsStationIds.push_back(id);
      }
    }

    // Max station distance (km) is taken into account with coordinate, bbox and wkt queries

    if (!(itsQueryOptions.itsLocationOptions.itsLonLats.empty() &&
          itsQueryOptions.itsLocationOptions.itsBBoxes.empty() &&
          itsQueryOptions.itsLocationOptions.itsWKTs.itsWKTs.empty()))
    {
      const char *errMsg =
          "Option maxdistance is required with latlon/lonlat, bbox and wkt options";
      std::string maxdistance =
          SmartMet::Spine::required_string(theRequest.getParameter("maxdistance"), errMsg);
      // If plain number is given it is kilometers
      if (std::isdigit(maxdistance.back()))
        maxdistance.append("km");

      itsQueryOptions.itsLocationOptions.itsMaxDistance =
          Fmi::DistanceParser::parse_meter(maxdistance);

      if (itsQueryOptions.itsLocationOptions.itsMaxDistance < 0)
        throw Fmi::Exception(BCP, "maxdistance can't be negative");
    }
    else
      itsQueryOptions.itsLocationOptions.itsMaxDistance = 0;

    // Number of nearest stations returned for point queries

    itsQueryOptions.itsLocationOptions.itsNumberOfNearestStations =
        SmartMet::Spine::optional_unsigned_long(theRequest.getParameter("numberofstations"), 1);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse time related query options
 */
// ----------------------------------------------------------------------

void Query::parseTimeOptions(const SmartMet::Spine::HTTP::Request &theRequest,
                             int maxTimeRangeInDays)
{
  try
  {
    // Either time range ('starttime' and 'endtime') or observation time
    // ('time', defaults to current time) can be given.
    // Rejected messages can only be queried with time range.
    //
    // Time range query for accepted messages returns either valid messages or
    // messages created within the range, depending on option 'validrangemessages'
    // (default 1 (nonzero); valid messages)

    string startTime = SmartMet::Spine::optional_string(theRequest.getParameter("startTime"), "");
    string endTime = SmartMet::Spine::optional_string(theRequest.getParameter("endTime"), "");
    string obsTime = SmartMet::Spine::optional_string(theRequest.getParameter("time"), "");

    itsQueryOptions.itsTimeOptions.itsQueryValidRangeMessages =
        (SmartMet::Spine::optional_unsigned_long(theRequest.getParameter("validrangemessages"), 1) >
         0);

    if (startTime.empty() != endTime.empty())
      throw Fmi::Exception(BCP, "'starttime' and 'endtime' options must be given simultaneously");

    if (!startTime.empty())
    {
      if (!obsTime.empty())
        throw Fmi::Exception(
            BCP,
            "Can't specify both time range ('starttime' and 'endtime') and observation time "
            "('time')");

      Fmi::DateTime st = Fmi::TimeParser::parse(startTime);
      Fmi::DateTime et = Fmi::TimeParser::parse(endTime);

      if (st > et)
        throw Fmi::Exception(BCP, "'starttime' must be earlier than 'endtime'");

      if ((maxTimeRangeInDays > 0) && ((et - st).hours() > (maxTimeRangeInDays * 24)))
        throw Fmi::Exception(
            BCP, "Time range too long, maximum is " + Fmi::to_string(maxTimeRangeInDays) + " days");

      itsQueryOptions.itsTimeOptions.itsStartTime =
          string("timestamptz '") + Fmi::to_iso_string(st) + "Z'";
      itsQueryOptions.itsTimeOptions.itsEndTime =
          string("timestamptz '") + Fmi::to_iso_string(et) + "Z'";
    }
    else if (itsQueryOptions.itsValidity == SmartMet::Engine::Avi::Rejected)
      throw Fmi::Exception(BCP, "Time range must be used to query rejected messages");
    else if (!obsTime.empty())
      itsQueryOptions.itsTimeOptions.itsObservationTime =
          string("timestamptz '") + Fmi::to_iso_string(Fmi::TimeParser::parse(obsTime)) + "Z'";
    else
      itsQueryOptions.itsTimeOptions.itsObservationTime = "current_timestamp";

    // Time output format

    itsQueryOptions.itsTimeOptions.itsTimeFormat =
        SmartMet::Spine::optional_string(theRequest.getParameter("timeformat"), "iso");

    if ((itsQueryOptions.itsTimeOptions.itsTimeFormat != "iso") &&
        (itsQueryOptions.itsTimeOptions.itsTimeFormat != "timestamp") &&
        (itsQueryOptions.itsTimeOptions.itsTimeFormat != "sql") &&
        (itsQueryOptions.itsTimeOptions.itsTimeFormat != "xml") &&
        (itsQueryOptions.itsTimeOptions.itsTimeFormat != "epoch"))
      throw Fmi::Exception(BCP,
                           "Unknown 'timeformat', use 'iso', 'timestamp', 'sql', 'xml' or 'epoch'");

    // Times always in utc
    //
    // itsQueryOptions.itsTimeOptions.itsTimeZone =
    // SmartMet::Spine::optional_string(theRequest.getParameter("tz"),"utc");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor parses query options
 */
// ----------------------------------------------------------------------

Query::Query(const SmartMet::Spine::HTTP::Request &theRequest,
             const SmartMet::Engine::Authentication::Engine *authEngine,
             const std::unique_ptr<Config> &config)
{
  try
  {
    // Parse 'message type' query option

    parseMessageTypeOption(theRequest);

    // Parse 'param' query option

    parseParamOption(theRequest);

    // Parse location related query options

    auto queryLimits = config->getQueryLimits(
        authEngine,
        SmartMet::Spine::optional_string(SmartMet::Spine::FmiApiKey::getFmiApiKey(theRequest), ""));

    // BRAINSTORM-3136; do not allow use of multiple location options
    //
    // When using bbox(es), message query now filters stations with the bbox(es)/maxdistance,
    // not with preselected station id list. If query would use bbox(es) and other location options,
    // the stations matching only the other location options would simply be ignored

    parseLocationOptions(theRequest, false /* queryLimits.getAllowMultipleLocationOptions() */);

    // 'validity' controls whether accepted or rejected messages are returned

    string validity = Fmi::ascii_tolower_copy(
        SmartMet::Spine::optional_string(theRequest.getParameter("validity"), "accepted"));

    if (validity == "accepted")
      itsQueryOptions.itsValidity = SmartMet::Engine::Avi::Accepted;
    else if (validity == "rejected")
      itsQueryOptions.itsValidity = SmartMet::Engine::Avi::Rejected;
    else
      throw Fmi::Exception(BCP, "Unknown 'validity', use 'accepted' or 'rejected'");

    // Parse time related query options

    parseTimeOptions(theRequest, queryLimits.getMaxMessageTimeRangeDays());

    // Message format

    itsQueryOptions.itsMessageFormat = Fmi::ascii_toupper_copy(
        SmartMet::Spine::optional_string(theRequest.getParameter("messageformat"), "TAC"));

    if ((itsQueryOptions.itsMessageFormat != "TAC") &&
        (itsQueryOptions.itsMessageFormat != "IWXXM"))
      throw Fmi::Exception(BCP, "Unknown 'messageformat', use 'TAC' or 'IWXXM'");

    // Format, output precision, debug on/off (whether engine writes generated sql to stderr)

    itsFormat = SmartMet::Spine::optional_string(theRequest.getParameter("format"), "ascii");
    itsPrecision = SmartMet::Spine::optional_unsigned_long(theRequest.getParameter("precision"), 6);

    itsQueryOptions.itsDebug = (itsFormat == "debug");
    if (!itsQueryOptions.itsDebug)
    {
      const auto pos = itsFormat.find("debug");
      itsQueryOptions.itsDebug = (pos != std::string::npos);

      if (itsQueryOptions.itsDebug)
        itsFormat.erase(pos, std::strlen("debug"));
    }

    // Whether to skip duplicate messages

    itsQueryOptions.itsDistinctMessages =
        (SmartMet::Spine::optional_unsigned_long(theRequest.getParameter("distinct"), 1) > 0);

    // Whether to filter (finnish) METARs (LIKE 'METAR%', if enabled by engine's configuration) or
    // not. Option is applicable to TAC output only

    itsQueryOptions.itsFilterMETARs = itsQueryOptions.itsMessageFormat == "TAC"
                                          ? (SmartMet::Spine::optional_unsigned_long(
                                                 theRequest.getParameter("filtermetars"), 1) > 0)
                                          : false;

    // https://jira.fmi.fi/browse/BRAINSTORM-1779
    //
    // Whether to exclude (finnish) SPECIs

    itsQueryOptions.itsExcludeSPECIs =
        (SmartMet::Spine::optional_unsigned_long(theRequest.getParameter("excludespecis"), 0) > 0);

    // Query limits

    itsQueryOptions.itsMaxMessageStations = queryLimits.getMaxMessageStations();
    itsQueryOptions.itsMaxMessageRows = queryLimits.getMaxMessageRows();
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

// ======================================================================
