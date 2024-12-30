/* $Id: CacheMisc.c 159129 2012-05-12 06:25:16Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       CacheMisc.c
** MODULE TITLE:    CCB Cache - Miscellaneous
**
** DESCRIPTION:     Cached data for the left over stuff that doesn't
**                  fit anywhere else
**
** Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "CacheMisc.h"

#include "CacheBay.h"
#include "CacheLoop.h"
#include "CacheManager.h"
#include "CachePDisk.h"
#include "CacheServer.h"
#include "CacheTarget.h"
#include "CmdLayers.h"
#include "convert.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "HWM.h"
#include "MR_Defs.h"
#include "ipc_heartbeat.h"
#include "ipc_sendpacket.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_Misc.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "quorum_utils.h"
#include "rm_val.h"
#include "ses.h"
#include "X1_Structs.h"
#include "X1_Utils.h"

#include <byteswap.h>

/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** Macro for bad status in SES Control Element - Common Control field.
** If any of the conditions are present the status will be returned
** as an error.
*/
#define CCFail(x)       (((x) & SES_CC_STAT_MASK) == SES_CC_STAT_UNINST)

#define BLOCKS_TO_MB        ((1024 * 1024) / 512)

/*****************************************************************************
** Private variables
*****************************************************************************/
static CACHE_ENV_STATS cacheEnvStats;
static HWM_STATUS controllerStatus;
static MR_FW_HEADER_RSP cacheFirmwareHeaders[MAX_FW_HDR_TYPES];

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
X1_CONFIG_ACCOUNT cacheAccountInfo;
CACHE_FREE_SPACE cacheFreeSpace;

UINT8       cacheVcgInfo1[CACHE_SIZE_VCG_INFO];
UINT8       cacheVcgInfo2[CACHE_SIZE_VCG_INFO];
UINT8      *cacheVcgInfo = NULL;
UINT8      *cacheTempVcgInfo = NULL;

PORT_INFO   cacheBEPortInfo1[MAX_BE_PORTS];
PORT_INFO   cacheBEPortInfo2[MAX_BE_PORTS];
PORT_INFO  *cacheBEPortInfo = NULL;
PORT_INFO  *cacheTempBEPortInfo = NULL;

UINT8       cacheWorksetInfo1[CACHE_SIZE_WORKSET_INFO] LOCATE_IN_SHMEM;
UINT8       cacheWorksetInfo2[CACHE_SIZE_WORKSET_INFO] LOCATE_IN_SHMEM;
UINT8      *cacheWorksetInfo = NULL;
UINT8      *cacheTempWorksetInfo = NULL;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 GetBufferBoardStatus(NVRAM_BOARD_INFO *pBoardInfo);
static void RefreshFirmwareHeader(UINT16 fwType, MR_FW_HEADER_RSP *pRsp);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Description: Get environmental data for ten minute periodic print.
**
** Outputs: air_temp - Air temperature
**          cpu_temp - CPU temperature
**          mbPerSecond - Server MB per second
**          ioPerSecond - Number of IOs per second
**--------------------------------------------------------------------------*/
void GetTenEnv(UINT8 *air_temp, UINT8 *cpu_temp, UINT32 *mbPerSecond,
               UINT32 *ioPerSecond)
{

    /*
     * Wait until the cache is not in the process of being updated. Once it is
     * in that state make sure it is set to in use so a update doesn't start
     * while it is being used.
     */
    CacheStateWaitUpdating(cacheEnvState);
    CacheStateSetInUse(cacheEnvState);

    *air_temp = cacheEnvStats.ctrlTempHost;
    *cpu_temp = cacheEnvStats.ctrlTempStore;
    /* Get latest mb/s and io/s. */
    *mbPerSecond = ((GetServerBytesPerSecond() * 100) >> 20);
    *ioPerSecond = GetServerIOPerSecond();

    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheEnvState);
}

/*----------------------------------------------------------------------------
** Function:    EnvironmentalGetStats
**
** Description: Copy environmental data from the cache into an X1
**              response packet.
**
** Inputs:
**
** Outputs:     NONE
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void EnvironmentalGetStats(UINT8 *pBuf)
{
    X1_ENV_STATS_RSP    *pX1Rsp = (X1_ENV_STATS_RSP*)pBuf;

    /*
    ** Wait until the cache is not in the process of
    ** being updated.  Once it is in that state make sure it is set
    ** to in use so a update doesn't start while it is being used.
    */
    CacheStateWaitUpdating(cacheEnvState);
    CacheStateSetInUse(cacheEnvState);

    /*
     * Copy data from the cache to the X1 response packet.
     */
    pX1Rsp->ctrlTempHost  =  cacheEnvStats.ctrlTempHost;
    pX1Rsp->ctrlTempStore =  cacheEnvStats.ctrlTempStore;
    pX1Rsp->ctrlAC1  =  cacheEnvStats.ctrlAC1;
    pX1Rsp->ctrlAC2  =  cacheEnvStats.ctrlAC2;
    pX1Rsp->ctrlDC1  =  cacheEnvStats.ctrlDC1;
    pX1Rsp->ctrlDC2  =  cacheEnvStats.ctrlDC2;
    pX1Rsp->ctrlFan1 =  cacheEnvStats.ctrlFan1;
    pX1Rsp->ctrlFan2 =  cacheEnvStats.ctrlFan2;
    pX1Rsp->ctrlBufferHost  =  cacheEnvStats.ctrlBufferHost;
    pX1Rsp->ctrlBufferStore =  cacheEnvStats.ctrlBufferStore;

    /*
     * The size of the following maps in cache is different than
     * the size defined in the X1 packet.  Copy only as much as
     * the X1 packet can handle.  This assumes that the X1 map size
     * is less than or equal to the cache map size.
     */
    memcpy(pX1Rsp->fibreBayExistBitmap, cacheEnvStats.fibreBayExistBitmap, X1_DISK_BAY_MAP_SIZE);

    memcpy(pX1Rsp->fibreBayTempIn1, cacheEnvStats.fibreBayTempIn1, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->fibreBayTempIn2, cacheEnvStats.fibreBayTempIn2, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->fibreBayTempOut1, cacheEnvStats.fibreBayTempOut1, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->fibreBayTempOut2, cacheEnvStats.fibreBayTempOut2, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->fibreBayPSFan, cacheEnvStats.fibreBayPSFan, X1_MAX_DISK_BAYS);

    pX1Rsp->mbPerSecond = cacheEnvStats.mbPerSecond;
    pX1Rsp->ioPerSecond = cacheEnvStats.ioPerSecond;
    pX1Rsp->beHeartbeat = cacheEnvStats.beHeartbeat;
    pX1Rsp->feHeartbeat = cacheEnvStats.feHeartbeat;

    /*
     * The size of the following maps in cache is different than
     * the size defined in the X1 packet.  Copy only as much as
     * the X1 packet can handle.  This assumes that the X1 map size
     * is less than or equal to the cache map size.
     */
    memcpy(pX1Rsp->sataBayExistBitmap, cacheEnvStats.sataBayExistBitmap, X1_DISK_BAY_MAP_SIZE);

    memcpy(pX1Rsp->sataBayTempOut1, cacheEnvStats.sataBayTempOut1, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->sataBayTempOut2, cacheEnvStats.sataBayTempOut2, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->sataBayPS, cacheEnvStats.sataBayPS, X1_MAX_DISK_BAYS);

    memcpy(pX1Rsp->sataBayFan, cacheEnvStats.sataBayFan, X1_MAX_DISK_BAYS);

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cacheEnvState);
}

/*----------------------------------------------------------------------------
** Function:    GetFirmwareHeader
**
** Description: Get the firmware header for the given firmware type.
**
** Inputs:      UINT16 fwType - Firmware type to retrieve.  This is
**                              an index into the cache using the
**                              values defined in CacheManager.h:
**                              - FW_HDR_TYPE_CCB_RUNTIME
**                              - FW_HDR_TYPE_BE_RUNTIME
**                              - FW_HDR_TYPE_FE_RUNTIME
**
** Outputs:     MR_FW_HEADER_RSP *pFWHeader
**
** Returns:     none
**--------------------------------------------------------------------------*/
void GetFirmwareHeader(UINT16 fwType, MR_FW_HEADER_RSP *pFWHeader)
{
    memcpy(pFWHeader, &cacheFirmwareHeaders[fwType], sizeof(*pFWHeader));
}

/*----------------------------------------------------------------------------
** Function:    GetFirmwareData
**
** Description: Get the firmware data.
**
** Outputs:     FW_DATA *pFWData
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void GetFirmwareData(FW_DATA *pFWData)
{
    pFWData->revision = cacheFirmwareHeaders[FW_HDR_TYPE_CCB_RUNTIME].fw.revision;
    pFWData->revCount = cacheFirmwareHeaders[FW_HDR_TYPE_CCB_RUNTIME].fw.revCount;
    pFWData->buildID = cacheFirmwareHeaders[FW_HDR_TYPE_CCB_RUNTIME].fw.buildID;
    pFWData->timeStamp = *(&cacheFirmwareHeaders[FW_HDR_TYPE_CCB_RUNTIME].fw.timeStamp);
}

/*----------------------------------------------------------------------------
** Function:    GetFirmwareCompatibilityIndex
**
** Description: Get the firmware compatibility index.
**
** Inputs:      NONE
**
** Returns:     Firmware compatibility index.
**
**--------------------------------------------------------------------------*/
UINT8 GetFirmwareCompatibilityIndex(void)
{
    return cacheFirmwareHeaders[FW_HDR_TYPE_CCB_RUNTIME].fw.fwCompatIndex;
}

/**
******************************************************************************
**
**  @brief      GetBufferBoardStatus
**
**              Get the Buffer Board Status
**
**  @param      none
**
**  @return     Buffer board status info (MRMMCARDGETBATTERYSTATUS_RSP)
**
**  @attention  Caller must allocate enough memory for the request.
**
******************************************************************************
**/
static INT32 GetBufferBoardStatus(NVRAM_BOARD_INFO *pBoardInfo)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pPacket = MallocWC(sizeof(PI_STATS_BUFFER_BOARD_REQ));
    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPkt.pHeader->commandCode = PI_STATS_BUFFER_BOARD_CMD;
    reqPkt.pHeader->length = sizeof(PI_STATS_BUFFER_BOARD_REQ);

    /*
     * Fill in the request parms.
     */
    ((PI_STATS_BUFFER_BOARD_REQ *)reqPkt.pPacket)->commandCode = NVRAM_BOARD_COMMAND_INFO_ONLY;

    /*
     * Issue the command through the top-level command handler
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    /*
     * If the request was successful copy the data into the buffer.
     */
    if (rc == PI_GOOD)
    {
        memcpy(pBoardInfo, &(((PI_STATS_BUFFER_BOARD_RSP *)rspPkt.pPacket)->boardInfo),
               sizeof(NVRAM_BOARD_INFO));
    }

    /*
     * Free the request and response headers.  If a timeout occurred keep
     * the response packet around - the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);
    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    RefreshEnvironmentals
**
** Description: Get Controller and Fibre Disk Bay Environmental Info / Stats
**              All of this information is cached in other places in the
**              CCB code.
**
** Inputs:      NONE
**
** WARNING:     This function is NOT currently run as part of a forked
**              task.  If changes are made to call it from a forked task
**              the cache state flag must be checked.
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshEnvironmentals(void)
{
    INT32       rc = PI_GOOD;
    PSES_DEVICE pSES;
    NVRAM_BOARD_INFO *pBoardInfo;
    UINT16      bayId;
    UINT8       firstTempElement;
    UINT8       firstPwrElement;
    UINT8       firstFanElement;
    UINT8       cc1;
    UINT8       cc2;
    UINT8       temp1;
    UINT8       temp2;
    UINT8       i;

    /*
     * fprintf(stderr, "%s: ENTER (0x%x)\n", __func__, K_timel);
     */

    /*
     * Wait for good cache status before continuing.  Set the flag
     * to indicate that the cache is updating.
     */
    CacheStateWaitOkToUpdate(cacheEnvState);
    CacheStateSetUpdating(cacheEnvState);

    /*
     * Get updated controller stats.  If this call fails we just return
     * the old data.
     */
    if (HWM_GetMonitorStatus(&controllerStatus) == GOOD)
    {
        cacheEnvStats.ctrlTempHost = controllerStatus.procBoardStatus.frontEndProcessorStatus.temperatureStatus.currentDegreesCelsius;

        cacheEnvStats.ctrlTempStore = controllerStatus.procBoardStatus.backEndProcessorStatus.temperatureStatus.currentDegreesCelsius;


        /*
         * Get the power supply condition value and translate it into
         * the value stored in the cache data.
         */
        temp1 = controllerStatus.frontEndPowerSupply.powerSupplyCondition.value;

        if (temp1 == POWER_SUPPLY_CONDITION_AC_FAILED)
        {
            cacheEnvStats.ctrlAC2 = CTRL_AC_FAIL;
        }
        else
        {
            cacheEnvStats.ctrlAC2 = CTRL_AC_GOOD;
        }

        temp2 = controllerStatus.backEndPowerSupply.powerSupplyCondition.value;

        if (temp2 == POWER_SUPPLY_CONDITION_AC_FAILED)
        {
            cacheEnvStats.ctrlAC1 = CTRL_AC_FAIL;
        }
        else
        {
            cacheEnvStats.ctrlAC1 = CTRL_AC_GOOD;
        }

        /*
         * Assume the power supplies exist, then check falure states.
         */
        cacheEnvStats.ctrlDC1 = CTRL_DC_PS_EXISTS;
        cacheEnvStats.ctrlDC2 = CTRL_DC_PS_EXISTS;

        if (temp1 == POWER_SUPPLY_CONDITION_NOT_PRESENT)
        {
            cacheEnvStats.ctrlDC2 = CTRL_DC_PS_NOT_EXIST;
        }

        if (temp1 == POWER_SUPPLY_CONDITION_DC_FAILED)
        {
            cacheEnvStats.ctrlDC2 |= CTRL_DC_FAIL;
        }
        else
        {
            cacheEnvStats.ctrlDC2 |= CTRL_DC_GOOD;
        }

        if (temp2 == POWER_SUPPLY_CONDITION_NOT_PRESENT)
        {
            cacheEnvStats.ctrlDC1 = CTRL_DC_PS_NOT_EXIST;
        }

        if (temp2 == POWER_SUPPLY_CONDITION_DC_FAILED)
        {
            cacheEnvStats.ctrlDC1 |= CTRL_DC_FAIL;
        }
        else
        {
            cacheEnvStats.ctrlDC1 |= CTRL_DC_GOOD;
        }

        /*
         * Get the fan status
         */
        temp1 = controllerStatus.frontEndPowerSupply.coolingFanConditionValue;

        if (temp1 == COOLING_FAN_CONDITION_GOOD)
        {
            cacheEnvStats.ctrlFan2 = CTRL_FAN_GOOD;
        }
        else
        {
            cacheEnvStats.ctrlFan2 = CTRL_FAN_FAIL;
        }

        temp2 = controllerStatus.backEndPowerSupply.coolingFanConditionValue;

        if (temp2 == COOLING_FAN_CONDITION_GOOD)
        {
            cacheEnvStats.ctrlFan1 = CTRL_FAN_GOOD;
        }
        else
        {
            cacheEnvStats.ctrlFan1 = CTRL_FAN_FAIL;
        }

        /*
         * Get the buffer board status
         */
        pBoardInfo = MallocWC(sizeof(*pBoardInfo));

        rc = GetBufferBoardStatus(pBoardInfo);

        /*
         * Look for any condition that might indicate that the buffer
         * board is not capable of sustaining the memory if the power
         * is removed.
         */
        if ((rc == GOOD) &&
            (pBoardInfo->boardStatus != NVRAM_BOARD_STATUS_UNKNOWN) &&
            (pBoardInfo->batteryCount > 0))
        {
            /*
             * Verify that the board status is valid, and that it has batteries
             */
            UINT32      counter = 0;
            UINT8      *pControl = NULL;

            /*
             * Initialize the host and store buffer status flags.  We'll
             * consider them good here, and look for failures below.
             */
            cacheEnvStats.ctrlBufferHost = CTRL_BUFFER_GOOD;
            cacheEnvStats.ctrlBufferStore = CTRL_BUFFER_GOOD;

            /*
             * Scan through all of the batteries
             */
            for (counter = 0;
                 ((counter < pBoardInfo->batteryCount) &&
                  (counter < dimension_of(pBoardInfo->batteryInformation)));
                 counter++)
            {
                /*
                 * Associate the even numbered batteries with the Bigfoot host
                 * side, and the odd numbered batteries with the storage side.
                 */
                if ((counter & 1) == 0)
                {
                    pControl = &cacheEnvStats.ctrlBufferHost;
                }
                else
                {
                    pControl = &cacheEnvStats.ctrlBufferStore;
                }

                /*
                 * The ICON uses the 'control' field to determine how the
                 * lights on the environmental status page should be colored.
                 * If there's any reason that the buffer board can't retain
                 * the data if power is lost, then we need to make sure that
                 * at least one of these is set to FAIL.
                 */
                switch (pBoardInfo->batteryInformation[counter].status)
                {
                        /*
                         * These conditions should match the conditions in
                         * IPMI_I2C function I2CRetrieveBatteryStatus.
                         */
                    case NVRAM_BATTERY_INFO_STATUS_GOOD:
                        if ((pBoardInfo->boardStatus != NVRAM_BOARD_STATUS_GOOD) &&
                            (pBoardInfo->boardStatus != NVRAM_BOARD_STATUS_LOW_BATTERY))
                        {
                            *pControl = CTRL_BUFFER_FAIL;
                        }
                        break;

                    default:
                        *pControl = CTRL_BUFFER_FAIL;
                        break;
                }
            }
        }
        else
        {
            /*
             * Board status is unknown, or no batteries exist
             */
            cacheEnvStats.ctrlBufferHost = CTRL_BUFFER_FAIL;
            cacheEnvStats.ctrlBufferStore = CTRL_BUFFER_FAIL;
        }

        Free(pBoardInfo);
    }
    else
    {
        rc = PI_ERROR;
        fprintf(stderr, "%s: HWM_GetMonitorStatus failed\n", __func__);
    }

    if (PowerUpBEReady())
    {
        /*
         * Use the cached disk bay info to create the disk bay map.
         * Wait until the Fibre Disk Bay cache is not in the process of
         * being updated.  Once it is in that state make sure it is set
         * to in use so a update doesn't start while it is being used.
         */
        CacheStateWaitUpdating(cacheDiskBaysState);
        CacheStateSetInUse(cacheDiskBaysState);

        /*
         * Copy the Fibre Disk Bay map into the environmental data.
         */
        memcpy(cacheEnvStats.fibreBayExistBitmap, cacheFibreBayMap, CACHE_SIZE_DISK_BAY_MAP);

        /*
         * Copy the SATA Disk Bay map into the environmental data.
         */
        memcpy(cacheEnvStats.sataBayExistBitmap, cacheSATABayMap, CACHE_SIZE_DISK_BAY_MAP);

        /*
         * No longer using the cache for this function...
         */
        CacheStateSetNotInUse(cacheDiskBaysState);

        /*
         * Get the fibre bay environmentals from the SES data.
         * Get a pointer to the list of fibre bays (SES enclosures).
         * Walk the SES list and pull out the necessary data.
         */
        pSES = GetSESList();

        while (pSES != NULL)
        {
            /*
             * Make sure the OldPage2 pointer is valid before continuing.
             */
            if (pSES->OldPage2 != NULL)
            {
                bayId = pSES->PID;

                /*
                 * If this is a fibre bay or an SBOD, load the appropriate parts
                 * of the cache structure.
                 */
                if ((pSES->devType == PD_DT_FC_SES) || (pSES->devType == PD_DT_SBOD_SES))
                {
                    /*
                     * The SES Map and Slot arrays hold the number and
                     * type of each element.  The index into Map[] is
                     * the element type.  The value of Map[] is the offset
                     * of the first element of that type in Page2 Control
                     * Element data. The first element is the Common Control
                     * type element for the element, which will be skipped.
                     *
                     * The value of Slot[] at the same index is the number of
                     * elements of this type - 1.  This is just FYI - we don't
                     * need this value.  Yes this is all very confusing.
                     * Consult SCSI Enclosure Services docs for more
                     * information.
                     */
                    firstTempElement = pSES->Map[SES_ET_TEMP_SENSOR] + 1;

                    cacheEnvStats.fibreBayTempIn1[bayId] = pSES->OldPage2->Control[firstTempElement++].Ctrl.Temp.Temp;

                    if (pSES->devType == PD_DT_FC_SES)
                    {
                        /*
                         * The fibre bay has more than one temp sensor.  The
                         * SBOD only have one.  Enter the extras for a FC bay.
                         */
                        cacheEnvStats.fibreBayTempIn2[bayId] = pSES->OldPage2->Control[firstTempElement++].Ctrl.Temp.Temp;

                        cacheEnvStats.fibreBayTempOut1[bayId] = pSES->OldPage2->Control[firstTempElement++].Ctrl.Temp.Temp;

                        cacheEnvStats.fibreBayTempOut2[bayId] = pSES->OldPage2->Control[firstTempElement].Ctrl.Temp.Temp;
                    }

                    /*
                     * 1 byte is used to indicate 4 power and 4 fan conditions
                     * for the fibre bays.  The code starts by assuming all
                     * elements are GOOD (1=GOOD).  If an element is determined
                     * to be bad the mask for the element is ANDed with the
                     * power/fan byte to set the state for that element to FAIL
                     * (FAIL=0).
                     *
                     * For the SBOD, there are only two fans.  They will be
                     * reported as the last two fans, skipping the first two.
                     */
                    cacheEnvStats.fibreBayPSFan[bayId] = BAY_PWR_FAN_ALL_GOOD;

                    /*
                     * Get the power supply status from the SES data.
                     * CommonCtrl indicates if the power supply exists.
                     * Ctrl2 indicates the status of the supply.
                     */
                    firstPwrElement = pSES->Map[SES_ET_POWER] + 2;

                    cc1 = pSES->OldPage2->Control[firstPwrElement].CommonCtrl;
                    temp1 = pSES->OldPage2->Control[firstPwrElement--].Ctrl.Generic.Ctrl2;
                    cc2 = pSES->OldPage2->Control[firstPwrElement].CommonCtrl;
                    temp2 = pSES->OldPage2->Control[firstPwrElement].Ctrl.Generic.Ctrl2;

                    /*
                     * Check the AC state for each supply.
                     * If CommonCtrl OR Ctrl2 indicates bad status set
                     * the FAIL flag.
                     */
                    if (CCFail(cc1) ||
                        (temp1 & (SES_C2PS_FAIL | SES_C2PS_OFF |
                                  SES_C2PS_TMPFAIL | SES_C2PS_ACFAIL)))
                    {
                        cacheEnvStats.fibreBayPSFan[bayId] &= BAY_AC_PS2_FAIL;
                    }

                    if (CCFail(cc2) ||
                        (temp2 & (SES_C2PS_FAIL | SES_C2PS_OFF |
                                  SES_C2PS_TMPFAIL | SES_C2PS_ACFAIL)))
                    {
                        cacheEnvStats.fibreBayPSFan[bayId] &= BAY_AC_PS1_FAIL;
                    }

                    /*
                     * Check the DC state for each supply.
                     */
                    if (CCFail(cc1) || (temp1 & SES_C2PS_DCFAIL))
                    {
                        cacheEnvStats.fibreBayPSFan[bayId] &= BAY_DC_PS2_FAIL;
                    }

                    if (CCFail(cc2) || (temp2 & SES_C2PS_DCFAIL))
                    {
                        cacheEnvStats.fibreBayPSFan[bayId] &= BAY_DC_PS1_FAIL;
                    }


                    /*
                     * Get the location of the first fan element in the SES
                     * data. Check the status of each of the fans.  Look at
                     * CommonControl and Ctrl2.
                     *
                     * For the SBOD, the A fan is in the second slot and the
                     * B fan is the first slot.  For the FC bay, the first two
                     * fans comprise the B fan unit and the second two are
                     * considered the B unit.
                     */
                    firstFanElement = pSES->Map[SES_ET_COOLING] + 1;

                    if (pSES->devType == PD_DT_FC_SES)
                    {
                        for (i = 0; i < 2; i++)
                        {
                            cc1 = pSES->OldPage2->Control[firstFanElement + i].CommonCtrl;
                            temp1 = pSES->OldPage2->Control[firstFanElement + i].Ctrl.Generic.Ctrl2;

                            /*
                             * If a fan failure is detected clear the appropriate fan bit.
                             */
                            if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                            {
                                cacheEnvStats.fibreBayPSFan[bayId] &= (~(FIBRE_BAY_FAN_BIT >> (i + 2)));
                            }
                        }

                        for (i = 0; i < 2; i++)
                        {
                            cc1 = pSES->OldPage2->Control[firstFanElement + 2 + i].CommonCtrl;
                            temp1 = pSES->OldPage2->Control[firstFanElement + 2 + i].Ctrl.Generic.Ctrl2;

                            /*
                             * If a fan failure is detected clear the appropriate fan bit.
                             */
                            if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                            {
                                cacheEnvStats.fibreBayPSFan[bayId] &= (~(FIBRE_BAY_FAN_BIT >> i));
                            }
                        }
                    }
                    else
                    {
                        cc1 = pSES->OldPage2->Control[firstFanElement].CommonCtrl;
                        temp1 = pSES->OldPage2->Control[firstFanElement].Ctrl.Generic.Ctrl2;

                        /*
                         * If a fan failure is detected clear the fan bit.
                         */
                        if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                        {
                            cacheEnvStats.fibreBayPSFan[bayId] &= (~(FIBRE_BAY_FAN_BIT >> 1));
                        }

                        cc1 = pSES->OldPage2->Control[firstFanElement + 1].CommonCtrl;
                        temp1 = pSES->OldPage2->Control[firstFanElement + 1].Ctrl.Generic.Ctrl2;

                        /*
                         * If a fan failure is detected clear the fan bit.
                         */
                        if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                        {
                            cacheEnvStats.fibreBayPSFan[bayId] &= (~(FIBRE_BAY_FAN_BIT >> 0));
                        }
                    }
                }
                else if (pSES->devType == PD_DT_SATA_SES)
                {
                    /*
                     * If this is a SATA bay, load the appropriate parts
                     * of the cache structure.
                     */

                    /*
                     * The SES Map and Slot arrays hold the number and
                     * type of each element.  The index into Map[] is
                     * the element type.  The value of Map[] is the offset
                     * of the first element of that type in Page2 Control
                     * Element data. The first element is the Common Control
                     * type element for the element, which will be skipped.
                     *
                     * The value of Slot[] at the same index is the number of
                     * elements of this type - 1.  This is just FYI - we don't
                     * need this value.  Yes this is all very confusing.
                     * Consult SCSI Enclosure Services docs for more
                     * information.
                     */
                    firstTempElement = pSES->Map[SES_ET_TEMP_SENSOR] + 1;

                    cacheEnvStats.sataBayTempOut1[bayId] = pSES->OldPage2->Control[firstTempElement++].Ctrl.Temp.Temp;

                    cacheEnvStats.sataBayTempOut2[bayId] = pSES->OldPage2->Control[firstTempElement].Ctrl.Temp.Temp;

                    /*
                     * 1 byte is used to indicate 4 power conditions
                     * for the SATA bays.  The code starts by assuming all
                     * elements are GOOD (1=GOOD).  If an element is determined
                     * to be bad the mask for the element is ANDed with the
                     * power byte to set the state for that element to FAIL
                     * (FAIL=0).
                     */
                    cacheEnvStats.sataBayPS[bayId] = SATA_BAY_PWR_ALL_GOOD;

                    /*
                     * Get the power supply status from the SES data.
                     * CommonCtrl indicates if the power supply exists.
                     * Ctrl2 indicates the status of the supply.
                     */
                    firstPwrElement = pSES->Map[SES_ET_POWER] + 2;

                    cc1 = pSES->OldPage2->Control[firstPwrElement].CommonCtrl;
                    temp1 = pSES->OldPage2->Control[firstPwrElement--].Ctrl.Generic.Ctrl2;
                    cc2 = pSES->OldPage2->Control[firstPwrElement].CommonCtrl;
                    temp2 = pSES->OldPage2->Control[firstPwrElement].Ctrl.Generic.Ctrl2;

                    /*
                     * Check the AC state for each supply.
                     * If CommonCtrl OR Ctrl2 indicates bad status set
                     * the FAIL flag.
                     */
                    if (CCFail(cc1) ||
                        (temp1 & (SES_C2PS_FAIL | SES_C2PS_OFF |
                                  SES_C2PS_TMPFAIL | SES_C2PS_ACFAIL)))
                    {
                        cacheEnvStats.sataBayPS[bayId] &= SATA_BAY_AC_PS2_FAIL;
                    }

                    if (CCFail(cc2) ||
                        (temp2 & (SES_C2PS_FAIL | SES_C2PS_OFF |
                                  SES_C2PS_TMPFAIL | SES_C2PS_ACFAIL)))
                    {
                        cacheEnvStats.sataBayPS[bayId] &= SATA_BAY_AC_PS1_FAIL;
                    }

                    /*
                     * Check the DC state for each supply.
                     */
                    if (CCFail(cc1) || (temp1 & SES_C2PS_DCFAIL))
                    {
                        cacheEnvStats.sataBayPS[bayId] &= SATA_BAY_DC_PS2_FAIL;
                    }

                    if (CCFail(cc2) || (temp2 & SES_C2PS_DCFAIL))
                    {
                        cacheEnvStats.sataBayPS[bayId] &= SATA_BAY_DC_PS1_FAIL;
                    }


                    /*
                     * 1 byte is used to indicate 6 fan conditions
                     * for the SATA bays.  The code starts by assuming all
                     * elements are GOOD (1=GOOD).  If an element is determined
                     * to be bad the mask for the element is ANDed with the
                     * fan byte to set the state for that element to FAIL
                     * (FAIL=0).
                     */
                    cacheEnvStats.sataBayFan[bayId] = SATA_BAY_FAN_ALL_GOOD;

                    /*
                     * Get the location of the first fan element in the
                     * SES data.  Check the status of each of the 6 fans.
                     * Look at CommonControl and Ctrl2.
                     */
                    firstFanElement = pSES->Map[SES_ET_COOLING] + 1;

                    for (i = 0; i < 3; i++)
                    {
                        cc1 = pSES->OldPage2->Control[firstFanElement + i].CommonCtrl;
                        temp1 = pSES->OldPage2->Control[firstFanElement + i].Ctrl.Generic.Ctrl2;

                        /*
                         * If a fan failure is detected clear the appropriate fan bit.
                         */
                        if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                        {
                            cacheEnvStats.sataBayFan[bayId] &= (~(SATA_BAY_FAN_BIT >> (i + 3)));
                        }
                    }

                    for (i = 0; i < 3; i++)
                    {
                        cc1 = pSES->OldPage2->Control[firstFanElement + 3 + i].CommonCtrl;
                        temp1 = pSES->OldPage2->Control[firstFanElement + 3 + i].Ctrl.Generic.Ctrl2;

                        /*
                         * If a fan failure is detected clear the appropriate fan bit.
                         */
                        if (CCFail(cc1) || (temp1 & (SES_C2FAN_FAIL | SES_C2FAN_OFF)))
                        {
                            cacheEnvStats.sataBayFan[bayId] &= (~(SATA_BAY_FAN_BIT >> i));
                        }
                    }
                }
#if defined(MODEL_7000) || defined(MODEL_4700)
                else if (pSES->devType == PD_DT_ISE_SES)
                {
                    /* Do nothing for now */
                }
#endif /* MODEL_7000 || MODEL_4700 */
                else
                {
                    dprintf(DPRINTF_DEFAULT, "Unknown bay type, ses page 2 -- %s:%u\n", __FILE__, __LINE__);
                }
            }

            /*
             * Done with this bay - go to the next one.
             */
            pSES = pSES->NextSES;
        }
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * Update the periodic performance statistics (IO/s and MB/s) and the
     * state of the FE and BE heartbeats.
     */
    cacheEnvStats.ioPerSecond = GetServerIOPerSecond();
    cacheEnvStats.mbPerSecond = ((GetServerBytesPerSecond() * 100) >> 20);
    cacheEnvStats.beHeartbeat = GetBEHeartbeatFlag();
    cacheEnvStats.feHeartbeat = GetFEHeartbeatFlag();

    /*
     * Done updating cache - set good status.
     */
    CacheStateSetUpdateDone(cacheEnvState);

    /*
     * dprintf(DPRINTF_DEFAULT, "\n\nBay environmentals\n");
     * dprintf(DPRINTF_DEFAULT, "\tFibre bay exists map\n");
     *
     * for(i = 0; i < CACHE_SIZE_DISK_BAY_MAP; ++i)
     * {
     * dprintf(DPRINTF_DEFAULT, "\t\texists %2d: 0x%2x\n", i, cacheEnvStats.fibreBayExistBitmap[i]);
     * }
     *
     * for(i = 0; i < 16; ++i)
     * {
     * dprintf(DPRINTF_DEFAULT, "\tBay %d information\n", i);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempIn1:  %d\n", cacheEnvStats.fibreBayTempIn1[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempIn2:  %d\n", cacheEnvStats.fibreBayTempIn2[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempOut1: %d\n", cacheEnvStats.fibreBayTempOut1[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempOut2: %d\n", cacheEnvStats.fibreBayTempOut2[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tPS/Fan:   %x\n", cacheEnvStats.fibreBayPSFan[i]);
     * }
     *
     * dprintf(DPRINTF_DEFAULT, "\tSATA bay exists map\n");
     *
     * for(i = 0; i < CACHE_SIZE_DISK_BAY_MAP; ++i)
     * {
     * dprintf(DPRINTF_DEFAULT, "\t\texists %2d: 0x%2x\n", i, cacheEnvStats.sataBayExistBitmap[i]);
     * }
     *
     * for(i = 0; i < 16; ++i)
     * {
     * dprintf(DPRINTF_DEFAULT, "\tBay %d information\n", i);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempOut1: %d\n", cacheEnvStats.sataBayTempOut1[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tTempOut2: %d\n", cacheEnvStats.sataBayTempOut2[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tPS:       %x\n", cacheEnvStats.sataBayPS[i]);
     * dprintf(DPRINTF_DEFAULT, "\t\tFan:      %x\n", cacheEnvStats.sataBayFan[i]);
     * }
     *
     * fprintf(stderr, "%s: EXIT (0x%x)\n", __func__, K_timel);
     */

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    RefreshFirmwareHeaders
**
** Description: Get info on all firmware headers.
**
** Inputs:      NONE
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void RefreshFirmwareHeaders(void)
{
    UINT8       count = 0;
    static const UINT16  fwTypes[] =
                { GET_CCB_RUNTIME_FW, GET_BE_RUNTIME_FW, GET_FE_RUNTIME_FW };

    for (count = 0; count < dimension_of(fwTypes); count++)
    {
        RefreshFirmwareHeader(fwTypes[count], &cacheFirmwareHeaders[count]);
    }
}

/*----------------------------------------------------------------------------
** Function:    RefreshFirmwareHeader
**
** Description: Get the firmware header info
**
** Inputs:      fwType  - Firmware type as defined in PI_Misc.h.
**                        One of the list below -
**                        GET_CCB_RUNTIME_FW
**                        GET_BE_RUNTIME_FW
**                        GET_FE_RUNTIME_FW
**
**              pRsp    -
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void RefreshFirmwareHeader(UINT16 fwType, MR_FW_HEADER_RSP *pRsp)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    INT32       rc;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pPacket = MallocWC(sizeof(PI_FW_VERSION_REQ));
    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPkt.pHeader->commandCode = PI_ADMIN_FW_VERSIONS_CMD;
    reqPkt.pHeader->length = sizeof(PI_FW_VERSION_REQ);

    /*
     * Fill in the request parms.
     */
    ((PI_FW_VERSION_REQ *)reqPkt.pPacket)->fwType = fwType;

    /*
     * Issue the command through the top-level command handler
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    /*
     * If the request was successful copy the data into the response packet.
     */
    if (rc == PI_GOOD)
    {
        memcpy(pRsp, rspPkt.pPacket, sizeof(MR_FW_HEADER_RSP));
    }

    /*
     * Free the request and response headers and packets.
     * If a timeout occurred keep the response packet around -
     * the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);

    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }
}


/*----------------------------------------------------------------------------
** Function:    RefreshVcgInfo
**--------------------------------------------------------------------------*/
INT32 RefreshVcgInfo(void)
{
    INT32       rc = PI_GOOD;
    PI_VCG_INFO_RSP *tmpVcgInfo = (PI_VCG_INFO_RSP *)cacheTempVcgInfo;
    UINT16      count = 0;
    UINT16      rspIndex = 0;
    UINT32      serialNum;
    QM_FAILURE_DATA *qmFailureData;

/*
    dprintf(DPRINTF_DEFAULT, "RefreshVcgInfo - ENTER (cacheVcgInfoState=0x%x)\n", cacheVcgInfoState);
 */

    /*
     * Clear out the temp cache.
     */
    memset(tmpVcgInfo, 0x00, CACHE_SIZE_VCG_INFO);

    tmpVcgInfo->vcgID = GetSerialNumber(SYSTEM_SN);
    tmpVcgInfo->vcgIPAddress = Qm_GetIPAddress();
    tmpVcgInfo->vcgMaxControllers = Qm_GetNumControllersAllowed();
    tmpVcgInfo->vcgCurrentControllers = 0;

    if (Qm_GetNumControllersAllowed() > 0)
    {
        /*
         * Loop through all the controllers in the controller
         * configuration map to find all necessary information
         * that is to be returned to the user.
         */
        qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
        for (count = 0; count < MAX_CONTROLLERS; count++)
        {
            /*
             * The controller configuration map is not a packed list
             * so there could be gaps, these gaps are identified by
             * a serial number of "0" and we can skip them.
             */
            if (cntlConfigMap.cntlConfigInfo[count].controllerSN == 0)
            {
                continue;
            }

            /*
             * Make sure that we have not gone past the maximum number
             * of controllers...it is a programming error if we did.
             */
            ccb_assert(rspIndex < Qm_GetNumControllersAllowed(), rspIndex);

            serialNum = cntlConfigMap.cntlConfigInfo[count].controllerSN;
            ccb_assert(serialNum != 0, serialNum);

            /* Initialize the failure data structure */
            memset(qmFailureData, 0x00, sizeof(*qmFailureData));

            /*
             * If we own drives, read up the failure state for
             * the controller.
             *
             * If we don't own drives, assume the controller is
             * operational.
             */
            if (Qm_GetOwnedDriveCount() > 0)
            {
                /*
                 * Attempt to read the failure data from the
                 * drive.
                 */
                if (ReadFailureData(serialNum, qmFailureData) != PI_GOOD)
                {
                    /*
                     * The read of the failure data from the
                     * drive failed so get the last known value
                     * from the election code (it read up the
                     * entire communications area).
                     */
                    qmFailureData->state = EL_GetFailureState(count);

                    /*
                     * If the election shows that the last known
                     * failure data was unused check if there
                     * is only one controller in the ACM and
                     * if the master controller is this controller.
                     * If that is the case make the controller
                     * operational since it seems that it is the
                     * only one in the group.
                     *
                     * NOTE: This handles the case when only one
                     *       controller is configured for the group,
                     *       drives cannot be read and the first
                     *       election has not yet occurred.
                     */
                    if (qmFailureData->state == FD_STATE_UNUSED &&
                        ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) == 1 &&
                        GetMyControllerSN() == Qm_GetMasterControllerSN())
                    {
                        qmFailureData->state = FD_STATE_OPERATIONAL;
                    }
                }

                /*
                 * If the state is something other than unused it means
                 * it is part of the CNC so add to the current controller
                 * count.
                 */
                if (qmFailureData->state > FD_STATE_UNUSED)
                {
                    tmpVcgInfo->vcgCurrentControllers++;
                }
            }
            else
            {
                if (serialNum == GetMyControllerSN())
                {
                    qmFailureData->state = FD_STATE_OPERATIONAL;
                    tmpVcgInfo->vcgCurrentControllers++;
                }
                else
                {
                    qmFailureData->state = FD_STATE_UNUSED;
                }
            }

            /* Save the controllers serial number */
            tmpVcgInfo->controllers[rspIndex].serialNumber = serialNum;

            /* Save the controllers IP address */
            tmpVcgInfo->controllers[rspIndex].ipAddress = cntlConfigMap.cntlConfigInfo[count].ipEthernetAddress;

            /* Save the controllers failure data */
            tmpVcgInfo->controllers[rspIndex].failureState = qmFailureData->state;

            /*
             * Save whether or not this controller is the master.
             * Depending on the failure state the controller may always
             * be a slave controller.
             */
            switch (qmFailureData->state)
            {
                case FD_STATE_UNUSED:
                case FD_STATE_FAILED:
                case FD_STATE_ADD_CONTROLLER_TO_VCG:
                case FD_STATE_FIRMWARE_UPDATE_INACTIVE:
                case FD_STATE_INACTIVATED:
                case FD_STATE_DISASTER_INACTIVE:
                    tmpVcgInfo->controllers[rspIndex].amIMaster = FALSE;
                    break;

                default:
                    tmpVcgInfo->controllers[rspIndex].amIMaster = TestforMaster(serialNum);
                    break;
            }

            /*
             * Increment the response index.
             */
            rspIndex++;
        }
        Free(qmFailureData);
    }

    /*
     * If we were successful in retrieving the list, swap the pointers.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Wait for good cache status before continuing.  Set the flag
         * to indicate that the cache is updating.
         */
        CacheStateWaitOkToUpdate(cacheVcgInfoState);
        CacheStateSetUpdating(cacheVcgInfoState);

        SwapPointersVcgInfo();

        CacheStateSetUpdateDone(cacheVcgInfoState);
    }

    /* dprintf(DPRINTF_DEFAULT, "RefreshVcgInfo - EXIT (rc=0x%X)\n", rc); */

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    CacheRefresh
**
** Description: Send Cache Refresh request to a controller.
**
** Inputs:      ctrlSN          - SN of controller to tunnel request to
**              cacheMask       - Cache refresh mask
**              waitForCmpl     - TRUE = remote cache refresh function will
**                                       not return until refresh is complete.
**                                FALSE = remote cache refresh function will
**                                        return immediately.
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
static INT32 CacheRefresh(UINT32 controllerSN, UINT32 cacheMask, bool waitForCmpl)
{
    XIO_PACKET  reqPkt;
    XIO_PACKET  rspPkt;
    INT32       rc = PI_ERROR;

    dprintf(DPRINTF_DEFAULT, "CacheRefresh - controllerSN=%d cacheMask=0x%X waitForCompletion=%s\n",
            controllerSN, cacheMask, (waitForCmpl ? "TRUE" : "FALSE"));

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pPacket = MallocWC(sizeof(PI_CACHE_REFRESH_CCB_REQ));
    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    rspPkt.pPacket = NULL;
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPkt.pHeader->commandCode = PI_CACHE_REFRESH_CCB_CMD;
    reqPkt.pHeader->length = sizeof(PI_CACHE_REFRESH_CCB_REQ);

    /*
     * Fill in the request parms.
     */
    ((PI_CACHE_REFRESH_CCB_REQ *)(reqPkt.pPacket))->cacheMask = cacheMask;
    ((PI_CACHE_REFRESH_CCB_REQ *)(reqPkt.pPacket))->waitForCompletion = waitForCmpl;

    /*
     * If the request is for this controller send it to the port server
     * directly.  If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
         */
        rc = PortServerCommandHandler(&reqPkt, &rspPkt);
    }
    else
    {
        UINT8   retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPkt.pPacket);
            }
            else
            {
                rspPkt.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPkt, &rspPkt);
        } while (rc != GOOD && (retries--) > 0);
    }

    /*
     * Free the request and response headers and packets.
     * If a timeout occurred keep the response packet around -
     * the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);

    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    dprintf(DPRINTF_DEFAULT, "CacheRefresh - EXIT (rc=0x%X)\n", rc);

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    CacheRefreshAllRmtCtrl
**
** Description: Tunnel a cache refresh request to all remote controllers
**
** Inputs:      ctrlSN          - SN of controller to tunnel request to
**              cacheMask       - Cache refresh mask
**              waitForCmpl     - TRUE = remote cache refresh function will
**                                       not return until refresh is complete.
**                                FALSE = remote cache refresh function will
**                                        return immediately.
**
**--------------------------------------------------------------------------*/
void CacheRefreshAllRmtCtrl(UINT32 cacheMask, bool waitForCmpl)
{
    UINT32      remoteSN = 0;
    INT16       remoteIndex = 0;
    INT32       rc;

    /*
     * Loop through all the remote controllers.
     */
    while ((remoteSN = GetNextRemoteControllerSN(&remoteIndex)) != 0)
    {
        /*
         * Send CCB Cache Refresh request to the remote controller.
         */
        rc = CacheRefresh(remoteSN, cacheMask, waitForCmpl);

        /*
         * Break out if an error occurred.
         */
        if (rc != PI_GOOD)
        {
            break;
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    RefreshBEPortInfo
**
** Description: Method to refresh the cached BE Port information.
**              Uses the Get BE Device List MRP to determine what other
**              controller(s) are on a given loop.
**
** Inputs:      none
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshBEPortInfo(void)
{
    PI_STATS_LOOP *pLoopStats = NULL;
    INT32       rc = PI_GOOD;
    UINT32      habMapBE = 0;
    UINT8       i = 0;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshBEPortInfo - ENTER\n");
     */

    if (PowerUpAllBEReady())
    {
        /*
         * The HAB map is handled as part of the Servers cache.
         * Wait until the Servers cache is not in the process of being
         * updated.  Once it is in that state make sure it is set to in
         * use so a update doesn't start while it is being used.
         */
        CacheStateWaitUpdating(cacheServersState);
        CacheStateSetInUse(cacheServersState);

        /* Get a local copy of the HAB map so the cache can be released. */
        habMapBE = cacheHABMapBE;

        /* No longer using the cache for this function... */
        CacheStateSetNotInUse(cacheServersState);

        /* Loop through all possible BE ports. */
        for (i = 0; i < MAX_BE_PORTS; i++)
        {
            /*
             * Only issue an MRP if a port really exists.
             * Derive the valid port numbers from the habMap.
             */
            if (habMapBE & (1 << i))
            {
                /*
                 * Initialize the remotePortId array for this port.
                 * The value indicates that no remote ports are seen
                 * on any controller.
                 */
                memset(cacheTempBEPortInfo[i].remotePortId, NO_REMOTE_PORT,
                       ((HACK_MAX_CONTROLLERS - 1) * sizeof(UINT16)));

                /*
                 * Determine the port on the other controller.
                 * This is actually done by looking at how we see the
                 * disk bays and comparing it to how the other CN sees
                 * the same bays.  Since remotePortId is an array
                 * cacheTempBEPortInfo[i].remotePortId is a pointer to
                 * the first element in the array.
                 */
                GetPortOnRemoteCN(i, cacheTempBEPortInfo[i].remotePortId);

                /*
                 * Wait for good cache status before continuing.  Set the flag
                 * to indicate that the cache is updating.
                 */
                CacheStateWaitOkToUpdate(cacheStatsBeState);
                CacheStateSetUpdating(cacheStatsBeState);

                /* Get a pointer from the stats cache for the input port. */
                pLoopStats = (PI_STATS_LOOP *)cacheBELoopStatsAddr[i];

                /* If the pointer is valid continue. */
                if (pLoopStats != NULL)
                {
                    cacheTempBEPortInfo[i].localPortState = pLoopStats->stats->state;
                }

                /* Done updating cache - set good status. */
                CacheStateSetUpdateDone(cacheStatsBeState);
            }
            else
            {
                /* If no BE port exists fill in default information. */
                cacheTempBEPortInfo[i].localPortState = PORT_NOTINSTALLED;

                memset(cacheTempBEPortInfo[i].remotePortId, NO_REMOTE_PORT,
                       ((HACK_MAX_CONTROLLERS - 1) * sizeof(UINT16)));
            }
        }

        /*
         * Wait for good cache status before continuing.  Set the flag
         * to indicate that the cache is updating.
         */
        CacheStateWaitOkToUpdate(cacheBEPortState);
        CacheStateSetUpdating(cacheBEPortState);

        /* Swap the pointers. */
        SwapPointersBEPortInfo();

        memset(cacheTempBEPortInfo, 0x00, CACHE_SIZE_BE_PORT_INFO);

        /* Done updating cache - set good status. */
        CacheStateSetUpdateDone(cacheBEPortState);
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshBEPortInfo - EXIT\n");
     */

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    RefreshBEDevicePaths
**
** Description: Method to refresh the cached BE device paths information.
**
** Inputs:      type -  PATH_PHYSICAL_DISK or
**                      PATH_ENCLOSURES (Disk Bays)
**--------------------------------------------------------------------------*/
void RefreshBEDevicePaths(UINT16 type)
{
    MRGETBEDEVPATHS_REQ *pMRPInPkt = NULL;
    MRGETBEDEVPATHS_RSP *pMRPOutPkt = NULL;
    INT32       rc;
    MRGETBEDEVPATHS_RSP_ARRAY *pMRPOutPath = NULL;
    MRGETBEDEVPATHS_RSP *pPathInfo = NULL;
    MRGETBEDEVPATHS_RSP_ARRAY *pPathList = NULL;
    UINT16      id;
    UINT16      i;

    /* dprintf(DPRINTF_CACHE_REFRESH, "RefreshBEDevicePaths - ENTER\n"); */

    if (PowerUpBEReady())
    {
        /*
         * Allocate the input packet used to retrieve the
         * information from the PROC.  Fill in the input parms
         */
        pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));
        pMRPInPkt->type = type;
        pMRPInPkt->format = FORMAT_PID_PATH_ARRAY;

        /*
         * Allocate an MRP output packet for the largest possible
         * response packet.
         */
        pMRPOutPkt = MallocSharedWC(CACHE_SIZE_PDISK_PATHS);

        /*
         * Try forever to get this MRP through.
         */
        rc = PI_ExecMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETBEDEVPATHS,
                        pMRPOutPkt, CACHE_SIZE_PDISK_PATHS, CACHE_MAX_REFRESH_MRP_TIMEOUT);

        /*
         * If the request was successful copy the data into the cache.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Copy the response data into the proper cache buffer.
             */
            if (type == PATH_ENCLOSURES)
            {
                /*
                 * Data is for a Disk Bay -
                 * Wait for good cache status before continuing.  Set the flag
                 * to indicate that the cache is updating.
                 */
                CacheStateWaitOkToUpdate(cacheDiskBaysState);
                CacheStateSetUpdating(cacheDiskBaysState);

                /*
                 * Clear the cache.
                 */
                memset(cacheDiskBayPaths, 0xFF, CACHE_SIZE_DISK_BAY_PATHS);

                /*
                 * Copy the header portion of the data into the cache.
                 */
                memcpy(cacheDiskBayPaths, pMRPOutPkt, sizeof(*pMRPOutPkt));

                /*
                 * Get a pointer to the start of the Disk Bay path cache and
                 * to the start of the array of path lists in the cache.
                 */
                pPathInfo = (MRGETBEDEVPATHS_RSP *)cacheDiskBayPaths;
                pPathList = (MRGETBEDEVPATHS_RSP_ARRAY *)(pPathInfo->list);

                /*
                 * Walk through the MRP response data and place it in the
                 * cache so that it is indexed by ID.  This takes
                 * some time now but will provide users quicker access
                 * to the data.
                 */
                for (i = 0; i < pMRPOutPkt->ndevs; i++)
                {
                    /*
                     * Get a pointer to the path info for the current item.
                     */
                    pMRPOutPath = (MRGETBEDEVPATHS_RSP_ARRAY *)(pMRPOutPkt->list);

                    /*
                     * Get the id of the current item.
                     */
                    id = pMRPOutPath[i].pid;

                    /*
                     * Copy the current item from the output packet into
                     * the cache at the location determined by its ID.
                     */
                    memcpy(&(pPathList[id]), &(pMRPOutPath[i]), sizeof(MRGETBEDEVPATHS_RSP_ARRAY));
                }

                /*
                 * Done updating cache - set good status.
                 */
                CacheStateSetUpdateDone(cacheDiskBaysState);
            }
            else if (type == PATH_PHYSICAL_DISK)
            {
                /*
                 * Data is for a Physical Disk -
                 * Wait for good cache status before continuing.  Set the flag
                 * to indicate that the cache is updating.
                 */
                CacheStateWaitOkToUpdate(cachePhysicalDisksState);
                CacheStateSetUpdating(cachePhysicalDisksState);

                /*
                 * Clear the cache.
                 */
                memset(cachePDiskPaths, 0xFF, CACHE_SIZE_PDISK_PATHS);

                /*
                 * Copy the header portion of the data into the cache.
                 */
                memcpy(cachePDiskPaths, pMRPOutPkt, sizeof(*pMRPOutPkt));

                /*
                 * Get a pointer to the start of the PDisk path cache and
                 * to the start of the array of path lists in the cache.
                 */
                pPathInfo = (MRGETBEDEVPATHS_RSP *)cachePDiskPaths;
                pPathList = (MRGETBEDEVPATHS_RSP_ARRAY *)(pPathInfo->list);

                /*
                 * Walk through the MRP response data and place it in the
                 * cache so that it is indexed by ID.  This takes
                 * some time now but will provide users quicker access
                 * to the data.
                 */
                for (i = 0; i < pMRPOutPkt->ndevs; i++)
                {
                    /*
                     * Get a pointer to the path info in the MRP output.
                     */
                    pMRPOutPath = (MRGETBEDEVPATHS_RSP_ARRAY *)(pMRPOutPkt->list);

                    /*
                     * Get the id of the current item.
                     */
                    id = pMRPOutPath[i].pid;

                    /*
                     * Copy the current item from the output packet into
                     * the cache at the location determined by its ID.
                     */
                    memcpy(&(pPathList[id]), &(pMRPOutPath[i]), sizeof(MRGETBEDEVPATHS_RSP_ARRAY));
                }

                /*
                 * Done updating cache - set good status.
                 */
                CacheStateSetUpdateDone(cachePhysicalDisksState);
            }
        }
        else
        {
            /*
             * Some sort of error occurred.
             */
            dprintf(DPRINTF_DEFAULT, "RefreshBEDevicePaths - Error returned from MRGETBEDEVPATHS (0x%x)\n", rc);
        }

        /*
         * Free the input packet and the output packet if the request
         * did not timeout.
         */
        Free(pMRPInPkt);

        if (rc != PI_TIMEOUT)
        {
            Free(pMRPOutPkt);
        }
    }
    /* dprintf(DPRINTF_CACHEMGR, "RefreshBEDevicePaths - EXIT\n"); */
}


/*----------------------------------------------------------------------------
** Function:    RefreshWorksets
**
** Description: Get the workset info
**
** Inputs:      none
**
** Returns:     PI_GOOD, PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 RefreshWorksets(void)
{
    MRGETWSINFO_REQ *pMRPInPkt = NULL;
    MRGETWSINFO_RSP *pTmpPtr = NULL;
    INT32       rc = PI_ERROR;

    dprintf(DPRINTF_CACHE_REFRESH, "RefreshWorksets - ENTER\n");

    /*
     * Need to wait for back end ready before issuing this request.
     */
    if (PowerUpBEReady())
    {
        /*
         * Allocate the input packet used to retrieve the
         * information from the PROC.
         */
        pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

        /*
         * Fill in the request parms to get all worksets.
         */
        pMRPInPkt->id = GET_ALL_WORKSETS;

        /*
         * Point to the temporary workset buffer.
         */
        pTmpPtr = (MRGETWSINFO_RSP *)cacheTempWorksetInfo;

        /*
         * Try forever to get this MRP through.
         */
        rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETWSINFO,
                           pTmpPtr, CACHE_SIZE_WORKSET_INFO,
                           CACHE_MAX_REFRESH_MRP_TIMEOUT,
                           PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

        /*
         * If the request was successful, swap the cache pointers so
         * subsequent requests to the cache get the new data.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Wait for good cache status before continuing.  Set the flag
             * to indicate that the cache is updating.
             */
            CacheStateWaitOkToUpdate(cacheWorksetState);
            CacheStateSetUpdating(cacheWorksetState);

            /*
             * Swap the pointers.
             */
            SwapPointersWorksets();

            /*
             * Done updating cache - set good status.
             */
            CacheStateSetUpdateDone(cacheWorksetState);
        }
        else
        {
            /*
             * Some sort of error occurred.
             */
            dprintf(DPRINTF_DEFAULT, "RefreshWorksets MRP error: status=0x%02hhX\n",
                    pTmpPtr->header.status);
        }

        /*
         * Free the MRP request packets.
         */
        Free(pMRPInPkt);
    }

    dprintf(DPRINTF_CACHE_REFRESH, "RefreshWorksets - EXIT\n");

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Update the cached free space data for each RAID type
**
**  @param      X1_CONFIG_ACCOUNT* pAccount - pointer to the account
**                                            information to use when
**                                            calculating the free
**                                            space.
**
**  @return     none
**
**  @attention  This function will attempt to update the cached free
**              space information if the given RAID type has a valid
**              physical disk list.  If not the value will remain
**              if the
**
******************************************************************************
**/
INT32 CacheFreeSpaceRefresh(void)
{
    /* Need to wait for back end ready before issuing this request. */
    if (!PowerUpBEReady())
    {
        return PI_ERROR;
    }

    /*
     * Wait for good cache status before continuing.  Set the flag
     * to indicate that the cache is updating.
     */
    CacheStateWaitOkToUpdate(cacheFreeSpaceState);
    CacheStateSetUpdating(cacheFreeSpaceState);

    memset(&cacheFreeSpace, 0, sizeof(cacheFreeSpace));

    /* Done updating cache - set good status. */
    CacheStateSetUpdateDone(cacheFreeSpaceState);

    return PI_GOOD;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
