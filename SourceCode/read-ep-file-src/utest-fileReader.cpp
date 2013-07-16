//--- Unit test for fileReader.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for fileReader.cpp.


//--- Includes.
#include <assert.h>

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "fileReader.h"


//--- Main driver.
//
//   Read a file and echo it back out.
//
int main(int argc, const char* argv[]) {
  //
  int lineNo;
  string s;
  const string delimiters = ",;";
  const string commentChars = "!";
  const string illegalChars = "!";  // Note could treat comment char as legal delimiter, if willing to check on return from fr.getToken().
  //
  // Get input file.
  if( 2 != argc ){
    cout << "Error: missing filename\nUsage: " << argv[0] << "  <name of input file to parse>\n";
    return(1);
  }
  fileReader fr(argv[1]);
  //
  // Attach an external reporting function.
  //   Not done for this unit test.
  // fr.attachErrorFcn(reportInputError);
  //
  fr.open();
  //
  cout << "lineNo:token pairs in file " << argv[1] << ":" << endl;
  //
  while( 1 ){
    lineNo = 0;  // Demonstrate that {lineNo} is set by the \c fileReader.
    fr.skipComment(commentChars, lineNo);
    // Get a token-- defined as a string that ends with a delimiter, or with EOF.
    fr.getToken(delimiters, illegalChars, s);
    // Consume the delimiter that caused getToken() to return above.
    if( ! fr.moveForward(1) ){
      // Here, hit EOF.
      break;
    }
    assert( ! fr.isEOF() );
    cout << lineNo << ":" << s << endl;
  }
  //
  // Here, hit EOF.
  assert( fr.isEOF() );
  if( s.length() > 0 ){
    cout << lineNo << ":" << s << " (non-delimited string) ";
  }
  cout << "EOF" << endl;
  fr.close();
  //
  return(0);
}  // End fcn main().
