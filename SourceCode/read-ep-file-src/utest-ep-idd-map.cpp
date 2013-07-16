//--- Unit test for ep-idd-map.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for ep-idd-map.cpp.


//--- Includes.
#include <assert.h>

#include <string>
using std::string;


#include "ep-idd-map.h"


//--- Main driver.
//
int main(int argc, const char* argv[]) {
  //
  const string key1 = "key1";
  const string desc1 = "AA";
  const string key2 = "key2";
  const string desc2 = "ANANAN";
  const string keyBad = "keyBad";
  const string descBad = "ABANANA";
  const string keyMissing = "keyMissing";
  //
  iddMap idd;
  idd[key1] = desc1;
  idd[key2] = desc2;
  idd[keyBad] = descBad;
  //
  //-- Test fcn iddMap_getDescriptor().
  string descGot;
  assert( iddMap_getDescriptor(idd, key1, descGot) );
  assert( desc1 == descGot );
  //
  assert( iddMap_getDescriptor(idd, key2, descGot) );
  assert( desc2 == descGot );
  //
  assert( iddMap_getDescriptor(idd, keyBad, descGot) );
  assert( descBad == descGot );
  //
  assert( ! iddMap_getDescriptor(idd, keyMissing, descGot) );
  assert( 0 == descGot.compare("") );
  //
  //-- Test fcn iddMap_countDescriptorTypes().
  int strCt, dblCt;
  assert( 0 == iddMap_countDescriptorTypes(desc1, strCt, dblCt) );
  assert( 2 == strCt );
  assert( 0 == dblCt );
  //
  assert( 0 == iddMap_countDescriptorTypes(desc2, strCt, dblCt) );
  assert( 3 == strCt );
  assert( 3 == dblCt );
  //
  assert( 1 == iddMap_countDescriptorTypes(descBad, strCt, dblCt) );
  assert( 4 == strCt );
  assert( 2 == dblCt );
  //
  //-- Test fcn iddMap_markDescriptorIdx().
  string markedDesc;
  iddMap_markDescriptorIdx(desc2, 0, markedDesc);
  assert( markedDesc.compare("<A>NANAN") == 0 );
  //
  iddMap_markDescriptorIdx(desc2, 1, markedDesc);
  assert( markedDesc.compare("A<N>ANAN") == 0 );
  //
  iddMap_markDescriptorIdx(desc2, 4, markedDesc);
  assert( markedDesc.compare("ANAN<A>N") == 0 );
  //
  iddMap_markDescriptorIdx(desc2, 5, markedDesc);
  assert( markedDesc.compare("ANANA<N>") == 0 );
  //
  iddMap_markDescriptorIdx(desc2, 6, markedDesc);
  assert( markedDesc.compare("ANANAN<>") == 0 );
  //
  // Should handle {markIdx} out-of-range.
  iddMap_markDescriptorIdx(desc2, -1, markedDesc);
  assert( markedDesc.compare("<A>NANAN") == 0 );
  //
  iddMap_markDescriptorIdx(desc2, 20, markedDesc);
  assert( markedDesc.compare("ANANAN<>") == 0 );
  //
  //-- Test fcn iddMap_compareEntry().
  string errStr;
  assert( 0 == iddMap_compareEntry(idd, key1, desc1, errStr) );
  assert( 0 == errStr.size() );
  //
  assert( 0 == iddMap_compareEntry(idd, key2, desc2, errStr) );
  assert( 0 == errStr.size() );
  //
  assert( 1 == iddMap_compareEntry(idd, keyMissing, desc1, errStr) );
  assert( 0 == errStr.compare("Input data dictionary does not contain key 'keyMissing'") );
  //
  assert( 2 == iddMap_compareEntry(idd, key1, desc2, errStr) );
  assert( 0 == errStr.compare("Input data dictionary does not contain key 'keyMissing'"
    "\nFor key 'key1', input data dictionary has descriptor 'AA', but expecting 'ANANAN'") );
  //
  return( 0 );
}  // End fcn main().
