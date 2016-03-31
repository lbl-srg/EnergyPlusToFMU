///////////////////////////////////////////////////////
/// \file   fileReader.h
/// \author Michael Wetter,
///         LBNL, Simulation Research Group,
///         MWetter@lbl.gov
/// \date   2001-06-19
///
/// \brief  Virtual file reader for input parameter and weather
///         data.
///
///////////////////////////////////////////////////////
#if !defined(__FILEREADER_H__)
#define __FILEREADER_H__

#include <fstream>
#include <string>


///////////////////////////////////////////////////////
/// File reader for input parameter and weather  data.
class fileReader {

public:
  /// Constructor. Does NOT open the file.
  /// \param fname Filename
  fileReader(const std::string& fname);

  ~fileReader() { close(); }

  /// Attach an error-reporting function.
  /// \param errFcn Pointer to function to be called in case of a \c fileReader error.
  void attachErrorFcn(void (*errFcn)(
    std::ostringstream& errorMessage, const std::string& fileName, int lineNo));

  /// Gets the current line.
  /// \retval str String where the current line will be stored.
  /// \retval lineNo Integer where the current line number will be stored.
  void getLine(std::string& str, int &lineNo);

  /// Gets the current character and moves stream pointer one position.
  /// \return Character.  For end-of-file, returns '\0'.
  char getChar(void);

  /// Skips all space characters until next non-space character.
  /// \retval lineNo Line number (after skipping all space characters).
  void skipSpace(int& lineNo);

  /// Skips to the end of the current line.
  /// \param noOfLines Number of times that the current line has to be skipped.
  /// \retval lineNo Line number (after skipping the current line).
  void skipLine(const int noOfLines, int& lineNo);

  /// Skips to the end of the current line.
  /// \retval lineNo Line number (after skipping the current line).
  void skipLine(int& lineNo);

  /// Skips all comments (and spaces) starting from the
  ///  current buffer position.
  ///
  /// After execution, the pointer
  /// of the \c ifstream points to the first character that is neither
  ///  a comment nor a space.
  /// \param commentSign String containing all characters that indicate the
  ///       begin of a line comment.
  /// \retval lineNo Line number.
  void skipComment(const std::string& commentSign, int& lineNo);

  /// Moves pointer of \c ifstream forward.
  /// \param skipCharCt Number of characters to move forward.
  /// \return \c false if moving the pointer is not possible due to end of file,
  ///         \c true otherwise.
  bool moveForward(int skipCharCt);

  /// Gets the current token.
  /// \param delimiters Characters that mark end of token.
  /// \param illegalChars Characters that should never appear.  If encountered, reports and dies.
  /// \retval token String where the current token is stored.
  // hoho dml  Why not fetch {lineNo} here, as do with other methods?  Seems like caller might
  //   like to know, e.g., for error reporting.
  // hoho dml  Caller has no direct way to determine whether or not got a token.  If hit EOF, or if hit
  //   delimiter with no actual token, returns without indication something out-of-ordinary happened.
  void getToken(const std::string& delimiters, const std::string& illegalChars, std::string& token);

  /// Gets the current token.
  /// \param delimiters Characters that mark end of token.
  /// \param illegalChars Characters that should never appear.  If encountered, reports and dies.
  /// \retval token String where the current token is stored.
  /// \retval token String where the current token with delimiter is stored.
  // hoho dml  Why not fetch {lineNo} here, as do with other methods?  Seems like caller might
  //   like to know, e.g., for error reporting.
  // hoho dml  Caller has no direct way to determine whether or not got a token.  If hit EOF, or if hit
  //   delimiter with no actual token, returns without indication something out-of-ordinary happened.
  void getToken(const std::string& delimiters, const std::string& illegalChars, std::string& token, std::string& tokenExt);

  /// Gets the current line number.
  /// \retval The current line number.
  inline int getLineNumber(){ return lineNumber; }

  /// Opens the file, writes an error message if file cannot be opened.
  void open();

  /// Closes the file.
  void close();

  /// Check for end-of-file.
  bool isEOF(){ return fileStream.eof(); }

protected:

  //--- Protected member data.
  std::string fileName;
  std::ifstream fileStream;
  int lineNumber;
  void (*externalErrorFcn)(std::ostringstream& errorMessage, const std::string& fileName, int lineNo);

  //--- Protected methods.
  void reportError(std::ostringstream& errorMessage);

private:
  fileReader();
};


#endif // __FILEREADER_H__


/*
***********************************************************************************
Copyright Notice
----------------

Functional Mock-up Unit Export of EnergyPlus (C)2013, The Regents of
the University of California, through Lawrence Berkeley National
Laboratory (subject to receipt of any required approvals from
the U.S. Department of Energy). All rights reserved.

If you have questions about your rights to use or distribute this software,
please contact Berkeley Lab's Technology Transfer Department at
TTD@lbl.gov.referring to "Functional Mock-up Unit Export
of EnergyPlus (LBNL Ref 2013-088)".

NOTICE: This software was produced by The Regents of the
University of California under Contract No. DE-AC02-05CH11231
with the Department of Energy.
For 5 years from November 1, 2012, the Government is granted for itself
and others acting on its behalf a nonexclusive, paid-up, irrevocable
worldwide license in this data to reproduce, prepare derivative works,
and perform publicly and display publicly, by or on behalf of the Government.
There is provision for the possible extension of the term of this license.
Subsequent to that period or any extension granted, the Government is granted
for itself and others acting on its behalf a nonexclusive, paid-up, irrevocable
worldwide license in this data to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so. The specific term of the license can be identified
by inquiry made to Lawrence Berkeley National Laboratory or DOE. Neither
the United States nor the United States Department of Energy, nor any of their employees,
makes any warranty, express or implied, or assumes any legal liability or responsibility
for the accuracy, completeness, or usefulness of any data, apparatus, product,
or process disclosed, or represents that its use would not infringe privately owned rights.


Copyright (c) 2013, The Regents of the University of California, Department
of Energy contract-operators of the Lawrence Berkeley National Laboratory.
All rights reserved.

1. Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

(1) Redistributions of source code must retain the copyright notice, this list
of conditions and the following disclaimer.

(2) Redistributions in binary form must reproduce the copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other
materials provided with the distribution.

(3) Neither the name of the University of California, Lawrence Berkeley
National Laboratory, U.S. Dept. of Energy nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

3. You are under no obligation whatsoever to provide any bug fixes, patches,
or upgrades to the features, functionality or performance of the source code
("Enhancements") to anyone; however, if you choose to make your Enhancements
available either publicly, or directly to Lawrence Berkeley National Laboratory,
without imposing a separate written license agreement for such Enhancements,
then you hereby grant the following license: a non-exclusive, royalty-free
perpetual license to install, use, modify, prepare derivative works, incorporate
into other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

NOTE: This license corresponds to the "revised BSD" or "3-clause BSD"
License and includes the following modification: Paragraph 3. has been added.


***********************************************************************************
*/
