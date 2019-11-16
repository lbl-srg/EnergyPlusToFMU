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

#include "../../utility/digest-md5.h"
#include "../../utility/file-help.h"
#include "../../utility/xml-output-help.h"


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

//
static void writeTag_outputVariable(std::ostream& outStream, const int indentLevel,
	int index);

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
  const char *const cosimToolTagName = "CoSimulation";
  const char *const modelTagName = "Model";
  const char *const modelStructureTagName = "ModelStructure";
  const char *const outputsTagName = "Outputs";
  //
  const char *const idfFileBaseName = idfFileName + findFileBaseNameIdx(idfFileName);
#define HS_MAX 64
  char helpStr[HS_MAX];
  //
  // Convenience variables.
  string composedStr;
  int numInps;
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
  xmlOutput_attribute(outStream, -1, "fmiVersion", "2.0");
  xmlOutput_attribute(outStream, 0, "modelName", idfFileBaseName);
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
  xmlOutput_attribute(outStream, 0, "numberOfEventIndicators", "0");
  //
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_comment(outStream, 1, "Note guid is an md5 checksum of the IDF file.");

  //-- Open tag for cosimulation EnergyPlus.
  xmlOutput_comment(outStream, 1, "EnergyPlus provided as tool (as opposed to source code or DLL).");
  xmlOutput_startTag(outStream, 1, cosimToolTagName);
  //
  //--- Write whole tag for capabilities.
  // Make the {modelIdentifier} acceptable according to FMU rules.
  composedStr = sanitizeIdfFileName(idfFileBaseName);
  xmlOutput_attribute(outStream, 2, "modelIdentifier", composedStr.c_str());
  xmlOutput_attribute(outStream, 2, "needsExecutionTool", "false");
  xmlOutput_attribute(outStream, 2, "canHandleVariableCommunicationStepSize", "false");
  xmlOutput_attribute(outStream, 2, "canInterpolateInputs", "false");
  xmlOutput_attribute(outStream, 2, "maxOutputDerivativeOrder", "0");
  xmlOutput_attribute(outStream, 2, "canGetAndSetFMUstate", "false");
  xmlOutput_attribute(outStream, 2, "canSerializeFMUstate", "false");
  // Note the FMI specification spells "asynchronously" wrong.
  xmlOutput_attribute(outStream, 2, "canRunAsynchronuously", "false");
  xmlOutput_attribute(outStream, 2, "canBeInstantiatedOnlyOncePerProcess", "false");
  xmlOutput_attribute(outStream, 2, "canNotUseMemoryManagementFunctions", "true");
  xmlOutput_attribute(outStream, 2, "providesDirectionalDerivative", "false");
  xmlOutput_endTag(outStream, -1, NULL);
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
	composedStr = "Index for next variable is '";
	snprintf(helpStr, HS_MAX, "%i", idx + 1);
	composedStr.append(helpStr).append("'.");
	xmlOutput_comment(outStream, 2, composedStr.c_str());
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toActuator_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toActuator_idfLineNo[idx], fmuIdfData._toActuator_initValue[idx]);
    }
  //
  //-- Write tags corresponding to {toSched} data exchange.
  datCt = (int)fmuIdfData._toSched_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
	composedStr = "Index for next variable is '";
	snprintf(helpStr, HS_MAX, "%i", (int)fmuIdfData._toActuator_idfLineNo.size() + idx + 1);
	composedStr.append(helpStr).append("'.");
	xmlOutput_comment(outStream, 2, composedStr.c_str());
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toSched_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toSched_idfLineNo[idx], fmuIdfData._toSched_initValue[idx]);
    }
  //
  //-- Write tags corresponding to {toVar} data exchange.
  datCt = (int)fmuIdfData._toVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
	  composedStr = "Index for next variable is '";
	  snprintf(helpStr, HS_MAX, "%i", (int)fmuIdfData._toActuator_idfLineNo.size() + 
		  (int)fmuIdfData._toSched_idfLineNo.size() + idx + 1);
	  composedStr.append(helpStr).append("'.");
	  xmlOutput_comment(outStream, 2, composedStr.c_str());
	  writeTag_scalarVariable(outStream, 2,
      fmuIdfData._toVar_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._toVar_idfLineNo[idx], fmuIdfData._toVar_initValue[idx]);
    }
  //
  // -- compute the number of inputs 
  numInps = (int)fmuIdfData._toActuator_idfLineNo.size() +
	  (int)fmuIdfData._toSched_idfLineNo.size() +
	  (int)fmuIdfData._toVar_idfLineNo.size();
  //-- Prepare to write tags corresponding to data passed out of EnergyPlus.
  toEP = false;
  valRef = 100001;  // hoho  Should test against unlikely case that have 100001 {toEP} variables.
  double dummyInitValue = 0.0;
  //
  //-- Write tags corresponding to {fromVar} data exchange.
  datCt = (int)fmuIdfData._fromVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx, ++valRef )
    {
	  composedStr = "Index for next variable is '";
	  snprintf(helpStr, HS_MAX, "%i", (numInps + idx + 1));
	  composedStr.append(helpStr).append("'.");
	  xmlOutput_comment(outStream, 2, composedStr.c_str());
    writeTag_scalarVariable(outStream, 2,
      fmuIdfData._fromVar_fmuVarName[idx].c_str(), valRef,
      toEP, fmuIdfData._fromVar_idfLineNo[idx], dummyInitValue);
    }
  //
  //-- Close tag for exposed model variables.
  xmlOutput_endTag(outStream, 1, modelVarsTagName);
  //
  //-- Open tag for model structure details.
  xmlOutput_comment(outStream, 1, "ModelStructure details for co-simulation.");
  xmlOutput_startTag(outStream, 1, modelStructureTagName);
  xmlOutput_startTag_finish(outStream);
  //
  //-- Open tag for model outputs details.
  xmlOutput_comment(outStream, 2, "Outputs variables of the FMU.");
  xmlOutput_startTag(outStream, 2, outputsTagName);
  xmlOutput_startTag_finish(outStream);

  //-- Write tags corresponding to {fromVar} data exchange.
  datCt = (int)fmuIdfData._fromVar_idfLineNo.size();
  //-- Write the output variable index based on the last input
  for (idx = 0; idx<datCt; ++idx, ++valRef)
  {
	  writeTag_outputVariable(outStream, 3, numInps + idx + 1);
  }
  //
  //-- Close outputs tag.
  xmlOutput_endTag(outStream, 2, outputsTagName);
  //
  //-- Close model structure tag.
  xmlOutput_endTag(outStream, 1, modelStructureTagName);
  
  //-- Close top-level tag.
  xmlOutput_endTag(outStream, 0, topTagName);
 
  outStream << endl;
  //
#undef HS_MAX
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


//--- Write an <OutputVariable> tag.
//
static void writeTag_outputVariable(std::ostream& outStream, const int indentLevel,
	int index)
{
	const char *const outputVarTagName = "Unknown";
	//
#define HS_MAX 64
	char helpStr[HS_MAX];
	//
	xmlOutput_startTag(outStream, indentLevel, outputVarTagName);
	snprintf(helpStr, HS_MAX, "%i", index);
	xmlOutput_attribute(outStream, -1, "index", helpStr);
	xmlOutput_endTag(outStream, -1, NULL);
	//
#undef HS_MAX
	//
}  // End fcn writeTag_outputVariable().


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
