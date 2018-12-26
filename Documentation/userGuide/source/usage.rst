.. highlight:: rest

.. _usage:

Usage of EnergyPlus as an FMU
=============================

The following items need to be observed when importing an FMU that contains EnergyPlus:

1. A tool that imports the FMU needs to make sure that the version of
   EnergyPlus which has been used to export the FMU
   is (a) installed and (b) added to the system PATH environment variable. Otherwise,
   the simulation will fail with an error. If EnergyPlus has been properly added to the
   system PATH environment variable, then it can be started from any DOS prompt on
   Windows, command shell console on Linux, or Terminal window on Mac OS by
   typing ``energyplus`` [#f1]_ .

2. The ``Number of Timesteps per Hour`` in EnergyPlus must be equal
   to the sampling time of the FMU. For example, consider the following
   EnergyPlus IDF snippet:

   .. code-block:: text

     Timestep,
     6;        !- Number of Timesteps per Hour

   Then, a tool that imports the FMU must synchronize it every 10 minutes.
   Otherwise, the simulation will stop with an error [#f2]_ .

3. EnergyPlus contains the object ``RunPeriod``.
   The start and end day of this object is ignored and replaced by the
   start and stop time provided by the master algorithm which imports
   the EnergyPlus FMU [#f3]_ . However, the entry ``Day of Week for Start Day``
   will be used. For example, consider the following IDF snippet:

   .. code-block:: text

      RunPeriod,         ! Winter Simulation
      Winter Simulation, !- Name
      1,                 !- Begin Month
      2,                 !- Begin Day of Month
      3,                 !- End Month
      31,                !- End Day of Month
      Monday,            !- Day of Week for Start Day
      Yes,               !- Use Weather File Holidays and Special Days
      Yes,               !- Use Weather File Daylight Saving Period
      No,                !- Apply Weekend Holiday Rule
      Yes,               !- Use Weather File Rain Indicators
      Yes;               !- Use Weather File Snow Indicators

   This IDF snippet declares January 2 to be a Monday.
   Hence, if an FMU is simulated with
   start time equal to 3 days, then the first day of the simulation
   will be Tuesday. There should only be one instance of RunPeriod in the IDF input file.
   If there are two RunPeriod in the IDF, then the first RunPeriod found in the IDF
   will be used. Any other RunPeriod object will be ignored.

4. A tool that imports the FMU must specify the start and stop time in seconds at initialization.
   The start and stop time will be converted to days and used by the EnergyPlus FMU.
   The start and stop time cannot have the value ``infinity``.

5. The weather file which comes along with an FMU is used to determine
   if the year is a ``leap year``. If no weather file is included in the FMU, then the
   assumption is that the year is not a ``leap year``.

6. During the warm-up period and the autosizing of EnergyPlus,
   no data exchange occurs between the FMU and the master program.
   Thus, inputs of EnergyPlus remain constant during these times and are equal
   to the initial values specified in the IDF input file.

7. The simulation results are saved in a result folder which is created in the current
   working directory. The name of the result folder is ``Output_EPExport_xxx``, where
   ``xxx`` is the FMU model ``instanceName`` as defined in the FMI specifications.


.. rubric:: Footnotes

.. [#f1] This is because the FMU implements the FMI for co-simulation
         in the `Tool Coupling <https://svn.modelica.org/fmi/branches/public/specifications/v1.0/FMI_for_ModelExchange_v1.0.pdf>`_ scenario.
.. [#f2] This is because the External Interface in EnergyPlus synchronizes
         the data at the zone time step which is constant throughout
         the simulation.
.. [#f3] This is because a tool that imports an FMU has its own definition
         of start time and stop time.
