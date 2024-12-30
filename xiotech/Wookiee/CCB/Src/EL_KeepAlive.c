/* $Id: EL_KeepAlive.c 156018 2011-05-27 16:01:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_KeepAlive.c
**
**  @brief      Election disaster detection and recovery code
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "EL_KeepAlive.h"

#include "cps_init.h"
#include "debug_files.h"
#include "EL.h"
#include "ipc_sendpacket.h"
#include "misc.h"
#include "nvram.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static UINT32 temporaryKeepAlive = FALSE;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT32 EL_KeepAliveCheckSchema(void);
static UINT32 EL_KeepAliveTestSlotValid(UINT16 slotNumber);
static UINT32 EL_KeepAliveTestSlotActive(UINT16 slotNumber);
static UINT32 EL_KeepAliveFindAlternate(UINT16 *slotNumberPtr);
static UINT32 EL_KeepAliveCountActive(void);
static UINT32 EL_KeepAliveGetUnfail(void);
static void EL_KeepAliveSystemDump(KEEP_ALIVE *keepAlivePtr);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      EL_KeepAliveSystemReset
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void EL_KeepAliveSystemReset(void)
{
    UINT32      counter = 0;

    dprintf(DPRINTF_ELECTION, "KASR: Resetting keepAlive system\n");

    /*
     * Make the keepAlive system enabled by default
     */
    masterConfig.keepAlive.header.schema = KEEP_ALIVE_SCHEMA;
    masterConfig.keepAlive.header.flags.value = 0;
    masterConfig.keepAlive.header.flags.bits.systemEnabled = TRUE;

    for (counter = 0; counter < dimension_of(masterConfig.keepAlive.keepAliveList); counter++)
    {
        masterConfig.keepAlive.keepAliveList[counter].value = 0;
    }

    for (counter = 0; counter < dimension_of(masterConfig.keepAlive.pad); counter++)
    {
        masterConfig.keepAlive.pad[counter] = 0;
    }
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSystemEnable
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void EL_KeepAliveSystemEnable(void)
{
    dprintf(DPRINTF_ELECTION, "KASE: Enabling keepAlive system\n");

    EL_KeepAliveCheckSchema();
    masterConfig.keepAlive.header.flags.bits.systemEnabled = TRUE;

    EL_KeepAliveSystemDump(&masterConfig.keepAlive);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSystemDisable
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void EL_KeepAliveSystemDisable(void)
{
    dprintf(DPRINTF_ELECTION, "KASD: Disabling keepAlive system\n");

    EL_KeepAliveCheckSchema();
    masterConfig.keepAlive.header.flags.bits.systemEnabled = FALSE;

    EL_KeepAliveSystemDump(&masterConfig.keepAlive);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSystemTestEnabled
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_KeepAliveSystemTestEnabled(void)
{
    UINT32      returnCode = ERROR;

    EL_KeepAliveCheckSchema();

    if (masterConfig.keepAlive.header.flags.bits.systemEnabled == TRUE)
    {
        returnCode = GOOD;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveCheckSchema
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_KeepAliveCheckSchema(void)
{
    UINT32      returnCode = GOOD;

    if (masterConfig.keepAlive.header.schema != KEEP_ALIVE_SCHEMA)
    {
        dprintf(DPRINTF_ELECTION, "KASC: keepAlive schema check failed\n");

        EL_KeepAliveSystemReset();
        returnCode = ERROR;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveTestSlotActive
**
**  @param      slotNumber - Slot number to test for keepAlive status
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_KeepAliveTestSlotActive(UINT16 slotNumber)
{
    UINT32      returnCode = ERROR;
    UINT16      acmNode = ERROR;

    ccb_assert(slotNumber < MAX_CONTROLLERS, slotNumber);

    if (slotNumber < MAX_CONTROLLERS)
    {
        /*
         * Check the the keepAlive slot is valid (is a keepAlive controller)
         * and that the controller is on the ACM (operational).
         */
        if ((EL_KeepAliveTestSlotValid(slotNumber) == GOOD) &&
            (ACM_GetNodeBySlot(Qm_ActiveCntlMapPtr(), &acmNode, slotNumber) == GOOD))
        {
            returnCode = GOOD;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "KATSA: slotNumber is out-of-range\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSetSlotValid
**
**  @param      slotNumber - Slot number to set
**  @param      validFlag  - Flag for valid/invalid (TRUE == valid)
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_KeepAliveSetSlotValid(UINT16 slotNumber, UINT32 validFlag)
{
    UINT32      returnCode = GOOD;

    ccb_assert(slotNumber < dimension_of(masterConfig.keepAlive.keepAliveList), slotNumber);

    if (slotNumber < dimension_of(masterConfig.keepAlive.keepAliveList))
    {
        /*
         * Validate the keepAlive schema
         */
        EL_KeepAliveCheckSchema();

        /*
         * Change the keepAlive slot, if input is from the correct source
         */
        dprintf(DPRINTF_ELECTION, "KASSV: Setting keepAlive[%d].slotValid to %d\n",
                slotNumber, validFlag);

        switch (validFlag)
        {
            case TRUE:
                masterConfig.keepAlive.keepAliveList[slotNumber].bits.slotValid = TRUE;
                break;

            case FALSE:
                masterConfig.keepAlive.keepAliveList[slotNumber].bits.slotValid = FALSE;
                break;

            default:
                dprintf(DPRINTF_ELECTION, "KASSV: Bad validFlag input\n");
                returnCode = ERROR;
                break;
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "KASSV: slotNumber %d is out-of-range\n", slotNumber);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveTestSlotValid
**
**  @param      slotNumber - Slot number to test
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_KeepAliveTestSlotValid(UINT16 slotNumber)
{
    UINT32      returnCode = ERROR;
    UINT16      thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

    ccb_assert(slotNumber < dimension_of(masterConfig.keepAlive.keepAliveList), slotNumber);

    /*
     * If this controller is performing a keepAlive version of the unfail, then
     * make sure that the slot tests as being valid.
     */
    if ((slotNumber == thisControllerCommSlot) && (EL_KeepAliveGetUnfail() == TRUE))
    {
        dprintf(DPRINTF_ELECTION, "KATSV: slot %d is doing keepAlive unfail\n", slotNumber);

        returnCode = GOOD;
    }
    else if (slotNumber < dimension_of(masterConfig.keepAlive.keepAliveList))
    {
        /*
         * Validate the keepAlive schema
         */
        EL_KeepAliveCheckSchema();

        /*
         * See if the specified slot is marked as being valid (keepAlive controller)
         */
        if (masterConfig.keepAlive.keepAliveList[slotNumber].bits.slotValid == TRUE)
        {
            /*
             * If, for some reason, a keepAlive controller gets the disaster
             * safeguard turned on and it's not doing a keepAlive unfail, then
             * make sure it doesn't claim to be a keepAlive controller.
             */
            if ((slotNumber != thisControllerCommSlot) ||
                (EL_DisasterCheckSafeguard() == GOOD))
            {
                returnCode = GOOD;
            }
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "KATSV: slotNumber %d is out-of-range\n", slotNumber);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveFindAlternate
**
**  @param      slotNumberPtr - Pointer to where slotNumber is to be returned
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_KeepAliveFindAlternate(UINT16 *slotNumberPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(slotNumberPtr != NULL, slotNumberPtr);

    if (slotNumberPtr != NULL)
    {
        *slotNumberPtr = Qm_GetActiveCntlMap(0);
        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "KAFA: slotNumberPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveCountActive
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_KeepAliveCountActive(void)
{
    UINT32      activeCount = 0;
    UINT32      slotCounter = 0;

    for (slotCounter = 0; slotCounter < dimension_of(masterConfig.keepAlive.keepAliveList); slotCounter++)
    {
        if (EL_KeepAliveTestSlotActive(slotCounter) == GOOD)
        {
            activeCount++;
        }
    }

    return (activeCount);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveGetUnfail
**
**  @return     TRUE or FALSE
**
******************************************************************************
**/
static UINT32 EL_KeepAliveGetUnfail(void)
{
    return (temporaryKeepAlive);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSetUnfail
**
**  @param      unfailFlag (TRUE or FALSE)
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_KeepAliveSetUnfail(UINT32 unfailFlag)
{
    UINT32      returnCode = GOOD;

    switch (unfailFlag)
    {
        case TRUE:
        case FALSE:
            temporaryKeepAlive = unfailFlag;
            break;

        default:
            returnCode = ERROR;
            break;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveSystemDump
**
**  @return     none
**
******************************************************************************
**/
static void EL_KeepAliveSystemDump(KEEP_ALIVE *keepAlivePtr)
{
    UINT32      counter = 0;
    UINT16      alternateSlot = 0;

    ccb_assert(keepAlivePtr != NULL, keepAlivePtr);
    dprintf(DPRINTF_ELECTION, "KASD: Dumping keepAlive system\n");

    if (keepAlivePtr != NULL)
    {
        /*
         * Find the alternate keepAlive controller
         */
        EL_KeepAliveFindAlternate(&alternateSlot);
        dprintf(DPRINTF_ELECTION, "KASD:   Alternate Slot:  %d\n", alternateSlot);

        /*
         * Dump the keepAlive information
         */
        dprintf(DPRINTF_ELECTION, "KASD:   Schema:  0x%04hx\n",
                keepAlivePtr->header.schema);
        dprintf(DPRINTF_ELECTION, "KASD:   Flags:   0x%04hx\n",
                keepAlivePtr->header.flags.value);

        if (keepAlivePtr->header.flags.bits.systemEnabled == TRUE)
        {
            for (counter = 0; counter < dimension_of(keepAlivePtr->keepAliveList); counter++)
            {
                EL_KeepAliveTestSlot(counter);
            }
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "KASD:   KeepAlive system is disabled\n");
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "KASD: keepAlivePtr is NULL\n");
    }
}


/**
******************************************************************************
**
**  @brief      EL_KeepAliveTestSlot
**
**  @param      slotNumber - Slot number to test
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_KeepAliveTestSlot(UINT16 slotNumber)
{
    UINT32      returnCode = ERROR;
    UINT16      alternateCommSlot = 0;
    UINT16      thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

    /*
     * Check if a disaster unfail in occurring.  If it is, and the slot to be
     * tested is this controller's slot, then return GOOD.  This needs to be
     * done independent of the KA system being enabled or disabled.
     */
    if ((slotNumber == thisControllerCommSlot) && (EL_KeepAliveGetUnfail() == TRUE))
    {
        dprintf(DPRINTF_ELECTION, "KATS: Slot %d is a keepAlive (Unfail flag is set)\n",
                slotNumber);

        returnCode = GOOD;
    }
    else if (EL_KeepAliveSystemTestEnabled() == GOOD)
    {
        /*
         * Check if this safeguard is in effect.  If it is, then we need to
         * ignore the keepAlive flags.  The only time we ignore the safeguard is
         * if the user has specified this controller to perform a keepAlive
         * type of unfail.
         */
        if ((slotNumber == thisControllerCommSlot) &&
            (EL_DisasterCheckSafeguard() == ERROR))
        {
            dprintf(DPRINTF_ELECTION, "KATS: Disaster safeguard in place - not a keepAlive\n");
        }
        else
        {
            if (EL_KeepAliveTestSlotValid(slotNumber) == GOOD)
            {
                dprintf(DPRINTF_ELECTION, "KATS: Slot %d is a keepAlive (Valid)\n",
                        slotNumber);

                returnCode = GOOD;
            }
            else if ((EL_KeepAliveCountActive() == 0) &&
                     (EL_KeepAliveFindAlternate(&alternateCommSlot) == GOOD) &&
                     (slotNumber == alternateCommSlot))
            {
                dprintf(DPRINTF_ELECTION, "KATS: Slot %d is a keepAlive (Alternate)\n",
                        slotNumber);

                returnCode = GOOD;
            }
        }
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
