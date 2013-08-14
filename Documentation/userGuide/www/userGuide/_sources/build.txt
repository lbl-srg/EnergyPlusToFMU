.. highlight:: rest

.. _build:

Creating an FMU
===============

This chapter describes how to build a Functional Mockup Unit, starting from an EnergyPlus IDF file.
It assumes you have already followed the :doc:`installation` instructions, and that you have already created an IDF file following the :doc:`bestPractice` guidelines.


Basic command-line use
^^^^^^^^^^^^^^^^^^^^^^

To build an FMU, open a command window (e.g., a DOS prompt on Windows, a command shell on Linux, or a Terminal window on MacOS).
The instructions that follow represent the command window like this:

.. code-block:: none

  # This is a comment.
  > (This is the command prompt, where you enter a command)
  (If shown, this is sample output in response to the command)

Note that your system may use a different symbol than "``>``" as the command prompt (for example, "``$``").
Furthermore, the prompt may include information such as the name of your system, or the name of the current subdirectory.

The basic invocation of the EnergyPlusToFMU tool is:

.. code-block:: none

  > python  <path-to-scripts-subdir>EnergyPlusToFMU.py  <path-to-idf-file>

For example:

.. code-block:: none

  # Windows:
  > python  scriptDir\EnergyPlusToFMU.py  test.idf

  # Linux, MacOS:
  > python  scriptDir/EnergyPlusToFMU.py  test.idf

where ``scriptDir`` is the path to the scripts directory of the EnergyPlusToFMU tools.
Typically this is the ``Scripts/EnergyPlusToFMU`` subdirectory of the installation directory.
See :doc:`installation` for the expected structure.

The ``<path-to-idf-file>`` also can include an absolute or relative path.
For example:

.. code-block:: none

  # Windows:
  > python  scriptDir\EnergyPlusToFMU.py  ..\all-idfs\test.idf

  # Linux, MacOS:
  > python  scriptDir/EnergyPlusToFMU.py  ../all-idfs/test.idf

For readability, the rest of these instructions omit the full paths to the script and input files.


Output
^^^^^^

The main output from running ``EnergyPlusToFMU.py`` consists of an FMU, named after the IDF file (e.g., ``test.fmu`` in the examples given above).
The FMU should appear in your current working directory, that is, in the directory from which you entered the command.

The FMU should be complete and self-contained.
Any secondary output from running the EnergyPlusToFMU tools can be deleted safely.

Secondary output includes:

- A utility executable, called ``idf-to-fmu-export-prep.exe`` on Windows, and
  ``idf-to-fmu-export-prep.app`` on Linux and MacOS (the different names allow
  dual-boot users to work in the same directory).
  This executable will appear in your current working directory.
  If you delete this application, it will be rebuilt on the next run of the tool.

- Compiled Python files, with the extension "``.pyc``".
  They should appear in the script directory.
  These files merely speed up Python the next time you run the EnergyPlusToFMU
  tools, and may be deleted.

If the EnergyPlusToFMU tool fails, you may also see intermediate files, including:

- Configuration files for the FMU (``variables.cfg`` and ``modelDescription.xml``).

- A utility executable ``util-get-address-size.exe``.
  This program gets rebuilt every time you run the EnergyPlusToFMU tools
  (in case you have modified the compiler/linker batch files as described
  in :doc:`installation`).

- Build directories (named like ``bld-...``).

- A shared library (named like ``test.dll`` or ``test.so`` or ``test.dylib``,
  depending on the platform).

- An log file, ``output.log``, containing error messages from ``idf-to-fmu-export-prep.exe``.

Note that the FMU is simply a zip file.
This means you can open and inspect its contents (it may help to change the "``.fmu``" extension to "``.zip``" in order to do so).


Advanced use
^^^^^^^^^^^^

The EnergyPlusToFMU tool supports a number of options:

+---------------------------+----------------------------------------------------+
| option <argument>         | Purpose                                            |
+===========================+====================================================+
| -i <path-to-idd-file>     | Use the named Input Data Dictionary.               |
|                           | If you do not specify this option, the tool reads  |
|                           | environment variable ``ENERGYPLUS_DIR``, and uses  |
|                           | data dictionary ``ENERGYPLUS_DIR/Energy+.idd``     |
|                           | (for most EnergyPlus users, this environment       |
|                           | variable, and the IDD file, typically already      |
|                           | exist).                                            |
+---------------------------+----------------------------------------------------+
| -w <path-to-weather-file> | Include the named weather file in the FMU.         |
+---------------------------+----------------------------------------------------+
| -d                        | Print diagnostics.                                 |
|                           | Produces a status line for every major action      |
|                           | taken by the EnergyPlusToFMU tools.                |
|                           | This option may be helpful for troubleshooting.    |
+---------------------------+----------------------------------------------------+
| -L                        | Litter, that is, do not clean up intermediate      |
|                           | files.                                             |
|                           | Typically the EnergyPlusToFMU tools will delete    |
|                           | most of the intermediate files that ultimately get |
|                           | packaged into the FMU.                             |
|                           | This option allows you to easily inspect           |
|                           | intermediate output.                               |
+---------------------------+----------------------------------------------------+

All these options must be supplied before the name of the IDF file.
However, they may be provided in any order.
If you repeat an option like ``-i`` or ``-w``, the last one specified will be used.

For example:

.. code-block:: none

  # Windows:
  > python  EnergyPlusToFMU.py  -i C:\eplus\Energy+.idd  test.idf

  > python  EnergyPlusToFMU.py  -d  test.idf


Setting environment variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To set the ``ENERGYPLUS_DIR`` environment variable:

.. code-block:: none

  # Windows:
  > set  ENERGYPLUS_DIR='/Applications/EnergyPlus-8-0-0'

  # Bash shell on Linux, MacOS:
  > export  ENERGYPLUS_DIR='/Applications/EnergyPlus-8-0-0'

  # C shell on Linux, MacOS:
  > setenv  ENERGYPLUS_DIR  '/Applications/EnergyPlus-8-0-0'


Troubleshooting
^^^^^^^^^^^^^^^

The main indicator that ``EnergyPlusToFMU.py`` has run correctly is the appearance of an FMU in your current working directory.
If you do not get an FMU, there should be some error output, indicating the nature of the problem.

With luck, the error message will be explicit enough to guide you to the source of the problem.
If not, consider the following hints.

If you have successfully made an FMU in the past, the problem is most likely with your IDF file.
Try running the export-preparation appication directly on your IDF file:

.. code-block:: none

  # Windows:
  > idf-to-fmu-export-prep.exe  Energy+.idd  test.idf

  # Linux, MacOS:
  #   Note the "./" before the name of the application.
  > ./idf-to-fmu-export-prep.app  Energy+.idd  test.idf

Note that you must explicitly name the IDD file, as this executable does not attempt to read the ``ENERGYPLUS_DIR`` environment variable.

If it encounters no problems, the export-preparation application should produce two files, ``modelDescription.xml`` and ``variables.cfg``.
Otherwise, it should produce an error message, which should also be echoed to an output file ``output.log``.

Note that the export-preparation application only looks at the IDF in a very shallow way.
Your IDF file may have modeling errors that will cause EnergyPlus to fail, but that the export-preparation application will not log.

If you do not find the export-preparation application in your working directory, if means the EnergyPlusToFMU tool did not even advance as far as creating the application.
Therefore you should check the configuration, according to the instructions in :doc:`installation`.

If the export-preparation application runs, then try turning on option ``-d`` when running ``EnergyPlusToFMU.py``.
By announcing each major step before it is taken, this option can help localize where, exactly, the problem occurs.
