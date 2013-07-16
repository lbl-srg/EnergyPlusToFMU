//--- Command-line executable to show size of a memory address.
//
//   I.e., was the executable built as a 32-bit or 64-bit application?
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Get memory address size, in bits.


//--- Includes.
//
#include <stdio.h>


//--- Main driver.
//
int main(int argc, const char* argv[]) {
  //
  // Fcn sizeof() returns number of bytes.
  //   Convert to bits using 8 bits/byte.
  const int addressSize = 8 * (int)sizeof(void *);
  //
  printf("%d", addressSize);
  //
  return( 0 );
  }  // End fcn main().
