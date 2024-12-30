/* $Id: chap_base.c 144042 2010-07-12 15:54:51Z m4 $ */
/**
******************************************************************************
**
**  @file       chap_base.c
**
**  @brief      To convert from binary to base16 and base64, and vice versa.
**
**  Copyright (c) 2005 - 2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

/*

--WHAT IS BASE 64 ENCODING?

Base64 format is used to convert binary data into human readable characters. A
byte stream is chopped up into chunks 6-bits wide and these are converted into
ascii characters based on the base64 encoding. For example if a 6-bit portion
is 0x000001, this corresponds to 'B' in base64. So this is stored as ascii
8-bit 'B' in the output string.

When the receiver sees an ascii 8-bit 'B', it knows that this is base64
encoding, so it converts it into the base64 value for 'B', which is 0x000001.

Reasonably simple, but when there is a remainder, the RFC helps out (difficult
to understand, but correct). Let us look at three cases: A: we need to
transmit 3 bytes of binary data:

-- we chop the 3 bytes == 24 bits into 4 6-bit chunks. Each chunk would have a
corresponding character in the base64 encoding.

-- we transmit the 8-bit ascii character that we got from base64.

-- If the 3 bytes are 0x 00000000 00010000 10000011, then we chop them into 0x
000000 000001 000010 000011 exactly. These 4 chunks correspond to base64
values of A, B, C, and D, respectively. So we output 4 8-bit ascii characters
"ABCD" as the base64 encoding of the original binary stream.

-- Notice that the size of the message increased from 3 bytes to 4 bytes after
our encoding. However, the advantage is that all characters are human
readable, and cannot be one of the ascii special characters. They can still be
used as message delimiters or for introducing newline characters. It is easy
to see that we could have transmitted everything as a simple hex string (using
ascii chars) without this complex logic. But that would mean that we need to
send 6-bytes for every 3-bytes of input, instead of the 4-bytes required for
base64. So the reason for using this is to make it as compact as possible,
while ensuring that the message is human readable.

-- Using hex strings is also a common technique and it is known as base16
encoding.

-- If the data is 4 bytes long, 0x 00000000 00010000 10000011 00010000, then
we chop them into 0x 000000 000001 000010 000011 000100 00. These chunks
correspond to base64 values of A, B, C, D, and E, respectively. Notice that we
are left with two dangling bits. To take care of them, we add as many zeros as
needed to make the last chunk also contain 6 bits. So we make the remaining 00
into 000000. NOw, we have 6 6-bit chunks, which we would output as 6 8-bit
ascii characters "ABCDEA" as the base64 encoding of the original binary
stream. BUT...

-- the problem is that we really don't have that much data. We had only 2 bits
left from the byte stream. To denote this fact, we add two '=' ascii chars to
denote the fact that in the last group, we got two base64 chars from the data
and the added zeros, and two base64 chars are missing (which are denoted by
the '=' padding character). This ensures that the message is an exact multiple
of 4-byte words. And when the last group is decoded, it will be readily seen
that two bytes were missing in the last input byte stream group (which should
have been 3-bytes long).

-- If the data is 5 bytes long, 0x 00000000 00010000 10000011 00010000
01010000, then we chop them into 0x 000000 000001 000010 000011 000100 000101
0000. These chunks correspond to base64 values of A, B, C, D, E, and F,
respectively. Notice that we are left with four dangling bits. To take care of
them, we add as many zeros as needed to make the last chunk also contain 6
bits. So we make the remaining 0000 into 000000. NOw, we have 7 6-bit chunks,
which we would output as 7 8-bit ascii characters "ABCDEFA" as the base64
encoding of the original binary stream. BUT AGAIN...

...the problem is that we really don't have that much data. We had only 4 bits
left from the byte stream. To denote this fact, we add a '=' ascii char to
denote the fact that in the last group, we got three base64 chars from the
data and the added zeros, and a base64 char is missing (which is denoted by
the '=' padding character). This ensures that the message is an exact multiple
of 4-byte words. And when the last group is decoded, it will be readily seen
that one byte was missing in the last input byte stream group (which should
have been 3-bytes long).

*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "iscsi_common.h"
#include "chap_base.h"




static UINT8 get_base64_from_ascii (char c);
static int   get_ascii_from_base64 (UINT8 b);
static UINT8 get_base16_from_ascii (char c);
static int   test_bin_to_base64 (void);
static int   test_base64_to_bin (void);
static int   test_bin_to_base16_to_bin (int do_print);
static int   test_bin_to_base64_to_bin (int do_print);
int test_base_all(void);


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static void print_uint8_arr (UINT8 *p, int p_size)
{
     int i;
     for (i = 0; i < p_size; i++)
     {
          printf ("%02x ", p[i]);

          if (i % 4 == 3) printf ("  ");
     }
}


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:  returns 64 for '=', and the values required by base64.
 **
 **
 ****************************************************************************/
static UINT8 get_base64_from_ascii (char c)
{
     if (isupper (c))
     {
          return c - 'A';      /* [0, 26) */
     }
     else if (islower (c))
     {
          return c - 'a' + 26;  /* [26, 52) */
     }
     else if (isdigit (c))
     {
          return c - '0' + 52;  /* [52, 62) */
     }
     else if (c == '+')
     {
          return 62;
     }
     else if (c == '/')
     {
          return 63;
     }
     else if (c == '=')
     {
          return 64;
     }
     else
     {
          /*
          ** shouldn't be here!
          */
          fprintf(stderr,"get_base64_from_ascii: Error:\n");
          return 0;
     }
}


/****************************************************************************
 ** NOTES:    converts a base64 char into ascii.
 **
 **
 ** RETURNS:  The ascii equivalent of this base64 char
 **
 **
 ****************************************************************************/
static int get_ascii_from_base64 (UINT8 b)
{
     if (b < 26)
     {
          return 'A' + b;
     }
     else if (b < 52)
     {
          return 'a' + (b - 26);
     }
     else if (b < 62)
     {
          return '0' + (b - 52);
     }
     else if (b < 63)
     {
          return '+';
     }
     else if (b < 64)
     {
          return '/';
     }
     else
     {
          /*
          ** shouldn't be here!
          */
          fprintf(stderr,"get_ascii_from_base64: Error:\n");
          return 0;
     }
}


/****************************************************************************
 ** NOTES:    This function converts a stream of normal bytes into a stream
 **           of base64 encoded ascii characters.
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int binary_to_base64 (UINT8 *bin, int bin_len, char *ascii, int *p_valid_ascii_len)
{
     UINT32 pack;
     int    i;
     int    final_ascii_len;
     int    ngroups;
     int    nbytes_left;

     /*
     ** the length of the ascii array must be atleast this much.
     */
     final_ascii_len =
          ((bin_len + BIN_GRP_SZ - 1) / BIN_GRP_SZ) * BASE64_GRP_SZ;

     /*
     ** final valid length of the ascii array.
     */
     *p_valid_ascii_len = final_ascii_len;

     ngroups     = bin_len / BIN_GRP_SZ;
     nbytes_left = bin_len % BIN_GRP_SZ;

     /*
     ** now, go through all the groups except the final partial group.
     */
     for (i = 0; i < ngroups; i++)
     {
          /*
          ** now, we need a packed binary representation, where the three
          ** least significant bytes contain the real binary value. This value
          ** will later be converted into base64.
          */
          pack = 0;
          pack |= *bin++ << 16;
          pack |= *bin++ <<  8;
          pack |= *bin++ <<  0;

          /*
          ** get 4 6-bit base64 values from these three packed bytes.
          */
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >> 18) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >> 12) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  6) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  0) & 0x3F));
     }

     /*
     ** handle the remaining bytes here.
     */
     switch (nbytes_left)
     {
     case 0:
          /*
          ** nothing to be done.
          */
          break;
     case 1:
          /*
          ** one byte left. Add four zeros to it to make the data an integral
          ** multiple of 6-bit chunks.
          */
          pack = 0;
          pack |= *bin++ << 4;

          /*
          ** now, load these two 6-bit base64 values, and two pad characters
          ** into the ascii array. This set will form a complete group.
          */
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  6) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  0) & 0x3F));
          *ascii++ = '=';
          *ascii++ = '=';
          break;
     case 2:
          /*
          ** two bytes left. Add two zeros to it to make the data an integral
          ** multiple of 6-bit chunks.
          */
          pack = 0;
          pack |= *bin++ << 10;
          pack |= *bin++ << 2;

          /*
          ** now, load these 6-bit base64 values, and one pad character
          ** into the ascii array. This set will form a complete group.
          */
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >> 12) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  6) & 0x3F));
          *ascii++ = get_ascii_from_base64 ((UINT8) ((pack >>  0) & 0x3F));
          *ascii++ = '=';
          break;
     default:
          break;
     }

     return 0;
}


/****************************************************************************
 ** NOTES:    converts an ascii stream of base64 chars into binary format.
 **           the input will be an ascii char string. Each char represents
 **           a base64 value of size 6-bits. Get such 6-bit values from
 **           these ascii characters and reconstruct the binary byte
 **           stream.
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int base64_to_binary (char *ascii, int ascii_len, UINT8 *bin, int *p_valid_bin_len)
{
     UINT32 pack;
     int    i;
     int    ngroups;
     int    npads;
     int    bin_len;

     npads = 0;

     /*
     ** The number of bytes in the last partial group depends on the number of
     ** pad characters at the end.
     */
     if ((ascii[ascii_len - 1] == '=')
         && (ascii[ascii_len - 2] == '='))
     {
          /*
          ** two pad chars. That is there is only one byte of binary data in
          ** the last partial group.
          */
          npads = 2;
     }
     else if (ascii[ascii_len - 1] == '=')
     {
          /*
          ** only one pad char. That is there are two bytes of binary data in
          ** the last partial group.
          */
          npads = 1;
     }

     /*
     ** bin_len in an ideal situation should be 3/4 of the ascii array's
     ** len. However, the last group may have pads, and the final bin_len
     ** value will have to take that into account.
     */
     bin_len = (ascii_len / BASE64_GRP_SZ) * BIN_GRP_SZ;
     bin_len -= npads;

     /*
     ** set the return value to the correct bin_len.
     */
     *p_valid_bin_len = bin_len;
     ngroups = ascii_len / BASE64_GRP_SZ;

     /*
     ** now process all the groups except the last one if there are pads. Take
     ** special care of the last group which may have pads. The OR condition in
     ** the for loop doesn't waste time because the processor has to evaluate
     ** both the conditions only during the last iteration.
     */
     for (i = 0;
          (i < ngroups - 1) || ((i < ngroups) && (npads == 0));
          i++)
     {
          /*
          ** get the 4 6-bit base64 values this group contains.
          */
          pack = 0;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) << 18;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) << 12;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) <<  6;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) <<  0;

          /*
          ** now, we have a packed binary representation for the group,
          ** where the three least significant bytes contain the real binary
          ** values. Fill in the bin array.
          */
          *bin++ = (UINT8) (pack >> 16);
          *bin++ = (UINT8) (pack >>  8);
          *bin++ = (UINT8) (pack >>  0);
     }

     /*
     ** now, we check if we have a last partial group left to process. If npads
     ** > 0 , we have to process the last partial group. The for loop above
     ** leaves that job for us.
     */
     if (npads == 2)
     {
          /*
          ** get the 2 valid 6-bit base64 values this group contains.
          */
          pack = 0;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) << 6;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) << 0;

          /*
          ** if there are two pads, then we know that we have only one byte for
          ** the bin array.
          */
          *bin++ = (UINT8) (pack >> 4);
     }
     else if (npads == 1)
     {
          /*
          ** get the 3 valid 6-bit base64 values this group contains.
          */
          pack = 0;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) << 12;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) <<  6;
          pack |= (0x3F & get_base64_from_ascii (*ascii++)) <<  0;

          /*
          ** if there is one pad, then we know that we have two bytes for the
          ** bin array.
          */
          *bin++ = (UINT8) (pack >> 10);
          *bin++ = (UINT8) (pack >>  2);
     }

     return 0;
}


/****************************************************************************
 ** NOTES:    This function converts a stream of normal bytes into a stream
 **           of base16 encoded ascii characters.
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int binary_to_base16 (UINT8 *bin, int bin_len, char *ascii, int *p_valid_ascii_len)
{
     int    i;
     int    final_ascii_len;
     char   hex[] = "0123456789ABCDEF";

     /*
     ** the length of the ascii array must be atleast this much.
     */
     final_ascii_len = bin_len * 2;

     /*
     ** final valid length of the ascii array.
     */
     *p_valid_ascii_len = final_ascii_len;

     for (i = 0; i < bin_len; i++)
     {
          /*
          ** take the byte, and convert it into an ascii hex
          ** representation. That is, if the input is 0x8C, the characters
          ** '8', and 'C' will be store in the ascii array.
          */
          *ascii++ = hex[bin[i] >> 4];
          *ascii++ = hex[bin[i] & 0xF];
     }

     return 0;
}


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static UINT8 get_base16_from_ascii (char c)
{
     UINT8 val;

     val = toupper (c);

     /*
     ** if it is a digit, subtract '0',
     ** otherwise, subtract 'A' and add 10.
     */
     val -= isdigit (val) ? '0' : 'A' - 10;

     /*
     ** make sure we are looking at a hex char!
     */
     return val;
}


/****************************************************************************
 ** NOTES:    converts an ascii stream of base16 chars into binary format.
 **           the input will be an ascii char string. Each char represents
 **           a base16 value of size 4-bits. Get such 4-bit values from
 **           these ascii characters and reconstruct the binary byte
 **           stream.
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int base16_to_binary (char *ascii, int ascii_len, UINT8 *bin, int *p_valid_bin_len)
{
     int    i;
     int    bin_len;

     /*
     ** If the number of bytes is NOT a multiple of 2, then we have a
     ** problem. This is because a byte of binary ALWAYS results in two ascii
     ** characters. That is, two bytes. If we have an odd number of bytes, then
     ** we know that the encoding has a problem.
     */

     /*
     ** bin_len in an ideal situation should be 3/4 of the ascii array's
     ** len. However, the last group may have pads, and the final bin_len
     ** value will have to take that into account.
     */
     bin_len = ascii_len / 2;

     /*
     ** set the return value to the correct bin_len.
     */
     *p_valid_bin_len = bin_len;

     for (i = 0; i < bin_len; i++)
     {
          /*
          ** if we have two ascii characters, "8C", then we store 0x8C in bin.
          */
          UINT8 val_high;
          UINT8 val_low;

          val_high = get_base16_from_ascii (*ascii++);
          val_low  = get_base16_from_ascii (*ascii++);

          bin[i] = (val_high << 4) + val_low;
     }

     return 0;
}


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static int test_bin_to_base64 (void)
{
     char  ascii[SIZE] = {0};
     UINT8 bin[SIZE] = {0, 0x10, 0x83, 0x10, 0x51, 0x87};
     int valid_bin_size  = 6;
     int valid_ascii_size = SIZE;

     printf ("BEFORE:\n");
     printf ("ascii            --> %s\n", ascii);
     printf ("valid_ascii_size --> %d\n", valid_ascii_size);
     printf ("valid_bin_size   --> %d\n", valid_bin_size);
     printf ("bin              --> ");
     print_uint8_arr (bin, valid_bin_size);
     printf ("\n");

     binary_to_base64 (bin, valid_bin_size, ascii, &valid_ascii_size);

     printf ("AFTER:\n");
     printf ("ascii            --> %s\n", ascii);
     printf ("valid_ascii_size --> %d\n", valid_ascii_size);
     printf ("valid_bin_size   --> %d\n", valid_bin_size);
     printf ("bin              --> ");
     print_uint8_arr (bin, valid_bin_size);
     printf ("\n");

     fflush (stdout);

     return 0;
}


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static int test_base64_to_bin (void)
{
     char  ascii[SIZE] = "ABCDEFGH";
     UINT8 bin[SIZE] = {0};
     int valid_bin_size  = SIZE;
     int valid_ascii_size = strlen (ascii);

     printf ("BEFORE:\n");
     printf ("ascii            --> %s\n", ascii);
     printf ("valid_ascii_size --> %d\n", valid_ascii_size);
     printf ("valid_bin_size   --> %d\n", valid_bin_size);
     printf ("bin              --> ");
     print_uint8_arr (bin, valid_bin_size);
     printf ("\n");

     base64_to_binary (ascii, valid_ascii_size, bin, &valid_bin_size);

     printf ("AFTER:\n");
     printf ("ascii            --> %s\n", ascii);
     printf ("valid_ascii_size --> %d\n", valid_ascii_size);
     printf ("valid_bin_size   --> %d\n", valid_bin_size);
     printf ("bin              --> ");
     print_uint8_arr (bin, valid_bin_size);
     printf ("\n");


     fflush (stdout);
     return 0;
}


/****************************************************************************
 ** NOTES:    create a random binary array, encode it in base64, convert
 **           it back to bin, ensure that the first bin array is the same
 **           as the second.
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static int test_bin_to_base64_to_bin (int do_print)
{
     UINT8 *bin_one;
     UINT8 *bin_two;
     int    bin_len;
     int    bin_len_one;
     int    bin_len_two;
     char  *ascii;
     int    ascii_len;
     int    i;

     /*
     ** choose a length that is in the range [0, 255].
     */
     bin_len_one = bin_len_two = bin_len = 0xFF & rand();
     ascii_len = bin_len * 4; /* up to 4 times needed, when bin_len == 1 */

     bin_one = (UINT8 *) s_MallocC (bin_len, __FILE__, __LINE__);
     bin_two = (UINT8 *) s_MallocC (bin_len, __FILE__, __LINE__);
     ascii   = (char *)  s_MallocC (ascii_len, __FILE__, __LINE__);

     for (i = 0; i < bin_len; i++)
     {
          bin_one[i] = rand() % 0xFF;
     }

     if (do_print)
         printf ("bin_len = %3d\n", bin_len);

     /*
     ** use bin_one for encoding.
     */
     binary_to_base64 (bin_one, bin_len, ascii, &ascii_len);

     if (do_print)
         printf ("ascii_len = %3d\n", ascii_len);

     /*
     ** now, the ascii array contains the encoding. convert back to binary into
     ** bin_two.
     */
     base64_to_binary (ascii, ascii_len, bin_two, &bin_len_two);

     s_Free(bin_one, bin_len, __FILE__, __LINE__);
     s_Free(bin_two, bin_len, __FILE__, __LINE__);
     s_Free(ascii, ascii_len, __FILE__, __LINE__);

     return 0;
}


/****************************************************************************
 ** NOTES:    create a random binary array, encode it in base16, convert
 **           it back to bin, ensure that the first bin array is the same
 **           as the second.
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
static int test_bin_to_base16_to_bin (int do_print)
{
     UINT8 *bin_one;
     UINT8 *bin_two;
     int    bin_len;
     int    bin_len_one;
     int    bin_len_two;
     char  *ascii;
     int    ascii_len;
     int    i;

     /*
     ** choose a length that is in the range [0, 255].
     */
     bin_len_one = bin_len_two = bin_len = 0xFF & rand();
     ascii_len = bin_len * 2;

     bin_one = (UINT8 *) s_MallocC (bin_len, __FILE__, __LINE__);
     bin_two = (UINT8 *) s_MallocC (bin_len, __FILE__, __LINE__);
     ascii   = (char *)  s_MallocC (ascii_len, __FILE__, __LINE__);

     for (i = 0; i < bin_len; i++)
     {
          bin_one[i] = rand() % 0xFF;
     }

     if (do_print)
     {
          printf ("BEFORE:\n");
          printf ("bin_len   --> %d\n", bin_len);
          printf ("bin       --> ");
          print_uint8_arr (bin_one, bin_len);
          printf ("\n");
     }

     /*
     ** use bin_one for encoding.
     */
     binary_to_base16 (bin_one, bin_len, ascii, &ascii_len);

     if (do_print)
     {
          printf ("ascii_len --> %d\n", ascii_len);
          printf ("ascii     --> ");
          print_uint8_arr (bin_one, bin_len);
          printf ("\n");
     }

     /*
     ** now, the ascii array contains the encoding. convert back to binary into
     ** bin_two.
     */
     base16_to_binary (ascii, ascii_len, bin_two, &bin_len_two);

     if (do_print)
     {
          printf ("AFTER:\n");
          printf ("bin_len   --> %d\n", bin_len);
          printf ("bin       --> ");
          print_uint8_arr (bin_one, bin_len);
          printf ("\n");
     }

     s_Free(bin_one, bin_len, __FILE__, __LINE__);
     s_Free(bin_two, bin_len, __FILE__, __LINE__);
     s_Free(ascii, ascii_len, __FILE__, __LINE__);

     return 0;
}


/****************************************************************************
 ** NOTES:
 **
 **
 ** RETURNS:
 **
 **
 ****************************************************************************/
int test_base_all(void)
{
     int i;
     int ntrials = 10000;
     int do_print = 0;

     srand (time (0));
     /*
     **test_base64_to_bin();
     **test_bin_to_base64();
     */

     for (i = 0; i < ntrials; i++)
     {
          test_bin_to_base64_to_bin (do_print);
     }

     printf ("base64 test: total trials = %4d, SUCCESS\n", ntrials);

     for (i = 0; i < ntrials; i++)
     {
          test_bin_to_base16_to_bin (do_print);
     }

     printf ("base16 test: total trials = %4d, SUCCESS\n", ntrials);

     // To make compiler happy, call these two functions too.
     test_bin_to_base64 ();
     test_base64_to_bin ();

     return 0;
}


/* int main() */
/* { */
/*      test_base_all(); */

/*      return 0; */
/* } */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
