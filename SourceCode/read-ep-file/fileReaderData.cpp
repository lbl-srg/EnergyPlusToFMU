//--- File reader for EnergyPlus input data file (IDF).


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.

#include <string>
using std::string;

#include <sstream>


#include "fileReaderData.h"

#include "ep-idd-map.h"
#include "../utility/string-help.h"


//--- Preprocessor definitions.


//--- Types.


//--- Global constants.
//
//   Note basic asymmetry in class design: for some methods, user has to pass
// delimiters in; other methods hard-code the delimiters.
//
const std::string IDF_DELIMITERS_ENTRY   = ",";   // {entryDel}
const std::string IDF_DELIMITERS_SECTION = ";";   // {sectionDel}
const std::string IDF_DELIMITERS_ALL     = ",;";  // {delimiter}
const std::string IDF_COMMENT_CHARS = "!";


//--- File-scope fcn prototypes.


//--- Functions.


//--- Constructor.
//
fileReaderData::fileReaderData(const std::string& fname,
  const string& entryDel, const std::string& sectionDel) :
  fileReader(fname) {
  //
  entryDelimiter = entryDel;
  sectionDelimiter = sectionDel;
  delimiter = entryDelimiter + sectionDelimiter;
}


//--- Read string and double values from the IDF file.
//
//   Follow pattern dictated by the descriptor {desc}.  Presumably {desc} comes
// from an input data dictionary (IDD), although this is not required.
//
// hoho dml  Consider returning number of entries read (negative value on failure).
//
bool fileReaderData::getValues(const string& desc, vString& strVals, vDouble& dblVals){
  //
  int descIdx = -1;
  //
  // Initialize return values.
  strVals.clear();
  dblVals.clear();
  //
  while( desc[++descIdx] != '\0' ){
    // Here, assume ready to read an entry of type desc[descIdx] from the IDF file.
    int lineNo;
    // Get next entry.
    skipComment(IDF_COMMENT_CHARS, lineNo);
    string entry;
    getToken(delimiter, IDF_COMMENT_CHARS, entry);
    // Consume delimiter that marked end of {entry} in IDF file.
    const char delimChar = getChar();
    // Parse the entry.
    trimEnd(entry);
    if( desc[descIdx] == 'A' ){
      // Here, {entry} should be a string.
      strVals.push_back(entry);
    }
    else if( desc[descIdx] == 'N' ){
      // Here, {entry} should be number.
      double temp;
      if( !strToDbl(entry, temp) ) {
        // Failed to read number.
        string markedDesc;
        std::ostringstream os;
        iddMap_markDescriptorIdx(desc, descIdx, markedDesc);
        if( errno != ERANGE )
          os << "Expected a number, received '" << entry;
        else if( temp == 0 )
          os << "Underflow, received '" << entry;
        else
          os << "Overflow, received '" << entry;
        os << "', while reading entry #" << descIdx+1 <<
          " according to dictionary descriptor '" << markedDesc << "' (at <>).";
        reportError(os);
        return false;
      }
      else{
        // Here, got a good number.
        // hoho dml  No need for "else" clause, since "if" block returns.
        dblVals.push_back(temp);
      }
    }
    else{
      // Here, {desc} contains other marker than 'A' or 'N'.
      string markedDesc;
      std::ostringstream os;
      iddMap_markDescriptorIdx(desc, descIdx, markedDesc);
      os << "Expecting 'A' or 'N' in dictionary descriptor '" << markedDesc <<
        "', got '" << desc[descIdx] <<
        "', at entry #" << descIdx+1 << " (at <>).";
      reportError(os);
      return false;
    }
    // Here, done reading and storing a token from the IDF file.
    // Check delimiter.
    //   Cases of interest:
    // ** This should be last entry in the data section, according to {desc}.
    // Therefore expect end-of-section delimiter.
    // ** Got end-of-section delimiter, even though {desc} declares there are
    // more entries.  This is not an error, since some input sections have a
    // variable count of values.
    if( containsChar(sectionDelimiter, delimChar) ){
      // hoho dml  If hard-code {sectionDelimiter}, since know it's a single
      // character, could make test simpler above.
      // Here, IDF file has a section delimiter.  Note it's OK if {desc} says
      // the section should have more entries-- don't flag this as an error.
      break;
    }
    else if( desc[descIdx+1] == '\0' ){
      // Here, expected this was the last entry in the section, but
      // did not get section delimiter.
      std::ostringstream os;
      os << "Expected end-of-section delimiter '" << sectionDelimiter <<
        "' after entry #" << descIdx+1 <<
        ", since reached end of dictionary descriptor '" << desc << "'.";
      reportError(os);
      return false;
    }
  } // end while
  return true;
}  // End method fileReaderData::getValues().


//--- Skip a section in the IDF file.
//
bool fileReaderData::skipSection(void)
  {
  //
  // Read until hit a section delimiter, or hit end-of-file.
  //   Assume not currently in a comment.
  if( ! isEOF() )
    {
    while( 1 )
      {
      // Get next character.
      const char ch = getChar();
      if( containsChar(sectionDelimiter,ch) || isEOF() )
        {
        // Here, reached end-of-section or end-of-file.
        // hoho dml  If hard-code {sectionDelimiter}, since know it's a single
        // character, could make test simpler above.
        break;
        }
      }
    }
  //
  return( isEOF() );
  }  // End method fileReaderData::skipSection().
