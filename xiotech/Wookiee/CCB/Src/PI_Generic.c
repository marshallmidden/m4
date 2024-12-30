/* $Id: PI_Generic.c 156535 2011-06-24 23:13:05Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_Generic.c
** MODULE TITLE:    Packet Interface for Generic Command
**
** DESCRIPTION:     Handler function for the "generic" command code.
**                  This feature allows other commands to be added by
**                  placing code in the function below
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include <arpa/inet.h>
#include "CachePDisk.h"
#include "ccb_hw.h"
#include "convert.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "EL_Disaster.h"
#include "EL_KeepAlive.h"
#include "error_handler.h"
#include "FCM_Counters.h"
#include "FIO_Maps.h"
#include "fm.h"
#include "FW_Header.h"
#include "idr_structure.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "misc.h"
#include "MR_Defs.h"
#include "mutex.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PacketStats.h"
#include "PI_CmdHandlers.h"
#include "PktCmdHdl.h"
#include "PI_Misc.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "PortServerUtils.h"
#include "PortServer.h"
#include "quorum_utils.h"
#include "rm.h"
#include "rtc.h"
#include "ses.h"
#include "trace.h"
#include "timer.h"
#include "XIO_Std.h"
#include "X1_AsyncEventHandler.h"
#include "X1_Structs.h"

#include "xk_init.h"

#include <byteswap.h>

/*****************************************************************************
** Private variables
*****************************************************************************/
static bool testCache = false;

/*****************************************************************************
** Public variables not externed in a header file
*****************************************************************************/
const char *procName[] = { "CCB", "FE", "BE" };

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 GetSosStructure(UINT16 pid, XIO_PACKET *pRspPacket);
static INT32 DisableHeartbeats(void);
static UINT32 GetPacketStats(int proc);
static void GenericTestCache(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_GenericCommand
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
#define GenericParm(num) (((PI_GENERIC_REQ *)(pReqPacket->pPacket))->data[(num)])
INT32 PI_GenericCommand(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_GENERIC_RSP *ptrOutPkt = NULL;
    UINT32      responseLength;
    INT32       rc = PI_GOOD;
    TASK_PARMS  parms;

    /* Allocate space for response data as required. */
    responseLength = ((PI_GENERIC_REQ *)(pReqPacket->pPacket))->responseLength;

    if (responseLength > 0)
    {
        ptrOutPkt = MallocWC(responseLength);
    }

    /*
     * If the memory allocation was successful or no response memory is
     * required check the "sub" command code for the function request.
     */
    if ((ptrOutPkt != NULL) || (responseLength == 0))
    {
        switch (((PI_GENERIC_REQ *)(pReqPacket->pPacket))->subCmdCode)
        {
            case PI_GENERIC_RESET:
                switch (GenericParm(1))
                {
                    case 0:    /* immediate / non-forked */
                        ProcessReset(GenericParm(0));
                        break;

                    default:   /* delayed / forked */
                        parms.p1 = (UINT32)GenericParm(0);
                        TaskCreate(ProcessResetTask, &parms);
                        break;
                }
                break;

            case PI_GENERIC_GLOBAL_MRP_TIMEOUT:
                SetGlobalMRPTimeout(GenericParm(0));
                break;

            case PI_GENERIC_FUNCTION_CALL:
                rc = PI_GenericFunc(GenericParm(0),
                                    GenericParm(1),
                                    GenericParm(2),
                                    GenericParm(3),
                                    GenericParm(4),
                                    GenericParm(5), GenericParm(6), GenericParm(7));
                break;

            case PI_GENERIC_DEBUG_ADDRESS:
                /* Valid channels 0 - 19 */
                if (GenericParm(1) >= DEBUGCON_MAX_CHANNELS)
                {
                    rc = PI_PARAMETER_ERROR;
                }

                if (rc == PI_GOOD)
                {
                    const char *strP = NULL;
                    char       *userIP = NULL;
                    struct sockaddr_in sa;
                    UINT32      len = sizeof(sa);
                    char        newIP[20] = { '?' };

                    switch (GenericParm(0))
                    {
                        case 0:
                            strP = "OFF";
                            break;

                        case 0xFFFFFFFF:
                            strP = "PI (packet intf)";
                            break;

                        default:
                            sa.sin_addr.s_addr = GenericParm(0);
                            strP = inet_ntoa(sa.sin_addr);
                            if (strP)
                            {
                                strcpy(newIP, strP);
                            }
                            strP = newIP;
                            break;
                    }

                    memset(&sa, 0, sizeof(sa));
                    getpeername(pReqPacket->pHeader->socket, (struct sockaddr *)&sa, &len);
                    userIP = inet_ntoa(sa.sin_addr);

                    dprintf(DPRINTF_DEFAULT, "1:DebugConsole IP set to %s / %d, by user at IP %s\n",
                            strP, GenericParm(1), userIP);

                    if (GetDebugConsoleIPAddr() != GenericParm(0) ||
                        GetDebugConsoleChannel() != GenericParm(1))
                    {

                        /*
                         * Allow the UDP eth writer time to send out first msg to the
                         * old address. Only delay if there is a change.
                         * Note: If we are way behind in the queue, this might
                         * not work as expected, but it should most of the time.
                         */
                        TaskSleepMS(500);

                        /* Now change the address */
                        SetDebugConsoleIPAddr(GenericParm(0));
                        SetDebugConsoleChannel(GenericParm(1));

                        /* Send it again on the new IP address / channel. */
                        dprintf(DPRINTF_DEFAULT, "2:DebugConsole IP set to %s / %d, by user at IP %s\n",
                                strP, GenericParm(1), userIP);
                    }
                }
                break;

            case PI_GENERIC_GLOBAL_PI_SELECT_TIMEOUT:
                SetSelectTimeoutBySocket(pReqPacket->pHeader->socket, GenericParm(0));
                break;

            case PI_GENERIC_DO_ELECTION:
                dprintf(DPRINTF_DEFAULT, "Election being called via packet interface\n");
                rc = EL_DoElectionNonBlocking();
                break;

            case PI_GENERIC_GLOBAL_IPC_TIMEOUT:
                SetGlobalIPCTimeout(GenericParm(0));
                break;

            case PI_GENERIC_DISABLE_HEARTBEATS:
                rc = DisableHeartbeats();
                break;

            case PI_GENERIC_ERROR_TRAP:
                if (GenericParm(0) == PI_GENERIC_ERROR_TRAP_FE ||
                    GenericParm(0) == PI_GENERIC_ERROR_TRAP_BE ||
                    GenericParm(0) == PI_GENERIC_ERROR_TRAP_ALL)
                {
                    /* Allocate the return packet */
                    if (ptrOutPkt == NULL)
                    {
                        responseLength = sizeof(*ptrOutPkt);
                        ptrOutPkt = MallocSharedWC(responseLength);
                    }

                    /* Check that we had good allocation */
                    if (ptrOutPkt == NULL)
                    {
                        rc = PI_MALLOC_ERROR;
                        responseLength = 0;
                    }

                    if (rc == PI_GOOD)
                    {
                        if (GenericParm(0) == PI_GENERIC_ERROR_TRAP_BE ||
                            GenericParm(0) == PI_GENERIC_ERROR_TRAP_ALL)
                        {
                            dprintf(DPRINTF_DEFAULT, "MANUAL BACK END ERROR TRAP!\n");

                            /* Kill the back end */
                            rc = PI_ExecMRP(NULL, 0, MRFORCEBEETRAP,
                                            ptrOutPkt, sizeof(*ptrOutPkt),
                                            GetGlobalMRPTimeout());
                        }

                        if (GenericParm(0) == PI_GENERIC_ERROR_TRAP_FE ||
                            GenericParm(0) == PI_GENERIC_ERROR_TRAP_ALL)
                        {
                            dprintf(DPRINTF_DEFAULT, "MANUAL FRONT END ERROR TRAP!\n");

                            /* Kill the front end */
                            rc = PI_ExecMRP(NULL, 0, MRFORCEFEETRAP,
                                            ptrOutPkt, sizeof(*ptrOutPkt),
                                            GetGlobalMRPTimeout());
                        }
                    }
                }
                else if (GenericParm(0) == PI_GENERIC_ERROR_TRAP_CCB ||
                         GenericParm(0) == PI_GENERIC_ERROR_TRAP_ALL)
                {
                    dprintf(DPRINTF_DEFAULT, "MANUAL CCB ERROR TRAP!\n");

                    /* Kill the CCB. *
                     * NOTE: This isn't really a suicide... we're just wanting
                     * to errortrap the CCB process.
                     */
                    DeadLoop(EVENT_MANUAL_SUICIDE, TRUE);
                }
                break;

            case PI_GENERIC_GET_SOS_STRUCTURE:
                /* Get the Sos Stucture */
                rc = GetSosStructure(((UINT16)GenericParm(0)), pRspPacket);

                /*
                 * The response packet will be generated in GetSosStructure.
                 * We need to return here so the response is not destroyed
                 * below.
                 */
                return rc;

            case PI_GENERIC_SET_PDISK_LED:
                {
                    IPC_LED_CHANGE *pEvent = MallocWC(sizeof(*pEvent));
                    UINT64 wwn1 = bswap_32(GenericParm(1));
                    UINT64 wwn2 = bswap_32(GenericParm(0));

                    wwn1 = (wwn2 | (wwn1 << 32));
                    pEvent->ledReq = GenericParm(2);
                    pEvent->serialNum = GetMyControllerSN();
                    memcpy(&pEvent->wwn, &wwn1, sizeof(UINT64));

                    /* pEvent will be freed when taken off the queue */
                    EnqueueToLedControlQueue(pEvent);
                    break;
                }

            case PI_GENERIC_SEND_LOG_EVENT:
                {
                    LOG_MRP     logEvent;
                    UINT8      *data = (UINT8 *)(((PI_GENERIC_REQ *)(pReqPacket->pPacket))->data);

                    memcpy((UINT8 *)&logEvent, data, 256);

                    LogEvent(&logEvent);
                    break;
                }

            case PI_GENERIC_CACHE_TEST:
                if (GenericParm(0) == PI_GENERIC_CACHE_TEST_START)
                {
                    if ((GenericParm(1) == PI_GENERIC_CACHE_TEST_CACHE) ||
                        (GenericParm(1) == PI_GENERIC_CACHE_TEST_ASYNC) ||
                        (GenericParm(1) == PI_GENERIC_CACHE_TEST_X1))
                    {
                        if (GenericParm(1) != PI_GENERIC_CACHE_TEST_ASYNC)
                        {
                            testCache = true;
                        }
                        parms.p1 = (UINT32)GenericParm(1);
                        parms.p2 = (UINT32)GenericParm(2);
                        TaskCreate(GenericTestCache, &parms);
                    }
                }
                else
                {
                    testCache = false;
                }
                break;

            case PI_GENERIC_DISASTER_TEST:
                {
                    DISASTER_DATA disasterData;

                    rc = PI_GOOD;

                    EL_DisasterGetFlags(&disasterData.flags);

                    switch (GenericParm(0))
                    {
                        case PI_GENERIC_DISASTER_TEST_RESET:
                            NVRAM_DisasterDataReset(&disasterData);
                            break;

                        case PI_GENERIC_DISASTER_TEST_CLEAR:
                            disasterData.flags.bits.disasterDetected = FALSE;
                            break;

                        case PI_GENERIC_DISASTER_TEST_SET:
                            disasterData.flags.bits.disasterDetected = TRUE;
                            EL_DisasterSetString("Generated by CCBE");
                            break;

                        default:
                            dprintf(DPRINTF_DEFAULT, "Unknown disaster test flag: %d\n", GenericParm(0));
                            rc = PI_PARAMETER_ERROR;
                            break;
                    }

                    if (rc == PI_GOOD)
                    {
                        rc = EL_DisasterSetFlags(&disasterData.flags);
                    }
                }
                break;

            case PI_GENERIC_KEEP_ALIVE_TEST:
                {
                    rc = PI_GOOD;

                    if (TestforMaster(GetMyControllerSN()) == true)
                    {
                        switch (GenericParm(0))
                        {
                            case PI_GENERIC_KEEP_ALIVE_TEST_RESET:
                                dprintf(DPRINTF_DEFAULT, "KeepAlive system reset\n");
                                EL_KeepAliveSystemReset();
                                break;

                            case PI_GENERIC_KEEP_ALIVE_TEST_CLEAR:
                                dprintf(DPRINTF_DEFAULT, "keepAlive slot %d clear\n", GenericParm(1));
                                EL_KeepAliveSetSlotValid(GenericParm(1), FALSE);
                                break;

                            case PI_GENERIC_KEEP_ALIVE_TEST_SET:
                                dprintf(DPRINTF_DEFAULT, "keepAlive slot %d set\n", GenericParm(1));
                                EL_KeepAliveSetSlotValid(GenericParm(1), TRUE);
                                break;

                            case PI_GENERIC_KEEP_ALIVE_TEST_DISABLE:
                                dprintf(DPRINTF_DEFAULT, "KeepAlive system disabled\n");
                                EL_KeepAliveSystemDisable();
                                break;

                            case PI_GENERIC_KEEP_ALIVE_TEST_ENABLE:
                                dprintf(DPRINTF_DEFAULT, "KeepAlive system enabled\n");
                                EL_KeepAliveSystemEnable();
                                break;

                            default:
                                dprintf(DPRINTF_DEFAULT, "Unknown keepAlive test flag: %d\n",
                                        GenericParm(0));
                                rc = PI_PARAMETER_ERROR;
                                break;
                        }

                        /*
                         * Check to see if the other controllers need to update
                         * their copy of the masterConfig.
                         */
                        if (rc == PI_GOOD)
                        {
                            /*
                             * Check to see that we own drives before attempting to write
                             * the new master configuration.
                             */
                            if (Qm_GetOwnedDriveCount() > 0)
                            {
                                SaveMasterConfig();
                            }

                            /* Tell any remaining slaves that the masterConfig has changed. */
                            dprintf(DPRINTF_DEFAULT, "Signal slaves - masterConfig change\n");
                            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_DEFAULT, "Not master - keepAlive test failed\n");
                        rc = PI_MASTER_CNT_ERROR;
                    }
                }
                break;

            case PI_GENERIC_FIO_MAP_TEST:
                {
                    rc = PI_GOOD;

                    /* Validate parameter 0 */
                    switch (GenericParm(0))
                    {
                        case PI_GENERIC_FIO_MAP_TEST_READ:
                        case PI_GENERIC_FIO_MAP_TEST_WRITE:
                            break;

                        default:
                            dprintf(DPRINTF_DEFAULT, "Unknown file I/O map test parameter 0: %d\n",
                                    GenericParm(0));
                            rc = PI_PARAMETER_ERROR;
                            break;
                    }

                    if (rc == PI_GOOD)
                    {
                        FIO_DISK_MAP diskMap;   /* Uninitialized */

                        switch (GenericParm(1))
                        {
                            case PI_GENERIC_FIO_MAP_TEST_RESET:
                                if (GenericParm(0) == PI_GENERIC_FIO_MAP_TEST_READ)
                                {
                                    dprintf(DPRINTF_DEFAULT, "Resetting readable diskMap\n");
                                    rc = FIO_ResetReadableDiskMap();
                                }
                                else
                                {
                                    dprintf(DPRINTF_DEFAULT, "Resetting writable diskMap\n");
                                    rc = FIO_ResetWritableDiskMap();
                                }
                                break;

                            case PI_GENERIC_FIO_MAP_TEST_CLEAR:
                                if (GenericParm(0) == PI_GENERIC_FIO_MAP_TEST_READ)
                                {
                                    dprintf(DPRINTF_DEFAULT, "Clearing readable diskMap\n");
                                    memset(&diskMap, 0, sizeof(diskMap));
                                    rc = FIO_SetReadableDiskMap(&diskMap);
                                }
                                else
                                {
                                    dprintf(DPRINTF_DEFAULT, "Clearing writable diskMap\n");
                                    memset(&diskMap, 0, sizeof(diskMap));
                                    rc = FIO_SetWritableDiskMap(&diskMap);
                                }
                                break;

                            case PI_GENERIC_FIO_MAP_TEST_SET:
                                if (GenericParm(0) == PI_GENERIC_FIO_MAP_TEST_READ)
                                {
                                    if (GenericParm(2) < MAX_PHYSICAL_DISKS)
                                    {
                                        dprintf(DPRINTF_DEFAULT, "Setting readable diskMap bit %d\n",
                                                GenericParm(2));
                                        rc = FIO_GetReadableDiskMap(&diskMap);
                                        if (rc == GOOD)
                                        {
                                            diskMap[GenericParm(2) / (sizeof(diskMap[0]) * 8)] |=
                                                1 << (GenericParm(2) % (sizeof(diskMap[0]) * 8));

                                            rc = FIO_SetReadableDiskMap(&diskMap);
                                        }
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_DEFAULT, "File I/O map test parameter 2: %d out-of-range\n",
                                                GenericParm(2));
                                        rc = PI_PARAMETER_ERROR;
                                    }
                                }
                                else
                                {
                                    if (GenericParm(2) < MAX_PHYSICAL_DISKS)
                                    {
                                        dprintf(DPRINTF_DEFAULT, "Setting writable diskMap bit %d\n",
                                                GenericParm(2));
                                        rc = FIO_GetWritableDiskMap(&diskMap);
                                        if (rc == GOOD)
                                        {
                                            diskMap[GenericParm(2) / (sizeof(diskMap[0]) * 8)] |=
                                                1 << (GenericParm(2) % (sizeof(diskMap[0]) * 8));

                                            rc = FIO_SetWritableDiskMap(&diskMap);
                                        }
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_DEFAULT, "File I/O map test parameter 2: %d out-of-range\n",
                                                GenericParm(2));
                                        rc = PI_PARAMETER_ERROR;
                                    }
                                }
                                break;

                            default:
                                dprintf(DPRINTF_DEFAULT, "Unknown file I/O map test parameter 1: %d\n",
                                        GenericParm(1));
                                rc = PI_PARAMETER_ERROR;
                                break;
                        }
                    }
                }
                break;

            case PI_GENERIC_FCM_COUNTER_TEST:
                {
                    /* Validate parameter 0 */
                    switch (GenericParm(0))
                    {
                        case PI_GENERIC_FCM_COUNTER_TEST_DUMP:
                            dprintf(DPRINTF_DEFAULT, "Dump FCM counters\n");
                            if (FCM_CountersDumpMap(&counterMap) == GOOD)
                            {
                                dprintf(DPRINTF_DEFAULT, "FCM counters dumped\n");
                                rc = PI_GOOD;
                            }
                            else
                            {
                                dprintf(DPRINTF_DEFAULT, "Error while dumping FCM counters\n");
                                rc = PI_ERROR;
                            }
                            break;

                        case PI_GENERIC_FCM_COUNTER_TEST_BASELINE:
                            dprintf(DPRINTF_DEFAULT, "Get baseline FCM counters\n");
                            if ((FCM_CountersUpdateMap(&counterMap) == GOOD) &&
                                (FCM_CountersBaselineMap(&counterMap) == GOOD))
                            {
                                dprintf(DPRINTF_DEFAULT, "Baseline FCM counters retrieved\n");
                                rc = PI_GOOD;
                            }
                            else
                            {
                                dprintf(DPRINTF_DEFAULT, "Error while getting baseline FCM counters\n");
                                rc = PI_ERROR;
                            }
                            break;

                        case PI_GENERIC_FCM_COUNTER_TEST_UPDATE:
                            dprintf(DPRINTF_DEFAULT, "Update FCM counters\n");
                            if (FCM_CountersUpdateMap(&counterMap) == GOOD)
                            {
                                dprintf(DPRINTF_DEFAULT, "FCM counters updated\n");
                                rc = PI_GOOD;
                            }
                            else
                            {
                                dprintf(DPRINTF_DEFAULT, "Error while updating FCM counters\n");
                                rc = PI_ERROR;
                            }
                            break;

                        case PI_GENERIC_FCM_COUNTER_TEST_DELTA:
                            dprintf(DPRINTF_DEFAULT, "Calculate FCM counters - delta\n");
                            if (FCM_CountersDeltaMap(&counterMap) == GOOD)
                            {
                                dprintf(DPRINTF_DEFAULT, "FCM delta calculated\n");
                                rc = PI_GOOD;
                            }
                            else
                            {
                                dprintf(DPRINTF_DEFAULT, "Error while calculating FCM delta\n");
                                rc = PI_ERROR;
                            }
                            break;

                        case PI_GENERIC_FCM_COUNTER_TEST_MAJOR_EVENT:
                            dprintf(DPRINTF_DEFAULT, "Generate major FCAL storage event\n");
                            FCM_CountersMajorStorageEvent();
                            dprintf(DPRINTF_DEFAULT, "FCM MajorStorageEvent Processed\n");
                            rc = PI_GOOD;
                            break;

                        case PI_GENERIC_FCM_COUNTER_TEST_MINOR_EVENT:
                            dprintf(DPRINTF_DEFAULT, "Generate minor FCAL storage event\n");
                            FCM_CountersMinorStorageEvent();
                            dprintf(DPRINTF_DEFAULT, "FCM MinorStorageEvent Processed\n");
                            rc = PI_GOOD;
                            break;

                        default:
                            dprintf(DPRINTF_DEFAULT, "Unknown file I/O map test parameter 0: %d\n",
                                    GenericParm(0));
                            rc = PI_PARAMETER_ERROR;
                            break;
                    }
                }
                break;

            default:
                pRspPacket->pHeader->status = PI_INVALID_CMD_CODE;
                rc = PI_INVALID_CMD_CODE;
                break;
        }
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = responseLength;
    pRspPacket->pHeader->status = rc;

    if (ptrOutPkt)
    {
        pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;
    }
    else
    {
        pRspPacket->pHeader->errorCode = 0;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_GenericMRP
**
** Description: Allow a generic MRP to be executed and results returned
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_GenericMRP(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_GENERIC_RSP *ptrOutPkt = NULL;
    INT32       rc = PI_GOOD;
    PI_GENERIC_MRP_REQ *reqP = (PI_GENERIC_MRP_REQ *)(pReqPacket->pPacket);

    /* Allocate memory for the MRP return data. */
    ptrOutPkt = MallocSharedWC(reqP->responseLength);

    if (ptrOutPkt != NULL)
    {
        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP((reqP->inputLength > 0) ? reqP->data : NULL,
                        reqP->inputLength, reqP->mrpCmd,
                        ptrOutPkt, reqP->responseLength, GetGlobalMRPTimeout());

        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = reqP->responseLength;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;
    }
    else
    {
        /* If the memory allocation failed return an error indication. */
        rc = PI_MALLOC_ERROR;

        pRspPacket->pPacket = NULL;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = 0;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_Generic2Command
**
** Description: Same as the PI_GenericCommand() handler, except the response
**              is sent from here instead of from PacketInterfaceServer().
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Generic2Command(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       socketRC;
    INT32       len = 0;

    /*
     * Lock the static "Big Buffer" mutex so that we can use it
     * can receive the firmware into it.
     */
    (void)LockMutex(&bigBufferMutex, MUTEX_WAIT);

    switch (((PI_GENERIC_REQ *)(pReqPacket->pPacket))->subCmdCode)
    {
        case PI_GENERIC2_GET_HEAP_STATS:
            len = GetHeapDumpStats(GenericParm(0));
            break;

        case PI_GENERIC2_GET_TRACE_STATS:
            len = GetTraceDump(GenericParm(0));
            break;

        case PI_GENERIC2_GET_PCB_STATS:
            len = GetPCBDump(GenericParm(0));
            break;

        case PI_GENERIC2_GET_PROFILE_STATS:
            len = GetProfileDump(GenericParm(0));
            break;

        case PI_GENERIC2_GET_PACKET_STATS:
            len = GetPacketStats(GenericParm(0));
            break;

        default:
            pRspPacket->pHeader->status = PI_INVALID_CMD_CODE;
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)gBigBuffer;
    pRspPacket->pHeader->length = len;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    /* Send the packet */
    socketRC = SendPacket(pReqPacket->pHeader->socket, pRspPacket,
                          GetSelectTimeoutBySocket(pReqPacket->pHeader->socket));

    /*
     * Set the pPacket pointer back to NULL so that it doesn't get
     * "free'd" by PacketInterfaceServer().
     */
    pRspPacket->pPacket = NULL;

    /* Free the "Big Buffer" */
    UnlockMutex(&bigBufferMutex);

    /* If anything failed, return a non-zero return code */
    if (rc != PI_GOOD || socketRC == PI_SOCKET_ERROR)
    {
        return PI_ERROR;
    }
    return PI_GOOD;
}


/*============================================================================
**  SECTION:        Heap Statistics and Memory Leak Detection Routines
**
**  DESCRIPTION:    Contains Heap Statistics and Memory Leak Detection
**                  functions.
**==========================================================================*/

/**********************************************************************
*                                                                     *
*  Name:        AddFirmwareHdrData()                                  *
*                                                                     *
*  Description: Add a common firmware header data section to the      *
*               sent out reports.                                     *
*                                                                     *
*  Input:       outP - Pointer to the buffer where the data is written*
*                                                                     *
*  Returns:     number of bytes written.                              *
*                                                                     *
**********************************************************************/
typedef struct
{
    char        revision[4];    /* Firmware Revision                        */
    char        revCount[4];    /* Firmware Revision Counter                */
    char        buildID[4];     /* Who / where firmware was built           */
    FW_HEADER_TIMESTAMP timeStamp;      /* Time Firmware was built              */
} GEN_HDR;

static UINT32 AddFirmwareHdrData(void *outP)
{
    GEN_HDR    *bufP = (GEN_HDR *)outP;
    FW_HEADER  *ccbP = (FW_HEADER *)CCBRuntimeFWHAddr;

    memcpy(bufP->revision, &ccbP->revision, 12);
    memcpy(&bufP->timeStamp, &ccbP->timeStamp, sizeof(bufP->timeStamp));

    return (sizeof(*bufP));
}


/**********************************************************************
*                                                                     *
*  Name:        GetHeapDumpStats()                                    *
*                                                                     *
*  Description: Search for all heap allocations (from C) and assemble *
*               the list in the "big" buffer. This is called via the  *
*               Generic2 handler above.                               *
*                                                                     *
*  Input:       proc - processor to process (only CCB at this time)   *
*                                                                     *
*  Returns:     length in bytes of assembled data.                    *
*                                                                     *
**********************************************************************/
UINT32 GetHeapDumpStats(UNUSED int proc)
{
    char       *bufP = gBigBuffer;

    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     *
     * First copy in the firmware header information - this should pertain
     * to the selected processor, when/if that feature is implemented.
     * This data is common to all of the reports that are sent out.
     */
    bufP += AddFirmwareHdrData(bufP);

    /* Go get the heap dump stuff */
    CompileHeapStats();
    memcpy(bufP, &heapStats, sizeof(HEAP_STATS));
    bufP += sizeof(HEAP_STATS);

    return (INT8 *)bufP - (INT8 *)gBigBuffer;
}


/**********************************************************************
*                                                                     *
*  Name:        GetTraceDump()                                        *
*                                                                     *
*  Description: Dump out the trace buffer.  It is post-processed      *
*               by the client.                                        *
*                                                                     *
*  Input:       proc - process to process                           *
*                                                                     *
*  Returns:     length in bytes of assembled data.                    *
*                                                                     *
**********************************************************************/
UINT32 GetTraceDump(int proc)
{
    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    char       *bufP = gBigBuffer;

    /* Only CCB for now */
    if (proc != PROCESS_CCB)
    {
        bufP += sprintf(bufP, "\nPCB/Tracing NOT implemented for FE/BE yet...\n");
        return (UINT32)bufP - (UINT32)gBigBuffer;
    }

    /*
     * First copy in the firmware header information - this should pertain
     * to the selected process, when/if that feature is implemented.
     * This data is common to all of the reports that are sent out.
     */
    bufP += AddFirmwareHdrData(bufP);

    /* Next copy in the trace buffer */
    if (evQueue.evBaseP != NULL)
    {
        memcpy(bufP, evQueue.evBaseP, TRACE_SIZE);
        bufP += TRACE_SIZE;
    }

    return (UINT32)bufP - (UINT32)gBigBuffer;
}

/*============================================================================
**  SECTION:        Process Control Block Dump Routines
**
**  DESCRIPTION:    Contains the Process Control Block Dump and Stack Unwind
**                  functions.
**==========================================================================*/

/*
 * Text definitions of the possible CCB Process states.
 * Note: this could get out-of-date quickly, if pcb.inc is changed
 * for either process, and associated changes are not made here.
 * So consider this list a "best-guess" at the time.
 */

#define statCCB     hnStatPcb

static const char *hnStatPcb[] = {
    "Ready",
    "Not Ready",
    "Internal SDRAM Wait",
    "Cacheable SDRAM Wait",
    "Non-Cacheable SDRAM Wait",
    "Remote SDRAM Wait",
    "Timer 0 Wait",
    "Timer 1 Wait",
    "Inbound Msg Reg 0 Wait",
    "Inbound Msg Reg 1 Wait",
    "Inbound DoorBell Reg bit 0 Wait",
    "Inbound DoorBell Reg bit 1 Wait",
    "Inbound DoorBell Reg bit 2 Wait",
    "Inbound DoorBell Reg bit 3 Wait",
    "I/O Wait",
    "NVA Wait",
    "ILT Wait",
    "Semaphore 1 Wait",
    "Semaphore 2 Wait",
    "SCSI Reset Wait",
    "Block Lockout Wait",
    "Sec Copy Lockout Wait",
    "DMA Unit 0 Wait",
    "DMA Unit 1 Wait",
    "ISP Request 1 Wait",
    "Non-cached Write Buffer Wait",
    "Cached Write Buffer Wait",
    "ISP Request 2 Wait",
    "FC Device Ready Wait",
    "Online/PDD Wait",
    "Write-cache Resource Wait",
    "Link i960 Submission Queue 0 Wait",
    "Link i960 Submission Queue 1 Wait",
    "Link i960 Submission Queue 2 Wait",
    "Link i960 Submission Queue 3 Wait",
    "Link i960 Submission Queue 4 Wait",
    "Link i960 Submission Queue 5 Wait",
    "Link i960 Submission Queue 6 Wait",
    "Link i960 Submission Queue 7 Wait",
    "ISP 2x00 Instance 0 Wait",
    "ISP 2x00 Instance 1 Wait",
    "ISP 2x00 Instance 2 Wait",
    "ISP 2x00 Instance 3 Wait",
    "Wait for MRP completion",
    "Sync NVA wait status",
    "RAID error wait",
    "Try to clean file systems",
    "Pause RAID initializations",
    "Wait for an I82559 request",
    "Wait for IPC",
    "Wait for event",
    "Target task input wait(Wookiee)",
    "Completion task input wait(Wookiee)"
};

/**********************************************************************
*                                                                     *
*  Name:        RangeCheck()                                          *
*                                                                     *
*  Description: Verifies that a pointer is within DRAM.               *
*                                                                     *
*  Input:       void *P - the pointer to test.                        *
*               char *desc - a description of the pointer.            *
*               char *bufP - pointer to the output buffer             *
*                                                                     *
*  Returns:     0 on success; number of bytes written on failure.     *
*                                                                     *
**********************************************************************/
static int RangeCheck(UNUSED void *P, UNUSED const char *descP, UNUSED char *bufP)
{
    return 0;
}


/**********************************************************************
*                                                                     *
*  Name:        GetPCBDump()                                          *
*                                                                     *
*  Description: Walks the PCB chain and prints out everything it      *
*               can about it. It then sends this report out.          *
*                                                                     *
*  Input:       proc - process to process.                          *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
UINT32 GetPCBDump(INT32 proc)
{
    PCB        *startPcbP;
    PCB        *pcbP;
    char       *bufP = gBigBuffer;
    INT32       rc;
    INT32       count = 1;
    char        tmp1[5];
    char        tmp2[5];
    char        tmp3[5];
    FW_HEADER  *fwPtr = (FW_HEADER *)CCBRuntimeFWHAddr;
    INT32       offset = CalcPCIOffset(proc);

    /* Allocate the "Big Buffer" */
    bufP = gBigBuffer;

    /* Add firmware level */
    bufP += AddFirmwareHdrData(bufP);

    /* Only CCB for now */
    if (proc != PROCESS_CCB)
    {
        bufP += sprintf(bufP, "\nPCB/Stack Dump NOT implemented for FE/BE yet...\n");
        return (UINT32)bufP - (UINT32)gBigBuffer;
    }

    /* Start at the current PCB */
    startPcbP = pcbP = (PCB *)XK_GetPcb();

    /*
     * Call TaskSwitch() here with the sole intent of it doing
     * an xchang() to get the pcb's pfp set correctly for this process.
     */
    TaskSwitch();

    /* Insert FW version info */
    fwPtr = (FW_HEADER *)ToPCIAddr(fwPtr, offset);
    strncpy(tmp1, (char *)&fwPtr->revision, 4);
    strncpy(tmp2, (char *)&fwPtr->revCount, 4);
    strncpy(tmp3, (char *)&fwPtr->buildID, 4);
    tmp1[4] = tmp2[4] = tmp3[4] = 0;
    bufP += sprintf(bufP, "%s FW Ver=%s %s %s CRC=%08X ",
                    procName[proc], tmp1, tmp2, tmp3, fwPtr->checksum);
    bufP += sprintf(bufP, "%02d/%02d/%02d %02d:%02d:%02d\n\n",
                    BCD2Short(fwPtr->timeStamp.month),
                    BCD2Short(fwPtr->timeStamp.date),
                    BCD2Short(fwPtr->timeStamp.year),
                    BCD2Short(fwPtr->timeStamp.hours),
                    BCD2Short(fwPtr->timeStamp.minutes),
                    BCD2Short(fwPtr->timeStamp.seconds));

    do
    {
        /* Make sure we have a valid pointer */
        rc = RangeCheck(pcbP, "pcbP->pc_thd", bufP);
        if (rc)
        {
            bufP += rc;
            bufP += sprintf(bufP, "\n\n");
            break;
        }
        pcbP = (PCB *)ToPCIAddr(pcbP, offset);

        /* Print vital statistics */
        bufP += sprintf(bufP, "--- Process %d ---\n", count++);
        bufP += sprintf(bufP, "Base Addr:   0x%08X\n", (UINT32)pcbP);
        bufP += sprintf(bufP, "Next PCB:    0x%08X\n", (UINT32)pcbP->pc_thd);
        bufP += sprintf(bufP, "Status:      %u - %s\n", pcbP->pc_stat,
                        (pcbP->pc_stat < (sizeof(statCCB) / 4)) ?
                        statCCB[pcbP->pc_stat] : "Unknown");
        bufP += sprintf(bufP, "Restore Reg: %u - %s\n", pcbP->pc_rsreg,
                        (pcbP->pc_rsreg == 0) ? "No" : "Yes");
        bufP += sprintf(bufP, "Time/Event:  %u\n", pcbP->pc_time);
        bufP += sprintf(bufP, "PFP:         0x%08X\n", (UINT32)pcbP->pc_pfp);

        /* Then unwind the stack */
        bufP += UnwindStackCCB(pcbP, bufP);
        bufP += sprintf(bufP, "\n\n");

        pcbP = (PCB *)pcbP->pc_thd;
    } while (pcbP != startPcbP);

    return (UINT32)bufP - (UINT32)gBigBuffer;
}


/**********************************************************************
*                                                                     *
*  Name:        UnwindStackCCB()                                      *
*                                                                     *
*  Description: Unwinds a process stack.  This is similar to the      *
*               above, and may replace it someday.  But for now it is *
*               used only in the error trap case.                     *
*                                                                     *
*  Input:       int proc - the process's stack to unwind            *
*               PCB *pcbP - pointer to the pcb.                       *
*               char *bufP - pointer to the output buffer             *
*                                                                     *
*  Returns:     Number of bytes written to output buffer.             *
*                                                                     *
**********************************************************************/
INT32 UnwindStackCCB(PCB *pcbP, char *bufP)
{
    char       *orgP = bufP;
    SF         *startFrameP,
               *curFrameP = NULL;
    UINT32      count = 0;

    if (pcbP == NULL)
    {
        curFrameP = (SF *)get_ebp();

        /* pcbP came in NULL.  Set it now to the current PCB. */
        pcbP = (PCB *)XK_GetPcb();
    }
    else
    {
        curFrameP = pcbP->pc_pfp;

        /* If pcb == current running pcb, taskswitch to update fp. */
        if (XK_GetPcb()->pc_pfp == curFrameP)
        {
            dprintf(DPRINTF_DEFAULT, "Doing self - update ebp\n");
            curFrameP = (SF *)get_ebp();
        }
    }

    /* startFrameP is set so that we know when to stop the unwind */
    startFrameP = pcbP->pc_sf0;

    /* Print out the call stack */
    bufP += sprintf(bufP, "\n  Call Stack:\n %3d)  FP:0x%08X\n", count++, (UINT32)curFrameP);
#ifdef NOTDONEYET
    UINT32      oneMoreTime = 0;
    INT32       rc;

    while (curFrameP >= (SF *)XK_CCB_BASE_DRAM &&
           curFrameP < (SF *)(XK_CCB_BASE_DRAM + SIZE_CCB_LTH) && oneMoreTime < 2)
    {
        /* Check the pointer before using it */
        rc = RangeCheck(curFrameP, "curFrameP->sf_pfp", bufP);
        if (rc)
        {
            bufP += rc;
            break;
        }

        bufP += sprintf(bufP, " %3d) PFP:0x%08X  SP:0x%08X  RIP:0x%08X\n",
                        count++, (UINT32)curFrameP->sf_pfp,
                        (((UINT32)curFrameP->sf_pfp) - sizeof(unsigned int *)),
                        (UINT32)curFrameP->sf_rip);

        /* Point to the next frame */
        curFrameP = curFrameP->sf_pfp;

        if (oneMoreTime)
        {
            oneMoreTime++;
        }
        else if (curFrameP == startFrameP)
        {
            oneMoreTime++;
        }
    }
#endif  /* NOTDONEYET */
    return (bufP - orgP);
}


/**********************************************************************
*  Name:        GetSosStructure()                                     *
**********************************************************************/
static INT32 GetSosStructure(UINT16 pid, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRGETSOS_REQ inputId;
    MRGETSOS_RSP *ptrListOut = NULL;
    UINT32      listOutSize;
    UINT32      numDevs;

    /* Insert the pid into the input structure to be sent by mrp */
    inputId.id = pid;

    if (rc == PI_GOOD)
    {
        /*
         * Why assume 0? If we are wrong, we process two messages to get
         * the data. Plus, we generate debug messages in the log. Plus, we generate
         * an extra malloc and free call.  On the other hand, if we assume a number
         * that usually works, we eliminate one message, and one log message, and one call
         * to malloc, and one call to free. The cost of doing this is that we need to malloc
         * 160 extra bytes the first time. It seems to me to make much more sense to
         * guess too high, rather than too low. So, we will start with assuming 10 items.
         * - Bruce Davis
         */
        numDevs = 10;

        do
        {
            listOutSize = (sizeof(*ptrListOut) + (numDevs * sizeof(MRGETSOS_RSP_ENTRY)));

            /*
             * If an output list was previously allocated, free it before
             * allocating a new one.
             */
            if (ptrListOut != NULL)
            {
                Free(ptrListOut);
            }

            ptrListOut = MallocSharedWC(listOutSize);

            /*
             * Send the mrp request.  This function handles timeout
             * conditions and task switches while waiting.
             */
            rc = PI_ExecMRP(&inputId, sizeof(inputId), MRGETSOS,
                            ptrListOut, listOutSize, GetGlobalMRPTimeout());

            /*
             * Save the number of devices in case we need to make the
             * request again.  Also grab the size of each entry from the
             * response packet.
             */
            numDevs = ptrListOut->sos.count;

        } while (ptrListOut != NULL &&
                 rc == PI_ERROR && (ptrListOut->header.status == DETOOMUCHDATA));
    }

    if (rc == PI_GOOD)
    {
        pRspPacket->pHeader->length = listOutSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = ptrListOut->header.status;
        pRspPacket->pPacket = (UINT8 *)ptrListOut;
    }

    else
    {
        /* Indicate an error condition and no return data in the header. */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;

        if (ptrListOut != NULL)
        {
            pRspPacket->pHeader->errorCode = ptrListOut->header.status;
        }

        /*
         * If we return NULL the caller can't free the memory so do it here.
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        if (rc != PI_TIMEOUT)
        {
            Free(ptrListOut);
        }

        rc = PI_ERROR;
    }

    return rc;
}

/**********************************************************************
*                                                                     *
*  Name:        DisableHeartbeats()                                   *
*                                                                     *
*  Description: Disable ALL heartbeats on the controller. Includes    *
*               intra and inter controller heartbeats.                *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     GOOD or ERROR                                         *
*                                                                     *
**********************************************************************/
static INT32 DisableHeartbeats(void)
{
    MODEDATA    data;
    MODEDATA    mask;
    INT32       rc;
    UINT32     *p = (UINT32 *)&data;

    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));

    mask.ccb.bits = MD_IPC_HEARTBEAT_DISABLE |  /* set to 1 */
        MD_IPC_HEARTBEAT_WATCHDOG_DISABLE | MD_LOCAL_HEARTBEAT_DISABLE;

    data.proc.word[3] = MD_PROC_HEARTBEAT_WATCHDOG_DISABLE;     /* set to 1 */
    mask.proc.word[3] = MD_PROC_HEARTBEAT_WATCHDOG_DISABLE;

    data.ccb.bits = MD_IPC_HEARTBEAT_DISABLE |
        MD_IPC_HEARTBEAT_WATCHDOG_DISABLE | MD_LOCAL_HEARTBEAT_DISABLE;

    rc = ModeSet(&data, &mask);

    ModeGet(&data);
    dprintf(DPRINTF_DEFAULT, "\n CCB Mode Bits: %08X %08X %08X %08X\n"
            "Proc Mode Bits: %08X %08X %08X %08X\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

    return rc;
}


/**********************************************************************
*                                                                     *
*  Name:        GetProfileDump()                                      *
*                                                                     *
*  Description: Profile until buffer full and ship out.               *
*                                                                     *
*  Input:       proc - process to process (CCB only for now).         *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
UINT32 GetProfileDump(int proc)
{
    char       *bufP;

    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    bufP = gBigBuffer;

    /* Add firmware level */
    bufP += AddFirmwareHdrData(bufP);

    /* Only CCB for now */
    if (proc != PROCESS_CCB)
    {
        bufP += sprintf(bufP, "\nPCB/Profiling NOT implemented for FE/BE yet...\n");
        return (UINT32)bufP - (UINT32)gBigBuffer;
    }

    /* Initialize the Profile Queue */
    ProfileInit(bufP);

    return (UINT32)bufP - (UINT32)gBigBuffer;
}


/**********************************************************************
*                                                                     *
*  Name:        GetPacketStats()                                      *
*                                                                     *
*  Description: Dump out packet statistics.  It is post-processed     *
*               by the client.                                        *
*                                                                     *
*  Input:       proc - process to process (only CCB at this time)     *
*                                                                     *
*  Returns:     length in bytes of assembled data.                    *
*                                                                     *
**********************************************************************/
static UINT32 GetPacketStats(int proc)
{
    /*
     * gBigBuffer is available here since the "bigBufferMutex" is locked
     * by the calling function.
     */
    char       *bufP = gBigBuffer;
    UINT8      *statsP = NULL;
    UINT32      statsLength;

    /* Only CCB for now */
    if (proc != PROCESS_CCB)
    {
        bufP += sprintf(bufP, "\nPCB/Tracing NOT implemented for FE/BE yet...\n");
        return (UINT32)bufP - (UINT32)gBigBuffer;
    }

    /*
     * First copy in the firmware header information - this should pertain
     * to the selected process, when/if that feature is implemented.
     * This data is common to all of the reports that are sent out.
     */
    bufP += AddFirmwareHdrData(bufP);

    /* Next get a pointer and length for  the packet statistics */
    statsP = GetPacketStatsPointer(&statsLength);

    /* If we got a valid pointer, copy the data to the message buffer */
    if (NULL != statsP)
    {
        memcpy(bufP, statsP, statsLength);
        bufP += statsLength;
    }

    return (UINT32)bufP - (UINT32)gBigBuffer;
}

/**********************************************************************
*                                                                     *
*  Name:        GenericTestCache()                                    *
*                                                                     *
*  Description: Test the caching capabilities.                        *
*                                                                     *
*  Input:       dummy    -   Forkable                                 *
*               testType -   Type of test to run                      *
*               timeout  -   Timeout between tests                    *
*                                                                     *
*  Returns:     None.                                                 *
*                                                                     *
**********************************************************************/
static void GenericTestCache(TASK_PARMS *parms)
{
    UINT32      testType = parms->p1;
    UINT32      timeout = parms->p2;

    switch (testType)
    {
        case PI_GENERIC_CACHE_TEST_X1:
            /* Run the test until testCache is set to false. */
            do
            {
                SendX1AsyncNotifications(NULL);
                if (timeout == 0)
                {
                    TaskSwitch();
                }
                else
                {
                    TaskSleepMS(timeout);
                }
            } while (testCache);
            break;

            /* If we are testing only the cache, hammer it. */
        case PI_GENERIC_CACHE_TEST_CACHE:
            /* Run the test until testCache is set to false. */
            do
            {
                /*
                 * We use timeout to help mix it up a little bit.
                 * Otherwise the cache requests start sequencing.
                 */
                switch (timeout % 7)
                {
                    case 0:
                        (void)InvalidateCache(CACHE_INVALIDATE_ALL, true);
                        break;

                    case 1:
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(true);
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(true);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                        InvalidateCacheTargets(true);
                        InvalidateCacheEnvironmentals(true);
#endif  /* NO_TARGET_CACHE */
                        InvalidateCacheStats(true);
                        InvalidateCacheBE(true);
                        InvalidateCacheFE(true);
                        InvalidateCacheDiskBays(true);
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(true);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(true);
#endif  /* NO_VDISK_CACHE */
                        break;

                    case 2:
                        InvalidateCacheStats(false);
                        InvalidateCacheEnvironmentals(false);
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(false);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                        InvalidateCacheTargets(false);
#endif  /* NO_TARGET_CACHE */
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(false);
#endif  /* NO_RAID_CACHE */
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(false);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(false);
#endif  /* NO_PDISK_CACHE */
                        InvalidateCacheDiskBays(false);
                        InvalidateCacheFE(false);
                        InvalidateCacheBE(false);
                        break;

                    case 3:
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(true);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(true);
#endif  /* NO_PDISK_CACHE */
                        InvalidateCacheDiskBays(true);
                        InvalidateCacheFE(true);
                        InvalidateCacheBE(true);
                        InvalidateCacheStats(true);
                        InvalidateCacheEnvironmentals(true);
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(true);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                        InvalidateCacheTargets(true);
#endif  /* NO_TARGET_CACHE */
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(true);
#endif  /* NO_RAID_CACHE */
                        break;

                    case 4:
                        InvalidateCacheDiskBays(false);
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(false);
#endif  /* NO_RAID_CACHE */
                        InvalidateCacheFE(false);
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(false);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                        InvalidateCacheTargets(false);
#endif  /* NO_TARGET_CACHE */
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(false);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(false);
#endif  /* NO_VDISK_CACHE */
                        InvalidateCacheStats(false);
                        InvalidateCacheEnvironmentals(false);
                        InvalidateCacheBE(false);
                        break;

                    case 5:
                        InvalidateCacheBE(true);
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(true);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(true);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(true);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(true);
#endif  /* NO_RAID_CACHE */
                        InvalidateCacheDiskBays(true);
                        InvalidateCacheStats(true);
                        InvalidateCacheFE(true);
                        InvalidateCacheEnvironmentals(true);
#ifndef NO_TARGETS_CACHE
                        InvalidateCacheTargets(true);
#endif  /* NO_TARGETS_CACHE */
                        break;

                    default:
                        InvalidateCacheBE(true);
                        InvalidateCacheFE(true);
                        InvalidateCacheDiskBays(true);
#ifndef NO_PDISK_CACHE
                        InvalidateCachePhysicalDisks(true);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
                        InvalidateCacheVirtualDisks(true);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
                        InvalidateCacheRaids(true);
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
                        InvalidateCacheServers(true);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
                        InvalidateCacheTargets(true);
#endif  /* NO_TARGET_CACHE */
                        InvalidateCacheEnvironmentals(true);
                        InvalidateCacheStats(true);
                        break;
                }

                if (timeout == 0)
                {
                    TaskSwitch();
                }
                else
                {
                    TaskSleepMS(timeout);
                }
            } while (testCache);
            break;

        default:
            break;
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
