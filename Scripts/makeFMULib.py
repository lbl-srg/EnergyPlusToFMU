#!/usr/bin/env  python


#--- Purpose.
#
#   Create a shared library that runs an EnergyPlus IDF file as a
# Functional Mockup Unit (FMU) for co-simulation.


#--- Note on directory location.
#
#   This script uses relative paths to locate some of the files it needs.
# Therefore it should not be moved from its default directory.
#   However, this script can be run from a different working directory.


#--- Ensure access.
#
import os
import re
import subprocess
import sys


#--- Identify system.
#
PLATFORM_NAME = sys.platform
#
if( PLATFORM_NAME.startswith('win') ):
    PLATFORM_SHORT_NAME = 'win'
    BATCH_EXTENSION = '.bat'
    SHARED_LIB_EXTENSION = '.dll'
elif( PLATFORM_NAME.startswith('linux')
    or PLATFORM_NAME.startswith('cygwin') ):
    PLATFORM_SHORT_NAME = 'linux'
    BATCH_EXTENSION = '.sh'
    SHARED_LIB_EXTENSION = '.so'
elif( PLATFORM_NAME.startswith('darwin') ):
    PLATFORM_SHORT_NAME = 'darwin'
    BATCH_EXTENSION = '.sh'
    SHARED_LIB_EXTENSION = '.dylib'
else:
    raise Exception('Unknown platform {' +PLATFORM_NAME +'}')


#--- Note on compile and link batch files.
#
#   This script uses separate, system-dependent, batch files to compile and
# link source code.  For more information, see fcns printCompileCBatchInfo(),
# printLinkCLibBatchInfo(), and printLinkCExeBatchInfo().
#
COMPILE_C_BATCH_FILE_NAME = 'compile-c' + BATCH_EXTENSION
LINK_C_LIB_BATCH_FILE_NAME = 'link-c-lib' + BATCH_EXTENSION
LINK_C_EXE_BATCH_FILE_NAME = 'link-c-exe' + BATCH_EXTENSION


#--- Running this script.
#
#   To run this script from the command line:
# > python  [python options]  <this-file-name>  <arguments>
#
#   On unix-like systems, this command-line invocation should work as well:
# > ./<this-file-name>  <arguments>
#
#   To call this script from the Python interpreter, or from another Python script:
# >>> import <this-file-base-name>
# >>> <this-file-base-name>.makeFmuSharedLib(arguments)


#--- Runtime help.
#
def printCmdLineUsage():
  #
  print 'USAGE:', os.path.basename(__file__),  \
    '[-d]  [-L]  <path-to-idf-file>'
  #
  print '-- Create a shared library that runs an EnergyPlus IDF file as an FMU'
  print '-- Option -d, print diagnostics'
  print '-- Option -L, litter, that is, do not clean up intermediate files'
  #
  print
  printCompileCBatchInfo()
  #
  print
  printLinkCLibBatchInfo()
  #
  print
  printLinkCExeBatchInfo()
  #
  # End fcn printCmdLineUsage().


def printCompileCBatchInfo():
  #
  print 'Require a batch file {' +COMPILE_C_BATCH_FILE_NAME +'}'
  print '-- The batch file should compile C source code files'
  print '-- The batch file should accept one argument, the name (including path) of the source code file to compile'
  print '-- The batch file should leave the resulting object file in the working directory'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printCompileCBatchInfo().


def printLinkCLibBatchInfo():
  #
  print 'Require a batch file {' +LINK_C_LIB_BATCH_FILE_NAME +'}'
  print '-- The batch file should link object files compiled via ' +COMPILE_C_BATCH_FILE_NAME
  print '-- The batch file should produce a shared library'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output shared library'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCLibBatchInfo().


def printLinkCExeBatchInfo():
  #
  print 'Require a batch file {' +LINK_C_EXE_BATCH_FILE_NAME +'}'
  print '-- The batch file should link object files compiled via ' +COMPILE_C_BATCH_FILE_NAME
  print '-- The batch file should produce a command-line executable'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output executable'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCExeBatchInfo().


#--- Fcn to print diagnostics.
#
def printDiagnostic(messageStr):
  #
  print '!', os.path.basename(__file__), '--', messageStr
  #
  # End fcn printDiagnostic().


#--- Fcn to quit due to an error.
#
def quitWithError(messageStr, showCmdLine):
  #
  print 'ERROR from script file {' +os.path.basename(__file__) +'}'
  #
  if( messageStr is not None ):
    print messageStr
  #
  if( showCmdLine ):
    print
    printCmdLineUsage()
  #
  sys.exit(1)
  #
  # End fcn quitWithError().


#--- Fcn to verify a file exists.
#
#   If file exists, return its absolute path.  Otherwise, quit.
#
def findFileOrQuit(fileDesc, fileName):
  #
  if( not os.path.isfile(fileName) ):
    (dirName, fileName) = os.path.split(os.path.abspath(fileName))
    if( not os.path.isdir(dirName) ):
      quitWithError('Missing directory {' +dirName +'} for ' +fileDesc +' file {' +fileName +'}', False)
    quitWithError('Missing ' +fileDesc +' file {' +fileName +'} in directory {' +dirName +'}', False)
  #
  return( os.path.abspath(fileName) )
  #
  # End fcn findFileOrQuit().


#--- Fcn to delete a file.
#
#   OK if file does not exist.
#
def deleteFile(fileName):
  #
  # Prevent removing util-get file which has restricted permissions on
  # some systems
  if( os.path.isfile(fileName) and not(fileName.startswith("util-get"))):
    try:
      os.remove(fileName)
    except:
      quitWithError('Unable to delete file {' +fileName +'}', False)
  elif( os.path.isdir(fileName) ):
    quitWithError('Expecting {' +fileName +'} to be a file; found a directory', False)
  #
  # End fcn deleteFile().


#--- Fcn to sanitize an identifier.
#
#   Make an identifier acceptable as the name of a C function.
#   In C, a function name:
# ** Can contain any of the characters {a-z,A-Z,0-9,_}.
# ** Cannot start with a number.
# ** Can contain universal character names from the ISO/IEC TR 10176 standard.
# However, universal character names are not supported here.
#
g_rexBadIdChars = re.compile(r'[^a-zA-Z0-9_]')
#
def sanitizeIdentifier(identifier):
  #
  if( len(identifier) <= 0 ):
    quitWithError('Require a non-null identifier', False)
  #
  # Can't start with a number.
  if( identifier[0].isdigit() ):
    identifier = 'f_' +identifier
  #
  # Replace all illegal characters with an underscore.
  identifier = g_rexBadIdChars.sub('_', identifier)
  #
  return( identifier )
  #
  # End fcn sanitizeIdentifier().


#--- Fcn to insert model identifier into a source code file.
#
#   Replace a dummy string in {origFileName} with the required one, and save as
# {modFileName}.
#   Assume the dummy string occurs exactly once in {origFileName}.
#
g_rexPoundDefineModelIdString = re.compile(r'^#define MODEL_IDENTIFIER(.*)$')
#
def poundDefineModelId(showDiagnostics, origFileName, modelIdName, modFileName):
  #
  if( showDiagnostics ):
    printDiagnostic('Setting {#define MODEL_IDENTIFIER} to {' +modelIdName +'} in copy of {' +origFileName +'}')
  #
  # Must have file {origFileName}, and can't be same as {modFileName}.
  origFileName = findFileOrQuit('original source', origFileName)
  if( os.path.abspath(modFileName) == origFileName ):
    quitWithError('Attempting to overwrite original file {' +origFileName +'}', False)
  #
  # Open files.
  try:
    origFile = open(origFileName, mode='r')
  except:
    quitWithError('Unable to open original file {' +origFileName +'}', False)
  #
  try:
    modFile = open(modFileName, mode='w')
  except:
    quitWithError('Unable to open modified file {' +modFileName +'}', False)
  #
  # Step through file until find dummy string.
  gotCt = 0
  for origLine in origFile:
    if( not g_rexPoundDefineModelIdString.match(origLine) ):
      modFile.write(origLine)
    else:
      gotCt += 1
      modFile.write('#define MODEL_IDENTIFIER ' +modelIdName +'\r\n')  # Note normally would use '\n' to get system-specific line ending, but want to force line ending on all systems.
  #
  # Close up.
  modFile.close()
  origFile.close()
  #
  if( gotCt < 1 ):
    quitWithError('Did not find expected dummy string in file {' +origFileName +'}', False)
  elif( gotCt > 1 ):
    quitWithError('Found more than one dummy string in file {' +origFileName +'}', False)
  #
  # End fcn poundDefineModelId().


#--- Fcn to create shared library for FMU.
#
#   The resulting shared library implements an EnergyPlus IDF file as an FMU
# for co-simulation.
#
#   Arguments:
# ** {modelIdName}, base name for shared library.  Note the name may be
# "sanitized" since it also has to be a valid function name in the C language.
#
def makeFmuSharedLib(showDiagnostics, litter,
  modelIdName):
  #
  if( showDiagnostics ):
    printDiagnostic('Begin creating shared FMU library for model {' +modelIdName +'}')
  #
  # Sanitize {modelIdName}.
  modelIdSanitizedName = sanitizeIdentifier(modelIdName)
  if( showDiagnostics and (modelIdSanitizedName != modelIdName) ):
    printDiagnostic('Converting model identifier from {' +modelIdName +'} to {' +modelIdSanitizedName +'}')
  #
  fmuSharedLibName = modelIdSanitizedName + SHARED_LIB_EXTENSION
  #
  # Form absolute path to system-specific script directory.
  if( showDiagnostics ):
    printDiagnostic('Using system-specific scripts from batch directory {' +PLATFORM_SHORT_NAME +'}')
  scriptDirName = os.path.abspath(os.path.dirname(__file__))
  batchDirAbsName = os.path.join(scriptDirName, PLATFORM_SHORT_NAME)
  if( not os.path.isdir(batchDirAbsName) ):
    quitWithError('Missing system-specific batch directory {' +batchDirAbsName +'}', False)
  #
  # Form names of system-specific scripts.
  compileCBatchFileName = os.path.join(batchDirAbsName, COMPILE_C_BATCH_FILE_NAME)
  findFileOrQuit('compiler batch', compileCBatchFileName)
  #
  linkCLibBatchFileName = os.path.join(batchDirAbsName, LINK_C_LIB_BATCH_FILE_NAME)
  findFileOrQuit('linker batch', linkCLibBatchFileName)
  #
  linkCExeBatchFileName = os.path.join(batchDirAbsName, LINK_C_EXE_BATCH_FILE_NAME)
  findFileOrQuit('linker batch', linkCExeBatchFileName)
  #
  # Insert model identifier into source code files.
  origMainName = os.path.join(scriptDirName, '../SourceCode/EnergyPlus/main.c')
  modMainName  = os.path.join(scriptDirName, '../SourceCode/EnergyPlus', 'temp-'+modelIdSanitizedName+'.c')
  poundDefineModelId(showDiagnostics, origMainName, modelIdSanitizedName, modMainName)
  #
  # Assemble names of source files.
  srcFileNameList = list()
  #
  srcFileNameList.append(modMainName)
  #
  srcDirName = os.path.join(scriptDirName, '../SourceCode/EnergyPlus')
  for theRootName in ['stack',
    'util',
    'utilSocket',
    'xml_parser_cosim'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.c'))
  #
  srcDirName = os.path.join(scriptDirName, '../SourceCode/Expat/lib')
  for theRootName in ['xmlparse',
    'xmlrole',
    'xmltok'  # Note {xmltok.c} directly #includes {xmltok_impl.c} and {xmltok_ns.c}, so they don't need to be in this list.
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.c'))
  #
  # Load modules expect to find in same directory as this script file.
  if( scriptDirName not in sys.path ):
    sys.path.append(scriptDirName)
  #
  findFileOrQuit('utility script', os.path.join(scriptDirName,'utilManageCompileLink.py'))
  try:
    import utilManageCompileLink
  except:
    quitWithError('Unable to import {utilManageCompileLink.py}', False)
  #
  # Build {fmuSharedLibName}.
  utilManageCompileLink.manageCompileLink(showDiagnostics, litter, True,
    compileCBatchFileName, linkCLibBatchFileName, srcFileNameList, fmuSharedLibName)
  #
  # Delete {modMainName}.
  #   Note always do this, regardless of {litter}, since the file is in the
  # source tree, rather than in the script directory or the user's working
  # directory (it has to be in the source tree, in order for the include
  # paths to make sense).
  deleteFile(modMainName)
  #
  # Make executable to determine size of memory address, in bits.
  getAddressSizeExeName = 'util-get-address-size.exe'
  if( showDiagnostics ):
    printDiagnostic('Building utility application {' +getAddressSizeExeName +'}')
  srcFileNameList = [
    os.path.join(scriptDirName, '../SourceCode/utility/get-address-size.c')
    ]
  utilManageCompileLink.manageCompileLink(showDiagnostics, litter, True,
    compileCBatchFileName, linkCExeBatchFileName, srcFileNameList, getAddressSizeExeName)
  #
  # Find size of memory address used in {fmuSharedLibName}.
  #   Note both the library and {getAddressSizeExeName} were compiled using the
  # same compiler batch file.
  if( showDiagnostics ):
    printDiagnostic('Running utility application {' +getAddressSizeExeName +'}')
  try:
    runList = [os.path.join(os.path.curdir, getAddressSizeExeName)]
    addressSizeProc = subprocess.Popen(runList, stdout=subprocess.PIPE)
    for addressSize in addressSizeProc.stdout:
      True
    if( showDiagnostics ):
      printDiagnostic('FMU shared library {' +fmuSharedLibName +'} has address size {' +addressSize +'}')
    if( addressSize!='32' and addressSize!='64' ):
      quitWithError('Unexpected address size {' +addressSize +'}', False)
    fmuBinDirName = PLATFORM_SHORT_NAME +addressSize
  except:
    # Check failure due to missing {getAddressSizeExeName} before complain about
    # unknown problem.
    getAddressSizeExeName = findFileOrQuit('utility application', getAddressSizeExeName)
    quitWithError('Failed to run utility application {' +getAddressSizeExeName +'}: reason unknown', False)
  #
  # Clean up intermediates.
  if( not litter ):
    if( showDiagnostics ):
      printDiagnostic('Cleaning up intermediate files')
    # deleteFile(modMainName)  # Done above.
    deleteFile(getAddressSizeExeName)
  #
  return( (fmuSharedLibName, fmuBinDirName) )
  #
  # End fcn makeFmuSharedLib().


#--- Run if called from command line.
#
#   If called from command line, force a complete build.
#
#   If called from command line, {__name__} is "__main__".  Otherwise,
# {__name__} is base name of the script file, without ".py".
#
if __name__ == '__main__':
  #
  # Set defaults for command-line options.
  showDiagnostics = False
  litter = False
  #
  # Get command-line options.
  lastIdx = len(sys.argv) - 1
  currIdx = 1
  while( currIdx < lastIdx ):
    currArg = sys.argv[currIdx]
    if( currArg.startswith('-d') ):
      showDiagnostics = True
    elif( currArg.startswith('-L') ):
      litter = True
    else:
      quitWithError('Bad command-line option {' +currArg +'}', True)
    # Here, processed option at {currIdx}.
    currIdx += 1
  #
  # Get {modelIdName}.
  if( currIdx != lastIdx ):
    quitWithError('Require exactly one command-line argument, <model-identifier>', True)
  modelIdName = sys.argv[lastIdx]
  if( showDiagnostics ):
    printDiagnostic('Setting model identifier to {' +modelIdName +'}')
  if( modelIdName.startswith('-') and len(modelIdName)==2 ):
    quitWithError('Expecting model identifier, got what looks like a command-line option {' +modelIdName +'}', True)
  #
  # Run.
  (fmuSharedLibName, fmuBinDirName) = makeFmuSharedLib(showDiagnostics, litter, modelIdName)
  if( showDiagnostics ):
    printDiagnostic('Created shared library {' +fmuSharedLibName +'} for FMU binary subdirectory {' +fmuBinDirName +'}')


#--- Copyright notice.
#
# Functional Mock-up Unit Export of EnergyPlus (C)2013, The Regents of
# the University of California, through Lawrence Berkeley National
# Laboratory (subject to receipt of any required approvals from
# the U.S. Department of Energy). All rights reserved.
#
# If you have questions about your rights to use or distribute this software,
# please contact Berkeley Lab's Technology Transfer Department at
# TTD@lbl.gov.referring to "Functional Mock-up Unit Export
# of EnergyPlus (LBNL Ref 2013-088)".
#
# NOTICE: This software was produced by The Regents of the
# University of California under Contract No. DE-AC02-05CH11231
# with the Department of Energy.
# For 5 years from November 1, 2012, the Government is granted for itself
# and others acting on its behalf a nonexclusive, paid-up, irrevocable
# worldwide license in this data to reproduce, prepare derivative works,
# and perform publicly and display publicly, by or on behalf of the Government.
# There is provision for the possible extension of the term of this license.
# Subsequent to that period or any extension granted, the Government is granted
# for itself and others acting on its behalf a nonexclusive, paid-up, irrevocable
# worldwide license in this data to reproduce, prepare derivative works,
# distribute copies to the public, perform publicly and display publicly,
# and to permit others to do so. The specific term of the license can be identified
# by inquiry made to Lawrence Berkeley National Laboratory or DOE. Neither
# the United States nor the United States Department of Energy, nor any of their employees,
# makes any warranty, express or implied, or assumes any legal liability or responsibility
# for the accuracy, completeness, or usefulness of any data, apparatus, product,
# or process disclosed, or represents that its use would not infringe privately owned rights.
#
#
# Copyright (c) 2013, The Regents of the University of California, Department
# of Energy contract-operators of the Lawrence Berkeley National Laboratory.
# All rights reserved.
#
# 1. Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the copyright notice, this list
# of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the copyright notice, this list
# of conditions and the following disclaimer in the documentation and/or other
# materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
# National Laboratory, U.S. Dept. of Energy nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# 2. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# 3. You are under no obligation whatsoever to provide any bug fixes, patches,
# or upgrades to the features, functionality or performance of the source code
# ("Enhancements") to anyone; however, if you choose to make your Enhancements
# available either publicly, or directly to Lawrence Berkeley National Laboratory,
# without imposing a separate written license agreement for such Enhancements,
# then you hereby grant the following license: a non-exclusive, royalty-free
# perpetual license to install, use, modify, prepare derivative works, incorporate
# into other computer software, distribute, and sublicense such enhancements or
# derivative works thereof, in binary and source code form.
#
# NOTE: This license corresponds to the "revised BSD" or "3-clause BSD"
# License and includes the following modification: Paragraph 3. has been added.
