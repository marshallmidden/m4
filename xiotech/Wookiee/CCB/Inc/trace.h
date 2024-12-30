/* $Id: trace.h 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
**  FILE NAME:      trace.h
**  MODULE TITLE:   Header file for trace.c
**
**  DESCRIPTION:    Contains the TraceEvent macro and associated id constants.
**
**  Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _TRACE_H_
#define _TRACE_H_

#include "XIO_Types.h"
#include "kernel.h"
#include "PortServer.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/* Profiler "timer" structure */
typedef struct
{
    UINT32      startT;
    UINT32      startR;
    UINT32      finishT;
    UINT32      finishR;
} PROFILE_TIME;

/* Trace event structures */
typedef struct TRACE_EVENT_STRUCT
{
    unsigned int id;
    unsigned int data;
    unsigned int tCoarse;
    unsigned int tFine;
} TRACE_EVENT;

typedef struct TRACE_QUEUE_STRUCT
{
    TRACE_EVENT *evBaseP;       /* this must be element 1 */
    TRACE_EVENT *evNextP;       /* this must be element 2 */
    TRACE_EVENT *evEndP;        /* this must be element 3 */
    unsigned int evRunFlag;     /* this must be element 4 */
    unsigned int evControl;
} TRACE_QUEUE;

typedef struct PROFILE_EVENT_STRUCT
{
    unsigned int rip;
} PROFILE_EVENT;

typedef struct PROFILE_QUEUE_STRUCT
{
    PROFILE_EVENT *prBaseP;     /* this must be element 1 */
    PROFILE_EVENT *prNextP;     /* this must be element 2 */
    PROFILE_EVENT *prEndP;      /* this must be element 3 */
    unsigned int prRunFlag;     /* this must be element 4 */
} PROFILE_QUEUE;

#define NUM_TRACE_EVENTS    10000
#define TRACE_SIZE          (sizeof(TRACE_EVENT) * NUM_TRACE_EVENTS)

#define NUM_PROFILE_EVENTS  250000

/*
** The "TraceEvent()" macro
*/
#define TraceEvent(id, data)                                \
    if (!((id) & evQueue.evControl)) {}                     \
    else {K$TraceEvent((id), (data));}

/*
** Trace Start and Stop macros
** Note: the trace facility is initialized to the "run" state.
*/
#define TraceStart()    evQueue.evRunFlag = 1
#define TraceStop()     evQueue.evRunFlag = 0

/***********************************************************
 *                                                         *
 *           "evControl" bit definitions                   *
 *                                                         *
 *  1) The top 16 bits are used to allow for 16 different  *
 *     trace types.                                        *
 *  2) The lower 16 bits, taken as a 16 bit integer, are   *
 *     used for the specific instance or sub-type of the   *
 *     major group.                                        *
 *                                                         *
 **********************************************************/
#define TRACE_MRP           0x80000000
#define TRACE_PACKET        0x40000000
#define TRACE_IPC           0x20000000
#define TRACE_LOG           0x10000000
#define TRACE_X1            0x08000000
#define TRACE_X1_VDC        0x04000000
#define TRACE_X1_BF         0x02000000
#define TRACE_RM            0x01000000
#define TRACE_SIGNAL        0x00800000
#define TRACE_UNUSED_B22    0x00400000
#define TRACE_UNUSED_B21    0x00200000
#define TRACE_UNUSED_B20    0x00100000
#define TRACE_UNUSED_B19    0x00080000
#define TRACE_UNUSED_B18    0x00040000
#define TRACE_UNUSED_B17    0x00020000
#define TRACE_UNUSED_B16    0x00010000

/*
** TRACE_MRP sub-types
*/
#define TRACE_MRP_START                 (TRACE_MRP + 99)
#define TRACE_MRP_TIMEOUT_CALLBACK      (TRACE_MRP + 100)
#define TRACE_MRP_GOOD                  (TRACE_MRP + PI_GOOD)
#define TRACE_MRP_ERROR                 (TRACE_MRP + PI_ERROR)
#define TRACE_MRP_IN_PROGRESS           (TRACE_MRP + PI_IN_PROGRESS)
#define TRACE_MRP_TIMEOUT               (TRACE_MRP + PI_TIMEOUT)
#define TRACE_MRP_INVALID_CMD_CODE      (TRACE_MRP + PI_INVALID_CMD_CODE)
#define TRACE_MRP_MALLOC_ERROR          (TRACE_MRP + PI_MALLOC_ERROR)
#define TRACE_MRP_PARAMETER_ERROR       (TRACE_MRP + PI_PARAMETER_ERROR)
#define TRACE_MRP_BE_Q_FREE             (TRACE_MRP + 128)
#define TRACE_MRP_BE_Q_BLOCKED          (TRACE_MRP_BE_Q_FREE + 1)
#define TRACE_MRP_BE_Q_FS_BLOCKED       (TRACE_MRP_BE_Q_FREE + 2)
#define TRACE_MRP_BE_Q_ALL_BLOCKED      (TRACE_MRP_BE_Q_FREE + 3)
#define TRACE_MRP_FE_Q_FREE             (TRACE_MRP + 256)
#define TRACE_MRP_FE_Q_BLOCKED          (TRACE_MRP_FE_Q_FREE + 1)
#define TRACE_MRP_GC_TASK_START         (TRACE_MRP + 512)
#define TRACE_MRP_GC_TASK_END           (TRACE_MRP + 513)

#define TRACE_PACKET_START              (TRACE_PACKET + 99)
#define TRACE_PACKET_GOOD               (TRACE_PACKET + PI_GOOD)
#define TRACE_PACKET_ERROR              (TRACE_PACKET + PI_ERROR)
#define TRACE_PACKET_IN_PROGRESS        (TRACE_PACKET + PI_IN_PROGRESS)
#define TRACE_PACKET_TIMEOUT            (TRACE_PACKET + PI_TIMEOUT)
#define TRACE_PACKET_INVALID_CMD_CODE   (TRACE_PACKET + PI_INVALID_CMD_CODE)
#define TRACE_PACKET_MALLOC_ERROR       (TRACE_PACKET + PI_MALLOC_ERROR)
#define TRACE_PACKET_PARAMETER_ERROR    (TRACE_PACKET + PI_PARAMETER_ERROR)

#define TRACE_IPC_START                 (TRACE_IPC + 99)
#define TRACE_IPC_CALLBACK              (TRACE_IPC + 98)
#define TRACE_IPC_DISPATCH_START        (TRACE_IPC + 97)
#define TRACE_IPC_DISPATCH_DONE         (TRACE_IPC + 96)
#define TRACE_IPC_DISPATCH_NULL         (TRACE_IPC + 95)
#define TRACE_IPC_DISPATCH_TUNNEL_START (TRACE_IPC + 94)
#define TRACE_IPC_TUNNEL_START          (TRACE_IPC + 93)
#define TRACE_TIME_OUT                  (TRACE_IPC + SENDPACKET_TIME_OUT)
#define TRACE_NO_PATH                   (TRACE_IPC + SENDPACKET_NO_PATH)
#define TRACE_ANY_PATH                  (TRACE_IPC + SENDPACKET_ANY_PATH)
#define TRACE_ETHERNET                  (TRACE_IPC + SENDPACKET_ETHERNET)
#define TRACE_FIBRE                     (TRACE_IPC + SENDPACKET_FIBRE)
// #define TRACE_QUORUM                    (TRACE_IPC + SENDPACKET_QUORUM)

#define TRACE_X1_START                  (TRACE_X1 + 99)
#define TRACE_X1_VDC_START              (TRACE_X1_VDC + 99)
#define TRACE_X1_BF_START               (TRACE_X1_BF + 99)

/*
** Pam HB Trace.
*/
#define TRACE_PAM_HB                    (TRACE_SIGNAL + 99)

/*
** Values for the data field of an RM trace event
*/
#define TRACE_RM_INIT_SHUTDOWN              0x00000001
#define TRACE_RM_INIT                       0x00000002
#define TRACE_RM_MOVE_TARGET                0x00000003
#define TRACE_RM_FAIL_CONTROLLER            0x00000004
#define TRACE_RM_UNFAIL_CONTROLLER          0x00000005
#define TRACE_RM_FAIL_INTERFACE             0x00000006
#define TRACE_RM_UNFAIL_INTERFACE           0x00000007
#define TRACE_RM_RESTORE_INTERFACE          0x00000008
#define TRACE_RM_BUILD_TARGETS              0x00000009

#define TRACE_RM_CONFIG_INIT                0x00001000
#define TRACE_RM_CONFIG_BUILD               0x00001001
#define TRACE_RM_CONFIG_READ                0x00001002
#define TRACE_RM_CONFIG_WRITE               0x00001003
#define TRACE_RM_CONFIG_RESTORE             0x00001004
#define TRACE_RM_CONFIG_CHECKPOINT          0x00001005
#define TRACE_RM_CONFIG_CURRENT             0x00001006

#define TRACE_RM_REALLOC_TASK               0x00002000
#define TRACE_RM_REALLOC_RELOCATE_TGT       0x00002001
#define TRACE_RM_REALLOC_PROC_FAILED        0x00002002
#define TRACE_RM_REALLOC_MV_TGT_FROM_INT    0x00002003
#define TRACE_RM_REALLOC_REDIST_TGTS        0x00002004
#define TRACE_RM_REALLOC_REDIST_TGT         0x00002005
#define TRACE_RM_REALLOC_UPD_INT_STATUS     0x00002006

#define TRACE_RM_HS_REQUEST_SPARE           0x00003000
#define TRACE_RM_HS_PROC_HS_REQUEST         0x00003001
#define TRACE_RM_HS_LOCATE_HS               0x00003002
#define TRACE_RM_HS_CHECK_HS                0x00003003
#define TRACE_RM_HS_BUILD_HS_LIST           0x00003004

#define TRACE_RM_MIRROR_RETRIEVE_MP         0x00004000
#define TRACE_RM_MIRROR_CHECK_MP            0x00004001
#define TRACE_RM_MIRROR_SET_MPS             0x00004002
#define TRACE_RM_MIRROR_DLT_FROM_MC         0x00004003
#define TRACE_RM_MIRROR_INSERT_MP           0x00004004
#define TRACE_RM_MIRROR_ADD_MP              0x00004005
#define TRACE_RM_MIRROR_SET_MP              0x00004006

#define TRACE_RM_MISC_GET_TGT_CACHE_STATUS  0x00005000
#define TRACE_RM_MISC_MOVE_TARGET           0x00005001
#define TRACE_RM_MISC_CLEANUP               0x00005002
#define TRACE_RM_MISC_BUILD_TARGET          0x00005003
#define TRACE_RM_MISC_GET_MP                0x00005004

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void TraceInit(void);
extern void CopyTraceDataToNVRAM(UINT8 *nvramP, UINT32 length);
extern void ProfileInit(char *bufP);

/*****************************************************************************
** Public variables
*****************************************************************************/
extern TRACE_QUEUE evQueue;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _TRACE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
