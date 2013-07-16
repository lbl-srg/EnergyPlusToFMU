//--- Utilities for producing XML output.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  XML output utilities.


#if !defined(__XML_OUTPUT_HELP_H__)
#define __XML_OUTPUT_HELP_H__


//--- Includes.
//
#include <ostream>


//--- Write a string as XML text.
//
//   Escape special characters.
//
//   Examples:
// ** This is a string. ==> This is a string.
// ** the inequality x < y ==> the inequality x &lt; y
// ** advise & consent ==> advise &amp; consent
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
// \param indentLevel -1=no newline, 0=newline only, 1=newline and indent one stop, etc.
// \param str Null-terminated string containing text to be written as XML.
//
extern void xmlOutput_text(std::ostream& outStream, const int indentLevel,
  const char *str);


//--- Write a name and value as an XML attribute.
//
//   In addition to any indentation, add spaces to separate from the preceding
// tag or attribute.
//
//   Examples:
// ** {checked}, {true} ==> {  checked="true"}
// ** {checked}, {not "as such"} ==> {  checked="not &quot;as such&quot"}
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
// \param indentLevel -1=no newline, 0=newline only, 1=newline and indent one stop, etc.
// \param attName Null-terminated string containing attribute name.
// \param attValue Null-terminated string containing attribute value.
//
void xmlOutput_attribute(std::ostream& outStream, const int indentLevel,
  const char *const attName, const char *const attValue);


//--- Write an XML start tag.
//
//   Examples:
// ** {theTag} ==> {<theTag}
//
//   Note this fcn begins a start tag.
//   To add attributes to the start tag, use fcn xmlOutput_attribute().
//   To finish the start tag in a way that allows adding content (text, other
// tags, comments, etc.), use fcn xmlOutput_startTag_finish().  However, if the
// start tag will not have any content, it may be finished by using
// fcn xmlOutput_endTag().
//   Ultimately, the tag started with this fcn must be closed by using
// fcn xmlOutput_endTag().
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
// \param indentLevel -1=no newline, 0=newline only, 1=newline and indent one stop, etc.
// \param tagName Null-terminated string containing tag name.
//
void xmlOutput_startTag(std::ostream& outStream, const int indentLevel,
  const char *const tagName);


//--- Write close of an XML start tag that will have content.
//
//   Examples:
// ** {>}
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
//
void xmlOutput_startTag_finish(std::ostream& outStream);


//--- Write an XML end tag.
//
//   Examples:
// ** {theTag} ==> {</theTag>}
// ** {NULL} ==> {/>}
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
// \param indentLevel -1=no newline, 0=newline only, 1=newline and indent one stop, etc.
// \param tagName Null-terminated string containing tag name.  If NULL, assume
//   the start tag had no contents, and has not already been finished via
//   fcn xmlOutput_startTag_finish().
//
void xmlOutput_endTag(std::ostream& outStream, const int indentLevel,
  const char *const tagName);


//--- Write a string as an XML comment.
//
//   Examples:
// ** {This is a comment.} ==> {<!-- This is a comment. -->}
//
// \param outStream Stream to which to write the text.  Can be an {ostringstream} if a string is desired.
// \param startNewLine, begin new line.
// \param indentLevel If > 0, indent by this count.
// \param str Null-terminated string containing comment to be written.
//
extern void xmlOutput_comment(std::ostream& outStream, const int indentLevel,
  const char *const str);


#endif // __XML_OUTPUT_HELP_H__
