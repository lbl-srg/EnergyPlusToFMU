.. highlight:: rest

.. _introduction:

Introduction
============
This user manual explains how to install, and use EnergyPlusToFMU.
EnergyPlusToFMU is software package written in Python which allows users to export the whole building simulation program EnergyPlus as a :term:`Functional Mockup Unit` (FMU) for co-simulation.
This FMU can then be imported into a variety of simulation programs that support the import of the :term:`Functional Mockup Interface` for co-simulation. This capability allows for instance to model the envelope of a building in 
EnergyPlus, export this model as an FMU, and import and link this model with an HVAC system model developed in a system simulation tool such as Modelica/Dymola.

