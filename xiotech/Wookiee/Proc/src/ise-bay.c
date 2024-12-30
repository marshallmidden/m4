/* $Id: ise-bay.c 160873 2013-04-05 22:04:52Z marshall_midden $ */
/**
 ******************************************************************************
 **
 **  @file       ise-bay.c
 **
 **  To provide a common means of handling the enclosure services for the
 **  ISE device.
 **
 **  Copyright (c) 2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#define _GNU_SOURCE     // features.h, we are compiling for GLIBC.

#include "ise.h"
#include "options.h"
#include "ses.h"
#include "chn.h"
#include "def.h"
#include "defbe.h"
#include "def_con.h"
#include "dev.h"
#include "ecodes.h"
#include "ficb.h"
#include "fsys.h"
#include "GR_Error.h"
#include "GR_LocationManager.h"
#include "ilt.h"
#include "fabric.h"
#include "isp.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "miscbe.h"
#include "nvram.h"
#include "online.h"
#include "pcb.h"
#include "pdd.h"
#include "pm.h"
#include "portdb.h"
#include "rebuild.h"
#include "scsi.h"
#include "system.h"
// #include "ssms.h"
// #include "scd.h"
// #include "dcd.h"
// #include "cor.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "fabric.h"
#include "rrp.h"
#include "CT_defines.h"

#include <string.h>
// extern size_t strnlen(const char *s, size_t maxlen);
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/inet.h>
// #include <time.h>
#include "CT_change_routines.h"

/*
 ******************************************************************************
 ** Private defines - constants
 ******************************************************************************
 */
#define MAX_RETRY_COUNT 36

/*
 ******************************************************************************
 ** Public variables - externed in the header file
 ******************************************************************************
 */

static UINT8 S_bgprun = FALSE;
PCB          *S_bgppcb = NULL;
SES_DEV_INFO_MAP *SES_DevInfoMaps = NULL;
UINT16      SES_DIMEntries = 0;

extern void L$send_packet(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);

/*
 ******************************************************************************
 ** Public variables - not in the header file
 ******************************************************************************
 */

UINT8       SBOD_Trunking[MAX_PORTS];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

static void ses_CheckPaths(void);
static void ise_LocateiseBays(void);
int         ISE_GetVolumeInfo(PDD *pPDD);

void ISE_LogEvent(UINT16 bayID,UINT8 busyFlag);
void ISE_BEBusy(UINT16 pid,UINT32 TimetoFail, UINT8 justthisonepid);
void ISE_BEClear(UINT16 pid, UINT8 justthisonepid);
void ISE_UpdateVDisks(void);
void ISE_TURSuccess(UINT16 pid);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Start the processing of SES information in background.
**
**              This function start the SES processing task if it
**              was asleep or set the processing flag otherwise.
**              on and in hotswap operations.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void SES_StartBGProcess_c(void)
{
    S_bgprun = TRUE;

    if (TaskGetState(S_bgppcb) == PCB_NOT_READY)
    {
#ifdef HISTORY_KEEP
        CT_history_pcb("SES_StartBGProcess_c setting ready pcb", (UINT32)(S_bgppcb));
#endif
        TaskSetState(S_bgppcb, PCB_READY);
    }
}


/**
******************************************************************************
**
**  @brief  Get ISE page 85 information
**
**  @param  device
**  @param  iseip1 - Pointer to area to get ISE IP address 1
**  @param  ispip2 - Pointer to area to get ISE IP address 2
**
**  @return Error code
**
******************************************************************************
**/
int ISE_GetPage85(DEV *device, UINT32 *iseip1, UINT32 *iseip2, int retries)
{
    ILT        *pILT;
    PDD        *pPDD;
    void       *pDataBuffer;
    PRP        *pPRP;
    UINT8      *buffptr;
    int         status;
    int         descr_len;
    int         next_desr;
    UINT8      *mgmtip1;
    UINT8      *mgmtip2;
    struct in_addr ise_ipaddr1;
    INQ_P85_MGMT_DESC *idDesc;

    if (device == NULL || device->pdd == NULL)
    {
        fprintf(stderr, "%s: device or PDD is NULL\n", __func__);
        return FAIL;
    }

    pPDD = device->pdd;

    do
    {
        pILT = ON_GenReq(&gTemplateMgmtNetworkPage85, pPDD, &pDataBuffer, &pPRP);
        ON_QueReq(pILT);
        status = MSC_ChkStat(pPRP);

        if (status != EC_OK)
        {
            ON_RelReq(pILT);
            fprintf(stderr, "%s: ISE#%d page 85 failed with %d, #retries=%d\n",
                    __func__, pPDD->pid, status, retries);
            TaskSleepMS(1000);
            continue;
        }

        /* First descriptor */
        idDesc = (INQ_P85_MGMT_DESC *)pDataBuffer;
        buffptr = (UINT8 *)pDataBuffer + 4;

        descr_len = (buffptr[2] << 8) + buffptr[3];

        mgmtip1 = buffptr + 11;
        inet_aton((char *)mgmtip1, &ise_ipaddr1);
        *iseip1 = ntohl(ise_ipaddr1.s_addr);

        /* Next descriptor */
        next_desr = descr_len + 4;
        buffptr = buffptr + next_desr;

        mgmtip2 = buffptr + 11;
        inet_aton((char *)mgmtip2, &ise_ipaddr1);
        *iseip2 = ntohl(ise_ipaddr1.s_addr);

        ON_RelReq(pILT);
        if (*iseip1 != 0 || *iseip2 != 0)
        {
            break;              /* Exit loop successfully */
        }

        fprintf(stderr, "%s: ISE#%d page 85 failed, #retries %d, both IP addresses are zero\n", __func__, pPDD->pid, retries);
        status = FAIL;
        TaskSleepMS(1000);
    } while (--retries > 0);

    return status;
}                               /* ISE_GetPage85 */


/**
******************************************************************************
**
**  @brief      adapter code for external references for Directly Addressable check
**
**              This function looks for a directly addressable bay. created to
**              avoid changes to external modules. crude, but effective...
**
**  @param      pdd pointer
**
**  @return     1 if found, 0 if match
**
******************************************************************************
**/
UINT8 SES_DirectlyAddressable(PDD *pdd UNUSED)
{
    return (0);
}

/*----------------------------------------------------------------------------
** Function Name: SES_BypassCtrl()
**
** Comments:
**  This function will send a 'drive delay' log message to the CCB.  In later
**  releases, this function will attempt to bypass a drive.
**
** Input:
**  pdd       - PDD of the drive to bypass
**
**--------------------------------------------------------------------------*/
void SES_BypassCtrl(struct PDD *pdd)
{
    fprintf(stderr, "%s: pdd is %p\n", __func__, pdd);
}

/**
******************************************************************************
**
**  @brief      Vendor Specifice Command(ISE)
**
**  @param      pPDD - pointer to the device record.
**
**  @return     UINT8  status
**
******************************************************************************
**/

int ISE_GetVolumeInfo(PDD *pPDD)
{
    ILT        *pILT;
    void       *pDataBuffer;
    PRP        *pPRP;
    ISE_VOLUMEINFO *ise_info;
    int         status;

    pILT = ON_GenReq(&gISEVolumeInfo, pPDD, &pDataBuffer, &pPRP);
    ON_QueReq(pILT);
    status = MSC_ChkStat(pPRP);
    if (status == EC_OK)
    {
        ise_info = (ISE_VOLUMEINFO *)pDataBuffer;
        strncpy((char *)pPDD->prodID, (char *)ise_info->ise_drive_id, 16);
    }
    else
    {
        fprintf(stderr, "%s: failed for ISE %p \n", __func__, pPDD);
        status = FAIL;
    }
    ON_RelReq(pILT);

    return status;
}                               /* ISE_GetVolumeInfo */


/**
******************************************************************************
**
**  @brief      Determine the device type -- always fibre channel disk for 7000.
**
**  @param      pPDD - pointer to the device record.
**
**  @return     UINT8 devType
**
******************************************************************************
**/
UINT8 SES_GetDeviceType(PDD *pPDD)
{
    UINT16      indx;
    UINT16      retValue = PD_DT_UNKNOWN;
    UINT32      vendIDLen;
    UINT32      prodIDLen;

    if (pPDD->lun == 0)
    {
        return PD_DT_ISE_SES;
    }

    vendIDLen = strnlen((char *)pPDD->vendID, 8);
    prodIDLen = strnlen((char *)pPDD->prodID, 16);

    if ((vendIDLen > 0) && (prodIDLen > 0))
    {
        for (indx = 0; indx <= SES_DIMEntries; ++indx)
        {
            if (!strncmp((char *)SES_DevInfoMaps[indx].devVendor, (char *)pPDD->vendID, (signed)vendIDLen) &&
                !strncmp((char *)SES_DevInfoMaps[indx].devProdID, (char *)pPDD->prodID, (signed)prodIDLen))
            {
                retValue = SES_DevInfoMaps[indx].devFlags[SES_DI_DEV_TYPE];
                /* Cannot have a bay on any LUN except zero -- which is done upon entering this routine. */
                if (retValue != PD_DT_ISE_SES)
                {
                    break;
                }
                retValue = PD_DT_UNKNOWN;
            }
        }
    }

    /*
     ** If we don't get a match from the drive table treat this as a standard
     ** FC disk drive from ISE.
     */
    if (retValue == PD_DT_UNKNOWN)
    {
        if (pPDD->devType == PD_DT_BAY_UNKNOWN)
        {
            retValue = PD_DT_BAY_UNKNOWN;
        }
        else
        {
            retValue = PD_DT_ISE_BALANCE;
        }
    }

    return retValue;
}


/**
******************************************************************************
**
**  @brief      Process SES enclosure discovery in background.
**
**              This is a task spawned off during initial power
**              on and in hotswap operations.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
NORETURN void SES_BackGroundProcess()
{
    while (1)
    {
        /*
         ** Go to sleep waiting for a signal to run.  If the signal is already
         ** present, then run.
         */

        if (!S_bgprun)
        {
            fprintf(stderr, "SES_BackGroundProcess sleeping\n");
            TaskSetMyState(PCB_NOT_READY);
            TaskSwitch();
        }

        fprintf(stderr, "SES_BackGroundProcess running\n");

        /*
         ** Set the flag to indicate that the processing ran.
         */
        S_bgprun = FALSE;

        ise_LocateiseBays();

        /*
         ** Check for path violations (less than two paths to each device).
         */
        ses_CheckPaths();

        /*
         ** Find if any of the drives is being inserted across the geo
         ** location. If yes, send a log event to CCB informing that the
         ** drive is being inserted across geo location and modify the
         ** location ID of the drive being inserted to the location ID of
         ** drive bay into which the insertion happend
         */
        GR_CheckForCrossLocationInsertion();

        /*
         ** Log a configuration changed message and save to NVRAM if there
         ** are no outstanding requests.
         */
        if (!S_bgprun)
        {
            NV_P2Update();
            ON_LogError(LOG_CONFIG_CHANGED);
        }

        /*
         ** Do load balancing.
         */
        FAB_BalanceLoad();

    }                           /* while loop */
}


/**
******************************************************************************
**
**  @brief      Locate ISE enclosures.
**
**      This also marks enclosed disks as non-existent when an ISE
**      is missing.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void ise_LocateiseBays()
{
    PDD        *ise_bay;
    PDD        *chkDrv;
    UINT32      iseindx;
    UINT32      drvCnt;
    UINT32      chkIndx;

    for (iseindx = 0; iseindx < MAX_DISK_BAYS; iseindx++)
    {
        ise_bay = gEDX.pdd[iseindx];

        if (ise_bay == NULL)
        {
            continue;
        }

        if (ise_bay->devStat == PD_NONX && !FAB_IsDevInUse(ise_bay->pDev))
        {
            fprintf(stderr, " %s: bay iseindx = %d missing\n", __func__, iseindx);

            ON_LogBayMissing(ise_bay);

            drvCnt = 0;

            for (chkIndx = 0; chkIndx < MAX_PHYSICAL_DISKS; chkIndx++)
            {
                chkDrv = gPDX.pdd[chkIndx];

                if ((chkDrv != NULL) && (chkDrv->ses == iseindx))
                {
                    drvCnt++;
                }
            }
            fprintf(stderr, " %s drivecount %d\n", __func__, drvCnt);

            if (drvCnt == 0 && !FAB_IsDevInUse(ise_bay->pDev))
            {
                /*
                 ** Free the slot for this bay.
                 */
                gEDX.pdd[iseindx] = NULL;
                gEDX.count--;

                /*
                 ** Delete it since it is not reporting.
                 */
                if (ise_bay->pDev != NULL)
                {
                    s_Free(ise_bay->pDev, sizeof(DEV), __FILE__, __LINE__);
                }
                DC_RelPDD(ise_bay);

            }                   /* if drvCnt==0 */

        }                       /* end of devstat == NONX */

    }                           /* end of maxdiskbay for loop */

}                               /* ise_LocateiseBays */





/**
******************************************************************************
**
**  @brief      Check the disk drives to see if they have sufficient
**              paths to them.
**
**              Each disk drive is checked to see if there is at least
**              two paths to it.  If there is only one, then a log message
**              is generated.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void ses_CheckPaths()
{
    UINT32      indx;           /* Pointer into drive list          */
    UINT32      paths;          /* Number of paths to the drive     */
    UINT32      channel;        /* Channel number                   */

    /*
     ** Make the check for disk drives.
     */
    for (indx = 0; indx < MAX_PHYSICAL_DISKS; indx++)
    {
        if ((gPDX.pdd[indx] != NULL) && (gPDX.pdd[indx]->devStat != PD_NONX))
        {
            /*
             ** Get the path count.
             */
            for (paths = 0, channel = 0; channel < MAX_PORTS; channel++)
            {
                if (gPDX.pdd[indx]->pDev->pLid[channel] != NO_CONNECT)
                {
                    paths++;
                }
            }

            if (paths == 1)
            {
                ON_LogDeviceSPath(gPDX.pdd[indx]);
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      Set up the ISE LUN 0 as a bay
**
**  @param      pdd     - the PDD to use.
**  @param      * cnt   - pointer to the count of bays being polled.
**  @param      logEntry- Boolean, log or do not log new entry
**
**  @return     none
**
******************************************************************************
**/
void SES_GetDirectEnclosure(UINT32 foo UNUSED, UINT32 bar UNUSED,
                            PDD *pdd, volatile UINT32 *cnt, UINT32 logEntry)
{
    if (pdd->ses >= MAX_DISK_BAYS)
    {
        fprintf(stderr, "%s: ses %d invalid for pdd %p\n", __func__, pdd->ses, pdd);
        goto out;
    }

    if (gEDX.pdd[pdd->ses] == NULL)
    {
        ++gEDX.count;
    }
    else if (gEDX.pdd[pdd->ses] != pdd)
    {
        fprintf(stderr, "%s: bay id %d, pdd %p replaced with pdd %p\n",
                __func__, pdd->ses, gEDX.pdd[pdd->ses], pdd);
    }
    gEDX.pdd[pdd->ses] = pdd;
    pdd->pid = pdd->ses;
    pdd->devType = SES_GetDeviceType(pdd);
    pdd->postStat = pdd->devStat = PD_OP;

    /*
     ** If the caller was requesting that this enclosure be logged
     ** if new, then log it.
     */
    if (logEntry)
    {
        ON_LogBayInserted(pdd);
    }

    /*
     ** Decrement the count of drives being checked.
     */
  out:
    if (cnt != NULL)
    {
        *cnt = *cnt - 1;
    }
}


/**
******************************************************************************
** NEW PAB IMPLEMENTATION
******************************************************************************
**/

/**
******************************************************************************
**
**  @brief      ISE_LogEvent
**
**  @param      bayID   - ID of the ISE
**              busyFlag - TRUE/FALSE
**  @return     none
**
******************************************************************************
**/

void ISE_LogEvent(UINT16 bayID,UINT8 busyFlag)
{
    LOG_ISE_SPECIAL_PKT  logMsg;
    logMsg.header.event = LOG_ISE_SPECIAL;
    logMsg.data.bayID        = bayID;
    logMsg.data.busyFlag     = busyFlag;
    MSC_LogMessageStack(&logMsg, sizeof(LOG_ISE_SPECIAL_PKT));

} /* ISE_LogEvent */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
