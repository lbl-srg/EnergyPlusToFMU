.. highlight:: rest

.. _notation:

Notation
========

This chapter shows the formatting conventions used throughout the User Guide.

The command-line is an interactive session for issuing commands to the operating system.
Examples include a DOS prompt on Windows, a command shell on Linux, and a Terminal window on MacOS.

The User Guide represents a command window like this:

.. code-block:: none

  # This is a comment.
  > (This is the command prompt, where you enter a command)
  (If shown, this is sample output in response to the command)

For example, to see a list of files in the current directory on Linux or MacOS:

.. code-block:: none

  # List all Python files.
  > ls  *.py
  EnergyPlusToFMU.py   makeExportPrepApp.py   ...

Note that your system may use a different symbol than "``>``" as the command prompt (for example, "``$``").
Furthermore, the prompt may include information such as the name of your system, or the name of the current subdirectory.
