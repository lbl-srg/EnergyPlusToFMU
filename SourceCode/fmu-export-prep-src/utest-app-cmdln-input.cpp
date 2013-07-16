//--- Unit test for app-cmdln-input.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for app-cmdln-input.


//--- Includes.
//
#include <assert.h>

#include <iostream>
using std::cout;
using std::endl;

#include "app-cmdln-input.h"


//--- File-scope fcn prototypes.
//
void printHeadsUp(const char *const msg);
void printCmdlnInputs(const cmdlnInput_s *const cmdlnInputP);


//--- Main driver.
//
int main(int argc, const char* argv[])
  {
  //
  cmdlnInput_s cmdlnInput;
  //
  const char *argv_plain[] = {"argv_plain", "p.idd", "p.idf", "this is an error"};
  const char *argv_v[] = {"argv_v", "-v", "v.idd", "v.idf"};
  const char *argv_h[] = {"argv_h", "-h", "h.idd", "h.idf"};
  const char *argv_vh[] = {"argv_vh", "-version", "-help", "vh.idd", "vh.idf"};
  const char *argv_badSwitch[] = {"argv_badSwitch", "-b", "b.idd", "b.idf"};
  const char *argv_w[] = {"argv_w", "-w", "w.wth", "w.idd", "w.idf"};
  const char *argv_vw[] = {"argv_vw", "-v", "-w", "vw.wth", "vw.idd", "vw.idf"};
  const char *argv_wv[] = {"argv_wv", "-w", "wv.wth", "-v", "wv.idd", "wv.idf"};
  //
  //-- With {argv_plain}.
  printHeadsUp("argv_plain: OK");
  assert( cmdlnInput_get(3, argv_plain, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_plain: args count error");
  assert( ! cmdlnInput_get(4, argv_plain, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_plain: args count error");
  assert( ! cmdlnInput_get(2, argv_plain, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_plain: args count error");
  assert( ! cmdlnInput_get(1, argv_plain, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_v}.
  printHeadsUp("argv_v: OK");
  assert( cmdlnInput_get(4, argv_v, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  // If ask for version, with no args, shouldn't write error message, but should return {FALSE}.
  printHeadsUp("argv_v: version");
  assert( ! cmdlnInput_get(2, argv_v, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_h}.
  printHeadsUp("argv_h: OK");
  assert( cmdlnInput_get(4, argv_h, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_h: args count error");
  assert( ! cmdlnInput_get(3, argv_h, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  // If ask for help, with no args, shouldn't write error message, but should return {FALSE}.
  printHeadsUp("argv_h: help");
  assert( ! cmdlnInput_get(2, argv_h, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_vh}.
  printHeadsUp("argv_vh: OK");
  assert( cmdlnInput_get(5, argv_vh, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_badSwitch}.
  //   Note a bad command-line switch isn't enough to cause an error return.
  printHeadsUp("argv_badSwitch: unknown switch error");
  assert( cmdlnInput_get(4, argv_badSwitch, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_w}.
  printHeadsUp("argv_w: OK");
  assert( cmdlnInput_get(5, argv_w, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_w: missing weather file");
  assert( ! cmdlnInput_get(2, argv_w, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  printHeadsUp("argv_w: args count error");
  assert( ! cmdlnInput_get(4, argv_w, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_vw}.
  printHeadsUp("argv_vw: OK");
  assert( cmdlnInput_get(6, argv_vw, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  //-- With {argv_wv}.
  printHeadsUp("argv_wv: OK");
  assert( cmdlnInput_get(6, argv_wv, &cmdlnInput, NULL) );
  printCmdlnInputs(&cmdlnInput);
  //
  return(0);
  }  // End fcn main().


//--- Print heads-up, e.g., for expected output.
//
void printHeadsUp(const char *const msg)
  {
  cout << "\n----- Expect " << msg << " -----\n";
  }  // End fcn printHeadsUp().


//--- Print received command-line inputs.
//
void printCmdlnInputs(const cmdlnInput_s *const cmdlnInputP)
  {
  if( cmdlnInputP->wthFileName )
    cout << "WTH file: " << cmdlnInputP->wthFileName << endl;
  if( cmdlnInputP->iddFileName )
    cout << "IDD file: " << cmdlnInputP->iddFileName << endl;
  if( cmdlnInputP->idfFileName )
    cout << "IDF file: " << cmdlnInputP->idfFileName << endl;
  }  // End fcn printCmdlnInputs().
