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

#include "../read-ep-file/ep-idd-map.h"
#include "../read-ep-file/fileReaderData.h"
#include "../read-ep-file/fileReaderDictionary.h"


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


  /// Read Weather file, collecting data needed to export an EnergyPlus simulation as an FMU.
  //
  /// \param frIdf IDF-file reader, configured to read from EnergyPlus Input Data File of interest.
  /// \param leapYear 1 if leap year 0 else.
  /// \param idfVer The IDF version extracted from the IDF file.
  /// \return 0 on success; or IDF line number where encountered a problem.
  int writeInputFile(fileReaderData& frIdf, int leapYear, int idfVer, string tStartFMU, string tStopFMU);

  /// Read Weather file, collecting data needed to export an EnergyPlus simulation as an FMU.
  //
  /// \param frIdf Weather-file reader, configured to read from EnergyPlus Weather Data File of interest.
  /// \return 1 if Leapyear success; or Weather line number where encountered a problem.
  int isLeapYear(fileReaderData& frIdf, int &leapYear);

  /// Read Weather file, collecting data needed to export an EnergyPlus simulation as an FMU.
  //
  /// \param frIdf Input-file reader, configured to read from EnergyPlus Weather Data File of interest.
  /// \return 0 on success; or IDF line number where encountered a problem.
  int getTimeStep(fileReaderData& frIdf);

  /// Read IDF file and get IDF version, collecting data needed to export an EnergyPlus simulation as an FMU.
  //
  /// \param frIdf Input-file reader, configured to read from EnergyPlus Weather Data File of interest.
  /// \return 0 on success; or IDF line number where encountered a problem.
  int getIDFVersion(fileReaderData& frIdf, int &idfVersion);

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

  std::vector<double> _runPer_numerics;
  std::vector<double> _timeStep;
  std::vector<std::string> _runPer_strings;

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
  void handleKey_runPer(fileReaderData& frIdf, int idfVer);
  //int handleKey_timeStep(fileReaderData& frIdf);

};


#endif // __FMU_EXPORT_IDF_DATA__


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
