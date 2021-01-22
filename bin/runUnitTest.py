import os, sys
import subprocess as sp
try:
    from pyfmi import load_fmu
except BaseException:
    print ('PyFMI not installed. Test will not be be run.')
import re
# Check if the requirements are met to run the tests
# Python
# pyFMI
# C Compiler and linker
# C++ Compiler and linker

#--- Fcn to sanitize an identifier.
#
#   Make an identifier acceptable as the name of a C function.
#   In C, a function name:
# ** Can contain any of the characters {a-z,A-Z,0-9,_}.
# ** Cannot start with a number.
# ** Can contain universal character names from the ISO/IEC TR 10176 standard.
# However, universal character names are not supported here.
#
g_rexBadIdChars = re.compile(r'[^a-zA-Z0-9_]')
#
def sanitizeIdentifier(identifier):
  #
  if( len(identifier) <= 0 ):
    quitWithError('Require a non-null identifier', False)
  #
  # Can't start with a number.
  if( identifier[0].isdigit() ):
    identifier = 'f_' +identifier
  #
  # Replace all illegal characters with an underscore.
  identifier = g_rexBadIdChars.sub('_', identifier)
  #
  return( identifier )
  #
  # End fcn sanitizeIdentifier().

#--- Fcn to run the FMUs.
#
#   This function runs the FMUs twice using PyFMI.
#   In the first run, the FMU will be run using the low
#  level PyFMI function do_step(). In the second run, the
#  high level python function simulate() will be used.
#
def run_fmu(fmu_path, api, exa):
    model="model_1_"+api
    model = load_fmu(fmu=fmu_path)
    start_time=0
    final_time=60*60*72
    #time_step=900
    if (api=="1"):
        print(('Running FMU file={!s}, API={!s}.'.format(fmu_path, api)))
        #model.setup_experiment(False, 0, 0, True, final_time)
        model.initialize(0, final_time)
    elif(api=="2"):
        print(('Running FMU file={!s}, API={!s}.'.format(fmu_path, api)))
        model.setup_experiment(False, 0, 0, True, final_time)
        model.initialize(0, final_time)
    else:
        print ('Only FMI version 1.0 and 2.0 are supported')
        exit(1);
    tim = 0
    while tim < final_time:
        if(exa=='Schedule'):
            model.set ('Q', tim/50)
            model.do_step(tim, 900, True)
            print((model.get ('TRooMea')));
            tim=tim+900
        elif(exa=='Actuator'):
            # change input variable (default in .idf is given to be 6)
            model.set('yShade', 6.0)
            model.do_step(tim, 600, True)
            print((model.get ('TRoo')));
            tim=tim+600
        else:
            # change input variable (default in .idf is given to be 6)
            model.set('yShadeFMU', 6.0)
            model.do_step(tim, 600, True)
            print((model.get ('TRoo')));
            tim=tim+600
    model.terminate()
    # model.free_instance(). This causes the simulation
    # to crash on Linux. Hence it is disabled here.

    # RERUN the FMUs
    # load .fmu with pyfmi
    model="model_2_"+api
    model = load_fmu(fmu=fmu_path)

    # get options object
    opts = model.simulate_options()

    # set number of communication points dependent on final_time and .idf steps per hour
    final_time = 60*60*72 # 72 hour simulation
    if(exa=='Schedule'):
        idf_steps_per_hour = 4
        ncp = 288
    else:
        idf_steps_per_hour = 6
        ncp = 432
    #ncp = final_time/(3600./idf_steps_per_hour)
    opts['ncp'] = ncp

    # run simulation and return results
    res = model.simulate(start_time=0., final_time=final_time, options=opts)
    if(exa=='Schedule'):
        print(('The computed room temperature={!s}'.format(res['TRooMea']))) # show result variables names
    else:
        print(('The computed room temperature={!s}'.format(res['TRoo']))) # show result variables names
  #
  # End fcn run_fmu().
unittest_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(unittest_path, '..'))
SCRIPT_PATH=os.path.join(root_path, "Scripts", "EnergyPlusToFMU.py")
# Check if the script file exists
if (os.path.exists(SCRIPT_PATH)):
    print(('The Path to the script file={!s}.'.format(SCRIPT_PATH)))
else:
    print(('The script file={!s} does not exist.'.format(SCRIPT_PATH)))
    exit(1)

# Get the environment variables
pat=os.getenv('PATH')
# Create the list with the variables
EP_SYS=False
spl_pat=pat.split(';')
for i in range(len(spl_pat)):
    if ("EnergyPlus" in spl_pat[i]):
        EP_SYS = True
        ep_alt = input('EnergyPlus version ' + spl_pat[i] + ' was found.' +
        ' This version will be used for exporting and running EnergyPlus FMUs. '
        ' If you want to use a different version, provide the full path of the version now.' +
        ' Otherwise press return: ')
        if (ep_alt is not None):
            # Check if the provided path exists
            if (os.path.exists(ep_alt)):
                print(('The Path to the EnergyPlus version provided={!s}.'.format(ep_alt)))
            else:
                print(('The Path to the EnergyPlus version provided={!s} does not exist.'
                'The EnergyPlus version found={!s} will be used.'.format(ep_alt, spl_pat[i])))
        # CONSTRUCT THE IDD-PATH
        IDD_PATH=os.path.join(spl_pat[i], "Energy+.idd")
        # CONSTRUCT THE IDD-PATH
        WEA_PATH=os.path.join(spl_pat[i], "WeatherData", "USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw")
        break;
if(EP_SYS==False):
    ep_alt = raw_input('EnergyPlus version was not found.' +
    ' Please provide the full path of the installation folder of EnergyPlus: ')
    if (ep_alt is not None):
        # Check if the provided path exists
        if (os.path.exists(ep_alt)):
            print(('The Path to the EnergyPlus version provided={!s}.'.format(ep_alt)))
        else:
            print ('The Path to the EnergyPlus version provided={!s} does not exist.')
            exit(1)
    # CONSTRUCT THE IDD-PATH
    IDD_PATH=os.path.join(ep_alt, "Energy+.idd")
    # CONSTRUCT THE IDD-PATH
    WEA_PATH=os.path.join(ep_alt, "WeatherData", "USA_CA_San.Francisco.Intl.AP.724940_TMY3.epw")

# Check if the IDD file exists
if (os.path.exists(IDD_PATH)):
    print(('The Path to the IDD file={!s}.'.format(IDD_PATH)))
else:
    print(('The IDD file={!s} does not exist.'.format(IDD_PATH)))
    exit(1)
# Check if the IDD file exists
if (os.path.exists(WEA_PATH)):
    print(('The Path to the Weather file={!s}.'.format(WEA_PATH)))
else:
    print(('The Weather file={!s} does not exist.'.format(WEA_PATH)))
    exit(1)

# Check if the example file exists
for exa in ['Schedule', 'Actuator', 'Variable']:
    if(exa=='Schedule'):
        EXAMPLE_PATH=os.path.join(root_path, "Examples", "Schedule", "_fmu-export-schedule.idf")
    elif(exa=='Actuator'):
        EXAMPLE_PATH=os.path.join(root_path, "Examples", "Actuator", "_fmu-export-actuator.idf")
    else:
        EXAMPLE_PATH=os.path.join(root_path, "Examples", "Variable", "_fmu-export-variable.idf")
    #EXAMPLE_PATH=os.path.join(root_path, "Examples", "Schedule", "_fmu-export-schedule.idf")
    if (os.path.exists(EXAMPLE_PATH)):
        print(('The Path to the example file={!s}.'.format(EXAMPLE_PATH)))
    else:
        print(('The example file={!s} does not exist.'.format(EXAMPLE_PATH)))
        exit(1)

    # Loop to export the idfs and the idds files for FMI version 1 and 2.
    #if (sys.version_info.major==2):
    for i in ["2", "1"]:
        tmp=os.path.join(unittest_path,'v_'+i)
        if not os.path.exists(tmp):
            print(('Create directory={!s} to run the unit test.'.format(tmp)))
            os.makedirs(tmp)
        elif (os.path.exists(tmp)):
            print(('Directory={!s} exists and will be deleted and recreated.'.format(tmp)))
            import shutil
            shutil.rmtree(tmp)
            os.makedirs(tmp)
        # Change directory to run the unit test
        os.chdir(tmp)
        # export the FMUs
        sp.call(["python", SCRIPT_PATH, "-i", IDD_PATH, "-w", WEA_PATH, "-a", i, EXAMPLE_PATH])
        base=os.path.basename(EXAMPLE_PATH)
        filename=os.path.splitext(base)[0]
        FMU_PATH=os.path.join(os.getcwd(), sanitizeIdentifier(filename))+".fmu"
        print(('The generated FMU file={!s}.'.format(FMU_PATH)))
        # Running the FMUs
        run_fmu(FMU_PATH, i, exa)
