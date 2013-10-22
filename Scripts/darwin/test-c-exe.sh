#!/usr/bin/env  bash


#--- Purpose.
#
#   Perform an end-to-end test of the current batch files for making an
# executable from C source code.


#--- Command-line invocation.
#
#   Requires no command-line arguments:
# > ./<this-script-name>
#
#   A successful run prints some diagnostics, and finally the address size of
# the machine.


echo "===== Checking for required files ====="
#
srcFileName="../../SourceCode/utility/get-address-size.c"
if test ! -f "${srcFileName}"
then
    echo "Error: missing source file ${srcFileName}"
    exit 1
fi
#
compilerBatchFileName="compile-c.sh"
if test ! -f "${compilerBatchFileName}"
then
    echo "Error: missing compiler batch file ${compilerBatchFileName}"
    exit 1
fi
#
linkerBatchFileName="link-c-exe.sh"
if test ! -f "${linkerBatchFileName}"
then
    echo "Error: missing linker batch file ${linkerBatchFileName}"
    exit 1
fi


echo "===== Removing old output files ====="
#
objFileName="get-address-size.o"
if test -f "${objFileName}"
then
    rm "${objFileName}"
fi
#
exeName="test.exe"
if test -f "${exeName}"
then
    rm "${exeName}"
fi


echo "===== Running compiler ====="
#
./"${compilerBatchFileName}"  "${srcFileName}"
#
if test $? -ne 0
then
    echo "Error: compiler batch file ${compilerBatchFileName} failed, with exit code $?"
    exit 1
fi
#
if test ! -f "${objFileName}"
then
    echo "Error: compiler batch file ${compilerBatchFileName} did not produce object file ${objFileName}"
    exit 1
fi


echo "===== Running linker ====="
#
./"${linkerBatchFileName}"  "${exeName}"  "${objFileName}"
#
if test $? -ne 0
then
    echo "Error: linker batch file ${linkerBatchFileName} failed, with exit code $?"
    exit 1
fi
#
if test ! -f "${exeName}"
then
    echo "Error: linker batch file ${linkerBatchFileName} did not produce executable ${exeName}"
    exit 1
fi


echo "===== Running output executable ====="
#
echo "== The address size, e.g., 32 or 64, should appear below =="
./"${exeName}"
echo
echo "== The address size should appear above =="
#
if test $? -ne 0
then
    echo "Error: executable ${exeName} failed, with exit code $?"
    exit 1
fi


echo "===== Cleaning up ====="
#
if test -f "${objFileName}"
then
    rm "${objFileName}"
fi
#
if test -f "${exeName}"
then
    rm "${exeName}"
fi
