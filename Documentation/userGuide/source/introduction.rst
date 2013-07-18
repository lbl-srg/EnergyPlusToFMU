.. highlight:: rest

.. _introduction:

Introduction
============
This user manual explains how to install, and use EnergyPlusToFMU.
EnergyPlusToFMU is a software package written in Python which allows users to export the whole building simulation program EnergyPlus as a :term:`Functional Mock-up Unit` (FMU) for co-simulation with `Functional Mock-up Interface` (FMI) 
for co-simulation `version 1.0 <https://svn.modelica.org/fmi/branches/public/specifications/FMI_for_CoSimulation_v1.0.pdf>`_ Application Programming Interface.
This FMU can then be imported into a variety of simulation programs that support the import of the :term:`Functional Mock-up Interface` for co-simulation. This capability allows for instance to model the envelope of a building in 
EnergyPlus, export the model as an FMU, import and link the model with an HVAC system model developed in a system simulation tool such as Modelica/Dymola.

