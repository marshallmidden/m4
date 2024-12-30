/* $Id: PI_BlkXfr.c 145021 2010-08-03 14:16:38Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_BlkXfr.c
** MODULE TITLE:    Packet Interface for Block Transfer Commands
**
** DESCRIPTION:     Handler functions for Firmware Download & Debug Report
**                  request packets.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#include "CacheBay.h"
#include "CachePDisk.h"
#include "CmdLayers.h"
#include "ddr.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "FIO.h"
#include "ipc_security.h"
#include "LargeArrays.h"
#include "misc.h"
#include "MR_Defs.h"
#include "mutex.h"
#include "PacketInterface.h"
#include "ParmVal.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "PortServerUtils.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include <sys/statvfs.h>

/*****************************************************************************
** Private variables
*****************************************************************************/

static UINT8 rxBuffer[SIZEOF_RXBUFFER] LOCATE_IN_SHMEM;
static UINT8 txBuffer[SIZEOF_TXBUFFER] LOCATE_IN_SHMEM;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 HandleFinalPacket(X1_MULTI_PART_XFER_REQ *mpxP, INT32 *reason);
static INT32 FetchFIDData(X1_MULTI_PART_XFER_REQ *mpxP, INT32 *reason);
static INT32 FetchLinuxFileData(X1_MULTI_PART_XFER_REQ *pMpx, INT32 *reason);

static INT32 ReadMemToBuffer(void *dest, void *src, UINT32 length, INT32 proc);
static INT32 UpdatePDiskCode(FW_HEADER *pFW, UINT16 pid);
static INT32 UpdateBayCode(FW_HEADER *pFW, UINT16 bid);
static INT32 WriteMemFromBuffer(void *dest, void *src, UINT32 length, INT32 proc);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    FSSpaceAvailable
**
** Description: Check for sufficient free space for firmware download.
**
** Inputs:      None
**
** Returns:     1 if ok, 0 if not ok
**
**--------------------------------------------------------------------------*/

static int FSSpaceAvailable(void)
{
    static struct
    {
        const char *const path;         /* Path to check for space */
        const uint32_t mbneeded;        /* Size in megabytes needed */
    } spchks[] = {
        { "/boot", 8},
        { "/lib", 8},
        { "/opt/xiotech", 20},
        { "/var/lib", 10},
        { "/etc", 1},
        { "/tmp", 10},
    };
    unsigned int i;

    for (i = 0; i < sizeof(spchks) / sizeof(spchks[0]); ++i)
    {
        struct statvfs vfs;
        unsigned long long blks;
        int         err;

        err = statvfs(spchks[i].path, &vfs);
        if (err)
        {
            dprintf(DPRINTF_DEFAULT, "FSSpaceAvailable: statvfs(%s) returned %d\n",
                    spchks[i].path, err);
            return 0;
        }
        blks = vfs.f_bavail;
        blks = blks * (vfs.f_bsize / 512);
        if ((blks / 2048) < spchks[i].mbneeded)
        {
            dprintf(DPRINTF_DEFAULT, "FSSpaceAvailable: only %lldMb for %s, %d needed\n",
                    blks / 2048, spchks[i].path, spchks[i].mbneeded);
            return 0;
        }
    }
    return 1;
}


/*----------------------------------------------------------------------------
** Function:    PI_FWDownload
**
** Description: Download and process a firmware binary image or
**              a license file.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FWDownload(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       tmpRC;
    INT32       tmpRC2 = PI_GOOD;
    INT32       rc = PI_GOOD;
    INT32       i;
    INT32       parmValEr = FALSE;
    PI_WRITE_BUFFER_MODE5_REQ *wbP;
    RC_STATUS   rcStat = { 0, 0 };
    UINT8      *pBuf;
    INT32       sock;

    /*
     * OK, there is a bit of an assumption being made here. "rxBuffer" is the
     * main buffer that is used for FW downloads when they come through the
     * MPX interface (generally from ICON).  There is no mutex on this
     * buffer!!  But as you can see, we are recklessly using it here -- OK
     * maybe not so recklessly.  There should never be a FW download occuring
     * from ICON at the same time that one is occurring from CCBE. Certainly
     * never in a customer environment.  Anyway, so it goes.
     * TODO -- figure out a way to lock rxBuffer to ensure no trouble in the
     * future.
     */
    pBuf = (UINT8 *)rxBuffer;

    /*
     * Lock the static "Big Buffer" mutex so that we can receive the firmware
     * into it. Don't need to do this for H/N, but shouldn't hurt either.
     */
    (void)LockMutex(&bigBufferMutex, MUTEX_WAIT);

    /*
     * Pull down the firmware. Remember, the socket ID was passed
     * via the request packet data pointer.
     */
    if ((pReqPacket != NULL) &&
        (pReqPacket->pPacket != NULL) &&
        (pReqPacket->pHeader->length <= SIZEOF_RXBUFFER) && FSSpaceAvailable())
    {

        sock = *(INT32 *)(pReqPacket->pPacket);
        tmpRC = ReceiveData(sock, pBuf, pReqPacket->pHeader->length,
                            GetSelectTimeoutBySocket(sock));

        /*
         * Check MD5. Fix up the req packet so it looks like a real
         * (i.e. normal) XIO_PACKET.
         */
        if (tmpRC > 0)
        {
            UINT8      *tmpP = pReqPacket->pPacket;

            pReqPacket->pPacket = pBuf;

            if (CheckDataMD5((IPC_PACKET *)pReqPacket) == FALSE)
            {
                dprintf(DPRINTF_PI_PROTOCOL, "PI_FWDownload: data MD5 incorrect\n");
                tmpRC = PI_MD5_ERROR;
                rc = PI_ERROR;
            }

            pReqPacket->pPacket = tmpP;
        }
    }
    else
    {
        if ((pReqPacket != NULL) && (pReqPacket->pPacket != NULL))
        {
            sock = *(INT32 *)(pReqPacket->pPacket);
            for (i = 0; i < (INT32)(pReqPacket->pHeader->length) / 1024; i++)
            {
                tmpRC = ReceiveData(sock, rxBuffer, 1024, GetSelectTimeoutBySocket(sock));
                if (tmpRC <= 0)
                {
                    break;
                }
            }
            i = recv(sock, rxBuffer, 1024, 0);
        }
        dprintf(DPRINTF_DEFAULT, "PI_FWDownload: Couldn't malloc rx buffer\n");
        tmpRC = FILE_TOO_LONG;
        rc = PI_ERROR;
    }

    /*
     * This is the NON-MPX path.
     * If the receive was good, go burn the fw.
     */
    if (rc == PI_GOOD)
    {

        switch (pReqPacket->pHeader->commandCode)
        {

            case PI_FIRMWARE_DOWNLOAD_CMD:
                /* Validate the fw header */
                tmpRC = ValidateFWHeader((FW_HEADER *)pBuf);
                if (tmpRC != PI_GOOD)
                {
                    break;
                }

                /* Go update the code */
                tmpRC = UpdateCode((FW_HEADER *)pBuf);
                break;

            case PI_TRY_CCB_FW_CMD:
                tmpRC = ILLEGAL_TRY_NON_CCB_FW;
                dprintf(DPRINTF_CODE_UPDATE, "PI_FWDownload: FAIL: Can't TRY any code.\n");
                break;

            case PI_WRITE_BUFFER_MODE5_CMD:

                wbP = (PI_WRITE_BUFFER_MODE5_REQ *)pBuf;

                if (wbP->count == 0)
                {
                    tmpRC = PARM_ERROR;
                    break;
                }

                /* Validate the fw header */
                tmpRC = ValidateFWHeader((FW_HEADER *) & wbP->wwnLun[wbP->count]);
                if (tmpRC != PI_GOOD)
                {
                    break;
                }

                /* Validate the input data */
                tmpRC = ParmCheckWriteBuffer(pReqPacket, pRspPacket, pBuf);
                if (tmpRC != PI_GOOD)
                {
                    parmValEr = TRUE;
                    break;
                }

                /* Go update the code (WRITE_BUFFER, NON-MPX command) */
                for (i = 0; i < (INT32)wbP->count; i++)
                {
                    switch (((FW_HEADER *) & wbP->wwnLun[wbP->count])->target)
                    {
                        case TARG_ID_EUROLOGIC_BAY:
                        case TARG_ID_ADAPTEC_SATA_ES_BAY:
                            rcStat = UpdateEurologicBaySingle((FW_HEADER *) & wbP->wwnLun[wbP->count],
                                                              wbP->wwnLun[i].wwn,
                                                              wbP->wwnLun[i].lun);
                            break;

                        case TARG_ID_ADAPTEC_SATA_BAY:
                            rcStat = UpdateAdaptecSataBaySingle((FW_HEADER *) & wbP->wwnLun[wbP->count],
                                                                wbP->wwnLun[i].wwn,
                                                                wbP->wwnLun[i].lun);
                            break;

                        case TARG_ID_XYRATEX_SBOD_BAY:
                            rcStat = UpdateXyratexBay((FW_HEADER *) & wbP->wwnLun[wbP->count],
                                                      wbP->wwnLun[i].wwn,
                                                      wbP->wwnLun[i].lun);
                            break;

                        case TARG_ID_DRIVE_BAY:
                        case TARG_ID_DISK_DRIVE:
                            rcStat = SCSIWriteBufferMode5((FW_HEADER *) & wbP->wwnLun[wbP->count],
                                                          wbP->wwnLun[i].wwn,
                                                          wbP->wwnLun[i].lun);
                            break;

                        default:
                            rcStat.rc = PI_ERROR;
                            break;
                    }

                    /* Indicate failure, but keep going */
                    if (rcStat.rc != PI_GOOD)
                    {
                        tmpRC2 = rcStat.status;
                        rc = PI_ERROR;
                    }
                }
                break;
        }
    }

    /* Unlock the buffer */
    UnlockMutex(&bigBufferMutex);

    /* If parm validation error, everything is already set up. */
    if (parmValEr == FALSE)
    {

        /* Indicate return data in the header.  */
        pRspPacket->pHeader->length = 0;

        /* Pass tmpRC as the error code */
        if (tmpRC2 != PI_GOOD)
        {
            pRspPacket->pHeader->errorCode = tmpRC2;
        }
        else
        {
            pRspPacket->pHeader->errorCode = tmpRC;
        }

        /* Indicate success/error condition. */
        if (tmpRC != PI_GOOD || rc != PI_GOOD)
        {
            rc = pRspPacket->pHeader->status = PI_ERROR;
        }
        else
        {
            rc = pRspPacket->pHeader->status = PI_GOOD;
        }
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    PI_MultiPartXfer
**
** Description: Transfer a large data block (>64K) to or from the CCB in
**              multiple smaller packets.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/

/*
** Note: 1 incoming and 1 outgoing xfer can be interleaved, but two incoming
** or 2 outgoing will not work!
*/
static UINT8 rxMap[MPX_BIT_MAP_SIZE] = { 0 };
static UINT8 rxN;
static UINT8 *rxBufP;

static UINT8 txMap[MPX_BIT_MAP_SIZE] = { 0 };
static UINT8 txN;
static UINT8 *txBufP;
static INT32 leftToSend;

static void BitSet(UINT8 *array, INT32 theBit)
{
    INT32       byte = theBit / 8;
    INT8        bit = 1 << (theBit % 8);

    array[byte] |= bit;
}

static INT8 isBitSet(UINT8 *array, INT32 theBit)
{
    INT32       byte = theBit / 8;
    INT8        bit = 1 << (theBit % 8);

    return array[byte] & bit;
}

INT32 PI_MultiPartXfer(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    X1_MULTI_PART_XFER_REQ *mpxP = (X1_MULTI_PART_XFER_REQ *)pReqPacket->pPacket;
    UINT32      dataLen = pReqPacket->pHeader->length - sizeof(X1_MULTI_PART_XFER_REQ);
    INT32       reason = PI_GOOD;
    INT32       rc = PI_GOOD;
    UINT32      proc = 0;

    /*
     * An MPX_WRITE is a transfer of data TO the CCB.
     * MPX_WRITE's are recorded in the rxMap, since they are incoming
     * packets to the CCB.
     */
    do
    {

        if (mpxP->flags & MPX_WRITE)
        {
            if (mpxP->partX == 0)
            {
                /* Illegal. Should never be 0 on a WRITE */
                rc = PI_ERROR;
                break;
            }

            else if (mpxP->partX == 1)
            {
                /*
                 * A new transfer is starting
                 * Make sure N is within limits
                 */
                if (mpxP->ofN == 0 /* || mpxP->ofN > MPX_MAX_NUM_PACKETS */ )
                {
                    /* Illegal. Should never be 0 or > maxN on a WRITE */
                    rc = PI_ERROR;
                    break;
                }

                /* Set new rxN */
                rxN = mpxP->ofN;

                /* Clear out the map and the rx data buffer */
                memset(rxMap, 0, sizeof(rxMap));
                memset(rxBuffer, 0, SIZEOF_RXBUFFER);

                /* Copy in the data */
                memcpy(rxBuffer, mpxP->data, dataLen);
                rxBufP = rxBuffer + dataLen;

                /* Set the bit in the bitmap */
                BitSet(rxMap, 1);

                /*
                 * Finally, if this was the LAST part (1 of 1 xfer), then do the appropriate
                 * action (burn the code etc).
                 */
                if (mpxP->partX == rxN)
                {
                    rc = HandleFinalPacket(mpxP, &reason);
                }
            }

            else if (mpxP->partX > rxN)
            {
                /* Illegal. Should never be > rxN on a WRITE */
                rc = PI_ERROR;
                break;
            }

            else
            {                   /*  1 < mpxP <= N */
                /* Make sure we got the *next* segment */
                if (isBitSet(rxMap, mpxP->partX))
                {
                    /* Illegal. This segment already rx'd */
                    rc = PI_ERROR;
                    break;
                }
                else if (isBitSet(rxMap, mpxP->partX - 1) == 0)
                {
                    /* Illegal. Previous segment not yet rx'd */
                    rc = PI_ERROR;
                    break;
                }
                /* Make sure N is correct */
                else if (mpxP->ofN != rxN)
                {
                    /* Illegal. Incorrect 'ofN', probably from some other xfer. */
                    rc = PI_ERROR;
                    break;
                }

                /* Copy in the data */
                if ((rxBufP + dataLen) < (rxBuffer + sizeof(rxBuffer)))
                {
                    memcpy(rxBufP, mpxP->data, dataLen);
                    rxBufP += dataLen;
                }
                else
                {
                    /* Too much data... */
                    rc = PI_ERROR;
                    break;
                }

                /* Set the bit in the bitmap */
                BitSet(rxMap, mpxP->partX);

                /*
                 * Finally, if this was the LAST part, then do the appropriate
                 * action (burn the code etc).
                 */
                if (mpxP->partX == rxN)
                {
                    rc = HandleFinalPacket(mpxP, &reason);
                }
            }

            /* Fill in return fields in the header. */
            pRspPacket->pHeader->length = sizeof(X1_MULTI_PART_XFER_RSP);

            /* Pass reason as the error code */
            pRspPacket->pHeader->errorCode = reason;

            /* Indicate success/error condition. */
            if (reason != PI_GOOD || rc != PI_GOOD)
            {
                rc = pRspPacket->pHeader->status = PI_ERROR;
            }
            else
            {
                rc = pRspPacket->pHeader->status = PI_GOOD;
            }

            /* Allocate return data segment */
            pRspPacket->pPacket = MallocWC(sizeof(X1_MULTI_PART_XFER_RSP));
            mpxP->p1 = rc;
            memcpy(pRspPacket->pPacket, pReqPacket->pPacket,
                   sizeof(X1_MULTI_PART_XFER_RSP));
        }

        /*
         * Its NOT an MPX_WRITE (its an MPX_READ) so it a transfer of data to XIOsrv.
         * MPX_READ's are recorded in the txMap, since they are outgoing packets
         * from the CCB.
         */
        else
        {
            if (mpxP->partX == 0)
            {
                /* Illegal. Should never be 0 on a READ */
                rc = PI_ERROR;
                break;
            }

            else if (mpxP->partX == 1)
            {
                /*
                 * A new transfer is starting
                 * Make sure N is 0 on the initial request
                 */
                if (mpxP->ofN != 0)
                {
                    /* Illegal. Should never be anything but 0 on initial READ */
                    rc = PI_ERROR;
                    break;
                }

                /* Go fetch the data to send back */
                switch (mpxP->subCmdCode)
                {
                    case MPX_FILEIO_SCMD:
                        if (DDRisLinuxFileReadFid(mpxP->p1))
                        {
                            reason = FetchLinuxFileData(mpxP, &reason);
                        }
                        else
                        {
                            reason = FetchFIDData(mpxP, &reason);
                        }
                        break;

                    case MPX_MEMIO_SCMD:
                        {
                            proc = (mpxP->flags & MPX_PROC_MASK) >> 1;
                            dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: reading %s memory at 0x%08X, %u bytes\n",
                                    procName[proc], mpxP->p1, mpxP->p2);

                            leftToSend = mpxP->p2;
                            if ((UINT32)leftToSend > sizeof(txBuffer))
                            {
                                rc = PI_ERROR;
                                break;
                            }
                            reason = ReadMemToBuffer(txBuffer, (void *)mpxP->p1, leftToSend, proc);
                            break;
                        }

                    default:
                        rc = PI_ERROR;
                        break;
                }

                /* Set new txN */
                if (((leftToSend + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE) >= 255)
                {
                    txN = mpxP->ofN = 255;
                }
                else
                {
                    txN = mpxP->ofN = (leftToSend + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE;
                }

                /* Set the first bit in the map */
                memset(txMap, 0, sizeof(txMap));
                BitSet(txMap, mpxP->partX);

                /* Calculate dataLen this time */
                dataLen = (leftToSend > MPX_MAX_TX_DATA_SIZE) ? MPX_MAX_TX_DATA_SIZE : leftToSend;
                leftToSend -= dataLen;

                /* Get data buffer */
                pRspPacket->pPacket = MallocWC(sizeof(X1_MULTI_PART_XFER_RSP) + dataLen);

                /* Copy in the control block and the data */
                memcpy(pRspPacket->pPacket, mpxP, sizeof(X1_MULTI_PART_XFER_RSP));
                memcpy(pRspPacket->pPacket + sizeof(X1_MULTI_PART_XFER_RSP), txBuffer, dataLen);

                /* Setup the buffer pointer for next time */
                txBufP = txBuffer + dataLen;
            }

            else if (mpxP->partX > txN)
            {
                /* Illegal. Should never be > txN on a READ */
                rc = PI_ERROR;
                break;
            }

            else
            {                   /*  1 < mpxP <= N */
                /* Make sure the request is for the *next* segment */
                if (isBitSet(txMap, mpxP->partX))
                {
                    /* Illegal. This segment already tx'd */
                    rc = PI_ERROR;
                    break;
                }
                else if (isBitSet(txMap, mpxP->partX - 1) == 0)
                {
                    /* Illegal. Previous segment not yet tx'd */
                    rc = PI_ERROR;
                    break;
                }
                /* Make sure N is correct */
                else if (mpxP->ofN != txN)
                {
                    /* Illegal. Incorrect 'ofN', probably from some other xfer.  */
                    rc = PI_ERROR;
                    break;
                }

                /* Set the bit in the map */
                BitSet(txMap, mpxP->partX);

                if (DDRisLinuxFileReadFid(mpxP->p1))
                {
                    dataLen = DDRLinuxFileRead(txBuffer, MPX_MAX_TX_DATA_SIZE);
                    txBufP = txBuffer;

                    if ((mpxP->partX >= (txN - 1)) &&
                        (leftToSend > 0) && (mpxP->ofN == 255))
                    {
                        if (((leftToSend + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE) >= 255)
                        {
                            txN = mpxP->ofN = 255;
                        }
                        else
                        {
                            txN = mpxP->ofN = (leftToSend + MPX_MAX_TX_DATA_SIZE - 1) / MPX_MAX_TX_DATA_SIZE;
                        }

                        mpxP->partX = 1;

                        /* Set the first bit in the map */
                        memset(txMap, 0, sizeof(txMap));
                        BitSet(txMap, mpxP->partX);
                    }

                    leftToSend -= dataLen;
                }
                else
                {
                    /* Calculate dataLen this time */
                    dataLen = (leftToSend > MPX_MAX_TX_DATA_SIZE) ? MPX_MAX_TX_DATA_SIZE : leftToSend;
                    leftToSend -= dataLen;
                }

                /* Get data buffer */
                pRspPacket->pPacket = MallocWC(sizeof(X1_MULTI_PART_XFER_RSP) + dataLen);

                /* Copy in the control block and the data */
                memcpy(pRspPacket->pPacket, mpxP, sizeof(X1_MULTI_PART_XFER_RSP));
                memcpy(pRspPacket->pPacket + sizeof(X1_MULTI_PART_XFER_RSP), txBufP, dataLen);

                /* Setup the buffer pointer for next time */
                txBufP += dataLen;

                /* If final section */
                if (mpxP->partX == txN)
                {
                    /* Clear out the tx'd map (don't need it anymore) */
                    memset(txMap, 0, sizeof(txMap));
                    if (DDRisLinuxFileReadFid(mpxP->p1))
                    {
                        DDRLinuxFileClose();
                    }
                }
            }

            /* Fill in return fields in the header.  */
            pRspPacket->pHeader->length = dataLen + sizeof(X1_MULTI_PART_XFER_RSP);

            /* Pass reason as the error code */
            pRspPacket->pHeader->errorCode = reason;

            /* Indicate success/error condition. */
            if (reason != PI_GOOD)
            {
                rc = PI_ERROR;
            }

            if (rc != PI_GOOD)
            {
                pRspPacket->pHeader->status = rc;
            }
            else
            {
                pRspPacket->pHeader->status = PI_GOOD;
            }

            mpxP = (X1_MULTI_PART_XFER_RSP *)pRspPacket->pPacket;
            mpxP->p1 = rc;

#if 0
            dprintf(DPRINTF_DEFAULT,
                    "txBuffer: 0x%08X, "
                    "txBufP: 0x%08X, "
                    "dataLen: %u, "
                    "X: %u, "
                    "N: %u, "
                    "leftToSend: %u, "
                    "hdr_len: %u, "
                    "reason: %d, "
                    "status: %d\n",
                    txBuffer, txBufP, dataLen,
                    (unsigned int)mpxP->partX,
                    (unsigned int)mpxP->ofN,
                    leftToSend,
                    pRspPacket->pHeader->length,
                    pRspPacket->pHeader->errorCode, pRspPacket->pHeader->status);
#endif  /* 0 */
        }
    } while (0);

    /* Tell what happened for debug purposes */
    if (rc)
    {
        dprintf(DPRINTF_DEFAULT, "%s: %s %u/%u, %u bytes (ERROR)\n",
            __func__, mpxP->flags & MPX_WRITE ? "recv" : "send",
            (unsigned int)mpxP->partX, (unsigned int)mpxP->ofN, dataLen);
    }
    return rc;
}

/*----------------------------------------------------------------------------
** Function:    HandleFinalPacket
**
** Description: Handle code burn etc on receipt of final packet.
**
** Inputs:      mpxP - pointer to the incoming packet header.
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 HandleFinalPacket(X1_MULTI_PART_XFER_REQ *mpxP, INT32 *reason)
{
    INT32       rc = PI_GOOD;
    UINT32      proc = 0;

    switch (mpxP->subCmdCode)
    {
        case MPX_FW_SCMD:
            /* Validate the fw header */
            *reason = ValidateFWHeader((FW_HEADER *)rxBuffer);

            /*
             * Go update the code
             *
             * p1 == fw type
             * p2 == device pid
             */
            if (*reason == PI_GOOD)
            {
                switch (mpxP->p1)
                {
                    case MPX_FW_TYPE_CONTROLLER:
                        dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: Updating controller fw...\n");
                        *reason = UpdateCode((FW_HEADER *)rxBuffer);
                        break;

                    case MPX_FW_TYPE_PDISK:
                        dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: Updating drive fw...\n");
                        *reason = UpdatePDiskCode((FW_HEADER *)rxBuffer, mpxP->p2);
                        break;

                    case MPX_FW_TYPE_BAY:
                        dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: Updating bay fw...\n");
                        *reason = UpdateBayCode((FW_HEADER *)rxBuffer, mpxP->p2);
                        break;

                    default:
                        *reason = PARM_ERROR;
                        break;
                }
            }
            break;

        case MPX_FILEIO_SCMD:
            dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: writing FID %d, %d bytes\n",
                    mpxP->p1, mpxP->p2);
            if (DDRisLinuxFileReadFid(mpxP->p1))
            {
                rc = DDRLinuxFileSet(rxBuffer, mpxP->p2);
            }
            else
            {
                if (mpxP->flags & MPX_WRITE_NO_HDR)
                {
                    rc = WriteFileAtOffset(mpxP->p1, 1, rxBuffer, mpxP->p2);
                }
                else
                {
                    rc = WriteFile(mpxP->p1, rxBuffer, mpxP->p2);
                }
            }
            break;

        case MPX_MEMIO_SCMD:
            proc = (mpxP->flags & MPX_PROC_MASK) >> 1;
            dprintf(DPRINTF_DEFAULT, "PI_MultiPartXfer: writing %s memory at 0x%08X, %u bytes\n",
                    procName[proc], mpxP->p1, mpxP->p2);

            rc = WriteMemFromBuffer((void *)mpxP->p1,   /* dest */
                                    rxBuffer,           /* src  */
                                    mpxP->p2,           /* length */
                                    proc);              /* processor */
            break;

        default:
            rc = PI_ERROR;
            break;
    }

    /* Clear out the rx'd map (don't need it anymore) */
    memset(rxMap, 0, sizeof(rxMap));

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    FetchFIDData
**
** Description: Go fetch the FID data and copy it to the txBuffer.
**
** Inputs:      pMpx - pointer to the incoming packet header.
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 FetchFIDData(X1_MULTI_PART_XFER_REQ *pMpx, INT32 *reason)
{
    UINT32      fid = pMpx->p1;
    INT32       rc = PI_GOOD;
    UINT32      proc = 0;
    PROC_DDR_TABLE *pDDRT = NULL;
    DDR_FID_HEADER *pFidHdr = (DDR_FID_HEADER *)txBuffer;

    leftToSend = 0;

    /* Initialize header */
    memset(pFidHdr, 0, sizeof(DDR_FID_HEADER));
    pFidHdr->magicNumber = DDR_FID_HEADER_MAGIC_NUM;
    pFidHdr->fid = fid;
    pFidHdr->version = FetchFIDVersion(fid);

    /* Filesystem physical FID */
    if (fid < CCB_FID)
    {
        if (fid >= FS_FID_FIRST_EMPTY)
        {
            return PI_PARAMETER_ERROR;
        }

        leftToSend = GetFileSize(fid);
        if (leftToSend > (INT32)MAX_SEND_SIZE)
        {
            return PI_ERROR;
        }

        /*
         * ReadFileAtOffsetBlockMode(), with offset 1, skips
         * CRC checks and copies directly to our buffer.
         */
        *reason = ReadFileAtOffsetBlockMode(fid, 1, TX_BUFFER_DATA_START, leftToSend);
    }
    else if (fid >= CCB_FID && fid < FE_FID)    /* CCB Logical FID */
    {
        CCB_DDR_ENTRY   *ccb_entry;

        fid &= 0xFF;

        if (fid >= ccbDdrTable.numEntries)
        {
            return PI_PARAMETER_ERROR;
        }

        ccb_entry = &ccbDdrTable.entry[fid];
        if (ccb_entry->pFunc)
        {
            (void)LockMutex(&bigBufferMutex, MUTEX_WAIT); /* Lock big buffer */

            leftToSend = ccb_entry->pFunc(ccb_entry->addr, ccb_entry->len);

            /* Copy the output data */
            leftToSend = leftToSend > (INT32)MAX_SEND_SIZE ? (INT32)MAX_SEND_SIZE : leftToSend;
            memcpy(TX_BUFFER_DATA_START, gBigBuffer, leftToSend);
            strncpy(pFidHdr->id, ccb_entry->id, 8);

            UnlockMutex(&bigBufferMutex);               /* Unlock big buffer */
        }
        else if (ccb_entry->addr)
        {
            /* Else simply copy the data */
            leftToSend = ccb_entry->len > MAX_SEND_SIZE ?
                    MAX_SEND_SIZE : ccbDdrTable.entry[fid].len;
            memcpy(TX_BUFFER_DATA_START, ccb_entry->addr, leftToSend);
            strncpy(pFidHdr->id, ccb_entry->id, 8);

            /* Set the address in the header */
            pFidHdr->startAddr = ccb_entry->addr;
        }
        else
        {
            return PI_ERROR;
        }
    }
    else if (fid >= FE_FID && fid < XX_FID) /* FE/BE Logical FIDs */
    {
        PROC_DDR_ENTRY  *proc_entry;

        if (fid < BE_FID)                   /* FE Logical FID */
        {
            proc = PROCESS_FE;
            pDDRT = (PROC_DDR_TABLE *)FE_DDR_BASE_ADDR;
        }
        else                                /* BE Logical FID */
        {
            proc = PROCESS_BE;
            pDDRT = (PROC_DDR_TABLE *)BE_DDR_BASE_ADDR;
        }

        fid &= 0xFF;

        proc_entry = &pDDRT->entry[fid];
        if (fid < pDDRT->numEntries)
        {
            strncpy(pFidHdr->id, proc_entry->id, 8);

            leftToSend = (proc_entry->len < MAX_SEND_SIZE) ?
                pDDRT->entry[fid].len : MAX_SEND_SIZE;
fprintf(stderr, "ReadMemToBuffer fid=%d (%8.8s), addr=%p, length=%d\n", pMpx->p1, proc_entry->id, proc_entry->addr, proc_entry->len);
            *reason = ReadMemToBuffer(TX_BUFFER_DATA_START,
                                      proc_entry->addr, leftToSend, proc);
        }
        else
        {
            return PI_PARAMETER_ERROR;
        }

        /* Set the address in the header */
        pFidHdr->startAddr = proc_entry->addr;
    }
    else if (fid >= XX_FID && fid < XX_FID + CCB_DMC_MAX)
    {
        fid &= 0xFF;
        rc = DMC(&leftToSend, pFidHdr->id, fid, TX_BUFFER_DATA_START, MAX_SEND_SIZE);
    }
    else
    {
        return PI_ERROR;
    }


    /* If we are sending anything at all, add in the length of the fid header. */
    if (leftToSend)
    {
        leftToSend += sizeof(DDR_FID_HEADER);
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    FetchLinuxFileData
**
** Description: Go fetch the FID data.
**
** Inputs:      pMpx - pointer to the incoming packet header.
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 FetchLinuxFileData(X1_MULTI_PART_XFER_REQ *pMpx, UNUSED INT32 *reason)
{
    UINT32      fid = pMpx->p1;
    INT32       rc = PI_GOOD;
    DDR_FID_HEADER *pFidHdr = (DDR_FID_HEADER *)txBuffer;

    dprintf(DPRINTF_DEFAULT, "FetchLinuxFileData: reading FID %d\n", fid);
    leftToSend = 0;

    do
    {
        /* Initialize header */
        memset(pFidHdr, 0, sizeof(DDR_FID_HEADER));
        pFidHdr->magicNumber = DDR_FID_HEADER_MAGIC_NUM;
        pFidHdr->fid = fid;
        pFidHdr->version = FetchFIDVersion(fid);

        /* CCB Logical FID */
        if (fid >= CCB_FID && fid < FE_FID)
        {
            fid &= 0xFF;

            if (fid >= ccbDdrTable.numEntries)
            {
                rc = PI_PARAMETER_ERROR;
                break;
            }

            if (ccbDdrTable.entry[fid].pFunc)
            {
                /* Lock the big buffer */
                (void)LockMutex(&bigBufferMutex, MUTEX_WAIT);

                /* Call the listed function */
                leftToSend = ccbDdrTable.entry[fid].pFunc(ccbDdrTable.entry[fid].addr,
                                                          ccbDdrTable.entry[fid].len);

                strncpy(pFidHdr->id, ccbDdrTable.entry[fid].id, 8);
                dprintf(DPRINTF_DEFAULT, "FetchLinuxFileData: reading CCB DDR table entry: %s, addr: 0x%08X, len: %u\n",
                        pFidHdr->id, (UINT32)ccbDdrTable.entry[fid].addr, leftToSend);

                /* Copy the first buffer */
                if (leftToSend)
                {
                    DDRLinuxFileRead(TX_BUFFER_DATA_START,
                                     (MPX_MAX_TX_DATA_SIZE - sizeof(DDR_FID_HEADER)));
                }

                /* Unlock the big buffer */
                UnlockMutex(&bigBufferMutex);
            }
            else
            {
                rc = PI_ERROR;
                break;
            }
        }
        else
        {
            rc = PI_ERROR;
            break;
        }
    } while (0);

    /* If we are sending anything at all, add in the length of the fid header. */
    if (leftToSend)
    {
        leftToSend += sizeof(DDR_FID_HEADER);
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    UpdatePDiskCode
**
** Description: Updates FW on a physical disk.
**
** Inputs:      pFW - pointer to FW header
**              pid - pid of the device to send the fw to
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 UpdatePDiskCode(FW_HEADER *pFW, UINT16 pid)
{
    MRGETPINFO_RSP pPInfo;
    INT32       rc;
    RC_STATUS   rc2 = { 0, 0 };

    if ((rc = GetPDiskInfoFromPid(pid, &pPInfo)) == GOOD)
    {
        rc2 = SCSIWriteBufferMode5(pFW, pPInfo.pdd.wwn, pPInfo.pdd.lun);
        rc = rc2.rc;
    }
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    UpdateBayCode
**
** Description: Updates FW on a Eurologic drive bay.
**              Called from the FWUPDATE, MPX command path
**
** Inputs:      pFW - pointer to FW header
**              pid - bid of the device to send the fw to
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 UpdateBayCode(FW_HEADER *pFW, UINT16 bid)
{
    MRGETEINFO_RSP pPInfo;
    INT32       rc = PI_ERROR;
    RC_STATUS   rc2 = { 0, 0 };

    if ((rc = GetDiskBayInfoFromBid(bid, &pPInfo)) == GOOD)
    {
        switch (pFW->target)
        {
            case TARG_ID_EUROLOGIC_BAY:
            case TARG_ID_ADAPTEC_SATA_ES_BAY:
                rc2 = UpdateEurologicBaySingle(pFW, pPInfo.pdd.wwn, pPInfo.pdd.lun);
                rc = rc2.rc;
                break;

            case TARG_ID_XYRATEX_SBOD_BAY:
                rc2 = UpdateXyratexBay(pFW, pPInfo.pdd.wwn, pPInfo.pdd.lun);
                rc = rc2.rc;
                break;

                /*
                 * The rx buffer for this path is not large enough to handle
                 * an Adaptec FW image file, so this path will never be taken.
                 * The CCBCL WRITEBUFFER cmd must be used to update these bays.
                 */
            case TARG_ID_ADAPTEC_SATA_BAY:
                rc2 = UpdateAdaptecSataBaySingle(pFW, pPInfo.pdd.wwn, pPInfo.pdd.lun);
                rc = rc2.rc;
                break;
        }
    }
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    ReadMemToBuffer
**
** Description: Read designated processor memory to static tx buffer.
**
** Inputs:      dest - where to write the data (typically txBuffer)
**              src - where the data is being read from
**              length - how much data to read
**              proc - which processor
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 ReadMemToBuffer(void *dest, void *src, UINT32 length, INT32 proc)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    UINT32      totalRead = 0;
    INT32       rc = PI_GOOD;

    if (length > sizeof(txBuffer))
    {
        return PI_ERROR;
    }

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in PortServerCommandHandler().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_DEBUG_MEM_RDWR_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;

    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    while (totalRead < length)
    {
        /* Fill in the request header */
        reqPacket.pHeader->commandCode = PI_DEBUG_MEM_RDWR_CMD;
        reqPacket.pHeader->length = sizeof(PI_DEBUG_MEM_RDWR_REQ);

        /* Fill in the request parms.  */
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->pAddr = ((UINT8 *)src) + totalRead;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->length = length - totalRead;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->processor = proc;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->mode = MEM_READ;

        /* Call the memread handler */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {
            memcpy(dest,
                   ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->data,
                   ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length);

            /* (UINT8 *)dest += ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length; */
            dest = (void *)((UINT8 *)dest + ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length);
            totalRead += ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length;
        }

        if (rspPacket.pPacket && rc != PI_TIMEOUT)
        {
            Free(rspPacket.pPacket);
        }

        if (rc != PI_GOOD)
        {
            break;
        }

        /*
         * Be a good neighbor and let others run here.  This could be
         * troublesome if we do it because we are now possibly creating a
         * non-contiguous code set.  However, if we don't do it, a 2M
         * copy to the txBuffer can take 2 seconds, which is too long between
         * xchangs...
         */
        TaskSwitch();
    }

    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    WriteMemFromBuffer
**
** Description: Write designated processor memory from static rx buffer.
**
** Inputs:      dest - where to write the data
**              src - where the data is being copied from (typically rxBuffer)
**              length - how much data to write
**              proc - which processor
**
** Returns:     PI_GOOD or PI_ERROR or a failure reason code.
**
**--------------------------------------------------------------------------*/
static INT32 WriteMemFromBuffer(void *dest, void *src, UINT32 length, INT32 proc)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    UINT32      totalWritten = 0;
    INT32       rc = PI_GOOD;

    if (length > sizeof(txBuffer))
    {
        return PI_ERROR;
    }

    /*
     * Allocate memory for the request (header and data) and the
     * response header.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(SIZE_64K + sizeof(PI_DEBUG_MEM_RDWR_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    while (totalWritten < length)
    {
        /* Fill in the request header */
        reqPacket.pHeader->commandCode = PI_DEBUG_MEM_RDWR_CMD;
        reqPacket.pHeader->length = sizeof(PI_DEBUG_MEM_RDWR_REQ);

        /* Fill in the request parms. */
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->pAddr = dest;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->length = length - totalWritten;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->processor = proc;
        ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->mode = MEM_WRITE;
        memcpy(((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->data, src,
               ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->length > SIZE_64K ?
               SIZE_64K : ((PI_DEBUG_MEM_RDWR_REQ *)(reqPacket.pPacket))->length);

        /* Call the memwrite handler */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {
            /* (UINT8 *)dest += ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length; */
            dest = (void *)((UINT8 *)dest + ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length);
            /* (UINT8 *)src  += ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length; */
            src = (void *)((UINT8 *)src + ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length);
            totalWritten += ((PI_DEBUG_MEM_RDWR_RSP *)(rspPacket.pPacket))->length;
        }

        if (rspPacket.pPacket && rc != PI_TIMEOUT)
        {
            Free(rspPacket.pPacket);
        }

        if (rc != PI_GOOD)
        {
            break;
        }

        /*
         * Be a good neighbor and let others run here.  This could be
         * troublesome if we do it because we are now possibly creating a
         * non-contiguous code set.  However, if we don't do it, a 2M
         * copy to the txBuffer can take 2 seconds, which is too long between
         * xchangs...
         */
        TaskSwitch();
    }

    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
