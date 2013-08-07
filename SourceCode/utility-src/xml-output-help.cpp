//--- Utilities for producing XML output.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include "xml-output-help.h"


//--- File-scope fcn prototypes.
//
static void newlineHelp(std::ostream& outStream, int indentLevel);


//--- Functions.


//--- Write a string as XML text.
//
void xmlOutput_text(std::ostream& outStream, const int indentLevel,
  const char *str)
  {
  //
  if( 0 <= indentLevel )
    newlineHelp(outStream, indentLevel);
  //
  while( 1 )
    {
    const char ch = *str;
    if( '\0' == ch )
      {
      // Here, done with {str}.
      break;
      }
    ++str;
    //
    // Write {ch} to {outStream}, escaping special characters.
    switch( ch )
      {
    case( '&' ):
      outStream.write("&amp;", 5);
      break;
    case( '>' ):
      outStream.write("&gt;", 4);
      break;
    case( '<' ):
      outStream.write("&lt;", 4);
      break;
    case( '"' ):
      outStream.write("&quot;", 6);
      break;
    case( '\'' ):
      outStream.write("&apos;", 6);
      break;
    default:
      outStream.put(ch);
      break;
      }
    }
  }  // End fcn xmlOutput_text().


//--- Write a name and value as an XML attribute.
//
void xmlOutput_attribute(std::ostream& outStream, const int indentLevel,
  const char *const attName, const char *const attValue)
  {
  //
  if( 0 <= indentLevel )
    newlineHelp(outStream, indentLevel);
  //
  // Write {  name="value"}.
  outStream.write("  ", 2);
  outStream << attName;
  outStream.write("=\"", 2);
  xmlOutput_text(outStream, -1, attValue);
  outStream.write("\"", 1);
  }  // End fcn xmlOutput_attribute().


//--- Write an XML start tag.
//
void xmlOutput_startTag(std::ostream& outStream, const int indentLevel,
  const char *const tagName)
  {
  //
  if( 0 <= indentLevel )
    newlineHelp(outStream, indentLevel);
  //
  // Start tag.
  outStream.write("<", 1);
  outStream << tagName;
  }  // End fcn xmlOutput_startTag().


//--- Write close of an XML start tag that will have content.
//
void xmlOutput_startTag_finish(std::ostream& outStream)
  {
  outStream.write(">", 1);
  }  // End fcn xmlOutput_startTag_finish().


//--- Write an XML end tag.
//
void xmlOutput_endTag(std::ostream& outStream, const int indentLevel,
  const char *const tagName)
  {
  //
  if( 0 <= indentLevel )
    newlineHelp(outStream, indentLevel);
  //
  // End tag.
  if( tagName )
    {
    outStream.write("</", 2);
    outStream << tagName;
    outStream.write(">", 1);
    }
  else
    {
    outStream.write("/>", 2);
    }
  //
  }  // End fcn xmlOutput_endTag().


//--- Write a string as an XML comment.
//
void xmlOutput_comment(std::ostream& outStream, const int indentLevel,
  const char *const str)
  {
  //
  if( 0 <= indentLevel )
    newlineHelp(outStream, indentLevel);
  //
  // Write comment.
  outStream.write("<!-- ", 5);
  outStream << str;
  outStream.write(" -->", 4);
  //
  }  // End fcn xmlOutput_comment().


//--- Handle new lines and indenting.
//
static void newlineHelp(std::ostream& outStream, int indentLevel)
  {
  // Assume {indentLevel} >= 0.
  outStream.put('\n');
  //
  // Indent if necessary.
  //   E.g., {indentLevel}==1 means indent by one stop.
  while( 0 < indentLevel-- )
    {
    outStream.write("  ", 2);
    }
  }  // End fcn newlineHelp().
