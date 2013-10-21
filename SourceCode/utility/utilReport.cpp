//--- Reporting functions.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <ctime>

#include <string>
using std::string;

#include <fstream>

#include <iostream>
using std::cerr;

#include <sstream>
using std::endl;

#include "utilReport.h"


//--- Preprocessor definitions.
const std::string LOGFILENAME = "output.log";


//--- Functions.


void reportProgramError(const string& functionName, const string& errorMessage){
  std::ostringstream os;
  os << errorMessage;
  reportProgramError(functionName, os);
}

void reportProgramError(const string& functionName, std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Program Error ===" << endl << 
    functionName << endl << errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportRuntimeError(const string& functionName, std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Run-Time Error ===" << endl << 
    functionName << endl << errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportRuntimeError(std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Run-Time Error ===" << endl << 
    errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportRuntimeWarning(std::ostringstream& warningMessage){
  std::ostringstream os;
  os << "=== Run-Time Warning ===" << endl << 
    warningMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportRuntimeInfo(std::ostringstream& infoMessage){
  std::ostringstream os;
  os << "=== Run-Time Info ===" << endl << 
    infoMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportError(const string& errorMessage){
  std::ostringstream os;
  os << errorMessage;
  reportError(os);
}

void reportError(std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Error ===" << endl << errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportInputError(std::ostringstream& errorMessage, 
    const string& fileName, 
    const int lineNo){
  std::ostringstream os;
  os << "=== Input Error ===" << endl
     << "File " << fileName << ", line " << lineNo << ": " << errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportInputError(const string& functionName, std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Input error ===" << endl 
     << "Detected in method '" << functionName << "'" << endl 
     << errorMessage.str() << endl << endl;
  flushLogStream(os);
}

void reportInputError(std::ostringstream& errorMessage){
  std::ostringstream os;
  os << "=== Input error ===" << endl 
     << errorMessage.str() << endl << endl;
  flushLogStream(os);
}


void flushLogStream(std::ostringstream& errorMessage){
  std::ofstream ofs(LOGFILENAME.c_str(), std::ios::app);
  if (!ofs.is_open())
    cerr << "Cannot open log file '" << LOGFILENAME << "'." << endl;
  else{
    ofs << errorMessage.str();
    ofs.close();
  }
  cerr << errorMessage.str();
}

///////////////////////////////////////////////////////
void writeLogHeader(){
  std::ofstream ofs(LOGFILENAME.c_str());
  if (!ofs.is_open()){
    cerr << "Cannot open log file '" << LOGFILENAME << "'." << endl;
  }
  else{
    time_t rawtime;
    struct tm * timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    ofs << " -- Log file." << endl
        << "% File generated on " << asctime (timeinfo);
  }
}
