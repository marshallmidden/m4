
/* MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
   rights reserved.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
*/

/* The following makes MD default to MD5 if it has not already been
   defined with C compiler flags.
*/
#ifndef MD
#define MD 5
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "chap_md5global.h"
#if MD == 2
#include "md2.h"
#endif
#if MD == 4

#include "md4.h"
#endif
#if MD == 5
#include "chap_md5.h"
#endif

/* Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 10000
#define TEST_BLOCK_COUNT 20000

int main_md5 (int argc, char* argv[]);
static void MDString  (char *);
static void MDTimeTrial  (void);
static void MDTestSuite  (void);
static void MDFile  (char *);
static void MDFilter  (void);
static void MDPrint  (unsigned char [16]);

#if MD == 2
#define MD_CTX MD2_CTX
#define MDInit MD2Init
#define MDUpdate MD2Update
#define MDFinal MD2Final
#endif
#if MD == 4
#define MD_CTX MD4_CTX
#define MDInit MD4Init
#define MDUpdate MD4Update
#define MDFinal MD4Final
#endif
#if MD == 5
#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
#endif

/* Main driver.

Arguments (may be any combination):
-sstring - digests string
-t       - runs time trial
-x       - runs test script
filename - digests file
(none)   - digests standard input
*/
int main_md5 (int argc, char* argv[])
{
     int i;

     if (argc > 1)
          for (i = 1; i < argc; i++)
               if (argv[i][0] == '-' && argv[i][1] == 's')
                    MDString (argv[i] + 2);
               else if (strcmp (argv[i], "-t") == 0)
                    MDTimeTrial ();
               else if (strcmp (argv[i], "-x") == 0)
                    MDTestSuite ();
               else
                    MDFile (argv[i]);
     else
          MDFilter ();

     return (0);
}

/* Digests a string and prints the result.
 */
static void MDString (char* string)
{
     MD_CTX context;
     unsigned char digest[16];
     unsigned int len = strlen (string);

     MDInit (&context);
     MDUpdate(&context, (unsigned char *)string, len);
     MDFinal (digest, &context);

     printf ("MD%d (\"%s\") = ", MD, string);
     MDPrint (digest);
     printf ("\n");
}

/* Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte
   blocks.
*/
static void MDTimeTrial (void)
{
     MD_CTX context;
     time_t endTime, startTime;
     unsigned char block[TEST_BLOCK_LEN], digest[16];
     unsigned int i;

     printf
          ("MD%d time trial. Digesting %d %d-byte blocks ...", MD,
           TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

     /* Initialize block */
     for (i = 0; i < TEST_BLOCK_LEN; i++)
          block[i] = (unsigned char)(i & 0xff);

     /* Start timer */
     time (&startTime);

     /* Digest blocks */
     MDInit (&context);
     for (i = 0; i < TEST_BLOCK_COUNT; i++)
          MDUpdate (&context, block, TEST_BLOCK_LEN);
     MDFinal (digest, &context);

     /* Stop timer */
     time (&endTime);

     printf (" done\n");
     printf ("Digest = ");
     MDPrint (digest);
     printf ("\nTime = %ld seconds\n", (long)(endTime-startTime));
     printf
          ("Speed = %ld bytes/second\n",
           (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT/(endTime-startTime));
}

/* Digests a reference suite of strings and prints the results.
 */
static void MDTestSuite (void)
{
     printf ("MD%d test suite:\n", MD);

     MDString ((char *) "");
     MDString ((char *) "a");
     MDString ((char *) "abc");
     MDString ((char *) "message digest");
     MDString ((char *) "abcdefghijklmnopqrstuvwxyz");
     MDString
          ((char *) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
     MDString
          ((char *) "1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890");
}

/* Digests a file and prints the result.

*/
static void MDFile (char* filename)
{
     FILE *file;
     MD_CTX context;
     int len;
     unsigned char buffer[1024], digest[16];

     if ((file = fopen (filename, "rb")) == NULL)
          printf ("%s can't be opened\n", filename);

     else {
          MDInit (&context);
          while ((len = fread (buffer, 1, 1024, file)))
               MDUpdate (&context, buffer, len);
          MDFinal (digest, &context);

          fclose (file);

          printf ("MD%d (%s) = ", MD, filename);
          MDPrint (digest);
          printf ("\n");
     }
}

/* Digests the standard input and prints the result.
 */
static void MDFilter (void)
{
     MD_CTX context;
     int len;
     unsigned char buffer[16], digest[16];

     MDInit (&context);
     while ((len = fread (buffer, 1, 16, stdin)))
          MDUpdate (&context, buffer, len);
     MDFinal (digest, &context);

     MDPrint (digest);
     printf ("\n");
}

/* Prints a message digest in hexadecimal.
 */
static void MDPrint (unsigned char digest[16])
{

     unsigned int i;

     for (i = 0; i < 16; i++)
          printf ("%02x", digest[i]);
}


/*
  A.5 Test suite

  The MD5 test suite (driver option "-x") should print the following
  results:

  MD5 test suite:
  MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
  MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
  MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
  MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
  MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
  MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
  d174ab98d277d9f5a5611c2c9f419d9f
  MD5 ("123456789012345678901234567890123456789012345678901234567890123456
  78901234567890") = 57edf4a22be3c955ac49da2e2107b67a

*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
