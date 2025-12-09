// ======================================================================
/*!
 * \brief SmartMet avidb plugin implementation
 */
// ======================================================================

#include "Plugin.h"
#include "Query.h"
#include <macgyver/Exception.h>
#include <macgyver/LocalDateTime.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TimeZoneFactory.h>
#include <macgyver/ValueFormatter.h>
#include <spine/Convenience.h>
#include <spine/HostInfo.h>
#include <spine/Reactor.h>
#include <spine/SmartMet.h>
#include <spine/Table.h>
#include <spine/TableFormatterFactory.h>
#include <timeseries/TableFeeder.h>
#include <iostream>

using namespace std;

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
    for (const auto &column : columns)
      headers.push_back(column.itsName);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
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
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
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

    Query query(theRequest, itsAuthEngine.get(), itsConfig);

    // Query

    SmartMet::Engine::Avi::StationQueryData stationData;
    SmartMet::Engine::Avi::QueryData rejectedMessageData;

    if (query.itsQueryOptions.itsValidity == SmartMet::Engine::Avi::Accepted)
      stationData = itsAviEngine->queryStationsAndMessages(query.itsQueryOptions);
    else
    {
      if (!query.itsQueryOptions.itsTimeOptions.itsObservationTime.empty())
        throw Fmi::Exception(BCP, "Time range must be used to query rejected messages");

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

    std::optional<Fmi::TimeZonePtr> timeZonePtr;

    if ((!query.itsQueryOptions.itsTimeOptions.itsTimeZone.empty()) &&
        (query.itsQueryOptions.itsTimeOptions.itsTimeZone != "utc"))
      timeZonePtr = Fmi::TimeZoneFactory::instance().time_zone_from_string(
          query.itsQueryOptions.itsTimeOptions.itsTimeZone);

    std::shared_ptr<Fmi::TimeFormatter> timeFormatter(
        Fmi::TimeFormatter::create(query.itsQueryOptions.itsTimeOptions.itsTimeFormat));

    // Fill table

    Table table;
    Fmi::ValueFormatterParam opt;
    Fmi::ValueFormatter valueFormatter(opt);
    TimeSeries::TableFeeder tf(table, valueFormatter, precisions, timeFormatter, timeZonePtr);

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

    std::shared_ptr<TableFormatter> formatter(TableFormatterFactory::create(query.itsFormat));
    auto out = formatter->format(table, headers, theRequest, itsConfig->tableFormatterOptions());

    theResponse.setContent(out);

    string mime = formatter->mimetype() + "; charset=UTF-8";
    theResponse.setHeader("Content-type", mime);
    theResponse.setHeader("Access-Control-Allow-Origin", "*");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
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
      Fmi::DateTime t_now = Fmi::SecondClock::universal_time();

      query(theRequest, theResponse);
      theResponse.setStatus(HTTP::Status::ok);

      // Build cache expiration time info

      Fmi::DateTime t_expires = t_now + Fmi::Seconds(expires_seconds);

      // The headers themselves

      std::shared_ptr<Fmi::TimeFormatter> tformat(Fmi::TimeFormatter::create("http"));

      std::string cachecontrol = "public, max-age=" + Fmi::to_string(expires_seconds);
      std::string expiration = tformat->format(t_expires);
      std::string modification = tformat->format(t_now);

      theResponse.setHeader("Cache-Control", cachecontrol);
      theResponse.setHeader("Expires", expiration);
      theResponse.setHeader("Last-Modified", modification);
    }
    catch (...)
    {
      // Catching all exceptions

      Fmi::Exception exception(BCP, "Request processing exception!", nullptr);
      exception.addParameter("URI", theRequest.getURI());
      exception.addParameter("ClientIP", theRequest.getClientIP());
      exception.addParameter("HostName", Spine::HostInfo::getHostName(theRequest.getClientIP()));
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
      theResponse.setHeader("X-Avi-Error", firstMessage);
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Plugin constructor
 */
// ----------------------------------------------------------------------

Plugin::Plugin(Reactor *theReactor, const char *theConfigFileName)
    : itsModuleName("Avi"), itsConfigFileName(theConfigFileName), itsReactor(theReactor)
{
  try
  {
    if (theReactor->getRequiredAPIVersion() != SMARTMET_API_VERSION)
    {
      std::cerr << "*** AviPlugin and Server SmartMet API version mismatch ***\n";
      return;
    }
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Plugin initialization
 */
// ----------------------------------------------------------------------

void Plugin::init()
{
  using namespace boost::placeholders;

  try
  {
    /* AviEngine */

    itsAviEngine = itsReactor->getEngine<SmartMet::Engine::Avi::Engine>("Avi", nullptr);

    itsConfig.reset(new Config(itsConfigFileName));

    /* AuthenticationEngine */

    if (itsConfig->useAuthentication())
    {
      itsAuthEngine = itsReactor->getEngine<SmartMet::Engine::Authentication::Engine>(
          "Authentication", nullptr);

      if (!itsAuthEngine->isEnabled())
        throw Fmi::Exception(BCP, "Authentication engine is disabled");
    }

    if (!(itsReactor->addContentHandler(
            this, "/avi", boost::bind(&Plugin::callRequestHandler, this, _1, _2, _3))))
      throw Fmi::Exception(BCP, "Failed to register avidb content handler");
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
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

bool Plugin::queryIsFast(const SmartMet::Spine::HTTP::Request & /* theRequest */) const
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
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

extern "C" void destroy(SmartMetPlugin *us)
{
  // This will call 'Plugin::~Plugin()' since the destructor is virtual

  delete us;
}

// ======================================================================
