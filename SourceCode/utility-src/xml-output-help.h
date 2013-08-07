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
