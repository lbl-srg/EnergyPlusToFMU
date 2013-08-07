//--- Unit test for string-help.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for string-help.cpp.


//--- Includes.
#include <assert.h>
#include <math.h>

#include <string>
using std::string;

#include "string-help.h"


//--- Main driver.
//
int main(int argc, const char* argv[]) {
  //
  string expected, s;
  //
  //-- Test fcn containsChar().
  expected = "asdf";
  assert( containsChar(expected, 'a') );
  assert( containsChar(expected, 's') );
  assert( containsChar(expected, 'd') );
  assert( containsChar(expected, 'f') );
  //
  assert( ! containsChar(expected, 'g') );
  assert( ! containsChar(expected, 'z') );
  //
  //-- Test fcn trimEnd();
  s = "1234  ";
  assert( s.length() == 6 );
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("1234") );
  //
  // Trim an untrimmable string.
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("1234") );
  //
  // Trim a zero-length string.
  s = "";
  assert( s.length() == 0 );
  trimEnd(s);
  assert( s.length() == 0 );
  assert( 0 == s.compare("") );
  //
  // Trim a string with embedded spaces.
  s = "123 567 9   ";
  assert( s.length() == 12 );
  trimEnd(s);
  assert( s.length() == 9 );
  assert( 0 == s.compare("123 567 9") );
  //
  // Trim a string with a tab.
  s = "123\t ";
  assert( s.length() == 5 );
  trimEnd(s);
  assert( s.length() == 4 );
  assert( 0 == s.compare("123\t") );
  //
  //-- Test fcn strToDbl().
  double dbl;
  //
  s = "1.2";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl-1.2) < 1e-18 );
  //
  s = "-4.6e2";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl+460) < 1e-18 );
  //
  s = "-0.9e-1";
  assert( strToDbl(s,dbl) );
  assert( fabs(dbl+0.09) < 1e-18 );
  //
  s = "one";
  assert( ! strToDbl(s,dbl) );
  //
  s = "1.2 y";
  assert( ! strToDbl(s,dbl) );
  //
  //-- Test fcn capitalize().
  s = "a";
  capitalize(s);
  assert( 0 == s.compare("A") );
  //
  s = "abc";
  capitalize(s);
  assert( 0 == s.compare("ABC") );
  //
  s = "aBC";
  capitalize(s);
  assert( 0 == s.compare("ABC") );
  //
  s = "a2b1 c ";
  capitalize(s);
  assert( 0 == s.compare("A2B1 C ") );
  //
  return( 0 );
}  // End fcn main().


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
