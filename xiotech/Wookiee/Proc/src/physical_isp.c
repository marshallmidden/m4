/* $Id: physical_isp.c 161128 2013-05-20 20:37:15Z marshall_midden $*/
/**
******************************************************************************
**
**  @file       physical_isp.c
**
**  @brief      Physical layer C functions
**
**      To provide a means of reentrantly controlling concurrent
**      physical device operations.  This version supports a Fibre Channel
**      interface via the QLogic 2400 controller.
**      This file is backend only, and is necessarily safe for 2300.
**
**  Copyright (c) 2002-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>

#include "system.h"
#include "CT_defines.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include "chn.h"
#include "ecodes.h"
#include "fabric.h"
#include "isp.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "miscbe.h"
#include "online.h"
#include "prp.h"
#include "qu.h"
#include "scsi.h"
#include "ses.h"
#include "dev.h"

#define DEBUG_MSG 0

extern UINT32   P_drvinits;
extern QU P_exec_qu;
extern CHN*     P_chn_ind[MAX_PORTS];

/* ------------------------------------------------------------------------ */
//io template for PHY_InitDrive
static PRP_TEMPLATE phy_inq_template =
{
    BTIMEOUT,
    56,
    PRP_INPUT,
    6,
    PRP_SLI_BIT | PRP_SNX_BIT | PRP_BLP_BIT | PRP_BNO_BIT,
#if defined(MODEL_7000) || defined(MODEL_4700)
    IORETRY,
#else  /* MODEL_7000 || MODEL_4700 */
    1,
#endif /* MODEL_7000 || MODEL_4700 */
    { 0x12, 0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

//io template for write-10
static PRP_TEMPLATE phy_write_10_seg =
{
    BTIMEOUT,               /* pr_timeout */
    DSKBALLOC,              /* pr_rqbytes */
    PRP_OUTPUT,             /* pr_func */
    10,                     /* pr_cbytes */
    0,                      /* pr_flags */
    IORETRY,                /* pr_retry */
    { 0x2a,                 /* pr_cmd */
      0,                    /* Flags/options for this command */
      0, 0, 0, 0,           /* LBA */
      0,                    /* reserved */
      ((DSKBALLOC/SECSIZE)>>8)&0xff, ((DSKBALLOC/SECSIZE))&0xff, /* Transfer length */
      0,                    /* Control */
      0, 0, 0, 0, 0, 0}     /* Pad to 16 bytes */
};

//io template for writesame-10
static PRP_TEMPLATE phy_writesame_10_seg =
{
    BTIMEOUT,               /* pr_timeout */
    512,                    /* pr_rqbytes */
    PRP_OUTPUT,             /* pr_func */
    10,                     /* pr_cbytes */
    0,                      /* pr_flags */
    IORETRY,                /* pr_retry */
    { 0x41,                 /* pr_cmd */
      0,                    /* Flags/options for this command */
      0, 0, 0, 0,           /* LBA */
      0,                    /* reserved */
      0, 0,                 /* Transfer length */
      0,                    /* Control */
      0, 0, 0, 0, 0, 0}     /* Pad to 16 bytes */
};

//io template for write-16
static PRP_TEMPLATE phy_write_16_seg =
{
    BTIMEOUT,               /* pr_timeout */
    DSKBALLOC,              /* pr_rqbytes */
    PRP_OUTPUT,             /* pr_func */
    10,                     /* pr_cbytes */
    0,                      /* pr_flags */
    IORETRY,                /* pr_retry */
    { 0x93,                 /* pr_cmd */
      0,                    /* Flags/options for this command */
      0,0,0,0, 0,0,0,0,     /* LBA */
      0,0,                  /* Transfer length upper */
      ((DSKBALLOC/SECSIZE)>>8)&0xff, ((DSKBALLOC/SECSIZE))&0xff, /* Transfer length lower */
      0,                    /* reserved */
      0}                    /* Control */
};

//io template for writesame-16
static PRP_TEMPLATE phy_writesame_16_seg =
{
    BTIMEOUT,               /* pr_timeout */
    512,                    /* pr_rqbytes */
    PRP_OUTPUT,             /* pr_func */
    10,                     /* pr_cbytes */
    0,                      /* pr_flags */
    IORETRY,                /* pr_retry */
    { 0x93,                 /* pr_cmd */
      0,                    /* Flags/options for this command */
      0,0,0,0, 0,0,0,0,     /* LBA */
      0,0,0,0,              /* Transfer length */
      0,                    /* reserved */
      0}                    /* Control */
};

/* ------------------------------------------------------------------------ */
UINT32 PHY_CheckForRetry(ILT * ilt, DEV * dev, PRP * prp);

void PHY_Setup4Retry(UINT32 a UNUSED, UINT32 b UNUSED, DEV * dev);
void PHY_escalate(void);
void PHY_InitDrive(UINT32 a UNUSED, UINT32 b UNUSED,PDD * pdd);
void PHY_InitDrivePart2(UINT32 status, struct ILT * ilt);
void PHY_SetMaxTags(PDD * pdd);

extern UINT32 P_que;            /* Points to the physical queue routine. */
extern void p$wakeup(DEV *pDev);
extern void CT_LC_PHY_Setup4Retry(int);
extern void CT_LC_PHY_escalate(void);
extern void CT_LC_PHY_InitDrivePart2(UINT32 status, struct ILT * ilt);

#define PHY_PRI_HI_THRESH 19
#define PHY_PRI_INIT_THRESH 13

#define    HANG_THRESHOLD 30
#define    SIXTY_MINS 3600
#if defined(MODEL_3000) || defined(MODEL_7400)
static UINT32 LastDriveBypassedTimeStamp = 0;
#endif /* MODEL_3000 || MODEL_7400 */

/*
** Recovery action.
*/
#define NO_RECOVERY_ACTION              0
#define LOGIN_RECOVERY_ACTION           1
#define TARGET_RESET_RECOVERY_ACTION    2
#define LIP_RESET_RECOVERY_ACTION       3

/*
** Recovery types.
*/
#define PORT_CHANGE_RECOVERY            1
#define LOGIN_RESET_RECOVERY            2
#define PORT_UNAVAIL_RECOVERY           3
#define LIP_RECOVERY                    4
#define BE_PROC                         1

/* ------------------------------------------------------------------------ */
ILT *generate_scsi_write(PDD *pdd, UINT64 sda, UINT32 length, PRP** prp);
ILT *generate_scsi_writesame(PDD *pdd, UINT64 sda, UINT32 length, PRP** prp);
/* ------------------------------------------------------------------------ */

/**
 ******************************************************************************
 **
 **  @brief      To provide a means of logging physical action events.
 **
 **  @param      recovery - type of recovery
 **  @param      action   - recovery action
 **  @param      pDEV     - pointer to DEV
 **
 **  @return     none
 **
 ******************************************************************************
 **/

static void PHY_LogPhyActionEvent(UINT8 recovery, UINT8 action, DEV *pDEV)
{
    LOG_PHY_ACTION_PKT     PhyActionEvent;


    /**
     ** Set event code and event length.
     */
    PhyActionEvent.header.event = LOG_PHY_ACTION;
    PhyActionEvent.header.length = sizeof(LOG_PHY_ACTION_PKT);

    /**
     ** Set up the log data packet.
     */
    PhyActionEvent.data.port = pDEV->port;
    PhyActionEvent.data.proc = BE_PROC;
    PhyActionEvent.data.action = action;
    PhyActionEvent.data.recovery = recovery;
    PhyActionEvent.data.handle = pDEV->lid;
    PhyActionEvent.data.lun = pDEV->lun;
    if (pDEV->pdd)
    {
        PhyActionEvent.data.pid = pDEV->pdd->pid;
    }
    else
    {
        PhyActionEvent.data.pid = 0xffff;
    }



    /**
     ** Send the log message It will be copied from the stack location.
     */
    MSC_LogMessageStack(&PhyActionEvent, sizeof(LOG_PORT_EVENT_PKT));
}

/**
 ******************************************************************************
 **I
 **  @brief      restarts and bumps priroty on IO on a device
 **
 **  @param      dev    - pointer to DEV
 **
 **  @return     None
 **
 ******************************************************************************
 **/
static void PHY_restartdev(DEV * dev)
{
    UINT32 prioritycount;
    ILT * ilt;

    p$wakeup(dev);
    if (!dev->blk || dev->iltQFHead == NULL)
    {
        return;
    }

    for (prioritycount = 0, ilt = dev->iltQFHead; ilt != NULL; ilt = ilt->fthd)
    {
        if (&(dev->hdaToken) != (void *)ilt)
        {
            if ((ilt->phy_io.phy_priority + 3) > MAX_PHY_PRIORITY)
            {
                ilt->phy_io.phy_priority = MAX_PHY_PRIORITY;
            }
            else
            {
                ilt->phy_io.phy_priority += 3;
            }
        }
        if ( ilt->phy_io.phy_priority  >= PHY_PRI_HI_THRESH)
        {
            prioritycount++;
        }
    }
    if (prioritycount > 0)
    {
        dev->pri = PHY_PRI_INIT_THRESH;
    }
}
/**
 ******************************************************************************
 **I
 **  @brief      Attempts to login to a given device it will try twice with a
 **             short pause between to allow the disk to get itself together
 **
 **  @param      dev    - pointer to DEV
 **
 **  @return    ISP_CMDE -  Command failed.
 **  @return    ISP_CMDC -  Login Success.
 **  @return    ISP_INVDEVPORT - no path avaiblable
 **
 ******************************************************************************
 **/
static UINT32 PHY_relogin(DEV * dev)
{
    UINT32 retval;
    UINT32 i;
    UINT32 port = dev->port;
    for (i = 0; i < 2; i++)
    {
        if (port >= MAX_PORTS)
        {
            return ISP_INVDEVPORT;
        }
        retval = FAB_ReValidateConnection(port, dev);
        if (retval == ISP_CMDC)
        {
            BIT_CLEAR(dev->unavail, port);
            return ISP_CMDC;
        }
        //one retry with a 4005 error
        if (retval != ISP_CMDE)
        {
            break;
        }
        //short sleep to see if device comes back
        TaskSleepMS(250);
    }
    BIT_SET(dev->unavail, port);
    return retval;
}

#define RECOVERY_LIPRESET_ONCE 1
#define RECOVERY_DO_TARGET_RESET 2


/**
 ******************************************************************************
 **I
 **  @brief     Attempts to find a valid path to a device that has had an error
 **             the original path will be tried first followed by an attempt on an
 **             alternate path if one is availble.  Checkforretry can give specific
 **             recovery actions with dev->recoveryflags.
 **
 **  @param      dev    - pointer to DEV
 **
 **  @return    none
 **
 ******************************************************************************
 **/

void  PHY_Setup4Retry(UINT32 a UNUSED, UINT32 b UNUSED, DEV * dev)
{
    UINT32 retval;
    UINT8  startport;
#if DEBUG_MSG
    if (dev->pdd)
        fprintf(stderr, "%s:%d starting setup4retry on DEV 0x%p port %d ses %d slot %d pid %d\n",
            __func__, __LINE__, dev, dev->port, dev->pdd->ses, dev->pdd->slot, dev->pdd->pid);
    else
        fprintf(stderr, "%s:%d starting setup4retry on DEV 0x%p NO PDD ?!\n", __func__, __LINE__, dev);
#endif
    startport = dev->port;
    retval = PHY_relogin(dev);
    if (retval == ISP_CMDC)
    {
        PHY_LogPhyActionEvent(LOGIN_RESET_RECOVERY, LOGIN_RECOVERY_ACTION, dev);
        goto Setup4RetrySuccesss;
    }
    /*
     * Check and see if discovery moved the device.  This is common
     * on cable and drive pulls.
    */
    if (startport == dev->port)
    {
        F_findAltPort(dev);
    }
    //if we still have a valid port and it is different than the one we already tried, try again.
    if (dev->port < MAX_PORTS && startport != dev->port)
    {
        retval = PHY_relogin(dev);

        if (retval == ISP_CMDC)
        {
            PHY_LogPhyActionEvent(PORT_CHANGE_RECOVERY, LOGIN_RECOVERY_ACTION, dev);
            goto Setup4RetrySuccesss;
        }
        else
        {
#if DEBUG_MSG
            fprintf(stderr, "<VIJAY>%s:%d DEV moving to org port 0x%p startport %d cur port %d pid=%d\n",
                    __func__, __LINE__, dev, startport, dev->port, (dev->pdd)?dev->pdd->pid:999);
#endif
            F_moveDevice(startport, dev);
        }

    }

    //no path to the device
    if ( dev->port < MAX_PORTS  &&  !BIT_TEST(dev->recoveryflags, RECOVERY_LIPRESET_ONCE))
    {
        /* reset the device*/
#if DEBUG_MSG
        fprintf(stderr, "%s<VIJAY>:%d startport=%d Try to  Lip Reset on port %d  for pid=%d\n",
                __func__, __LINE__, startport, dev->port, dev->pdd->pid);
#endif
        if ((ispLastLIP[startport] + 15) <= timestamp)
        {
            ispLastLIP[startport] = timestamp;
            ISP_LipReset(startport, dev->lid);
            PHY_LogPhyActionEvent(LIP_RECOVERY, LIP_RESET_RECOVERY_ACTION, dev);
#if DEBUG_MSG
            fprintf(stderr, "%s<VIJAY>:%d startport=%d Lip Reset on port %d  for pid=%d\n",
                    __func__, __LINE__, startport, dev->port, dev->pdd->pid);
#endif
            BIT_SET(dev->recoveryflags, RECOVERY_LIPRESET_ONCE);

        }
    }
    else
    {
#if DEBUG_MSG
        fprintf(stderr, "%s:%d NO working paths for DEV going offline 0x%p startport %d cur port %d pid=%d\n",
                __func__, __LINE__, dev, startport, dev->port, dev->pdd->pid);
#endif
        //setting offline makes escalate detach the current dev->port then findalt port.

        // we will wait a bit and retry if it fails again the second port will be detached and all
        // commands failed.
        BIT_SET(dev->flags, DV_OFFLINE);

    }

    dev->wait = MAX(dev->wait, 2000/QUANTUM);
    dev->setupretryactive = FALSE;
    return;

Setup4RetrySuccesss:
#if DEBUG_MSG
   fprintf(stderr, "%s:%d Good exit for DEV 0x%p startport %d cur port %d \n",
           __func__, __LINE__, dev, startport, dev->port);
#endif
    if (BIT_TEST(dev->recoveryflags, RECOVERY_DO_TARGET_RESET))
    {
#if DEBUG_MSG
        fprintf(stderr, "%s<VIJAY>:%d startport=%d targetreset on port %d  for pid=%d\n",
                __func__, __LINE__, startport, dev->port, dev->pdd->pid);
#endif
        PHY_LogPhyActionEvent(LOGIN_RESET_RECOVERY, TARGET_RESET_RECOVERY_ACTION, dev);
        ISP_TargetReset(dev->port, dev->lid);
        dev->wait = MAX(dev->wait, 250/QUANTUM);
    }
    else
    {
        dev->wait = 0;
    }
    dev->setupretryactive = FALSE;
    dev->recoveryflags = 0;
    BIT_CLEAR(dev->flags, DV_OFFLINE);
    PHY_restartdev(dev);
}
/**
 ******************************************************************************
 **
 **  @brief     starts PHY_StartSetup4Retry on the given deivce
 **
 **  @param      dev    - pointer to DEV
 **
 ******************************************************************************
 **/
static void PHY_StartSetup4Retry(DEV * dev)
{
    if (!dev->setupretryactive)
    {
        dev->setupretryactive = TRUE;
        CT_fork_tmp = (unsigned long)"PHY_StartSetup4Retry";
        TaskCreate3(C_label_referenced_in_i960asm(PHY_Setup4Retry), PHY_SETUP4RETRY_PRI, (UINT32)dev);
    }

}

/**
 ******************************************************************************
 **
 **  @brief      To provide a means detecting and bypassing failing drives
 **
 **  @param      dev    - pointer to DEV
 **
 **  @return     none
 **
 ******************************************************************************
 **/
#if defined(MODEL_3000) || defined(MODEL_7400)
static void CheckForandLogDriveDelay(DEV * dev)
{

    if (dev->pdd == NULL )
    {
        return;
    }
    /*
    *   If a 'drive delay' message has been logged in the last 60 minutes,
    *   don't log another one.
    */
    if ( LastDriveBypassedTimeStamp == 0 || ((timestamp - LastDriveBypassedTimeStamp) >= SIXTY_MINS))
    {
        /*
        * --- Increment pdd-> hangCount, if > 30, then call SES_BypassCtrl to log a
        *     ‘drive delay’ message and bypass the drive
        */
        dev->pdd->hangCount++;
        if (dev->pdd->hangCount < HANG_THRESHOLD)
        {
            return;
        }
        LastDriveBypassedTimeStamp = timestamp;
        SES_BypassCtrl(dev->pdd);
    }
    else
    {
        /*
        * --- A 'drive delay' message has been logged in the last 60 minutes.  Zero out
        *     pd_hangcnt so that the hang count can start from zero for the next 60
        *     minute cycle.
        */
        dev->pdd->hangCount = 0;
    }

}
#endif /* MODEL_3000 || MODEL_7400 */

/**
 ******************************************************************************
 **
 **  @brief      To provide a means of handling QLogic errors.
 **
 **  @param      ilt    - pointer to ILT
 **  @param      dev    - pointer to DEV
 **  @param      prp    - pointer to PRP
 **
 **  @return     retry:  TRUE if the request can be retried
 **                      FALSE if the request cannot be retried
 **
 ******************************************************************************
 **/

static UINT32 PHY_ProcessQLogicErr(ILT *ilt UNUSED, DEV *dev, PRP *prp)
{
    //device not connected no retries
    if (dev->port >= MAX_PORTS)
    {
        return FALSE;
    }
#if defined(MODEL_3000) || defined(MODEL_7400)
    if ( prp->qLogicStatus != QLOGIC_STATUS_GOOD)
    {
        CheckForandLogDriveDelay(dev);
    }
#endif /* MODEL_3000 || MODEL_7400 */
    switch (prp->qLogicStatus)
    {
        case QLOGIC_STATUS_GOOD:
        case QLOGIC_STATUS_DMAERROR:
        case QLOGIC_STATUS_OVERRUN:
            /* No retry if no QLogic error, or if DMA error */
            return FALSE;

        case QLOGIC_STATUS_TRANSPORT_ERR:
        case QLOGIC_STATUS_RESET:
        case QLOGIC_STATUS_TASKABORT:
            /* Short pause and retry */
            dev->wait = MAX(dev->wait, 500/QUANTUM);
            return TRUE;

        case QLOGIC_STATUS_QUEUEFULL:
        case QLOGIC_STATUS_ABORTED_BY_TARG:
            /* A second pause */
            dev->wait = MAX(dev->wait, 1000/QUANTUM);
            return TRUE;

        case QLOGIC_STATUS_UNDERRUN:
            if ( dev->port >= MAX_PORTS )
            {
                return FALSE;
            }
            dev->wait = MAX(dev->wait, 250/QUANTUM);
            return TRUE;

        case QLOGIC_STATUS_FW_RESOURCE_OUT:
        case QLOGIC_STATUS_TMF_OVERRUN:
            /* Take a long nap on this device and see if things settle. */
            dev->wait = MAX(dev->wait, 2000/QUANTUM);
            return TRUE;

        case QLOGIC_STATUS_PORTUNAVAIL:
        case QLOGIC_STATUS_PORTLOGGEDOUT:
        case QLOGIC_STATUS_PORTCHANGED:
            if (dev->pdd != NULL && BIT_TEST(dev->pdd->flags, PD_BEBUSY))
            {
                /* Change error code */
                prp->reqStatus = EC_BEBUSY;
                return FALSE;
            }
            /* We lost connectivity on this port resestablish if possible then retry */
            dev->wait = MAX(dev->wait, 2000/QUANTUM);
            return TRUE;

        case QLOGIC_STATUS_DATA_ASSEMB_ERR:
            /* This should not be possible. */
            fprintf(stderr, "%s:%d What the heck, DATA reassembly error? out of order data is not on \n", __func__, __LINE__);
            return FALSE;

        case QLOGIC_STATUS_TIMEOUT:
        {
            prp->timeoutCnt++;
            if (prp->timeoutCnt > 5)
            {
                prp->retry = 0;
            }
            if (prp->timeoutCnt > 3)
            {
                BIT_SET(dev->recoveryflags, RECOVERY_DO_TARGET_RESET);
            }
            if (prp->retry == 0 && !BIT_TEST(prp->flags, PRP_BNO))
            {
                dev->physErr++;
                dev->physErr &= 0x1f; //odd artificial error count limit
            }
            if ( dev->port >= MAX_PORTS )
            {
                return FALSE;
            }
            dev->wait = MAX(dev->wait, 2000/QUANTUM);
            return TRUE;
        }
        default:
            return TRUE;
    }

}

/**
 ******************************************************************************
 **I
 **  @brief      To provide a means for physical retry event.
 **
 **  @param      pPRP    - pointer to PRP
 **  @param      pDEV    - pointer to DEV
 **
 **  @return     None
 **
 ******************************************************************************
 **/

static void PHY_LogPhyRetryEvent(PRP *pPRP, DEV *pDEV)
{
    LOG_PHY_RETRY_PKT           PhyRetryEvent;



    /**
     ** Set event code and event length.
     */
    PhyRetryEvent.header.event = LOG_PHY_RETRY;
    PhyRetryEvent.header.length = sizeof(LOG_PHY_RETRY_PKT);

    /**
     ** Set up the log data packet.
     */
    PhyRetryEvent.data.prpstat = pPRP->reqStatus;
    PhyRetryEvent.data.scsistat = pPRP->scsiStatus;
    PhyRetryEvent.data.qstatus = pPRP->qLogicStatus;
    PhyRetryEvent.data.retry = pPRP->retry;
    PhyRetryEvent.data.port = pDEV->port;
    PhyRetryEvent.data.lun = pPRP->lun;
    PhyRetryEvent.data.id = pDEV->lid;
    PhyRetryEvent.data.wwn = pDEV->nodeName;
    if ( pDEV->pdd)
    {
        PhyRetryEvent.data.pid = pDEV->pdd->pid;
    }
    else
    {
        PhyRetryEvent.data.pid = 0xffff;
    }
    memcpy((void *)(PhyRetryEvent.data.cdb), (void *)pPRP->cmd,
            (sizeof(UINT8) * 16));

    /**
     ** Check if SCSI status is check condition (i.e., has sense info).
     */
    if (pPRP->scsiStatus == SCS_ECHK)
    {
        /**
         ** If SCSI status is check condition, copy sense data.
         */
        memcpy((void *)PhyRetryEvent.data.sense,
                (void *)pPRP->sense, sizeof(SNS));
    }

    /**
     ** Log event. It will be copied from the stack location.
     */
    MSC_LogMessageStack(&PhyRetryEvent, sizeof(LOG_PHY_RETRY_PKT));
}
/**
 ******************************************************************************
 **
 **  @brief checks retry count and decrements queues to error queue if retry
 **
 **
 **  @param  ilt - ILT structure
 **  @param  dev - device command was issued on
 **  @param  prp - Pointer to a PRP structure (the command)
 **
 **  @return 0 - No retry
 **  @return 1 - Yes retry
 **
 ******************************************************************************
**/
static UINT32 CheckAndQueueForRetry(ILT * ilt, DEV * dev, PRP * prp)
{
#if DEBUG_MSG
    fprintf(stderr, "%s:%d dev %08X ilt %p retry count %d ses=%d slot=%d\n",
            __func__, __LINE__, (UINT32)dev, ilt, prp->retry, dev->pdd->ses, dev->pdd->slot);
#endif
    /* Check if retry counter has expired. */
    if (prp->retry == 0 || dev->port >= MAX_PORTS)
    {
        /* No more retries. */
        return FALSE;
    }

    if (prp->func != PRP_CTL)
    {
        if (dev->pdd && dev->pdd->devStat == PD_INOP)
        {
            return FALSE;           /* Fast timeout of read/write ops when device is failed. */
        }
    }

    /* Task can be retried. Update retry counter. */
    prp->retry--;
    if  (!BIT_TEST(prp->logflags, PRP_SRL))
    {
        PHY_LogPhyRetryEvent(prp, dev);
    }
    BIT_CLEAR(prp->logflags, PRP_SRL);

    // update prp
    prp->channel = dev->port;
    prp->id = dev->lid;
    prp->qLogicStatus = EC_OK;
    prp->reqStatus = EC_OK;
    prp->scsiStatus = EC_OK;

    /* Task is to be retried. Link this task into DEV failed queue. */
    if (dev->failQTail == NULL)
    {
        dev->failQHead = ilt;
    }
    else
    {
        dev->failQTail->fthd = ilt;
    }
    dev->failQTail = ilt;
    ilt->fthd = NULL;
    return TRUE;
}
/**
 ******************************************************************************
 **
 **  @brief      To provide a means of logging smart events.
 **
 **  @param      pDEV    - pointer to DEV
 **  @param      pPRP    - pointer to PRP
 **
 **  @return     none
 **
 ******************************************************************************
 **/

static void PHY_LogSmartEvent(UINT32 event, PRP *pPRP, DEV *pDEV)
{
    LOG_SMART_EV_PKT     SmartEvent;

    /**
     ** Set event code and event length.
     */
    SmartEvent.header.event = event;
    SmartEvent.header.length = sizeof(LOG_SMART_EV_PKT);

    /**
     ** Set up the log data packet.
     */
    SmartEvent.data.port = pPRP->channel;
    if (pDEV->pdd)
    {
        SmartEvent.data.rsv  = pDEV->pdd->ses;
    }
    SmartEvent.data.lun = pPRP->lun;
    SmartEvent.data.id = pPRP->id;
    SmartEvent.data.wwn = pDEV->nodeName;
    memcpy((void *)SmartEvent.data.cdb, (void *)pPRP->cmd,
            (sizeof(UINT8) * 16));
    memcpy((void *)SmartEvent.data.sense, (void *)pPRP->sense, sizeof(SNS));

    /**
     ** Send the log message.
     */
    MSC_LogMessageStack(&SmartEvent, sizeof(LOG_SMART_EV_PKT));
}
/**
 ******************************************************************************
 **
 **  @brief      To provide a means of processing sense errors for retyr.
 **
 **  @param      pDEV    - pointer to DEV
 **  @param      pPRP    - pointer to PRP
 **
 **  @return     true   - retry
 **  @return     false  - no retry
 **
 ******************************************************************************
 **/
static UINT32 PHY_ProcessSenseError(PRP * prp, DEV * dev)
{
    UINT8 key       = prp->sense[2] & 0xf;
    UINT8 asc       = prp->sense[12];
    UINT8 ascq      = prp->sense[13];
    UINT8 frucode   = prp->sense[14];

    if (key == SCK_ILLEGAL || key == SCK_MISCOMPARE)
    {
        return FALSE;
    }

#if defined(MODEL_7000) || defined(MODEL_4700)
    /* remove or modify retry on data miscompare*/
    if (key == SCK_MEDIUM)
    {
        //any media error
        return FALSE;
    }
    if (asc == 0x04 && ascq == 0x03)
    {
        //lun inop
        return FALSE;
    }
#endif /* MODEL_7000 || MODEL_4700 */
    if (asc == 0x04 && ascq == 0x01)
    {
        dev->wait = 5000/QUANTUM;
        return true;
    }
    /**
    ** Log ASCQ 29xx with FRU code 0xCC (drive fault/reset)
    ** Log ASCQ 2904 (device internal reset)
    **/
    if (key == SCK_UNITATT && asc == 0x29)
    {
        if (frucode == 0xcc || ascq == 0x04)
        {
            PHY_LogSmartEvent(LOG_DRV_FLT, prp, dev);

        }
        else
        {
            //surpress log message for reset events
            BIT_SET(prp->logflags, PRP_SRL);
            //free retry for power on unit attention.
            prp->retry++;
        }
        return TRUE;
    }

    /**
     **   Don't log a message if a Receive Diag command returns with an
     **   asc/ascq of 0x3502.  There's too many of these in the field and
     **   it's clogging customers' logs.  It's not indicative of any bay issues.
     */

    if ((prp->cmd[0] == 0x1C) && (asc == 0x35) && (ascq == 0x02))
    {
         //surpress log message for reset events
        BIT_SET(prp->logflags, PRP_SRL);
        return TRUE;
    }

#if defined(MODEL_3000) || defined(MODEL_7400)
    if (asc == 0x5D)
    {
        PHY_LogSmartEvent(LOG_SMART_EVENT, prp, dev);
        if (prp->sense[15] == 0x82 || prp->sense[15] == 0x84)
        {
            //surpress log message for busy events
            BIT_SET(prp->logflags, PRP_SRL);

            return TRUE;
        }
    }
#endif /* MODEL_3000 || MODEL_7400 */

    // crc error of some sort detected by drive
    if (asc == 0x47)
    {
        if ( dev->port >= MAX_PORTS )
        {
            return FALSE;
        }
        //port has already been changed around
        if ( dev->port == prp->channel && prp->retry < 2)
        {
            UINT8 altPort;
            altPort = F_findAltPort(dev);
            if(altPort < MAX_PORTS)
            {
                // Alternative path to device is found on other port
                // Now check if this altport is on unavailable list..
                if(BIT_TEST(altPort, dev->unavail) == FALSE)
                {
                    // The alternate port is available..
                    dev->wait = 2000/QUANTUM;
                    return TRUE;
                }
                /*
                ** Since altport is unavailable move this device back to the original port.
                */
                if( prp->channel != 0xFF)
                {
                    F_moveDevice(prp->channel, dev);
                }
            }

        }
        dev->wait = MAX(dev->wait, 250/QUANTUM);
        return TRUE;
    }
    //if luns have changed
    if (asc == 0x3f && ascq == 0x0e)
    {
        F_rescanDevice(RESCAN_EXISTING_NO_WAIT);
        TaskSwitch();
    }
    //all specific unit attentions taken care of gen
    if (BIT_TEST(prp->flags, PRP_BCC))
    {
        return FALSE;
    }
#if defined(MODEL_7000) || defined(MODEL_4700)
    // If the ISP error processing resulted in setting the PID as BUSY, don't retry.
    if (BIT_TEST(prp->pDev->pdd->flags, PD_BEBUSY))
    {
        return FALSE;
    }
#endif /* MODEL_7000 || MODEL_4700 */
    return TRUE;
}
/**
 ******************************************************************************
 **
 **  @brief      To provide a means for processing SCSI errors.
 **
 **  @param      ilt    - pointer to ILT
 **  @param      dev    - pointer to DEV
 **  @param      prp    - pointer to PRP
 **
 **  @return     retry:  TRUE if the request can be retried
 **                      FALSE if the request cannot be retried
 **
 ******************************************************************************
 **/

static UINT32 PHY_ProcessSCSIErr(ILT * ilt UNUSED, DEV * dev, PRP * prp)
{
    switch (prp->scsiStatus)
    {
        case SCS_QUEF:
            dev->wait = MAX(dev->wait, 1000/QUANTUM);
            BIT_SET(dev->flags,  DV_QUEUEFULL);
            return TRUE;

        case SCS_ECHK:
            return PHY_ProcessSenseError(prp, dev);

#if defined(MODEL_7000) || defined(MODEL_4700)
        case SCS_BUSY:
            return FALSE;
#endif /* MODEL_7000 || MODEL_4700 */

        default:
            dev->wait = MAX(dev->wait, 2000/QUANTUM);
            return TRUE;
    }
}

/**
 ******************************************************************************
 **
 **  @brief
 **              This routine determines if a task can be retried. If the task
 **              can be retried, the retry counter is checked to determine if
 **              a retry is allowed. If a retry is allowed, the task is placed
 **              on the end of the DEV failed queue and g0 is returned with a
 **              1 to indicate a retry is being performed. If a retry is not
 **              allowed, the error condition is logged and g0 is returned to
 **              indicate the task is not being retried.
 **
 **              The following table describes the possible error and
 **              the recovery that is performed.
 **
 **     Error
 **    Code (h)              Error                Retry ?         discovery ?
 **    --------   --------------------------      -------         -----------
 **
 **    SCSI
 **       02      Check Condition
 **
 **               Key
 **               ---
 **               00 - no error                   no              no
 **               01 - recovered error            no              no
 **               02 - not ready                  yes             yes
 **               03 - medium error               yes             no
 **               05 - illegal command            no              no
 **               06 - unit attention             yes             no
 **               0B - aborted command            yes             no
 **
 **       08      Busy                            yes             no
 **       28      Task Set Full                   yes             no
 **
 **    Qlogic
 **       02      DMA                             no              no
 **       04      Reset                           yes             no
 **       05      Aborted                         no              no
 **       06      Timeout                         yes             no
 **       07      Data overrun                    no              no
 **       15      Data underrun                   yes  (alt path) no
 **       1C      Queue full                      yes             no
 **       28      Port unavailable                yes (alt path)  yes
 **       29      Port logged out                 yes (alt path)  yes
 **       2A      Port configuration changed      yes             yes
 **
 **       Note: The discovery is done before the retry is attempted.
 **
 **
 **  @param  ilt - ILT structure
 **  @param  dev - device command was issued on
 **  @param  prp - Pointer to a PRP structure (the command)
 **
 **  @return 0 - No retry
 **  @return 1 - Yes retry
 **
 ******************************************************************************
 **/
UINT32 PHY_CheckForRetry(ILT * ilt, DEV * dev, PRP * prp)
{
    UINT32      retry;
#if DEBUG_MSG
    fprintf(stderr, "%s:%d dev %08X prp %08x Sstat %04X Qstat %04X prp port %d ses=%d slot=%d prp_retry=%d devport %d \n",
             __func__, __LINE__, (UINT32)dev, (UINT32)prp, prp->scsiStatus, prp->qLogicStatus, prp->channel,
                                dev->pdd->ses, dev->pdd->slot, prp->retry, dev->port);
#endif
    /**
     ** Check SCSI status.
     */
    if (prp->scsiStatus == SCS_NORM)
    {
        /**
         ** If no SCSI error, check if there is QLogic error.
         */
        retry = PHY_ProcessQLogicErr(ilt, dev, prp);
    }
    else
    {
        /**
         ** If error in scsi status, process the error.
         */
        retry = PHY_ProcessSCSIErr(ilt, dev, prp);
    }
    if (retry)
    {
        retry = CheckAndQueueForRetry(ilt, dev, prp);
    }
    else
    {
        /* no retry clear counter*/
        prp->retry = 0;
    }
    return retry;
}
/**
 ******************************************************************************
 **
 **  @brief     Updates PDD statistics fields.
 **
 **  @param      pdd           - channel to process
 **  @param      cyclecounter  - indicates whether to update pdd stats.
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void PHY_updatepdd(DEV * dev, UINT32 cyclecounter)
{
    if (dev->pdd == NULL || cyclecounter != 8)
    {
        return;
    }
    dev->pdd->rps = dev->sprc;
    if (dev->pdd->rps)
    {
        dev->pdd->avgSC = dev->spsc / dev->pdd->rps;
        if (dev->spsc % dev->pdd->rps >= dev->pdd->rps >> 1)
        {
            dev->pdd->avgSC++;
        }
    }
    else
    {
        dev->pdd->avgSC = 0;
    }
    dev->sprc = 0;
    dev->spsc = 0;
}
/**
 ******************************************************************************
 **
 **  @brief      To provide an automatic means of checking devs on a channel
 **             devices on a channel will be process on at a time.
 **
 **  @param      chan           - channel to process
 **  @param      cyclecounter   - indicates whether to update pdd stats.
 **
 **  @return     none
 **
 ******************************************************************************
 **/
static void PHY_ecalateCheckDevs(CHN * chan, UINT32 cyclecounter)
{
    DEV * dev;
    UINT32 i;

    dev = chan->devList;
    if (chan->devCnt != 0 && dev == NULL)
    {
#if DEBUG_MSG
        fprintf(stderr, "%s:%d device list is fubar \n", __func__, __LINE__);
#endif
        return;
    }
    for (i = 0; i < chan->devCnt; i++, dev = dev->nDev )
    {
        if (dev == NULL)
        {
            return;
        }
        if (dev->setupretryactive)
        {
            PHY_updatepdd(dev, cyclecounter);
            continue;
        }
        if (dev->wait > 10000/QUANTUM)
        {
#if DEBUG_MSG
            fprintf(stderr, "%s:%d device wait is huge fixing \n", __func__, __LINE__);
#endif
            dev->wait = 250/QUANTUM;
        }
        if (dev->wait > 0)
        {
            dev->wait--;
        }
        else
        {
            PHY_restartdev(dev);
            PHY_updatepdd(dev, cyclecounter);
            continue;
        }
        if (dev->wait != 0)
        {
            PHY_updatepdd(dev, cyclecounter);
            continue;
        }
        //ok dev-> wait has been decremented to 0 lets recover or disconect
        if (BIT_TEST(dev->flags, DV_OFFLINE))
        {
            // If the available path count goes to 0 FAB_removeDevice will fail the commands
#if DEBUG_MSG
            fprintf(stderr, "%s:%d detaching device %p from port %d \n", __func__, __LINE__, dev, chan->channel);
#endif
            FAB_removeDevice(chan->channel, dev);
            F_findAltPort(dev);
        }
        else
        {
            //Set wait count to hold off I/O until after set4retry
            dev->wait = 1;
            PHY_StartSetup4Retry(dev);
            PHY_updatepdd(dev, cyclecounter);
        }

    }
}
/**
 ******************************************************************************
 **
 **  @brief      To provide an automatic means of escalating the priority
 **              of outstanding requests based upon the length of time
 **              spent within this module.  The PDD is also updated on a
 **              second cycle
 **
 **              The priority of each request queued to this module
 **              is escalated each and every QUANTUM ms period. Over
 **              the course of consecutive periods a request's priority
 **              will escalate into the next range
 **              (i.e., low to normal or normal to high etc.).
 **
 **              If this is a 1 second cycle, the PDD is updated with
 **              the average sector count and requests per second
 **              statistics.  Each device is checked to determine if a
 **              timeout has occurred.  If so, timeout processing is
 **              invoked.
 **
 **              This is a process call.
 **
 **  @param      NONE
 **
 **  @return     NONE
 **
 ******************************************************************************
 **/
NORETURN
void PHY_escalate(void)
{
    UINT32 cyclecounter;
    UINT32 i;
    ILT * ilt;
    fprintf(stderr, "%s:%d STARTING \n", __func__, __LINE__);
    cyclecounter = 0;
    while (1)
    {
        TaskSleepMS(125);
        cyclecounter++;
        //prcocess P_exec qu
        for (ilt = P_exec_qu.head; ilt != NULL; ilt = ilt->fthd)
        {
            if ((ilt->phy_io.phy_priority + 3) > MAX_PHY_PRIORITY)
            {
                ilt->phy_io.phy_priority = MAX_PHY_PRIORITY;
            }
            else
            {
                ilt->phy_io.phy_priority += 3;
            }
        }
        /* process channels*/
        for (i = 0; i < MAX_PORTS; i++)
        {
            if ( P_chn_ind[i] != NULL)
            {
                PHY_ecalateCheckDevs(P_chn_ind[i], cyclecounter);
            }
        }
        if (cyclecounter == 8)
        {
            cyclecounter = 0;
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief To provide a common means of expanding the number of concurrent
 **       queue tags that may be used for a given device.
 **       The tag map field with the DEV structure is modified to expand
 **       the number of concurrent queue tag entries that may be used.
 **       The tag mask is also modified to reflect availability of the
 **       additional queue tags.
 **
 **  @param      PDD
 **
 **  @return     NONE
 **
 ******************************************************************************
 **/
void PHY_SetMaxTags(PDD * pdd)
{
    UINT32 maxtag;
    UINT32 i;
    if (pdd->pDev == NULL)
    {
        return;
    }
    if (pdd->devType == PD_DT_SATA)
    {
        pdd->pDev->tMapMask = MAXTAGSATAMSK;
        maxtag = MAX_TAG_SATA;
    }
    else
    {
        pdd->pDev->tMapMask = MAXTAGMSK;
        maxtag = MAX_TAG;
    }
    for (i = MIN_TAG; i < maxtag; i++)
    {
        if (pdd->pDev->tagIlt[i] == NULL)
        {
            BIT_CLEAR(pdd->pDev->tMapAsgn,i);
        }
    }
/* --- If the Maximum Tagged Command Depth shrunk, then need to set bits that
     are not used so that they do not get used.  If the tag is already used,
     when the op completes the completion routine will leave the bit set.*/
    for (; i < MAX_TAG; i++)
    {
        if (pdd->pDev->tagIlt[i] != NULL)
        {
            BIT_SET(pdd->pDev->tMapAsgn,i);
        }
    }
}

/**
 ******************************************************************************
 **
 **  @brief completion reoutine of the test unit ready started in PHY_InitDrive
 **
 **  @param      ilt - command being worked on
 **
 **  @return     NONE
 **
 ******************************************************************************
**/
void PHY_InitDrivePart2(UINT32 status UNUSED, ILT * ilt)
{
    UINT32 stat;
    PRP * prp;

    prp = (PRP*)ilt->ilt_normal.w0;
    stat  = MSC_ChkStat(prp);
    if (stat == EC_OK)
    {
        prp->pDev->physErr = 0;
    }
    /*  the original p$init_drv in i960 had a bunch of code at this
        point for processing Magnitudes conneted to the BE.
        I removed it because it is most likely broken.
        discover lid should be expanded for this case.
    */
    ON_RelReq(ilt);
    P_drvinits--;
    //wake up whoever is waiting for P_drvinits to hit 0
    TaskReadyByState(PCB_FC_READY_WAIT);
}

/**
 ******************************************************************************
 **
 **  @brief  sends a test unit ready to the given PDD and sets max tags
 **
 **  @param      pdd - pdd being worked on
 **
 **  @return     NONE
 **
 ******************************************************************************
 **/
void PHY_InitDrive(UINT32 a UNUSED, UINT32 b UNUSED,PDD * pdd)
{
    ILT * ilt;
    PRP * prp;

    if (pdd == NULL || pdd->pDev == NULL)
    {
        P_drvinits--;
        return;
    }
#ifdef DEBUG_FLIGHTREC_PD
    UINT32 parm1;
    parm1 = (pdd->pDev->port<< 8) | pdd->pDev->lid;
    MSC_FlightRec(FR_P_INIT_DRV,parm1,pdd,(UINT32)pdd->pDev);
#endif
    PHY_SetMaxTags(pdd);
    // set block device
    pdd->pDev->blk = 1;
    ilt = ON_GenReq(&phy_inq_template, pdd, NULL, &prp);
    EnqueueILT((void *)P_que, ilt, (void*)C_label_referenced_in_i960asm(PHY_InitDrivePart2));
}

/* ------------------------------------------------------------------------ */
/* NOTE: g0, g1, g2, g3 destroyed. */
ILT *generate_scsi_write(PDD *pdd, UINT64 sda, UINT32 length, PRP** prp)
{
    ILT *ilt;

    if ((sda & ~0xffffffffULL) == 0ULL)     /* If less than 32 bits */
    {
        UINT32 t = sda & 0xffffffff;
        UINT16 l = length & 0xffff;

        SCSI_WRITE_EXTENDED *s = (SCSI_WRITE_EXTENDED*)&phy_write_10_seg.cmd;
        ilt = ON_GenReq(&phy_write_10_seg, pdd, NULL, prp);
        s->lba = bswap_32(t);
        s->numBlocks = bswap_16(l);
    } else {
        SCSI_WRITE_16 *s = (SCSI_WRITE_16*)&phy_write_16_seg.cmd;
        ilt = ON_GenReq(&phy_write_16_seg, pdd, NULL, prp);
        s->lba = bswap_64(sda);
        s->numBlocks = bswap_32(length);
    }
    return(ilt);
}   /* End of generate_scsi_write */

/* ------------------------------------------------------------------------ */
/* NOTE: g0, g1, g2, g3 destroyed. */
ILT *generate_scsi_writesame(PDD *pdd, UINT64 sda, UINT32 length, PRP** prp)
{
    ILT *ilt;

    if ((sda & ~0xffffffffULL) == 0ULL)     /* If less than 32 bits */
    {
        UINT32 t = sda & 0xffffffff;
        UINT16 l = length & 0xffff;

        SCSI_WRITE_SAME *s = (SCSI_WRITE_SAME*)&phy_writesame_10_seg.cmd;
        ilt = ON_GenReq(&phy_writesame_10_seg, pdd, NULL, prp);
        s->lba = bswap_32(t);
        s->numBlocks = bswap_16(l);
    } else {
        SCSI_WRITESAME_16 *s = (SCSI_WRITESAME_16*)&phy_writesame_16_seg.cmd;
        ilt = ON_GenReq(&phy_writesame_16_seg, pdd, NULL, prp);
        s->lba = bswap_64(sda);
        s->numBlocks = bswap_32(length);
    }
    return(ilt);
}   /* End of generate_scsi_write */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
