///////////////////////////////////////////////////////
/// \file   fileReaderDictionary.h
/// \author Michael Wetter,
///         LBNL, Simulation Research Group,
///         MWetter@lbl.gov
/// \date   2001-06-19
///
/// \brief  File reader for EnergyPlus input data dictionary (IDD) file.
///
///////////////////////////////////////////////////////
#if !defined(__FILEREADERDICTIONARY_H__)
#define __FILEREADERDICTIONARY_H__


#include "fileReader.h"

#include "ep-idd-map.h"


///////////////////////////////////////////////////////
/// File reader for input data dictionary.
class fileReaderDictionary : public fileReader {

  public:
  fileReaderDictionary(const std::string& fname) :  fileReader(fname) { } ;

  /// Gets the next keyword and the descriptor from the input file stream.
  ///
  ///  In case of input error, the error is reported by this method, and \c false
  ///  is returned.
  /// \retval keyword String that contains the keyword after execution.
  /// \retval desc string to store the descriptor from the data dictionary.
  /// \return \c true if input reading is successful, \c false otherwise.
  bool getKeywordAndDescriptor(std::string& keyword, std::string& desc);

  /// Gets all keywords and their corresponding data descriptors from 
  ///  the input file stream. 
  ///  
  ///  After execution, all keywords and their descriptors are stored
  ///  in the argument \c idd.
  ///
  ///  \note In case of input error, the program terminates.
  ///
  /// \pre This method requires the input file stream to be open. 
  /// \retval idd Map that contains the keywords and their descriptors.
  void getMap(iddMap& idd);

};


#endif // __FILEREADERDICTIONARY_H__


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
