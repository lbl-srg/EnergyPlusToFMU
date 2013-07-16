Glossary
========

.. glossary::

   Functional Mock-up Unit
      A simulation model or program which implements the FMI standard is called FMU (Functional Mock-up Unit). 
      An FMU comes along with a small set of easy to use C-functions (FMI functions) whose input and return arguments are defined by the FMI standard}. 
      These C-functions can be provided in source and/or binary form. The FMI functions are called by a simulator to create one or more instances of the FMU. 
      The functions are also used to run the FMUs, typically together with other models. An FMU may either require the simulator 
      to perform numerical integration (model-exchange) or be self-integrating (co-simulation). An FMU is distributed in the form of a zip-file that contains
      shared libraries, which contain the implementation of the FMI functions and/or source code of the FMI functions,
      an XML-file, also called the model description file, which contains the variable definitions as well as meta-information of the model,
      additional files such as tables, images or documentation that might be relevant for the model.


   Functional Mock-up Interface
      The Functional Mock-up Interface (FMI) is the result of the Information Technology for European Advancement (ITEA2) project MODELISAR. 
      The FMI standard is a tool independent standard to support both model exchange and co-simulation of dynamic models using a combination of XML-files, 
      C-header files, and C-code in source or binary form. 