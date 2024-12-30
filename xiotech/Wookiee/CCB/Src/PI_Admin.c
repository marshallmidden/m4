/* $Id: PI_Admin.c 143020 2010-06-22 18:35:56Z m4 $*/
/**
******************************************************************************
**
**  @file   PI_Admin.c
**
**  @brief  Packet Interface for Administrative Commands
**
**  Handler functions for Administrative request packets
**  such as Versions & Credits, Save/Restore Config, etc.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "PI_CmdHandlers.h"

#include "cps_init.h"
#include "debug_files.h"
#include "kernel.h"
#include "led.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PI_Misc.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "realtime.h"
#include "serial_num.h"
#include "X1_Structs.h"
#include "X1_Utils.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_FirmwareVersionInfo
**
** Description: Get Firmware Header information for the specified fw piece
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FirmwareVersionInfo(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_FW_HEADER_RSP *ptrOutPkt = NULL;
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the MRP return data.
     */
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(PI_FW_VERSION_RSP);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    switch (((PI_FW_VERSION_REQ *)(pReqPacket->pPacket))->fwType)
    {
        case GET_CCB_RUNTIME_FW:
            memcpy(&((PI_FW_VERSION_RSP *)(pRspPacket->pPacket))->fw,
                   (UINT8 *)CCBRuntimeFWHAddr, sizeof(FW_HEADER));
            ((PI_FW_VERSION_RSP *)(pRspPacket->pPacket))->header.len = sizeof(*ptrOutPkt);
            return PI_GOOD;

        case GET_CCB_DIAG_FW:
            pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
            pRspPacket->pHeader->errorCode = PI_ERROR;
            return PI_PARAMETER_ERROR;

        default:
            /*
             * Send the request to Thunderbolt.  This function handles timeout
             * conditions and task switches while waiting.
             */
            rc = PI_ExecMRP(NULL, 0, ((PI_FW_VERSION_REQ *)(pReqPacket->pPacket))->fwType,
                            ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

            /*
             * Modify the MRP return data with its actual return value.
             */
            pRspPacket->pHeader->status = rc;
            pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;
            return rc;
    }
}

/*----------------------------------------------------------------------------
** Function:    PI_FirmwareSystemReleaseLevel
**
** Description: Get the Firmware System Release Level.  This involves looking
**              at the system release level for each firmware component in
**              the system.  A function was written to do this (and some
**              other stuff) for X1.  Use it here to just return the
**              System Release Level.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FirmwareSystemReleaseLevel(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    X1_FWVERSIONS *pFWVersions = NULL;
    PI_FW_SYS_REL_LEVEL_RSP *pSysRelRsp = NULL;
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the firmware version info and thesystem release
     * response packet.
     */
    pFWVersions = MallocWC(sizeof(*pFWVersions));
    pSysRelRsp = MallocWC(sizeof(*pSysRelRsp));

    /*
     * Get all firmware version info.  This function will examine the
     * system release level of each component and derive a single
     * system release level.
     */
    GetFirmwareVersions(pFWVersions);

    /*
     * Return the UINT32 system release level and the 8 character tag field.
     */
    pSysRelRsp->systemRelease = pFWVersions->system.verMajMin;
    memcpy(pSysRelRsp->tag, pFWVersions->system.tag, 8);

    dprintf(DPRINTF_PI_COMMANDS, "PI_FirmwareSystemReleaseLevel - systemRelease=0x%08X  tag=%s\n",
            pSysRelRsp->systemRelease, pSysRelRsp->tag);

    /*
     * Set up the response packet return values.
     */
    pRspPacket->pPacket = (UINT8 *)pSysRelRsp;
    pRspPacket->pHeader->length = sizeof(*pSysRelRsp);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    /*
     * Free memory used for the firmware versions request.
     */
    Free(pFWVersions);

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_SetTime
**
** Description: Set the time for the controller's real time clock
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_SetTime(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /*
     * Set the time based on the passed system seconds.
     */

    SetClockFromSysTime(((PI_SETTIME_REQ *)(pReqPacket->pPacket))->sysTime);

    /*
     * This function does not return anything to the caller.  Set the
     * length to 0 and the pointer to NULL.
     */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_GetTime
**
** Description: Get the controller's time
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     Controller time
**
**--------------------------------------------------------------------------*/
INT32 PI_GetTime(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_ADMIN_GETTIME_RSP *pTime;

    /*
     * Get the controller time.
     */
    pTime = MallocWC(sizeof(*pTime));
    pTime->sysTime = RTC_GetLongTimeStamp();

    /*
     * Fill in the response header and packet.
     */
    pRspPacket->pHeader->length = sizeof(*pTime);
    pRspPacket->pPacket = (UINT8 *)pTime;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_LedControl
**
** Description: Turn on / off system LEDs
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_LedControl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT8       rc = PI_GOOD;
    INT32       errCode = 0;

    /*
     * This function does not return anything to the caller.  Set the
     * length to 0 and the pointer to NULL.
     */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pPacket = NULL;

    {
        /*
         * Determine which led to control and what state to set it. If
         * it is not a recognized led, return an error.
         */
        switch (((PI_LEDCNTL_REQ *)(pReqPacket->pPacket))->led)
        {
                /*
                 * Is it the attention led?  If so, turn it on /off based on the
                 * value passed.
                 */
            case PI_LED_ATTENTION:
                if (((PI_LEDCNTL_REQ *)(pReqPacket->pPacket))->value)
                {
                    LEDSetAttention(TRUE);
                }
                else
                {
                    LEDSetAttention(FALSE);
                }
                break;
            default:
                errCode = 0;
                rc = PI_PARAMETER_ERROR;
                break;
        }
    }

    pRspPacket->pHeader->errorCode = errCode;
    pRspPacket->pHeader->status = rc;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_SetIpAddress
**
** Description: Set the controllers ip address,
**              subnet mask, and gateway address
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_SetIpAddress(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT32      serialNum;      /* Controller Serial Number */
    UINT32      ipAddress;      /* IP Address               */
    UINT32      subnetMask;     /* Subnet Mask Address      */
    UINT32      gatewayAddress; /* Gateway Address          */
    INT32       errorCode = GOOD;
    UINT8       rc = PI_GOOD;
    UINT8       cont = 0;

    dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - ENTER\n");

    ipAddress = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->ipAddress;
    subnetMask = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->subnetMask;
    gatewayAddress = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->gatewayAddress;
    serialNum = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->serialNum;

    /*
     * Make sure the controller is in a POWER_UP_COMPLETE,
     * OPERATIONAL state.  Also check that the IP Address
     * and subnet mask are not 0.  Gateway can be 0.
     */
    if ((GetPowerUpState() != POWER_UP_COMPLETE) ||
        (GetControllerFailureState() != FD_STATE_OPERATIONAL) ||
        (ipAddress == 0) || (subnetMask == 0))
    {
        rc = PI_ERROR;
        errorCode = DE_IP_OR_SUBNET_ZERO;
    }

    /*
     * Do an additional validity check on IP and subnet.  This is done
     * in AddressQuickCheck() for serial input.
     */
    if (gatewayAddress && ((gatewayAddress & subnetMask) != (ipAddress & subnetMask)))
    {
        rc = ERROR;
        errorCode = DE_SET_IP_ERROR;
    }

    if (rc == PI_GOOD)
    {
        /*
         * If we are not the master Controller we need to
         * update our own NVRAM
         */
        if ((!TestforMaster(GetMyControllerSN())) && (serialNum == GetMyControllerSN()))
        {
            dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - Updating NVRAM\n");

            SetIPAddress(ipAddress);
            SetSubnetMask(subnetMask);
            SetGatewayAddress(gatewayAddress);
        }

        /*
         * Else if we are the master Controller we need to
         * update the Controller Config Map
         */
        else if (TestforMaster(GetMyControllerSN()))
        {
            dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - We are Master\n");
            dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - Reading Controller Config Map\n");

            /*
             * Update the network settings in the controller configuration
             * map.  The false signals that only the "new" network settings
             * should be updated.
             */
            /*
             * Load the Controller Map.
             */
            if (LoadControllerMap() != GOOD)
            {
                rc = ERROR;
                errorCode = DE_SET_IP_ERROR;
            }

            if (rc == GOOD)
            {
                /*
                 * Find the contoller being modified
                 * in the controller config map.
                 */
                cont = GetCommunicationsSlot(serialNum);

                /*
                 * If we could not find ourselves set the error
                 */
                if (cont == MAX_CONTROLLERS)
                {
                    dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - Failed to find controller in controller map\n");
                    errorCode = DENONXDEV;
                    rc = ERROR;
                }
            }

            /*
             * If we found the controller in the controller map,
             * update its network addresses.
             */
            if (rc == GOOD)
            {
                CCM_IPAddressNew(cont) = ipAddress;
                CCM_SubnetNew(cont) = subnetMask;
                CCM_GatewayNew(cont) = gatewayAddress;

                if (WriteControllerMap(&cntlConfigMap) != 0)
                {
                    dprintf(DPRINTF_DEFAULT, "PI_SetIpAddress - Failed to write controller map\n");
                    errorCode = DENONXDEV;
                    rc = ERROR;
                }
            }
        }
        else
        {
            rc = ERROR;
        }
    }

    /*
     * This function does not return anything to the caller.  Set the
     * length to 0 and the pointer to NULL.
     */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_GetIpAddress
**
** Description: Get the controllers ip address,
**              subnet mask, and gateway address
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_GetIpAddress(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /*
     * Allocate memory for the MRP return data.
     */
    PI_GET_IP_RSP *pOutPkt = MallocWC(sizeof(*pOutPkt));

    /*
     * Fill in the return data
     */
    pOutPkt->serialNum = GetSerialNumber(CONTROLLER_SN);
    pOutPkt->ipAddress = CntlSetup_GetIPAddress();
    pOutPkt->subnetMask = CntlSetup_GetSubnetMask();
    pOutPkt->gatewayAddress = CntlSetup_GetGatewayAddress();

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pOutPkt;
    pRspPacket->pHeader->length = sizeof(*pOutPkt);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    return (PI_GOOD);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
