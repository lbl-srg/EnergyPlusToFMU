// Methods for Functional Mock-up Unit Export of EnergyPlus.
/////////////////////////////////////////////////////////////////////
// \file   main.c
//
// \brief  FMI functions for FMU export project.
//
// \author Thierry Stephane Nouidui
//         Nouidui Consulting Group,
//         nouidui.consulting@gmail.com
//
// \date   2020-01-10
//
/////////////////////////////////////////////////////////////////////

// define the model identifier name used in for
// the FMI functions
#define MODEL_IDENTIFIER SmOffPSZ
// define the FMI version supported
#define FMIVERSION "3.0"
#define NUMFMUsMax 10000
#define PATHLEN 10000
#define MAXHOSTNAME  10000
#define MAX_MSG_SIZE 1000
#define MAXBUFFSIZE 1000
#define MAX_NUM_INPUTS 100000
//#define LIBXML_STATIC 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../utility/util.h"
#include "../../socket/utilSocket.h"
#include "defines.h"
#include <errno.h>
#include <sys/stat.h>

// Addition for FMI 2.0 support
#include "../fmusdk-shared/include/FMI3.h"
//#include "../fmusdk-shared/sim_support.h"
#include "../fmusdk-shared/xmlVersionParser.h"
#include "../fmusdk-shared/include/cosimulation.h"

#ifdef _MSC_VER
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "ws2_32.lib")
// #include <vld.h>
#include <process.h>
#include <windows.h>
#include <direct.h>
#include "dirent_win.h"
#else
#include <dirent.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <sys/types.h> /* pid_t */
#include <sys/wait.h>
#include <sys/ioctl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#endif

int zI=0;
int insNum=0;
int firstCallIns=1;

int arrsize=0;
ModelInstance **fmuInstances;
int fmuLocCoun=0;
#define DELTA 10


/////////////////////////////////////////////////////////////////
/////  This method is return a log 
/////
/////\param ModelDescription FMU model description file
//////////////////////////////////////////////////////////////////
//void logError(ModelInstance* comp, const char* message, ...) {
//
//	if (!comp || !comp->logErrors) return;
//
//	va_list args;
//	va_start(args, message);
//	logMessage(comp, Error, "logStatusError", message, args);
//	va_end(args);
//}

///////////////////////////////////////////////////////////////
///  This method is used to get the number of outputs in the FMU
///
///\param ModelDescription FMU model description file
////////////////////////////////////////////////////////////////
void getNumInputOutputVariablesInFMU(ModelDescription *md, int* inp, int* out) {
	int k;

	int n = getScalarVariableSize(md);
	for (k = 0; k < n; k++) {
		ScalarVariable *svTemp = getScalarVariable(md, k);
		//if (getAlias(svTemp)!=enu_noAlias) continue;
		if (getCausality(svTemp) == enu_input) {
			*inp = *inp + 1;
		}
		else if (getCausality(svTemp) == enu_output) {
			*out = *out + 1;
		}
		else continue;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function deletes temporary created files.
///////////////////////////////////////////////////////////////////////////////
int findFileDelete()
{
	struct stat stat_p;
	int res;

	if (stat(VARCFG, &stat_p) >= 0)
	{
		remove(VARCFG);
	}
	if (stat(SOCKCFG, &stat_p) >= 0)
	{
		remove(SOCKCFG);
	}
	if (stat(EPBAT, &stat_p) >= 0)
	{
		remove(EPBAT);
	}
	if (stat(FTIMESTEP, &stat_p) >= 0)
	{
		remove(FTIMESTEP);
	}
	if (stat(FRUNWEAFILE, &stat_p) >= 0){
		// cleanup .epw files
#ifdef _MSC_VER
		res = system("del *.epw");
#else
		res = system("rm -f *.epw");
#endif
	}
	return 0;
}

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

////////////////////////////////////////////////////////////////////////////////////
/// Replace forward slash with backward slash
///
///\param s The input string.
///\param s The character to find.
///\param s The character to replace.
////////////////////////////////////////////////////////////////////////////////////
int replace_char (char *s, const char find, const char replace) {
	while (*s !=0) {
		if (*s==find)
			*s=replace;
		s++;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// create a list of pointer to FMUs
///
///\param s The Pointer to FMU.
////////////////////////////////////////////////////////////////////////////////////
void addfmuInstances(ModelInstance* s){
	ModelInstance **temp;
	if(fmuLocCoun==arrsize){
		temp=(ModelInstance**)malloc(sizeof(ModelInstance*) * (DELTA + arrsize));
		arrsize +=DELTA;
		memcpy(temp, fmuInstances, fmuLocCoun);
		free(fmuInstances);
		fmuInstances=temp;
	}
	fmuInstances[fmuLocCoun++]=s;
}

///////////////////////////////////////////////////////////////////////////////
/// This function finds a file with a specific extension in a folder.
/// It returns the name of the found file with its extension.
///
///\param path The path to a file.
///\param pattern The pattern to search.
///\return The name of the file found with extension.
///        Otherwise, return 1 to indicate error.
///////////////////////////////////////////////////////////////////////////////
int findNameFile(ModelInstance * _c, char* in_file, fmi3String pattern)
{
	int found = 0;
	char name[MAXBUFFSIZE];
	DIR *dirp = opendir(_c->fmuResourceLocation);
	struct dirent entry;
	struct dirent *dp = &entry;
	// read directory
	logEvent(_c,
		"Searching for following pattern %s\n", pattern);
	while ((dp = readdir(dirp)))
	{
		logEvent(_c,
			"Read directory and search for *.idf, *.epw, or *.idd file.\n");
		// search pattern the filename
		if ((strstr(dp->d_name, pattern)) != 0)
		{
			logEvent(_c, "Found matching file %s.\n", dp->d_name);
			found++;
			strcpy(name, dp->d_name);
			// copy filename to be returned
			strncpy(in_file, name, strlen(name));
			//close directory
			closedir(dirp);
			return found;
		}
	}
	closedir(dirp);
	return found;
}

///////////////////////////////////////////////////////////////////////////////
/// This function finds a file with a specific extension in a folder.
/// It returns the name of the found file with its extension.
///
///\param file The input file.
///\param path The path to a file.
///\param pattern The pattern to search.
///\return The name of the file found with extension.
///        Otherwise, return 1 to indicate error.
///////////////////////////////////////////////////////////////////////////////

void logEvent(ModelInstance* comp, const char* message, ...);

int getResFile(ModelInstance *_c, fmi3String pattern)
{
	char in_file[MAXBUFFSIZE] = { 0 };
	int found;
	logEvent(_c,
		"Get input file from resource folder %s.\n", _c->fmuResourceLocation);
	found = findNameFile(_c, in_file, pattern);
	logEvent(_c,
		"done searching pattern %s\n", pattern);
	if (found > 1){
		logError(_c, "fmi3InstantiateCosimulation: Found more than "
			" (%d) with extension %s in directory %s. This is not valid.\n", found, pattern, _c->fmuResourceLocation);
		return 1;
	}
	if (strlen(in_file) != 0)
	{
		if (strncmp(pattern, ".idf", 4) == 0){
			_c->in_file = (char*)(calloc(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->in_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
		else if (strncmp(pattern, ".epw", 4) == 0){
			_c->wea_file = (char*)(calloc(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->wea_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
		else if (strncmp(pattern, ".idd", 4) == 0){
			_c->idd_file = (char*)(calloc(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->idd_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
	}
	else
	{
		if (strncmp(pattern, ".idf", 4) == 0){
			logEvent(_c, "Input file not found.");
			logError(_c, "fmi3InstantiateCosimulation: No file with extension"
				" .idf found in the resource location %s. This is not valid.\n", _c->fmuResourceLocation);
			return 1;
		}
		if (strncmp(pattern, ".idd", 4) == 0){
			logEvent(_c, "IDD file not found.\n");
			logError(_c, "fmi3InstantiateCosimulation: No file with extension"
				" .idd found in the resource location %s. This is not valid.\n", _c->fmuResourceLocation);
			return 1;
		}
		if (strncmp(pattern, ".epw", 4) == 0){
			logEvent(_c, "Weather file not found.\n");
			logWarning(_c, "fmi3InstantiateCosimulation: No file with extension"
				" .epw found in the resource location %s.\n", _c->fmuResourceLocation);
			return 0;
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
/// write socket description file
///
///\param _c The FMU instance.
///\param porNum The port number.
///\param hostName The host name.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
int write_socket_cfg(ModelInstance *_c, int portNum, const char* hostName)
{
	FILE *fp;
	fp=fopen("socket.cfg", "w");
	if (fp==NULL) {
		logError(_c,  "Can't open socket.cfg file.\n");
		return 1;  // STL error code: File not open.
	}

	/////////////////////////////////////////////////////////////////////////////
	// write socket configuration file
	fprintf(fp, "<\?xml version=\"1.0\" encoding=\"ISO-8859-1\"\?>\n");
	fprintf(fp, "<BCVTB-client>\n");
	fprintf(fp, "  <ipc>\n");
	fprintf(fp, "    <socket port=\"%d\" hostname=\"%s\"/>\n", portNum, hostName);
	fprintf(fp, "  </ipc>\n");
	fprintf(fp, "</BCVTB-client>\n");
	fclose(fp);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// copy variables.cfg file into the results folder
///
///\param _c The FMU instance.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
int copy_var_cfg(ModelInstance *_c)
{
	char *tmp_str;
	int retVal;
	tmp_str=(char*)(calloc(strlen(_c->tmpResCon) + strlen (_c->fmuOutput) + 30, sizeof(char)));

#ifdef _MSC_VER
	//"\"" are quotes needed for path with spaces in the names
	sprintf(tmp_str, "xcopy %s%s %s%s%s /Y /I", "\"", _c->tmpResCon, "\"", _c->fmuOutput, "\"");
#elif __linux__
	sprintf(tmp_str, "cp -f %s%s %s%s%s", "\"", _c->tmpResCon,  "\"", _c->fmuOutput, "\"");
#elif __APPLE__
	sprintf(tmp_str, "cp -f %s%s %s%s%s", "\"", _c->tmpResCon,  "\"", _c->fmuOutput, "\"");
#else
	printf ("Cannot execute %s. The FMU export is only supported on Windows, Linux and Mac OS.\n", tmp_str);
#endif
	logEvent(_c,
		"Command executes to copy content of resources folder: %s\n", tmp_str);
	retVal=system (tmp_str);
	free(tmp_str);
	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////
/// create results folder
///
///\param _c The FMU instance.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
int create_res(ModelInstance *_c)
{
	char *tmp_str;
	int retVal;
	// The 30 are for the additional characters in tmp_str
	tmp_str=(char*)(calloc(strlen(_c->fmuOutput) + 30, sizeof(char)));

	sprintf(tmp_str, "mkdir %s%s%s", "\"", _c->fmuOutput, "\"");
	retVal=system (tmp_str);
	free (tmp_str);
	return retVal;
}


////////////////////////////////////////////////////////////////////////////////////
/// delete old results folder
///
///\param _c The FMU instance.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
int removeFMUDir (ModelInstance* _c)
{
	int retVal;
	char *tmp_str;
	// The 30 are for the additional characters in tmp_str
	tmp_str=(char*)(calloc(strlen(_c->fmuOutput) + 30, sizeof(char)));
	logEvent(_c,
		"This is the output folder %s\n", _c->fmuOutput);

#ifdef _MSC_VER
	sprintf(tmp_str, "rmdir /S /Q %s%s%s", "\"", _c->fmuOutput, "\"");
#else
	sprintf(tmp_str, "rm -rf %s%s%s", "\"", _c->fmuOutput, "\"");
#endif
	logEvent(_c,
		"This is the command to be executed to delete existing directory %s\n", tmp_str);
	retVal=system (tmp_str);
	free (tmp_str);
	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////
/// start simulation
///
///\param _c The FMU instance.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
int start_sim(ModelInstance* _c)
{
	struct stat stat_p;

#ifdef _MSC_VER
	FILE *fpBat;
#else
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
	extern char **environ;
#endif
	int retVal;
#endif
	logEvent(_c,
		"This version uses the **energyplus** command line interface to "
		" call the EnergyPlus executable. **RunEPlus.bat** and **runenergyplus** ,"
		" which were used in earlier versions, were deprecated as of August 2015.");
#ifdef _MSC_VER
	fpBat=fopen("EP.bat", "w");
	if (stat(FRUNWEAFILE, &stat_p)>=0){
		// write the command string
		fprintf(fpBat, "energyplus %s %s %s %s %s %s %s %s %s %s",
			"-w", FRUNWEAFILE, "-p", _c->mID, "-s", "C", "-x", "-m",
			"-r", _c->in_file_name);
	}
	else
	{
		// write the command string
		fprintf(fpBat, "energyplus %s %s %s %s %s %s %s %s",
			"-p", _c->mID, "-s", "C", "-x", "-m", "-r", _c->in_file_name);
	}
	fclose (fpBat);
	_c->pid=(HANDLE)_spawnl(P_NOWAIT, "EP.bat", "EP.bat", NULL);
	if (_c->pid > 0 ) {
		return 0;
	}
	else {
		return 1;
	}
#else
	if (stat (FRUNWEAFILE, &stat_p)>=0){
		//char *const argv[]={"runenergyplus", _c->mID, FRUNWEAFILE, NULL};
		char *const argv[]={"energyplus", "-w", FRUNWEAFILE, "-p", _c->mID,
			"-s", "C", "-x", "-m", "-r", _c->in_file_name, NULL};
		// execute the command string
		retVal=posix_spawnp( &_c->pid, argv[0], NULL, NULL, argv, environ);
		return retVal;
	}
	else
	{
		//char *const argv[]={"runenergyplus", _c->mID, NULL};
		char *const argv[]={"energyplus", "-p", _c->mID, "-s", "C", "-x",
			"-m", "-r", _c->in_file_name, NULL};
		// execute the command string
		retVal=posix_spawnp( &_c->pid, argv[0], NULL, NULL, argv, environ);
		return retVal;
	}
#endif
}
//
//// The methods below should be used only when testing the main program below.
/////////////////////////////////////////////////////////////////////////////////
///// FMI status
/////
/////\param status FMI status.
/////////////////////////////////////////////////////////////////////////////////
////#if 0
//const char* fmi3StatusToString(fmi3Status status){
//	switch (status){
//	case fmi3OK:      return "ok";
//	case fmi3Warning: return "warning";
//	case fmi3Discard: return "discard";
//	case fmi3Error:   return "error";
//	default:         return "?";
//	}
//}
////#endif
////
////
///////////////////////////////////////////////////////////////////////////////////
/////// FMU logger
///////
///////\param c The FMU instance.
///////\param instanceName FMI string.
///////\param status FMI status.
///////\param category FMI string.
///////\param message Message to be recorded.
///////////////////////////////////////////////////////////////////////////////////
//void fmuLogger(fmi3Instance c, fmi3String instanceName, fmi3Status status,
//	fmi3String category, fmi3String message, ...) {
//		char msg[MAX_MSG_SIZE];
//		char* copy;
//		va_list argp;
//
//		// Replace C format strings
//		va_start(argp, message);
//		vsprintf(msg, message, argp);
//
//		// Replace e.g. ## and #r12#
//		copy=strdup(msg);
//		free(copy);
//
//		// Print the final message
//		if (!instanceName) instanceName="?";
//		if (!category) category="?";
//		printf("%s %s (%s): %s\n", fmi3StatusToString(status), instanceName, category, msg);
//}

//////////////////////////////////////////////////////////////
///  This method is used to get the fmi types of platform
/// \return fmiPlatform.
//////////////////////////////////////////////////////////////
DllExport const char* fmi3GetTypesPlatform()
{
	return "default";
}

////////////////////////////////////////////////////////////////
///  This method is used to get the fmi version
///\return fmiVersion.
////////////////////////////////////////////////////////////////
DllExport const char* fmi3GetVersion()
{   // This function always returns 3.0
	return FMIVERSION;
}

////////////////////////////////////////////////////////////////////////////////
///// Print formatted debug message
/////
/////\param _c The FMU instance.
/////\param str1 Message to be printed for debugging
/////\param str2 String variable to be printed for debugging
////////////////////////////////////////////////////////////////////////////////
//void printfDebug(ModelInstance *_c, const char* str1, const char* str2){
//	if (_c->loggingOn==1)
//	{
//		fprintf(stdout, "Debug: ");
//		fprintf(stdout, str1, str2);
//	}
//}

////////////////////////////////////////////////////////////////
///  This method is used to free the FMU instance
///
///\param _c The FMU instance.
////////////////////////////////////////////////////////////////
int freeInstanceResources(ModelInstance* _c) {
	logEvent(_c,
		"freeInstanceResources: %s will be freed.\n", _c->instanceName);
	// free model ID
	if (_c->mID!=NULL) free(_c->mID);
	_c->mID = NULL;
	// free model instanceToken
	if (_c->mInstTok!=NULL) free(_c->mInstTok);
	_c->mInstTok = NULL;
	// free xml file
	if (_c->xml_file!=NULL) free(_c->xml_file);
	_c->xml_file = NULL;
	// free input file
	if (_c->in_file != NULL) free(_c->in_file);
	_c->in_file = NULL;
	// free weather file
	if (_c->wea_file != NULL) free(_c->wea_file);
	_c->wea_file = NULL;
	// free idd file
	if (_c->idd_file != NULL) free(_c->idd_file);
	_c->idd_file = NULL;
	// free resource location
	if (_c->fmuResourceLocation!=NULL) free(_c->fmuResourceLocation);
	_c->fmuResourceLocation = NULL;
	// free unzip location
	if (_c->fmuUnzipLocation!=NULL) free(_c->fmuUnzipLocation);
	_c->fmuUnzipLocation=NULL;
	// free output location
	if (_c->fmuOutput!=NULL) free(_c->fmuOutput);
	_c->fmuOutput = NULL;
	// free temporary result folder
	if (_c->tmpResCon!=NULL) free(_c->tmpResCon);
	_c->tmpResCon=NULL;
	// deallocate memory for inVec
	if (_c->inVec != NULL)  free(_c->inVec);
	_c->inVec = NULL;
	// deallocate memory for outVec
	if (_c->outVec != NULL)  free(_c->outVec);
	_c->outVec = NULL;
	 // free fmu instance
	if (_c!=NULL) free(_c);
	_c=NULL;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// This function writes the path to the fmu resource location.
///
///\param _c The FMU instance.
///\param path The path to the resource location.
///\return 0 if no error.
///////////////////////////////////////////////////////////////////////////////
int getResourceLocation(ModelInstance *_c, fmi3String fmuLocation)
{
	char tmpResLoc[5]={0};
	struct stat st;
	fmi3Boolean errDir;
	int lenUnzLoc;

	// allocate memory for fmuResourceLocation
	_c->fmuResourceLocation = (char*)calloc(strlen(fmuLocation) + 1, sizeof(char));

	// store the fmuResourceLocation
	sprintf(_c->fmuResourceLocation, "%s", fmuLocation);
	logEvent(_c,
		"fmi3InstantiateCosimulation: Path to fmuResourceLocation %s\n", _c->fmuResourceLocation);
	//return 0;

	// length of fmuUnzipLocation
	lenUnzLoc = strlen(fmuLocation) - ((strlen(RESOURCES) + strlen(PATH_SEP)));

	// create the fmuUnzipLocation
	_c->fmuUnzipLocation = (char*)calloc(lenUnzLoc + 1, sizeof(char));

	// copy first 5 characters of fmuLocation
	strncpy (_c->fmuUnzipLocation, fmuLocation, lenUnzLoc);

	// check whether fmuUnzipLocation exists
	errDir=stat(_c->fmuUnzipLocation, &st);
	if (errDir < 0)
	{
		logError(_c,
			"fmi3InstantiateCosimulation: Path to fmuUnzipLocation %s does not exist.\n", _c->fmuUnzipLocation);
		free(_c->fmuUnzipLocation);
		return 1;

	}
	else {
		logEvent(_c,
			"fmi3InstantiateCosimulation: Path to fmuUnzipLocation %s exists.\n", _c->fmuUnzipLocation);
		return 0;
	}

//	// allocate memory for fmuUnzipLocation
//	_c->fmuUnzipLocation=(char *)calloc(strlen (fmuLocation)
//		+ strlen(PATH_SEP) + 1, sizeof(char));
//
//	// extract the URI information from the fmuUnzipLocation path
//#ifdef _MSC_VER
//	if (strnicmp (tmpResLoc, "file", 4)==0)
//#else
//	if (strncasecmp (tmpResLoc, "file", 4)==0)
//#endif
//	{
//		// The specification for defining whether the file should start with file:/, file://, or file:///
//		// is not clear (e.g. see FMI 1.0, and FMI 2.0). We will thus check all cases to see what we have
//		// case file: (If Ptolemy unzips an FMU in /tmp then the fmuLocation is file:/temp)
//		strncpy(_c->fmuUnzipLocation, fmuLocation + 5, strlen(fmuLocation + 5)-10);
//		// check whether fmuUnzipLocation exists
//		errDir=stat(_c->fmuUnzipLocation, &st);
//		if (errDir<0)
//		{
//			logEvent(_c,
//				"fmi3InstantiateCosimulation: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);
//			free (_c->fmuUnzipLocation);
//			_c->fmuUnzipLocation=NULL;
//			// allocate memory for fmuUnzipLocation
//			_c->fmuUnzipLocation=(char *)calloc(strlen (fmuLocation) + 1, sizeof(char));
//			// case file:/
//			strncpy(_c->fmuUnzipLocation, fmuLocation + 6, strlen(fmuLocation + 6)-10);
//			// check whether fmuUnzipLocation exists
//			errDir=stat(_c->fmuUnzipLocation, &st);
//			if(errDir<0)
//			{
//				logEvent(_c,
//					"fmi3InstantiateCosimulation: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);
//				free(_c->fmuUnzipLocation);
//				_c->fmuUnzipLocation=NULL;
//				// allocate memory for fmuUnzipLocation
//				_c->fmuUnzipLocation=(char *)calloc(strlen (fmuLocation) + 1, sizeof(char));
//				// case file://
//				strncpy(_c->fmuUnzipLocation, fmuLocation + 7, strlen(fmuLocation + 7)-10);
//
//				// check whether fmuUnzipLocation exists
//				errDir=stat(_c->fmuUnzipLocation, &st);
//				if(errDir<0)
//				{
//					logEvent(_c,
//						"fmi3InstantiateCosimulation: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);
//					free(_c->fmuUnzipLocation);
//					_c->fmuUnzipLocation=NULL;
//					// allocate memory for fmuUnzipLocation
//					_c->fmuUnzipLocation=(char *)calloc(strlen (fmuLocation) + 1, sizeof(char));
//					// case file:///
//					strncpy(_c->fmuUnzipLocation, fmuLocation + 8, strlen(fmuLocation + 8)-10);
//					// check whether fmuUnzipLocation exists
//					errDir=stat(_c->fmuUnzipLocation, &st);
//					if(errDir<0)
//					{
//						logError(_c, "fmi3InstantiateCosimulation: The path to the unzipped"
//							" folder %s is not valid. The path does not start with file: file:/, file:// or file:///\n", fmuLocation);
//						free(_c->fmuUnzipLocation);
//						_c->fmuUnzipLocation=NULL;
//						return 1;
//					}
//				}
//			}
//		}
//	}
//#ifdef _MSC_VER
//	else if ((strnicmp (tmpResLoc, "ftp", 3)==0) || (strnicmp (tmpResLoc, "fmi", 3)==0))
//#else
//	else if ((strncasecmp (tmpResLoc, "ftp", 3)==0) || (strncasecmp (tmpResLoc, "fmi", 3)==0))
//#endif
//	{
//		strncpy(_c->fmuUnzipLocation, fmuLocation + 6, strlen(fmuLocation + 6)-10);
//		logEvent(_c,
//			"fmi3InstantiateCosimulation: Path to fmuUnzipLocation without ftp:// or fmi:// %s\n", _c->fmuUnzipLocation);
//	}
//
//#ifdef _MSC_VER
//	else if ((strnicmp (tmpResLoc, "https", 5)==0))
//#else
//	else if ((strncasecmp (tmpResLoc, "https", 5)==0))
//#endif
//	{
//		strncpy(_c->fmuUnzipLocation, fmuLocation + 8, strlen(fmuLocation + 8)-10);
//		logEvent(_c,
//			"fmi3InstantiateCosimulation: Path to fmuUnzipLocation without https:// %s\n", _c->fmuUnzipLocation);
//	}
//	else
//	{
//		strcpy(_c->fmuUnzipLocation, fmuLocation);
//		logEvent(_c,
//			"fmi3InstantiateCosimulation: Path to fmuUnzipLocation %s\n", _c->fmuUnzipLocation);
//	}
//
//	// Add back slash so we can copy files to the fmuUnzipLocation folder afterwards
//	sprintf(_c->fmuUnzipLocation, "%s%s", _c->fmuUnzipLocation, PATH_SEP);
//	logEvent(_c,
//		"fmi3InstantiateCosimulation: Path to fmuUnzipLocation %s\n", _c->fmuUnzipLocation);
//
//	// allocate memory for fmuResourceLocation
//	_c->fmuResourceLocation = (char *)calloc(strlen(_c->fmuUnzipLocation)
//		+ strlen(RESOURCES) + strlen(PATH_SEP) + 1, sizeof(char));
//
//	sprintf(_c->fmuResourceLocation, "%s%s%s", _c->fmuUnzipLocation, RESOURCES, PATH_SEP);
//	logEvent(_c,
//		"fmi3InstantiateCosimulation: Path to fmuResourceLocation %s\n", _c->fmuResourceLocation);
//	return 0;
}

////////////////////////////////////////////////////////////////
///  This method is used to instantiated the FMU (fixme add correct variables)
///
///\param instanceName A unique identifier for the FMU instance..
///\param instantiationToken The instantiationToken.
///\param resourcePath The FMU resource location path.
///\param visible The flag to executes the FMU in windowless mode.
///\param loggingOn The flag to enable or disable debug.
///\param eventModeUsed The flag to indicate that the importer can handle events
///\param earlyReturnAllowed The flag to indicate tha the importer can handle early return
///\param requiredIntermediateVariables[] An array of the value references of all input variables
///\param nRequiredIntermediateVariables Number of requiredIntermediateVariables           
///\param instanceEnvironment A pointer that must be passed to fmi3IntermediateUpdateCallback
///\param logMessage The callback function to log messages
///\param intermediateUpdate The callback to intermediate update
////////////////////////////////////////////////////////////////
DllExport fmi3Instance fmi3InstantiateCosimulation(fmi3String instanceName,
	fmi3String instantiationToken, /*Change this to instantiationToken*/
	fmi3String resourcePath , /*Change this to resourcePath*/
	fmi3Boolean visible,
	fmi3Boolean loggingOn,
	fmi3Boolean eventModeUsed,
	fmi3Boolean earlyReturnAllowed,
	const fmi3ValueReference requiredIntermediateVariables[],
	size_t nRequiredIntermediateVariables,
	fmi3InstanceEnvironment instanceEnvironment,
	fmi3LogMessageCallback logMessage,
	fmi3IntermediateUpdateCallback intermediateUpdate)

{
	int retVal;
	fmi3String mID;
	fmi3String mInstTok;
	fmi3String mFmiVers;
	struct stat st;
	fmi3Boolean errDir;

	UNUSED(visible);
	UNUSED(requiredIntermediateVariables);
	UNUSED(nRequiredIntermediateVariables);


	//initialize the model instance
	ModelInstance* _c = createModelInstance(
		(loggerType)logMessage,
		(intermediateUpdateType)intermediateUpdate,
		instanceEnvironment,
		instanceName,
		instantiationToken,
		resourcePath,
		loggingOn,
		CoSimulation);

	if (!_c) {
		logError(_c,
			"fmi3InstantiateCosimulation: Instantiation of %s failed.\n", instanceName);
		return NULL;
	}


	//// check instance name -- fixme, check in createModelInstance for the actual implementation
	//if (!instanceName || strlen(instanceName) == 0 || (strlen(instanceName) > MAX_VARNAME_LEN)) {
	//	logError(_c,
	//		"fmi3InstantiateCosimulation: Missing instance name or instance name longer than 300 characters."
	//		" Instantiation of %s failed.\n", instanceName);
	//	return NULL;
	//}
	// indicate the instance name
	logEvent(_c,
		"fmi3InstantiateCosimulation: The FMU instance name is %s.\n",
		instanceName);

	// check whether the path to the resource folder has been provided
	if ((resourcePath == NULL) || (strlen(resourcePath) == 0)) {
		logError(_c, 
			"fmi3InstantiateCosimulation: The path"
			" to the folder where the FMU is unzipped is not specified. This is not valid."
			" Instantiation of %s failed.\n", instanceName);
		// Free resources allocated to instance.
		//freeInstanceResources(_c);
		return NULL;
	}

	// indicate the resource path
	logEvent(_c,
		"fmi3InstantiateCosimulation: The Resource location of FMU with instance name %s is %s.\n",
		instanceName, resourcePath);

	// Check the visible parameter
	if (visible == fmi3True) {
		logWarning(_c,
			"fmi3InstantiateCosimulation: Argument visible is set to %d\n."
			" This is not supported. visible will default to '0'.\n", visible);
	}


	//// check the logging parameter
	//if (loggingOn == fmi3True) {
	//	logWarning(_c,
	//		"fmi3InstantiateCosimulation: Argument loggingOn is set to %d\n."
	//		" This is not supported. loggingOn will default to '0'.\n", loggingOn);
	//}

	//loggingOn = fmi3False;


	// Assign default values to model instance
	if (_c) {
		_c->earlyReturnAllowed = earlyReturnAllowed;
		_c->eventModeUsed = eventModeUsed;
		_c->state = Instantiated;
	}


	// get current working directory
#ifdef _MSC_VER
	if (_getcwd(_c->cwd, sizeof(_c->cwd))==NULL)
#else
	if (getcwd(_c->cwd, sizeof(_c->cwd))==NULL)
#endif
	{
		logError(_c,
			"fmi3InstantiateCosimulation: Cannot get current working directory."
			" Instantiation of %s failed.\n", _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	else
	{
		logEvent(_c, "The current working directory is %s\n", _c->cwd);
	}

	// create the output folder for current FMU in working directory
	_c->fmuOutput=(char *)calloc(strlen ("Output_EPExport_") + strlen (_c->instanceName)
		+ strlen (_c->cwd) + 5, sizeof(char));
	sprintf(_c->fmuOutput, "%s%s%s%s", _c->cwd, PATH_SEP, "Output_EPExport_", _c->instanceName);

	// check if directory exists and deletes it
	errDir=stat(_c->fmuOutput, &st);
	if(errDir>=0) {
		logWarning(_c,
			"fmi3InstantiateCosimulation: The fmuOutput directory %s exists. It will now be deleted.\n", _c->fmuOutput);
		if(removeFMUDir (_c)!=0){
			logWarning(_c,
				"fmi3InstantiateCosimulation: The fmuOutput directory %s could not be deleted\n", _c->fmuOutput);
		}
	}


	// get the FMU resource location
	retVal=getResourceLocation (_c, resourcePath);
	if (retVal!=0 ){
		logError(_c,
		"fmi3InstantiateCosimulation: Could not get the resource or unzip location."
			" Instantiation of %s failed.\n", _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
#ifdef _MSC_VER
	// replace eventual forward slash with backslash on Windows
	replace_char (_c->fmuResourceLocation, '//', '\\');
	replace_char (_c->fmuUnzipLocation, '//', '\\');
#endif
	
	// create content of resources folder in created output directory
	// "\"" is to make sure that we have quotation at the end of the path for variables.cfg.
	_c->tmpResCon=(char *)calloc(strlen (_c->fmuResourceLocation) + strlen (VARCFG) + strlen ("\"") + 1, sizeof(char));
	sprintf(_c->tmpResCon, "%s%s%s", _c->fmuResourceLocation, "\"", VARCFG);
	
	// create the output directory
	retVal=create_res(_c);
	if (retVal!=0){
		logEvent(_c, "fmi3InstantiateCosimulation: Could not create the output"
			" directory %s. Instantiation of %s failed.\n", _c->fmuOutput, _c->instanceName);
	}
	
	// add the end slash to the fmuOutput
	sprintf(_c->fmuOutput, "%s%s", _c->fmuOutput, PATH_SEP);
	
	// copy the variables cfg into the output directory
	retVal=copy_var_cfg(_c);
	if (retVal!=0){
		logEvent(_c, "fmi3InstantiateCosimulation: Could not copy"
			" variables.cfg to the output directory folder %s. Instantiation of %s failed.\n", _c->cwd, _c->instanceName);
		// Free resources allocated to instance.
		//return NULL;
		freeInstanceResources (_c);
		return NULL;
	}
	
	// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
	retVal=_chdir(_c->fmuOutput);
#else
	retVal=chdir(_c->fmuOutput);
#endif
	if (retVal!=0){
		logEvent(_c, "fmi3InstantiateCosimulation: Could not switch"
			" to the output folder %s. Instantiation of %s failed.\n", _c->fmuOutput, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	
	// create path to xml file
	_c->xml_file=(char *)calloc(strlen (_c->fmuUnzipLocation) + strlen (XML_FILE) + 1, sizeof(char));
	sprintf(_c->xml_file, "%s%s", _c->fmuUnzipLocation, XML_FILE);
	logEvent(_c,
		"fmi3InstantiateCosimulation: Path to model description file is %s.\n", _c->xml_file);
	
	// get model description of the FMU
	_c->md=parse(_c->xml_file);
	if (!_c->md) {
		logEvent(_c, "fmi3InstantiateCosimulation: Failed to parse the model description"
			" found in directory %s. Instantiation of %s failed\n", _c->xml_file, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	
	// get the modelID of the FMU
	mID = getAttributeValue((Element *)getCoSimulation(_c->md), att_modelIdentifier);
	
	// copy model ID to FMU
	_c->mID=(char *)calloc(strlen(mID) + 1, sizeof(char));
	strcpy(_c->mID, mID);
	logEvent(_c, "fmi3InstantiateCosimulation: The FMU modelIdentifier is %s.\n", _c->mID);
	
	// get the model instanceToken (GUID in FMI 2.0) of the FMU
	mInstTok = getAttributeValue((Element *)getCoSimulation(_c->md), att_instantiationToken);
	
	// copy model instanceToken to FMU
	_c->mInstTok=(char *)calloc(strlen (mInstTok) + 1,sizeof(char));
	strcpy(_c->mInstTok, mInstTok);
	logEvent(_c, "fmi3InstantiateCosimulation: The FMU modelinstanceToken is %s.\n", _c->mInstTok);
	
	// check whether instanceTokens are consistent with modelDescription file
	if(strcmp(instantiationToken, _c->mInstTok) !=0)
	{
		logEvent(_c,
			" fmi3InstantiateCosimulation: Wrong instanceToken %s. Expected %s. Instantiation of %s failed.\n", instantiationToken, _c->mInstTok, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	
	// check whether the model is exported for FMI version 1.0
	mFmiVers = extractVersion(_c->xml_file);
	if(strcmp(mFmiVers, FMIVERSION) !=0){
		logEvent(_c, "fmi3InstantiateCosimulation: Wrong FMI version %s."
			" FMI version 3.0 is currently supported. Instantiation of %s failed.\n", mFmiVers, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	logEvent(_c,
		"fmi3InstantiateCosimulation: Slave %s is instantiated.\n", _c->instanceName);
	
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal=_chdir(_c->cwd);
#else
	retVal=chdir(_c->cwd);
#endif
	if (retVal!=0){
		logEvent(_c, "fmi3InstantiateCosimulation: Could not switch to"
			" the working directory folder %s. Instantiation of %s failed.\n", _c->cwd, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	
	// This is required to prevent Dymola to call fmi3SetReal before the initialization
	_c->firstCallIni=1;
	logEvent(_c,
		"fmi3InstantiateCosimulation: Instantiation of %s succeded.\n", _c->instanceName);

	return(_c);
}

////////////////////////////////////////////////////////////////
///  This method is used to enter initialization in the FMU
///
///\param c The FMU instance.
///\param The flag to indicate that the tolerance is defined
///\param The tolerance
///\param The simulation start time
///\param The flag to indicate that the stop time is defined
///\param The simulation stop time
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3EnterInitializationMode(fmi3Instance c,
	fmi3Boolean toleranceDefined,
	fmi3Float64 tolerance,
	fmi3Float64 startTime,
	fmi3Boolean stopTimeDefined,
	fmi3Float64 stopTime)
{
	int retVal;
	ModelInstance* _c=(ModelInstance *)c;
	FILE *fp;
	char tStartFMUstr[100];
	char tStopFMUstr[100];
	char command[100];
	char *tmpstr;
	char *cmdstr;
	char *cmdstrEXE;

#ifdef _MSC_VER
	int sockLength;

#else
	struct stat stat_p;
	socklen_t sockLength;
#endif
	struct sockaddr_in   server_addr;
	int                  port_num;
	char                 ThisHost[10000];
	struct  hostent *hp;

#ifdef _MSC_VER
	WORD wVersionRequested=MAKEWORD(2,2);
	WSADATA wsaData;
#endif

#ifndef _MSC_VER
	mode_t process_mask=umask(0);
#endif

	_c->tStartFMU = startTime;
	_c->tStopFMU = stopTime;

	//// Check if setup experiment has been called
	//if (_c->setupExperiment != 1) {
	//	logError(_c,
	//		"fmi3EnterInitializationMode: fmi3SetupExperiment must be called at least once prior "
	//		" to calling fmi3EnterInitializationMode\n");
	//	return fmi3Error;
	//}

	//// reset flag for indicating that fmi3SetupExperiment() has been called.
	//_c->setupExperiment = 0;

	// initialize structure variables
	_c->firstCallGetReal      =1;
	//_c->firstCallSetReal      =1;
	_c->firstCallDoStep       =1;
	_c->firstCallFree         =1;
	_c->firstCallTerm         =1;
	_c->firstCallRes          =1;
	_c->flaGetRealCall        =0;
	_c->flaGetRea=0;
	_c->flaWri=0;
	_c->flaRea=0;
	_c->wea_file = NULL;
	_c->in_file = NULL;
	_c->in_file = NULL;
	_c->getCounter=0;
	_c->setCounter=0;
	_c->readReady=0;
	_c->writeReady=0;
	_c->numInVar =-1;
	_c->numOutVar=-1;

	// check the stopTimeDefined parameter
	if (stopTimeDefined == fmi3False) {
		logWarning(_c,
			"fmi3EnterInitializationMode: The StopTimeDefined parameter is set to %d. This is not valid."
			" EnergyPlus FMU requires a stop time and will use the stop time %f which is provided.\n",
			stopTimeDefined, stopTime);
}

	// check the toleranceDefined parameter
	if (toleranceDefined == fmi3True) {
		logWarning(_c,
			"fmi3EnterInitializationMode: The toleranceDefined parameter is set to %d."
			" However, EnergyPlus FMU won't use the tolerance %f which is provided.\n",
			toleranceDefined, tolerance);
	}

	// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
	retVal=_chdir(_c->fmuOutput);
#else
	retVal=chdir(_c->fmuOutput);
#endif
	if (retVal!=0){
		logError(_c,
			"fmi3EnterInitializationMode: The path to the output folder %s is not valid.\n", _c->fmuOutput);
		return fmi3Error;
	}
	///////////////////////////////////////////////////////////////////////////////////
	// create the socket server

#ifdef _MSC_VER
	// initialize winsock  /************* Windows specific code ********/
	if (WSAStartup(wVersionRequested, &wsaData)!=0)
	{
		logError(_c,
		"fmi3EnterInitializationMode: WSAStartup failed with error %ld.\n", WSAGetLastError());
		WSACleanup();
		return fmi3Error;
	}
	// check if the version is supported
	if (LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2 )
	{
		logError(_c,
		"fmi3EnterInitializationMode: Could not find a usable WinSock DLL for WinSock version %u.%u.\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.		wVersion));
		WSACleanup();
		return fmi3Error;
	}
#endif  /************* End of Windows specific code *******/

	_c->sockfd=socket(AF_INET, SOCK_STREAM, 0);
	// check for errors to ensure that the socket is a valid socket.
	if (_c->sockfd==INVALID_SOCKET)
	{
		logError(_c,
		"fmi3EnterInitializationMode: Opening socket failed"
			" sockfd=%d.\n", _c->sockfd);
		return fmi3Error;
	}
	logEvent(_c, "fmi3EnterInitializationMode: The sockfd is %d.\n", _c->sockfd);
	// initialize socket structure server address information
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;                 // Address family to use
	server_addr.sin_port=htons(0);                  // Port number to use
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // Listen on any IP address

	// bind the socket
	if (bind(_c->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		logError(_c, "fmi3EnterInitializationMode: bind() failed.\n");
		closeipcFMU (&(_c->sockfd));
		return fmi3Error;
	}

	// get socket information information
	sockLength=sizeof(server_addr);
	if ( getsockname (_c->sockfd, (struct sockaddr *)&server_addr, &sockLength)) {
		logError(_c,
		"fmi3EnterInitializationMode: Get socket name failed.\n");
		return fmi3Error;
	}

	// get the port number
	port_num=ntohs(server_addr.sin_port);
	logEvent(_c, "fmi3EnterInitializationMode: The port number is %d.\n", port_num);

	// get the hostname information
	gethostname(ThisHost, MAXHOSTNAME);
	if  ((hp=gethostbyname(ThisHost))==NULL ) {
		logError(_c,
		"fmi3EnterInitializationMode: Get host by name failed.\n");
		return fmi3Error;
	}

	// write socket cfg file
	retVal=write_socket_cfg (_c, port_num, ThisHost);
	logEvent(_c, "fmi3EnterInitializationMode: This hostname is %s.\n", ThisHost);
	if  (retVal !=0) {
		logError(_c,
		"fmi3EnterInitializationMode: Write socket cfg failed.\n");
		return fmi3Error;
	}
	// listen to the port
	if (listen(_c->sockfd, 1)==SOCKET_ERROR)
	{
		logError(_c, "fmi3EnterInitializationMode: listen() failed.\n");
		closeipcFMU (&(_c->sockfd));
		return fmi3Error;
	}
	logEvent(_c, "fmi3EnterInitializationMode: TCPServer Server waiting for clients on port: %d.\n", port_num);

	// Initialize the number of inputs and output variables
	_c->numInVar = 0;
	_c->numOutVar = 0;

	// Get the number of inputs and output variables
	getNumInputOutputVariablesInFMU(_c->md, &_c->numInVar, &_c->numOutVar);

	if (_c->numInVar!=0)
	{
		// initialize the input vectors
		_c->inVec=(fmi3Float64*)calloc(_c->numInVar, sizeof(fmi3Float64));
	}
	logEvent(_c, "fmi3EnterInitializationMode: The number of input variables is %d.\n", _c->numInVar);

	// get the number of output variables of the FMU
	if (_c->numOutVar!=0)
	{
		// initialize the output vector
		_c->outVec=(fmi3Float64*)calloc(_c->numOutVar, sizeof(fmi3Float64));
	}
	logEvent(_c, "fmi3EnterInitializationMode: The number of output variables is %d.\n", _c->numOutVar);

	if ( (_c->numInVar + _c->numOutVar)==0){
		logError(_c,
			"fmi3EnterInitializationMode: The FMU instance %s has no input and output variables. Please check the model description file.\n",
			_c->instanceName);
		return fmi3Error;
	}
	// create the input and weather file for the run
	// Need to see how we will parste the start and stop time so
	// they become strings and can be used by str when calling the system command.

	// get input file from the folder. There must be only one input file in the folder.
	retVal = getResFile(_c, ".idf");
	if (retVal != 0){
		logError(_c, "fmi3EnterInitializationMode: Could not get"
			" the .idf input file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmi3Error;
	}
	// get the weather file from the folder. there must be only one weather file in the folder.
	retVal = getResFile(_c, ".epw");
	if (retVal != 0){
		logError(_c, "fmi3EnterInitializationMode: Could not"
			" get the .epw weather file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmi3Error;
	}
	// get the idd file from the folder. there must be only one idd file in the folder.
	retVal = getResFile(_c, ".idd");
	if (retVal != 0){
		logError(_c, "fmi3EnterInitializationMode: Could not"
			" get the .idd dictionary file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmi3Error;
	}

	// Check the validity of the FMU start and stop time
	if (modulusOp((_c->tStopFMU - _c->tStartFMU), 86400) != 0){
		logError(_c, "fmi3EnterInitializationMode: The delta"
			" between the FMU stop time %f and the FMU start time %f must be a multiple of 86400."
			" This is required by EnergyPlus. The current delta is %f. Instantiation of %s failed.\n",
			_c->tStopFMU, _c->tStartFMU, _c->tStopFMU - _c->tStartFMU, _c->instanceName);
		return fmi3Error;
	}

	sprintf(tStartFMUstr, "%f", _c->tStartFMU);
	sprintf(tStopFMUstr, "%f", _c->tStopFMU);

	//retVal=createRunInfile(_c);

#ifdef _MSC_VER
	strcpy(command, "idf-to-fmu-export-prep-win.exe");
#elif __linux__
	strcpy(command, "idf-to-fmu-export-prep-linux");
#elif __APPLE__
	strcpy(command, "idf-to-fmu-export-prep-darwin");
#endif

	if (_c->wea_file != NULL){
		cmdstr = (char *)calloc(strlen(_c->fmuResourceLocation) + strlen(command) + 10, sizeof(char));
		sprintf(cmdstr, "%s%s", _c->fmuResourceLocation, command);
		//Make file executable if UNIX
#ifndef _MSC_VER
		cmdstrEXE = (char *)calloc(strlen(cmdstr) + 10, sizeof(char));
		sprintf(cmdstrEXE, "%s %s", "chmod +x", cmdstr);
		retVal = system(cmdstrEXE);
		if (retVal != 0){
			logError(_c, "fmi3EnterInitializationMode: Could not"
				" make preprocessor executable. Initialization of %s failed.\n",
				_c->instanceName);
			return fmi3Error;
		}
		free(cmdstrEXE);
#endif
		tmpstr = (char *)calloc(strlen(cmdstr) + strlen(_c->wea_file) +
			strlen(_c->idd_file) + strlen(_c->in_file) + strlen(tStartFMUstr) + strlen(tStopFMUstr) + 50, sizeof(char));
		sprintf(tmpstr, "%s -w %s -b %s -e %s %s %s", cmdstr, _c->wea_file, tStartFMUstr, tStopFMUstr, _c->idd_file, _c->in_file);
		retVal = system(tmpstr);
	}
	else{
		cmdstr = (char *)calloc(strlen(_c->fmuResourceLocation) + strlen(command) + 10, sizeof(char));
		sprintf(cmdstr, "%s%s", _c->fmuResourceLocation, command);
#ifndef _MSC_VER
		cmdstrEXE = (char *)calloc(strlen(cmdstr) + 10, sizeof(char));
		sprintf(cmdstrEXE, "%s %s", "chmod +x", cmdstr);
		retVal = system(cmdstrEXE);
		if (retVal != 0){
			logError(_c, "fmi3EnterInitializationMode: Could not"
				" make preprocessor executable. Initialization of %s failed.\n",
				_c->instanceName);
			return fmi3Error;
		}
		free(cmdstrEXE);
#endif
		tmpstr = (char *)calloc(strlen(cmdstr) +
			strlen(_c->idd_file) + strlen(_c->in_file) + strlen(tStartFMUstr) + strlen(tStopFMUstr) + 50, sizeof(char));
		sprintf(tmpstr, "%s -b %s -e %s %s %s", cmdstr, tStartFMUstr, tStopFMUstr, _c->idd_file, _c->in_file);
		retVal = system(tmpstr);
	}
	free(cmdstr);
	free(tmpstr);
	if (retVal != 0){
		logError(_c, "fmi3EnterInitializationMode: Could not"
			" create the input and weather file. Initialization of %s failed.\n",
			_c->instanceName);
		return fmi3Error;
	}

	// rename found idf to have the correct name.
	tmpstr = (char *)calloc(strlen(_c->mID) + strlen(".idf") + 1, sizeof(char));
	sprintf(tmpstr, "%s%s", _c->mID, ".idf");
	strcpy(_c->in_file_name, tmpstr);
	// free tmpstr
	free(tmpstr);
	retVal = rename(FRUNINFILE, _c->in_file_name);
	if (retVal != 0){
		logError(_c, "fmi3EnterInitializationMode: Could not"
			" rename the temporary input file. Initialization of %s failed.\n",
			_c->instanceName);
		return fmi3Error;
	}

	if((fp=fopen(FTIMESTEP, "r")) !=NULL) {
		retVal=fscanf(fp, "%d", &(_c->timeStepIDF));
		fclose (fp);
		// check if the timeStepIDF is null to avoid division by zero
		if (_c->timeStepIDF==0){
			logError(_c,
				"fmi3EnterInitializationMode: The time step in IDF cannot be null.\n");
			logError(_c,   "fmi3EnterInitializationMode: Time step in IDF is null.\n");
			return fmi3Error;
		}
	}
	else
	{
		logError(_c,
			"fmi3EnterInitializationMode: A valid time step could not be determined.\n");
		logError(_c,   "fmi3EnterInitializationMode: Can't read time step file.\n");
		return fmi3Error;
	}

#ifndef _MSC_VER
	umask(process_mask);
#endif
	// start the simulation
	retVal=start_sim(_c);
	_c->newsockfd=accept(_c->sockfd, NULL, NULL);
	logEvent(_c, "fmi3EnterInitializationMode: The connection has been accepted.\n");
	// check whether the simulation could start successfully
	if  (retVal !=0) {
		logError(_c,
		"fmi3EnterInitializationMode: The FMU instance could %s not be initialized. "
			"EnergyPlus can't start . Check if EnergyPlus is installed and on the system path.\n",
			_c->instanceName);
		return fmi3Error;
	}

	// reset firstCallIni
	if (_c->firstCallIni)
	{
		_c->firstCallIni=0;
	}
	logEvent(_c, "fmi3EnterInitializationMode: Slave %s is initialized.\n", _c->instanceName);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal=_chdir(_c->cwd);
#else
	retVal=chdir(_c->cwd);
#endif
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to exit initialization in the FMU
///
///\param c The FMU instance.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3ExitInitializationMode(fmi3Instance c)
{
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to do the time stepping the FMU
///
///\param c The FMU instance.
///\param currentCommunicationPoint The communication point.
///\param communicationStepSize The communication step size.
///\param noSetFMUStatePriorToCurrentPoint) The flag to control rollback.
///\param Flag to indicate that event can be handled
///\param Flag to terminate the simulation
///\param The flag to indicate early return
///\param The last successful time
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3DoStep(fmi3Instance c, 
	fmi3Float64 currentCommunicationPoint,
	fmi3Float64 communicationStepSize, 
	fmi3Boolean noSetFMUStatePriorToCurrentPoint,
	fmi3Boolean* eventHandlingNeeded,
	fmi3Boolean* terminateSimulation,
	fmi3Boolean* earlyReturn,
	fmi3Float64* lastSuccessfulTime)
{
	ModelInstance* _c=(ModelInstance *)c;
	int retVal;

	// get current communication point
	_c->curComm=currentCommunicationPoint;
	// get current communication step size
	_c->communicationStepSize=communicationStepSize;
	// assign current communication point to value to be sent
	_c->simTimSen=_c->curComm;

	// initialize the nexComm value to start communication point
	if (_c->firstCallDoStep){
		_c->nexComm=_c->curComm;

		// check for the first communication instant
		if (eventHandlingNeeded)
		{
			logWarning(_c,
				"fmi3DoStep: eventHandlingNeeded has been set to %d. "
				" EnergyPlus FMU does however not support this option. The flag will be ignored..\n",
				eventHandlingNeeded);
			return fmi3Warning;
		}

		if (earlyReturn)
		{
			logWarning(_c,
				"fmi3DoStep: earlyReturn has been set to %d. "
				" EnergyPlus FMU does however not support this option. The flag will be ignored..\n",
				eventHandlingNeeded);
			return fmi3Warning;
		}

		// check if FMU needs to support rollback
		if (!noSetFMUStatePriorToCurrentPoint)
		{
			logWarning(_c,
				"fmi3DoStep: noSetFMUStatePriorToCurrentPoint has been set to %d."
				" EnergyPlus FMU does however not support this option. The flag will be ignored.",
				noSetFMUStatePriorToCurrentPoint);
			return fmi3Warning;
		}

	}

	// check for the first communication instant
	if (_c->firstCallDoStep && (fabs(_c->curComm -
		_c->tStartFMU) > 1e-10))
	{
		logError(_c,
			"fmi3DoStep: An error occured in a previous call. First communication time: %f !=tStart: %f.\n",
			_c->curComm, _c->tStartFMU);
		return fmi3Error;
	}


	// check whether the communication step size is different from null
	if (_c->communicationStepSize==0)
	{
		logError(_c,
		"fmi3DoStep: An error occured in a previous call. CommunicationStepSize cannot be null.\n");
		return fmi3Error;
	}

	// check whether the communication step size is different from time step in input file
	if ( fabs(_c->communicationStepSize - (3600/_c->timeStepIDF)) > 1e-10)
	{
		logError(_c, "fmi3DoStep:"
			" An error occured in a previous call. CommunicationStepSize: %f is different from time step: %d in input file.\n",
			_c->communicationStepSize, _c->timeStepIDF);
		return fmi3Error;
	}

	// check whether communication point is valid
	if ((_c->curComm) < 0 || ((_c->firstCallDoStep==0)
		&& (_c->curComm > _c->nexComm))){
			logError(_c, "fmi3DoStep:"
				" An error occured in a previous call. Communication point must be positive and monoton increasing.\n");
			return fmi3Error;
	}

	// check whether current communication point is valid
	if ((_c->firstCallDoStep==0)
		&& (fabs(_c->curComm - _c->nexComm) > 1e-10))
	{
		logError(_c, "fmi3DoStep: "
			"Current communication point: %f is not equals to the previous simulation time + "
			"communicationStepSize: %f + %f.\n",
			_c->curComm, _c->nexComm,
			_c->communicationStepSize);
		return fmi3Error;
	}

	// check end of simulation
	if (_c->curComm==_c->tStopFMU){
		// set the communication flags to 1 to send stop signal to EnergyPlus
		logWarning(_c,
			"Warning", "fmi3DoStep: Current communication point: %f of FMU instance: %s "
			"is equals to end of simulation: %f.\n",
			_c->curComm, _c->instanceName, _c->tStopFMU);
		return fmi3Warning;
	}

	// check if current communication is larger than end of simulation
	if (_c->curComm > _c->tStopFMU){
		// set the communication flags to 1 to send stop signal to EnergyPlus
		logError(_c, "fmi3DoStep:"
			" Current communication point: %f is larger than end of simulation time: %f.\n",
			_c->curComm, _c->tStopFMU);
		return fmi3Error;
	}
	// check end of simulation
	if (_c->curComm +
		_c->communicationStepSize > _c->tStopFMU){
			// set the communication flags to 1 to send stop signal to EnergyPlus
			logError(_c, "fmi3DoStep: "
				"Current communication point: %f  + communicationStepsize: %f  is larger than "
				"end of simulation time: %f.\n",
				_c->curComm, _c->communicationStepSize,
				_c->tStopFMU);
			return fmi3Error;
	}

	// check whether all input and outputs are available and then do the time stepping
	if (_c->firstCallDoStep
		||
		(_c->firstCallDoStep==0)
		&& _c->curComm <=(_c->tStopFMU -
		_c->communicationStepSize)) {
			if (_c->flaWri !=1){
				_c->flaGetRea=1;
				if (_c->flaGetRealCall==0)
				{
					retVal=readfromsocketFMU(&(_c->newsockfd), &(_c->flaRea),
						&(_c->numOutVar), &zI, &zI, &(_c->simTimRec),
						_c->outVec, NULL, NULL);
				}
				retVal=writetosocketFMU(&(_c->newsockfd), &(_c->flaWri),
					&_c->numInVar, &zI, &zI, &(_c->simTimSen),
					_c->inVec, NULL, NULL);

				if (_c->flaGetRealCall==1)
				{
					_c->flaGetRealCall=0;
				}
			}
			_c->readReady=0;
			_c->writeReady=0;
			_c->setCounter=0;
			_c->getCounter=0;
	}

	// calculate next communication point
	_c->nexComm=_c->curComm
		+ _c->communicationStepSize;
	// set the firstcall flag to zero
	if (_c->firstCallDoStep)
	{
		_c->firstCallDoStep=0;
	}
	return fmi3OK;
}

//////////////////////////////////////////////////////////////////
/////  This method is used to setup the parameters for the FMU
/////
/////\param c The FMU instance.
/////\param toleranceDefined The flag to specify if tolerance is defined.
/////\param tolerance The FMU tolerance.
/////\param startTime The FMU start time.
/////\param stopTimeDefined The flag to specify if stopTime is defined.
/////\param stopTime The FMU stop time.
/////\return fmi3Warning if no error occurred.
//////////////////////////////////////////////////////////////////
//DllExport fmi3Status fmi3SetupExperiment(fmi3Instance c, fmi3Boolean toleranceDefined,
//	fmi3Float64 tolerance, fmi3Float64 startTime, fmi3Boolean stopTimeDefined, fmi3Float64 stopTime)
//{
//	ModelInstance* _c = (ModelInstance *)c;
//	_c->tStartFMU = startTime;
//	_c->tStopFMU = stopTime;
//
//	if (stopTimeDefined == fmi3False) {
//		logWarning(_c,
//			"fmi3SetupExperiment: The StopTimeDefined parameter is set to %d. This is not valid."
//			" EnergyPlus FMU requires a stop time and will use the stop time %f which is provided.\n",
//			stopTimeDefined, stopTime);
//	}
//
//	if (toleranceDefined == fmi3True) {
//		logWarning(_c,
//			"fmi3SetupExperiment: The toleranceDefined parameter is set to %d."
//			" However, EnergyPlus FMU won't use the tolerance %f which is provided.\n",
//			toleranceDefined, tolerance);
//	}
//	return fmi3OK;
//}
//

////////////////////////////////////////////////////////////////
///  This method is used to terminate the FMU instance
///
///\param c The FMU instance.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3Terminate(fmi3Instance c)
{
		ModelInstance* _c=(ModelInstance *)c;
		logEvent(_c,
		"fmi3Terminate: fmiFreeInstanceSlave must be called to free the FMU instance.\n");
		return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to reset the FMU instance
///
///\param c The FMU instance.
///\return fmi3Warning if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fm2ResetSlave(fmi3Instance c)
{
	ModelInstance* _c=(ModelInstance *)c;
	logWarning(_c,
		"fm2ResetSlave: fm2ResetSlave:: is not provided.\n");
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to free the FMU instance
///
///\param c The FMU instance.
////////////////////////////////////////////////////////////////
DllExport void fmi3FreeInstance(fmi3Instance c)
{
	if (c!=NULL){
		ModelInstance* _c=(ModelInstance *)c;
		int retVal;

#ifndef _MSC_VER
		int status;
#endif
		// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
		retVal=_chdir(_c->fmuOutput);
#else
		retVal=chdir(_c->fmuOutput);
#endif
		logEvent(_c,
		"fmi3FreeInstance: The function fmi3FreeInstance of instance %s is executed.\n",
		_c->instanceName);
		// send end of simulation flag
		_c->flaWri=1;
		_c->flaRea=1;
		retVal=exchangedoubleswithsocketFMUex (&(_c->newsockfd), &(_c->flaWri),
			&(_c->flaRea), &(_c->numOutVar), &(_c->numInVar),
			&(_c->simTimRec), _c->outVec, &(_c->simTimSen),
			_c->inVec);
		// close socket
		closeipcFMU(&(_c->sockfd));
		closeipcFMU(&(_c->newsockfd));
		// clean-up temporary files
		findFileDelete();
#ifdef _MSC_VER
		// wait for object to terminate
		WaitForSingleObject (_c->pid, INFINITE);
		TerminateProcess(_c->pid, 0);
#else
		waitpid (_c->pid, &status, 0);
#endif

#ifdef _MSC_VER
		// clean-up winsock
		WSACleanup();
#endif
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal=_chdir(_c->cwd);
#else
		retVal=chdir(_c->cwd);
#endif
		// FIXME: Freeing the FMU instance seems to cause
		// segmentation fault in Dymola 2016, thus
		// the FMU instance will not be released here.
		freeInstanceResources(_c);
		return;
	}
	else{
		printf ("fmi3FreeInstance: FMU instance was NULL. fmi3FreeInstance will return.\n");
		return;
	}
}

////////////////////////////////////////////////////////////////
///  This method is used to set reals in the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param nvr The number of variables to set.
///\param value The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3SetReal(fmi3Instance c, 
	const fmi3ValueReference vr[], 
	size_t nvr, 
	const fmi3Float64 value[])
{
	ModelInstance* _c=(ModelInstance *)c;
	// to prevent the fmi3SetReal to be called before the FMU is initialized
	if (_c->firstCallIni==0)
	{
		// fmi3ValueReference to check for input variable
		fmi3ValueReference vrTemp;
		//ScalarVariable** vars;
		int i, k;
		int n = getScalarVariableSize(_c->md);

		//fixme
		//vars=_c->md->modelVariables;
		if (!_c->writeReady){
			for(i=0; i<nvr; i++)
			{
				for (k=0; k<n; k++) {
					// fixme
					//ScalarVariable* svTemp=vars [k];
					ScalarVariable *svTemp = getScalarVariable(_c->md, k);
					//if (getAlias(svTemp)!=enu_noAlias) continue;
					if (getCausality(svTemp) !=enu_input) continue;
					vrTemp=getValueReference(svTemp);
					if (vrTemp==vr[i]){
						_c->inVec[vr[i]-1]=value[i];
						_c->setCounter++;
					}
				}
			}
			if (_c->setCounter==_c->numInVar)
			{
				_c->writeReady=1;
			}
		}
		//if (_c->firstCallSetReal){
		//	_c->firstCallSetReal=0;
		//}
		return fmi3OK;
	}
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set integers in the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param value The values of variables to set.
/// \param nvr The number of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3SetInt64(fmi3Instance c, const fmi3ValueReference vr[], const fmi3Int64 value[], size_t nvr)
{
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		logError(_c,
			"fmi3SetInt64: fmi3SetInt64: was called. The FMU does not contain integer variables to set.\n");
		return fmi3Error;
	}
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set booleans in the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param value The values of variables to set.
/// \param nvr The number of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3SetBoolean(fmi3Instance c, const fmi3ValueReference vr[], const fmi3Boolean value[], size_t nvr)
{
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		logError(_c,
			"fmi3SetBoolean: fmi3SetBoolean: was called. The FMU does not contain boolean variables to set.\n");
		return fmi3Error;
	}
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set strings in the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param value The values of variables to set.
/// \param nvr The number of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3SetString(fmi3Instance c, const fmi3ValueReference vr[], const fmi3String value[], size_t nvr)
{
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		logError(_c,
		"fmi3SetString: fmi3SetString: was called. The FMU does not contain string variables to set.\n");
		return fmi3Error;
	}
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get reals from the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param value The values of variables to get.
/// \param nvr The number of variables to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3GetFloat64(fmi3Instance c, const fmi3ValueReference vr[], fmi3Float64 value[], size_t nvr)
{
	ModelInstance* _c=(ModelInstance *)c;
	int retVal;
	// to prevent the fmi3GetFloat64 to be called before the FMU is initialized
	if (_c->firstCallIni==0){
		fmi3ValueReference vrTemp;
		//ScalarVariable** vars;
		int i, k;
		int n = getScalarVariableSize(_c->md);
		//fixme
		//vars=_c->md->modelVariables;
		_c->flaGetRealCall=1;

		if (_c->firstCallGetReal||((_c->firstCallGetReal==0)
			&& (_c->flaGetRea)))  {
				// read the values from the server
				retVal=readfromsocketFMU(&(_c->newsockfd), &(_c->flaRea),
					&(_c->numOutVar), &zI, &zI, &(_c->simTimRec),
					_c->outVec, NULL, NULL);
				// reset flaGetRea
				_c->flaGetRea=0;
		}
		if (!_c->readReady)
		{
			for(i=0; i<nvr; i++)
			{
				for (k=0; k<n; k++) {
					// fixme
					//ScalarVariable* svTemp=vars [k];
					ScalarVariable *svTemp = getScalarVariable(_c->md, k);
					//if (getAlias(svTemp)!=enu_noAlias) continue;
					if (getCausality(svTemp) !=enu_output) continue;
					vrTemp=getValueReference(svTemp);
					if (vrTemp==vr[i]){
						value[i]=_c->outVec[vr[i]- MAX_NUM_INPUTS+1];
						_c->getCounter++;
					}
				}
			}
			if (_c->getCounter==_c->numOutVar)
			{
				_c->readReady=1;
			}
		}
		if(_c->firstCallGetReal)
		{
			_c->firstCallGetReal=0;
		}
		return fmi3OK;
	}
	return fmi3OK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set debug logging
///
///\param c The FMU instance.
///\param loggingOn The FMU loggingOn flag.
///\param nCategories The number of categories.
///\param categories The categories.
///\return fmi3Warning if no error occurred.
////////////////////////////////////////////////////////////////
Status fmi3SetDebugLogging(ModelInstance* comp, bool loggingOn, size_t nCategories, const char* const categories[]) {

	if (loggingOn) {
		for (size_t i = 0; i < nCategories; i++) {
			if (categories[i] == NULL) {
				logError(comp, "Log category[%d] must not be NULL", i);
				return Error;
			}
			else if (strcmp(categories[i], "logEvents") == 0) {
				comp->logEvents = true;
			}
			else if (strcmp(categories[i], "logStatusError") == 0) {
				comp->logErrors = true;
			}
			else if (strcmp(categories[i], "logStatusWarning") == 0) {
				comp->logWarnings = true;
			}
			else {
				logError(comp, "Log category[%d] must be one of logEvents, logStatusError, or logStatusWarning but was %s", i, categories[i]);
				return Error;
			}
		}
	}
	else {
		// disable logging
		comp->logEvents = false;
		comp->logErrors = false;
		comp->logWarnings = false;
	}

	return OK;
}


/***************************************************
Types for Common Functions (UNUSED)
****************************************************/


/* tag::Instantiate[] */

////////////////////////////////////////////////////////////////
///  This method is used to instantiated the FMU
///
///\param instanceName A unique identifier for the FMU instance..
///\param instantiationToken The instantiationToken.
///\param resourcePath The FMU resource location path.
///\param visible The flag to executes the FMU in windowless mode.
///\param loggingOn The flag to enable or disable debug.
///\param instanceEnvironment A pointer that must be passed to fmi3IntermediateUpdateCallback
///\param logMessage The callback function to log messages
////////////////////////////////////////////////////////////////
DllExport fmi3Instance fmi3InstantiateModelExchange(
	fmi3String                 instanceName,
	fmi3String                 instantiationToken,
	fmi3String                 resourcePath,
	fmi3Boolean                visible,
	fmi3Boolean                loggingOn,
	fmi3InstanceEnvironment    instanceEnvironment,
	fmi3LogMessageCallback     logMessage) 
{
	logMessage(instanceEnvironment, fmi3Error, "error",
		"fmi3InstantiateModelExchange: fmi3InstantiateModelExchange has been called but "
		" the FMU implements the Co-Simulation API."); 
	return NULL;
}

////////////////////////////////////////////////////////////////
///  This method is used to instantiated the FMU
///
///\param instanceName A unique identifier for the FMU instance..
///\param instantiationToken The instantiationToken.
///\param resourcePath The FMU resource location path.
///\param visible The flag to executes the FMU in windowless mode.
///\param loggingOn The flag to enable or disable debug.
///\param instanceEnvironment A pointer that must be passed to fmi3IntermediateUpdateCallback
///\param logMessage The callback function to log messages
///\param clockUpdate The callback to clock update
///\param lockPreemption The callback to for lock preemption
///\param unlockPreemption The callback to for unlock preemption
////////////////////////////////////////////////////////////////
DllExport fmi3Instance fmi3InstantiateScheduledExecution(
	fmi3String                     instanceName,
	fmi3String                     instantiationToken,
	fmi3String                     resourcePath,
	fmi3Boolean                    visible,
	fmi3Boolean                    loggingOn,
	fmi3InstanceEnvironment        instanceEnvironment,
	fmi3LogMessageCallback         logMessage,
	fmi3ClockUpdateCallback        clockUpdate,
	fmi3LockPreemptionCallback     lockPreemption,
	fmi3UnlockPreemptionCallback   unlockPreemption)
{
	logMessage(instanceEnvironment, fmi3Error, "error",
		"fmi3InstantiateModelExchange: fmi3InstantiateModelExchange has been called but "
		" the FMU implements the Co-Simulation API."); /*fixme*/
	return NULL;
};
/* end::Instantiate[] */


/* Enter and exit initialization mode, enter event mode, terminate and reset */

/* tag::EnterEventMode[] */
////////////////////////////////////////////////////////////////
///  This method is used to serialize the FMU states
///
///\param c The FMU instance.
///\return fmi3Warning if no error occured.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3EnterEventMode(fmi3Instance c)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3EnterEventMode: fmi3EnterEventMode: is not provided.\n");
	 return fmi3Warning;

 };
/* end::EnterEventMode[] */


/* Getting and setting variable values */
/* tag::Getters[] */

 ////////////////////////////////////////////////////////////////
///  This method is used to get floats in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetFloat32(fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Float32 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetFloat32: fmi3GetFloat32: was called. The FMU does not contain Float32 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };

////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetInt8   (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int8 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetInt8: fmi3GetInt8: was called. The FMU does not contain Int8 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };


////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetUInt8  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt8 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetUInt8: fmi3GetUInt8: was called. The FMU does not contain UInt8 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };

////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetInt16  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int16 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetInt16: fmi3GetInt16: was called. The FMU does not contain Int16 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };


////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetUInt16 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt16 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetUInt16: fmi3GetUInt16: was called. The FMU does not contain UInt16 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };


////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetInt32  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Int32 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetInt32: fmi3GetInt32: was called. The FMU does not contain Int32 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };

////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetUInt32 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt32 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetUInt32: fmi3GetUInt32: was called. The FMU does not contain UInt32 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };


 ////////////////////////////////////////////////////////////////
///  This method is used to get integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetUInt64 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3UInt64 values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetUInt64: fmi3GetUInt64: was called. The FMU does not contain UInt64 variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };

 ////////////////////////////////////////////////////////////////
///  This method is used to get binary in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param The value sizes 
///\param values The values of variables to get.
///\param The number of values to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetBinary (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      size_t valueSizes[],
                                      fmi3Binary values[],
                                      size_t nValues)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nValues > 0)
	 {
		 logError(_c,
			 "fmi3GetBinary: fmi3GetBinary: was called. The FMU does not contain Binary variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 };
/* end::Getters[] */

/* tag::GetClock[] */

////////////////////////////////////////////////////////////////
///  This method is used to get clocks in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetClock  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      fmi3Clock values[])
 {
	 ModelInstance* _c = (ModelInstance*)c;

	logWarning(_c,
		"fmi3GetClock: fmi3GetClock: was called. The FMU does not contain any Clock variables to get.\n");

	 return fmi3Warning;
 };
/* end::GetClock[] */

/* tag::Setters[] */

////////////////////////////////////////////////////////////////
///  This method is used to set floats in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param The number of value references.
///\param values The values of variables to set.
/// \param nValues The number of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetFloat32(fmi3Instance c,
	 const fmi3ValueReference valueReferences[],
	 size_t nValueReferences,
	 const fmi3Float32 values[],
	 size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetFloat32: fmi3SetFloat32: was called. The FMU does not contain Float32 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }


 ////////////////////////////////////////////////////////////////
 ///  This method is used to set integers in the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param valueReferences The value references.
 ///\param The number of value references.
 ///\param values The values of variables to set.
 /// \param nValues The number of variables to set.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetInt8   (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int8 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetInt8: fmi3SetInt8: was called. The FMU does not contain Int8 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to set integers in the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param valueReferences The value references.
 ///\param The number of value references.
 ///\param values The values of variables to set.
 /// \param nValues The number of variables to set.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetUInt8  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt8 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetUInt8: fmi3SetUInt8: was called. The FMU does not contain UInt8 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to set integers in the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param valueReferences The value references.
 ///\param The number of value references.
 ///\param values The values of variables to set.
 /// \param nValues The number of variables to set.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetInt16  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int16 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetInt16: fmi3SetInt16: was called. The FMU does not contain Int16 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to set integers in the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param valueReferences The value references.
 ///\param nValueReferences The number of value references to set.
 ///\param values The values of variables to set.
 ///\param nValues The values of variables to set.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
DllExport fmi3Status fmi3SetUInt16 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt16 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetUInt16: fmi3SetUInt16: was called. The FMU does not contain UInt16 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }


////////////////////////////////////////////////////////////////
///  This method is used to set integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param nValueReferences The number of value references to set.
///\param values The values of variables to set.
///\param nValues The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetInt32  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Int32 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetInt32: fmi3SetInt32: was called. The FMU does not contain Int32 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

////////////////////////////////////////////////////////////////
///  This method is used to set integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param nValueReferences The number of value references to set.
///\param values The values of variables to set.
///\param nValues The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetUInt32 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt32 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetUInt32: fmi3SetUInt32: was called. The FMU does not contain UInt32 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

////////////////////////////////////////////////////////////////
///  This method is used to set integers in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param nValueReferences The number of value references to set.
///\param values The values of variables to set.
///\param nValues The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetUInt64 (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3UInt64 values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetUInt64: fmi3SetUInt64: was called. The FMU does not contain UInt64 variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }


////////////////////////////////////////////////////////////////
///  This method is used to set binary in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param nValueReferences The number of value references to set.
///\param valueSizes The value sizes.
///\param values The values of variables to set.
///\param nValues The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetBinary (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const size_t valueSizes[],
                                      const fmi3Binary values[],
                                      size_t nValues)
 {
	 if (nValues > 0)
	 {
		 ModelInstance* _c = (ModelInstance*)c;

		 logError(_c,
			 "fmi3SetBinary: fmi3SetBinary: was called. The FMU does not contain Binary variables to set.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }
/* end::Setters[] */
/* tag::SetClock[] */

////////////////////////////////////////////////////////////////
///  This method is used to set clocks in the FMU instance
///
///\param c The FMU instance.
///\param valueReferences The value references.
///\param nValueReferences The number of value references to set.
///\param values The values of variables to set.
///\param nValues The values of variables to set.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetClock  (fmi3Instance c,
                                      const fmi3ValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmi3Clock values[])
 {
	 ModelInstance* _c = (ModelInstance*)c;

	 logWarning(_c,
		 "fmi3SetClock: fmi3SetClock: was called. The FMU does not contain any Clock variables to set.\n");

	 return fmi3Warning;

 }
 
/* end::SetClock[] */

/* Getting Variable Dependency Information */
/* tag::GetNumberOfVariableDependencies[] */

////////////////////////////////////////////////////////////////
///  This method is used to get the number of variable dependencies
///
///\param c The FMU instance.
///\param valueReference The value reference.
///\param nDependencies The number of dependencies.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance c,
                                                           fmi3ValueReference valueReference,
                                                           size_t* nDependencies)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetNumberOfVariableDependencies: fmi3GetNumberOfVariableDependencies:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetNumberOfVariableDependencies[] */

/* tag::GetVariableDependencies[] */
 ////////////////////////////////////////////////////////////////
///  This method is used to get the variable dependencies
///
///\param c The FMU instance.
///\param dependent The dependent.
///\param elementIndicesOfDependent The number of dependent indices.
///\param independents The independent.
///\param elementIndicesOfIndependents The number of independent indices.
///\param dependencyKinds The dependency kind.
 ///\param nDependencies The number of dependencies.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetVariableDependencies(fmi3Instance c,
                                                   fmi3ValueReference dependent,
                                                   size_t elementIndicesOfDependent[],
                                                   fmi3ValueReference independents[],
                                                   size_t elementIndicesOfIndependents[],
                                                   fmi3DependencyKind dependencyKinds[],
                                                   size_t nDependencies)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetVariableDependencies: fmi3GetVariableDependencies:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetVariableDependencies[] */

/* Getting partial derivatives */

/* tag::GetAdjointDerivative[] */
 ////////////////////////////////////////////////////////////////
///  This method is used to get the adjoint derivatives
///
///\param c The FMU instance.
///\param unknowns The unknowns.
///\param nUnknowns The number of unknowns.
///\param knowns The knowns.
///\param seed The seeds.
///\param sensitivity The sensitivity values.
 ///\param nSensitivity The number of sensitivity values.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////

 DllExport fmi3Status fmi3GetAdjointDerivative(fmi3Instance c,
                                                const fmi3ValueReference unknowns[],
                                                size_t nUnknowns,
                                                const fmi3ValueReference knowns[],
                                                size_t nKnowns,
                                                const fmi3Float64 seed[],
                                                size_t nSeed,
                                                fmi3Float64 sensitivity[],
                                                size_t nSensitivity)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetAdjointDerivative: fmi3GetAdjointDerivative:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetAdjointDerivative[] */

/* Entering and exiting the Configuration or Reconfiguration Mode */

/* tag::EnterConfigurationMode[] */
 ////////////////////////////////////////////////////////////////
///  This method is used to enter configuration mode
///
///\param c The FMU instance.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3EnterConfigurationMode(fmi3Instance c)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3EnterConfigurationMode: fmi3EnterConfigurationMode:: is not provided.\n");
	 return fmi3OK;
 }
/* end::EnterConfigurationMode[] */

/* tag::ExitConfigurationMode[] */
  ////////////////////////////////////////////////////////////////
///  This method is used to exit configuration mode
///
///\param c The FMU instance.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3ExitConfigurationMode(fmi3Instance c)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3ExitConfigurationMode: fmi3ExitConfigurationMode:: is not provided.\n");
	 return fmi3OK;
 }
/* end::ExitConfigurationMode[] */

/* tag::GetIntervalDecimal[] */
 DllExport fmi3Status fmi3GetIntervalDecimal(fmi3Instance c,
                                              const fmi3ValueReference valueReferences[],
                                              size_t nValueReferences,
                                              fmi3Float64 intervals[],
                                              fmi3IntervalQualifier qualifiers[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetIntervalDecimal: fmi3GetIntervalDecimal:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetIntervalDecimal[] */

/* tag::GetIntervalFraction[] */
 DllExport fmi3Status fmi3GetIntervalFraction(fmi3Instance c,
                                               const fmi3ValueReference valueReferences[],
                                               size_t nValueReferences,
                                               fmi3UInt64 counters[],
                                               fmi3UInt64 resolutions[],
                                               fmi3IntervalQualifier qualifiers[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetIntervalFraction: fmi3GetIntervalFraction:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetIntervalFraction[] */

/* tag::GetShiftDecimal[] */
 DllExport fmi3Status fmi3GetShiftDecimal(fmi3Instance c,
                                           const fmi3ValueReference valueReferences[],
                                           size_t nValueReferences,
                                           fmi3Float64 shifts[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetShiftDecimal: fmi3GetShiftDecimal:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetShiftDecimal[] */

/* tag::GetShiftFraction[] */
 DllExport fmi3Status fmi3GetShiftFraction(fmi3Instance c,
                                            const fmi3ValueReference valueReferences[],
                                            size_t nValueReferences,
                                            fmi3UInt64 counters[],
                                            fmi3UInt64 resolutions[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetShiftFraction: fmi3GetShiftFraction:: is not provided.\n");
	 return fmi3OK;
 }
/* end::GetShiftFraction[] */

/* tag::SetIntervalDecimal[] */
 DllExport fmi3Status fmi3SetIntervalDecimal(fmi3Instance c,
                                              const fmi3ValueReference valueReferences[],
                                              size_t nValueReferences,
                                              const fmi3Float64 intervals[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SetIntervalDecimal: fmi3SetIntervalDecimal:: is not provided.\n");
	 return fmi3OK;
 }
/* end::SetIntervalDecimal[] */

/* tag::SetIntervalFraction[] */
 DllExport fmi3Status fmi3SetIntervalFraction(fmi3Instance c,
                                               const fmi3ValueReference valueReferences[],
                                               size_t nValueReferences,
                                               const fmi3UInt64 counters[],
                                               const fmi3UInt64 resolutions[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SetIntervalFraction: fmi3SetIntervalFraction:: is not provided.\n");
	 return fmi3OK;
 }
/* end::SetIntervalFraction[] */

/* tag::SetShiftDecimal[] */
 DllExport fmi3Status fmi3SetShiftDecimal(fmi3Instance c,
                                           const fmi3ValueReference valueReferences[],
                                           size_t nValueReferences,
                                           const fmi3Float64 shifts[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SetShiftDecimal: fmi3SetShiftDecimal:: is not provided.\n");
	 return fmi3OK;
 }
/* end::SetShiftDecimal[] */

/* tag::SetShiftFraction[] */
 DllExport fmi3Status fmi3SetShiftFraction(fmi3Instance c,
                                            const fmi3ValueReference valueReferences[],
                                            size_t nValueReferences,
                                            const fmi3UInt64 counters[],
                                            const fmi3UInt64 resolutions[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SetShiftFraction: fmi3SetShiftFraction:: is not provided.\n");
	 return fmi3OK;
 }
/* end::SetShiftFraction[] */

/* tag::EvaluateDiscreteStates[] */
 ////////////////////////////////////////////////////////////////
///  This method is used to evaluate discrete states
///
///\param c The FMU instance.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance c)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3EvaluateDiscreteStates: fmi3EvaluateDiscreteStates:: is not provided.\n");
	 return fmi3OK;
 }
/* end::EvaluateDiscreteStates[] */

/* tag::UpdateDiscreteStates[] */
 DllExport fmi3Status fmi3UpdateDiscreteStates(fmi3Instance c,
                                                fmi3Boolean* discreteStatesNeedUpdate,
                                                fmi3Boolean* terminateSimulation,
                                                fmi3Boolean* nominalsOfContinuousStatesChanged,
                                                fmi3Boolean* valuesOfContinuousStatesChanged,
                                                fmi3Boolean* nextEventTimeDefined,
                                                fmi3Float64* nextEventTime)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3UpdateDiscreteStates: fmi3UpdateDiscreteStates:: is not provided.\n");
	 return fmi3OK;
 }
/* end::UpdateDiscreteStates[] */


 ////////////////////////////////////////////////////////////////
///  This method is used to get integers from the FMU instance
///
///\param c The FMU instance.
///\param fmi3ValueReference The value reference.
///\param value The values of variables to get.
/// \param nvr The number of variables to get.
///\return fmi3OK if no error occurred.
////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetInt64(fmi3Instance c, const fmi3ValueReference vr[], fmi3Int64 value[], size_t nvr)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nvr > 0)
	 {
		 logError(_c,
			 "fmi3GetInt64: fmi3GetInt64: was called. The FMU does not contain integer variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to get booleans from the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param fmi3ValueReference The value reference.
 ///\param value The values of variables to get.
 /// \param nvr The number of variables to get.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetBoolean(fmi3Instance c, const fmi3ValueReference vr[], fmi3Boolean value[], size_t nvr)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nvr > 0)
	 {
		 logError(_c,
			 "fmi3GetBoolean: fmi3GetBoolean: was called. The FMU does not contain boolean variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to get strings from the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param fmi3ValueReference The value reference.
 ///\param value The values of variables to get.
 /// \param nvr The number of variables to get.
 ///\return fmi3OK if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetString(fmi3Instance c, const fmi3ValueReference vr[], fmi3String  value[], size_t nvr)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 if (nvr > 0)
	 {
		 logError(_c,
			 "fmi3GetString: fmi3GetString: was called. The FMU does not contain string variables to get.\n");
		 return fmi3Error;
	 }
	 return fmi3OK;
 }


 ////////////////////////////////////////////////////////////////
 ///  This method is used to get directional derivatives
 ///  from the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param vUnknown_ref The value reference of the unknowns variables.
 ///\param nUnknown The number of unkown variables.
 ///\param vKnown_ref The value reference of the knowns variables.
 ///\param nKnown The number of known variables.
 ///\param dvKnown The values of known variables.
 ///\param see The values of seed variables.
 ///\param The values of the sensitivity array.
 ///\param The length of the sensitivity array. 
 ///\return fmi3Warning if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetDirectionalDerivative(fmi3Instance c,
	 const fmi3ValueReference unknowns[],
	 size_t nUnknowns,
	 const fmi3ValueReference knowns[],
	 size_t nKnowns,
	 const fmi3Float64 seed[],
	 size_t nSeed,
	 fmi3Float64 sensitivity[],
	 size_t nSensitivity)

 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetDirectionalDerivative: fmi3GetDirectionalDerivative: Directional derivatives are not provided.\n");
	 return fmi3Warning;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to get real output
 ///  derivatives from the FMU instance
 ///
 ///\param c The FMU instance.
 ///\param fmi3ValueReference The value reference.
 ///\param nvr The number of variables to get.
 ///\param order The order of the derivatives.
 ///\param value The values of variables to get.
 ///\return fmi3Warning if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetOutputDerivatives(fmi3Instance c, const fmi3ValueReference vr[], size_t nvr,
	 const fmi3Int32 order[], fmi3Float64 value[])
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetFloat64OutputDerivatives: fmi3GetFloat64OutputDerivatives: Real Output Derivatives are not provided.\n");
	 return fmi3Warning;
 }


 ////////////////////////////////////////////////////////////////
 ///  This method is used to reset the FMU
 ///
 ///\param c The FMU instance.
 ///\return fmi3Warning if no error occurred.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3Reset(fmi3Instance c)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "Warning", "fmi3Reset: The function fmi3Reset: is not provided.\n");
	 return fmi3Warning;
 }

 //////////////////////////////////////////////////////////////////
 /////  This method is used to get FMU status
 /////
 /////\param c The FMU instance.
 /////\param fmi3StatusKind The status information.
 /////\param value The status value.
 /////\return fmi3Warning if no error occured.
 //////////////////////////////////////////////////////////////////
 //DllExport fmi3Status fmi3GetStatus(fmi3Instance c, const fmi3StatusKind s, fmi3Status* value)
 //{
 //	ModelInstance* _c=(ModelInstance *)c;
 //	logWarning(_c,
 //		"fmi3GetStatus: fmi3GetStatus: is not provided.\n");
 //	return fmi3Warning;
 //}


 ////////////////////////////////////////////////////////////////
 ///  This method is used to get the FMU states
 ///
 ///\param c The FMU instance.
 ///\param fmi3FMUState* FMUState.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3GetFMUState(fmi3Instance c, fmi3FMUState* FMUState)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3GetFMUState: fmi3GetFMUState: is not provided.\n");
	 return fmi3Warning;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to set the FMU states
 ///
 ///\param c The FMU instance.
 ///\param fmi3FMUState FMUState.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SetFMUState(fmi3Instance c, fmi3FMUState FMUState)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SetFMUState: fmi3SetFMUState: is not provided.\n");
	 return fmi3Warning;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to free the FMU states
 ///
 ///\param c The FMU instance.
 ///\param fmi3FMUState* FMUState.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3FreeFMUState(fmi3Instance c, fmi3FMUState* FMUState)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3FreeFMUState: fmi3FreeFMUState: is not provided.\n");
	 return fmi3Warning;
 }


 ////////////////////////////////////////////////////////////////
 ///  This method is used to get the size of the byte vector
 ///  needed to store the FMU states
 ///
 ///\param c The FMU instance.
 ///\param fmi3FMUState FMUState.
 ///\param size_t *size.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SerializedFMUStateSize(fmi3Instance c,
	 fmi3FMUState FMUState, size_t* size)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SerializedFMUStateSize: fmi3SerializedFMUStateSize: is not provided.\n");
	 return fmi3Warning;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to serialize the FMU states
 ///
 ///\param c The FMU instance.
 ///\param fmi3FMUState FMUState.
 ///\param fmi3Byte serializedState[].
 ///\param size_t size.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3SerializeFMUState(fmi3Instance c, fmi3FMUState FMUState,
	 fmi3Byte serializedState[], size_t size)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3SerializeFMUState: fmi3SerializeFMUState: is not provided.\n");
	 return fmi3Warning;
 }

 ////////////////////////////////////////////////////////////////
 ///  This method is used to deserialize the FMU states
 ///
 ///\param c The FMU instance.
 ///\param const fmi3Byte serializedState[].
 ///\param size_t size.
 ///\param fmi3FMUState* FMUState.
 ///\return fmi3Warning if no error occured.
 ////////////////////////////////////////////////////////////////
 DllExport fmi3Status fmi3DeSerializeFMUState(fmi3Instance c,
	 const fmi3Byte serializedState[], size_t size, fmi3FMUState* FMUState)
 {
	 ModelInstance* _c = (ModelInstance*)c;
	 logWarning(_c,
		 "fmi3DeSerializeFMUState: fmi3DeSerializeFMUState: is not provided.\n");
	 return fmi3Warning;
 }




void main(){
	double time;
	double tStart=0;               // start time
	double tStop=86400;               // start time
	const char* instanceToken="{19751346-37ce-4422-86bf-bd1609f0d579}";                // global unique id of the fmu
	fmi3Instance c;                  // instance of the fmu
	//fmi3Status fmiFlag;               // return code of the fmu functions
	const char* fmuResourceLocation ="D:\proj\\lbnl\\eplustofmu\\_fmu_export_schedule\\resources\\"; // path to the unzipped fmu location as URL
	const char* modelDescriptionPath = "D:\\proj\\lbnl\\eplustofmu\\_fmu_export_schedule\\modelDescription.xml"; // path to the unzipped fmu location as URL
	const char* mimeType="application/x-fmu-sharedlibrary"; // denotes tool in case of tool coupling
	fmi3Float64 timeout=1000;          // wait period in milli seconds, 0 for unlimited wait period"
	fmi3Boolean visible=fmi3False;   // no simulator user interface
	fmi3Boolean interactive=fmi3False; // simulation run without user interaction
	//fmi3CallbackFunctions callbacks= {fmuLogger, calloc, free, NULL, NULL};  // called by the model during simulation
	ModelDescription* md;            // handle to the parsed XML file
	fmi3String instanceName;          // instance name of the FMU
	int nSteps=0;
	int loggingOn=0;
	int retVal;
	//FILE* file;
	const fmi3ValueReference valRefIn[]={1};
	const fmi3ValueReference valRefOut[]={100001};
	fmi3Float64 valIn[1]={50000};
	fmi3Float64 valOut[1];
	char* fmiVersionStr;
	Element *defaultExp;
	fmi3Status fmi3Flag;                    // return code of the fmu functions
	fmi3Boolean toleranceDefined = fmi3False;  // true if model description define tolerance
	//int nCategories;
	//char **categories;
	fmi3Float64 tolerance = 0;                    // used in setting up the experiment
	ValueStatus vs = valueMissing;
	double tEnd=86400;

	// allocate memory for md
	md = (ModelDescription*)malloc(sizeof(ModelDescription*));

	md = parse(modelDescriptionPath);

	// get the GUID
	//guid = getAttributeValue((Element *)md, att_instatia);

	// get the instance name
	instanceName = getAttributeValue((Element *)getCoSimulation(md), att_modelIdentifier);

	//// instantiate the fmu
	c=fmi3InstantiateCosimulation("_fmu_export_schedule", instanceToken, fmuResourceLocation,
		visible, loggingOn, "False", "False", NULL, 0, NULL, NULL, NULL);

	tEnd = 12000;

	////if (nCategories > 0) {
	////	fmi3Flag = fmu->setDebugLogging(c, fmi3True, nCategories, categories);
	////	if (fmi3Flag > fmi3Warning) {
	////		return error("could not initialize model; failed FMI set debug logging");
	////	}
	////}

	////defaultExp = getDefaultExperiment(md);
	////if (defaultExp) tolerance = getAttributeDouble(defaultExp, att_tolerance, &vs);
	////if (vs == valueDefined) {
	////	toleranceDefined = fmi3True;
	////}

	//// setup experiment parameters in the FMU
	//fmi3Flag = fmi3SetupExperiment(c, toleranceDefined, tolerance, tStart, fmi3True, tEnd);
	//if (fmi3Flag > fmi3Warning) {
	//	 printf("could not initialize model; failed FMI setup experiment");
	//	 return;
	//}

	//// enter initialization in the FMU
	//fmi3Flag = fmi3EnterInitializationMode(c);
	//if (fmi3Flag > fmi3Warning) {
	//	printf("could not initialize model; failed FMI enter initialization mode");
	//	return;
	//}

	//// exit initialization in the FMU
	//fmi3Flag = fmi3ExitInitializationMode(c);
	//if (fmi3Flag > fmi3Warning) {
	//	 printf("could not initialize model; failed FMI exit initialization mode");
	//	 return;
	//}

	//time=0;

	//fmiVersionStr = extractVersion(modelDescriptionPath);
	//printf("This is the fmi version %s\n", fmiVersionStr);
	//while (time < tStop) {
	//	// set the inputs
	//	retVal = fmi3SetReal(c, valRefIn, 1, valIn);
	//	// do step
	//	retVal= fmi3DoStep(c, time, 900, 1);
	//	// get the outputs
	//	retVal = fmi3GetFloat64(c, valRefOut, 1, valOut);
	//	printf ("This is the value of output %f\n", valOut);
	//	time=time+900;
	//}
	//// terminate the FMU
	//retVal=fmi3Terminate(c);
	//// free te FMU
	//fmi3FreeInstance(c);
	//printf ("Simulation successfully terminated\n");
}

/*

***********************************************************************************
Copyright Notice
----------------

Functional Mock-up Unit Export of EnergyPlus \A92013, The Regents of
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