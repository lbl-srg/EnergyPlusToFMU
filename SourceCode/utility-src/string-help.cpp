//--- String utilities.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <algorithm>

#include "string-help.h"


//--- Check whether a character is among several expected possibilities.
//
bool containsChar(const std::string& expected, const char received){
  // Test whether got an expected character.
  return( expected.find(received) != std::string::npos );
}  // End fcn containsChar().


//--- Remove trailing spaces from a string.
//
void trimEnd(std::string& str){
  const int strLen = str.length();
  int iEnd = strLen;
  //
  // Identify trailing spaces.
  for( int i=strLen-1; i>0; i-- ){
    if( str[i] == ' ' )
      iEnd = i;
    else
      break;
  }
  //
  // Remove trailing spaces if necessary.
  if( iEnd <= (strLen-1) ){
    str = str.substr(0, iEnd);
  }
  //
  return;
}  // End fcn trimEnd().


//--- Capitalize a string.
//
void capitalize(std::string& str)
  {
  transform(str.begin(), str.end(), str.begin(), toupper);
  }  // End fcn capitalize().
