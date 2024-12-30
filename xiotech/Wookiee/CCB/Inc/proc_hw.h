/* $Id: proc_hw.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       proc_hw.h
** MODULE TITLE:    Definitions for the proc hardware
**
** DESCRIPTION:     These MUST match the constants used in the proc code.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PROC_HW_H_
#define _PROC_HW_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/* Address and length for BE/FE NVRAM */
#define PROC_NVRAM_START    0xFE800000  /* Entire NVRAM */
#define PROC_NVRAM_LENGTH   0x170000

#define PROC_NVSRAMRES      0x8000      /* Part 1 diag */

#define PROC_NVSRAMP2START  PROC_NVRAM_START+PROC_NVSRAMRES     /* Part 2 config */
#define PROC_NVSRAMP2HSIZ   0x40
#define PROC_NVSRAMP2SIZ    0x170000

#define PROC_NVSRAMP3START  PROC_NVSRAMP2START+PROC_NVSRAMP2SIZ /* Part 3 nva */
#define PROC_NVSRAMP3SIZ    32*16*0x60

#define PROC_NVSRAMP4START  PROC_NVSRAMP3START+PROC_NVSRAMP3SIZ /* Part 4 snva */
#define PROC_NVSRAMP4SIZ    PROC_NVSRAMP3SIZ

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PROC_HW_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
