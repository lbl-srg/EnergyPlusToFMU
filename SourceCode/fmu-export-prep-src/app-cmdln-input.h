//--- Get inputs for command-line app to export an EnergyPlus simulation as an FMU.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  Get command-line inputs.


#if !defined(__APP_CMDLN_INPUT__)
#define __APP_CMDLN_INPUT__


//--- Includes.
//
#include <sstream>


//--- Types.
//
typedef struct cmdlnInput_s
  {
  const char *iddFileName;
  const char *idfFileName;
  const char *wthFileName;
  } cmdlnInput_s;


//--- Get command-line inputs.
//
//   Require exactly two arguments:
// ** The path to an EnergyPlus input data dictionary (IDD).
// ** The path to an EnergyPlus input data file (IDF).
//
//   Accept following command-line switches:
// ** -v, print version information.
// ** -h, print help.
// ** -w, path to a weather file.
//
//   Arguments:
// ** {argc}, count of strings in array {argv}, as in the standard call of main().
// ** {argv}, array of strings, as in the standard call of main().
// ** {cmdlnInputP}, pointer to structure that will hold command-line inputs
// to be returned to caller.  Note no memory will be allocated to the ptrs in
// the struct; they are merely set to point to strings in {argv}.
// ** {errFcn}, an error function to be used to report any errors.  If NULL,
// use {stderr}.
//
//   Return {TRUE} on successful handling of command-line, i.e., if able to
// return the required inputs.  Return {FALSE} if failed to get exactly the
// required arguments.
//
bool cmdlnInput_get(const int argc, const char *argv[],
  cmdlnInput_s *const cmdlnInputP,
  void (*errFcn)(std::ostringstream& errorMessage));


#endif // __APP_CMDLN_INPUT__
