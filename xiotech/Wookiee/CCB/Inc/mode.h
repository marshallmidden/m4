/* $Id: mode.h 145021 2010-08-03 14:16:38Z m4 $ */
/*============================================================================
** FILE NAME:       mode.h
** MODULE TITLE:    Header file for mode.c
**
** DESCRIPTION:
**      Definition of mode settable bits which control features within
**      the ccb and the proc. Functions to provide access and control
**      of these mode settable features.
**
** Copyright (c) 2002-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _MODE_H_
#define _MODE_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/* CCB mode bits */
#define MD_IPC_HEARTBEAT_DISABLE            0x00000001
#define MD_IPC_HEARTBEAT_WATCHDOG_DISABLE   0x00000002
#define MD_LOCAL_HEARTBEAT_DISABLE          0x00000004
/* #define MD_LOCAL_STATISTICS_DISABLE         0x00000008 */
#define MD_DUMP_LOCAL_IMAGE_ENABLE          0x00000010
#define MD_FM_DISABLE                       0x00000020  /* Disable failure manager */
#define MD_CONTROLLER_SUICIDE_DISABLE       0x00000040
/* #define MD_UNUSED_UNUSED_UNUSED             0x00000080 */
#define MD_FM_RESTART_DISABLE               0x00000100  /* Disable system restart if N=1 */
#define MD_DIAG_PORTS_ENABLE                0x00000200
#define MD_DISABLE_INACTIVATE_POWER     0x00001000

/* Proc mode bits */
#define MD_PROC_HEARTBEAT_WATCHDOG_DISABLE    0x00000001
// #define MD_PROC_BOOT_ERRTRAP_HANDLING_DISABLE 0x00000002
// #define MD_PROC_CTRL_SHUTDOWN                 0x00000004

/* CCB DEBUG PRINTF BITS */
#define MD_DPRINTF_OFF                      0x00000000

#define MD_DPRINTF_DEFAULT                  0x00000001
#define MD_DPRINTF_CACHE_REFRESH            0x00000002
#define MD_DPRINTF_XSSA_DEBUG               0x00000004
#define MD_DPRINTF_ELECTION                 0x00000008

#define MD_DPRINTF_IPC                      0x00000010
#define MD_DPRINTF_IPC_MSGS                 0x00000020
#define MD_DPRINTF_X1_COMMANDS              0x00000040
#define MD_DPRINTF_X1_PROTOCOL              0x00000080

#define MD_DPRINTF_I2C                      0x00000100
#define MD_DPRINTF_RM                       0x00000200
#define MD_DPRINTF_SES                      0x00000400
#define MD_DPRINTF_ETHERNET                 0x00000800

#define MD_DPRINTF_MD5                      0x00001000
#define MD_DPRINTF_FCALMON                  0x00002000
#define MD_DPRINTF_SM_HB                    0x00004000
#define MD_DPRINTF_PI_COMMANDS              0x00008000

#define MD_DPRINTF_PROC_PRINTF              0x00010000
#define MD_DPRINTF_ELECTION_VERBOSE         0x00020000
#define MD_DPRINTF_IPMI                     0x00040000
#define MD_DPRINTF_IPMI_VERBOSE             0x00080000

#define MD_DPRINTF_RAIDMON                  0x00100000
#define MD_DPRINTF_UNUSED_21                0x00200000
#define MD_DPRINTF_UNUSED_22                0x00400000
#define MD_DPRINTF_UNUSED_23                0x00800000

#define MD_DPRINTF_UNUSED_24                0x01000000
#define MD_DPRINTF_UNUSED_25                0x02000000
#define MD_DPRINTF_UNUSED_26                0x04000000
#define MD_DPRINTF_UNUSED_27                0x08000000

#define MD_DPRINTF_UNUSED_28                0x10000000
#define MD_DPRINTF_UNUSED_29                0x20000000
#define MD_DPRINTF_UNUSED_30                0x40000000
#define MD_DPRINTF_UNUSED_31                0x80000000

/* Initial DPRINTF setting */
#ifndef ALL_DPRINTF
#define INITIAL_DPRINTF_MODE_BITS    (    \
        MD_DPRINTF_DEFAULT           |    \
        MD_DPRINTF_ELECTION          |    \
        MD_DPRINTF_RM                |    \
        MD_DPRINTF_PROC_PRINTF )
#else   /* ALL_DPRINTF */
#define INITIAL_DPRINTF_MODE_BITS    (    \
        MD_DPRINTF_DEFAULT           |    \
        MD_DPRINTF_CACHE_REFRESH     |    \
        MD_DPRINTF_XSSA_DEBUG        |    \
        MD_DPRINTF_ELECTION          |    \
        MD_DPRINTF_IPC               |    \
        MD_DPRINTF_IPC_MSGS          |    \
        MD_DPRINTF_X1_COMMANDS       |    \
        MD_DPRINTF_X1_PROTOCOL       |    \
        MD_DPRINTF_I2C               |    \
        MD_DPRINTF_RM                |    \
        MD_DPRINTF_SES               |    \
        MD_DPRINTF_FCALMON           |    \
        MD_DPRINTF_SM_HB             |    \
        MD_DPRINTF_PI_COMMANDS       |    \
        MD_DPRINTF_PROC_PRINTF       |    \
        MD_DPRINTF_ELECTION_VERBOSE  |    \
        MD_DPRINTF_IPMI              |    \
        MD_DPRINTF_IPMI_VERBOSE      |    \
        MD_DPRINTF_RAIDMON )
#endif  /* ALL_DPRINTF */

/* Bit operators */
#define modeBits        (modeData.ccb.bits)
#define TestModeBit(A)  (modeBits & (A))
#define SetModeBit(A)   (modeBits |= (A))
#define ClrModeBit(A)   (modeBits &= (~(A)))

/*****************************************************************************
** Public variables
*****************************************************************************/
typedef struct
{
    UINT32      bits;
    UINT32      bitsDPrintf;
    UINT32      rsvd1;
    UINT32      rsvd2;
} MODEDATA_CCB;

typedef struct
{
    UINT32      word[4];
} MODEDATA_PROC;

typedef struct
{
    MODEDATA_CCB ccb;
    MODEDATA_PROC proc;
} MODEDATA;

extern MODEDATA modeData;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void InitModeData(void);
extern INT32 ModeSet(MODEDATA *modeDataPtr, MODEDATA *modeMaskPtr);
extern INT32 ModeGet(MODEDATA *modeDataPtr);

extern void SetProcSuicide(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _MODE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
