/* $Id: rebuild.c 160802 2013-03-22 16:39:33Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       rebuild.c
**
**  @brief      Support for rebuild and hotsparing.
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "rebuild.h"

#include "daml.h"
#include "defbe.h"
#include "def_lun.h"
#include "def_con.h"
#include "error.h"
#include "fr.h"
#include "GR_Error.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "pdd.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "system.h"
#include "ses.h"
#include "stdio.h"
#include "string.h"
#include "online.h"
#include "fsys.h"
#include "xdl.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "ddr.h"
#include "misc.h"

/* The following are for created tasks. */
extern unsigned long CT_fork_tmp;
extern void CT_LC_RB_ClearRebuildWaitStateTask(int);
extern void CT_LC_RB_UpdateRebuildWriteStateTask(int);
extern void CT_LC_RB_pdiskAutoFailBackTask(int);
extern void ON_WriteFailedLabel(void);

/*
******************************************************************************
** Private defines
******************************************************************************
*/
#define RB_NO_STATE         0x00        /* Unknown or uninitialized state   */
#define RB_ERROR_POSTED     0x01        /* Error reported for this type     */
#define RB_OK               0x02        /* Error gone - repost if it occurs */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
UINT8       RBExcessState[PD_DT_MAX_DISK + 1];  /* Data space excessive state   */
UINT8       RBNoHS[PD_DT_MAX_DISK + 1]; /* No hot spares left in system */
bool        gClearRebuildWaitStateActive = false;
bool        gAutoFailBackEnable = false;
RB_FAILBACK_UPDATE gFailBackPkt;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
PCB        *gRLLocalImageIPCheckPCB;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
UINT8       RB_CheckOperablePDisks(void);
void        RB_MarkPSDForRebuild(UINT32 pid);
void        RB_CheckHSDepletionWithType(PDD *pInputPDD, UINT8 devType);
void        RB_CheckHSCapacityWithType(PDD *pInputPDD, UINT8 devType);
void        RB_ClearRebuildWaitStateTask(UINT32 dummy1, UINT32 dummy2);
UINT64      RB_FindCapacity(UINT16);
bool        RB_IsRaidRebuildWriteActive(RDD *);
void        RB_UpdateRaidRebuildWritesStatus(void);
void        RB_UpdateRebuildWriteStateTask(UINT32, UINT32, bool, bool);
void        RB_pdiskAutoFailBackTask(UINT32, UINT32, UINT32, UINT32);
PDD        *RB_GetFailedDataPDD(PDD *phsPDD);
UINT8       RB_pdiskRebuildBack(PDD *, PDD *);
void        RB_CheckPDDForFailBack(PDD *);
void        RB_ReInitUsedHotSpare(PDD *);
void        RB_SendUpdateEventToMaster(PDD *, PDD *);
void        RB_FailBackEventHandler(void *, UINT32);
void        RB_NonOpEventHandler(void *, UINT32);
void        RB_MakeHotSpareOperational(PDD *, PDD *);
void        RB_SendNonOpEventToSlave(PDD *);
void        RB_SendUpdateEventToSlave(PDD *, PDD *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Determine starting and ending disk addresses of an RAID op
**
**              This will take the parms for a RAID op and the VDisk and RAID
**              records to calculate the address range of the stripes that
**              contain the RAID op. The starting stripe address for the
**              starting address will be calculated and returned in the SDA
**              and then the starting address plus the length will
**              be used to calculate the ending address and then the ending
**              stripe address.
**
**  @param      RID, SDA, and Length of I/O
**
**  @return     returns starting disk and ending disk address
**
******************************************************************************
**/
void RB_CalcAddressRange(UINT16 rid, UINT32 length, UINT64 sda, UINT64 *pSDA,
                         UINT64 *pEDA)
{
    VDD        *pVDD;
    RDD        *pRDD;
    UINT64      start = 0;
    UINT32      spu = 0;
    UINT32      sps = 0;

    /* Initialize the return values. */
    *pSDA = *pEDA = 0;

    /*
     * Determine the starting address for the RAID within the VDisk.
     * This is done by accumulating the device capacities of the RAIDs
     * before this one in the virtual disk. This value will then be
     * applied to the start and end stripe addresses calculated in the
     * next steps.
     */
    pVDD = gVDX.vdd[gRDX.rdd[rid]->vid];
    pRDD = pVDD->pRDD;

    while (pRDD != gRDX.rdd[rid])
    {
        start += pRDD->devCap;
        pRDD = pRDD->pNRDD;
    }

    /*
     * pRDD now points to the RAID we are operating upon. Calculate the
     * starting RAID LBA by determining the stripe we are in and then
     * determining the starting LBA in that stripe. This is an integer
     * division and then a multiply by the sectors in the unit (SPU).
     */
    if (pRDD->type == RD_RAID5)
    {
        sps = pRDD->sps;
        spu = pRDD->spu;
    }
    else if (pRDD->type == RD_RAID10)
    {
        spu = sps = pRDD->sps;
    }

    /* Now that we have sps and spu, do the math. */
    if (sps != 0)
    {
        *pSDA = start + ((sda / spu) * spu);
        *pEDA = start + (((sda + length - 1) / spu) * spu) + spu - 1;
    }
}


/**
******************************************************************************
**
**  @brief      Determine if there are operable PDisks in the system.
**
**  @param      none
**
**  @return     returns GOOD or ERROR
**
******************************************************************************
**/
UINT8 RB_CheckOperablePDisks(void)
{
    UINT16      i;              /* General purpose counter */
    LOG_ALL_DEV_MISSING_PKT *pLogMsg;   /* mledevmissing log message */
    UINT32      size;

    // This function has no meaning if there are no raids declared.
    if (gRDX.count == 0)
    {
        return GOOD;            /* Return if no raids */
    }

    /* Check each valid PDD in the system */
    for (i = 0; i < MAX_PHYSICAL_DISKS; ++i)
    {
        if (gPDX.pdd[i] && (gPDX.pdd[i]->devStat == PD_OP))
        {
            return GOOD;        /* Return if any drive is operable */
        }
    }

    /*
     * Gather source-dest map  that are in sync and Log all drives are missing along with
     * the data gathered
     */
    size = sizeof(LOG_HEADER_PKT) + sizeof(LOG_ALL_DEV_MISSING_DAT);
    pLogMsg = (LOG_ALL_DEV_MISSING_PKT *)s_MallocC(size, __FILE__, __LINE__);
    (pLogMsg->header).event = LOG_ALL_DEV_MISSING;
    GR_SetVdiskInfoAtAllDevMiss((LOG_ALL_DEV_MISSING_DAT *)(pLogMsg->data));
    MSC_LogMessageRel(pLogMsg, size);

    return ERROR;
}


/**
******************************************************************************
**
**  @brief      Determine whether or not the specified PDD references
**              inoperative/unavailable PSD segments
**
**              A check is made of all RAID devices to determine if any
**              inoperative/unavailable PSD definitions encompass the
**              physical device specified by the PDD's SES information.
**              A TRUE/FALSE indicator is returned based upon the
**              results of this check.
**
**  @param      cSES - current slot to check
**              cSlot - current slot to check
**              pPDD - do not examine this PDD
**              hsOnly - only check hot spares
**              ppPDD - pointer to the conflicting PDD
**
**  @return     returns TRUE if there is a hot spare that is used and
**              was in this location when hot spared or if the location
**              previously contained a drive with RAIDs on it.
**
******************************************************************************
**/
UINT8 RB_InOpCheck(UINT8 cSES, UINT8 cSlot, PDD *pPDD, UINT8 hsOnly, PDD **ppPDD)
{
    UINT16      PDDIndex;
    PDD        *pConflict = NULL;
    UINT8       conflict = FALSE;       /* Assume no conflict   */

    /*
     ** Check each valid PDD in the system. If there is a used hot spare,
     ** see if the original location indicates that it came from the same
     ** slot as the drive passed in.
     */
    for (PDDIndex = 0; PDDIndex < MAX_PHYSICAL_DISKS; ++PDDIndex)
    {
        if ((gPDX.pdd[PDDIndex]) && (pPDD != gPDX.pdd[PDDIndex]))
        {
            if ((*(UINT32 *)(&gPDX.pdd[PDDIndex]->devName) != 0) &&
                (hsOnly != TRUE) &&
                (gPDX.pdd[PDDIndex]->devName[PD_DNAME_CSES] == cSES) &&
                (gPDX.pdd[PDDIndex]->devName[PD_DNAME_CSLOT] == cSlot) &&
                (gPDX.pdd[PDDIndex]->devStat != PD_OP))
            {
                pConflict = gPDX.pdd[PDDIndex];
                break;
            }
        }
    }

    /*
     ** If there is a conflicting PDD, check if the drive is either failed
     ** or is a used hot spare. If either of these cases is true, return
     ** a conflict.
     */
    if (pConflict != NULL)
    {
        if (RB_ActDefCheck(pConflict))
        {
            if ((pConflict->devStat != PD_OP) || (pConflict->devClass == MLDSPARELABEL))
            {
                conflict = TRUE;
            }
        }
    }

    /* Set the return PDD pointer. */
    *ppPDD = pConflict;

    return (conflict);
}


/**
******************************************************************************
**
**  @brief      Determine whether or not the specified PDD references
**              any PSD segments
**
**              A check is made of all RAID devices to determine if any
**              PSD definitions encompass the physical device specified
**              by the PDD
**
**              A TRUE/FALSE indicator is returned based upon the
**              results of this check.
**
**  @param      pPDD - PDD being checked
**
**  @return     returns TRUE if there is a RAID defined in this drive.
**
******************************************************************************
**/
UINT8 RB_ActDefCheck(PDD *pPDD)
{
    UINT8       retCode = FALSE;
    UINT16      rIndx;
    RDD        *pRDD;
    PSD        *pPSD;

    /*
     * If the PDD is non-NULL and the drive is labelled, check for the
     * drive allocation map to contain only one entry (the reserved area).
     * If this is the case, then there are no active definitions on the
     * drive.
     */
    if (pPDD != NULL)
    {
        if ((pPDD->devClass != PD_UNLAB) && (pPDD->devCap != 0))
        {
            /* Make sure the PDD is updated. */
            DA_CalcSpace(pPDD->pid, FALSE);

            if ((pPDD->pDAML != NULL) && (pPDD->pDAML->count != 1))
            {
                return TRUE;
            }
        }
        else
        {
            for (rIndx = 0; (retCode == FALSE) && (rIndx < MAX_RAIDS); ++rIndx)
            {
                pRDD = gRDX.rdd[rIndx];

                if (pRDD != NULL && pRDD->type < RD_LINKDEV)
                {
                    pPSD = *((PSD **)(pRDD + 1));
                    do
                    {
                        if (pPSD->pid == pPDD->pid)
                        {
                            retCode = TRUE;
                            break;
                        }
                        pPSD = pPSD->npsd;
                    } while (pPSD != *((PSD **)(pRDD + 1)));
                }
            }
        }
    }
    return retCode;
}


/**
******************************************************************************
**
**  @brief      Mark all PSDs on the given PDD to be rebuilt
**
**              This function will scan through all the RDDs in the system
**              and mark affected PSDs for rebuild.
**
**  @param      pid - PDisk ID
**
**  @return     none
**
******************************************************************************
**/
void RB_MarkPSDForRebuild(UINT32 pid)
{
    RDD        *pRDD;           /* Pointer to an RDD */
    PSD        *pPSD;           /* Pointer to a PSD */
    PSD        *startpsd;       /* Pointer to a PSD */
    UINT16      i;              /* General purpose counter */

    /* Check each valid RDD in the system for PSDs with matching PIDs */
    for (i = 0; i < MAX_RAIDS; ++i)
    {
        pRDD = R_rddindx[i];
        if (pRDD &&             /* Valid RDD     */
            (pRDD->type != RD_STD) &&   /* Not redundant */
            (pRDD->type != RD_RAID0) && /* Not redundant */
            (pRDD->type != RD_LINKDEV) &&       /* Not here      */
            (pRDD->type != RD_SLINKDEV) &&      /* Not here      */
            (DL_AmIOwner(pRDD->vid)))   /* Our raid      */
        {
            /*
             * The PSDs form a circular list. Grab the first one (it follows
             * the RDD structure in memory) and run around the list.
             */
            startpsd = pPSD = *((PSD **)(pRDD + 1));
            do
            {
                if (pPSD->pid == pid)   /* If PIDs match */
                {
                    BIT_SET(DMC_bits, CCB_DMC_raidcache);       /* Flag raid data has changed. */
                    BIT_SET(pPSD->aStatus, PSDA_REBUILD);       /* Mark for rebuild */

                    /*
                     * If the current status is operable or defragging then
                     * change to rebuild
                     */
                    if (pPSD->status >= PSD_OP)
                    {
                        pPSD->status = PSD_REBUILD;
                    }
                }
                pPSD = pPSD->npsd;      /* Next PSD */
            } while (startpsd != pPSD);
        }
    }
}


/**
******************************************************************************
**
**  @brief      To provide a means of finding the total allocated capacity on
**              the designated device.
**
**              The RDDs with their associated PSDs are searched to determine
**              the capacity in use for the designated device.
**
**  @param      PID - ID of the drive being replaced/hot spared
**
**  @return     capacity (64 bit) of the failing drive
**
******************************************************************************
**/
UINT64 RB_FindCapacity(UINT16 PID)
{
    UINT64      capacity = 0;
    UINT16      rid;
    PSD        *pPSD = NULL;
    RDD        *pRDD;

    /* For each valid RAID device. */
    for (rid = 0; rid < MAX_RAIDS; ++rid)
    {
        pRDD = gRDX.rdd[rid];

        /*
         * If not NULL, traverse the PSDs within the RDD looking for a
         * matching PID. If it matches, accumulate the capacity.
         */
        if ((pRDD != NULL) && (pRDD->type != RD_SLINKDEV) && (pRDD->type != RD_LINKDEV))
        {
            pPSD = *((PSD **)(pRDD + 1));
            do
            {
                if (pPSD->pid == PID)
                {
                    capacity += pPSD->sLen;
                }
                pPSD = pPSD->npsd;
            } while (pPSD != *((PSD **)(pRDD + 1)));
        }
    }

    return (capacity);
}


/**
******************************************************************************
**
**  @brief      To provide a means of finding the best hot spare to use
**              given the device being replaced.
**
**              The hot spares are examined to see which one meets the
**              criteria for replacement. We will chose a hot spare based
**              upon the following criteria.
**
**              1)  Same slot as the drive being replaced.
**              2)  Same bay as the drive being replaced.
**              3)  Same geo-pool if the drive being replaced had geo-RAIDs.
**              4)  Any other hot spare.
**
**              Of course, the replacement must be big enough to handle the
**              space requirements of the drive being replaced. If more than
**              one candidate exists, the best fit is chosed.
**
**  @param      PID - ID of the drive being replaced/hot spared
**
**  @return     pPDD - PDD of the replacement drive
**
******************************************************************************
**/

#define     FHS_SLOT                    0
#define     FHS_BAY                     1
#define     FHS_SAME_LOCATION_CODE      2

PDD        *RB_FindHotSpare(UINT64 capacity, UINT16 pid)
{
    UINT16      PDDIndex;
    PDD        *pPDD;
    UINT16      R_index;
    UINT64      bestCapacity[FHS_SAME_LOCATION_CODE + 1];
    PDD        *bestPDD[FHS_SAME_LOCATION_CODE];
    UINT16      hotSpareCount = 0;
    UINT8       bestChoiceType = 0xFF;

    /*
     * Increment the capacity that comes in to reflect the reserved area.
     * This prevents us from having to calculate all the capacities of
     * the hot spares as we process them.
     */
    capacity += RESERVED_AREA_SIZE;

    /* Initialize the best candidate values. */
    for (R_index = 0; R_index <= FHS_SAME_LOCATION_CODE; ++R_index)
    {
        bestCapacity[R_index] = 0xFFFFFFFFFFFFFFFFLL;
        bestPDD[R_index] = NULL;
    }

    /* For each valid physical disk device. */
    for (PDDIndex = 0; PDDIndex < MAX_PHYSICAL_DISKS; ++PDDIndex)
    {
        pPDD = gPDX.pdd[PDDIndex];

        /*
         * If not NULL, check the location of the drive and determine
         * the max accordingly. Do not allow a drive of one type to
         * spare to a drive of another type.
         */
        if ((pPDD != NULL) &&
            (pPDD->devClass == PD_HOTLAB) &&
            (pPDD->devType == gPDX.pdd[pid]->devType) &&
            (pPDD->devStat == PD_OP) &&
            (pPDD->ssn == K_ficb->vcgID) &&
            (!RB_ActDefCheck(pPDD)) && (pPDD->devCap != 0))
        {
            /* Update the counter of hot spares. */
            hotSpareCount++;

            /*
             * OK. The PDD is now pointing at a hot spare that is
             * operational and usable by the system. Check if the
             * hot spare is better than the ones we had so far.
             */
            if (pPDD->devCap >= capacity)
            {
                if ((pPDD->devName[PD_DNAME_CSES] == gPDX.pdd[pid]->devName[PD_DNAME_CSES]) &&
                    (pPDD->devName[PD_DNAME_CSLOT] == gPDX.pdd[pid]->devName[PD_DNAME_CSLOT]))
                {
                    bestCapacity[FHS_SLOT] = pPDD->devCap;
                    bestPDD[FHS_SLOT] = pPDD;
                    bestChoiceType = MIN(FHS_SLOT, bestChoiceType);
                }
                else if ((pPDD->devName[PD_DNAME_CSES] == gPDX.pdd[pid]->devName[PD_DNAME_CSES]) &&
                         (pPDD->devCap < bestCapacity[FHS_BAY]))
                {
                    bestCapacity[FHS_BAY] = pPDD->devCap;
                    bestPDD[FHS_BAY] = pPDD;
                    bestChoiceType = MIN(FHS_BAY, bestChoiceType);
                }
                else if ((pPDD->geoLocationId == gPDX.pdd[pid]->geoLocationId) &&
                         (pPDD->devCap < bestCapacity[FHS_SAME_LOCATION_CODE]))
                {
                    /*
                     * This is the case for any bay with the same location code
                     * as the failing PDD
                     */
                    bestCapacity[FHS_SAME_LOCATION_CODE] = pPDD->devCap;
                    bestPDD[FHS_SAME_LOCATION_CODE] = pPDD;
                    bestChoiceType = MIN(FHS_SAME_LOCATION_CODE, bestChoiceType);
                }
            }
        }
    }

    /* Decide on what to log and what to return. */
    if (bestChoiceType != 0xFF)
    {
        pPDD = bestPDD[bestChoiceType];

        /*
         * Check for hot spare depletion cases. These are checked in a separate
         * function so that they can be checked at other times also.
         */
        RB_CheckHSDepletion(pPDD);
        RB_CheckHSCapacity(pPDD);

        MSC_FlightRec(FR_H_MISC4, (UINT32)(pPDD->devCap >> 32), (UINT32)pPDD->devCap, (UINT32)pPDD);
    }
    else
    {
        pPDD = NULL;
    }

    return (pPDD);
}


/**
******************************************************************************
**
**  @brief      To provide a means of checking the hot spares to see
**              if they have been depleted.
**
**  @param      pPDD - if non-NULL, this is the PDD being used up and
**                     checks should be put in place to see if the pool
**                     or bay has been depleted.
**
**  @return     None.
**
******************************************************************************
**/
void RB_CheckHSDepletion(PDD *pInputPDD)
{
    UINT8       devType;

    for (devType = PD_DT_FC_DISK; devType <= PD_DT_MAX_DISK; ++devType)
    {
        RB_CheckHSDepletionWithType(pInputPDD, devType);
    }
}


/**
******************************************************************************
**
**  @brief      To provide a means of checking the hot spares to see
**              if they have been depleted.
**
**  @param      pPDD - if non-NULL, this is the PDD being used up and
**                     checks should be put in place to see if the pool
**                     or bay has been depleted.
**
**              devType - type of the drive being checked (SATA, FC, SSD)
**
**  @return     None.
**
******************************************************************************
**/
void RB_CheckHSDepletionWithType(PDD *pInputPDD, UINT8 devType)
{
    UINT16      R_index;
    PDD        *pPDD;
    UINT8       typeCount = 0;
    UINT16      numHS = 0;

    /*
     * Get the count of hotspares not including the one passed in since
     * it is going to be used.
     */
    for (R_index = 0; R_index < MAX_PHYSICAL_DISKS; ++R_index)
    {
        pPDD = gPDX.pdd[R_index];
        if ((pPDD != NULL) && (pPDD->devStat == PD_OP) && (pPDD->ssn == K_ficb->vcgID))
        {
            if (pPDD->devType == devType)
            {
                ++typeCount;
                if ((pPDD->devClass == PD_HOTLAB) &&
                    (pPDD != pInputPDD) && (!RB_ActDefCheck(pPDD)))
                {
                    numHS++;
                }
            }
        }
    }

    if (typeCount != 0)
    {
        /* For Nitrogen don't send the log message no enterprise hotspare */
#if defined(MODEL_3000) || defined(MODEL_7400)
        if ((numHS == 0) && (RBNoHS[devType] != RB_ERROR_POSTED))
        {
            RB_LogHSDepleted(HSD_CNC, 0, devType);
            RBNoHS[devType] = RB_ERROR_POSTED;
        }
        else if ((numHS != 0) && (RBNoHS[devType] == RB_ERROR_POSTED))
        {
            RB_LogHSDepleted(HSD_CNC_OK, 0, devType);
            RBNoHS[devType] = RB_OK;
        }
#endif /* MODEL_3000 || MODEL_7400 */
    }
}


/**
******************************************************************************
**
**  @brief      To provide a means of checking the hot spares to see
**              if there is a large enough hot spare to accommodate the
**              largest drive in use.
**
**  @param      pPDD - if non-NULL, this is the PDD being used up and
**                     checks should be put in place
**
**  @return     None.
**
******************************************************************************
**/
void RB_CheckHSCapacity(PDD *pInputPDD)
{
    UINT8       devType;

    for (devType = PD_DT_FC_DISK; devType <= PD_DT_MAX_DISK; ++devType)
    {
        RB_CheckHSCapacityWithType(pInputPDD, devType);
    }
}


/**
******************************************************************************
**
**  @brief      To provide a means of checking the hot spares to see
**              if there is a large enough hot spare to accommodate the
**              largest drive in use.
**
**  @param      pPDD - if non-NULL, this is the PDD being used up and
**                     checks should be put in place
**
**              devType - type of drive (FC, SATA, SSD, etc)
**
**  @return     None.
**
******************************************************************************
**/
void RB_CheckHSCapacityWithType(PDD *pInputPDD, UINT8 devType)
{
    UINT16      R_index;
    PDD        *pPDD;
    UINT16      typeCount = 0;
    UINT16      numHS = 0;
    UINT64      largestDrive = 0;
    UINT64      largestHS = 0;

    /* Get the best capacity and the largest usage in the PDisks. */
    for (R_index = 0; R_index < MAX_PHYSICAL_DISKS; ++R_index)
    {
        pPDD = gPDX.pdd[R_index];

        /* If not NULL, check the capacities. */
        if ((pPDD != NULL) && (pPDD->devStat == PD_OP) && (pPDD->ssn == K_ficb->vcgID))
        {
            if (pPDD->devType == devType)
            {
                ++typeCount;
                if (pPDD->devClass == PD_HOTLAB)
                {
                    if ((pPDD != pInputPDD) && (!RB_ActDefCheck(pPDD)))
                    {
                        numHS++;
                        if (pPDD->devCap > largestHS)
                        {
                            largestHS = pPDD->devCap;
                        }
                    }
                }
                else if (pPDD->devClass == PD_DATALAB)
                {
                    DA_CalcSpace(pPDD->pid, FALSE);
                    if ((pPDD->devCap - pPDD->tas) > largestDrive)
                    {
                        largestDrive = pPDD->devCap - pPDD->tas;
                    }
                }
            }
        }
    }

    if ((numHS != 0) && (typeCount != 0))
    {
        /*
         * Trim down the largest capacities to be in multiples of allocation
         * units to avoid misreporting the mismatch of sizes.
         */
        largestDrive = (largestDrive / DISK_SECTOR_ALLOC) * DISK_SECTOR_ALLOC;
        largestHS = (largestHS / DISK_SECTOR_ALLOC) * DISK_SECTOR_ALLOC;

        if (largestDrive > largestHS)
        {
            if (RBExcessState[devType] != RB_ERROR_POSTED)
            {
                RB_LogHSDepleted(HSD_TOO_SMALL, 0, devType);
                RBExcessState[devType] = RB_ERROR_POSTED;
            }
        }
        else if (RBExcessState[devType] == RB_ERROR_POSTED)
        {
            RB_LogHSDepleted(HSD_JUST_RIGHT, 0, devType);
            RBExcessState[devType] = RB_OK;
        }
    }
    else
    {
        /*
         * The number of hot spares is now zero, so obviously we need
         * to reset the error logging for this function so that if a
         * new hot spare is introduced, it will be checked and reported
         * correctly.
         */
        RBExcessState[devType] = RB_NO_STATE;
    }
}


/**
******************************************************************************
**
**  @brief       This increments rb_hotsparewait and forks the rb$hotsparewaittask
**               task if it's not already running.
**
**               When a new IO error is detected this routine will start a task
**               to delay x seconds before attempting to hotspare the drive or
**               check for RAIDs to rebuild.
**
**  @param      none.
**
**  @return     None.
**
******************************************************************************
**/
void RB_AcceptIOError(void)
{

    PDD        *pPDD;
    UINT32      R_index;
    UINT32      nonOpCount = 0;

    /*
     * Scan through the PDD list looking for NON-Existent drives. If more then
     * one is found (indication that we are possibly missing an entire bay
     * of drives, then delay 2 minutes before attempting the hotspare.
     */

    /* Get the best capacity and the largest usage in the PDisks. */
    for (R_index = 0; R_index < MAX_PHYSICAL_DISKS; ++R_index)
    {
        pPDD = gPDX.pdd[R_index];

        /* If not NULL, check the drive. */
        if ((pPDD != NULL) && (pPDD->devStat == PD_NONX))
        {
            ++nonOpCount;
        }
    }

    /*
     * Set up the Hotspare delay based on the number of non existent
     * drives that where detected.
     */
    if (nonOpCount > 1)
    {
        gHotspareWait = 120;
    }
    else
    {
        gHotspareWait = 1;
    }

    /* If task is being created, wait. */
    while (gHotspareWaitPCB == (PCB *)-1)
    {
        TaskSleepMS(50);
    }

    /* If the wait task is not started, then start it now. */
    if (gHotspareWaitPCB == NULL)
    {
        CT_fork_tmp = (unsigned long)"RB_HotspareWaitTask";
        gHotspareWaitPCB = (PCB *)-1;   // Flag task being created.
        gHotspareWaitPCB = TaskCreate2(RB_HotspareWaitTask, REBUILD_PRIORITY);
    }
}


/**
******************************************************************************
**
**  @brief       Method of determining if the passed Raid device is actively
**               requiring rebuilds before writes.
**
**  @param      RDD* - pointer to an RDD.
**
**  @return     false   - not actively rebuildable
**              true    - actively rebuildable
**
******************************************************************************
**/
bool RB_IsRaidRebuildWriteActive(RDD *pRDD)
{
    UINT32      rc = false;
    PSD        *pPSD;
    PSD        *pStartPSD;

    if ((pRDD != NULL) &&
        (BIT_TEST(pRDD->aStatus, RD_A_REBUILD_WRITES)) &&
        (BIT_TEST(pRDD->aStatus, RD_A_REBUILD)))
    {
        /* Get the first PSD in the list */
        pPSD = pStartPSD = *((PSD **)(pRDD + 1));

        /*
         * Loop through all devices. Find the rebuilding PSD and check to
         * see if the device is currently rebuilding (operable).
         */
        do
        {
            if (BIT_TEST(pPSD->aStatus, PSDA_REBUILD))
            {
                if (gPDX.pdd[pPSD->pid]->devStat == PD_OP)
                {
                    rc = true;
                    break;
                }
            }

            /* Get next PSD */
            pPSD = pPSD->npsd;
        } while (pPSD != pStartPSD);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief       Method of determining if the passed PSD is actively
**               requiring rebuilds before writes.
**
**  @param      PSD* - pointer to a PSD.
**
**  @return     false   - not actively rebuildable
**              true    - actively rebuildable
**
******************************************************************************
**/
bool RB_IsPSDRebuildWriteActive(PSD *pPSD)
{
    UINT32      rc = false;
    RDD        *pRDD;

    if (pPSD != NULL)
    {
        /*
         * If the PDD is not operable, flag the PSD that we are in a rebuild
         * wait state. This will force the rebuild write active state to
         * be synchrionized with a cache stop, preventing it from changing
         * asynchronous with a drive becoming operable.
         */
        if (gPDX.pdd[pPSD->pid]->devStat != PD_OP)
        {
            /*
             * If the wait bit is not set, set it and initiate an update
             * to propagate to the other controller
             */
            if (!(BIT_TEST(pPSD->aStatus, PSDA_REBUILD_WAIT)))
            {
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                BIT_SET(pPSD->aStatus, PSDA_REBUILD_WAIT);
                NV_P2Update();
            }

        }
        else if (BIT_TEST(pPSD->aStatus, PSDA_REBUILD_WAIT))
        {
            /*
             * Create a task to clear the rebuild wait flag, if a task is
             * not already active.
             */
            if (gClearRebuildWaitStateActive == false)
            {
                gClearRebuildWaitStateActive = true;
                CT_fork_tmp = (unsigned long)"RB_ClearRebuildWaitStateTask";
                TaskCreate2(C_label_referenced_in_i960asm(RB_ClearRebuildWaitStateTask),
                            REBLD_WRITE_PRIORITY);
            }
        }
        else
        {
            /* Get the RDD associate with this PSD */
            pRDD = gRDX.rdd[pPSD->rid];

            /*
             * Check that we:
             *  1. Have a valid RDD
             *  2. The RDD is flagged for Rebuild Writes
             *  3. The RDD is flagged for rebuild
             *  4. The PSD is flagged for rebuild
             *  5. The associated physical device is operable
             */
            if ((pRDD != NULL) &&
                (BIT_TEST(pRDD->aStatus, RD_A_REBUILD_WRITES)) &&
                (BIT_TEST(pRDD->aStatus, RD_A_REBUILD)) &&
                (BIT_TEST(pPSD->aStatus, PSDA_REBUILD)) &&
                (gPDX.pdd[pPSD->pid]->devStat == PD_OP))
            {
                rc = true;
            }
        }
    }
    return rc;
}


/**
******************************************************************************
**
**  @brief       Method to update raid alternate status indicating we are
**               entering or exiting a state where we need to resync (rebuild)
**               a stripe of data before writing the stripe.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RB_UpdateRaidRebuildWritesStatus(void)
{
    RDD        *pRDD;
    PSD        *pPSD;
    PSD        *pStartPSD;
    UINT16      rid;
    bool        rebuildsRequired;

    /* For each valid RAID device. */
    for (rid = 0; rid < MAX_RAIDS; ++rid)
    {
        /* Get the RDD and initialize the rebuilds flag. */
        pRDD = gRDX.rdd[rid];
        rebuildsRequired = false;

        /*
         * If the RDD is valid and we are the owners of this raid device,
         * then search through the PSDs to find one that is rebuilding.
         *
         * TBOLT00012739:   Removed check for ownership when setting the
         * the rebuild before write status. This eliminates a race condition to
         * get the bit set and propagated to the FE when a fail-over occurs and
         * the cache is being flushed.
         */
        if ((pRDD != NULL) && (pRDD->type != RD_SLINKDEV))      /* && (DL_AmIOwner(pRDD->vid))) */
        {
            /* Get the first PSD in the list */
            pPSD = pStartPSD = *((PSD **)(pRDD + 1));

            /*
             * Loop through all devices. Find the rebuilding PSD and check to
             * see if the device is currently rebuilding (operable).
             */
            do
            {
                if (BIT_TEST(pPSD->aStatus, PSDA_REBUILD))
                {
                    if (gPDX.pdd[pPSD->pid] && gPDX.pdd[pPSD->pid]->devStat == PD_OP)
                    {
                        rebuildsRequired = true;
                        break;
                    }
                }

                /* Get next PSD */
                pPSD = pPSD->npsd;

            } while (pPSD != pStartPSD);

            /*
             * If a rebuilding drive was found, then set the RDD flag indicating
             * a rebuild is required before any write.
             */
            BIT_SET(DMC_bits, CCB_DMC_raidcache);       /* Flag raid data has changed. */
            if (rebuildsRequired == true)
            {
                BIT_SET(pRDD->aStatus, RD_A_REBUILD_WRITES);
            }
            else
            {
                BIT_CLEAR(pRDD->aStatus, RD_A_REBUILD_WRITES);
            }
#if DEBUG_FLT_REC_HOT_SPARE
            MSC_FlightRec(FR_H_RBLDA, (UINT32)rid, (UINT32)pRDD->status, (UINT32)pRDD->aStatus);
#endif
        }
    }
}


/**
******************************************************************************
**
**  @brief       Task to update raid alternate status and optionally notifying
**               the FE processor that we are entering  or
**               exiting a state where we need to resync (rebuild) a stripe
**               of data before writing the stripe.
**
**  @param
**              updateRMTCache   false   no Stopio or update remote cache
**                               true    Stopio and update remote cache
**              p2Update         false   no p2update required
**                               true    p2update required
**
**  @return     none
**
******************************************************************************
**/
void RB_UpdateRebuildWriteStateTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED,
                                    bool updateRMTCache, bool p2Update)
{
    if (updateRMTCache == true)
    {
        /* Stop IO during the update */
        DEF_CacheStop(STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                      START_STOP_IO_USER_BE_REBUILD);

        RB_UpdateRaidRebuildWritesStatus();

        /* Update the FE processor */
        DEF_UpdRmtCache();

        /* Resume the IO */
        DEF_CacheResume(START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                        START_STOP_IO_USER_BE_REBUILD);
    }
    else
    {
        RB_UpdateRaidRebuildWritesStatus();
    }

    /* If requested, perform a P2 Update */
    if (p2Update == true)
    {
        NV_P2Update();
    }
}


/**
******************************************************************************
**
**  @brief       Method of update raid alternate status and optionally notifying
**               the FE processor that we are entering  or
**               exiting a state where we need to resync (rebuild) a stripe
**               of data before writing the stripe.
**
**  @param
**              updateRMTCache   false   no Stopio or update remote cache
**                               true    Stopio and update remote cache
**              p2Update         false   no p2update required
**                               true    p2update required
**
**  @return     none
**
**  @attention  Spawns task to avoid deadlocks with the raid error handling.
******************************************************************************
**/
void RB_UpdateRebuildWriteState(bool updateRMTCache, bool p2Update)
{
    CT_fork_tmp = (unsigned long)"RB_UpdateRebuildWriteStateTask";
    TaskCreate4(C_label_referenced_in_i960asm(RB_UpdateRebuildWriteStateTask),
                REBLD_WRITE_PRIORITY, updateRMTCache, p2Update);
}


/**
******************************************************************************
**
**  @brief       Task to determine if the RAID AStatus Local Image In Progress
**               bit has been turned off within 15 seconds of setting it.
**               If it has not gone off in 15 seconds, send up another P2Update
**               to try and get the image to the master successfully.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RL_LocalImageIPCheck(void)
{
    RDD        *pRDD;           /* RDD being Processed at the time  */
    bool        continueFlag = false;   /* Loop Continue flag               */
    UINT16      rid;            /* RAID ID                          */

    do
    {
        /*
         * Sleep 15 seconds and then check all the RAIDs to see if any have
         * the Additional Status Local Image In Progress bit on. If so,
         * do another P2 Update to get the NVRAM config sent again.
         */
        TaskSleepMS(15000);

        continueFlag = false;   /* Reset flag each time through loop */

        for (rid = 0; ((rid < MAX_RAIDS) && (continueFlag == false)); ++rid)
        {
            pRDD = R_rddindx[rid];
            if ((pRDD != NULL) && (BIT_TEST(pRDD->aStatus, RD_A_LOCAL_IMAGE_IP)))
            {
                /*
                 * Found a RAID that has the Local Image in Progress bit still
                 * on. Do a P2 Update and continue waiting
                 */
                continueFlag = true;
                NV_P2Update();
            }
        }
    } while (continueFlag);

    /* All done, clear out the PCB so another task can be started when needed */
    gRLLocalImageIPCheckPCB = 0;
}


/**
******************************************************************************
**
**  @brief       Clear the Rebuild Wait flag in the PSD. This flag signals a
**               transition in the PDD from a non OP state to an OP state. The
**               clearing of this flag is done with cache operations stopped,
**               to prevent split raid operations and rebuilds before writes
**               to be exposed to the transition.
**
**  @param
**              pPSD             pointer to the PSD
**
**  @return     none
**
******************************************************************************
**/
void RB_ClearRebuildWaitStateTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED)
{
    RDD        *pRDD;
    PSD        *pPSD;
    PSD        *pStartPSD;
    UINT16      rid;
    bool        p2Update = false;

    /* Stop IO during the update */
    DEF_CacheStop(STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                  START_STOP_IO_USER_BE_REBUILD);

    /* For each valid RAID device. */
    for (rid = 0; rid < MAX_RAIDS; ++rid)
    {
        /* Get the RDD and initialize the rebuilds flag */
        pRDD = gRDX.rdd[rid];

        /*
         * If the RDD is valid and we are the owners of this raid device,
         * then search through the PSDs to find one that is rebuilding.
         */
        if ((pRDD != NULL) && (pRDD->type != RD_SLINKDEV) && (DL_AmIOwner(pRDD->vid)))
        {
            /* Get the first PSD in the list */
            pPSD = pStartPSD = *((PSD **)(pRDD + 1));

            /*
             * Loop through all devices. Find the PSDs that are in a
             * rebuild wait state and there drive is now operable.
             */
            do
            {
                if (BIT_TEST(pPSD->aStatus, PSDA_REBUILD_WAIT))
                {
                    if (gPDX.pdd[pPSD->pid]->devStat == PD_OP)
                    {
                        /*
                         * Clear the Rebuild wait flag and flag that a
                         * P2 update is required
                         */
                        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                        BIT_CLEAR(pPSD->aStatus, PSDA_REBUILD_WAIT);
                        p2Update = true;
                    }
                }

                /* Get next PSD */
                pPSD = pPSD->npsd;
            } while (pPSD != pStartPSD);
        }
    }

    /*
     * Clear the active flag, now that we are done processing the PSDs. Do
     * this before the context switch in the cache resume, to allow a
     * new task to start (if needed).
     */
    gClearRebuildWaitStateActive = false;

    /* Resume the IO */
    DEF_CacheResume(START_IO_OPTION_CLEAR_ONE_STOP_COUNT, START_STOP_IO_USER_BE_REBUILD);

    /* If requested, perform a P2 Update */
    if (p2Update == true)
    {
        NV_P2Update();

    }
}


/**
******************************************************************************
**
**  @brief      This function calculates the percentage remaining in a
**              rebuild or defrag.
**
**              If the rebuild is a real rebuild, just use the values in
**              the PDD for amount remaining. This calculation is based
**              upon:
**
**                  % remaining = blocks remaining to be rebuilt /
**                                (device capacity - total available space)
**
**              If the rebuild is defrag generated, then use the amount
**              in the current rebuild plus the amount in the rest of the
**              disk past the current rebuild. This may not be the amount
**              to really be moved, but it is more accurate than the other
**              calculation.
**
**  @param      pPDD - pointer to the PDD on which the calculation is to be done
**
**  @return     none
**
**  @attention  pPDD->pctRem modified.
**
******************************************************************************
**/
void RB_CalcPercentRemaining(PDD *pPDD)
{
    UINT64      denominator;
    UINT64      numerator = 0;
    DAMLX      *pDAMLX = NULL;

    denominator = pPDD->devCap - RESERVED_AREA_SIZE - pPDD->tas;

    /* Two cases. First is a defrag operation. Second is a rebuild. */
    if (BIT_TEST(pPDD->miscStat, PD_MB_DEFRAGGING) && (pPDD->las != pPDD->tas))
    {
        /*
         * Grab the capacity up to the point of the first gap in the drive
         * maps and then use it and the remaining count to determine how
         * much space is left to defrag.
         */
        DA_CalcSpace(pPDD->pid, FALSE);

        pDAMLX = &(pPDD->pDAML->damlx[pPDD->pDAML->firstGap]);

        if (pPDD->rbRemain != 0)
        {
            numerator = denominator - (((pDAMLX->auSda + pDAMLX->auSLen) * DISK_SECTOR_ALLOC)
                 - RESERVED_AREA_SIZE - pPDD->rbRemain);
        }
    }
    else
    {
        /*
         * Rebuild case (or done with other cases). Take the remaining
         * block count in the PDD as the numerator and the capacity of
         * the drive in use as the denominator. Round up also.
         */
        numerator = pPDD->rbRemain;
    }

    /* Make sure that the data has not already been moved from the device. */
    if (denominator == 0)
    {
        pPDD->pctRem = 0;
    }
    else
    {
        pPDD->pctRem = ((100 * numerator) + denominator - 1) / denominator;
    }
}


/**
******************************************************************************
**
**  @brief      This function checks the intergrity of the rebuild remaining
**              values for all disks in the rebuild queue. It is debug only.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RB_CheckRBRemainingIntegrity(void)
{
    RBR        *pRBR;
    UINT64      rbRemain;
    UINT16      pid;

    /*
     * For each PID in the system, check the list of RBR records and sum
     * the blocks remaining. If the count remaining does not match the
     * count in the PDD, then flag an error.
     */
    if (gRBRHead != NULL)
    {
        for (pid = 0; pid < MAX_PHYSICAL_DISKS; ++pid)
        {
            if (P_pddindx[pid] != NULL)
            {
                /*
                 * Find all PID matches and then add in the count from the
                 * RBR. There may be partials, so make sure it gets added
                 * in correctly.
                 */
                pRBR = gRBRHead;
                rbRemain = 0;

                do
                {
                    if (pRBR->psd->pid == pid)
                    {
                        rbRemain += pRBR->rlen;
                    }
                    pRBR = pRBR->nextRBR;
                } while (pRBR != NULL);

//                /* Now check the value with the PDD. */
//                if (rbRemain != P_pddindx[pid]->rbRemain)
//                {
//                    ERP_Halt();
//                }
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      This function either unfails the specified hotspare back to the
**              data drive from which slot the data drive is failed. (or)
**              cancels the unfail operation for this drive.
**
**  @param      hspid ----- Hotspare PID to be unfailed
**
**  @return     None
**
******************************************************************************
**/
void RB_CheckPDDForFailBack(PDD *pInsertedPDD)
{
    UINT32      failedLocation;
    UINT8       failedSlot;
    UINT16      failedSes;
    PDD        *phsPDD;
    int         count = 0;
    int         idx;

    /*
     *  Now process through all the drives and try to find for any used hotspare that is to be
     *  failedback to this newly inserted drive.
     */
    for (idx = 0; ((idx < MAX_PHYSICAL_DISKS) && (count <= gPDX.count)); ++idx)
    {
        phsPDD = gPDX.pdd[idx];
        if (phsPDD)
        {
            count++;
        }
        else
        {
            continue;
        }
        if (phsPDD->pid == pInsertedPDD->pid)
        {
            /* If it's the same disk, skip it. */
            continue;
        }
        if ((phsPDD->devClass == PD_HOTLAB) &&
            (*(UINT32 *)(&(phsPDD->hsDevName[0])) != 0))
        {
            failedLocation = *(UINT32 *)(&(phsPDD->hsDevName[0]));
            failedSlot = (failedLocation & 0xFF000000) >> 24;
            failedSes = (failedLocation & 0x00FF0000) >> 16;

            if ((pInsertedPDD->ses == failedSes) && (pInsertedPDD->slot == failedSlot))
            {
                /*
                 * Spawn the task to rebuild back the data from the source hotspare drive on
                 * to the newly inserted drive from which originally data has been hotspared
                 */
                if (BIT_TEST(pInsertedPDD->flags, PD_AFAILBACK_IN_PROGRESS))
                {
                    fprintf(stderr, "%s - Rebuild already in progress for PID %d\n",
                            __func__, pInsertedPDD->pid);
                    return;
                }
                BIT_SET(pInsertedPDD->flags, PD_AFAILBACK_IN_PROGRESS);

                CT_fork_tmp = (unsigned long)"RB_pdiskAutoFailBackTask";
                TaskCreate4(C_label_referenced_in_i960asm(RB_pdiskAutoFailBackTask),
                            REBUILD_PRIORITY,
                            (UINT32)(UINT32 *)phsPDD, (UINT32)(UINT32 *)pInsertedPDD);
                return;
            }
        }
    }

    /*
     * This new drive does not need any failback, as it did not find any hotspare on which
     * the data from this location (if new drive) has been rebuilt. Log it.
     */
    fprintf(stderr, "%s - No Autofailback is being invoked for the new disk"
            " (PID %d), since there are no valid hotspares on which"
            " data from this drive was rebuilt\n", __func__, pInsertedPDD->pid);
}


/**
******************************************************************************
**
**  @brief      This function either unfails the specified hotspare back to the
**              data drive from which slot the data drive is failed. (or)
**              cancels the unfail operation for this drive.
**
**  @param      sourceHsPDD        -- Hotspare PDD from where failback happens
**              destinationNewPDD  -- Destination(new) drive onto which rebuild happens
**
**  @return     None
**
******************************************************************************
**/
void RB_pdiskAutoFailBackTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED,
                              UINT32 sourceHsPDD, UINT32 destinationNewPDD)
{
    UINT8       retVal;
    PDD        *pSourceHsPDD = (PDD *)(UINT32 *)sourceHsPDD;
    PDD        *pDestinationNewPDD = (PDD *)(UINT32 *)destinationNewPDD;
    LOG_AUTO_FAILBACK_INFO_PKT iPkt;

    /* Now really rebuild the hotspare back to the newly inserted drive */
    retVal = RB_pdiskRebuildBack(pSourceHsPDD, pDestinationNewPDD);

    /*
     * Clear the "task in progress" bit so that another rebuild task will
     * be spawned, if required.
     */
    BIT_CLEAR(pDestinationNewPDD->flags, PD_AFAILBACK_IN_PROGRESS);

    /* Clear the flags if rebuild succeeded. */
    if (retVal == DEOK)
    {
        if (BIT_TEST(pDestinationNewPDD->flags, PD_USER_INSERTED))
        {
            BIT_CLEAR(pDestinationNewPDD->flags, PD_USER_INSERTED);
        }
        if (BIT_TEST(pDestinationNewPDD->flags, PD_USER_REATTACHED))
        {
            BIT_CLEAR(pDestinationNewPDD->flags, PD_USER_REATTACHED);
        }
    }

    /* Send a log message */
    iPkt.header.event = LOG_AUTO_FAILBACK;
    iPkt.status = retVal;
    iPkt.hswwn = pSourceHsPDD->wwn;
    iPkt.dstwwn = pDestinationNewPDD->wwn;

    /* Note: message is short, and L$send_packet copies into the MRP. */
    MSC_LogMessageStack(&iPkt, sizeof(LOG_AUTO_FAILBACK_INFO_PKT));
}


/**
******************************************************************************
**
**  @brief      This function either unfails the specified hotspare back to
**              the data drive in the slot from which the drive is failed, or,
**              cancels the unfail operation for this drive.
**              (Manual failback through MRP processing)
**
**  @param      hspid   ---  Hotspare PID to be unfailed
**              options ---  Bit -1 (if set cancels the unfail op)
**
**  @return     status
**
******************************************************************************
**/
UINT8 RB_pdiskFailBack(UINT16 hspid, UINT8 options)
{
    PDD        *pPDD = NULL;
    PDD        *pFailedDataPDD;

    /* Check whether the PID is within the range or exists or hotspare device */
    if ((hspid >= MAX_PHYSICAL_DISKS) || ((pPDD = gPDX.pdd[hspid]) == NULL))
    {
        /* PID is out of range or does not exists .. to send a log message */
        return (DEINVPID);
    }

    if (pPDD->devClass != PD_HOTLAB)
    {
        return (DENOTHOTLAB);
    }

    /* Check if the faild PDD information is available or failback is cancelled */
    if ((*(UINT32 *)(&(pPDD->hsDevName[0]))) == 0)
    {
        /*
         * Failback information not available or failback cancelled/already-done--
         * <to send a log message- TBD >
         */
        return (DENOHSDNAME);
    }

    /* Check if 'failback cancel' bit is set -- user requested for failback cancel */
    if (options & FBP_FAILBACK_CANCEL)
    {
        /* Nullifying failback information --  store changed value in NVRAM */
        memset(pPDD->hsDevName, 0, 4);
        NV_P2UpdateConfig();

        return (DEOK);
    }

    /*
     * Get the failed data device from where hotspare occurred --
     * we are infact getting the data drive from the slot from
     * where the data drive is failed over to this hotspare.
     */
    pFailedDataPDD = RB_GetFailedDataPDD(pPDD);

    /*
     * Ensure that the drive exists
     * Ensure also that it is really a disk drive
     */
    if ((pFailedDataPDD == NULL) || (pFailedDataPDD->devType > PD_DT_MAX_DISK))
    {
        return (DENOFAILBACKPDD);
    }

    /* Now really rebuild the hotspare back to the newly inserted drive */
    return (RB_pdiskRebuildBack(pPDD, pFailedDataPDD));
}


/**
******************************************************************************
**
**  @brief  Rebuild hotspare back to inserted drive
**
**  @param  pSourceHsPDD        -- Hotspare PDD from where failback happens
**          pDestinationNewPDD  -- Destination(new) drive onto which
**                                 rebuild happens
**
******************************************************************************
**/
UINT8 RB_pdiskRebuildBack(PDD *pSourceHsPDD, PDD *pDestinationNewPDD)
{
    PSD        *pSourcePSD;
    UINT64      sourceDevCapacity;
    UINT64      destinationDevCapacity;
    LOG_AUTO_FAILBACK_INFO_PKT iPkt;

    /*
     * The prerequisite to perform autofailback or failback is that the
     * designated destination device must be labeled as HOTSPARE.
     *
     * Make sure that the newly inserted drive is labeled as HOTSPARE.
     * If not, do not proceed further and return with an error saying
     * that the destination device is not labelled as HOTSPARE.
     */
    if (pDestinationNewPDD->devClass != PD_HOTLAB)
    {
        /* It may be needed to send a log message to the user */
        fprintf(stderr, "%s - Destination device is not labeled as HOTSPARE\n", __func__);
        return DEDESTNOTHOTLAB;
    }

    /*
     * TBolt 00018879
     * Make sure that the device type of the drive on to which the data is
     * going to rebuild back is of the same type as that of the device type of
     * the Hotspare.
     * We don't allow hotspare from one type of drive on to a different type
     * of drive. For example, from economy drive to economy enterprise drive
     */
    if (pSourceHsPDD->devType != pDestinationNewPDD->devType)
    {
        /* If yes, don't continue to rebuild the data */
        return DETYPEMISMATCH;
    }

    /*
     * Check if SES is in progress. If it is, don't hotspare back the
     * data on to the inserted drive
     * This is to avoid the race condition (TBolt00014397)
     */
    if (TaskGetState(S_bgppcb) != PCB_NOT_READY)
    {
        return DESESINPROGRESS;
    }

    /*
     * Check if the newly inserted drive is from any other system
     * If yes, we don't allow rebuild back on to this drive
     * We allow failback only when the user relabels this drive
     * TBolt00014397
     */
    if (pDestinationNewPDD->ssn != K_ficb->vcgID && pDestinationNewPDD->ssn != 0)
    {
        /*
         * Log a message indicating mismatch of serial numbers of the drive
         * and that of the controller.
         * Store the WWN of the hotspare and the inserted drive
         */
        iPkt.header.event = LOG_RB_FAILBACK_CTLR_MISMATCH;
        iPkt.hswwn = pSourceHsPDD->wwn;
        iPkt.dstwwn = pDestinationNewPDD->wwn;

        /* Note: message is short, and L$send_packet copies into the MRP. */
        MSC_LogMessageStack(&iPkt, sizeof(LOG_AUTO_FAILBACK_INFO_PKT));
        return DEINVCTRL;
    }

    /* Check if any data exists on the hotspare (source)..data should exist */
    pSourcePSD = DC_ConvPDD2PSD(pSourceHsPDD);
    if (pSourcePSD == NULL)
    {
        /* No raid is existing on this drive .. to send a log message */
        return DEPIDNOTUSED;
    }

    /*
     * Determine if the failing  back device(destination) is used by any
     * RAID on any controller it should be empty.
     */
    if (DC_ConvPDD2PSD(pDestinationNewPDD))
    {
        /* The drive contains data(raids) */
        return DEPIDUSED;
    }

    /*
     * Ensure that there is sufficient redundancy for the raids in
     * the(source) drive to be rebuilt back on to the (destination)
     * data drive
     */
    if (RB_CanSpare(pSourcePSD) != DEOK)
    {
        return DEINSUFFREDUND;
    }

    /*
     * Make sure that the data drive on which rebuidling back, has enough
     * capacity. Obtain the capacity of the drive which we are going to
     * failback (source). Obtain the capacity of the drive on which we are
     * rebuilding back(destination).
     * If the destination has not enough space, return error.
     */
    sourceDevCapacity = RB_FindCapacity(pSourceHsPDD->pid);
    destinationDevCapacity = pDestinationNewPDD->devCap;

    if (destinationDevCapacity < sourceDevCapacity)
    {
        return DEINSDEVCAP;
    }

    /* Write the fail label on to the HotSpare */
    fprintf(stderr, "%s - Going to write the fail label on the PID = %d\n", __func__,
            pSourceHsPDD->pid);
    pSourceHsPDD->failedLED = PD_LED_FAIL;

    ON_LedChanged(pSourceHsPDD);

    /*
     * Set the device status of the used HotSpare to inoperative
     * This is to avoid extra writes on the used hotspare during
     * rebuild to the original data drive is in progress.
     */
    fprintf(stderr, "%s - Going to change the state of  PID = %d to INOP\n", __func__,
            pSourceHsPDD->pid);
    pSourceHsPDD->devStat = PD_INOP;

    /*
     * SAN-1501
     * Set the AutoFailBack in progress bit on the source HS PDD.
     * This bit gets cleared just before making the HS PDD into OP state
     * once the data is completely failed back.
     */
    BIT_SET(pSourceHsPDD->flags, PD_AFAILBACK_IN_PROGRESS);
    fprintf(stderr, "%s - AFAILBACK in progress bit set for PID = %d\n", __func__,
            pSourceHsPDD->pid);

    /*
     * Send a datagram to the slave controller to mark the HotSpare drive
     * as non operational. This is basically to avoid extra writes on this
     * drive during failback is in progress
     */
    RB_SendNonOpEventToSlave(pSourceHsPDD);

    /* Spawn the task to write the failed label on to the used HotSpare */
    CT_fork_tmp = (unsigned long)"ON_WriteFailedLabel";

    TaskCreate4(&ON_WriteFailedLabel,
                OSPINDOWNPRIO, (UINT32)XD_FAIL_GEN, (UINT32)(UINT32 *)pSourceHsPDD);

    /* Ya. Its time to rebuild */
    fprintf(stderr, "%s - Going rebuild the data back from PID = %d on to PID  = %d to INOP\n",
            __func__, pSourceHsPDD->pid, pDestinationNewPDD->pid);
    RB_RedirectPSD(pSourcePSD, pDestinationNewPDD);

    return DEOK;
}


/**
******************************************************************************
**
**  @brief   Put the used hotspare drive back to operatinal state
**
**  @param   pdestinationPDD --  PDD on which  data rebuilt
**
**  @return  None
**
******************************************************************************
**/
void RB_ReInitUsedHotSpare(PDD *pdestinationPDD)
{
    UINT16      i;
    PDD        *psourceHsPDD = NULL;
    VDD        *pVDD = NULL;
    RDD        *pRDD = NULL;
    PSD        *pPSD = NULL;
    PSD        *startpsd;
    UINT8       flag = FALSE;
    UINT8       foundHS = FALSE;

    /*
     * It seems rebuild is done with all the PSDs. Now, it is time
     * to do further processing
     */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        psourceHsPDD = gPDX.pdd[i];

        /*
         * Make sure that the HSDEVNAME stored on the HotSpare indicates
         * that the data was previously failed over to this HotSpare.
         */
        if ((psourceHsPDD != NULL) &&
            (psourceHsPDD->devClass == PD_HOTLAB) &&
            (psourceHsPDD->hsDevName[2] == pdestinationPDD->ses) &&
            (psourceHsPDD->hsDevName[3] == pdestinationPDD->slot))
        {
            fprintf(stderr, "%s - Source PID = %d(%d) Dest PID = %d(%d)\n", __func__,
                    psourceHsPDD->pid, psourceHsPDD->devClass, pdestinationPDD->pid,
                    pdestinationPDD->devClass);
            foundHS = TRUE;
            break;
        }
    }

    /*
     * SAN-1549
     * The following check is required to distinguish between rebuild
     * is because of issuing "PDISKFAIL" or because of "PDISKAUTOFAILBACK".
     * If the rebuild is due to "PDISKFAIL", then the following check makes
     * us to bailout from executing the logic below this check.
     */
    if (foundHS == FALSE)
    {
        return;
    }

    /*
     *  Find if the rebuild is really completed. This routine gets called from
     *  both the Master and Slave controllers. It is required to find if rebuild
     *  is completed by both the Master and Slave controllers.
     *  The "flag" will be set to true if rebuild is to be done for a PSD. This
     *  "flag" will be false only when all the PSDs are done with the rebuilds.
     *  Rebuild gets completed early by that controller which has got less number
     *  of virtual disks comapred to the other controller.
     */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        pVDD = gVDX.vdd[i];

        if (pVDD != NULL)
        {
            pRDD = pVDD->pRDD;
            while ((pRDD != NULL) && (flag != TRUE))
            {
                startpsd = pPSD = *((PSD **)(pRDD + 1));
                do
                {
                    if ((pdestinationPDD->pid == pPSD->pid) &&
                        (BIT_TEST(pPSD->aStatus, PSDA_REBUILD)))
                    {
                        fprintf(stderr, "%s - Rebuild Astatus = %d for the RID = %d\n",
                                __func__, pPSD->aStatus, pRDD->rid);
                        flag = TRUE;
                        break;
                    }
                    pPSD = pPSD->npsd;  /* Next PSD */
                } while (startpsd != pPSD);
                pRDD = pRDD->pNRDD;
            }
        }
    }
    if (flag != TRUE)
    {
        /*
         * It is time to put the HotSpare drive into operational state.
         * Directly make the HotSpare into operational state if this is the Master
         * controller as the File System updates are allowed from the Master.
         */
        if (BIT_TEST(K_ii.status, II_MASTER))
        {
            RB_MakeHotSpareOperational(psourceHsPDD, pdestinationPDD);

            /*
             * Send a datagram to the slave controller to put the
             * source HOTSPARE device back in OP state.
             */
            RB_SendUpdateEventToSlave(psourceHsPDD, pdestinationPDD);
        }
        else
        {
            /*
             * This is the Slave controller. Therefore, send a datagram to
             * the Master to update the file system etc,.
             * File system gets updated only by the Master controller
             */
            RB_SendUpdateEventToMaster(psourceHsPDD, pdestinationPDD);

            /*
             * File system got updated by the Master. Now, It is time to make the
             * HotSpare as operational
             */
            psourceHsPDD->devStat = PD_OP;

            /*
             * SAN-1501
             * Clear the AutoFailback in progress bit on the source HS PDD
             */
            BIT_CLEAR(psourceHsPDD->flags, PD_AFAILBACK_IN_PROGRESS);
            fprintf(stderr, "%s - AFAILBACK in progress bit cleared for PID = %d\n",
                    __func__, psourceHsPDD->pid);

            /* Clear HSDName information -- Earlier it was really this. */
            memset(psourceHsPDD->hsDevName, 0, 4);

            /* Set the hsDevname in 'now' data drive as 0 */
            memset(pdestinationPDD->hsDevName, 0, 4);

            /*
             * Clear the misc status bits as the Master controller
             * has already cleared these bits after receving the
             * datagram event from this (slave) controller
             */
            fprintf(stderr, "%s - Clearing FS error bit\n", __func__);

            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SERIALCHANGE);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SCHEDREBUILD);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REBUILDING);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REPLACE);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_FSERROR);
        }
    }
    else
    {
        fprintf(stderr, "%s - Rebuild is not yet complete\n", __func__);
    }
}


/**
******************************************************************************
**
**  @brief      This routine sends (by the Master controller) datagram event to
**              the slave controller to put the HOTSPARE device in INOP state.
**
**  @param      psourceHsPDD   --  Source HOTSPARE
**
**  @return     None
**
******************************************************************************
**/
void RB_SendNonOpEventToSlave(PDD *psourceHsPDD)
{
    fprintf(stderr, "%s - Sending INOP event to Slave controller\n", __func__);

    gFailBackPkt.ccsm_event.len = sizeof(gFailBackPkt);
    gFailBackPkt.ccsm_event.type = RB_INOP_EVT;
    gFailBackPkt.ccsm_event.fc = RB_HS_INOP;
    gFailBackPkt.ccsm_event.sendsn = K_ficb->cSerial;
    gFailBackPkt.failBack.HotSparePid = psourceHsPDD->pid;
    gFailBackPkt.failBack.DestPid = 0;

    DEF_ReportEvent(&gFailBackPkt, sizeof(gFailBackPkt), PUTDG, DGBTSLAVE, 0);
}


/**
******************************************************************************
**
**  @brief      This routine is the datagram event handler (sent by the Master)
**              for the slave controller to  put the HOTSPARE device in INOP
**              state as indicated by the Master controller
**
**  @param      pReq    --- Datagram request packet.
**
**  @return     None
**
******************************************************************************
**/
void RB_NonOpEventHandler(void *pReq, UINT32 len UNUSED)
{
    RB_FAILBACK_UPDATE *pReqPkt;
    PDD        *pSourceHsPDD;

    pReqPkt = (RB_FAILBACK_UPDATE *)pReq;
    if (pReqPkt->ccsm_event.fc == RB_HS_INOP)
    {
        pSourceHsPDD = gPDX.pdd[pReqPkt->failBack.HotSparePid];

        fprintf(stderr, "%s - Recvd HS PID = %d\n", __func__, pSourceHsPDD->pid);

        pSourceHsPDD->failedLED = PD_LED_FAIL;

        /*
         * Spawn the task to write the failed label on to the
         * used HotSpare
         */
        CT_fork_tmp = (unsigned long)"ON_WriteFailedLabel";

        TaskCreate4(&ON_WriteFailedLabel,
                    OSPINDOWNPRIO, (UINT32)XD_FAIL_GEN, (UINT32)(UINT32 *)pSourceHsPDD);

        /* Set the device status of the used HotSpare to inoperative */
        fprintf(stderr, "%s - Going to change the state of  PID = %d to INOP\n", __func__,
                pSourceHsPDD->pid);
        pSourceHsPDD->devStat = PD_INOP;

        /*
         * SAN-1501
         * Set the AutoFailBack in progress bit on the source HS PDD.
         * This bit gets cleared just before making the HS PDD into OP state
         * once the data is completely failed back.
         */
        BIT_SET(pSourceHsPDD->flags, PD_AFAILBACK_IN_PROGRESS);
        fprintf(stderr, "%s - AFAILBACK in progress bit set for PID = %d\n", __func__,
                pSourceHsPDD->pid);
    }
}


/**
******************************************************************************
**
**  @brief     This routine send a datagram event to the Master controller
**             to get the file system updated and put the HOTSPARE drive
**             back into operational state.
**
**             NOTE: File system gets updated only by the Master.
**
**  @param     psourceHsPDD     --  Source HOTSPARE PDD
**             pdestinationPDD  --  Destination PDD
**
**  @return    None
**
******************************************************************************
**/
void RB_SendUpdateEventToMaster(PDD *psourceHsPDD, PDD *pdestinationPDD)
{
    fprintf(stderr, "%s - Sending Update event to Master controller\n", __func__);

    gFailBackPkt.ccsm_event.len = sizeof(gFailBackPkt);
    gFailBackPkt.ccsm_event.type = RB_EVT_FAILBACK;
    gFailBackPkt.ccsm_event.fc = RB_FAILBACK_COMP;
    gFailBackPkt.ccsm_event.sendsn = K_ficb->cSerial;
    gFailBackPkt.failBack.HotSparePid = psourceHsPDD->pid;
    gFailBackPkt.failBack.DestPid = pdestinationPDD->pid;

    DEF_ReportEvent(&gFailBackPkt, sizeof(gFailBackPkt), PUTDG, DGBTMASTER, 0);
}


/**
******************************************************************************
**
**  @brief     This routine send a datagram event to the Slave controller
**             to put the HOTSPARE drive back into operational state.
**
**             NOTE: File system gets updated only by the Master.
**
**  @param     psourceHsPDD     --  Source HOTSPARE PDD
**             pdestinationPDD  --  Destination PDD
**
**  @return    None
**
******************************************************************************
**/
void RB_SendUpdateEventToSlave(PDD *psourceHsPDD, PDD *pdestinationPDD)
{
    fprintf(stderr, "%s - Sending Update event to Slave controller\n", __func__);

    gFailBackPkt.ccsm_event.len = sizeof(gFailBackPkt);
    gFailBackPkt.ccsm_event.type = RB_EVT_FAILBACK;
    gFailBackPkt.ccsm_event.fc = RB_FAILBACK_COMP;
    gFailBackPkt.ccsm_event.sendsn = K_ficb->cSerial;
    gFailBackPkt.failBack.HotSparePid = psourceHsPDD->pid;
    gFailBackPkt.failBack.DestPid = pdestinationPDD->pid;

    DEF_ReportEvent(&gFailBackPkt, sizeof(gFailBackPkt), PUTDG, DGBTSLAVE, 0);
}


/**
******************************************************************************
**
**  @brief      This routine is the Master's datagram event (sent by the slave)
**              handler
**
**  @param      void*  -- Request packet
**
**  @return     None
**
******************************************************************************
**/
void RB_FailBackEventHandler(void *pReq, UINT32 len UNUSED)
{
    RB_FAILBACK_UPDATE *pReqPkt;
    PDD        *psourceHsPDD;
    PDD        *pdestinationPDD;

    pReqPkt = (RB_FAILBACK_UPDATE *)pReq;
    if (pReqPkt->ccsm_event.fc == RB_FAILBACK_COMP)
    {
        psourceHsPDD = gPDX.pdd[pReqPkt->failBack.HotSparePid];
        pdestinationPDD = gPDX.pdd[pReqPkt->failBack.DestPid];

        fprintf(stderr, "%s - Recvd HS PID = %d\n", __func__, psourceHsPDD->pid);
        fprintf(stderr, "%s - Recvd DEST PID = %d\n", __func__, pdestinationPDD->pid);

        /*
         * Get the file system updated and put the HOTSPARE device
         * back into OP state
         */
        RB_MakeHotSpareOperational(psourceHsPDD, pdestinationPDD);
    }
    else
    {
        fprintf(stderr, "%s - Both the controllers are not in OP state\n", __func__);
    }
}


/**
******************************************************************************
**
**  @brief      This function labels the destination device back as DATA
**              Remember that this device was originally a DATA device. Therefore,
**              marking this device back as DATA device.
**
**  @param      pdestinationPDD -- destination PDD
**
**  @return     None
**
******************************************************************************
**/
void RB_LabelDestDrive(PDD *pdestinationPDD)
{
    UINT8       retCode;
    void       *pBuffer;
    UINT32      devName;

    devName = 0x00004450 | ((pdestinationPDD->ses & 0xff) << 16) | (pdestinationPDD->slot << 24);
    pdestinationPDD->devClass = PD_DATALAB;
    memcpy(pdestinationPDD->devName, &devName, sizeof(devName));

    /* Now write the label. */
    pBuffer = s_MallocC(BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);

    /* Copy the default label text into the buffer. */
    memcpy(((XDL *)pBuffer)->text, &O_devlab, XD_LABEL_LEN);
    memcpy(((XDL *)pBuffer)->devName, &devName, sizeof(devName));

    ((XDL *)pBuffer)->sSerial = K_ficb->vcgID;
    ((XDL *)pBuffer)->devClass = PD_DATALAB;
    ((XDL *)pBuffer)->wwn = pdestinationPDD->wwn;

    retCode = FS_WriteFile(FID_LABEL, pBuffer, FS_SIZE_LABEL, pdestinationPDD, 1);

    s_Free(pBuffer, BYTES_PER_SECTOR * FS_SIZE_LABEL, __FILE__, __LINE__);
}


/**
******************************************************************************
**
**  @brief   This routine puts the HotSpare drive back into OP state.
**           Note that this was put into INOP state just before the data
**           failed back on to the original device
**
**  @param   psourceHsPDD    - HotSpare PDD
**           pdestinationPDD - Destination PDD
**
**  @return  None
**
******************************************************************************
**/
void RB_MakeHotSpareOperational(PDD *psourceHsPDD, PDD *pdestinationPDD)
{
    UINT8       retCode;

    fprintf(stderr, "%s  -Received HotSpare PID = %d\n", __func__, psourceHsPDD->pid);
    fprintf(stderr, "%s  -Received Dest PID = %d\n", __func__, pdestinationPDD->pid);

    /* SAN-1501  Clear the AutoFailback in progress bit on the source HS PDD */
    BIT_CLEAR(psourceHsPDD->flags, PD_AFAILBACK_IN_PROGRESS);
    fprintf(stderr, "%s - AFAILBACK in progress bit cleared for PID = %d\n", __func__,
            psourceHsPDD->pid);

    /* Clear HSDName information -- Earlier it was really this. */
    memset(psourceHsPDD->hsDevName, 0, 4);

    /* Set the hsDevname in 'now' data drive as 0 */
    memset(pdestinationPDD->hsDevName, 0, 4);

    /* Update the PDD states. */
    O_drvinits++;
    ON_InitDrive(psourceHsPDD, 3, &O_drvinits);

    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        /*
         * Update the file system from any other good device
         * Pass 1 to not allow fs stop to interrupt process
         */
        retCode = FS_UpdateFS(psourceHsPDD, 1);

        if (retCode == DEOK)
        {
            psourceHsPDD->devStat = PD_OP;
            psourceHsPDD->postStat = PD_OP;
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SERIALCHANGE);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SCHEDREBUILD);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REBUILDING);
            BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REPLACE);
            psourceHsPDD->rbRemain = 0;
            psourceHsPDD->pctRem = 0;
        }
    }
    else
    {
        psourceHsPDD->devStat = PD_OP;
        psourceHsPDD->postStat = PD_OP;
        BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SERIALCHANGE);
        BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_SCHEDREBUILD);
        BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REBUILDING);
        BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_REPLACE);
        BIT_CLEAR(psourceHsPDD->miscStat, PD_MB_FSERROR);
        psourceHsPDD->rbRemain = 0;
        psourceHsPDD->pctRem = 0;
    }

    /* Update the PDD states. */
    O_drvinits++;
    ON_InitDrive(psourceHsPDD, 3, &O_drvinits);

    /* Change LED status */
    ON_LedChanged(psourceHsPDD);


    if (BIT_TEST(K_ii.status, II_MASTER))
    {
        /* Update nvram P2 */
        NV_P2UpdateConfig();
    }

    /*
     * Data fail back onto the destination device is complete.
     * Now, it is time to label the destination drive as DATA.
     */
    RB_LabelDestDrive(pdestinationPDD);
}


/**
******************************************************************************
**
**  @brief      This function searches all the devices (PDDs) with devName available
**              in hotspare PDD which indicates the slot and ses from where the
**              data is rebuilt(hotspared) to this device. If any drive (PDD) is
**              available within the slot, it returns that PDD , else NULL.
**
**  @param      phsPDD ----- Hotspare PDD continaing the rebuilt data
**
**  @return     Data drive PDD that is hotspared
**
******************************************************************************
**/
PDD        *RB_GetFailedDataPDD(PDD *phsPDD)
{
    UINT32      idx;
    UINT32      failedSes;
    UINT32      failedSlot;
    UINT32      failedLocation;
    PDD        *pPDD = NULL;

    /* Get the location from where this hotspare device is rebuilt */
    failedLocation = *(UINT32 *)(&(phsPDD->hsDevName[0]));
    failedSlot = (failedLocation & 0xFF000000) >> 24;
    failedSes = (failedLocation & 0x00FF0000) >> 16;

    /* Now check if any drive is placed in that location */
    for (idx = 0; idx < MAX_PHYSICAL_DISKS; ++idx)
    {
        pPDD = gPDX.pdd[idx];
        if (pPDD != NULL)
        {
            if ((pPDD->ses == failedSes) && (pPDD->slot == failedSlot))
            {
                break;
            }
        }
    }
    return (pPDD);
}


/**
******************************************************************************
**
**  @brief      This function enables the global auto failback variable
**              if it is enabled by the user (It is disabled by default)
**
**  @param      option
**
**  @return     status (DEOK or any-error)
**
******************************************************************************
**/

/*UINT8 RB_AutoFailBackEnableDisable (UINT8 option) */
#if defined(MODEL_3000) || defined(MODEL_7400)
UINT8 RB_AutoFailBackEnableDisable(MR_PKT *pMRP)
{
    UINT8       status = DEOK;
    MRPDISKAUTOFAILBACKENABLEDISABLE_REQ *pReq = (MRPDISKAUTOFAILBACKENABLEDISABLE_REQ *)(pMRP->pReq);
    MRPDISKAUTOFAILBACKENABLEDISABLE_RSP *pRsp = (MRPDISKAUTOFAILBACKENABLEDISABLE_RSP *)(pMRP->pRsp);
    LOG_AF_ENABLE_DISABLE_INFO_PKT iPkt;

    if (pReq->options == MR_RB_AUTOFAILBACK_DISABLE)
    {
        if (gAutoFailBackEnable == TRUE)
        {
            gAutoFailBackEnable = FALSE;

            /* Update NVRAM part II. */
            NV_P2UpdateConfig();
        }
        pRsp->mode = gAutoFailBackEnable;
    }
    else if (pReq->options == MR_RB_AUTOFAILBACK_ENABLE)
    {
        if (gAutoFailBackEnable == FALSE)
        {
            gAutoFailBackEnable = TRUE;

            /* Update NVRAM part II. */
            NV_P2UpdateConfig();
        }
        pRsp->mode = gAutoFailBackEnable;
    }
    else if (pReq->options == MR_RB_AUTOFAILBACK_DISPLAY)
    {
        pRsp->mode = gAutoFailBackEnable;
    }
    else
    {
        status = DEINVOPT;
    }

    if ((pReq->options == MR_RB_AUTOFAILBACK_DISABLE) ||
        (pReq->options == MR_RB_AUTOFAILBACK_ENABLE))
    {
        /* Send a log message to CCB */
        iPkt.header.event = LOG_AF_ENABLE_DISABLE;
        iPkt.status = gAutoFailBackEnable;

        /* Note: message is short, and L$send_packet copies into the MRP. */
        MSC_LogMessageStack(&iPkt, sizeof(LOG_AF_ENABLE_DISABLE_INFO_PKT));
    }
    return (status);
}

#else  /* MODEL_3000 || MODEL_7400 */

/**
******************************************************************************
**
**  @brief      This function always disables the  auto failback variable
**              for Nitrogen.
**
**  @param      option
**
**  @return     status (DEINVOP)
**
******************************************************************************
**/

UINT8 RB_AutoFailBackEnableDisable(MR_PKT *pMRP)
{
    MRPDISKAUTOFAILBACKENABLEDISABLE_RSP *pRsp;

    pRsp = (MRPDISKAUTOFAILBACKENABLEDISABLE_RSP *)(pMRP->pRsp);
    gAutoFailBackEnable = FALSE;
    pRsp->mode = gAutoFailBackEnable;

    return DEINVOP;
}

#endif /* MODEL_3000 || MODEL_7400 */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
