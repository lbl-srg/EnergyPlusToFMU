.. highlight:: rest

.. _mathematics:


Mathematical Description
========================

This section describes the algorithm for exchanging data between EnergyPlus, packaged as an FMU, and an external program.

Suppose we have a system with two simulation programs.  Let simulation program 1 be EnergyPlus, the slave simulation program, which is packaged as an FMU for co-simulation; and let simulation program 2 be the master simulation program which supports the import of FMU for co-simulation. Suppose each program solves an initial-value ordinary differential equation that is coupled to the differential equations of the other program. 

Let :math:`N` denote the number of time steps and let :math:`k` denote the time step. We will use the subscripts 1 and 2 to denote the state variable and the function that computes the next value of the state variable of the simulator 1 and 2, respectively.
Then program 1 computes the sequence

	:math:`x_{1}(k+1) = f_{1}(x_{1}(k), x_{2}(k))`,

and, similarly, program 2 computes the sequence

	:math:`x_{2}(k+1) = f_{2}(x_{2}(k), x_{1}(k))`,

with initial conditions :math:`x_{1}(0) = x_{1,0}` and :math:`x_{2}(0) = x_{2,0}`.

To advance from time :math:`k` to :math:`k+1`, each program uses its own integration algorithm. At the end of the time step, program 1 sends its new state :math:`x_{1}(k+1)` to program 2, and receives the state :math:`x_{2}(k+1)` from program 2. 
The same procedure is done with the program 2. Program 2, which is the master simulation program, imports the FMU, and manages the data exchange between the two programs. 
In comparison to numerical methods of differential equations, this scheme is equal to an explicit Euler integration, which is an integration algorithm that solves an ordinary differential equation with specified initial values,

	:math:`dx/dt = h(x)`, 

	:math:`x(0)  = x_{0}`,

on the time interval :math:`t \in [0, 1]`, using he following steps:


**Step 0:**	

	Initialize counter :math:`k=0` and number of steps, :math:`N > 0`.
			
	Set initial state :math:`x(k) = x_{0}` and set time step  :math:`\Delta t = 1/N`.


**Step 1:**	

	Compute new state  :math:`x(k+1) = x(k) + h(x(k)) \Delta t`.

	Replace :math:`k` by :math:`k+1`.

**Step 2:**	

	If :math:`k=N` stop, else go to Step 1.

However, this scheme does not require each simulation tool to use the explicit Euler method for its internal time integration algorithm; the analogy to explicit Euler applies only to the data exchange between the programs.  In the situation where the differential equation is solved using co-simulation, the above algorithm becomes

**Step 0:**	

	Initialize counter :math:`k=0` and number of steps, :math:`N > 0`.

	Set initial state :math:`x_{1}(k) = x_{1,0}` and :math:`x_{2}(k) = x_{2,0}`. Set the time step  :math:`\Delta t = 1/N`.

**Step 1:**	

	Compute new states

	:math:`x_{1}(k+1) = x_{1}(k) + f_{1}(x_{1}(k), x_{2}(k)) \Delta t`, and

	:math:`x_{2}(k+1) = x_{2}(k) + f_{2}(x_{2}(k), x_{1}(k)) \Delta t`.

	Replace :math:`k` by :math:`k+1`.

**Step 2:**	

	If :math:`k=N` stop, else go to Step 1.

