//--- Utilities for time calculations.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  Utilities for time calculations.



#if !defined(__TIME_HELP_H__)
#define __TIME_HELP_H__


//--- Get current date-time, UTC.
//
//   Expected format:
// ** {YYYY-MM-DDThh:mm:ssZ}
// ** Note one "T" between date and time.
// ** Note "Z" characterizes the Zulu time zone, i.e., GMT time zone).
// ** Example: "2009-12-08T14:33:22Z".
//
// \param currTimeUTC Character array with enough space for time string,
// including terminating null character.
//
extern void getCurrTimeUTC(char currTimeUTC[21]);


#endif // __TIME_HELP_H__
