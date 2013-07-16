//--- Handle IDF data needed to export an EnergyPlus simulation as an FMU.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  Handle IDF data needed to export an EnergyPlus simulation as an FMU.


#if !defined(__FMU_EXPORT_IDF_DATA__)
#define __FMU_EXPORT_IDF_DATA__


//--- Includes.
//
//#include <fstream>
//#include <string>

#include "../read-ep-file-src/ep-idd-map.h"
#include "../read-ep-file-src/fileReaderData.h"
#include "../read-ep-file-src/fileReaderDictionary.h"


//-- Read and store data from EnergyPlus IDF file.
//
class fmuExportIdfData {

public:

  /// Constructor.
  fmuExportIdfData(void);

  /// Attach an error-reporting function.
  /// \param errFcn Pointer to function to be called in case of an error.
  void attachErrorFcn(void (*errFcn)(std::ostringstream& errorMessage));


  //--- Validate IDD file.
  //
  //   Convenience method, to verify that an Input Data Dictionary is compatible
  // with this version of class \c fmuExportIdfData.
  //
  /// \param idd Map containing input data keywords and descriptors (Input Data Dictionary).
  /// \return 1 if \c idd is compatible with expected entries for method \c populateFromIDF().
  bool haveValidIDD(const iddMap& idd, string& errStr) const;

  /// Read IDF file, collecting data needed to export an EnergyPlus simulation as an FMU.
  //
  /// \param frIdf IDF-file reader, configured to read from EnergyPlus Input Data File of interest.
  /// \return 0 on success; or IDF line number where encountered a problem.
  int populateFromIDF(fileReaderData& frIdf);

  /// Check have a complete set of data.
  bool check(void);

  //-- Public data.
  //
  //   Public because don't want to create a formal API to access data.
  // However, user should treat these as read-only.
  //
  std::vector<int> _toActuator_idfLineNo;
  std::vector<std::string> _toActuator_epName;
  std::vector<std::string> _toActuator_fmuVarName;
  std::vector<double> _toActuator_initValue;
  //
  std::vector<int> _toSched_idfLineNo;
  std::vector<std::string> _toSched_epSchedName;
  std::vector<std::string> _toSched_fmuVarName;
  std::vector<double> _toSched_initValue;
  //
  std::vector<int> _toVar_idfLineNo;
  std::vector<std::string> _toVar_epName;
  std::vector<std::string> _toVar_fmuVarName;
  std::vector<double> _toVar_initValue;
  //
  std::vector<int> _fromVar_idfLineNo;
  std::vector<std::string> _fromVar_epKeyName;
  std::vector<std::string> _fromVar_epVarName;
  std::vector<std::string> _fromVar_fmuVarName;

private:

  //-- Private data.
  //
  bool _goodRead;
  void (*_externalErrorFcn)(std::ostringstream& errorMessage);
  bool _gotKeyExtInt;

  //-- Private methods.
  //
  void reportError(std::ostringstream& errorMessage) const;
  void handleKey_extInt(fileReaderData& frIdf);
  void handleKey_extInt_fmuExport_toActuator(fileReaderData& frIdf);
  void handleKey_extInt_fmuExport_toSched(fileReaderData& frIdf);
  void handleKey_extInt_fmuExport_fromVar(fileReaderData& frIdf);
  void handleKey_extInt_fmuExport_toVar(fileReaderData& frIdf);

};


#endif // __FMU_EXPORT_IDF_DATA__


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
