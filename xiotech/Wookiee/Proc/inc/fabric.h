/* $Id: fabric.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       fabric.h
**
**  @brief      Fabric Header file
**
**    To provide support of discovery of devices on the fabric.
**
**  Copyright (c) 2003-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _FABRIC_H_
#define _FABRIC_H_

#include "XIO_Types.h"
#include "system.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define MRDEXISTING     0       /* Rescan existing devices                  */
#define MRDLUNS         1       /* Rescan LUNS                              */
#define MRDREDISCOVER   2       /* Rediscover devices                       */

#define RESCAN_EXISTING           0x00    /* Rescan existing devices        */
#define RESCAN_LOOP               0x02    /* Rescan loop*/
#define RESCAN_EXISTING_NO_WAIT   0x03    /* Rescan existing devices no wait*/
#define RESCAN_LOOP_NO_WAIT       0x04    /* Rescan existing devices no wait*/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct DEV;

struct nst_t
{
    UINT32 portId;
    UINT32 reserved;
    UINT64 nodeName;
    UINT32 lid;
    UINT32 status;
    UINT64 portName;
};

#if defined(MODEL_7000) || defined(MODEL_4700)
/* ISE related defines and structures */

typedef struct ISE_VOLUMEINFO
{
    UINT8   ise_volstatus;
    UINT8   ise_rsvd[3];
    UINT32  ise_volindex;
    UINT64  ise_wwnid;
    UINT8   ise_volraid_type;
    UINT8   ise_prod_id[16];
    UINT8   ise_drive_id[16];
} ISE_VOLUMEINFO;

/* Bit definitions for ISE volume status definitions */

#define ISE_VOL_SLAVE       7
#define ISE_VOL_ZEROING     6
#define ISE_VOL_REBUILD     5
#define ISE_VOL_HOTSPARING  4
#define ISE_VOL_POOLBIT0    3
#define ISE_VOL_POOLBIT1    2
#define ISE_VOL_rsvd1       1
#define ISE_VOL_rsvd2       0
#endif /* MODEL_7000 || MODEL_4700 */

#define RSCN_QUEUE_SIZE 128
typedef struct RSCN_EVENT_QUEUE
{
    UINT32  out;
    UINT32  in;
    UINT32  queue[RSCN_QUEUE_SIZE];
} RSCN_EVENT_QUEUE;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern struct nst_t * fabNameServerTable[MAX_PORTS];
extern UINT32 fabNameServerSize[MAX_PORTS];
extern UINT32 fabNameServerCount[MAX_PORTS];
extern UINT32 F_notifyreq;
extern RSCN_EVENT_QUEUE rscnQueue[MAX_PORTS];

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void FAB_BalanceLoad(void);
extern UINT32 FAB_IsDevInUse(struct DEV *pDevice);
extern UINT32 FAB_ReValidateConnection(UINT8 port,struct DEV* device);
extern UINT32 F_rescanDevice(UINT32 scanType);
extern void FAB_BypassDevice(UINT32 parm1, UINT32 parm2, UINT16 pid);
extern void FAB_putLid(UINT8 port, UINT32 lid);
extern UINT32 F_findAltPort(struct DEV *);
extern void FAB_removeDevice(UINT8, struct DEV *);
extern void F_moveDevice(UINT8, struct DEV *);
extern void FAB_InsertRSCNEvent(UINT8 port, UINT32 portid);
extern void F_startPortMonitor(UINT8);

#endif /* _FABRIC_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
