//--- Unit test for digest-md5.c.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
/// \brief  Unit test for digest-md5.c.


//--- Copyright notice.
//
//   Please see the header file.


//--- Includes.
//
#include <stdio.h>
#include <string.h>


#include "digest-md5.h"


//--- Preprocessor definitions.
//
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000


//--- File-scope constants.


//--- File-scope function prototypes.
//
static void check_md5_string(const char *const string, const char *const expectHexDigest);
static void print_md5_file(const char *const fileName);


//--- Functions.


//--- Main driver.
//
//   Optional argument names a file for which to compute the MD5 checksum.
//
int main(int argc, const char *argv[])
  {
  //
  //-- Find checksums of some standard strings.
  check_md5_string("", "d41d8cd98f00b204e9800998ecf8427e");
  //
  check_md5_string("a", "0cc175b9c0f1b6a831c399e269772661");
  check_md5_string("A", "7fc56270e7a70fa81a5935b72eacbe29");
  //
  check_md5_string("abc", "900150983cd24fb0d6963f7d28e17f72");
  check_md5_string("Abc", "35593b7ce5020eae3ca68fd5b6f3e031");
  check_md5_string("aBc", "dbbbbe4975e026e04a687871f296a2b2");
  check_md5_string("abC", "36cf1fa7e384dfd385f07a50ffdf0418");
  check_md5_string("AbC", "25aa3ee1c93cad3f274567281066dc18");
  //
  check_md5_string("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
  //
  check_md5_string("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
  check_md5_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "d174ab98d277d9f5a5611c2c9f419d9f");
  //
  check_md5_string("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
    "57edf4a22be3c955ac49da2e2107b67a");
  //
  //-- Find checksum of a file, if listed on command line.
  if( argc > 1 )
    {
    int i;
    for( i=1; i<argc; i++ )
      {
      print_md5_file(argv[i]);
      }
    }
  else
    {
    printf("Note naming files on command line will print their MD5 checksums.\n");
    }
  }  // End fcn main().


//--- Check the MD5 checksum for a string.
//
static void check_md5_string(const char *const string, const char *const expectHexDigest)
  {
  char hexDigestStr[33];
  //
  digest_md5_fromStr(string, hexDigestStr);
  //
  if( 0 != strcmp(expectHexDigest, hexDigestStr) )
    {
    printf("Error, for string '%s', expecting hex digest %s, got %s\n",
      string, expectHexDigest, hexDigestStr);
    }
  }  // End fcn check_md5_string().


//--- Find MD5 checksum for a file.
//
static void print_md5_file(const char *const fileName)
  {
  char hexDigestStr[33];
  digest_md5_fromFile(fileName, hexDigestStr);
  printf("File %s has MD5 checksum (all zeros means error opening): %s\n", fileName, hexDigestStr);
  }  // End fcn print_md5_file().
