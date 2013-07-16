//--- Write all files needed to export an EnergyPlus simulation as an FMU.


//--- Includes.
//
// #include <assert.h>
// 
// #include <sstream>
#include <cstdlib>
#include <fstream>

#include <iostream>
using std::cout;
using std::endl;

#include "app-cmdln-input.h"
#include "fmu-export-idf-data.h"
#include "fmu-export-write-model-desc.h"
#include "fmu-export-write-vars-cfg.h"

#include "../read-ep-file-src/ep-idd-map.h"
#include "../read-ep-file-src/fileReaderData.h"
#include "../read-ep-file-src/fileReaderDictionary.h"

#include "../utility-src/file-help.h"
#include "../utility-src/time-help.h"
#include "../utility-src/utilReport.h"


//--- File-scope constants.


//--- Functions.
//
static void getIdfData(cmdlnInput_s& cmdlnInput, fmuExportIdfData& fmuIdfData);


//--- Main driver.
//
int main(int argc, const char* argv[])
  {
  //
  // Read command line.
  cmdlnInput_s cmdlnInput;
  if( false == cmdlnInput_get(argc, argv, &cmdlnInput, reportInputError) )
    {
    return( EXIT_FAILURE );
    }
  //
  // Read data from IDF file.
  fmuExportIdfData fmuIdfData;
  getIdfData(cmdlnInput, fmuIdfData);
  //
  // Write {modelDescription.xml}.
  std::ofstream outStream;
  std::string errStr;
  if( ! openOutputFile(outStream, "modelDescription.xml", std::ios::out | std::ios::trunc, errStr) )
    {
    reportError(errStr);
    return( EXIT_FAILURE );
    }
  char currTimeUTC[21];
  getCurrTimeUTC(currTimeUTC);
  modelDescXml_write(outStream,
    argv[0], currTimeUTC, cmdlnInput.idfFileName, fmuIdfData, cmdlnInput.wthFileName);
  if( outStream.is_open() )
    outStream.close();
  //
  // Write {variables.cfg}.
  if( ! openOutputFile(outStream, "variables.cfg", std::ios::out | std::ios::trunc, errStr) )
    {
    reportError(errStr);
    return( EXIT_FAILURE );
    }
  varsCfg_write(outStream, cmdlnInput.idfFileName, fmuIdfData);
  if( outStream.is_open() )
    outStream.close();
  //
  // Finalize.
  //
  }  // End fcn main().


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
