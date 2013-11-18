// Methods for Functional Mock-up Unit Export of EnergyPlus.
/////////////////////////////////////////////////////////////////////
// \file   main.c
//
// \brief  FMI functions for FMU export project.
//
// \author Thierry Stephane Nouidui, David Lorenzetti, Michael Wetter
//         Simulation Research Group,
//         LBNL,
//         TSNouidui@lbl.gov
//
// \date   2012-08-03
//
/////////////////////////////////////////////////////////////////////

// define the model identifier name used in for
// the FMI functions
#define MODEL_IDENTIFIER SmOffPSZ
// define the FMI version supported
#define FMIVERSION "1.0"
#define NUMFMUsMax 10000
#define PATHLEN 10000
#define MAXHOSTNAME  10000
#define MAX_MSG_SIZE 1000
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "utilSocket.h" 
#include "fmiPlatformTypes.h"
#include "fmiFunctions.h"
#include "xml_parser_cosim.h"
#include "defines.h"
#include "reader.h" 
#include <errno.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#include <process.h>
#include <windows.h>
#include <direct.h>
#else
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <sys/types.h> /* pid_t */
#include <sys/ioctl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#endif

typedef struct idfFmu_t {
	int index;
	fmiCallbackFunctions functions;
	char instanceName[PATHLEN];
	char cwd[256];
	char* fmuLocation;
	char* fmuCalLocation;
	char* resources_p;
	char *xml_file_p;
	char* tmpResCon;
	char *fmuOutput;
	char* mID;
	char *mGUID;
	int numInVar;
	int numOutVar;
	int sockfd;
	int newsockfd;
	fmiReal timeout; 
	fmiBoolean visible;
	fmiBoolean interactive;
	fmiBoolean loggingOn;

	int firstCallGetReal;
	int firstCallSetReal;
	int firstCallDoStep;

	int firstCallFree;
	int firstCallTerm;
	int firstCallIni;
	int firstCallRes;
	int flaGetRealCall;
	int flaGetWri;
	int flaGetRea;
	int preInDoStep;
	int preInGetReal;
	int preInSetReal;
	int preInFree;
	int preInTerm;
	int preInIni;
	int preInRes;
	int flaWri;
	int flaRea;
	int readReady;
	int writeReady;
	int timeStepIDF;
	int getCounter;
	int setCounter;
	ModelDescription* md;

	fmiReal *inVec;
	fmiReal *outVec;
	fmiReal tStartFMU;
	fmiReal tStopFMU;
	fmiReal nexComm;
	fmiReal simTimSen;
	fmiReal simTimRec;
	fmiReal communicationStepSize;
	fmiReal curComm;

#ifdef _MSC_VER
	HANDLE  handle_EP;
	int pid;
#else
	pid_t  pid;
	pid_t  pidLoc;
#endif
} idfFmu_t;

static int zI = 0;
static int insNum = 0;
static int firstCallIns = 1;

static int arrsize = 0;
static idfFmu_t **fmuInstances;
static int fmuLocCoun = 0;
#define DELTA 10

////////////////////////////////////////////////////////////////////////////////////
/// Replace forward slash with backward slash
///
///\param s The input string.
///\param s The character to find.
///\param s The character to replace.
////////////////////////////////////////////////////////////////////////////////////
static void replace_char (char *s, const char find, const char replace) {
	while (*s != 0) {
		if (*s == find)
			*s = replace;
		s++;
	}
}

////////////////////////////////////////////////////////////////////////////////////
/// create a list of pointer to FMUs
///
///\param s The Pointer to FMU.
////////////////////////////////////////////////////////////////////////////////////
static void addfmuInstances(idfFmu_t* s){
	idfFmu_t **temp;
	if(fmuLocCoun == arrsize){
		temp = (idfFmu_t**)malloc(sizeof(idfFmu_t*) * (DELTA + arrsize));
		arrsize += DELTA;
		memcpy(temp, fmuInstances, fmuLocCoun);
		free(fmuInstances);
		fmuInstances = temp;
	}
	fmuInstances[fmuLocCoun++] = s;
}

////////////////////////////////////////////////////////////////////////////////////
/// write socket description file
///
///\param porNum The port number.
///\param hostName The host name.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
static int write_socket_cfg(int portNum, const char* hostName)
{
	FILE *fp;
	fp = fopen("socket.cfg", "w");
	if (fp == NULL) {
		printf("Can't open socket.cfg file!\n");
		exit(42);  // STL error code: File not open.
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
///\param str The path to the resource folder.
///\param str The path to the results folder.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
static int copy_var_cfg (fmiString str,  fmiString des)
{
	char *tmp_str;
	int retVal;
	tmp_str = (char*)(calloc(sizeof(char), strlen(str) + strlen (des) + 30));

#ifdef _MSC_VER
	//"\"" are quotes needed for path with spaces in the names
	sprintf(tmp_str, "xcopy %s%s %s%s%s /Y /I", "\"", str, "\"", des, "\"");
#elif __linux__
	sprintf(tmp_str, "cp -f %s%s %s%s%s", "\"", str,  "\"", des, "\"");
#elif __APPLE__
	sprintf(tmp_str, "cp -f %s%s %s%s%s", "\"", str,  "\"", des, "\"");
#else
	printf ("Cannot execute %s! The FMU export is only supported on Windows, Linux and Mac OS!\n", tmp_str);
#endif
	printf ("Command executes to copy content of resources folder: %s\n", tmp_str);
	retVal = system (tmp_str);
	free (tmp_str);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// create results folder 
///
///\param str The path to the results folder.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
static int create_res (fmiString str)
{
	char *tmp_str;
	int retVal;
	tmp_str = (char*)(calloc(sizeof(char), strlen(str) + 10));

	sprintf(tmp_str, "mkdir %s%s%s", "\"", str, "\"");
	retVal = system (tmp_str);
	free (tmp_str);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
/// delete old results folder
///
///\param str The path to the results folder.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
static int removeFMUDir (fmiString str)
{
	char *tmp_str;
	int retVal;
	tmp_str = (char*)(calloc(sizeof(char), strlen(str) + 30));

#ifdef _MSC_VER
	sprintf(tmp_str, "rmdir /S /Q %s%s%s", "\"", str, "\"");
#else
	sprintf(tmp_str, "rm -rf %s%s%s", "\"", str, "\"");
#endif
	printf ("This is the command to be executed to delete existing directory %s\n", tmp_str);
	retVal = system (tmp_str);
	free (tmp_str);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// start simulation
///
///\param c The FMU instance.
///\return 0 if no error occurred.
////////////////////////////////////////////////////////////////////////////////////
static int start_sim(fmiComponent c)
{
	idfFmu_t* _c = (idfFmu_t*)c;
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

#ifdef _MSC_VER
	fpBat = fopen("EP.bat", "w");
	if (stat(FRUNWEAFILE, &stat_p)>=0){
		// write the command string
		fprintf(fpBat, "Epl-run.bat %s %s %s %s %s %s %s %s %s %s %s", fmuInstances[_c->index]->mID,
			fmuInstances[_c->index]->mID, "idf", FRUNWEAFILE, "EP", "N", "nolimit", "N", "Y", "N", "1");
	}
	else
	{
		// write the command string
		fprintf(fpBat, "Epl-run.bat %s %s %s %s %s %s %s %s %s %s %s", fmuInstances[_c->index]->mID,
			fmuInstances[_c->index]->mID, "idf", "\" \"", "NONE", "N", "nolimit", "N", "Y", "N", "1");
	}
	fclose (fpBat);
	fmuInstances[_c->index]->handle_EP = (HANDLE)_spawnl(P_NOWAIT, "EP.bat", "EP.bat", NULL); 
	if (fmuInstances[_c->index]->handle_EP > 0 ) {
		return 0;
	}
	else {
		return 1;
	}

#else
	if (stat (FRUNWEAFILE, &stat_p)>=0){
		char *const argv[] = {"runenergyplus", fmuInstances[_c->index]->mID, FRUNWEAFILE, NULL};
		// execute the command string
		retVal = posix_spawnp( &fmuInstances[_c->index]->pidLoc, argv[0], NULL, NULL, argv, environ);
		return retVal;
	}
	else
	{
		char *const argv[] = {"runenergyplus", fmuInstances[_c->index]->mID, NULL};
		// execute the command string
		retVal = posix_spawnp( &fmuInstances[_c->index]->pidLoc, argv[0], NULL, NULL, argv, environ);
		return retVal;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
/// FMI status
///
///\param status FMI status.
///////////////////////////////////////////////////////////////////////////////
//#if 0
static const char* fmiStatusToString(fmiStatus status){
	switch (status){
	case fmiOK:      return "ok";
	case fmiWarning: return "warning";
	case fmiDiscard: return "discard";
	case fmiError:   return "error";		
	case fmiPending: return "pending";	
	default:         return "?";
	}
}
//#endif


///////////////////////////////////////////////////////////////////////////////
/// FMU logger
///
///\param c The FMU instance.
///\param instanceName FMI string.
///\param status FMI status.
///\param category FMI string.
///\param message Message to be recorded.
///////////////////////////////////////////////////////////////////////////////
static void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status,
	fmiString category, fmiString message, ...) {
		char msg[MAX_MSG_SIZE];
		char* copy;
		va_list argp;

		// Replace C format strings
		va_start(argp, message);
		vsprintf(msg, message, argp);

		// Replace e.g. ## and #r12#
		copy = strdup(msg);
		free(copy);

		// Print the final message
		if (!instanceName) instanceName = "?";
		if (!category) category = "?";
		printf("%s %s (%s): %s\n", fmiStatusToString(status), instanceName, category, msg);
}

////////////////////////////////////////////////////////////////
///  This method is used to get the fmi types of platform
///\return fmiPlatform.
////////////////////////////////////////////////////////////////
DllExport const char* fmiGetTypesPlatform()
{
	return fmiPlatform;
}

////////////////////////////////////////////////////////////////
///  This method is used to get the fmi version
///\return fmiVersion.
////////////////////////////////////////////////////////////////
DllExport const char* fmiGetVersion()
{   // This function always returns 1.0
	return FMIVERSION;
}

////////////////////////////////////////////////////////////////
///  This method is used to instantiated the FMU
///
///\param instanceName The modelIdentifier.
///\param fmuGUID The GUID.
///\param fmuLocation The FMU Location.
///\param fmumimeType The fmu mimeType.
///\param timeout The communication timeout value in milli-seconds.
///\param visible The flag to executes the FMU in windowless mode.
///\param interactive The flag to execute the FMU in interactive mode.
///\param functions The callbacks functions.
///\param loggingOn The flag to enable or disable debug.
///\return FMU instance if no error occured.
////////////////////////////////////////////////////////////////

DllExport fmiComponent fmiInstantiateSlave(fmiString instanceName,
	fmiString fmuGUID, fmiString fmuLocation,
	fmiString mimetype, fmiReal timeout, fmiBoolean visible,
	fmiBoolean interactive, fmiCallbackFunctions functions,
	fmiBoolean loggingOn)
{
	int retVal;
	fmiString mID;
	fmiString mGUID;
	char tmpResLoc[5] ={0};
	struct stat st;
	fmiBoolean errDir;
	fmiComponent c = (fmiComponent)calloc(1, sizeof(struct idfFmu_t));
	idfFmu_t* _c = (idfFmu_t*)c;

	_c->index = insNum;
	addfmuInstances (_c);
	insNum++;

	// initialize Strings to store variables for calculations
	fmuInstances[_c->index]->fmuOutput = NULL;
	fmuInstances[_c->index]->fmuCalLocation = NULL;
	fmuInstances[_c->index]->tmpResCon = NULL;
	fmuInstances[_c->index]->fmuLocation = NULL;
	fmuInstances[_c->index]->resources_p = NULL;
	fmuInstances[_c->index]->xml_file_p = NULL;
	fmuInstances[_c->index]->mID = NULL;
	fmuInstances[_c->index]->mGUID = NULL;

	// get current working directory
#ifdef _MSC_VER
	if (_getcwd(fmuInstances[_c->index]->cwd, sizeof(fmuInstances[_c->index]->cwd)) == NULL)
#else
	if (getcwd(fmuInstances[_c->index]->cwd, sizeof(fmuInstances[_c->index]->cwd)) == NULL)
#endif
	{
		perror("getcwd() error");
	}
	else
	{
		printf("The current working directory is: %s\n", fmuInstances[_c->index]->cwd);
	}

	// create the output folder for current FMU in working directory
	fmuInstances[_c->index]->fmuOutput = (char *)calloc(sizeof(char), strlen ("Output_EPExport_") + strlen (instanceName) 
		+ 10 + strlen (fmuInstances[_c->index]->cwd));
#ifdef _MSC_VER
	sprintf(fmuInstances[_c->index]->fmuOutput, "%s%s%s%s", fmuInstances[_c->index]->cwd,"\\", "Output_EPExport_", instanceName);
#else
	sprintf(fmuInstances[_c->index]->fmuOutput, "%s%s%s%s", fmuInstances[_c->index]->cwd,"//", "Output_EPExport_", instanceName);
#endif
	// check if directory exists and deletes it 
	errDir = stat(fmuInstances[_c->index]->fmuOutput, &st);
	if(errDir>=0) {
		printf("The fmuOutput directory %s already exists and will be deleted!\n", fmuInstances[_c->index]->fmuOutput);
		removeFMUDir (fmuInstances[_c->index]->fmuOutput);
	}

	// check whether the path to the resources folder has been provided
	if((fmuLocation == NULL) || (strlen(fmuLocation) == 0)) {
		fmuLogger(0, instanceName, fmiFatal, "Fatal Error", "fmiInstantiateSlave: The path"
			" to the resources folder: %s is not specified!\n", fmuLocation);
		exit(1);
	}
	// copy instanceName to the FMU
	strcpy(fmuInstances[_c->index]->instanceName, instanceName);

	// copy first 5 characters of fmuLocation
	strncpy (tmpResLoc, fmuLocation, 5);

	// allocate memory for fmuLocation 
	fmuInstances[_c->index]->fmuLocation = (char *)calloc(sizeof(char), strlen (fmuLocation) + 1);
	// extract the URI information from the fmuResourceLocation path
#ifdef _MSC_VER
	if (strnicmp (tmpResLoc, "file", 4)== 0)
#else
	if (strncasecmp (tmpResLoc, "file", 4)== 0)
#endif
	{
		// The specification for defining whether the file should start with file:/, file://, or file:///
		// is not clear (e.g. see FMI 1.0, and FMI 2.0). We will thus check all cases to see what we have
		// case file: (If Ptolemy unzips an FMu in /tmp then the fmuLocation is file:/temp)
		strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 5, strlen(fmuLocation + 5));
		printf ("fmiInstantiateSlave: Path to fmuLocation without file: %s\n", 
			fmuInstances[_c->index]->fmuLocation);
		// check whether fmuLocation exists
		errDir = stat(fmuInstances[_c->index]->fmuLocation, &st);
		if (errDir<0)
		{
			free (fmuInstances[_c->index]->fmuLocation);
			// allocate memory for fmuLocation
			fmuInstances[_c->index]->fmuLocation = (char *)calloc(sizeof(char), strlen (fmuLocation) + 1);
			// case file:/
			strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 6, strlen(fmuLocation + 6));
			printf ("fmiInstantiateSlave: Path to fmuLocation without file:/ %s\n", 
				fmuInstances[_c->index]->fmuLocation);
			// check whether fmuLocation exists
			errDir = stat(fmuInstances[_c->index]->fmuLocation, &st);
			if(errDir<0) 
			{
				free (fmuInstances[_c->index]->fmuLocation);
				// allocate memory for fmuLocation 
				fmuInstances[_c->index]->fmuLocation = (char *)calloc(sizeof(char), strlen (fmuLocation) + 1);
				// case file://
				strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 7, strlen(fmuLocation + 7));
				printf ("fmiInstantiateSlave: Path to fmuLocation without file:// %s\n", 
					fmuInstances[_c->index]->fmuLocation);

				// check whether fmuLocation exists
				errDir = stat(fmuInstances[_c->index]->fmuLocation, &st);
				if(errDir<0) 
				{
					free (fmuInstances[_c->index]->fmuLocation);
					// allocate memory for fmuLocation 
					fmuInstances[_c->index]->fmuLocation = (char *)calloc(sizeof(char), strlen (fmuLocation) + 1);
					// case file:///
					strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 8, strlen(fmuLocation + 8));
					printf ("fmiInstantiateSlave: Path to fmuLocation without file:/// %s\n", 
						fmuInstances[_c->index]->fmuLocation);
					// check whether fmuLocation exists
					errDir = stat(fmuInstances[_c->index]->fmuLocation, &st);
					if(errDir<0) 
					{
						fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiInstantiateSlave: The path to the resources"
							" folder: %s is not valid! The path does not start with file: file:/, file:// or file:///\n", 
							fmuLocation);
						free (fmuInstances[_c->index]->fmuLocation);
						exit(1);
					}
				}
			}
		}
	}
#ifdef _MSC_VER
	else if ((strnicmp (tmpResLoc, "ftp", 3)== 0) || (strnicmp (tmpResLoc, "fmi", 3)== 0))
#else
	else if ((strncasecmp (tmpResLoc, "ftp", 3)== 0) || (strncasecmp (tmpResLoc, "fmi", 3)== 0))
#endif
	{
		strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 6, strlen(fmuLocation + 6));
		printf ("fmiInstantiateSlave: Path to fmuLocationPath without ftp:// or fmi:// %s\n", 
			fmuInstances[_c->index]->fmuLocation);
	}

#ifdef _MSC_VER
	else if ((strnicmp (tmpResLoc, "https", 5)== 0))
#else
	else if ((strncasecmp (tmpResLoc, "https", 5)== 0))
#endif
	{
		strncpy(fmuInstances[_c->index]->fmuLocation, fmuLocation + 8, strlen(fmuLocation + 8));
		printf("fmiInstantiateSlave: Path to fmuLocation without https:// %s\n", fmuInstances[_c->index]->fmuLocation);
	}
	else
	{
		strcpy(fmuInstances[_c->index]->fmuLocation, fmuLocation);
		printf("fmiInstantiateSlave: Path to fmuLocation %s\n", fmuInstances[_c->index]->fmuLocation);
	}
#ifdef _MSC_VER
	// replace eventual forward slash with backslash on Windows
	replace_char (fmuInstances[_c->index]->fmuLocation, '//', '\\');
#endif
	printf("fmiInstantiateSlave: Path to fmuLocation used in the program %s\n", fmuInstances[_c->index]->fmuLocation);

	fmuInstances[_c->index]->resources_p = (char *)calloc(sizeof(char), strlen (fmuInstances[_c->index]->fmuLocation) + strlen (RESOURCES) + 1);
	sprintf(fmuInstances[_c->index]->resources_p, "%s%s", fmuInstances[_c->index]->fmuLocation, RESOURCES);

	// create content of resources folder in created output directory
	fmuInstances[_c->index]->tmpResCon = (char *)calloc(sizeof(char), strlen (fmuInstances[_c->index]->resources_p) + strlen ("\""VARCFG) + 10);
	sprintf(fmuInstances[_c->index]->tmpResCon, "%s%s", fmuInstances[_c->index]->resources_p, "\""VARCFG);

	// create the output directory
	retVal = create_res (fmuInstances[_c->index]->fmuOutput);
	// add the end slash to the fmuOutput
#ifdef _MSC_VER
	sprintf(fmuInstances[_c->index]->fmuOutput, "%s%s", fmuInstances[_c->index]->fmuOutput,"\\");
#else
	sprintf(fmuInstances[_c->index]->fmuOutput, "%s%s", fmuInstances[_c->index]->fmuOutput,"//");
#endif

	// copy the vriables cfg into the output directory
	retVal = copy_var_cfg (fmuInstances[_c->index]->tmpResCon, fmuInstances[_c->index]->fmuOutput);

	// reallocate memory for fmuLocation and assign it to the folder created
	fmuInstances[_c->index]->fmuCalLocation = (char *)calloc(sizeof(char), strlen (fmuInstances[_c->index]->fmuOutput) + 1);
	strcpy(fmuInstances[_c->index]->fmuCalLocation, fmuInstances[_c->index]->fmuOutput);

	// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
	retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
	if (retVal!=0){
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiInstantiateSlave: The path"
			" to the resources folder: %s is not valid!\n", fmuInstances[_c->index]->fmuCalLocation);
		exit(1);
	}

	// create path to xml file
	fmuInstances[_c->index]->xml_file_p = (char *)calloc(sizeof(char), strlen (fmuInstances[_c->index]->fmuLocation) + strlen (XML_FILE) + 1);
	sprintf(fmuInstances[_c->index]->xml_file_p, "%s%s", fmuInstances[_c->index]->fmuLocation, XML_FILE);

	// get model description of the FMU
	fmuInstances[_c->index]->md = parse(fmuInstances[_c->index]->xml_file_p);
	if (!fmuInstances[_c->index]->md) {
		fprintf(stderr, "fmiInstantiateSlave: Failed to get the modelDescription\n");
		exit(1);
	}

	// gets the modelID of the FMU
	mID = getModelIdentifier(fmuInstances[_c->index]->md);

	// copy model ID to FMU
	fmuInstances[_c->index]->mID = (char *)calloc(sizeof(char), strlen (mID) + 1);
	strcpy(fmuInstances[_c->index]->mID, mID);
	printf("fmiInstantiateSlave: The FMU modelIdentifier is %s.\n", fmuInstances[_c->index]->mID);

	// get the model GUID of the FMU
	mGUID = getString(fmuInstances[_c->index]->md, att_guid);

	// copy model GUID to FMU
	fmuInstances[_c->index]->mGUID = (char *)calloc(sizeof(char), strlen (mGUID) + 1);
	strcpy(fmuInstances[_c->index]->mGUID, mGUID);
	printf("fmiInstantiateSlave: The FMU modelGUID is %s.\n", fmuInstances[_c->index]->mGUID);

	// check whether the model is exported for FMI version 1.0
	if(strcmp(getString(fmuInstances[_c->index]->md, att_fmiVersion), FMIVERSION) != 0){
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiInstantiateSlave: Wrong FMI version"
			" FMI version 1.0 is currently supported!\n");
		exit(1);
	}

	// check whether GUIDs are consistent with modelDescription file
	if(strcmp(fmuGUID, fmuInstances[_c->index]->mGUID) != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiInstantiateSlave: The given"
			" GUID %s is not equal to the GUID of the binary"
			" (%s)!\n", fmuGUID, fmuInstances[_c->index]->mGUID);
		exit(1);
	}
	// assign FMU parameters
	fmuInstances[_c->index]->timeout = timeout;
	fmuInstances[_c->index]->visible = visible;
	fmuInstances[_c->index]->interactive = interactive; 
	// free xml_file_p;
	free (fmuInstances[_c->index]->xml_file_p);
	// free tmpResCon
	free (fmuInstances[_c->index]->tmpResCon);
	printf("fmiInstantiateSlave: Slave %s is instantiated!\n", fmuInstances[_c->index]->instanceName);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return(c); 
}

////////////////////////////////////////////////////////////////
///  This method is used to initialize the FMU
///
///\param c The FMU instance.
///\param tStart The simulation start time.
///\param StopTimeDefined The stop time define.
///\param tStop simulation The stop time.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiInitializeSlave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop)
{
	int retVal;
	idfFmu_t* _c = (idfFmu_t *)c;

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
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;
#endif

#ifndef _MSC_VER
	mode_t process_mask = umask(0);
#endif 

	// save start of the simulation time step
	fmuInstances[_c->index]->tStartFMU = tStart;
	// save end of smulation time step
	fmuInstances[_c->index]->tStopFMU = tStop;

	// initialize structure variables
	fmuInstances[_c->index]->firstCallGetReal       = 1;
	fmuInstances[_c->index]->firstCallSetReal       = 1;
	fmuInstances[_c->index]->firstCallDoStep        = 1;
	fmuInstances[_c->index]->firstCallFree          = 1;
	fmuInstances[_c->index]->firstCallTerm          = 1;
	fmuInstances[_c->index]->firstCallIni           = 1;
	fmuInstances[_c->index]->firstCallRes           = 1;
	fmuInstances[_c->index]->flaGetRealCall         = 0;

	fmuInstances[_c->index]->flaGetRea = 0;
	fmuInstances[_c->index]->preInDoStep = 0;
	fmuInstances[_c->index]->preInGetReal = 0;
	fmuInstances[_c->index]->preInSetReal = 0;
	fmuInstances[_c->index]->preInFree = 0;
	fmuInstances[_c->index]->preInTerm = 0;
	fmuInstances[_c->index]->preInIni = 0;
	fmuInstances[_c->index]->preInRes = 0;
	fmuInstances[_c->index]->flaWri = 0;
	fmuInstances[_c->index]->flaRea = 0;
	fmuInstances[_c->index]->numInVar  = -1;
	fmuInstances[_c->index]->numOutVar = -1;
	fmuInstances[_c->index]->getCounter = 0;
	fmuInstances[_c->index]->setCounter = 0;
	fmuInstances[_c->index]->readReady = 0;
	fmuInstances[_c->index]->writeReady = 0;

	// change the directory to make sure that FMUs are not overwritten
	if (fmuInstances[_c->index]->firstCallIni || fmuInstances[_c->index]->preInIni!= fmuInstances[_c->index]->index) {
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
		retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif

	}
	if (retVal!=0){
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", 
			"fmiInitializeSlave: The path to the resources folder: %s is not valid!\n", fmuInstances[_c->index]->fmuCalLocation);
		return fmiFatal;
	}
	fmuInstances[_c->index]->preInIni = fmuInstances[_c->index]->index;

	///////////////////////////////////////////////////////////////////////////////////
	// create the socket server

#ifdef _MSC_VER
	// initialize winsock  /************* Windows specific code ********/
	if (WSAStartup(wVersionRequested, &wsaData)!= 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName , fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: WSAStartup failed with error %ld!\n", WSAGetLastError());
		WSACleanup();
		return fmiFatal;
	}
	// check if the version is supported
	if (LOBYTE(wsaData.wVersion)!= 2 || HIBYTE(wsaData.wVersion)!= 2 )
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: Could not find a usable WinSock DLL for WinSock version %u.%u!\n",
			LOBYTE(wsaData.wVersion),HIBYTE(wsaData.		wVersion));
		WSACleanup();
		return fmiFatal;
	}
#endif  /************* End of Windows specific code *******/

	fmuInstances[_c->index]->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// check for errors to ensure that the socket is a valid socket.
	if (fmuInstances[_c->index]->sockfd == INVALID_SOCKET)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: Opening socket failed"
			" sockfd = %d!\n", fmuInstances[_c->index]->sockfd);
		return fmiFatal;
	}
	printf("fmiInitializeSlave: The sockfd is %d.\n", fmuInstances[_c->index]->sockfd);
	// initialize socket structure server address information
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                 // Address family to use
	server_addr.sin_port = htons(0);                  // Port number to use
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address

	// bind the socket
	if (bind(fmuInstances[_c->index]->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: bind() failed!\n");
		closeipcFMU (&(fmuInstances[_c->index]->sockfd));
		return fmiFatal;
	}

	// get socket information information
	sockLength = sizeof(server_addr);
	if ( getsockname (fmuInstances[_c->index]->sockfd, (struct sockaddr *)&server_addr, &sockLength)) {
		fmuLogger(0,  fmuInstances[_c->index]->instanceName, fmiFatal,
			"Fatal Error", "fmiInitializeSlave: Get socket name failed!\n");
		return fmiFatal;
	}

	// get the port number
	port_num= ntohs(server_addr.sin_port);
	printf("fmiInitializeSlave: The port number is %d.\n", port_num);

	// get the hostname information
	gethostname(ThisHost, MAXHOSTNAME);
	if  ((hp = gethostbyname(ThisHost)) == NULL ) {
		fmuLogger(0,  fmuInstances[_c->index]->instanceName, fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: Get host by name failed!\n");
		return fmiFatal;
	}

	// write socket cfg file
	retVal = write_socket_cfg (port_num, ThisHost);
	printf("fmiInitializeSlave: This hostname is %s.\n", ThisHost);
	if  (retVal != 0) {
		fmuLogger(0,  fmuInstances[_c->index]->instanceName, fmiFatal, 
			"Fatal Error", "fmiInitializeSlave: Write socket cfg failed!\n");
		return fmiFatal;
	}
	// listen to the port
	if (listen(fmuInstances[_c->index]->sockfd, 1) == SOCKET_ERROR)
	{
		fmuLogger(0,  fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiInitializeSlave: listen() failed!\n");
		closeipcFMU (&(fmuInstances[_c->index]->sockfd));
		return fmiFatal;
	}
	printf("fmiInitializeSlave: TCPServer Server waiting for clients on port: %d.\n", port_num);

	fmuInstances[_c->index]->pid = 0;

	// get the number of input variables of the FMU
	if (fmuInstances[_c->index]->numInVar ==-1)
	{
		fmuInstances[_c->index]->numInVar = getNumInputVariablesInFMU (fmuInstances[_c->index]->md);
		// initialize the input vectors
		fmuInstances[_c->index]->inVec = (fmiReal*)malloc(fmuInstances[_c->index]->numInVar*sizeof(fmiReal));
	}
	printf("fmiInitializeSlave: The number of input variables is %d!\n", fmuInstances[_c->index]->numInVar);

	// get the number of output variables of the FMU
	if (fmuInstances[_c->index]->numOutVar ==-1)
	{
		fmuInstances[_c->index]->numOutVar =  getNumOutputVariablesInFMU (fmuInstances[_c->index]->md);
		// initialize the output vector
		fmuInstances[_c->index]->outVec = (fmiReal*)malloc(fmuInstances[_c->index]->numOutVar*sizeof(fmiReal));
	}
	printf("fmiInitializeSlave: The number of output variables is %d!\n", fmuInstances[_c->index]->numOutVar);

	if ( (fmuInstances[_c->index]->numInVar + fmuInstances[_c->index]->numOutVar) == 0){
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", 
			"fmiInitializeSlave: The FMU instance %s has no input and output variables. Please check the model description file!\n",
			fmuInstances[_c->index]->instanceName);
		return fmiFatal;
	}
	// fmiInitialize just active when pid of the child is invoked
	if ((fmuInstances[_c->index]->pid == 0)){

		// create the input and weather file for the run
		retVal = createRunInFile(fmuInstances[_c->index]->tStartFMU , fmuInstances[_c->index]->tStopFMU, 
			fmuInstances[_c->index]->mID,  fmuInstances[_c->index]->resources_p);

		// free resources_p which is not longer used
		free (fmuInstances[_c->index]->resources_p);
		if  (retVal != 0) {

			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
				"Fatal Error", "fmiInitializeSlave: The FMU instance could not be initialized!\n");
			fprintf(stderr, "fmiInitializeSlave: Can't create input file cfg!\n");
			return fmiFatal;
		}
#ifndef _MSC_VER
		// create a directory and copy the weather file into it
		if (stat (FRUNWEAFILE, &stat_p)>=0)
		{
			if (stat ("WeatherData", &stat_p)<0)
			{
				char *str;
				mkdir ("WeatherData", S_IRWXU | S_IRWXG | S_IRWXO);
				str = (char *)calloc(sizeof(char), strlen (FRUNWEAFILE) + strlen ("WeatherData/") + 50);
				sprintf(str, "cp -f %s %s", FRUNWEAFILE, "WeatherData/");
				retVal = system (str);
				free(str);
			}
			// set environment variable for weather file
			setenv ("ENERGYPLUS_WEATHER", "WeatherData", 0);
		}
#endif

#ifndef _MSC_VER
		umask(process_mask);
#endif
		// start the simulation
		retVal = start_sim(c);
		fmuInstances[_c->index]->newsockfd = accept(fmuInstances[_c->index]->sockfd, NULL, NULL);
		printf("fmiInitializeSlave: The connection has been accepted!\n");
		// check whether the simulation could start successfully
		if  (retVal != 0) {
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
				"Fatal Error", "fmiInitializeSlave: The FMU instance could %s not be initialized. "
				"EnergyPlus can't start . Check if EnergyPlus is installed and on the system path!\n", 
				fmuInstances[_c->index]->instanceName);
			return fmiFatal;
		}

		fmuInstances[_c->index]->pid = 1;

		// reset firstCallIni
		if (fmuInstances[_c->index]->firstCallIni) 
		{
			fmuInstances[_c->index]->firstCallIni = 0;
		}
		printf("fmiInitializeSlave: Slave %s is initialized!\n", fmuInstances[_c->index]->instanceName);
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
		return fmiOK;
	}
	printf("fmiInitializeSlave: Slave %s is initialized!\n", fmuInstances[_c->index]->instanceName);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return fmiOK;
} 

////////////////////////////////////////////////////////////////
///  This method is used to do the time stepping the FMU
///
///\param c The FMU instance.
///\param currentCommunicationPoint The communication point.
///\param communicationStepSize The communication step size.
///\param newStep The flag to accept or refect communication step.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	int retVal;
	if (fmuInstances[_c->index]->pid != 0)
	{
		FILE *fp;
		// get current communication point
		fmuInstances[_c->index]->curComm = currentCommunicationPoint;
		// get current communication step size
		fmuInstances[_c->index]->communicationStepSize = communicationStepSize;
		// assign current communication point to value to be sent
		fmuInstances[_c->index]->simTimSen = fmuInstances[_c->index]->curComm;
		if (fmuInstances[_c->index]->firstCallDoStep || (fmuInstances[_c->index]->index!=fmuInstances[_c->index]->preInDoStep)){
			// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
			retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
			retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
		}
		// save previous index of doStep
		fmuInstances[_c->index]->preInDoStep = fmuInstances[_c->index]->index;

		// check if timeStep is defined
		if (fmuInstances[_c->index]->firstCallDoStep){
			// initialize the nexComm value to start communication point
			fmuInstances[_c->index]->nexComm = fmuInstances[_c->index]->curComm;
			if((fp = fopen(FTIMESTEP, "r")) != NULL) {
				retVal = fscanf(fp, "%d", &(fmuInstances[_c->index]->timeStepIDF));
				fclose (fp);
			}
			else
			{
				fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", 
					"fmiDoStep: A valid time step could not be determined!\n");
				fprintf(stderr, "fmiDoStep: Can't read time step file!\n");
				return fmiFatal;
			}
		}

		// check for the first communication instant
		if (fmuInstances[_c->index]->firstCallDoStep && (fabs(fmuInstances[_c->index]->curComm - 
			fmuInstances[_c->index]->tStartFMU) > 1e-10))
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", 
				"fmiDoStep: An error occured in a previous call. First communication time: %f != tStart: %f!\n",
				fmuInstances[_c->index]->curComm, fmuInstances[_c->index]->tStartFMU);
			return fmiFatal;
		}
		// check if FMU needs to reject time step
		if(!newStep)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", 
				"fmiDoStep: FMU can not reject time steps.");
			return fmiFatal;
		}

		// check whether the communication step size is different from null
		if (fmuInstances[_c->index]->communicationStepSize == 0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, 
				"Fatal Error", "fmiDoStep: An error occured in a previous call. CommunicationStepSize cannot be null!\n");
			return fmiFatal;
		}

		// check whether the communication step size is different from time step in input file
		if ( fabs(fmuInstances[_c->index]->communicationStepSize - (3600/fmuInstances[_c->index]->timeStepIDF)) > 1e-10)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiDoStep:"
				" An error occured in a previous call. CommunicationStepSize: %f is different from time step: %d in input file!\n",
				fmuInstances[_c->index]->communicationStepSize, fmuInstances[_c->index]->timeStepIDF);
			return fmiFatal;
		}

		// check whether communication point is valid
		if ((fmuInstances[_c->index]->curComm) < 0 || ((fmuInstances[_c->index]->firstCallDoStep == 0) 
			&& (fmuInstances[_c->index]->curComm > fmuInstances[_c->index]->nexComm))){
				fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiDoStep:"
					" An error occured in a previous call. Communication point must be positive and monoton increasing!\n");
				return fmiFatal;
		}

		// check whether current communication point is valid
		if ((fmuInstances[_c->index]->firstCallDoStep == 0)
			&& (fabs(fmuInstances[_c->index]->curComm - fmuInstances[_c->index]->nexComm) > 1e-10))
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", "fmiDoStep: "
				"Current communication point: %f is not equals to the previous simulation time + "
				"communicationStepSize: %f + %f!\n",
				fmuInstances[_c->index]->curComm, fmuInstances[_c->index]->nexComm, 
				fmuInstances[_c->index]->communicationStepSize);
			return fmiError;
		}

		// check end of simulation
		if (fmuInstances[_c->index]->curComm == fmuInstances[_c->index]->tStopFMU){
			// set the communication flags to 1 to send stop signal to EnergyPlus
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiDoStep: Current communication point: %f of FMU instance: %s "
				"is equals to end of simulation: %f!\n", 
				fmuInstances[_c->index]->curComm, fmuInstances[_c->index]->instanceName, fmuInstances[_c->index]->tStopFMU);
			return fmiWarning;
		}

		// check if current communication is larger than end of simulation
		if (fmuInstances[_c->index]->curComm > fmuInstances[_c->index]->tStopFMU){
			// set the communication flags to 1 to send stop signal to EnergyPlus
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", "fmiDoStep:"
				" Current communication point: %f is larger than end of simulation time: %f!\n", 
				fmuInstances[_c->index]->curComm, fmuInstances[_c->index]->tStopFMU);
			return fmiError;
		}
		// check end of simulation
		if (fmuInstances[_c->index]->curComm + 
			fmuInstances[_c->index]->communicationStepSize > fmuInstances[_c->index]->tStopFMU){
				// set the communication flags to 1 to send stop signal to EnergyPlus
				fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiFatal, "Fatal Error", "fmiDoStep: "
					"Current communication point: %f  + communicationStepsize: %f  is larger than "
					"end of simulation time: %f!\n", 
					fmuInstances[_c->index]->curComm, fmuInstances[_c->index]->communicationStepSize,  
					fmuInstances[_c->index]->tStopFMU);
				return fmiFatal;
		}

		// check if inputs are set
		if ((fmuInstances[_c->index]->firstCallDoStep == 1) 
			&& !fmuInstances[_c->index]->writeReady) 
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"All inputs of FMU instance %s are not set before first call of fmiDoStep.", 
				fmuInstances[_c->index]->instanceName);
			return fmiOK;
		}

		// check if inputs are set
		if ((fmuInstances[_c->index]->firstCallDoStep == 0) 
			&& !fmuInstances[_c->index]->writeReady) 
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiDoStep: All inputs of FMU instance %s are not set!\n", 
				fmuInstances[_c->index]->instanceName);
			return fmiOK;
		}
		// check whether all input and outputs are available and then do the time stepping
		if (fmuInstances[_c->index]->firstCallDoStep
			||
			(fmuInstances[_c->index]->firstCallDoStep == 0) 
			&& fmuInstances[_c->index]->curComm <= (fmuInstances[_c->index]->tStopFMU - 
			fmuInstances[_c->index]->communicationStepSize)) {
				if (fmuInstances[_c->index]->flaWri != 1){
					fmuInstances[_c->index]->flaGetRea = 1;
					if (fmuInstances[_c->index]->flaGetRealCall == 0)
					{
						retVal = readfromsocketFMU(&(fmuInstances[_c->index]->newsockfd), &(fmuInstances[_c->index]->flaRea),
							&(fmuInstances[_c->index]->numOutVar), &zI, &zI, &(fmuInstances[_c->index]->simTimRec), 
							fmuInstances[_c->index]->outVec, NULL, NULL);
					}
					retVal = writetosocketFMU(&(fmuInstances[_c->index]->newsockfd), &(fmuInstances[_c->index]->flaWri),
						&fmuInstances[_c->index]->numInVar, &zI, &zI, &(fmuInstances[_c->index]->simTimSen),
						fmuInstances[_c->index]->inVec, NULL, NULL);

					if (fmuInstances[_c->index]->flaGetRealCall = 1)
					{
						fmuInstances[_c->index]->flaGetRealCall = 0;
					}
				}
				fmuInstances[_c->index]->readReady = 0;
				fmuInstances[_c->index]->writeReady = 0;
				fmuInstances[_c->index]->setCounter = 0;
				fmuInstances[_c->index]->getCounter = 0;
		}

		// calculate next communication point
		fmuInstances[_c->index]->nexComm = fmuInstances[_c->index]->curComm + fmuInstances[_c->index]->communicationStepSize;
		// set the firstcall flag to zero
		if (fmuInstances[_c->index]->firstCallDoStep)
		{
			fmuInstances[_c->index]->firstCallDoStep = 0;
		}
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
		return fmiOK;
	}
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return fmiOK;
}  

////////////////////////////////////////////////////////////////
///  This method is used to cancel a step in the FMU
///
///\param c The FMU instance.
///\return fmiWarning if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiCancelStep(fmiComponent c)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
			"Warning", "fmiCancelStep: The function fmiCancelStep(..) is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to terminate the FMU instance
///
///\param c The FMU instance.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiTerminateSlave(fmiComponent c)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	int retVal;
	if (fmuInstances[_c->index]->pid != 0)
	{
#ifndef _MSC_VER
		int status;
#endif

		if (fmuInstances[_c->index]->firstCallFree == 0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiTerminateSlave: fmiFreeSlaveInstance(..) was already called on FMU instance %s!\n", 
				fmuInstances[_c->index]->instanceName);
			return fmiOK;
		}

		if (fmuInstances[_c->index]->firstCallTerm == 0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiTerminateSlave: fmiTerminateSlave(..) was already called on FMU instance %s!\n", 
				fmuInstances[_c->index]->instanceName);
			return fmiOK;
		}
		if (fmuInstances[_c->index]->firstCallTerm || (fmuInstances[_c->index]->index!=fmuInstances[_c->index]->preInTerm)){
			// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
			retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
			retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
		}
		// save previous index of doStep
		fmuInstances[_c->index]->preInTerm = fmuInstances[_c->index]->index;

		// send end of simulation flag
		fmuInstances[_c->index]->flaWri = 1;
		fmuInstances[_c->index]->flaRea = 1;
		retVal = exchangedoubleswithsocketFMUex (&(fmuInstances[_c->index]->newsockfd), &(fmuInstances[_c->index]->flaWri), 
			&(fmuInstances[_c->index]->flaRea), &(fmuInstances[_c->index]->numOutVar), &(fmuInstances[_c->index]->numInVar), 
			&(fmuInstances[_c->index]->simTimRec), fmuInstances[_c->index]->outVec, &(fmuInstances[_c->index]->simTimSen), 
			fmuInstances[_c->index]->inVec);
		// close socket
		closeipcFMU(&(fmuInstances[_c->index]->sockfd));
		closeipcFMU(&(fmuInstances[_c->index]->newsockfd));
		// clean-up temporary files
		findFileDelete();
#ifdef _MSC_VER
		// wait for object to terminate
		WaitForSingleObject (fmuInstances[_c->index]->handle_EP, INFINITE);
		TerminateProcess(fmuInstances[_c->index]->handle_EP, 0);
#else
		waitpid (fmuInstances[_c->index]->pidLoc, &status, 0);
#endif

#ifdef _MSC_VER
		// clean-up winsock
		WSACleanup();
#endif
		if (fmuInstances[_c->index]->firstCallTerm){
			fmuInstances[_c->index]->firstCallTerm = 0;
		}
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will terminate the simulation but returns false
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
		// FIXME: free FMU instance does not work with Dymola 2014
		//free (_c);
		return fmiOK;
	}
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will terminate the simulation but returns false
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to reset the FMU instance
///
///\param c The FMU instance.
///\return fmiWarning if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiResetSlave(fmiComponent c)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiResetSlave: fmiResetSlave(..): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to free the FMU instance
///
///\param c The FMU instance.
////////////////////////////////////////////////////////////////
DllExport void fmiFreeSlaveInstance(fmiComponent c)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	int retVal;
	if (fmuInstances[_c->index]->pid != 0)
	{
#ifndef _MSC_VER
		int status;
#endif
		// if Terminate has already been called, do not do anything here.
		if (fmuInstances[_c->index]->firstCallTerm == 0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiFreeSlaveInstance: fmiTerminateSlave(..) was already called on FMU instance %s!\n", 
				fmuInstances[_c->index]->instanceName);
			return ;
		}

		// if Free has already been called, do not do anything here.
		if (fmuInstances[_c->index]->firstCallFree == 0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, 
				"Warning", "fmiFreeSlaveInstance: fmiFreeSlaveInstance(..) was already called on FMU instance %s!\n", 
				fmuInstances[_c->index]->instanceName);
			return ;
		}
		if (fmuInstances[_c->index]->firstCallFree || (fmuInstances[_c->index]->index!=fmuInstances[_c->index]->preInFree)){
			// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
			retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
			retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
		}
		// save previous index of doStep
		fmuInstances[_c->index]->preInFree = fmuInstances[_c->index]->index;

		// send end of simulation flag
		fmuInstances[_c->index]->flaWri = 1;
		fmuInstances[_c->index]->flaRea = 1;
		retVal = exchangedoubleswithsocketFMUex (&(fmuInstances[_c->index]->newsockfd), &(fmuInstances[_c->index]->flaWri), 
			&(fmuInstances[_c->index]->flaRea), &(fmuInstances[_c->index]->numOutVar), &(fmuInstances[_c->index]->numInVar), 
			&(fmuInstances[_c->index]->simTimRec), fmuInstances[_c->index]->outVec, &(fmuInstances[_c->index]->simTimSen), 
			fmuInstances[_c->index]->inVec);
		// close socket
		closeipcFMU(&(fmuInstances[_c->index]->sockfd));
		closeipcFMU(&(fmuInstances[_c->index]->newsockfd));
		// clean-up temporary files
		findFileDelete();
#ifdef _MSC_VER
		// wait for object to terminate
		WaitForSingleObject (fmuInstances[_c->index]->handle_EP, INFINITE);
		TerminateProcess(fmuInstances[_c->index]->handle_EP, 0);
#else
		waitpid (fmuInstances[_c->index]->pidLoc, &status, 0);
#endif

#ifdef _MSC_VER
		// clean-up winsock
		WSACleanup();
#endif
		if (fmuInstances[_c->index]->firstCallFree){
			fmuInstances[_c->index]->firstCallFree = 0;
		}
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
		// FIXME: free FMU instance does not work with Dymola 2014
		// free (_c);
#endif
	}
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will terminate the simulation but returns false
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif

}

////////////////////////////////////////////////////////////////
///  This method is used to set the debug logging in the FMU
///
///\param c The FMU instance.
///\param loggingOn The loginggOn activate/deactivate.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetDebugLogging (fmiComponent c, fmiBoolean loggingOn)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiSetDebugLogging: fmiSetDebugLogging(): fmiSetDebugLogging is not provided!\n");
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set reals in the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to set.
///\param value The values of variables to set.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	int retVal;
	if (fmuInstances[_c->index]->pid != 0)
	{
		// fmiValueReference to check for input variable
		fmiValueReference vrTemp;
		ScalarVariable** vars;
		int i, k;

		if (fmuInstances[_c->index]->firstCallSetReal || (fmuInstances[_c->index]->index!=fmuInstances[_c->index]->preInSetReal)){
			// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
			retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
			retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
		}
		// save previous index of doStep
		fmuInstances[_c->index]->preInSetReal = fmuInstances[_c->index]->index;

		vars = fmuInstances[_c->index]->md->modelVariables;
		if (!fmuInstances[_c->index]->writeReady){
			for(i=0; i<nvr; i++)
			{
				for (k=0; vars[k]; k++) {
					ScalarVariable* svTemp = vars [k];
					if (getAlias(svTemp)!=enu_noAlias) continue;
					if (getCausality(svTemp) != enu_input) continue; 
					vrTemp = getValueReference(svTemp);
					if (vrTemp == vr[i]){
						fmuInstances[_c->index]->inVec[vr[i]-1] = value[i]; 
						fmuInstances[_c->index]->setCounter++;
					}
				}
			}
			if (fmuInstances[_c->index]->setCounter == fmuInstances[_c->index]->numInVar)
			{
				fmuInstances[_c->index]->writeReady = 1;
			}
		}
		if (fmuInstances[_c->index]->firstCallSetReal){
			fmuInstances[_c->index]->firstCallSetReal = 0;
		}
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
		return fmiOK;
	}
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set integers in the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to set.
///\param value The values of variables to set.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", 
				"fmiSetInteger: fmiSetInteger(..) was called. The FMU does not contain integer variables to set!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set booleans in the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to set.
///\param value The values of variables to set.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", 
				"fmiSetBoolean: fmiSetBoolean(..) was called. The FMU does not contain boolean variables to set!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to set strings in the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to set.
///\param value The values of variables to set.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, 
				"Error", "fmiSetString: fmiSetString(..) was called. The FMU does not contain string variables to set!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get reals from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to get.
///\param value The values of variables to get.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	int retVal;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmiValueReference vrTemp;
		ScalarVariable** vars;
		int i, k;

		vars = fmuInstances[_c->index]->md->modelVariables;
		fmuInstances[_c->index]->flaGetRealCall = 1;

		if (fmuInstances[_c->index]->firstCallGetReal || (fmuInstances[_c->index]->index!=fmuInstances[_c->index]->preInGetReal)){
			// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
			retVal = _chdir(fmuInstances[_c->index]->fmuCalLocation);
#else
			retVal = chdir(fmuInstances[_c->index]->fmuCalLocation);
#endif
		}
		// save previous index of doStep
		fmuInstances[_c->index]->preInGetReal = fmuInstances[_c->index]->index;

		if (fmuInstances[_c->index]->firstCallGetReal||((fmuInstances[_c->index]->firstCallGetReal == 0) 
			&& (fmuInstances[_c->index]->flaGetRea)))  {
				// read the values from the server
				retVal = readfromsocketFMU(&(fmuInstances[_c->index]->newsockfd), &(fmuInstances[_c->index]->flaRea),
					&(fmuInstances[_c->index]->numOutVar), &zI, &zI, &(fmuInstances[_c->index]->simTimRec), 
					fmuInstances[_c->index]->outVec, NULL, NULL);
				// reset flaGetRea
				fmuInstances[_c->index]->flaGetRea = 0;
		}
		if (!fmuInstances[_c->index]->readReady)
		{
			for(i=0; i<nvr; i++)
			{
				for (k=0; vars[k]; k++) {
					ScalarVariable* svTemp = vars [k];
					if (getAlias(svTemp)!=enu_noAlias) continue;
					if (getCausality(svTemp) != enu_output) continue; 
					vrTemp = getValueReference(svTemp);
					if (vrTemp == vr[i]){
						value[i] = fmuInstances[_c->index]->outVec[vr[i]-10001];
						fmuInstances[_c->index]->getCounter++;
					}
				}
			}
			if (fmuInstances[_c->index]->getCounter == fmuInstances[_c->index]->numOutVar)
			{
				fmuInstances[_c->index]->readReady = 1;
			}
		} 
		if(fmuInstances[_c->index]->firstCallGetReal)
		{
			fmuInstances[_c->index]->firstCallGetReal = 0;
		}
		// reset the current working directory. This is particularly important for Dymola
		// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
		retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
		retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
		return fmiOK;
	}
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal = _chdir(fmuInstances[_c->index]->cwd);
#else
	retVal = chdir(fmuInstances[_c->index]->cwd);
#endif
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get integers from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to get.
///\param value The values of variables to get.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", 
				"fmiGetInteger: fmiGetInteger(..) was called. The FMU does not contain integer variables to get!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get booleans from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to get.
///\param value The values of variables to get.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, "Error", 
				"fmiGetBoolean: fmiGetBoolean(..) was called. The FMU does not contain boolean variables to get!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get strings from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to get.
///\param value The values of variables to get.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetString (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		if(nvr>0)
		{
			fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiError, 
				"Error", "fmiGetString: fmiGetString(..) was called. The FMU does not contain string variables to get!\n");
			return fmiError;
		}
		return fmiOK;
	}
	return fmiOK;
}

////////////////////////////////////////////////////////////////
///  This method is used to get real output
///  derivatives from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to get.
///\param order The order of the derivatives.
///\param value The values of variables to get.
///\return fmiWarning if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetRealOutputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, 
	const fmiInteger order[], fmiReal value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetRealOutputDerivatives: fmiGetRealOutputDerivatives(): Real Output Derivatives are not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to set real input
///  derivatives from the FMU instance
///
///\param c The FMU instance.
///\param fmiValueReference The value reference.
///\param nvr The number of variables to set.
///\param order The order of the derivatives.
///\param value The values of variables to set.
///\return fmiWarning if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiSetRealInputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, 
	const fmiInteger order[], const fmiReal value[])
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiSetRealInputDerivatives: fmiSetRealInputDerivatives(): Real Input Derivatives are not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to get FMU status
///
///\param c The FMU instance.
///\param fmiStatusKind The status information.
///\param value The status value.
///\return fmiWarning if no error occured.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetStatus(fmiComponent c, const fmiStatusKind s, fmiStatus* value)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetStatus: fmiGetStatus(): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to get fmiGetReal status
///
///\param c The FMU instance.
///\param fmiStatusKind The status information.
///\param value The status value.
///\return fmiWarning if no error occured.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetRealStatus(fmiComponent c, const fmiStatusKind s, fmiReal* value)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetRealStatus: fmiGetRealStatus(): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to get fmiGetInteger status
///
///\param c The FMU instance.
///\param fmiStatusKind The status information.
///\param value The status value.
///\return fmiWarning if no error occured.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetIntegerStatus(fmiComponent c, const fmiStatusKind s, fmiInteger* value)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)

	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetIntegerStatus: fmiGetIntegerStatus(): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}
////////////////////////////////////////////////////////////////
///  This method is used to get fmiGetBoolean status
///
///\param c The FMU instance.
///\param fmiStatusKind The status information.
///\param value The status value.
///\return fmiWarning if no error occured.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s, fmiBoolean* value)
{
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetBooleanStatus: fmiGetBooleanStatus(): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}
////////////////////////////////////////////////////////////////
///  This method is used to get fmiGetString status
///
///\param c The FMU instance.
///\param fmiStatusKind The status information.
///\param value The status value.
///\return fmiWarning if no error occured.
////////////////////////////////////////////////////////////////

DllExport fmiStatus fmiGetStringStatus (fmiComponent c, const fmiStatusKind s, fmiString* value)
{	
	idfFmu_t* _c = (idfFmu_t *)c;
	if (fmuInstances[_c->index]->pid != 0)
	{
		fmuLogger(0, fmuInstances[_c->index]->instanceName, fmiWarning, "Warning", 
			"fmiGetStringStatus: fmiGetStringStatus(): is not provided!\n");
		return fmiWarning;
	}
	return fmiWarning;
}

/*

***********************************************************************************
Copyright Notice
----------------

Functional Mock-up Unit Export of EnergyPlus ©2013, The Regents of 
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