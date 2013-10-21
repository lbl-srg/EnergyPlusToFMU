//--- Handle IDF data needed to export an EnergyPlus simulation as an FMU.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <assert.h>

#include <sstream>
#include <iostream>
using std::string;
using std::cerr;
using std::cout;
using std::endl;

#include "fmu-export-idf-data.h"

#include "../utility/string-help.h"


//--- File-scope constants.
//
// Expected keywords and dictionary descriptors.
const string g_key_extInt = "EXTERNALINTERFACE";
const string g_desc_extInt = "A";
//
const string g_key_extInt_fmuExport_toActuator = "EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:TO:ACTUATOR";
const string g_desc_extInt_fmuExport_toActuator = "AAAAAN";
//
const string g_key_extInt_fmuExport_toSched = "EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:TO:SCHEDULE";
const string g_desc_extInt_fmuExport_toSched = "AAAN";
//
const string g_key_extInt_fmuExport_fromVar = "EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:FROM:VARIABLE";
const string g_desc_extInt_fmuExport_fromVar = "AAA";
//
const string g_key_extInt_fmuExport_toVar = "EXTERNALINTERFACE:FUNCTIONALMOCKUPUNITEXPORT:TO:VARIABLE";
const string g_desc_extInt_fmuExport_toVar = "AAN";


//--- Functions.


//--- Constructor.
//
fmuExportIdfData::fmuExportIdfData(void)
  {
  _goodRead = false;
  _externalErrorFcn = 0;
  _gotKeyExtInt = false;
  //
  #ifdef _DEBUG
    // Vectors, on construction, should be initialized to zero length.
    assert( _toActuator_idfLineNo.empty() );
    assert( _toActuator_epName.empty() );
    assert( _toActuator_fmuVarName.empty() );
    assert( _toActuator_initValue.empty() );
    //
    assert( _toSched_idfLineNo.empty() );
    assert( _toSched_epSchedName.empty() );
    assert( _toSched_fmuVarName.empty() );
    assert( _toSched_initValue.empty() );
    //
    assert( _toVar_idfLineNo.empty() );
    assert( _toVar_epName.empty() );
    assert( _toVar_fmuVarName.empty() );
    assert( _toVar_initValue.empty() );
    //
    assert( _fromVar_idfLineNo.empty() );
    assert( _fromVar_epKeyName.empty() );
    assert( _fromVar_epVarName.empty() );
    assert( _fromVar_fmuVarName.empty() );
  #endif
  }  // End constructor fmuExportIdfData::fmuExportIdfData().


//--- Attach an error-reporting function.
//
void fmuExportIdfData::attachErrorFcn(void (*errFcn)(std::ostringstream& errorMessage))
  {
  _externalErrorFcn = errFcn;
  }  // End method fmuExportIdfData::attachErrorFcn().


//--- Validate IDD file.
//
bool fmuExportIdfData::haveValidIDD(const iddMap& idd, string& errStr) const
  {
  //
  #ifdef _DEBUG
    assert( ! idd.empty() );
  #endif
  //
  return(
    0==iddMap_compareEntry(idd, g_key_extInt, g_desc_extInt, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toActuator, g_desc_extInt_fmuExport_toActuator, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toSched, g_desc_extInt_fmuExport_toSched, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_fromVar, g_desc_extInt_fmuExport_fromVar, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toVar, g_desc_extInt_fmuExport_toVar, errStr)
    );
  }  // End method fmuExportIdfData::haveValidIDD().


//--- Read IDF file, collecting data needed to export an EnergyPlus simulation as an FMU.
//
int fmuExportIdfData::populateFromIDF(fileReaderData& frIdf)
  {
  //
  int lineNo;
  string idfKey, iddDesc;
  //
  #ifdef _DEBUG
    assert( ! frIdf.isEOF() );
  #endif
  //
  // Initialize.
  lineNo = 0;
  _goodRead = true;
  //
  // Run through the IDF file.
  while( _goodRead )
    {
    // Here, assume looking for next keyword.
    frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
    frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, idfKey);
    // Consume the delimiter that caused getToken() to return.
    const char delimChar = frIdf.getChar();
    if( frIdf.isEOF() )
      {
      // Here, hit EOF.
      //   OK to hit EOF, provided don't actually have a keyword.
      if( 0 != idfKey.length() )
        {
        _goodRead = false;
        std::ostringstream os;
        os << "Error: IDF file ends after keyword '" << idfKey << "' on line " << lineNo;
        reportError(os);
        }
      break;
      }
    // Here, have a keyword (although may be zero length).
    capitalize(idfKey);
    // Handle or skip IDF entry for this keyword.
    if( 0 == g_key_extInt.compare(idfKey) )
      {
      handleKey_extInt(frIdf);
      }
    else if( 0 == g_key_extInt_fmuExport_toActuator.compare(idfKey) )
      {
      handleKey_extInt_fmuExport_toActuator(frIdf);
      }
    else if( 0 == g_key_extInt_fmuExport_toSched.compare(idfKey) )
      {
      handleKey_extInt_fmuExport_toSched(frIdf);
      }
    else if( 0 == g_key_extInt_fmuExport_fromVar.compare(idfKey) )
      {
      handleKey_extInt_fmuExport_fromVar(frIdf);
      }
    else if( 0 == g_key_extInt_fmuExport_toVar.compare(idfKey) )
      {
      handleKey_extInt_fmuExport_toVar(frIdf);
      }
    else
      {
      // Here, don't need to know about this key, or its contents, in order to
      // export the IDF file for use as an FMU.
      //   Skip to the next keyword, without attempting to diagnose any problems.
      // Note if {delimChar}==';', means section had only a keyword, and no
      // need to skip.
      if( ';'!=delimChar && frIdf.skipSection() )
        {
        // Here, hit EOF.
        break;
        }
      }
    // Here, ready to look for next keyword.
    }
  //
  // Here, ran through whole IDF file.
  frIdf.close();
  //
  if( _goodRead )
    {
    lineNo = 0;
    }
  return( lineNo );
  }  // End method fmuExportIdfData::populateFromIDF().


//--- Check have a complete set of data.
//
//   Assume have already called method populateFromIDF().
//
bool fmuExportIdfData::check(void)
  {
  std::ostringstream os;
  //
  if( ! _goodRead )
    {
    os << "IDF file was not parsed, or error encountered during parsing.";
    }
  //
  // Require got keyword {g_key_extInt}.
  // hoho  What if IDF file defines nothing for FMU to do?
  if( _goodRead && !_gotKeyExtInt )
    {
    _goodRead = false;
    os << "IDF file missing keyword '" << g_key_extInt << "'.";
    }
  //
  if( ! _goodRead )
    {
    reportError(os);
    }
  //
  return( _goodRead );
  }  // End method fmuExportIdfData::check().


//--- Report an error.
//
void fmuExportIdfData::reportError(std::ostringstream& errorMessage) const
  {
  //
  // Call user-supplied error fcn if available.
  if( _externalErrorFcn )
    {
    (*_externalErrorFcn)(errorMessage);
    }
  else
    {
    // Here, no user-supplied error fcn.
    //   Note flush both {cout} and {cerr}, to avoid overlapped writes.
    cout.flush();
    cerr << "=== Error ===" << endl
      << errorMessage.str() << endl << endl;
    cerr.flush();
    }
  }  // End method fmuExportIdfData::reportError().


//--- Read IDF values for key {g_key_extInt}.
//
//   Sample input:
// ExternalInterface,            !- Activate the external interface
//   FunctionalMockupUnitExport; !- Name of external interface
//
void fmuExportIdfData::handleKey_extInt(fileReaderData& frIdf)
  {
  bool entryOK;
  vString strVals;
  vDouble dblVals;
  std::ostringstream os;
  //
  // Assume just read key {g_key_extInt} from the IDF file.
  const int keyLineNo = frIdf.getLineNumber();
  //
  // Read values from IDF file.
  entryOK = true;
  if( ! frIdf.getValues(g_desc_extInt, strVals, dblVals) )
    {
    entryOK = false;
    os << "IDF parsing error.";
    }
  //
  // Check count of values.
  if( entryOK
    &&
    ( 1!=strVals.size() || 0!=dblVals.size() ) )
    {
    entryOK = false;
    os << "Wrong number of entries.";
    }
  //
  // Interface name.
  if( entryOK )
    {
    const char *const expectInterfaceName = "FUNCTIONALMOCKUPUNITEXPORT";
    string &gotInterfaceName = strVals[0];
    capitalize(gotInterfaceName);
    if( 0 != gotInterfaceName.compare(expectInterfaceName) )
      {
      entryOK = false;
      os << "Expecting external interface '" << expectInterfaceName << "'; got '" << gotInterfaceName << "'.";
      }
    else
      {
      // Note don't bother testing whether got this IDF input twice.
      _gotKeyExtInt = true;
      }
    }
  //
  if( ! entryOK )
    {
    _goodRead = false;
    os << "\nError encountered while reading values for keyword '" << g_key_extInt
      << "', starting on line " << keyLineNo << " of IDF file.";
    reportError(os);
    }
  //
  return;
  }  // End method fmuExportIdfData::handleKey_extInt().


//--- Read IDF values for key {g_key_extInt_fmuExport_toActuator}.
//
//   Sample input:
// ExternalInterface:FunctionalMockupUnitExport:To:Actuator,  !- FMU master will set the value for this actuator
//   myHTGSETP_SCH,           !- Name
//   HTGSETP_SCH,             !- Actuated component unique name
//   Schedule:Constant,       !- Actuated component type
//   Schedule Value,          !- Actuated component control type
//   EpActuator1,             !- FMU variable name
//   0;                       !- Initial value
//
void fmuExportIdfData::handleKey_extInt_fmuExport_toActuator(fileReaderData& frIdf)
  {
  bool entryOK;
  vString strVals;
  vDouble dblVals;
  std::ostringstream os;
  //
  // Assume just read key {g_key_extInt_fmuExport_toActuator} from the IDF file.
  const int keyLineNo = frIdf.getLineNumber();
  _toActuator_idfLineNo.push_back(keyLineNo);
  //
  // Read values from IDF file.
  entryOK = true;
  if( ! frIdf.getValues(g_desc_extInt_fmuExport_toActuator, strVals, dblVals) )
    {
    entryOK = false;
    os << "IDF parsing error.";
    }
  //
  // Check count of values.
  if( entryOK
    &&
    ( 5!=strVals.size() || 1!=dblVals.size() ) )
    {
    entryOK = false;
    os << "Wrong number of entries.";
    }
  //
  // Name (actuator name in IDF file).
  if( entryOK )
    {
    _toActuator_epName.push_back(strVals[0]);
    // In principle, could check that the IDF file contains the corresponding
    // entry.  However, this would complicate the code here considerably.
    }
  //
  // Actuated component unique name.
  //   Not needed for FMU export.
  //
  // Actuated component type.
  //   Not needed for FMU export.
  //
  // Actuated component control type.
  //   Not needed for FMU export.
  //
  // FMU variable name (name in FMU master).
  if( entryOK )
    {
    _toActuator_fmuVarName.push_back(strVals[4]);
    }
  //
  // Initial value.
  if( entryOK )
    {
    _toActuator_initValue.push_back(dblVals[0]);
    }
  //
  // Check for duplicate names.
  if( entryOK )
    {
    const int elCt = (int)_toActuator_idfLineNo.size();
    //
    #ifdef _DEBUG
      assert( elCt == (int)_toActuator_epName.size() );
      assert( elCt == (int)_toActuator_fmuVarName.size() );
      assert( elCt == (int)_toActuator_initValue.size() );
    #endif
    //
    const string &epName = _toActuator_epName[elCt-1];
    for( int idx=0; idx<elCt-1; ++idx )
      {
      if( 0 == epName.compare(_toActuator_epName[idx]) )
        {
        entryOK = false;
        os << "FMU master already sets value of IDF actuator '" << epName
          << "'. See line " << _toActuator_idfLineNo[idx] << " of IDF file.";
        break;
        }
      // hoho dml  Presumably it's OK for one named value in the FMU master to
      // control more than one actuator (or an actuator and something else) in
      // the EnergyPlus simulation.  If not, should add a comparison of
      // {_toActuator_fmuVarName[idx]} here.
      }
    }
  //
  if( ! entryOK )
    {
    _goodRead = false;
    os << "\nError encountered while reading values for keyword '" << g_key_extInt_fmuExport_toActuator
      << "', starting on line " << keyLineNo << " of IDF file.";
    reportError(os);
    }
  //
  return;
  }  // End method fmuExportIdfData::handleKey_extInt_fmuExport_toActuator().


//--- Read IDF values for key {g_key_extInt_fmuExport_toSched}.
//
//   Sample input:
// ExternalInterface:FunctionalMockupUnitExport:To:Schedule,  !- FMU master will set the value for this schedule
//   ExternalSchedule1,          !- Schedule Name
//   Any Number,                 !- Schedule Type Limits Name
//   EpModelSchedule1,           !- FMU variable name
//   0;                          !- Initial value
//
void fmuExportIdfData::handleKey_extInt_fmuExport_toSched(fileReaderData& frIdf)
  {
  bool entryOK;
  vString strVals;
  vDouble dblVals;
  std::ostringstream os;
  //
  // Assume just read key {g_key_extInt_fmuExport_toSched} from the IDF file.
  const int keyLineNo = frIdf.getLineNumber();
  _toSched_idfLineNo.push_back(keyLineNo);
  //
  // Read values from IDF file.
  entryOK = true;
  if( ! frIdf.getValues(g_desc_extInt_fmuExport_toSched, strVals, dblVals) )
    {
    entryOK = false;
    os << "IDF parsing error.";
    }
  //
  // Check count of values.
  if( entryOK
    &&
    ( 3!=strVals.size() || 1!=dblVals.size() ) )
    {
    entryOK = false;
    os << "Wrong number of entries.";
    }
  //
  // Schedule Name (schedule name in IDF file).
  if( entryOK )
    {
    _toSched_epSchedName.push_back(strVals[0]);
    // In principle, could check that the IDF file contains the corresponding
    // entry.  However, this would complicate the code here considerably.
    }
  //
  // Schedule type limits name.
  //   Not needed for FMU export.
  //
  // FMU variable name (name in FMU master).
  if( entryOK )
    {
    _toSched_fmuVarName.push_back(strVals[2]);
    }
  //
  // Initial value.
  if( entryOK )
    {
    _toSched_initValue.push_back(dblVals[0]);
    }
  //
  // Check for duplicate names.
  if( entryOK )
    {
    const int elCt = (int)_toSched_idfLineNo.size();
    //
    #ifdef _DEBUG
      assert( elCt == (int)_toSched_epSchedName.size() );
      assert( elCt == (int)_toSched_fmuVarName.size() );
      assert( elCt == (int)_toSched_initValue.size() );
    #endif
    //
    const string &epSchedName = _toSched_epSchedName[elCt-1];
    for( int idx=0; idx<elCt-1; ++idx )
      {
      if( 0 == epSchedName.compare(_toSched_epSchedName[idx]) )
        {
        entryOK = false;
        os << "FMU master already sets value of IDF schedule '" << epSchedName
          << "'. See line " << _toSched_idfLineNo[idx] << " of IDF file.";
        break;
        }
      // hoho dml  Presumably it's OK for one named value in the FMU master to
      // control more than one schedule (or a schedule and something else) in
      // the EnergyPlus simulation.  If not, should add a comparison of
      // {_toSched_fmuVarName[idx]} here.
      }
    }
  //
  if( ! entryOK )
    {
    _goodRead = false;
    os << "\nError encountered while reading values for keyword '" << g_key_extInt_fmuExport_toSched
      << "', starting on line " << keyLineNo << " of IDF file.";
    reportError(os);
    }
  //
  return;
  }  // End method fmuExportIdfData::handleKey_extInt_fmuExport_toSched().


//--- Read IDF values for key {g_key_extInt_fmuExport_fromVar}.
//
//   Sample input:
// ExternalInterface:FunctionalMockupUnitExport:From:Variable,  !- FMU master can read the value of this variable
//   ZONE ONE,                   !- Output:Variable Index Key Name
//   Zone Mean Air Temperature,  !- Output:Variable Name
//   TRoom;                      !- FMU variable name
//
void fmuExportIdfData::handleKey_extInt_fmuExport_fromVar(fileReaderData& frIdf)
  {
  bool entryOK;
  vString strVals;
  vDouble dblVals;
  std::ostringstream os;
  //
  // Assume just read key {g_key_extInt_fmuExport_fromVar} from the IDF file.
  const int keyLineNo = frIdf.getLineNumber();
  _fromVar_idfLineNo.push_back(keyLineNo);
  //
  // Read values from IDF file.
  entryOK = true;
  if( ! frIdf.getValues(g_desc_extInt_fmuExport_fromVar, strVals, dblVals) )
    {
    entryOK = false;
    os << "IDF parsing error.";
    }
  //
  // Check count of values.
  if( entryOK
    &&
    ( 3!=strVals.size() || 0!=dblVals.size() ) )
    {
    entryOK = false;
    os << "Wrong number of entries.";
    }
  //
  // Output:Variable Index Key Name (key name in IDF file).
  if( entryOK )
    {
    _fromVar_epKeyName.push_back(strVals[0]);
    // In principle, could check that the IDF file contains the corresponding
    // entry.  However, this would complicate the code here considerably.
    }
  //
  // Output:Variable Name (variable name in IDF file).
  if( entryOK )
    {
    _fromVar_epVarName.push_back(strVals[1]);
    // In principle, could check that the IDF file contains the corresponding
    // entry.  However, this would complicate the code here considerably.
    }
  //
  // FMU variable name (Name in FMU master).
  if( entryOK )
    {
    _fromVar_fmuVarName.push_back(strVals[2]);
    }
  //
  // Check for duplicate names.
  if( entryOK )
    {
    const int elCt = (int)_fromVar_idfLineNo.size();
    //
    #ifdef _DEBUG
      assert( elCt == (int)_fromVar_epKeyName.size() );
      assert( elCt == (int)_fromVar_epVarName.size() );
      assert( elCt == (int)_fromVar_fmuVarName.size() );
    #endif
    //
    const string &fmuVarName = _fromVar_fmuVarName[elCt-1];
    for( int idx=0; idx<elCt-1; ++idx )
      {
      if( 0 == fmuVarName.compare(_fromVar_fmuVarName[idx]) )
        {
        entryOK = false;
        os << "FMU master already reading variable '" << fmuVarName
          << "'. See line " << _fromVar_idfLineNo[idx] << " of IDF file.";
        break;
        }
      // hoho dml  Presumably it's OK for the FMU master to read the same
      // EnergyPlus value into more than one variable.  If not, should add
      // comparisons of {_fromVar_epKeyName[idx]} and {_fromVar_epVarName[idx]}
      // here.
      }
    }
  //
  if( ! entryOK )
    {
    _goodRead = false;
    os << "\nError encountered while reading values for keyword '" << g_key_extInt_fmuExport_fromVar
      << "', starting on line " << keyLineNo << " of IDF file.";
    reportError(os);
    }
  //
  return;
  }  // End method fmuExportIdfData::handleKey_extInt_fmuExport_fromVar().


//--- Read IDF values for key {g_key_extInt_fmuExport_toVar}.
//
//   Sample input:
// ExternalInterface:FunctionalMockupUnitExport:To:Variable,  !- FMU master will set the value for this variable
//   yShade,                  !- Name
//   bldgShadeSig,            !- FMU Variable Name
//   1;                       !- Initial Value
//
void fmuExportIdfData::handleKey_extInt_fmuExport_toVar(fileReaderData& frIdf)
  {
  bool entryOK;
  vString strVals;
  vDouble dblVals;
  std::ostringstream os;
  //
  // Assume just read key {g_key_extInt_fmuExport_toVar} from the IDF file.
  const int keyLineNo = frIdf.getLineNumber();
  _toVar_idfLineNo.push_back(keyLineNo);
  //
  // Read values from IDF file.
  entryOK = true;
  if( ! frIdf.getValues(g_desc_extInt_fmuExport_toVar, strVals, dblVals) )
    {
    entryOK = false;
    os << "IDF parsing error.";
    }
  //
  // Check count of values.
  if( entryOK
    &&
    ( 2!=strVals.size() || 1!=dblVals.size() ) )
    {
    entryOK = false;
    os << "Wrong number of entries.";
    }
  //
  // Name (variable name in IDF file).
  if( entryOK )
    {
    _toVar_epName.push_back(strVals[0]);
    // In principle, could check that the IDF file contains the corresponding
    // entry.  However, this would complicate the code here considerably.
    }
  //
  // FMU variable name (name in FMU master).
  if( entryOK )
    {
    _toVar_fmuVarName.push_back(strVals[1]);
    }
  //
  // Initial value.
  if( entryOK )
    {
    _toVar_initValue.push_back(dblVals[0]);
    }
  //
  // Check for duplicate names.
  if( entryOK )
    {
    const int elCt = (int)_toVar_idfLineNo.size();
    //
    #ifdef _DEBUG
      assert( elCt == (int)_toVar_epName.size() );
      assert( elCt == (int)_toVar_fmuVarName.size() );
      assert( elCt == (int)_toVar_initValue.size() );
    #endif
    //
    const string &epName = _toVar_epName[elCt-1];
    for( int idx=0; idx<elCt-1; ++idx )
      {
      if( 0 == epName.compare(_toVar_epName[idx]) )
        {
        entryOK = false;
        os << "FMU master already sets value of IDF variable '" << epName
          << "'. See line " << _toVar_idfLineNo[idx] << " of IDF file.";
        break;
        }
      // hoho dml  Presumably it's OK for one named value in the FMU master to
      // control more than one variable (or a variable and something else) in
      // the EnergyPlus simulation.  If not, should add a comparison of
      // {_toVar_fmuVarName[idx]} here.
      }
    }
  //
  if( ! entryOK )
    {
    _goodRead = false;
    os << "\nError encountered while reading values for keyword '" << g_key_extInt_fmuExport_toVar
      << "', starting on line " << keyLineNo << " of IDF file.";
    reportError(os);
    }
  //
  return;
  }  // End method fmuExportIdfData::handleKey_extInt_fmuExport_toVar().
