//--- Write file modelDescription.xml needed to export an EnergyPlus simulation as an FMU.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <cstdio>
#include <ctype.h>
#include <string>
using std::string;
#include <ostream>
using std::endl;

#include "fmu-export-write-model-desc.h"

#include "../utility/digest-md5.h"
#include "../utility/file-help.h"
#include "../utility/xml-output-help.h"


//--- Microsoft doesn't implement the modern standard.
//
#ifdef _MSC_VER
  #define snprintf sprintf_s
#endif


//--- File-scope constants.


//--- Functions.
//
static void writeTag_scalarVariable(std::ostream& outStream, const int indentLevel,
  const char *const fmuVarName, const int valueReference,
  const bool toEP, const int idfLineNo, const double initValue);

static string sanitizeIdfFileName(const char *const idfFileBaseName);


//-- Write file {modelDescription.xml}.
//
void modelDescXml_write(std::ostream& outStream,
  const char *const genToolName, const char *const genDateTime,
  const char *const idfFileName, const fmuExportIdfData& fmuIdfData,
  const char *const wthFileName)
  {
  // Constants.
  const char *const topTagName = "fmiModelDescription";
  const char *const modelVarsTagName = "ModelVariables";
  const char *const implementationTagName = "Implementation";
  const char *const cosimToolTagName = "CoSimulation_Tool";
  const char *const modelTagName = "Model";
  //
  const char *const idfFileBaseName = idfFileName + findFileBaseNameIdx(idfFileName);
  //
  // Convenience variables.
  string composedStr;
  //
  //-- Write header.
  outStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  composedStr = "This is file 'modelDescription.xml' for input '";
  composedStr.append(idfFileBaseName).append("'.");
  xmlOutput_comment(outStream, 0, composedStr.c_str());
  //
  //-- Open top-level tag.
  xmlOutput_startTag(outStream, 0, topTagName);
  //
  xmlOutput_attribute(outStream, -1, "fmiVersion", "1.0");
  xmlOutput_attribute(outStream, 0, "modelName", idfFileBaseName);
  //
  // Make the {modelIdentifier} acceptable according to FMU rules.
  composedStr = sanitizeIdfFileName(idfFileBaseName);
  xmlOutput_attribute(outStream, 0, "modelIdentifier", composedStr.c_str());
  //
  // Find GUID as MD5 checksum of IDF file.
  char hexDigestStr[33];
  digest_md5_fromFile(idfFileName, hexDigestStr);
  xmlOutput_attribute(outStream, 0, "guid", hexDigestStr);
  //
  composedStr = "Automatically generated from EnergyPlus input file ";
  composedStr.append(idfFileBaseName);
  xmlOutput_attribute(outStream, 0, "description", composedStr.c_str());
  //
  xmlOutput_attribute(outStream, 0, "generationTool", genToolName+findFileBaseNameIdx(genToolName));
  //
  xmlOutput_attribute(outStream, 0, "generationDateAndTime", genDateTime);
  //
  xmlOutput_attribute(outStream, 0, "variableNamingConvention", "flat");
  //
  // Number of continuous states.
  //   Don't know what this should be.  The FMU documentation is unclear.  The
  // documentation states you can't figure it out by inspecting other contents
  // of the XML file, so it isn't anything obvious like the number of input or
  // output variables.
  //   Just set to 0, and if an FMU master ever complains, figure it out from
  // there.
  xmlOutput_attribute(outStream, 0, "numberOfContinuousStates", "0");
  //
  xmlOutput_attribute(outStream, 0, "numberOfEventIndicators", "0");
  //
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_comment(outStream, 1, "Note guid is an md5 checksum of the IDF file.");
  //
  //-- Open tag for exposed model variables.
  xmlOutput_comment(outStream, 1, "Exposed model variables.");
  xmlOutput_startTag(outStream, 1, modelVarsTagName);
  xmlOutput_startTag_finish(outStream);
  xmlOutput_comment(outStream, 2, "Note valueReferences are (1, 2, 3...) for causality=\"input\" (to E+).");
  xmlOutput_comment(outStream, 2, "Note valueReferences are (100001, 100002, 100003...) for \"output\" (from E+).");
  xmlOutput_comment(outStream, 2, "Note the order of valueReferences should match the order of elements in file 'variables.cfg'.");
  //
  //-- Prepare to write tags corresponding to data passed to EnergyPlus.
  bool toEP = true;
  int valRef = 1;
  int datCt, idx;
  //
  //-- Write tags corresponding to {toActuator} data exchange.
  datCt = (int)fmuIdfData._toActuator_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toActuator_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toActuator_idfLineNo[idx], fmuIdfData._toActuator_initValue[idx]);
    }
  //
  //-- Write tags corresponding to {toSched} data exchange.
  datCt = (int)fmuIdfData._toSched_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toSched_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toSched_idfLineNo[idx], fmuIdfData._toSched_initValue[idx]);
    }
  //
  //-- Write tags corresponding to {toVar} data exchange.
  datCt = (int)fmuIdfData._toVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toVar_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toVar_idfLineNo[idx], fmuIdfData._toVar_initValue[idx]);
    }
  //
  //-- Prepare to write tags corresponding to data passed out of EnergyPlus.
  toEP = false;
  valRef = 100001;  // hoho  Should test against unlikely case that have 100001 {toEP} variables.
  double dummyInitValue = 0.0;
  //
  //-- Write tags corresponding to {fromVar} data exchange.
  datCt = (int)fmuIdfData._fromVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._fromVar_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._fromVar_idfLineNo[idx], dummyInitValue);
    }
  //
  //-- Close tag for exposed model variables.
  xmlOutput_endTag(outStream, 1, modelVarsTagName);
  //
  //-- Open tag for implementation details.
  xmlOutput_comment(outStream, 1, "Implementation details for co-simulation.");
  xmlOutput_startTag(outStream, 1, implementationTagName);
  xmlOutput_startTag_finish(outStream);
  //
  //-- Open tag for cosim-tool EnergyPlus.
  xmlOutput_comment(outStream, 2, "EnergyPlus provided as tool (as opposed to source code or DLL).");
  xmlOutput_startTag(outStream, 2, cosimToolTagName);
  xmlOutput_startTag_finish(outStream);
  //
  //--- Write whole tag for capabilities.
  xmlOutput_startTag(outStream, 3, "Capabilities");
  xmlOutput_attribute(outStream, 3, "canHandleVariableCommunicationStepSize", "false");
  xmlOutput_attribute(outStream, 3, "canHandleEvents", "false");
  xmlOutput_attribute(outStream, 3, "canRejectSteps", "false");
  xmlOutput_attribute(outStream, 3, "canInterpolateInputs", "false");
  xmlOutput_attribute(outStream, 3, "maxOutputDerivativeOrder", "0");
  // Note the FMI specification spells "asynchronously" wrong.
  xmlOutput_attribute(outStream, 3, "canRunAsynchronuously", "false");
  xmlOutput_attribute(outStream, 3, "canSignalEvents", "false");
  xmlOutput_attribute(outStream, 3, "canBeInstantiatedOnlyOncePerProcess", "false");
  xmlOutput_attribute(outStream, 3, "canNotUseMemoryManagementFunctions", "true");
  xmlOutput_endTag(outStream, -1, NULL);
  //
  //-- Open tag for model.
  xmlOutput_startTag(outStream, 3, modelTagName);
  //
  composedStr = "fmu://resources/";
  composedStr.append(idfFileBaseName);
  xmlOutput_attribute(outStream, 3, "entryPoint", composedStr.c_str());
  //
  xmlOutput_attribute(outStream, 3, "manualStart", "false");
  xmlOutput_attribute(outStream, 3, "type", "text/plain");
  xmlOutput_startTag_finish(outStream);
  //
  //-- Write whole tag for file variables.cfg.
  xmlOutput_startTag(outStream, 4, "File");
  xmlOutput_attribute(outStream, -1, "file", "fmu://resources/variables.cfg");
  xmlOutput_endTag(outStream, -1, NULL);
  //
  //-- Write whole tag for weather file, if necessary.
  if( NULL != wthFileName )
    {
    xmlOutput_startTag(outStream, 4, "File");
    composedStr = "fmu://resources/";
    composedStr.append(wthFileName+findFileBaseNameIdx(wthFileName));
    xmlOutput_attribute(outStream, -1, "file", composedStr.c_str());
    xmlOutput_endTag(outStream, -1, NULL);
    }
  else
    {
    xmlOutput_comment(outStream, 4, "No weather file specified.");
    }
  //
  //-- Close tag for model.
  xmlOutput_endTag(outStream, 3, modelTagName);
  //
  //-- Close tag for cosim-tool EnergyPlus.
  xmlOutput_endTag(outStream, 2, cosimToolTagName);
  //
  //-- Close tag for implementation details.
  xmlOutput_endTag(outStream, 1, implementationTagName);
  //
  //-- Close top-level tag.
  xmlOutput_endTag(outStream, 0, topTagName);
  outStream << endl;
  //
  }  // End fcn modelDescXml_write().


//--- Write a <ScalarVariable> tag.
//
static void writeTag_scalarVariable(std::ostream& outStream, const int indentLevel,
  const char *const fmuVarName, const int valueReference,
  const bool toEP, const int idfLineNo, const double initValue)
  {
  const char *const scalarVarTagName = "ScalarVariable";
  //
  #define HS_MAX 64
  char helpStr[HS_MAX];
  //
  xmlOutput_startTag(outStream, indentLevel, scalarVarTagName);
  xmlOutput_attribute(outStream, -1, "name", fmuVarName);
  //
  snprintf(helpStr, HS_MAX, "%i", valueReference);
  xmlOutput_attribute(outStream, -1, "valueReference", helpStr);
  //
  xmlOutput_attribute(outStream, indentLevel, "variability", "continuous");
  xmlOutput_attribute(outStream, -1, "causality", (toEP ? "input" : "output"));
  //
  snprintf(helpStr, HS_MAX, "IDF line %i", idfLineNo);
  xmlOutput_attribute(outStream, indentLevel, "description", helpStr);
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_startTag(outStream, indentLevel+1, "Real");
  if( toEP )
    {
    snprintf(helpStr, HS_MAX, "%g", initValue);
    xmlOutput_attribute(outStream, -1, "start", helpStr);
    }
  xmlOutput_endTag(outStream, -1, NULL);
  //
  xmlOutput_endTag(outStream, indentLevel, scalarVarTagName);
  //
  #undef HS_MAX
  //
  }  // End fcn writeTag_scalarVariable().


//--- Sanitize the name of an IDF file.
//
//   For reasons related to the FMU, the "model identifier" name has to be
// acceptable as the name of a C function.  It also must match the base name of
// the ZIP file that will contain the FMU.
//   In C, a function name:
// - Can contain any of the characters {a-z,A-Z,0-9,_}.
// - Cannot start with a number.
// - Can contain universal character names from the ISO/IEC TR 10176 standard.
//   However, universal character names are not supported here.
//
static string sanitizeIdfFileName(const char *const idfFileBaseName)
  {
  string composedStr = idfFileBaseName;
  //
  // Can't start with a number.
  if( isdigit(composedStr[0]) )
    {
    composedStr.insert(0, "f_");
    }
  //
  // Replace all illegal characters with an underscore.
  const int len = composedStr.size();
  for( int idx=0; idx<len; ++idx )
    {
    if( ! isalnum(composedStr[idx]) )
      {
      composedStr[idx] = '_';
      }
    }
  //
  // Strip ".idf" if possible.
  //   Note it has been changed to "_idf".
  const int baseNameLenM4 = len - 4;
  if( 0<baseNameLenM4
    &&
    (0==composedStr.compare(baseNameLenM4, 4, "_idf") || 0==composedStr.compare(baseNameLenM4, 4, "_IDF")))
    {
    composedStr.replace(baseNameLenM4, 4, "");
    }
  //
  return( composedStr );
  }  // End fcn sanitizeIdfFileName().
