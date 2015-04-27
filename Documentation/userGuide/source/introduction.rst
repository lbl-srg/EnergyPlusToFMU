.. highlight:: rest

.. _introduction:

Introduction
============
This user manual explains how to install and use EnergyPlusToFMU.
EnergyPlusToFMU is a software package written in :term:`Python` which allows users to export the building simulation program `EnergyPlus <http://apps1.eere.energy.gov/buildings/energyplus/?utm_source=EnergyPlus&utm_medium=redirect&utm_campaign=EnergyPlus%2Bredirect%2B1>`_ version 8.0 or higher as a :term:`Functional Mock-up Unit` (FMU) for co-simulation using the :term:`Functional Mock-up Interface` (FMI) 
standard `version 1.0 <https://svn.modelica.org/fmi/branches/public/specifications/v1.0/FMI_for_ModelExchange_v1.0.pdf>`_.
This FMU can then be imported into a variety of simulation programs that support the import of the :term:`Functional Mock-up Interface` for co-simulation. This capability allows for instance to model the envelope of a building in 
EnergyPlus, export the model as an FMU, import and link the model with an HVAC system model developed in a system simulation tool such as the :term:`Modelica` environment :term:`Dymola`.

