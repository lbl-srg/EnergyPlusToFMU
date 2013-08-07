//--- Unit test for xml-output-help.cpp.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for xml-output-help.cpp.


//--- Includes.
//
#include <string>
#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;


#include "xml-output-help.h"


//--- File-scope fcn prototypes.
//
static void compareStrings(const char *const testIdStr,
  std::ostringstream& result, const char *const expectStr);
static void check_xmlOutput_text(const int indentLevel,
  const char *const givenStr,
  const char *const expectStr);
static void check_xmlOutput_attribute(const int indentLevel,
  const char *const attName, const char *const attValue,
  const char *const expectStr);
static void check_xmlOutput_startTag(const int indentLevel,
  const char *const tagName,
  const char *const expectStr);
static void check_xmlOutput_startTag_finish(void);
static void check_xmlOutput_endTag(const int indentLevel,
  const char *const tagName,
  const char *const expectStr);
static void check_xmlOutput_comment(const int indentLevel,
  const char *const givenStr,
  const char *const expectStr);


//--- Functions.


//--- Main driver.
//
int main(int argc, const char* argv[]) {
  //
  //-- Test fcn xmlOutput_text().
  check_xmlOutput_text(-1, "asdf", "asdf");
  //
  // Escape "&".
  check_xmlOutput_text(-1, "A&B", "A&amp;B");
  check_xmlOutput_text(-1, "&B&", "&amp;B&amp;");
  check_xmlOutput_text(-1, "&&", "&amp;&amp;");
  //
  // Escape ">".
  check_xmlOutput_text(-1, ">", "&gt;");
  check_xmlOutput_text(-1, "-->--", "--&gt;--");
  check_xmlOutput_text(-1, ">-->", "&gt;--&gt;");
  //
  // Escape "<".
  check_xmlOutput_text(-1, "<j<", "&lt;j&lt;");
  //
  // Escape '"'.
  check_xmlOutput_text(-1, "a\"b", "a&quot;b");
  //
  // Escape '\''.
  check_xmlOutput_text(-1, "c'd", "c&apos;d");
  //
  // Escape combinations.
  check_xmlOutput_text(-1, "<>", "&lt;&gt;");
  check_xmlOutput_text(-1, "<&>", "&lt;&amp;&gt;");
  check_xmlOutput_text(-1, "one >< and & two >>>", "one &gt;&lt; and &amp; two &gt;&gt;&gt;");
  check_xmlOutput_text(-1, "<\n'>", "&lt;\n&apos;&gt;");
  check_xmlOutput_text(-1, "&\"<", "&amp;&quot;&lt;");
  //
  // Request newlines.
  check_xmlOutput_text(-1, "A", "A");
  check_xmlOutput_text(0, "A", "\nA");
  check_xmlOutput_text(1, "A", "\n  A");
  check_xmlOutput_text(2, "A", "\n    A");
  //
  //-- Test fcn xmlOutput_attribute().
  check_xmlOutput_attribute(-1, "name0", "val0", "  name0=\"val0\"");
  check_xmlOutput_attribute(-1, "name1", "val1", "  name1=\"val1\"");
  check_xmlOutput_attribute(0, "name2", "val2 > val1", "\n  name2=\"val2 &gt; val1\"");
  check_xmlOutput_attribute(1, "name3", "val\"3\"", "\n    name3=\"val&quot;3&quot;\"");
  check_xmlOutput_attribute(2, "name4", "val4", "\n      name4=\"val4\"");
  //
  //-- Test fcn xmlOutput_startTag().
  check_xmlOutput_startTag(-1, "tag0", "<tag0");
  check_xmlOutput_startTag(0, "tag1", "\n<tag1");
  check_xmlOutput_startTag(1, "tag2", "\n  <tag2");
  //
  //-- Test fcn xmlOutput_startTag_finish().
  check_xmlOutput_startTag_finish();
  //
  //-- Test fcn xmlOutput_endTag().
  check_xmlOutput_endTag(-1, "tag0", "</tag0>");
  check_xmlOutput_endTag(0, "tag1", "\n</tag1>");
  check_xmlOutput_endTag(1, "tag2", "\n  </tag2>");
  //
  check_xmlOutput_endTag(-1, NULL, "/>");
  check_xmlOutput_endTag(0, NULL, "\n/>");
  check_xmlOutput_endTag(1, NULL, "\n  />");
  //
  //--- Put together complete start tags.
  std::ostringstream result;
  //
  xmlOutput_startTag(result, -1, "tag0");
  xmlOutput_startTag_finish(result);
  compareStrings("whole tag0", result, "<tag0>");
  //
  xmlOutput_endTag(result, -1, "tag0");
  compareStrings("whole tag0 with end", result, "<tag0></tag0>");
  //
  result.str("");  // Reset {result} to empty string.
  xmlOutput_startTag(result, -1, "tag1");
  xmlOutput_attribute(result, -1, "name0", "val0");
  xmlOutput_endTag(result, -1, NULL);
  compareStrings("whole tag1", result, "<tag1  name0=\"val0\"/>");
  //
  result.str("");
  xmlOutput_startTag(result, -1, "tag2");
  xmlOutput_endTag(result, 0, NULL);
  compareStrings("whole tag2", result, "<tag2\n/>");
  //
  result.str("");
  xmlOutput_startTag(result, -1, "tag3");
  xmlOutput_attribute(result, -1, "name0", "val0");
  xmlOutput_attribute(result, 0, "name1", "val1");
  xmlOutput_startTag_finish(result);
  xmlOutput_endTag(result, 0, "tag3");
  compareStrings("whole tag3", result, "<tag3  name0=\"val0\"\n  name1=\"val1\">\n</tag3>");
  //
  //-- Test fcn xmlOutput_comment().
  check_xmlOutput_comment(-1, "0", "<!-- 0 -->");
  check_xmlOutput_comment(0, "1", "\n<!-- 1 -->");
  check_xmlOutput_comment(1, "2", "\n  <!-- 2 -->");
  //
  return( 0 );
}  // End fcn main().


//--- Compare two strings.
//
static void compareStrings(const char *const testIdStr,
  std::ostringstream& result, const char *const expectStr)
  {
  if( 0 != result.str().compare(expectStr) )
    {
    cerr << "Test: " << testIdStr << endl <<
      "..Expecting..." << expectStr << "..." << endl <<
      "..Got........." << result.str() << "..." << endl;
    }
  }  // End fcn compareStrings().


//--- Check fcn xmlOutput_text().
//
//   Write {givenStr}, and make sure it results in {expectStr}.
//   Note using an {ostringstream}, but could use e.g., {stdout}.
//
static void check_xmlOutput_text(const int indentLevel,
  const char *const givenStr,
  const char *const expectStr)
  {
  std::ostringstream result;
  //
  xmlOutput_text(result, indentLevel, givenStr);
  compareStrings("fcn check_xmlOutput_text", result, expectStr);
  //
  }  // End fcn check_xmlOutput_text().


//--- Check fcn xmlOutput_attribute().
//
static void check_xmlOutput_attribute(const int indentLevel,
  const char *const attName, const char *const attValue,
  const char *const expectStr)
  {
  std::ostringstream result;
  //
  xmlOutput_attribute(result, indentLevel,
    attName, attValue);
  compareStrings("fcn check_xmlOutput_attribute", result, expectStr);
  }  // End fcn check_xmlOutput_attribute().


//--- Check fcn xmlOutput_startTag().
//
static void check_xmlOutput_startTag(const int indentLevel,
  const char *const tagName,
  const char *const expectStr)
  {
  std::ostringstream result;
  //
  xmlOutput_startTag(result, indentLevel,
    tagName);
  compareStrings("fcn check_xmlOutput_startTag", result, expectStr);
  //
  }  // End fcn check_xmlOutput_startTag().


//--- Check fcn xmlOutput_startTag_finish().
//
static void check_xmlOutput_startTag_finish(void)
  {
  std::ostringstream result;
  const char *const expectStr = ">";
  //
  xmlOutput_startTag_finish(result);
  compareStrings("fcn check_xmlOutput_startTag_finish", result, expectStr);
  //
  }  // End fcn check_xmlOutput_startTag_finish().


//--- Check fcn xmlOutput_endTag().
//
static void check_xmlOutput_endTag(const int indentLevel,
  const char *const tagName,
  const char *const expectStr)
  {
  std::ostringstream result;
  //
  xmlOutput_endTag(result, indentLevel, tagName);
  compareStrings("fcn check_xmlOutput_endTag", result, expectStr);
  //
  }  // End fcn check_xmlOutput_endTag().


//--- Check fcn xmlOutput_comment().
//
static void check_xmlOutput_comment(const int indentLevel,
  const char *const givenStr,
  const char *const expectStr)
  {
  std::ostringstream result;
  //
  xmlOutput_comment(result, indentLevel, givenStr);
  compareStrings("fcn check_xmlOutput_comment", result, expectStr);
  //
  }  // End fcn check_xmlOutput_comment().


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
