//--- File management utilities.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  File management utilities.


#if !defined(__FILE_HELP_H__)
#define __FILE_HELP_H__


//--- Includes.
//
#include <fstream>


//--- Identify file basename.
//
//   Return index, in {fileName}, of character one past the last path separator
// (i.e., '\' or '/') in the string.
//   This allows, for example:
// const char *const fileBaseName = fileName + findFileBaseNameIdx(fileName);
//
//   Examples:
// ** {C:\Doc\file.txt} ==> 7.
// ** {../more.doc} ==> 3.
// ** {this.idf} ==> 0.
int findFileBaseNameIdx(const char *fileName);


//--- Open an output file.
//
// \return TRUE on successfully opening the file.
// \param mode Bitwise "or" of one or more of the following flags:
// ** std::ios::app, append.
// ** std::ios::ate, at end.
// ** std::ios::binary, binary.
// ** std::ios::in, input.
// ** std::ios::out, output.
// ** std::ios::trunc, truncate.
// \param errStr, contains error message in case fcn fails.
//
bool openOutputFile(std::ofstream& fileStream, const char *const fileName, std::ios_base::openmode mode,
  std::string& errStr);


#endif // __FILE_HELP_H__
