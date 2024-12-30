/* $Id: IPMI_BMC.c 161210 2013-06-05 19:56:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       IPMI_BMC.c
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "IPMI_Defines.h"

#include "debug_files.h"
#include "IPMI_Commands.h"
#include "nvram.h"
#include "PortServerUtils.h"
#include "XIO_Std.h"
#include "HWM.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
struct IPMI_BMC_PRIVATE
{
    IPMI_INTERFACE *pInterface;
    UINT8       configureEthernetFlag;
};

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT32 IPMI_BMCDestroy(IPMI_BMC **pBMCPtr);
static UINT32 BMCDisablePEF(IPMI_BMC *);
static UINT32 BMCDisableLANAccess(IPMI_BMC *);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize and validate the BMC
**
**  @param      File descriptor of IPMI interface
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 IPMI_BMCCreate(IPMI_BMC **pBMCPtr, IPMI_INTERFACE *pInterface)
{
    UINT32      returnCode = GOOD;

    if (!pBMCPtr || !pInterface)
    {
        dprintf(DPRINTF_IPMI, "Input pointer is NULL\n");
        return ERROR;
    }

    /* Initialize the BMC information */

    *pBMCPtr = MallocWC(sizeof(**pBMCPtr));
    (*pBMCPtr)->pInterface = pInterface;
    (*pBMCPtr)->configureEthernetFlag = TRUE;

    if (!pInterface)
    {
        dprintf(DPRINTF_IPMI, "Interface is NULL\n");
        returnCode = ERROR;
        goto out;
    }

    if (returnCode != GOOD)
    {
        goto out;
    }

    /*
     * Turn off all unwanted LAN behavior of the BMC
     * NOTE: Some systems don't support LAN access, so don't change returnCode.
     */
    if ((hwm_platform->flags & PLATFORM_FLAG_DISABLE_BMC_NET))
    {
        /* Check if this is an IPMI v2.0 or newer BMC */

        /* Disable all inbound LAN channels to the BMC */

        dprintf(DPRINTF_DEFAULT, "Disable inbound LAN access to the BMC\n");
        BMCDisableLANAccess(*pBMCPtr);

        /* Disable automatic BMC alert broadcasts */

        dprintf(DPRINTF_DEFAULT, "Disable BMC alert (SNMP) broadcasts\n");
        BMCDisablePEF(*pBMCPtr);
    }

  out:
    /* Free the BMC structure on any ERROR */

    if (returnCode != GOOD)
    {
        IPMI_BMCDestroy(pBMCPtr);
    }

    return returnCode;
}


/**
******************************************************************************
**
**  @brief      Free all resources allocated to the BMC structure
**
**  @param      Pointer to a BMC pointer
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 IPMI_BMCDestroy(IPMI_BMC **pBMCPtr)
{
    dprintf(DPRINTF_IPMI, "Destroying BMC\n");

    if (!pBMCPtr || !*pBMCPtr)
    {
        dprintf(DPRINTF_IPMI, "BMC pointer already NULL\n");
        return ERROR;
    }

    /* Remove the BMC from the interface */

    Free(*pBMCPtr);

    /* All resources pertaining to the BMC are freed */

    return GOOD;
}


/**
******************************************************************************
**
**  @brief      Return the interface pointer for the given BMC
**
**  @param      Pointer to a BMC
**
**  @return     Interface pointer (NULL on ERROR)
**
******************************************************************************
**/
static IPMI_INTERFACE *IPMI_BMCGetInterface(IPMI_BMC *pBMC)
{
    if (!pBMC)
    {
        dprintf(DPRINTF_IPMI, "BMC pointer is NULL\n");
        return NULL;
    }

    return pBMC->pInterface;
}


/**
******************************************************************************
**
**  @brief      Configure the BMC's IP address, netmask, and gateway
**              The BMC's gratuitous ARPs will use this IP address
**
**  @param      Pointer to a BMC
**
**  @return     GOOD or ERROR
**
**  @attention  Call this function only when the desired IP address, netmask
**              and gateway are stored in the controller setup structure.
**              This is called at part of the manufacturing clean process.
**
******************************************************************************
**/
UINT32 IPMI_BMCConfigureEthernet(IPMI_BMC *pBMC)
{
    UINT32      returnCode = GOOD;
//----------------------------------------------------------------------------
// NOTE: If controller(s) have messed up IP address and you cannot put system
//       on that network to change it back. Controllers must be talking to
//       each other and have access to the BE disks/bays/ISEs - to change IP.
//  Procedure: a) take slave and master down.
//  b) Bring master up, serial port change to new IP address.
//  c) If you cannot get the IP to take completely ... vcginfo shows bad things.
//  d) You can "fidread -S -t binary -f /tmp/fid6 6" and "fidread -S -t binary -f /tmp/fid7 7" and 
//     binary edit the right things into them, then "fidwrite -N 6 /tmp/fid6" and 7.
//  e) Or ... you can use nvrsetup, ifconfig eth0, /etc/sysconfig/network/ifcfg-eth0 and
//  f) also #define FORCE_IP_CHANGE in file quorum_util.c, IPMI_BMC.c, and nvram.c.
//  g) Edit in what you want to force, etc. IPMI_BMC.c is to get around the MM card if
//     you cannot use serial port to change the IP -- because that powers it off.
//----------------------------------------------------------------------------
// #define FORCE_IP_CHANGE
#ifndef FORCE_IP_CHANGE
    IPMI_INTERFACE *pInterface;
    COMMAND_SET_LAN_CONFIG setLANConfig;        /* Uninitialized */
    UINT32      ipaddr;
    ETHERNET_MAC_ADDRESS ethernetMACAddress;
    UINT32      netmask;
    UINT32      gwaddr;

    if (!pBMC)
    {
        dprintf(DPRINTF_IPMI, "pBMC is NULL\n");
        return ERROR;
    }

    pInterface = IPMI_BMCGetInterface(pBMC);

    /*
     * Check that the IPMI interface and controller Ethernet have been
     * configured and are ready to be programmed into the BMC.
     */
    if (!pInterface)
    {
        dprintf(DPRINTF_IPMI, "pInterface is NULL\n");
        return ERROR;
    }

    if (!pBMC->configureEthernetFlag)
    {
        fprintf(stderr, "%s: Etherenet configuration not needed\n", __func__);
        return GOOD;
    }

    if (!IsControllerSetup())
    {
        fprintf(stderr, "%s: Can't set BCM Ethernet yet\n", __func__);
        return ERROR;
    }

    /* Parameter 0 - Changes in progress */
    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_SET_IN_PROGRESS;
    setLANConfig.request.parameterUnion.setInProgress.actionFlag = LCP_ACTION_FLAG_IN_PROGRESS;

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.setInProgress));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    /*
     * Ignore an 'already in progress' error code (0x81), since we're
     * the only ones that should be updating this information.
     */
    if (returnCode != GOOD && setLANConfig.response.completionCode != 0x81)
    {
        return returnCode;
    }

    ipaddr = GetIPAddress();

    /* Parameter 3 */
    dprintf(DPRINTF_DEFAULT, "BMC IP Address: %d.%d.%d.%d\n",
            (UINT8)(ipaddr >> 0), (UINT8)(ipaddr >> 8),
            (UINT8)(ipaddr >> 16), (UINT8)(ipaddr >> 24));

    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_IP_ADDRESS;
    setLANConfig.request.parameterUnion.ipAddress.ipAddress1_MSB = (UINT8)(ipaddr >> 0);
    setLANConfig.request.parameterUnion.ipAddress.ipAddress2 = (UINT8)(ipaddr >> 8);
    setLANConfig.request.parameterUnion.ipAddress.ipAddress3 = (UINT8)(ipaddr >> 16);
    setLANConfig.request.parameterUnion.ipAddress.ipAddress4_LSB = (UINT8)(ipaddr >> 24);

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.ipAddress));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 4 */
    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_IP_ADDRESS_SOURCE;
    setLANConfig.request.parameterUnion.ipAddressSource.bits.source = LCP_IP_ADDRESS_SOURCE_STATIC;

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.ipAddressSource));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 5 */
    ethernetMACAddress = GetMacAddrFromInterface(ethernetDriver.interfaceHandle);

    dprintf(DPRINTF_DEFAULT, "BMC MAC:        %02x:%02x:%02x:%02x:%02x:%02x\n",
            ethernetMACAddress.macByte[0], ethernetMACAddress.macByte[1],
            ethernetMACAddress.macByte[2], ethernetMACAddress.macByte[3],
            ethernetMACAddress.macByte[4], ethernetMACAddress.macByte[5]);

    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_MAC_ADDRESS;
    setLANConfig.request.parameterUnion.macAddress.macAddress1_MSB = ethernetMACAddress.macByte[0];
    setLANConfig.request.parameterUnion.macAddress.macAddress2 = ethernetMACAddress.macByte[1];
    setLANConfig.request.parameterUnion.macAddress.macAddress3 = ethernetMACAddress.macByte[2];
    setLANConfig.request.parameterUnion.macAddress.macAddress4 = ethernetMACAddress.macByte[3];
    setLANConfig.request.parameterUnion.macAddress.macAddress5 = ethernetMACAddress.macByte[4];
    setLANConfig.request.parameterUnion.macAddress.macAddress6_LSB = ethernetMACAddress.macByte[5];

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.macAddress));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 6 */
    netmask = GetSubnetMask();
    dprintf(DPRINTF_DEFAULT, "BMC Subnet:     %d.%d.%d.%d\n",
            (UINT8)(netmask >> 0), (UINT8)(netmask >> 8),
            (UINT8)(netmask >> 16), (UINT8)(netmask >> 24));

    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_SUBNET_MASK;
    setLANConfig.request.parameterUnion.subnetMask.subnetMask1_MSB = (UINT8)(netmask >> 0);
    setLANConfig.request.parameterUnion.subnetMask.subnetMask2 = (UINT8)(netmask >> 8);
    setLANConfig.request.parameterUnion.subnetMask.subnetMask3 = (UINT8)(netmask >> 16);
    setLANConfig.request.parameterUnion.subnetMask.subnetMask4_LSB = (UINT8)(netmask >> 24);

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.subnetMask));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 12 */
    gwaddr = GetGatewayAddress();

    dprintf(DPRINTF_DEFAULT, "BMC Gateway:    %d.%d.%d.%d\n",
            (UINT8)(gwaddr >> 0), (UINT8)(gwaddr >> 8),
            (UINT8)(gwaddr >> 16), (UINT8)(gwaddr >> 24));

    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_DEFAULT_GATEWAY_ADDRESS;
    setLANConfig.request.parameterUnion.defaultGatewayAddress.ipAddress1_MSB = (UINT8)(gwaddr >> 0);
    setLANConfig.request.parameterUnion.defaultGatewayAddress.ipAddress2 = (UINT8)(gwaddr >> 8);
    setLANConfig.request.parameterUnion.defaultGatewayAddress.ipAddress3 = (UINT8)(gwaddr >> 16);
    setLANConfig.request.parameterUnion.defaultGatewayAddress.ipAddress4_LSB = (UINT8)(gwaddr >> 24);

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.defaultGatewayAddress));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /*
     * Parameter 0 - Commit the changes
     * NOTE: This will fail on systems that don't support rollback,
     *       so don't alter the return code for this command.
     */
    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_SET_IN_PROGRESS;
    setLANConfig.request.parameterUnion.setInProgress.actionFlag = LCP_ACTION_FLAG_COMMIT_WRITE;

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.setInProgress));

    /* Parameter 0 - Changes complete */
    memset(&setLANConfig, 0, sizeof(setLANConfig));
    setLANConfig.request.channel.number = 1;
    setLANConfig.request.parameterSelector = LCP_PARAMETER_SET_IN_PROGRESS;
    setLANConfig.request.parameterUnion.setInProgress.actionFlag = LCP_ACTION_FLAG_COMPLETE;

    CommandSetLANConfig(pInterface, &setLANConfig,
                        sizeof(setLANConfig.request.channel) +
                        sizeof(setLANConfig.request.parameterSelector) +
                        sizeof(setLANConfig.request.parameterUnion.setInProgress));

    returnCode = CommandCheckGoodCompletion(&setLANConfig.header);

    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /*
     * Remember, in the BMC structure, that we've successfully programmed
     * the Ethernet settings.
     */
#endif /* FORCE_IP_CHANGE */

    dprintf(DPRINTF_DEFAULT, "BMC Ethernet parameters configured\n");

    pBMC->configureEthernetFlag = FALSE;

    return returnCode;
}


/**
******************************************************************************
**
**  @brief      Disable the PEF system - don't send alerts out via Ethernet
**
**  @param      Pointer to a BMC
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 BMCDisablePEF(IPMI_BMC *pBMC)
{
    UINT32      returnCode;
    IPMI_INTERFACE *pInterface;
    COMMAND_GET_PEF_CONFIG getPEFConfig;
    COMMAND_SET_PEF_CONFIG setPEFConfig;

    if (!pBMC)
    {
        dprintf(DPRINTF_IPMI, "pBMC is NULL\n");
        return ERROR;
    }

    pInterface = IPMI_BMCGetInterface(pBMC);
    if (!pInterface)
    {
        dprintf(DPRINTF_IPMI, "pInterface is NULL\n");
        return ERROR;
    }

    /* Parameter 1 - Read */
    memset(&getPEFConfig, 0, sizeof(getPEFConfig));
    getPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_PEF_CONTROL;

    CommandGetPEFConfig(IPMI_BMCGetInterface(pBMC), &getPEFConfig,
                        sizeof(getPEFConfig.response.parameterUnion.pefControl));

    returnCode = CommandCheckGoodCompletion(&getPEFConfig.header);
    if (returnCode != GOOD)
    {
        dprintf(DPRINTF_IPMI, "ERROR reading PEF control\n");
        return returnCode;
    }

    if (!getPEFConfig.response.parameterUnion.pefControl.bits.pefEnabled)
    {
        dprintf(DPRINTF_IPMI, "PEF alerts already disabled\n");
        return returnCode;
    }

    dprintf(DPRINTF_DEFAULT, "Disable BMC automatic (PEF) alerts\n");

    /* Parameter 0 - Set In Progress - Begin changes */
    memset(&setPEFConfig, 0, sizeof(setPEFConfig));
    setPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_SET_IN_PROGRESS;
    setPEFConfig.request.parameterUnion.setInProgress.actionFlag = PEFCP_ACTION_FLAG_IN_PROGRESS;

    CommandSetPEFConfig(pInterface, &setPEFConfig,
                        sizeof(setPEFConfig.request.parameterSelector) +
                        sizeof(setPEFConfig.request.parameterUnion.setInProgress));

    returnCode = CommandCheckGoodCompletion(&setPEFConfig.header);
    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 1 - Modify */
    memset(&setPEFConfig, 0, sizeof(setPEFConfig));
    setPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_PEF_CONTROL;
    setPEFConfig.request.parameterUnion.pefControl.value =
        getPEFConfig.response.parameterUnion.pefControl.value;
    setPEFConfig.request.parameterUnion.pefControl.bits.pefEnabled = 0;

    CommandSetPEFConfig(pInterface, &setPEFConfig,
                        sizeof(setPEFConfig.request.parameterSelector) +
                        sizeof(setPEFConfig.request.parameterUnion.pefControl));

    returnCode = CommandCheckGoodCompletion(&setPEFConfig.header);
    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /*
     * Parameter 0 - Set In Progress - Commit
     * NOTE: This will fail on systems that don't support rollback,
     *       so don't alter the return code for this command.
     */
    memset(&setPEFConfig, 0, sizeof(setPEFConfig));
    setPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_SET_IN_PROGRESS;
    setPEFConfig.request.parameterUnion.setInProgress.actionFlag = PEFCP_ACTION_FLAG_COMMIT_WRITE;

    CommandSetPEFConfig(pInterface, &setPEFConfig,
                        sizeof(setPEFConfig.request.parameterSelector) +
                        sizeof(setPEFConfig.request.parameterUnion.setInProgress));

    /* Parameter 0 - Set In Progress - Complete */
    memset(&setPEFConfig, 0, sizeof(setPEFConfig));
    setPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_SET_IN_PROGRESS;
    setPEFConfig.request.parameterUnion.setInProgress.actionFlag = PEFCP_ACTION_FLAG_COMPLETE;

    CommandSetPEFConfig(pInterface, &setPEFConfig,
                        sizeof(setPEFConfig.request.parameterSelector) +
                        sizeof(setPEFConfig.request.parameterUnion.setInProgress));

    returnCode = CommandCheckGoodCompletion(&setPEFConfig.header);
    if (returnCode != GOOD)
    {
        return returnCode;
    }

    /* Parameter 1 - Verify */
    memset(&getPEFConfig, 0, sizeof(getPEFConfig));
    getPEFConfig.request.parameterSelector.parameter = PEFCP_PARAMETER_PEF_CONTROL;

    CommandGetPEFConfig(pInterface, &getPEFConfig,
                        sizeof(getPEFConfig.response.parameterUnion.pefControl));

    returnCode = CommandCheckGoodCompletion(&getPEFConfig.header);

    /* Verify that the PEF alerts are, in fact, disabled */

    if (returnCode == GOOD &&
        getPEFConfig.response.parameterUnion.pefControl.bits.pefEnabled)
    {
        dprintf(DPRINTF_DEFAULT, "Unable to disable PEF alerts\n");
        return ERROR;
    }

    return returnCode;
}


/**
******************************************************************************
**
**  @brief      Disable the inbound LAN access to the BMC
**
**  @param      Pointer to a BMC
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 BMCDisableLANAccess(IPMI_BMC *pBMC)
{
    IPMI_INTERFACE *pInterface;
    COMMAND_GET_CHANNEL_ACCESS getChannelAccess;        /* Uninitialized */
    COMMAND_SET_CHANNEL_ACCESS setChannelAccess;        /* Uninitialized */

    pInterface = IPMI_BMCGetInterface(pBMC);
    if (!pInterface)
    {
        return ERROR;
    }

    /* Scan through and disable all BMC channels */

    memset(&getChannelAccess, 0, sizeof(getChannelAccess));
    getChannelAccess.request.channel.bits.channelNumber = 0x01;
    getChannelAccess.request.setting.bits.changeType = CHANGE_TYPE_NON_VOLATILE;
    CommandGetChannelAccess(pInterface, &getChannelAccess);

    if (CommandCheckGoodCompletion(&getChannelAccess.header) != GOOD)
    {
        return ERROR;
    }

    /* If the channel is not disabled, issue the command to disable it */

    if (getChannelAccess.response.access.bits.accessMode == ACCESS_MODE_DISABLED)
    {
        dprintf(DPRINTF_IPMI, "BMC LAN channel already disabled\n");

        return GOOD;
    }

    dprintf(DPRINTF_IPMI, "Disabling BMC LAN channel\n");

    memset(&setChannelAccess, 0, sizeof(setChannelAccess));
    setChannelAccess.request.channel.bits.channelNumber = 0x01;
    setChannelAccess.request.access.bits.accessMode = ACCESS_MODE_DISABLED;
    setChannelAccess.request.access.bits.pefAlertingDisable = TRUE;
    setChannelAccess.request.access.bits.changeType = CHANGE_TYPE_NON_VOLATILE;
    setChannelAccess.request.privilege.bits.levelLimit = LEVEL_LIMIT_ADMINISTRATOR;
    CommandSetChannelAccess(pInterface, &setChannelAccess);

    return CommandCheckGoodCompletion(&setChannelAccess.header);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
