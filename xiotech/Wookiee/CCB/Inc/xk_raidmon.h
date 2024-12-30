/* $Id: xk_raidmon.h 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       xk_raidmon.h
**
**  @brief      Header for module to monitor raid status of local
**              controller scsi disks
**
**  Header for module to monitor raid status of local controller scsi disks
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XK_RAIDMON_H_
#define _XK_RAIDMON_H_

#include "XIO_Types.h"
#include "xk_kernel.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif
/*******************************************************************************
**** Public defines - constants
*******************************************************************************/
#define XK_RAIDMON_MAGIC                    0x00000001

/* 6 drives + backplane */
#define XK_RAIDMON_MAX_DEVS                 7
#define XK_RAIDMON_MAX_DRIVES               6
#define XK_RAIDMON_MIN_DRIVE_THRESHOLD      2

/* present defines */
#define XK_RAIDMON_DEVICE_MISSING   0
#define XK_RAIDMON_DEVICE_PRESENT   1

/* type defines */
#define XK_RAIDMON_TYPE_DISK        1
#define XK_RAIDMON_TYPE_BACKPLANE   2

/* status defines */
#define XK_RAIDMON_STATUS_NONE      0
#define XK_RAIDMON_STATUS_FAILED    1
#define XK_RAIDMON_STATUS_REMOVED   2
#define XK_RAIDMON_STATUS_ONLINE    3
#define XK_RAIDMON_STATUS_RESYNCING 4

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef struct _XK_RAIDMON_DEVICE
{
    CHAR        name[32];       /* Name of the device               */
    CHAR        vendor[32];     /* Vendor                           */
    CHAR        model[32];      /* Device model                     */
    CHAR        serial[32];     /* serial number                    */
    CHAR        fwV[8];         /* fw level                         */
    UINT8       slot;           /* slot                             */
    UINT8       oldSlot;        /* old slot                         */
    UINT8       rsvd[2];        /* Reserved                         */
    UINT8       present;        /* device present                   */
    UINT8       type;           /* Type of device                   */
    UINT8       status;         /* status of the drive              */
    UINT8       resyncPC;       /* resync % complete                */
    volatile unsigned long resyncDevs;  /* Bitmap of resync devs            */
    volatile unsigned long failDevs;    /* Bitmap of failes devs            */
    UINT8       rsvd2[128];     /* Reserved 2                       */
} XK_RAIDMON_DEVICE;

typedef struct _XK_RAIDMON_INFO
{
    UINT32      magic;          /* Magic number                 */
    UINT8       dataMD5[16];    /* MD5 Signature mdstat file    */
    XK_RAIDMON_DEVICE disks[XK_RAIDMON_MAX_DEVS];
    volatile unsigned long actDevices;  /* Active disks by slot [0-5]   */
    UINT8       rsvd[1020];     /* rsvd for expansion           */
} XK_RAIDMON_INFO;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void XK_RaidMonitorTask(TASK_PARMS *tParms);
extern UINT8 XK_RaidMonitorIsResyncing(void);
extern UINT32 XK_RaidMonitorResyncingControllers(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XK_RAIDMON_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
