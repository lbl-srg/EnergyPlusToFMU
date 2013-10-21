//--- Compute MD5 checksums.
//
/// \author David Lorenzetti,
///         Lawrence Berkeley National Laboratory,
///         dmlorenzetti@lbl.gov
///
/// \brief  Compute MD5 checksums.


#if !defined(__DIGEST_MD5__)
#define __DIGEST_MD5__


//--- Copyright notice.
//
//   This code is based on the reference MD5 implementation described by RFC 1321.
// See http://256.com/sources/md5/rfc1321.txt
//
//   Here is the original copyright notice from that implementation:
//
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
//
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.


//--- Type for internal calculations.
//
//   MD5 definition requires storing state in four-byte (32-bit) registers.
// This may require defining preprocessor symbol {INT32} at compile-time.
//
#ifndef INT32
  #define INT32 0
#endif
//
#if( 0 == INT32 )
  typedef unsigned int UINT4;
#elif( 1 == INT32 )
  typedef unsigned long int UINT4;
#elif( 2 == INT32 )
  typedef unsigned short int UINT4;
#elif( 199901L <= __STDC_VERSION__ )
  #include <stdint.h>
  typedef uint32_t UINT4;  // This is optional-- not guaranteed to exist.
#endif


//-- MD5 context.
//
typedef struct {
  UINT4 state[4];  // State (ABCD).
  UINT4 count[2];  // Number of bits, modulo 2^64 (lsb first).
  unsigned char buffer[64];  // Input buffer.
  } MD5_CTX;


//--- Find MD5 checksum for a string, in hex format.
//
//   Perform a complete checksum, using the low-level routines declared below.
//   Return a 32-character hex string (plus a terminating null).
//
void digest_md5_fromStr(const char *const string, char hexDigestStr[33]);


//--- Find MD5 checksum for a file, in hex format.
//
//   Perform a complete checksum, using the low-level routines declared below.
//   Return a 32-character hex string (plus a terminating null).
//
void digest_md5_fromFile(const char *const fileName, char hexDigestStr[33]);


//--- Prepare to find an MD5 checksum.
//
//   Initialize the context.
//
void digest_md5_lowLevel_init(MD5_CTX *const context);


//--- Update MD5 checksum to reflect a new string of input.
//
//   Continue an MD5 message-digest operation, processing another message
// block, and updating the context.
//
void digest_md5_lowLevel_update(MD5_CTX *const context,
  const unsigned char *const input, const unsigned int inputLen);


//--- Retrieve a completed MD5 checksum.
//
//   Write the message digest, and clear the context.
//
void digest_md5_lowLevel_finish(MD5_CTX *const context, unsigned char digest[16]);


//--- Convert an MD5 checksum digest to a hex string.
//
//   Note {hexDigestStr} must have at least 33 characters-- 32 for the hex
// result, and one extra for the terminating null character ('\0').
//
void digest_md5_lowLevel_toHex(const unsigned char digest[16], char *hexDigestStr);


#endif // __DIGEST_MD5__
