//--- Work with EnergyPlus input data dictionary (IDD) key-descriptor pairs.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  Work with EnergyPlus input data dictionary (IDD) key-descriptor pairs.


#if !defined(__EP_IDD_MAP_H__)
#define __EP_IDD_MAP_H__


//--- Sample IDD mapping.
//
// The following entry in the IDD file maps keyword "VERSION" to descriptor "A":
// Version,
//       \unique-object
//       \format singleLine
//   A1; \field Version Identifier
//       \required-field
//       \default 7.0


//--- Sample IDD mapping.
//
// The following entry maps keyword "SHADOWCALCULATION" to descriptor "NNAA":
// ShadowCalculation,
//        \unique-object
//   N1 , \field Calculation Frequency
//        \required-field
//        \type integer
//        \minimum 1
//        \default 20
//        \note 0=Use Default Periodic Calculation|<else> calculate every <value> day
//        \note only really applicable to RunPeriods
//        \note warning issued if >31
//   N2 , \field Maximum Figures in Shadow Overlap Calculations
//        \note Number of allowable figures in shadow overlap calculations
//        \type integer
//        \minimum 200
//        \default 15000
//   A1 , \field Polygon Clipping Algorithm
//        \note Advanced Feature.  Internal default is SutherlandHodgman
//        \note Refer to InputOutput Reference and Engineering Reference for more information
//        \type choice
//        \key ConvexWeilerAtherton
//        \key SutherlandHodgman
//   A2 ; \field Sky Diffuse Modeling Algorithm
//        \note Advanced Feature.  Internal default is SimpleSkyDiffuseModeling
//        \note If you have shading elements that change transmittance over the
//        \note year, you may wish to choose the detailed method.
//        \note Refer to InputOutput Reference and Engineering Reference for more information
//        \type choice
//        \key SimpleSkyDiffuseModeling
//        \key DetailedSkyDiffuseModeling


//--- Includes.

#include <string>
using std::string;
#include <map>


//--- Types.


//--- Store an IDD mapping.
//
//   An \c iddMap maps an input data dictionary keyword to its data descriptor.
//
// hoho dml  Definitions are fairly constrained, to either "A", "N", or nothing.
// Should consider mapping keyword to an array of some struct that indicates the
// data type.  That way, would parse only once, in case wanted to use the dictionary
// to check entries while reading an IDF file.
//
// hoho dml  Note there's a lot of information in the data dictionary that gets
// thrown away as "comments".  For example, in the ShadowCalculation entry above,
// fact that it's a "unique-object" doesn't get encoded in the data dictionary.
//
// hoho dml  Consider creating an {iddMap} class.
//
typedef std::map<std::string, std::string> iddMap;


//--- Functions.


/// Get descriptor for a given keyword.
///
/// \param idd The \c iddMap to query.
/// \param key Keyword for which to retrieve the descriptor from the data dictionary.
/// \retval desc String containing the descriptor (blank if invalid key).
/// \return \c true if \c key is valid, \c false otherwise.
bool iddMap_getDescriptor(const iddMap& idd, const string& key, string& desc);


/// Count the "A" (string) and "N" (double) markers in a data dictionary descriptor.
///
/// \param desc Data descriptor to be scanned for \c N and \c A'
/// \retval strCt number of \c A's in the descriptor.
/// \retval dblCt number of \c N's in the descriptor.
/// \return 0 if the descriptor contains only characters \c N or \c A; otherwise, index of offending marker.
int iddMap_countDescriptorTypes(const std::string& desc, int& strCt, int& dblCt) ;


/// Visually mark a particular entry in a data dictionary descriptor.
//
//   Assume {markIdx} is zero-indexed.
//
//   Examples:
// ** Mark index 0 in "ANA" --> "<A>NA".
// ** Mark index 1 in "ANA" --> "A<N>A".
// ** Mark index 2 in "ANA" --> "AN<A>".
// ** Mark index 3 in "ANA" --> "ANA<>".
///
/// \param desc  Data descriptor.
/// \param markIdx  Index of entry to mark, starting from 0.
/// \retval markedDesc  Descriptor \c desc, with entry \c markIdx visually marked.
void iddMap_markDescriptorIdx(const string& desc, int markIdx, string& markedDesc);


//--- Check an Input Data Dictionary has an expected keyword and descriptor.
//
//   Report on strict match (including capitalization) between expected entries.
//
// \return 0 for success; 1 for missing keyword; 2 for incorrect descriptor.
// \retval errStr Append error message, if any, to this string.
//
int iddMap_compareEntry(const iddMap& idd, const string& expectKey, const string& expectDesc,
  string &errStr);


#endif //__EP_IDD_MAP_H__


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
