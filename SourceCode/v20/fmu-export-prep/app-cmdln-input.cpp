//--- Get inputs for command-line app to export an EnergyPlus simulation as an FMU.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <assert.h>

#include <ostream>
using std::ostream;
#include <iostream>
using namespace std;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include "app-cmdln-input.h"

#include "app-cmdln-version.h"


//--- Preprocessor definitions.


//--- Types.


//--- Global variables.


//--- File-scope fcn prototypes.
//
void cmdlnInput_showVersion(ostream& toStream, const char *const progName);
void cmdlnInput_showHelp(ostream& toStream, const char *const progName);
//
static void reportError(void (*errFcn)(std::ostringstream& errorMessage),
  std::ostringstream& errorMessage);


//--- Functions.


////--- Get command-line inputs.
////
  bool cmdlnInput_get(const int argc, const char *argv[],
	  cmdlnInput_s *const cmdlnInputP,
	  void(*errFcn)(std::ostringstream& errorMessage))
  {
	  bool showVersion;
	  bool showHelp;
	  bool haveInputs;
	  int cmdIdx;
	  //
#ifdef _DEBUG
	  assert(0 < argc);
	  assert(NULL != argv);
	  assert(NULL != cmdlnInputP);
#endif
	  //
	  // Initialize.
	  cmdlnInputP->iddFileName = NULL;
	  cmdlnInputP->idfFileName = NULL;
	  cmdlnInputP->wthFileName = NULL;
	  cmdlnInputP->tStartFMU = NULL;
	  cmdlnInputP->tStopFMU = NULL;
	  //
	  showVersion = 0;
	  showHelp = 0;
	  haveInputs = 1;
	  //
	  // Get any command-line switches.
	  //   Note argv[0] is program name, so start at argv[1].
	  cmdIdx = 1;
	  while (cmdIdx < argc)
	  {
		  const char *const cmdStr = argv[cmdIdx];
		  // Command-line switch starts with "-".
		  if ('-' != cmdStr[0])
			  break;
		  switch (cmdStr[1])
		  {
		  case 'v':
			  // Note don't report error for over-long input like '-version'.
			  showVersion = 1;
			  break;
		  case 'h':
			  showHelp = 1;
			  break;
		  case 'w':
			  // Next argument names the weather file.
			  //   Note can't have e.g. "-wWeatherFileName".
			  ++cmdIdx;
			  if (cmdIdx < argc)
			  {
				  cmdlnInputP->wthFileName = argv[cmdIdx];
			  }
			  else
			  {
				  std::ostringstream os;
				  os << "Missing weather file name";
				  reportError(errFcn, os);
				  haveInputs = 0;
			  }
			  break;
		  case 'b':
			  // Next argument names the weather file.
			  //   Note can't have e.g. "-wWeatherFileName".
			  ++cmdIdx;
			  if (cmdIdx < argc)
			  {
				  cmdlnInputP->tStartFMU = argv[cmdIdx];
			  }
			  else
			  {
				  std::ostringstream os;
				  os << "Missing FMU simulation start time in seconds";
				  reportError(errFcn, os);
				  haveInputs = 0;
			  }
			  break;

		  case 'e':
			  // Next argument names the weather file.
			  //   Note can't have e.g. "-wWeatherFileName".
			  ++cmdIdx;
			  if (cmdIdx < argc)
			  {
				  cmdlnInputP->tStopFMU = argv[cmdIdx];
			  }
			  else
			  {
				  std::ostringstream os;
				  os << "Missing FMU simulation stop time in seconds";
				  reportError(errFcn, os);
				  haveInputs = 0;
			  }
			  break;
		  default:
			  std::ostringstream os;
			  os << "Unknown command-line switch '" << cmdStr << "'";
			  reportError(errFcn, os);
			  // Treat this as a call for help, not a hard error.
			  showHelp = 1;
			  break;
		  }
		  //
		  // Here, done handling command-line argument {cmdIdx}.
		  ++cmdIdx;
	  }
	  //
	  // Here, done reading command-line switches.
	  //
	  // Get command-line arguments.
	  if (2 == argc - cmdIdx)
	  {
		  cmdlnInputP->iddFileName = argv[cmdIdx];
		  cmdlnInputP->idfFileName = argv[cmdIdx + 1];
	  }
	  else if (haveInputs)
	  {
		  // Here, got wrong number of command-line arguments, and did not already
		  // issue an error message.
		  haveInputs = 0;
		  // Report error-- unless caller asked for version or help, with no other args.
		  if (argc != cmdIdx || (!showVersion && !showHelp))
		  {
			  std::ostringstream os;
			  os << "Program " << argv[0] <<
				  " requires exactly 2 command-line arguments, got " << argc - cmdIdx;
			  reportError(errFcn, os);
			  showHelp = 1;
		  }
	  }
	  //
	  // Handle any special outputs.
	  if (showHelp)
	  {
		  cmdlnInput_showHelp(cout, argv[0]);
	  }
	  else if (showVersion)
	  {
		  // Note no need to show version if already showed help.
		  cmdlnInput_showVersion(cout, argv[0]);
	  }
	  //
	  return(haveInputs);
  }  // End fcn cmdlnInput_get().

//--- Show program name and version.
//
void cmdlnInput_showVersion(ostream& toStream, const char *const progName)
  {
  toStream << progName << " version " << gp_cmdln_versionStr << endl;
  }  // End fcn cmdlnInput_showVersion().


//--- Show valid command-line options.
//
void cmdlnInput_showHelp(ostream& toStream, const char *const progName)
  {
  cmdlnInput_showVersion(toStream, progName);
  toStream << "Prepare to export an EnergyPlus input file for use as an FMU" << endl;
  toStream << "Usage:\n " << progName << " [-h] [-v] [-w weatherFile] "
	  "[-b simulation starttime] [-e simulation stoptime] iddFile idfFile" << endl;
  toStream << " -h: show this help message\n -v: show version information" << endl;
  }  // End fcn cmdlnInput_showHelp().


//--- Report an error.
//
static void reportError(void (*errFcn)(std::ostringstream& errorMessage),
  std::ostringstream& errorMessage)
  {
  //
  // Call user-supplied error fcn if available.
  if( errFcn )
    {
    (*errFcn)(errorMessage);
    }
  else
    {
    // Here, no user-supplied error fcn.
    //   Note flush both {cout} and {cerr}, to avoid overlapped writes.
    cout.flush();
    cerr << "=== Error reading command line ===" << endl
      << errorMessage.str() << endl << endl;
    cerr.flush();
    }
  }  // End fcn fileReader::reportError().
