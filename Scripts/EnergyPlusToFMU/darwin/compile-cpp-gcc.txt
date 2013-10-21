#!/usr/bin/env  bash


#--- Purpose.
#
#   Compile the source code file(s) named as command-line arguments.
# ** Use gcc/c++.
# ** Native address size.


#--- Command-line invocation.
#
scriptBaseName=$(basename "$0")
usageStr="USAGE: ./${scriptBaseName}  <name(s) of files to compile>"


#--- Check command-line arguments.
#
if test $# -lt 1
then
  echo "Error: ${scriptBaseName}: require at least one command-line argument"  1>&2
  echo "${usageStr}"  !>&2
  exit 1
fi


#--- Compile.
#
g++ -c  "$@"
