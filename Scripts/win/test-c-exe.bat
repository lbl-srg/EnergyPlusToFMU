@ECHO OFF


::--- Purpose.
::
::   Perform an end-to-end test of the current batch files for making an
:: executable from C source code.


::--- Command-line invocation.
::
::   Requires no command-line arguments:
:: > <this-script-name>
::
::   A successful run prints some diagnostics, and finally the address size of
:: the machine.


ECHO ===== Checking for required files =====
::
SET srcFileName=..\..\SourceCode\utility\get-address-size.c
IF NOT EXIST %srcFileName% (
    ECHO Error: missing source file %srcFileName%
    GOTO done
    )
::
SET compilerBatchFileName=compile-c.bat
IF NOT EXIST %compilerBatchFileName% (
    ECHO Error: missing compiler batch file %compilerBatchFileName%
    GOTO done
    )
::
SET linkerBatchFileName=link-c-exe.bat
IF NOT EXIST %linkerBatchFileName% (
    ECHO Error: missing linker batch file %linkerBatchFileName%
    GOTO done
    )


ECHO ===== Removing old output files =====
::
SET objFileName=get-address-size.obj
IF EXIST %objFileName% (
    DEL %objFileName%
    )
::
SET exeName=test.exe
IF EXIST %exeName% (
    DEL %exeName%
    )


ECHO ===== Running compiler =====
::
CALL %compilerBatchFileName%  %srcFileName%
::
IF %ERRORLEVEL% NEQ 0 (
    ECHO Error: compiler batch file %compilerBatchFileName% failed
    GOTO done
    )
::
IF NOT EXIST %objFileName% (
    ECHO Error: compiler batch file %compilerBatchFileName% did not produce object file %objFileName%
    GOTO done
    )


ECHO ===== Running linker =====
::
CALL %linkerBatchFileName%  %exeName%  %objFileName%
::
IF %ERRORLEVEL% NEQ 0 (
    ECHO Error: linker batch file %linkerBatchFileName% failed
    GOTO done
    )
::
IF NOT EXIST %exeName% (
    ECHO Error: linker batch file %linkerBatchFileName% did not produce executable %exeName%
    GOTO done
    )


ECHO ===== Running output executable =====
::
ECHO == The address size, e.g., 32 or 64, should appear below ==
CALL %exeName%
ECHO.
ECHO == The address size should appear above ==
::
IF %ERRORLEVEL% NEQ 0 (
    ECHO Error: executable %exeName% failed
    GOTO done
    )


ECHO ===== Cleaning up =====
::
IF EXIST %objFileName% (
    DEL %objFileName%
    )
::
IF EXIST %exeName% (
    DEL %exeName%
    )


:done
