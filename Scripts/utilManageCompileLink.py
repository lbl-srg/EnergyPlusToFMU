#!/usr/bin/env  python


#--- Purpose.
#
#   Manage a build process based on compiling and linking a bunch of object files.
#   Specifically, compile a number of source code files, and link the resulting
# object files into an executable or dynamic link library.


#--- Note on directory location.
#
#   This script can be run from any working directory, and does not need to
# reside in that working directory.
#   The only requirement is that the file names passed to the script must
# include the correct path to the files.


#--- Note on compile and link batch files.
#
#   This script uses separate, system-dependent, batch files to compile and
# link source code.  For more information, see fcns printCompileBatchInfo()
# and printLinkBatchInfo() below.


#--- Running this script.
#
#   To call this script from the Python interpreter, or from another Python script:
# >>> import <this-file-base-name>
# >>> <this-file-base-name>.manageCompileLink(arguments)


#--- Runtime help.
#
def printCompileBatchInfo(compileBatchFileName):
  #
  print 'Require a batch file {' +compileBatchFileName +'}'
  print '-- The batch file should compile the source code files of interest'
  print '-- The batch file should accept one argument, the name (including path) of the source code file to compile'
  print '-- The batch file should leave the resulting object file in the working directory'
  #
  # End fcn printCompileBatchInfo().


def printLinkBatchInfo(linkBatchFileName, compileBatchFileName):
  #
  print 'Require a batch file {' +linkBatchFileName +'}'
  print '-- The batch file should link object files compiled via ' +compileBatchFileName
  print '-- The batch file should produce a command-line executable or a library'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output executable or library'
  print '  ** the name(s) of the object files to link'
  #
  # End fcn printLinkBatchInfo().


#--- Ensure access.
#
import os
import subprocess
import sys


#--- Fcn to print diagnostics.
#
def printDiagnostic(messageStr):
  #
  print '!', os.path.basename(__file__), '--', messageStr
  #
  # End fcn printDiagnostic().


#--- Fcn to quit due to an error.
#
def quitWithError(messageStr):
  #
  print 'ERROR from script file {' +os.path.basename(__file__) +'}'
  #
  if( messageStr is not None ):
    print messageStr
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
      quitWithError('Missing directory {' +dirName +'} for ' +fileDesc +' file {' +fileName +'}')
    quitWithError('Missing ' +fileDesc +' file {' +fileName +'} in directory {' +dirName +'}')
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
      quitWithError('Unable to delete file {' +fileName +'}')
  elif( os.path.isdir(fileName) ):
    quitWithError('Expecting {' +fileName +'} to be a file; found a directory')
  #
  # End fcn deleteFile().


#--- Fcn to create a directory if necessary.
#
#   Return {True} if directory already exists.
#
def ensureDir(showDiagnostics, dirDesc, dirName):
  #
  if( os.path.isdir(dirName) ):
    return( True )
  #
  if( os.path.isfile(dirName) ):
    quitWithError('Expecting {' +dirName +'} to be a directory; found a file')
  #
  if( showDiagnostics ):
    printDiagnostic('Creating ' +dirDesc +' directory {' +dirName +'}')
  try:
    os.mkdir(dirName)
  except:
    quitWithError('Unable to create ' +dirDesc +' directory {' +dirName +'}')
  #
  return( False )
  #
  # End fcn ensureDir().


#--- Fcn to clean up an existing directory, or create it.
#
def ensureCleanDir(showDiagnostics, dirDesc, dirName):
  #
  if( ensureDir(showDiagnostics, dirDesc, dirName) ):
    # Here, {dirName} already existed; need to clean it up.
    if( showDiagnostics ):
      printDiagnostic('Cleaning up existing ' +dirDesc +' directory {' +dirName +'}')
    for theEntryName in os.listdir(dirName):
      theEntryFullName = os.path.abspath(os.path.join(dirName, theEntryName))
      deleteFile(theEntryFullName)
  #
  # End fcn ensureCleanDir().


#--- Fcn to remove an existing directory.
#
#   Assume the directory has no subdirectories.
#
def deleteDir(showDiagnostics, dirDesc, dirName):
  #
  if( showDiagnostics ):
    printDiagnostic('Removing ' +dirDesc +' directory {' +dirName +'}')
  #
  if( not os.path.isdir(dirName) ):
    quitWithError('Expecting a directory called {' +dirName +'}')
  #
  # Remove files.
  for theEntryName in os.listdir(dirName):
    theEntryFullName = os.path.abspath(os.path.join(dirName, theEntryName))
    deleteFile(theEntryFullName)
  #
  # Remove directory.
  try:
    os.rmdir(dirName)
  except:
    quitWithError('Unable to delete directory {' +dirName +'}')
  #
  # End fcn deleteDir().


#--- Fcn to compile a source code file.
#
#   Perform work in the current working directory.
#   Return object file name.
#
def runCompiler(showDiagnostics, compileBatchFileName, srcFileName):
  #
  if( showDiagnostics ):
    printDiagnostic('Compiling {' +os.path.basename(srcFileName) +'}')
  #
  try:
    subprocess.call([compileBatchFileName, srcFileName])
  except:
    # Check failure due to missing files, before complain about unknown problem.
    srcFileName = findFileOrQuit('source', srcFileName)
    compileBatchFileName = findFileOrQuit('compiler batch', compileBatchFileName)
    quitWithError('Failed to run compiler batch file {' +compileBatchFileName +
      '} on source code file {' +srcFileName +'}: reason unknown')
  #
  # Check made an object file.
  (objFileBaseName, ext) = os.path.splitext(os.path.basename(srcFileName))
  objFileName = objFileBaseName +'.obj'
  if( not os.path.isfile(objFileName) ):
    objFileName = objFileBaseName +'.o'
    if( not os.path.isfile(objFileName) ):
      quitWithError('Failed to create object file for source code file {' +srcFileName +'}')
  #
  return( objFileName )
  #
  # End fcn runCompiler().


#--- Fcn to manage a compile-link build process.
#
#   Note this fcn doesn't escape characters, such as spaces, in directory and
# file names.  The caller must take care of these issues.
#
def manageCompileLink(showDiagnostics, litter, forceRebuild,
  compileBatchFileName, linkBatchFileName, srcFileNameList, outputFileName):
  #
  if( showDiagnostics ):
    printDiagnostic('Begin compile-link build of {' +outputFileName +'}')
  #
  # Short-circuit work if possible.
  #   TODO: Consider being make-like, by only rebuilding object files that are old
  # compared to sources, and only rebuilding output that is old compared to
  # object files.
  if(  os.path.isfile(outputFileName) and (not forceRebuild) ):
    if( showDiagnostics ):
      printDiagnostic('Output file {' +outputFileName + '} already exists; doing nothing')
    return
  #
  # Delete expected output if it already exists.
  #   To prevent confusion in case of an error.
  deleteFile(outputFileName)
  #
  # Get absolute paths of all inputs, prior to changing directory.
  #   Note for strings, which are immutable, can re-assign to same name without
  # affecting caller.  However, for lists, need to assign to a new variable.
  compileBatchFileName = findFileOrQuit('compiler batch', compileBatchFileName)
  linkBatchFileName = findFileOrQuit('linker batch', linkBatchFileName)  # TODO: Figure out a way to dump the printLinkBatchInfo() information.
  srcFileAbsNameList = list()
  for srcFileName in srcFileNameList:
    srcFileAbsNameList.append(findFileOrQuit('source',srcFileName))
  outputFileName = os.path.abspath(outputFileName)
  #
  # Name the build directory.
  #   Put it in the caller's current working directory (which may differ from
  # both the directory for {outputFileName}, and the directory where this
  # script file resides).
  bldDirName = 'bld-' +os.path.basename(outputFileName).replace('.', '-')
  #
  # Create or clean the build directory.
  ensureCleanDir(showDiagnostics, 'build', bldDirName)
  #
  # Jump to build directory.
  #   Want all compiler and linker output in the build directory, in order to
  # keep things organized, and to be able to do a complete cleanup.
  if( showDiagnostics ):
    printDiagnostic('Jumping to build directory {' +bldDirName +'}')
  origDirName = os.path.abspath(os.getcwd())
  bldDirName = os.path.abspath(bldDirName)
  os.chdir(bldDirName)
  #
  # Compile sources.
  if( showDiagnostics ):
    printDiagnostic('Compiling files using {' +compileBatchFileName +'}')
  objFileNameList = list()
  for srcFileName in srcFileAbsNameList:
    objFileName = runCompiler(showDiagnostics, compileBatchFileName, srcFileName)
    objFileNameList.append(os.path.join(bldDirName, objFileName))
  #
  # Link objects into {outputFileName}.
  #   But keep it in the build directory.
  outputFileBaseName = os.path.basename(outputFileName)
  if( showDiagnostics ):
    printDiagnostic('Linking object files using {' +linkBatchFileName +'}')
    printDiagnostic('Linking to create {' +outputFileBaseName +'}')
  subprocess.call([linkBatchFileName, outputFileBaseName] +objFileNameList)
  if( not os.path.isfile(outputFileBaseName) ):
    quitWithError('Failed to link object files into {' +outputFileBaseName +'}')
  #
  # Move output file to destination directory.
  try:
    if( showDiagnostics ):
      printDiagnostic('Moving output file {' +outputFileBaseName +'} to directory {' +os.path.dirname(outputFileName) +'}')
    os.rename(outputFileBaseName, outputFileName)
  except:
    quitWithError('Unable to move output file {' +outputFileBaseName +'} to directory {' +os.path.dirname(outputFileName) +'}: reason unknown')
  #
  # Return to original directory.
  os.chdir(origDirName)
  #
  # Clean up intermediates.
  if( not litter ):
    if( showDiagnostics ):
      printDiagnostic('Cleaning up intermediate files')
    deleteDir(showDiagnostics, 'build', bldDirName)
  #
  return( outputFileName )
  #
  # End fcn manageCompileLink().


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
