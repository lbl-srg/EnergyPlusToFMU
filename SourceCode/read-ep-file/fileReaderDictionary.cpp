//--- File reader for EnergyPlus input data dictionary (IDD) file.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
#include <string>
using std::string;

#include <sstream>

#include "fileReaderDictionary.h"

#include "../utility/string-help.h"


//--- Preprocessor definitions.


//--- Types.


//--- Global variables.
static const std::string IDD_DELIMITERS = ",;";

// Note distinction, here, between {IDD_COMMENT_CHARS_TOKEN} and
// {IDD_COMMENT_CHARS_COMMENT}.  This preserves behavior of original code.
// However, is it sure that a token can't be delimited by '\\'?
static const std::string IDD_COMMENT_CHARS_TOKEN = "!";
static const std::string IDD_COMMENT_CHARS_COMMENT = "!\\";


//--- File-scope fcn prototypes.


//--- Functions.


///////////////////////////////////////////////////////
bool fileReaderDictionary::getKeywordAndDescriptor(string& keyword, string& desc){
  bool onKeyword = 1;
  char ch;
  //
  do{
    int lineNo;
    skipComment(IDD_COMMENT_CHARS_COMMENT, lineNo);
    if( onKeyword ){
      getToken(IDD_DELIMITERS, IDD_COMMENT_CHARS_TOKEN, keyword);
      capitalize(keyword);
      onKeyword = 0;
    }
    else{
      string s;
      getToken(IDD_DELIMITERS, IDD_COMMENT_CHARS_TOKEN, s);
      if( ! containsChar("AN;", s[0]) ){
        std::ostringstream os;
        os << "For IDD keyword " << keyword
           << ", expecting 'A' (alpha), 'N' (numeric), or ';', got '"
           << s[0] << "'.";
        fileReader::reportError(os);
        return false; // error
      }
      if (s[0] != ';')
        desc += s[0];
    }
    skipComment(IDD_COMMENT_CHARS_COMMENT, lineNo);
    if (isEOF())
      return true;
    ch = getChar();
    if( ! containsChar(IDD_DELIMITERS, ch) ){
      std::ostringstream os;
      os << "Expecting a delimiter '" << IDD_DELIMITERS << "', got '" << ch << "'.";
      fileReader::reportError(os);
      return false; // error
    }
  } while( ! ( ch==';' || isEOF() ) );
  //
  return true;
}  // End method fileReaderDictionary::getKeywordAndDescriptor().


///////////////////////////////////////////////////////
void fileReaderDictionary::getMap(iddMap& idd){
  //
  while (! isEOF() ){
    string kw = "";
    string desc = "";
    if( ! getKeywordAndDescriptor(kw, desc) ){
      std::ostringstream os;
      os << "fileReaderDictionary::getMap(): Exit with error.";
      fileReader::reportError(os);
      exit(1);
    }
    // Store new keyword and descriptor.
    // hoho dml  Note no check whether overwriting an existing entry.
    idd[kw] = desc;
    int lineNo;
    skipComment(IDD_COMMENT_CHARS_COMMENT, lineNo);
  }// end while
}
