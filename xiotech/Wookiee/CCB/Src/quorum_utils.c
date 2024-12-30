/* $Id: quorum_utils.c 161210 2013-06-05 19:56:42Z marshall_midden $ */
/*============================================================================
** FILE NAME:       quorum_utils.c
** MODULE TITLE:    quorum access utilities implementation
**
** DESCRIPTION:     This file contains the implementation for a number a
**                  access routines related to quorum area data structures.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "quorum_utils.h"

#include "kernel.h"
#include "debug_files.h"
#include "nvram.h"
#include "XIO_Std.h"
#include "errorCodes.h"

/*****************************************************************************
** Private functions
*****************************************************************************/
static INT32 WriteFWCompatIndexWithRetries(FW_HEADER *ccbFwHdr);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: GetCommuicationsSlot
**
**  Description:
**      This function returns the communications slot index for the specified
**      controller.
**
**  Inputs:   controllerSN - Controller serial number
**
**  Returns:  commSlot - Communications slot index
**            MAX_CONTROLLERS - if controler S/N was not found
**
**--------------------------------------------------------------------------*/
UINT16 GetCommunicationsSlot(UINT32 controllerSN)
{
    int         i;

    /*
     *  Walk through the communications slots looking for a match to the
     *  requested serial number. If no match is found, return the max number
     *  of controllers.
     */
    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (controllerSN == cntlConfigMap.cntlConfigInfo[i].controllerSN)
        {
            return (i);
        }
    }

    return (MAX_CONTROLLERS);
}


/*----------------------------------------------------------------------------
**  Function Name: TestforMaster
**
**  Description:
**      This function determines whether the specified controller is the
**      master of a Virtrual Controller group..
**
**  Inputs:   controllerSN - Controller serial number
**
**  Returns:  1 = controller is master
**            0 = controller is NOT master
**
**--------------------------------------------------------------------------*/
bool TestforMaster(UINT32 controllerSN)
{
    /*
     * Assume the controller is not master unless proven otherwise.
     */
    bool        bMaster = FALSE;

    /*
     * To be considered a master controller a controller must meet the
     * following conditions:
     *
     * - Not inactivated or FW Update Inactive
     * - One of the two following conditions
     *   - Match the master controller serial number in the master config
     *   - Master controller serial number in the master config is zero
     *     (this happens when a controller has not yet been configured)
     */
    if (GetControllerFailureState() != FD_STATE_INACTIVATED &&
        GetControllerFailureState() != FD_STATE_FIRMWARE_UPDATE_INACTIVE &&
        (Qm_GetMasterControllerSN() == controllerSN || Qm_GetMasterControllerSN() == 0))
    {
        bMaster = TRUE;
    }

    return bMaster;
}


/*----------------------------------------------------------------------------
**  Function Name: GetControllerSN
**
**  Description:
**      This function returns the controller S/N for the specified
**      communications slot index.
**
**  Inputs:   commSlot - Communications slot index
**
**  Returns:  controllerSN - Controller serial number
**            0 - if slot index is out of range
**
**--------------------------------------------------------------------------*/
UINT32 GetControllerSN(UINT16 slotID)
{
    /*
     *  Validity check the slot index passed in. If it is valid, return the
     *  controller s/n for that index. Otherwise, return zero.
     */
    if (slotID < MAX_CONTROLLERS)
    {
        return (cntlConfigMap.cntlConfigInfo[slotID].controllerSN);
    }

    return (0);
}


/*----------------------------------------------------------------------------
**  Function Name: GetTransportType
**
**  Description:
**      This function returns the transport mechanism assigned to a given
**      IP Address
**
**  Inputs:   ipAddress - IP address
**
**  Returns:  PATH_TYPE -       physical transport mechanism
**                              - Ethernet, Fibre, or No path
**
**--------------------------------------------------------------------------*/
PATH_TYPE GetTransportType(UINT32 ipAddress)
{
    int         i;

    /*
     *  Walk through the controller map and try to match the given IP
     *  address to an address for a Ethernet or fibre transport.
     */
    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (ipAddress == cntlConfigMap.cntlConfigInfo[i].ipEthernetAddress)
        {
            return (SENDPACKET_ETHERNET);
        }
    }

    return (SENDPACKET_NO_PATH);
}


/*----------------------------------------------------------------------------
**  Function Name: SerialNumberToIPAddress
**
**  Description:
**      This function returns the IP address for a given controller serial
**      number and a physical transport mechanism. If the transport
**      mechnism is unknown, the function returns 0 for the serial number.
**
**  Inputs:   controllerSN      - controller serial number.
**            PATH_TYPE          - physical transport mechanism
**
**  Returns:   ipAddress - IP address
**                       - 0 - if unknown
**
**--------------------------------------------------------------------------*/
UINT32 SerialNumberToIPAddress(UINT32 controllerSN, PATH_TYPE pt)
{
    int         i;

    /*
     *  Walk through the controller map and try to match the controller
     *  serial number and transport mechanism. If a match is found,
     *  return the IP address for that controller.
     */
    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (controllerSN == cntlConfigMap.cntlConfigInfo[i].controllerSN)
        {
            if (pt == SENDPACKET_ETHERNET)
            {
                return (cntlConfigMap.cntlConfigInfo[i].ipEthernetAddress);
            }
            else
            {
                return (0);
            }
        }
    }

    return (0);
}


/*----------------------------------------------------------------------------
**  Function Name: IPAddressToSerialNumber
**
**  Description:
**      This function returns the controller serial number given an IP address
**      (either ethernet or fibre).
**
**  Inputs:   ipAddress - IP address
**
**  Returns:  controllerSN  - Controller serial number
**                          - 0 - if unknown
**
**--------------------------------------------------------------------------*/
UINT32 IPAddressToSerialNumber(UINT32 ipAddress)
{
    int         i;

    /*
     *  Walk through the controller map and try to match the given IP
     *  address to an address for a Ethernet or fibre transport. If found,
     *  return the serial number for that controller.
     */
    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (ipAddress == cntlConfigMap.cntlConfigInfo[i].ipEthernetAddress)
        {
            return (cntlConfigMap.cntlConfigInfo[i].controllerSN);
        }
    }

    return (0);
}


/*----------------------------------------------------------------------------
**  Function Name: LoadMasterConfiguration
**
**  Description:
**      This function loads the master configuration record. It will first
**      attempt to load the record from the quorum area. If this was not
**      successful, it will load the last configuration from the NVRAM. If
**      this is not valid, it will load defaults.
**
**  Returns:    0 = Data restored from quorum
**              1 = Restored from NVRAM
**              2 = Defaults loaded
**
**--------------------------------------------------------------------------*/
UINT32 LoadMasterConfiguration(void)
{
    UINT32      rc;

    dprintf(DPRINTF_DEFAULT, "LoadMasterConfiguration\n");

    /*
     * Read the master configuration from the quorum.
     */
    rc = ReadMasterConfiguration(&masterConfig);

    /*
     * If the read failed or the quorum does not contain a valid master
     * configuration record, retore the configuration from NVRAM. If the
     * NVRAM is not valid, load with defaults.
     */
    if (rc || (masterConfig.schema != SCHEMA) ||
        (masterConfig.magicNumber != NVRAM_MAGIC_NUMBER))
    {
        dprintf(DPRINTF_QUORUMUTILS, "FAILURE - Master Configuration Error\n");
        dprintf(DPRINTF_QUORUMUTILS, "    rc = %d\n", rc);
        dprintf(DPRINTF_QUORUMUTILS, "    SCHEMA = 0x%x\n", masterConfig.schema);
        dprintf(DPRINTF_QUORUMUTILS, "    magic number = 0x%x\n",
                masterConfig.magicNumber);

        /*
         * Get the latest configuration from the NVRAM
         */
        LoadMasterConfigFromNVRAM();
        rc = 2;

    }
    else
    {
        rc = 0;
    }

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
#ifdef FORCE_IP_CHANGE
dprintf(DPRINTF_DEFAULT, "CHECK: SN=%d ES=%d MID:%08x inVCG=%d MIP=%08x GW=%08x MSK=%08x\n",
    masterConfig.virtualControllerSN, masterConfig.electionSerial, masterConfig.currentMasterID, masterConfig.numControllersInVCG,
    masterConfig.ipAddress, masterConfig.gatewayAddress, masterConfig.subnetMask);

// if (((masterConfig.ipAddress >>24) & 0xff) != 2)
{
// #define M_FORCE_IP          (192 | (168 << 8) | (100 << 16) | (101 << 24))
// #define M_FORCE_MSK         255 | (255 << 8) | (255 << 16) | (0 << 24)
// #define M_FORCE_GW          0
// #define M_FORCE_MASTER_CN   0
#define M_FORCE_IP          (10 | (64 << 8) | (102 << 16) | (2 << 24))
#define M_FORCE_MSK         255 | (255 << 8) | (240 << 16) | (0 << 24)
#define M_FORCE_GW          (10 | (64 << 8) | (96 << 16) | (1 << 24))
#define M_FORCE_MASTER_CN   0
    masterConfig.ipAddress = M_FORCE_IP;
    masterConfig.gatewayAddress = M_FORCE_GW;
    masterConfig.subnetMask = M_FORCE_GW;
    masterConfig.currentMasterID = (masterConfig.virtualControllerSN << 4) | M_FORCE_MASTER_CN;

    (void)WriteFile(FS_FID_QM_MASTER_CONFIG, &masterConfig, sizeof(masterConfig));
dprintf(DPRINTF_DEFAULT, "FORCE: SN=%d ES=%d MID:%08x inVCG=%d MIP=%08x GW=%08x MSK=%08x\n",
    masterConfig.virtualControllerSN, masterConfig.electionSerial, masterConfig.currentMasterID, masterConfig.numControllersInVCG,
    masterConfig.ipAddress, masterConfig.gatewayAddress, masterConfig.subnetMask);
}
#endif /* FORCE_IP_CHANGE */
    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: LoadControllerMap
**
**  Description:
**      This function loads the controler map record. It will first
**      attempt to load the record from the quorum area.  If
**      this is not valid, it will load defaults.
**
**  Returns:    0 = Data restored from quorum
**              2 = Defaults loaded
**
**--------------------------------------------------------------------------*/
UINT32 LoadControllerMap(void)
{
    UINT32      rc;

    dprintf(DPRINTF_DEFAULT, "LoadControllerMap\n");

    /*
     * Read the controller map from the quorum. If the quorum read was bad or
     * the data is not valid, load defaults.
     */
    rc = ReadControllerMap(&cntlConfigMap);
    if (rc)
    {
        dprintf(DPRINTF_QUORUMUTILS, "FAILURE - Controller Map Read Error\n");

        /*
         * Fill in the defaults for the controller map.
         */
        memset(&cntlConfigMap, 0, sizeof(QM_CONTROLLER_CONFIG_MAP));
        cntlConfigMap.schema = CONTROLLER_MAP_SCHEMA;
        rc = 2;

    }
    else
    {
        rc = 0;
    }

// See above.
#ifdef FORCE_IP_CHANGE
dprintf(DPRINTF_DEFAULT, "CHECK0: SN=%d ip=%08x nip=%08x gw=%08x ngw=%08x m=%08x nm=%08x\n",
    cntlConfigMap.cntlConfigInfo[0].controllerSN, cntlConfigMap.cntlConfigInfo[0].ipEthernetAddress,
    cntlConfigMap.cntlConfigInfo[0].newIpEthernetAddress, cntlConfigMap.cntlConfigInfo[0].gatewayAddress,
    cntlConfigMap.cntlConfigInfo[0].newGatewayAddress, cntlConfigMap.cntlConfigInfo[0].subnetMask,
    cntlConfigMap.cntlConfigInfo[0].newSubnetMask);
dprintf(DPRINTF_DEFAULT, "CHECK1: SN=%d ip=%08x nip=%08x gw=%08x ngw=%08x m=%08x nm=%08x\n",
    cntlConfigMap.cntlConfigInfo[1].controllerSN, cntlConfigMap.cntlConfigInfo[1].ipEthernetAddress,
    cntlConfigMap.cntlConfigInfo[1].newIpEthernetAddress, cntlConfigMap.cntlConfigInfo[1].gatewayAddress,
    cntlConfigMap.cntlConfigInfo[1].newGatewayAddress, cntlConfigMap.cntlConfigInfo[1].subnetMask,
    cntlConfigMap.cntlConfigInfo[1].newSubnetMask);

if ((cntlConfigMap.cntlConfigInfo[0].ipEthernetAddress & 0xff) != 192)
{
    /* FORCE VALUES */
#define FORCE_SN    10233
#define FORCE_IP1   (10 | (64 << 8) | (102 << 16) | (2 << 24))
#define FORCE_IP2   (10 | (64 << 8) | (102 << 16) | (3 << 24))
#define FORCE_MSK   255 | (255 << 8) | (240 << 16) | (0 << 24)
#define FORCE_GW    (10 | (64 << 8) | (96 << 16) | (1 << 24))
    cntlConfigMap.schema = CONTROLLER_MAP_SCHEMA;
    cntlConfigMap.cntlConfigInfo[0].controllerSN = (FORCE_SN << 4);
    cntlConfigMap.cntlConfigInfo[0].ipEthernetAddress = FORCE_IP1;
    cntlConfigMap.cntlConfigInfo[0].newIpEthernetAddress = FORCE_IP1;
    cntlConfigMap.cntlConfigInfo[0].gatewayAddress = FORCE_GW;
    cntlConfigMap.cntlConfigInfo[0].newGatewayAddress = FORCE_GW;
    cntlConfigMap.cntlConfigInfo[0].subnetMask = FORCE_MSK;
    cntlConfigMap.cntlConfigInfo[0].newSubnetMask = FORCE_MSK;

    cntlConfigMap.cntlConfigInfo[1].controllerSN = (FORCE_SN << 4) + 1;
    cntlConfigMap.cntlConfigInfo[1].ipEthernetAddress = FORCE_IP2;
    cntlConfigMap.cntlConfigInfo[1].newIpEthernetAddress = FORCE_IP2;
    cntlConfigMap.cntlConfigInfo[1].gatewayAddress = FORCE_GW;
    cntlConfigMap.cntlConfigInfo[1].newGatewayAddress = FORCE_GW;
    cntlConfigMap.cntlConfigInfo[1].subnetMask = FORCE_MSK;
    cntlConfigMap.cntlConfigInfo[1].newSubnetMask = FORCE_MSK;

dprintf(DPRINTF_DEFAULT, "FORCE0: SN=%d ip=%08x nip=%08x gw=%08x ngw=%08x m=%08x nm=%08x\n",
    cntlConfigMap.cntlConfigInfo[0].controllerSN, cntlConfigMap.cntlConfigInfo[0].ipEthernetAddress,
    cntlConfigMap.cntlConfigInfo[0].newIpEthernetAddress, cntlConfigMap.cntlConfigInfo[0].gatewayAddress,
    cntlConfigMap.cntlConfigInfo[0].newGatewayAddress, cntlConfigMap.cntlConfigInfo[0].subnetMask,
    cntlConfigMap.cntlConfigInfo[0].newSubnetMask);
dprintf(DPRINTF_DEFAULT, "FORCE1: SN=%d ip=%08x nip=%08x gw=%08x ngw=%08x m=%08x nm=%08x\n",
    cntlConfigMap.cntlConfigInfo[1].controllerSN, cntlConfigMap.cntlConfigInfo[1].ipEthernetAddress,
    cntlConfigMap.cntlConfigInfo[1].newIpEthernetAddress, cntlConfigMap.cntlConfigInfo[1].gatewayAddress,
    cntlConfigMap.cntlConfigInfo[1].newGatewayAddress, cntlConfigMap.cntlConfigInfo[1].subnetMask,
    cntlConfigMap.cntlConfigInfo[1].newSubnetMask);

    INT32 rcw = WriteFile(FS_FID_QM_CONTROLLER_MAP, &cntlConfigMap, sizeof(cntlConfigMap));
    dprintf(DPRINTF_DEFAULT, "Wrote Controller Map, rcw=%x (0=GOOD, errors like FS_ERROR_WRITE_NULL_BUFFER)\n", rcw);
}
#endif /* FORCE_IP_CHANGE */

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UINT32 ReloadMasterConfigWithRetries
**
**  Inputs:     None
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
#define MASTER_CONFIG_RETRIES   10

UINT32 ReloadMasterConfigWithRetries(void)
{
    QM_MASTER_CONFIG *tempMasterConfig;
    UINT32      rc = ERROR;
    UINT32      retryCount = 0;

    dprintf(DPRINTF_DEFAULT, "ReloadMasterConfigWithRetries\n");

    tempMasterConfig = MallocSharedWC(sizeof(*tempMasterConfig));
    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the master configuration from the quorum.
         */
        rc = ReadMasterConfiguration(tempMasterConfig);

        if (rc != GOOD || (tempMasterConfig->schema != SCHEMA) ||
            (tempMasterConfig->magicNumber != NVRAM_MAGIC_NUMBER))
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_READ_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_READ_DATA_PI_TIMEOUT)
            {
#if defined(MODEL_7000) || defined(MODEL_4700)
                TaskSleepMS(1000);
#else  /* MODEL_7000 || MODEL_4700 */
                break;          /* For 4000 break to end of while() loop */
#endif /* MODEL_7000 || MODEL_4700 */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR reading Master Configuration - retrying...\n");
                TaskSleepMS(1000);
            }
        }

    } while ((rc != GOOD) && (retryCount++ < MASTER_CONFIG_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if ((rc == GOOD) && (retryCount < MASTER_CONFIG_RETRIES))
    {
        memcpy(&masterConfig, tempMasterConfig, sizeof(masterConfig));
    }
    else
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED reading Master Configuration.\n");
    }
    Free(tempMasterConfig);

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UINT32 ReadCommAreaWithRetries
**
**  Description: Read Communications area from the quorum with retries
**
**  Inputs:     None
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
#if defined(MODEL_7000) || defined(MODEL_4700)
#define COMM_AREA_RETRIES   10
#else  /* MODEL_7000 || MODEL_4700 */
#define COMM_AREA_RETRIES   2
#endif /* MODEL_7000 || MODEL_4700 */
UINT32 ReadCommAreaWithRetries(UINT16 slotID, QM_CONTROLLER_COMM_AREA *commPtr)
{
    UINT32      rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the communications area  from the quorum.
         */
        rc = ReadCommArea(slotID, commPtr);

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_READ_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_READ_DATA_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR reading Comm Area - retrying...\n");
                TaskSleepMS(1000);
            }
        }

    } while ((rc != GOOD) && (retryCount++ < COMM_AREA_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED reading Comm Area.\n");
    }

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UINT32 ReadAllCommunicationsWithRetries
**
**  Description: Read Communications area from the quorum with retries
**
**  Inputs:     None
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
UINT32 ReadAllCommunicationsWithRetries(QM_CONTROLLER_COMM_AREA commPtr[])
{
    UINT32      rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the communications area  from the quorum.
         */
        rc = ReadAllCommunications(commPtr);

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_READ_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_READ_DATA_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR reading ALL Comm Area - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < COMM_AREA_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED reading ALL Comm Area.\n");
    }

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UINT32 ReadFailureDataWithRetries
**
**  Description: Read Communications area from the quorum with retries
**
**  Inputs:     None
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
#if defined(MODEL_7000) || defined(MODEL_4700)
#define FAILURE_DATA_RETRIES   10
#else  /* MODEL_7000 || MODEL_4700 */
#define FAILURE_DATA_RETRIES   2
#endif /* MODEL_7000 || MODEL_4700 */
UINT32 ReadFailureDataWithRetries(unsigned long controllerSN, QM_FAILURE_DATA *failurePtr)
{
    UINT32      rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the communications area  from the quorum.
         */
        rc = ReadFailureData(controllerSN, failurePtr);

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_READ_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_READ_DATA_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR reading Failure Data - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < FAILURE_DATA_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED reading Failure Data.\n");
    }

    return (rc);
}



/*----------------------------------------------------------------------------
**  Function Name: UINT32 WriteElectionDataWithRetries
**
**  Description: Write Failure data  area in the quorum with retries
**
**  Inputs:     None
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
UINT32 WriteElectionDataWithRetries(unsigned long controllerSN,
                                    QM_ELECTION_DATA *electionPtr)
{
    UINT32      rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the communications area  from the quorum.
         */
        rc = WriteElectionData(controllerSN, electionPtr);

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_WRITE_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR writing election data - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < FAILURE_DATA_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED writing election data.\n");
    }

    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: UINT32 WriteFailureDataWithRetries
**
**  Description: Write Failure data  area in the quorum with retries
**
**  Inputs:     None
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
UINT32 WriteFailureDataWithRetries(unsigned long controllerSN,
                                   QM_FAILURE_DATA *failurePtr)
{
    UINT32      rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the master configuration, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the communications area  from the quorum.
         */
        rc = WriteFailureData(controllerSN, failurePtr);

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_WRITE_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR writing failure data - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < FAILURE_DATA_RETRIES));

    /*
     * If we were successful reading the master config, copy the data
     * to the global config area.
     */
    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED writing failure data.\n");
    }

    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: UINT32 ReadFWCompatIndexWithRetries
**
**  Description: Read FW Compat Index from the filesystem with retries
**
**  Inputs:     Pointer to area to store FW compat data
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
#if defined(MODEL_7000) || defined(MODEL_4700)
#define FW_COMPAT_DATA_RETRIES 10
#else  /* MODEL_7000 || MODEL_4700 */
#define FW_COMPAT_DATA_RETRIES 2
#endif /* MODEL_7000 || MODEL_4700 */
INT32 ReadFWCompatIndexWithRetries(FW_COMPAT_DATA *fwCompatData)
{
    INT32       rc = GOOD;
    UINT32      retryCount = 0;

    /*
     * Attempt to read the FW Compat Data, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Read the FW Compat Data FID. Reading at offset 1 skips reading
         * and CRC checking the header.
         */
        rc = ReadFileAtOffset(FS_FID_FW_COMPAT_DATA, 1, fwCompatData,
                              sizeof(*fwCompatData));

        if (rc == GOOD)
        {
            /*
             * Check the CRC of the data
             */
            if (fwCompatData->crc != CRC32((UINT8 *)fwCompatData, sizeof(*fwCompatData) - sizeof(UINT32)))
            {
                dprintf(DPRINTF_DEFAULT, "ReadFWCompatIndexWithRetries: CRC mismatch\n");
                WriteFWCompatIndexWithRetries((FW_HEADER *)CCBRuntimeFWHAddr);
                dprintf(DPRINTF_DEFAULT, "ReadFWCompatIndexWithRetries: FID9_HACK written\n");
                rc = FS_ERROR_READ_CRC_CHECK_DATA;
            }
        }

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_READ_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_READ_DATA_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR reading FW Compat Data - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < FW_COMPAT_DATA_RETRIES));

    if (rc != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "FAILED reading FW Compat Data, rc = %u\n", rc);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
**  Function Name: UINT32 WriteFWCompatIndexWithRetries
**
**  Description: Write FW Compat Index from the filesystem with retries
**
**  Inputs:      Pointer to CCB FW Header
**
**  Modifies:
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static INT32 WriteFWCompatIndexWithRetries(FW_HEADER *ccbFwHdr)
{
    INT32       rc = GOOD;
    UINT32      retryCount = 0;
    FW_COMPAT_DATA *fwCompatData;

    /*
     * Initialize the data to write
     */
    fwCompatData = MallocSharedWC(sizeof(*fwCompatData));
    memcpy(&fwCompatData->ccbHdr, ccbFwHdr, sizeof(*ccbFwHdr));
    fwCompatData->schema = FW_COMPAT_DATA_SCHEMA;
    memset(fwCompatData->reserved, 0, sizeof(fwCompatData->reserved));
    fwCompatData->crc = CRC32((UINT8 *)fwCompatData, sizeof(*fwCompatData) - sizeof(UINT32));

    /*
     * Attempt to write the FW Compat Data, until successful or the
     * retries are expired.
     */
    do
    {
        /*
         * Write the FW Compat Data FID
         */
        rc = WriteFile(FS_FID_FW_COMPAT_DATA, fwCompatData, sizeof(*fwCompatData));

        if (rc != GOOD)
        {
            /*
             * Don't retry on file system timeout
             */
            if (rc == FS_ERROR_WRITE_HEADER_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT ||
                rc == FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT)
            {
                break;          /* break to end of while() loop */
            }
            else
            {
                /*
                 * Wait some time before trying again
                 */
                dprintf(DPRINTF_DEFAULT, "ERROR writing FW Compat Data - retrying...\n");
                TaskSleepMS(1000);
            }
        }
    } while ((rc != GOOD) && (retryCount++ < FW_COMPAT_DATA_RETRIES));

    Free(fwCompatData);

    if (rc != GOOD)
    {
        rc = ERROR;
        dprintf(DPRINTF_DEFAULT, "FAILED writing FW Compat Data.\n");
    }

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
