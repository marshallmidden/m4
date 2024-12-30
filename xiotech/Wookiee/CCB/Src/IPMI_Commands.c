/* $Id: IPMI_Commands.c 156535 2011-06-24 23:13:05Z m4 $ */
/**
******************************************************************************
**
**  @file       IPMI_Commands.c
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "IPMI_Commands.h"

#include "debug_files.h"
#include "IPMI_Defines.h"
#include "XIO_Std.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT32 CommandSend(IPMI_INTERFACE *, COMMAND_HEADER *);
static UINT32 CommandWaitForCompletion(COMMAND_HEADER *pCommandHeader);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Convert completion code into string
**
**              IPMI v2.0 - Section 5.2 - p40
**
**  @param      Standard IPMI completion code argument
**
**  @return     Pointer to string for given completion code
**
******************************************************************************
**/
static const char *GetCompletionCodeString(UINT32 completionCode)
{
    const char *pReturn = "Reserved";

    if (completionCode >= IPMI_COMPLETION_CODE_OEM_RANGE_LOW &&
        completionCode <= IPMI_COMPLETION_CODE_OEM_RANGE_HIGH)
    {
        return "Device Specific";
    }

    if (completionCode >= IPMI_COMPLETION_CODE_COMMAND_SPECIFIC_RANGE_LOW &&
        completionCode <= IPMI_COMPLETION_CODE_COMMAND_SPECIFIC_RANGE_HIGH)
    {
        return "Command Specific";
    }

    switch (completionCode)
    {
        case IPMI_COMPLETION_CODE_NORMAL:
            return "Normal";

        case IPMI_COMPLETION_CODE_NODE_BUSY:
            return "Node busy";

        case IPMI_COMPLETION_CODE_INVALID_COMMAND:
            return "Invalid command";

        case IPMI_COMPLETION_CODE_COMMAND_INVALID_FOR_LUN:
            return "Command invalid for LUN";

        case IPMI_COMPLETION_CODE_COMMAND_TIMEOUT:
            return "Timeout";

        case IPMI_COMPLETION_CODE_OUT_OF_SPACE:
            return "Out of space";

        case IPMI_COMPLETION_CODE_RESERVATION_CANCELLED:
            return "Reservation cancelled";

        case IPMI_COMPLETION_CODE_REQUEST_DATA_TRUNCATED:
            return "Request data truncated";

        case IPMI_COMPLETION_CODE_REQUEST_DATA_LENGTH_INVALID:
            return "Request data length invalid";

        case IPMI_COMPLETION_CODE_REQUEST_DATA_LENGTH_EXCEEDED:
            return "Request data field limit excedded";

        case IPMI_COMPLETION_CODE_PARAMETER_OUT_OF_RANGE:
            return "Parameter out of range";

        case IPMI_COMPLETION_CODE_CANNOT_RETURN_NUMBER_OF_BYTES:
            return "Cannot return number of requested bytes";

        case IPMI_COMPLETION_CODE_REQUESTED_ITEM_NOT_PRESENT:
            return "Requested item not present";

        case IPMI_COMPLETION_CODE_INVALID_DATA_FIELD_IN_REQUEST:
            return "Invalid data field in request";

        case IPMI_COMPLETION_CODE_ILLEGAL_COMMAND:
            return "Command illegal for sensor type";

        case IPMI_COMPLETION_CODE_NO_COMMAND_RESPONSE:
            return "Command response could not be provided";

        case IPMI_COMPLETION_CODE_CANNOT_EXECUTE_PRIOR_REQUEST:
            return "Cannot execute duplicated request";

        case IPMI_COMPLETION_CODE_SDR_IN_UPDATE_MODE:
            return "Cannot provide response (SDR update mode)";

        case IPMI_COMPLETION_CODE_DEVICE_IN_FIRMWARE_UPDATE_MODE:
            return "Cannot provide response (firmware update mode)";

        case IPMI_COMPLETION_CODE_BMC_INITIALIZATION_IN_PROGRESS:
            return "Cannot provide response (BMC init mode)";

        case IPMI_COMPLETION_CODE_DESTINATION_UNAVAILABLE:
            return "Destination unavailable";

        case IPMI_COMPLETION_CODE_SECURITY_RESTRICTION:
            return "Insufficient priviledge level";

        case IPMI_COMPLETION_CODE_COMMAND_NOT_SUPPORTED:
            return "Cannot execute command (present state)";

        case IPMI_COMPLETION_CODE_COMMAND_DISABLED:
            return "Cannot execute command (unavailable)";

        case IPMI_COMPLETION_CODE_UNSPECIFIED:
            return "Unspecified";

        default:
            /* Reserved */
            break;
    }

    return pReturn;
}


/**
******************************************************************************
**
**  @brief      Check for a good and normal IPMI command completion
**
**              Check the command header for the completeion code
**
**  @param      pCommandHeader - pointer to the command header
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 CommandCheckGoodCompletion(COMMAND_HEADER *pHeader)
{
    UINT32      returnCode = GOOD;

    if (pHeader == NULL)
    {
        dprintf(DPRINTF_IPMI_COMMAND, " Command pHeader is NULL\n");
        returnCode = ERROR;
    }

    if ((returnCode == GOOD) && (pHeader->flags.bits.sendCommandError == TRUE))
    {
        dprintf(DPRINTF_IPMI_COMMAND, " Command sendCommandError is set\n");
        returnCode = ERROR;
    }

    if ((returnCode == GOOD) && (pHeader->flags.bits.receiveResponseError == TRUE))
    {
        dprintf(DPRINTF_IPMI_COMMAND, " Command receiveResponseError is set\n");
        returnCode = ERROR;
    }

    if ((returnCode == GOOD) && (pHeader->flags.bits.commandComplete == FALSE))
    {
        dprintf(DPRINTF_IPMI_COMMAND, " Command did not complete\n");
        returnCode = ERROR;
    }

    if ((returnCode == GOOD) && (pHeader->responseLength == 0))
    {
        dprintf(DPRINTF_IPMI_COMMAND, " pHeader->responseLength is zero\n");
        returnCode = ERROR;
    }

    if ((returnCode == GOOD) && (pHeader->pResponse == NULL))
    {
        dprintf(DPRINTF_IPMI_COMMAND, " pHeader->pResponse is NULL\n");
        returnCode = ERROR;
    }

    if (returnCode == GOOD)
    {
        dprintf(DPRINTF_IPMI_COMMAND, " Completion:  0x%02x [%s]\n",
                pHeader->pResponse[0], GetCompletionCodeString(pHeader->pResponse[0]));

        if (pHeader->pResponse[0] != IPMI_COMPLETION_CODE_NORMAL)
        {
            returnCode = ERROR;
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      Send a command to the BMC and wait for it to complete
**
**              This is a blocking call
**
**  @param      pCommandHeader - Pointer to the command header
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 CommandSend(IPMI_INTERFACE *pInterface, COMMAND_HEADER *pCommandHeader)
{
    UINT32      returnCode;

    dprintf(DPRINTF_IPMI_COMMAND, "** Command Send **\n");
    dprintf(DPRINTF_IPMI_COMMAND, "  pHeader:     %p\n", pCommandHeader);
    dprintf(DPRINTF_IPMI_COMMAND, "  pResponse:   %p\n", &pCommandHeader->pResponse);

    if (!pCommandHeader)
    {
        dprintf(DPRINTF_IPMI_COMMAND, "%s: pCommandHeader is NULL\n", __func__);
        return ERROR;
    }

    returnCode = IPMI_InterfaceSend(pInterface, pCommandHeader);
    dprintf(DPRINTF_IPMI_COMMAND, "  InterfaceSend returned: %d\n", returnCode);

    /*
     * If the command was issued successfully, wait for it to complete.
     * We'll return GOOD here, even if the command's completionCode is not
     * good.  A GOOD return from here means the BMC handled the command.
     */
    if (returnCode != GOOD)
    {
        dprintf(DPRINTF_IPMI_COMMAND, "%s: Not waiting for completion\n", __func__);
        return returnCode;
    }

    returnCode = CommandWaitForCompletion(pCommandHeader);

    return returnCode;
}


/**
******************************************************************************
**
**  @brief      Wait for the, now issued, command to complete
**
**              Poll the command header for the commandComplete flag
**              to be set, and return GOOD if no errors are detected.
**
**  @param      pCommandHeader - pointer to the command header
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 CommandWaitForCompletion(COMMAND_HEADER *pCommandHeader)
{
    if (!pCommandHeader)
    {
        dprintf(DPRINTF_IPMI_COMMAND, "%s: pCommandHeader is NULL\n", __func__);
        return ERROR;
    }

    /* Wait for the commandComplete flag to be set */

    while (pCommandHeader->flags.bits.commandComplete == FALSE)
    {
        /*
         * McMaster - This would be a good spot for a completion signal
         *   so this task could sleep while waiting for the command.
         */
        TaskSleepMS(20);
    }

    /* Check for command errors */

    if (pCommandHeader->flags.bits.sendCommandError == TRUE)
    {
        dprintf(DPRINTF_IPMI_COMMAND, "%s: Error sending command\n", __func__);
        return ERROR;
    }
    if (pCommandHeader->flags.bits.receiveResponseError == TRUE)
    {
        dprintf(DPRINTF_IPMI_COMMAND, "%s: Error receiving response\n", __func__);
        return ERROR;
    }

    /* No errors, so return with good status */

    dprintf(DPRINTF_IPMI_COMMAND, "%s: GOOD - 0x%02x - [%s]\n",
            __func__, pCommandHeader->pResponse[0],
            GetCompletionCodeString(pCommandHeader->pResponse[0]));

    return GOOD;
}


/**
******************************************************************************
**
**  @brief      Issue 'set channel access' command to the BMC
**
**              IPMI v2.0 - Section 22.22 - p288
**
**  @param      pSetChannelAccess - Pointer to command structure
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 CommandSetChannelAccess(IPMI_INTERFACE *pInterface,
                               COMMAND_SET_CHANNEL_ACCESS *pSetChannelAccess)
{
    UINT32      returnCode = ERROR;

    if (pSetChannelAccess != NULL)
    {
        /*
         * Fill in the header information
         */
        pSetChannelAccess->header.flags.value = 0;
        pSetChannelAccess->header.message.netFn = IPMI_NETFN_APP;
        pSetChannelAccess->header.message.command = IPMI_CMD_APP_SET_CHANNEL_ACCESS;
        pSetChannelAccess->header.message.dataLength = sizeof(pSetChannelAccess->request);
        pSetChannelAccess->header.message.pData = (UINT8 *)&pSetChannelAccess->request;
        pSetChannelAccess->header.responseLength = sizeof(pSetChannelAccess->response);
        pSetChannelAccess->header.pResponse = (UINT8 *)&pSetChannelAccess->response;

        /*
         * Send the command to the BMC
         */
        dprintf(DPRINTF_IPMI_COMMAND, "** Command - Set Channel Access **\n");
        returnCode = CommandSend(pInterface, &pSetChannelAccess->header);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      Issue 'get channel access' command to the BMC
**
**              IPMI v2.0 - Section 22.23 - p290
**
**  @param      pGetChannelAccess - Pointer to command structure
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 CommandGetChannelAccess(IPMI_INTERFACE *pInterface,
                               COMMAND_GET_CHANNEL_ACCESS *pGetChannelAccess)
{
    UINT32      returnCode = ERROR;

    if (pGetChannelAccess != NULL)
    {
        /*
         * Fill in the header information
         */
        pGetChannelAccess->header.flags.value = 0;
        pGetChannelAccess->header.message.netFn = IPMI_NETFN_APP;
        pGetChannelAccess->header.message.command = IPMI_CMD_APP_GET_CHANNEL_ACCESS;
        pGetChannelAccess->header.message.dataLength = sizeof(pGetChannelAccess->request);
        pGetChannelAccess->header.message.pData = (UINT8 *)&pGetChannelAccess->request;
        pGetChannelAccess->header.responseLength = sizeof(pGetChannelAccess->response);
        pGetChannelAccess->header.pResponse = (UINT8 *)&pGetChannelAccess->response;

        /*
         * Send the command to the BMC
         */
        dprintf(DPRINTF_IPMI_COMMAND, "** Command - Get Channel Access **\n");
        returnCode = CommandSend(pInterface, &pGetChannelAccess->header);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      Issue 'Set LAN parameters' command to the BMC
**
**              IPMI v2.0 - Section 23.1 - p299
**
**  @param      pInterface    - Pointer to an IPMI interface structure
**  @param      pGetLANConfig - Pointer to command structure
**
**  @return     GOOD or ERROR
**
**  @attention  header.message.dataLength must be set by caller
**
******************************************************************************
**/
UINT32 CommandSetLANConfig(IPMI_INTERFACE *pInterface,
                           COMMAND_SET_LAN_CONFIG *pSetLANConfig, UINT32 requestLength)
{
    if (!pSetLANConfig)
    {
        return ERROR;
    }

    /* Fill in the header information */

    pSetLANConfig->header.flags.value = 0;
    pSetLANConfig->header.message.netFn = IPMI_NETFN_TRANSPORT;
    pSetLANConfig->header.message.command = IPMI_CMD_TRANSPORT_SET_LAN_CONFIGURATION_PARAMETERS;
    pSetLANConfig->header.message.dataLength = requestLength;
    pSetLANConfig->header.message.pData = (UINT8 *)&pSetLANConfig->request;
    pSetLANConfig->header.responseLength = sizeof(pSetLANConfig->response);
    pSetLANConfig->header.pResponse = (UINT8 *)&pSetLANConfig->response;

    /* Send the command to the BMC */

    dprintf(DPRINTF_IPMI_COMMAND, "** Command - Set LAN Configuration **\n");
    return CommandSend(pInterface, &pSetLANConfig->header);
}


/**
******************************************************************************
**
**  @brief      Issue 'CommandSetPEFConfig' command to the BMC
**
**              IPMI v2.0 - Section 30.3 - p376
**
**  @param      pSetPEFConfig - Pointer to command structure
**
**  @return     GOOD or ERROR
**
**  @attention  header.message.dataLength must be set by caller
**
******************************************************************************
**/
UINT32 CommandSetPEFConfig(IPMI_INTERFACE *pInterface,
                           COMMAND_SET_PEF_CONFIG *pSetPEFConfig, UINT32 requestLength)
{
    UINT32      returnCode = ERROR;

    if (pSetPEFConfig != NULL)
    {
        /*
         * Fill in the header information
         */
        pSetPEFConfig->header.flags.value = 0;
        pSetPEFConfig->header.message.netFn = IPMI_NETFN_SENSOR_EVENT;
        pSetPEFConfig->header.message.command = IPMI_CMD_EVENT_SET_PEF_CONFIGURATION_PARAMETERS;
        pSetPEFConfig->header.message.dataLength = requestLength;
        pSetPEFConfig->header.message.pData = (UINT8 *)&pSetPEFConfig->request;
        pSetPEFConfig->header.responseLength = sizeof(pSetPEFConfig->response);
        pSetPEFConfig->header.pResponse = (UINT8 *)&pSetPEFConfig->response;

        /*
         * Send the command to the BMC
         */
        dprintf(DPRINTF_IPMI_COMMAND, "** Command - Set PEF Configuration **\n");
        returnCode = CommandSend(pInterface, &pSetPEFConfig->header);

        if (returnCode == 0)
        {
            /* Byte 0 */
            if (pSetPEFConfig->header.responseLength > 0)
            {
                dprintf(DPRINTF_IPMI_COMMAND, "  Completion:  %d [%s]\n",
                        pSetPEFConfig->response.completionCode,
                        GetCompletionCodeString(pSetPEFConfig->response.completionCode));
            }
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      Issue 'CommandGetPEFConfig' command to the BMC
**
**              IPMI v2.0 - Section 30.4 - p377
**
**  @param      pGetPEFConfig - Pointer to command structure
**
**  @return     GOOD or ERROR
**
**  @attention  header.responseLength must be set by caller
**
******************************************************************************
**/
UINT32 CommandGetPEFConfig(IPMI_INTERFACE *pInterface,
                           COMMAND_GET_PEF_CONFIG *pGetPEFConfig, UINT32 responseLength)
{
    UINT32      returnCode = ERROR;

    if (pGetPEFConfig != NULL)
    {
        /*
         * Fill in the header information
         */
        pGetPEFConfig->header.flags.value = 0;
        pGetPEFConfig->header.message.netFn = IPMI_NETFN_SENSOR_EVENT;
        pGetPEFConfig->header.message.command = IPMI_CMD_EVENT_GET_PEF_CONFIGURATION_PARAMETERS;
        pGetPEFConfig->header.message.dataLength = sizeof(pGetPEFConfig->request);
        pGetPEFConfig->header.message.pData = (UINT8 *)&pGetPEFConfig->request;
        pGetPEFConfig->header.responseLength = responseLength;
        pGetPEFConfig->header.pResponse = (UINT8 *)&pGetPEFConfig->response;

        /*
         * Send the command to the BMC
         */
        dprintf(DPRINTF_IPMI_COMMAND, "** Command - Get PEF Configuration **\n");
        returnCode = CommandSend(pInterface, &pGetPEFConfig->header);

        if (returnCode == 0)
        {
            /* Byte 0 */
            if (pGetPEFConfig->header.responseLength > 0)
            {
                dprintf(DPRINTF_IPMI_COMMAND, "  Completion:  %d [%s]\n",
                        pGetPEFConfig->response.completionCode,
                        GetCompletionCodeString(pGetPEFConfig->response.completionCode));
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
