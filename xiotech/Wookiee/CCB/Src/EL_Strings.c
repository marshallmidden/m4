/* $Id: EL_Strings.c 144191 2010-07-15 20:23:53Z steve_wirtz $ */
/*============================================================================
** FILE NAME:       EL_Strings.c
** MODULE TITLE:    Bigfoot election module
**
** DESCRIPTION:     The functions in this module are used for coordinating
**                  and controlling elections.
**
** Copyright (c) 2001-2009 XIOtech Corporation.  All rights reserved.
**==========================================================================*/
#include "EL_Strings.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else
#include "XIO_Std.h"
#endif

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: EL_GetElectionStateString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetElectionStateString(char *stringPtr, ELECTION_DATA_STATE stateNumber,
                               UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (stateNumber)
        {
            case ED_STATE_END_TASK:
                strncpy(stringPtr, "END_TASK", stringLength);
                break;

            case ED_STATE_BEGIN_ELECTION:
                strncpy(stringPtr, "BEGIN_ELECTION", stringLength);
                break;

            case ED_STATE_CHECK_MASTERSHIP_ABILITY:
                strncpy(stringPtr, "CHECK_MASTERSHIP_ABILITY", stringLength);
                break;

            case ED_STATE_TIMEOUT_CONTROLLERS:
                strncpy(stringPtr, "TIMEOUT_CONTROLLERS", stringLength);
                break;

            case ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE:
                strncpy(stringPtr, "TIMEOUT_CONTROLLERS_COMPLETE", stringLength);
                break;

            case ED_STATE_CONTACT_ALL_CONTROLLERS:
                strncpy(stringPtr, "CONTACT_ALL_CONTROLLERS", stringLength);
                break;

            case ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE:
                strncpy(stringPtr, "CONTACT_ALL_CONTROLLERS_COMPLETE", stringLength);
                break;

            case ED_STATE_WAIT_FOR_MASTER:
                strncpy(stringPtr, "WAIT_FOR_MASTER", stringLength);
                break;

            case ED_STATE_CHECK_MASTER:
                strncpy(stringPtr, "CHECK_MASTER", stringLength);
                break;

            case ED_STATE_NOTIFY_SLAVES:
                strncpy(stringPtr, "NOTIFY_SLAVES", stringLength);
                break;

            case ED_STATE_FAILED:
                strncpy(stringPtr, "FAILED", stringLength);
                break;

            case ED_STATE_FINISHED:
                strncpy(stringPtr, "FINISHED", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetContactMapStateString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetContactMapStateString(char *stringPtr,
                                 ELECTION_DATA_CONTACT_MAP_ITEM stateNumber,
                                 UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (stateNumber)
        {
            case ED_CONTACT_MAP_NO_ACTIVITY:
                strncpy(stringPtr, "NO_ACTIVITY", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACTING:
                strncpy(stringPtr, "CONTACTING", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACT_FAILED:
                strncpy(stringPtr, "CONTACT_FAILED", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACT_TIMEOUT:
                strncpy(stringPtr, "CONTACT_TIMEOUT", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACTED_QUORUM:
                strncpy(stringPtr, "CONTACTED_QUORUM", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACTED_FIBRE:
                strncpy(stringPtr, "CONTACTED_FIBRE", stringLength);
                break;

            case ED_CONTACT_MAP_CONTACTED_ETHERNET:
                strncpy(stringPtr, "CONTACTED_ETHERNET", stringLength);
                break;

            case ED_CONTACT_MAP_NOTIFY_SLAVE:
                strncpy(stringPtr, "NOTIFY_SLAVE", stringLength);
                break;

            case ED_CONTACT_MAP_SLAVE_NOTIFIED:
                strncpy(stringPtr, "SLAVE_NOTIFIED", stringLength);
                break;

            case ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED:
                strncpy(stringPtr, "NOTIFY_SLAVE_FAILED", stringLength);
                break;

            case ED_CONTACT_MAP_NOTIFY_SLAVE_TIMEOUT:
                strncpy(stringPtr, "NOTIFY_SLAVE_TIMEOUT", stringLength);
                break;

            case ED_CONTACT_MAP_MASTER_CONTROLLER:
                strncpy(stringPtr, "MASTER_CONTROLLER", stringLength);
                break;

            case ED_CONTACT_MAP_TIMEOUT_CONTROLLER:
                strncpy(stringPtr, "TIMEOUT_CONTROLLER", stringLength);
                break;

            case ED_CONTACT_MAP_CONTROLLER_TIMED_OUT:
                strncpy(stringPtr, "CONTROLLER_TIMED_OUT", stringLength);
                break;

            case ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED:
                strncpy(stringPtr, "TIMEOUT_CONTROLLER_FAILED", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetSendPacketResultString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetSendPacketResultString(char *stringPtr, PATH_TYPE stateNumber,
                                  UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (stateNumber)
        {
            case SENDPACKET_TIME_OUT:
                strncpy(stringPtr, "TIME_OUT", stringLength);
                break;

            case SENDPACKET_NO_PATH:
                strncpy(stringPtr, "NO_PATH", stringLength);
                break;

            case SENDPACKET_ANY_PATH:
                strncpy(stringPtr, "ANY_PATH", stringLength);
                break;

            case SENDPACKET_ETHERNET:
                strncpy(stringPtr, "ETHERNET", stringLength);
                break;

            case SENDPACKET_FIBRE:
                strncpy(stringPtr, "FIBRE", stringLength);
                break;

            case SENDPACKET_QUORUM:
                strncpy(stringPtr, "QUORUM", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetFailureDataStateString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 stateNumber - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetFailureDataStateString(char *stringPtr, FAILURE_DATA_STATE stateNumber,
                                  UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (stateNumber)
        {
            case FD_STATE_UNUSED:
                strncpy(stringPtr, "UNUSED", stringLength);
                break;

            case FD_STATE_FAILED:
                strncpy(stringPtr, "FAILED", stringLength);
                break;

            case FD_STATE_OPERATIONAL:
                strncpy(stringPtr, "OPERATIONAL", stringLength);
                break;

            case FD_STATE_POR:
                strncpy(stringPtr, "POR", stringLength);
                break;

            case FD_STATE_ADD_CONTROLLER_TO_VCG:
                strncpy(stringPtr, "ADD_CONTROLLER_TO_VCG", stringLength);
                break;

            case FD_STATE_STRANDED_CACHE_DATA:
                strncpy(stringPtr, "STRANDED_CACHE_DATA", stringLength);
                break;

            case FD_STATE_FIRMWARE_UPDATE_INACTIVE:
                strncpy(stringPtr, "FIRMWARE_UPDATE_INACTIVE", stringLength);
                break;

            case FD_STATE_FIRMWARE_UPDATE_ACTIVE:
                strncpy(stringPtr, "FIRMWARE_UPDATE_ACTIVE", stringLength);
                break;

            case FD_STATE_UNFAIL_CONTROLLER:
                strncpy(stringPtr, "UNFAIL_CONTROLLER", stringLength);
                break;

            case FD_STATE_VCG_SHUTDOWN:
                strncpy(stringPtr, "VCG_SHUTDOWN", stringLength);
                break;

            case FD_STATE_INACTIVATED:
                strncpy(stringPtr, "INACTIVATED", stringLength);
                break;

            case FD_STATE_ACTIVATE:
                strncpy(stringPtr, "ACTIVATE", stringLength);
                break;

            case FD_STATE_DISASTER_INACTIVE:
                strncpy(stringPtr, "DISASTER_INACTIVE", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name: EL_GetMastershipAbilityString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 abilityNumber - capability number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetMastershipAbilityString(char *stringPtr,
                                   ELECTION_DATA_MASTERSHIP_ABILITY abilityNumber,
                                   UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (abilityNumber)
        {
            case ED_MASTERSHIP_ABILITY_NOT_TESTED:
                strncpy(stringPtr, "NOT_TESTED", stringLength);
                break;

            case ED_MASTERSHIP_ABILITY_QUALIFIED:
                strncpy(stringPtr, "QUALIFIED", stringLength);
                break;

            case ED_MASTERSHIP_ABILITY_NOT_QUALIFIED:
                strncpy(stringPtr, "NOT_QUALIFIED", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: EL_GetICONConnectivityString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 connectivityNumber - connectivity number to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetICONConnectivityString(char *stringPtr,
                                  ELECTION_DATA_ICON_CONNECTIVITY connectivityNumber,
                                  UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (connectivityNumber)
        {
            case ED_ICON_CONNECTIVITY_NOT_TESTED:
                strncpy(stringPtr, "NOT_TESTED", stringLength);
                break;

            case ED_ICON_CONNECTIVITY_CONNECTED:
                strncpy(stringPtr, "CONNECTED", stringLength);
                break;

            case ED_ICON_CONNECTIVITY_NOT_CONNECTED:
                strncpy(stringPtr, "NOT_CONNECTED", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  EL_GetTaskStateString
**
**  Parameters:     stringPtr - pointer to store the constructed string
**                  taskStateNumber - state number to decode
**                  stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void EL_GetTaskStateString(char *stringPtr, UINT8 taskStateNumber, UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (taskStateNumber)
        {
            case ELECTION_NOT_YET_RUN:
                strncpy(stringPtr, "NOT_YET_RUN", stringLength);
                break;

            case ELECTION_STARTING:
                strncpy(stringPtr, "STARTING", stringLength);
                break;

            case ELECTION_IN_PROGRESS:
                strncpy(stringPtr, "IN_PROGRESS", stringLength);
                break;

            case ELECTION_STAYING_MASTER:
                strncpy(stringPtr, "STAYING_MASTER", stringLength);
                break;

            case ELECTION_SWITCHING_TO_MASTER:
                strncpy(stringPtr, "SWITCHING_TO_MASTER", stringLength);
                break;

            case ELECTION_STAYING_SLAVE:
                strncpy(stringPtr, "STAYING_SLAVE", stringLength);
                break;

            case ELECTION_SWITCHING_TO_SLAVE:
                strncpy(stringPtr, "SWITCHING_TO_SLAVE", stringLength);
                break;

            case ELECTION_STAYING_SINGLE:
                strncpy(stringPtr, "STAYING_SINGLE", stringLength);
                break;

            case ELECTION_SWITCHING_TO_SINGLE:
                strncpy(stringPtr, "SWITCHING_TO_SINGLE", stringLength);
                break;

            case ELECTION_AM_MASTER:
                strncpy(stringPtr, "AM_MASTER", stringLength);
                break;

            case ELECTION_AM_SLAVE:
                strncpy(stringPtr, "AM_SLAVE", stringLength);
                break;

            case ELECTION_AM_SINGLE:
                strncpy(stringPtr, "AM_SINGLE", stringLength);
                break;

            case ELECTION_FINISHED:
                strncpy(stringPtr, "FINISHED", stringLength);
                break;

            case ELECTION_FAILED:
                strncpy(stringPtr, "FAILED", stringLength);
                break;

            case ELECTION_INACTIVE:
                strncpy(stringPtr, "INACTIVE", stringLength);
                break;

            case ELECTION_DISASTER:
                strncpy(stringPtr, "DISASTER", stringLength);
                break;

            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
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
