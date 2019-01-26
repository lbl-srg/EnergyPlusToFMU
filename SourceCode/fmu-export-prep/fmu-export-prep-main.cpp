//--- Write all files needed to export an EnergyPlus simulation as an FMU.


//--- Includes.
//
// #include <assert.h>
//
// #include <sstream>
#include <cstdlib>
#include <fstream>
#include <cstring>

#include <iostream>
using std::cout;
using std::endl;

#include "app-cmdln-input.h"
#include "fmu-export-idf-data.h"
#include "fmu-export-write-model-desc.h"
#include "fmu-export-write-vars-cfg.h"

#include "../read-ep-file/ep-idd-map.h"
#include "../read-ep-file/fileReaderData.h"
#include "../read-ep-file/fileReaderDictionary.h"

#include "../utility/file-help.h"
#include "../utility/time-help.h"
#include "../utility/utilReport.h"


//--- File-scope constants.


//--- Functions.
//
static void getIdfData(cmdlnInput_s& cmdlnInput, fmuExportIdfData& fmuIdfData);

//
static void getInputData(cmdlnInput_s& cmdlnInput, fmuExportIdfData& fmuIdfData);


//--- Main driver.
//
int main(int argc, const char* argv[])
{
	//
	// Read command line.
	cmdlnInput_s cmdlnInput;
	if (false == cmdlnInput_get(argc, argv, &cmdlnInput, reportInputError))
	{
		return(EXIT_FAILURE);
	}
	// Read data from IDF file.
	fmuExportIdfData fmuIdfData;
	if (!cmdlnInput.tStartFMU && !cmdlnInput.tStopFMU){
		cout << "Reading input and weather file for EnergyPlusToFMU program." << endl;
		getIdfData(cmdlnInput, fmuIdfData);
		//
		// Write {modelDescription.xml}.
		std::ofstream outStream;
		std::string errStr;
		if (!openOutputFile(outStream, "modelDescription.xml", std::ios::out | std::ios::trunc, errStr))
		{
			reportError(errStr);
			return(EXIT_FAILURE);
		}
		char currTimeUTC[21];
		getCurrTimeUTC(currTimeUTC);
		modelDescXml_write(outStream,
			argv[0], currTimeUTC, cmdlnInput.idfFileName, fmuIdfData, cmdlnInput.wthFileName);
		if (outStream.is_open())
			outStream.close();
		//
		// Write {variables.cfg}.
		if (!openOutputFile(outStream, "variables.cfg", std::ios::out | std::ios::trunc, errStr))
		{
			reportError(errStr);
			return(EXIT_FAILURE);
		}
		varsCfg_write(outStream, cmdlnInput.idfFileName, fmuIdfData);
		if (outStream.is_open())
			outStream.close();
	}
	else{
		cout << "Reading input and weather file for preprocessor program." << endl;
		getInputData(cmdlnInput, fmuIdfData);
	}
	//
	// Finalize.
	//
}  // End fcn main().

////--- Main driver.
////
//int main(int argc, const char* argv[])
//  {
//  //
//  // Read command line.
//  cmdlnInput_s cmdlnInput;
//  if( false == cmdlnInput_get(argc, argv, &cmdlnInput, reportInputError) )
//    {
//    return( EXIT_FAILURE );
//    }
//  //
//  // Read data from IDF file.
//  fmuExportIdfData fmuIdfData;
//  getIdfData(cmdlnInput, fmuIdfData);
//  //
//  // Write {modelDescription.xml}.
//  std::ofstream outStream;
//  std::string errStr;
//  if( ! openOutputFile(outStream, "modelDescription.xml", std::ios::out | std::ios::trunc, errStr) )
//    {
//    reportError(errStr);
//    return( EXIT_FAILURE );
//    }
//  char currTimeUTC[21];
//  getCurrTimeUTC(currTimeUTC);
//  modelDescXml_write(outStream,
//    argv[0], currTimeUTC, cmdlnInput.idfFileName, fmuIdfData, cmdlnInput.wthFileName);
//  if( outStream.is_open() )
//    outStream.close();
//  //
//  // Write {variables.cfg}.
//  if( ! openOutputFile(outStream, "variables.cfg", std::ios::out | std::ios::trunc, errStr) )
//    {
//    reportError(errStr);
//    return( EXIT_FAILURE );
//    }
//  varsCfg_write(outStream, cmdlnInput.idfFileName, fmuIdfData);
//  if( outStream.is_open() )
//    outStream.close();
//  //
//  // Finalize.
//  //
//  }  // End fcn main().


//--- Read required data from IDF file.
//
static void getIdfData(cmdlnInput_s& cmdlnInput, fmuExportIdfData& fmuIdfData)
  {
  //
  // Set up data dictionary.
  fileReaderDictionary frIdd(cmdlnInput.iddFileName);
  frIdd.attachErrorFcn(reportInputError);
  frIdd.open();
  iddMap idd;
  frIdd.getMap(idd);  // Terminates on error.
  //
  // Check data dictionary.
  string errStr;
  // Do not check Runperiod at this point since the descriptor depends on the version of E+
  if( ! fmuIdfData.haveValidIDD(idd, errStr) )
    {
    cout << "Incompatible IDD file " << cmdlnInput.iddFileName <<
      endl << errStr << endl;
    exit( EXIT_FAILURE );
    }
  //
  // Initialize input data file.
  fileReaderData frIdf(cmdlnInput.idfFileName, IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
  frIdf.attachErrorFcn(reportInputError);
  frIdf.open();
  //
  // Read IDF file for data of interest.
  const int failLine = fmuIdfData.populateFromIDF(frIdf);
  if( 0 < failLine )
    {
    cout << "Error detected while reading IDF file " << cmdlnInput.idfFileName << ", at line #" << failLine << endl;
    exit( EXIT_FAILURE );
    }
  //
  // Finish checking data extracted from IDF file.
  // hoho  What if IDF file defines nothing for FMU to do?
  if( ! fmuIdfData.check() )
    {
    cout << "Error detected after finished reading IDF file " << cmdlnInput.idfFileName << endl;
    exit( EXIT_FAILURE );
    }
  //
  // Here, successfully extracted data of interest from the IDF file.
  }  // End fcn getIdfData().


//--- Read required data from IDF file.
//
static void getInputData(cmdlnInput_s& cmdlnInput, fmuExportIdfData& fmuIdfData)
{
	//
	// Set up data dictionary.
	fileReaderDictionary frIdd(cmdlnInput.iddFileName);
	int idfVer;
	frIdd.attachErrorFcn(reportInputError);
	frIdd.open();
	iddMap idd;
	int timeStep;
	int leapYear;
	frIdd.getMap(idd);  // Terminates on error.
	//
	// Check data dictionary.
	string errStr;
	if (!fmuIdfData.haveValidIDD(idd, errStr))
	{
		cout << "Incompatible IDD file " << cmdlnInput.iddFileName <<
			endl << errStr << endl;
		exit(EXIT_FAILURE);
	}


	//
	// Initialize IDF file and get IDFVersion.
	fileReaderData frIdf0(cmdlnInput.idfFileName, IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
	frIdf0.attachErrorFcn(reportInputError);
	frIdf0.open();
	// Read IDF file for data of interest.
	int failLine = fmuIdfData.getIDFVersion(frIdf0, idfVer);
	if (0 < failLine)
	{
		cout << "Error detected while reading IDF version of IDF file " << cmdlnInput.idfFileName << ", at line #" << failLine << endl;
		exit(EXIT_FAILURE);
	}

	cout << "The IDF version of the input file " << cmdlnInput.idfFileName << " starts with " << idfVer << endl;

	// Initialize weather data file.
	fileReaderData frIdf1(cmdlnInput.wthFileName, IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
	frIdf1.attachErrorFcn(reportInputError);
	frIdf1.open();
	// Read IDF file for data of interest.
	failLine = fmuIdfData.isLeapYear(frIdf1, leapYear);
	if (0 < failLine)
	{
		cout << "Error detected while reading Weather file " << cmdlnInput.wthFileName << ", at line #" << failLine << endl;
		exit(EXIT_FAILURE);
	}

	// Initialize input data file.
	fileReaderData frIdf2(cmdlnInput.idfFileName, IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
	frIdf2.attachErrorFcn(reportInputError);
	frIdf2.open();
	//
	// Read IDF file for data of interest.
	failLine = fmuIdfData.writeInputFile(frIdf2, leapYear, idfVer, cmdlnInput.tStartFMU, cmdlnInput.tStopFMU);
	if (0 < failLine)
	{
		cout << "Error detected while reading IDF file " << cmdlnInput.idfFileName << ", at line #" << failLine << endl;
		exit(EXIT_FAILURE);
	}

	// Initialize input data file.
	fileReaderData frIdf3(cmdlnInput.idfFileName, IDF_DELIMITERS_ENTRY, IDF_DELIMITERS_SECTION);
	frIdf3.attachErrorFcn(reportInputError);
	frIdf3.open();
	//
	// Read IDF file for data of interest.
	failLine = fmuIdfData.getTimeStep(frIdf3);
	if (0 < failLine)
	{
		cout << "Error detected while reading IDF file " << cmdlnInput.idfFileName << ", at line #" << failLine << endl;
		exit(EXIT_FAILURE);
	}
	//
	// Here, successfully extracted data of interest from the IDF file.
}  // End fcn getInputData().


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
