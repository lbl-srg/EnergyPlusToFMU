// Methods for Functional Mock-up Unit Export of EnergyPlus.

///////////////////////////////////////////////////////////////////
// \file   reader.c
//
// \brief  Functions used to read an IDF file and create a new
//         one with start, end time and day of week of RUNPERIOD
//         based on FMU parameters. This function
//         creates the  runinfile.idf and a runweafile.epw 
//         for the simulation. This function assumes that the idf
//         file as well as the weather file are both in the resources
//         folder distributed with the FMU.
//
// \author Thierry Stephane Nouidui,
//         Simulation Research Group, 
//         LBNL,
//         TSNouidui@lbl.gov
//
// \date   2012-09-26
//
////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include "fmiModelTypes.h"
#include "defines.h"
#include <string.h>
#include<sys/stat.h>

#ifdef _MSC_VER
#include <windows.h>
#include "dirent_win.h"

#else
#include <dirent.h>
//#include <sys/stat.h>
#include <ctype.h>
#endif
#include "reader.h"
#include "util.h"

// LOG is created if more than two RunPeriod
const char* LOG           = "log_runP.txt";
const char* BEGINDAYMONTH = "beginDayMonth.txt";
const char* ENDDAYMONTH   = "endDayMonth.txt";
const char* BEGINMONTH    = "beginMonth.txt";
const char* ENDMONTH      = "endMonth.txt";
const char* NEWDAYWEEK    = "newDayWeek.txt";
const char* OLDDAYWEEK    = "oldDayWeek.txt";

#define MAXBUFFSIZE 1024

///////////////////////////////////////////////////////////////////////////////
/// This function calculates the modulo of tow doubles. 
///
///\param a First input.
///\param b Second input.
///\return The modulo of two doubles. 
///////////////////////////////////////////////////////////////////////////////
static double modulus(double a, double b)
{
	int result = (int)( a / b );
	return a - (double)( result ) * b;
}

///////////////////////////////////////////////////////////////////////////////
/// This function finds a file with a specific extension in a folder. 
/// It returns the name of the found file with its extension.
///
///\param path The path to file.
///\param pattern The pattern to search.
///\return The name of the file found with extension. 
///        Otherwise, return 1 to indicate error. 
///////////////////////////////////////////////////////////////////////////////
char *findNameFile(char *path, char *pattern)      
{
	int found =0;
	char name[MAXBUFFSIZE];
	DIR *dirp=opendir(path);
	struct dirent entry;
	struct dirent *dp=&entry;
	char *infile = NULL;
	// read directory 
	while(dp = readdir(dirp))
	{
		printDebug("Read directory and search for *.idf or *.epw!\n");
		// search pattern the filename
		if((strstr(dp->d_name, pattern))!= 0)
		{    
			printDebug("Found *.idf or *.epw!\n");
			found++;
			strcpy(name, dp->d_name);
			// allocate memory to save filename
			infile = (char*)(calloc(sizeof(char), strlen(name) + 1));
			// copy filename to be returned
			strncpy(infile, name, strlen(name));
			//close directory
			closedir(dirp);
			return infile;
		}
	}
	closedir(dirp);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// This function deletes temporary create files. 
///////////////////////////////////////////////////////////////////////////////
void findFileDelete()      
{
	struct stat stat_p;
	int res;

	// delete temporary files
	if (stat(BEGINDAYMONTH, &stat_p)>=0)
	{
		remove(BEGINDAYMONTH);
	}
	if (stat (BEGINMONTH, &stat_p)>=0)
	{
		remove(BEGINMONTH);
	}
	if (stat (ENDDAYMONTH, &stat_p)>=0)
	{
		remove(ENDDAYMONTH);
	}
	if (stat (ENDMONTH, &stat_p)>=0)
	{
		remove(ENDMONTH);
	}
	if (stat(NEWDAYWEEK, &stat_p)>=0)
	{
		remove(NEWDAYWEEK);
	}
	if (stat (OLDDAYWEEK, &stat_p)>=0)
	{
		remove(OLDDAYWEEK);
	}
	if (stat (VARCFG, &stat_p)>=0)
	{
		remove(VARCFG);
	}
	if (stat (SOCKCFG, &stat_p)>=0)
	{
		remove(SOCKCFG);
	}
	if (stat (EPBAT, &stat_p)>=0)
	{
		remove(EPBAT);
	}
	if (stat (FTIMESTEP, &stat_p)>=0)
	{
		remove(FTIMESTEP);
	}
	if (stat (FRUNWEAFILE, &stat_p)>=0){
		// cleanup .epw files
#ifdef _MSC_VER
		res = system ("del *.epw");
#else
		res = system ("rm -f *.epw");
#endif
	}
}    
///////////////////////////////////////////////////////////////////////////////
/// This function removes tabs and line ends in a string.
///
///\param fname The string.
///\return The length of the string without tabs and line ends.
///////////////////////////////////////////////////////////////////////////////
static int isEmptyLine(const char *s) {
	static const char *WS = " \t\n";
	return strspn(s, WS) == strlen(s);
}

///////////////////////////////////////////////////////////////////////////////
/// This function removes spaces and make upper characters.
///
///\param fname The string.
///////////////////////////////////////////////////////////////////////////////
void remSpaces_makeUpper(char *infile){
	char *tmp = infile;
	int i,j = 0;

	for(i=0;i<=strlen(infile);i++){
		if(infile[i]!=' '){
			tmp[j] = toupper(infile[i]);
			j++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function replace a character with another one.
///
///\param from The character to replace
///\param to The character to use
///\param to The string to modify
///////////////////////////////////////////////////////////////////////////////
static void replace_char_from_string(char from, char to, char *str)
{
	int i = 0;
	int len = strlen(str)+1;

	for(i=0; i<len; i++)
	{
		if(str[i] == from)
		{
			str[i] = to;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function reads a file extracts relevant information for date and print 
/// the information in a file.
///
///\param temp The string.
///\param fname The filename to print.
///\param fname The index of string to be printed.
///////////////////////////////////////////////////////////////////////////////
static void printDataForDate (char *temp, const char* fname){
	FILE *fp;
	int i;
	char result[MAXBUFFSIZE] = {0};
	remSpaces_makeUpper(temp);
	if (strlen(temp)==0)
	{
		if((fp = fopen(fname, "w")) == NULL) {
			printf("Can't open file!\n");
			exit(42);  // STL error code: File not open.
		}
		else
		{   
			fprintf(fp, "%s", " ");
			fclose (fp);
			return;
		}
	}
	else{
		for(i=0; i<strlen(temp); i++)
		{
			if (temp[i] != ',')
			{
				result[i] = temp [i];
			}
			else if (temp[i] == ',')
			{
				break;
			}
			else
			{
				continue;
			}
		}
		// write timestep in file
		if((fp = fopen(fname, "w")) == NULL) {
			printf("Can't open file!\n");
			exit(42);  // STL error code: File not open. 
		}
		else
		{   
			remSpaces_makeUpper(result);
			fprintf(fp, "%s", result);
			fclose (fp);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts a string value written from a file into an integer.
///
///
///\param fname The filename.
///\return The converted string as an integer.
///////////////////////////////////////////////////////////////////////////////
static int getNumValue(const char* fname){
	FILE *fp;
	int result, err_n;
	char buff[3]= {0};
	// open file
	if((fp = fopen(fname, "r")) == NULL) {
		printf ("Can't open file %s!\n", fname);
		return(1);
	}
	// read the file
	if(fgets( buff, sizeof buff, fp ) != NULL);
	// convert the string into an integer
	err_n = sscanf( buff, "%d", &result);
	if ( err_n !=1) {
		printf("Can't read begin of month!\n");
	}
	//close the file
	fclose (fp);
	return result;
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
	const char *fname, const char *fname1){
		FILE *fp1;
		FILE *fp2;
		char day_of_week[MAXBUFFSIZE] = {0};
		int modDat;
		char arr[7][10]= { "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };
		int new_index;
		int old_index;
		int change = 1;

		printDebug("Get current day of week from input file and update accoring to FMU parameters");
		// open file
		if((fp1 = fopen(fname, "r")) == NULL) {
			printf ("Can't open file %s\n", fname);
			return(1);
		}
		// read file line by line
		if(fgets( day_of_week, MAXBUFFSIZE, fp1) != NULL);
		fclose(fp1);

		//t_start_idf = t_start_idf + 86400;
		// deternmine th difference between start time in idf and start time in fmu
		modDat =(((int)(t_start_fmu  - t_start_idf)/86400)%86400)%7;

		// remove blanks line and make upper
		remSpaces_makeUpper (day_of_week);
		// open file
		if((fp2 = fopen(fname1, "w")) == NULL) {
			printf ("Can't open file %s\n", fname1);
			return(1);
		}
		if (strstr(day_of_week, "SUNDAY") != NULL) 
		{
			old_index = 0;
		} 
		else if (strstr(day_of_week, "MONDAY") != NULL) 
		{
			old_index = 1;
		}
		else if (strstr(day_of_week, "TUESDAY") != NULL) 
		{
			old_index =  2;
		}
		else if (strstr(day_of_week, "WEDNESDAY") != NULL) 
		{
			old_index = 3;
		}
		else if (strstr(day_of_week, "THURSDAY") != NULL) 
		{
			old_index = 4;
		}
		else if (strstr(day_of_week, "FRIDAY") != NULL) 
		{
			old_index = 5;
		}

		else if (strstr(day_of_week, "SATURDAY") != NULL) 
		{
			old_index = 6;
		}

		else if (strstr(day_of_week, "USEWEATHERFILE") != NULL) 
		{
			change = 0;
			// write the new day of week
			fprintf(fp2, "%s", day_of_week);
			printDebug("Day of week: UseWeatherFile has been specified and will be used.");
			fclose (fp2);
			return 0;
		}
		else
		{
			// write the new day of week, no day of week was specified.
			fprintf(fp2, "%s", " ");
			printDebug("Day of week was left blank in input file.");
			fclose (fp2);
			return 0;
		}

		if (change !=0) {
			// determine the new index
			if (modDat > 0)
			{
				new_index = (old_index + modDat)%7; 
			}
			else if (modDat < 0)
			{
				new_index = modDat + 7;
				new_index = (new_index + old_index)%7;
			}

			else
			{
				new_index = old_index;
			}

			// write the new day of week
			fprintf(fp2, "%s", arr[new_index]);
		}

		// close file
		fclose (fp2);
		return 0;
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

	if(!(leapyear)) {
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
			simtime = 90 + 1 ;
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

	simtime = 24 * (simtime +(day-1));
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

	if(!(leapyear)) {
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
			simtime = 90 + 1 ;
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
///
///\param month The time.
///\param leapyear The flag for leap year.
///\return The month.
///////////////////////////////////////////////////////////////////////////////
static int getMonth(int time_s, int leapyear){

	int month;
	int tmp;

	tmp = time_s/86400;

	if(!(leapyear)) {

		if (tmp <= 31) {
			month = 1;
		}
		else if ((tmp > 31)  && (tmp <= 59)){
			month = 2;
		}
		else if ((tmp > 59)  && (tmp <= 90)){
			month = 3;
		}
		else if ((tmp > 90)  && (tmp <= 120)){
			month = 4;
		}
		else if ((tmp > 120)  && (tmp <= 151)){
			month = 5;
		}
		else if ((tmp > 151)  && (tmp <= 181)){
			month = 6;
		}
		else if ((tmp > 181)  && (tmp <= 212)){
			month = 7;
		}
		else if ((tmp > 212)  && (tmp <= 243)){
			month = 8;
		}
		else if ((tmp > 243)  && (tmp <= 273)){
			month = 9;
		}
		else if ((tmp > 273)  && (tmp <= 304)){
			month = 10;
		}
		else if ((tmp > 304)  && (tmp <= 334)){
			month = 11;
		}
		else if ((tmp > 334)  && (tmp <= 365)){
			month = 12;
		}
		//if the time is larger than a year
		else{
			printf ("Time set is larger than maximum allowed (365 days). Month will be set to 12!\n");
			month = 12;
		}
	}

	else{
		if (tmp <= 31) {
			month = 1;
		}
		else if ((tmp > 31)  && (tmp <= 60)){
			month = 2;
		}
		else if ((tmp > 60)  && (tmp <= 91)){
			month = 3;
		}
		else if ((tmp > 91)  && (tmp <= 121)){
			month = 4;
		}
		else if ((tmp > 121)  && (tmp <= 152)){
			month = 5;
		}
		else if ((tmp > 152)  && (tmp <= 182)){
			month = 6;
		}
		else if ((tmp > 182)  && (tmp <= 213)){
			month = 7;
		}
		else if ((tmp > 213)  && (tmp <= 244)){
			month = 8;
		}
		else if ((tmp > 244)  && (tmp <= 274)){
			month = 9;
		}
		else if ((tmp > 274)  && (tmp <= 305)){
			month = 10;
		}
		else if ((tmp > 305)  && (tmp <= 335)){
			month = 11;
		}
		else if ((tmp > 335)  && (tmp <= 366)){
			month = 12;
		}
		//if the time is larger than a year
		else{
			printf ("Time set is larger than maximum allowed (366 days). Month will be set to 12!\n");
			month = 12;
		}
	}
	return month;
}

///////////////////////////////////////////////////////////////////////////////
/// This function converts the time in seconds in month.
///
///
///\param time_s The time in seconds.
///\param leapyear The flag for leap year.
///\return The time in months.
///////////////////////////////////////////////////////////////////////////////
static int getCurrentMonth(double time_s, int leapyear){
	int month;
	month = getMonth (time_s, leapyear);
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

	num_days = getNumDays (month, leapyear);
	if ((time_s == 0 ) || (int)(time_s/86400)< 1){
		printf ("Time set is smaller than minimun allowed (1 day). Day will be set to 1!\n");
		day = 1;
	}

	else{
		if(!(leapyear)) {
			if (modulus(time_s, tmp) == 0) {
				day = 31;
			}
			else{
				if (time_s > tmp) 
				{
					printf ("Time set is larger than maximum allowed (365 days). Day will be set to 31!\n");
					day = 31;
				}
				else
				{	
					day = (int)(time_s/86400) - num_days;
				}
			}
		}
		else{
			if (modulus(time_s, tmpp) == 0) {
				day = 31;
			}
			else{
				if (time_s > tmpp) 
				{
					printf ("Time set is larger than maximum allowed (366 days). Day will be set to 31!\n");
					day = 31;
				}
				else {
					day = (int)(time_s/86400) - num_days;
				}
			}
		}
	}
	return day;
}


///////////////////////////////////////////////////////////////////////////////
/// This function reads the weather file and determines if leapyear or not.
///
///\param resources_p The path to resources folder
///////////////////////////////////////////////////////////////////////////////
static int getLeapYear(char * resources_p)
{
	char *runweafile;
	struct stat st;
	FILE *fp1;
	FILE *fp2;
	int leapyear = 0;
	int i;

	int found = 0;
	int ncount = 0;
	char *weafile;
	char leapIndic [2*MAXBUFFSIZE] = {0};
	char low_case_wea[2*MAXBUFFSIZE] = {0};
	char wea[2*MAXBUFFSIZE] = {0};
	const char* WEATHER = "HOLIDAYS/DAYLIGHT";
	if (stat(FRUNWEAFILE, &st)>=0)
	{
		fp1 = fopen(FRUNWEAFILE, "r");
		// read input file
		while(fgets(wea, sizeof wea, fp1) != NULL) {
			//remove space and make upper case
			remSpaces_makeUpper(wea);
			if((strstr(wea, WEATHER)) != NULL) {
				for(i=0; i<strlen(wea); i++)
				{
					if (wea[i] != ',' && found != 1)
					{
						ncount++;
						continue;
					}
					else if (wea [i] == ',')
					{
						found = 1;
						ncount++;
						continue;
					}
					else
					{
						if (wea[i] != ',')
						{
							leapIndic[i-ncount] = wea [i];
						}
						else if (wea[i] == ',')
						{
							break;
						}
						else{
							continue;
						}
					}
				}
				if((strstr(leapIndic, "YES")) != NULL)
				{
					leapyear = 1;
				}
			}
		}
		fclose (fp1);
	}
	else
	{
		printfDebug("Get weather file from resource folder %s!\n", resources_p);
		weafile = findNameFile(resources_p, ".epw");
		if(weafile != NULL)
		{
			runweafile = (char*)(calloc(sizeof(char),strlen(weafile) + strlen (resources_p)+ 1));
			sprintf(runweafile, "%s%s", resources_p, weafile);

			fp1 = fopen(runweafile, "r");
			fp2 = fopen(FRUNWEAFILE, "w");

			// read input file
			while(fgets(wea, sizeof wea, fp1) != NULL) {
				strcpy (low_case_wea, wea );
				//remove space and make upper case
				remSpaces_makeUpper(wea);
				fprintf(fp2, "%s", low_case_wea);  
				if((strstr(wea, WEATHER)) != NULL) {
					for(i=0; i<strlen(wea); i++)
					{
						if (wea[i] != ',' && found != 1)
						{
							ncount++;
							continue;
						}
						else if (wea [i] == ',')
						{
							found = 1;
							ncount++;
							continue;
						}
						else
						{
							if (wea[i] != ',')
							{
								leapIndic[i-ncount] = wea [i];
							}
							else if (wea[i] == ',')
							{
								break;
							}
							else{
								continue;
							}

						}
					}
					if((strstr(leapIndic, "YES")) != NULL)
					{
						leapyear = 1;
					}
				}
			}
			fclose (fp1);
			fclose (fp2);
			// free weafile which was allocated in findNameFile
			free (weafile);
			free (runweafile);
		}
		else
		{
			printf ("There is not weather file in the resource folder!\n");
		}
	}
	return leapyear;
}

///////////////////////////////////////////////////////////////////////////////
/// This function gets the time step from the input file.
///
///\param str The input file
///////////////////////////////////////////////////////////////////////////////
static int getTimeStep (char *str)
{
	int i;
	int found = 0;
	int ncount = 0;
	char TSStr [4*MAXBUFFSIZE] = {0};
	replace_char_from_string ('\n', ' ', str);
	for(i=0; i<strlen(str); i++)
	{
		if (str[i] != ',' && found != 1)
		{
			ncount++;
			continue;
		}
		else if (str [i] == ',')
		{
			found = 1;
			ncount++;
			continue;
		}

		else
		{
			if (str[i] != ';')
			{
				TSStr[i-ncount] = str [i];
			}
			else
			{
				continue;
			}
		}
	}
	// convert the string to an integer
	return atoi (TSStr);
}

///////////////////////////////////////////////////////////////////////////////
/// This function creates the input file to be used for the run.
///
///\param t_start_FMU The FMU start time.
///\param t_stop_FMU The FMU stop time.
///\param t_start_FMU The FMU model ID.
///\param resources_p The path to resources folder
///////////////////////////////////////////////////////////////////////////////
int createRunInFile (fmiReal t_start_FMU, fmiReal t_end_FMU, fmiString modelID, char * resources_p)
{
	char *infile;
	char *runinfile;
	char *fruninfile;
	char temp[MAXBUFFSIZE] = {0};
	char low_case_temp[MAXBUFFSIZE] = {0};
	char tmpDayWeek[MAXBUFFSIZE] = {0};
	char NewDayOfWeek [2*MAXBUFFSIZE] = {0};
	char old_temp[4*MAXBUFFSIZE] = {0};
	char new_TS[4*MAXBUFFSIZE] = {0};
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	struct stat st;

	int TS;

	int val;
	int i;

	int NewBeginMonth;
	int NewBeginDayOfMonth;
	int NewEndMonth;
	int NewEndDayOfMonth;
	int leapyear = 0;
	int foundEndTS = 0;

	double t_start_IDF;
	char intTostr[10] = {0};

	const char* RUNPERIOD= "RUNPERIOD,";
	char* TIMESTEP= "TIMESTEP,";
	int find_result_RP = 0;
	int exit_loop;

	// check whether start and end make sense
	if (t_end_FMU <= t_start_FMU)
	{
		printf("End of the simulation should come after start of the simulation!\n");
		return 1;
	}
	// get input file from the folder
	printfDebug("Get input file from resource folder %s!\n", resources_p);
	infile = findNameFile(resources_p, ".idf");
	if(infile != NULL)
	{
		runinfile = (char*)(calloc(sizeof(char), strlen(infile) + strlen (resources_p)+ 1));
		fruninfile = (char*)(calloc(sizeof(char), strlen(modelID) + 10));
		sprintf(runinfile, "%s%s", resources_p, infile);
		sprintf(fruninfile, "%s%s", modelID, ".idf");
		// free infile which was allocated in findNameFile
		free (infile);
	}
	else
	{
		printf ("Input file not found!\n");
		return 1;
	}

	// get the leapyear value, 1 is leap year, 0 is not leapyear 
	leapyear = getLeapYear (resources_p);

	// open original input file
	if((fp1 = fopen(runinfile, "r")) == NULL) {
		printf ("Can't open file %s!\n", runinfile);
		return 1;
	}
	// open temporary input file
	if((fp2 = fopen(fruninfile, "w")) == NULL) {
		printf ("Can't open file %s!\n", fruninfile);
		return 1;
	}

	// read input file and write in TEMP file
	while(fgets(temp, sizeof temp, fp1) != NULL) {
		strcpy (low_case_temp, temp);
		remSpaces_makeUpper(temp);
		if ((temp[0] !='!') && (isEmptyLine(temp) == 0)){
			// This implementation assumes a RunPeriod which consits of several lines as in the IDD
			// If the the structure of the RunPeriod differs from that the code will not work
			if((strstr(temp, RUNPERIOD) != NULL && find_result_RP<1) || strstr(temp, RUNPERIOD) == NULL)
			{
				fprintf(fp2, "%s", low_case_temp);
			}
			// If time step is found, write to file
			if((strstr(temp, TIMESTEP)) != NULL && (temp[0] =='T')) {
				while (foundEndTS != 1){
					for(i=0; i<strlen(temp); i++)
					{
						if (temp[i]!='!')
						{
							new_TS[i] = temp[i];
						}
						else
						{
							break;
						}

						if (temp[i]!=';')
						{
							foundEndTS = 0;
						}
						else
						{
							foundEndTS = 1;
							break;

						}
					}
					// concatenate to create TimeStep string
					strcat (old_temp, new_TS);
					// empty string used
					memset(new_TS, 0, sizeof new_TS);

					if (foundEndTS != 1){
						if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
						fprintf(fp2, "%s", temp);

					}
				}
				// get time step
				TS = getTimeStep (old_temp);
				if (TS==0){
					printf("The time step found is %d.\n This shouldn't be. Please check the input file!\n", TS);
					return 1;
				}

				// write timestep in file
				if(stat(FTIMESTEP, &st)>=0){
					printf ("The file %s exists and will be used to determine the time step for the simulation\n", FTIMESTEP);
				}
				else
				{
					if((fp3 = fopen(FTIMESTEP, "w")) == NULL){
						printf("Can't open and write timestep!\n");
						return 1;  
					}
					else
					{   
						fprintf(fp3, "%d", TS);
						fclose (fp3);
					}
				}
			}
			// rewrite the runperiod
			// This implementation assumes a RunPeriod which consits of several comma separated lines as in the IDD
			// If the the structure of the RunPeriod differs from that the code will not write the values 
			// found at the appropriate places. Alternative could be to ship an IDD to determine based on
			// the IDD the current syntax of the IDF file.
			if((strstr(temp, RUNPERIOD)) != NULL) {
				find_result_RP++;
				if (find_result_RP <=1){
					//skip first line
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					fprintf(fp2, "%s", temp);
					//write the Begin Month
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					printDataForDate(temp, BEGINMONTH);

					NewBeginMonth = getCurrentMonth (t_start_FMU, leapyear);
					// check begin month
					if((NewBeginMonth < 0) || (NewBeginMonth > 12)) {
						printf("Begin Month cannot be negativ or greater than 12!\n");
						return 1;  
					}

					sprintf(intTostr, "%d,\n", NewBeginMonth);
					fprintf(fp2, "%s", intTostr);

					//write the Begin Day of Month
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					printDataForDate(temp, BEGINDAYMONTH);

					NewBeginDayOfMonth = getCurrentDay (t_start_FMU, NewBeginMonth, leapyear);
					// check begin day of month
					if((NewBeginDayOfMonth < 0) || (NewBeginDayOfMonth > 31)) {
						printf("Begin Day of Month cannot be negativ or greater than 31!\n");
						return 1; 
					}
					sprintf(intTostr, "%d,\n", NewBeginDayOfMonth);
					fprintf(fp2, "%s", intTostr);

					//write the End Month
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					printDataForDate(temp, ENDMONTH);

					NewEndMonth = getCurrentMonth (t_end_FMU, leapyear);
					// check end month
					if((NewEndMonth < 0) || (NewEndMonth > 12)) {
						printf("End Month cannot be negativ or greater than 12!\n");
						return 1;  
					}
					sprintf(intTostr, "%d,\n", NewEndMonth);
					fprintf(fp2, "%s", intTostr);
					//write the End Day of Month
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					printDataForDate(temp, ENDDAYMONTH);

					NewEndDayOfMonth = getCurrentDay (t_end_FMU, NewEndMonth, leapyear);
					// check end day of month
					if((NewEndDayOfMonth < 0) || (NewEndDayOfMonth > 31)) {
						printf("End Day of Month cannot be negativ or greater than 31!\n");
						return 1;  
					}
					sprintf(intTostr, "%d,\n", NewEndDayOfMonth);
					fprintf(fp2, "%s", intTostr);
					//write the Day of Week
					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL);
					printDataForDate(temp, OLDDAYWEEK);

					//convert start and end time obtained from the master algorithm
					printDebug("Get simulation time in IDF!\n");
					t_start_IDF = getSimTimeSeconds( getNumValue (BEGINDAYMONTH), getNumValue (BEGINMONTH), leapyear);

					// get the current day of the week
					printDebug("Get current day of week!\n");
					val = getCurrentDayOfWeek (t_start_IDF, t_start_FMU, OLDDAYWEEK, NEWDAYWEEK);  

					// write new day of week file in another file
					if((fp3 = fopen(NEWDAYWEEK, "r")) == NULL) {
						printf ("File with new day of week not found!\n");
					}
					else
					{
						if(fgets(tmpDayWeek, MAXBUFFSIZE, fp3) != NULL);
						sprintf(NewDayOfWeek, "%s,\n", tmpDayWeek);
					}
					fclose (fp3);

					if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
					fprintf(fp2, "%s", NewDayOfWeek);
				}
				// delete run period, only one is allowed
				else
				{   
					int j = 0;
					exit_loop = 0;
					if(fgets(temp, sizeof temp, fp1) != NULL);
					while((exit_loop!=1))
					{
						if(temp[j]==',') {
							strcpy (temp, " " );
							fprintf(fp2, "%s", temp);  
							if(fgets(temp, sizeof temp, fp1) != NULL);
							exit_loop = 0;
							j = 0;
						}
						else if ((temp[j] == ';')){
							strcpy (temp, " " );
							fprintf(fp2, "%s", temp);  
							exit_loop = 1;
						}
						j = j+1;
					}

				}
			}

		}
	}

	if(find_result_RP == 0) {
		printf("Could not find a runperiod in input file!\n");
		return 1;
	}
	if(find_result_RP > 1) {
		// open temporary input file
		if((fp4 = fopen(LOG, "w")) == NULL) {
			printf("Can't open log file!\n");
			return 1; 
		}
		else
		{
			fprintf(fp4, "More than one RunPeriod found. %d deleted!\n",find_result_RP-1);
			fclose (fp4);
		}
	}
	//Close the file if still open.
	fclose(fp1);
	fclose(fp2);
	//deallocate runinfile
	free (runinfile);
	//deallocate fruninfile
	free (fruninfile);
	return 0;
}

//int main ()
//{
//	createRunInFile (0, 86400, "test", "c:\\temp\\");
//}

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
