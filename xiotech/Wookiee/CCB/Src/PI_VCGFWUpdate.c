/* $Id: PI_VCGFWUpdate.c 145038 2010-08-03 19:33:37Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_VCGFWUpdate.c
** MODULE TITLE:    Packet Interface for VCG FW update commands
**
** DESCRIPTION:     Handler functions for Failover and Failback request packets.
**
** Copyright (c) 2002-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "cps_init.h"
#include "CacheManager.h"
#include "CacheRaid.h"
#include "debug_files.h"
#include "EL.h"
#include "errorCodes.h"
#include "ipc_sendpacket.h"
#include "PacketInterface.h"
#include "PI_VDisk.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "serial_num.h"
#include "sm.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "L_Misc.h"
#include "PI_CmdHandlers.h"
#include "ddr.h"
#include "LargeArrays.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define SEC_WAIT_FOR_ELECTION_TO_COMPLETE           60
#define SEC_WAIT_FOR_ELECTION_AND_MP_TO_COMPLETE    (SEC_WAIT_FOR_ELECTION_TO_COMPLETE+120)
#define SEC_WAIT_FOR_RAID5_MIRRORING_CHECK          20

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 Raid5NotMirroringCheck(INT32 timeoutSec);
static INT32 RaidsDegradedOrInitingCheck(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_RollingUpdatePhase()
**
** Description: Failover a controller's targets to ready it for
**              firmware upgrades, and rebalance the targets after
**              an upgrade.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_RollingUpdatePhase(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_ROLLING_UPDATE_PHASE_REQ *phaseReqP = NULL;
    INT32       rc = 0;
    INT32       error = 0;
    UINT32      mirrorPartnerSN;
    UINT32      count = 0;
    MODEDATA    data;
    MODEDATA    mask;
    bool        bTempDisableCache = false;
    bool        bStopIO = false;

    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));

    if (pRspPacket)
    {
        phaseReqP = (PI_ROLLING_UPDATE_PHASE_REQ *)pReqPacket->pPacket;
    }

    dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Starting phase %u for controller %u\n",
            phaseReqP->phase, phaseReqP->controllerSN);

    /*
     * NOTE: We are passing in 'controllerSN' here, but the raid and mirror
     * partner checks in this function assume that the contoller that is going
     * to inactivated is THIS controller (where this function is running).
     * ICON never issues this call for a different controller, so we are OK.
     * This should be fixed at some point though to avoid confusion.
     */

    do
    {
        /*
         * Phase 1 is called prior to the firmware being updated on a
         * controller.
         */
        if (phaseReqP->phase == 1)
        {
            while (TRUE)
            {
                /*
                 * Wait here if an another election is running
                 */
                if (EL_TestInProgress() ||
                    (RMGetState() != RMRUNNING && RMGetState() != RMDOWN))
                {
                    dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Waiting for previous election to complete ... el:%u rm:%u\n",
                            EL_TestInProgress(), RMGetState());
                }

                count = SEC_WAIT_FOR_ELECTION_AND_MP_TO_COMPLETE;

                mirrorPartnerSN = GetCachedMirrorPartnerSN();
                while (EL_TestInProgress() ||
                       (RMGetState() != RMRUNNING && RMGetState() != RMDOWN) ||
                       (mirrorPartnerSN == 0) || (mirrorPartnerSN == GetMyControllerSN()))
                {
                    /*
                     * Sleep for 1 sec and check again.
                     */
                    TaskSleepMS(1000);

                    if (--count == 0)
                    {
                        if (mirrorPartnerSN == 0 ||
                            (mirrorPartnerSN == GetMyControllerSN()))
                        {
                            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: no valid mirror partner found ... mp: %u\n",
                                    mirrorPartnerSN);
                            error = PHASE_BAD_MIRROR_PARTNER;
                            rc = PI_ERROR;
                            break;      /* to the end of this while() loop */
                        }

                        dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: mirror partner found: %u\n",
                                mirrorPartnerSN);

                        /*
                         * If it wasn't a mirror partner problem, it was an election
                         * problem.
                         */
                        dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Election never completed ... el:%u rm:%u\n",
                                EL_TestInProgress(), RMGetState());

                        error = PHASE_ELECTION_FAILURE;
                        rc = PI_ERROR;
                        break;  /* to the end of this while() loop */
                    }

                    /*
                     * Refresh the local mirror partner variable for the next
                     * pass thru the loop.
                     */
                    mirrorPartnerSN = GetCachedMirrorPartnerSN();
                }

                /*
                 * If an error got set in the above loop, break out of the
                 * while(1) loop here.
                 */
                if (rc != GOOD)
                {
                    break;
                }

                /*
                 * Check that all raid5's are mirroring.
                 */
                rc = Raid5NotMirroringCheck(SEC_WAIT_FOR_RAID5_MIRRORING_CHECK);

                /*
                 * Raid5NotMirroringCheck() returns 0 if all is well.
                 */
                if (rc != 0)
                {
                    error = PHASE_NON_MIRRORED_RAID5S;
                    break;
                }

                /*
                 * Check that there are no degraded raid5's
                 * and no raids (any type) that ate init'ing.
                 */
                rc = RaidsDegradedOrInitingCheck();

                /*
                 * RaidsDegradedOrInitingCheck() returns 0 if all is well.
                 */
                if (rc != 0)
                {
                    error = PHASE_RAIDS_NOT_READY;
                    break;
                }

                /*
                 * Send a test PING to the mirror partner.
                 */
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Pinging controller %u\n",
                        mirrorPartnerSN);

                rc = IpcSendPingWithRetries(mirrorPartnerSN, SENDPACKET_ANY_PATH, 3);

                if (rc != GOOD)
                {
                    error = PHASE_PING_FAILURE;
                    break;
                }

                /*
                 * After all of those checks, make sure the mirror partner is the
                 * same as what we started with. If not, something must have
                 * happened -- apparently another election has run and our
                 * mirror partner got switched. Oh well, start over...
                 */
                if (mirrorPartnerSN == GetCachedMirrorPartnerSN())
                {
                    break;      /* all is well, break out of while(1) */
                }
            }

            /*
             * If an error got set in the above loop, break out of the
             * do-while(0) loop here.
             */
            if (rc != GOOD)
            {
                break;
            }

            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Disable Cache\n");

            /*
             * Temporarily disable caching to allow the failover processing
             * to proceed a little faster.  This will also wait for the cache
             * to be flushed before continuing.
             */
            rc = RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_INACTIVATE, 0);

            if (rc == PI_GOOD)
            {
                bTempDisableCache = true;
                RMWaitForCacheFlush();
            }
            else
            {
                error = PHASE_TDISCACHE_FAILED;
                rc = PI_ERROR;
                break;
            }

            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Stop IO\n");

            /*
             * Stop the IO on the controller being updated.
             */
            rc = StopIO(phaseReqP->controllerSN,
                        STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                        STOP_NO_SHUTDOWN, START_STOP_IO_USER_CCB_SM, TMO_RCU_STOP_IO);

            /*
             * If the stop request completed successfully or a timeout
             * occurred during the request, set the flag indicating
             * that a stop request is outstanding so we know to start
             * IO in case of an error after this.
             */
            if (rc == PI_GOOD || rc == PI_TIMEOUT)
            {
                bStopIO = true;
            }

            if (rc != GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Failed to stop IO (rc: 0x%x)\n", rc);

                error = PHASE_STOP_IO_FAILED;
                rc = PI_ERROR;
                break;
            }

            /*
             * Set the controller to FD_STATE_UPDATE_INACTIVE.
             */
            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Setting controller state to UPDATE_INACTIVE\n");

            rc = WriteFailureDataState(phaseReqP->controllerSN,
                                       FD_STATE_FIRMWARE_UPDATE_INACTIVE);

            if (rc != GOOD)
            {
                error = PHASE_STATE_SET_FAILURE;
                break;
            }

            /*
             * Run an election to take this controller out of the heartbeat tree.
             * Resource manager will also fail its targets off at the end of
             * the election, since it won't show up in the active controller
             * map.
             */
            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Calling an election\n");

            rc = EL_DoElection();

            if (rc != GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: EL_DoElection() failed, rc = %d\n", rc);

                error = PHASE_ELECTION_FAILURE;
                rc = PI_ERROR;
                break;
            }

            /*
             * Wait here for the election to finish
             */
            if (EL_TestInProgress())
            {
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Waiting for the election to complete ...\n");
            }

            count = SEC_WAIT_FOR_ELECTION_TO_COMPLETE;

            while (EL_TestInProgress())
            {
                TaskSleepMS(1000);
                if (--count == 0)
                {
                    dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Election never completed ... el:%u\n",
                            EL_TestInProgress());

                    error = PHASE_ELECTION_FAILURE;
                    rc = PI_ERROR;
                    break;
                }
            }

            /*
             * If we are not now in the UPDATE_INACTIVE state, something went
             * terribly wrong...
             */
            if (GetControllerFailureState() != FD_STATE_FIRMWARE_UPDATE_INACTIVE)
            {
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Not in UPDATE_INACTIVE state. (%u)\n",
                        GetControllerFailureState());
                error = PHASE_ELECTION_FAILURE;
                rc = PI_ERROR;
                break;
            }

            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Reset FE Interfaces\n");

            /*
             * Reset and hold the FE interfaces to make sure the hosts
             * do not attempt any IO.
             */
            rc = ResetInterfaceFE(phaseReqP->controllerSN, 0xFF, RESET_PORT_NO_INIT);

            if (rc != GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Failed to reset FE interfaces (rc: 0x%x)\n",
                        rc);

                error = PHASE_RESET_QLOGIC_FAILED;
                rc = PI_ERROR;
                break;
            }

            /*
             * Tell the BEP that this controller will be a slave controller.
             */
            MRP_Awake(MRAWAKE_SLAVE);

            /*
             * Wait until the targets move off of this controller.  This
             * function will restore the configuration information and
             * check if this controller still owns targets.  If it still
             * owns targets it will again restore the configuration and
             * check.
             */
            RM_RestoreAndCheckForTargets();

            /*
             * Wait for the resync resources to be moved to another
             * controller.
             */
            RM_WaitForResyncInactivateOperations(180);

            /*
             * Disable failure manager.
             * This controller is now inactive, which means that it's not
             * handling any server I/O, so it's safe to run with FM disabled.
             */
            dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Disabling failure manager\n");

            data.ccb.bits = MD_FM_DISABLE;
            mask.ccb.bits = MD_FM_DISABLE;
            ModeSet(&data, &mask);

            /*
             * Set the clean shutdown flag
             */
            if (SetCleanShutdown() != GOOD)
            {
                /*
                 * This isn't catastrophic, just print it out.
                 */
                dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Failed to set \"Clean Shutdown Flag\"\n");
            }
        }
        else
        {
            rc = PI_PARAMETER_ERROR;
        }

    } while (FALSE);

    /*
     * If there is an error from above we need to check whether we should
     * be starting IO and caching back up.
     */
    if (rc != PI_GOOD)
    {
        /*
         * If the stop is outstanding and we are not returning a good status
         * we should attempt to start IO back up.
         */
        if (bStopIO)
        {
            StartIO(phaseReqP->controllerSN,
                    START_IO_OPTION_CLEAR_ONE_STOP_COUNT, START_STOP_IO_USER_CCB_SM, 0);
        }

        /*
         * If cache has been disabled and the overall request is not good
         * we need to re-enable caching.  Otherwise the controller will be
         * reset and the caching will be enabled when necessary during
         * the power-up and reallocation sequencing.
         */
        if (bTempDisableCache)
        {
            (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD,
                                     TEMP_DISABLE_INACTIVATE, 0);
        }
    }

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = error;
    pRspPacket->pHeader->length = 0;

    dprintf(DPRINTF_DEFAULT, "PI_RollingUpdatePhase: Exit. status: %d  error: 0x%X\n",
            rc, error);

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    Raid5NotMirroringCheck()
**
** Description: Check for valid mirror partner and that none of this
**              controller's raid5's are non-mirroring.
**
** Inputs:      timeoutSec - timeout in seconds to keep trying.
**
** Returns:     GOOD (meaning all is well to continue)  or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 Raid5NotMirroringCheck(INT32 timeoutSec)
{
    MR_LIST_RSP *pRaids = NULL;
    INT32       rc = ERROR;
    bool        bNotMirroring = true;

    /*
     * Increment the 1 sec timeout request by 1 so that we get one final test
     * before leaving the routine.  E.g. timeoutSec == 1, checks twice,
     * timeoutSec == 0, checks once.
     */
    ++timeoutSec;

    while (bNotMirroring && timeoutSec > 0)
    {
        /* Check that owned Raid5's are mirroring */
        dprintf(DPRINTF_DEFAULT, "Raid5NotMirroringCheck: Checking for non-mirroring Raid5's\n");

        /* Invalidate and wait for virtual disk and raid caches to be refreshed. */
        (void)InvalidateCache(
#ifndef NO_RAID_CACHE
            CACHE_INVALIDATE_RAID |
#endif  /* NO_RAID_CACHE */
#ifndef NO_VDISK_CACHE
            CACHE_INVALIDATE_VDISK |
#endif  /* NO_VDISK_CACHE */
            0, true);

        /* Get the list of Raids owned by this controller. */
        pRaids = RaidsOwnedByController(GetMyControllerSN());

        /* Check if there are raids that are not mirroring. */
        rc = CheckForNotMirroringRaid5s(pRaids);

        /*
         * Free the raid list, they were only needed for the
         * not mirroring check.
         */
        Free(pRaids);

        if (rc == 0)
        {
            rc = GOOD;
            bNotMirroring = false;
            break;
        }

        /* Decrement the timout counter */
        --timeoutSec;

        /*
         * If there are still loops to attempt, sleep for a little
         * bit and then start the loop again.
         */
        if (timeoutSec > 0)
        {
            TaskSleepMS(1000);
        }
    }

    if (bNotMirroring)
    {
        dprintf(DPRINTF_DEFAULT, "Raid5NotMirroringCheck: Non-mirroring Raid5's found!\n");

        rc = ERROR;
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    RaidsDegradedOrInitingCheck()
**
** Description: Check for any raids that are Degraded or Init'ing.
**
** Inputs:      none
**
** Returns:     GOOD (meaning all is well to continue)  or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 RaidsDegradedOrInitingCheck(void)
{
    UINT16      numRaids;
    UINT32      count;
    UINT32      index1;
    UINT8      *pCacheEntry;
    MRGETRINFO_RSP *pTmpRaid;
#ifdef NO_RAID_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_raidcache;
#endif  /* NO_RAID_CACHE */

    dprintf(DPRINTF_DEFAULT, "RaidsDegradedOrInitingCheck: enter\n");

    /*
     * Wait until the RAID cache is not in the process of being updated.  Once
     * it is in that state make sure it is set to in use so a update doesn't
     * start while it is being used.
     */
#ifndef NO_RAID_CACHE
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);

    /* Get a pointer to the beginning of the raid cache. */
    pCacheEntry = cacheRaids;
#else   /* NO_RAID_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    pCacheEntry = cacheRaidBuffer_DMC.cacheRaidBuffer_DMC;
#endif  /* NO_RAID_CACHE */

    /* Loop through raids, and see if any are initializing or degraded raid5's. */
    numRaids = RaidsCount();

    for (index1 = 0, count = 0; index1 < numRaids; ++index1)
    {
        /* Get a pointer to the raid we need to look at. */
        pTmpRaid = (MRGETRINFO_RSP *)pCacheEntry;

        /* Check to see if this raid is initializing. */
        if ((pTmpRaid->status == RD_INIT) ||
            (pTmpRaid->status == RD_UNINIT) || (pTmpRaid->pctRem > 0))
        {
            dprintf(DPRINTF_DEFAULT, "Raid %hu Init'ing\n", pTmpRaid->rid);
            count++;
        }
        else if ((pTmpRaid->type == RD_RAID5) && (pTmpRaid->status == RD_DEGRADED))
        {
            dprintf(DPRINTF_DEFAULT, "Raid5 %hu Degraded\n", pTmpRaid->rid);
            count++;
        }

        /* Increment our cache pointer. */
        pCacheEntry += pTmpRaid->header.len;
    }

#ifndef NO_RAID_CACHE
    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheRaidsState);
#else   /* NO_RAID_CACHE */
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

    /* Done */
    dprintf(DPRINTF_DEFAULT, "Raids Init'ing or Degraded Count == %d\n", count);

    return count ? FAIL : GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
