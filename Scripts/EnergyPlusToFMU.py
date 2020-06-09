#!/usr/bin/env  python


#--- Purpose.
#
#   Export an EnergyPlus model as a Functional Mockup Unit (FMU) for co-simulation.


#--- Note on directory location.
#
#   This script uses relative paths to locate some of the files it needs.
# Therefore it should not be moved from its default directory.
#   However, this script can be run from a different working directory.


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
# >>> <this-file-base-name>.exportEnergyPlusAsFMU(arguments)


#--- Runtime help.
#
def printCmdLineUsage():
  #
  print 'USAGE:', os.path.basename(__file__),  \
    '-i <path-to-idd-file>  [-w <path-to-weather-file>]  [-a <fmi-version>] [-d]  [-L]  <path-to-idf-file>'
  #
  print '-- Export an EnergyPlus model as a Functional Mockup Unit (FMU) for co-simulation'
  print '-- Input -i, use the named Input Data Dictionary (required)'
  print '-- Option -w, use the named weather file'
  print '-- Option -a, specify the FMI version'
  print '-- Option -d, print diagnostics'
  print '-- Option -L, litter, that is, do not clean up intermediate files'
  # TODO: Add -V to set version number of FMI standard.  Currently 1.0 is only one supported.
  #
  # End fcn printCmdLineUsage().


#--- Ensure access.
#
import os
import subprocess
import sys
import zipfile

PLATFORM_NAME = sys.platform

#
if( PLATFORM_NAME.startswith('win') ):
    PLATFORM_SHORT_NAME = 'win'
elif( PLATFORM_NAME.startswith('linux')
    or PLATFORM_NAME.startswith('cygwin') ):
    PLATFORM_SHORT_NAME = 'linux'
elif( PLATFORM_NAME.startswith('darwin') ):
    PLATFORM_SHORT_NAME = 'darwin'
else:
    raise Exception('Unknown platform {' +PLATFORM_NAME +'}')

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


#--- Fcn to add a file to a zip file.
#
def addToZipFile(theZipFile, addFileName, toDir, addAsName):
  #
  # Get path and name for use in zip file.
  if( addAsName is None ):
    addAsName = os.path.basename(addFileName)
  if( toDir is not None ):
    addAsName = os.path.join(toDir, addAsName)
  #
  try:
    theZipFile.write(addFileName, addAsName)
  except:
    # Diagnose error if possible.
    if( theZipFile.__class__ != zipfile.ZipFile ):
      quitWithError('Expecting a zip file, got {' +theZipFile.__class__.__name__ +'}', False)
    # Here, {theZipFile} is a zip file.  Won't be using it any further.
    theZipFile.close()
    # Check whether {addFileName} exists.
    addFileName = findFileOrQuit('zip member', addFileName)
    # Here, {addFileName} is a good file.
    quitWithError('Failed to add file {' +addFileName +'} to zip file; reason unknown', False)
  #
  # Here, successfully added {addFileName} to {theZipFile}.
  #
  # End fcn addToZipFile().


#--- Fcn to export an EnergyPlus IDF file as an FMU.
#
def exportEnergyPlusAsFMU(showDiagnostics, litter, iddFileName, wthFileName, fmiVersion, idfFileName):
  #
  if( showDiagnostics ):
    printDiagnostic('Begin exporting IDF file {' +idfFileName +'} as an FMU')
  #
  # Check file names passed as arguments, and get absolute paths.
  idfFileName = findFileOrQuit('IDF', idfFileName)
  iddFileName = findFileOrQuit('IDD', iddFileName)
  if( wthFileName is None ):
    if( showDiagnostics ):
      printDiagnostic('Note no WTH file given')
  else:
    wthFileName = findFileOrQuit('WTH', wthFileName)
  #
  # Get directory of this script file.
  scriptDirName = os.path.abspath(os.path.dirname(__file__))
  #
  # Load modules expect to find in same directory as this script file.
  if( scriptDirName not in sys.path ):
    sys.path.append(scriptDirName)
  #
  findFileOrQuit('utility script', os.path.join(scriptDirName, 'makeFMULib.py'))
  try:
    import makeFMULib
  except:
    quitWithError('Unable to import {makeFMULib.py}', False)
  #
  findFileOrQuit('utility script', os.path.join(scriptDirName, 'makeExportPrepApp.py'))
  try:
    import makeExportPrepApp
  except:
    quitWithError('Unable to import {makeExportPrepApp.py}', False)
  #
  # Get valid model identifier.
  modelIdName = os.path.basename(idfFileName)
  if( modelIdName.endswith('.idf') or modelIdName.endswith('.IDF') ):
    modelIdName = modelIdName[:-4]
  modelIdName = makeFMULib.sanitizeIdentifier(modelIdName)
  if( showDiagnostics ):
    printDiagnostic('Using model identifier {' +modelIdName +'}')
  #
  # Delete expected outputs if they already exist.
  #   To prevent confusion in case of an error.
  OUT_modelDescFileName = 'modelDescription.xml'
  deleteFile(OUT_modelDescFileName)
  #
  OUT_variablesFileName = 'variables.cfg'
  deleteFile(OUT_variablesFileName)
  #
  OUT_workZipFileName = modelIdName +'.zip'
  deleteFile(OUT_workZipFileName)
  #
  OUT_fmuFileName = modelIdName +'.fmu'
  deleteFile(OUT_fmuFileName)
  #
  # Create export-prep application.
  #   The resulting executable will extract FMU-related information from an
  # EnergyPlus IDF file.
  #   Do not force a rebuild.
  if( showDiagnostics ):
    printDiagnostic('Checking for export-prep application')
  exportPrepExeName = makeExportPrepApp.makeExportPrepApp(showDiagnostics, litter, True, fmiVersion)
  #
  # Run the export-prep application.
  if( showDiagnostics ):
    printDiagnostic('Running export-prep application {' +exportPrepExeName +'}')
  runList = [os.path.join(os.path.curdir, exportPrepExeName)]
  if( wthFileName is not None ):
    runList.extend(['-w', wthFileName])
  runList.extend([iddFileName, idfFileName])
  subprocess.call(runList)
  if( (not os.path.isfile(OUT_modelDescFileName)) or (not os.path.isfile(OUT_variablesFileName)) ):
    quitWithError('Failed to extract FMU information from IDF file {' +idfFileName +'}', False)
  #
  # Create the shared library.
  (OUT_fmuSharedLibName, fmuBinDirName) = makeFMULib.makeFmuSharedLib(showDiagnostics, litter, modelIdName, fmiVersion)
  findFileOrQuit('shared library', OUT_fmuSharedLibName)
  #
  # Create zip file that will become the FMU.
  #   Note to get compression, need zlib, but can proceed without it.
  try:
    import zlib
    if( showDiagnostics ):
      printDiagnostic('Creating zip file {' +OUT_workZipFileName +'}, with compression on')
    workZipFile = zipfile.ZipFile(OUT_workZipFileName, 'w', zipfile.ZIP_DEFLATED)
  except:
    # Here, either didn't find zlib, or couldn't create zip file.
    if( showDiagnostics ):
      printDiagnostic('Creating zip file {' +OUT_workZipFileName +'}, without compression')
    try:
      workZipFile = zipfile.ZipFile(OUT_workZipFileName, 'w', zipfile.ZIP_STORED)
    except:
      quitWithError('Failed to create zip file {' +OUT_workZipFileName +'}', False)
  #
  # Populate zip file.
  #   Note fcn addToZipFile() closes the zip file if it encounters an error.
  addToZipFile(workZipFile, OUT_modelDescFileName, None, None)
  addToZipFile(workZipFile, idfFileName, 'resources', modelIdName+'.idf')
  addToZipFile(workZipFile, OUT_variablesFileName, 'resources', None)
  addToZipFile(workZipFile, iddFileName, 'resources', None)
  addToZipFile(workZipFile, exportPrepExeName, 'resources', None)
  if( wthFileName is not None ):
    addToZipFile(workZipFile, wthFileName, 'resources', None)
  addToZipFile(workZipFile, OUT_fmuSharedLibName, os.path.join('binaries',fmuBinDirName), None)
  #
  # Finish up zip file.
  if( showDiagnostics ):
    printDiagnostic('Renaming completed zip file {' +OUT_workZipFileName +'} to {' +OUT_fmuFileName +'}')
  workZipFile.close()
  findFileOrQuit('zip', OUT_workZipFileName)
  os.rename(OUT_workZipFileName, OUT_fmuFileName)
  #
  # Clean up intermediates.
  if( not litter ):
    if( showDiagnostics ):
      printDiagnostic('Cleaning up intermediate files')
    # deleteFile(exportPrepExeName)  # Keep this executable, since it does not vary from run to run (i.e., not really intermediate).
    deleteFile(OUT_modelDescFileName)
    deleteFile(OUT_variablesFileName)
    deleteFile(OUT_fmuSharedLibName)
  #
  # End fcn exportEnergyPlusAsFMU().


#--- Run if called from command line.
#
#   If called from command line, {__name__} is "__main__".  Otherwise,
# {__name__} is base name of the script file, without ".py".
#
if __name__ == '__main__':
  #
  # Set defaults for command-line options.
  iddFileName = None
  wthFileName = None
  fmiVersion = None
  showDiagnostics = False
  litter = False
  #
  # Get command-line options.
  lastIdx = len(sys.argv) - 1
  currIdx = 1
  while( currIdx < lastIdx ):
    currArg = sys.argv[currIdx]
    if( currArg.startswith('-i') ):
      currIdx += 1
      iddFileName = sys.argv[currIdx]
      if( showDiagnostics ):
        printDiagnostic('Setting IDD file to {' +iddFileName +'}')
    elif( currArg.startswith('-w') ):
      currIdx += 1
      wthFileName = sys.argv[currIdx]
      if( showDiagnostics ):
        printDiagnostic('Setting WTH file to {' +wthFileName +'}')
    elif( currArg.startswith('-a') ):
      currIdx += 1
      fmiVersion = sys.argv[currIdx]
      if( showDiagnostics ):
        printDiagnostic('Setting FMI API version (1 or 2) to {' + fmiVersion + '}')
    elif( currArg.startswith('-d') ):
      showDiagnostics = True
    elif( currArg.startswith('-L') ):
      litter = True
    else:
      quitWithError('Bad command-line option {' +currArg +'}', True)
    # Here, processed option at {currIdx}.
    currIdx += 1
  #
  # Get {idfFileName}.
  if( currIdx != lastIdx ):
    # Here, either an option like {-i} consumed the entry at {lastIdx}, or had
    # no options or arguments at all.
    quitWithError('Require exactly one command-line argument, <path-to-idf-file>', True)
  idfFileName = sys.argv[lastIdx]
  if( showDiagnostics ):
    printDiagnostic('Setting IDF file to {' +idfFileName +'}')
  if( idfFileName.startswith('-') and len(idfFileName)==2 ):
    quitWithError('Expecting IDF file name, got what looks like a command-line option {' +idfFileName +'}', True)
  #
  # Get {iddFileName}.
  if( iddFileName is None ):
    quitWithError('Missing required input, <path-to-idd-file>', True)
  # Get {FMI version}.
  if( fmiVersion is None ):
      fmiVersion = "1.0"
      printDiagnostic('FMI version is unspecified. It will be set to {' +fmiVersion +'}')
  if not (fmiVersion in [1, 2, "1", "2", "1.0", "2.0"]):
      quitWithError('FMI version "1" and "2" are supported, got FMI version {' +fmiVersion +'}', True)
  if (int(float(fmiVersion))==2):
      import struct
      nbits=8 * struct.calcsize("P")
      ops=PLATFORM_SHORT_NAME+str(nbits)
      if( PLATFORM_NAME.startswith('lin') and str(nbits)=='64'):
          dirname, filename = os.path.split(os.path.abspath(__file__))
          incLinkerLibs = os.path.join(dirname, "..", "SourceCode", "v20",
            "fmusdk-shared", "parser", ops, "libxml2.so.2")
          printDiagnostic('\nIMPORTANT NOTE: The FMU generated will run in the fmuChecker 2.0.4 only '
          'if libxml2.so.2 is symbollicaly link to  {' +incLinkerLibs +'}.\n'
          ' This version of libxml2.so.2 has been compiled excluding zlib.'
          ' The official released version of libxml2.so.2 (version 2.9) '
          ' which includes zlib causes the FMU to fail in the fmuChecker.\n'
          ' However, the FMU will work fine with master algorithms'
          ' such as PyFMI even if the FMU links to the official version of libxml2.\n')
      if( PLATFORM_NAME.startswith('lin') and str(nbits)=='32'):
          quitWithError('FMI version 2.0 for Co-Simulation is not supported on {' +ops +'}', False)
      #if( PLATFORM_NAME.startswith('darwin')):
    #      quitWithError('FMI version 2.0 for Co-Simulation is not supported on {' +ops +'}', False)

  # Run.
  exportEnergyPlusAsFMU(showDiagnostics, litter, iddFileName, wthFileName, int(float(fmiVersion)), idfFileName)


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
