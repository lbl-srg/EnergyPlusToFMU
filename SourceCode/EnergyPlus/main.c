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
#define MAXBUFFSIZE 1000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "utilSocket.h" 
#include "defines.h"
//#include "reader.h" 
#include <errno.h>
#include <sys/stat.h>


#ifdef _MSC_VER
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

///////////////////////////////////////////////////////////////////////////////
/// This function deletes temporary created files. 
///////////////////////////////////////////////////////////////////////////////
void findFileDelete()
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
}

////////////////////////////////////////////////////////////////////////////////////
/// Replace forward slash with backward slash
///
///\param s The input string.
///\param s The character to find.
///\param s The character to replace.
////////////////////////////////////////////////////////////////////////////////////
void replace_char (char *s, const char find, const char replace) {
	while (*s !=0) {
		if (*s==find)
			*s=replace;
		s++;
	}
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
int findNameFile(ModelInstance * _c, char* in_file, fmiString pattern)
{
	int found = 0;
	char name[MAXBUFFSIZE];
	DIR *dirp = opendir(_c->fmuResourceLocation);
	struct dirent entry;
	struct dirent *dp = &entry;
	// read directory 
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
		"Searching for follwoing pattern %s\n", pattern);
	while ((dp = readdir(dirp)))
	{
		_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
			"Read directory and search for *.idf, *.epw, or *.idd file.\n");
		// search pattern the filename
		if ((strstr(dp->d_name, pattern)) != 0)
		{
			_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "Found matching file %s.\n", dp->d_name);
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
int getResFile(ModelInstance *_c, fmiString pattern)
{
	char in_file[MAXBUFFSIZE] = { 0 };
	int found;
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
		"Get input file from resource folder %s.\n", _c->fmuResourceLocation);
	found = findNameFile(_c, in_file, pattern);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
		"done searching pattern %s\n", pattern);
	if (found > 1){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiate: Found more than "
			" (%d) with extension %s in directory %s. This is not valid.\n", found, pattern, _c->fmuResourceLocation);
		return 1;
	}
	if (in_file != NULL && strlen(in_file) != 0)
	{
		if (strncmp(pattern, ".idf", 4) == 0){
			_c->in_file = (char*)(_c->functions.allocateMemory(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->in_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
		else if (strncmp(pattern, ".epw", 4) == 0){
			_c->wea_file = (char*)(_c->functions.allocateMemory(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->wea_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
		else if (strncmp(pattern, ".idd", 4) == 0){
			_c->idd_file = (char*)(_c->functions.allocateMemory(strlen(in_file) + strlen(_c->fmuResourceLocation) + 1, sizeof(char)));
			sprintf(_c->idd_file, "%s%s", _c->fmuResourceLocation, in_file);
		}
	}
	else
	{
		if (strncmp(pattern, ".idf", 4) == 0){
			_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "Input file not found.");
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiate: No file with extension"
				" .idf found in the resource location %s. This is not valid.\n", _c->fmuResourceLocation);
			return 1;
		}
		if (strncmp(pattern, ".idd", 4) == 0){
			_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "IDD file not found.\n");
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiate: No file with extension"
				" .idd found in the resource location %s. This is not valid.\n", _c->fmuResourceLocation);
			return 1;
		}
		if (strncmp(pattern, ".epw", 4) == 0){
			_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "Weather file not found.\n");
			_c->functions.logger(NULL, _c->instanceName, fmiWarning, "warning", "fmiInstantiate: No file with extension"
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
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error",  "Can't open socket.cfg file.\n");
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
	tmp_str=(char*)(_c->functions.allocateMemory(strlen(_c->tmpResCon) + strlen (_c->fmuOutput) + 30, sizeof(char)));

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
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  
		"Command executes to copy content of resources folder: %s\n", tmp_str);
	retVal=system (tmp_str);
	_c->functions.freeMemory(tmp_str);
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
	tmp_str=(char*)(_c->functions.allocateMemory(strlen(_c->fmuOutput) + 30, sizeof(char)));

	sprintf(tmp_str, "mkdir %s%s%s", "\"", _c->fmuOutput, "\"");
	retVal=system (tmp_str);
	_c->functions.freeMemory (tmp_str);
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
	tmp_str=(char*)(_c->functions.allocateMemory(strlen(_c->fmuOutput) + 30, sizeof(char)));
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"This is the output folder %s\n", _c->fmuOutput);

#ifdef _MSC_VER
	sprintf(tmp_str, "rmdir /S /Q %s%s%s", "\"", _c->fmuOutput, "\"");
#else
	sprintf(tmp_str, "rm -rf %s%s%s", "\"", _c->fmuOutput, "\"");
#endif
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  
		"This is the command to be executed to delete existing directory %s\n", tmp_str);
	retVal=system (tmp_str);
	_c->functions.freeMemory (tmp_str);
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
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"**runenergyplus** has been deprecated as of August 2015."
		" EnergyPlusToFMU uses **energyplus** to call the EnergyPlus executable.");
#ifdef _MSC_VER
	fpBat=fopen("EP.bat", "w");
	if (stat(FRUNWEAFILE, &stat_p)>=0){
		// write the command string
		fprintf(fpBat, "energyplus %s %s %s %s %s %s %s %s", 
			"-w", FRUNWEAFILE, "-p", _c->mID, "-s", "C", 
			"-r", _c->in_file_name);
	}
	else
	{
		// write the command string
		fprintf(fpBat, "energyplus %s %s %s %s %s %s", 
			"-p", _c->mID, "-s", "C", "-r", _c->in_file_name);
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
		char *const argv[]={"energyplus", "-w", FRUNWEAFILE, "-p", _c->mID, "-s", "C", "-r", _c->in_file_name, NULL};
		// execute the command string
		retVal=posix_spawnp( &_c->pid, argv[0], NULL, NULL, argv, environ);
		return retVal;
	}
	else
	{
		//char *const argv[]={"runenergyplus", _c->mID, NULL};
		char *const argv[]={"energyplus", "-p", _c->mID, "-s", "C", "-r", _c->in_file_name, NULL};
		// execute the command string
		retVal=posix_spawnp( &_c->pid, argv[0], NULL, NULL, argv, environ);
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
const char* fmiStatusToString(fmiStatus status){
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
//
//
/////////////////////////////////////////////////////////////////////////////////
///// FMU logger
/////
/////\param c The FMU instance.
/////\param instanceName FMI string.
/////\param status FMI status.
/////\param category FMI string.
/////\param message Message to be recorded.
/////////////////////////////////////////////////////////////////////////////////
void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status,
	fmiString category, fmiString message, ...) {
		char msg[MAX_MSG_SIZE];
		char* copy;
		va_list argp;

		// Replace C format strings
		va_start(argp, message);
		vsprintf(msg, message, argp);

		// Replace e.g. ## and #r12#
		copy=strdup(msg);
		free(copy);

		// Print the final message
		if (!instanceName) instanceName="?";
		if (!category) category="?";
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
void freeInstanceResources(ModelInstance* _c) {
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"freeInstanceResources: %s will be freed.\n", _c->instanceName);
	// free model ID
	if (_c->mID!=NULL) _c->functions.freeMemory(_c->mID);
	_c->mID = NULL;
	// free model GUID
	if (_c->mGUID!=NULL) _c->functions.freeMemory(_c->mGUID);
	_c->mGUID = NULL;
	// free xml file
	if (_c->xml_file!=NULL) _c->functions.freeMemory(_c->xml_file);
	_c->xml_file = NULL;
	// free input file
	if (_c->in_file != NULL) _c->functions.freeMemory(_c->in_file);
	_c->in_file = NULL;
	// free weather file
	if (_c->wea_file != NULL) _c->functions.freeMemory(_c->wea_file);
	_c->wea_file = NULL;
	// free idd file
	if (_c->idd_file != NULL) _c->functions.freeMemory(_c->idd_file);
	_c->idd_file = NULL;
	// free resource location
	if (_c->fmuResourceLocation!=NULL) _c->functions.freeMemory(_c->fmuResourceLocation);
	_c->fmuResourceLocation = NULL;
	// free unzip location
	if (_c->fmuUnzipLocation!=NULL) _c->functions.freeMemory(_c->fmuUnzipLocation);
	_c->fmuUnzipLocation=NULL;
	// free output location
	if (_c->fmuOutput!=NULL) _c->functions.freeMemory(_c->fmuOutput);
	_c->fmuOutput = NULL;
	// free temporary result folder
	if (_c->tmpResCon!=NULL) _c->functions.freeMemory(_c->tmpResCon);
	_c->tmpResCon=NULL;
	// deallocate memory for inVec
	if (_c->inVec != NULL)  _c->functions.freeMemory(_c->inVec);
	_c->inVec = NULL;
	// deallocate memory for outVec
	if (_c->outVec != NULL)  _c->functions.freeMemory(_c->outVec);
	_c->outVec = NULL;
	 // free fmu instance
	if (_c!=NULL) _c->functions.freeMemory(_c);
	_c=NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// This function writes the path to the fmu resource location. 
///
///\param _c The FMU instance.
///\param path The path to the resource location.
///\return 0 if no error.
///////////////////////////////////////////////////////////////////////////////
int getResourceLocation(ModelInstance *_c, fmiString fmuLocation)      
{
	char tmpResLoc[5]={0};
	struct stat st;
	fmiBoolean errDir;

	// copy first 5 characters of fmuLocation
	strncpy (tmpResLoc, fmuLocation, 5);

	// allocate memory for fmuUnzipLocation 
	_c->fmuUnzipLocation=(char *)_c->functions.allocateMemory(strlen (fmuLocation) 
		+ strlen(PATH_SEP) + 1, sizeof(char));

	// extract the URI information from the fmuUnzipLocation path
#ifdef _MSC_VER
	if (strnicmp (tmpResLoc, "file", 4)==0)
#else
	if (strncasecmp (tmpResLoc, "file", 4)==0)
#endif
	{
		// The specification for defining whether the file should start with file:/, file://, or file:///
		// is not clear (e.g. see FMI 1.0, and FMI 2.0). We will thus check all cases to see what we have
		// case file: (If Ptolemy unzips an FMU in /tmp then the fmuLocation is file:/temp)
		strncpy(_c->fmuUnzipLocation, fmuLocation + 5, strlen(fmuLocation + 5));
		// check whether fmuUnzipLocation exists
		errDir=stat(_c->fmuUnzipLocation, &st);
		if (errDir<0)
		{
			_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
				"fmiInstantiateSlave: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);	
			_c->functions.freeMemory (_c->fmuUnzipLocation);
			_c->fmuUnzipLocation=NULL;
			// allocate memory for fmuUnzipLocation
			_c->fmuUnzipLocation=(char *)_c->functions.allocateMemory(strlen (fmuLocation) + 1, sizeof(char));
			// case file:/
			strncpy(_c->fmuUnzipLocation, fmuLocation + 6, strlen(fmuLocation + 6));
			// check whether fmuUnzipLocation exists
			errDir=stat(_c->fmuUnzipLocation, &st);
			if(errDir<0) 
			{
				_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
					"fmiInstantiateSlave: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);	
				_c->functions.freeMemory(_c->fmuUnzipLocation);
				_c->fmuUnzipLocation=NULL;
				// allocate memory for fmuUnzipLocation 
				_c->fmuUnzipLocation=(char *)_c->functions.allocateMemory(strlen (fmuLocation) + 1, sizeof(char));
				// case file://
				strncpy(_c->fmuUnzipLocation, fmuLocation + 7, strlen(fmuLocation + 7));

				// check whether fmuUnzipLocation exists
				errDir=stat(_c->fmuUnzipLocation, &st);
				if(errDir<0) 
				{
					_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",
						"fmiInstantiateSlave: Path to fmuUnzipLocation is not %s.\n", _c->fmuUnzipLocation);	
					_c->functions.freeMemory(_c->fmuUnzipLocation);
					_c->fmuUnzipLocation=NULL;
					// allocate memory for fmuUnzipLocation 
					_c->fmuUnzipLocation=(char *)_c->functions.allocateMemory(strlen (fmuLocation) + 1, sizeof(char));
					// case file:///
					strncpy(_c->fmuUnzipLocation, fmuLocation + 8, strlen(fmuLocation + 8));
					// check whether fmuUnzipLocation exists
					errDir=stat(_c->fmuUnzipLocation, &st);
					if(errDir<0) 
					{
						_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: The path to the unzipped"
							" folder %s is not valid. The path does not start with file: file:/, file:// or file:///\n", fmuLocation);
						_c->functions.freeMemory(_c->fmuUnzipLocation);
						_c->fmuUnzipLocation=NULL;
						return 1;
					}
				}
			}
		}
	}
#ifdef _MSC_VER
	else if ((strnicmp (tmpResLoc, "ftp", 3)==0) || (strnicmp (tmpResLoc, "fmi", 3)==0))
#else
	else if ((strncasecmp (tmpResLoc, "ftp", 3)==0) || (strncasecmp (tmpResLoc, "fmi", 3)==0))
#endif
	{
		strncpy(_c->fmuUnzipLocation, fmuLocation + 6, strlen(fmuLocation + 6));		
		_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
			"fmiInstantiateSlave: Path to fmuUnzipLocation without ftp:// or fmi:// %s\n", _c->fmuUnzipLocation);
	}

#ifdef _MSC_VER
	else if ((strnicmp (tmpResLoc, "https", 5)==0))
#else
	else if ((strncasecmp (tmpResLoc, "https", 5)==0))
#endif
	{
		strncpy(_c->fmuUnzipLocation, fmuLocation + 8, strlen(fmuLocation + 8));
		_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
			"fmiInstantiateSlave: Path to fmuUnzipLocation without https:// %s\n", _c->fmuUnzipLocation);
	}
	else
	{
		strcpy(_c->fmuUnzipLocation, fmuLocation);
		_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
			"fmiInstantiateSlave: Path to fmuUnzipLocation %s\n", _c->fmuUnzipLocation);
	}
	// Add back slash so we can copy files to the fmuUnzipLocation folder afterwards
	sprintf(_c->fmuUnzipLocation, "%s%s", _c->fmuUnzipLocation, PATH_SEP);

	// allocate memory for fmuResourceLocation
	_c->fmuResourceLocation=(char *)_c->functions.allocateMemory(strlen (_c->fmuUnzipLocation) 
		+ strlen (RESOURCES) + strlen(PATH_SEP) + 1, sizeof(char));
	sprintf(_c->fmuResourceLocation, "%s%s%s", _c->fmuUnzipLocation, RESOURCES, PATH_SEP);
	return 0;
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
	fmiString mFmiVers;
	struct stat st;
	fmiBoolean errDir;
	ModelInstance* _c;

	// Perform checks.
	if (!functions.logger)
		return NULL;
	if (!functions.allocateMemory || !functions.freeMemory){
		functions.logger(NULL, instanceName, fmiError, "error",
			"fmiInstantiateSlave: Missing callback functions: allocateMemory or freeMemory."
			" Instantiation of %s failed.\n", instanceName);
		return NULL;
	}

	// check instance name
	if (!instanceName || strlen(instanceName)==0 || (strlen(instanceName) > MAX_VARNAME_LEN)) {
		functions.logger(NULL, instanceName, fmiError, "error",
			"fmiInstantiateSlave: Missing instance name or instance name longer than 100 characters."
			" Instantiation of %s failed.\n", instanceName);
		return NULL;
	}

	//initialize the model instance
	_c=(ModelInstance *)functions.allocateMemory(1, sizeof(struct ModelInstance));

	// write instanceName to the struct
	strcpy(_c->instanceName, instanceName);

	// assign FMU parameters
	_c->functions=functions;
	_c->loggingOn=loggingOn;
	_c->visible=visible;
	_c->interactive=interactive;
	_c->timeout=timeout;

	// get current working directory
#ifdef _MSC_VER
	if (_getcwd(_c->cwd, sizeof(_c->cwd))==NULL)
#else
	if (getcwd(_c->cwd, sizeof(_c->cwd))==NULL)
#endif
	{
		_c->functions.logger(NULL, instanceName, fmiError, "error",
			"fmiInstantiateSlave: Cannot get current working directory."
			" Instantiation of %s failed.\n", _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	else
	{
		_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "The current working directory is %s\n", _c->cwd);
	}

	// create the output folder for current FMU in working directory
	_c->fmuOutput=(char *)_c->functions.allocateMemory(strlen ("Output_EPExport_") + strlen (_c->instanceName) 
		+ strlen (_c->cwd) + 5, sizeof(char));
	sprintf(_c->fmuOutput, "%s%s%s%s", _c->cwd, PATH_SEP, "Output_EPExport_", _c->instanceName);

	// check if directory exists and deletes it 
	errDir=stat(_c->fmuOutput, &st);
	if(errDir>=0) {
		_c->functions.logger(NULL, _c->instanceName, fmiWarning, "warning",
			"fmiInstantiate: The fmuOutput directory %s exists. It will now be deleted.\n", _c->fmuOutput);
		if(removeFMUDir (_c)!=0){
			_c->functions.logger(NULL, _c->instanceName, fmiWarning, "warning",
				"fmiInstantiate: The fmuOutput directory %s could not be deleted\n", _c->fmuOutput); 
		}
	}

	// check whether the path to the resource folder has been provided
	// note that in FMI 1.0 the resource location (fmuLocation) is actaully the folder where
	// the FMU has been unzipped whereas in 2.0, the resource location is the folder 
	// where the resource files are. So it is important to know this to avoid
	// looking for files in the wrong location.
	if((fmuLocation==NULL) || (strlen(fmuLocation)==0)) {
		_c->functions.logger(NULL, instanceName, fmiError, "error", "fmiInstantiateSlave: The path"
			" to the folder where the FMU is unzipped is not specified. This is not valid."
			" Instantiation of %s failed.\n", _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}

	// get the FMU resource location
	retVal=getResourceLocation (_c, fmuLocation);
	if (retVal!=0 ){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Could not get the resource location."
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
	_c->tmpResCon=(char *)_c->functions.allocateMemory(strlen (_c->fmuResourceLocation) + strlen (VARCFG) + strlen ("\"") + 1, sizeof(char));
	sprintf(_c->tmpResCon, "%s%s%s", _c->fmuResourceLocation, "\"", VARCFG);

	// create the output directory
	retVal=create_res(_c);
	if (retVal!=0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Could not create the output"
			" directory %s. Instantiation of %s failed.\n", _c->fmuOutput, _c->instanceName);
	}

	// add the end slash to the fmuOutput
	sprintf(_c->fmuOutput, "%s%s", _c->fmuOutput, PATH_SEP);

	// copy the vriables cfg into the output directory
	retVal=copy_var_cfg(_c);
	if (retVal!=0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Could not copy"
			" variables.cfg to the output directory folder %s. Instantiation of %s failed.\n", _c->cwd, _c->instanceName);
		// Free resources allocated to instance.
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
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Could not switch"
			" to the output folder %s. Instantiation of %s failed.\n", _c->fmuOutput, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}

	// create path to xml file
	_c->xml_file=(char *)_c->functions.allocateMemory(strlen (_c->fmuUnzipLocation) + strlen (XML_FILE) + 1, sizeof(char));
	sprintf(_c->xml_file, "%s%s", _c->fmuUnzipLocation, XML_FILE);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"fmiInstantiateSlave: Path to model description file is %s.\n", _c->xml_file);

	// get model description of the FMU
	_c->md=parse(_c->xml_file);
	if (!_c->md) {
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Failed to parse the model description"
			" found in directory %s. Instantiation of %s failed\n", _c->xml_file, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}

	// gets the modelID of the FMU
	mID=getModelIdentifier(_c->md);

	// copy model ID to FMU
	_c->mID=(char *)_c->functions.allocateMemory(strlen(mID) + 1, sizeof(char));
	strcpy(_c->mID, mID);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "fmiInstantiateSlave: The FMU modelIdentifier is %s.\n", _c->mID);

	// get the model GUID of the FMU
	mGUID=getString(_c->md, att_guid);

	// copy model GUID to FMU
	_c->mGUID=(char *)_c->functions.allocateMemory(strlen (mGUID) + 1,sizeof(char));
	strcpy(_c->mGUID, mGUID);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", "fmiInstantiateSlave: The FMU modelGUID is %s.\n", _c->mGUID);
	// check whether GUIDs are consistent with modelDescription file
	if(strcmp(fmuGUID, _c->mGUID) !=0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			" fmiInstantiateSlave: Wrong GUID %s. Expected %s. Instantiation of %s failed.\n", fmuGUID, _c->mGUID, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}

	// check whether the model is exported for FMI version 1.0
	mFmiVers=getString(_c->md, att_fmiVersion);
	if(strcmp(mFmiVers, FMIVERSION) !=0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Wrong FMI version %s."
			" FMI version 1.0 is currently supported. Instantiation of %s failed.\n", mFmiVers, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"fmiInstantiateSlave: Slave %s is instantiated.\n", _c->instanceName);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal=_chdir(_c->cwd);
#else
	retVal=chdir(_c->cwd);
#endif
	if (retVal!=0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInstantiateSlave: Could not switch to"
			" the working directory folder %s. Instantiation of %s failed.\n", _c->cwd, _c->instanceName);
		// Free resources allocated to instance.
		freeInstanceResources (_c);
		return NULL;
	}
	// This is required to prevent Dymola to call fmiSetReal before the initialization
	_c->firstCallIni=1;
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok", 
		"fmiInstantiateSlave: Instantiation of %s succeded.\n", _c->instanceName);
	return(_c); 
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
	ModelInstance* _c=(ModelInstance *)c;
	FILE *fp;
	char tStartFMUstr[100];
	char tStopFMUstr[100];
	char command[100];
	char *tmpstr;
	char *cmdstr;

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

	// save start of the simulation time step
	_c->tStartFMU=tStart;
	// save end of smulation time step
	_c->tStopFMU=tStop;

	// initialize structure variables
	_c->firstCallGetReal      =1;
	_c->firstCallSetReal      =1;
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

	// change the directory to make sure that FMUs are not overwritten
#ifdef _MSC_VER
	retVal=_chdir(_c->fmuOutput);
#else
	retVal=chdir(_c->fmuOutput);
#endif
	if (retVal!=0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			"fmiInitializeSlave: The path to the output folder %s is not valid.\n", _c->fmuOutput);
		return fmiError;
	}
	///////////////////////////////////////////////////////////////////////////////////
	// create the socket server

#ifdef _MSC_VER
	// initialize winsock  /************* Windows specific code ********/
	if (WSAStartup(wVersionRequested, &wsaData)!=0)
	{
		_c->functions.logger(NULL, _c->instanceName , fmiError, 
			"error", "fmiInitializeSlave: WSAStartup failed with error %ld.\n", WSAGetLastError());
		WSACleanup();
		return fmiError;
	}
	// check if the version is supported
	if (LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2 )
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: Could not find a usable WinSock DLL for WinSock version %u.%u.\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.		wVersion));
		WSACleanup();
		return fmiError;
	}
#endif  /************* End of Windows specific code *******/

	_c->sockfd=socket(AF_INET, SOCK_STREAM, 0);
	// check for errors to ensure that the socket is a valid socket.
	if (_c->sockfd==INVALID_SOCKET)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: Opening socket failed"
			" sockfd=%d.\n", _c->sockfd);
		return fmiError;
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: The sockfd is %d.\n", _c->sockfd);
	// initialize socket structure server address information
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;                 // Address family to use
	server_addr.sin_port=htons(0);                  // Port number to use
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // Listen on any IP address

	// bind the socket
	if (bind(_c->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))==SOCKET_ERROR)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: bind() failed.\n");
		closeipcFMU (&(_c->sockfd));
		return fmiError;
	}

	// get socket information information
	sockLength=sizeof(server_addr);
	if ( getsockname (_c->sockfd, (struct sockaddr *)&server_addr, &sockLength)) {
		_c->functions.logger(NULL,  _c->instanceName, fmiError,
			"error", "fmiInitializeSlave: Get socket name failed.\n");
		return fmiError;
	}

	// get the port number
	port_num=ntohs(server_addr.sin_port);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: The port number is %d.\n", port_num);

	// get the hostname information
	gethostname(ThisHost, MAXHOSTNAME);
	if  ((hp=gethostbyname(ThisHost))==NULL ) {
		_c->functions.logger(NULL,  _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: Get host by name failed.\n");
		return fmiError;
	}

	// write socket cfg file
	retVal=write_socket_cfg (_c, port_num, ThisHost);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: This hostname is %s.\n", ThisHost);
	if  (retVal !=0) {
		_c->functions.logger(NULL,  _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: Write socket cfg failed.\n");
		return fmiError;
	}
	// listen to the port
	if (listen(_c->sockfd, 1)==SOCKET_ERROR)
	{
		_c->functions.logger(NULL,  _c->instanceName, fmiError, "error", "fmiInitializeSlave: listen() failed.\n");
		closeipcFMU (&(_c->sockfd));
		return fmiError;
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: TCPServer Server waiting for clients on port: %d.\n", port_num);

	// get the number of input variables of the FMU
	if (_c->numInVar==-1)
	{
		_c->numInVar=getNumInputVariablesInFMU (_c->md);
		// initialize the input vectors
		_c->inVec=(fmiReal*)_c->functions.allocateMemory(_c->numInVar, sizeof(fmiReal));
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: The number of input variables is %d.\n", _c->numInVar);

	// get the number of output variables of the FMU
	if (_c->numOutVar==-1)
	{
		_c->numOutVar= getNumOutputVariablesInFMU (_c->md);
		// initialize the output vector
		_c->outVec=(fmiReal*)_c->functions.allocateMemory(_c->numOutVar, sizeof(fmiReal));
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: The number of output variables is %d.\n", _c->numOutVar);

	if ( (_c->numInVar + _c->numOutVar)==0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			"fmiInitializeSlave: The FMU instance %s has no input and output variables. Please check the model description file.\n",
			_c->instanceName);
		return fmiError;
	}
	// create the input and weather file for the run
	// Need to see how we will parste the start and stop time so 
	// they become strings and can be used by str when calling the system command.

	// get input file from the folder. There must be only one input file in the folder.
	retVal = getResFile(_c, ".idf");
	if (retVal != 0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInitializeSlave: Could not get"
			" the .idf input file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmiError;
	}
	// get the weather file from the folder. there must be only one weather file in the folder.
	retVal = getResFile(_c, ".epw");
	if (retVal != 0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInitializeSlave: Could not"
			" get the .epw weather file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmiError;
	}
	// get the idd file from the folder. there must be only one idd file in the folder.
	retVal = getResFile(_c, ".idd");
	if (retVal != 0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInitializeSlave: Could not"
			" get the .idd dictionary file. Instantiation of %s failed.\n",
			_c->instanceName);
		return fmiError;
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
		cmdstr = (char *)_c->functions.allocateMemory(strlen(_c->fmuResourceLocation) + strlen(command) + 10, sizeof(char));
		sprintf(cmdstr, "%s%s", _c->fmuResourceLocation, command);
		tmpstr = (char *)_c->functions.allocateMemory(strlen(cmdstr) + strlen(_c->wea_file) +
			strlen(_c->idd_file) + strlen(_c->in_file) + strlen(tStartFMUstr) + strlen(tStopFMUstr) + 50, sizeof(char));
		sprintf(tmpstr, "%s -w %s -b %s -e %s %s %s", cmdstr, _c->wea_file, tStartFMUstr, tStopFMUstr, _c->idd_file, _c->in_file);
		retVal = system(tmpstr);
	}
	else{
		cmdstr = (char *)_c->functions.allocateMemory(strlen(_c->fmuResourceLocation) + strlen(command) + 10, sizeof(char));
		sprintf(cmdstr, "%s%s", _c->fmuResourceLocation, command);
		tmpstr = (char *)_c->functions.allocateMemory(strlen(cmdstr) + 
			strlen(_c->idd_file) + strlen(_c->in_file) + strlen(tStartFMUstr) + strlen(tStopFMUstr) + 50, sizeof(char));
		sprintf(tmpstr, "%s -b %s -e %s %s %s", cmdstr, tStartFMUstr, tStopFMUstr, _c->idd_file, _c->in_file);
		retVal = system(tmpstr);
	}
	_c->functions.freeMemory(cmdstr);
	_c->functions.freeMemory(tmpstr);
	if (retVal != 0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInitializeSlave: Could not"
			" create the input and weather file. Initialization of %s failed.\n",
			_c->instanceName);
		return fmiError;
	}
//#ifndef _MSC_VER
//	// create a directory and copy the weather file into it
//	if (stat (FRUNWEAFILE, &stat_p)>=0)
//	{
//		if (stat ("WeatherData", &stat_p)<0)
//		{
//			mkdir ("WeatherData", S_IRWXU | S_IRWXG | S_IRWXO);
//			tmpstr=(char *)_c->functions.allocateMemory(strlen (FRUNWEAFILE) + strlen ("WeatherData/") + 10, sizeof(char));
//			sprintf(tmpstr, "cp -f %s %s", FRUNWEAFILE, "WeatherData/");
//			retVal=system (tmpstr);
//			_c->functions.freeMemory(tmpstr);
//		}
//		// set environment variable for weather file
//		setenv ("ENERGYPLUS_WEATHER", "WeatherData", 0);
//	}
//#endif

	// rename found idf to have the correct name.
	tmpstr = (char *)_c->functions.allocateMemory(strlen(_c->mID) + strlen(".idf") + 1, sizeof(char));
	sprintf(tmpstr, "%s%s", _c->mID, ".idf");
	strcpy(_c->in_file_name, tmpstr);
	// free tmpstr
	_c->functions.freeMemory(tmpstr);
	retVal = rename(FRUNINFILE, _c->in_file_name);
	if (retVal != 0){
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiInitializeSlave: Could not"
			" rename the temporary input file. Initialization of %s failed.\n",
			_c->instanceName);
		return fmiError;
	}

	if((fp=fopen(FTIMESTEP, "r")) !=NULL) {
		retVal=fscanf(fp, "%d", &(_c->timeStepIDF));
		fclose (fp);
		// check if the timeStepIDF is null to avoid division by zero
		if (_c->timeStepIDF==0){
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
				"fmiInitializeSlave: The time step in IDF cannot be null.\n");
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error",   "fmiInitializeSlave: Time step in IDF is null.\n");
			return fmiError;
		}	
	}
	else
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			"fmiInitializeSlave: A valid time step could not be determined.\n");
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error",   "fmiInitializeSlave: Can't read time step file.\n");
		return fmiError;
	}

#ifndef _MSC_VER
	umask(process_mask);
#endif
	// start the simulation
	retVal=start_sim(_c);
	_c->newsockfd=accept(_c->sockfd, NULL, NULL);
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: The connection has been accepted.\n");
	// check whether the simulation could start successfully
	if  (retVal !=0) {
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"error", "fmiInitializeSlave: The FMU instance could %s not be initialized. "
			"EnergyPlus can't start . Check if EnergyPlus is installed and on the system path.\n", 
			_c->instanceName);
		return fmiError;
	}

	// reset firstCallIni
	if (_c->firstCallIni) 
	{
		_c->firstCallIni=0;
	}
	_c->functions.logger(NULL, _c->instanceName, fmiOK, "ok",  "fmiInitializeSlave: Slave %s is initialized.\n", _c->instanceName);
	// reset the current working directory. This is particularly important for Dymola
	// otherwise Dymola will write results at wrong place
#ifdef _MSC_VER
	retVal=_chdir(_c->cwd);
#else
	retVal=chdir(_c->cwd);
#endif
	return fmiOK;
} 

////////////////////////////////////////////////////////////////
///  This method is used to do the time stepping the FMU
///
///\param c The FMU instance.
///\param currentCommunicationPoint The communication point.
///\param communicationStepSize The communication step size.
///\param newStep The flag to accept or reflect communication step.
///\return fmiOK if no error occurred.
////////////////////////////////////////////////////////////////
DllExport fmiStatus fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep)
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
	}

	// check for the first communication instant
	if (_c->firstCallDoStep && (fabs(_c->curComm - 
		_c->tStartFMU) > 1e-10))
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			"fmiDoStep: An error occured in a previous call. First communication time: %f !=tStart: %f.\n",
			_c->curComm, _c->tStartFMU);
		return fmiError;
	}
	// check if FMU needs to reject time step
	if(!newStep)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", 
			"fmiDoStep: FMU can not reject time steps.");
		return fmiError;
	}

	// check whether the communication step size is different from null
	if (_c->communicationStepSize==0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"error", "fmiDoStep: An error occured in a previous call. CommunicationStepSize cannot be null.\n");
		return fmiError;
	}

	// check whether the communication step size is different from time step in input file
	if ( fabs(_c->communicationStepSize - (3600/_c->timeStepIDF)) > 1e-10)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiDoStep:"
			" An error occured in a previous call. CommunicationStepSize: %f is different from time step: %d in input file.\n",
			_c->communicationStepSize, _c->timeStepIDF);
		return fmiError;
	}

	// check whether communication point is valid
	if ((_c->curComm) < 0 || ((_c->firstCallDoStep==0) 
		&& (_c->curComm > _c->nexComm))){
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiDoStep:"
				" An error occured in a previous call. Communication point must be positive and monoton increasing.\n");
			return fmiError;
	}

	// check whether current communication point is valid
	if ((_c->firstCallDoStep==0)
		&& (fabs(_c->curComm - _c->nexComm) > 1e-10))
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", "fmiDoStep: "
			"Current communication point: %f is not equals to the previous simulation time + "
			"communicationStepSize: %f + %f.\n",
			_c->curComm, _c->nexComm, 
			_c->communicationStepSize);
		return fmiError;
	}

	// check end of simulation
	if (_c->curComm==_c->tStopFMU){
		// set the communication flags to 1 to send stop signal to EnergyPlus
		_c->functions.logger(NULL, _c->instanceName, fmiWarning, 
			"Warning", "fmiDoStep: Current communication point: %f of FMU instance: %s "
			"is equals to end of simulation: %f.\n", 
			_c->curComm, _c->instanceName, _c->tStopFMU);
		return fmiWarning;
	}

	// check if current communication is larger than end of simulation
	if (_c->curComm > _c->tStopFMU){
		// set the communication flags to 1 to send stop signal to EnergyPlus
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", "fmiDoStep:"
			" Current communication point: %f is larger than end of simulation time: %f.\n", 
			_c->curComm, _c->tStopFMU);
		return fmiError;
	}
	// check end of simulation
	if (_c->curComm + 
		_c->communicationStepSize > _c->tStopFMU){
			// set the communication flags to 1 to send stop signal to EnergyPlus
			_c->functions.logger(NULL, _c->instanceName, fmiError, "error", "fmiDoStep: "
				"Current communication point: %f  + communicationStepsize: %f  is larger than "
				"end of simulation time: %f.\n", 
				_c->curComm, _c->communicationStepSize,  
				_c->tStopFMU);
			return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, 
		"Warning", "fmiCancelStep: The function fmiCancelStep: is not provided.\n");
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
		ModelInstance* _c=(ModelInstance *)c;
		_c->functions.logger(NULL, _c->instanceName, fmiOK, 
		"ok", "fmiTerminateSlave: fmiFreeInstanceSlave must be called to free the FMU instance.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiResetSlave: fmiResetSlave:: is not provided.\n");
	return fmiWarning;
}

////////////////////////////////////////////////////////////////
///  This method is used to free the FMU instance
///
///\param c The FMU instance.
////////////////////////////////////////////////////////////////
DllExport void fmiFreeSlaveInstance(fmiComponent c)
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
		_c->functions.logger(NULL, _c->instanceName, fmiOK, 
		"ok", "fmiFreeSlaveInstance: The function fmiFreeSlaveInstance of instance %s is executed.\n", 
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
		printf ("fmiFreeSlaveInstance: FMU instance was NULL. fmiFreeSlaveInstance will return.\n");
		return;
	}
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiSetDebugLogging: fmiSetDebugLogging(): fmiSetDebugLogging is not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	int retVal;
	// to prevent the fmiSetReal to be called before the FMU is initialized
	if (_c->firstCallIni==0)
	{
		// fmiValueReference to check for input variable
		fmiValueReference vrTemp;
		ScalarVariable** vars;
		int i, k;

		vars=_c->md->modelVariables;
		if (!_c->writeReady){
			for(i=0; i<nvr; i++)
			{
				for (k=0; vars[k]; k++) {
					ScalarVariable* svTemp=vars [k];
					if (getAlias(svTemp)!=enu_noAlias) continue;
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
		if (_c->firstCallSetReal){
			_c->firstCallSetReal=0;
		}
		return fmiOK;
	}
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", 
			"fmiSetInteger: fmiSetInteger: was called. The FMU does not contain integer variables to set.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", 
			"fmiSetBoolean: fmiSetBoolean: was called. The FMU does not contain boolean variables to set.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"Error", "fmiSetString: fmiSetString: was called. The FMU does not contain string variables to set.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	int retVal;
	// to prevent the fmiGetReal to be called before the FMU is initialized
	if (_c->firstCallIni==0){
		fmiValueReference vrTemp;
		ScalarVariable** vars;
		int i, k;

		vars=_c->md->modelVariables;
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
				for (k=0; vars[k]; k++) {
					ScalarVariable* svTemp=vars [k];
					if (getAlias(svTemp)!=enu_noAlias) continue;
					if (getCausality(svTemp) !=enu_output) continue; 
					vrTemp=getValueReference(svTemp);
					if (vrTemp==vr[i]){
						value[i]=_c->outVec[vr[i]-10001];
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
		return fmiOK;
	}
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", 
			"fmiGetInteger: fmiGetInteger: was called. The FMU does not contain integer variables to get.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, "Error", 
			"fmiGetBoolean: fmiGetBoolean: was called. The FMU does not contain boolean variables to get.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	if(nvr>0)
	{
		_c->functions.logger(NULL, _c->instanceName, fmiError, 
			"Error", "fmiGetString: fmiGetString: was called. The FMU does not contain string variables to get.\n");
		return fmiError;
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetRealOutputDerivatives: fmiGetRealOutputDerivatives: Real Output Derivatives are not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiSetRealInputDerivatives: fmiSetRealInputDerivatives: Real Input Derivatives are not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetStatus: fmiGetStatus: is not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetRealStatus: fmiGetRealStatus: is not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetIntegerStatus: fmiGetIntegerStatus: is not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetBooleanStatus: fmiGetBooleanStatus: is not provided.\n");
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
	ModelInstance* _c=(ModelInstance *)c;
	_c->functions.logger(NULL, _c->instanceName, fmiWarning, "Warning", 
		"fmiGetStringStatus: fmiGetStringStatus: is not provided.\n");
	return fmiWarning;
}

//int main(){
//	double time;
//	double tStart=0;               // start time
//	double tStop=86400;               // start time
//	const char* guid="b2bb88a46354b2a1a56ca004d67bb91a";                // global unique id of the fmu
//	fmiComponent c;                  // instance of the fmu 
//	fmiStatus fmiFlag;               // return code of the fmu functions
//	const char* fmuLocation="file:///Z:\\linux\\proj\\fmi\\src\\svn\\fmu\\EnergyPlus\\export\\trunk\\Scripts\\_fmu_export_schedule"; // path to the fmu as URL, "file://C:\QTronic\sales"
//	const char* mimeType="application/x-fmu-sharedlibrary"; // denotes tool in case of tool coupling
//	fmiReal timeout=1000;          // wait period in milli seconds, 0 for unlimited wait period"
//	fmiBoolean visible=fmiFalse;   // no simulator user interface
//	fmiBoolean interactive=fmiFalse; // simulation run without user interaction
//	fmiCallbackFunctions callbacks;  // called by the model during simulation
//	ModelDescription* md;            // handle to the parsed XML file
//	int nSteps=0;
//	int loggingOn=0;
//	int retVal;
//	FILE* file;
//	const fmiValueReference valRefIn[]={1};
//	const fmiValueReference valRefOut[]={100001};
//	fmiReal valIn[1]={0};
//	fmiReal valOut[1];
//
//	
//	callbacks.logger=fmuLogger;
//	callbacks.allocateMemory=calloc;
//	callbacks.freeMemory=free;
//	callbacks.stepFinished=NULL; // fmiDoStep has to be carried out synchronously
//	// instantiate the fmu
//	c=fmiInstantiateSlave("_fmu_export_schedule", guid, fmuLocation, mimeType,
//		timeout, visible, interactive, callbacks, loggingOn);
//	// initialize the FMU
//	retVal=fmiInitializeSlave(c, tStart, fmiTrue, tStop);
//	time=0;
//	while (time < tStop) {
//		// set the inputs
//		retVal = fmiSetReal(c, valRefIn, 1, valIn);
//		// do step
//		retVal= fmiDoStep(c, time, 900, 1);
//		// get the outputs
//		retVal = fmiGetReal(c, valRefOut, 1, valOut);
//		printf ("This is the value of output %f\n", valOut);
//		time=time+900;
//	}
//	// terminate the FMU
//	retVal=fmiTerminateSlave(c);
//	// free te FMU
//	fmiFreeSlaveInstance(c);
//	printf ("Simulation successfully terminated\n");
//}

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