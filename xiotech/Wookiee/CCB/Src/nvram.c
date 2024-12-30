/* $Id: nvram.c 161210 2013-06-05 19:56:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   nvram.c
**
**  @brief  NVRAM data management for the CCB
**
**  This code will manage and check the NVRAM structures
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "nvram.h"

#include "AsyncEventHandler.h"
#include "ccb_flash.h"
#include "ccb_statistics.h"
#include "crc32.h"
#include "debug_files.h"
#include "EL.h"
#include "error_handler.h"
#include "errorCodes.h"
#include "FIO.h"
#include "i82559.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "mutex.h"
#include "names.h"
#include "nvram_structure.h"
#include "PI_Utils.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "rtc.h"
#include "SerBuff.h"
#include "serial_num.h"
#include "trace.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include "HWM.h"
#include <unistd.h>
#include <arpa/inet.h>
#include "xk_mapmemfile.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define MRP_INIT_REQUEST_FLAG       0x80000000

/*****************************************************************************
** Private variables
*****************************************************************************/

static const PORT_CONFIG def_port_config =
{
#ifndef S_SPLINT_S
#if defined(MODEL_7000) || defined(MODEL_4700)
    .be = { 4, { [0 ... 3] = ISP_CONFIG_4 } },
#else  /* MODEL_7000 || MODEL_4700 */
    .be = { 4, { [0 ... 3] = ISP_CONFIG_AUTO } },
#endif /* MODEL_7000 || MODEL_4700 */
    .fe = { 4, { [0 ... 3] = ISP_CONFIG_AUTO } },
#else  /* S_SPLINT_S */
    0
#endif /* S_SPLINT_S */
};

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
CONTROLLER_SETUP cntlSetup LOCATE_IN_SHMEM;     /* Controller Setup Information     */
MUTEX       backtraceMutex;

/*****************************************************************************
** Public routines - NOT externed in any header file
*****************************************************************************/
extern void DisplayAddress(const char *name, const UINT32 addr);
extern INT32 NetworkSetup(void);
extern void SetConfigEthernet(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void NVRAMLogErrTask(TASK_PARMS *parms);
static UINT32 MemCpyNVRAMBytes(void *dest, void *src, UINT32 length);

/*****************************************************************************
** Code Start
*****************************************************************************/

void StoreMasterConfigToNVRAM(void)
{
    char       *pScratchDRAM;
    char       *pScratchNVRAM;

    dprintf(DPRINTF_DEFAULT, "%s\n", __func__);

    pScratchDRAM = (char *)&masterConfig;
    pScratchNVRAM = (char *)&NVRAMData.masterConfigRecord;

    /* Recompute CRC */
    Qm_SetCRC(CRC32(pScratchDRAM, sizeof(masterConfig) - sizeof(unsigned int)));

    /* Copy from DRAM to NVRAM */
    memcpy(pScratchNVRAM, pScratchDRAM, sizeof(masterConfig));

    /* Flush the NVRAM */
    MEM_FlushMapFile(pScratchNVRAM, sizeof(masterConfig));
}


/*
 * To send a log message during the initialization process, we need
 * to fork a task to do it so that initialization can continue.  If we
 * don't, then we get deadlocked in the logging code, where we are trying
 * to email out the error message, but the email sub-system isn't ready
 * yet.  It never will be ready, unless initialization is allowed to
 * continue.
 */
void NVRAMLogErrTask(TASK_PARMS *parms)
{
    int         eventCode = (int)parms->p1;

    /*
     * Send a log event
     */
    SendAsyncEvent(eventCode, 0, NULL);
}


void LoadMasterConfigFromNVRAM(void)
{
    char       *pScratchDRAM;
    char       *pScratchNVRAM;
    unsigned long crc;
    TASK_PARMS  parms;

    dprintf(DPRINTF_DEFAULT, "%s\n", __func__);

    pScratchDRAM = (char *)&masterConfig;
    pScratchNVRAM = (char *)&NVRAMData.masterConfigRecord;

    /* Copy from NVRAM into DRAM */
    memcpy(pScratchDRAM, pScratchNVRAM, sizeof(masterConfig));

    /* Confirm that this is a good NVRAM image. Clean it up if it isn't. */

    /* Get the CRC */
    crc = CRC32(pScratchDRAM, sizeof(masterConfig) - sizeof(unsigned long));

    /*
     * If something is wrong with this NVRAM image, reset the NVRAM to defaults
     */
    if (crc != Qm_GetCRC() ||
        Qm_GetMagicNumber() != NVRAM_MAGIC_NUMBER || Qm_GetSchema() != SCHEMA)
    {
        ResetMasterConfigNVRAM();
        return;
    }

    /*
     * Send a log event
     */
    parms.p1 = (UINT32)LOG_CCB_NVRAM_RESTORED;
    TaskCreate(NVRAMLogErrTask, &parms);
}


/**********************************************************************
*                                                                     *
*  Name:        ResetMasterConfigNVRAM()                              *
*                                                                     *
*  Description: Reset the CCB NVRAM to default values.                *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void ResetMasterConfigNVRAM(void)
{
    TASK_PARMS  parms;

    dprintf(DPRINTF_DEFAULT, "ResetMasterConfigNVRAM\n");

    /*
     * Send a log event
     */
    parms.p1 = (UINT32)LOG_CCB_NVRAM_RESET;
    TaskCreate(NVRAMLogErrTask, &parms);

    /*
     * Clear out the entire Master Config area first
     */
    memset(&masterConfig, 0, sizeof(masterConfig));

    Qm_SetSchema(SCHEMA);

    /*
     * Empty the active controller map (and padding) and the controller
     * configuration map
     */
    memset(&masterConfig.activeControllerMap,
           ACM_NODE_UNDEFINED, sizeof(masterConfig.activeControllerMap));

    Qm_SetMagicNumber(NVRAM_MAGIC_NUMBER);

    /*
     * Update the actual NVRAM
     */
    StoreMasterConfigToNVRAM();
}


/**
******************************************************************************
**
**  @brief      NVRAM_DisasterDataLoad
**
**              Loads the copy of the NVRAM data into the address pointed
**              to by the input parameter.  If the CRC on the NVRAM copy
**              fails, then defaults will be loaded into both the NVRAM
**              copy and the input pointer addresses.
**
**  @param      disasterDataPtr - pointer to where data should be loaded
**
**  @return     GOOD or ERROR
**
**  @attention  Defaults loaded on ERROR return (assuming non-NULL input)
**
******************************************************************************
**/
UINT32 NVRAM_DisasterDataLoad(DISASTER_DATA *disasterDataPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      calculatedCRC = 0;

    if (disasterDataPtr != NULL)
    {
        calculatedCRC = CRC32((UINT8 *)&NVRAMData.disasterData,
                              sizeof(*disasterDataPtr) - sizeof(UINT32));

        /*
         * Validate the CRCs.  Schema is not checked at the moment, since
         * there's only one version.  Any schema conversion routine would
         * be called inside the 'valid CRC' statement below.
         */
        if (calculatedCRC == NVRAMData.disasterData.crc)
        {
            /*
             * Copy the new disasterData structure from NVRAM into DRAM
             */
            MemCpyBytes(disasterDataPtr, &NVRAMData.disasterData,
                        sizeof(*disasterDataPtr));

            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DDL: Bad CRC - Calling DisasterDataReset\n");
            NVRAM_DisasterDataReset(disasterDataPtr);
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      NVRAM_DisasterDataSave
**
**              Stores the copy of the disaster data pointed to by the
**              input parameter into NVRAM.  This function will recalculate
**              the disaster data CRC before storing it into NVRAM.
**
**  @param      disasterDataPtr - pointer to data which is to be stored
**
**  @return     GOOD or ERROR
**
**  @attention  Not saved on ERROR return
**
******************************************************************************
**/
UINT32 NVRAM_DisasterDataSave(DISASTER_DATA *disasterDataPtr)
{
    UINT32      returnCode = ERROR;

    if (disasterDataPtr != NULL)
    {
        /*
         * Force the schema to current version
         */
        disasterDataPtr->schema = DISASTER_DATA_SCHEMA;

        /*
         * Calculate the new CRC
         */
        disasterDataPtr->crc = CRC32((UINT8 *)disasterDataPtr,
                                     sizeof(*disasterDataPtr) - sizeof(UINT32));

        /*
         * Copy the new disasterData structure into NVRAM
         */
        MemCpyNVRAMBytes(&NVRAMData.disasterData, disasterDataPtr,
                         sizeof(*disasterDataPtr));


        returnCode = GOOD;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      NVRAM_DisasterDataReset
**
**              Resets the disaster data pointed to by the input parameter.
**              This function will recalculate the disaster data CRC.
**
**  @param      disasterDataPtr - pointer to data which is to be reset
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 NVRAM_DisasterDataReset(DISASTER_DATA *disasterDataPtr)
{
    UINT32      returnCode = ERROR;

    if (disasterDataPtr != NULL)
    {
        dprintf(DPRINTF_ELECTION, "DisasterDataReset\n");

        /*
         * Clear out the entire disasterData structure first
         */
        memset(disasterDataPtr, 0, sizeof(*disasterDataPtr));

        /*
         * Set the default values, for those that are possibly nonzero
         */
        disasterDataPtr->schema = DISASTER_DATA_SCHEMA;
        disasterDataPtr->flags.bits.disasterDetected = FALSE;

        /*
         * Write the new data back into NVRAM (this also calculates the CRC)
         */
        returnCode = NVRAM_DisasterDataSave(disasterDataPtr);
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
** Function:    DisplayAddress
**
** Inputs:      name & addr to display
**
** Returns:     void
**--------------------------------------------------------------------------*/
void DisplayAddress(const char *name, const UINT32 addr)
{
    char        buf[20];

    InetToAscii(addr, buf);
    dprintf(DPRINTF_DEFAULT, "%s = %s\n", name, buf);
}

/*----------------------------------------------------------------------------
** Function:    NetworkSetup
**
** Inputs:      NONE
**
**--------------------------------------------------------------------------*/
#define SETUP_REQUIRED  98
INT32 NetworkSetup(void)
{
    INT32       rc = ERROR;

    UINT32      nvIP = GetIPAddress();
    UINT32      nvSN = GetSubnetMask();
    UINT32      nvGW = GetGatewayAddress();

    UINT32      sysIP = GetIpAddressFromInterface(ethernetDriver.interfaceHandle);
    UINT32      sysSN = GetSubnetFromInterface(ethernetDriver.interfaceHandle);
    UINT32      sysGW = GetGatewayFromInterface(ethernetDriver.interfaceHandle);

    UINT32      nvOK = 0;
    UINT32      sysOK = 0;

    /*
     * Check NVRAM values for nonzero and that gateway is reachable
     * with given subnet mask.
     */
    if ((nvIP && nvSN && nvGW && ((nvGW & nvSN) == (nvIP & nvSN))) ||
        (nvIP && nvSN && nvGW == 0))
    {
        nvOK = 1;
    }

    /*
     * Check current system values for nonzero and that gateway is reachable
     * with given subnet mask.
     */
    if ((sysIP && sysSN && sysGW && ((sysGW & sysSN) == (sysIP & sysSN))) ||
        (sysIP && sysSN && sysGW == 0))
    {
        sysOK = 1;
    }

    /*
     * Show what we know
     */
    dprintf(DPRINTF_DEFAULT, "NetworkSetup: Addresses:\n");
    DisplayAddress("  nvIP ", nvIP);
    DisplayAddress("  sysIP", sysIP);
    DisplayAddress("  nvSN ", nvSN);
    DisplayAddress("  sysSN", sysSN);
    DisplayAddress("  nvGW ", nvGW);
    DisplayAddress("  sysGW", sysGW);
    dprintf(DPRINTF_DEFAULT, "  NVRAM Addresses %s\n", nvOK ? "OK" : "BAD");
    dprintf(DPRINTF_DEFAULT, "  System Addresses %s\n", sysOK ? "OK" : "BAD");

    if (!nvOK)
    {
        /*
         * No matter the current state of the NVRAM there should always be
         * system settings for the network information.
         */
        ccb_assert(sysOK == 1, sysOK);

        /*
         * System is powered up with system network addresses and CCB NVRAM
         * contains no network addresses (Not Setup).
         */
        SetIPAddress(sysIP);
        SetSubnetMask(sysSN);
        SetGatewayAddress(sysGW);
        SetConfigEthernet();

        dprintf(DPRINTF_DEFAULT, "NetworkSetup: NVRAM set to System values\n");

        /*
         * The default system configuration is considered a good network
         * configurations since we have default IP addresses.
         */
        rc = GOOD;
    }
    else                        /* nvOK */
    {
        /*
         * System is powered up with or without system network addresses and
         * CCB NVRAM contains network addresses, and the addresses do not
         * match (Setup).
         */
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
        if (sysIP != nvIP || sysSN != nvSN || sysGW != nvGW)
        {
            INT32       rc2;

            /*
             * CCB will correct the network mismatch replacing the system
             * network addresses with what is stored in NVRAM.  CCB will
             * reboot the system.
             */

            dprintf(DPRINTF_DEFAULT, "NetworkSetup: Attempting to set System values to NVRAM values\n");
            rc2 = UpdateIfcfgScript(nvIP, nvSN, nvGW);
            if (rc2)
            {
                dprintf(DPRINTF_DEFAULT, "NetworkSetup: Failed to set System values to NVRAM values, rc %d\n",
                        rc2);
                rc = ERROR;
            }
            else
            {
                dprintf(DPRINTF_DEFAULT, "NetworkSetup: System values successfully set to NVRAM values\n");
                rc = GOOD;
            }
        }
        /*
         * System is powered up with system network addresses and CCB NVRAM
         * contains network addresses, and the addresses match (Setup).
         */
        else                    /* sysIP == nvIP && sysSN == nvSN && sysGW == nvGW */
#endif /* FORCE_IP_CHANGE */
        {
            /*
             * Nothing to do!!
             */
            dprintf(DPRINTF_DEFAULT, "NetworkSetup: Nothing to do!\n");
            rc = GOOD;
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    UpdateIfcfgScript
**--------------------------------------------------------------------------*/
INT32 UpdateIfcfgScript(UINT32 ipAddr, UINT32 snMask, UINT32 gwAddr)
{
    char        ip[20];
    char        sn[20];
    char        gw[20];
    char        cmdBuf[256];    /* If you change string below, might need to resize! */
    char        fName[128];     /* If you change string below, might need to resize! */
    ETHERNET_MAC_ADDRESS mac;
    INT32       rc = FAIL;

    /*
     * Try to find the 'eth0' config file
     */
    sprintf(fName, "/etc/sysconfig/network/ifcfg-%s",
            (char *)ethernetDriver.interfaceHandle);

    /*
     * If we can't find the 'eth0' file, try the MAC addr named file
     */
    if (access(fName, F_OK))
    {
        /*
         * Go get the MAC address so we can find the congfiguration file
         */
        mac = GetMacAddrFromInterface(ethernetDriver.interfaceHandle);
        sprintf(fName,
                "/etc/sysconfig/network/ifcfg-eth-id-%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                mac.macByte[0], mac.macByte[1], mac.macByte[2],
                mac.macByte[3], mac.macByte[4], mac.macByte[5]);
    }

    if (!access(fName, F_OK))
    {
        /*
         * Convert all of the IP-type addresses to x.x.x.x form
         */
        InetToAscii(ipAddr, ip);
        InetToAscii(snMask, sn);
        InetToAscii(gwAddr, gw);

        /*
         * Change the network configuration scripts.
         */
        sprintf(cmdBuf, "/usr/bin/perl -U ./chgnetcfg -r "   /* restart the network */
                "-v "           /* verbose mode */
                "%s %s %s %s /etc/sysconfig/network/routes", ip, sn, gw, fName);

        rc = XK_System(cmdBuf);

#ifdef ENABLE_SLP
        /*
         * If SLP is enabled on the system, we need to
         * restart it to pick up the new ip addresses.
         */
        XK_System("/etc/init.d/slpd restart &>/tmp/slpd_restart_stat");
#endif  /* ENABLE_SLP */
    }

    /*
     * Reconfigure the BMC's Ethernet parameters to the same as the controller.
     * NOTE: Don't change rc, since the IPMI interface might not be ready.
     */
    if (rc == 0)
    {
        HWM_ConfigureEthernet();
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    IsControllerSetup
**
** Description: Checks if the controller has been setup.  In order to be
**              considered as setup the following values in the controller
**              setup (cntlSetup) must be non-zero:
**                  - IP Address
**                  - Subnet Mask
**                  - System Serial Number
**                  - Controller Serial Number
**
** Inputs:      NONE
**
** Returns:     true if the controller setup is complete, false otherwise.
**--------------------------------------------------------------------------*/
bool IsControllerSetup(void)
{
    return (CntlSetup_GetIPAddress() != 0 &&
            CntlSetup_GetSubnetMask() != 0 &&
            CntlSetup_GetSystemSN() != 0 && CntlSetup_GetControllerSN() != 0);
}


/*
******************************************************************************
**
**  @brief  Send FC port config to proc
**
**  @param  proc    - Processor ID (PROCESS_BE or PROCESS_FE)
**
**  @return none
**
******************************************************************************
*/
void SendPortConfig(UINT32 proc)
{
    UINT32      cmd;
    UINT32      rc;
    const char *pname;
    MRCONFIG_REQ *req;
    const ISP_CONFIG *cfg;
    MR_GENERIC_RSP *rsp;

    switch (proc)
    {
        case PROCESS_BE:
            cmd = MRSETBEPORTCONFIG;
            cfg = &cntlSetup.config.be;
            pname = "BE";
            break;

        case PROCESS_FE:
            cmd = MRSETFEPORTCONFIG;
            cfg = &cntlSetup.config.fe;
            pname = "FE";
            break;

        default:
            fprintf(stderr, "%s: Unknown processor %d\n", __func__, proc);
            return;
    }

    req = MallocWC(sizeof(*req));
    req->config = *cfg;

    rsp = MallocSharedWC(sizeof(*rsp));

    LogMessage(LOG_TYPE_INFO, "%s port config set #%d - %02X %02X %02X %02X",
               pname, cfg->count,
               cfg->config[0], cfg->config[1], cfg->config[2], cfg->config[3]);
    rc = PI_ExecMRP(req, sizeof(*req), cmd, rsp, sizeof(*rsp),
                    GetGlobalMRPTimeout() | MRP_INIT_REQUEST_FLAG);
    fprintf(stderr, "%s: Got response %d status %d from %s\n", __func__,
            rc, rsp->header.status, pname);

    Free(req);

    if (rc != PI_TIMEOUT)
    {
        Free(rsp);
    }
}


/*----------------------------------------------------------------------------
** Function:    WaitForControllerSetup
**
** Description: Waits in a loop until the basic controller setup
**              information has been loaded by the user through the
**              serial console.  This information includes the IP
**              address, CNC ID and controller slot number.
**
** Inputs:      NONE
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void WaitForControllerSetup(void)
{
    UINT32      sserial = 0;
    UINT32      cserial = 0;
    UINT16      rc = PI_GOOD;
    INT32       networkSetupRC;
    char        msg[] = { "\n\n! CONTROLLER CONFIGURATION REQUIRED !\n\n" };

    dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - ENTER\n");

    /*
     * If there is BE processor communication and the serial
     * numbers for both system and controller are zero, we
     * need to check if the PROC has values for these and
     * update the CCB values.
     */
    if (ProcessorCommReady(PROCESS_BE) &&
        CntlSetup_GetSystemSN() == 0 && CntlSetup_GetControllerSN() == 0)
    {
        dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Controller setup serial numbers need to be updated...\n");

        rc = GetProcSerialNumbers(&sserial, &cserial);

        if (rc == PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Retrieved PROC serial numbers (0x%x, 0x%x)\n",
                    sserial, cserial);

            CntlSetup_SetSystemSN(sserial);
            CntlSetup_SetControllerSN(cserial);

            if (sserial > 0 && cserial > 0)
            {
                SetLicenseApplied();
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Failed to get the PROC serial numbers.\n");
        }
    }

    /*
     * Go through the network configuration stuff for HN.
     */
    while (1)
    {
        networkSetupRC = NetworkSetup();

        if (networkSetupRC == SETUP_REQUIRED || CntlSetup_GetControllerSN() == 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "Waiting for serial port controller setup.");
        }
        else if (networkSetupRC == ERROR)
        {
            /*
             * We got here because the UpdateIfcfgScript() call failed for
             * some reason. Shouldn't happen... If it keeps failing after
             * repeated attempts, at least the controller is up so that
             * someone can log into the console to service it.
             */
            LogMessage(LOG_TYPE_DEBUG, "NETWORK SETUP ERROR! Forcing serial port setup.");

            /*
             * Set nvram values to zero.
             */
            SetIPAddress(0);
            SetSubnetMask(0);
            SetGatewayAddress(0);
        }
        else
        {
            break;
        }

        /* Loop until the controller is configured. */

        SerialBufferedWriteString(msg, strlen(msg));
        SerialBufferedWriteFlush(TRUE);

        while (!IsControllerSetup())
        {
            TaskSleepMS(1000);
        }

        LogMessage(LOG_TYPE_DEBUG, "Controller setup complete.");
    }

    if (ProcessorCommReady(PROCESS_BE))
    {
        SendPortConfig(PROCESS_BE);
    }

    if (ProcessorCommReady(PROCESS_FE))
    {
        SendPortConfig(PROCESS_FE);
    }

    /*
     * If there is BE processor communication attempt to update
     * the serial numbers (if necessary) and update the CNC name
     * (if necessary).
     */
    if (ProcessorCommReady(PROCESS_BE))
    {
        dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Checking if PROC serial numbers need to be updated.\n");

        /*
         * If the controller was "wiped_clean" then the serial numbers
         * in the controller setup could be non-zero and the PROC values
         * could be zero.  If this is the case we want to update the
         * PROC values.
         */
        rc = GetProcSerialNumbers(&sserial, &cserial);

        if (rc == PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Retrieved PROC serial numbers (0x%x, 0x%x)\n",
                    sserial, cserial);

            if (sserial == 0 || cserial == 0)
            {
                dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Updating PROC serial numbers.\n");

                UpdateProcSerialNumber(CONTROLLER_SN, CntlSetup_GetControllerSN());
            }
            else
            {
                /*
                 * Check if the system serial numbers for the PROC
                 * and CCB match.
                 */
                if (CntlSetup_GetSystemSN() != sserial)
                {
                    dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Mismatched system serial number (CCB: 0x%x, PROC: 0x%x).\n",
                            CntlSetup_GetSystemSN(), sserial);
                }

                /*
                 * Check if the controller serial numbers for the PROC
                 * and CCB match.
                 */
                if (CntlSetup_GetControllerSN() != cserial)
                {
                    dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Mismatched controller serial number (CCB: 0x%x, PROC: 0x%x).\n",
                            CntlSetup_GetControllerSN(), cserial);
                }
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Failed to get the PROC serial numbers.\n");
        }

        dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - Checking if CNC name needs to be updated.\n");

        /*
         * Update the CNC name if it is not set.
         */
        if (!ControllerNodeCluster_IsNameSet())
        {
            /*
             * Update the controller node cluster name with the
             * default value of CNCnnnnnnnnnn.
             */
            ControllerNodeCluster_SetDefaultName();
        }
    }

    dprintf(DPRINTF_DEFAULT, "WaitForControllerSetup - EXIT\n");
}


/**
******************************************************************************
**
**  @brief  Load controller configuration from NVRAM
**
**  Load the controller configuration information from NVRAM to a dram copy.
**  If the information does not pass the checksum, load default values.
**
**  @param  none
**
**  @return none
**
******************************************************************************
*/
void LoadControllerSetup(void)
{
    UINT32      crc;

    /* Copy structure from NVRAM to DRAM */

    cntlSetup = NVRAMData.cntlSetup;

    /* Compute checksum on current data */

    crc = CRC32((UINT8 *)&cntlSetup, sizeof(cntlSetup) - sizeof(UINT32));

    /* If the NVRAM schema isn't set, set it now */

    if (NVRAMData.schema == 0)
    {
        /*
         * Set the new CCB NVRAM schema field.  The next time we change
         * the NVRAM layout, we can check for this schema change.
         * This word is not CRC'd (probably should be), so there is no CRC
         * to update along with this change.
         */
        NVRAMData.schema = CCB_NVRAM_SCHEMA;
    }

    if (cntlSetup.config.be.count > ISP_MAX_CONFIG_PORTS)
    {
        cntlSetup.config.be = def_port_config.be;
        SaveControllerSetup();
    }
    if (cntlSetup.config.fe.count > ISP_MAX_CONFIG_PORTS)
    {
        cntlSetup.config.fe = def_port_config.fe;
        SaveControllerSetup();
    }

    /* If the checksum fails, load defaults and save back to NVRAM */

    if (crc != cntlSetup.crc || cntlSetup.schema != CONTROLLER_SETUP_SCHEMA)
    {
        memset(&cntlSetup, 0, sizeof(cntlSetup));

        cntlSetup.schema = CONTROLLER_SETUP_SCHEMA;
        cntlSetup.ipAddress = inet_addr("0.0.0.0");
        cntlSetup.gatewayAddress = inet_addr("0.0.0.0");
        cntlSetup.subnetMask = inet_addr("0.0.0.0");

        cntlSetup.systemSN = 0;
        cntlSetup.controllerSN = 0;

        cntlSetup.useDHCP = FALSE;
        cntlSetup.ethernetConfigured = FALSE;

        memset(&cntlSetup.rsvd1, 0xFF, sizeof(cntlSetup.rsvd1));
        cntlSetup.config = def_port_config;
        memset(&cntlSetup.rsvd2, 0xFF, sizeof(cntlSetup.rsvd2));
        memset(&cntlSetup.rsvd3, 0xFF, sizeof(cntlSetup.rsvd3));
        memset(&cntlSetup.reserved, 0xFF, sizeof(cntlSetup.reserved));

        SaveControllerSetup();
    }
}

/*----------------------------------------------------------------------------
**  Function Name: SaveControllerSetup
**
**  Description:
**      Save the controller configuration information from dram to a NVRAM copy.
**      The checksum for this data is computed and saved.
**
**  Inputs:   none
**
**--------------------------------------------------------------------------*/
void SaveControllerSetup(void)
{
    /*
     * Only update if the schema matches the expected schema (sanity
     * check on the data)
     */
    if (cntlSetup.schema != CONTROLLER_SETUP_SCHEMA)
    {
        return;
    }

    memset(&cntlSetup.rsvd1, 0xFF, sizeof(cntlSetup.rsvd1));
    memset(&cntlSetup.rsvd2, 0xFF, sizeof(cntlSetup.rsvd2));
    memset(&cntlSetup.rsvd3, 0xFF, sizeof(cntlSetup.rsvd3));
    memset(&cntlSetup.reserved, 0xFF, sizeof(cntlSetup.reserved));

    /* Calculate the checksum and save back to NVRAM */

    cntlSetup.crc = CRC32((UINT8 *)&cntlSetup, sizeof(cntlSetup) - sizeof(unsigned long));

    /* Copy structure from DRAM to NVRAM */

    NVRAMData.cntlSetup = cntlSetup;

    /* Flush the NVRAM */

    MEM_FlushMapFile(&NVRAMData.cntlSetup, sizeof(NVRAMData.cntlSetup));
    dprintf(DPRINTF_DEFAULT, "Flushing Controller setup NVRAM\n");
}


/*----------------------------------------------------------------------------
**  Function Name: StrNCpyBytes()
**
**  Description:   Byte copy string data to NVRAM (or anywhere else)
**
**  Inputs:        dest - destination pointer
**                 src - source pointer
**                 length - number of bytes to copy
**
**  Returns:       number of bytes written
**--------------------------------------------------------------------------*/
static UINT32 StrNCpyBytes(void *dest, void *src, UINT32 length)
{
    UINT32      i;

    for (i = 0; i < length; i++)
    {
        if (((UINT8 *)src)[i] == 0)
        {
            ((UINT8 *)dest)[i] = 0;
            break;
        }

        ((UINT8 *)dest)[i] = ((UINT8 *)src)[i];
    }

    return i;
}

/*----------------------------------------------------------------------------
**  Function Name: MemSetBytes()
**
**  Description:   Byte set data in NVRAM (or anywhere else)
**
**  Inputs:        dest - destination pointer
**                 val - data value to set to
**                 length - number of bytes to copy
**
**  Returns:       number of bytes written
**--------------------------------------------------------------------------*/
UINT32 MemSetBytes(void *dest, UINT8 val, UINT32 length)
{
    UINT32      i;

    for (i = 0; i < length; i++)
    {
        ((UINT8 *)dest)[i] = val;
    }

    return length;
}

/*----------------------------------------------------------------------------
**  Function Name: MemCpyBytes()
**
**  Description:   Byte copy data to NVRAM (or anywhere else)
**
**  Inputs:        dest - destination pointer
**                 src - source pointer
**                 length - number of bytes to copy
**
**  Returns:       number of bytes written
**--------------------------------------------------------------------------*/
UINT32 MemCpyBytes(void *dest, void *src, UINT32 length)
{
    UINT32      i;

    for (i = 0; i < length; i++)
    {
        ((UINT8 *)dest)[i] = ((UINT8 *)src)[i];
    }

    return length;
}

/*----------------------------------------------------------------------------
**  Function Name: MemSetNVRAMBytes()
**
**  Description:   Byte set data in NVRAM and sunc contents of NVRAM
**
**  Inputs:        dest - destination pointer
**                 val - data value to set to
**                 length - number of bytes to copy
**
**  Returns:       number of bytes written
**--------------------------------------------------------------------------*/
UINT32 MemSetNVRAMBytes(void *dest, UINT8 val, UINT32 length)
{
    UINT32      rc;

    rc = MemSetBytes(dest, val, length);

    MEM_FlushMapFile(dest, length);         /* Flush the NVRAM */

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: MemCpyVRAMBytes()
**
**  Description:   Byte copy data to NVRAM and sync contents of NVRAM
**
**  Inputs:        dest - destination pointer
**                 src - source pointer
**                 length - number of bytes to copy
**
**  Returns:       number of bytes written
**--------------------------------------------------------------------------*/
static UINT32 MemCpyNVRAMBytes(void *dest, void *src, UINT32 length)
{
    UINT32      rc;

    rc = MemCpyBytes(dest, src, length);

    MEM_FlushMapFile(dest, length);         /* Flush the NVRAM */

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: CopyBacktraceDataToNVRAM()
**
**  Description:   Copy critical failure data to NVRAM for later analysis.
**
**  Inputs:        none
**
**  Returns:       void
**--------------------------------------------------------------------------*/
void CopyBacktraceDataToNVRAM(void)
{
    UINT32      crcValue = 0;
    TIMESTAMP   ts;
    FW_DATA     fd;

    /*
     * Check that the mutex is locked before saving the backtrace to NVRAM.
     * If it is not lockable, then some other task is already doing the
     * copy, so skip this task's call to do the same.
     */

    if (LockMutex(&backtraceMutex, MUTEX_NOWAIT) == TRUE)
    {
        /*
         * Clear backtrace area in NVRAM
         * do not initialize counters, skip that area
         */
        MemSetBytes(&NVRAMData.errortrapDataRun.errorSnapshot,
                    0,
                    (sizeof(ERRORTRAP_DATA_RUN) - sizeof(ERRORTRAP_DATA_ERROR_COUNTERS)));

        /*
         * First get timestamp
         */
        RTC_GetTimeStamp(&ts);
        MemCpyBytes(&NVRAMData.errortrapDataRun.errorSnapshot.timestamp,
                    &ts, sizeof(NVRAMData.errortrapDataRun.errorSnapshot.timestamp));

        /*
         * Get fw header information
         */
        fd.revision = fwHeader.revision;
        fd.revCount = fwHeader.revCount;
        fd.buildID = fwHeader.buildID;
        memcpy(&fd.timeStamp, &fwHeader.timeStamp, sizeof(fd.timeStamp));

        MemCpyBytes(&NVRAMData.errortrapDataRun.errorSnapshot.firmwareRevisionData,
                    &fd,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.firmwareRevisionData));

        /*
         * Copy i960 registers
         */
        /* 8 bit registers */
        MemCpyBytes(&NVRAMData.errortrapDataRun.errorSnapshot.traceRegisters.statusGpodReg,
                    (UINT8 *)&GPOD,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.traceRegisters.statusGpodReg));

        /*
         * Copy trace event data to NVRAM
         */
        CopyTraceDataToNVRAM((UINT8 *)NVRAMData.errortrapDataRun.traceEvData,
                             sizeof(NVRAMData.errortrapDataRun.traceEvData));

        /*
         * Unwind the call stack.
         * stackDmp is defined in i82559.c -- 4096 bytes.
         */
        UnwindStackCCB(NULL, stackDmp);

        /*
         * Terminate stackDmp at 1K.
         */
        stackDmp[CALLSTACK_SIZE - 1] = 0;
        StrNCpyBytes(NVRAMData.errortrapDataRun.errorSnapshot.callStack,
                     stackDmp,
                     sizeof(NVRAMData.errortrapDataRun.errorSnapshot.callStack));

        /*
         * Copy register data to NVRAM.
         */
        MemCpyBytes(&NVRAMData.errortrapDataRun.errorSnapshot.cpuRegisters,
                    &cpuRegisters,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.cpuRegisters));

        /*
         * Copy MACH registers
         */
        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.systemStatus0,
                    (UINT8 *)&mach->systemStatus0,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.systemStatus0));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.diagSwitchesStatus,
                    (UINT8 *)&mach->diagSwitchesStatus,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.diagSwitchesStatus));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.flashSwitchesStatus,
                    (UINT8 *)&mach->flashSwitchesStatus,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.flashSwitchesStatus));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.boardMachRevStatus,
                    (UINT8 *)&mach->boardMachRevStatus,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.boardMachRevStatus));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.frontPanelControl,
                    (UINT8 *)&mach->frontPanelControl,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.frontPanelControl));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.miscControl,
                    (UINT8 *)&mach->miscControl,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.miscControl));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.heartbeatToggleControl,
                    (UINT8 *)&mach->heartbeatToggleControl,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.heartbeatToggleControl));

        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.watchDogReTriggerControl,
                    (UINT8 *)&mach->watchDogReTriggerControl,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.machRegisters.watchDogReTriggerControl));

        /*
         * Copy CCB Statistics structure
         */
        MemCpyBytes((UINT8 *)&NVRAMData.errortrapDataRun.ccbStatistics,
                    (UINT8 *)&CCBStats, sizeof(CCB_STATS_STRUCTURE));

// This is no longer reasonable what-so-ever, but as it is in the NVRAM structure -- at least zero it.
        memset((UINT8 *)&NVRAMData.errortrapDataRun.heapStatsNV, 0, sizeof(HEAP_STATS_IN_NVRAM));

        /* Calculate and write CRC to validate snapshot data */
        crcValue = CRC32((UINT8 *)&NVRAMData.errortrapDataRun.errorSnapshot,
                         (sizeof(NVRAMData.errortrapDataRun.errorSnapshot) -
                          sizeof(NVRAMData.errortrapDataRun.errorSnapshot.snapshotDataCRC)));

        MemCpyBytes(&NVRAMData.errortrapDataRun.errorSnapshot.snapshotDataCRC,
                    &crcValue,
                    sizeof(NVRAMData.errortrapDataRun.errorSnapshot.snapshotDataCRC));

        UnlockMutex(&backtraceMutex);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "  Backtrace Mutex locked - skipping CopyBacktraceToNVRAM\n");
    }

    /* Flush the NVRAM */
    MEM_FlushMapFile(&NVRAMData.errortrapDataRun,
                sizeof(NVRAMData.errortrapDataRun));
}


/*----------------------------------------------------------------------------
**  Function Name: CopyNVRAMBacktraceDataToFlash()
**
**  Description:   Backs up NVRAM Backtrace data to flash.  Actually, copy all
**                 of CCB NVRAM to backup storage area in flash.
**
**  Inputs:        none
**
**  Returns:       void
**--------------------------------------------------------------------------*/
#define FLASH_BACKTRACE_COPY_SIZE  0x10000
void CopyNVRAMBacktraceDataToFlash(void)
{
    UINT32      backtraceNum;
    UINT32      flashAddr;
    UINT32      slotNum;
    UINT32      sector;
    INT32       rc = GOOD;
    NVRAM_STRUCTURE *pNvramInFlash = NULL;
    TIMESTAMP   ts1;
    TIMESTAMP   ts2;
    UINT32     *pAddr;
    UINT32     *pTop;
    UINT32      erase = 0;

    /*
     * Get the current backtrace sequence number.
     * We can read UINT32's from NVRAM, but can only write bytes.
     */
    backtraceNum = NVRAMData.miscData.backtraceNum;

    /*
     * Calculate current slot number
     */
    slotNum = backtraceNum % 8;

    /*
     * Calculate flash write address
     */
    flashAddr = BASE_FLASH_BACKTRACE_ADDRESS + (slotNum * FLASH_BACKTRACE_COPY_SIZE);

    /*
     * Check the backup dates (boot and runtime) in the saved copy in this slot.
     * If they match what is in NVRAM, we're done.
     */
    pNvramInFlash = (NVRAM_STRUCTURE *)flashAddr;

    MemCpyBytes(&ts1, &NVRAMData.errortrapDataRun.errorSnapshot.timestamp, sizeof(ts1));
    MemCpyBytes(&ts2, &NVRAMData.errortrapDataBoot.errorSnapshot.timestamp, sizeof(ts2));

    if (memcmp
        (&pNvramInFlash->errortrapDataRun.errorSnapshot.timestamp, &ts1, sizeof(ts1)) ||
        memcmp(&pNvramInFlash->errortrapDataBoot.errorSnapshot.timestamp, &ts2,
               sizeof(ts2)))
    {
        /*
         * Increment the the sequence number and save it back out. It doesn't really
         * matter what the initial value is, as long as we go sequentially from there.
         */
        backtraceNum++;
        MemCpyNVRAMBytes(&NVRAMData.miscData.backtraceNum, &backtraceNum, sizeof(UINT32));

        /*
         * Calculate new slot number
         */
        slotNum = backtraceNum % 8;

        /*
         * Calculate new flash write address
         */
        flashAddr = BASE_FLASH_BACKTRACE_ADDRESS + (slotNum * FLASH_BACKTRACE_COPY_SIZE);

        /*
         * Make sure the flash space that we will be writing is clean, else
         * we'll hang when we program it (not sure why). Erase it if not.
         * Always erase if slot == 0 or 4 (the start of a segment)
         */
        if (slotNum == 0 || slotNum == 4)
        {
            erase = 1;
        }
        else
        {
            pAddr = (UINT32 *)flashAddr;
            pTop = pAddr + FLASH_BACKTRACE_COPY_SIZE / 4;
            while (pAddr < pTop)
            {
                if (*pAddr++ != 0xFFFFFFFF)
                {
                    erase = 1;
                    break;
                }
            }
        }

        /*
         * Erase flash segment if it is not erased already.
         * If the erase fails, abort the copy.
         */
        if (erase)
        {
            rc = CCBFlashGetSectorFromAddress((CCB_FLASH *)flashAddr, &sector);
            if (rc == GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "  - Erasing flash sector %d\n", sector);
                rc = CCBFlashEraseSector(sector);
            }
        }

        /*
         * Copy NVRAM data out
         */
        if (rc == GOOD)
        {
            /*
             * This is OK to call with NVRAM address directly since NVRAM reads
             * are word wide.
             */
            dprintf(DPRINTF_DEFAULT, "  - Copying NVRAM seq %d to slot %d, 0x%08X\n",
                    backtraceNum, slotNum, flashAddr);

            /*
             * Ignore any errors on programming the flash, as the NVRAM
             * contents can change before we finish (ipcSequenceNumber)
             */
            CCBFlashProgramData((CCB_FLASH *)flashAddr, (CCB_FLASH *)&NVRAMData,
                                (FLASH_BACKTRACE_COPY_SIZE / 4));
        }

        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "  - FAIL: Unable to copy NVRAM data to flash\n");
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "  - Nothing to do, backup is up-to-date (seq %d in slot %d, 0x%08X)\n",
                backtraceNum, slotNum, flashAddr);
    }
}

/*----------------------------------------------------------------------------
** Function:    IsConfigured
**
** Description: Checks if the controller has been setup.  In order to be
**              considered as setup the following values in the controller
**              setup (cntlSetup) must be non-zero:
**                  - IP Address
**                  - Subnet Mask
**                  - System Serial Number
**                  - Controller Serial Number
**
** Inputs:      NONE
**
** Returns:     true if the controller setup is complete, false otherwise.
**--------------------------------------------------------------------------*/
bool IsConfigured(void)
{
    return (CntlSetup_GetSystemSN() != 0);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
