//--- Write file variables.cfg needed to export an EnergyPlus simulation as an FMU.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <string>
using std::string;
#include <ostream>
using std::endl;

#include "fmu-export-write-vars-cfg.h"

#include "../utility/file-help.h"
#include "../utility/xml-output-help.h"


//--- File-scope constants.
//
const char *const variableTagName = "variable";
//
const char *const sourceAttName = "source";
const char *const sourceAttValue_toEP = "Ptolemy";
const char *const sourceAttValue_fromEP = "EnergyPlus";
//
const char *const energyPlusTagName = "EnergyPlus";


//--- Functions.
static void writeTag_variable_toActuator(std::ostream& outStream, const int indentLevel,
  const char *const epName);
static void writeTag_variable_toSched(std::ostream& outStream, const int indentLevel,
  const char *const epSchedName);
static void writeTag_variable_toVar(std::ostream& outStream, const int indentLevel,
  const char *const epName);
static void writeTag_variable_fromVar(std::ostream& outStream, const int indentLevel,
  const char *const epKeyName, const char *const epVarName);


//-- Write file {modelDescription.xml}.
//
void varsCfg_write(std::ostream& outStream,
  const char *const idfFileName,
  const fmuExportIdfData& fmuIdfData)
  {
  // Constants.
  const char *const topTagName = "BCVTB-variables";
  //
  // Convenience variables.
  int datCt, idx;
  string composedStr;
  //
  //-- Write header.
  outStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << endl <<
    "<!DOCTYPE BCVTB-variables SYSTEM \"variables.dtd\">";
  composedStr = "This is file 'variables.cfg' for input '";
  composedStr.append(idfFileName+findFileBaseNameIdx(idfFileName)).append("'.");
  xmlOutput_comment(outStream, 0, composedStr.c_str());
  //
  //-- Write top-level tag.
  xmlOutput_startTag(outStream, 0, topTagName);
  xmlOutput_startTag_finish(outStream);
  xmlOutput_comment(outStream, 1, "Note these are not really BCVTB-related.  We are bootstrapping BCVTB capabilities to support FMU export.");
  xmlOutput_comment(outStream, 1, "Note the order of these elements determines the data order in the exchange vectors.");
  //
  //-- Write tags corresponding to {toActuator} data exchange.
  datCt = (int)fmuIdfData._toActuator_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx )
    {
    writeTag_variable_toActuator(outStream, 1,
      fmuIdfData._toActuator_epName[idx].c_str());
    }
  //
  //-- Write tags corresponding to {toSched} data exchange.
  datCt = (int)fmuIdfData._toSched_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx )
    {
    writeTag_variable_toSched(outStream, 1,
      fmuIdfData._toSched_epSchedName[idx].c_str());
    }
  //
  //-- Write tags corresponding to {toVar} data exchange.
  datCt = (int)fmuIdfData._toVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx )
    {
    writeTag_variable_toVar(outStream, 1,
      fmuIdfData._toVar_epName[idx].c_str());
    }
  //
  //-- Write tags corresponding to {fromVar} data exchange.
  datCt = (int)fmuIdfData._fromVar_idfLineNo.size();
  for( idx=0; idx<datCt; ++idx )
    {
    writeTag_variable_fromVar(outStream, 1,
      fmuIdfData._fromVar_epKeyName[idx].c_str(), fmuIdfData._fromVar_epVarName[idx].c_str());
    }
  //
  //-- Close top-level tag.
  xmlOutput_endTag(outStream, 0, topTagName);
  outStream << endl;
  //
  }  // End fcn varsCfg_write().


//--- Write a <variable> tag for {toActuator} data exchange.
//
static void writeTag_variable_toActuator(std::ostream& outStream, const int indentLevel,
  const char *const epName)
  {
  xmlOutput_startTag(outStream, indentLevel, variableTagName);
  xmlOutput_attribute(outStream, -1, sourceAttName, sourceAttValue_toEP);
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_startTag(outStream, indentLevel+1, energyPlusTagName);
  xmlOutput_attribute(outStream, -1, "actuator", epName);
  xmlOutput_endTag(outStream, -1, NULL);
  //
  xmlOutput_endTag(outStream, indentLevel, variableTagName);
  }  // End fcn writeTag_variable_toActuator().


//--- Write a <variable> tag for {toSched} data exchange.
//
static void writeTag_variable_toSched(std::ostream& outStream, const int indentLevel,
  const char *const epSchedName)
  {
  xmlOutput_startTag(outStream, indentLevel, variableTagName);
  xmlOutput_attribute(outStream, -1, sourceAttName, sourceAttValue_toEP);
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_startTag(outStream, indentLevel+1, energyPlusTagName);
  xmlOutput_attribute(outStream, -1, "schedule", epSchedName);
  xmlOutput_endTag(outStream, -1, NULL);
  //
  xmlOutput_endTag(outStream, indentLevel, variableTagName);
  }  // End fcn writeTag_variable_toSched().


//--- Write a <variable> tag for {toVar} data exchange.
//
static void writeTag_variable_toVar(std::ostream& outStream, const int indentLevel,
  const char *const epName)
  {
  xmlOutput_startTag(outStream, indentLevel, variableTagName);
  xmlOutput_attribute(outStream, -1, sourceAttName, sourceAttValue_toEP);
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_startTag(outStream, indentLevel+1, energyPlusTagName);
  xmlOutput_attribute(outStream, -1, "variable", epName);
  xmlOutput_endTag(outStream, -1, NULL);
  //
  xmlOutput_endTag(outStream, indentLevel, variableTagName);
  }  // End fcn writeTag_variable_toVar().


//--- Write a <variable> tag for {fromVar} data exchange.
//
static void writeTag_variable_fromVar(std::ostream& outStream, const int indentLevel,
  const char *const epKeyName, const char *const epVarName)
  {
  xmlOutput_startTag(outStream, indentLevel, variableTagName);
  xmlOutput_attribute(outStream, -1, sourceAttName, sourceAttValue_fromEP);
  xmlOutput_startTag_finish(outStream);
  //
  xmlOutput_startTag(outStream, indentLevel+1, energyPlusTagName);
  xmlOutput_attribute(outStream, -1, "name", epKeyName);
  xmlOutput_attribute(outStream, -1, "type", epVarName);
  xmlOutput_endTag(outStream, -1, NULL);
  //
  xmlOutput_endTag(outStream, indentLevel, variableTagName);
  }  // End fcn writeTag_variable_fromVar().
