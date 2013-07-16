//--- Unit test for fileReaderDictionary.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for fileReaderDictionary.cpp.


//--- Includes.
#include <assert.h>

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "fileReaderDictionary.h"


//--- Main driver.
//
//   Read an IDD file and echo its contents.
//
int main(int argc, const char* argv[]) {
  //
  // Get input file.
  if( 2 != argc ){
    cout << "Error: missing filename\nUsage: " << argv[0] << "  <name of IDD file to parse>\n";
    return(1);
  }
  fileReaderDictionary frIdd(argv[1]);
  //
  // Attach an external reporting function.
  //   Not done for this unit test.
  // frIdd.attachErrorFcn(reportInputError);
  //
  frIdd.open();
  //
  // Read contents into a map.
  iddMap idd;
  frIdd.getMap(idd);
  //
  // Echo contents.
  iddMap::const_iterator it;
  cout << "(Keyword-->Descriptor) pairs in IDD file " << argv[1] << ":" << endl;
  for( it=idd.begin(); it!=idd.end(); ++it ){
    cout << "(" << it->first << "-->" << it->second << ")" << endl;
  }
  //
  frIdd.close();
  //
  return(0);
}  // End fcn main().
