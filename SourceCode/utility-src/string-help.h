//--- String utilities.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  String utilities.


#if !defined(__STRING_HELP_H__)
#define __STRING_HELP_H__


//--- Includes.
//
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>


/// Check whether a character is among several expected possibilities.
///
/// \param expected String containing all valid values of \c received.
/// \param received The received character.
/// \return \c true if \c received is contained in \c expected, \c false otherwise.
///
extern bool containsChar(const std::string& expected, const char received);


/// Remove trailing spaces from a string.
///
/// \param str String from which trailing spaces are to be removed.
///
extern void trimEnd(std::string& str);


/// Read a double value out of a string.
///
inline bool strToDbl(const std::string& str, double& dbl){
  char* pEnd;
  const char* pBeg = str.c_str();
  dbl = strtod(pBeg  ,&pEnd);
  return ( (size_t)(pEnd-pBeg)==strlen(pBeg) && (errno!=ERANGE) );
}


/// Capitalize a string.
///
extern void capitalize(std::string& str);


#endif // __STRING_HELP_H__
