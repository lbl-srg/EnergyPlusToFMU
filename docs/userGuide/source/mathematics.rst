.. highlight:: rest

.. _mathematics:

Mathematical Description
========================

fixme: Explain when variables are updated in E+.

Note: In FMU, :math:`x(t_i)` is the value of :math:`x(\cdot)` for
:math:`t \in [t_i, \, t_{i+1})`, but in EnergyPlus, I believe 
that discrete variables are not yet updated when the data are exchanged.

This section describes the algorithm for exchanging data between EnergyPlus, packaged as an FMU, and an external program.

Suppose we have a system with two simulation programs.  Let simulation program 1 be EnergyPlus, the slave simulation program, which is packaged as an FMU for co-simulation; and let simulation program 2 be the master simulation program which supports the import of FMU for co-simulation. Suppose each program solves an initial-value ordinary differential equation that is coupled to the differential equations of the other program. 

Let   denote the number of time steps and let   denote the time steps. We will use the subscripts 1 and 2 to denote the state variable and the function that computes the next state variable of the simulator 1 and 2, respectively.
Then program 1 computes, for  the sequence
x1(k+1) = f1(x1(k), x2(k))
and, similarly, program 2 computes the sequence
x2(k+1) = f2(x2(k), x1(k))
with initial conditions x1(0) = x1,0 and x2(0) = x2,0.

To advance from time k to k+1, each program uses its own integration algorithm. At the end of the time step, program 1 sends its new state x1(k+1) to program 2, and receives the state x2(k+1) from program 2. The same procedure is done with the program 2. Program 2, which is the master simulation program, imports the FMU, and manages the data-exchange between the two programs. 
In comparison to numerical methods of differential equations, this scheme resembles an explicit Euler integration, which is an integration algorithm that solves an ordinary differential equation with specified initial values,
dx/dt = h(x), 
x(0)  = x0,
on the time interval t ? [0, 1], the following sequence:
Step 0:	Initialize counter k=0 and number of steps  .

	Set initial state x(k) = x0 and set time step ?t = 1/N.
Step 1:	Compute new state x(k+1) = x(k) + h(x(k)) ?t.
	Replace k by k+1.
Step 2:	If k=N stop, else go to Step 1.

However, this scheme does not require each simulation tool to use explicit Euler for its internal time-stepping; the analogy to explicit Euler applies only to the data exchange between programs.  In the situation where the differential equation is solved using co-simulation, the above algorithm becomes
Step 0:	Initialize counter k=0 and number of steps  .

	Set initial state x1(k) = x1,0 and x2(k) = x2,0. Set the time step ?t = 1/N.
Step 1:	Compute new states
  x1(k+1) = x1(k) + f1(x1(k), x2(k)) ?t, and
  x2(k+1) = x2(k) + f2(x2(k), x1(k)) ?t.
	Replace k by k+1.
Step 2:	If k=N stop, else go to Step 1.

