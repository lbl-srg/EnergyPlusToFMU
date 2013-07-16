//--- Unit test for string-help.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for string-help.cpp.


//--- Includes.
#include <assert.h>
#include <math.h>

#include <string>
using std::string;

#include "string-help.h"


//--- Main driver.
//
int main(int argc, const char* argv[]) {
  //
  string expected, s;
  //
  //-- Test fcn containsChar().
  expected = "asdf";
  assert( containsChar(expected, 'a') );
  assert( containsChar(expected, 's') );
  assert( containsChar(expected, 'd') );
  assert( containsChar(expected, 'f') );
  //
  assert( ! containsChar(expected, 'g') );
  assert( ! containsChar(expected, 'z') );
  //
  //-- Test fcn trimEnd();
  s = "1234  ";
  assert( s.length() == 6 );
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("1234") );
  //
  // Trim an untrimmable string.
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("1234") );
  //
  // Trim a zero-length string.
  s = "";
  assert( s.length() == 0 );
  trimEnd(s);
  assert( s.length() == 0 );
  assert( 0 == s.compare("") );
  //
  // Trim a string with embedded spaces.
  s = "123 567 9   ";
  assert( s.length() == 12 );
  trimEnd(s);
  assert( s.length() == 9 );
  assert( 0 == s.compare("123 567 9") );
  //
  // Trim a string with a tab.
  s = "123\t ";
  assert( s.length() == 5 );
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("123\t") );
  //
  //-- Test fcn strToDbl().
  double dbl;
  //
  s = "1.2";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl-1.2) < 1e-18 );
  //
  s = "-4.6e2";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl+460) < 1e-18 );
  //
  s = "-0.9e-1";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl+0.09) < 1e-18 );
  //
  s = "one";
  assert( ! strToDbl(s,dbl) );
  //
  s = "1.2 y";
  assert( ! strToDbl(s,dbl) );
  //
  //-- Test fcn capitalize().
  s = "a";
  capitalize(s);
  assert( 0 == s.compare("A") );
  //
  s = "abc";
  capitalize(s);
  assert( 0 == s.compare("ABC") );
  //
  s = "aBC";
  capitalize(s);
  assert( 0 == s.compare("ABC") );
  //
  s = "a2b1 c ";
  capitalize(s);
  assert( 0 == s.compare("A2B1 C ") );
  //
  return( 0 );
}  // End fcn main().
