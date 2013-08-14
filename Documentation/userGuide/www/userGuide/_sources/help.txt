Help
====

Before submitting a bug report, 
 * check https://corbu.lbl.gov/trac/fmu/browser/EnergyPlus#export/trunk if the bug is already known, and
 * make sure that it is a bug of the FMU implementation and not a bug of the EnergyPlus model. EnergyPlus bugs should be isolated so that they do not require use of an FMU, and then sent to the EnergyPlus help desk at http://energyplus.helpserve.com/
 
When submitting a bug report, please provide:
 * A model that is as small as possible and still reproduces your error. The chances of quickly finding and fixing a bug are much higher if the bug is part of a small test problem. In addition, creating a small test problem may help finding the root cause for the bug yourself, or realizing that there is no bug at all, and hence your problem can be solved much sooner.
 * A description of the expected and the observed results.
 * Information about the operating system and the EnergyPlus version.

To report the bug, send email to https://groups.google.com/group/energyplus-fmu. This is an open group and everyone can join it. No invitation is needed. 

Known Issues
============

Release 0.1.0
-------------

``Dymola 2014`` cannot import EnergyPlus as an FMU. The reason is a bug in ``Dymola 2014`` which does not pass the path to the resources location folder to the FMU when invoking the ``fmiInstantiateSlave()`` method. 
This information is needed by the FMU for its correct execution. 

``PyFMI 1.3.1`` cannot import EnergyPlus as an FMU since it currently does not support FMI for co-simulation tool coupling.

``Ptolemy II`` cannot import EnergyPlus as an FMU since it does not set and get all input and outputs of the FMU, as specified in :doc:`usage`, prior to calling the ``fmiDoStep()`` method.