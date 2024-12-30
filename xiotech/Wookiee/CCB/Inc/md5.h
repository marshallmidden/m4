/* $Id: md5.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       md5.h
** MODULE TITLE:    Header file for md5.c
**
** RSA Data Security, Inc. MD5 Message Digest Algorithm
** Created: 2/17/90 RLR
** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version
** Revised (for MD5): RLR 4/27/91
**   -- G modified to have y&~z instead of y&z
**   -- FF, GG, HH modified to add in last register done
**   -- Access pattern: round 2 works mod 5, round 3 works mod 3
**   -- distinct additive constant for each step
**   -- round 4 added, working mod 7
**
** Copyright (c) 2001  Xiotech Corporation.  All rights reserved.
** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
**
** License to copy and use this software is granted provided that
** it is identified as the "RSA Data Security, Inc. MD5 Message
** Digest Algorithm" in all material mentioning or referencing this
** software or this function.
**
** License is also granted to make and use derivative works
** provided that such works are identified as "derived from the RSA
** Data Security, Inc. MD5 Message Digest Algorithm" in all
** material mentioning or referencing the derived work.
**
** RSA Data Security, Inc. makes no representations concerning
** either the merchantability of this software or the suitability
** of this software for any particular purpose.  It is provided "as
** is" without express or implied warranty of any kind.
**
** These notices must be retained in any copies of any part of this
** documentation and/or software.
**
**==========================================================================*/
#ifndef _MD5_H_
#define _MD5_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/* typedef a 32 bit type */
typedef unsigned long int UINT4;

/* Data structure for MD5 (Message Digest) computation */
typedef struct
{
    UINT4       i[2];           /* number of _bits_ handled mod 2^64 */
    UINT4       buf[4];         /* scratch buffer */
    unsigned char in[64];       /* input buffer */
    unsigned char digest[16];   /* actual digest after MD5Final call */
} MD5_CONTEXT;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void MD5Update(MD5_CONTEXT *mdContext, unsigned char *inBuf, unsigned int inLen);
extern void MD5Final(MD5_CONTEXT *mdContext);
extern void MD5Init(MD5_CONTEXT *mdContext);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MD5_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
