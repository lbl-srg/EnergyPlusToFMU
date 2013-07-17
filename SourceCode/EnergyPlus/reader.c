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

#ifdef _MSC_VER
#include <windows.h>
#include "dirent_win.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#endif

#include "reader.h"
#include "util.h"

// constant parameters for filenames
const char* TEMP1         = "tmp1.idf";
const char* TEMP2         = "tmp2.idf";
const char* LOG           = "log.txt";
const char* BEGINDAYMONTH = "beginDayMonth.txt";
const char* ENDDAYMONTH   = "endDayMonth.txt";
const char* BEGINMONTH    = "beginMonth.txt";
const char* ENDMONTH      = "endMonth.txt";
const char* NEWDAYWEEK    = "newDayWeek.txt";
const char* OLDDAYWEEK    = "oldDayWeek.txt";

#define MAXBUFFSIZE 512

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
	free (infile);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// This function deletes temporary create files. 
///////////////////////////////////////////////////////////////////////////////
void findFileDelete()      
{
	struct stat stat_p;

	// delete temporary files
	if (!stat (TEMP1, &stat_p))
	{
		remove(TEMP1);
	}
	if (!stat (TEMP2, &stat_p))
	{
		remove(TEMP2);
	}
	if (!stat(BEGINDAYMONTH, &stat_p))
	{
		remove(BEGINDAYMONTH);
	}
	if (!stat (BEGINMONTH, &stat_p))
	{
		remove(BEGINMONTH);
	}
	if (!stat (ENDDAYMONTH, &stat_p))
	{
		remove(ENDDAYMONTH);
	}
	if (!stat (ENDMONTH, &stat_p))
	{
		remove(ENDMONTH);
	}

	if (!stat(NEWDAYWEEK, &stat_p))
	{
		remove(NEWDAYWEEK);
	}

	if (!stat (OLDDAYWEEK, &stat_p))
	{
		remove(OLDDAYWEEK);
	}
	if (!stat (VARCFG, &stat_p))
	{
		remove(VARCFG);
	}

	if (!stat (SOCKCFG, &stat_p))
	{
		remove(SOCKCFG);
	}

	if (!stat (EPBAT, &stat_p))
	{
		remove(EPBAT);
	}
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
	int result;
	char *infile;
	char *runinfile;
	char *fruninfile;

	infile = findNameFile(resources_p, ".idf");
	if(infile != NULL)
	{
		runinfile = (char*)(calloc(sizeof(char), strlen(infile) + strlen (resources_p)+ 1));
		fruninfile = (char*)(calloc(sizeof(char), strlen(modelID) + 10));
		sprintf(runinfile, "%s%s", resources_p, infile);
		sprintf(fruninfile, "%s%s", modelID, ".idf");
	}
	else
	{
		printf ("Input file not found!\n");
		return 1;
	}
	printDebug("Start cleaning input file if duplicate runperiods!\n");
	result = writeCleanInFile (runinfile, TEMP1, TEMP2, LOG);
	printDebug("Finish cleaning input file if duplicate runperiods!\n");
	if (result != 0)
	{
		printf ("Could not clean the input idf file!\n");
		return 1;
	}
	printDebug("Start writing new runperiod according to FMU parameters!\n");
	result = writeNewRunperiod(TEMP2);
	printDebug("Finish writing new runperiod according to FMU parameters!\n");
	if (result != 0)
	{
		printf ("Could not write new run period!\n");
		return 1;
	}
	printDebug("Start updating runperiod in the input file!\n");

	result = updateRunperiodInFile(TEMP2, fruninfile, t_start_FMU, t_end_FMU, resources_p);
	printDebug("Finish updating runperiod in the input file");
	if (result != 0)
	{
		printf ("Could not update new run period!\n");
		return 1;
	}
	//deallocate runinfile
	free (runinfile);
	//deallocate fruninfile
	free (fruninfile);
	return 0;
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
static void remSpaces_makeUpper(char *infile){
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
/// This function reads a file extracts relevant information for date and print 
/// the information in a file.
///
///\param temp The string.
///\param fname The filename to print.
///\param fname The index of string to be printed.
///////////////////////////////////////////////////////////////////////////////
static void printDataForDate (char *temp, const char* fname, int index){
	FILE *fp;
	int i;
	char *result[sizeof(strtok(temp, ","))];
	result[0] = strtok( temp, "," );

	for(i=1; i<sizeof(strtok(temp, " ")); i++)
	{
		result[i] = strtok (NULL, ";");
	}

	// write timestep in file
	if((fp = fopen(fname, "w")) == NULL) {
		printf("Can't open file!\n");
		exit(42);  // STL error code: File not open.
	}
	else
	{   
		remSpaces_makeUpper(result[index]);
		fprintf(fp, "%s", result[index]);
		fclose (fp);
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
	char buff[3];
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
	const char *fname, const char*fname1){
		FILE *fp1;
		FILE *fp2;
		char day_of_week [MAXBUFFSIZE];
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
		if(fgets( day_of_week, sizeof day_of_week, fp1) != NULL);
		fclose(fp1);

		t_start_idf = t_start_idf + 86400;
		// deternmine th difference between start time in idf and start time in fmu
		//modDat = (t_start_fmu  - t_start_idf)%7;
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

		else 
		{
			change = 0;
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
		
		else
		{
			// write the new day of week
			fprintf(fp2, "%s", day_of_week);
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
/// This function reads a file, remove the blanks line and write the new lines
/// into a new file.
///
///\param fname1 The file to read. 
///\param fname2 The file to write. 

///////////////////////////////////////////////////////////////////////////////
static int cleanBlanksInFile(const char *fname1, const char *fname2) {
	FILE *fp1;
	FILE *fp2;
	char temp[MAXBUFFSIZE];
	char low_case_temp[MAXBUFFSIZE];

	printDebug("Remove blanks from input file!\n");
	// open original input file
	if((fp1 = fopen(fname1, "r")) == NULL) {
		printf ("Can't open file %s!\n", fname1);
		return(1);
	}
	// open temporary input file
	if((fp2 = fopen(fname2, "w")) == NULL) {
		printf ("Can't open file %s!\n", fname2);
		return(1);
	}

	// read input file
	while(fgets(temp, sizeof temp, fp1) != NULL) {

		strcpy (low_case_temp, temp);
		remSpaces_makeUpper(temp);
		if ((temp[0] !='!') && (isEmptyLine(temp) == 0)){

			fprintf(fp2, "%s", low_case_temp);  
		}
	}

	//Close the file if still open.
	if(fp1) {
		fclose(fp1);
	}
	if(fp2) {
		fclose(fp2);
	}
	return(0);
}

///////////////////////////////////////////////////////////////////////////////
/// This function reads a the temporary input file, remove all duplicates
/// runperiod, write the new file into another one and writes a log file 
/// if duplicates are found.
///
///\param fname1 The file to read. 
///\param fname2 The file without blanks. 
///\param fname3 The file without duplicates runperiod.
///\param fname4 The log file.
///\return 0 if no error occurred.

///////////////////////////////////////////////////////////////////////////////
int writeCleanInFile(char *fname1, const char *fname2, const char *fname3, const char *fname4) {
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	//int line_num = 1;  // Updated but never used.
	int find_result_RP = 0;
	//int find_result_TS = 0;
	char temp[MAXBUFFSIZE];
	char low_case_temp[MAXBUFFSIZE];
	const char* RUNPERIOD= "RUNPERIOD,";
	int exit_loop = 0;

	int blank = 0;

	blank = cleanBlanksInFile (fname1, fname2);
	// open original input file
	if(blank !=0) {
		printf ("Can't clean input file!\n");
		return(1);
	}

	// open original input file
	if((fp2 = fopen(fname2, "r")) == NULL) {
		printf ("Can't open file %s!\n", fname2);
		return(1);
	}
	// open temporary input file
	if((fp3 = fopen(fname3, "w")) == NULL) {
		printf ("Can't open file %s!\n", fname3);
		return(1);
	}
	// read input file
	while(fgets(temp, sizeof temp, fp2) != NULL) {
		strcpy (low_case_temp, temp );
		//remove space and make upper case
		remSpaces_makeUpper(temp);
		if((strstr(temp, RUNPERIOD)) != NULL) {
			find_result_RP++;
			// delete run period, only one is allowed
			if (find_result_RP > 1)
			{   
				int j = 0;
				exit_loop = 0;
				if(fgets(temp, sizeof temp, fp2) != NULL);
				while((exit_loop!=1))

				{
					if(temp[j]==',') {
						strcpy (low_case_temp, " " );
						fprintf(fp3, "%s", low_case_temp);  
						if(fgets(temp, sizeof temp, fp2) != NULL);
						exit_loop = 0;
						j = 0;
					}
					else if ((temp[j] == ';')){
						strcpy (low_case_temp, " " );
						fprintf(fp3, "%s", low_case_temp);  
						exit_loop = 1;
					}
					j = j+1;
				}

			}
		}

		fprintf(fp3, "%s", low_case_temp);  

	}

	if(find_result_RP == 0) {
		printf("Could not find a runperiod in input file!\n");
		return 1;
	}

	if(find_result_RP > 1) {
		// open temporary input file
		if((fp4 = fopen(fname4, "w")) == NULL) {
			printf("Can't open log file!\n");
			exit(42);  // STL error code: File not open.
		}
		else
		{
			fprintf(fp4, "More than one RunPeriod found. %d deleted!\n",find_result_RP-1);
			fclose (fp4);
		}
	}
	//Close the file if still open.
	if(fp2) {
		fclose(fp2);
	}
	if(fp3) {
		fclose(fp3);
	}
	return(0);
}


///////////////////////////////////////////////////////////////////////////////
/// This function extracts runperiod from file and print in a file
/// 
///
///\param fname The file to read.
///\return 0 if no error occurred.
///////////////////////////////////////////////////////////////////////////////
int writeNewRunperiod(const char *fname) {
	FILE *fp;
	FILE *fp1;
	int find_result_RP = 0;
	char temp[MAXBUFFSIZE];
	char low_case_temp[MAXBUFFSIZE];
	const char* RUNPERIOD= "RUNPERIOD,";
	char* TIMESTEP= "TIMESTEP,";
	int i = 0;
	// open original input file
	if((fp = fopen(fname, "r")) == NULL) {
		printf ("Can't open file %s!\n", fname);
		return(1);
	}

	// read input file
	while(fgets(temp, MAXBUFFSIZE, fp) != NULL) {
		if (temp[0] !='!' ){
			strcpy (low_case_temp, temp );
			//remove space and make upper case
			remSpaces_makeUpper(temp);
			find_result_RP++;
			if((strstr(temp, RUNPERIOD)) != NULL) {
				//skip first line
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL); 
				//write the Begin Month
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL);
				printDataForDate(temp, BEGINMONTH, 0);
				//write the Begin Day of Month
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL);
				printDataForDate(temp, BEGINDAYMONTH, 0);
				//write the End Month
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL);
				printDataForDate(temp, ENDMONTH, 0);
				//write the End Day of Month
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL);
				printDataForDate(temp, ENDDAYMONTH, 0);
				//write the Day of Week
				if(fgets(temp, MAXBUFFSIZE, fp) != NULL);
				printDataForDate(temp, OLDDAYWEEK, 0);
			}

			if((strstr(temp, TIMESTEP)) != NULL) {
				char *timeStep[sizeof(strtok(temp, ","))];

				timeStep[0] = strtok( temp, "," );
				for(i=1; i< sizeof(strtok(temp, " ")); i++)
				{
					timeStep[i] = strtok (NULL, ";");
				}
				// write timestep in file
				if((fp1 = fopen(FTIMESTEP, "w")) == NULL){
					printf("Can't open and write timestep!\n");
					exit(42);  // STL error code: File not open.
				}
				else
				{   
					remSpaces_makeUpper(timeStep[1]);
					fprintf(fp1, "%s", timeStep[1]);
					fclose (fp1);
				}

			}

		}
	}

	if(find_result_RP == 0) {
		printf("Could not find a runperiod in input file!\n");
		return 1;
	}
	return(0);
}

///////////////////////////////////////////////////////////////////////////////
/// This function writes a new runperiod in the file used for the simulation
/// 
///
///\param fname1 The file to read. 
///\param fname2 The file to write.
///\return 0 if no error occurred.
///////////////////////////////////////////////////////////////////////////////
int updateRunperiodInFile(const char *fname1, char *fname2, double t_start_FMU, double t_end_FMU, char * resources_p ) {
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	FILE *fp5;
	char *weafile;
	char temp[MAXBUFFSIZE];
	char low_case_temp[2* MAXBUFFSIZE];
	char low_case_wea[2*MAXBUFFSIZE];
	char wea[2*MAXBUFFSIZE];
	char NewDayOfWeek [2*MAXBUFFSIZE];

	const char* RUNPERIOD= "RUNPERIOD,";
	const char* WEATHER = "HOLIDAYS/DAYLIGHT";

	int val;
	int i;
	int OldBeginMonth;
	int OldBeginDayOfMonth;
	int OldEndMonth;
	int OldEndDayOfMonth;

	int NewBeginMonth;
	int NewBeginDayOfMonth;
	int NewEndMonth;
	int NewEndDayOfMonth;
	int leapyear = 0;

	double t_start_IDF;

	char intTostr[10];
	char *runweafile;

	// open original input file
	if((fp1 = fopen(fname1, "r")) == NULL) {
		return(1);
	}
	// open temporary input file
	if((fp2 = fopen(fname2, "w")) == NULL) {
		printf ("Can't open file to update RunPeriod!\n" );
		return(1);
	}
	printDebug("Get weather file name from the resources directory!\n");
	weafile = findNameFile(resources_p, ".epw");
	if(weafile != NULL)
	{
		runweafile = (char*)(calloc(sizeof(char),strlen(weafile) + strlen (resources_p)+ 1));
		sprintf(runweafile, "%s%s", resources_p, weafile);

		fp4 = fopen(runweafile, "r");
		fp5 = fopen(FRUNWEAFILE, "w");

		// read input file
		while(fgets(wea, sizeof wea, fp4) != NULL) {
			strcpy (low_case_wea, wea );
			//remove space and make upper case
			remSpaces_makeUpper(wea);
			fprintf(fp5, "%s", low_case_wea);  
			if((strstr(wea, WEATHER)) != NULL) {
				char *leapyear_ch[sizeof(strtok(wea, ","))];
				leapyear_ch[0] = strtok( wea, "," );
				for(i=1; i< sizeof(strtok(wea, " ")); i++)
				{
					leapyear_ch[i] = strtok (NULL, ",");
				}
				remSpaces_makeUpper(leapyear_ch[1]);
				if (strcmp (leapyear_ch[1], "YES")== 0)
				{
					leapyear = 1;
				}
			}
		}
		fclose (fp4);
		fclose (fp5);
	}
	else
	{
		printf ("Can't open weather file!\n");
		runweafile = NULL;
	}
	// get the numerical values for old runperiod
	printDebug("Get current month, day for runperiod based on IDF!\n");
	OldBeginMonth = getNumValue (BEGINMONTH);
	OldBeginDayOfMonth = getNumValue (BEGINDAYMONTH);
	OldEndMonth = getNumValue (ENDMONTH);
	OldEndDayOfMonth = getNumValue (ENDDAYMONTH);

	//convert start and end time obtained from the master algorithm
	printDebug("Get simulation time in IDF!\n");
	t_start_IDF = getSimTimeSeconds(OldBeginDayOfMonth, OldBeginMonth, leapyear);

	// get the current day of the week
	printDebug("Get current day of week!\n");
	val = getCurrentDayOfWeek (t_start_IDF, t_start_FMU, OLDDAYWEEK, NEWDAYWEEK);  

	// rewrite weather file in another file
	if((fp3 = fopen(NEWDAYWEEK, "r")) == NULL) {
		printf ("File with new day of week not found!\n");
	}
	else
	{
		if(fgets(temp, MAXBUFFSIZE, fp3) != NULL);
		sprintf(NewDayOfWeek, "%s,\n", temp);
	}
	fclose (fp3);

	// check whether start and end make sense
	if (t_end_FMU < t_start_FMU)
	{
		printf("End of the simulation should come after start of the simulation!\n");
		return 1;
	}
	// get the numerical values for new runperiod
	printDebug("Get current month, day for runperiod based on FMU parameters!\n");
	NewBeginMonth = getCurrentMonth (t_start_FMU, leapyear);
	// check begin month
	if((NewBeginMonth < 0) || (NewBeginMonth > 12)) {
		printf("Begin Month cannot be negativ or greater than 12!\n");
		exit(1);  
	}
	NewBeginDayOfMonth = getCurrentDay (t_start_FMU, NewBeginMonth, leapyear);
	// check begin day of month
	if((NewBeginDayOfMonth < 0) || (NewBeginDayOfMonth > 31)) {
		printf("Begin Day of Month cannot be negativ or greater than 31!\n");
		exit(1); 
	}

	
	NewEndMonth = getCurrentMonth (t_end_FMU, leapyear);
	// check end month
	if((NewEndMonth < 0) || (NewEndMonth > 12)) {
		printf("End Month cannot be negativ or greater than 12!\n");
		exit(1);  
	}

	NewEndDayOfMonth = getCurrentDay (t_end_FMU, NewEndMonth, leapyear);
	// check end day of month
	if((NewEndDayOfMonth < 0) || (NewEndDayOfMonth > 31)) {
		printf("End Day of Month cannot be negativ or greater than 31!\n");
		exit(1);  
	}

	// read input file
	while(fgets(temp, MAXBUFFSIZE, fp1) != NULL) {
		if (temp[0] !='!' ){
			strcpy (low_case_temp, temp );
			//remove space and make upper case
			remSpaces_makeUpper(temp);
			fprintf(fp2, "%s", low_case_temp);  
			if((strstr(temp, RUNPERIOD)) != NULL) {
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				fprintf(fp2, "%s", temp);
				//overwrite the Begin Month
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				sprintf(intTostr, "%d,\n", NewBeginMonth);
				fprintf(fp2, "%s", intTostr);

				//overwrite the Begin Day of Month
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				sprintf(intTostr, "%d,\n", NewBeginDayOfMonth);
				fprintf(fp2, "%s", intTostr);

				//overwrite the End Month
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				sprintf(intTostr, "%d,\n", NewEndMonth);
				fprintf(fp2, "%s", intTostr);

				//overwrite the End Day of Month
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				sprintf(intTostr, "%d,\n", NewEndDayOfMonth);
				fprintf(fp2, "%s", intTostr);

				//overwrite the End Day of Month
				if(fgets(temp, MAXBUFFSIZE, fp1) != NULL); 
				fprintf(fp2, "%s", NewDayOfWeek);
			}
		}
	}

	//Close the file if still open.
	if(fp1) {
		fclose(fp1);
	}
	if(fp2) {
		fclose(fp2);
	}
	free (runweafile);
	return(0);
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