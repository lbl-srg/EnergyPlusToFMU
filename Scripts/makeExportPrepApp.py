#!/usr/bin/env  python


#--- Purpose.
#
#   Create an executable that reads an EnergyPlus IDF file, and extracts the
# information needed to export the simulation as a Functional Mockup Unit (FMU)
# for co-simulation.


#--- Note on directory location.
#
#   This script uses relative paths to locate some of the files it needs.
# Therefore it should not be moved from its default directory.
#   However, this script can be run from a different working directory.


#--- Ensure access.
#
import os
import sys


#--- Identify system.
#
PLATFORM_NAME = sys.platform
#
if( PLATFORM_NAME.startswith('win') ):
    PLATFORM_SHORT_NAME = 'win'
    BATCH_EXTENSION = '.bat'
elif( PLATFORM_NAME.startswith('linux')
    or PLATFORM_NAME.startswith('cygwin') ):
    PLATFORM_SHORT_NAME = 'linux'
    BATCH_EXTENSION = '.sh'
elif( PLATFORM_NAME.startswith('darwin') ):
    PLATFORM_SHORT_NAME = 'darwin'
    BATCH_EXTENSION = '.sh'
else:
    raise Exception('Unknown platform {' +PLATFORM_NAME +'}')


#--- Note on compile and link batch files.
#
#   This script uses separate, system-dependent, batch files to compile and
# link source code.  For more information, see fcns printCompileCppBatchInfo()
# and printLinkCppExeBatchInfo() below.
#
COMPILE_CPP_BATCH_FILE_NAME = 'compile-cpp' + BATCH_EXTENSION
LINK_CPP_EXE_BATCH_FILE_NAME = 'link-cpp-exe' + BATCH_EXTENSION


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
# >>> <this-file-base-name>.makeExportPrepApp(arguments)


#--- Runtime help.
#
def printCmdLineUsage():
  #
  print 'USAGE:', os.path.basename(__file__),  \
    '[-d]  [-L]'
  #
  print '-- Create an executable that extracts FMU-related information from an EnergyPlus IDF file'
  print '-- Option -d, print diagnostics'
  print '-- Option -L, litter, that is, do not clean up intermediate files'
  #
  print
  printCompileCppBatchInfo()
  #
  print
  printLinkCppExeBatchInfo()
  #
  # End fcn printCmdLineUsage().


def printCompileCppBatchInfo():
  #
  print 'Require a batch file {' +COMPILE_CPP_BATCH_FILE_NAME +'}'
  print '-- The batch file should compile C++ source code files'
  print '-- The batch file should accept one argument, the name (including path) of the source code file to compile'
  print '-- The batch file should leave the resulting object file in the working directory'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printCompileCppBatchInfo().


def printLinkCppExeBatchInfo():
  #
  print 'Require a batch file {' +LINK_CPP_EXE_BATCH_FILE_NAME +'}'
  print '-- The batch file should link object files compiled via ' +COMPILE_CPP_BATCH_FILE_NAME
  print '-- The batch file should produce a command-line executable'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output executable'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCppExeBatchInfo().


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
  if( os.path.isfile(fileName) ):
    try:
      os.remove(fileName)
    except:
      quitWithError('Unable to delete file {' +fileName +'}', False)
  elif( os.path.isdir(fileName) ):
    quitWithError('Expecting {' +fileName +'} to be a file; found a directory', False)
  #
  # End fcn deleteFile().


#--- Fcn to create export-prep executable.
#
#   The resulting executable extracts FMU information from an EnergyPlus IDF file.
#
#   Note this fcn doesn't escape characters, such as spaces, in directory and
# file names.  It uses paths relative to the directory where this file resides,
# and relies on there being no problematic characters in the path names.
#
def makeExportPrepApp(showDiagnostics, litter, forceRebuild):
  #
  # Form executable name.
  #   Make name distinct on each platform, in order to allow two executables to
  # co-exist when running a virtual machine.
  exportPrepExeName = 'idf-to-fmu-export-prep-' + PLATFORM_SHORT_NAME
  # Add file extension for Windows/DOS.
  if( PLATFORM_SHORT_NAME == 'win' ):
    exportPrepExeName = exportPrepExeName +'.exe'
  if( showDiagnostics ):
    printDiagnostic('Begin creating executable {' +exportPrepExeName +'}')
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
  compileCppBatchFileName = os.path.join(batchDirAbsName, COMPILE_CPP_BATCH_FILE_NAME)
  findFileOrQuit('compiler batch', compileCppBatchFileName)
  #
  linkCppExeBatchFileName = os.path.join(batchDirAbsName, LINK_CPP_EXE_BATCH_FILE_NAME)
  findFileOrQuit('linker batch', linkCppExeBatchFileName)
  #
  # Assemble names of source files.
  srcFileNameList = list()
  #
  srcDirName = os.path.join(scriptDirName, '../SourceCode/fmu-export-prep')
  for theRootName in ['app-cmdln-input',
    'app-cmdln-version',
    'fmu-export-idf-data',
    'fmu-export-write-model-desc',
    'fmu-export-write-vars-cfg',
    'fmu-export-prep-main'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
  #
  srcDirName = os.path.join(scriptDirName, '../SourceCode/read-ep-file')
  for theRootName in ['ep-idd-map',
    'fileReader',
    'fileReaderData',
    'fileReaderDictionary'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
  #
  srcDirName = os.path.join(scriptDirName, '../SourceCode/utility')
  for theRootName in ['digest-md5',
    'file-help',
    'string-help',
    'time-help',
    'utilReport',
    'xml-output-help'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
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
  # Build executable.
  utilManageCompileLink.manageCompileLink(showDiagnostics, litter, forceRebuild,
    compileCppBatchFileName, linkCppExeBatchFileName, srcFileNameList, exportPrepExeName)
  #
  # Clean up intermediates.
  #   Nothing to do-- no intermediates generated at this level of work.
  #
  return( exportPrepExeName )
  #
  # End fcn makeExportPrepApp().


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
  while( currIdx <= lastIdx ):
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
  # Run.
  exportPrepExeName = makeExportPrepApp(showDiagnostics, litter, True)


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
