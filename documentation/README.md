This is the API reference of avi plugin.

# Response Parameters

## Parameters

`param` option is used for selecting the columns to be extracted from aviation message database.

```
param=name1,name2,...,nameN
```

The available parameters/columns are listed below. Time related columns are formatted as selected with option 'timeformat' (see Time formatting). Floating point values are presented with number of decimal digits set with option 'precision' (see Decimal Precision).

### Locations Related Parameters (avidb_stations table)

```
stationid: station id
icao: icao code of the station
name: station name
latitude : latitude of the station
longitude : longitude of the station
lonlat : longitude and latitude of the station
latlon : latitude and longitude of the station
distance: distance of the station from requested location (not available for areas and paths; NaN is returned)
bearing: bearing to the station from requested location  (not available for areas and paths; NaN is returned)
elevation : elevation of the station in meters
stationvalidfrom: start of the station valid time range
stationvalidto: end of the station valid time range
stationmodified: time stamp when station information was last modified
iso2 : country 2-character isocode
```

### Message Types Related Parameters (avidb_message_types table)

```
messagetype: message type abbreviation
messagetypedescription: message type description
messagetypemodified: time stamp when message type information was last modified
```

### Message Sources Related Parameters (avidb_message_routes table)

```
route: name of the route a.k.a message source
routedescription: route description
routemodified: time stamp when route information was last modified
```

### Messages Related Parameters (avidb_messages table)

```
messageid: id of the message
message: the content of the message
messagetime: time stamp in the message
messagevalidfrom: start of the message valid time range
messagevalidto: end of the message valid time range
messagecreated: when message is saved in avidb
messagefilemodified: time when message information was last modified
messirheading: messir heading
messageversion: message version (i.e. CCA, CCB, RRA...)
```

### Rejected Messages Related Parameters (avidb_rejected_messages table; validity=rejected)

Note that rejected messages selection is always made using time range and based on row creation time.

In addition to columns listed below, rejected messages have columns `message, messagetime, messagevalidfrom, messagevalidto, messagecreated, messagefilemodified, messirheading and messageversion`; see Messages Related Parameters above.

```
messagerejectedreason: information why message is rejected (1: Unknown ICAO code, 2: Message time stamp in the future)
messagerejectedicao: ICAO code of the rejected message
```

# Query Parameters

## Locations

### ICAO Codes

If any of the specified codes is unknown, an error message will be returned.

```
icao=icao1&icao=icao2&...
icaos=icao1,icao2,icao3,...
```

### Named Locations

AVI plugin recognises location names from avidb_stations table. If any of the specified locations is unknown, an error message will be returned.

```
place=name1&place=name2&...
places=name1,name2,name3,...
```

Note that `places` option will not work correctly if the place names contain commas!

### Geographic Coordinates

Station search with coordinate will be done with given maxdistance (see Distance Limit). Coordinates are given in decimal format, such as 25.02 where 02 is hundreds of a degree.

```
lonlat=lon1,lat1&lonlat=lon2,lat2&...
latlon=lat1,lon1&latlon=lat2,lon2&...
lonlats=lon1,lat1,lon2,lat2&
latlons=lat1,lon1,lat2,lon2&
```

### Station Database Identifiers

Identifiers are given as integers. If any of the specified id's is unknown, an error message will be returned.

```
stationid=id1&stationid=id2&...
stationids=id1,id2,id3,...
```

### BBOX

Station search with bounding box will be done with given maxdistance (see Distance Limit). Coordinates are in order lllon, lllat, urlon, urlat.

```
bbox=24,60,25,61&bbox=25,61,26,62&...
```

### WKT

Well Known Text is a standard representation for location recognised by PostGIS.

Only POINT, LINESTRING and POLYGON wkts are allowed. Single LINESTRING is taken as a route for which messages are returned in the order of route segments and station's distance from the first near enough route segment's starting point. Station search with wkt will be done with given maxdistance (see Distance Limit).

```
WKT=POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))&
```

### ISO country code

iso2 country code. If any of the specified codes is unknown, an error message will be returned.

```
country=FI&
countries=FI,SE&
```

### Distance Limit

Distance limit in kilometers for searching stations applies to and is mandatory with coordinate, bbox and wkt queries.

```
maxdistance=50&
```

### Number of Nearest Locations

One can also control how many nearest stations at maximum is selected with coordinate (including POINT wkt) queries.The default is 1.

```
numberofstations=3&
```

### Choosing message type

The default is all available message types. If any of the specified types is unknown, an error message will be returned.

```
messagetype=TAF,METAR,GAFOR&
```

Available types (to be revised):

- METAR/SPECI
- AWSMETAR
- TAF
- GAFOR
- METREP/SPECIAL
- WRNG
- WXREP
- ARS
- SIGMET
- VA-SIGMET
- VAA

Coming

- Säteily-SIGMET

**NOTE:** AWS-METAR - stations are handled as a special case. When querying nearest stations and message type other than AWSMETAR, AWS-stations are not considered in the result set. If querying only station metadata, AWS stations are considered. This is due to the lack of station metadata information on issued messages.

### Duplicate Message Filtering

Duplicate messages are filtered by default, thus only one copy of the same message originating from different routes is returned unless route table colums are also selected. Filtering can be omitted with zero valued 'distinct' option

```
distinct=0&
```

### Finnish METAR Filtering

By default finnish METARs are returned only when they start with "METAR" (METARs from some stations get stored with and without "METAR" in the beginning of message). Filtering can be omitted with zero valued 'filtermetars' option

```
filtermetars=0&
```

## Time

There are four basic methods of selecting data with timestamps:

- **requesting only latest valid messages (default)**
- requesting only latest valid messages from certain observation time (same as #1 looked from certain time stamp)
- using a specific time range for valid messages
- using a specific time range for created messages

Longest possible time range is 31 days (configured in plugin's configuration). Messages whose valid time range intersects with requested time range are returned.

Note: given time range is taken as a half open range where starttime <= time < endtime; thus query with time range t1-t2 does not return messages which are valid
startting from t2 (or created at t2).

### Input Time Formats

The option handler can automatically recognize several time formats:

- epoch: unix epoch time
- iso: The format is YYYYMMDDTHHMISS where letter T separates the date and time of day parts
- timestamp: The format is YYYYMMDDHHMI
- sql: The format is YYYY-MM-DD HH:MI:SS
- xml: The format is YYYY-MM-DDTHH:MI:SSZ (iso-extended format)
- offsets: The format is +-NNNNx, where x is an optional units specifier m, h or d. The sign is compulsory.
- zero offset: The value 0 is interpreted to mean now

### Time Range (valid messages)

```
starttime=201512021000&endtime=201512021200&
```

All valid messages within requested time range are returned; e.g. `2015-12-02T12:05:00Z` amended TAF would not be returned even if it is valid within requested time range

### Time Range (created messages)

```
starttime=201512021000&endtime=201512021200&validrangemessages=0
```

All messages created within requested time range are returned. Time restriction is based on message_time column.

### Observation Time

```
time=201512021200&
```

Default is current time. Latest valid messages at requested time are returned; e.g. `2015-12-02T12:05:00Z` amended TAF would not be returned even if it is valid at requested moment

### Validity

Controls whether querying accepted or rejected messages. Default validity value is accepted.

```
validity=accepted|rejected&
```

Note: time range must be used to query rejected messages

# Output Formatting

## Missing Values

Missing values (NULLs) are returned as given value. The default value for the option is "nan". The option is relevant with other output formats except "json", for which "null" is used.

```
missingtext=null&
```

## Time formatting

Time columns are formatted according to the selected format. The default format is `iso`.

```
timeformat=iso|timestamp|sql|epoch&
```

- iso: The format is YYYYMMDDTHHMISS where letter T separates the date and time of day parts
- timestamp: The format is YYYYMMDDHHMI
- sql: The format is YYYY-MM-DD HH:MI:SS
- xml: The format is YYYY-MM-DDTHH:MI:SSZ (iso-extended format)
- epoch: unix epoch time

## Decimal Precision

Number of decimal digits for floating point values is set with option 'decimals'. The default precision is 6.

```
decimals=3&
```

## Data output format

```
format=ascii|debug|serial|json&
```

The default format is `ascii`.

### ASCII

ASCII output can be formatted using 'separator' option, which sets the separator between the columns. Typical values include space (default), tabulator and comma. Option is relevant only for ascii output.

```
separator=,&
```

### DEBUG

The output is intended to be viewed directly with a browser.

### SERIAL

The output can be read with PHP function unserialize

### JSON

JSON format output .

## Error Handling

The plugin will return a `204 No Content` response in all formats except the debug format. Please note that the body of 204 responses is always empty.

In debug format the response is `200 OK`, and the message body consists of the error message.

A HTTP header `X-AVIPlugin-Error` will be returned in all formats. The value of the header is the error message truncated to 100 characters.

# Configuration Files

TODO: The configuration files probably shouldn't be here.

## Plugin configuration

## Engine configuration

# Regression Test Requests

TBA
