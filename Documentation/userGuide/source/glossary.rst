Glossary
========

.. glossary::

   Dymola
      Dymola, Dynamic Modeling Laboratory, is a modeling and simulation environment for the Modelica language.
      
   Functional Mock-up Interface
      The Functional Mock-up Interface (FMI) is the result of the Information Technology for European Advancement (ITEA2) project  :term:`MODELISAR`. 
      The FMI standard is a tool independent standard to support both model exchange and co-simulation of dynamic models using a combination of XML-files, 
      C-header files, C-code or binaries. 
      
   Functional Mock-up Unit
      A simulation model or program which implements the FMI standard is called Functional Mock-up Unit (FMU). 
      An FMU comes along with a small set of C-functions (FMI functions) whose input and return arguments are defined by the FMI standard. 
      These C-functions can be provided in source and/or binary form. The FMI functions are called by a simulator to create one or more instances of the FMU. 
      The functions are also used to run the FMUs, typically together with other models. An FMU may either require the importing tool 
      to perform numerical integration (model-exchange) or be self-integrating (co-simulation). An FMU is distributed in the form of a zip-file that contains
      shared libraries, which contain the implementation of the FMI functions and/or source code of the FMI functions,
      an XML-file, also called the model description file, which contains the variable definitions as well as meta-information of the model,
      additional files such as tables, images or documentation that might be relevant for the model.
      
   Modelica
      Modelica is a non-proprietary, object-oriented, equation-based language to conveniently model complex physical systems containing, 
      e.g., mechanical, electrical, electronic, hydraulic, thermal, control, electric power or process-oriented subcomponents.

   MODELISAR
      MODELISAR is an ITEA 2 (Information Technology for European Advancement) European project aiming to improve the design of systems and of embedded software in vehicles.

   PyFMI
      PyFMI is a package for loading and interacting with Functional Mock-Up Units (FMUs), which are compiled dynamic models compliant with the Functional Mock-Up Interface (FMI).
      
   Python
      Python is a dynamic programming language that is used in a wide variety of application domains.
