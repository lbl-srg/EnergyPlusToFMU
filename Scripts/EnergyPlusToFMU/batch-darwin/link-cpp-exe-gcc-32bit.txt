#!/usr/bin/env  bash


#--- Purpose.
#
#   Link the object file(s) named as command-line arguments.
# ** Make an executable.
# ** Use gcc/c++.
# ** Force 32-bit.


#--- Command-line invocation.
#
scriptBaseName=$(basename "$0")
usageStr="USAGE: ./${scriptBaseName}  <name of output>  <name(s) of files to link>"


#--- Check command-line arguments.
#
if test $# -lt 2
then
  echo "Error: ${scriptBaseName}: require at least two command-line arguments"  1>&2
  echo "${usageStr}"  !>&2
  exit 1
fi
#
outputName="$1"
#
# Shift {outputName} off, so object arguments start at $1.
shift 1


#--- Link.
#
g++ -m32  -o "${outputName}"  "$@"
