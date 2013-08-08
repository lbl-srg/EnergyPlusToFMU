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

  > python  EnergyPlusToFMU.py  <path-to-idf-file>

This assumes you are in the same directory as the script file.
Typically this will be in the ``Scripts/EnergyPlusToFMU`` subdirectory of the installation directory.
See :doc:`installation` for the expected structure.

It is also possible to run the tool from another location, say from the same directory as the IDF file:

.. code-block:: none

  > python  <path-to-scripts-subdir>EnergyPlusToFMU.py  <name-of-idf-file>

For example:

.. code-block:: none

  # Windows:
  > python  installDir\Scripts\EnergyPlusToFMU\EnergyPlusToFMU.py  test.idf

  # Linux, MacOS:
  > python  installDir/Scripts/EnergyPlusToFMU/EnergyPlusToFMU.py  test.idf

where ``installDir`` is the complete path to the installation directory.

For readability, the rest of these instructions do not include the full paths to the script and input files.


Output
^^^^^^

The primary output consists of an FMU, named after the IDF file (e.g., ``test.fmu`` in the examples given above).
Note that the FMU is simply a zip file.
This means you can open and inspect its contents (you may have to change the "``.fmu``" extension to "``.zip``").

Secondary output includes:

- A utility executable, called ``idf-to-fmu-export-prep.exe`` on Windows, and
  ``idf-to-fmu-export-prep.app`` on Linux and MacOS (the different names allow
  dual-boot users to work in the same directory).
  You can delete this application if desired; it will be rebuilt on the next
  run of the tool.

- Compiled Python files, with the extension "``.pyc``".
  These files merely speed up Python the next time you run the EnergyPlusToFMU
  tools, and may be deleted.

If the EnergyPlusToFMU tool fails, you may also see intermediate files, including:

- Configuration files for the FMU (``variables.cfg`` and ``modelDescription.xml``).

- A utility executable ``util-get-address-size.exe``.
  This program gets rebuilt every time you run the EnergyPlusToFMU tools
  (in case you have modified the compiler/linker batch files as described
  in :doc:`installation`).

- Object directories (named like ``obj-...``).

- A shared library (named like ``test.dll`` or ``test.so`` or ``test.dylib``,
  depending on the platform).


Advanced use
^^^^^^^^^^^^

The EnergyPlusToFMU tool supports a number of options:

+---------------------------+----------------------------------------------------+
| option <argument>         | Purpose                                            |
+===========================+====================================================+
| -i <path-to-idd-file>     | Use the named Input Data Dictionary.               |
|                           | If you do not specify this option, the tool reads  |
|                           | environment variable ``ENERGYPLUS_DIR``, and uses  |
|                           | data dictionary ``ENERGYPLUS_DIR/bin/Energy+.idd`` |
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
  > python  EnergyPlusToFMU.py  -i C:\eplus\bin\Energy+.idd  test.idf

  > python  EnergyPlusToFMU.py  -d  test.idf


Troubleshooting
^^^^^^^^^^^^^^^

TODO: fill in
