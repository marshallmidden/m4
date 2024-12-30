/* $Id: RMCmdHdl.c 162911 2014-03-20 22:45:34Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       RMCmdHdl.c
** MODULE TITLE:    Resource Manager CCB Command Handler
**
** DESCRIPTION:     Defines the command layering within the Resoure Manager
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "CachePDisk.h"
#include "CacheVDisk.h"
#include "CacheManager.h"
#include "RMCmdHdl.h"
#include "PacketInterface.h"
#include "debug_files.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "ipc_session_manager.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "MR_Defs.h"
#include "misc.h"
#include "PI_Utils.h"
#include "PktCmdHdl.h"
#include "PR.h"
#include "rm.h"
#include "sm.h"
#include "mode.h"
#include "fm.h"
#include "X1_AsyncEventHandler.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "CT_history.h"

/*****************************************************************************
 ** Private defines
 *****************************************************************************/

/*
** Timeout for updating a local image.
*/
#define TMO_UPDATE_LCL_IMAGE    60000   /* 60  second timeout */

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
static UINT32 gLocalImageSequence[MAX_CONTROLLERS];

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
MUTEX       configUpdateMutex;

/*****************************************************************************
 ** Private function prototypes
 *****************************************************************************/
static void LocalImageUpdateSequence(void *pLocalImage);
static void RMSlavesConfigurationPropagation(UINT8 restoreOption, UINT32 reason);
static void ConfigurationUpdateFailure(UINT32 serialNum);

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    RMCommandHandlerPreProcessImpl
**
** Description: Implements the Resource Manager pre-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 RMCommandHandlerPreProcessImpl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    INT32       lock = FALSE;
    bool        bTempDisableCache = FALSE;
    MRGETPINFO_RSP *pPID = NULL;
    MRGETPINFO_RSP *pHSID = NULL;

    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_VCG_REMOVE_CONTROLLER_CMD:
            {
                /*
                 * The remove controller request is sent to both the
                 * master and the controller being removed and the
                 * configuration update should only happen from the
                 * master so check if this controller is the master
                 * before sending the configuration update.
                 */
                if (TestforMaster(GetMyControllerSN()))
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_RAID_CONTROL_CMD:
            {
                PI_RAID_CONTROL_REQ *pRequest;

                pRequest = (PI_RAID_CONTROL_REQ *)pReqPacket->pPacket;

                /*
                 * For raid control commands the configuration needs
                 * to be propogated only when scrubbing changed or
                 * the parity scan is being enabled or disabled.
                 */
                if ((pRequest->scrubcontrol & SCRUB_CHANGE) > 0 ||
                    (pRequest->paritycontrol & PARITY_SCAN_ENABLE) > 0 ||
                    (pRequest->paritycontrol & PARITY_SCAN_DISABLE) > 0)
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_PROC_NAME_DEVICE_CMD:
            {
                PI_PROC_NAME_DEVICE_REQ *pRequest;

                pRequest = (PI_PROC_NAME_DEVICE_REQ *)pReqPacket->pPacket;

                /*
                 * For the name device command we only want to cause a
                 * configuration propogation if the option is not a
                 * "retrieve controller".
                 */
                if (pRequest->option == MNDSERVER ||
                    pRequest->option == MNDVDISK || pRequest->option == MNDVCG)
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
            {
                PI_DEBUG_INIT_PROC_NVRAM_REQ *pRequest;

                pRequest = (PI_DEBUG_INIT_PROC_NVRAM_REQ *)pReqPacket->pPacket;

                /*
                 * Only lock the mutex if this is a full INIT of NVRAM.
                 */
                if (pRequest->type == MXNALL)
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_VCG_APPLY_LICENSE_CMD:
            {
                /*
                 * For the apply license command we only want to lock the
                 * mutext if there are multiple controllers in the group.
                 *
                 * If there is only one the configuration will not be
                 * propagated and it also handles the case of initial
                 * license apply and configuration (the initial configuration
                 * requires the mutex to be available in the master
                 * controller power-up).
                 */
                if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_PDISK_DEFRAG_CMD:
            {
                PI_PDISK_DEFRAG_REQ *pRequest;

                pRequest = (PI_PDISK_DEFRAG_REQ *)pReqPacket->pPacket;

                /*
                 * Only when we are killing the orphans do we want to
                 * lock the mutex.
                 */
                if (pRequest->id == MRDEFRAGMENT_ORPHANS)
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            break;

            /*
             * All of the following command require that the configuration
             * update mutex be locked until the request has been fulfuilled.
             */
        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
            if (((MRPDISKAUTOFAILBACKENABLEDISABLE_REQ *)(pReqPacket->pPacket))->options != 2)
            {
                lock = TRUE;
                bTempDisableCache = TRUE;
            }
            break;

        case PI_PDISK_FAIL_CMD:
            if (((PI_PDISK_FAIL_REQ *)pReqPacket->pPacket)->options != 0)
            {
                UINT16 ix;

                ix = ((PI_PDISK_FAIL_REQ *)pReqPacket->pPacket)->pid;
                if (ix < MAX_PHYSICAL_DISKS)
                {
                    pPID = (MRGETPINFO_RSP *)cachePhysicalDiskAddr[ix];
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "%s: pid %d out of range\n", __func__, ix);
                    pPID = NULL;
                }

                ix = ((PI_PDISK_FAIL_REQ *)pReqPacket->pPacket)->hspid;
                if (ix < MAX_PHYSICAL_DISKS)
                {
                    pHSID = (MRGETPINFO_RSP *)cachePhysicalDiskAddr[ix];
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "%s: hspid %d out of range\n", __func__, ix);
                    pHSID = NULL;
                }

                if (pPID != NULL && pHSID != NULL)
                {
                    if (pPID->pdd.devType != pHSID->pdd.devType)
                    {
                        rc = ERROR;
                        pRspPacket->pHeader->status = ERROR;
                        pRspPacket->pHeader->errorCode = DEINVHSPTYPE;
                        return (rc);
                    }
                }
                else
                {
                    lock = TRUE;
                    bTempDisableCache = TRUE;
                }
            }
            else
            {
                lock = TRUE;
                bTempDisableCache = TRUE;
            }

            break;

        case PI_PDISK_LABEL_CMD:
        case PI_PDISK_SPINDOWN_CMD:
        case PI_PDISK_FAILBACK_CMD:
        case PI_SET_GEO_LOCATION_CMD:
        case PI_CLEAR_GEO_LOCATION_CMD:
        case PI_PDISK_UNFAIL_CMD:
        case PI_PDISK_DELETE_CMD:
        case PI_DISK_BAY_DELETE_CMD:
        case PI_SERVER_CREATE_CMD:
        case PI_SERVER_DELETE_CMD:
        case PI_SERVER_SET_PROPERTIES_CMD:
        case PI_TARGET_SET_PROPERTIES_CMD:
        case PI_VDISK_SET_ATTRIBUTE_CMD:
        case PI_VDISK_SET_PRIORITY_CMD:
        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
        case PI_ISCSI_SET_TGTPARAM_CMD:
        case PI_ISCSI_SET_CHAP_CMD:
        case PI_SETISNSINFO_CMD:
        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_EXPAND_CMD:
        case PI_VLINK_CREATE_CMD:
        case PI_VLINK_BREAK_LOCK_CMD:
        case PI_SERVER_ASSOCIATE_CMD:
        case PI_SERVER_DISASSOCIATE_CMD:
        case PI_VDISK_CONTROL_CMD:
        case PI_VDISK_DELETE_CMD:
        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_SET_CACHE_CMD:
        case PI_MISC_SET_WORKSET_INFO_CMD:
        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
        case PI_PROC_RESTORE_NVRAM_CMD:
        case PI_MISC_CFGOPTION_CMD:
            lock = TRUE;
            bTempDisableCache = TRUE;
            break;

        case PI_PROC_FAIL_CTRL_CMD:
            lock = TRUE;
            break;

            /*
             * The rest of the commands do not need any pre-processing.
             */
        default:
            break;
    }

    /*
     * If the command requires that cache be temporarily disabled, do it.
     */
    if (bTempDisableCache)
    {
        /*
         * Temporarily disable caching to allow configuration updates to
         * proceed a little smoother.  This will also wait for the cache
         * to be flushed before continuing.
         */
        (void)RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_CONFIG_PROP, 0);
        RMWaitForCacheFlush();
    }

    /*
     * If the lock flag has been set we need to lock the configuration
     * update mutex here.
     *
     * It will be unlocked in the post processing.
     */
    if (lock == TRUE)
    {
        /*
         * Aquire the mutex for the configuration update and hold until the
         * request has been fulfilled.
         */
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
    }

    if (rc != GOOD)
    {
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = (UINT8)rc;
        pRspPacket->pHeader->errorCode = 0;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    RMCommandHandlerPostProcessImpl
**
** Description: Implements the Resource Manager post-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 RMCommandHandlerPostProcessImpl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT32      reason = 0;             /* This matches lock in PreProc */
    UINT32      needToLock = FALSE;
    bool        bTempDisableCache = FALSE;
    UINT32      cacheDirty = 0;

    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_VCG_REMOVE_CONTROLLER_CMD:
            /*
             * The remove controller request is sent to both the
             * master and the controller being removed and the
             * configuration update should only happen from the
             * master so check if this controller is the master
             * before sending the configuration update.
             */
            if (TestforMaster(GetMyControllerSN()))
            {
                reason = X1_ASYNC_VCG_CFG_CHANGED;
                bTempDisableCache = TRUE;
            }
            break;

        case PI_RAID_CONTROL_CMD:
            {
                /*
                 * For raid control commands the configuration needs
                 * to be propogated only when scrubbing changed or
                 * the parity scan is being enabled or disabled.
                 */
                PI_RAID_CONTROL_REQ *pRequest;

                pRequest = (PI_RAID_CONTROL_REQ *)pReqPacket->pPacket;

                if ((pRequest->scrubcontrol & SCRUB_CHANGE) > 0 ||
                    (pRequest->paritycontrol & PARITY_SCAN_ENABLE) > 0 ||
                    (pRequest->paritycontrol & PARITY_SCAN_DISABLE) > 0)
                {
                    reason = X1_ASYNC_RCHANGED;
#ifndef NO_RAID_CACHE
                    cacheDirty = 1 << PI_CACHE_INVALIDATE_RAID;
#endif  /* NO_RAID_CACHE */
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_PROC_NAME_DEVICE_CMD:
            {
                /*
                 * For the name device command we only want to cause a
                 * configuration propogation if the option is not a
                 * "retrieve controller".
                 */
                PI_PROC_NAME_DEVICE_REQ *pRequest;

                pRequest = (PI_PROC_NAME_DEVICE_REQ *)pReqPacket->pPacket;

                if (pRequest->option == MNDSERVER)
                {
                    reason = X1_ASYNC_ZCHANGED;
                }
                else if (pRequest->option == MNDVDISK)
                {
                    reason = X1_ASYNC_VCHANGED;
#ifndef NO_VDISK_CACHE
                    cacheDirty = 1 << PI_CACHE_INVALIDATE_VDISK;
#endif  /* NO_VDISK_CACHE */
                }
                else if (pRequest->option == MNDVCG)
                {
                    reason = X1_ASYNC_VCG_CFG_CHANGED;
                }

                if (reason != 0)
                {
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
            {
                PI_DEBUG_INIT_PROC_NVRAM_REQ *pRequest;

                pRequest = (PI_DEBUG_INIT_PROC_NVRAM_REQ *)pReqPacket->pPacket;

                if (pRequest->type == MXNALL)
                {
                    reason = X1_ASYNC_CONFIG_ALL;
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_SERVER_CREATE_CMD:
            {
                PI_SERVER_CREATE_RSP *pResponse;

                pResponse = (PI_SERVER_CREATE_RSP *)pRspPacket->pPacket;

                /*
                 * Only if the server did not previously exists do we want
                 * to send the configuration update to the slave controllers.
                 */
                if (pResponse->flags == false)
                {
                    reason = X1_ASYNC_ZCHANGED;
                }

                bTempDisableCache = TRUE;
            }
            break;

        case PI_VCG_APPLY_LICENSE_CMD:
            reason = X1_ASYNC_FECHANGED;

            if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) <= 1)
            {
                needToLock = TRUE;
            }
            else
            {
                bTempDisableCache = TRUE;
            }
            break;

        case PI_PDISK_DEFRAG_CMD:
            {
                PI_PDISK_DEFRAG_REQ *pRequest;

                pRequest = (PI_PDISK_DEFRAG_REQ *)pReqPacket->pPacket;

                /*
                 * Only when we are killing the orphans do we want to
                 * lock the mutex.
                 */
                if (pRequest->id == MRDEFRAGMENT_ORPHANS)
                {
                    reason = X1_ASYNC_RCHANGED;
#ifndef NO_RAID_CACHE
                    cacheDirty = 1 << PI_CACHE_INVALIDATE_RAID;
#endif  /* NO_RAID_CACHE */
                    bTempDisableCache = TRUE;
                }
            }
            break;

        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
            if (((MRPDISKAUTOFAILBACKENABLEDISABLE_REQ *)(pReqPacket->pPacket))->options != 2)
            {
                reason = X1_ASYNC_PCHANGED;
                bTempDisableCache = TRUE;
#ifndef NO_PDISK_CACHE
                cacheDirty = 1 << PI_CACHE_INVALIDATE_PDISK;
#endif  /* NO_PDISK_CACHE */
            }
            break;

        case PI_PDISK_LABEL_CMD:
        case PI_PDISK_FAIL_CMD:
        case PI_PDISK_SPINDOWN_CMD:
        case PI_PDISK_FAILBACK_CMD:
        case PI_PDISK_UNFAIL_CMD:
        case PI_PDISK_DELETE_CMD:
        case PI_DISK_BAY_DELETE_CMD:
            reason = X1_ASYNC_PCHANGED;
            bTempDisableCache = TRUE;
#ifndef NO_PDISK_CACHE
            cacheDirty = 1 << PI_CACHE_INVALIDATE_PDISK;
#endif  /* NO_PDISK_CACHE */
            break;

        case PI_SET_GEO_LOCATION_CMD:
        case PI_CLEAR_GEO_LOCATION_CMD:
            reason = X1_ASYNC_PCHANGED | X1_ASYNC_VCHANGED;
            bTempDisableCache = TRUE;
#if !defined(NO_PDISK_CACHE) || !defined(NO_VDISK_CACHE)
            cacheDirty =
#ifndef NO_VDISK_CACHE
                    (1 << PI_CACHE_INVALIDATE_VDISK) |
#endif  /* NO_VDISK_CACHE */
#ifndef NO_PDISK_CACHE
                    (1 << PI_CACHE_INVALIDATE_PDISK) |
#endif  /* NO_PDISK_CACHE */
                    0;
#endif  /* NO_PDISK_CACHE || NO_VDISK_CACHE */
            break;

        case PI_SERVER_DELETE_CMD:
        case PI_SERVER_SET_PROPERTIES_CMD:
        case PI_TARGET_SET_PROPERTIES_CMD:
        case PI_ISCSI_SET_TGTPARAM_CMD:
        case PI_ISCSI_SET_CHAP_CMD:
        case PI_SETISNSINFO_CMD:
            reason = X1_ASYNC_ZCHANGED;
            bTempDisableCache = TRUE;
            break;

        case PI_VDISK_SET_ATTRIBUTE_CMD:
        case PI_VDISK_SET_PRIORITY_CMD:
        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_EXPAND_CMD:
        case PI_VLINK_CREATE_CMD:
        case PI_VLINK_BREAK_LOCK_CMD:
            reason = X1_ASYNC_VCHANGED;
            bTempDisableCache = TRUE;
#ifndef NO_VDISK_CACHE
            cacheDirty = 1 << PI_CACHE_INVALIDATE_VDISK;
#endif  /* NO_VDISK_CACHE */
            break;

        case PI_VDISK_CONTROL_CMD:
            if (((PI_VDISK_CONTROL_REQ *)(pReqPacket->pPacket))->subtype == MVCCOPYCONT)
            {
                /*
                 * Sleep awhile to give BE time to process the mirror request.
                 * This is here to prevent a scripting issue that issued back to
                 * back requests that ended up being processed at the same time.
                 * In this case the size of the vdisk could not be modified
                 * because it was the source of another mirror (second request).
                 * If the requests were handle serially, there is no problem.
                 */
                TaskSleepMS(1000);
            }
            /* Intentionally falling through to send change notifications */
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD:
        case PI_BATCH_SNAPSHOT_START_CMD:
        case PI_BATCH_SNAPSHOT_SEQUENCE_CMD:
        case PI_BATCH_SNAPSHOT_EXECUTE_CMD:
        case PI_SERVER_ASSOCIATE_CMD:
        case PI_SERVER_DISASSOCIATE_CMD:
        case PI_VDISK_DELETE_CMD:
            reason = X1_ASYNC_VCHANGED | X1_ASYNC_ZCHANGED;
            bTempDisableCache = TRUE;
#ifndef NO_VDISK_CACHE
            cacheDirty = 1 << PI_CACHE_INVALIDATE_VDISK;
#endif  /* NO_VDISK_CACHE */
            break;

        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_SET_CACHE_CMD:
            reason = X1_ASYNC_VCG_CFG_CHANGED;
            bTempDisableCache = TRUE;
            break;

        case PI_MISC_SET_WORKSET_INFO_CMD:
            reason = X1_ASYNC_VCG_WORKSET_CHANGED;
            bTempDisableCache = TRUE;
            break;

        case PI_PROC_FAIL_CTRL_CMD:
        case PI_SET_PR_CMD:
            reason = X1_ASYNC_FECHANGED;
            break;

        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
        case PI_PROC_RESTORE_NVRAM_CMD:
            reason = X1_ASYNC_CONFIG_ALL;
            bTempDisableCache = TRUE;
            break;

        case PI_MISC_CFGOPTION_CMD:
            reason = X1_ASYNC_CONFIG_ALL;
            break;

            /* The rest of the commands do not need any post-processing. */
        default:
            break;
    }

    /*
     * If there is a reason to send the configuration update, send it!
     *
     * In the case where there is not a reason code but the command
     * request was a server creation request the mutex still needs
     * to be unlocked.  The server create request will always lock
     * the mutex in the pre-processing and as such needs to always
     * unlock the mutex in the post-processing.
     */
    if (reason != 0)
    {
        /*
         * Only if the configuration operation succeeded should we send
         * the configuration update.  If the operation failed we may
         * still need to unlock the configuration update mutex.
         */
        if (pRspPacket->pHeader->status == PI_GOOD)
        {
            RMSlavesConfigurationUpdate(reason, needToLock);
            /*
             * make the appropriate ccb cache dirty based
             * on the cacheDirty flag
             */
#ifndef NO_VDISK_CACHE
            if (cacheDirty & (1 << PI_CACHE_INVALIDATE_VDISK))
            {
                PI_MakeVDisksCacheDirty();
            }
#endif  /* NO_VDISK_CACHE */
#ifndef NO_PDISK_CACHE
            if (cacheDirty & (1 << PI_CACHE_INVALIDATE_PDISK))
            {
                PI_MakePDisksCacheDirty();
            }
#endif  /* NO_PDISK_CACHE */
#ifndef NO_RAID_CACHE
            if (cacheDirty & (1 << PI_CACHE_INVALIDATE_RAID))
            {
                PI_MakeRaidsCacheDirty();
            }
#endif  /* NO_RAID_CACHE */
        }
        else
        {
            /*
             * If this configuration update was not supposed to lock
             * the mutex itself then it was locked in the pre-command
             * processing and now needs to be unlocked.
             */
            if (needToLock == FALSE)
            {
                /* Unlock the configuration update mutex. */
                UnlockMutex(&configUpdateMutex);
            }
        }
    }
    else if (pReqPacket->pHeader->commandCode == PI_SERVER_CREATE_CMD)
    {
        /* Unlock the configuration update mutex. */
        UnlockMutex(&configUpdateMutex);
    }

    /*
     * If the command required that cache be temporarily disabled we now
     * need to clear that to re-enable cache.
     */
    if (bTempDisableCache)
    {
        /*
         * Re-enable caching since we have completed the configuration
         * update.
         */
        (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_CONFIG_PROP,
                                 T_DISABLE_CLEAR_ONE);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: RMSlavesConfigurationUpdate
**
**  Description:
**      Loops through the active controllers in the virtual controller group
**      (VCG) and sends each an IPC message (IPC_CONFIGURATION_UPDATE).  This
**      message tells them to reload their configuration.
**
**  Inputs:
**      UINT32 reason - Any combination of:
**                          X1_ASYNC_PCHANGED
**                          X1_ASYNC_RCHANGED
**                          X1_ASYNC_VCHANGED
**                          X1_ASYNC_HCHANGED
**                          X1_ASYNC_ACHANGED
**                          X1_ASYNC_ZCHANGED
**                          X1_ASYNC_VCG_ELECTION_STATE_CHANGE
**                          X1_ASYNC_VCG_ELECTION_STATE_ENDED
**                          X1_ASYNC_VCG_POWERUP
**                          X1_ASYNC_VCG_CFG_CHANGED
**      UINT32 needToLock - TRUE if it is necessary to lock the configUpdateMutex
**                          FALSE if the mutex is already locked
**--------------------------------------------------------------------------*/
void RMSlavesConfigurationUpdate(UINT32 reason, UINT32 needToLock)
{
    /* If this controller is not the master it does not need to do anything */
    if (!TestforMaster(GetMyControllerSN()))
    {
        dprintf(DPRINTF_RMCMDHDL, "RMSlavesConfigurationUpdate: Not master\n");

        /*
         * If this configuration update was not supposed to lock
         * the mutex itself then it was locked in the pre-command
         * processing and now needs to be unlocked.
         */
        if (!needToLock)
        {
            /*
             * Unlock the configuration update mutex.
             */
            UnlockMutex(&configUpdateMutex);
        }
        return;
    }

    /*
     * Aquire the mutex for the configuration update and hold until the
     * request has been fulfilled.
     */
    if (needToLock)
    {
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
    }

    RMSlavesConfigurationPropagation(MRNOFSYS | MRNOOVERLAY, reason);

    /*
     * As long as we have executed successfully to this point and we have more
     * than one controller in our VCG we need to send each of the slaves a
     * refresh configuration IPC packet.
     */
    RMSlavesRefreshNVRAM(reason);

    /*
     * Unlock the configuration update mutex.
     */
    UnlockMutex(&configUpdateMutex);
}

/*----------------------------------------------------------------------------
**  Function Name: RMSlavesRefreshNVRAM
**
**  Description:
**      Loops through the active controllers in the virtual controller group
**      (VCG) and sends each an IPC message (IPC_CONFIGURATION_UPDATE).  This
**      message tells them to refresh their configuration.
**
**  Inputs:
**      UINT32 reason - Any combination of:
**                          X1_ASYNC_PCHANGED
**                          X1_ASYNC_RCHANGED
**                          X1_ASYNC_VCHANGED
**                          X1_ASYNC_HCHANGED
**                          X1_ASYNC_ACHANGED
**                          X1_ASYNC_ZCHANGED
**                          X1_ASYNC_VCG_ELECTION_STATE_CHANGE
**                          X1_ASYNC_VCG_ELECTION_STATE_ENDED
**                          X1_ASYNC_VCG_POWERUP
**                          X1_ASYNC_VCG_CFG_CHANGED
**--------------------------------------------------------------------------*/
void RMSlavesRefreshNVRAM(UINT32 reason)
{
    /* If this controller is not the master it does not need to do anything */
    if (!TestforMaster(GetMyControllerSN()))
    {
        dprintf(DPRINTF_RMCMDHDL, "RMSlavesRefreshNVRAM: Not master\n");

    }
    else
    {
        RMSlavesConfigurationPropagation(MRNOFSYS | MRNOREFRESH, reason);
    }
}

/*----------------------------------------------------------------------------
**  Function Name: RMSlavesConfigurationPropagation
**
**  Description:
**      Loops through the active controllers in the virtual controller group
**      (VCG) and sends each an IPC message (IPC_CONFIGURATION_UPDATE).  This
**      message tells them to refresh their configuration.
**
**  Inputs:
**      UINT8 restoreOption - What type of configuration propagation
**      UINT32 reason - Any combination of:
**                          X1_ASYNC_PCHANGED
**                          X1_ASYNC_RCHANGED
**                          X1_ASYNC_VCHANGED
**                          X1_ASYNC_HCHANGED
**                          X1_ASYNC_ACHANGED
**                          X1_ASYNC_ZCHANGED
**                          X1_ASYNC_VCG_ELECTION_STATE_CHANGE
**                          X1_ASYNC_VCG_ELECTION_STATE_ENDED
**                          X1_ASYNC_VCG_POWERUP
**                          X1_ASYNC_VCG_CFG_CHANGED
**--------------------------------------------------------------------------*/
static void RMSlavesConfigurationPropagation(UINT8 restoreOption, UINT32 reason)
{
    PARALLEL_REQUEST *pRequests;
    PR_CONFIG_UPDATE_PARAM param;

    /* If this controller is not the master it does not need to do anything */
    if (!TestforMaster(GetMyControllerSN()))
    {
        dprintf(DPRINTF_RMCMDHDL, "RMSlavesConfigurationPropagation: Not master\n");
        return;
    }

//    LogMessage(LOG_TYPE_DEBUG, "CONFIG_PROP: Process (0x%x, 0x%x)", restoreOption,
//               reason);

    /*
     * Initialize the parallel configuration update template.
     */
    memset(&param, 0x00, sizeof(param));
    param.restoreOption = restoreOption;
    param.reason = reason;

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(0, &param, sizeof(param));

    /*
     * Send the PORTS parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE | PR_DEST_OTHERS, PR_TYPE_CONFIG, pRequests,
                    PR_SendTaskConfigUpdate);

    /*
     * Free the parallel requests
     */
    PR_Release(&pRequests);

//    LogMessage(LOG_TYPE_DEBUG, "CONFIG_PROP: Send tasks complete");
}


/*----------------------------------------------------------------------------
**  Function Name: RMSendIpcConfigurationUpdate
**
**  Description:
**      Sends the IPC_CONFIGURATION_UPDATE packet to the controller specified
**      by serialNum.
**
**  Inputs:
**      UINT32 serialNum - serial number of the controller to send the
**                          configuration update packet to.
**      UINT32 reason - Any combination of:
**                          X1_ASYNC_PCHANGED
**                          X1_ASYNC_RCHANGED
**                          X1_ASYNC_VCHANGED
**                          X1_ASYNC_HCHANGED
**                          X1_ASYNC_ACHANGED
**                          X1_ASYNC_ZCHANGED
**                          X1_ASYNC_VCG_ELECTION_STATE_CHANGE
**                          X1_ASYNC_VCG_ELECTION_STATE_ENDED
**                          X1_ASYNC_VCG_POWERUP
**                          X1_ASYNC_VCG_CFG_CHANGED
**--------------------------------------------------------------------------*/
UINT32 RMSendIpcConfigurationUpdate(UINT32 serialNum, UINT8 restoreOption, UINT32 reason)
{
    UINT32                    rc = ERROR;
    IPC_PACKET               *rx;
    PATH_TYPE                 pathType;
    IPC_PACKET               *ptrPacket;
    IPC_CONFIGURATION_UPDATE *ptrData;
    IPC_PACKET_DATA          *pRspData;
    UINT8                     retries = 2;          /* Ethernet, Fiber(1), Disk Quorum(2) */

// dprintf(DPRINTF_RMCMDHDL, "RMSendIpcConfigurationUpdate: ENTER (0x%x, 0x%x, 0x%x)\n",
//         serialNum, restoreOption, reason);

    ptrPacket = CreatePacket(PACKET_IPC_CONFIGURATION_UPDATE,
                             sizeof(IPC_CONFIGURATION_UPDATE), __FILE__, __LINE__);

    ptrData = (IPC_CONFIGURATION_UPDATE *)ptrPacket->data;
    ptrData->restoreOption = restoreOption;
    ptrData->reason = reason;

    rx = MallocSharedWC(sizeof(*rx));

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n)\n", __FILE__, __LINE__, __func__, rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx->data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     ptrPacket, rx, NULL, NULL, NULL, RMCMDHDL_IPC_SEND_TMO);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        LogMessage(LOG_TYPE_DEBUG, "Failed to send packet (sn: 0x%x, pathType: 0x%x).",
                   serialNum, pathType);

        /* The following is believed to fix an issue at Federated CQT21363 */
        /* The controller needs to be failed as it failed to update its configuration. */
        ConfigurationUpdateFailure(serialNum);
        rc = ERROR;
    }
    else if (rx->data->localImage.status != IPC_COMMAND_SUCCESSFUL)
    {
        LogMessage(LOG_TYPE_DEBUG, "Failed to send configuration update to controller (0x%x, status: 0x%x).",
                   serialNum, rx->data->localImage.status);

        /*
         * NOTE: The failing of the controller needs to be reviewed for N-way.
         * We should keep track of the controllers that failed the update and
         * fail them after all of the controllers have been processed (through
         * the refresh).
         *
         * Chris Nigbur/Jeff Williams
         */

        /*
         * The controller needs to be failed since it failed
         * to update its configuration.
         */
        ConfigurationUpdateFailure(serialNum);
        rc = ERROR;
    }
    else if ((restoreOption & MRNOOVERLAY) > 0)
    {
        /* If restore and configuration update is good, update local image. */
//        LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE SCU-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
//                   ((UINT32 *)rx->data->localImage.image)[3], ((UINT32 *)rx->data->localImage.image)[0],
//                   ((UINT32 *)rx->data->localImage.image)[1], ((UINT32 *)rx->data->localImage.image)[2]);

        rc = UpdateLocalImage(rx->data->localImage.image);

        if (rc != GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "Failed to update local image from controller (0x%x).",
                       serialNum);

            (void)EL_DoElectionNonBlocking();
        }
    }
    else
    {
        rc = GOOD;
    }

    FreePacket(&ptrPacket, __FILE__, __LINE__);

    /*
     * Save the return data pointer and free it separately since a part of it
     * is used as a PCI address to the PROC. This needs freeing via DelayedFree.
     */
    pRspData = rx->data;
    rx->data = NULL;
    FreePacketStaticPacketPointer(rx, __FILE__, __LINE__);
    Free(rx);
    DelayedFree(MRPUTLCLIMAGE, pRspData);

// dprintf(DPRINTF_RMCMDHDL, "RMSendIpcConfigurationUpdate: EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UpdateLocalImage
**
**  Description:
**      Sends the MRPUTLCLIMAGE MRP to the BEP to have a local image updated.
**
**  Inputs:
**      UINT8* pLocalImage - Pointer to the local image data to be processed
**                           by the MRP.
**
**  Returns:
**      UINT16 rc - Status of the MRP
**
**  Warning:
**      The local image buffer passed into this function must be deallocated
**      by the caller.  The caller must take caution, if the result is a
**      timeout the caller must use the DelayedFree method of freeing the
**      memory.
**--------------------------------------------------------------------------*/
UINT16 UpdateLocalImage(UINT8 *pLocalImage)
{
    MRPUTLCLIMAGE_REQ *ptrInPkt;
    MRPUTLCLIMAGE_RSP *ptrOutPkt;
    UINT32      rc = PI_GOOD;

    /*
     * dprintf(DPRINTF_RMCMDHDL, "UpdateLocalImage: ENTER\n");
     */

    ccb_assert(pLocalImage != NULL, pLocalImage);

    /*
     * Update the local image map sequence value for this local image's
     * controller.
     */
    LocalImageUpdateSequence(pLocalImage);

    /*
     * Allocate the memory for the input and output requests.
     */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    ptrInPkt->address = (void *)pLocalImage;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRPUTLCLIMAGE,
                    ptrOutPkt, sizeof(*ptrOutPkt),
                    MAX(GetGlobalMRPTimeout(), TMO_UPDATE_LCL_IMAGE));

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "Failed to update local image (pLocalImage: 0x%x, rc: 0x%x, status: 0x%x).",
                   (UINT32)pLocalImage, rc, ptrOutPkt->header.status);
    }

    /*
     * Free the memory that was allocated.  The output packet can
     * only be freed if there was not a timeout.
     *
     * Use delayed free on the input packet since it contains
     * a PCI address.
     */
    DelayedFree(MRPUTLCLIMAGE, ptrInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    /*
     * dprintf(DPRINTF_RMCMDHDL, "UpdateLocalImage: EXIT\n");
     */

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ConfigurationUpdateFailure
**
** Description: If an error occurs during a configuration update this
**              function is called to fail the controller that was
**              unable to complete the configuration update.
**
** Inputs:      UINT32 serialNum - Serial number of the controller that
**                                 failed the configuration update.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void ConfigurationUpdateFailure(UINT32 serialNum)
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;

    LogMessage(LOG_TYPE_DEBUG, "Controller (0x%x) failed part of the configuration update sequence and must be FAILED!",
               serialNum);

    /* Send a controller Failure */
    pFailurePacket = MallocWC(SIZEOF_IPC_CONTROLLER_FAILURE);
    pFailurePacket->Type = IPC_FAILURE_TYPE_CONTROLLER_FAILED;
    pFailurePacket->FailureData.ControllerFailure.DetectedBySN = GetMyControllerSN();
    pFailurePacket->FailureData.ControllerFailure.FailedControllerSN = serialNum;
    pFailurePacket->FailureData.ControllerFailure.ErrorType = CONTROLLER_FAILURE_CONFIGURATION_FAILED;

    FailureManager(pFailurePacket, SIZEOF_IPC_CONTROLLER_FAILURE);
    /* Don't release the failure packet, FailureManager owns it now */
}

/*----------------------------------------------------------------------------
**  Function Name: DumpLocalImage
**
**  Description:
**      Prints the local image to the console using DebugPrintf.
**--------------------------------------------------------------------------*/
void DumpLocalImage(const char *location, UINT32 imageSize, void *pLocalImage)
{
    UINT32      i = 0;
    char        buf[35];
    UINT32      bIndex = 0;
    UINT32     *pImage = pLocalImage;

    if (TestModeBit(MD_DUMP_LOCAL_IMAGE_ENABLE))
    {
        memset(&buf, 0x00, 35);

        dprintf(DPRINTF_RMCMDHDL, "\n%s\n", location);

        while (i < imageSize)
        {
            UINT32      iVal = *pImage;

            sprintf((char *)&buf[bIndex], "%08x", iVal);

            bIndex += 8;
            pImage++;
            i += 4;

            if (i > 0 && (i % 16) == 0)
            {
                dprintf(DPRINTF_RMCMDHDL, "%s\n", buf);
                memset(&buf, 0x00, 35);
                bIndex = 0;
            }
            else
            {
                buf[bIndex] = ' ';
                bIndex++;
            }
        }

        if (bIndex != 0)
        {
            dprintf(DPRINTF_RMCMDHDL, "%s\n", buf);
        }
        dprintf(DPRINTF_RMCMDHDL, "\n\n");
    }
}


/**
******************************************************************************
**
**  @brief      Initialize the local image sequence information.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void LocalImageInitSequence(void)
{
    memset(&gLocalImageSequence, 0x00, sizeof(UINT32) * MAX_CONTROLLERS);
}


/**
******************************************************************************
**
**  @brief      Function to determine if the input local image should still
**              be processed.  This checks if the sequence number for the
**              input local image is greater than the last image processed.
**
**  @param      void* pLocalImage - pointer to the local image that might
**                                  be processed.
**
**  @return     bool - true if the image should be processed, false otherwise.
**
******************************************************************************
**/
bool LocalImageStillProcess(void *pLocalImage)
{
    bool        bProcess = true;
    INT32       controllerNode = Qm_SlotFromSerial(((UINT32 *)pLocalImage)[1]);
    FAILURE_DATA_STATE fd;

    /*
     * If the controller node is valid, check the sequence map to see
     * if the local image should be processed.
     */
    if (controllerNode >= 0 && controllerNode < MAX_CONTROLLERS)
    {
        fd = EL_GetFailureState(controllerNode);

        if (fd != FD_STATE_OPERATIONAL &&
            fd != FD_STATE_POR &&
            fd != FD_STATE_ADD_CONTROLLER_TO_VCG &&
            fd != FD_STATE_FIRMWARE_UPDATE_ACTIVE &&
            fd != FD_STATE_UNFAIL_CONTROLLER && fd != FD_STATE_ACTIVATE)
        {
            LogMessage(LOG_TYPE_DEBUG, "LocalImageStillProcess-Skipping image (state: 0x%x)", fd);

            bProcess = false;
        }

        /*
         * If the sequence number for the local image's controller in the map
         * is not zero and is greater than the sequence number from the local
         * image itself, the local image should not be processed since we have
         * already processed an image was generated later.
         */
        if (bProcess && gLocalImageSequence[controllerNode] != 0 &&
            gLocalImageSequence[controllerNode] > ((UINT32 *)pLocalImage)[3])
        {
            bProcess = false;
        }
    }

    return bProcess;
}


/**
******************************************************************************
**
**  @brief      Function to update the sequence map for a local image's
**              controller with the sequence number in the local image.
**
**  @param      void* pLocalImage - pointer to the local image has been
**                                  processed.
**
**  @return     none
**
******************************************************************************
**/
static void LocalImageUpdateSequence(void *pLocalImage)
{
    INT32       controllerNode;

    controllerNode = Qm_SlotFromSerial(((UINT32 *)pLocalImage)[1]);

    /*
     * If the controller node is valid, update the sequence map with
     * the sequence number of this local image.
     */
    if (controllerNode >= 0 && controllerNode < MAX_CONTROLLERS)
    {
        gLocalImageSequence[controllerNode] = ((UINT32 *)pLocalImage)[3];
    }
}


/**
******************************************************************************
**
**  @brief      Function to update the sequence map for a local image's
**              controller with the sequence number in the local image.
**
**  @param      UINT32 commandCode - Command to send to the temporary
**                                   disable of cache routine.  This is
**                                   one of the two PI command codes for
**                                   the temp disable, SET or CLR.
**  @param      UINT8 user - User for this temporary disable request
**  @param      UINT8 option - Option for the CLR call for a temp disable.
**
**  @return     INT32 - PI_GOOD if all requests were satisfied successfully,
**                      PI_ERROR if any request failed or timed out.
**
******************************************************************************
**/
INT32 RMTempDisableCache(UINT32 commandCode, UINT8 user, UINT8 option)
{
    INT32       rc = PI_GOOD;
    INT32       count;
    PR_TDISCACHE_PARAM param;
    PARALLEL_REQUEST *pRequests;

    /*
     * Initialize the parallel configuration update template.
     */
    memset(&param, 0x00, sizeof(param));
    param.user = user;
    param.option = option;

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(commandCode, &param, sizeof(param));

    /*
     * Send the Temporary Disable of Cache parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_TDISCACHE, pRequests, PR_SendTaskTempDisableCache);

    /*
     * Loop through the requests and see if any of the requests failed.
     * Basically, if the request structure has a controller serial number
     * it should also have a return code saved for the result of the
     * request.
     */
    for (count = 0; rc == PI_GOOD && count < MAX_CONTROLLERS; ++count)
    {
        /*
         * Does this request have a controller serial number but a failed
         * request return code?
         */
        if (pRequests[count].controllerSN != 0 && pRequests[count].rc != PI_GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "RMTempDisableCache-Failed to disable cache (0x%x)",
                       pRequests[count].controllerSN);

            rc = PI_ERROR;
        }
    }

    /*
     * Release the parallel request buffers.
     */
    PR_Release(&pRequests);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Function to look at all active controllers and wait until
**              they have flushed their write cache data.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void RMWaitForCacheFlush(void)
{
    bool        bFlushed = false;
    PARALLEL_REQUEST *pRequests;
    PI_MISC_QTDISCACHE_RSP *pResponse;
    UINT32      count;

    LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlush-ENTER");

    /*
     * Continue looping until all active controllers have flushed their
     * write cache data.
     */
    while (!bFlushed)
    {
        /*
         * Allocate the parallel request buffers.
         */
        pRequests = PR_Alloc();

        /*
         * Send the global cache parallel requests.
         */
        PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_QTDISCACHE, pRequests,
                        PR_SendTaskQueryTempDisableCache);

        /*
         * Loop through the global cache request data and see if anything
         * that should be available is not.  Basically, if the request
         * structure has a controller serial number it should have global
         * cache port information.  If it doesn't, it is not valid and is
         * an error.
         */
        for (count = 0; count < MAX_CONTROLLERS; ++count)
        {
            /*
             * Does this request have a controller serial number but no
             * global cache data?
             */
            if (pRequests[count].controllerSN != 0 && pRequests[count].pData == NULL)
            {
                LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlush-Missing information (0x%x)",
                           pRequests[count].controllerSN);

                /*
                 * Free the parallel requests, this will NULL out the
                 * pRequests pointer and force us to retry the requests.
                 */
                PR_Release(&pRequests);
                break;
            }
        }

        /*
         * If the requests are valid (we have data from all active controllers)
         * then we need to look to see if cache has been flushed.
         */
        if (pRequests)
        {
            /*
             * Loop through all the requests.
             */
            for (count = 0; count < MAX_CONTROLLERS; ++count)
            {
                /*
                 * Make sure that this is a valid request (it must have
                 * a controller serial number to be valid).
                 */
                if (pRequests[count].controllerSN == 0)
                {
                    continue;
                }

                /*
                 * The data is the stats global cache information so pull
                 * it out into a local we can use.
                 */
                pResponse = (PI_MISC_QTDISCACHE_RSP *)pRequests[count].pData;

                /*
                 * Check if the query temp disable cache indicates that the
                 * flush is still in progress
                 */
                if (pResponse->data.status != MQTD_DONE &&
                    pResponse->data.status != MQTD_VDISK_IN_ERROR)
                {
                    LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlush-Flush NOT Complete (c=0x%x, s=0x%x)",
                               pRequests[count].controllerSN, pResponse->data.status);

                    break;
                }
            }

            /*
             * If we reached the end of the list everything has been
             * flushed.
             */
            if (count == MAX_CONTROLLERS)
            {
                bFlushed = true;
            }

            /*
             * Free the parallel requests, this will NULL out the
             * pRequests pointer and force us to skip the next
             * section which interrogates the request information.
             */
            PR_Release(&pRequests);
        }

        /*
         * If the flushes are not complete, wait a second and then check
         * again.
         */
        if (!bFlushed)
        {
            /*
             * Wait for just a second before looking at the
             * statistics again.
             */
            TaskSleepMS(1000);
        }
    }

    LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlush-EXIT");
}


/**
******************************************************************************
**
**  @brief      Retrieves the global cache information and waits for the
**              free and resident cache tags to equal the number of cache
**              tags.
**
**  @param      UINT32 controllerSN - Which controller to wait for cache
**
**  @return     none
**
******************************************************************************
**/
void RMWaitForCacheFlushController(UINT32 controllerSN)
{
    bool        bFlushed = false;
    PI_MISC_QTDISCACHE_RSP *pResponse;

    LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlushController-ENTER (0x%x)", controllerSN);

    while (!bFlushed)
    {
        pResponse = SM_QueryTempDisableCache(controllerSN);

        /*
         * We need to get the information...
         */
        if (pResponse != NULL)
        {
            /*
             * Check if the query temp disable cache indicates that the
             * flush is still in progress
             */
            if (pResponse->data.status == MQTD_DONE ||
                pResponse->data.status == MQTD_VDISK_IN_ERROR)
            {
                LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlushController-Flush Complete (c=0x%x, s=0x%x)",
                           controllerSN, pResponse->data.status);

                /*
                 * Looks like cache has been flushed so we can continue.
                 */
                bFlushed = true;
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlushController-Flush In Progress (c=0x%x, s=0x%x)",
                           controllerSN, pResponse->data.status);
            }

            Free(pResponse);
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlushController-No Info (0x%x)", controllerSN);
        }

        /*
         * If the flushes are not complete, wait a second and then check
         * again.
         */
        if (!bFlushed)
        {
            /*
             * Wait for just a second before looking at the
             * statistics again.
             */
            TaskSleepMS(1000);
        }
    }

    LogMessage(LOG_TYPE_DEBUG, "RMWaitForCacheFlushController-EXIT (0x%x)", controllerSN);
}


/***
 ** Modelines:
 ** Local Variables:
 ** tab-width: 4
 ** indent-tabs-mode: nil
 ** End:
 ** vi:sw=4 ts=4 expandtab
 **/
