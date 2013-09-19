.. highlight:: rest

.. _build:

Creating an FMU
===============

This chapter describes how to create a Functional Mockup Unit, starting from an EnergyPlus IDF file.
It assumes you have followed the :doc:`installation` instructions, and that you have created an IDF file following the :doc:`bestPractice` guidelines.


Basic command-line use
^^^^^^^^^^^^^^^^^^^^^^

To create an FMU,
open a command-line window (see :doc:`notation`).
The basic invocation of the EnergyPlusToFMU tool is:

.. code-block:: none

  > python  <path-to-scripts-subdir>EnergyPlusToFMU.py  <path-to-idf-file>

For example:

.. code-block:: none

  # Windows:
  > python  scriptDir\EnergyPlusToFMU.py  test.idf

  # Linux, MacOS:
  > python  scriptDir/EnergyPlusToFMU.py  test.idf

where ``scriptDir`` is the path to the scripts directory of EnergyPlusToFMU.
Typically this is the ``Scripts/EnergyPlusToFMU`` subdirectory of the installation directory.
See :doc:`installation` for details.

The ``<path-to-idf-file>`` can include an absolute or relative path.
For example:

.. code-block:: none

  # Windows:
  > python  scriptDir\EnergyPlusToFMU.py  ..\all-idfs\test.idf

  # Linux, MacOS:
  > python  scriptDir/EnergyPlusToFMU.py  ../all-idfs/test.idf

For readability, the rest of these instructions omit the full path to the script and input files.


Output
^^^^^^

The main output from running ``EnergyPlusToFMU.py`` consists of an FMU, named after the IDF file (e.g., ``test.fmu`` in the examples given above).
The FMU is written to the current working directory, that is, in the directory from which you entered the command.

The FMU is complete and self-contained.
Any secondary output from running the EnergyPlusToFMU tools can be deleted safely.
Secondary output includes:

- A utility executable, called ``idf-to-fmu-export-prep.exe`` on Windows, and
  ``idf-to-fmu-export-prep.app`` on Linux and MacOS (the different names allow
  dual-boot users to work in the same directory).
  This executable will appear in your current working directory.
  If deleted, it will be rebuilt on the next run of EnergyPlusToFMU.

- Compiled Python files, with the extension "``.pyc``".
  They are written to the script directory.
  These files speed up Python the next time you run the EnergyPlusToFMU
  tools, and may be deleted.

If the EnergyPlusToFMU tool fails, you may also see intermediate files, including:

- The configuration files for the FMU, ``variables.cfg`` and ``modelDescription.xml``.

- A utility executable ``util-get-address-size.exe``.
  This program is rebuilt every time you run the EnergyPlusToFMU tools
  (in case you have modified the compiler/linker batch files as described
  in :doc:`installation`).

- Build directories, named like ``bld-*``.

- A shared library, named like ``*.dll`` or ``*.so`` or ``*.dylib``,
  depending on the system.

- A log file, ``output.log``, containing error messages from ``idf-to-fmu-export-prep.exe``.

All these intermediate files can be deleted.

Note that the FMU is a zip file.
This means you can open and inspect its contents.
To do so, it may help to change the "``.fmu``" extension to "``.zip``".


Advanced use
^^^^^^^^^^^^

The EnergyPlusToFMU tool supports a number of options:

+---------------------------+-------------------------------------------------------+
| option <argument>         | Purpose                                               |
+===========================+=======================================================+
| -i <path-to-idd-file>     | Use the named Input Data Dictionary.                  |
|                           | If you do not specify this option, the tool reads the |
|                           | environment variable ``ENERGYPLUS_DIR``, and uses the |
|                           | data dictionary ``ENERGYPLUS_DIR/Energy+.idd``.       |
|                           | For most EnergyPlus users, this environment           |
|                           | variable, and the IDD file, typically already         |
|                           | exist.                                                |
+---------------------------+-------------------------------------------------------+
| -w <path-to-weather-file> | Include the named weather file in the FMU.            |
+---------------------------+-------------------------------------------------------+
| -d                        | Print diagnostics.                                    |
|                           | Produces a status line for every major action         |
|                           | taken by the EnergyPlusToFMU tools.                   |
|                           | This option may be helpful for troubleshooting.       |
+---------------------------+-------------------------------------------------------+
| -L                        | Litter, that is, do not clean up intermediate         |
|                           | files.                                                |
|                           | Typically the EnergyPlusToFMU tools will delete       |
|                           | most of the intermediate files that ultimately get    |
|                           | packaged into the FMU.                                |
|                           | This option allows you to easily inspect              |
|                           | intermediate output.                                  |
+---------------------------+-------------------------------------------------------+

All these options must be supplied before the name of the IDF file.
However, they may be provided in any order.
For repeated options like ``-i`` or ``-w``, the last one specified will be used.

For example:

.. code-block:: none

  # Windows:
  > python  EnergyPlusToFMU.py  -i C:\eplus\Energy+.idd  test.idf

  > python  EnergyPlusToFMU.py  -d  test.idf


Setting environment variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To set the ``ENERGYPLUS_DIR`` environment variable, proceed as follows:

.. code-block:: none

  # Windows:
  > set  ENERGYPLUS_DIR="/Applications/EnergyPlus-8-0-0"

  # Bash shell on Linux, MacOS:
  > export  ENERGYPLUS_DIR="/Applications/EnergyPlus-8-0-0"

  # C shell on Linux, MacOS:
  > setenv  ENERGYPLUS_DIR  "/Applications/EnergyPlus-8-0-0"


Troubleshooting
^^^^^^^^^^^^^^^

To check whether ``EnergyPlusToFMU.py`` has run correctly, look for an FMU in your current working directory.
If you do not get an FMU, there will be some error output, indicating the nature of the problem.

The error message should be explicit enough to guide you to the source of the problem.
If not, consider the following hints.

If you have successfully made an FMU in the past, the problem is most likely with your IDF file.
Try running the export-preparation application directly on your IDF file:

.. code-block:: none

  # Windows:
  > idf-to-fmu-export-prep.exe  Energy+.idd  test.idf

  # Linux, MacOS:
  #   Note the "./" before the name of the application.
  > ./idf-to-fmu-export-prep.app  Energy+.idd  test.idf

Note that you must explicitly name the IDD file, as this executable does not attempt to read the ``ENERGYPLUS_DIR`` environment variable.

If running the export-preparation application as shown above works correctly, it produces two files, ``modelDescription.xml`` and ``variables.cfg``.
Otherwise, it should produce an error message, which should also be echoed to an output file ``output.log``.

Note that the export-preparation application processes only parts of the IDF file.
It does not attempt to identify modeling errors, or problems in IDF sections that do not relate to the FMU.
Therefore EnergyPlus may fail to run an IDF file, even if the export-preparation application handles it successfully.

If you do not find the export-preparation application in your working directory, then EnergyPlusToFMU did not advance to creating the application.
Therefore you should check the configuration, according to the instructions in :doc:`installation`.

If the export-preparation application runs, then try turning on option ``-d`` when running ``EnergyPlusToFMU.py``.
By announcing each major step before it is taken, this option helps to localize the problem.
