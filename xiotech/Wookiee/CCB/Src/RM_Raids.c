/* $Id: RM_Raids.c 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       RM_Raids.c
**
**  @brief      Resource Manager Raids Management
**
**  To provide management for RAIDS at the CCB level.
**
**  Copyright (c) 1996-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "RM_Raids.h"
#include "debug_files.h"
#include "EL.h"
#include "misc.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "pcb.h"
#include "PI_Server.h"
#include "PI_Target.h"
#include "PI_VDisk.h"
#include "PI_WCache.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "sm.h"
#include "X1_AsyncEventHandler.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
typedef struct RM_R5_MISSING_LIST
{
    UINT32      from;
    UINT32      mp;
    UINT32      to;
} RM_R5_MISSING_LIST;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static PCB *pR5StripeResyncMonitorTaskPCB = NULL;
static bool bR5StripeResyncInProgress = false;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void RM_R5StripeResyncMonitorTask(TASK_PARMS *parms);
static void RM_R5BuildMissingControllerList(PI_VCG_GET_MP_LIST_RSP *pMPList,
                                            RM_R5_MISSING_LIST *pMissingList);
static void RM_R5FailMissingControllers(PI_VCG_GET_MP_LIST_RSP *pMPList,
                                        RM_R5_MISSING_LIST *pMissingList);
static void RM_R5ResyncControllers(PI_VCG_GET_MP_LIST_RSP *pMPList,
                                   RM_R5_MISSING_LIST *pMissingList);
static UINT32 RM_R5GetResyncController(UINT32 prefOwner, PI_VCG_GET_MP_LIST_RSP *pMPList,
                                       PI_TARGETS_RSP *pTargets);
static void RM_R5ResyncNotMirrored(void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Start the RAID 5 Stripe Resync Monitor Task.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RM_R5StripeResyncMonitorStart(void)
{
    /*
     * Check if the monitor task is already running, if it is not then
     * create the task and set the in progress flag to assume there are
     * operations in progress.
     */
    if (pR5StripeResyncMonitorTaskPCB == NULL)
    {
        /*
         * Assume RAID 5 stripe resync operations are in progress until
         * the monitor task has a chance to investigate the raids to
         * determine the truth.
         */
        bR5StripeResyncInProgress = true;

        /*
         * Create the monitor task.
         */
        pR5StripeResyncMonitorTaskPCB = TaskCreate(RM_R5StripeResyncMonitorTask, NULL);
    }
}


/**
******************************************************************************
**
**  @brief      Task to monitor the progress of RAID 5 stripe resync
**              operations - FORKED
**
**  @param      UINT32 dummy - required parameter for forking a task.
**
**  @return     none
**
******************************************************************************
**/
void RM_R5StripeResyncMonitorTask(UNUSED TASK_PARMS *parms)
{
    PI_RAIDS_RSP *pRaids = NULL;
    PI_RAID_INFO_RSP *pRaid = NULL;
    UINT16      i = 0;

    dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: ENTER\n");

    do
    {
        dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: Get the raids\n");

        /*
         * Get the currently defined raids.
         */
        pRaids = Raids();

        if (pRaids != NULL)
        {
            pRaid = (PI_RAID_INFO_RSP *)pRaids->raids;

            dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: Loop on the raids (0x%x)\n",
                    pRaids->count);

            /*
             * Loop on all the raids to see if there is a stripe resync
             * in progress.
             */
            for (i = 0; i < pRaids->count; ++i)
            {
                /*
                 * Check the astatus field to see if the stripe resync
                 * in progress bit is set.
                 */
                if (BIT_TEST(pRaid->aStatus, RD_A_R5SRIP))
                {
                    dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: RIP (0x%x)\n",
                            pRaid->rid);

                    /*
                     * There is a stripe resync in progress so make sure
                     * we continue looping.
                     */
                    bR5StripeResyncInProgress = true;

                    /*
                     * Sleep for 1 second(s) to give the PROC a chance
                     * to complete the stripe resync operations.
                     */
                    TaskSleepMS(1000);

                    break;
                }

                pRaid = (PI_RAID_INFO_RSP *)((UINT8 *)pRaid + pRaid->header.len);
            }

            /*
             * If the index into the raids is equal to the count, there
             * were no raids running stripe resync operations.  Clear
             * the in progress indicator.
             */
            if (i == pRaids->count)
            {
                dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: No RIP\n");

                bR5StripeResyncInProgress = false;
            }

            Free(pRaids);
        }
    } while (bR5StripeResyncInProgress);

    /*
     * Clear our PCB, so that we can run again.
     */
    pR5StripeResyncMonitorTaskPCB = NULL;

    dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncMonitorTask: EXIT\n");
}


/**
******************************************************************************
**
**  @brief      Query if there are RAID5 stripe resync operations in progress.
**
**  @param      none
**
**  @return     true if there are operations in progress, false otherwise.
**
******************************************************************************
**/
bool RM_R5StripeResyncInProgress(void)
{
    return bR5StripeResyncInProgress;
}


/**
******************************************************************************
**
**  @brief      Starts the required processing for handling RAID 5 mirror
**              records at power-up time in a replacement scenario.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RM_R5PowerupReplacement(void)
{
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-R5 Replacement processing");

    /*
     * Mark all raids as requiring parity scan.
     */
    SM_MRResyncWithRetry(MRBALLRAIDS, 0, NULL);
}


/**
******************************************************************************
**
**  @brief      Starts the required processing for handling RAID 5 mirror
**              records at power-up time.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RM_R5PowerupProcessing(void)
{
    PI_VCG_GET_MP_LIST_RSP *pMPList = NULL;
    RM_R5_MISSING_LIST aMissing[MAX_CONTROLLERS];

    /*
     * Initialize the missing controller list to all 0xFFFF.
     */
    memset(aMissing, 0xFF, sizeof(RM_R5_MISSING_LIST) * MAX_CONTROLLERS);

    /*
     * Get the current mirror partner list.
     */
    pMPList = SM_GetMirrorPartnerList();

    if (pMPList != NULL)
    {
        /*
         * Build the list of missing controllers.  This is defined
         * as controller that were in the mirror partner list but
         * are not in the active controller map.
         */
        RM_R5BuildMissingControllerList(pMPList, aMissing);

        /*
         * Now that we have determined which controllers are missing,
         * fail the targets over to good controllers and resync the
         * RAID 5 raids.
         */

        /*
         * Fail the missing controllers to its mirror partners.
         */
        RM_R5FailMissingControllers(pMPList, aMissing);

        /*
         * Resync the raid 5 raids on the controllers.
         */
        RM_R5ResyncControllers(pMPList, aMissing);

        /*
         * Free the mirror partner list.
         */
        Free(pMPList);
    }

    /*
     * Start resync operations on raids that are not mirrored.
     */
    RM_R5ResyncNotMirrored();
}


/**
******************************************************************************
**
**  @brief      Searches the mirror partner list and looks for controllers
**              that should be considered missing.  If the source of the
**              mirror partner entry is not in the active controller map
**              it is considered missing.
**
**  @param      PI_VCG_GET_MP_LIST_RSP* pMPList - Pointer to the mirror
**                                                partner list.
**  @param      UINT16* pMissingList - Pointer to the start of an array
**                                     that will get filled with the
**                                     indexes into the mirror partner
**                                     list of the missing controllers.
**
**  @return     none
**
******************************************************************************
**/
void RM_R5BuildMissingControllerList(PI_VCG_GET_MP_LIST_RSP *pMPList,
                                     RM_R5_MISSING_LIST *pMissingList)
{
    UINT16      cActive;
    UINT16      iList;
    UINT16      iActive;
    UINT16      iMissing = 0;
    UINT32      controllerSN;
    UINT32      mirrorPartner;
    bool        bActive;

    /*
     * Get the count of active controllers from the ACM.
     */
    cActive = ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr());

    /*
     * First task is to loop through the mirror partner list
     * and determine which controllers have passed on to a
     * better life.
     */
    for (iList = 0; iList < pMPList->count; ++iList)
    {
        /*
         * Each time through the loop, assume that the controller
         * is missing (not active), missing until proven otherwise.
         */
        bActive = false;

        for (iActive = 0; iActive < cActive; ++iActive)
        {
            controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(iActive));

            if (controllerSN == pMPList->list[iList].source)
            {
                /*
                 * Hey, the controller that once was lost has now
                 * been found.
                 */
                bActive = true;
                break;
            }
        }

        /*
         * If the controller has truely pass on, add it to the missing
         * controller list.
         */
        if (!bActive)
        {
            pMissingList[iMissing].from = pMPList->list[iList].source;
            pMissingList[iMissing].mp = pMPList->list[iList].dest;

            /*
             * If the controller is currently mirroring to itself then
             * set the TO controller to itself also, it doesn't need
             * to be failed over.
             */
            if (pMissingList[iMissing].from == pMissingList[iMissing].mp)
            {
                pMissingList[iMissing].to = pMissingList[iMissing].mp;
            }

            ++iMissing;
        }
    }

    /*
     * Loop through the missing controller list and determine who
     * these controllers should fail over to.  This code will look
     * to see if the current mirror partner for the missing controller
     * is active.  If it is active that is the controller to use
     * otherwise the search will continue for the mirror partner's
     * mirror partner and so on...
     */
    for (iMissing = 0; iMissing < MAX_CONTROLLERS; ++iMissing)
    {
        /*
         * Assume the mirror partner is not active until proven otherwise.
         */
        bActive = false;

        /*
         * Get the mirror partner for this missing controller.
         */
        mirrorPartner = pMissingList[iMissing].mp;

        iActive = 0;

        if (pMissingList[iMissing].from != pMissingList[iMissing].mp)
        {
            while (!bActive)
            {
                controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(iActive));

                if (mirrorPartner == controllerSN)
                {
                    bActive = true;
                    break;
                }

                ++iActive;

                /*
                 * Have we reached the end of the active list?  If so then
                 * the mirror partner is not active and we need to find
                 * the next available mirror partner.
                 */
                if (iActive == cActive)
                {
                    /*
                     * Loop through the mirror partner list to find the
                     * entry for this mirror partner, this will give us
                     * that controllers partner who is the next in line
                     * to take over the targets.
                     */
                    for (iList = 0; iList < pMPList->count; ++iList)
                    {
                        if (mirrorPartner == pMPList->list[iList].source)
                        {
                            mirrorPartner = pMPList->list[iList].dest;
                            break;
                        }
                    }

                    /*
                     * Reset the active index.
                     */
                    iActive = 0;
                }
            }
        }

        pMissingList[iMissing].to = mirrorPartner;
    }

    /*
     * Log/Output the missing controller information that was built.
     */
    for (iMissing = 0; iMissing < MAX_CONTROLLERS; ++iMissing)
    {
        if (pMissingList[iMissing].from != 0xFFFFFFFF)
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-Missing Controller (0x%x, 0x%x, 0x%x)",
                       pMissingList[iMissing].from, pMissingList[iMissing].mp,
                       pMissingList[iMissing].to);
        }
    }
}


/**
******************************************************************************
**
**  @brief      Fails controllers in the missing list if they are not
**              currently mirrored to themselves (that indicates they
**              have already been failed).
**
**  @param      PI_VCG_GET_MP_LIST_RSP* pMPList - Pointer to the mirror
**                                                partner list.
**  @param      UINT16* pMissingList - Pointer to the start of an array
**                                     that contains the indexes into
**                                     the mirror partner list of the
**                                     missing controllers.
**
**  @return     none
**
******************************************************************************
**/
void RM_R5FailMissingControllers(UNUSED PI_VCG_GET_MP_LIST_RSP *pMPList,
                                 RM_R5_MISSING_LIST *pMissingList)
{
    UINT16      iMissing;
    INT32       rc = GOOD;

    for (iMissing = 0; iMissing < MAX_CONTROLLERS; ++iMissing)
    {
        /*
         * The missing list is a packed list of indexes into the
         * mirror partner list.  Once an index is 0xFFFF there
         * are no more items to process.
         */
        if (pMissingList[iMissing].from == 0xFFFFFFFF)
        {
            break;
        }

        /*
         * If the controller is mirroring to itself then it has
         * already been failed over, if not then it needs to be
         * failed to get the targets moved (and raid ownership).
         */
        if (pMissingList[iMissing].from != pMissingList[iMissing].mp)
        {
            /*
             * Fail the controller to its mirror partner.
             */
            rc = FailCtrl(pMissingList[iMissing].from, pMissingList[iMissing].to,
                          FALSE, 0xFFFF);

            /*
             * Signal all the active slaves to retrieve the new configuration.
             */
            RMSlavesConfigurationUpdate(X1_ASYNC_CONFIG_ALL, TRUE);
        }
    }
}


/**
******************************************************************************
**
**  @brief      Fails controllers in the missing list if they are not
**              currently mirrored to themselves (that indicates they
**              have already been failed).
**
**  @param      PI_VCG_GET_MP_LIST_RSP* pMPList - Pointer to the mirror
**                                                partner list.
**  @param      UINT16* pMissingList - Pointer to the start of an array
**                                     that contains the indexes into
**                                     the mirror partner list of the
**                                     missing controllers.
**
**  @return     none
**
******************************************************************************
**/
void RM_R5ResyncControllers(PI_VCG_GET_MP_LIST_RSP *pMPList,
                            RM_R5_MISSING_LIST *pMissingList)
{
    INT32       rc = GOOD;
    PI_TARGETS_RSP *pTargets = NULL;
    UINT16      iMissing;
    UINT16      iMirrorToFailed;
    UINT32      controllerSN = 0;
    UINT16      iActive;
    UINT16      cActive;
    UINT8       resyncType;

    /*
     * Get the count of active controllers from the ACM.
     */
    cActive = ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr());

    /*
     * Get the current target configuration.
     */
    pTargets = Targets(GetMyControllerSN());

    for (iMissing = 0; iMissing < MAX_CONTROLLERS; ++iMissing)
    {
        /*
         * The missing list is a packed list of indexes into the
         * mirror partner list.  Once an index is 0xFFFF there
         * are no more items to process.
         */
        if (pMissingList[iMissing].from == 0xFFFFFFFF)
        {
            break;
        }

        /*
         * Search the mirror partner list to see if anyone is mirroring to
         * the failed controller.  If so, that controller needs to be told
         * to continue without its mirror partner.
         */
        for (iMirrorToFailed = 0; iMirrorToFailed < pMPList->count; ++iMirrorToFailed)
        {
            /*
             * If this is not the failed controller entry in the list and
             * the controller is mirroring to the failed controller, tell it
             * to continue without its mirror partner.
             */
            if (pMPList->list[iMirrorToFailed].dest == pMissingList[iMissing].from)
            {
                for (iActive = 0; iActive < cActive; ++iActive)
                {
                    controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(iActive));

                    if (controllerSN == pMPList->list[iMirrorToFailed].source)
                    {
                        /*
                         * Tell the controller to continue without its
                         * mirror partner.
                         */
                        rc = SM_ContinueWithoutMP(pMPList->list[iMirrorToFailed].source);

                        /*
                         * Tell the controller to flush its BE write cache
                         * information since its partner is no longer around
                         * to do it for itself.
                         */
                        rc = WCacheFlushBEWC(pMPList->list[iMirrorToFailed].source);

                        break;
                    }
                }
            }
        }

        /*
         * If this is a replacement controller skip the actual mirror
         * resync processing as a full resync will be done by the
         * replacement controller processing.
         */
        if (IsReplacementController())
        {
            continue;
        }

        /*
         * Assume a stripe resync is required unless we determine otherwise.
         */
        resyncType = MRBSTRIPE;

        /*
         * If the missing controller is mirroring to itself then
         * the controller to resync needs to be found via the
         * target records (who owns the targets for this missing
         * controller).
         *
         * If it is not mirroring to itself the controller to
         * resync is its partner.
         */
        if (pMissingList[iMissing].from == pMissingList[iMissing].to)
        {
            controllerSN = RM_R5GetResyncController(pMissingList[iMissing].from,
                                                    pMPList, pTargets);
        }
        else
        {
            controllerSN = pMissingList[iMissing].to;

            /*
             * If the TO controller is not the controllers mirror
             * partner then the resync records are not valid and
             * all the raids need to be resync'd.
             */
            if (pMissingList[iMissing].mp != pMissingList[iMissing].to)
            {
                resyncType = MRBALLRAIDS;
            }
        }

        /*
         * If there is a controller that needs the resync request, send it.
         */
        if (controllerSN != 0)
        {
            SM_ResyncMirrorRecords(controllerSN, resyncType, 0);
        }
    }

    /*
     * Free the targets list.
     */
    Free(pTargets);
}


/**
******************************************************************************
**
**  @brief      Looks through the available targets to find the controller
**              that currently owns the targets for the controller at the
**              given index (iPrefOwner) in the mirror partner list.
**
**  @param      UINT16 iPrefOwner - index into the mirror partner list for
**                                  the controller which the resync
**                                  controller is required
**  @param      PI_VCG_GET_MP_LIST_RSP* pMPList - Pointer to the mirror
**                                                partner list.
**
**  @return     UINT32 - Controller serial number for the controller that
**                       will require the resync or 0 if the controller
**                       cannot be found or does not really need to resync.
**
******************************************************************************
**/
UINT32 RM_R5GetResyncController(UINT32 prefOwner,
                                PI_VCG_GET_MP_LIST_RSP *pMPList,
                                PI_TARGETS_RSP *pTargets)
{
    UINT16      iTarget;
    UINT32      resyncController = 0;
    UINT16      iList = 0;

    if (pTargets == NULL)
    {
        return (0);             /* Controller can not be found. */
    }

    dprintf(DPRINTF_DEFAULT, "RM_R5GetResyncController: Searching targets (%d)\n",
            pTargets->count);

    /*
     * Loop through the targets to find one that is for the
     * preferred owner.
     */
    for (iTarget = 0; iTarget < pTargets->count; ++iTarget)
    {
        /*
         * Check if this targets preferred owner is the one
         * we are looking for.
         */
        if (pTargets->targetInfo[iTarget].powner == prefOwner)
        {
            /*
             * This is the preferred owner's target, the current
             * owner of the target is the resync controller we
             * are trying to find.
             */
            resyncController = pTargets->targetInfo[iTarget].owner;

            dprintf(DPRINTF_DEFAULT, "RM_R5GetResyncController: Found targets for missing controller (0x%x)\n",
                    resyncController);

            break;
        }
    }

    /*
     * If we found a resync controller there is still a chance that
     * it may not need to have the RAID 5 stripe resync operations.
     */
    if (resyncController != 0)
    {
        dprintf(DPRINTF_DEFAULT, "RM_R5GetResyncController: Checking if resync controller mirrors to itself (0x%x)\n",
                resyncController);

        /*
         * Loop through the mirror partner list for the resync
         * controller.
         */
        for (iList = 0; iList < pMPList->count; ++iList)
        {
            /*
             * If this is the resync controller and it is mirroring
             * to itself, it doesn't need to be recync'd.
             *
             * NOTE: This check may not work for N-way since the
             *       controller mirror in a round-robin fashion.
             */
            if (pMPList->list[iList].source == resyncController &&
                pMPList->list[iList].source == pMPList->list[iList].dest)
            {
                dprintf(DPRINTF_DEFAULT, "RM_R5GetResyncController: Resync controller mirrors to itself\n");

                resyncController = 0;
                break;
            }
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "RM_R5GetResyncController: Failed to find the targets for the missing controller.\n");
    }

    /*
     * Return the serial number of the resync controller, it may be
     * zero if we could not find the controller or if it doesn't need
     * to be resync'd.
     */
    return resyncController;
}


/**
******************************************************************************
**
**  @brief      Start resync operations on any raids that are not
**              currently mirrored.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RM_R5ResyncNotMirrored(void)
{
    UINT16      i = 0;
    UINT32      controllerSN = 0;

    /*
     * Loop through the active controllers and send the RAID 5 mirror resync
     * request with the NOT MIRRORED option.
     */
    for (i = 0; i < ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()); ++i)
    {
        controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(i));

        /*
         * Send the RAID 5 mirror resync request to the controller.
         */
        if (SM_ResyncMirrorRecords(controllerSN, MRBALLNOTMIRROR, 0) != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "RM_R5StripeResyncNotMirrored: Failed to resync controller (0x%x)\n",
                    controllerSN);
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
