.. highlight:: rest

.. _usage:

Usage
=====

fixme: describe how to use the software

Following items need to be observed when importing an FMU that contains EnergyPlus:

1. A tool that imports the FMU must start and stop the simulation at midnight, 
   otherwise the simuluation stops with an error. [#f1]_
2. The ``Number of Timesteps per Hour`` in EnergyPlus must be equal
   to the sampling time of the FMU. For example, consider the following
   EnergyPlus IDF snippet:

   .. code-block:: idf

     Timestep, 
     6;        !- Number of Timesteps per Hour

   Then, a tool that imports the FMU must synchronize it every 10 minutes.
   Otherwise, the simulation will stop with an error. [#f2]_

3. EnergyPlus contains the object ``RunPeriod`` and ``RunPeriod:CustomRange``. 
   The start and end day of these objects are ignored. [#f3]_ However,
   the entry ``Day of Week for Start Day`` will be used. For example, 
   consider the following IDF snippet:

   .. code-block:: idf

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
   will be Tuesday.

4. During the warm-up period and the autosizing of EnergyPlus, 
   no communication occurs between
   the FMU and the master program. Thus, inputs from the co-simulation 
   master program to EnergyPlus remain constant during these times.
   
5. A tool that imports the FMU needs to make sure that the version of EnergyPlus which has been used to export the FMU
   is (a) installed and (b) on the system path. Otherwise, the simulation will fail with an error. [#f4]_
   
6. A tool that imports the FMU must have the version of EnergyPlus which has been used to export the FMU on the system path. Otherwise, the simulation will fail to start with an error.
   
7. The simulation results of the FMU are in the folder where the FMU has been unzipped.

8. The weather file which comes along with an FMU is used to determine if the year is a leap year. It no weather file 
   is included in the FMU, then the assumption is that the year is not a leap year.


.. rubric:: Footnotes

.. [#f1] This is because EnergyPlus requires simulation to start and end at
         midnight.
.. [#f2] This is because the External Interface in EnergyPlus synchronizes
         the data at the zone time step which is constant throughout
         the simulation. Synchronizing the
         data at the system time step would not avoid this problem because
         in EnergyPlus, the system time step cannot be smaller 
         than one minute.
.. [#f3] This is because a tool that imports an FMU has its own definition 
         of start time and stop time.

.. [#f4] This is because the FMU implement the FMI for co-simulation in Tool Coupling scenario.
