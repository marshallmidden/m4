/* $Id: def_con.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       def_con.c
**
**  @brief      Define configuration
**
**  To provide a common means of handling the deinfe configuration.
**
**  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include "def_con.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "daml.h"
#include "defbe.h"
#include "DEF_Workset.h"
#include "error.h"
#include "fabric.h"
#include "globalOptions.h"
#include "ilt.h"
#include "LOG_Defs.h"
#include "LL_LinuxLinkLayer.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "pdd.h"
#include "qu.h"
#include "RL_RDD.h"
#include "vdd.h"
#include "ses.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "dev.h"
#include "misc.h"
#include "ddr.h"
#include "lvm.h"

#include "CT_change_routines.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
extern QU   V_exec_qu;
extern QU   V_exec_mqu;
extern QU   V_exec_hqu;

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/*
 * This structure will hold all of the DAML pointers for the drives in
 * a single bay.  If the critical bay is set to TRUE then this bay must
 * be given preferential treatment in getting the drives from it into
 * the resultant RDA as soon as possible.  The used at variable indicates
 * the last location in the resultant RDA where this bay had a drive placed.
 */
typedef struct BAY_SORT
{
    struct BAY_SORT *pNext;
    UINT16      bayID;
    UINT16      usedAt;
    UINT8       driveCount;
    UINT8       criticalBay;
    DAML       *pDAML[MAX_DISK_BAY_SLOTS + 1];
} BAY_SORT;

/*
 * This structure is used on a pool basis.  It contains a linked list of
 * bay sort structures that are in this pool.  The pool drive count is the
 * count of drives either available in this pool or to be used from this
 * pool.
 */
typedef struct GEO_SORT
{
    UINT16      poolDriveCount;
    BAY_SORT   *pHeadBay;
    DAML       *pResultantDAML[MAX_PHYSICAL_DISKS + 1];
} GEO_SORT;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

DEF_WORKSET gWorksetTable[DEF_MAX_WORKSETS];

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void        DEF_OptRDA(CEV *pCEV);
void        DEF_SortRDA(GEO_SORT *pGeoSort, CEV *pCEV, UINT8 poolCount);
void        DEF_GetRDASize(GEO_SORT *pGS, UINT8 depth, CEV *pCEV, UINT8 poolCount);
void        DEF_FreeGeoSort(GEO_SORT *pGS);

BAY_SORT   *DEF_LocateBaySort(GEO_SORT *pGS, UINT16 bayID);

DAML       *DEF_GetLargestDAML(DAML **pDAML);

void        DEF_RelRDA(CEV *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Check for a mismatch in the device classes passed into an
**              expansion operation or a create.
**
**              First check if all the drive classes are the same.  If not,
**              return an error.  If they are the same, and the operation is
**              an expand operation, then check to see if the drive class is
**              the same as the class of the vdisk being expanded.
**
**  @param      pCEV  - pointer to the input CEV
**
**  @return     Error code (DE_OK if no error)
**
******************************************************************************
**/
UINT32 DEF_CheckDeviceType(CEV *pCEV)
{
//    UINT8       deviceType;
    UINT16      index_d;

//    if (gPDX.pdd[pCEV->dMap[0]] == NULL)
//    {
//        return (DEBADDEVTYPE);
//    }
//    deviceType = gPDX.pdd[pCEV->dMap[0]]->devType;

    for (index_d = 0; index_d < pCEV->numDAML; ++index_d)
    {
        if (gPDX.pdd[pCEV->dMap[index_d]] == NULL)
        {
            return DEBADDEVTYPE;
            break;
        }
        if (gPDX.pdd[pCEV->dMap[index_d]]->devType == PD_DT_UNKNOWN)
        {
            return DEBADDEVTYPE;
            break;
        }
//        else if (deviceType != gPDX.pdd[pCEV->dMap[index_d]]->devType)
//        {
//            return DETYPEMISMATCH;
//            break;
//        }
    }

//    /*
//     * Now check if the operation was an expand and if so, make sure that
//     * the device type of the drives in the input list are the same as the
//     * first drive in the current vdisk.  This will ensure all are the same.
//     */
//    if ((pCEV->op == MCR_OP_EXPAND) &&
//        (gPDX.pdd[gVDX.vdd[pCEV->vid]->pRDD->extension.pPSD[0]->pid]->devType != deviceType))
//    {
//        return DETYPEMISMATCH;
//    }

    return DEOK;
}

/**
******************************************************************************
**
**  @brief      Dynamically allocate and generate a optimized list of DAMLs
**
**              An array is generated within local SRAM which contains the
**              individual DAML pointers specified by the drive map within the
**              MRP structure.  This array is terminated with a null pointer.
**
**  @param      pCEV  - pointer to the input CEV
**
**  @return     Optimized RDA pointed to by the CEV structure
**
******************************************************************************
**/
void DEF_GetRDA(CEV *pCEV)
{
    DEF_RDA    *pRDA;
    UINT16      damlIndex;
    UINT16      rdaIndex = 0;
    DAML      **damlList;
    UINT16      minThresh;

    /*
     * If background processing is runnning, do not attempt to build a
     * RDA until it completes.  This is done since the SES information
     * will be in flux and cannot be trusted if this is running.
     */
    while ((S_bgppcb != NULL) && (TaskGetState(S_bgppcb) != PCB_NOT_READY))
    {
        TaskSwitch();
    }

    /*
     * Set local variables to the structure pointers to simplify the
     * code readability and speed.
     */
    pRDA = (DEF_RDA *)pCEV->rda;
    damlList = (DAML **)(pCEV->daml);

    /*
     * Set the min size allowed to the threshold passed in or 9 which is
     * the value required for RAID 5/9.  This is required so that the
     * RAID can lay evenly through the disks chosen.  This is a simple
     * way to ensure a even amount of stripes in any given parity choice.
     */
    minThresh = MAX(pCEV->thresh, 9);

    /*
     * Get all of the drives from the input drive list that have an
     * SES enlosure identified for them and that have at least the
     * minimum amount of space available.
     */
    for (damlIndex = 0; damlIndex < pCEV->numDAML; ++damlIndex)
    {
        /*
         * Make sure the drive is in a bay and it has at least the min
         * threshold allocation units available.
         */
        if ((damlList[damlIndex]->largest >= minThresh) &&
            (damlList[damlIndex]->pdd->ses != SES_NO_BAY_ID))
        {
            pRDA->daml[rdaIndex++] = damlList[damlIndex];
        }
    }

    /*
     * Now that we are done, set the next pointer to NULL to make sure
     * that there is a NULL terminator.
     */
    pRDA->daml[rdaIndex] = NULL;

    /*
     * Optimize the list.  The optimaization is based upon the input
     * parameter list (CEV).  The count of drives and the DAML will
     * be updated to reflect this optimization.
     */
    if (rdaIndex != 0)
    {
        DEF_OptRDA(pCEV);
    }
    else
    {
        pCEV->rdaDrives = 0;
    }

    /*
     * Put the number of entries in the list into the CEV.  The input
     * parms in the CEV will tell us if we have to use exactly minPD
     * drives or at least minPD.  Ajdust accordingly.  In all cases,
     * do not allow more than the maximum allowed per RAID.
     */
    if (pCEV->rdaDrives < pCEV->minPD)
    {
        pCEV->rdaDrives = 0;
    }
    else if (CE_FLAGS_MINPD & pCEV->flags)
    {
        pCEV->rdaDrives = pCEV->minPD;
    }
    else
    {
        pCEV->rdaDrives = MIN(MAX_PDISKS_PER_RAID, pCEV->rdaDrives);
    }

    /* Null terminate the DAML. */
    pRDA->daml[pCEV->rdaDrives] = NULL;
}

/**
******************************************************************************
**
**  @brief      Optimize the RDA indicated in the CEV structure
**
**              The DAML entries within the RDA are reordered so
**              that the bay utilization is balanced according to
**              the parameters in the CEV.  Each successive DAML
**              entry is balanced so that different bays are used.
**              No concern over PCI busses are needed since there
**              is only one PCI bus for the FC cards.
**
**  @param      pCEV  - pointer to the input CEV
**
**  @return     Optimized RDA pointed to by the CEV structure
**
** Since old GeoRaid is not in use, the GeoRaid flag/option in never set in
** the MRP request during Vdisk creation/expansion. Also the Geopools are
** are never created. So all the drives are treated to be available only in
** one pool.
**
** So following changes are done in this function
**
** >> Removed the code handling when GeoRaid flag/option is is set in MRP
**    request.
** >> Multiple Pools checking (array refereces)are removed, instead used
**    scalar pointer for storing only one pool information.
** >> In the original version (where-in old GeoRaid code exists), the bay
**    and GeoSort logic are processed on multiple pools (when GeoRaid flag is
**    specified) or on oth pool when GeoRaid flag is not specifed(means only
**    one pool). The current change contains logic that has single pool
**    without old-GeoRaid only. Rest of the code sections are removed. So
**    the memory for only for one pool is allocated.
******************************************************************************
*/
void DEF_OptRDA(CEV *pCEV)
{
    DEF_RDA   *pRDA = (DEF_RDA*)pCEV->rda;
    UINT16     index_D;
    GEO_SORT  *pGS;
    BAY_SORT  *pBS;
    GEO_SORT  *pGeoSort=NULL;
    UINT8      numPools = 0;
    UINT16     minResultantCount = 0xFFFF;

    /*
     * Allocation one sort structure per pool.  since we are not doing a
     * a geo-RAID, then only allocate one.  Note contents are cleared
     * upon allocation.
     */

    /* Now fill in the sort structure and pass it to the sorter. */
    for (index_D = 0; pRDA->daml[index_D] != NULL; ++index_D)
    {
        /*
         * If the pool is not already represented in the table,
         * allocate one and fill it in.
         */
        if (pGeoSort == NULL)
        {
            ++numPools;
            pGS = pGeoSort = p_MallocC(sizeof(struct GEO_SORT), __FILE__, __LINE__);
        }
        else
        {
            pGS = pGeoSort;
        }

        pBS = DEF_LocateBaySort(pGS, pRDA->daml[index_D]->pdd->ses);
        pBS->pDAML[pBS->driveCount++] = pRDA->daml[index_D];
        ++pGS->poolDriveCount;
    }

    /*
     * Check the pool count.  If we are doing pools and are requesting a
     * geoRAID, then the min PD count cannot be anything but a multiple
     * of the depth for strict min PD.
     */
    if ((pCEV->flags & CE_FLAGS_MINPD) && (pCEV->minPD % numPools))
    {
        pCEV->rdaDrives = 0;
        pRDA->daml[0] = NULL;
    }
    else
    {
        /*
         * Now sort the RDA for this single pool.  Once they are sorted,
         * we will do a combination of the pools to form the final
         * list.
         */

        /* Sort. */
        if (pGeoSort != NULL)
        {
            DEF_SortRDA(pGeoSort, pCEV, numPools);
            minResultantCount = MIN(minResultantCount, pGeoSort->poolDriveCount);
        }

        /*
         * Set the return value to zero until there are drives put into the list.
         * this will prevent a RAID from being built if no drives are available.
         */
        pCEV->rdaDrives = 0;
        pRDA->daml[0] = NULL;

        /*
         * In the case of geo-RAID, combine from each pool.  In the case
         * of a non-geo-RAID, just return the list from the sort.
         */
        if (minResultantCount != 0)
        {
            if (pGeoSort != NULL)
            {
                memcpy((void*)pRDA->daml,
                       (void*)pGeoSort->pResultantDAML,
                       (signed)(minResultantCount * sizeof(DAML*)));

                pRDA->daml[minResultantCount] = NULL;
                pCEV->rdaDrives = minResultantCount;
            }
        }
    }

    /* Free any remaining used memory. */
    if (pGeoSort != NULL)
    {
        DEF_FreeGeoSort(pGeoSort);
    }
}

/**
******************************************************************************
**
**  @brief      Find a Bay Sort structure within a Geo Sort structure
**
**              If the structure does not exist in the geo sort structure
**              one is allocated and filled in.
**
**  @param      pGS - pointer to the input GEO_SORT
**              bayID - SES ID of the bay to be located
**
**  @return     pBS - pointer to the bay structure
**
******************************************************************************
**/
BAY_SORT   *DEF_LocateBaySort(GEO_SORT *pGS, UINT16 bayID)
{
    BAY_SORT   *pBS = pGS->pHeadBay;

    /*
     * Walk the bay sort chain and find it.  If it does not exist, we
     * will allocate one and put it in the chain.
     */
    while ((pBS != NULL) && (pBS->bayID != bayID))
    {
        pBS = pBS->pNext;
    }

    /* If the pointer is NULL, assign a new one and initialize it. */
    if (pBS == NULL)
    {
        pBS = p_MallocC(sizeof(struct BAY_SORT), __FILE__, __LINE__);
        pBS->pNext = pGS->pHeadBay;
        pBS->bayID = bayID;
        pBS->usedAt = 0xFFFF;
        pBS->criticalBay = FALSE;
        pGS->pHeadBay = pBS;
    }

    return (pBS);
}

/**
******************************************************************************
**
**  @brief      Free a Geo Sort structure and the accompanying Bay Sort structs
**
**  @param      pGS - pointer to the input GEO_SORT
**
**  @return     none
**
******************************************************************************
**/
void DEF_FreeGeoSort(GEO_SORT *pGS)
{
    BAY_SORT   *pBS;

    /* Walk the bay sort chain and dump them all. */
    while (pGS->pHeadBay != NULL)
    {
        pBS = pGS->pHeadBay;
        pGS->pHeadBay = pGS->pHeadBay->pNext;

        p_Free((void *)pBS, sizeof(BAY_SORT), __FILE__, __LINE__);
    }

    p_Free((void *)pGS, sizeof(GEO_SORT), __FILE__, __LINE__);
}

/**
******************************************************************************
**
**  @brief      Sort the RDA indicated in the CEV and the GEO_SORT structures
**
**  @param      pGeoSort - pointer to the input GEO_SORT
**              pCEV  - pointer to the input CEV
**              poolCount - count of pools being used
**
**  @return     none
**
******************************************************************************
**/
void DEF_SortRDA(GEO_SORT *pGS, CEV *pCEV, UINT8 poolCount)
{
    BAY_SORT   *pBay = NULL;
    BAY_SORT   *pBestNonZero = NULL;
    UINT16      maxAllowed = 0;
    UINT16      insIndex;
    UINT8       minSep;
    UINT8       bay_lu[5];
    UINT8       bay_busy[MAX_DISK_BAYS];
    int         i = 0;
    int         dome = 9;
    int         got_something_to_do = 0;

    /* Minimum separation is rounded up depth divided by pool count. */
    minSep = (pCEV->depth + poolCount - 1) / poolCount;

    /*
     * Determine the count for the pool that is optimal and then start
     * round robin insertions for the drives.  When doing the insertions,
     * round robin is used, but critical bays will prevail.  A bay is
     * critical if the depth (pool count adjusted) times the number of
     * drives left in the bay will barely fit in the remaining drive
     * count.
     */
    DEF_GetRDASize(pGS, minSep, pCEV, poolCount);

    for (i = 0; i < MAX_DISK_BAYS; i++)
    {
        bay_busy[i] = 0;
    }
    for (insIndex = 0; insIndex < pGS->poolDriveCount; ++insIndex)
    {
        if (insIndex < 5)
        {
            bay_lu[insIndex] = -1;
        }
        /*
         * Start with a search through just the critical bays.  If there is
         * one, check for the need to pull a drive from it.  Otherwise, we
         * will examine non-critical drives, then round robin.
         */
        for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
        {
            if ((pBay->criticalBay == TRUE) &&
                (pBay->driveCount != 0) &&
                ((pBay->usedAt == 0xFFFF) || (pBay->usedAt + minSep <= insIndex)))
            {
                pGS->pResultantDAML[insIndex] = DEF_GetLargestDAML(pBay->pDAML);
                pBay->usedAt = insIndex;
                --pBay->driveCount;

                if (insIndex < 5)
                {
                    bay_lu[insIndex] = pBay->bayID;     // track the bay id for first 'max minsep' disks
                }

                if (pBay->bayID < MAX_DISK_BAYS)
                {
                    bay_busy[pBay->bayID] = minSep;     // set current bay used to minsep
                }

                for (i = 0; i < MAX_DISK_BAYS; i++)     // for each possible bay
                {
                    if (bay_busy[i] > 0)        // ...if bay is recently used
                    {
                        bay_busy[i]--;  // .......age it by 1 seperation
                    }
                }
                break;
            }
        }

        /* Now find the best one of the remaining items. */
        if (pBay == NULL)
        {
            pBestNonZero = NULL;

            for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
            {
                if (pBay->driveCount != 0)
                {
                    if (pBestNonZero == NULL)
                    {
                        pBestNonZero = pBay;
                    }
                    else
                    {
                        if (pBestNonZero->usedAt != 0xFFFF)
                        {
                            if (pBay->usedAt == 0xFFFF)
                            {
                                pBestNonZero = pBay;
                            }
                            else if (pBestNonZero->usedAt > pBay->usedAt)
                            {
                                if ((insIndex - pBay->usedAt) >= minSep)
                                {
                                    pBestNonZero = pBay;
                                }
                            }
                        }
                    }
                }
            }

            /* Fill it in. */
            if (pBestNonZero != NULL)
            {
                pGS->pResultantDAML[insIndex] = DEF_GetLargestDAML(pBestNonZero->pDAML);
                pBestNonZero->usedAt = insIndex;
                --pBestNonZero->driveCount;
                if (insIndex < 5)
                {
                    bay_lu[insIndex] = pBestNonZero->bayID;     // track the bay id for first 'max minsep' disks
                }
                if (pBestNonZero->bayID < MAX_DISK_BAYS)
                {
                    bay_busy[pBestNonZero->bayID] = minSep;     // set current bay used to minsep
                }
                for (i = 0; i < MAX_DISK_BAYS; i++)     // for each possible bay
                {
                    if (bay_busy[i] > 0)        // ...if bay is recently used
                    {
                        bay_busy[i]--;  // .......age it by 1 seperation
                    }
                }
            }
        }

        maxAllowed = 0;

        for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
        {
            if (pBay->bayID < MAX_DISK_BAYS)
            {
                if (!bay_busy[pBay->bayID])     // ignore pdisks on busy bays
                {
                    maxAllowed = MAX(pBay->driveCount, maxAllowed);
                }
            }
        }
        got_something_to_do = 0;
        dome = 9;               // set dome to out of bounds for now.
        for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
        {
            pBay->criticalBay = FALSE;
            if (pBay->bayID < MAX_DISK_BAYS)
            {
                if (!bay_busy[pBay->bayID])
                {
                    if (pBay->driveCount == maxAllowed)
                    {
                        pBay->criticalBay = TRUE;
                        /*
                         * Following will shift 'dome' to be as close to 0 as possible.
                         * this will ensure we use up the bays that are first in the
                         * raid list first...but only when we have to.
                         */
                        for (i = 0; i < 5; i++)
                        {
                            if (dome > i && pBay->bayID == bay_lu[i] && minSep > i)
                            {
                                dome = i;       // track dome..it will be first possible bay in raid
                            }
                        }
                        got_something_to_do++;
                    }
                }
            }
        }
        /*
         * Now, did we get more than one 'critical' that we need to reduce down
         * to a single critical in the case where one prevents wraparound?
         */
        if (got_something_to_do > 1 && dome != 9)
        {
            /*
             * Yes, so now make ONLY the wraparound bay critical so that we
             *  pull a disk from it right away.
             */
            for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
            {
                pBay->criticalBay = FALSE;
                if (pBay->bayID < MAX_DISK_BAYS)
                {
                    if (!bay_busy[pBay->bayID])
                    {
                        if (pBay->driveCount == maxAllowed)
                        {
                            if ((dome == 0 && pBay->bayID == bay_lu[0]) ||
                                (dome == 1 && pBay->bayID == bay_lu[1]) ||
                                (dome == 2 && pBay->bayID == bay_lu[2]) ||
                                (dome == 3 && pBay->bayID == bay_lu[3]) ||
                                (dome == 4 && pBay->bayID == bay_lu[4]))
                            {
                                pBay->criticalBay = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Get the largest LAS DAML from the list of DAMLs and return it.
**
**              This function will pull the largest available space DAML
**              from the DAML list and will return it to the caller.  It
**              will also clear that DAML from the structure to make sure
**              it is not reused.
**
**  @param      pDAML - pointer to the list of DAMLs
**
**  @return     retDAML - DAML pointer for the largest
**
******************************************************************************
**/
DAML       *DEF_GetLargestDAML(DAML **pDAML)
{
    UINT32      largest = 0;
    UINT16      largestIndex = 0xFFFF;
    UINT16      index_D;
    DAML       *retDAML = NULL;

    /*
     * Search through the list until the largest is found.  When found,
     * make sure the list is cleaned up so that this one has been removed
     * and collapsed.
     */
    for (index_D = 0; pDAML[index_D] != NULL; index_D++)
    {
        if (pDAML[index_D]->largest > largest)
        {
            largest = pDAML[index_D]->largest;
            largestIndex = index_D;
        }
    }

    /*
     * Now move the last element into the array where we are pulling
     * the DAML.
     */
    retDAML = pDAML[largestIndex];
    pDAML[largestIndex] = pDAML[index_D - 1];
    pDAML[index_D - 1] = NULL;

    return (retDAML);
}

/**
******************************************************************************
**
**  @brief      Get the drive count for this pool
**
**              This function will go through an iterative process to determine
**              the count of drives in the RDA for this pool.  The value is
**              based upon the max count for any RAID, the number of pools,
**              the depth of the array and the drive counts in each bay.
**
**  @param      pSG - GEO_SORT structure for this pool
**              depth - depth of the RAID
**              pCEV - pointer to create/expand virtual disk parms
**              poolCount - number of pools being used
**
**  @return     pSG is adusted to bring drive counts to max allowed and
**              drive count in the pool to the size of the RDA to be created
**
******************************************************************************
**/
void DEF_GetRDASize(GEO_SORT *pGS, UINT8 depth, CEV *pCEV, UINT8 poolCount)
{
    UINT8       done = FALSE;
    BAY_SORT   *pBay;
    UINT16      maxAllowed = 0;
    UINT16      finalDriveCount = 0;
    UINT16      targetFinalCount = 0;
    BAY_SORT   *pMaxBay = NULL;

    /* Make passes until the size of the max bay does not change. */
    while (!done)
    {
        done = TRUE;
        finalDriveCount = 0;

        /*
         * Determine the maximum drives allowed per bay.  This is done
         * by taking the minimum of the pdisks per raid divided by the
         * depth or the pool count divided by the depth.  In addition,
         * we have to check for the min pd count being exact.
         */
        if ((CE_FLAGS_MINPD & pCEV->flags) && (CE_FLAGS_REDUN & pCEV->flags))
        {
            maxAllowed = MIN((MAX_PDISKS_PER_RAID + poolCount - 1) / depth / poolCount, pGS->poolDriveCount / depth);
            maxAllowed = MIN((pCEV->minPD + poolCount - 1) / depth / poolCount, maxAllowed);
            targetFinalCount = pCEV->minPD;
        }
        else if (CE_FLAGS_REDUN & pCEV->flags)
        {
            maxAllowed = MIN((MAX_PDISKS_PER_RAID + poolCount - 1) / depth / poolCount, pGS->poolDriveCount / depth);
            targetFinalCount = MAX_PDISKS_PER_RAID;
        }
        else if (CE_FLAGS_MINPD & pCEV->flags)
        {
            maxAllowed = MIN(MAX_DISK_BAY_SLOTS, pCEV->minPD);
            targetFinalCount = pCEV->minPD;
        }
        else
        {
            maxAllowed = MIN(MAX_DISK_BAY_SLOTS, pGS->poolDriveCount);
            targetFinalCount = MAX_PDISKS_PER_RAID;
        }

        for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
        {
            if (pBay->driveCount > maxAllowed)
            {
                pGS->poolDriveCount -= (pBay->driveCount - maxAllowed);
                pBay->driveCount = maxAllowed;
                finalDriveCount += maxAllowed;
                done = FALSE;
            }
            else if (pBay->driveCount == maxAllowed)
            {
                finalDriveCount += maxAllowed;
                pBay->criticalBay = TRUE;
            }
            else
            {
                finalDriveCount += pBay->driveCount;
            }
        }

        /*
         * Make a clean up pass.  We may have multiple critical bays which
         * can cause us to exceed the min drive count.  If this is the case,
         * then run through the bays and find enough bays to reduce by one
         * and then pass through again.
         */
        if (done &&
            ((CE_FLAGS_MINPD & pCEV->flags) || (pCEV->flags == 0)) &&
            (finalDriveCount > targetFinalCount))
        {
            pMaxBay = pGS->pHeadBay;

            for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
            {
                if (pBay->driveCount == maxAllowed)
                {
                    --pGS->poolDriveCount;
                    --finalDriveCount;
                    --pBay->driveCount;
                    done = FALSE;

                    if (finalDriveCount == targetFinalCount)
                    {
                        break;
                    }
                }

                if (pBay->driveCount >= pMaxBay->driveCount)
                {
                    if (pBay->driveCount == pMaxBay->driveCount)
                    {
                        /*
                         * If multiples found, then try to trim just the one that has smallest LAS
                         * To do this, we need to look for the largest DAML in all DAMLs for current
                         * bay and the previous max bay. Could optimize this a bit by checking head.
                         */
                        UINT32      largestcur = 0;
                        UINT32      largestmax = 0;
                        UINT16      index_D;

                        for (index_D = 0; pBay->pDAML[index_D] != NULL; index_D++)
                        {
                            if (pBay->pDAML[index_D]->largest > largestcur)
                            {
                                largestcur = pBay->pDAML[index_D]->largest;
                            }
                        }
                        for (index_D = 0; pMaxBay->pDAML[index_D] != NULL; index_D++)
                        {
                            if (pMaxBay->pDAML[index_D]->largest > largestmax)
                            {
                                largestmax = pMaxBay->pDAML[index_D]->largest;
                            }
                        }
                        /* does this bay contain the smallest largest pdisk? */
                        if (largestcur < largestmax)
                        {
                            pMaxBay = pBay;     /* yes, select it for trimming */
                        }
                    }
                    else        /* first time hit */
                    {
                        pMaxBay = pBay;
                    }
                }
            }

            /*
             * If done is false, then we decremented a bay that was critical
             * and do not have to look for more decrementing.
             */
            if (done && (finalDriveCount > targetFinalCount))
            {
                --pGS->poolDriveCount;
                --finalDriveCount;
                --pMaxBay->driveCount;
                done = FALSE;
            }
        }
    }

    /*
     * Lastly, pass through the list of bays and mark all bays with the
     * max number of drives as critical.
     */
    maxAllowed = 0;

    for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
    {
        maxAllowed = MAX(pBay->driveCount, maxAllowed);
    }

    for (pBay = pGS->pHeadBay; pBay != NULL; pBay = pBay->pNext)
    {
        if (pBay->driveCount == maxAllowed)
        {
            pBay->criticalBay = TRUE;
        }
        else
        {
            pBay->criticalBay = FALSE;
        }
    }
}

/**
******************************************************************************
**
**  @brief      Release the RDA indicated in the CEV structure
**
**  @param      pCEV  - pointer to the input CEV
**
**  @return     none
**
******************************************************************************
**/
void DEF_RelRDA(CEV *pCEV)
{
    if ((pCEV != NULL) && (pCEV->rda != NULL))
    {
        p_Free((void *)pCEV->rda, sizeof(DEF_RDA), __FILE__, __LINE__);
    }
}

/**
******************************************************************************
**
**  @brief      Fetch the workset indicated in the MRP
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/
UINT8 DEF_GetWorkset(MR_PKT *pMRP)
{
    UINT16      id;
    UINT8       retCode = DEOK;

    /* Assume one packet return size. */
    ((MRGETWSINFO_RSP *)pMRP->pRsp)->header.len = sizeof(MRGETWSINFO_RSP);

    /* Get the ID requested.  An 0xffff indicates return all of them. */
    id = ((MRGETWSINFO_REQ *)pMRP->pReq)->id;

    if ((id >= DEF_MAX_WORKSETS) && (id != 0xFFFF))
    {
        retCode = DEINVWSID;
    }
    else
    {
        if (id < DEF_MAX_WORKSETS)
        {
            if (pMRP->rspLen != (sizeof(MRGETWSINFO_RSP) + sizeof(DEF_WORKSET)))
            {
                retCode = DERETLENBAD;
            }
            else
            {
                memcpy((void *)&(((MRGETWSINFO_RSP *)pMRP->pRsp)->workset[0]),
                       (void *)&(gWorksetTable[id]),
                       sizeof(DEF_WORKSET));
            }
        }
        else
        {
            /*
             * We are requested to send all of them.  Copy the entire
             * table if enough space was allocated.
             */
            if (pMRP->rspLen != (sizeof(MRGETWSINFO_RSP) +
                                 (DEF_MAX_WORKSETS * sizeof(DEF_WORKSET))))
            {
                retCode = DETOOMUCHDATA;
            }
            else
            {
                ((MRGETWSINFO_RSP *)pMRP->pRsp)->header.len =
                    sizeof(MRGETWSINFO_RSP) +
                    (DEF_MAX_WORKSETS * sizeof(DEF_WORKSET));

                memcpy((void *)&(((MRGETWSINFO_RSP *)pMRP->pRsp)->workset[0]),
                       (void *)&(gWorksetTable[0]),
                       DEF_MAX_WORKSETS * sizeof(DEF_WORKSET));
            }
        }
    }

    return (retCode);
}


/**
******************************************************************************
**
**  @brief      Emulate Qlogic timeout for pdisk
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/
UINT8 DEF_BEQlogicTimeout(MR_PKT *pMRP)
{
    UINT16      pid;
    UINT16      flag;
    PDD        *pdd;
    DEV        *dev;

    ((MRPDISKQLTIMEOUT_RSP *)pMRP->pRsp)->header.len = sizeof(MRPDISKQLTIMEOUT_RSP);

    pid = ((MRPDISKQLTIMEOUT_REQ *)pMRP->pReq)->pid;
    if (pid >= MAX_PHYSICAL_DISKS)
    {
        fprintf(stderr, "PDD %d is greater than MAX_PHYSICAL_DISKS (%d)\n", pid, MAX_PHYSICAL_DISKS);
        return (DENONXDEV);
    }
    flag = ((MRPDISKQLTIMEOUT_REQ *)pMRP->pReq)->flag;

    pdd = gPDX.pdd[pid];
    if (pdd == NULL)
    {
        fprintf(stderr, "PDD %d is null\n", pid);
        return (DENONXDEV);
    }
    if (pdd->pid != pid)
    {
        fprintf(stderr, "pdd->pid (%d) is not pid=%d\n", pdd->pid, pid);
        return (DENONXDEV);
    }
    dev = pdd->pDev;
    if (dev == NULL)
    {
        fprintf(stderr, "dev for pid %d is null\n", pid);
        return (DENONXDEV);
    }

    if (flag == 0)
    {
        BIT_CLEAR(dev->flags, DV_QLTIMEOUTEMULATE);
    }
    else
    {
        BIT_SET(dev->flags, DV_QLTIMEOUTEMULATE);
    }

    return (DEOK);
}


/**
******************************************************************************
**
**  @brief      Set the workset indicated in the MRP
**
**  @param      pMRP  - pointer to the input MRP from the CCB
**
**  @return     return status
**
******************************************************************************
**/
UINT8 DEF_SetWorkset(MR_PKT *pMRP)
{
    UINT16      id;
    UINT8       retCode = DEOK;

    ((MRSETWSINFO_RSP *)pMRP->pRsp)->header.len = sizeof(MRSETWSINFO_RSP);

    id = ((MRSETWSINFO_REQ *)pMRP->pReq)->id;

    if (id >= DEF_MAX_WORKSETS)
    {
        retCode = DEINVWSID;
    }
    else
    {
        memcpy((void *)&(gWorksetTable[id]),
               (void *)&(((MRSETWSINFO_REQ *)pMRP->pReq)->workset),
               sizeof(DEF_WORKSET));

        /* Update NVRAM part II. */
        NV_P2UpdateConfig();
    }

    return (retCode);
}


/**
******************************************************************************
**
**  @brief      Find an orphan RAID.
**
**  @param      raidID - specific RAID to check.
**
**  @return     Boolean - TRUE for orphan, FALSE for no orphan.
**
******************************************************************************
**/
UINT8 DEF_IsOrphanRAID(UINT16 raidID)
{
    RDD        *pRDD = NULL;
    UINT8       orphan;

    /*
     * Validate the input parm and then check if this RAID is
     * an orphan or not.
     */
    if ((raidID >= MAX_RAIDS) || (gRDX.rdd[raidID] == NULL))
    {
        orphan = FALSE;
    }
    else
    {
        orphan = TRUE;
    }

    if (orphan)
    {
        /*
         * Check if the parent VDD thinks this is the RDD that it
         * contains.  If not, we have an orphan.
         */
        if (gVDX.vdd[gRDX.rdd[raidID]->vid] != NULL)
        {
            pRDD = gVDX.vdd[gRDX.rdd[raidID]->vid]->pRDD;

            while (pRDD != NULL)
            {
                /* If found, stop checking. */
                if (pRDD->rid == raidID)
                {
                    orphan = FALSE;
                    break;
                }
                else
                {
                    pRDD = pRDD->pNRDD;
                }
            }

            /*
             * If we still think this is an orphan, check for expanding
             * RAIDs in the vdisk.
             */
            if (orphan)
            {
                pRDD = gVDX.vdd[gRDX.rdd[raidID]->vid]->pDRDD;

                while (pRDD != NULL)
                {
                    /* If found, stop checking. */
                    if (pRDD->rid == raidID)
                    {
                        orphan = FALSE;
                        break;
                    }
                    else
                    {
                        pRDD = pRDD->pNRDD;
                    }
                }
            }
        }
    }

    return (orphan);
}


/**
******************************************************************************
**
**  @brief      Log orphan RAID
**
**              This function will log the RID of an orphan RAID.
**
**  @param      rid - RAID ID of the orphan RAID
**
**  @return     None
**
******************************************************************************
**/
void DEF_LogOrphanRAID(UINT16 raidID)
{
    LOG_ORPHAN_DETECTED_PKT orphanEvent;

    orphanEvent.header.event = LOG_ORPHAN_DETECTED;

    orphanEvent.data.raidID = raidID;
    orphanEvent.data.vdiskID = gRDX.rdd[raidID]->vid;

    /* Send the log message. */
    MSC_LogMessageStack(&orphanEvent, sizeof(LOG_ORPHAN_DETECTED_PKT));
}

/**
******************************************************************************
**
**  @brief      Set VDisk Priority
**
**  @param      Vid-Priority Pairs
**
**  @return     None
**
******************************************************************************
**/
UINT8 DEF_SetVPri(MR_PKT * pMRP)
{
    UINT8       retVal = MSPOK;
    UINT16      vid;
    UINT16      i;
    UINT8       pri;
    MRSETVPRI_REQ *pReq = (MRSETVPRI_REQ *)(pMRP->pReq);
    MRSETVPRI_RSP *pRsp = (MRSETVPRI_RSP *)(pMRP->pRsp);

    if (pReq->count == 0)
    {
        retVal = MSPNOVIDS;
    }
    else if (pReq->count > MAX_VIRTUAL_DISKS)
    {
        retVal = MSPBADCNT;
    }
    else if (pReq->opt > MSPSETLOALL)
    {
        retVal = MSPBADOPT;
    }
    else
    {
        if (pReq->opt == MSPNOCHANGE)
        {
            pRsp->count = pReq->count;
            for (i = 0; i < pReq->count; i++)
            {
                vid = pReq->lst[i].vid;
                pri = pReq->lst[i].pri;
                if (pri > MSPSETHI)
                {
                    pRsp->lst[i].vid = vid;
                    pRsp->lst[i].pri = MSPBADPRI;
                }
                else if (gVDX.vdd[vid] == NULL)
                {
                    pRsp->lst[i].vid = vid;
                    pRsp->lst[i].pri = MSPNOVDD;
                }
                else
                {
                    /* Update Priority */
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    gVDX.vdd[vid]->priority = pri;
                    D_SetVPri(0, vid, pri);
                    pRsp->lst[i].vid = vid;
                    pRsp->lst[i].pri = MSPDONE;
                }
            }
        }
        else
        {
            pRsp->count = 0;
            for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
            {
                pRsp->lst[i].pri = 0xFF;
            }
            for (i = 0; i < pReq->count; i++)
            {
                vid = pReq->lst[i].vid;
                pri = pReq->lst[i].pri;
                if (pri > MSPSETHI)
                {
                    pRsp->lst[vid].vid = vid;
                    pRsp->lst[vid].pri = MSPBADPRI;
                }
                else if (vid >= MAX_VIRTUAL_DISKS || gVDX.vdd[vid] == NULL)
                {
                    pRsp->lst[vid].vid = vid;
                    pRsp->lst[vid].pri = MSPNOVDD;
                }
                else
                {
                    /* Update Priority */
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                    gVDX.vdd[vid]->priority = pri;
                    D_SetVPri(0, vid, pri);
                    pRsp->lst[vid].vid = vid;
                    pRsp->lst[vid].pri = MSPDONE;
                }
                pRsp->count++;
            }
            for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
            {
                if (pRsp->lst[i].pri == 0xFF)
                {
                    if (gVDX.vdd[i] != NULL)
                    {
                        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                        gVDX.vdd[i]->priority = MSPSETLO;
                        D_SetVPri(0, i, MSPSETLO);
                        pRsp->lst[i].vid = i;
                        pRsp->lst[i].pri = MSPDONE;
                        pRsp->count++;
                    }
                }
            }
        }
    }
    return (retVal);
}

/**
******************************************************************************
**
**  @brief      Enable/Disable VDisk Priority Feature
**
**  @param      Mode
**
**  @return     Status (Success/Failure)
**
******************************************************************************
**/
UINT8 DEF_VPriEnable(MR_PKT *pMRP)
{
    UINT8       retVal = DEOK;
    MRVPRI_ENABLE_REQ *pReq = (MRVPRI_ENABLE_REQ *)(pMRP->pReq);
    MRVPRI_ENABLE_RSP *pRsp = (MRVPRI_ENABLE_RSP *)(pMRP->pRsp);

    switch (pReq->mode)
    {
        case MSPDISABLE:
            if (VPri_enable != FALSE)
            {
                /* Move all requests from V_exec_hqu and V_exec_mqu to V_exec_qu */
                if (V_exec_mqu.tail != NULL)
                {
                    /* Move contents of mqu to qu */
                    if (V_exec_qu.head == NULL)
                    {
                        V_exec_qu.head = V_exec_mqu.head;
                        V_exec_qu.tail = V_exec_mqu.tail;
                        V_exec_qu.qcnt = V_exec_mqu.qcnt;
                    }
                    else
                    {
                        V_exec_mqu.tail->fthd = V_exec_qu.head;
                        V_exec_qu.head->bthd = V_exec_mqu.tail;
                        V_exec_qu.head = V_exec_mqu.head;
                        V_exec_qu.qcnt += V_exec_mqu.qcnt;
                    }

                    /* Clear mqu */
                    V_exec_mqu.head = NULL;
                    V_exec_mqu.tail = NULL;
                    V_exec_mqu.qcnt = 0;
                }

                if (V_exec_hqu.tail != NULL)
                {
                    /* Move contents of hqu to qu */
                    if (V_exec_qu.head == NULL)
                    {
                        V_exec_qu.head = V_exec_hqu.head;
                        V_exec_qu.tail = V_exec_hqu.tail;
                        V_exec_qu.qcnt = V_exec_hqu.qcnt;
                    }
                    else
                    {
                        V_exec_hqu.tail->fthd = V_exec_qu.head;
                        V_exec_qu.head->bthd = V_exec_hqu.tail;
                        V_exec_qu.head = V_exec_hqu.head;
                        V_exec_qu.qcnt += V_exec_hqu.qcnt;
                    }

                    /* Clear hqu */
                    V_exec_hqu.head = NULL;
                    V_exec_hqu.tail = NULL;
                    V_exec_hqu.qcnt = 0;
                }

                VPri_enable = FALSE;
                /* Update NVRAM part II. */
                NV_P2UpdateConfig();
                pRsp->mode = VPri_enable;
            }
            break;
        case MSPENABLE:
            if (VPri_enable != TRUE)
            {
                /*
                 * For each DEV in each chn_ind port
                 * pDEV->locnt = pDEV->qcnt;
                 */
                VPri_enable = TRUE;

                /* Update NVRAM part II. */
                NV_P2UpdateConfig();
                pRsp->mode = VPri_enable;
            }
            break;
        case MSPSTATUS:
            pRsp->mode = VPri_enable;
            break;

        default:
            break;
    }

/*
   Write in NVRAM
   Communicate to other controllers in DSC
*/
    return (retVal);
}

#if defined(MODEL_7000) || defined(MODEL_4700)
/**
******************************************************************************
**
**  @brief      This function will Get the ISE ip addresses.
**
**  @param      mrp - MRP structure
**
**  @return     return status.
**
******************************************************************************
**/
UINT8 DEF_GetISEIP(MR_PKT *pMRP)
{
    MRGETISEIP_RSP *pRsp;
    MRGETISEIP_REQ *pReq;
    struct DEV *pDev;
    PDD        *isepdd;
    UINT16      bayid;
    UINT32      ip1 = 0;
    UINT32      ip2 = 0;
    UINT8       retstatus = DEOK;
    UINT64      wwn = 0;

    pReq = (MRGETISEIP_REQ *)pMRP->pReq;
    pRsp = (MRGETISEIP_RSP *)pMRP->pRsp;

    bayid = pReq->bayid;

    if (bayid >= MAX_DISK_BAYS)
    {
        goto out;
    }

    isepdd = gEDX.pdd[bayid];

    if (isepdd)
    {
        pDev = isepdd->pDev;
        wwn = gEDX.pdd[bayid]->wwn;

        if (pDev != NULL)
        {
            retstatus = ISE_GetPage85(pDev, &ip1, &ip2, 1);
        }
        else
        {
            fprintf(stderr, "%s: isepdd dev is null for bay id %d\n", __func__, bayid);
            retstatus = DENONXDEV;
        }
    }
    else
    {
        fprintf(stderr, "%s: ise pdd is null for bay id %d\n", __func__, bayid);
        retstatus = DEFAILED;
    }

  out:
    pRsp->bayid = bayid;
    pRsp->ip1 = ip1;
    pRsp->ip2 = ip2;
    pRsp->wwn = wwn;

    return retstatus;
}
#endif /* MODEL_7000 || MODEL_4700 */

/**
******************************************************************************
**
**  @brief      This routine gets seconds since epoh
**
**  @param      None
**
**  @return     Seconds since epoch
**
******************************************************************************
**/
UINT32 GetSysTime(void)
{
    return (time(NULL));
}

/**
******************************************************************************
**
**  @brief      This routine allocates the DRAM memory for stats information
**
******************************************************************************
**/
void DEF_AllocMemoryForVDStats(VDD *pVDD)
{
    pVDD->pLastHourAvgStats = p_MallocC(sizeof(struct STATS_PER_HOUR) | BIT31, __FILE__, __LINE__);
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/**
******************************************************************************
**
**  @brief      This routine deallocates  the DRAM memory whenever a VDisk
**              is deleted
**
******************************************************************************
**/
void DEF_DeallocVDStatsMemory(VDD *pVDD)
{
    if (pVDD->pLastHourAvgStats != NULL)
    {
        p_Free(pVDD->pLastHourAvgStats, sizeof(STATS_PER_HOUR), __FILE__, __LINE__);
    }
}

/**
******************************************************************************
**
**  @brief      This routine  collects the VDisk statistics like
**              IO per second average and sector count per second over last hour
**
******************************************************************************
**/

void DEF_VdiskLastHrStats(VDD *pVDD)
{
    STATS_PER_HOUR *pAvgStats;
    INT32       IOCount;
    INT32       SectorCount;
    INT32       previousIoCount = 0;
    INT32       previousSectorCount = 0;
    UINT8       OneMinuteIndex;
    INT32       PreviousIOAverage = 0;
    INT32       PreviousSCAverage = 0;
    INT32       totIO;
    INT32       totSC;

    /*
     * Check if memory for the stats information has already been
     * allocated. If not, go ahead and allocate memory
     */
    if (pVDD->pLastHourAvgStats == NULL)
    {
        DEF_AllocMemoryForVDStats(pVDD);
    }

    /* Get the pointer to the stats structure */
    pAvgStats = pVDD->pLastHourAvgStats;

    /* Get the current minute index */
    OneMinuteIndex = pAvgStats->currentIndex;

    /*
     * By the time we reach this routine, statistics have already been collected
     * for one second. Therefore, increment the one second indicator.
     */
    pAvgStats->oneSecondIndicator++;

    /*
     * Check whether one minute is elapsed since the previous collection
     * of statistics.
     */
    if (pAvgStats->oneSecondIndicator >= 60)
    {
        /*
         * 1 minute interval is hit.  Accumulated the stats for 60 seconds.
         * Now compute the average I/O per second and SC/second during last
         *  one hour. Update the same in VDD.
         */
        IOCount = pAvgStats->cummIOCount[OneMinuteIndex];
        SectorCount = pAvgStats->cummSC[OneMinuteIndex];

        /*
         * Make sure if one hour is elapsed since the previous collection
         * of statistics.
         * If it is, we need to find the average IO per second and SC
         * per second over last hour and update the values in the corresponding
         * VDD
         * Check current index position (at what minute we are currently at..)
         */
        if (OneMinuteIndex < 60)
        {
            /*
             * We have not yet crossed 60 minutes interval
             * Get the previous minute accumulation. If we are at the
             * beginning of the 60 minute interval values table, its previous
             * value is obviously is last entry in the table.
             */
            previousIoCount = pAvgStats->PrevIOCount;
            previousSectorCount = pAvgStats->PrevSectorCount;

            ++OneMinuteIndex;
        }


        /*
         * Compute the average IO per second and SC per second during last
         * one hour.
         */
        PreviousIOAverage = pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond;
        PreviousSCAverage = pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond;

        /*
         * Check if statistics have collected for more than an hour
         * This is to avoid dividing by 3600 while calculating the Average IO and
         * SC per second within the first one hour.Perform wraparound when
         * reached the end of 60 minute interval values table.
         */
        if (OneMinuteIndex == 60)
        {
            OneMinuteIndex = 0;
            pAvgStats->statsFlag = TRUE;
        }

        /* Update the new index */
        pAvgStats->currentIndex = OneMinuteIndex;

        /* Update the averages */
        totIO = (PreviousIOAverage - previousIoCount + IOCount);
        pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond = (totIO < 0) ? 0 : totIO;

        totSC = (PreviousSCAverage - previousSectorCount + SectorCount);
        pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond = (totSC < 0) ? 0 : totSC;

        /*
         * Reset the seconds clock to enable to accumulate statistics
         * for next seconds for this VDD.
         */
        pAvgStats->oneSecondIndicator = 0;
        pAvgStats->PrevIOCount = pAvgStats->cummIOCount[OneMinuteIndex];
        pAvgStats->PrevSectorCount = pAvgStats->cummSC[OneMinuteIndex];
        pAvgStats->cummIOCount[OneMinuteIndex] = 0;
        pAvgStats->cummSC[OneMinuteIndex] = 0;
    }

    /* Collect the IO per second and sector count Per second stats */
    pAvgStats->cummIOCount[OneMinuteIndex] += pVDD->rps;
    pAvgStats->cummSC[OneMinuteIndex] += pVDD->avgSC;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/**
******************************************************************************
**
** @brief      This routine sorts the given PDisks in the increasing order of
**             Largest Available Space (LAS)
**
** @return     none
**
******************************************************************************
**/
void DEF_SortPDisks(UINT16 *dmap, UINT16 driveCount)
{
    UINT16      i;
    UINT16      j;
    UINT16      temp;
    int         done = 0;       /* Exit from bubble sort when nothing changes */

    for (i = 0; i < driveCount && !done; i++)
    {
        done = 1;
        for (j = 0; j < (driveCount - 1); j++)
        {
            if (!gPDX.pdd[dmap[j]])
            {
                continue;       /* If this one is null, do nothing */
            }
            if (!gPDX.pdd[dmap[j + 1]] ||
                gPDX.pdd[dmap[j + 1]]->las > gPDX.pdd[dmap[j]]->las)
            {
                temp = dmap[j + 1];
                dmap[j + 1] = dmap[j];
                dmap[j] = temp;
                done = 0;
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Counts the number of servers assigned to the specified VDisk.
**
**  @param      vid - vdisk to count attached servers.
**
**  @return     TRUE - if XIO controller attached to vdisk.
**
**  @attention  Does not include XIOtech controllers in the count.
**
******************************************************************************
**/
UINT32 dlm_cnt_servers(UINT16 vid)
{
    UINT32 count;
    UINT16 attribute;
    UINT16 i;
    struct LVM *lvm;

    /* Isolate VDisk attribute field. */
    attribute = V_vddindx[vid]->attr & VD_VDMASK;
    if (BIT_TEST(attribute, VD_HIDDEN) || BIT_TEST(attribute, VD_PRIVATE))
    {
        return 0;                       /* Return no servers. */
    }

    /* Loop through the SDDs */
    for (i= 0, count=0; i <  MAX_SERVERS; i++)
    {
        if (S_sddindx[i] == NULL)
        {
            continue;                   /* If no SDD, continue to the next. */
        }

        /* Loop through the LVMs on the link list */
        for (lvm = S_sddindx[i]->lvm; lvm != NULL; lvm = lvm->nlvm)
        {
            if (lvm->vid == vid)
            {
                /* Do not include other XIOtech controllers */
                if (M_chk4XIO(S_sddindx[i]->wwn) == 0)
                {
                    /* Matching VID, Increment the number of servers. */
                    count++;
                }
                break;
            }
        }
    }
    return count;
}   /* End of dlm_cnt_servers */

/**
******************************************************************************
**
**  @brief      This function will determine if XIO controller attached to vdisk.
**
**  @param      vid - vdisk to check server list for XIO controller.
**
**  @return     TRUE - if XIO controller attached to vdisk.
**
******************************************************************************
**/
UINT8 check_vdisk_XIO_attached(UINT16 vid)
{
    UINT16 attribute;
    UINT16 i;
    struct LVM *lvm;

    /* Isolate VDisk attribute field. */
    attribute = V_vddindx[vid]->attr & VD_VDMASK;
    if (BIT_TEST(attribute, VD_HIDDEN) || BIT_TEST(attribute, VD_PRIVATE))
    {
        return FALSE;                       /* Return no servers. */
    }

    /* Loop through the SDDs */
    for (i= 0; i <  MAX_SERVERS; i++)
    {
        if (S_sddindx[i] == NULL)
        {
            continue;                   /* If no SDD, continue to the next. */
        }

        /* Loop through the LVMs on the link list */
        for (lvm = S_sddindx[i]->lvm; lvm != NULL; lvm = lvm->nlvm)
        {
            if (lvm->vid == vid)
            {
                /* Do not include other XIOtech controllers */
                if (M_chk4XIO(S_sddindx[i]->wwn) != 0)
                {
                    return TRUE;
                }
                break;
            }
        }
    }
    return FALSE;
}   /* End of check_vdisk_XIO_attached */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
