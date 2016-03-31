//--- Virtual file reader for input parameter and weather data.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.

#include <cstdlib>

#include <iostream>
using std::string;
using std::cerr;
using std::cout;
using std::endl;

#include <sstream>

#include "fileReader.h"


///////////////////////////////////////////////////////
fileReader::fileReader(const string& fname){
  fileName = fname;
  lineNumber = 0;
  externalErrorFcn = 0;
}

///////////////////////////////////////////////////////
void fileReader::open(){
  fileStream.open(fileName.c_str(), std::ios::in);
  if( ! fileStream.is_open() ){
     std::ostringstream os;
     os << "Cannot open file";
     reportError(os);
     exit(1);
  }
  lineNumber = 1;
}


//--- Close the file.
//
void fileReader::close()
  {
  if( fileStream.is_open() )
    {
    fileStream.close();
    }
  lineNumber = 0;
  }  // End method fileReader::close().


//--- Attach an error-reporting function.
//
void fileReader::attachErrorFcn(void (*errFcn)(
  std::ostringstream& errorMessage, const std::string& fileName, int lineNo)){
  externalErrorFcn = errFcn;
}  // End method fileReader::attachErrorFcn().


//--- Return the next character.
//
char fileReader::getChar(void)
  {
  const int charAsInt = fileStream.get();
  if( (int)'\n' == charAsInt )
    {
    ++lineNumber;
    }
  else if( EOF == charAsInt )
    {
    return( '\0' );
    }
  //
  return( (char)charAsInt );
  }  // End method fileReader::getChar().


//--- Skip comments and spaces.
//
// hoho dml  Note could return EOF status.
//
void fileReader::skipComment(const string& commentSign, int& lineNo){
  char ch;
  //
  if( ! fileStream.eof() ){
    while( 1 ){
      // Here, assume next content on current line may be a comment.
      // Get first non-space character.
      skipSpace(lineNo);
      if( fileStream.eof() )
        break;
      fileStream.get(ch);
      // Check {ch}.
      if( fileStream.eof() )
        break;
      if( commentSign.find(ch) == std::string::npos ){
        // Not a comment line.
        fileStream.putback(ch);
        break;
      }
      // Here, {ch} starts a comment.
      //   Skip rest of line, then go back to check whether next line also
      // is a comment.
      skipLine(lineNo);
    }
  }
  //
}  // End method fileReader::skipComment().


//--- Get next token.
//
//   Note the token must be delimited by a character in {delimiters}.
//   Note a token can span more than one line, in which case the token includes
// the '\n' character.
//
// hoho dml  Note could return EOF status, or line number, or both (if encode
// line number as 0==(file unstarted) and -1==EOF.
// Alternately, could also code for illegal delimiter found.
// Alternately, could return length of token, to guard against getting a
// delimiter as first character.
// Minimally, could at least return a boolean "success" so that caller knows
// to check for a failure (hit illegal character, or zero-length token).
//
void fileReader::getToken(const string& delimiters, const string& illegalChars,
  string& token){
  //
  token = "";
  char ch;
  //
  if( ! fileStream.eof() ){
    while( 1 ){
      // Get next character (which may or may not be part of the token).
      fileStream.get(ch);
      // Reasons to end the token:
      // ** Hit EOF.
      // ** Hit a delimiter.
      // ** Hit a comment (which is an error).
      //   Note hitting EOL does not end a token.
      if( fileStream.eof() )
        break;
      if( '\n' == ch )
        ++lineNumber;
      if( delimiters.find(ch) != std::string::npos ){
        // Here, found delimiter.
        //   Put delimiter back before finish.
        fileStream.putback(ch);
        break;
      }
      if( illegalChars.find(ch) != std::string::npos ){
        // Error, illegal character.
        std::ostringstream os;
        os << "Encountered illegal character '" << ch << "' while reading a token.";
        reportError(os);
        exit(1);
      }
      // Here, still reading token.
      token += ch;
    }
  }
  //
  return;
}  // End method fileReader::getToken().

void fileReader::getToken(const string& delimiters, const string& illegalChars,
	string& token, string& tokenExt){
	//
	token = "";
	tokenExt = "";
	char ch;
	//
	if (!fileStream.eof()){
		while (1){
			// Get next character (which may or may not be part of the token).
			fileStream.get(ch);
			// Reasons to end the token:
			// ** Hit EOF.
			// ** Hit a delimiter.
			// ** Hit a comment (which is an error).
			//   Note hitting EOL does not end a token.
			if (fileStream.eof())
				break;
			if ('\n' == ch)
				++lineNumber;
			if (delimiters.find(ch) != std::string::npos){
				// Here, found delimiter.
				//   Put delimiter back before finish.
				tokenExt += ch;
				fileStream.putback(ch);
				break;
			}
			//if (illegalChars.find(ch) != std::string::npos){
			//	// Error, illegal character.
			//	std::ostringstream os;
			//	os << "Encountered illegal character '" << ch << "' while reading a token.";
			//	reportError(os);
			//	exit(1);
			//}
			// Here, still reading token.
			token += ch;
			tokenExt += ch;
		}
	}
	//
	return;
}  // End method fileReader::getToken().


///////////////////////////////////////////////////////
void fileReader::getLine(string& line, int& lineNo){
  getline(fileStream, line);
  lineNo = lineNumber++;
  return;
}


//--- Skip all spaces.
//
void fileReader::skipSpace(int& lineNo) {
  //
  if( ! fileStream.eof() ){
    while( 1 ){
      // Get next character.
      const char ch = getChar();
      if( fileStream.eof() )
        break;
      if( ! isspace(ch) ){
        // Return non-space to fileStream before finish.
        fileStream.putback(ch);
        break;
      // Here, still consuming spaces.
      }
    }
  }
  //
  lineNo = lineNumber;
  return;
}  // End method fileReader::skipSpace().


///////////////////////////////////////////////////////
void fileReader::skipLine(int& lineNo){
  string dummy;
  getline(fileStream, dummy);
  lineNo = ++lineNumber;
  return;
}

///////////////////////////////////////////////////////
void fileReader::skipLine(const int noOfLines, int& lineNo){
  string dummy;
  for (int i = 0; i < noOfLines; i++){
    getline(fileStream, dummy);
    lineNo = ++lineNumber;
  }
  return;
}


///////////////////////////////////////////////////////
bool fileReader::moveForward(int skipCharCt){
  while( skipCharCt > 0 ){
    const int charAsInt = fileStream.get();
    --skipCharCt;
    if( (int)'\n' == charAsInt ){
      ++lineNumber;
    }
    else if( EOF == charAsInt ){
      // Hit EOF before ate \c skipCharCt characters.
      return false;
    }
  }
  // Here, ate {skipCharCt} characters, without hitting EOF.
  return true;
}  // End method fileReader::moveForward().


//--- Report an error.
//
void fileReader::reportError(std::ostringstream& errorMessage){
  //
  // Call user-supplied error fcn if available.
  if( externalErrorFcn ){
    (*externalErrorFcn)(errorMessage, fileName, lineNumber);
  }
  else{
    // Here, no user-supplied error fcn.
    //   Note flush both {cout} and {cerr}, to avoid overlapped writes.
    cout.flush();
    cerr << "=== Input Error ===" << endl
      << "File " << fileName << ", line " << lineNumber << ": " << errorMessage.str() << endl << endl;
    cerr.flush();
  }
}  // End method fileReader::reportError().
