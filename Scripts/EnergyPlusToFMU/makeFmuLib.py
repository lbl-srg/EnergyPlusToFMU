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


#--- Note on compile and link batch files.
#
#   This script uses separate, system-dependent, batch files to compile and
# link source code.  For more information, see fcns printCompileCBatchInfo(),
# printLinkCLibBatchInfo(), and printLinkCExeBatchInfo().
#
g_compileCBatchFileName = 'compile-c.bat'
g_linkCLibBatchFileName = 'link-c-lib.bat'
g_linkCExeBatchFileName = 'link-c-exe.bat'


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
  print 'Require a batch file {' +g_compileCBatchFileName +'}'
  print '-- The batch file should compile C source code files'
  print '-- The batch file should accept one argument, the name (including path) of the source code file to compile'
  print '-- The batch file should leave the resulting object file in the working directory'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printCompileCBatchInfo().


def printLinkCLibBatchInfo():
  #
  print 'Require a batch file {' +g_linkCLibBatchFileName +'}'
  print '-- The batch file should link object files compiled via ' +g_compileCBatchFileName
  print '-- The batch file should produce a shared library'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output shared library'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCLibBatchInfo().


def printLinkCExeBatchInfo():
  #
  print 'Require a batch file {' +g_linkCExeBatchFileName +'}'
  print '-- The batch file should link object files compiled via ' +g_compileCBatchFileName
  print '-- The batch file should produce a command-line executable'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output executable'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCExeBatchInfo().


#--- Ensure access.
#
import os
import re
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
  # Replace all illegal characters.
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
def makeFmuSharedLib(showDiagnostics, litter, modelIdName):
  #
  if( showDiagnostics ):
    printDiagnostic('Begin creating shared FMU library for model {' +modelIdName +'}')
  #
  # Sanitize {modelIdName}.
  modelIdSanitizedName = sanitizeIdentifier(modelIdName)
  if( showDiagnostics and (modelIdSanitizedName != modelIdName) ):
    printDiagnostic('Converting model identifier from {' +modelIdName +'} to {' +modelIdSanitizedName +'}')
  #
  # Set working directory to same directory as this script file.
  #   To allow using relative paths, including import of other modules.
  origWorkDirName = os.path.abspath(os.getcwd())
  scriptDirName = os.path.abspath(os.path.dirname(__file__))
  if( scriptDirName != origWorkDirName ):
    if( showDiagnostics ):
      printDiagnostic('Jumping to script directory {' +scriptDirName +'}')
    os.chdir(scriptDirName)
  #
  # Choose system-specific values.
  platformName = sys.platform
  systemBatchDirName = None
  fmuSharedLibName = None
  #
  if( platformName.startswith('win') ):
    platformName = 'win'
    systemBatchDirName = 'batch-dos'
    fmuSharedLibName = modelIdSanitizedName +'.dll'
  elif( platformName.startswith('linux')
    or platformName.startswith('cygwin') ):
    platformName = 'linux'
    systemBatchDirName = 'batch-linux'
    fmuSharedLibName = modelIdSanitizedName +'.so'
  elif( platformName.startswith('darwin') ):
    platformName = 'darwin'
    systemBatchDirName = 'batch-darwin'
    fmuSharedLibName = modelIdSanitizedName +'.dylib'
  else:
    quitWithError('Unknown platform {' +platformName +'}', False)
  #
  if( showDiagnostics ):
    printDiagnostic('Using system-specific scripts from batch directory {' +systemBatchDirName +'}')
  if( not os.path.isdir(systemBatchDirName) ):
    quitWithError('Missing system-specific batch directory {' +os.path.join(scriptDirName, systemBatchDirName) +'}', False)
  #
  # Form names of system-specific scripts.
  compileCBatchFileName = os.path.join(systemBatchDirName, g_compileCBatchFileName)
  findFileOrQuit('compiler batch', compileCBatchFileName)
  #
  linkCLibBatchFileName = os.path.join(systemBatchDirName, g_linkCLibBatchFileName)
  findFileOrQuit('linker batch', linkCLibBatchFileName)
  #
  linkCExeBatchFileName = os.path.join(systemBatchDirName, g_linkCExeBatchFileName)
  findFileOrQuit('linker batch', linkCExeBatchFileName)
  #
  # Insert model identifier into source code files.
  modMainName = '../../SourceCode/EnergyPlus/temp-' +modelIdSanitizedName +'.c'
  poundDefineModelId(showDiagnostics, '../../SourceCode/EnergyPlus/main.c', modelIdSanitizedName, modMainName)
  #
  # Assemble names of source files.
  srcFileNameList = list()
  #
  srcFileNameList.append(modMainName)
  #
  srcDirName = '../../SourceCode/EnergyPlus'
  for theRootName in ['reader',
    'stack',
    'util',
    'utilSocket',
    'xml_parser_cosim'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.c'))
  #
  srcDirName = '../../SourceCode/Expat/lib'
  for theRootName in ['xmlparse',
    'xmlrole',
    'xmltok'  # Note {xmltok.c} directly #includes {xmltok_impl.c} and {xmltok_ns.c}, so they don't need to be in this list.
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.c'))
  #
  # Access scripts from {utilManageCompileLink}.
  #   Note deferred this import until have error-reporting mechanism in place,
  # and until made sure were in the expected directory.
  findFileOrQuit('utility script', 'utilManageCompileLink.py')
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
  srcFileNameList = ['../../SourceCode/utility-src/get-address-size.c']
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
    fmuBinDirName = platformName +addressSize
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
  # Jump back to starting directory.
  if( scriptDirName != origWorkDirName ):
    if( showDiagnostics ):
      printDiagnostic('Jumping back to original directory {' +origWorkDirName +'}')
    os.chdir(origWorkDirName)
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
