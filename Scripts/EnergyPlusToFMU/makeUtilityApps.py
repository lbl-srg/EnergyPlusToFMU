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


#--- Note on compile and link batch files.
#
#   This script uses separate, system-dependent, batch files to compile and
# link source code.  For more information, see fcns printCompileCppBatchInfo()
# and printLinkCppExeBatchInfo() below.
#
g_compileCppBatchFileName = 'compile-cpp.bat'
g_linkCppExeBatchFileName = 'link-cpp-exe.bat'


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
  print 'Require a batch file {' +g_compileCppBatchFileName +'}'
  print '-- The batch file should compile C++ source code files'
  print '-- The batch file should accept one argument, the name (including path) of the source code file to compile'
  print '-- The batch file should leave the resulting object file in the working directory'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printCompileCppBatchInfo().


def printLinkCppExeBatchInfo():
  #
  print 'Require a batch file {' +g_linkCppExeBatchFileName +'}'
  print '-- The batch file should link object files compiled via ' +g_compileCppBatchFileName
  print '-- The batch file should produce a command-line executable'
  print '-- The batch file should accept at least two arguments, in this order:'
  print '  ** the name of the output executable'
  print '  ** the name(s) of the object files to link'
  print '-- Place the batch file in the system-specific batch directory'
  #
  # End fcn printLinkCppExeBatchInfo().


#--- Ensure access.
#
import os
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


#--- Fcn to create export-prep executable.
#
#   The resulting executable extracts FMU information from an EnergyPlus IDF file.
#
#   Note this fcn doesn't escape characters, such as spaces, in directory and
# file names.  It uses paths relative to the directory where this file resides,
# and relies on there being no problematic characters in the path names.
#
def makeExportPrepApp(showDiagnostics, litter, forceRebuild, exportPrepExeName):
  #
  # Form executable name.
  if( exportPrepExeName is None ):
    exportPrepExeName = 'idf-to-fmu-export-prep'
    # Choose different extension for Windows/DOS.  This is a convenience, to
    # allow two executables to stay in place when running in a virtual machine.
    if( sys.platform.startswith('win') ):
      exportPrepExeName = exportPrepExeName +'.exe'
    else:
      exportPrepExeName = exportPrepExeName +'.app'
  if( showDiagnostics ):
    printDiagnostic('Begin creating executable {' +exportPrepExeName +'}')
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
  # Choose subdirectory for system-specific scripts.
  systemBatchDirName = sys.platform
  if( systemBatchDirName.startswith('win') ):
    systemBatchDirName = 'batch-dos'
  elif( systemBatchDirName.startswith('linux')
    or systemBatchDirName.startswith('cygwin') ):
    systemBatchDirName = 'batch-linux'
  elif( systemBatchDirName.startswith('darwin')
    or systemBatchDirName.startswith('freebsd') ):
    systemBatchDirName = 'batch-macos'
  else:
    quitWithError('Unknown platform {' +systemBatchDirName +'}', False)
  #
  if( showDiagnostics ):
    printDiagnostic('Using system-specific scripts from batch directory {' +systemBatchDirName +'}')
  if( not os.path.isdir(systemBatchDirName) ):
    quitWithError('Missing system-specific batch directory {' +os.path.join(scriptDirName, systemBatchDirName) +'}', False)
  #
  # Form names of system-specific scripts.
  compileCppBatchFileName = os.path.join(systemBatchDirName, g_compileCppBatchFileName)
  linkCppExeBatchFileName = os.path.join(systemBatchDirName, g_linkCppExeBatchFileName)
  findFileOrQuit('compiler batch', compileCppBatchFileName)
  findFileOrQuit('linker batch', linkCppExeBatchFileName)
  #
  # Assemble names of source files.
  srcFileNameList = list()
  #
  srcDirName = '../../SourceCode/fmu-export-prep-src'
  for theRootName in ['app-cmdln-input',
    'app-cmdln-version',
    'fmu-export-idf-data',
    'fmu-export-write-model-desc',
    'fmu-export-write-vars-cfg',
    'fmu-export-prep-main'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
  #
  srcDirName = '../../SourceCode/read-ep-file-src'
  for theRootName in ['ep-idd-map',
    'fileReader',
    'fileReaderData',
    'fileReaderDictionary'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
  #
  srcDirName = '../../SourceCode/utility-src'
  for theRootName in ['digest-md5',
    'file-help',
    'string-help',
    'time-help',
    'utilReport',
    'xml-output-help'
    ]:
    srcFileNameList.append(os.path.join(srcDirName, theRootName +'.cpp'))
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
  # Build executable.
  utilManageCompileLink.manageCompileLink(showDiagnostics, litter, forceRebuild,
    compileCppBatchFileName, linkCppExeBatchFileName, srcFileNameList, exportPrepExeName)
  #
  # Clean up intermediates.
  #   Nothing to do-- no intermediates generated at this level of work.
  #
  # Jump back to starting directory.
  if( scriptDirName != origWorkDirName ):
    if( showDiagnostics ):
      printDiagnostic('Jumping back to original directory {' +origWorkDirName +'}')
    os.chdir(origWorkDirName)
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
  exportPrepExeName = makeExportPrepApp(showDiagnostics, litter, True, None)
