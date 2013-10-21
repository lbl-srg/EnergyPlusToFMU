//--- File management utilities.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include "file-help.h"


//--- Identify file basename.
//
//   Note not under unit test.
//
int findFileBaseNameIdx(const char *fileName)
  {
  int idx, baseNameIdx;
  //
  idx = 0;
  baseNameIdx = 0;
  for( idx=0; 1; ++idx, ++fileName )
    {
    const char testChar = *fileName;
    if( '\0' == testChar )
      break;
    if( '\\'==testChar || '/'==testChar )
      baseNameIdx = idx + 1;
    }
  //
  return( baseNameIdx );
  }  // End fcn findFileBaseNameIdx().


//--- Open an output file.
//
//   Note not under unit test.
//
bool openOutputFile(std::ofstream& fileStream, const char *const fileName, std::ios_base::openmode mode,
  std::string& errStr)
  {
  //
  // Start with a clean slate.
  if( fileStream.is_open() )
    fileStream.close();
  fileStream.clear(std::ios_base::goodbit);
  //
  // Open file.
  fileStream.open(fileName, mode);
  if( fileStream.fail() )
    {
    if( 0 != errStr.size() )
      errStr.push_back('\n');
    errStr.append("Failed to open file '").append(fileName).push_back('\'');
    return( 0 );
    }
  //
  return( 1 );
  }  // End fcn openOutputFile().
