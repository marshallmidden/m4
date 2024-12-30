/* $Id: DEF_BEGetInfo.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       DEF_BEGetInfo.c
**
**  To provide a common means of handling the define configuration.
**
**  Copyright (c) 1996-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "DEF_BEGetInfo.h"

#include "XIO_Types.h"

#include "defbe.h"
#include "def_lun.h"
#include "isp.h"
#include "MR_Defs.h"
#include "online.h"
#include "rebuild.h"
#include "RL_RDD.h"
#include "sdd.h"
#include "string.h"
#include "vdd.h"
#include "ddr.h"
#include "misc.h"

#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include <stdio.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define DE_COPYTERMINATE_PENDING        5       /* Copy termination pending flag */
#define DE_PDDLIST                      1       /* Physical device list         */
#define DE_SESLIST                      2       /* SES device list              */
#define DE_MISCLIST                     3       /* Miscelleneous device list    */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

UINT32      DEF_GetDeviceData(MR_PKT *, PDD *);
UINT32      DEF_GetPhysicalDeviceData(MR_PKT *, UINT8);
UINT32      DEF_GetPhysicalDeviceList(MR_PKT *, UINT8);
UINT32      DEF_CheckForRedundancy(RDD *, PDD *, UINT16);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the list of
**              devices which are defined.
**
**              This function will dump the list of devices regardless
**              of their operational status.
**
**  @param      pMRP    : MRP packet
**
**  @return     UINT32 status
**
******************************************************************************
**/

UINT32 DEF_GetVList(MR_PKT *pMRP)
{
    UINT16      D_index;
    UINT32      reqLen;
    UINT32      retLen;
    UINT16      virtualID;
    MRGETVLIST_REQ *pREQ;
    MRGETVLIST_RSP *pRSP;

    /* Initialize parameters */
    pREQ = pMRP->pReq;
    pRSP = (MRGETVLIST_RSP *) pMRP->pRsp;

    /*
     * Set initial return length, and calculate maximum data allowed.
     * the message always contains a header of size sizeof (*pRSP)
     */
    reqLen = pMRP->rspLen;

    if (sizeof(MRGETVLIST_RSP) > reqLen)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETVLIST_RSP);    /* So we can get more data on next try. */
        return DETOOMUCHDATA;
    }

    retLen = sizeof(MRGETVLIST_RSP) - sizeof(UINT16);   /* Array[1], subtract that out. */

    virtualID = pREQ->id;

    /*
     * Validate starting VID. i.e return if ID is greater than or equal to the
     * Maximum ID, else continue getting list of devices.
     */
    if (virtualID >= MAX_VIRTUAL_DISKS)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETVLIST_RSP);
        return DEINVVID;
    }

    pRSP->ndevs = gVDX.count;

    /*
     * Traverse through the list for all device IDs, and save into outPut list.
     * Check for the presence of valid device and enough space to hold it.
     */
    for (D_index = 0; virtualID < MAX_VIRTUAL_DISKS; virtualID++)
    {
        if ((retLen + sizeof(UINT16)) > reqLen)
        {
            pMRP->pRsp->rspLen = retLen + sizeof(UINT16);   /* So we can get more data on next try. */
            return DETOOMUCHDATA;
        }

        if (gVDX.vdd[virtualID] != 0)
        {
            pRSP->list[D_index++] = virtualID;
            retLen = retLen + sizeof(UINT16);
        }
    }

    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the statistic and
**              configuration data from a virtual disk.
**
**              This function will dump the configuration and statistical data
**              from a virtual device definition.
**
**  @param      pMRP    : MRP packet
**
**  @return     error code
**
**  @attention  If the response data size exceeds the maximum data allowed,
**              in assembly code attempted to check whether space is available
**              for at least headers, but not practically implemented in code.
**              In 'C' code the same was not implemented.
**
******************************************************************************
**/
UINT32 DEF_GetVirtualData(MR_PKT *pMRP)
{
    UINT16      raidIndex;
    UINT32      retLen;
    UINT32      reqLen;
    UINT32      vidIndex;
    VDD        *pVDD;
    RDD        *pRDD;
    RDD        *pDRDD;
    MRGETVINFO_REQ *pREQ;
    MRGETVINFO_RSP *pRSP;

    /* Initialize local variables. */
    pREQ = (MRGETVINFO_REQ *) pMRP->pReq;
    pRSP = (MRGETVINFO_RSP *) pMRP->pRsp;
    reqLen = pMRP->rspLen;

    vidIndex = pREQ->id;

    /* Validate the vid. */
    if (vidIndex >= MAX_VIRTUAL_DISKS)
    {
        pMRP->pRsp->rspLen = 0;
        return DEINVVID;
    }

    pVDD = gVDX.vdd[vidIndex];

    /* Check the validity of the VID. */
    if (pVDD == NULL)
    {
        pMRP->pRsp->rspLen = 0;
        return DEINVVID;
    }

    if (reqLen < sizeof(MRGETVINFO_RSP))
    {
        pMRP->pRsp->rspLen = 0;
        return DETOOMUCHDATA;
    }

    /*
     * Fill the structure with statistical information. Copy
     * data from VDD structure up to RAID pointers. In the
     * memcpy function, length to copy is reduced by 16 bytes.
     * Careful with pointer arithmetic.
     */
    retLen = sizeof(MRGETVINFO_RSP);
    pRDD = pVDD->pRDD;
    if (pRDD != NULL)
    {
        if (pRDD->type == RD_SLINKDEV)
        {
            if (pVDD->status == VD_INOP)
            {
                pVDD->grInfo.vdOpState = 0;   // GR_VD_INOP;
            }
            else
            {
                pVDD->grInfo.vdOpState = 3;   // GR_VD_OP;
            }
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
    }

    memcpy((UINT8 *)pRSP + sizeof(pRSP->header), pVDD,
           sizeof(MRGETVINFO_RSP) - sizeof(MR_HDR_RSP));

    /*
     * Update the time of last access - this is the number
     * of recent seconds for which the disk is NOT accessed.
     */
    if (pVDD->lastAccess == 0)
    {
        pRSP->lastAccess = -1;
    }
    else
    {
        /*
         * If we are the owner, compute the last access.
         * If not, reset the value.
         */
        if (DL_ExtractDCN(K_ficb->cSerial) == pVDD->owner)
        {
            pRSP->lastAccess = (K_ii.time - pVDD->lastAccess) / 8;
        }
        else
        {
            pVDD->lastAccess = 0;
            pRSP->lastAccess = -1;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        }
    }

    /* Store the average IO per second and SC per second over last hour */
    if (pVDD->pLastHourAvgStats != NULL)
    {
        if (pVDD->pLastHourAvgStats->statsFlag)
        {
            pRSP->lastHrAvgIOPerSec = pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond / 3600;
            pRSP->lastHrAvgSCPerSec = pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond / 3600;
        }
        else if (pVDD->pLastHourAvgStats->currentIndex == 0)
        {
            pRSP->lastHrAvgIOPerSec = 0;
            pRSP->lastHrAvgSCPerSec = 0;
        }
        else
        {
            pRSP->lastHrAvgIOPerSec =
                pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond /
                (pVDD->pLastHourAvgStats->currentIndex * 60);
            pRSP->lastHrAvgSCPerSec =
                pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond /
                (pVDD->pLastHourAvgStats->currentIndex * 60);
        }
    }

    /*
     * Traverse through the RAID list for all LUN to Vdisk
     * mappings. Before getting the data check for space
     * availability in response structure.
     */
    if (retLen > (reqLen - ((pVDD->raidCnt + pVDD->draidCnt) * sizeof(UINT16))))
    {
        pMRP->pRsp->rspLen = retLen;
        return DETOOMUCHDATA;
    }

    retLen += ((pVDD->raidCnt + pVDD->draidCnt) * sizeof(UINT16));
    pRDD = pVDD->pRDD;
    raidIndex = 0;

    while (pRDD)
    {
        pRSP->rid[raidIndex++] = pRDD->rid;
        pRDD = pRDD->pNRDD;
    }

    /*
     * Traverse the deffered RAID list for all LUN to
     * Vdisk Mappings. For each deffered RAID before
     * getting the data check for space availability
     * in response structure.
     */
    pDRDD = pVDD->pDRDD;

    while (pDRDD)
    {
        pRSP->rid[raidIndex++] = pDRDD->rid;
        pDRDD = pDRDD->pNRDD;
    }

    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide the extended information for virtual disk
**
**
**              This function will get the extended data
**              from a virtual device definition.
**
**  @param      pMRP    : MRP packet
**
**  @return     error code
**
******************************************************************************
**/
UINT32 DEF_GetExtendVirtualData(MR_PKT *pMRP)
{
    UINT32      status = DEINVVID;
    UINT32      retLen = 0;
    UINT32      reqLen;
    UINT32      vidIndex;
    VDD        *pVDD = NULL;
    MRGETEXTEND_VINFO_REQ *pREQ;
    MREXTEXTEND_VINFO_RSP *pRSP;

    /* Initialize local variables. */
    pREQ = (MRGETEXTEND_VINFO_REQ *) pMRP->pReq;
    pRSP = (MREXTEXTEND_VINFO_RSP *) pMRP->pRsp;
    reqLen = pMRP->rspLen;

    /* Validate the vid. */
    if (pREQ->id < MAX_VIRTUAL_DISKS)
    {
        vidIndex = pREQ->id;

        /* Check the validity of the VID. */
        if (gVDX.vdd[vidIndex] != NULL)
        {
            pVDD = gVDX.vdd[vidIndex];

            if (pVDD)
            {
                status = DEOK;

                if (retLen <= (reqLen - sizeof(MREXTEXTEND_VINFO_RSP)))
                {
                    /*
                     * Fill the structure with extended vdisk information
                     * We are adding the entire structure length as return
                     * length.
                     */
                    retLen += sizeof(MREXTEXTEND_VINFO_RSP);
                    pRSP->data.breakTime = pVDD->breakTime;
                }
                else
                {
                    status = DETOOMUCHDATA;
                }
            }
        }
    }
    pMRP->pRsp->rspLen = retLen;
    return (status);
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the list of
**              servers which are defined.
**
**              This function will dump the list of servers regardless
**              of their operational status.
**
**  @param      MR_PKT      *pMRP    : MRP packet
**
**  @return     UINT32      status   : Function return status
**
******************************************************************************
**/
/* MRP 0x117 */
UINT32 DEF_GetSList(MR_PKT *pMRP)
{
    UINT16      D_index;
    UINT32      reqLen;
    UINT32      retLen;
    UINT16      serverID;
    MRGETSLIST_REQ *pREQ;
    MRGETSLIST_RSP *pRSP;

    /* Initialize local variables */
    pREQ = (MRGETSLIST_REQ *) pMRP->pReq;
    pRSP = (MRGETSLIST_RSP *) pMRP->pRsp;

    /*
     * Set initial return length, and calculate maximum data allowed.
     * the message always contains a header of size sizeof (*pRSP)
     */
    reqLen = pMRP->rspLen;

    if (sizeof(MRGETSLIST_RSP) > reqLen)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETSLIST_RSP);    /* So we can get more data on next try. */
        return DETOOMUCHDATA;
    }

    retLen = sizeof(MRGETSLIST_RSP) - sizeof(UINT16);   /* Array[1], so subtract that out. */

    serverID = pREQ->id;

    /*
     * Validate ID, i.e return from ID is greater than or equal to the
     * Maximum ID, else continue getting list of devices.
     */
    if (serverID >= MAX_SERVERS)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETVLIST_RSP);
        return DEINVSID;
    }

    pRSP->ndevs = gSDX.count;

    /*
     * Traverse through the list for all server IDs, and save into outPut list.
     * Check for the presence of valid device and enough space to hold it.
     */
    for (D_index = 0; serverID < MAX_SERVERS; serverID++)
    {
        if ((retLen + sizeof(UINT16)) > reqLen)
        {
            pMRP->pRsp->rspLen = retLen + sizeof(UINT16);   /* So we can get more data on next try. */
            return DETOOMUCHDATA;
        }

        if (gSDX.sdd[serverID] != 0)
        {
            pRSP->list[D_index++] = serverID;
            retLen = retLen + sizeof(UINT16);
        }
    }

    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the list of
**              raids which are defined.
**
**              This function will dump the list of raids
**
**  @param      MR_PKT      *pMRP    : MRP packet
**
**  @return     UINT32      status   : Function return status
**
******************************************************************************
**/
/* MRP 0x119 */
UINT32 DEF_GetRList(MR_PKT *pMRP)
{
    UINT16      D_index;
    UINT32      reqLen;
    UINT32      retLen;
    UINT16      raidID;
    MRGETRLIST_REQ *pREQ;
    MRGETRLIST_RSP *pRSP;

    /* Initialize local variables */
    pREQ = (MRGETRLIST_REQ *) pMRP->pReq;
    pRSP = (MRGETRLIST_RSP *) pMRP->pRsp;

    /*
     * Set initial return length, and calculate maximum data allowed.
     * the message always contains a header of size sizeof (*pRSP)
     */
    reqLen = pMRP->rspLen;

    if (sizeof(MRGETRLIST_RSP) > reqLen)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETRLIST_RSP);    /* So we can get more data on next try. */
        return DETOOMUCHDATA;
    }

    retLen = sizeof(MRGETRLIST_RSP) - sizeof(UINT16);

    raidID = pREQ->id;

    /*
     * Validate RID. i.e return if ID is greater than or equal to the
     * Maximum ID, else continue getting list of devices.
     */
    if (raidID >= MAX_RAIDS)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETRLIST_RSP);
        return DEINVRID;
    }

    pRSP->ndevs = gRDX.count;

    /*
     * Traverse through the list for all RAID IDs, and save into outPut list.
     * Check for the presence of valid device and enough space to hold it.
     */
    for (D_index = 0; raidID < MAX_RAIDS; raidID++)
    {
        if ((retLen + sizeof(UINT16)) > reqLen)
        {
            pMRP->pRsp->rspLen = retLen + sizeof(UINT16);   /* So we can get more data on next try. */
            return DETOOMUCHDATA;
        }

        if (gRDX.rdd[raidID])
        {
            pRSP->list[D_index++] = raidID;
            retLen = retLen + sizeof(UINT16);
        }
    }

    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the list of
**              targets which are defined.
**
**              This function will dump the list of targets
**
**  @param      MR_PKT      *pMRP    : MRP packet
**
**  @return     UINT32      status   : Function return status
**
******************************************************************************
**/
/* MRP 0x131 */
UINT32 DEF_GetTList(MR_PKT *pMRP)
{
    UINT16      D_index;
    UINT32      reqLen;
    UINT32      retLen;
    UINT16      targetID;
    MRGETTLIST_REQ *pREQ;
    MRGETTLIST_RSP *pRSP;

    /* Initialize local variables */
    pREQ = (MRGETTLIST_REQ *) pMRP->pReq;
    pRSP = (MRGETTLIST_RSP *) pMRP->pRsp;

    /*
     * Set initial return length, and calculate maximum data allowed.
     * the message always contains a header of size sizeof (*pRSP)
     */
    reqLen = pMRP->rspLen;

    if (sizeof(MRGETTLIST_RSP) > reqLen)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETTLIST_RSP);    /* So we can get more data on next try. */
        return DETOOMUCHDATA;
    }

    retLen = sizeof(MRGETTLIST_RSP) - sizeof(UINT16);   /* Array[1], subtract that out. */

    targetID = pREQ->id;

    /*
     * Validate ID, i.e return from ID is greater than or equal to the
     * Maximum ID, else continue getting list of devices.
     */
    if (targetID >= MAX_TARGETS)
    {
        pMRP->pRsp->rspLen = sizeof(MRGETVLIST_RSP);
        return DEINVTID;
    }

    pRSP->ndevs = gTDX.count;

    /*
     * Traverse through the list for all target IDs, and save into outPut list.
     * Check for the presence of valid device and enough space to hold it.
     */
    for (D_index = 0; targetID < MAX_TARGETS; targetID++)
    {
        if ((retLen + sizeof(UINT16)) > reqLen)
        {
            pMRP->pRsp->rspLen = retLen + sizeof(UINT16);   /* So we can get more data on next try. */
            return DETOOMUCHDATA;
        }

        if (gTDX.tgd[targetID])
        {
            pRSP->list[D_index++] = targetID;
            retLen = retLen + sizeof(UINT16);
        }
    }

    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      This function retrieves the statistic and configuration
**              data from a RAID device.
**
**              This function will dump the configuration and statistical data
**              from a RAID device definition.
**
**  @param      MR_PKT  *pMRP       : MRP
**
**  @return     UINT32  status      : Function status
**
******************************************************************************
**/
/* MRP 0x11D */
UINT32 DEF_GetRAIDDeviceData(MR_PKT *pMRP)
{
    UINT32      retLen;
    UINT32      reqLen;
    UINT16      raidID;
    RDD        *pRDD;
    PSD        *pPSD;
    PSD        *pPSDStart;
    UINT8      *pRspData;
    MRGETRINFO_RSP_EXT *pPsdExt;
    MRGETRINFO_REQ *pREQ;
    MRGETRINFO_RSP *pRSP;

    /*
     * Initialize local variables (request length, return length, response
     * structure data pointer).
     */
    pREQ = (MRGETRINFO_REQ *) pMRP->pReq;
    pRSP = (MRGETRINFO_RSP *) pMRP->pRsp;
    reqLen = pMRP->rspLen;
    retLen = sizeof(pRSP->header);
    pRspData = (UINT8 *)((UINT8 *)pRSP + sizeof(pRSP->header));

    raidID = pREQ->id;

    /* Validate RID. */
    if (raidID >= MAX_RAIDS)
    {
        pMRP->pRsp->rspLen = retLen;
        return DEINVRID;
    }

    pRDD = gRDX.rdd[raidID];

    if (pRDD == NULL)
    {
        pMRP->pRsp->rspLen = retLen;
        return DEINVRID;
    }

    if (reqLen < sizeof(MRGETRINFO_RSP))
    {
        pMRP->pRsp->rspLen = retLen;
        return DETOOMUCHDATA;
    }

    /*
     * Fill the response structure with statistical RAID information from RDD
     * structure up to VLOP address (not including VLOP).
     */

    retLen = retLen + sizeof(RDD) - sizeof(struct VLOP *);
    memcpy(pRspData, pRDD, sizeof(RDD) - sizeof(struct VLOP *));

    /*
     * Traverse through the PSD list (circular) for all disk Mappings.
     * For each PSD, before getting the data, check for space availability in
     * response structure.
     */
    pRspData = (UINT8 *)(pRspData + sizeof(RDD) - sizeof(struct VLOP *));
    pPsdExt = (MRGETRINFO_RSP_EXT *) pRspData;
    pPSD = *(PSD **)(pRDD + 1);
    pPSDStart = pPSD;

    do
    {
        /*
         * Increment the return length. And compare with request length, error
         * if return length exceeds request length.
         */
        retLen = retLen + sizeof(MRGETRINFO_RSP_EXT);

        if (retLen > reqLen)
        {
            pMRP->pRsp->rspLen = retLen;
            return DETOOMUCHDATA;
        }

        /* Save PSD pid, status, astatus in response buffer. */
        pPsdExt->pid = pPSD->pid;
        pPsdExt->pidstat = pPSD->status;
        pPsdExt->pidastat = pPSD->aStatus;

        /*
         * Calculate percentage of Rebuild Complete.
         * % Rebuild = (amount of rebuild so far * 100) / (segment size)
         */
        if (pPSD->sLen == 0)
        {
            pPsdExt->rpc = 0;
        }
        else
        {
            pPsdExt->rpc = (UINT8)((pPSD->rLen * 100) / pPSD->sLen);
        }

        /* Get next PSD in the circular list, and continue if not start. */
        pPSD = pPSD->npsd;
        pPsdExt = pPsdExt + 1;
        pRspData = (UINT8 *)pPsdExt;
    }
    while (pPSD != pPSDStart);

    /* Update return length in response structure and return. */
    pMRP->pRsp->rspLen = retLen;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the statistic and
**              configuration data from a physical device.
**
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**  @param      pPDD  - pointer to the PDD of the physical device (disk,
**                            SES, miscellaneous)
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetDeviceData(MR_PKT *pMRP, PDD *pPDD)
{
    UINT32      retStatus = DEOK;
    UINT8       options;
    UINT8       port;
    UINT32      rspLen;

    /* Get the options requested from the MRP request. */
    options = ((MRGETPINFO_REQ *) pMRP->pReq)->options;

    if (BIT_TEST(options, MIP_BIT_REFRESH) && (pPDD->devType <= PD_DT_MAX_DISK)
        && !BIT_TEST(pPDD->flags, PD_BEBUSY))
    {

        /*
         * If the refresh option is requested, and the device type is a
         * disk, do a quick inquiry test. Add one more device to O_drvinits
         * in progress, and call ON_InitDrive to test the drive. Update the
         * PSD and RAID status, and update the virtual status.
         */
        O_drvinits++;
        ON_InitDrive(pPDD, 1, &O_drvinits);
        RB_setpsdstat();
        RB_SetVirtStat();
    }

    if (BIT_TEST(options, MIP_BIT_RLSELS))
    {

        /*
         * If options requested is RLS ELS, get the channel number for the
         * ID requested. Get the pointer to the structure of this channel
         * number. If the pointer is NULL (no card installed), zero out the
         * FC data area. If card is plugged in, get the link statistics for
         * FC ID and update the FC data area.
         */
        port = pPDD->channel;
        if (ispstr[port] == NULL)
        {
            memset((void *)&pPDD->rls, 0, sizeof(ISP_RLS));
        }
        else
        {
            ISP_GetLinkStatus(port, pPDD->id, (void *)&pPDD->rls);
        }
    }

    /*
     * Check for space in MRP response for PDD data. Set return status to
     * 'bad return length' if MRP response does not have enough space. Else
     * copy PDD data to MRP response.
     */
    rspLen = sizeof(MRGETPINFO_RSP);

    if (pMRP->rspLen < rspLen)
    {
        retStatus = DERETLENBAD;
    }
    else
    {
        memcpy((void *)&(((MRGETPINFO_RSP *) pMRP->pRsp)->pdd),
               (void *)pPDD, sizeof(PDD));
        pMRP->pRsp->rspLen = rspLen;
        retStatus = DEOK;
    }

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the statistic and
**              configuration data for physical devices of a certain type
**              (disk, SES, miscellaneous).
**
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**  @param      devType  - the type of physical device (disk, SES or
**                          miscellaneous)
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetPhysicalDeviceData(MR_PKT *pMRP, UINT8 devType)
{
    UINT32      retStatus = DEINVPID;
    UINT32      maxNdevs = 0;
    PDD       **pIndxTbl = NULL;
    UINT32      count = 0;
    UINT16      id;
    PDD        *pPDD = NULL;

    /*
     * Check the device type and get the associated PDD index table, and the
     * value of the max number of devices. If device type is not SES, MISC,
     * or DISK, set return status to 'invalid option'.
     */
    if (devType == MWLSES)
    {
        count = gEDX.count;
        pIndxTbl = gEDX.pdd;
        maxNdevs = MAX_DISK_BAYS;
    }
    else if (devType == MWLMISC)
    {
        count = gMDX.count;
        pIndxTbl = gMDX.pdd;
        maxNdevs = MAX_MISC_DEVICES;
    }
    else if (devType == MWLDISK)
    {
        count = gPDX.count;
        pIndxTbl = gPDX.pdd;
        maxNdevs = MAX_PHYSICAL_DISKS;
    }
    else
    {
        retStatus = DEINVOPT;
    }

    if (retStatus != DEINVOPT)
    {

        /*
         * If the device type id is valid, get the ID requested from the
         * MRP request.
         */
        id = ((MRGETPINFO_REQ *) pMRP->pReq)->id;

        /* Validate id. */
        if (id < maxNdevs)
        {

            /*
             * Get the PDD for the PID. If the PDD for the PID does not
             * exist, set return status to 'invalid PID'. Else,
             * call DEF_GetDeviceData to get data for this device.
             */
            pPDD = pIndxTbl[id];

            if (pPDD)
            {
                retStatus = DEF_GetDeviceData(pMRP, pPDD);
            }
            else
            {
                retStatus = DEINVPID;
            }
        }
    }
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the statistic and
**              configuration data from disk devices.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetPData(MR_PKT *pMRP)
{

    UINT32      retStatus = DEINVPID;

    /* Call DEF_GetPhysicalDeviceData to retrieve the PDD Data for DISK devices */
    retStatus = DEF_GetPhysicalDeviceData(pMRP, MWLDISK);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the statistic and
**              configuration data from MISC devices.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetMData(MR_PKT *pMRP)
{
    UINT32      retStatus = DEINVPID;

    /*
     * Call DEF_GetPhysicalDeviceData to retrieve the PDD Data for MISC
     * devices
     */
    retStatus = DEF_GetPhysicalDeviceData(pMRP, MWLMISC);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the statistic and
**              configuration data from SES devices.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetEData(MR_PKT *pMRP)
{
    UINT32      retStatus = DEINVPID;

    /* Call DEF_GetPhysicalDeviceData to retrieve the PDD Data for SES devices */
    retStatus = DEF_GetPhysicalDeviceData(pMRP, MWLSES);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a standard means of retrieving the list of physical
**              devices which are defined.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**  @param      devType  - type of physical device (disk, SES, miscellaneous)
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetPhysicalDeviceList(MR_PKT *pMRP, UINT8 devType)
{
    UINT32      maxNdevs;
    UINT16      pid;
    PDD        *pPDD;
    MR_LIST_RSP *pRsp;
    PDD       **pIndxTbl;
    UINT16      id;
    UINT32      count;
    UINT32      PDDCnt = 0;
    UINT32      rspLen = 0;

    /*
     * Check the device type and get the associated PDD index table, the
     * value of the max number of devices, and the PDDs count from XDX table.
     * If device type is not SES, MISC, or DISK, set return status to
     * 'invalid option'.
     */
    if (devType == MWLSES)
    {
        pIndxTbl = gEDX.pdd;
        maxNdevs = MAX_DISK_BAYS;
        count = gEDX.count;
    }
    else if (devType == MWLMISC)
    {
        pIndxTbl = gMDX.pdd;
        maxNdevs = MAX_MISC_DEVICES;
        count = gMDX.count;
    }
    else if (devType == MWLDISK)
    {
        pIndxTbl = gPDX.pdd;
        maxNdevs = MAX_PHYSICAL_DISKS;
        count = gPDX.count;
    }
    else
    {
        return DEINVOPT;
    }

    /* If the device type is valid, get the ID requested from the MRP request. */
    id = ((MR_DEVID_REQ *) pMRP->pReq)->id;

    /* If the id is too big, set return status to 'invalid PID'. */
    if (id >= maxNdevs)
    {
        return DEINVPID;
    }

    /* If the id is valid, set the PDDs count value in the MRP response. */
    pRsp = (MR_LIST_RSP *) pMRP->pRsp;
    pRsp->ndevs = count;

    /* Go through the PIDs in the PDD Index table and get the PDD for that PID. */
    for (pid = id; pid < maxNdevs; pid++)
    {
        pPDD = pIndxTbl[pid];
        if (pPDD != NULL)
        {
            /* If the PDD exists, check if enough space for data. */
            rspLen = sizeof(MR_LIST_RSP) + sizeof(pid) * PDDCnt;
            if (pMRP->rspLen < rspLen)
            {
                return DETOOMUCHDATA;
            }

            /* Copy PDD index to response. */
            memcpy((void *)&(pRsp->list[PDDCnt]), (void *)&pid, sizeof(pid));
            PDDCnt++;
        }
    }

    /* Set the response length in the MRP response. */
    pRsp->header.len = sizeof(MR_LIST_RSP) + sizeof(pid) * PDDCnt;
    return DEOK;
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the list of DISK
**              devices which are defined.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetPList(MR_PKT *pMRP)
{
    UINT32      retStatus = DEINVPID;

    /* Call DEF_GetPhysicalDeviceList to retrieve the list for DISK devices */
    retStatus = DEF_GetPhysicalDeviceList(pMRP, MWLDISK);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the list of MISC
**              devices which are defined.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetMList(MR_PKT *pMRP)
{
    UINT32      retStatus = DEINVPID;

    /* Call DEF_GetPhysicalDeviceList to retrieve the list for MISC devices */
    retStatus = DEF_GetPhysicalDeviceList(pMRP, MWLMISC);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the list of SES
**              devices which are defined.
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetEList(MR_PKT *pMRP)
{
    UINT32      retStatus = DEINVPID;

    /* Call DEF_GetPhysicalDeviceList to retrieve the list for SES devices */
    retStatus = DEF_GetPhysicalDeviceList(pMRP, MWLSES);

    /* Return status. */
    return (retStatus);
}


/**
******************************************************************************
**
**  @brief      To provide a means of retrieving the redundacy of the given
**              virtual disk
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/

UINT32 DEF_GetVdiskRedundancy(MR_PKT *pMRP)
{
    UINT16      vid;
    UINT32      status;
    MRVIRTREDUNDANCY_REQ *pReq = (MRVIRTREDUNDANCY_REQ *) (pMRP->pReq);
    MRVIRTREDUNDANCY_RSP *pRsp = (MRVIRTREDUNDANCY_RSP *) (pMRP->pRsp);

    /* Get the VDisk ID */
    vid = pReq->vid;

    /* Check if the given VID exists */
    if ((vid < MAX_VIRTUAL_DISKS) && (gVDX.vdd[vid] != NULL))
    {
        pRsp->status = DEF_IsBayRedundant(gVDX.vdd[vid]);
        status = DEOK;
    }
    else
    {
        status = DEINVVIRTID;
    }
    return status;
}


/**
******************************************************************************
**
**  @brief      To provide a means of knowing whether the given VDisk is
**              redundant.
**
**              To determine the redundancy of a specific VDisk, need to
**              traverse the PDD list in the RDD to ensure that no two drives
**              from the same bay fall in a stripe. Need to ensure that no two
**              adjacent drives are in the same bay with wrap. If it is a R5 P5,
**              then no span of five can be in the same bay. There are following
**              two cases:
**
**              (a) RAID is not wrapped:
**                       RAID is not wrapped, if the PSD count for the RAID is
**                       evenly divisible by the depth or parity. In this case,
**                       need to check all of the drives in the same section of
**                       the RAID to make sure that they are not in the same
**                       BOD as the drive that is being checked.
**
**              Following example explains how to determine a RAID is not wrapped:
**
**              Let's consider a R5 P3 RAID stripped across 6  physical
**              disks as an example and see how this RAID is evenly distributed:
**
**                       RAID stripped across 6 physical disks means, the PSD
**                       count is 6 (from 0 to 5).
**
**                       PSD          stripe1      stripe2      stripe3
**                    ----------   ------------  -----------  ----------
**                        0             D1           D1           D1
**                        1             D2           D2           D2
**                        2             P            P            P
**                        3             D1           D1           D1
**                        4             D2           D2           D2
**                        5             P            P            P
**
**                        If observed the above example, it clearly shows that the
**                        RAID is evenly distributed. Therefore, there is no wrap
**                        in this case.
**
**                        Above, D1 indicates a data stripe
**                               D2 indicates a data stripe
**                               P  indicates a parity stripe
**
**              (b) RAID is wrapped:
**                       RAID is wrapped, if the PSD count for the RAID is not
**                       evenly divisible by the depth of parity. In this case,
**                       need to check the (depth - 1) or (parity -1) drives
**                       before and after the drive of interest. If any of them
**                       are in the same BOD, then there is no redundancy.
**
**              Following example explains how to determine a RAID is wrapped:
**
**              Let's consider a R5 P3 RAID stripped across 10 physical disks as
**              an example and see how this RAID is not evenly distributed:
**
**                       RAID stripped across 10 physical disks means the PSD
**                       count is 10 (from 0 to 9).
**
**                       PSD           Column1    Column2     Column3
**                    ---------     ------------ ----------  ----------
**                        0               D1        D2          P
**                        1               D2        P           D1
**                        2               P         D1          D2
**                        3               D1        D2          P
**                        4               D2        P           D1
**                        5               P         D1          D2
**                        6               D1        D2          P
**                        7               D2        P           D1
**                        8               P         D1          D2
**                        9               D1        D2          P
**
**                        Above, D1 indicates a data stripe
**                               D2 indicates a data stripe
**                               P  indicates a parity stripe
**
**
**                       If observed, the second column (column1), after D1 is
**                       striped on PSD 9, the next data stripe (D2) is striped
**                       on PSD 0, parity (P) is stripped on PSD 1 etc..,Same is
**                       also in clolumn3. Therefore, this RAID is wrapped.
**
**  @param      pVDD  - pointer to the VDD
**
**  @return     TRUE  - If Bay is redundant
**              FALSE - If Bay is not redundant
**
**  @attention  This routine expects valid VDD
**              Validation must be done by the caller.
**
******************************************************************************
**/
#define  NORMAL_RAID      0
#define  DEFERRED_RAID    1

UINT32 DEF_IsBayRedundant(VDD *pVDD)
{
    PDD        *pPDD = NULL;
    RDD        *pRDD = NULL;
    PSD       **pPSD = NULL;
    UINT32      IsRedundant = VDISK_REDUNDANT;
    UINT16      driveLocation;
    UINT8       i;

    /* Check for both normal RAID list and deferred RAID list */
    for (i = NORMAL_RAID; i <= DEFERRED_RAID; i++)
    {
        if (i == NORMAL_RAID)
        {
            /* Get the RAID */
            pRDD = pVDD->pRDD;
        }
        else
        {
            /* Get the deferred RAID */
            pRDD = pVDD->pDRDD;
        }

        /*
         * Check if the RAID is Linked RAID
         * Lets not handle Linked RAID because the RAID is on
         * the remote DSC
         * Just return as Invalid RAID type.
         */
        if ((pRDD != NULL) && (pRDD->type == RD_LINKDEV || pRDD->type == RD_SLINKDEV))
        {
            return DEINVRTYPE;
        }

        /* Check for all the RAIDs (including deferred) */
        for (; (pRDD != NULL) && (!IsRedundant); pRDD = pRDD->pNRDD)
        {
            /* Get the first PSD corresponding to this RAID on this disk */
            pPSD = ((PSD **)(pRDD + 1));

            /* Check for all the PSDs in the RAID */
            for (driveLocation = 0; (driveLocation < pRDD->psdCnt) && (!IsRedundant);
                 driveLocation++)
            {
                /* Get the corresponding PDD */
                pPDD = gPDX.pdd[pPSD[driveLocation]->pid];

                /*
                 * Now, check whether the specified PDD on which stripe exists is
                 * maintaining the redundancy
                 */
                IsRedundant = DEF_CheckForRedundancy(pRDD, pPDD, driveLocation);
            }
        }
    }
    return IsRedundant;
}


/**
******************************************************************************
**
**  @brief      This routine checks whether the specified RAID is maintaining
**              the redundancy by following the logic explained in the above
**              example.
**
**  @param      pRDD          -   Pointer to the RDD
**              pPDD          -   Pointer to the PDD
**              driveLocation -   Location of the drive in the PSD array
**
**  @return     TRUE or FALSE
**
******************************************************************************
**/
UINT32 DEF_CheckForRedundancy(RDD *pRDD, PDD *pPDD, UINT16 driveLocation)
{
    UINT32      startLocation = 0;
    UINT32      D_index;
    UINT16      i;
    PSD       **pPSD = NULL;

    /*
     * Check if the specified RAID is evenly distributed
     *
     * Get the first PSD
     */
    pPSD = ((PSD **)(pRDD + 1));

    if ((pRDD->psdCnt) % (pRDD->depth))
    {
        /*
         * If yes, the RAID is wrapped
         * Get the start location of the interested drive from
         * where the stripe started
         * Get the end location of the drive on to which the last
         * stripe exists
         * Consider R5 Parity 3 and PSD count=10 as an example
         *   For drive at location 0,
         *          startlocation = 10 + 0 -3 + 1 = 8
         *          endlocation   = 0 + 3 - 1 = 2
         *        Therefore, the range of drives to be checked for
         *           are at location 8,9,1 and 2
         *   For drive at location = 8,
         *          startlocation = 8 - 3 + 1 = 6
         *           endlocation  = 8 + 3 - 1 - 10 = 0
         *        Therefore, the range of drives to be checked for
         *           are at location 6,7,9 and 0
         *   For drive at location = 5,
         *          startlocation = 5 - 3 + 1 = 3
         *          endlocation = 5 + 3 - 1 = 7
         *        Therefore, the range of drives to be chacked for
         *           are at location 3,4,6 and 7
         */

        if (pRDD->depth == 0)
            return (VDISK_NOT_REDUNDANT);       /* just a sanity check */

        startLocation = (driveLocation < pRDD->depth - 1) ?
            pRDD->psdCnt + driveLocation - (pRDD->depth - 1) :
            driveLocation - (pRDD->depth - 1);

        D_index = startLocation;

        /*
         * Walk through the group of drives on which the interested
         * RAID is stripped
         */
        for (i = 0; i < (pRDD->depth * 2) - 1; i++)
        {
            /*
             * Check if the RAID is stripped on to a drive in the same
             * enclosure as that of the drive that is being checked for
             * Get the ID of the bay in which the stripe exists
             */
            if ((pPSD[D_index]->pid != pPDD->pid) &&
                (gPDX.pdd[pPSD[D_index]->pid]->ses == pPDD->ses))
            {
                /* If yes, RAID lost its redundancy */
                return (VDISK_NOT_REDUNDANT);
            }
            D_index++;

            if (D_index >= pRDD->psdCnt)
            {
                D_index = 0;
            }
        }
    }
    else
    {
        /*
         * RAID is not wrapped
         * Consider R5, parity 3 and PSD count=9 as an example
         *   For drive location = 0
         *      The range of drives to be checked for start from
         *      startlocation = (0/3)* 3 = 0
         *      which means that the drives to be checked for at
         *      location 0,1 and 2
         */
        startLocation = (driveLocation / pRDD->depth) * (pRDD->depth);

        D_index = startLocation;

        while (D_index < (startLocation + pRDD->depth))
        {
            /*
             * Check if the RAID is stripped on to a drive in the same
             * enclosure as that of the drive that is being checked for
             * Get the ID of the bay in which the stripe exists
             */
            if ((pPSD[D_index]->pid != pPDD->pid) &&
                (gPDX.pdd[pPSD[D_index]->pid]->ses == pPDD->ses))
            {
                /* If yes, RAID lost its redundancy */
                return (VDISK_NOT_REDUNDANT);
            }
            D_index++;
        }
    }
    return (VDISK_REDUNDANT);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
