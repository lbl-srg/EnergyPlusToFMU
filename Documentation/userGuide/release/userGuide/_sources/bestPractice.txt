.. highlight:: rest

.. _bestPractice:


Best Practice
=============

This section explains to users best practice in configuring an EnergyPlus model 
for an FMU. 

To export EnergyPlus as an FMU, four objects have been added to the EnergyPlus data structure. These objects are:

- The ``ExternalInterface:FunctionalMockupUnitExport:From:Variable`` object, 
  which is used to map the outputs of the FMU to the EnergyPlus 
  objects ``Output:Variable`` and ``EnergyManagementSystem:OutputVariable``.
 
- The ``ExternalInterface:FunctionalMockupUnitExport:To:Schedule``, 
  ``ExternalInterface:FunctionalMockupUnitExport:To:Actuator``, 
  and ``ExternalInterface:FunctionalMockupUnitExport:To:Variable`` 
  which are used to map the inputs of the FMU to EnergyPlus schedule and 
  EMS actuators and variables.
  
These objects are described in the Input/Output reference of the EnergyPlus manual 
(http://apps1.eere.energy.gov/buildings/energyplus/pdfs/inputoutputreference.pdf) 

Configuring an EnergyPlus model which uses the ``Schedule`` object
------------------------------------------------------------------

Suppose, we would like to export an EnergyPlus model of a room with 
an ideal HVAC system, that delivers sensible and latent heat gains as schedules
to maintain a certain room temperature. The HVAC system in EnergyPlus requires 
outdoor dry-bulb temperature, outdoor air relative humidity, 
room dry-bulb temperature and room air relative humidity in the zone to compute 
the sensible and latent heat gains. 

Such an  EnergyPlus model could be exported as an FMU with 
four inputs and two outputs. The four inputs of the FMU will map to the 
outdoor dry-bulb temperature, outdoor air relative humidity, 
room dry-bulb temperature and room air relative humidity in the zone, whereas 
the two outputs map to the sensible and latent heat gains.

The Energyplus model needs to contain the following three items:

- An object that instructs EnergyPlus to activate the external interface.

- EnergyPlus objects that read inputs of the FMU and send to EnergyPlus.

- EnergyPlus objects that read data from EnergyPlus and send to the outputs of the FMU.

The code below shows how the objects will be in the idf.
To activate the external interface, we use:

   .. code-block:: text

      ExternalInterface,	   !- Object to activate external interface`
      FunctionalMockupUnitExport;  !- Name of external interface

To define the inputs of the FMU, we use:

   .. code-block:: text

      ExternalInterface:FunctionalMockupUnitExport:From:Variable,
      Environment,                           !- EnergyPlus Key Value
      Site Outdoor Air Drybulb Temperature,  !- EnergyPlus Variable Name
      TDryBul;                               !- FMU Variable Name
      
      ExternalInterface: FunctionalMockupUnitExport:From:Variable,
      ZONE ONE,                  !- EnergyPlus Key Value
      Zone Mean Air Temperature, !- EnergyPlus Variable Name
      TRooMea;                   !- FMU Variable Name
      
      
      ExternalInterface: FunctionalMockupUnitExport:From:Variable,
      Environment,                         !- EnergyPlus Key Value
      Site Outdoor Air Relative Humidity,  !- EnergyPlus Variable Name
      outRelHum;                           !- FMU Variable Name
      
      ExternalInterface:FunctionalMockupUnitExport:From:Variable,
      ZONE ONE,                    !- EnergyPlus Key Value
      Zone Air Relative Humidity,  !- EnergyPlus Variable Name
      rooRelHum;                   !- FMU Variable Name 


Along with the FMU's inputs definition, the
EnergyPlus output variables which correspond to the FMU inputs need 
to be specified in the idf file:

   .. code-block:: text
   
	Output:Variable,
	Environment,                             !- Key Value
	Site Outdoor Air Drybulb Temperature,    !- Variable Name
	TimeStep;                                !- Reporting Frequency

	Output:Variable,
	ZONE ONE,                    !- Key Value
	Zone Mean Air Temperature,   !- Variable Name
	TimeStep;                    !- Reporting Frequency 

	Output:Variable,
	Environment,                         !- Key Value
	Site Outdoor Air Relative Humidity,  !- Variable Name
	TimeStep;                            !- Reporting Frequency

	Output:Variable,
	ZONE ONE,                    !- Key Value
	Zone Air Relative Humidity,  !- Variable Name 
	TimeStep;                    !- Reporting Frequency

To define the outputs of the FMU, we use:

   .. code-block:: text
   
	ExternalInterface:FunctionalMockupUnitExport:To:Schedule,
	FMU_OthEquLat_ZoneOne,   !- EnergyPlus Variable Name
	Any Number,              !- Schedule Type Limits Names
	QSensible,               !- FMU Variable Name
	0;                       !- Initial Value
    
	ExternalInterface:FunctionalMockupUnitExport:To:Schedule,
	FMU_OthEquSen_ZoneOne,   !- EnergyPlus Variable Name
	Any Number,              !- Schedule Type Limits Names
	QLatent,                 !- FMU Variable Name
	0;                       !- Initial Value

Configuring an EnergyPlus model which uses the ``EMS Actuator`` object
----------------------------------------------------------------------

Suppose, we would like to export an EnergyPlus model of a room with a window 
model which has a shading controller which actuates a blind as function of 
boundary conditions. The shading controller requires as inputs the outside 
temperature (TRoo) and the solar irradiation (ISolExt) that is incident on 
the window to compute the shading actuation signal (yShade).

Such an  EnergyPlus model could be exported as an FMU with 
2 inputs and one outputs. The two inputs of the FMU will map to the 
outside temperature (TRoo) and the solar irradiation (ISolExt), whereas 
the output to the shading actuation signal.

The code below shows how the objects will be in the idf.
To activate the external interface, we use:

   .. code-block:: text
   
      ExternalInterface,	   !- Object to activate external interface`
      FunctionalMockupUnitExport;  !- Name of external interface

To define the inputs of the FMU, we use:

   .. code-block:: text
   
	ExternalInterface:FunctionalMockupUnitExport:From:Variable,
	WEST ZONE,                                   !- EnergyPlus Key Value
	Zone Mean Air Temperature,                   !- EnergyPlus Variable Name
	TRoo;                                        !- FMU Variable Name

	ExternalInterface:FunctionalMockupUnitExport:From:Variable,
	Zn001:Wall001:Win001,                                        !- EnergyPlus Key Value
	Surface Outside Face Incident Solar Radiation Rate per Area, !- EnergyPlus Variable Name
	ISolExt;                                                     !- FMU Variable Name

Along with the FMU's inputs definition, the
EnergyPlus output variables which correspond to the FMU inputs need 
to be specified in the idf file:

   .. code-block:: text

	Output:Variable,
	Zn001:Wall001:Win001,                                         !- Key Value
	Surface Outside Face Incident Solar Radiation Rate per Area,  !- Variable Name
	TimeStep;                                                     !- Reporting Frequency

	Output:Variable,``
	WEST ZONE,                          !- Key Value
	Zone Mean Air Temperature,          !- Variable Name
	TimeStep;                           !- Reporting Frequency

To define the output of the FMU, we use:

   .. code-block:: text
    
	ExternalInterface:FunctionalMockupUnitExport:To:Actuator,
	Zn001_Wall001_Win001_Shading_Deploy_Status,  !- EnergyPlus Variable Name
	Zn001:Wall001:Win001,                        !- Actuated Component Unique Name
	Window Shading Control,                      !- Actuated Component Type
	Control Status,                              !- Actuated Component Control Type
	yShade,                                      !- FMU Variable Name
	6;                                           !- Initial Value


Configuring an EnergyPlus model which uses the ``EMS Variable`` object
----------------------------------------------------------------------

This configuration is almost the same as in the previous example with the only 
difference being that the shading actuation signal will be mapped to an EMS variable
(Shade_Signal) that can be used in an EMS program.

To define the output of the FMU, we use: 

   .. code-block:: text
   
	ExternalInterface:FunctionalMockupUnitExport:To:Variable,
	Shade_Signal,            !- EnergyPlus Variable Name
	yShade,                  !- FMU Variable Name
	6;                       !- Initial Value

Please see the Input/Output reference of the EnergyPlus manual 

(http://apps1.eere.energy.gov/buildings/energyplus/pdfs/inputoutputreference.pdf) 
for more details.

Please read :doc:`installation` to see how to generate an FMU.



