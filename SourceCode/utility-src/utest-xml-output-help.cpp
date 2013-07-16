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
