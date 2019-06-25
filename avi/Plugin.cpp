// ======================================================================
/*!
 * \brief SmartMet avidb plugin implementation
 */
// ======================================================================

#include "Plugin.h"
#include "Query.h"

#include <spine/Convenience.h>
#include <spine/Exception.h>
#include <spine/Reactor.h>
#include <spine/SmartMet.h>
#include <spine/Table.h>
#include <spine/TableFeeder.h>
#include <spine/TableFormatterFactory.h>
#include <spine/ValueFormatter.h>

#include <macgyver/StringConversion.h>
#include <macgyver/TimeZoneFactory.h>

#include <boost/date_time/local_time/local_time.hpp>

#include <iostream>
#include <stdexcept>

using namespace std;
using namespace boost::posix_time;
using namespace boost::local_time;

namespace SmartMet
{
namespace Plugin
{
namespace Avi
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Set column headers using the order the columns were listed in the request
 */
// ----------------------------------------------------------------------

void setColumnHeaders(TableFormatter::Names &headers, const SmartMet::Engine::Avi::Columns &columns)
{
  try
  {
    for (SmartMet::Engine::Avi::Columns::const_iterator it = columns.begin(); (it != columns.end());
         it++)
      headers.push_back(it->itsName);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set column precision
 */
// ----------------------------------------------------------------------

void setPrecisions(size_t nColumns, const Query &query, vector<int> &precisions)
{
  try
  {
    for (size_t n = 0; (n < nColumns); n++)
      precisions.push_back(query.itsPrecision);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

}  // anonymous namespace

// ----------------------------------------------------------------------
/*!
 * \brief Perform an avi query
 */
// ----------------------------------------------------------------------

void Plugin::query(const SmartMet::Spine::HTTP::Request &theRequest,
                   SmartMet::Spine::HTTP::Response &theResponse)
{
  try
  {
    // Parse query options

    Query query(theRequest, itsAuthEngine, itsConfig);

    // Query

    SmartMet::Engine::Avi::StationQueryData stationData;
    SmartMet::Engine::Avi::QueryData rejectedMessageData;

    if (query.itsQueryOptions.itsValidity == SmartMet::Engine::Avi::Accepted)
      stationData = itsAviEngine->queryStationsAndMessages(query.itsQueryOptions);
    else
    {
      if (!query.itsQueryOptions.itsTimeOptions.itsObservationTime.empty())
        throw SmartMet::Spine::Exception(BCP, "Time range must be used to query rejected messages");

      rejectedMessageData = itsAviEngine->queryRejectedMessages(query.itsQueryOptions);
    }

    // Set column headers

    TableFormatter::Names headers;

    setColumnHeaders(headers,
                     (query.itsQueryOptions.itsValidity == SmartMet::Engine::Avi::Accepted)
                         ? stationData.itsColumns
                         : rejectedMessageData.itsColumns);

    // Set column precisions

    vector<int> precisions;

    setPrecisions(headers.size(), query, precisions);

    // Get formatter and timezone for time columns

    boost::optional<boost::local_time::time_zone_ptr> timeZonePtr;

    if ((!query.itsQueryOptions.itsTimeOptions.itsTimeZone.empty()) &&
        (query.itsQueryOptions.itsTimeOptions.itsTimeZone != "utc"))
      timeZonePtr = Fmi::TimeZoneFactory::instance().time_zone_from_string(
          query.itsQueryOptions.itsTimeOptions.itsTimeZone);

    boost::shared_ptr<Fmi::TimeFormatter> timeFormatter(
        Fmi::TimeFormatter::create(query.itsQueryOptions.itsTimeOptions.itsTimeFormat));

    // Fill table

    Table table;
    ValueFormatter valueFormatter(theRequest);
    SmartMet::Spine::TimeSeries::TableFeeder tf(
        table, valueFormatter, precisions, timeFormatter, timeZonePtr);

    int columnNumber = 0;

    if (query.itsQueryOptions.itsValidity == SmartMet::Engine::Avi::Accepted)
    {
      for (const auto &column : stationData.itsColumns)
      {
        tf.setCurrentRow(0);
        tf.setCurrentColumn(columnNumber);

        if (column.itsType == SmartMet::Engine::Avi::ColumnType::TS_LatLon)
          tf << TimeSeries::LonLatFormat::LATLON;
        else if (column.itsType == SmartMet::Engine::Avi::ColumnType::TS_LonLat)
          tf << TimeSeries::LonLatFormat::LONLAT;

        for (auto stationId : stationData.itsStationIds)
          tf << stationData.itsValues[stationId][column.itsName];

        columnNumber++;
      }
    }
    else
    {
      for (const auto &column : rejectedMessageData.itsColumns)
      {
        tf.setCurrentRow(0);
        tf.setCurrentColumn(columnNumber);

        if (column.itsType == SmartMet::Engine::Avi::ColumnType::TS_LatLon)
          tf << TimeSeries::LonLatFormat::LATLON;
        else if (column.itsType == SmartMet::Engine::Avi::ColumnType::TS_LonLat)
          tf << TimeSeries::LonLatFormat::LONLAT;

        tf << rejectedMessageData.itsValues[column.itsName];

        columnNumber++;
      }
    }

    // Formatted output

    ostringstream output;
    boost::shared_ptr<TableFormatter> formatter(TableFormatterFactory::create(query.itsFormat));
    formatter->format(output, table, headers, theRequest, itsConfig->tableFormatterOptions());

    theResponse.setContent(output.str());

    string mime = formatter->mimetype() + "; charset=UTF-8";
    theResponse.setHeader("Content-type", mime.c_str());
    theResponse.setHeader("Access-Control-Allow-Origin", "*");
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Main content handler
 */
// ----------------------------------------------------------------------

void Plugin::requestHandler(Reactor & /* theReactor */,
                            const SmartMet::Spine::HTTP::Request &theRequest,
                            SmartMet::Spine::HTTP::Response &theResponse)
{
  try
  {
    bool isdebug =
        (SmartMet::Spine::optional_string(theRequest.getParameter("format"), "") == "debug");

    try
    {
      const int expires_seconds = 60;
      ptime t_now = second_clock::universal_time();

      query(theRequest, theResponse);
      theResponse.setStatus(HTTP::Status::ok);

      // Build cache expiration time info

      ptime t_expires = t_now + seconds(expires_seconds);

      // The headers themselves

      boost::shared_ptr<Fmi::TimeFormatter> tformat(Fmi::TimeFormatter::create("http"));

      std::string cachecontrol = "public, max-age=" + Fmi::to_string(expires_seconds);
      std::string expiration = tformat->format(t_expires);
      std::string modification = tformat->format(t_now);

      theResponse.setHeader("Cache-Control", cachecontrol.c_str());
      theResponse.setHeader("Expires", expiration.c_str());
      theResponse.setHeader("Last-Modified", modification.c_str());
    }
    catch (...)
    {
      // Catching all exceptions

      SmartMet::Spine::Exception exception(BCP, "Request processing exception!", nullptr);
      exception.addParameter("URI", theRequest.getURI());
      exception.addParameter("ClientIP", theRequest.getClientIP());
      exception.printError();

      if (isdebug)
      {
        // Delivering the exception information as HTTP content

        std::string fullMessage = std::string("Error: ") + exception.getHtmlStackTrace();
        theResponse.setContent(fullMessage);
        theResponse.setStatus(HTTP::Status::ok);
      }
      else
      {
        theResponse.setStatus(HTTP::Status::bad_request);
      }

      // Adding the first exception information into the response header

      std::string firstMessage = exception.what();
      boost::algorithm::replace_all(firstMessage, "\n", " ");
      firstMessage = firstMessage.substr(0, 300);
      theResponse.setHeader("X-Avi-Error", firstMessage.c_str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Plugin constructor
 */
// ----------------------------------------------------------------------

Plugin::Plugin(Reactor *theReactor, const char *theConfigFileName)
    : SmartMetPlugin(),
      itsModuleName("Avi"),
      itsConfigFileName(theConfigFileName),
      itsConfig(),
      itsReactor(theReactor)
{
  try
  {
    if (theReactor->getRequiredAPIVersion() != SMARTMET_API_VERSION)
    {
      std::cerr << "*** AviPlugin and Server SmartMet API version mismatch ***" << std::endl;
      return;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Plugin initialization
 */
// ----------------------------------------------------------------------

void Plugin::init()
{
  try
  {
    /* AviEngine */

    auto engine = itsReactor->getSingleton("Avi", nullptr);
    if (!engine)
      throw SmartMet::Spine::Exception(BCP, "Avi engine unavailable");

    itsAviEngine = reinterpret_cast<SmartMet::Engine::Avi::Engine *>(engine);

    itsConfig.reset(new Config(itsConfigFileName));

    /* AuthenticationEngine */

    if (itsConfig->useAuthentication())
    {
      engine = itsReactor->getSingleton("Authentication", nullptr);

      if (!engine)
        throw SmartMet::Spine::Exception(BCP, "Authentication engine unavailable");

      itsAuthEngine = reinterpret_cast<SmartMet::Engine::Authentication::Engine *>(engine);
    }

    if (!(itsReactor->addContentHandler(
            this, "/avi", boost::bind(&Plugin::callRequestHandler, this, _1, _2, _3))))
      throw SmartMet::Spine::Exception(BCP, "Failed to register avidb content handler");
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Shutdown the plugin
 */
// ----------------------------------------------------------------------

void Plugin::shutdown()
{
  std::cout << "  -- Shutdown requested (aviplugin)\n";
}
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Plugin::~Plugin() {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the plugin name
 */
// ----------------------------------------------------------------------

const std::string &Plugin::getPluginName() const
{
  return itsModuleName;
}
// ----------------------------------------------------------------------
/*!
 * \brief Return the required version
 */
// ----------------------------------------------------------------------

int Plugin::getRequiredAPIVersion() const
{
  return SMARTMET_API_VERSION;
}
// ----------------------------------------------------------------------
/*!
 * \brief Performance query implementation.
 */
// ----------------------------------------------------------------------

bool Plugin::queryIsFast(const SmartMet::Spine::HTTP::Request &) const
{
  return false;
}

}  // namespace Avi
}  // namespace Plugin
}  // namespace SmartMet

/*
 * Server knows us through the 'SmartMetPlugin' virtual interface, which
 * the 'Plugin' class implements.
 */

extern "C" SmartMetPlugin *create(SmartMet::Spine::Reactor *theReactor,
                                  const char *theConfigFileName)
{
  try
  {
    return new SmartMet::Plugin::Avi::Plugin(theReactor, theConfigFileName);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

extern "C" void destroy(SmartMetPlugin *us)
{
  // This will call 'Plugin::~Plugin()' since the destructor is virtual

  delete us;
}

// ======================================================================
