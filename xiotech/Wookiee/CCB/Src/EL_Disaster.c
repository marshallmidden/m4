/* $Id: EL_Disaster.c 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_Disaster.c
**
**  @brief      Election disaster detection and recovery code
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "EL_Disaster.h"

#include "AsyncEventHandler.h"
#include "CacheMisc.h"
#include "CachePDisk.h"
#include "CacheSize.h"
#include "cps_init.h"
#include "debug_files.h"
#include "EL.h"
#include "EL_BayMap.h"
#include "EL_DiskMap.h"
#include "logdef.h"
#include "logview.h"
#include "misc.h"
#include "nvram.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "X1_Structs.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private functions
*****************************************************************************/
static UINT32 EL_DisasterCheckDiskBayWritable(ELECTION_BAY_MAP *bayMapPtr,
                                              char *reasonStringPtr, UINT32 stringLength);
static UINT32 EL_DisasterLog(void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      EL_DisasterGetString
**
**  @param      returnStringPtr - Pointer to new disaster string
**  @param      length - Size of input buffer
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DisasterGetString(char *returnStringPtr, UINT32 length)
{
    UINT32      returnCode = ERROR;
    DISASTER_DATA disasterData;

    ccb_assert(returnStringPtr != NULL, returnStringPtr);

    if (returnStringPtr != NULL)
    {
        returnCode = NVRAM_DisasterDataLoad(&disasterData);
        strncpy(returnStringPtr, disasterData.reasonString, length);

        /*
         * Make sure the strncpy is terminated
         */
        if (length > 0)
        {
            returnStringPtr[length - 1] = '\0';
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterSetString
**
**  @param      newStringPtr - Pointer to new disaster string
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DisasterSetString(const char *newStringPtr)
{
    UINT32      returnCode = ERROR;
    DISASTER_DATA disasterData;

    ccb_assert(newStringPtr != NULL, newStringPtr);

    if (newStringPtr != NULL)
    {
        NVRAM_DisasterDataLoad(&disasterData);
        strncpy(disasterData.reasonString, newStringPtr,
                sizeof(disasterData.reasonString));

        /*
         * Make sure the strncpy is terminated
         */
        if (sizeof(disasterData.reasonString) > 0)
        {
            disasterData.reasonString[sizeof(disasterData.reasonString) - 1] = '\0';
        }

        if (NVRAM_DisasterDataSave(&disasterData) == GOOD)
        {
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DSS: NVRAM_DisasterDataSave returned ERROR\n");
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterGetFlags
**
**  @param      returnFlagsPtr - Pointer to new disaster string
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DisasterGetFlags(DISASTER_DATA_FLAGS *returnFlagsPtr)
{
    UINT32      returnCode = ERROR;
    DISASTER_DATA disasterData;

    ccb_assert(returnFlagsPtr != NULL, returnFlagsPtr);

    if (returnFlagsPtr != NULL)
    {
        returnCode = NVRAM_DisasterDataLoad(&disasterData);
        memcpy(returnFlagsPtr, &disasterData.flags, sizeof(disasterData.flags));
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterSetFlags
**
**  @param      newFlagsPtr - Pointer to new disaster flags
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DisasterSetFlags(DISASTER_DATA_FLAGS *newFlagsPtr)
{
    UINT32      returnCode = ERROR;
    DISASTER_DATA disasterData;

    ccb_assert(newFlagsPtr != NULL, newFlagsPtr);

    if (newFlagsPtr != NULL)
    {
        NVRAM_DisasterDataLoad(&disasterData);
        memcpy(&disasterData.flags, newFlagsPtr, sizeof(disasterData.flags));

        if (NVRAM_DisasterDataSave(&disasterData) == GOOD)
        {
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DSF: NVRAM_DisasterDataSave returned ERROR\n");
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterCheck - Check for any possible disaster conditions
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DisasterCheck(void)
{
    UINT32      returnCode = GOOD;
    char        errorString[MMC_MESSAGE_SIZE] = "None";
    ELECTION_BAY_MAP bayMap;    /* Uninitialized */

    dprintf(DPRINTF_ELECTION, "DC: Performing disaster check\n");

    /*
     * McMaster - Fill in option 1
     *
     * Actions for the non-keepAlive controllers:
     *   1) Verify connectivity to at least one keepAlive controller and
     *      as an intersecting pDisk with the keepAlive
     *   - OR -
     *   2) Verify that at least one drive in each diskBay is writable
     *   3) Verify that a majority of the drives in each GeoPool are writable
     */
    if (EL_BayMapUpdate(&bayMap) == GOOD)
    {
        /*
         * For debug, print out the bayMap
         */
        EL_BayMapDump(&bayMap);

        /*
         * Now that we know how many disks are in which bays, process the data
         */
        if (EL_DisasterCheckDiskBayWritable(&bayMap, errorString, sizeof(errorString)) != GOOD)
        {
            dprintf(DPRINTF_ELECTION, "DC: EL_DisasterCheckDiskBayWritable returned ERROR\n");
            returnCode = EL_DisasterTakeAction(errorString);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DC: EL_BayMapUpdate returned ERROR\n");
        returnCode = EL_DisasterTakeAction(errorString);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterCheckSafeguard
**
**  @param      none
**
**  @return     GOOD  - No disaster detected
**  @return     ERROR - Disaster condition detected (safeguard active)
**
******************************************************************************
**/
UINT32 EL_DisasterCheckSafeguard(void)
{
    UINT32      returnCode = GOOD;
    DISASTER_DATA disasterData;

    memset(&disasterData, 0, sizeof(disasterData));

    if (EL_DisasterGetFlags(&disasterData.flags) == GOOD)
    {
        if (disasterData.flags.bits.disasterDetected == TRUE)
        {
            EL_DisasterGetString(disasterData.reasonString,
                                 sizeof(disasterData.reasonString));

            dprintf(DPRINTF_ELECTION, "DCS: Disaster safeguard is in place - reason:[%s]\n",
                    disasterData.reasonString);

            returnCode = ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DCS: Disaster flags invalid, no safeguard\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterSafeguardBypass
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DisasterBypassSafeguard(void)
{
    UINT32      returnCode = GOOD;
    DISASTER_DATA_FLAGS disasterDataFlags;

    dprintf(DPRINTF_ELECTION, "DBS: Bypass the disaster safeguard\n");

    EL_DisasterGetFlags(&disasterDataFlags);

    if (disasterDataFlags.bits.disasterDetected == TRUE)
    {
        EL_DisasterSetString("None");

        disasterDataFlags.bits.disasterDetected = FALSE;
        returnCode = EL_DisasterSetFlags(&disasterDataFlags);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterCheckDiskBayWritable - Disk bay writability check
**
**              Checks each disk bay for writability by making sure that at
**              least one drive in each bay was able to be written by
**              the file system.
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DisasterCheckDiskBayWritable(ELECTION_BAY_MAP *bayMapPtr,
                                              char *reasonStringPtr, UINT32 stringLength)
{
    UINT32      returnCode = GOOD;
    UINT32      bayCounter = 0;
    char        errorString[MMC_MESSAGE_SIZE] = "Writable disk bay check failed";
    char        bayString[MMC_MESSAGE_SIZE] = "None";

    ccb_assert(bayMapPtr != NULL, bayMapPtr);
    ccb_assert(reasonStringPtr != NULL, reasonStringPtr);
    ccb_assert(stringLength != 0, stringLength);

    dprintf(DPRINTF_ELECTION, "DCDB: Check for DiskBay writability\n");

    if (bayMapPtr != NULL)
    {
        /*
         * Check that the controller is able to write at least one disk in
         * each bay.
         */
        for (bayCounter = 0;
             (bayCounter < dimension_of(bayMapPtr->map)) && (returnCode == GOOD);
             bayCounter++)
        {
            if (bayMapPtr->flags.bits.mapValid == TRUE)
            {
                if ((bayMapPtr->map[bayCounter].flags.bits.bayPresent == TRUE) ||
                    (bayMapPtr->map[bayCounter].flags.bits.disksPresent == TRUE))
                {
                    if ((bayMapPtr->map[bayCounter].goodCount == 0) &&
                        (bayMapPtr->map[bayCounter].unknownCount == 0) &&
                        (bayMapPtr->map[bayCounter].unlabeledCount < bayMapPtr->map[bayCounter].totalCount))
                    {
                        dprintf(DPRINTF_ELECTION, "DCDB: bayMap[%02d] - unwritable\n", bayCounter);

                        /*
                         * Build the descriptive error string
                         */
                        DiskbayIdToString(bayCounter, bayString);
                        sprintf(errorString, "Bay %s inaccessible", bayString);
                        returnCode = ERROR;
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DCDB: bayMap is invalid - ERROR\n");
                returnCode = ERROR;
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DCDB: bayMapPtr is NULL\n");
        returnCode = ERROR;
    }

    /*
     * Copy the reason for the ERROR into the returnString
     */
    if ((returnCode != GOOD) && (reasonStringPtr != NULL) && (stringLength > 0))
    {
        strncpy(reasonStringPtr, errorString, stringLength);
        reasonStringPtr[stringLength - 1] = '\0';
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterTakeAction - Take action on disaster condition
**
**  @param      reasonStringPtr
**
**  @return     GOOD  - No disaster action taken (keepAlive - or connected to one)
**  @return     ERROR - This controller will go offline (suicide)
**
******************************************************************************
**/
UINT32 EL_DisasterTakeAction(const char *reasonStringPtr)
{
    UINT32      returnCode = ERROR;
    UINT16      thisControllerCommSlot = ACM_NODE_UNDEFINED;
    DISASTER_DATA_FLAGS disasterFlags = { 0 };

    ccb_assert(reasonStringPtr != NULL, reasonStringPtr);

    dprintf(DPRINTF_ELECTION, "DTA: Taking disaster action\n");

    /*
     * Check to see if the keepAlive system is enabled.  If it is not enabled,
     * then run like we do in previous releases (oblivious to disasters).
     */
    if (EL_KeepAliveSystemTestEnabled() == GOOD)
    {
        /*
         * Check if this controller is designated as a keepAlive
         */
        thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

        if ((EL_KeepAliveTestSlot(thisControllerCommSlot) == GOOD) ||
            (EL_CheckKeepAliveConnectivity() == GOOD))
        {
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DTA: Not connected to a keepAlive - allow disaster\n");
        }
    }
    else
    {
        /*
         * If the keepAlive system is disabled, then run like we do in
         * the R1/R2 releases.
         */
        dprintf(DPRINTF_ELECTION, "DTA: keepAlive system is disabled - prevent disaster mode\n");
        returnCode = GOOD;
    }

    /*
     * Perform the action, if one needs to be taken
     */
    if (returnCode == GOOD)
    {
        dprintf(DPRINTF_ELECTION, "DTA: No disaster action taken\n");
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DTA: Disaster - failing controller\n");

        /*
         * Write the signature into NVRAM before failing the controller
         */
        disasterFlags.bits.disasterDetected = TRUE;
        EL_DisasterSetFlags(&disasterFlags);

        /*
         * Don't let a bad input prevent the disaster action
         */
        if (reasonStringPtr != NULL)
        {
            EL_DisasterSetString(reasonStringPtr);
        }
        else
        {
            EL_DisasterSetString("EL_DisasterTakeAction");
        }

        /*
         * Log the reason disaster
         */
        EL_DisasterLog();

        /*
         * Cause the controller to suicide
         */
        EL_NotifyOtherTasks(ELECTION_DISASTER);
        EL_ChangeState(ED_STATE_FAILED);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DisasterLog - Make log entry for disaster condition
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DisasterLog(void)
{
    UINT32      returnCode = ERROR;
    DISASTER_DATA_FLAGS disasterFlags = { 0 };
    LOG_DISASTER_PKT logMsg = { {0} };

    dprintf(DPRINTF_ELECTION, "DL: Disaster log entry\n");

    /*
     * Make the log message, but only if a disaster has been detected.
     */
    if ((EL_DisasterGetFlags(&disasterFlags) == GOOD) &&
        (disasterFlags.bits.disasterDetected == TRUE) &&
        (EL_DisasterGetString((char *)logMsg.reasonString, sizeof(logMsg.reasonString)) == GOOD))
    {
        dprintf(DPRINTF_ELECTION, "DL: Disaster reason: %s\n", logMsg.reasonString);
        SendAsyncEvent(LOG_DISASTER, sizeof(logMsg), &logMsg);
    }

    return (returnCode);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
