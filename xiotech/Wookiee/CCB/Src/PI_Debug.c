/* $Id: PI_Debug.c 159391 2011-07-09 21:01:56Z m4 $ */
/*===========================================================================
** FILE NAME:       PI_Debug.c
** MODULE TITLE:    Packet Interface for Debug Commands
**
** DESCRIPTION:     Handler functions for Debug request packets such as
**                  Init Proc NVRAM, Set Serial Number, etc.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "PI_CmdHandlers.h"

#include "LOG_Defs.h"
#include "debug_files.h"
#include "debug_struct.h"
#include "EL.h"
#include "errorCodes.h"
#include "LargeArrays.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "nvram_structure.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "rm.h"
#include "RM_headers.h"
#include "serial_num.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "Snapshot.h"

#include <byteswap.h>


/*****************************************************************************
** Private defines
*****************************************************************************/
#define MAX_XFER_LENGTH     (0x10000-256)
#define SCSI_DATA_LEN       8192
#define READWRITE_DATA_LEN   512

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_DebugMemRdWr
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugMemRdWr(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_DEBUG_MEM_RDWR_REQ *pReq;
    PI_DEBUG_MEM_RDWR_RSP *ptrOutPkt = NULL;
    MR_RW_MEMORY_REQ *inPkt = NULL;
    MR_RW_MEMORY_RSP *outPkt = NULL;
    UINT8      *pData = NULL;
    UINT32      i;
    INT32       rc = PI_GOOD;
    UINT32      length;
    UINT32      force = 0;

    pReq = (PI_DEBUG_MEM_RDWR_REQ *)pReqPacket->pPacket;
    force = pReq->length & 0x80000000;
    length = ((pReq->length & 0x7fffffff) > MAX_XFER_LENGTH) ? MAX_XFER_LENGTH : (pReq->length & 0x7fffffff);

    switch (pReq->mode)
    {
        case MEM_READ:

            switch (pReq->processor)
            {
                case PROCESS_CCB:

                    /*
                     * Allocate memory for the return data.
                     */
                    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt) + length);

                    for (i = 0; i < length; i++)
                    {
                        ptrOutPkt->data[i] = ((UINT8 *)(pReq->pAddr))[i];
                    }
                    break;

                case PROCESS_FE:
                case PROCESS_BE:
                    /*
                     * Allocate memory for the return data.
                     */
                    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt) + length);

                    /*
                     * Only call the MRP if there is data to process
                     */
                    if (length)
                    {
                        inPkt = MallocWC(sizeof(*inPkt));
                        outPkt = MallocSharedWC(sizeof(*outPkt));

                        /*
                         * Setup input structure from the input parms.
                         */
                        inPkt->srcAddr = (char *)pReq->pAddr;
                        inPkt->dstAddr = ptrOutPkt->data;
                        inPkt->length = length | force;

                        /*
                         * Send the request to Thunderbolt.  This function handles
                         * timeout conditions and task switches while waiting.
                         */
                        rc = PI_ExecMRP(inPkt, sizeof(*inPkt),
                                        pReq->processor == PROCESS_FE ? MRFRWMEM : MRBRWMEM,
                                        outPkt, sizeof(*outPkt), GetGlobalMRPTimeout());
                    }

                    break;

                    /*
                     * Incorrect "processor"
                     */
                default:
                    rc = PI_ERROR;
                    break;
            }

            /*
             * Attach the MRP return data packet to the main response packet.
             * Fill in the header length and status fields.
             */
            if (rc != PI_GOOD)
            {
                DelayedFree(pReq->processor == PROCESS_FE ? MRFRWMEM : MRBRWMEM,
                            ptrOutPkt);

                pRspPacket->pPacket = NULL;
                pRspPacket->pHeader->length = 0;
            }
            else
            {

                ptrOutPkt->length = length;

                pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
                pRspPacket->pHeader->length = sizeof(*ptrOutPkt) + length;
            }

            pRspPacket->pHeader->status = rc;
            if (outPkt)
            {
                pRspPacket->pHeader->errorCode = outPkt->header.status;
            }
            else
            {
                pRspPacket->pHeader->errorCode = 0;
            }
            break;

        case MEM_WRITE:

            switch (pReq->processor)
            {
                case PROCESS_CCB:

                    /*
                     * Allocate memory for the return data.
                     */
                    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

                    for (i = 0; i < length; i++)
                    {
                        ((UINT8 *)(pReq->pAddr))[i] = pReq->data[i];
                    }
                    break;

                case PROCESS_FE:
                case PROCESS_BE:
                    /*
                     * Allocate memory for the return data.
                     */
                    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

                    /*
                     * Only call the MRP if there is data to process
                     */
                    if (length)
                    {
                        inPkt = MallocWC(sizeof(*inPkt));
                        outPkt = MallocSharedWC(sizeof(*outPkt));
                        pData = MallocSharedWC(length);

                        /*
                         * Setup input structure from the input parms.
                         */
                        memcpy(pData, pReq->data, length);
                        inPkt->srcAddr = pData;

                        inPkt->dstAddr = (char *)pReq->pAddr;
                        inPkt->length = length | force;

                        /*
                         * Send the request to Thunderbolt.  This function handles
                         * timeout conditions and task switches while waiting.
                         */
                        rc = PI_ExecMRP(inPkt, sizeof(*inPkt),
                                        pReq->processor ==
                                        PROCESS_FE ? MRFRWMEM : MRBRWMEM, outPkt,
                                        sizeof(*outPkt), GetGlobalMRPTimeout());

                        /*
                         * DelayedFree() the data area that we passed down to the Proc.
                         */
                        DelayedFree(pReq->processor == PROCESS_FE ? MRFRWMEM : MRBRWMEM,
                                    pData);
                    }
                    break;

                    /*
                     * Incorrect "processor"
                     */
                default:
                    rc = PI_ERROR;
                    break;
            }

            /*
             * Attach the MRP return data packet to the main response packet.
             * Fill in the header length and status fields.
             */
            if (rc != PI_GOOD)
            {
                Free(ptrOutPkt);

                pRspPacket->pPacket = NULL;
                pRspPacket->pHeader->length = 0;
            }
            else
            {
                ptrOutPkt->length = length;

                pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
                pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
            }

            pRspPacket->pHeader->status = rc;
            if (outPkt)
            {
                pRspPacket->pHeader->errorCode = outPkt->header.status;
            }
            else
            {
                pRspPacket->pHeader->errorCode = 0;
            }
            break;

            /*
             * Incorrect "mode"
             */
        default:
            rc = PI_ERROR;
            pRspPacket->pPacket = NULL;
            pRspPacket->pHeader->length = 0;
            pRspPacket->pHeader->status = PI_ERROR;
            pRspPacket->pHeader->errorCode = 0;
            break;
    }

    /*
     * Cleanup the MRP input and output packets
     */
    Free(inPkt);
    if (rc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_SCSICmd
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_SCSICmd(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_DEBUG_SCSI_CMD_REQ *pReq = NULL;
    PI_DEBUG_SCSI_CMD_RSP *ptrOutPkt = NULL;
    MRSCSIIO_REQ *inPkt = NULL; /* for mrp call */
    MRSCSIIO_RSP *outPkt = NULL;        /* for mrp call */
    UINT8      *scsiData = NULL;        /* for mrp call */
    INT32       rc = PI_GOOD;
    UINT32      inpDataLen;

    pReq = (PI_DEBUG_SCSI_CMD_REQ *)pReqPacket->pPacket;
    inpDataLen = (pReq->dataLen > SCSI_DATA_LEN) ? SCSI_DATA_LEN : pReq->dataLen;

    /* Allocate memory */
    scsiData = MallocSharedWC(SCSI_DATA_LEN);
    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt) + SCSI_DATA_LEN);
    if (inpDataLen)
    {
        memcpy(scsiData, pReq->data, inpDataLen);
    }

    /* Setup input structure from the input parms. */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = pReq->wwnLun.wwn;
    inPkt->cmdlen = pReq->cdbLen;
    inPkt->func = inpDataLen ? MRSCSIIO_OUTPUT : MRSCSIIO_INPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 8;
    inPkt->flags = MRSCSIIO_FLAGS_SLI;
    inPkt->lun = pReq->wwnLun.lun;
    inPkt->retry = 1;
    inPkt->bptr = scsiData;
    inPkt->blen = SCSI_DATA_LEN;
    memcpy(inPkt->cdb, pReq->cdb, 16);

    dprintf(DPRINTF_SCSICMD, "%s: Issuing scsi cmd to WWN %08X%08X  "
            "LUN %u\n", __func__, bswap_32((UINT32)inPkt->wwn),
            bswap_32((UINT32)(inPkt->wwn >> 32)), inPkt->lun);

    dprintf(DPRINTF_SCSICMD, "%s: CDBLen = %u, inpDataLen = %u\n",
            __func__, inPkt->cmdlen, inpDataLen);

    if (inpDataLen)
    {
        dprintf(DPRINTF_SCSICMD, "%s: inpData = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX ...\n",
                __func__, scsiData[0],
                scsiData[1], scsiData[2], scsiData[3], scsiData[4], scsiData[5],
                scsiData[6], scsiData[7], scsiData[8], scsiData[9], scsiData[10],
                scsiData[11], scsiData[12], scsiData[13], scsiData[14], scsiData[15]);
    }

    dprintf(DPRINTF_SCSICMD, "%s: CDB = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
            __func__, inPkt->cdb[0],
            inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3], inPkt->cdb[4], inPkt->cdb[5],
            inPkt->cdb[6], inPkt->cdb[7], inPkt->cdb[8], inPkt->cdb[9], inPkt->cdb[10],
            inPkt->cdb[11], inPkt->cdb[12], inPkt->cdb[13], inPkt->cdb[14],
            inPkt->cdb[15]);
    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), 35000);

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    ptrOutPkt->sense = outPkt->sense;
    ptrOutPkt->asc = outPkt->asc;
    ptrOutPkt->ascq = outPkt->ascq;
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    switch (rc)
    {
        case PI_GOOD:
            ptrOutPkt->length = SCSI_DATA_LEN;
            memcpy(ptrOutPkt->data, scsiData, SCSI_DATA_LEN);
            pRspPacket->pHeader->length = sizeof(*ptrOutPkt) + SCSI_DATA_LEN;

            if (!inpDataLen)
            {
                dprintf(DPRINTF_SCSICMD, "%s: outData = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX ...\n",
                        __func__, scsiData[0], scsiData[1], scsiData[2], scsiData[3], scsiData[4],
                        scsiData[5], scsiData[6], scsiData[7], scsiData[8], scsiData[9],
                        scsiData[10], scsiData[11], scsiData[12], scsiData[13],
                        scsiData[14], scsiData[15]);
            }

            break;

        case PI_ERROR:
            ptrOutPkt->length = 0;
            pRspPacket->pHeader->length = sizeof(*ptrOutPkt);

            dprintf(DPRINTF_DEFAULT, "%s: Error with scsi cmd to WWN %08X%08X LUN %u\n", __func__,
                    bswap_32((UINT32)inPkt->wwn), bswap_32((UINT32)(inPkt->wwn >> 32)),
                    inPkt->lun);

            dprintf(DPRINTF_DEFAULT, "%s - command failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                    __func__, ptrOutPkt->sense, ptrOutPkt->asc, ptrOutPkt->ascq);
            break;

        default:
            Free(ptrOutPkt);
            pRspPacket->pPacket = NULL;
            pRspPacket->pHeader->length = 0;
            break;
    }

    pRspPacket->pHeader->status = rc;
    if (outPkt)
    {
        pRspPacket->pHeader->errorCode = outPkt->status;
    }
    else
    {
        pRspPacket->pHeader->errorCode = 0;
    }

    /* Cleanup the MRP input packets */
    if (rc != PI_TIMEOUT)
    {
        Free(inPkt);
        Free(outPkt);
    }
    DelayedFree(MRSCSIIO, scsiData);

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_READWRITECmd
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_READWRITECmd(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_DEBUG_READWRITE_CMD_REQ *pReq;
    PI_DEBUG_READWRITE_CMD_RSP *ptrOutPkt;
    MRREADWRITEIO_REQ *inPkt = NULL;        /* for mrp call */
    MRREADWRITEIO_RSP *outPkt = NULL;       /* for mrp call */
    UINT8      *Data = NULL;                /* for mrp call */
    INT32       rc = PI_GOOD;
    UINT32      inpDataLen;

    pReq = (PI_DEBUG_READWRITE_CMD_REQ *)pReqPacket->pPacket;
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt) + READWRITE_DATA_LEN);

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));
    Data = MallocSharedWC(READWRITE_DATA_LEN);       /* Allocate memory */

    inpDataLen = (pReq->dataInLen > READWRITE_DATA_LEN) ? READWRITE_DATA_LEN : pReq->dataInLen;
    if (pReq->rw == 'w')
    {
        memcpy(Data, pReq->data, inpDataLen);
    }

    /* Setup input structure from the input parms. */
    inPkt->pv = pReq->pv;
    inPkt->rw = pReq->rw;
    inPkt->id = pReq->id;
    inPkt->block = pReq->block;
    inPkt->bptr = Data;
    inPkt->dataInLen = READWRITE_DATA_LEN;

    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRREADWRITEIO, outPkt, sizeof(*outPkt), 35000);

    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;

    switch (rc)
    {
        case PI_GOOD:
            if (pReq->rw == 'r')
            {
                ptrOutPkt->length = READWRITE_DATA_LEN;
                memcpy(ptrOutPkt->data, Data, READWRITE_DATA_LEN);
                pRspPacket->pHeader->length = sizeof(*ptrOutPkt) + READWRITE_DATA_LEN;
            }
            else
            {
                ptrOutPkt->length = 0;
                pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
            }
            break;

        case PI_ERROR:
            ptrOutPkt->length = 0;
            pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
            dprintf(DPRINTF_DEFAULT, "%s: Error with %c cmd to %c id %u block=%llu\n",
                    __func__, inPkt->rw, inPkt->pv, inPkt->id, inPkt->block);
            break;

        default:
            Free(ptrOutPkt);
            pRspPacket->pPacket = NULL;
            pRspPacket->pHeader->length = 0;
            break;
    }

    pRspPacket->pHeader->status = rc;
    if (outPkt)
    {
        pRspPacket->pHeader->errorCode = outPkt->header.status;
    }
    else
    {
        pRspPacket->pHeader->errorCode = 0;
    }

    /* Cleanup the MRP input packets */
    if (rc != PI_TIMEOUT)
    {
        Free(inPkt);
        Free(outPkt);
    }
    DelayedFree(MRREADWRITEIO, Data);

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_DebugInitCCBNVRAM
**
** Description: Initialize the CCB NVRAM
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugInitCCBNVRAM(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_DEBUG_INIT_CCB_NVRAM_REQ *pRequest = NULL;

    dprintf(DPRINTF_DEFAULT, "PI_DebugInitCCBNVRAM - ENTER\n");

    pRequest = (PI_DEBUG_INIT_CCB_NVRAM_REQ *)pReqPacket->pPacket;

    if (pRequest->type == INIT_CCB_NVRAM_TYPE_FULL)
    {
        dprintf(DPRINTF_DEFAULT, "PI_DebugInitCCBNVRAM - Full initialization\n");

        /*
         * Update the PROC serial numbers to make sure they get set
         * to zero.
         */
        UpdateProcSerialNumber(CONTROLLER_SN, 0);

        /*
         * Destroy everything we hold dear and true, our NVRAM
         */
        MemSetNVRAMBytes(&NVRAMData, 0, sizeof(NVRAMData));

        /*
         * Clear and reset the master config and save.
         */
        ResetMasterConfigNVRAM();

        /*
         * This function will load the default controller setup as the
         * crc has been destroyed.
         */
        LoadControllerSetup();

        /*
         * Reset the device configuration information.
         */
        SaveDeviceConfig(0, NULL);
    }
    else if (pRequest->type == INIT_CCB_NVRAM_TYPE_LICENSE)
    {
        dprintf(DPRINTF_DEFAULT, "PI_DebugInitCCBNVRAM - Clear license initialization\n");

        /*
         * If the request is to only clear the licensed flag
         * then just do that and leave the rest of the setup
         * intact.
         */
        ClearLicenseApplied();

        /*
         * Update the PROC serial numbers to make sure they get set
         * to zero.
         */
        UpdateProcSerialNumber(CONTROLLER_SN, 0);

        /*
         * Reset the device configuration information.
         */
        SaveDeviceConfig(0, NULL);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "PI_DebugInitCCBNVRAM - Invalid initialization type (0x%x).\n",
                pRequest->type);

        rc = PI_ERROR;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_DebugGetSerNum
**
** Description: Get the system or controller serial number
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_MALLOC_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugGetSerNum(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_DEBUG_GET_SERIAL_NUM_RSP *ptrOutPkt = NULL;
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the return data.
     */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    ptrOutPkt->serialNumber = GetSerialNumber(((PI_DEBUG_GET_SERIAL_NUM_REQ *)(pReqPacket->pPacket))->type);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_DebugStructDisplay
**
** Description: Display internal data structure
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugStructDisplay(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       socketRC;
    INT32       len = 0;

    /*
     * Lock the static "Big Buffer" mutex so that we can use it
     */
    (void)LockMutex(&bigBufferMutex, MUTEX_WAIT);

    switch (((PI_DEBUG_STRUCT_DISPLAY_REQ *)(pReqPacket->pPacket))->type)
    {
        case DISPLAY_STRUCT_SOCKET_STATS:
            {
                /*
                 * Initialize a pointer to store buffer malloc'd by callee in
                 */
                char       *str = NULL;

                len = DisplaySocketStats(&str, 1);

                if (len)
                {
                    strcpy(gBigBuffer, str);
                }

                /*
                 * Free the buffer passed to us
                 */
                if (str)
                {
                    Free(str);
                }
            }
            break;

        case DISPLAY_STRUCT_MASTER_CONFIG:
            len = DisplayMasterConfigStruct();
            break;

        case DISPLAY_STRUCT_SES_DEVICE:
            len = DisplaySESDeviceStruct();
            break;

        case DISPLAY_STRUCT_SNAPSHOT_DIR:
            len = DisplaySnapshotDirectoryVerbose();
            break;

        case DISPLAY_STRUCT_CCB_STATS:
            len = DisplayCCBStatistics();
            break;

        default:
            rc = PI_PARAMETER_ERROR;
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

    /*
     * Send the packet
     */
    socketRC = SendPacket(pReqPacket->pHeader->socket, pRspPacket,
                          GetSelectTimeoutBySocket(pReqPacket->pHeader->socket));

    /*
     * Set the pPacket pointer back to NULL so that it doesn't get
     * "free'd" by PacketInterfaceServer().
     */
    pRspPacket->pPacket = NULL;

    /*
     * Free the "Big Buffer"
     */
    UnlockMutex(&bigBufferMutex);

    /*
     * If anything failed, return a non-zero return code
     */
    if (rc != PI_GOOD || socketRC == PI_SOCKET_ERROR)
    {
        return PI_ERROR;
    }

    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Function:    PI_DebugGetElecSt
**
** Description: Get the election state
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_MALLOC_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugGetElecSt(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_DEBUG_ELECTION_STATE_RSP *ptrOutPkt = NULL;

    /*
     * Allocate memory for the return data.
     */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    ptrOutPkt->state = EL_GetCurrentElectionState();

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_DebugGetState_RM
**
** Description: Get the resource manager state
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_MALLOC_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DebugGetState_RM(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_DEBUG_GET_STATE_RM_RSP *ptrOutPkt = NULL;

    /*
     * Allocate memory for the return data.
     */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    ptrOutPkt->state = RMGetState();

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

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
