/* $Id: fabric.c 161368 2013-07-29 14:53:10Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       fabric.c
**
**  @brief      Fabric C functions
**
**  To provide support of discovery of devices on the fabric.
**
**  Copyright (c) 2002-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "fabric.h"

#include "chn.h"
#include "def_con.h"
#include "dev.h"
#include "ficb.h"
#include "ilt.h"
#include "isp.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "loop.h"
#include "misc.h"
#include "MR_Defs.h"
#include "online.h"
#include "OS_II.h"
#include "pcb.h"
#include "portdb.h"
#include "pdd.h"
#include "ses.h"
#include "system.h"
#include "target.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include <stdio.h>
#include <byteswap.h>
#include <arpa/inet.h>
#if defined(MODEL_7000) || defined(MODEL_4700)
#include "ecodes.h"
#include "miscbe.h"
#endif /* MODEL_7000 || MODEL_4700 */

/* The following are for created tasks. */
extern unsigned long CT_fork_tmp;
extern void CT_LC_f_discovery(int);
extern void CT_LC_f_portMonitor(int);
extern void CT_LC_PHY_InitDrive(int);
/* Foreign includes. */
extern UINT8 SBOD_Trunking[MAX_PORTS];

extern PDX  gPDX;
extern EDX  gEDX;

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define ST_GPN_OK           0x4444
#define FABRIC_DEBUG        0x00        /* disable/enable debug fprintfs */

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define REQ_UP_DOWN_RSCN   ((1 << CH_LOOP_UP_REQ) | (1 << CH_LOOP_DN_REQ) | (1 << CH_RSCN_REQ))
#define REQ_UP_DOWN   ((1 << CH_LOOP_UP_REQ) | (1 << CH_LOOP_DN_REQ) )
#define REQ_RSCN_DOWN ((1 << CH_RSCN_REQ) | (1 << CH_LOOP_DN_REQ))
#define REQ_UP        (1 << CH_LOOP_UP_REQ)
#define REQ_DOWN      (1 << CH_LOOP_DN_REQ)
#define REQ_DISC      (1 << CH_LUN_DISC_REQ)
#define REQ_PURGE     (1 << CH_PURGE_REQ)
#define UP            (1 << CH_LOOP_UP)
#define REQ_RSCN      (1 << CH_RSCN_REQ)

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
struct
{
    UINT32      count;
    UINT32      size;
    UINT32      rsvd1;
    UINT32      rsvd2;

    struct
    {
        UINT8       port;
        UINT8       rsvd3;
        UINT16      lun;
        UINT32      lid;
        DEV        *dev;
        PDD        *pdd;
    } entry[1];
}          *F_dvlist;

/* Device list send to online log event */
static LOG_DVLIST_PKT *edvl;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
PCB        *fDiscoveryPcb;
PCB        *fPortMonitorPcb;
UINT32      F_notifyreq;
#define LID_USED_ELEMENT_COUNT 60
UINT32      lidUsed[MAX_PORTS][LID_USED_ELEMENT_COUNT];

UINT32      fabRscnCount[MAX_PORTS];

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern CHN *P_chn_ind[MAX_PORTS];
extern PDD *P_pddindx[MAX_PHYSICAL_DISKS];
extern PDD *E_pddindx[MAX_DISK_BAYS];
extern PDD *M_pddindx[MAX_MISC_DEVICES];
extern TGD *T_tgdindx[MAX_TARGETS];
extern UINT32 P_drvinits;

struct nst_t *fabNameServerTable[MAX_PORTS];
UINT32      fabNameServerSize[MAX_PORTS];
UINT32      fabNameServerCount[MAX_PORTS];

RSCN_EVENT_QUEUE rscnQueue[MAX_PORTS];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
extern void p$wakeup(DEV *pDev);
extern void P$fail_dev_cmds(DEV *pDev);
INT32       isp$check_initiator(UINT8 port, UINT16 lid, UINT64 *pNdn);
extern void CT_LC_FAB_PathMonitor(void);

void        FAB_PathMonitor(void);
DEV        *F_find_dev(UINT8, UINT64 *, UINT32, UINT16);
void        f_discovery(void);
void        f_portMonitor(UINT32, UINT32, UINT8);

#if defined(MODEL_7000) || defined(MODEL_4700)
static UINT8 f_ISEgetPortNumber(UINT32 port);
#else  /* MODEL_7000 || MODEL_4700 */
static UINT8 f_getPortNumber(PDB *pPortDb);
#endif /* MODEL_7000 || MODEL_4700 */
extern void ON_BEBusy(UINT16 pid, UINT32 TimetoFail, UINT8 justthisonepid);
extern void ON_BEClear(UINT16 pid, UINT8 justthisonepid);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      FAB_getLid
**
**              Acquires an unused LID in the range 0x80 - 0xFF, if available.
**              Marks this lid as being used.
**
**  @param      port    - Port
**
**  @return     lid     - Loop ID, 0 = no LIDs available
**
******************************************************************************
**/
static UINT32 FAB_getLid(UINT8 port)
{
    UINT32      map;
    UINT32      F_index;
    UINT32      lid = 0;        /* Initialize to 0 = no LID     */

    /* Find an available LID. */
    for (F_index = 0; F_index < LID_USED_ELEMENT_COUNT; ++F_index)
    {
        /* Get the next set of 32 LIDs, and check for unused LIDs in this set. */
        map = lidUsed[port][F_index];
        if (map != 0xFFFFFFFF)
        {
            /* Find the lowest bit not set. */
            lid = (F_index << 5) + 0x80;
            while ((map & 1) == 1)
            {
                ++lid;
                map >>= 1;
            }

            /* Mark the LID as in use. */
            lidUsed[port][F_index] |= (1 << (lid & 0x1F));
            break;
        }
    }
    return lid;
}   /* End of FAB_getLid */

/**
******************************************************************************
**
**  @brief      FAB_putLid
**
**              Marks the specified LID (range 0x80 - 0x7FF) as available
**              by clearing the corresponding bit in the lidUsed table.
**
**  @param      port    - Port
**  @param      lid     - Loop ID
**
**  @return     none
**
******************************************************************************
**/
void FAB_putLid(UINT8 port, UINT32 lid)
{
    /* Ensure the specified LID is only int the range 0x80 - 0x7FF. */
    lid &= 0x7FF;
    if (lid >= 0x80)
    {
        /* Mark the LID as unused. */
        lidUsed[port][(lid - 0x80) >> 5] &= ~(1 << (lid & 0x1F));
    }
}   /* End of FAB_putLid */

/**
******************************************************************************
**
**  @brief      FAB_useLid
**
**              Marks the specified LID (range 0x80 - 0x7FF) as used by setting
**              the corresponding bit in the lidUsed table.
**
**  @param      port    - Port
**  @param      lid     - Loop ID
**
**  @return     none
**
******************************************************************************
**/
static void FAB_useLid(UINT8 port, UINT32 lid)
{
    /* Ensure the requested LID is only in the range 0x80 - 0x7FF. */
    lid &= 0x7FF;
    if (lid >= 0x80)
    {
        /* Mark the LID as used. */
        lidUsed[port][(lid - 0x80) >> 5] |= (1 << (lid & 0x1F));
    }
}   /* End of FAB_useLid */

/**
******************************************************************************
**
**  @brief      FAB_clearLid
**
**              Initialize the lidUsed table by clearing the bits for all LIDs
**              and setting the bits for LIDs reserved for use by the hardware.
**
**  @param      port    - Port
**
**  @return     none
**
******************************************************************************
**/
void FAB_clearLid(UINT8 port)
{
    UINT32      F_index;

    /* Set LIDs 0x80 to 0xFF as unused (reserve LIDs 0x80 and 0xFF) */

    for (F_index = 0; F_index < LID_USED_ELEMENT_COUNT; ++F_index)
    {
        lidUsed[port][F_index] = 0x00000000;
    }

    lidUsed[port][LID_USED_ELEMENT_COUNT - 1] = 0xFFFF0000;     /* 0x7f0-0x7ff reserved */
}   /* End of FAB_clearLid */

/**
******************************************************************************
**
**  @brief      Moves a device record from the owning port to this port, as a
**              method of balancing the configuration.
**
**              This routine examines the device list for this port to see
**              whether this WWN is present. If so, the LID and port are
**              verified; if the device has moved, the lists are updated
**              accordingly, and the ONLINE layer is notified.
**
**  @param      port    - New port number
**  @param      * dev   - Pointer to device record
**
**  @return     none
**
******************************************************************************
**/
void F_moveDevice(UINT8 port, DEV *dev)
{
    UINT8       oldPort;

    if (port >= MAX_PORTS)
    {
        fprintf(stderr, "%s:%d faulting DEV list corruption\n", __func__, __LINE__);
        // This will break the list so aborting here will get a core dump.
        abort();
    }

    /* Get the current port number from the DEVice record */
    oldPort = dev->port;

    /* Check if the port to move the device from does not exist. */
    if (oldPort < MAX_PORTS)
    {
#ifdef DEBUG_FLIGHTREC_FD
        /* Fabric - Move DEV */
        MSC_FlightRec(0x4026, oldPort | (port << 8) | (dev->lid << 16),
                      *((UINT32 *)&dev->pLid), (UINT32)dev);
#endif /* DEBUG_FLIGHTREC_FD */

        /*
         * Check if this device is at the head of the device list,
         * and if so update the head of the device list.
         */
        if (P_chn_ind[oldPort]->devList == dev)
        {
            /* Is this the only device on the list. */
            if (dev->nDev == dev)
            {
                /* The list is now empty. */
                P_chn_ind[oldPort]->devList = NULL;
            }
            else
            {
                /* The next device becomes the head. */
                P_chn_ind[oldPort]->devList = dev->nDev;
            }

#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0x01004026,
                          ((P_chn_ind[oldPort]->devList) ? (UINT32)P_chn_ind[oldPort]->devList->pdev : 0),
                          (UINT32)P_chn_ind[oldPort]->devList,
                          ((P_chn_ind[oldPort]->devList) ? (UINT32)P_chn_ind[oldPort]->devList->nDev : 0));
#endif /* DEBUG_FLIGHTREC_FD */
        }

        /*
         * Check if this device is at the head of the start list,
         * and if so update the head of the start list.
         */
        if (P_chn_ind[oldPort]->startDev == dev)
        {
            /* Is this the only device on the list. */
            if (dev->nDev == dev)
            {
                /* The list is now empty. */
                P_chn_ind[oldPort]->startDev = NULL;
            }
            else
            {
                /* The next device becomes the head. */
                P_chn_ind[oldPort]->startDev = dev->nDev;
            }

#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0x02004026,
                          ((P_chn_ind[oldPort]->startDev) ? (UINT32)P_chn_ind[oldPort]->
                           startDev->pdev : 0), (UINT32)P_chn_ind[oldPort]->startDev,
                          ((P_chn_ind[oldPort]->startDev) ? (UINT32)P_chn_ind[oldPort]->
                           startDev->nDev : 0));
#endif /* DEBUG_FLIGHTREC_FD */
        }

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x03004026, (UINT32)dev->nDev, (UINT32)dev->pdev,
                      (UINT32)P_chn_ind[oldPort]->devList);
#endif /* DEBUG_FLIGHTREC_FD */
    }

    /* Remove DEV record from owning port's valid attached device list. */
    if ((dev->nDev != NULL) && (dev->pdev != NULL))
    {
        dev->nDev->pdev = dev->pdev;
        dev->pdev->nDev = dev->nDev;
    }

    /* Is the list current empty? */
    if (P_chn_ind[port]->devList == NULL)
    {
        P_chn_ind[port]->devList = dev;
        P_chn_ind[port]->startDev = dev;

        dev->nDev = dev;
        dev->pdev = dev;
    }
    else
    {
        dev->nDev = P_chn_ind[port]->devList;
        dev->pdev = P_chn_ind[port]->devList->pdev;
        dev->nDev->pdev = dev;
        dev->pdev->nDev = dev;
    }

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x04004026, (UINT32)dev->nDev, (UINT32)dev->pdev, (UINT32)P_chn_ind[port]->devList);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Update port pointer in DEV record. */
    dev->port = port;

    /* Update LID in DEV record. */
    dev->lid = dev->pLid[port];

    /* Update PDD for this DEV. */
    if (dev->pdd)
    {
        dev->pdd->id = dev->lid;
        dev->pdd->channel = port;
    }

    /* Update device counts in port records. */
    if (oldPort < MAX_PORTS)
    {
        --P_chn_ind[oldPort]->devCnt;
    }
    ++P_chn_ind[port]->devCnt;
}   /* End of F_moveDevice */

/**
******************************************************************************
**
**  @brief      Removes the path from the specified port to all devices.
**
**  @param      port - port to be removed from.
**
**  @return     none
**
******************************************************************************
**/
static void f_detachPort(UINT8 port)
{
    DEV        *device;
    UINT32      i;
    UINT32      count;

    /*
     * All devices on all ports are processed. The path to port
     * which the loop down occurred is marked as nonexistent.
     */
    for (i = 0; i < MAX_PORTS; ++i)
    {
        /* Start with the first device on each port. */
        if (P_chn_ind[i] != NULL)
        {
            device = P_chn_ind[i]->devList;
            count = P_chn_ind[i]->devCnt;

            while (count-- > 0)
            {
                /*
                 * Invalidate the path to every device on loop down port.
                 * Set the wait time to a long time. If device is still attached,
                 * the wait time will be set to zero. If the device is missing,
                 * the wait time will be set to a more reasonable value later
                 * when find alt path get called by purge device.
                 */
                device->pLid[port] = NO_CONNECT;
                device->sLid[port] = NO_CONNECT;

                if (device->port == port)
                {
#ifdef DEBUG_FLIGHTREC_FD
                    /* Fabric - Detach DEV */
                    MSC_FlightRec(0x9026, port | (device->lun << 8) | (device->lid << 16),
                                  *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */
                    device->wait = 0x7FFF;
                }

                /* Advance to next device. */
                device = device->nDev;
            }
        }
    }
}   /* End of f_detachPort */

/**
******************************************************************************
**
**  @brief      To find a path to a device on a different port.
**              than the specified port.
**
**              Each port is examined to see if it has a path
**              to the device. The port with the fewest
**              active devices attached is selected.
**
**              If an alternate port is found, moveDevice is called
**              to move the device from the current owning port to
**              the alternate port.
**
**  @param      * device    - Device pointer
**
**  @return     New port
**
******************************************************************************
**/
UINT32 F_findAltPort(DEV *device)
{
    UINT8       altPort = 0xFF;
    UINT8       port;

    /* Check if a device is valid. */
    if (device != NULL)
    {
        /* Search for an alternate path. */
        for (port = 0; port < MAX_PORTS; ++port)
        {
            if (port != device->port && device->pLid[port] != NO_CONNECT)
            {
                altPort = port;
                break;
            }
        }

        /* Was an alternate path found */
        if (altPort != 0xFF)
        {
            /* Move this device to the alternate port.  */
            F_moveDevice(altPort, device);

            /*
             * Catch the case where detach port set an abnormally high wait count.
             * Otherwise, don't touch the wait count (set by recovery actions).
             */
            if (device->wait > 10000 / QUANTUM) /* 10s */
            {
                device->wait = 250 / QUANTUM;   /* 250ms */
            }
        }
#ifdef DEBUG_FLIGHTREC_FD
        /* Fabric - Find Alt Port */
        MSC_FlightRec(0x6026, device->port | (device->lun << 8) | (device->lid << 16),
                      *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */
    }
    return altPort;
}   /* End of F_findAltPort */


/**
******************************************************************************
**
**  @brief      Check if any path to the specified exists.
**
**              The path in the device structure is examined for a value
**              other than 0xFF which indicates a device is connected
**              on that port.
**
**  @param      * device    - Device pointer
**
**  @return     Number of paths to device.
**
******************************************************************************
**/
static UINT32 f_doesPathExist(DEV *dev)
{
    UINT32      i;
    UINT32      pathCount = 0;

    /* Check if any path exists for this device */
    for (i = 0; i < MAX_PORTS; ++i)
    {
        /* Check if a path exist for this port */
        if (dev->pLid[i] != NO_CONNECT)
        {
            /* Increment the path count. */
            ++pathCount;
        }

        /* Check if a secondary path exists for this port */
        if (dev->sLid[i] != NO_CONNECT)
        {
            /* Increment the path count. */
            ++pathCount;
        }
    }
    return pathCount;
}   /* End of f_doesPathExist */

/**
******************************************************************************
**
**  @brief      Remove the specified device from the specified port
**
**              Checks if this device is attached to this port.
**              To remove from the port, the device is removed
**              from the device linked list. The device linked list
**              head pointer and device next pointer are updated
**              if appropriate. The device is removed from the
**              port device array. The device LID is removed from
**              the port path. The port device count is
**              decrement by one. Any outstanding commands to this
**              device are failed back to the next higher level.
**              Finally the memory for the device structure is released.
**
**  @param      port        - Port to be removed from
**  @param      * device    - Device pointer
**
**  @return     none
**
******************************************************************************
**/
void FAB_removeDevice(UINT8 port, DEV *dev)
{
    /* Check if this port and the device exists. */
    if (P_chn_ind[port] != NULL && dev != NULL)
    {
#ifdef DEBUG_FLIGHTREC_FD
        /* Fabric - Remove DEV */
        MSC_FlightRec(0x5026, dev->port | (port << 8) | (dev->lid << 16),
                      *((UINT32 *)&dev->pLid), (UINT32)dev);
#endif /* DEBUG_FLIGHTREC_FD */

        /* Check if this port owns this device */
        if (dev->port == port)
        {
            /*
             * Check if this device is at the head of the device list,
             * and if so update the head of the device list.
             */
            if (P_chn_ind[port]->devList == dev)
            {
                /* Is this the only device on the list. */
                if (dev->nDev == dev)
                {
                    /* The list is now empty. */
                    P_chn_ind[port]->devList = NULL;
                }
                else
                {
                    /* The next device becomes the head. */
                    P_chn_ind[port]->devList = dev->nDev;
                }
            }

            /*
             * Check if this device is at the head of the start list,
             * and if so update the head of the start list.
             */
            if (P_chn_ind[port]->startDev == dev)
            {
                /* Is this the only device on the list. */
                if (dev->nDev == dev)
                {
                    /* The list is now empty. */
                    P_chn_ind[port]->startDev = NULL;
                }
                else
                {
                    /* The next device becomes the head. */
                    P_chn_ind[port]->startDev = dev->nDev;
                }
            }

            /* Remove DEV record from owning port's valid attached device list. */
            dev->nDev->pdev = dev->pdev;
            dev->pdev->nDev = dev->nDev;

            /* Clear DEVices next and previous pointers */
            dev->nDev = NULL;
            dev->pdev = NULL;
            dev->port = 0xff;

            /* Update device counts in port records. */
            --P_chn_ind[port]->devCnt;
        }

        /* Clear the path to this device for this port. */
        dev->pLid[port] = NO_CONNECT;
        dev->sLid[port] = NO_CONNECT;

        /* Check if any path exists for this device */
        if (f_doesPathExist(dev) == 0)
        {
#ifdef DEBUG_FLIGHTREC_FD
            /* Fabric - Remove DEV */
            MSC_FlightRec(0x8026, dev->port | (dev->lun << 8) | (dev->lid << 16),
                          *((UINT32 *)&dev->pLid), (UINT32)dev);
#endif /* DEBUG_FLIGHTREC_FD */

            /* Invalidate the port for this DEVice. */
            dev->port = 0xFF;
#if defined(MODEL_7000) || defined(MODEL_4700)
            if (dev->pdd && (BIT_TEST(dev->pdd->flags, PD_BEBUSY) == FALSE) &&
                dev->pdd->devType <= PD_DT_MAX_DISK)
            {
                fprintf(stderr, "%s setting busy pid %d ts %u\n", __func__, dev->pdd->pid, timestamp);
                ON_BEBusy(dev->pdd->pid, 45, 0);
            }
#endif /* MODEL_7000 || MODEL_4700 */
            /*
             * Return any commands queued to this DEV as
             * failed for nonexistent device.
             */
            P$fail_dev_cmds(dev);
        }
    }
}   /* End of FAB_removeDevice */

/**
******************************************************************************
**
**  @brief      The function attempts to find alternate paths
**              for all devices on a specified port.
**
**              Examines each device on the port by walking the
**              device array and searches for an alternate path.
**              The device is then removed from the current port.
**
**              If the port is in the process of executing a
**              command or if there are ILTs queued for a particular
**              device, online is notified of the configuraion change.
**
**  @param      port    - Port
**
**  @return     none
**
******************************************************************************
**/
static void f_failover(UINT8 port)
{
    DEV        *device;
    DEV        *nextDevice;
    UINT32      count;

    /* Is this port valid? */
    if (P_chn_ind[port] != NULL)
    {
        /* Clear the loop down request and clear Loop Up Status */
        P_chn_ind[port]->state &= ~((REQ_DOWN) | (UP));

        /* Update all device with the path to this port as nonexistent. */
        f_detachPort(port);
        // f_detachPort has now blocked io processing by setting dev->wait to a huge value.

        /* Start with first device in device list.  */
        device = P_chn_ind[port]->devList;

        if (device != NULL)
        {
            /* Get the count of device on this port. */
            count = P_chn_ind[port]->devCnt;

            /* Process each device attached to this port. */
            while (count-- > 0)
            {
                /* Get next device in linked list. */
                nextDevice = device->nDev;

                /* Remove the device from this port. */
                FAB_removeDevice(port, device);
                // FAB_removeDevice will fail all IO to this device if we go to 0 connections.

                /* Check if an alternate path is available for this device. */
                if (F_findAltPort(device) >= MAX_PORTS)
                {
                    /*
                     * Indicate this device is offline. Set the wait time
                     * to prevent commands for this device from being failed
                     * in case the device comes online during this time period.
                     */
                    device->flags = (1 << DV_OFFLINE);
                }
                // Unblock device.
                device->wait = 0;

                /* Advance to next device and check if next device exists. */
                device = nextDevice;
                if (device == NULL)
                {
                    break;
                }
            }
        }
    }
}   /* End of f_failover */

/**
******************************************************************************
**
**  @brief      Fork a discovery process for all devices on the specified ports.
**
**  @param      port    - Port
**
**  @return     none
**
******************************************************************************
**/
static void f_lunDiscovery(UINT8 port)
{
    DEV        *device;
    DEV        *nextDev;
    UINT32      count;

    /* Clear the lun discovery request for this port. */
    P_chn_ind[port]->state &= ~(REQ_DISC);

    /* Start with first device in device list. */
    device = P_chn_ind[port]->devList;

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0000F026, port, P_chn_ind[port]->devCnt, (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */

    if (device == NULL)
    {
        return;
    }

    /* Notify the online module of devices found. */
    F_notifyreq |= (1 << port);

    /* Get the count of device on this port. */
    count = P_chn_ind[port]->devCnt;

    /* Process each device attached to this port. */
    while (count-- > 0)
    {
        nextDev = device->nDev;

        /* Only Process LUN zero. */
        if (device->lun == 0)
        {
            /* Increment device initializations count. */
            ++P_drvinits;

            /* Fork the LUN disovery process */
            CT_fork_tmp = (unsigned long)"PHY_InitDrive";
            TaskCreate3(C_label_referenced_in_i960asm(PHY_InitDrive), F_INIT_DRV_PRI, (UINT32)device->pdd);
            TaskSwitch();

#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0x0100F026, port, P_drvinits, (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */
        }

        /* Check if next device exists. */
        device = nextDev;
        if (device == NULL)
        {
            break;
        }
    }
}   /* End of f_lunDiscovery */

/**
******************************************************************************
**
**  @brief      f_bld_dev
**
**  @param      port   - Port number
**  @param      lid    - Device LID
**  @param      * wwn  - Pointer to node WWN for this device
**  @param      lun    - LUN
**
**  @return     Pointer to a device structure
**
******************************************************************************
**/
static DEV *f_bld_dev(UINT8 port, UINT32 lid, UINT64 *wwn, UINT16 lun)
{
    DEV        *device;
    PDD        *pdd;

    /* Allocate DEV structure for this device */
    device = s_MallocC(sizeof(DEV) | BIT31, __FILE__, __LINE__);

    /* Store device's node WWN */
    device->nodeName = *wwn;

    /* Allocate and initialize a PDD for this device */
    pdd = DC_AllocPDD();

    pdd->channel = port;
    pdd->lun = lun;
    pdd->wwn = *wwn;
    pdd->id = lid;

    device->pdd = pdd;          /* Link PDD to DEV              */
    pdd->pDev = device;         /* Link DEV to PDD              */

    return device;
}   /* End of f_bld_dev */

/**
******************************************************************************
**
**  @brief      Initialize a device structure.
**
**  @param      * dev  - Pointer to device structure to initialize
**  @param      port   - port number
**  @param      lid    - Device LID
**  @param      lun    - LUN
**
**  @return     none
**
******************************************************************************
**/
static void f_initDevice(DEV *dev, UINT8 port, UINT32 lid, UINT16 lun)
{
    UINT32      i;

    /* Update port, LID, and LUN in DEV record. */
    dev->port = port;
    dev->lid = lid;
    dev->lun = lun;

    /* Check if any path exists for this device */
    for (i = 0; i < MAX_PORTS; ++i)
    {
        /* Invalidate path. */
        dev->pLid[i] = NO_CONNECT;
        dev->sLid[i] = NO_CONNECT;
    }

    /* Set Path for this port */
    dev->pLid[port] = lid;

#ifdef DEBUG_FLIGHTREC_FD
    /* Fabric - Init DEV */
    MSC_FlightRec(0x1026, port | (lun << 8) | (dev->lid << 16), *((UINT32 *)&dev->pLid), (UINT32)dev);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Initialize tagged command map */
    dev->simpleCnt = MAX_SIMPLE;        /* Set up maximum simple tagged count   */
    if ((dev->pdd) && (dev->pdd->devType != PD_DT_SATA))
    {
        /* Non-SATA type drive */
        dev->tMapAsgn = ~MAXTAGMSK;     /* Set up tag assignment map            */
        dev->tMapMask = MAXTAGMSK;      /* Set up tag assignment mask           */
    }
    else
    {
        /* SATA type drive */
        dev->tMapAsgn = ~MAXTAGSATAMSK; /* Set up tag assignment map       */
        dev->tMapMask = MAXTAGSATAMSK;  /* Set up tag assignment mask       */
    }

    /* Link HDA token to ILT queue */
    if (dev->iltQFHead != &dev->hdaToken && dev->iltQFHead != NULL)
    {
        // If ios
        abort();
    }
    dev->iltQFHead = &dev->hdaToken;
    dev->iltQTail = &dev->hdaToken;
    dev->hdaToken.bthd = (ILT *)dev;
    dev->hdaToken.fthd = NULL;

    /* Clear other fields */
    dev->tagLock = 0;           /* Unlock this device                   */
    dev->pri = 0;               /* Default priority criteria            */
    dev->sprc = 0;              /* Clear periodic request count         */
    dev->spsc = 0;              /* Clear periodic sector count          */
    dev->flags = 0;             /* Clear flags                          */

    /* Is the device list for this port empty? */
    if (P_chn_ind[port]->devList == NULL)
    {
        /* Initialize count of attached devices for this port to one. */
        P_chn_ind[port]->devCnt = 1;

        /* Initialize prev and next DEV pointers */
        dev->nDev = dev;
        dev->pdev = dev;

        /* Insert at the head (the only member) of the device list. */
        P_chn_ind[port]->devList = dev;
        P_chn_ind[port]->startDev = dev;
    }
    else
    {
        /* Increment count of attached devices for this port. */
        ++P_chn_ind[port]->devCnt;

        /* Initialize prev and next DEV pointers. */
        dev->nDev = P_chn_ind[port]->devList;
        dev->pdev = P_chn_ind[port]->devList->pdev;

        /* Insert at the tail of the device list. */
        dev->nDev->pdev = dev;
        dev->pdev->nDev = dev;
    }

    /* Clear tag ilt area */
    for (i = 0; i < MAX_TAG; ++i)
    {
        /* Clear entry */
        dev->tagIlt[i] = NULL;
    }
}   /* End of f_initDevice */

/**
******************************************************************************
**
**  @brief      Create a list of devices of debug purposes.
**
**              The total device count is determined by summing the
**              device count from each port. The device list
**              is scanned on each port and data from each device
**              is written to memory.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void F_showDevice(void)
{
    DEV        *device;
    UINT32      i;
    UINT32      j = 0;
    UINT32      count = 0;

    /* Count the number of devices. */
    for (i = 0; i < MAX_PORTS; ++i)
    {
        /* Get the count of devices on each port. */
        if (P_chn_ind[i] != NULL)
        {
            count += P_chn_ind[i]->devCnt;
        }
    }

    /* Has a block of memory been previously allocated. */
    if (F_dvlist != NULL)
    {
        /* Free previous block of memory if too small. */
        if (count > F_dvlist->count)
        {
            s_Free(F_dvlist, F_dvlist->size, __FILE__, __LINE__);
            F_dvlist = NULL;
        }
    }

    if (F_dvlist == NULL)
    {
        /* Allocate block of memory for debug infomation. */
        F_dvlist = s_MallocC((count + 1) * 16, __FILE__, __LINE__);
        F_dvlist->size = (count + 1) * 16;
    }
    else
    {
        /* Zero out memory block if old block is used. */
    }

    /* Set the count of devices. */
    F_dvlist->count = count;

    /* Check if any device exist. */
    if (count == 0)
    {
        return;
    }

    /* Gather information from each device */
    for (i = 0; i < MAX_PORTS && j < F_dvlist->count; ++i)
    {
        /* Get the count of devices on each port. */
        if (P_chn_ind[i] != NULL)
        {
            device = P_chn_ind[i]->devList;
            count = P_chn_ind[i]->devCnt;

            /* Process each device */
            while (count-- > 0 && device != NULL)
            {
                /* Store information for this device in debug area. */
                F_dvlist->entry[j].port = device->port;
                F_dvlist->entry[j].lun = device->lun;
                F_dvlist->entry[j].lid = device->lid;
                F_dvlist->entry[j].dev = device;
                F_dvlist->entry[j].pdd = device->pdd;

                /* Advance to next device.  */
                device = device->nDev;

                /* Increment index. */
                ++j;

                /* Make sure something didn't change on us. */
                if (j >= F_dvlist->count)
                {
                    break;
                }
            }
        }
    }
}   /* End of F_showDevice */

/**
******************************************************************************
**
**  @brief      Attempt to load balance the two connected BE ports.
**
**              This routine examines the device list for this port to see
**              whether this WWN is present. If so, the LID and port are
**              verified; if the device has moved, the lists are updated
**              accordingly, and the ONLINE layer is notified.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void FAB_BalanceLoad(void)
{
    DEV        *device;
    DEV        *prevDevice;
    DEV        *firstDevice;
    UINT32      minDeviceCount;
    UINT32      deviceCount[MAX_PORTS];
    UINT8       altPort;
    UINT8       port;
    UINT32      j;

    /*
     * Initialize the device count for each port to 0 before starting to load
     * balance. Note that deviceCount[0] is initialized inside second loop.
     */
    for (port = 1; port < MAX_PORTS; ++port)
    {
        deviceCount[port] = 0;
    }

    for (port = 0; port < MAX_PORTS; ++port)
    {
        /*
         * Zero the count for the port working on. The port before it most
         * likely has changed it by "moving" a device to it for load sharing.
         * By zero-ing it here, then we will leave the first xxx alone and
         * only move devices if "more than the first port" are found to be on
         * it -- which will happen in normal circumstances. This code probably
         * should be changed to count how many devices are on each port, and
         * move only those needed to balance it out. Of course, an even better
         * choice would be to balance out devices by how heavily they are used.
         */
        deviceCount[port] = 0;

        /*
         * Walk the linked list of devices in reverse order. Get the previous
         * pointer now since the device may be moved.
         */
        if (P_chn_ind[port] != NULL && P_chn_ind[port]->devList != NULL)
        {
            /* Get the first device on the list. */
            firstDevice = P_chn_ind[port]->devList;
            prevDevice = firstDevice->pdev;

            do
            {
                /* Advance to previous device */
                device = prevDevice;

                /* Get next previous before moving this device. */
                prevDevice = device->pdev;

                /* Initialize for this loop. */
                altPort = port;
                minDeviceCount = deviceCount[port];

                /*
                 * Check each device for multiple path and choose the port
                 * which contains the least number devices.
                 */
                for (j = 0; j < MAX_PORTS; ++j)
                {
                    /* Don't do the port a second time (outer loop). */
                    if (j == port)
                    {
                        continue;
                    }

                    /*
                     * Skip if this port is not connected, or if setup4retry
                     * (error recovery) is running on the device.
                     */
                    if (device->pLid[j] != NO_CONNECT && !device->setupretryactive)
                    {
                        /*
                         * If a single controller (this can be the case where
                         * it is an edge controller or the other controller(s)
                         * failed us to a single controller; or this can be the
                         * case where all 4 BE ports are used when BE Fabric
                         * configuration exists for a 2-way system), or if we
                         * are on fabric, or the loop only has SBODs and they
                         * are wired as trunking.
                         */
                        if (((UINT32)T_tgdindx[-1] <= TARGETS_PER_CTRL) ||
                            !(BIT_TEST(K_ii.status, II_CCBREQ)) ||
                            (ISP_IsFabricMode(j)) ||
                            (SBOD_Trunking[j] == TRUE))
                        {
                            /* Get device count for this port. */
                            if (deviceCount[j] < minDeviceCount)
                            {
                                minDeviceCount = deviceCount[j];

                                /* Save this port as a candidate to switch to. */
                                altPort = j;
                            }
                        }
                        else
                        {
                            /*
                             * Controllers with an even serial number use even
                             * numbered ports. Controllers with an odd serial
                             * number use odd numbered ports.
                             */
                            if ((j & 1) == (K_ficb->cSerial & 1))
                            {
                                /* Save this port as a candidate to switch to. */
                                altPort = j;
                                break;
                            }
                        }
                    }
                }

                /* Check if the port currently in use is degraded. */
                if (P_chn_ind[altPort]->degraded == TRUE)
                {
                    /*
                     * Check each device for multiple path and choose the port
                     * that is not degraded.
                     */
                    for (j = 0; j < MAX_PORTS; ++j)
                    {
                        /* Don't do the port a second time (outer loop). */
                        if (j == altPort)
                        {
                            continue;
                        }

                        /* Skip if this port is not connected. */
                        if (device->pLid[j] != NO_CONNECT)
                        {
                            /* Check if the desired port is degraded. */
                            if (P_chn_ind[j]->degraded == FALSE)
                            {
                                /* Save this port as a candidate to switch to. */
                                altPort = j;
                                break;
                            }
                        }
                    }
                }

                /* Was an alternate port found? */
                if (altPort != port)
                {
                    /* Move this device to the alternate port. */
                    F_moveDevice(altPort, device);

                    /* Wakeup any pending commands on the new port. */
                    p$wakeup(device);
                }

                /* Increment load balanced device count for selected port. */
                deviceCount[altPort]++;
            } while (device != firstDevice);
        }
    }

    /* Show devices for debug */
    F_showDevice();
}   /* End of FAB_BalanceLoad */

/**
******************************************************************************
**
**  @brief      Determine whether a specified device was
**              previously known to be attached to the system.
**
**              This routine examines the device list for this port to see
**              whether this WWN is present. If so, the LID and port are
**              verified; if the device has moved, the lists are updated
**              accordingly, and the ONLINE layer is notified.
**
**  @param      * wwn  - Pointer to device node WWN
**  @param      lun    - LUN
**
**  @return     Pointer to PDD struct if device found,
**              else 0 if new device WWN.
**
******************************************************************************
**/
static PDD *f_find_pdd(UINT64 *wwn, UINT16 lun)
{
    UINT32      i;
    UINT32      j;
    UINT32      limit;
    PDD       **pdd;
    PDD        *returnPdd = NULL;

    /*
     * Three PDD lists will be checked, the drive list,
     * the enclosure list, and the miscellaneous device list.
     */
    for (j = 0; j < 3; ++j)
    {
        /* Is this the first time thru the loop? */
        if (j == 0)
        {
            /* Check the list of drives. */
            pdd = P_pddindx;
            limit = MAX_PHYSICAL_DISKS;
        }

        /* Is this the second time thru the loop? */
        else if (j == 1)
        {
            /* Check the list of enclosures. */
            pdd = E_pddindx;
            limit = MAX_DISK_BAYS;
        }

        /* Is this the last time thru the loop? */
        else
        {
            /* Check the list of miscellaneous devices. */
            pdd = M_pddindx;
            limit = MAX_MISC_DEVICES;
        }

        /* Check each device in list. */
        for (i = 0; i < limit; ++i)
        {
            /* Is there a PDD at this slot? */
            if (pdd[i] != NULL)
            {
                /* Check for WWN and LUN */
                if (pdd[i]->lun == lun && pdd[i]->wwn == *wwn)
                {
                    /* The PDD for this WWN and LUN has been found. */
                    returnPdd = pdd[i];
                    break;
                }
            }
        }

        /* Was a PDD found? */
        if (returnPdd != NULL)
        {
            break;
        }
    }

    return returnPdd;
}   /* End of f_find_pdd */

#if defined(MODEL_7000) || defined(MODEL_4700)

/**
******************************************************************************
**
**  @brief  Return array of luns on device
**
**  @param  device - Pointer to DEV structure for device LUN 0
**  @param  luns - Pointer to array to receive single-byte LUN values
**  @param  lunmax - Maximum number of LUNs to return
**
**  @return Number of LUNs returned
**
******************************************************************************
**/

extern PRP_TEMPLATE P_t_reportluns;

static int get_device_luns(DEV *device, UINT8 *luns, UINT16 lunmax)
{
    ILT        *pILT;
    UINT8      *pBuf;
    PRP        *pPRP;
    int         status;
    int         try;
    PRP_TEMPLATE rl_cmd;

    rl_cmd = P_t_reportluns;
    rl_cmd.rqBytes = 16 + 8 * MAX_ISE_LUNS;
    rl_cmd.cmd[6] = ((16 + 8 * MAX_ISE_LUNS) >> 24) & 0xff;
    rl_cmd.cmd[7] = ((16 + 8 * MAX_ISE_LUNS) >> 16) & 0xff;
    rl_cmd.cmd[8] = ((16 + 8 * MAX_ISE_LUNS) >> 8) & 0xff;
    rl_cmd.cmd[9] = (16 + 8 * MAX_ISE_LUNS) & 0xff;

    if (device == NULL || device->pdd == NULL)
    {
        fprintf(stderr, "%s: device or PDD is NULL\n", __func__);
        return 0;
    }

    for (try = 0; try < MAX_REPORTLUN_RETRIES; ++try)
    {
        pILT = ON_GenReq(&rl_cmd, device->pdd, (void **)&pBuf, &pPRP);
        ON_QueReq(pILT);
        status = MSC_ChkStat(pPRP);
        if (status == EC_OK)
        {
            break;
        }
        ON_RelReq(pILT);
        fprintf(stderr, "%s: Report LUNs command returned %d, try=%d\n",
                __func__, status, try);
    }
    if (status != EC_OK)
    {
        return 0;
    }

    int         cnt;
    int         ix;
    int         ox;

    /*
     * Assemble the total LUN count from pBuf.
     *
     * Note that this is the numbr of LUNs the target
     * would like to report. Not how many fit into the
     * buffer length we supplied.
     *
     * Error if the target has more LUNs to report than
     * we provided space for.
     */
    cnt = bswap_32(*((UINT32 *)(&pBuf[0]))) / 8;

    if (cnt > lunmax)
    {
        fprintf(stderr, "%s: cnt (%d) too big\n", __func__, cnt);
        ON_RelReq(pILT);
        return 0;
    }

    for (ix = 0, ox = 0; ix < cnt; ++ix)
    {
        UINT64      lun;
        int         lunix;

        lunix = (ix + 1) * 8;
        lun = 0LL;

        /* Retrieve the full 64-bit LUN from pBuf. */
        lun = bswap_64(*((UINT64 *)(&pBuf[lunix])));

        /*
         * For Nitrogen, the ISE serves up the LUN as a first-level,
         * peripheral device address. See SAM-3 Section 4.9.5/6 for
         * details - that will explain why it is not at the low
         * end of the 64-bit value.
         *
         * Note the >>= 48 in the if()
         */
        if ((lun & 0xFF00FFFFFFFFFFFFLL) != 0 || (lun >>= 48) > MAX_ISE_LUNS)
        {
            fprintf(stderr, "%s: lun number out of range: %016llx\n", __func__, lun);
            continue;
        }
        luns[ox++] = (UINT8)lun;
    }
    ON_RelReq(pILT);

    return ox;
}   /* End of get_device_luns */
#endif /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
**  @brief      Determine whether a specified device was previously known to
**              be attached to the system.
**
**              This routine examines the device list for this port to see
**              whether this WWN is present. If so, the LID and port are
**              verified; if the device has moved, the lists are updated
**              accordingly, and the ONLINE layer is notified.
**
**  @param      port    - Port
**  @param      * wwn   - Pointer to device node WWN
**  @param      lid     - Current LID
**  @param      lun     - LUN
**
**  @return     Pointer to device structure
**
******************************************************************************
**/
DEV        *F_find_dev(UINT8 port, UINT64 *wwn, UINT32 lid, UINT16 lun)
{
    UINT32      i = port;
    DEV        *device = NULL;
    PDD        *pdd;
    UINT32      count;
    UINT32      deviceFound = FALSE;

#ifdef DEBUG_FLIGHTREC_FD
    UINT32      frParm0;        /* Temporary flight recorder parm 0 */

    /* Fabric - Find DEV */
    MSC_FlightRec(0x2026, port | (lun << 8) | (lid << 16), ((UINT32 *)wwn)[0], ((UINT32 *)wwn)[1]);
#endif /* DEBUG_FLIGHTREC_FD */
    /* Do not try to find a device if the port is not attached. */
    if (port >= MAX_PORTS)
    {
        fprintf(stderr, "<INVDEVPORT> invalid port is passed to F_find_dev.\n");
        return (NULL);
    }

    do
    {
        /* Process each device in attached to this port. */
        if (P_chn_ind[i] != NULL)
        {
            /* Get starting device and count of devices */
            device = P_chn_ind[i]->devList;
            count = P_chn_ind[i]->devCnt;

            /* Does the device list exist. */
            if (device != NULL)
            {
                /* Process and devices in the list. */
                while (count-- > 0)
                {
                    /* Check for matching WWN and lun */
                    if (device->nodeName == *wwn && device->lun == lun)
                    {
                        /* Set Path for this port */
                        device->pLid[port] = lid;

                        /*
                         * Check if this device is currently assigned to
                         * another port and is offline on the other port.
                         */
                        if (device->port != port &&
                            ((device->port >= MAX_PORTS) ||
                             (device->port < MAX_PORTS && device->pLid[device->port] == NO_CONNECT) ||
                             (device->flags & (1 << DV_OFFLINE)) != 0))
                        {
                            if (device->port >= MAX_PORTS)
                            {
                                fprintf(stderr, "<INVDEVPORT> F_find_dev-invalid port %d device structure\n", device->port);
                            }

                            /*
                             * This is an important hack because F_moveDevice
                             * will not detach a device properly and decrement
                             * its count when its on a P_chn_ind[i]->devList
                             * but its port is not valid. So set its port to
                             * this list we just found it on so everything works.
                             */
                            device->port = i;

                            /* Move device to this port */
                            F_moveDevice(port, device);
                        }

                        if (device->port == port)
                        {
                            /* Update LID in DEV and PDD record. */
                            device->lid = lid;
                            if (device->pdd != NULL)
                            {
                                device->pdd->id = lid;
                            }

                            /*
                             * Indicate the device is no longer offline,
                             * clear the wait time, and clear the port
                             * unavailable flags.
                             */
                            device->flags = 0;
                        }

                        /*
                         * Catch the case where detach port
                         * set an abnormally high wait count.
                         * Otherwise, don't touch the wait count
                         * (set by recovery actions).
                         */
                        if (device->wait > 10000 / QUANTUM)     /* 10s */
                        {
                            device->wait = 250 / QUANTUM;       /* 250ms */
                        }
                        device->unavail = 0;

                        /* wakeup when command are pending */
                        p$wakeup(device);

#ifdef DEBUG_FLIGHTREC_FD
                        MSC_FlightRec(0x01002026, port | (lun << 8) | (lid << 16),
                                      *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */

                        /* Indicate the device was found. */
                        deviceFound = TRUE;
#if defined(MODEL_7000) || defined(MODEL_4700)
                        /* Indicate the device was found. */
                        if (device->pdd == NULL)
                        {
                            pdd = DC_AllocPDD();

                            pdd->channel = port;
                            pdd->lun = lun;
                            pdd->wwn = *wwn;
                            pdd->id = lid;
                            pdd->ses = SES_NO_BAY_ID;
                            device->pdd = pdd;  /* Link PDD to DEV */
                            pdd->pDev = device; /* Link DEV to PDD */
                        }
#endif /* MODEL_7000 || MODEL_4700 */
                        break;
                    }

                    /* Advance to next device. */
                    device = device->nDev;
                }
            }
        }

        /* Switch to next port */
        if (++i >= MAX_PORTS)
        {
            /* Wrap back to port 0 */
            i = 0;
        }

    } while (i != port && deviceFound == FALSE);

    /* Was the device found? */
    if (deviceFound == FALSE)
    {
        /* The DEVice was not found, search the PPD lists */
        pdd = f_find_pdd(wwn, lun);

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x02002026, port | (lun << 8) | (lid << 16),
                      (UINT32)pdd, pdd ? (UINT32)pdd->pDev : 0);
#endif /* DEBUG_FLIGHTREC_FD */

        if (pdd != NULL)
        {
            /* PDD was found. Update port and id. */
            pdd->channel = port;
            pdd->id = lid;

            /* Check if a device exist for this PDD */
            if (pdd->pDev == NULL)
            {
                /* Allocate DEV structure for this device */
                device = s_MallocC(sizeof(DEV) | BIT31, __FILE__, __LINE__);

                /* Store device's node WWN and link PPD and DEV together */
                device->nodeName = *wwn;        /* Store WWN                    */
                device->pdd = pdd;      /* Link PDD to DEV              */
                pdd->pDev = device;     /* Link DEV to PDD              */

#ifdef DEBUG_FLIGHTREC_FD
                frParm0 = 0x03002026;
#endif /* DEBUG_FLIGHTREC_FD */
            }
            else
            {
                /* Get device from PDD. */
                device = pdd->pDev;

#ifdef DEBUG_FLIGHTREC_FD
                frParm0 = 0x04002026;
#endif /* DEBUG_FLIGHTREC_FD */
            }
        }
        else
        {
            /*
             * No PDD was found. This is a new device,
             * build DEVice and PDD structures.
             */
            device = f_bld_dev(port, lid, wwn, lun);
#ifdef DEBUG_FLIGHTREC_FD
            frParm0 = 0x05002026;
#endif /* DEBUG_FLIGHTREC_FD */
        }

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(frParm0, port | (lun << 8) | (lid << 16),
                      *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */

        /* Initialize new DEVice */
        f_initDevice(device, port, lid, lun);

        /*
         * A new device was found, set the online delay. Delay on
         * this port to allow the port to settle; other new devices
         * may show up in a short amount of time.
         */
        if (P_chn_ind[port] != NULL)
        {
            P_chn_ind[port]->wait = ONLINE_DELAY;
        }
    }

#ifdef DEBUG_FLIGHTREC_FD
    /* Fabric - Find DEV */
    MSC_FlightRec(0x09002026, device->port | (device->lun << 8) | (device->lid << 16),
                  *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */

    return device;
}   /* End of F_find_dev */

/**
******************************************************************************
**
**  @brief      Removes all device from the specified port which have
**              have no path to the specified port.
**
**              Calls findAltPort on all devices to move or remove
**              all devices for the specified port.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void f_purgeDevices(void)
{
    UINT8       port;
    DEV        *device;
    DEV        *nextDev;
    UINT32      deviceCount;

    /* Check all ports. */
    for (port = 0; port < MAX_PORTS; ++port)
    {
        /* Check for purge up request for this port. */
        if (P_chn_ind[port] != NULL && (P_chn_ind[port]->state & REQ_PURGE))
        {
            /*
             * After devices have been discovered and assigned to each port,
             * any device that does not have a path on the port it is assigned
             * is moved or removed.
             */
            device = P_chn_ind[port]->devList;
            deviceCount = P_chn_ind[port]->devCnt;

            while (deviceCount-- > 0)
            {

                nextDev = device->nDev;

                /* Check if the path on this port is nonexistent. */
                if (device->pLid[port] == NO_CONNECT)
                {
#ifdef DEBUG_FLIGHTREC_FD
                    /* Fabric - Purge DEV */
                    MSC_FlightRec(0x7026, port | (device->lun << 8) | (device->lid << 16),
                                  *((UINT32 *)&device->pLid), (UINT32)device);
#endif /* DEBUG_FLIGHTREC_FD */

                    /*
                     * No path exist on this port, find an alternate path or
                     * remove the device if no path exists on any port
                     */
                    if (F_findAltPort(device) >= MAX_PORTS)
                    {
                        /*
                         * When a device is missing, wait before notifing online
                         * in case the device shows back up.
                         */
                        P_chn_ind[port]->wait = ONLINE_DELAY;

                        /*
                         * Indicate this device is offline. Set the wait time
                         * to prevent commands for this device from being failed
                         * in case the device comes online during this period.
                         */
                        device->flags = (1 << DV_OFFLINE);
                        device->wait = OFFLINE_DELAY;
                    }
                    else
                    {
                        /* Remove device from original port */
                        FAB_removeDevice(port, device);
                    }
                }

                /* Advance to next device. */
                device = nextDev;
            }

            /* Clear purge request. */
            P_chn_ind[port]->state &= ~REQ_PURGE;
        }
    }
}   /* End of f_purgeDevices */

/**
******************************************************************************
**
**  @brief      Notify the online module of changes in the device configuration.
**
**      Each port is scanned for actively attached devices, which are added to
**      a temp PDD list. Online is woken up via the reset wait semaphore.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void f_notifyOnline(void)
{
    DEV        *device;
    DEV        *firstDevice;
    UINT32      i;
    UINT32      j = 0;
    UINT32      count = 0;
    UINT32      size;

    /* Count the number of devices. */
    for (i = 0; i < MAX_PORTS; ++i)
    {
        /* Get the count of devices on each port. */
        if (P_chn_ind[i] != NULL)
        {
            count += P_chn_ind[i]->devCnt;
        }
    }

    /* Allocate space for the temp PDD list. */
    O_p_pdd_list = s_MallocC((4 * MAX_OPDDLIST_COUNT) + 4, __FILE__, __LINE__);

    /* Set the count of PDDs. */
    ((UINT32 *)O_p_pdd_list)[0] = count;

    /* Check if any device exist. */
    if (count > 0)
    {
        /* Calculate size of log message */
        size = 12 + count * sizeof(edvl->data.pid[0]);

        /* Allocate space for message */
        edvl = s_MallocC(size, __FILE__, __LINE__);

        /* Set log event type and device count in log message. */
        edvl->header.event = LOG_DVLIST;
        edvl->data.devcnt = count;

        for (i = 0; i < MAX_PORTS; ++i)
        {
            /* Get the count of devices on each port. */
            if (P_chn_ind[i] != NULL && P_chn_ind[i]->devList != NULL)
            {
                /* Get first device in the list. */
                device = firstDevice = P_chn_ind[i]->devList;

                do
                {
                    /* Copy this PID to log message. */
                    if (device->pdd != NULL)
                    {
                        edvl->data.pid[j] = device->pdd->pid;

                        /* Store the PDD in the temp list */
                        ((PDD **)O_p_pdd_list)[++j] = device->pdd;
                    }
                    else
                    {
                        edvl->data.pid[j] = 0xff;
                    }

                    /* Get next device in linked list. */
                    device = device->nDev;
                } while (device != firstDevice);
            }
        }

        /* Send debug log message to CCB and Release space used for message. */
        MSC_LogMessageRel(edvl, size);

        /* Indicate DEVice discovery is done, Physical is ready */
        K_ii.status |= (1 << II_PHY);
    }

    /* Clear the notify request */
    F_notifyreq = FALSE;

    /* Indicate change to DEFINE/ONLINE layer */
    TaskReadyByState(PCB_SCSI_RESET_WAIT);
}   /* End of f_notifyOnline */

/**
******************************************************************************
**
**  @brief      Determine if a request is pending.
**
**              Examines all ports for a loop up, loop down, or
**              LUN discovery request.
**
**  @param      request - Request to check
**
**  @return     TRUE if discovery is pending, otherwise FALSE.
**
******************************************************************************
**/
static UINT32 f_requestPending(UINT32 request)
{
    UINT32      port;
    UINT32      rc = FALSE;

    /* Check for Discovery Request on all ports. */
    for (port = 0; port < MAX_PORTS; ++port)
    {
        /* Is this port valid? */
        if (P_chn_ind[port] != NULL)
        {
            /* Check for Request */
            if (P_chn_ind[port]->state & request)
            {
                rc = TRUE;
                break;
            }
        }
    }
    return rc;
}   /* End of f_requestPending */

/**
******************************************************************************
**
**  @brief      f_discovery
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void f_discovery(void)
{
    UINT8       port;
    INT32       requestFlag;
    INT32       waitFlag;

    /* Wait here until Online completes processing of a prior PDD list. */
    while (O_p_pdd_list != NULL)
    {
        /* Set process to wait for signal and Exchange process */
        TaskSetMyState(PCB_ONLINE_WAIT);
        TaskSwitch();
    }

    /* Keep processing while requests are pending */
    while (f_requestPending(REQ_DISC) != FALSE || F_notifyreq != 0)
    {
#ifdef DEBUG_FLIGHTREC_FD
        /* Fabric - Discovery */
        MSC_FlightRec(0xC026, F_notifyreq, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
#endif /* DEBUG_FLIGHTREC_FD */

        for (port = 0; port < MAX_PORTS; ++port)
        {
            /* Check for loop up request for this port. */
            if (P_chn_ind[port] != NULL)
            {
                /* Check for Loop Up Request */
                if (P_chn_ind[port]->state & (REQ_DISC))
                {
                    f_lunDiscovery(port);
                }
            }
        }

        /* Do forever (while loop settle delay in effect and no request) */
        while (TRUE)
        {
            /* Initialize the wait and request flags */
            waitFlag = FALSE;
            requestFlag = FALSE;

            for (port = 0; port < MAX_PORTS; ++port)
            {
                if (P_chn_ind[port] != NULL)
                {
                    /* Check for a request. */
                    if (P_chn_ind[port]->state & ((REQ_DISC) | (REQ_DOWN)))
                    {
                        /* Indicate a request is pending. */
                        requestFlag = TRUE;
                    }

                    /* Check if we need to wait for the loop to settle. */
                    else if (P_chn_ind[port]->wait != 0)
                    {
                        /* Decrement wait count. */
                        --P_chn_ind[port]->wait;
                        waitFlag = TRUE;
                    }
                }
            }

            /* IF no request is pending check if wait is in effect. */
            if (requestFlag == FALSE && waitFlag == TRUE)
            {
                /* Wait for 125 msec */
                TaskSleepMS(125);
            }
            else
            {
                /*
                 * A request is pending or no wait is in effect,
                 * the loop can be exited.
                 */
                break;
            }
        }                       /* while (TRUE) */

        /* Is a discovery request pending? */
        if (f_requestPending(REQ_DISC) == TRUE)
        {
            continue;
        }

        /*
         * Wait for Lun Discovery to complete. Note: completion routine may
         * start a physical discovery. Task switch does not guarentee that
         * The discoveries have completed.
         */
        while (P_drvinits != 0)
        {
            /* Set process to wait for signal and Exchange process */
            TaskSetMyState(PCB_FC_READY_WAIT);
            TaskSwitch();
        }

        /* Is a request pending? */
        if (f_requestPending(REQ_UP_DOWN | REQ_DISC) == TRUE)
        {
            /* Wait for 125 msec */
            TaskSleepMS(125);
        }
        else
        {
            /* Balance the number of devices among all ports. */
            FAB_BalanceLoad();

            /* Awaken ONLINE with updated device list via the PDD list. */
            if (F_notifyreq != 0)
            {
                f_notifyOnline();
            }
        }
    }                           /* while requests pending */

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0300C026, 0, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Clear PCB */
    fDiscoveryPcb = NULL;
}   /* End of f_discovery */

/**
******************************************************************************
**
**  @brief      Fork the discovery task.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void F_startDiscovery(void)
{
    /* Wait if discovery process is being created. */
    while (fDiscoveryPcb == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    /* Check if the Discovery process is already active */
    if (fDiscoveryPcb == NULL)
    {
        CT_fork_tmp = (unsigned long)"f_discovery";
        fDiscoveryPcb = (PCB *)-1;      // Flag process is being created.
        fDiscoveryPcb = TaskCreate2(C_label_referenced_in_i960asm(f_discovery), ISP_DISC_PRI);

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0100C026, 0, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
#endif /* DEBUG_FLIGHTREC_FD */
    }

#ifdef DEBUG_FLIGHTREC_FD
    else
    {
        MSC_FlightRec(0x0200C026, 0, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
    }
#endif /* DEBUG_FLIGHTREC_FD */
}   /* End of F_startDiscovery */

/**
******************************************************************************
**
**  @brief      FAB_login
**
**  @param      port     - Port number
**  @param      nst      - pointer to name server table entry
**
**  @return     status
**
******************************************************************************
**/
static UINT32 FAB_login(UINT8 port, struct nst_t *nst)
{
    UINT32      rc;
    UINT32      lid2;

    /* Get a Loop ID to use for this device. */
    if (nst->lid < NO_LID && nst->lid > 0x80)
    {
        /* Use the Loop ID previously used by this port ID. */
        FAB_useLid(port, nst->lid);
    }
    else
    {
        /* Get a Loop ID to use for this device. */
        nst->lid = FAB_getLid(port);
    }

    do
    {
        /* Save the LID assigned to this port. */
        lid2 = nst->lid;

        /* Perform a fabric login.  */
        rc = ISP_LoginFabricPortIOCB(port, &(nst->lid), nst->portId, 0) & 0xffff;
        nst->status = rc;
        if (rc == ISP_PIU)
        {
            /*
             * ISP firmware already used a different LID for this port ID.
             * Return the original LID to the unused pool and try again
             * using the LID returned.
             */
            FAB_putLid(port, lid2);

            /*
             * Indicate the LID returned by the ISP firmware is used.
             * NOTE:  ISP_LoginFabricPortIOCB already updated nst->lid.
             */
            FAB_useLid(port, nst->lid);
        }
        else if (rc == ISP_LIU)
        {
            /*
             * This Loop ID is being used by a different Port ID.
             * Get another Loop ID for this port ID.
             */
            nst->lid = FAB_getLid(port);
        }
        else if (rc == ISP_CPE && nst->lid == ispLid[port])
        {
            /*
             * This Loop ID is the same as the ISP23xx loop ID.
             * Get another Loop ID for this port ID.
             */
            nst->lid = FAB_getLid(port);
        }
    } while (rc == ISP_PIU || rc == ISP_LIU);

    return rc;
}   /* End of FAB_login */

/**
******************************************************************************
**
**  @brief      FAB_ReValidateConnection
**              Does the device have a connection on the given port.
**
**  @param      port     - Port number
**  @param      device   - node name
**
**  @return     error
**
******************************************************************************
**/
UINT32 FAB_ReValidateConnection(UINT8 port, DEV *device)
{
    UINT32      retValue;
    UINT32      alpa;
    UINT32      handle;
    UINT32      handle2;

    handle = handle2 = device->pLid[port];

    if (P_chn_ind[port] == NULL || !BIT_TEST(ispOnline, port))
    {
        return ISP_CMDE;
    }
    if (ISP_IsFabricMode(port))
    {
        // Is the drive connected to the switch on this port
        retValue = isp_GidNN(port, device->nodeName, &alpa);

        if (retValue != GOOD)
        {
#if FABRIC_DEBUG
            fprintf(stderr, "%s:%d port %d  NodeName %016llX returns %08X handle %04X\n",
                    __func__, __LINE__, port, device->nodeName, retValue, handle);
#endif /* FABRIC_DEBUG */
            /* This should flush any io in the qlogic back to us with a logout error */
            if (handle != NO_CONNECT)
            {
                ISP_LogoutFabricPort(port, handle, 0);
            }
            return ISP_CPE;
        }

        /*
         * Ok device is logged into switch are we logged into it?
         * Lets do a PDISC and find out.
         */
        if (handle != NO_CONNECT)
        {
            retValue = ISP_PDisc(port, handle);
            if (retValue != ISP_CMDC)
            {
                /* Ok we are not logged in first logout to flush any IO out */
                ISP_LogoutFabricPort(port, handle, 0);
            }
        }

        /* OK now we are ready to do the next part
         * if we are not logged in we will be if we and we will and get the right handle
         * saved into the DEV
         */
        /*
         * Attempt to login using the handle we started with.
         * if not get the right one and call login againg to confirm.
         */
        if (handle != NO_CONNECT)
        {
            handle = handle2 = FAB_getLid(port);
        }
        do
        {
            retValue = ISP_LoginFabricPortIOCB(port, &handle, alpa, 0) & 0xffff;
            //  fprintf(stderr, "%s:%d port %d  NodeName %016llX ISP_LoginFabricPort returns %08X?? handle %04X\n",
            //     __func__, __LINE__, port, device->nodeName, retValue, handle);
            if (retValue == ISP_PIU)
            {
                /*
                 * ISP firmware already used a different handle for this port ID.
                 * Return the original handle to the unused pool and try again
                 * using the handle returned.
                 */
                FAB_putLid(port, handle2);

                /*
                 * Indicate the handle returned by the ISP firmware is used.
                 * NOTE:  ISP_LoginFabricPortIOCB already updated nst->lid.
                 */
                FAB_useLid(port, handle);
            }
            else if (retValue == ISP_LIU)
            {
                /*
                 * This handle is being used by a different alpa.
                 * Get another Loop ID for this port ID.
                 */
                handle = handle2 = FAB_getLid(port);
            }
            else if (retValue == ISP_CPE && handle == ispLid[port])
            {
                /*
                 * This handle is the same as the ISP23xx loop ID.
                 * Get another Loop ID for this port ID.
                 */
                handle = handle2 = FAB_getLid(port);
            }
        } while (retValue == ISP_PIU || retValue == ISP_LIU);

        if (retValue != ISP_CMDC)
        {
            return ISP_CPE;
        }
    }                           // end if fabric

    // Update port database and mask of the port login state from the ret val
    retValue = ISP_EnhancedGetPortDBHdl(port, handle) & 0x0000ffff;
    if (retValue == ISP_CMDC)
    {
        device->pLid[port] = handle;
        if (device->port == port)
        {
            device->lid = handle;
        }
    }
    return retValue;
}   /* End of FAB_ReValidateConnection */

/**
******************************************************************************
**
**  @brief      fab_primaryLoggedIn.
**
**  @param      port     - Port number
**  @param      nodeName - device node WWN
**
******************************************************************************
**/
static UINT32 fab_isPrimaryPort(UINT32 port, UINT64 portName)
{
    /*
     * Controllers with an even serial number use
     * the 'A' port (or even numbered ports).
     * Controllers with an odd serial number use
     * the 'B' port (or odd numbered ports).
     */
    if ((port & 1) == (K_ficb->cSerial & 1))
    {
        /* Is this the 'A' port? */
        if ((portName & 0xF) == 1)
        {
            return TRUE;
        }
    }
    else
    {
        /* Is this the 'B' port? */
        if ((portName & 0xF) != 1)
        {
            return TRUE;
        }
    }
    return FALSE;
}   /* End of fab_isPrimaryPort */

/**
******************************************************************************
**
**  @brief      fab_primaryLoggedIn.
**
**  @param      port     - Port number
**  @param      nodeName - device node WWN
**
******************************************************************************
**/
static UINT32 fab_primaryLoggedIn(UINT32 port, UINT64 nodeName)
{
    UINT32      i;
    struct nst_t *nst;

    /* Check each name server table entry. */
    for (i = 0; i < fabNameServerCount[port]; ++i)
    {
        /* Get pointer to name server table entry. */
        nst = &fabNameServerTable[port][i];

        /*
         * Check for matching node name and port name for the 'A' port
         * and if the port is logged in (has a LID).
         */
        if (nst->nodeName == nodeName && nst->lid < NO_LID &&
            fab_isPrimaryPort(port, nst->portName))
        {
            return TRUE;
        }
    }
    return FALSE;
}   /* End of fab_primaryLoggedIn */

/**
******************************************************************************
**
**  @brief      build the NST table for discovery
**
**  @return     none
**
******************************************************************************
**/
static UINT32 FAB_BuildNSTTable(UINT8 port)
{
    UINT32      rc;
    UINT32      i;
    struct nst_t *nst;

    // Let things settle.
    TaskSleepMS(1000);

    /* Issue GNN_FT to get list of port ID and node names. */
    rc = isp_gnnFt(port);

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0xE026, port, rc, fabNameServerCount[port]);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Check if the operation failed and another event is not pending. */
    if (rc != GOOD && (P_chn_ind[port]->state & REQ_DOWN) == 0)
    {
        /* Retry the GNN_FT */
        rc = isp_gnnFt(port);

        /* Check if the operation failed and another event is not pending. */
        if (rc != GOOD && ((P_chn_ind[port]->state & REQ_DOWN) == 0))
        {
            /*
             * Do not issue another LIP if a LIP was performed
             * in the last 15 seconds.
             */
            if (ispLastLIP[port] + 15 > timestamp)
            {
                /* Initiate a LIP. Store current time as time of last LIP. */
                ISP_initiateLip(port);
                ispLastLIP[port] = timestamp;
            }
        }
    }                           /* GNN_FT failed & no pending events */

    /*
     * Check if the previous operation was successful and if
     * a name server table was allocated.
     */
    if (rc == GOOD && fabNameServerTable[port] != NULL)
    {
        /* Issue GPN_ID to get port Name for each device. */
        for (i = 0; i < fabNameServerCount[port]; ++i)
        {
            /* Check if a RSCN or loop down event occurred. */
            if (P_chn_ind[port]->state & REQ_RSCN_DOWN)
            {
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE126, port, 0, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
                rc = ERROR;
                break;
            }

            /* Get pointer to name server table entry. */
            nst = &fabNameServerTable[port][i];

            /* Check if this is not another Magnitude 3D back end port. */
            if ((nst->nodeName & 0xFFFF00F0F0LL) != 0xB2D000A020LL)
            {
                /* Get the port name for this port ID */
                rc = isp_gpnId(port, nst->portId, &nst->portName);

#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE226, port, rc, nst->portId);
#endif /* DEBUG_FLIGHTREC_FD */
                if (rc == GOOD)
                {
                    nst->status = ST_GPN_OK;
                }
                else if ((P_chn_ind[port]->state & REQ_RSCN_DOWN) == 0)
                {
                    /* Retry the GPN_ID command. */
                    rc = isp_gpnId(port, nst->portId, &nst->portName);

                    if (rc == GOOD)
                    {
                        nst->status = ST_GPN_OK;
                    }
                    else
                    {
                        nst->status = 0x4044;
                    }
                }
                else
                {
                    nst->status = 0x4004;
                }
            }
            else
            {
                nst->status = 0x5555;
            }
        }                       /* GPN_ID loop */

        /* Login into the primary port each device. */
        for (i = 0; i < fabNameServerCount[port]; ++i)
        {
            /* Check if a RSCN or loop down event occurred. */
            if (P_chn_ind[port]->state & REQ_RSCN_DOWN)
            {
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE126, port, 1, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
                rc = ERROR;
                break;
            }

            /* Get pointer to name server table entry. */
            nst = &fabNameServerTable[port][i];

#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0xE326, port, rc, nst->portId);
#endif /* DEBUG_FLIGHTREC_FD */
            /* Login to the primary port of the drive. */
            if (nst->status == ST_GPN_OK &&
                fab_isPrimaryPort(port, nst->portName) != FALSE)
            {
                /* Log in this device. */
                rc = FAB_login(port, nst);

#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE426, port, rc, nst->portId);
#endif /* DEBUG_FLIGHTREC_FD */
            }
            else
            {
                /* Invalidate the LID for this device in the table. */
                nst->lid = NO_LID;
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE526, port, rc, nst->portId);
#endif /* DEBUG_FLIGHTREC_FD */
            }
        }                       /* Primary Port Login loop */

        /* If necessary, login into the secondary port each device. */
        for (i = 0; i < fabNameServerCount[port]; ++i)
        {
            /* Check if a RSCN or loop down event occurred. */
            if (P_chn_ind[port]->state & REQ_RSCN_DOWN)
            {
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0xE126, port, 1, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
                rc = ERROR;
                break;
            }

            /* Get pointer to name server table entry. */
            nst = &fabNameServerTable[port][i];

            /* Login to the secondary port of the drive. */
            if (nst->status == ST_GPN_OK &&
                fab_isPrimaryPort(port, nst->portName) == FALSE)
            {
                /*
                 * Check if this device is already logged in
                 * on the primary port.
                 */
                if (fab_primaryLoggedIn(port, nst->nodeName) == FALSE)
                {
                    /* Log in this device. */
                    rc = FAB_login(port, nst);
                }
            }
        }                       /* Secondary Port Login Loop */
    }                           /* GNN_FT succeeded & name server table exists */

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0xE926, port, P_chn_ind[port]->state, fabNameServerCount[port]);
#endif /* DEBUG_FLIGHTREC_FD */
    return rc;
}   /* End of FAB_BuildNSTTable */

#if defined(MODEL_7000) || defined(MODEL_4700)

/**
******************************************************************************
**
**  @brief  discover lid
**
**  @param  port - port number
**  @param  lid - lid (ALPA)
**
**  @return none
**
******************************************************************************
**/
static void discover_lid(UINT32 port, UINT32 lid)
{
    UINT32      rc;
    PDB        *pPortDb;

    /* Update port database for this device LID */
    rc = ISP_GetPortDB(port, lid, 0);

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0400A026, port, lid, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
        return;
    }

    /*
     * Check for Good completion. Retry if necessary.
     * Request will fail if we're not logged-in to the LID at the moment.
     */
    if (rc != ISP_CMDC && rc != ISP_CPE)
    {
        rc = ISP_GetPortDB(port, lid, 0);
    }

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0400A026, port, lid, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
        return;
    }

    /* Set a pointer to the database entry for this LID. */
    pPortDb = portdb[port] + lid;

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0500A026, pPortDb->sst, pPortDb->mst, pPortDb->prliw3);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Check if the port is logged in by examining the master state. */
    if (rc != ISP_CMDC || pPortDb->mst != 6)
    {
        return;
    }

    /* Check for a non-XIOtech initiator on the back end. */
    if ((pPortDb->prliw3 & 0x20) != 0 &&
        isp$check_initiator(port, lid, &pPortDb->ndn) == 0)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0800A026, rc,
                      ((UINT32 *)&pPortDb->ndn)[0],
                      ((UINT32 *)&pPortDb->ndn)[1]);
#endif /* DEBUG_FLIGHTREC_FD */
        return;
    }

    /* Check if this device has target capability. */
    if ((pPortDb->prliw3 & 0x10) == 0)
    {
        return;
    }

    int         ses;
    int         lun;
    int         lun_count;
    DEV        *device;
    DEV        *device0;
    UINT32      iseip1;
    UINT32      iseip2;
    UINT8       luns[MAX_ISE_LUNS];
    LOG_ISE_IP_DISCOVER_PKT eldn;
    {
        /*
         * Get a device pointer for LUN 0, then issue
         * REPORT LUNS. Accept a maximum of 16 LUNs
         * in the reply.
         */
        device0 = F_find_dev(port, &pPortDb->ndn, lid, 0);
        if (!device0)
        {
            return;
        }

        lun_count = get_device_luns(device0, &luns[0], MAX_ISE_LUNS);

        ses = SES_NO_BAY_ID;

        rc = ISE_GetPage85(device0, &iseip1, &iseip2, MAX_PAGE85_RETRIES);
        if (rc != EC_OK)
        {
            fprintf(stderr, "%s: ISE_GetPage85 failed with %d\n", __func__, rc);
        }
        else if (iseip1 == 0 && iseip2 == 0)
        {
            fprintf(stderr, "%s: ISE has no IP addresses!\n", __func__);
        }
        else
        {
#define BAY_ID(ip) ((((ip) & 0xFF) >> 1) % MAX_ISE)
            /*
             * Derive ses from lowest octet of IP address.
             * IP addresses are allocated in sequential pairs,
             * so shift >> 1, and take care to avoid zero,
             * which is not a valid ses/bay number.
             *
             * slot is simply the LUN.
             */

            if (!(iseip1 & 0xFF))
            {
                ses = BAY_ID(iseip2);
            }
            else if (!(iseip2 & 0xFF))
            {
                ses = BAY_ID(iseip1);
            }
            else if ((iseip1 & 0xFF) < (iseip2 & 0xFF))
            {
                ses = BAY_ID(iseip1);
            }
            else
            {
                ses = BAY_ID(iseip2);
            }
        }
        /*
         * Preserve the ses id it has if it already has one
         * new pdds are inited to SES_NO_BAY_ID
         * old ones should have something valid
         */
        if (ses == SES_NO_BAY_ID && device0->pdd && device0->pdd->ses != SES_NO_BAY_ID)
        {
            ses = device0->pdd->ses;
        }

        /* Log the discovered ip address if there are any. */
        eldn.header.event = LOG_ISE_IP_DISCOVER;
        eldn.data.ip1 = iseip1;
        eldn.data.ip2 = iseip2;
        eldn.data.bayid = ses;
        eldn.data.wwn = device0->nodeName;

        /* Note: message is short, and L$send_packet copies into the MRP. */
        MSC_LogMessageStack(&eldn, sizeof(LOG_ISE_IP_DISCOVER_PKT));

        fprintf(stderr, "%s: ISE ses %d, WWN %016llX, IP %d.%d.%d.%d, %d.%d.%d.%d\n",
                __func__, ses, device0->nodeName,
                (iseip1 >> 24) & 0xFF, (iseip1 >> 16) & 0xFF, (iseip1 >> 8) & 0xFF,
                iseip1 & 0xFF,
                (iseip2 >> 24) & 0xFF, (iseip2 >> 16) & 0xFF, (iseip2 >> 8) & 0xFF,
                iseip2 & 0xFF);

        device0->dvPort[port] = f_ISEgetPortNumber(port);

        if (device0->pdd)
        {
            device0->pdd->ses = ses;
            device0->pdd->slot = 0xFF;
        }

        /*
         * Cycle through the LUNs returned.
         *
         * Note special case for LUN 0.
         */
        for (lun = 0; lun < lun_count; lun++)
        {
            if (luns[lun] == 0)
            {
                device = device0;       /* If doing 0, use what we already have */
            }
            else
            {
                /*
                 *  If this is a new device (not known on any port),
                 *  build DEVice record.
                 */
                device = F_find_dev(port, &pPortDb->ndn, lid, luns[lun]);
            }

            if (!device)
            {
                fprintf(stderr, "%s: No device found for port %d, lid %04x, lun %d\n",
                        __func__, port, lid, luns[lun]);
                continue;
            }

            if (device->pdd)
            {
                device->pdd->ses = ses;
                device->pdd->slot = luns[lun];
            }

            /*
             * Indicate whether this connection is to the 'a' port
             * or 'b' port of the drive.
             */
            device->dvPort[port] = f_ISEgetPortNumber(port);
            if (device->dvPort[port] == 1)
            {
                /* Store port ID for port 'a' */
                device->portId[0] = pPortDb->pid;
            }
            else
            {
                /* Store port ID for port 'b' */
                device->portId[1] = pPortDb->pid;
            }

            /* wakeup when command are pending */
            p$wakeup(device);
        }
    }
}   /* End of discover_lid */

#else  /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
** This functionality should be folded into discover_lid()
** above for non-7000 platforms.
**
******************************************************************************
**/
static DEV *stuff(UINT32 port, UINT32 lid)
{
    UINT32      rc;
    DEV        *device = NULL;
    PDB        *pPortDb;

    /* Update port database for this device LID */
    rc = ISP_GetPortDB(port, lid, 0);

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0400A026, port, lid, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
        return device;
    }

    /*
     * Check for Good completion. Retry if necessary.
     * Request will fail if we're not logged-in to the LID at the moment.
     */
    if (rc != ISP_CMDC && rc != ISP_CPE)
    {
        rc = ISP_GetPortDB(port, lid, 0);
    }

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0400A026, port, lid, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
        return device;
    }

    /* Set a pointer to the database entry for this LID. */
    pPortDb = portdb[port] + lid;

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0500A026, pPortDb->sst, pPortDb->mst, pPortDb->prliw3);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Check if the port is logged in by examining the master state. */
    if (rc == ISP_CMDC && pPortDb->mst == 6)
    {
        /* Check for a non-XIOtech initiator on the back end. */
        if ((pPortDb->prliw3 & 0x20) != 0 &&
            isp$check_initiator(port, lid, &pPortDb->ndn) == 0)
        {
#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0x0800A026, rc,
                          ((UINT32 *)&pPortDb->ndn)[0],
                          ((UINT32 *)&pPortDb->ndn)[1]);
#endif /* DEBUG_FLIGHTREC_FD */
        }

        /* Check if this device has target capability. */
        else if (pPortDb->prliw3 & 0x10)
        {
            /*
             *  If this is a new device (not known on any port),
             *  build DEVice record.
             */
            device = F_find_dev(port, &pPortDb->ndn, lid, 0);
            if (device != NULL)
            {
                /*
                 * Indicate whether this connection is to the 'a' port
                 * or 'b' port of the drive.
                 */
                device->dvPort[port] = f_getPortNumber(pPortDb);

                if (device->dvPort[port] == 1)
                {
                    /* Store port ID for port 'a' */
                    device->portId[0] = pPortDb->pid;
                }
                else
                {
                    /* Store port ID for port 'b' */
                    device->portId[1] = pPortDb->pid;
                }
            }
        }
    }
    return device;
}   /* End of stuff */

#endif /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
**  @brief  discover_loop_devices
**
**  @param  port - Port number
**  @param  deviceCount - Number of devices in position map
**
**  @return none
**
******************************************************************************
**/
static void discover_loop_devices(UINT8 port, UINT8 deviceCount)
{
    UINT32      i;
    typeof(P_chn_ind[0]->state) state;

    /* Loop through AL_PAs */
    for (i = 1; i <= deviceCount; ++i)
    {
        UINT32      lid;
        UINT32      rc = ISP_CMDC;
        UINT8       tries;
        PDB        *pPortDb;

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0300A026, port, i, deviceCount);
#endif /* DEBUG_FLIGHTREC_FD */
        /*
         * Check if another loop up request occurred on this port.
         * If so, restart discovery.
         */
        state = P_chn_ind[port]->state;
        if (state & REQ_UP_DOWN)
        {
            goto rediscover;
        }

        /*
         * Check if this entry in the loop map is
         * either the Qlogic adapter or the switch.
         * Note: the switch always is AL_PA 0.
         */
        if (lpmap[port][i] == (UINT8)portid[port] || lpmap[port][i] == 0)
        {
            continue;
        }

        /*
         * Login & update port database for this device.
         * Retry once if unsuccessful.
         */
        for (tries = 0; tries < 2; ++tries)
        {
            /* Convert AL_PA to Loop ID */
            lid = isp_alpa2handle(port, lpmap[port][i], 0);
            if (lid == NO_LID)
            {
                lid = FAB_getLid(port);
            }

            /*
             * Check if another loop up/down request occurred
             * on this port. If so, restart discovery.
             */
            state = P_chn_ind[port]->state;
            if (state & REQ_UP_DOWN)
            {
                goto rediscover;
            }

            /*
             * Login & update port database for this device LID.
             * Retry once if unsuccessful.
             */
            rc = ISP_EnhancedGetPortDB(port, lid, lpmap[port][i]) & 0xFFFF;

            /*
             * Check if another loop up/down request occurred
             * on this port. If so, restart discovery.
             */
            state = P_chn_ind[port]->state;
            if (state & REQ_UP_DOWN)
            {
                goto rediscover;
            }

            /* Check for Good completion - retry if necessary. */
            if (rc == ISP_CMDC)
            {
                break;
            }
        }

        /* Check for NOT Good completion. */
        if (rc != ISP_CMDC)
        {
            continue;
        }

        /* Set a pointer to the database entry for this LID. */
        pPortDb = portdb[port] + lid;

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0500A026, pPortDb->sst, pPortDb->mst, pPortDb->prliw3);
#endif /* DEBUG_FLIGHTREC_FD */

        /* Check if the port is logged in by examining the master state. */
        if (pPortDb->mst != 6)
        {
            continue;
        }

        /* Check for a non-XIOtech initiator on the back end. */
        if (pPortDb->prliw3 & 0x20)
        {
            if (isp$check_initiator(port, lid, &pPortDb->ndn) == 0)
            {
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0x0800A026, rc,
                              ((UINT32 *)&pPortDb->ndn)[0],
                              ((UINT32 *)&pPortDb->ndn)[1]);
#endif /* DEBUG_FLIGHTREC_FD */
                continue;
            }
        }

        /* Check if this device has target capability. */
        if (pPortDb->prliw3 & 0x10)
        {
            DEV        *device;

            /*
             * If this is a new device (not known on any port),
             * build DEVice record.
             */
            device = F_find_dev(port, &pPortDb->ndn, lid, 0);
            if (device != NULL)
            {
                /*
                 * Indicate whether this connection is
                 * to the 'a' port or 'b' port of the drive.
                 * Save the index into the loop map.
                 */
#if defined(MODEL_7000) || defined(MODEL_4700)
                device->dvPort[port] = f_ISEgetPortNumber(port);
#else  /* MODEL_7000 || MODEL_4700 */
                device->dvPort[port] = f_getPortNumber(pPortDb);
#endif /* MODEL_7000 || MODEL_4700 */
                if (device->pdd != NULL)
                {
                    device->pdd->loopMap = i;
                }
            }
        }
    }
    return;

  rediscover:
#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0400A026, port, 0, state);
#endif /* DEBUG_FLIGHTREC_FD */
}   /* End of discover_loop_devices */

/**
******************************************************************************
**
**  @brief  discover_fabric_devices
**
**  @param  port - port number
**
******************************************************************************
**/

static void discover_fabric_devices(UINT8 port)
{
    UINT32      i;

    /* If fabric mode, discover fabric devices. */
    if (ISP_IsFabricMode(port))
    {
        /* Discover Fabric Devices. */
        for (i = 0; i < fabNameServerCount[port]; ++i)
        {
            struct nst_t *nst;

            /* Get pointer to name server table entry. */
            nst = &fabNameServerTable[port][i];

            if (nst->lid < NO_LID)
            {
#if defined(MODEL_7000) || defined(MODEL_4700)
                discover_lid(port, nst->lid);
#else  /* MODEL_7000 || MODEL_4700 */
                stuff(port, nst->lid);
#endif /* MODEL_7000 || MODEL_4700 */
            }

            /*
             * Check if another loop up/down request occurred
             * on this port. If so, restart discovery.
             */
            if (P_chn_ind[port]->state & REQ_UP_DOWN)
            {
#ifdef DEBUG_FLIGHTREC_FD
                MSC_FlightRec(0x0400A026, port, 0, P_chn_ind[port]->state);
#endif /* DEBUG_FLIGHTREC_FD */
                break;
            }
        }
    }
}   /* End of discover_fabric_devices */

/**
******************************************************************************
**
**  @brief  discover_fibre_devices
**
**  @param  port - port number
**
**  @return none
**
******************************************************************************
**/
static void discover_fibre_devices(UINT8 port)
{
    UINT8       deviceCount;

    /* Get length from position map (_lpmap) */
    deviceCount = lpmap[port][0];

#ifdef DEBUG_FLIGHTREC_FD
    /* Fabric - REQ_UP */
    MSC_FlightRec(0xA026, port, 0, deviceCount);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Remove the path from this port to all attached devices. */
    if (!ISP_IsFabricMode(port))
    {
        f_detachPort(port);
        discover_loop_devices(port, deviceCount);
    }
    else
    {
        discover_fabric_devices(port);
    }
}   /* End of discover_fibre_devices */


/**
******************************************************************************
**
**  @brief  process_loop_down_request
**
**  @param  port - Port to monitor
**
**  @return none
**
******************************************************************************
**/
static void process_loop_down_request(UINT8 port)
{
#ifdef DEBUG_FLIGHTREC_FD
    /* Fabric - REQ_DOWN */
    MSC_FlightRec(0xB026, port, 0, 0);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Indicate no devices are connected. */
    fabNameServerCount[port] = 0;

    /* Return all Loop IDs to the free pool. */
    FAB_clearLid(port);

    /* Attempt to fail-over to an alternate port. */
    f_failover(port);

    /* Check if this was the last loop up/down request. */
    if (f_requestPending(REQ_UP_DOWN) == FALSE)
    {
        /* Balance the number of devices among all ports. */
        FAB_BalanceLoad();
    }

    /* Check if a LIP was issued. */
    if (ispLipIssued[port] != 0xFFFF)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0C00A026, port, ispLipIssued[port], 0);
#endif /* DEBUG_FLIGHTREC_FD */
        /* Issue a Marker IOCB to this port, and reset LIP Issued condition. */
        ISP_SubmitMarker(port, 2, 0, 0);
        ispLipIssued[port] = 0xFFFF;
    }
}   /* End of process_loop_down_request */

/**
******************************************************************************
**
**  @brief  process_loop_up_request
**
**  @param  port - Port to monitor
**
**  @return Returns true if to re-do this port
**
******************************************************************************
**/
static UINT8 process_loop_up_request(UINT8 port)
{
    /* Clear loop up request for this port. */
    P_chn_ind[port]->state &= ~REQ_UP;

    /* Set purge request and set Loop Up Status */
    P_chn_ind[port]->state |= REQ_PURGE | UP;

    /* Request FC-AL position map from QLogic (wait/exchange here!) */
    ISP_GetPositionMap(port);

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
        return TRUE;            /* Re-do this port */
    }

    discover_fibre_devices(port);

    /* Context switch to allow the monitor async process to run. */
    TaskSwitch();

    /*
     * Check if another loop up/down request occurred on this port.
     * If so, restart discovery.
     */
    if (P_chn_ind[port]->state & REQ_UP_DOWN)
    {
        return TRUE;            /* Re-do this port */
    }

    /* Check if this was the last loop up/down request. */
    if (f_requestPending(REQ_UP_DOWN) == FALSE)
    {
        /*
         * After devices have been discovered and assigned
         * to each port, any device that do not have a path
         * on the port it is assigned is moved or removed.
         */
        f_purgeDevices();

        /* Balance the number of devices among all ports. */
        FAB_BalanceLoad();
    }

    /* Check if a LIP was issued. */
    if (ispLipIssued[port] != 0xFFFF)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0A00A026, port, ispLipIssued[port], 0);
#endif /* DEBUG_FLIGHTREC_FD */
        /* Issue a Marker IOCB to this port, and reset LIP Issued condition. */
        ISP_SubmitMarker(port, 2, 0, 0);
        ispLipIssued[port] = 0xFFFF;
    }

    /* Set the LUN discovery request for this port. */
    P_chn_ind[port]->state |= REQ_DISC;

    /* Check if this was the last loop up request. */
    if (f_requestPending(REQ_UP) == FALSE)
    {
        /* Start the Discovery process */
        F_startDiscovery();
    }

    return FALSE;
}   /* End of process_loop_up_request */

/**
 ******************************************************************************
 **
 **  @brief  checks if the RSCN event queue is full
 **
 **  @param  port - Port
 **
 **  @return 1 full
 **  @return 0 not full
 **
 ******************************************************************************
 **/
static UINT32 FAB_IsRSCNQueueFull(UINT8 port)
{
    /* !!!!!!!!!!!!! do not task switch in this function !!!!!!!!!!!! */
    if ((rscnQueue[port].in == rscnQueue[port].out - 1) ||
        (rscnQueue[port].in == (RSCN_QUEUE_SIZE - 1) && rscnQueue[port].out == 0))
    {
        return 1;
    }
    return 0;
}   /* End of FAB_IsRSCNQueueFull */

/**
 ******************************************************************************
 **
 **  @brief gets next RSCN event
 **
 **  @param  port - Port
 **
 **  @return FC address of next event
 **
 ******************************************************************************
 **/
static UINT32 FAB_GetRSCNEvent(UINT8 port)
{
    UINT32      alpa;

    /* !!!!!!!!!!!!! do not task switch in this function !!!!!!!!!!!! */
    // If empty why are we here?
    if (rscnQueue[port].out == rscnQueue[port].in)
    {
        return NO_CONNECT;
    }

    alpa = rscnQueue[port].queue[rscnQueue[port].out];
    rscnQueue[port].out++;
    if (rscnQueue[port].out == RSCN_QUEUE_SIZE)
    {
        rscnQueue[port].out = 0;
    }
    return alpa;
}   /* End of FAB_GetRSCNEvent */

/**
 ******************************************************************************
 **
 **  @brief puts next RSCN event on queue
 **
 **  @param  port - Port
 **  @param  FC address of next event
 **
 ******************************************************************************
 **/
void FAB_InsertRSCNEvent(UINT8 port, UINT32 alpa)
{
    UINT32      i;

    /* !!!!!!!!!!!!! do not task switch in this function !!!!!!!!!!!! */

    // If not emtpy
    if (rscnQueue[port].out != rscnQueue[port].in)
    {
        // is the portid already in the queue to be processed
        // do not duplicate
        for (i = rscnQueue[port].out; i != rscnQueue[port].in;)
        {
            if (rscnQueue[port].queue[rscnQueue[port].in] == alpa)
            {
                // We found it just return and call it good.
                return;
            }
            i++;
            if (i == RSCN_QUEUE_SIZE)
            {
                i = 0;
            }
        }
    }

    // is the queue full?  This shoudn't happen with 128 entries and a max of 64 ise.
    // considering the de dupliication code above.
    if (FAB_IsRSCNQueueFull(port))
    {
        /*
         * Set loop up request for this port and make queue empty
         * we will do full discovery now.
         */
        fprintf(stderr, "%s port %d RSCN queue full going to FULL discovery \n", __func__, port);
        P_chn_ind[port]->state |= REQ_UP;
        rscnQueue[port].in = rscnQueue[port].out = 0;
    }
    else
    {
        rscnQueue[port].queue[rscnQueue[port].in] = alpa;
        rscnQueue[port].in++;
        if (rscnQueue[port].in == RSCN_QUEUE_SIZE)
        {
            rscnQueue[port].in = 0;
        }
    }
}   /* End of FAB_InsertRSCNEvent */

/**
 ******************************************************************************
 **
 **  @brief Gets a device structure if one exists that matches a FC address
 **
 **  @param port - Port
 **  @param alpa - FC address to match
 **
 **  @return device
 **
 ******************************************************************************
 **/
static DEV *FAB_GetDEVfromPortid(UINT8 port, UINT32 alpa)
{
    UINT32      indx;
    PDD        *pdd;
    UINT32      AorB;

    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gPDX.pdd[indx]) != NULL)
        {
            if (pdd->pDev != NULL)
            {
                AorB = pdd->pDev->dvPort[port] - 1;
                //  fprintf(stderr, " %s alpa in %06X dev portid 0 %06llX\n", __func__, alpa, pdd->pDev->portId[0]);
                if (pdd->pDev->portId[AorB] == alpa)
                {
                    return pdd->pDev;
                }
            }
        }
    }
    return NULL;
}   /* End of FAB_GetDEVfromPortid */

/**
 ******************************************************************************
 **
 **  @brief Gets a device structure if one exists that matches a FC Node name
 **
 **  @param alpa - FC address to match
 **
 **  @return device
 **
 ******************************************************************************
 **/
static DEV *FAB_GetDEVfromNN(UINT64 nodename)
{
    UINT32      indx;
    PDD        *pdd;

    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gPDX.pdd[indx]) != NULL)
        {
            if (pdd->pDev != NULL && pdd->pDev->nodeName == nodename)
            {
                return pdd->pDev;
            }
        }
    }
    return NULL;
}   /* End of FAB_GetDEVfromNN */

/**
 ******************************************************************************
 **
 **  @brief Disconnects all Luns and bays on an ise.
 **
 **  @param port - Port
 **  @param nodename - ISE UUID
 **  @param ClearBusy - clear the busy after disconnect
 **
 **
 ******************************************************************************
 **/
static void FAB_DisconnectISEPort(UINT8 port, UINT64 nodename)
{
    UINT32      indx;
    PDD        *pdd;

    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gPDX.pdd[indx]) != NULL)
        {
            // fprintf(stderr, " %s NN in %016llX dev NN %016llX\n", __func__, nodename, pdd->pDev->nodeName );
            if (pdd->pDev != NULL && pdd->pDev->nodeName == nodename)
            {
                //    fprintf(stderr, " %s port %d Removing path NN %016llX\n", __func__, port, nodename );
                if (pdd->pDev->port == port)
                {
                    // If we are using this path move stuff about.
                    F_findAltPort(pdd->pDev);
                }
                FAB_removeDevice(port, pdd->pDev);
            }
        }
    }

    // Don't forget the diskbays
    for (indx = 0; indx < MAX_DISK_BAYS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gEDX.pdd[indx]) != NULL)
        {
            // fprintf(stderr, " %s NN in %016llX dev NN %016llX\n", __func__, nodename, pdd->pDev->nodeName );
            if (pdd->pDev != NULL && pdd->pDev->nodeName == nodename)
            {
                // fprintf(stderr, " %s port %d Removing path bay id %d\n", __func__, port, pdd->pid );
                if (pdd->pDev->port == port)
                {
                    // If we are using this path move stuff about.
                    F_findAltPort(pdd->pDev);
                }
                FAB_removeDevice(port, pdd->pDev);
            }
        }
    }
}   /* End of FAB_DisconnectISEPort */

/**
 ******************************************************************************
 **
 **  @brief FAB_ProcessRSCN
 **
 **  @param port - port to run on
 **
 ******************************************************************************
 **/
static void FAB_ProcessRSCN(UINT8 port)
{
    UINT32      alpa;
    UINT32      swappedalpa;
    UINT64      NodeName;
    UINT32      retval;
    UINT32      i;
    UINT32      handleforlogout;
    DEV        *devfromportid;
    DEV        *devfromNN;
    struct nst_t *nst;

    fabRscnCount[port]++;       // Debug counter

    // No task switch
    // fprintf(stderr, " %s port %d Enter in %d out %d\n", __func__, port, rscnQueue[port].out, rscnQueue[port].in);
    if (rscnQueue[port].out == rscnQueue[port].in)
    {
        // Clear RSCN request for this port. and exit
        P_chn_ind[port]->state &= ~REQ_RSCN;
        return;
    }

    // I can task switch now
    alpa = FAB_GetRSCNEvent(port);

    if ((alpa & 0xFF00) != 0 || alpa == 0)
    {
        // Its an RSCN for more than just a single device do full discovery
        P_chn_ind[port]->state |= REQ_UP;
        P_chn_ind[port]->state &= ~REQ_RSCN;
        rscnQueue[port].out = rscnQueue[port].in = 0;
        fprintf(stderr, "%s port %d Whole Fabric RSCN Do Full Discovery\n", __func__, port);
        return;
    }

    // First get the device if one exists
    devfromportid = FAB_GetDEVfromPortid(port, alpa);

    swappedalpa = (alpa >> 16) | (alpa << 16);
    retval = isp_gnnId(port, swappedalpa, &NodeName);
    // fprintf(stderr, " %s port %d alpa %06X retval %08X\n", __func__, port, alpa, retval);
    if (retval != GOOD && devfromportid == NULL)
    {
        // It is a device that has been removed and we don't have
        // a dev structure for it, do nothing
        fprintf(stderr, "%s port %d Exit isp_gnnId != GOOD && dev == NULL \n", __func__, port);
        return;
    }

    // we have a device but nothing on the fabric it must be gone
    if (retval != GOOD)
    {
        // this order is important we need to get busy set through disconnect
        // before we flush the IO generating a bunch of 0x29 (logged out) errors
        fprintf(stderr, "%s Removing port %d from ISE %016llX handle %04X\n", __func__, port, devfromportid->nodeName, devfromportid->pLid[port]);
        // FAB_DisconnectISEPort will set plid to NO_CONNECT so we need to cache the handle for the logout.
        handleforlogout = devfromportid->pLid[port];
        FAB_DisconnectISEPort(port, devfromportid->nodeName);
        // Flush IO from qlogic
        ISP_LogoutFabricPort(port, handleforlogout, 0);
        return;
    }
    if ((NodeName & 0xFFFF00F0F0LL) == 0xB2D000A020LL)
    {
        fprintf(stderr, "%s Another controller ignoring \n", __func__);
        return;
    }

    // The device has appeared look for it by NN
    devfromNN = FAB_GetDEVfromNN(NodeName);
    /* fprintf(stderr, " %s port %d devfromportid %p devfromNN %p  RSCN PortID %06X\n", __func__, port, devfromportid, devfromNN, alpa);
       fprintf(stderr, " %s GETNN: %016llX \n", __func__, NodeName);
       if (devfromportid != NULL)
       {
           fprintf(stderr, " %s devfromportid NN: %016llX devfromNN->portid %06X\n", __func__, devfromportid->nodeName, devfromNN->pLid[port]);
       }
     */
    nst = NULL;
    for (i = 0; i < fabNameServerCount[port]; ++i)
    {
        if (NodeName == fabNameServerTable[port][i].nodeName)
        {
            nst = &fabNameServerTable[port][i];
            // fprintf(stderr, " %s found nst %p\n", __func__, nst);
            break;
        }
    }

    // a completely new device
    if ((devfromportid == NULL && devfromNN == NULL) || nst == NULL)
    {
        fprintf(stderr, "%s Completely new dev Port: %d NN:%016llX\n", __func__, port, NodeName);
        // A new device has appereared and is not on the controller
        // Initiate full discovery. // Clear RSCN queue.
        P_chn_ind[port]->state |= REQ_UP;
        P_chn_ind[port]->state &= ~REQ_RSCN;
        rscnQueue[port].out = rscnQueue[port].in = 0;
        return;
    }

    if (devfromNN && devfromNN->pLid[port] == NO_CONNECT)
    {
        fprintf(stderr, "%s  NN:%016llX connected rebuilding paths port %d\n", __func__, NodeName, port);
        FAB_login(port, nst);
        // Discover lid will get it attached to a port if coming from 0 I think SMW
#if defined(MODEL_7000) || defined(MODEL_4700)
        discover_lid(port, nst->lid);
#else  /* MODEL_7000 || MODEL_4700 */
        stuff(port, nst->lid);
#endif /* MODEL_7000 || MODEL_4700 */
        return;
    }

    fprintf(stderr, "%s  NN:%016llX RSCN on good port %d starting full discovery\n", __func__, NodeName, port);
    P_chn_ind[port]->state |= REQ_UP;
    P_chn_ind[port]->state &= ~REQ_RSCN;
    rscnQueue[port].out = rscnQueue[port].in = 0;
}   /* End of FAB_ProcessRSCN */

/**
 ******************************************************************************
 **
 **  @brief      f_lookforpsdinnstanddisconnect
 **             searchs NST table for a pdd if it is not there disconnects it
 **             from port
 **
 **  @param      port - Port to to process
 **  @param      pdd  - pdd to to process
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void f_lookforpsdinnstanddisconnect(UINT32 port, PDD *pdd)
{
    UINT32      i;
    UINT32      handleforlogout;
    DEV        *dev;
    struct nst_t *nst;

    dev = pdd->pDev;
    for (i = 0; i < fabNameServerCount[port]; ++i)
    {
        nst = &fabNameServerTable[port][i];
        if (nst->lid < NO_LID && nst->nodeName == dev->nodeName)
        {
            // We have a connection return
            return;
        }
    }

    // not in nst but we think we are connected take care of it
    if (dev->pLid[port] != NO_CONNECT)
    {
        handleforlogout = dev->pLid[port];
        if (dev->port == port)
        {
            // If we are using this path move stuff about.
            F_findAltPort(dev);
        }
        FAB_removeDevice(port, dev);
        // Flush IO from qlogic
        ISP_LogoutFabricPort(port, handleforlogout, 0);
    }
}   /* End of f_lookforpsdinnstanddisconnect */

/**
 ******************************************************************************
 **
 **  @brief     FAB_ProcessNSTForMissingDevs
 **             finds all devs that are no longer connected on a port and pdates
 **             them
 **
 **  @param      port - Port to to process
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void FAB_ProcessNSTForMissingDevs(UINT8 port)
{
    UINT32      indx;
    PDD        *pdd;

    // Get starting device and count of devices
    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gPDX.pdd[indx]) == NULL || pdd->pDev == NULL)
        {
            continue;
        }
        f_lookforpsdinnstanddisconnect(port, pdd);
    }

    for (indx = 0; indx < MAX_DISK_BAYS; indx++)
    {
        // Check if the PDD exists
        if ((pdd = gEDX.pdd[indx]) == NULL || pdd->pDev == NULL)
        {
            continue;
        }
        f_lookforpsdinnstanddisconnect(port, pdd);
    }
}   /* End of FAB_ProcessNSTForMissingDevs */

/**
******************************************************************************
**
**  @brief      f_portMonitor
**
**  @param      p1 - Not used
**  @param      p2 - Not used
**  @param      port - Port to monitor
**
**  @return     none
**
******************************************************************************
**/
void f_portMonitor(UINT32 p1 UNUSED, UINT32 p2 UNUSED, UINT8 port)
{
    UINT8       next_port;

    fprintf(stderr, "%s Starting\n", __func__);

    for (; f_requestPending(REQ_UP_DOWN_RSCN); port = next_port)
    {
        next_port = port + 1;
        if (next_port >= MAX_PORTS)
        {
            next_port = 0;
        }

        /* Check if this port is valid. */
        if (P_chn_ind[port] == NULL)
        {
            continue;
        }

        /*
         * Check for RSCN Request
         * Hold off handling the RSCN Request until after any outstanding
         * Loop Down Request has been handled first.
         */
        while ((P_chn_ind[port]->state & REQ_RSCN) &&
               !(P_chn_ind[port]->state & REQ_DOWN) && !(P_chn_ind[port]->state & REQ_UP))
        {
            FAB_ProcessRSCN(port);
            continue;
        }

        /* Check for loop up and loop down requests. */
        if (P_chn_ind[port]->state & REQ_DOWN)  /* Loop down request? */
        {
            process_loop_down_request(port);
        }
        else if (P_chn_ind[port]->state & REQ_UP)       /* Loop up request? */
        {
            // Clear out rscn event queue since we are doing the whole deal.
            P_chn_ind[port]->state &= ~REQ_RSCN;
            rscnQueue[port].out = rscnQueue[port].in = 0;

            if (ISP_IsFabricMode(port))
            {
                if (FAB_BuildNSTTable(port) == ERROR && (P_chn_ind[port]->state & REQ_RSCN))
                {
                    /*
                     * ok this check may seem weird but we are trying to avoid and infinite loop
                     * which is what would happen if we replaced the following with a continue.
                     * This catches the case where the other controller generates a RSCN stopping
                     * FAB_BuildNSTTable and leading ot no discovery on this port becasue RSCNs from
                     * controllers are NO OPs. This way a flapping port should mostly be handled
                     * with FAB_ProcessRSCN and not full discovery.
                     */
                    P_chn_ind[port]->state &= ~REQ_RSCN;
                    rscnQueue[port].out = rscnQueue[port].in = 0;
                    FAB_BuildNSTTable(port);
                }
                FAB_ProcessNSTForMissingDevs(port);
            }
            if (process_loop_up_request(port))
            {
                next_port = port;       /* Re-do this port */
            }
        }
    }

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0F00A026, 0, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
#endif /* DEBUG_FLIGHTREC_FD */

    /* Clear the Discovery PCB. */
    fPortMonitorPcb = NULL;
}   /* End of f_portMonitor */

/**
******************************************************************************
**
**  @brief      Fork the port monitor process if one is not already running.
**
**  @param      port    - Port to monitor
**
**  @return     none
**
******************************************************************************
**/
void F_startPortMonitor(UINT8 port)
{
    /* Wait if ISP online process is being created. */
    while (fPortMonitorPcb == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    /* Check if ISP online process is already active */
    if (fPortMonitorPcb == NULL)
    {
        /*
         * Spawn a new process to handle the backend online event, since
         * we'll need to wait until the PDD list has been processed.
         * This current process handles all ISP events, and will deadlock
         * if we block on Online, which is actually indirectly waiting on us.
         */
        CT_fork_tmp = (unsigned long)"f_portMonitor";
        fPortMonitorPcb = (PCB *)-1;    // Flag process is being created.
        fPortMonitorPcb = TaskCreate3(C_label_referenced_in_i960asm(f_portMonitor), ISP_PMON_PRI, port);

#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0100A026, port, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
#endif /* DEBUG_FLIGHTREC_FD */
    }

#ifdef DEBUG_FLIGHTREC_FD
    else
    {
        MSC_FlightRec(0x0200A026, port, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb);
    }
#endif /* DEBUG_FLIGHTREC_FD */
}   /* End of F_startPortMonitor */

#if defined(MODEL_7000) || defined(MODEL_4700)

/**
******************************************************************************
**
**  @brief      Determine the port number for ISE
**
**  @param      port    - Port number
**
**  @return     1 or 2
**
******************************************************************************
**/
static UINT8 f_ISEgetPortNumber(UINT32 port)
{
    if (BIT_TEST(port, 0) == TRUE)
    {
        return 2;
    }
    return 1;
}   /* End of f_ISEgetPortNumber */

#else  /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
**  @brief      Determine the port number
**
**              This function will use the WWN to determine which port
**              the device is reporting on.
**
**  @param      pPortDb    - Port data base record
**
**  @return     Port number, or 0xF
**
******************************************************************************
**/
UINT8 f_getPortNumber(PDB *pPortDb)
{
    UINT64      pdn = pPortDb->pdn;
    UINT64      oui = MASK_IEEE_48BIT(pdn);

    /*
     * Determine the type of device based upon the WWN format. At this
     * point, we only have two types. One from Seagate and the other
     * from Adaptec/Ario. A more generic method is needed at some point
     * so this code does not have to change for each new type of device.
     */
    switch (pdn & 0xF0)
    {
        case 0x20:
            /*
             * Sierra Logic, Xiotech and Curtis encode the port number
             * differently, so check for them and handle it.
             */
            if (oui == SHIFT_IEEE_48BIT(XIO_OUI))
            {
                return (pdn >> 8) & 0x03;
            }
            if (oui == SHIFT_IEEE_48BIT(CURTIS_OUI))
            {
                return (pdn & 0x03) ^ 0x03;
            }
            if (oui == SHIFT_IEEE_48BIT(SL_OUI))
            {
                if (pdn & 0x0000000000000100)
                {
                    return 1;
                }
                if (pdn & 0x0000000000000200)
                {
                    return 2;
                }
                return 0xF;
            }

            /* This default is used for all normal devices, such as Seagate disks. */
            return pdn & 0x03;

        case 0x50:
            /* This is the IEEE registered format. Ario uses this format. */
            if (MASK_IEEE_REG(pdn) == SHIFT_IEEE_REG(ARIO_OUI))
            {
                return (pdn & 0x0000000001000000) ? 2 : 1;
            }
            /* NOTE: FALL THROUGH. */
        default:
            break;
    }

    return 0xF;
}   /* End of f_getPortNumber */
#endif /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
**  @brief      Initiate the rescanning of physical devices.
**
**              The monitor loop process is started. This process
**              notifies online of the list devices attached to
**              the back end.
**
**  @param      scanType    - 0 = rescan existing devices,
**                            1 = rescan LUNS,
**                            2 = rediscover devices
**                            3 rescan existing no wait
**
**  @return     return code.e
**
******************************************************************************
**/
UINT32 F_rescanDevice(UINT32 scanType)
{
    UINT8       port;
    UINT8       firstPort = 0xFF;
    UINT32      rc = DEOK;

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0000D026, scanType, 0, 0);
#endif /* DEBUG_FLIGHTREC_FD */
    /* Check all ports. */
    for (port = 0; port < MAX_PORTS; ++port)
    {
        /* Is this port valid? */
        if (P_chn_ind[port] != NULL)
        {
            /* Remember the first port found. */
            if (firstPort == 0xFF)
            {
                firstPort = port;
            }

            /* Notify online of physical devices. */
            F_notifyreq |= (1 << port);

            /* Check the Scan Type. */
            if (scanType == RESCAN_EXISTING || scanType == RESCAN_EXISTING_NO_WAIT)
            {
                /*
                 * For a Rescan Existing request, only perform Lun Discovery
                 * Set the LUN discovery request for the port.
                 */
                P_chn_ind[port]->state |= (REQ_DISC);
            }
            else
            {
                /*
                 * If Loop Down request is not set,
                 * set the Loop Up request.
                 */
                if ((P_chn_ind[port]->state & REQ_DOWN) == 0)
                {
                    P_chn_ind[port]->state |= REQ_UP;
                }
            }
        }
    }

    /* Check if any valid ports where found. */
    if (firstPort == 0xFF)
    {
        rc = DENONXDEV;
    }

    /* Check the Scan Type. */
    else if ((scanType == RESCAN_EXISTING || scanType == RESCAN_EXISTING_NO_WAIT) && fPortMonitorPcb == NULL)
    {
        /* Start the Discovery process unless fPortMonitorPcb is already running */
        F_startDiscovery();
    }
    else
    {
        F_startPortMonitor(firstPort);
    }

    if (scanType == RESCAN_EXISTING_NO_WAIT || scanType == RESCAN_LOOP_NO_WAIT)
    {
        return DEOK;
    }

    /* Wait for rescan (f_portMonitor and f_discovery) to complete. */
    while (fPortMonitorPcb != NULL || fDiscoveryPcb != NULL)
    {
        /* Wait for Port Monitor and Discovery processes to complete. */
        TaskSleepMS(125);
    }

    /*
     * Port Monitor (f_portMonitor) and Discovery (f_discovery) processes
     * have completed.
     * Wait for processing of the temp PDD list to complete.
     */
    if (O_p_pdd_list != NULL)
    {
#ifdef DEBUG_FLIGHTREC_FD
        MSC_FlightRec(0x0200D026, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb, (UINT32)O_p_pdd_list);
#endif /* DEBUG_FLIGHTREC_FD */
        /*
         * A temp PDD list still exists.
         * Set process to wait for signal and Exchange process.
         */
        TaskSetMyState(PCB_ONLINE_WAIT);
        TaskSwitch();
    }

    /* Rescan is complete,  exit. */
#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0F00D026, (UINT32)fPortMonitorPcb, (UINT32)fDiscoveryPcb, (UINT32)O_p_pdd_list);
#endif /* DEBUG_FLIGHTREC_FD */
    return rc;
}   /* End of F_rescanDevice */

/**
******************************************************************************
**
**  @brief      Determine if the specified device is in use.
**
**              The queues of the device are examined to determined
**              if the device is in use. When the queues are empty and
**              the device is not on any device list, the device and
**              associated PDD may safely be deleted.
**
**  @param      * device    - Pointer to a device structure
**
**  @return     TRUE - device is in use, FALSE - device is not is use
**
******************************************************************************
**/
UINT32 FAB_IsDevInUse(DEV *device)
{
    UINT32      port;
    UINT32      i;
    DEV        *thisDevice;
    PDD        *pdd;
    UINT32      deviceCount;
    UINT32      retValue = FALSE;

    if (device != NULL)
    {
        /* Are there any commands on the incoming queue? */
        if (device->qCnt != 0)
        {
            /* The device incoming queue is not empty. */
            retValue = TRUE;
        }

        /* Are there any outstanding requests? */
        else if (device->orc != 0)
        {
            /* Outstanding requests exists. */
            retValue = TRUE;
        }

        /* Are there any commands on device fail queue? */
        else if (device->failQHead != NULL)
        {
            /* The device incoming queue is not empty. */
            retValue = TRUE;
        }

        else
        {
            /* Check for outstanding ILTs. */
            for (i = 0; i < MAX_TAG; ++i)
            {
                /* Check for an ILT in this slot. */
                if (device->tagIlt[i] != NULL)
                {
                    retValue = TRUE;
                    break;
                }
            }
        }

        /* Check if this device is on any list for any of the ports. */
        for (port = 0; port < MAX_PORTS && retValue == FALSE; ++port)
        {
            /* Check each port. */
            if (P_chn_ind[port] != NULL)
            {
                /*
                 * Get the head of the device list and the
                 * count of devices on the queue.
                 */
                thisDevice = P_chn_ind[port]->devList;
                deviceCount = P_chn_ind[port]->devCnt;

                /* Check each device in the list. */
                while (deviceCount-- > 0)
                {
                    /* Is this device in the list? */
                    if (device == thisDevice)
                    {
                        /* Device was found. */
                        retValue = TRUE;
                        break;
                    }

                    /* Get next device. */
                    thisDevice = thisDevice->nDev;
                }
            }
        }

        /* Check if temp PDD list exists. */
        if (retValue == FALSE && O_p_pdd_list != NULL)
        {
            /* Get the number of PPDs in the list. */
            i = ((UINT32 *)O_p_pdd_list)[0];

            /* Check every entry in the list for this device. */
            while (i != 0)
            {
                /* Get PDD from PDD array. */
                pdd = ((PDD *)((UINT32 *)O_p_pdd_list)[i--]);

                /* Check for this device. */
                if (pdd != NULL && (pdd == device->pdd || pdd->pDev == device))
                {
                    /* Device was found. */
                    retValue = TRUE;
                    break;
                }
            }
        }
    }

    return retValue;
}   /* End of FAB_IsDevInUse */

/**
******************************************************************************
**
**  @brief      Bypass a device on all available ports until non-existant.
**
**              The Loop Port Bypass (LPB) primitive will be issued to this
**              device on all available ports. The DEV record is examined
**              to determine the necessary LIDs for the existing paths.
**              A rescan is then performed to determine if the device was
**              bypassed successfully. If the device status is not
**              non-existant, then the bypass attempt is repeated up to 3
**              times.
**
**  @param      pid - Physial ID (PID) for the specified device to bypass
**
**  @return     None
**
******************************************************************************
**/
void FAB_BypassDevice(UINT32 parm1 UNUSED, UINT32 parm2 UNUSED, UINT16 pid)
{
    LOG_PORT_EVENT_PKT eldn;    /* Log event        */
    DEV        *device;         /* DEVice record    */
    PDD        *pdd;            /* PDD to bypass    */
    UINT8       port;           /* Port number      */
    UINT16      lid;            /* Loop ID          */
    UINT16      qStatus;        /* Status from LPB  */
    UINT32      retry = 1;      /* Retry Count      */

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0000E026, pid, (UINT32)P_pddindx[pid], retry);
#endif /* DEBUG_FLIGHTREC_FD */

    /*
     *  Attempt to bypass the input device.
     *  Loop until the device has been successfully bypassed.
     *  Exit if any of the following are true:
     *      - The PDD been removed
     *      - The PDD device status changed to non-existant
     *      - The device record has been deleted
     *      - 3 bypass retries have been exhausted
     */
    while (((pdd = P_pddindx[pid]) != NULL) &&
           (pdd->devStat != PD_NONX) &&
           ((device = pdd->pDev) != NULL) &&
           (retry <= 3))
    {
        /* Issue Loop Port Bypass commands to device for all available paths */
        for (port = 0; port < MAX_PORTS; ++port)
        {
            /* Get the Lid for this port */
            lid = device->pLid[port];

            /* Check to see if the device is currently attached to this port. */
            if (lid != 0xFF)
            {
                /*
                 * The device is attached to this port.
                 * Send Loop Port Bypass to valid port/LID pair.
                 */
                qStatus = ISP_LoopPortBypass(port, lid);
            }

            else
            {
                /*
                 * The device is not attached to this port.
                 * Set the reason code in log message to indicate
                 * no action taken.
                 */
                qStatus = 0xffff;
            }

            /* Log the Bypass Device attempt for this port. */
            eldn.header.event = LOG_BYPASS_DEVICE;
            eldn.data.port = port;
            eldn.data.proc = 1;
            eldn.data.reason = (retry << 16) | qStatus;
            eldn.data.count = lid;

            /* Note: message is short, and L$send_packet copies into the MRP. */
            MSC_LogMessageStack(&eldn, sizeof(LOG_PORT_EVENT_PKT));

#ifdef DEBUG_FLIGHTREC_FD
            MSC_FlightRec(0x0100E026, pid,
                          ((UINT32)port << 24) | ((UINT32)lid << 16) | ((UINT32)qStatus),
                          retry);
#endif /* DEBUG_FLIGHTREC_FD */
        }

        /*
         * Initiate a rescan of the physical devices. This will update the
         * list of devices sent to online. The next loop test will determine
         * if the bypass was successful on all paths or not. If it was
         * successful, the device status will change to non-existant or the
         * the PDD may possibly be removed.
         */
        F_rescanDevice(RESCAN_EXISTING);

        /* Increment the retry loop counter */
        retry++;
    }

#ifdef DEBUG_FLIGHTREC_FD
    MSC_FlightRec(0x0F00E026, pid, (UINT32)P_pddindx[pid], retry);
#endif /* DEBUG_FLIGHTREC_FD */
}   /* End of FAB_BypassDevice */

/**
 ******************************************************************************
 **
 **  @brief     Examines a Devices paths and attempts to recover them
 **
 **  @param      Dev - Device to check
 **
 **  @return     1  = run rescan to poke online
 **  @return     0  = no changes do not run rescan do not poke online
 **
 ******************************************************************************
 **/
static UINT32 FAB_CheckOfflinePaths(DEV *dev)
{
    UINT32      pathcount;
    UINT32      i;
    UINT32      rc;
    UINT32      alpa;

    if (dev->TimetoFail > 0 && BIT_TEST(dev->pdd->flags, PD_BEBUSY))
    {
        dev->TimetoFail--;
        if (dev->TimetoFail == 0)
        {
            BIT_SET(dev->flags, DV_OFFLINE);
            return 1;
        }
    }
    pathcount = f_doesPathExist(dev);

    // we have a path but pdd is nonx or inop time to poke online.
    // hopefully this doesn't create thrashing. Where a drive
    // maintains connectivity but gets set nonx or inop
    if (pathcount > 0 && (dev->pdd->devStat == PD_NONX || dev->pdd->devStat == PD_INOP) && dev->pdd->postStat == PD_NONX)
    {
        return 1;
    }

    // All is good
    if (pathcount >= 2)
    {
        return 0;
    }

    // Are we degraded and on the wrong port
    if (pathcount == 1 && dev->pLid[dev->port] == NO_CONNECT)
    {
        F_findAltPort(dev);
    }
    for (i = 0; i < MAX_PORTS; i++)
    {
        // First is the port vaild and online?
        if (P_chn_ind[i] == NULL || !BIT_TEST(ispOnline, i))
        {
            continue;
        }
        if (dev->pLid[i] != NO_CONNECT)
        {
            continue;
        }

        // Ask the switch if the dev is out there
        rc = isp_GidNN(i, dev->nodeName, &alpa);

        // If so tell path monitor to do a full discovery
        if (rc == GOOD)
        {
            return 2;
        }
    }                           // For max ports
    return 0;
}   /* End of FAB_CheckOfflinePaths */

/**
 ******************************************************************************
 **
 **  @brief     Monitors the FC paths of all devices
 **
 ******************************************************************************
 **/
NORETURN void FAB_PathMonitor(void)
{
    UINT32      indx;
    PDD        *pdd;
    UINT32      dorescan;
    UINT32      rc;

    fprintf(stderr, "%s Starting \n", __func__);
    while (1)
    {
        TaskSleepMS(1000);
        dorescan = 0;
        for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
        {
            // Check if the PDD exists
            if ((pdd = gPDX.pdd[indx]) != NULL && pdd->devClass != PD_UNLAB)
            {
                // Don't operate on unlabeled drives we don't care about them
                if (pdd->pDev != NULL)
                {
                    rc = FAB_CheckOfflinePaths(pdd->pDev);
                    if (rc == 1 && dorescan == 0)
                    {
                        dorescan = 1;
                    }
                    else if (rc == 2 && dorescan < 2)
                    {
                        dorescan = 2;
                    }
                }
            }
        }

        // Don't forget the diskbays
        for (indx = 0; indx < MAX_DISK_BAYS; indx++)
        {
            // Check if the PDD exists
            if ((pdd = gEDX.pdd[indx]) != NULL)
            {
                if (pdd->pDev != NULL)
                {
                    rc = FAB_CheckOfflinePaths(pdd->pDev);
                    if (rc == 1 && dorescan == 0)
                    {
                        dorescan = 1;
                    }
                    else if (rc == 2 && dorescan < 2)
                    {
                        dorescan = 2;
                    }
                }
            }
        }
        if (dorescan == 1)
        {
            fprintf(stderr, "%s Starting rescan existing \n", __func__);
            F_rescanDevice(RESCAN_EXISTING);
        }
        else if (dorescan == 2)
        {
            fprintf(stderr, "%s Starting rescan loop\n", __func__);
            F_rescanDevice(RESCAN_LOOP);
        }
    }
}   /* End of FAB_PathMonitor */

/**
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
