//--- Utilities for time calculations.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <stdio.h>
#include <time.h>

#include "time-help.h"


//--- Microsoft doesn't implement the modern standard.
//
#ifdef _MSC_VER
  #define snprintf sprintf_s
#endif


//--- Get current date-time, UTC time.
//
void getCurrTimeUTC(char currTimeUTC[21])
  {
  //
  // Get current time.
  time_t currTime;
  time(&currTime);
  //
  // Find components, in UTC.
  const struct tm *const utcTime = gmtime(&currTime);
  //
  // Write to string.
  snprintf(currTimeUTC, 21, "%04i-%02i-%02iT%02i:%02i:%02iZ",
    1900+utcTime->tm_year, 1+utcTime->tm_mon, utcTime->tm_mday,
    utcTime->tm_hour, utcTime->tm_min,
    (utcTime->tm_sec<61 ? utcTime->tm_sec : 60));
  //
  }  // End fcn getCurrTimeUTC().
