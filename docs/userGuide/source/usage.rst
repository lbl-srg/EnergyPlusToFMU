.. highlight:: rest

.. _usage:

Usage
=====

fixme: describe how to use the software

When the Functional Mockup Unit that contains an EnergyPlus model, the
following items need to be observed.

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
   The start and end day of these objects are ignored [#f3]_. However,
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

   Then, January 2 is a Monday. Hence, if an FMU is simulated with 
   start time equal to 3 days, then the first day of the simulation
   will be Tuesday.

4. During the warm-up period and the autosizing of EnergyPlus, 
   no communication occurs between
   the FMU and the master program. Thus, inputs from the co-simulation 
   master program to EnergyPlus remain constant during these times.


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
