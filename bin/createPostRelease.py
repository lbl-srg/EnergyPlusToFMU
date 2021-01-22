# Python script which is used to create the zip folder
# This script should be run outside of the trunk
# This script requires two inputs:
# Path to folder where the source filers are
# Release version number

# Typical usage is python createRelease.py basedir releasenumber,
# where basedir is the path to the folder which contains the
# files and releasenumber is the release version number
# The user should run cleanfilesystem prior to running this file

# TSNouidui@lbl.gov                                        013-31-07
#####################################################################

#!/usr/bin/env python
# from __future__ import with_statement
from subprocess import call
def printCmdLineUsage():
  #
  print('USAGE:', os.path.basename(__file__), \
    '[Path to folder with source files]  [Release version number]')

def quitWithError(messageStr, showCmdLine):
  #
  print('ERROR from script file {' + os.path.basename(__file__) + '}')
  #
  if(messageStr is not None):
    print(messageStr)
  #
  if(showCmdLine):
    print()
    printCmdLineUsage()
  #
  sys.exit(1)

from contextlib import closing
from zipfile import ZipFile, ZIP_DEFLATED
import os
import sys

def zipdir(basedir, archivename):
    n_gits=0
    n_md=0
    assert os.path.isdir(basedir)
    with closing(ZipFile(archivename, "w", ZIP_DEFLATED)) as z:
        for root, dirs, files in os.walk(basedir):
            #NOTE: ignore empty directories
            #if 'bin' in dirs:
            #    print "found bin folder in " + str(dirs)
            #    #continue
            #    dirs.remove('bin')
            if '.git' in dirs:
                n_gits=n_gits+1
                print("found .git folder in " + str(dirs))
                #continue
                dirs.remove('.git')
            for fn in files:
                if (fn.endswith('.md')):
                    absfn = os.path.join(root, fn)
                    print("found file with extension '.md'. The file is in " + absfn)
                    n_md=n_md+1
                    continue
                elif (fn.endswith('.pyc')):
                    absfn = os.path.join(root, fn)
                    print("found file with extension '.pyc'. The file is in " + absfn)
                    continue
                else:
                    absfn = os.path.join(root, fn)
                    zfn = absfn[len(basedir) + len(os.sep):]
                    z.write(absfn, zfn)
    if(n_md>1):
        print("found more than one file (" + str(n_md) + ")" + " with extension .md. Program terminated.")
        sys.exit()


if __name__ == '__main__':
    import sys
    len_args = len(sys.argv)
    if (len_args != 3):
        quitWithError('The number of input arguments must be exactly 2.', True)
    else:
        basedir = sys.argv[1]
        releasenumber = sys.argv[2]
        archivename = 'EnergyPlusToFMU-' + releasenumber + '.zip'
        zipdir(basedir, archivename)
print("************************************************************")
print(("Create tag for release with version number: " + releasenumber))
print("************************************************************")

#call(['rsync', archivename, "thierry@simulationresearch.lbl.gov:/usr/local/www/simulationresearch/fmu/EnergyPlus/export/releases/" + #releasenumber + "/"])
