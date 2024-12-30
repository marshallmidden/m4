/* $Id: PI_Misc.h 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       PI_Misc.h
** MODULE TITLE:    Header file for PI_Misc.h
**
** Copyright (c) 2003-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PI_MISC_H_
#define _PI_MISC_H_

#include "XIO_Types.h"
#include "MR_Defs.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Non-MRP constants used in conjunction with the "get fw hdr" mrp's
*/
#define GET_CCB_DIAG_FW     0xFFF0
#define GET_CCB_RUNTIME_FW  0xFFF1
#define GET_BE_BOOT_FW      MRBEBOOT
#define GET_BE_DIAG_FW      MRBEDIAG
#define GET_BE_RUNTIME_FW   MRBEPROC
#define GET_FE_RUNTIME_FW   MRFEPROC

#define HEARTBEAT_RETRIES   10

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 GetTraceDump(int proc);
extern UINT32 GetPCBDump(int proc);
extern UINT32 GetProfileDump(int proc);
extern UINT32 GetHeapDumpStats(int proc);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PI_MISC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
