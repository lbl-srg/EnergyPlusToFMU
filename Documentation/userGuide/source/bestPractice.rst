.. highlight:: rest

.. _bestPractice:

Best Practice
=============

This section explains to users best practice in configuring an EnergyPlus model 
for a Functional Mockup Unit.


Configuration of an EnergyPlus model
------------------------------------

To export an EnergyPlus model as a Functional Mock-up Unit for co-simulation, 
the user 

ExternalInterface:FunctionalMockupUnitExport:To:Schedule

Suppose, we would like to export an EnergyPlus model of a room with 
an ideal HVAC system, that delivers sensible and latent heat gains as schedules
to maintain a certain room temperature. The HVAC system in EnergyPlus requires 
outdoor dry-bulb temperature, outdoor air relative humidity, 
room dry-bulb temperature and room air relative humidity in the zone to compute 
the heat gains. This EnergyPlus model could be exported as an FMU with 
four inputs and two outputs. The four inputs of the FMU map with 
utdoor dry-bulb temperature, outdoor air relative humidity, 
room dry-bulb temperature and room air relative humidity in the zone, whereas 
the two outputs map with the sensible and latent heat gains.


The Energyplus model needs to contain the following four items are needed:
	An object that instructs EnergyPlus to activate the external interface.
	EnergyPlus objects that read inputs of the FMU and send to EnergyPlus.
	EnergyPlus objects that read data from EnergyPlus and send to 
	the outputs of the FMU.

The code below shows how the objects will be in the idf.
To activate the external interface, we use:
ExternalInterface,           !- Object to activate the external interface
 FunctionalMockupUnitExport; !- Name of external interface

To define the inputs of the FMU, we use:
ExternalInterface:FunctionalMockupUnitExport:From:Variable,
    Environment,             !- EnergyPlus Key Value
    Site Outdoor Air Drybulb Temperature,  !- EnergyPlus Variable Name
    TDryBul;                 !- FMU Variable Name

ExternalInterface: FunctionalMockupUnitExport:From:Variable,
    ZONE ONE,                  !- EnergyPlus Key Value
    Zone Mean Air Temperature, !- EnergyPlus Variable Name
    TRooMea;                   !- FMU Variable Name

ExternalInterface: FunctionalMockupUnitExport:From:Variable,
    Environment,                !- EnergyPlus Key Value
    Site Outdoor Air Relative Humidity,  !- EnergyPlus Variable Name
    outRelHum;                  !- FMU Variable Name

ExternalInterface:FunctionalMockupUnitExport:From:Variable,
    ZONE ONE,                    !- EnergyPlus Key Value
    Zone Air Relative Humidity,  !- EnergyPlus Variable Name
    rooRelHum;                   !- FMU Variable Name 

Along with the FMU's inputs definition, the
corresponding output variables need to be specified in the idf file:
Output:Variable,
    Environment,                 !- Key Value
    Site Outdoor Air Drybulb Temperature,            !- Variable Name
    TimeStep;                    !- Reporting Frequency

Output:Variable,
    ZONE ONE,                    !- Key Value
    Zone Mean Air Temperature,   !- Variable Name
    TimeStep;                    !- Reporting Frequency 

Output:Variable,
    Environment,                 !- Key Value
    Site Outdoor Air Relative Humidity,   !- Variable Name
    TimeStep;                    !- Reporting Frequency

Output:Variable,
    ZONE ONE,                    !- Key Value
    Zone Air Relative Humidity,  !- Variable Name 
    TimeStep;                    !- Reporting Frequency

To define the outputs of the FMU, we use:
ExternalInterface:FunctionalMockupUnitImport:To:Schedule,
    FMU_OthEquSen_ZoneOne,   !- EnergyPlus Variable Name
    Any Number,              !- Schedule Type Limits Names
    MoistAir.fmu,            !- FMU Filename
    Model1,                  !- FMU Model Name
    QSensible,               !- FMU Model Variable Name
    0;                       !- Initial Value
    
    
ExternalInterface:FunctionalMockupUnitExport:To:Schedule,
    FMU_OthEquLat_ZoneOne,   !- EnergyPlus Variable Name
    Any Number,              !- Schedule Type Limits Names
    QSensible,               !- FMU Variable Name
    0;                       !- Initial Value
    
ExternalInterface:FunctionalMockupUnitExport:To:Schedule,
    FMU_OthEquSen_ZoneOne,   !- EnergyPlus Variable Name
    Any Number,              !- Schedule Type Limits Names
    QLatent,               !- FMU Variable Name
    0;                       !- Initial Value


ExternalInterface:FunctionalMockupUnitImport:To:Schedule,
    FMU_OthEquLat_ZoneOne,   !- EnergyPlus Variable Name
    Any Number,              !- Schedule Type Limits Names
    MoistAir.fmu,            !- FMU Filename
    Model1,                  !- FMU Model Name
    QLatent,                 !- FMU Model Variable Name
    0;                       !- Initial Value

Please see the Input/Output reference of the EnergyPlus manual (http://apps1.eere.energy.gov/buildings/energyplus/pdfs/inputoutputreference.pdf) 
to see how to configure an EnergyPlus model so that it can be exported as an FMU.

fixme: Explain coupling to slow varying states vs. fast varying states.

Testing of an FMU
-----------------

fixem: Explain how to test and debug an FMU.
