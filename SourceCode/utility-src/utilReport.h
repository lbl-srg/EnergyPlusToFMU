//--- Reporting functions.
//
///////////////////////////////////////////////////////
/// \file   utilReport.h
/// \author Michael Wetter,
///         LBNL, Simulation Research Group,
///         MWetter@lbl.gov
/// \date   2002-07-05
///
/// \brief  Reporting functions.
///
///////////////////////////////////////////////////////
#if !defined(__UTILREPORT_H__)
#define __UTILREPORT_H__

#include <string>
#include <sstream>


/// Reports a program error. This method does NOT exit the program.
/// \param functionName Name of function where error occured.
/// \param errorMessage Error message.
extern void reportProgramError(const std::string& functionName, 
             const std::string& errorMessage);

/// Reports a program error. This method does NOT exit the program.
/// \param functionName Name of function where error occured.
/// \param errorMessage Error message.
extern void reportProgramError(const std::string& functionName, 
             std::ostringstream& errorMessage);

/// Reports a run-time error. This method does NOT exit the program.
/// \param functionName Name of function where error occured.
/// \param errorMessage Error message.
extern void reportRuntimeError(const std::string& functionName, 
             std::ostringstream& errorMessage);

/// Reports a run-time error. This method does NOT exit the program.
/// \param errorMessage Error message.
extern void reportRuntimeError(std::ostringstream& errorMessage);

/// Reports a run-time warning. This method does NOT exit the program.
/// \param warningMessage Warning message.
extern void reportRuntimeWarning(std::ostringstream& warningMessage);

/// Reports a run-time info.
/// \param infoMessage Info message.
extern void reportRuntimeInfo(std::ostringstream& infoMessage);

/// Reports an error. This method does NOT exit the program.
/// \param ErrorMessage error message.
extern void reportError(std::ostringstream& errorMessage);

/// Reports an error. This method does NOT exit the program.
/// \param errorMessage Error message.
extern void reportError(const std::string& errorMessage);

/// Reports an input error. This method does NOT exit the program.
/// \param errorMessage Error message to be appended to file name and line number.
/// \param fileName File name where input error occured.
/// \param lineNo Line number where input error occured.
extern void reportInputError(std::ostringstream& errorMessage, 
           const std::string& fileName, 
           const int lineNo);

/// Reports an input error. This method does NOT exit the program.
/// \param FunctionName name of function where error has been detected.
/// \param ErrorMessage error message.
extern void reportInputError(const std::string& functionName, std::ostringstream& errorMessage);

/// Reports an input error. This method does NOT exit the program.
/// \param functionName Name of function where error has been detected.
/// \param errorMessage Error message.
extern void reportInputError(std::ostringstream& errorMessage);

/// Flushs the error message to \c cerr and to the log file.
/// \param errorMessage Error message.
extern void flushLogStream(std::ostringstream& errorMessage);

/// Writes the header of the log file.
extern void writeLogHeader();
/// Print optional string \c optStr followed by
///  all elements of the collection \c coll
template <class T> void PRINT_ELEMENTS(const T& coll, 
               const std::string& optStr=""){
  typename T::const_iterator pos;
  
  std::cout << optStr;
  for (pos = coll.begin(); pos != coll.end(); ++pos){
    std::cout << *pos << ' ';
  }
  std::cout << std::endl;
}

#endif // __UTILREPORT_H__

/* COPYRIGHT NOTICE

BuildOpt Copyright (c) 2010, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy). All rights reserved.

NOTICE.  This software was developed under partial funding from the U.S.
Department of Energy.  As such, the U.S. Government has been granted for
itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable, worldwide license in the Software to reproduce, prepare
derivative works, and perform publicly and display publicly.  Beginning
five (5) years after the date permission to assert copyright is obtained
from the U.S. Department of Energy, and subject to any subsequent five
(5) year renewals, the U.S. Government is granted for itself and others
acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
license in the Software to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so.


Modified BSD License agreement

BuildOpt Copyright (c) 2010, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy).  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. Neither the name of the University of California, Lawrence
      Berkeley National Laboratory, U.S. Dept. of Energy nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes,
patches, or upgrades to the features, functionality or performance of
the source code ("Enhancements") to anyone; however, if you choose to
make your Enhancements available either publicly, or directly to
Lawrence Berkeley National Laboratory, without imposing a separate
written license agreement for such Enhancements, then you hereby grant
the following license: a non-exclusive, royalty-free perpetual license
to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.
*/
