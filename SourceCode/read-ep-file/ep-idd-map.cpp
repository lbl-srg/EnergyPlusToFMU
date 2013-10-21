//--- Work with EnergyPlus input data dictionary (IDD) key-descriptor pairs.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include "ep-idd-map.h"


//--- Preprocessor definitions.


//--- Types.


//--- Global variables.


//--- File-scope fcn prototypes.


//--- Functions.


//--- Get descriptor for a given keyword.
//
bool iddMap_getDescriptor(const iddMap& idd, const string& key, string& desc){
  //
  const iddMap::const_iterator it = idd.find(key);
  //
  if( idd.end() != it ){
    desc = it->second;
    return true;
  }
  //
  // Here, invalid keyword.
  desc = "";
  return false;
}  // End fcn iddMap_getDescriptor().


//--- Count the "A" (string) and "N" (double) markers in a data dictionary descriptor.
//
int iddMap_countDescriptorTypes(const string& desc, int& strCt, int& dblCt){
  //
  int badIdx;
  const int charCt = desc.length();
  //
  badIdx = 0;
  strCt = 0;
  dblCt = 0;
  //
  for( int i=0; i<charCt; i++ ){
    const char mark = desc[i];
    if( mark == 'N' ){
      dblCt++;
    }
    else if( mark == 'A' ){
      strCt++;
    }
    else{
      // Error, have neither 'N' nor 'A'.
      badIdx = i;
    }
  }
  //
  return badIdx;
}  // End fcn iddMap_countDescriptorTypes().


//--- Visually mark a particular entry in a data dictionary descriptor.
//
void iddMap_markDescriptorIdx(const string& desc, int markIdx, string& markedDesc){
  //
  const int descLen = (int)desc.length();
  //
  // Initialize.
  markedDesc = desc;
  //
  // Normalize {markIdx}.
  //   E.g., for five-character string, can be 0 through 5.
  if( markIdx < 0 )
    markIdx = 0;
  else if( markIdx > descLen )
    markIdx = descLen;
  //
  // Insert '>'.
  //   Note doing end first because doing '<' first would change index for end.
  if( markIdx < descLen-1 ){
    // Here, insert '>' between two existing characters.
    //   E.g., marking up to character 4 (index 3) in a 5-character string.
    markedDesc.insert(markIdx+1, 1, '>');
  }
  else{
    // Here, marking last character or end-of-string.
    markedDesc.push_back('>');
  }
  //
  // Insert '<'.
  markedDesc.insert(markIdx, 1, '<');
}  // End fcn iddMap_markDescriptorIdx().


//--- Check an Input Data Dictionary has an expected keyword and descriptor.
//
int iddMap_compareEntry(const iddMap& idd, const string& expectKey, const string& expectDesc,
  string &errStr)
  {
  //
  string iddDesc;
  //
  if( ! iddMap_getDescriptor(idd, expectKey, iddDesc) )
    {
    if( 0 != errStr.size() )
      errStr.push_back('\n');
    errStr.append("Input data dictionary does not contain key '").append(expectKey).push_back('\'');
    return( 1 );
    }
  //
  if( 0 != iddDesc.compare(expectDesc) )
    {
    if( 0 != errStr.size() )
      errStr.push_back('\n');
    errStr.append("For key '").append(expectKey);
    errStr.append("', input data dictionary has descriptor '").append(iddDesc);
    errStr.append("', but expecting '").append(expectDesc).push_back('\'');
    return( 2 );
    }
  //
  return( 0 );
  }  // End fcn iddMap_compareEntry.
