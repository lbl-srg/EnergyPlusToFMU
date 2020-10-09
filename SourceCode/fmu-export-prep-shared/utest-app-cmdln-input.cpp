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


/*
***********************************************************************************
Copyright Notice
----------------

Functional Mock-up Unit Export of EnergyPlus (C)2013, The Regents of
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
