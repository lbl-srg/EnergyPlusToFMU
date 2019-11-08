//--- Handle IDF data needed to export an EnergyPlus simulation as an FMU.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <assert.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

#include "fmu-export-idf-data.h"

#include "../utility/string-help.h"

//--- Microsoft doesn't implement the modern standard.
//
#ifdef _MSC_VER
#define snprintf sprintf_s
#endif


//--- File-scope constants.
//
// Expected keywords and dictionary descriptors.
const string g_key_timeStep = "TIMESTEP";
const string g_desc_timeStep = "N";

const string g_key_output = "OUTPUT:";
//const string g_desc_timeStep = "N";

// Expected keywords and dictionary descriptors.
const string g_key_runPer = "RUNPERIOD";

// Expected keywords and dictionary descriptors.
const string g_key_idfVer = "VERSION";
const string g_desc_idfVer = "A";

//'ANNNNNNAAAAAAA', but expecting 'ANNNNAAAAAANAN'

// Expected keywords and dictionary descriptors.
const string g_key_leapYear = "HOLIDAYS/DAYLIGHT SAVINGS";
const string g_desc_leapYear = "A";

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
	assert(_runPer_numerics.empty())
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
	//0== iddMap_compareEntry(idd, g_key_timeStep, g_desc_timeStep, errStr) &&
	// We do not check the Runperiod at this point since the descriptor depends on the version 
	//0== iddMap_compareEntry(idd, g_key_runPer, g_desc_runPer, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt, g_desc_extInt, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toActuator, g_desc_extInt_fmuExport_toActuator, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toSched, g_desc_extInt_fmuExport_toSched, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_fromVar, g_desc_extInt_fmuExport_fromVar, errStr) &&
    0==iddMap_compareEntry(idd, g_key_extInt_fmuExport_toVar, g_desc_extInt_fmuExport_toVar, errStr)
    );
  }  // End method fmuExportIdfData::haveValidIDD().

///////////////////////////////////////////////////////////////////////////////
/// This function calculates the modulo of two doubles. 
///
///\param a First input double.
///\param b Second input double.
///\return The modulo of two doubles. 
///////////////////////////////////////////////////////////////////////////////
static double modulusOp(double a, double b)
{
	int result = (int)(a / b);
	return a - (double)(result)* b;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the date into seconds.
///
///
///\param day The day.
///\param month The month.
///\param leapyear The flag for leap year.
///\return The date in seconds.
///////////////////////////////////////////////////////////////////////////////
static double getSimTimeSeconds(int day, int month, int leapyear){

	double simtime;

	if (!(leapyear)) {
		if (month == 1) {
			simtime = 0;
		}
		else if (month == 2){
			simtime = 31;
		}
		else if (month == 3){
			simtime = 59;
		}
		else if (month == 4) {
			simtime = 90;
		}
		else if (month == 5) {
			simtime = 120;
		}
		else if (month == 6) {
			simtime = 151;
		}
		else if (month == 7) {
			simtime = 181;
		}
		else if (month == 8) {
			simtime = 212;
		}
		else if (month == 9) {
			simtime = 243;
		}
		else if (month == 10) {
			simtime = 273;
		}
		else if (month == 11) {
			simtime = 304;
		}
		else if (month == 12) {
			simtime = 334;
		}
		else{
			simtime = 0;
		}
	}
	else
	{
		if (month == 1) {
			simtime = 0;
		}
		else if (month == 2){
			simtime = 31;
		}
		else if (month == 3){
			simtime = 59 + 1;
		}
		else if (month == 4) {
			simtime = 90 + 1;
		}
		else if (month == 5) {
			simtime = 120 + 1;
		}
		else if (month == 6) {
			simtime = 151 + 1;
		}
		else if (month == 7) {
			simtime = 181 + 1;
		}
		else if (month == 8) {
			simtime = 212 + 1;
		}
		else if (month == 9) {
			simtime = 243 + 1;
		}
		else if (month == 10) {
			simtime = 273 + 1;
		}
		else if (month == 11) {
			simtime = 304 + 1;
		}
		else if (month == 12) {
			simtime = 334 + 1;
		}
		else{
			simtime = 0;
		}
	}

	simtime = 24 * (simtime + (day - 1));
	simtime = simtime * 3600;
	return simtime;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the month in days.
///
///
///\param month The month.
///\param leapyear The flag for leap year.
///\return The month in days.
///////////////////////////////////////////////////////////////////////////////
static int getNumDays(int month, int leapyear){

	int simtime;

	if (!(leapyear)) {
		if (month == 1) {
			simtime = 0;
		}
		else if (month == 2){
			simtime = 31;
		}
		else if (month == 3){
			simtime = 59;
		}
		else if (month == 4) {
			simtime = 90;
		}
		else if (month == 5) {
			simtime = 120;
		}
		else if (month == 6) {
			simtime = 151;
		}
		else if (month == 7) {
			simtime = 181;
		}
		else if (month == 8) {
			simtime = 212;
		}
		else if (month == 9) {
			simtime = 243;
		}
		else if (month == 10) {
			simtime = 273;
		}
		else if (month == 11) {
			simtime = 304;
		}
		else if (month == 12) {
			simtime = 334;
		}
		else{
			simtime = 0;
		}
	}
	else
	{
		if (month == 1) {
			simtime = 0;
		}
		else if (month == 2){
			simtime = 31;
		}
		else if (month == 3){
			simtime = 59 + 1;
		}
		else if (month == 4) {
			simtime = 90 + 1;
		}
		else if (month == 5) {
			simtime = 120 + 1;
		}
		else if (month == 6) {
			simtime = 151 + 1;
		}
		else if (month == 7) {
			simtime = 181 + 1;
		}
		else if (month == 8) {
			simtime = 212 + 1;
		}
		else if (month == 9) {
			simtime = 243 + 1;
		}
		else if (month == 10) {
			simtime = 273 + 1;
		}
		else if (month == 11) {
			simtime = 304 + 1;
		}
		else if (month == 12) {
			simtime = 334 + 1;
		}
		else{
			simtime = 0;
		}
	}

	return simtime;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the time in months.
///
///\param month The time.
///\param leapyear The flag for leap year.
///\return The month.
///////////////////////////////////////////////////////////////////////////////
static int getMonth(int time_s, int leapyear){

	int month;
	int tmp;
	std::ostringstream os;

	tmp = time_s / 86400;

	if (!(leapyear)) {

		if (tmp <= 31) {
			month = 1;
		}
		else if ((tmp > 31) && (tmp <= 59)){
			month = 2;
		}
		else if ((tmp > 59) && (tmp <= 90)){
			month = 3;
		}
		else if ((tmp > 90) && (tmp <= 120)){
			month = 4;
		}
		else if ((tmp > 120) && (tmp <= 151)){
			month = 5;
		}
		else if ((tmp > 151) && (tmp <= 181)){
			month = 6;
		}
		else if ((tmp > 181) && (tmp <= 212)){
			month = 7;
		}
		else if ((tmp > 212) && (tmp <= 243)){
			month = 8;
		}
		else if ((tmp > 243) && (tmp <= 273)){
			month = 9;
		}
		else if ((tmp > 273) && (tmp <= 304)){
			month = 10;
		}
		else if ((tmp > 304) && (tmp <= 334)){
			month = 11;
		}
		else if ((tmp > 334) && (tmp <= 365)){
			month = 12;
		}
		//if the time is larger than a year
		else{
			cout << "Time (" << time_s << ") set is larger than maximum allowed (365 days)."
				" Month will be set to 12." << endl;
			month = 12;
		}
	}

	else{
		if (tmp <= 31) {
			month = 1;
		}
		else if ((tmp > 31) && (tmp <= 60)){
			month = 2;
		}
		else if ((tmp > 60) && (tmp <= 91)){
			month = 3;
		}
		else if ((tmp > 91) && (tmp <= 121)){
			month = 4;
		}
		else if ((tmp > 121) && (tmp <= 152)){
			month = 5;
		}
		else if ((tmp > 152) && (tmp <= 182)){
			month = 6;
		}
		else if ((tmp > 182) && (tmp <= 213)){
			month = 7;
		}
		else if ((tmp > 213) && (tmp <= 244)){
			month = 8;
		}
		else if ((tmp > 244) && (tmp <= 274)){
			month = 9;
		}
		else if ((tmp > 274) && (tmp <= 305)){
			month = 10;
		}
		else if ((tmp > 305) && (tmp <= 335)){
			month = 11;
		}
		else if ((tmp > 335) && (tmp <= 366)){
			month = 12;
		}
		//if the time is larger than a year
		else{
			cout << "Time (" << time_s << ") set is larger than maximum allowed (366 days)."
				" Month will be set to 12." << endl;
			month = 12;
		}
	}
	return month;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the time in seconds in month.
///
///\param time_s The time in seconds.
///\param leapyear The flag for leap year.
///\return The time in months.
///////////////////////////////////////////////////////////////////////////////
static int getCurrentMonth(double time_s, int leapyear){
	int month;
	month = getMonth(time_s, leapyear);
	return month;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the time in seconds in days.
///
///
///\param time_s The time in seconds.
///\param leapyear The flag for leap year.
///\return The time in days.
///////////////////////////////////////////////////////////////////////////////
static int getCurrentDay(double time_s, int month, int leapyear){

	int day;
	int num_days;
	double tmp = 365 * 24 * 3600;
	double tmpp = 366 * 24 * 3600;

	num_days = getNumDays(month, leapyear);
	if ((time_s == 0) || (int)(time_s / 86400)< 1){
		cout << "Time (" << time_s << ") set is smaller than minimun allowed (1 day)."
			" Day will be set to 1." << endl;
		day = 1;
	}

	else{
		if (!(leapyear)) {
			if (modulusOp(time_s, tmp) == 0) {
				day = 31;
			}
			else{
				if (time_s > tmp)
				{
					cout << "Time (" << time_s << ") set is larger than maximum allowed (365 days)."
						" Day will be set to 31." << endl;
					day = 31;
				}
				else
				{
					day = (int)(time_s / 86400) - num_days;
				}
			}
		}
		else{
			if (modulusOp(time_s, tmpp) == 0) {
				day = 31;
			}
			else{
				if (time_s > tmpp)
				{
					cout << "Time (" << time_s << ") set is larger than maximum allowed (366 days)."
						" Day will be set to 31." << endl;
					day = 31;
				}
				else {
					day = (int)(time_s / 86400) - num_days;
				}
			}
		}
	}
	return day;
}

///////////////////////////////////////////////////////////////////////////////
/// This function gets the current day of the week.
///
///
///\param t_start_idf The start time in the IDF.
///\param t_start_fmu The start time in the FMU.
///\param fname The filename to extract idf information.
///\param fname The filename to write new date information.
///\return 0 if no error occurred.
///////////////////////////////////////////////////////////////////////////////
static int getCurrentDayOfWeek(double t_start_idf, double t_start_fmu, 
	string day_of_week, char *new_day_week){
		int modDat;
		char arr[7][10]={ "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };
		int new_index;
		int old_index;
		int change=1;

		// deternmine the difference between start time in idf and start time in fmu
		modDat=(((int)(t_start_fmu  - t_start_idf)/86400)%86400)%7;

		capitalize(day_of_week);
		if (day_of_week.compare("SUNDAY")==0) 
		{
			old_index=0;
		} 
		else if (day_of_week.compare("MONDAY")==0)
		{
			old_index=1;
		}
		else if (day_of_week.compare("TUESDAY")==0)
		{
			old_index= 2;
		}
		else if (day_of_week.compare("WEDNESDAY")==0)
		{
			old_index=3;
		}
		else if (day_of_week.compare("THURSDAY")==0)
		{
			old_index=4;
		}
		else if (day_of_week.compare("FRIDAY")==0)
		{
			old_index=5;
		}

		else if (day_of_week.compare("SATURDAY")== 0)
		{
			old_index=6;
		}

		else if (day_of_week.compare("USEWEATHERFILE")== 0)
		{
			change=0;
			cout << "Day of week: UseWeatherFile has been specified and will be used." << endl;
            sprintf(new_day_week, "%s", "USEWEATHERFILE");
			return 0;
		}
		else
		{
			// write the new day of week, no day of week was specified.
			sprintf(new_day_week, "%s", " ");
			cout << "Day of week was left blank in input file." << endl;
			return 0;
		}
		if (change !=0) {
			// determine the new index
			if (modDat > 0)
			{
				new_index=(old_index + modDat)%7; 
			}
			else if (modDat < 0)
			{
				new_index=modDat + 7;
				new_index=(new_index + old_index)%7;
			}
			else
			{
				new_index=old_index;
			}
			// write the new day of week
			sprintf(new_day_week, "%s", arr[new_index]);
		}

		// close file
		return 0;
}

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

//--- Read IDF file, collecting data needed to run an EnergyPlus simulation as an FMU.
//
int fmuExportIdfData::writeInputFile(fileReaderData& frIdf, int leapYear, int idfVer, string tStartFMU, string tStopFMU)
{
	//
	int lineNo;
	int nRunPer;
	string inputKey, iddDesc, inputKeyExt;
	string line;
	ofstream runInfile;
	//
#ifdef _DEBUG
	assert(!frIdf.isEOF());
#endif
	//
	// Initialize.
	lineNo = 0;
	nRunPer = 0;
	//nTStep = 0;
	_goodRead = true;
	//
	// Run through the IDF file.
#define HS_MAX 10

	runInfile.open("runinfile.idf");
	char valueStr[HS_MAX];
	while (_goodRead)
	{
		frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
		//frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, inputKey, inputKeyExt);
		frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, inputKey, inputKeyExt);
		// Consume the delimiter that caused getToken() to return.
		char delimChar = frIdf.getChar();		
		capitalize(inputKey);

		// handle all Output: explicitely to make sure that we do not 
		// get a runperiod which we shouldn't be getting.
		// key RunPeriod is only used in Output: thus we can handle them 
		// exactly and preven them to be used later one.
		if (inputKey.find(g_key_output) != string::npos){
			// write token till we reach end of 
			runInfile << inputKeyExt << '\n';
			// obtained from the scripts.
			while (';' != delimChar)
			{
				// Here, hit EOF.
				frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
				frIdf.getToken(IDF_DELIMITERS_ALL, IDF_COMMENT_CHARS, inputKey, inputKeyExt);
				runInfile << inputKeyExt << '\n';
				delimChar = frIdf.getChar();
			}
		}

		// handle RunPeriod
		else if ((0 == g_key_runPer.compare(inputKey)) && !(inputKey.find(g_key_output) != string::npos)){
				nRunPer++;
				if (nRunPer < 2){
					// FMU start time
					double t_start_fmu = 0.0;
					double t_start_fmuDofW = 0.0;
					double t_stop_fmu = 86400.0;
					if (idfVer < 9) {
						// g_desc_runPer_idf = "ANNNNAAAAAANAN";
						handleKey_runPer(frIdf, idfVer);
						std::string runPeriod("RUNPERIOD, \n");
						if (_runPer_strings.size() > 1) {
							runPeriod.append(_runPer_strings[0]);
							runPeriod.append(",\n");
						}

						istringstream(tStartFMU) >> t_start_fmu;
						istringstream(tStopFMU) >> t_stop_fmu;

						// Save original FMU start time to be used to determine the day of the week
						t_start_fmuDofW = t_start_fmu;

						// Change the start time so we compute the correct time.
						if (t_start_fmu >= 86400) t_start_fmu = t_start_fmu + 86400;

						// get the start month
						int begMonth = getCurrentMonth(t_start_fmu, leapYear);
						snprintf(valueStr, HS_MAX, "%d", begMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the Begin Month: " << begMonth << endl;

						// get the day of the month 
						int begDayMonth = getCurrentDay(t_start_fmu, begMonth, leapYear);
						snprintf(valueStr, HS_MAX, "%d", begDayMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the Day of the Begin Month: " << begDayMonth << endl;

						// get the end month
						int endMonth = getCurrentMonth(t_stop_fmu, leapYear);
						snprintf(valueStr, HS_MAX, "%d", endMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the End Month: " << endMonth << endl;

						// get the day of the month
						int endDayMonth = getCurrentDay(t_stop_fmu, endMonth, leapYear);
						snprintf(valueStr, HS_MAX, "%d", endDayMonth);
						runPeriod.append(valueStr);

						cout << "This is the Day of the End Month: " << endDayMonth << endl;

						// get the idf start time in seconds
						double t_start_idf = getSimTimeSeconds(_runPer_numerics[1], _runPer_numerics[0], 0);
						char new_day_week[20];
						int retVal = getCurrentDayOfWeek(t_start_idf, t_start_fmuDofW, _runPer_strings[1], new_day_week);

						cout << "This is the New Day of Week: " << new_day_week << endl;

						if (_runPer_strings.size() > 6) {
							runPeriod.append(",\n");
							// write new day of the week
							runPeriod.append(new_day_week);
							runPeriod.append(",\n");
							runPeriod.append(_runPer_strings[2] + ",\n");
							runPeriod.append(_runPer_strings[3] + ",\n");
							runPeriod.append(_runPer_strings[4] + ",\n");
							runPeriod.append(_runPer_strings[5] + ",\n");
							runPeriod.append(_runPer_strings[6]);
						}
						if (_runPer_numerics.size() > 4) {
							cout << "The field **Number of Times Runperiod to be Repeated**"
								"  of the RunPeriod object is ignored. This entry will be set to its default." << endl;
							runPeriod.append(",\n");
							//snprintf(valueStr, HS_MAX, "%d", (int)_runPer_numerics[4]);
							runPeriod.append(" ");

						}
						if (_runPer_strings.size() > 7) {
							cout << "The field **Increment Day of Week on repeat**"
								" of the RunPeriod object is ignored. This entry will be set to its default." << endl;
							runPeriod.append(",\n");
							//runPeriod.append(_runPer_strings[7]);
							runPeriod.append(" ");
						}
						if (_runPer_numerics.size() > 5) {
							cout << "The field **Start Year** of the RunPeriod object is ignored."
								" This entry will be set to its default." << endl;
							runPeriod.append(",\n");
							runPeriod.append(" ");
						}
						runPeriod.append(";\n");
						runInfile << runPeriod;
					}
					else {
						// g_desc_runPer_idf = "ANNNNNNAAAAAAA";
						string startYear;
						string endYear;

						istringstream(tStartFMU) >> t_start_fmu;
						istringstream(tStopFMU) >> t_stop_fmu;

						istringstream(startYear) >> _runPer_numerics[2];
						istringstream(endYear) >> _runPer_numerics[5];

						handleKey_runPer(frIdf, idfVer);
						std::string runPeriod("RUNPERIOD, \n");
						if (_runPer_strings.size() > 1) {
							runPeriod.append(_runPer_strings[0]);
							runPeriod.append(",\n");
						}
						// FMU start time
						double t_start_fmu = 0.0;
						double t_start_fmuDofW = 0.0;
						double t_stop_fmu = 86400.0;

						istringstream(tStartFMU) >> t_start_fmu;
						istringstream(tStopFMU) >> t_stop_fmu;

						// Save original FMU start time to be used to determine the day of the week
						t_start_fmuDofW = t_start_fmu;

						// Change the start time so we compute the correct time.
						if (t_start_fmu >= 86400) t_start_fmu = t_start_fmu + 86400;

						// get the start month
						int begMonth = getCurrentMonth(t_start_fmu, leapYear);
						snprintf(valueStr, HS_MAX, "%d", begMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the Begin Month: " << begMonth << endl;

						// get the day of the month 
						int begDayMonth = getCurrentDay(t_start_fmu, begMonth, leapYear);
						snprintf(valueStr, HS_MAX, "%d", begDayMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the Day of the Begin Month: " << begDayMonth << endl;

						// get the start year
						if (_runPer_numerics[2] != 0) {
							snprintf(valueStr, HS_MAX, "%d", (int)_runPer_numerics[2]);
							runPeriod.append(valueStr);
						}
						else {
							runPeriod.append("");
						}
						runPeriod.append(",\n");

						// get the end month
						int endMonth = getCurrentMonth(t_stop_fmu, leapYear);
						snprintf(valueStr, HS_MAX, "%d", endMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the End Month: " << endMonth << endl;

						// get the day of the month
						int endDayMonth = getCurrentDay(t_stop_fmu, endMonth, leapYear);
						snprintf(valueStr, HS_MAX, "%d", endDayMonth);
						runPeriod.append(valueStr);
						runPeriod.append(",\n");

						cout << "This is the Day of the End Month: " << endDayMonth << endl;

						// get the end year
						if (_runPer_numerics[5] != 0) {
							snprintf(valueStr, HS_MAX, "%d", (int)_runPer_numerics[5]);
							runPeriod.append(valueStr);
						}
						else {
							runPeriod.append("");
						}
						runPeriod.append(",\n");

						// get the idf start time in seconds
						double t_start_idf = getSimTimeSeconds(_runPer_numerics[1], _runPer_numerics[0], 0);
						char new_day_week[20];
						int retVal = getCurrentDayOfWeek(t_start_idf, t_start_fmuDofW, _runPer_strings[1], new_day_week);

						cout << "This is the New Day of Week: " << new_day_week << endl;

						if (_runPer_strings.size() > 5) {
							// write new day of the week
							runPeriod.append(new_day_week);
							runPeriod.append(",\n");
							runPeriod.append(_runPer_strings[2] + ",\n");
							runPeriod.append(_runPer_strings[3] + ",\n");
							runPeriod.append(_runPer_strings[4] + ",\n");
							runPeriod.append(_runPer_strings[5] + ",\n");
						}

						if (_runPer_strings.size() == 7) {
							//runPeriod.append(_runPer_strings[6] + ",\n");
							runPeriod.append(_runPer_strings[6]);
						}
						else if (_runPer_strings.size() == 8) {
							runPeriod.append(_runPer_strings[6] + ",\n");
							runPeriod.append(_runPer_strings[7]);
						}
						runPeriod.append(";\n");
						runInfile << runPeriod;

					}
				}
				else{
					cout << "There is more than one RunPeriod(" << nRunPer << ") in the IDF file."
						" The first RunPeriod will be considered. Other RunPeriods will be ignored." << endl;
				}
		}
		else{
			runInfile << inputKeyExt << '\n';
		}
		if (frIdf.isEOF())
		{
			// Here, hit EOF.
			//   OK to hit EOF, provided don't actually have a keyword.
			if (0 != inputKey.length())
			{
				_goodRead = false;
				std::ostringstream os;
				os << "Error: IDF file ends after keyword '" << inputKey << "' on line " << lineNo;
				reportError(os);
			}
			break;
		}
		// Here, ready to look for next keyword.
	}

	// Here, ran through whole IDF file.
	frIdf.close();
	runInfile.close();
	//
	if (_goodRead)
	{
		lineNo = 0;
	}
#undef HS_MAX
	return(lineNo);
}  // End method fmuExportIdfData::writeInputFile().

//--- Read Weather file, collecting data needed to run an EnergyPlus simulation as an FMU.
//
int fmuExportIdfData::isLeapYear(fileReaderData& frIdf, int &leapYear)
{
	//
	int lineNo;
	int nLeapYear;
	string inputKey, iddDesc;
	string line, inputKeyExt;
	ofstream runWeafile;
	runWeafile.open("runweafile.epw");
	//
#ifdef _DEBUG
	assert(!frIdf.isEOF());
#endif
	//
	// Initialize.
	lineNo = 0;
	leapYear = 0;
	nLeapYear = 0;
	_goodRead = true;
	//
	// Run through the IDF file.
	while (_goodRead)
	{
		frIdf.getToken(",", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
		// Consume the delimiter that caused getToken() to return.
		const char delimChar = frIdf.getChar();
		capitalize(inputKey);
		if (inputKey.find(g_key_leapYear) != string::npos){
			nLeapYear++;
			runWeafile << inputKeyExt;
			frIdf.getToken(",", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
			capitalize(inputKey);
			if (0== inputKey.compare("YES")){
				leapYear = 1;
			}
		}
		// Write weather file
		runWeafile << inputKeyExt;
		if (frIdf.isEOF())
		{
			// Here, hit EOF.
			//   OK to hit EOF, provided don't actually have a keyword.
			if (nLeapYear == 0){
				cout << "Finish reading weather file without finding leap year indicator." << endl;
			}
			else{
				cout << "Successfully finish reading weather file." << endl;
			}
			break;
		}
		// Here, ready to look for next keyword.
	}

	// Here, ran through whole IDF file.
	frIdf.close();
	runWeafile.close();
	//
	if (_goodRead)
	{
		lineNo = 0;
	}
	return(lineNo);
}  // End method fmuExportIdfData::isLeapYear().

//--- Read IDF file, collecting data needed to run an EnergyPlus simulation as an FMU.
//
int fmuExportIdfData::getTimeStep(fileReaderData& frIdf)
{
	//
	int lineNo;
	int nTStep;
	//int leapYear;
	string inputKey, iddDesc;
	string line, inputKeyExt;
	ofstream tStepfile;
	tStepfile.open("tstep.txt");
	//
#ifdef _DEBUG
	assert(!frIdf.isEOF());
#endif
	//
	// Initialize.
	lineNo = 0;
	nTStep = 0;
	_goodRead = true;
	//
	// Run through the IDF file.
	while (_goodRead)
	{
		frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
		frIdf.getToken(",", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
		// Consume the delimiter that caused getToken() to return.
		const char delimChar = frIdf.getChar();
		capitalize(inputKey);
		if (inputKey.find(g_key_timeStep) != string::npos){
			nTStep++;
			frIdf.skipComment(IDF_COMMENT_CHARS, lineNo);
			frIdf.getToken(";", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
			capitalize(inputKey);
			if ((inputKey.find(",") != string::npos)){
				//this is not the correct timestep
				continue;
			}
			else{
				tStepfile << inputKey;
				break;
			}
		}
		if (frIdf.isEOF())
		{
			break;
		}
		// Here, ready to look for next keyword.
	}

	// Report if we couldn't find a time step in the file.
	if (nTStep==0){
		_goodRead = false;
		std::ostringstream os;
		os << "Error: There is no TimeStep object in the IDF input file";
		reportError(os);
	}

	// Here, ran through whole IDF file.
	frIdf.close();
	tStepfile.close();
	//
	if (_goodRead)
	{
		lineNo = 0;
	}
#undef HS_MAX
	return(lineNo);
}  // End method fmuExportIdfData::getTimeStep().


   //--- Read version of the IDF file, collecting data needed to run an EnergyPlus simulation as an FMU.
   //
int fmuExportIdfData::getIDFVersion(fileReaderData& frIdf, int &idfVersion)
{
	//
	int lineNo;
	string inputKey, iddDesc;
	string line, inputKeyExt;
	//string idfVer

	//
#ifdef _DEBUG
	assert(!frIdf.isEOF());
#endif
	//
	// Initialize.
	lineNo = 0;
	_goodRead = true;
	//
	// Run through the IDF file.
	while (_goodRead)
	{
		frIdf.getToken(",", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
		// Consume the delimiter that caused getToken() to return.
		const char delimChar = frIdf.getChar();
		capitalize(inputKey);
		if (inputKey.find(g_key_idfVer) != string::npos) {
			frIdf.getToken(",", IDF_COMMENT_CHARS, inputKey, inputKeyExt);
			capitalize(inputKey);
			//idfVer.assign(inputKeyExt);
			idfVersion = inputKeyExt[0] - '0';
			cout << "The IDF version found is :" << idfVersion << endl;
		}
		if (frIdf.isEOF())
		{
			// Here, hit EOF.
			break;
		}
		// Here, ready to look for next keyword.
	}

	// Here, ran through whole IDF file.
	frIdf.close();
	//
	if (_goodRead)
	{
		lineNo = 0;
	}
	return(lineNo);
}  // End method fmuExportIdfData::getIDFVersion().



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


////--- Read IDF values for key {g_key_timeStep}.
//// Timestep, 4;
////
//int fmuExportIdfData::handleKey_timeStep(fileReaderData& frIdf)
//{
//	bool entryOK;
//	vString strVals;
//	vDouble dblVals;
//	std::ostringstream os;
//	//
//	// Assume just read key {g_key_timeStep} from the IDF file.
//	const int keyLineNo = frIdf.getLineNumber();
//	_fromVar_idfLineNo.push_back(keyLineNo);
//	//
//	// Read values from IDF file.
//	entryOK = true;
//	if (!frIdf.getValues(g_desc_timeStep, strVals, dblVals))
//	{
//		entryOK = false;
//		os << "IDF parsing error.";
//	}
//	//
//	// Check count of values.
//	if (entryOK
//		&&
//		(1 != dblVals.size()))
//	{
//		entryOK = false;
//		os << "Wrong number of entries.";
//	}
//	//
//	// TimeStep value (key name in IDF file).
//	if (entryOK)
//	{
//		_timeStep.push_back(dblVals[0]);
//		// In principle, could check that the IDF file contains the corresponding
//		// entry.  However, this would complicate the code here considerably.
//	}
//	//
//
//	if (!entryOK)
//	{
//		_goodRead = false;
//		cout << "\nError encountered while reading values for keyword '" << g_key_timeStep
//			<< "', starting on line " << keyLineNo << " of IDF file.";
//		return 1;
//		//reportError(os);
//	}
//	//
//	return 0;
//}  // End method fmuExportIdfDa

//--- Read IDF values for key {g_key_runPer}.
//RunPeriod for EnergyPlus version < 9,//
//, !- Name
//1, !- Begin Month
//1, !- Begin Day of Month
//12, !- End Month
//31, !- End Day of Month
//, !- Day of Week for Start Day
//, !- Use Weather File Holidays and Special Days
//, !- Use Weather File Daylight Saving Period
//, !- Apply Weekend Holiday Rule
//, !- Use Weather File Rain Indicators
//, !- Use Weather File Snow Indicators
//, !- Number of Times Runperiod to be Repeated
//, !- Increment Day of Week on repeat
//; !- Start Year


//RunPeriod for EnergyPlus version >= 9,//
//,     !- Name
//1,    !- Begin Month
//1,    !- Begin Day of Month
//2002, ! - Start Year
//12,   !- End Month
//31,   !- End Day of Month
//2002, ! - Start Year
//,     !- Day of Week for Start Day
//,     !- Use Weather File Holidays and Special Days
//,     !- Use Weather File Daylight Saving Period
//,     !- Apply Weekend Holiday Rule
//,     !- Use Weather File Rain Indicators
//,     !- Use Weather File Snow Indicators
//;     !- Treat Weather as Actual


void fmuExportIdfData::handleKey_runPer(fileReaderData& frIdf, int idfVer)
{
	bool entryOK;
	vString strVals;
	vDouble dblVals;
	string g_desc_runPer;
	std::ostringstream os;
	//
	// Assume just read key {g_key_runPer} from the IDF file.
	const int keyLineNo = frIdf.getLineNumber();
	_fromVar_idfLineNo.push_back(keyLineNo);

	// determine the descriptor based on the IDF version number.
	if (idfVer < 9) {
		// Runperiod descriptor for E+ version < 9
		g_desc_runPer = "ANNNNAAAAAANAN";

	}
	else {
		// Runperiod descriptor for E+ version >= 9
		g_desc_runPer = "ANNNNNNAAAAAAA";
	}
	
	// Read values from IDF file.
	entryOK = true;
	if (!frIdf.getValues(g_desc_runPer, strVals, dblVals))
	{
		entryOK = false;
		os << "IDF parsing error.";
	}

	// Check count of values.
	if (entryOK && ((dblVals.size() < 4)))
	{
		entryOK = false;
		os << "Wrong number of entries. The number of numeric "
			<< "entries must be higher than 3. Now it is " << dblVals.size();
	}
	
	// get the proper version and set the decriptor according to the version
	// Name of the RunPeriod.
	if (strVals.size() > 1){
		if (entryOK)
		{
			_runPer_strings.push_back(strVals[0]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}
	}
	// The next sections are required field so they must be present in the IDF
	if (idfVer < 9) {

		// Begin Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[0]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// Begin Day of Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[1]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// End Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[2]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// End Day of Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[3]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}
		if (strVals.size() > 6)
		{
			// Day of Week for Start Day.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[1]);
				// In principle, could check that the IDF file contains the corresponding
				// entry.  However, this would complicate the code here considerably.
			}

			// Use Weather File Holidays and Special Days.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[2]);
				// In principle, could check that the IDF file contains the corresponding
				// entry.  However, this would complicate the code here considerably.
			}

			//
			// Use Weather File Daylight Saving Period.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[3]);
			}

			//
			// Apply Weekend Holiday Rule.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[4]);
			}

			//
			// Use Weather File Rain Indicators.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[5]);
			}

			//
			// Use Weather File Snow Indicators.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[6]);
			}

		}
		if (dblVals.size() > 4) {
			//
			// Number of Times Runperiod to be Repeated.
			if (entryOK)
			{
				_runPer_numerics.push_back(dblVals[4]);
			}
		}

		if (strVals.size() > 7)
		{
			//
			// Increment Day of Week on repeat.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[7]);
			}
		}

		if (dblVals.size() > 5) {
			//
			// Start Year.
			if (entryOK)
			{
				_runPer_numerics.push_back(dblVals[5]);
			}
		}
	}
	else {

		// Begin Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[0]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// Begin Day of Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[1]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// Begin Year.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[2]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// End Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[3]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// End Day of Month.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[4]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		// End Year.
		if (entryOK)
		{
			_runPer_numerics.push_back(dblVals[5]);
			// In principle, could check that the IDF file contains the corresponding
			// entry.  However, this would complicate the code here considerably.
		}

		if (strVals.size() > 6)
		{
			// Day of Week for Start Day.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[1]);
				// In principle, could check that the IDF file contains the corresponding
				// entry.  However, this would complicate the code here considerably.
			}

			// Use Weather File Holidays and Special Days.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[2]);
				// In principle, could check that the IDF file contains the corresponding
				// entry.  However, this would complicate the code here considerably.
			}

			//
			// Use Weather File Daylight Saving Period.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[3]);
			}

			//
			// Apply Weekend Holiday Rule.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[4]);
			}

			//
			// Use Weather File Rain Indicators.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[5]);
			}

			//
			// Use Weather File Snow Indicators.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[6]);
			}
		}
		if (strVals.size() > 7)
		{
			//
			// Treat Weather as Actual.
			if (entryOK)
			{
				_runPer_strings.push_back(strVals[7]);
			}
		}
	}
	//
	if (!entryOK)
	{
		_goodRead = false;
		os << "\nError encountered while reading values for keyword '" << g_key_runPer
			<< "', starting on line " << keyLineNo << " of IDF file.";
		reportError(os);
	}
	
	return;
}  // End method fmuExportIdfData::handleKey_runPer().