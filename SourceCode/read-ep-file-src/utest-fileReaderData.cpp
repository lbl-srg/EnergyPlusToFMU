//--- Unit test for fileReaderData.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for fileReaderData.cpp.


//--- Includes.
#include <assert.h>

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include "fileReaderData.h"

#include "ep-idd-map.h"
#include "fileReaderDictionary.h"

#include "../utility-src/string-help.h"


//--- Main driver.
//
//   Read an IDF file and echo its contents.
//
int main(int argc, const char* argv[]) {
  //
  int lineNo, ct, idx;
  string idfKey, iddDesc;
  vString strVals;
  vDouble dblVals;
  //
  // Check arguments.
  if( 3 != argc ){
    cout << "Error: missing filename\nUsage: " << argv[0] << "  <name of IDD file to guide parsing>  <name of IDF file to parse>\n";
    return(1);
  }
  //
  //-- Set up data dictionary.
  fileReaderDictionary frIdd(argv[1]);
  frIdd.open();
  iddMap idd;
  frIdd.getMap(idd);
  //
  // Echo contents of IDD file.
  iddMap::const_iterator it;
  cout << "(Keyword-->Descriptor) pairs in IDD file " << argv[1] << ":" << endl;
  for( it=idd.begin(); it!=idd.end(); ++it ){
    cout << "(" << it->first << "-->" << it->second << ")" << endl;
  }
  //
  //-- Prepare input data file.
  fileReaderData frIdf(argv[2], IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
  //
  // Attach an external reporting function.
  //   Not done for this unit test.
  // frIdf.attachErrorFcn(reportInputError);
  //
  //-- Pass #1 through IDF file: print contents (keys and values).
  //   Tests method getValues().
  //
  frIdf.open();
  //
  cout << endl << "Keyword--strings--numbers** entries in IDF file" << argv[2] << ":" << endl;
  //
  // Run through the IDF file.
  while( 1 ){
    // Here, assume looking for next keyword.
    frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
    frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, idfKey);
    // Consume the delimiter that caused getToken() to return.
    if( ! frIdf.moveForward(1) ){
      // Here, hit EOF.
      //   OK to hit EOF, provided don't actually have a keyword.
      if( 0 != idfKey.length() ){
        cout << "Error: IDF file ends after keyword '" << idfKey << "' on line " << frIdf.getLineNumber() << endl;
      }
      break;
    }
    assert( ! frIdf.isEOF() );
    // Get descriptor for this keyword.
    //   Note keywords in {idd} are capitalized.
    capitalize(idfKey);
    if( ! iddMap_getDescriptor(idd, idfKey, iddDesc) ){
      cout << "Error: IDF file contains unknown keyword '" << idfKey << "' on line " << frIdf.getLineNumber() << endl;
      break;
    }
    // Get IDF entry, reading according to the descriptor.
    if( ! frIdf.getValues(iddDesc, strVals, dblVals) ){
      cout << "Error encountered while reading values for keyword '" << idfKey << "'" << endl;
      break;
    }
    // Report out.
    cout << idfKey << "\n  --\n";
    ct = (int)strVals.size();
    for( idx=0; idx<ct; ++idx ){
      cout << "  " << strVals[idx] << endl;
    }
    cout << "  --\n";
    ct = (int)dblVals.size();
    for( idx=0; idx<ct; ++idx ){
      cout << "  " << dblVals[idx] << endl;
    }
    cout << "  **\n";
    // Here, ready to look for next keyword.
  }
  // Here, ran through whole IDF file, or stopped due to error.
  //
  frIdf.close();
  //
  //-- Pass #2 through IDF file: print just keys.
  //   Tests method skipSection().
  //
  frIdf.open();
  //
  cout << endl << "Keywords in IDF file" << argv[2] << ":" << endl;
  //
  // Run through the IDF file.
  while( 1 )
    {
    // Here, assume looking for next keyword.
    frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
    frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, idfKey);
    if( frIdf.isEOF() )
      {
      break;
      }
    // Report keyword.
    cout << "  " << idfKey << endl;
    // Skip contents of this key.
    if( frIdf.skipSection() )
      {
      // Here, hit EOF.
      break;
      }
    // Here, ready to look for next keyword.
    }
  // Here, ran through whole IDF file.
  //
  frIdf.close();
  //
  return(0);
}  // End fcn main().
