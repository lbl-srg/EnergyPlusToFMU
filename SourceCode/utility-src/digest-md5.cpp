//--- Compute MD5 checksums.


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
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
//
// Basic MD5 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
//
// Rotate x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
//
// Transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition, to prevent recomputation.
//
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }


//--- File-scope constants.


//--- File-scope function prototypes.
//
static void md5_transform(UINT4 state[4], const unsigned char block[64]);
static void md5_encode(unsigned char *const output,
  const UINT4 *const input, const unsigned int len);
static void md5_decode(UINT4 *const output,
  const unsigned char *const input, const unsigned int len);
static void md5_memcpy(unsigned char *const output,
  const unsigned char *const input, const unsigned int len);
static void md5_memset(unsigned char *const output,
  const int value, const unsigned int len);


//--- Functions.


//--- Find MD5 checksum for a string, in hex format.
//
void digest_md5_fromStr(const char *const string, char hexDigestStr[33])
  {
  const unsigned int len = strlen(string);
  unsigned char digest[16];
  MD5_CTX context;
  //
  digest_md5_lowLevel_init(&context);
  digest_md5_lowLevel_update(&context, (unsigned char *)string, len);
  digest_md5_lowLevel_finish(&context, digest);
  //
  digest_md5_lowLevel_toHex(digest, hexDigestStr);
  }  // End fcn digest_md5_fromStr().


//--- Find MD5 checksum for a file, in hex format.
//
void digest_md5_fromFile(const char *const fileName, char hexDigestStr[33])
  {
  FILE *file;
  //
  file = fopen (fileName, "rb");
  if( file != NULL )
    {
    MD5_CTX context;
    int len;
    unsigned char buffer[1024];
    unsigned char digest[16];
    digest_md5_lowLevel_init(&context);
    while( (len=fread(buffer, 1, 1024, file)) )
      {
      digest_md5_lowLevel_update(&context, buffer, len);
      }
    digest_md5_lowLevel_finish(&context, digest);
    fclose(file);
    digest_md5_lowLevel_toHex(digest, hexDigestStr);
    }
  else
    {
    for( int idx=0; idx<32; ++idx )
      hexDigestStr[idx] = '0';
    hexDigestStr[32] = '\0';
    }
  }  // End fcn digest_md5_fromFile().


//--- Prepare to find an MD5 checksum.
//
void digest_md5_lowLevel_init(MD5_CTX *const context)
  {
  context->count[0] = context->count[1] = 0;
  //
  // Load magic initialization constants.
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
  }  // End fcn digest_md5_lowLevel_init().


//--- Update MD5 checksum to reflect a new string of input.
//
void digest_md5_lowLevel_update(MD5_CTX *const context,
  const unsigned char *const input, const unsigned int inputLen)
  {
  unsigned int i, index, partLen;
  //
  // Compute number of bytes mod 64.
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);
  //
  // Update number of bits.
  if( (context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3) )
    context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);
  //
  partLen = 64 - index;
  //
  // Transform as many times as possible.
  if( inputLen >= partLen )
    {
    md5_memcpy((unsigned char *)&context->buffer[index], (unsigned char *)input, partLen);
    md5_transform(context->state, context->buffer);
    for( i=partLen; i+63<inputLen; i+=64 )
      {
      md5_transform(context->state, &input[i]);
      }
    index = 0;
    }
  else
    {
    i = 0;
    }
  //
  // Buffer remaining input.
  md5_memcpy((unsigned char *)&context->buffer[index], (unsigned char *)&input[i], inputLen-i);
  }  // End fcn digest_md5_lowLevel_update().


//--- Retrieve a completed MD5 checksum.
//
void digest_md5_lowLevel_finish(MD5_CTX *const context, unsigned char digest[16])
  {
  unsigned char bits[8];
  unsigned int index, padLen;
  static unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
  //
  // Save number of bits.
  md5_encode(bits, context->count, 8);
  //
  // Pad out to 56 mod 64.
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  digest_md5_lowLevel_update(context, PADDING, padLen);
  //
  // Append length (before padding).
  digest_md5_lowLevel_update(context, bits, 8);
  //
  // Store state in digest.
  md5_encode(digest, context->state, 16);
  //
  // Clear sensitive information.
  md5_memset((unsigned char *)context, 0, sizeof (*context));
  //
  // Ensure obvious problem in case a type mismatch makes work not done
  // using 32-bit registers in buffer.
  //   Note operator sizeof() gives size relative to that of {unsigned char}.
  // Assume that {unsigned char} is 8 bits.
  if( 4 != sizeof(UINT4) )
    {
    md5_memset(digest, 0, 16);
    }
  }  // End fcn digest_md5_lowLevel_finish().


//--- Convert an MD5 checksum digest to a hex string.
//
void digest_md5_lowLevel_toHex(const unsigned char digest[16], char *hexDigestStr)
  {
  unsigned int id;
  //
  // Convert every character in {digest} into two hex characters.
  for( id=0; id<16; id++, hexDigestStr+=2 )
    {
    sprintf(hexDigestStr, "%02x", digest[id]);
    }
  //
  // Terminate the string.
  *hexDigestStr = '\0';
  }  // End fcn digest_md5_lowLevel_toHex().


//--- Perform basic MD5 transformation.
//
//   Transform {state} based on {block}.
//
static void md5_transform(UINT4 state[4], const unsigned char block[64])
  {
  UINT4 a = state[0];
  UINT4 b = state[1];
  UINT4 c = state[2];
  UINT4 d = state[3];
  UINT4 x[16];
  //
  md5_decode(x, block, 64);
  //
  // Round 1.
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
  //
  // Round 2.
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
  //
  // Round 3.
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */
  //
  // Round 4.
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */
  //
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  //
  // Clear sensitive information.
  md5_memset((unsigned char *)x, 0, sizeof (x));
  }  // End fcn md5_transform().


//--- Encode {input} into {output}.
//
//   Assume {len} is a multiple of 4.
//
static void md5_encode(unsigned char *const output,
  const UINT4 *const input, const unsigned int len)
  {
  unsigned int i, j;
  //
  for( i=0, j=0; j<len; i++, j+=4 )
    {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
  }  // End fcn md5_encode().


//--- Decode {input} into {output}.
//
//   Assume {len} is a multiple of 4.
//
//   Note: consider replacing for-loop with standard memcpy.
//
static void md5_decode(UINT4 *const output,
  const unsigned char *const input, const unsigned int len)
  {
  unsigned int i, j;
  //
  for( i=0, j=0; j<len; i++, j+=4 )
    {
    output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
      (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
    }
  }  // End fcn md5_decode().


//--- Copy {input} to {output}.
//
//   Note: consider replacing with standard memcpy.
//
static void md5_memcpy(unsigned char *const output,
  const unsigned char *const input, const unsigned int len)
  {
  unsigned int i;
  //
  for( i=0; i<len; i++ )
    {
    output[i] = input[i];
    }
  }  // End fcn md5_memcpy().


//--- Set {output} to {value}.
//
//   Note: consider replacing for-loop with standard memcpy.
//
// hoho  Here and elsewhere, {len} probably should be {size_t}.
//
// hoho  Why is {value} an {int} rather than an {unsigned char}?
//
// hoho  Since this is only used to clear things, should just call it clear, and
// skip argument {value}.
//
static void md5_memset(unsigned char *const output,
  const int value, const unsigned int len)
  {
  unsigned int i;
  //
  for( i=0; i<len; i++ )
    {
    ((char *)output)[i] = (char)value;
    }
  }  // End fcn md5_memset().
