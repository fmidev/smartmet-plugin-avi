// ======================================================================

#pragma once

class QueryLimits
{
 public:
  QueryLimits(int maxMessageStations = -1,
              int maxMessages = -1,
              int maxMessageTimeRangeDays = -1,
              bool allowMultipleLocationOptions = false)
      : itsMaxMessageStations(maxMessageStations),
        itsMaxMessages(maxMessages),
        itsMaxMessageTimeRangeDays(maxMessageTimeRangeDays),
        itsAllowMultipleLocationOptions(allowMultipleLocationOptions)
  {
  }

  void setMaxMessageStations(int maxMessageStations) { itsMaxMessageStations = maxMessageStations; }
  int getMaxMessageStations() const { return itsMaxMessageStations; }
  void setMaxMessageRows(int maxMessageRows) { itsMaxMessages = maxMessageRows; }
  int getMaxMessageRows() const { return itsMaxMessages; }
  void setMaxMessageTimeRangeDays(int maxMessageTimeRangeDays)
  {
    itsMaxMessageTimeRangeDays = maxMessageTimeRangeDays;
  }
  int getMaxMessageTimeRangeDays() const { return itsMaxMessageTimeRangeDays; }
  void setAllowMultipleLocationOptions(bool allowMultipleLocationOptions)
  {
    itsAllowMultipleLocationOptions = allowMultipleLocationOptions;
  }
  bool getAllowMultipleLocationOptions() const { return itsAllowMultipleLocationOptions; }
 private:
  int itsMaxMessageStations;
  int itsMaxMessages;
  int itsMaxMessageTimeRangeDays;
  bool itsAllowMultipleLocationOptions;
};
