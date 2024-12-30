/* $Id: FW_HeaderSrc.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       FW_HeaderSrc.c
**
**  @brief      This file defines the common FW header.
**
**  All FW modules that are downloaded to a system need this FW header
**  pre-pended to them to define CRC's, load locations etc.  This file,
**  FW_HeaderSrc.c, never actually gets compiled by itself, but is the
**  template used to create FW_Header.c, which does get compiled.  NOTE: The
**  constants marked as "PRE_PATCH_xxx" are patched by UpdFwHdr.pl at the
**  beginning of the build -- Do NOT change them here! Those marked as
**  "POST_PATCH_xxx" are patched at the end of the build, and need initial
**  values to build with, which are defined here.
**
**  Copyright (c) 2003-2008 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "FW_Header.h"

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define POST_PATCH_LENGTH           0
#define POST_PATCH_FW_CRC           0
#define POST_PATCH_FW_COMPAT_IDX    0
#define POST_PATCH_FW_BL_IDX        0
#define POST_PATCH_FW_SEQ_IDX       0
#define POST_PATCH_HDR_CRC          0

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
// 2024-12-29_19-13-08 FW_HEADER fwHeader __attribute__ ((section (".fw.header"))) = {
FW_HEADER fwHeader = {

    /* QUADs 0,1 */
    I960_BR_FWD_128,        /* UINT32 branch;           */
    {0,0,0,0,0,0,0},        /* UINT32 rsvd_0x00[7];     */

    /* QUAD 2 */
    MAGIC_NUMBER,           /* UINT32 magicNumber;      */
    0,                      /* UINT32 rsvd_0x24;        */
    PROD_ID,                /* UINT32 productID;        */
    TARG_ID,                /* UINT32 target;           */

    /* QUAD 3 */
    PRE_PATCH_REVISION,     /* UINT32 revision;         */
    PRE_PATCH_REVCOUNT,     /* UINT32 revCount;         */
    PRE_PATCH_BUILDID,      /* UINT32 buildID;          */
    PRE_PATCH_RELEASE,      /* UINT32 systemRelease;    */

    /* QUAD 4 */
    {
        PRE_PATCH_YEAR,     /* UINT16 year;             */
        PRE_PATCH_MONTH,    /* UINT8  month;            */
        PRE_PATCH_DATE,     /* UINT8  date;             */
        PRE_PATCH_DAY,      /* UINT8  day;              */
        PRE_PATCH_HOURS,    /* UINT8  hours;            */
        PRE_PATCH_MINUTES,  /* UINT8  minutes;          */
        PRE_PATCH_SECONDS   /* UINT8  seconds;          */
    },
    0,                      /* UINT32 rsvd_0x48;        */
    0,                      /* UINT32 burnSequence;     */

    /* QUAD 5 */
    0,
    0,
    0,                      /* UINT32 targAddr;         */
    POST_PATCH_LENGTH,      /* UINT32 length;           */

    /* QUADs 6,7 */
    POST_PATCH_FW_CRC,      /* UINT32 checksum;         */
    POST_PATCH_FW_COMPAT_IDX, /* UINT8  fwCompatIndex;  */
    POST_PATCH_FW_BL_IDX,   /* UINT8  fwBackLevelIndex; */
    POST_PATCH_FW_SEQ_IDX,  /* UINT8  fwSequencingIndex;*/
    0,                      /* UINT8  rsvd_0x67;        */
    PRE_PATCH_FW_MAJ,       /* FW Major Release Level   */
    PRE_PATCH_FW_MIN,       /* FW Minor Release Level   */

    {0,0,0,0},              /* UINT32 rsvd_0x68[5];     */
    POST_PATCH_HDR_CRC      /* UINT32 hdrCksum;         */
};

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
