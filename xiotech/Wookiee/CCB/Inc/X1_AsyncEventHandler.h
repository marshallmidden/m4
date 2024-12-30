/* $Id: X1_AsyncEventHandler.h 143845 2010-07-07 20:51:58Z mdr $*/
/*===========================================================================
** FILE NAME:       X1_AsyncEventHandler.h
** MODULE TITLE:    X1 Asynchronous Event Handler
**
** DESCRIPTION:     Handle X1 Async events.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _X1ASYNCEVENTHANDLER_H_
#define _X1ASYNCEVENTHANDLER_H_

#include "logging.h"
#include "PortServer.h"
#include "slink.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** The following defines are for the reason.
** Also see files CCBE/XIOTech/xiotechPackets.pm and cmLogs.pm.
*/
#define X1_ASYNC_NONE                       0x00000000
#define X1_ASYNC_PCHANGED                   0x00000001
#define X1_ASYNC_RCHANGED                   0x00000002
#define X1_ASYNC_VCHANGED                   0x00000004
#define X1_ASYNC_HCHANGED                   0x00000008
#define X1_ASYNC_ACHANGED                   0x00000010
#define X1_ASYNC_ZCHANGED                   0x00000020
#define ASYNC_ENV_CHANGE                    0x00000040
#define ASYNC_DEFRAG_CHANGE                 0x00000080
#define ASYNC_PDATA_CREATE                  0x00000100
#define ASYNC_PDATA_REMOVE                  0x00000200
#define ASYNC_PDATA_MODIFY                  0x00000400
#define ASYNC_ISNS_MODIFY                   0x00000800
#define ASYNC_BUFFER_BOARD_CHANGE           0x00001000
#define ASYNC_GLOBAL_CACHE_CHANGE           0x00002000
#define ASYNC_PRES_CHANGED                  0x00004000
#define ASYNC_APOOL_CHANGED                 0x00008000
#define X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
#define X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
#define X1_ASYNC_VCG_POWERUP                0x00040000
#define X1_ASYNC_VCG_CFG_CHANGED            0x00080000
#define X1_ASYNC_VCG_WORKSET_CHANGED        0x00100000

/*
** The event bit (0x00200000) can be reused by any application
*/
#define SNAPPOOL_CHANGED                    0x00400000
#define ISE_ENV_CHANGED                     0x00800000
#define X1_ASYNC_BE_PORT_CHANGE             0x01000000
#define X1_ASYNC_FE_PORT_CHANGE             0x02000000

/* unused entry here. Do not use this we are using this bit for more
   granular PI async events*/

/* unused entry here. */
#define ASYNC_PING_EVENT                    0x10000000
#define LOG_STD_MSG                         0x20000000
#define LOG_XTD_MSG                         0x40000000
#define LOG_BIN_MSG                         0x80000000

#define X1_ASYNC_DEVICES_CHANGED            (X1_ASYNC_PCHANGED | \
                                             X1_ASYNC_RCHANGED | \
                                             X1_ASYNC_VCHANGED)

#define X1_ASYNC_BECHANGED                  (X1_ASYNC_DEVICES_CHANGED | \
                                             X1_ASYNC_BE_PORT_CHANGE)

#define X1_ASYNC_FECHANGED                  (X1_ASYNC_HCHANGED | \
                                             X1_ASYNC_ZCHANGED | \
                                             X1_ASYNC_FE_PORT_CHANGE)

#define X1_ASYNC_VCGCHANGED                 (X1_ASYNC_VCG_ELECTION_STATE_CHANGE | \
                                             X1_ASYNC_VCG_ELECTION_STATE_ENDED | \
                                             X1_ASYNC_VCG_POWERUP | \
                                             X1_ASYNC_VCG_CFG_CHANGED)

#define X1_ASYNC_CONFIG_ALL                 (X1_ASYNC_BECHANGED | \
                                             X1_ASYNC_FECHANGED | \
                                             X1_ASYNC_VCG_CFG_CHANGED)

/* Some more granual PI async changed events higer 32 bits in 64 bit eventmap*/
#define PI_EVENT_ISE_CTRLR_CHANGED           0x00000001
#define PI_EVENT_ISE_DPAC_CHANGED            0x00000002
#define PI_EVENT_ISE_PS_CHANGED              0x00000004
#define PI_EVENT_ISE_BATTERY_CHANGED         0x00000008
#define PI_EVENT_ISE_CHASSIS_CHANGED         0x00000010

#define PI_ASYNC_NONE                       0x00000000


/*****************************************************************************
** Public variables
*****************************************************************************/

typedef union _ASYNC_CHANGED_EVENT
{
    UINT64      eventType;
    struct
    {
        UINT32      eventType1;
        UINT32      eventType2;
    } extendEvnt;

} ASYNC_CHANGED_EVENT;

#define PIASYNC_EVENT_FIRST32_MAP 0x0001
#define PIASYNC_EVENT_SECOND32_MAP 0x0002

typedef struct _PI_ASYNC_EVENT_QUEUE
{
    UINT32      event1;
    UINT32      event2;
    UINT16      bitmap;
    UINT16      rsvd;
} PI_ASYNC_EVENT_QUEUE;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void SendX1AsyncNotifications(LOG_MRP *logMRP);
extern INT32 X1_ElectionNotify(UINT8 electionState);
extern void X1_MonitorRaidInitialization(void);
extern UINT32 ResolveX1ReasonFromActionReason(UINT32 action, UINT32 reason);

/*----------------------------------------------------------------------------
** Function:    SendX1ChangeEvent
**
** Description: Sends an X1 change event.
**
** Inputs:      reason   -  Any combination of:
**                          X1_ASYNC_PCHANGED
**                          X1_ASYNC_RCHANGED
**                          X1_ASYNC_VCHANGED
**                          X1_ASYNC_HCHANGED
**                          X1_ASYNC_ACHANGED
**                          X1_ASYNC_ZCHANGED
**                          X1_ASYNC_VCG_ELECTION_STATE_CHANGE
**                          X1_ASYNC_VCG_ELECTION_STATE_ENDED
**                          X1_ASYNC_VCG_POWERUP
**                          X1_ASYNC_VCG_CFG_CHANGED
**
** Returns:     none
**
** WARNING:     none
**
**--------------------------------------------------------------------------*/
extern void SendX1ChangeEvent(UINT32 reason);

/*----------------------------------------------------------------------------
** Function:    EnqueueX1AsyncNotification
**
** Description: Enqueues AsyncNotifications to be sent later.
**
** Inputs:      action   -  What action to take.
**                              see cachemgr.h
**
**              reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
**
** Returns:     none
**
** WARNING:     none
**
**--------------------------------------------------------------------------*/
extern void EnqueueX1AsyncNotification(UINT32 action, UINT32 reason);
extern void EnqueuePIAsyncEvent(UINT32 eventMask, UINT16 bitmap);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _X1ASYNCEVENTHANDLER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
