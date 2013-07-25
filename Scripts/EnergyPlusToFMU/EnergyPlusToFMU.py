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
    '[-i path-to-idd-file]  [-w path-to-weather-file]  [-d]  [-L]  <path-to-idf-file>'
  #
  print '-- Export an EnergyPlus model as a Functional Mockup Unit (FMU) for co-simulation'
  print '-- Option -i, use the named Input Data Dictionary'
  print '   Lacking -i, read environment variable {ENERGYPLUS_DIR}, and use {ENERGYPLUS_DIR/bin/Energy+.idd}'
  print '-- Option -w, use the named weather file'
  print '-- Option -d, print diagnostics'
  print '-- Option -L, litter, that is, do not clean up intermediate files'
  # hoho add -V to set version number of FMI standard.  Currently 1.0 is only one supported.
  #
  # End fcn printCmdLineUsage().


#--- Ensure access.
#
import os
import subprocess
import sys
import zipfile


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


#--- Fcn to search for an IDD file using environment variable {ENERGYPLUS_DIR}.
#
#   Return a tuple (found, resultStr).
#   If {found}, set {resultStr} to fully-resolved name of IDD file.
# Otherwise, set {resultStr} to an appropriate error message.
#
def findIddFileViaEnvDir():
  #
  epDirName = os.environ.get('ENERGYPLUS_DIR')
  if( epDirName is None ):
    return(False, 'Cannot find environment variable {ENERGYPLUS_DIR}')
  if( not os.path.isdir(epDirName) ):
    return(False, 'Environment variable {ENERGYPLUS_DIR} refers to missing directory ' +epDirName)
  #
  binDirName = os.path.join(os.path.abspath(binDirName), 'bin')
  if( not os.path.isdir(binDirName) ):
    return(False, 'Missing EnergyPlus directory ' +binDirName)
  #
  iddFileName = os.path.join(binDirName, 'Energy+.idd')
  if( not os.path.isfile(iddFileName) ):
    return(False, 'Missing IDD file ' +iddFileName)
  #
  return(True, iddFileName)
  #
  # End fcn findIddFileViaEnvDir().


#--- Fcn to export an EnergyPlus IDF file as an FMU.
#
def exportEnergyPlusAsFMU(showDiagnostics, litter, iddFileName, wthFileName, idfFileName):
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
  # Set working directory to same directory as this script file.
  #   To allow using relative paths, rather than calling os.path.abspath() on
  # everything.
  origWorkDirFullName = os.path.abspath(os.getcwd())
  scriptDirFullName = os.path.abspath(os.path.dirname(__file__))
  if( scriptDirFullName != origWorkDirFullName ):
    if( showDiagnostics ):
      printDiagnostic('Jumping to script directory {' +scriptDirFullName +'}')
    os.chdir(scriptDirFullName)
  #
  # Load modules expect to find in same directory as this script file.
  findFileOrQuit('utility script', 'makeFmuLib.py')
  try:
    import makeFmuLib
  except:
    quitWithError('Unable to import {makeFmuLib.py}', False)
  #
  findFileOrQuit('utility script', 'makeUtilityApps.py')
  try:
    import makeUtilityApps
  except:
    quitWithError('Unable to import {makeUtilityApps.py}', False)
  #
  # Get valid model identifier.
  modelIdName = os.path.basename(idfFileName)
  if( modelIdName.endswith('.idf') or modelIdName.endswith('.IDF') ):
    modelIdName = modelIdName[:-4]
  modelIdName = makeFmuLib.sanitizeIdentifier(modelIdName)
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
  #   Do not force a rebuild.  Accept the name returned by fcn makeExportPrepApp().
  if( showDiagnostics ):
    printDiagnostic('Checking for export-prep application')
  exportPrepExeName = makeUtilityApps.makeExportPrepApp(showDiagnostics, litter, False, None)
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
  (OUT_fmuSharedLibName, fmuBinDirName) = makeFmuLib.makeFmuSharedLib(showDiagnostics, litter, modelIdName)
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
  # Return to original directory.
  if( scriptDirFullName != origWorkDirFullName ):
    os.chdir(origWorkDirFullName)
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
  # Get {iddFileName} if necessary.
  if( iddFileName is None ):
    if( showDiagnostics ):
      printDiagnostic('Begin looking for IDD file via environment variable {ENERGYPLUS_DIR}')
    (success, iddFileName) = findIddFileViaEnvDir()
    if( not success ):
      # Note here, {iddFileName} is actually an error string.
      quitWithError('Could not find IDD file via environment variable {ENERGYPLUS_DIR}: ' +iddFileName, True)
    if( showDiagnostics ):
      printDiagnostic('Setting IDD file to {' +iddFileName +'}')
  #
  # Run.
  exportEnergyPlusAsFMU(showDiagnostics, litter, iddFileName, wthFileName, idfFileName)
