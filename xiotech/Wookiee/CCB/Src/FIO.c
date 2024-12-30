/* $Id: FIO.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       FIO.c
** MODULE TITLE:    File System Functions
**
** DESCRIPTION:     Utility functions for reading and writing files from disk
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "FIO.h"

#include "AsyncEventHandler.h"
#include "ccb_flash.h"
#include "cps_init.h"
#include "crc32.h"
#include "debug_files.h"
#include "EL.h"
#include "errorCodes.h"
#include "FIO_Maps.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "mutex.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "proc_hw.h"
#include "rtc.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private variables
*****************************************************************************/

/* Cached directory structure and flag */

/* Note, bss section is zero upon startup -- thus fileDir is zero. */
static FS_DIR_ENTRY fileDir[256] LOCATE_IN_SHMEM;
static INT32 fileDirCached = 0;
static FIO_DISK_MAP readMap LOCATE_IN_SHMEM;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/* FileIO mutex */
MUTEX       fileIOMutex;
MUTEX       fileSystemMutex;

/*****************************************************************************
** Code Start
*****************************************************************************/

/**********************************************************************
*                                                                     *
*  Name:        SendFileioErr()                                       *
*                                                                     *
*  Description: Write a log message with the error info.
*                                                                     *
*  Input:       f   - Pointer to (this) function (unused)             *
*               api - the api that failed (from the above list)       *
*               rc  - the failing return code                         *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void SendFileioErr(INT32 type, UINT8 api, INT32 rc, UINT8 status,
                   UINT32 fid, UINT16 wr_good, UINT16 wr_err)
{
    LOG_FILEIO_ERR_PKT logMsg;

    memset(&logMsg, 0, sizeof(logMsg));
    logMsg.api = api;
    logMsg.rc = rc;
    logMsg.status = status;
    logMsg.fid = fid;
    logMsg.wr_good = wr_good;
    logMsg.wr_err = wr_err;
    SendAsyncEvent(type, sizeof(logMsg), &logMsg);
}


/**********************************************************************
*                                                                     *
*  Name:        BuildDirectory()                                      *
*                                                                     *
*  Description: Initializes an in-complete directory structure.       *
*               To be called internal to this module only.            *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
static void FillDirectoryEntry(UINT32 fid, UINT32 size, const char *name)
{
    strncpy((char *)fileDir[fid].fileName, name, 10);
    fileDir[fid].lbaOffset = fileDir[fid - 1].lbaOffset + fileDir[fid - 1].lbaCount;
    fileDir[fid].lbaCount = size + 1;
}

static void BuildDirectory(void)
{
    UINT32      i;
    char        name[20];

    /*
     * The first 6 FIDs (0-5) have been written by the BEP.
     * We'll leave those as is.  The rest of these MUST BE CONSECUTIVE FIDS,
     * AND LISTED IN ORDER!!!
     */
    FillDirectoryEntry(FS_FID_QM_MASTER_CONFIG, FS_SIZE_QM_MASTER_CONFIG,
                       FS_FNAME_QM_MASTER_CONFIG);
    FillDirectoryEntry(FS_FID_QM_CONTROLLER_MAP, FS_SIZE_QM_CONTROLLER_MAP,
                       FS_FNAME_QM_CONTORLLER_MAP);
    FillDirectoryEntry(FS_FID_QM_COMM_AREA, FS_SIZE_QM_COMM_AREA, FS_FNAME_QM_COMM_AREA);
    FillDirectoryEntry(FS_FID_FW_COMPAT_DATA, FS_SIZE_FW_COMPAT_DATA,
                       FS_FNAME_FW_COMPAT_DATA);
    FillDirectoryEntry(FS_FID_RSVD_10, FS_SIZE_RSVD_FID_10, FS_FNAME_RSVD_10);
    FillDirectoryEntry(FS_FID_COPY_NVRAM, FS_SIZE_COPY_NVRAM, FS_FNAME_COPY_NVRAM);
    FillDirectoryEntry(FS_FID_RSVD_12, FS_SIZE_RSVD_FID_12, FS_FNAME_RSVD_12);
    FillDirectoryEntry(FS_FID_RSVD_13, FS_SIZE_RSVD_FID_13, FS_FNAME_RSVD_13);
    FillDirectoryEntry(FS_FID_RSVD_14, FS_SIZE_RSVD_FID_14, FS_FNAME_RSVD_14);
    FillDirectoryEntry(FS_FID_RSVD_15, FS_SIZE_RSVD_FID_15, FS_FNAME_RSVD_15);
    FillDirectoryEntry(FS_FID_RSVD_16, FS_SIZE_RSVD_FID_16, FS_FNAME_RSVD_16);
    FillDirectoryEntry(FS_FID_RSVD_17, FS_SIZE_RSVD_FID_17, FS_FNAME_RSVD_17);
    FillDirectoryEntry(FS_FID_RSVD_18, FS_SIZE_RSVD_FID_18, FS_FNAME_RSVD_18);
    FillDirectoryEntry(FS_FID_RSVD_19, FS_SIZE_RSVD_FID_19, FS_FNAME_RSVD_19);
    FillDirectoryEntry(FS_FID_RSVD_20, FS_SIZE_RSVD_FID_20, FS_FNAME_RSVD_20);
    FillDirectoryEntry(FS_FID_RSVD_21, FS_SIZE_RSVD_FID_21, FS_FNAME_RSVD_21);
    FillDirectoryEntry(FS_FID_RSVD_22, FS_SIZE_RSVD_FID_22, FS_FNAME_RSVD_22);
    FillDirectoryEntry(FS_FID_CKP_DIRECTORY, FS_SIZE_CKP_DIRECTORY,
                       FS_FNAME_CKP_DIRECTORY);

    for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
    {
        sprintf(name, "CKP_MCNF%02u", i);
        FillDirectoryEntry(FS_FID_CKP_MASTER_CONFIG + i, FS_SIZE_QM_MASTER_CONFIG, name);
    }

    for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
    {
        sprintf(name, "CKP_CTLM%02u", i);
        FillDirectoryEntry(FS_FID_CKP_CONTROLLER_MAP + i, FS_SIZE_QM_CONTROLLER_MAP,
                           name);
    }

    for (i = 0; i < FS_NUM_SNAPSHOT_FIDS; i++)
    {
        sprintf(name, "CKP_BENV%02u", i);
        FillDirectoryEntry(FS_FID_CKP_BE_NVRAM + i, fileDir[FS_FID_BE_NVRAM].lbaCount - 1,
                           name);
    }

    /*
     * Fill in the unused fids
     */
    for (i = FS_FID_FIRST_EMPTY; i <= FS_FID_LAST_EMPTY; i++)
    {
        sprintf(name, "EMPTY  %03u", i);
        FillDirectoryEntry(i, 0, name);
    }

    return;
}


/**********************************************************************
*                                                                     *
*  Name:        RefreshDirectory()                                    *
*                                                                     *
*  Description: Forces a re-write of the Directory fid to refresh     *
*               any newly labeled drives.                             *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     The rc of WriteFile(). Refer to that function         *
*               for details. (0 on success)                           *
*                                                                     *
**********************************************************************/
INT32 RefreshDirectory(void)
{
    return WriteFile(FS_FID_DIRECTORY, &fileDir, sizeof(fileDir));
}

/**********************************************************************
*                                                                     *
*  Name:        InitFileSystem()                                      *
*                                                                     *
*  Description: Initializes the file system for use.                  *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
static void InitFileSystem(void)
{
    /*
     * Read up the file directory and cache it.
     */
    if (ReadFile(FS_FID_DIRECTORY, &fileDir, sizeof(fileDir)) == 0)
    {
        /*
         * Write out the full directory to refresh any new drives
         */
        BuildDirectory();
        fileDirCached = 1;
        RefreshDirectory();
    }
}

/**********************************************************************
*                                                                     *
*  Name:        FileSystemInitialized()                               *
*                                                                     *
*  Description: Returns the state of the private variable             *
*               fileDirCached                                         *
*                                                                     *
*  Input:       void                                                  *
*                                                                     *
*  Returns:     fileDirCached (private variable)                      *
*                                                                     *
**********************************************************************/
INT32 FileSystemInitialized(void)
{
    INT32       rc = 0;

    if (Qm_GetOwnedDriveCount() > 0)
    {

        /*
         * If the file directory is not cached, initialize the file system.
         */
        if (!fileDirCached)
        {
            InitFileSystem();
        }

        /*
         * Check again to see if the file directory is cached.  If it is the
         * size of the requested file is returned.  If not a size of 0
         * is returned.
         */
        if (fileDirCached)
        {
            rc = fileDirCached;
        }
    }
    else
    {
        rc = fileDirCached = 0;
    }

    return rc;
}

/**********************************************************************
*                                                                     *
*  Name:        WriteFile()                                           *
*                                                                     *
*  Description: Writes a file to the file system.                     *
*                                                                     *
*  Input:       fileID - the "file identifier" that is its index in   *
*                        directory.                                   *
*               buffer - pointer to the buffer where the write data   *
*                        will be copied from.                         *
*               length - the length of the data to write.             *
*                                                                     *
*  Returns:     0 on success, -1..-6 on error. Refer to the code      *
*               for error locations/reasons.                          *
*                                                                     *
**********************************************************************/
INT32 WriteFile(UINT32 fileID, void *buffer, UINT32 length)
{
    /*
     * Acquire the mutex to ensure the file system is in a usable
     * state. Immediately free the mutex to allow overlapped file
     * operations.
     */
    (void)LockMutex(&fileSystemMutex, MUTEX_WAIT);
    UnlockMutex(&fileSystemMutex);

    return (WriteFileAtOffset(fileID, 0, buffer, length));
}

/**********************************************************************
*                                                                     *
*  Name:        WriteFileAtOffset()                                   *
*                                                                     *
*  Description: Writes a file to the file system.                     *
*                                                                     *
*  Input:       fileID - the "file identifier" that is its index in   *
*                        directory.                                   *
*               blkOffset - block offset within file to start xfer    *
*                           0 = start with header                     *
*               buffer - pointer to the buffer where the write data   *
*                        will be copied from.                         *
*               length - the length of the data to write.             *
*                                                                     *
*  Returns:     0 on success, -1..-6 on error. Refer to the code      *
*               for error locations/reasons.                          *
*                                                                     *
**********************************************************************/
INT32 WriteFileAtOffset(UINT32 fileID, UINT32 blkOffset, void *buffer, UINT32 length)
{
    FS_FILE_HEADER *fileHdr = NULL;
    MRFSYSOP_REQ *ptrInPkt = NULL;
    MRFSYSOP_RSP *ptrOutPkt = NULL;
    char       *flashData = NULL;
    TIMESTAMP   ts;
    INT32       fileBlkCnt;
    INT32       rc = 0;
    INT32       mrpRc = PI_GOOD;
    INT32       mutexLocked = 0;
    UINT8       mrpStat = 0;
    UINT16      wr_good = 0;
    UINT16      wr_err = 0;
    INT32       totalBlkCnt;
    INT32       segmentLen;

    do
    {
        /*
         * Make sure the caller provided a buffer to read from. Other input
         * parameters are not range checked, but its a good idea to check
         * pointers...
         */
        if (buffer == NULL)
        {
            rc = FS_ERROR_WRITE_NULL_BUFFER;
            break;
        }

        /* Make sure the directory has been initialized */
        if (!fileDirCached)
        {
            InitFileSystem();

            /* If it didn't work, abort the write. */
            if (!fileDirCached)
            {
                rc = FS_ERROR_WRITE_DIRECT_INIT;
                break;
            }
        }

        /* Lock the fileio mutex */
        (void)LockMutex(&fileIOMutex, MUTEX_WAIT);
        mutexLocked = 1;

        /* Make sure length is within range for this fid */
        if (length > ((UINT32)(fileDir[fileID].lbaCount - 1) * BLOCK_SZ))
        {
            rc = FS_ERROR_WRITE_RANGE_LENGTH;
            break;
        }

        /* Allocate memory for the MRP output packet. */
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));
        fileHdr = MallocSharedWC(sizeof(*fileHdr));

#ifdef ELECTION_SIMULATE_SLOW_FILE_SYSTEM
#define INJECT_SLOW_WRITE_COUNTER       4
#define INJECT_BIG_WRITE_DELAY_COUNTER  10

        if (1)
        {
            static UINT32 slowWriteInjectionCounter = INJECT_SLOW_WRITE_COUNTER;
            static UINT32 bigWriteDelayInjectionCounter = INJECT_BIG_WRITE_DELAY_COUNTER;

            if (EL_TestInProgress() == TRUE)
            {
                if (slowWriteInjectionCounter > 0)
                {
                    if (K_timel & 1)
                    {
                        slowWriteInjectionCounter--;

                        dprintf(DPRINTF_ELECTION, "WFAO: ***** Slow write injection counter is now %d *****\n",
                                slowWriteInjectionCounter);
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "WFAO: Injecting slow write\n");
                    TaskSleepMS(1000);
                    slowWriteInjectionCounter = INJECT_SLOW_WRITE_COUNTER;

                    /* Inject a much bigger delay - less often than the little delay */
                    if (bigWriteDelayInjectionCounter > 0)
                    {
                        if (K_timel & 1)
                        {
                            bigWriteDelayInjectionCounter--;

                            dprintf(DPRINTF_ELECTION, "WFAO: ***** Big write delay injection counter is now %d *****\n",
                                    bigWriteDelayInjectionCounter);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "WFAO: Injecting big write delay\n");
                        TaskSleepMS(4000);
                        bigWriteDelayInjectionCounter = INJECT_BIG_WRITE_DELAY_COUNTER;
                    }
                }
            }
        }
#endif /* ELECTION_SIMULATE_SLOW_FILE_SYSTEM */

        /*
         * If a block offset was not specified, write out the file header
         * with the data
         */
        if (blkOffset == 0)
        {
            /*
             * Fill in the stuff we know
             */
            memset(fileHdr, 0, sizeof(*fileHdr));
            memcpy(fileHdr->fileName, fileDir[fileID].fileName,
                   sizeof(fileHdr->fileName));
            fileHdr->fid = fileID;
            fileHdr->length = length;
            fileHdr->dataCRC = CRC32(buffer, length);

            /*
             * Get a timestamp and fill that in
             */
            RTC_GetTimeStamp(&ts);
            fileHdr->timeStamp.year = ts.year;
            fileHdr->timeStamp.month = ts.month;
            fileHdr->timeStamp.date = ts.date;
            fileHdr->timeStamp.day = ts.day;
            fileHdr->timeStamp.hours = ts.hours;
            fileHdr->timeStamp.minutes = ts.minutes;
            fileHdr->timeStamp.seconds = ts.seconds;

            /*
             * Finally, get a crc of this header
             */
            fileHdr->hdrCRC = CRC32((char *)fileHdr, sizeof(*fileHdr) - sizeof(UINT32));

            /*
             * Allocate memory for the MRP input packet.
             */
            ptrInPkt = MallocWC(sizeof(*ptrInPkt));

            /*
             * Construct and issue the fsysop WRITE mrp to write the header.
             */
            ptrInPkt->op = MFSOPWR;
            ptrInPkt->fid = fileID;
            ptrInPkt->offset = 0;
            ptrInPkt->bcount = 1;
            ptrInPkt->buffptr = fileHdr;
            ptrInPkt->pGoodMap = NULL;

            /*
             * Send the request to Thunderbolt.  This function handles timeout
             * conditions and task switches while waiting.
             */
            mrpRc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFSYSOP,
                               ptrOutPkt, sizeof(*ptrOutPkt), MRP_FSYSOP_TIMEOUT);

            /*
             * Free the input packet
             */
            if (mrpRc != PI_TIMEOUT)
            {
                if (ptrInPkt)
                {
                    Free(ptrInPkt);
                }
            }

            /*
             * At this point, we should have received the return packet.
             */
            mrpStat = ptrOutPkt->header.status;
            wr_good = ptrOutPkt->good;
            wr_err = ptrOutPkt->error;
            switch (mrpRc)
            {
                case PI_GOOD:
                    /*
                     * Remember the disks that were written
                     */
                    FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                    break;

                case PI_TIMEOUT:
                    /*
                     * Getting a timeout is bad, but we can't stop now --
                     * if we do and this write eventually completes, then
                     * we end up with a corrupted file, that can never be
                     * cleaned up.  So, if we get a timeout, go ahead and
                     * write the data too.
                     * MallocW a new output packet, since the old one
                     * is still in use (by the MRP that timed out) and we
                     * are continuing on.
                     */
                    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

                    dprintf(DPRINTF_DEFAULT, "WriteFileAtOffset: MRP timeout writing the file header. Continuing on and writing the data anyway.\n");

                    /* rc = FS_ERROR_WRITE_HEADER_PI_TIMEOUT; */
                    break;

                case PI_ERROR:
                    /*
                     * If 2 or more disks written, call it a good write
                     */
                    if (ptrOutPkt->good >= 2 &&
                        ptrOutPkt->error > 0 && mrpStat == DEIOERR)
                    {
                        SendFileioErr(LOG_FILEIO_DEBUG, FILEIO_WRITE,
                                      mrpRc, mrpStat, fileID, wr_good, wr_err);

                        /*
                         * Remember the disks that were written
                         */
                        FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                        break;
                    }

                    /* Fall through */
                default:
                    rc = FS_ERROR_WRITE_NO_WRITES_HEADER;
                    break;

            }

            /*
             * If error, break out now
             */
            if (rc)
            {
                break;
            }

            /*
             * Increment blkOffset so that the data will be written after
             * the header with the following write.
             */
            blkOffset++;
        }

        if (flashData == NULL)
        {
            /* Calculate a total file block length */
            fileBlkCnt = (length + BLOCK_SZ - 1) / BLOCK_SZ;

            /* Re-Allocate memory for MRP input packet. */
            ptrInPkt = MallocWC(sizeof(*ptrInPkt));

            /* Construct and issue the fsysop WRITE mrp to write the header + data. */
            ptrInPkt->op = MFSOPWR;
            ptrInPkt->fid = fileID;
            ptrInPkt->offset = blkOffset;
            ptrInPkt->bcount = fileBlkCnt;
            ptrInPkt->buffptr = buffer;
            ptrInPkt->pGoodMap = NULL;

            /*
             * Send the request to Thunderbolt.  This function handles timeout
             * conditions and task switches while waiting.
             */
            mrpRc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFSYSOP,
                               ptrOutPkt, sizeof(*ptrOutPkt), MRP_FSYSOP_TIMEOUT);

            /* Free the input packet */
            if (mrpRc != PI_TIMEOUT)
            {
                if (ptrInPkt)
                {
                    Free(ptrInPkt);
                }
            }

            /* At this point, we should have received the return packet. */
            mrpStat = ptrOutPkt->header.status;
            wr_good = ptrOutPkt->good;
            wr_err = ptrOutPkt->error;
            switch (mrpRc)
            {
                case PI_GOOD:
                    /* Remember the disks that were written */
                    FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                    break;

                case PI_TIMEOUT:
                    rc = FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT;
                    break;

                case PI_ERROR:
                    /* If 2 or more disks written, call it a good write */
                    if (ptrOutPkt->good >= 2 &&
                        ptrOutPkt->error > 0 && mrpStat == DEIOERR)
                    {
                        SendFileioErr(LOG_FILEIO_DEBUG, FILEIO_WRITE,
                                      mrpRc, mrpStat, fileID, wr_good, wr_err);

                        /* Remember the disks that were written */
                        FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                        break;
                    }

                    /* Fall through */
                default:
                    rc = FS_ERROR_WRITE_HEADER_DATA_SINGLE;
                    break;
            }

            /* If error, break out now */
            if (rc)
            {
                break;
            }
        }
        else
        {
            /* Loop on 256K writes until complete */
            totalBlkCnt = (length + BLOCK_SZ - 1) / BLOCK_SZ;

            while (totalBlkCnt)
            {
                /* Calculate a total file block length */
                fileBlkCnt = (totalBlkCnt > (SIZE_256K / BLOCK_SZ)) ?  (SIZE_256K / BLOCK_SZ) : totalBlkCnt;
                totalBlkCnt -= fileBlkCnt;

                /* copy the flash data to SRAM */
                segmentLen = fileBlkCnt * BLOCK_SZ;
                memcpy(flashData, buffer, segmentLen);
                /* (char *)buffer += segmentLen; */
                buffer = (void *)((char *)buffer + segmentLen);

                /* Re-Allocate memory for MRP input packet. */
                ptrInPkt = MallocWC(sizeof(*ptrInPkt));

                /* Construct and issue the fsysop WRITE mrp to write the header + data. */
                ptrInPkt->op = MFSOPWR;
                ptrInPkt->fid = fileID;
                ptrInPkt->offset = blkOffset;
                ptrInPkt->bcount = fileBlkCnt;
                ptrInPkt->buffptr = flashData;
                ptrInPkt->pGoodMap = NULL;

                /*
                 * Send the request to Thunderbolt.  This function handles timeout
                 * conditions and task switches while waiting.
                 */
                mrpRc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFSYSOP,
                                   ptrOutPkt, sizeof(*ptrOutPkt), MRP_FSYSOP_TIMEOUT);
                /* Free the input packet */
                if (mrpRc != PI_TIMEOUT)
                {
                    if (ptrInPkt)
                    {
                        Free(ptrInPkt);
                    }
                }

                /* Recalculate the block offset after the write is complete */
                blkOffset += fileBlkCnt;

                /* At this point, we should have received the return packet. */
                mrpStat = ptrOutPkt->header.status;
                wr_good = ptrOutPkt->good;
                wr_err = ptrOutPkt->error;
                switch (mrpRc)
                {
                    case PI_GOOD:
                        /* Remember the disks that were written */
                        FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                        break;

                    case PI_TIMEOUT:
                        rc = FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT;
                        break;

                    case PI_ERROR:
                        /* If 2 or more disks written, call it a good write */
                        if (ptrOutPkt->good >= 2 &&
                            ptrOutPkt->error > 0 && mrpStat == DEIOERR)
                        {
                            SendFileioErr(LOG_FILEIO_DEBUG, FILEIO_WRITE,
                                          mrpRc, mrpStat, fileID, wr_good, wr_err);

                            /* Remember the disks that were written */
                            FIO_SetWritableDiskMap(&ptrOutPkt->goodMap);
                            break;
                        }

                        /* Fall through */
                    default:
                        rc = FS_ERROR_WRITE_HEADER_DATA_LOOP;
                        break;
                }

                /*
                 * If error, break out of the while loop
                 */
                if (rc)
                {
                    break;
                }
            }
        }
    } while (0);

    /* Unlock the fileio mutex */
    if (mutexLocked)
    {
        UnlockMutex(&fileIOMutex);
    }

    /*
     * Free the malloced storage.  Use the delayed free method here
     * since these pointers were passed to the BE by the MRP's.
     */
    DelayedFree(MRFSYSOP, flashData);
    DelayedFree(MRFSYSOP, fileHdr);

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (ptrOutPkt && mrpRc != PI_TIMEOUT)
    {
        /* Note: the input packet is freed by ExecMRP() */
        Free(ptrOutPkt);
    }

    /*
     * 'rc' should only be 0 or negative here.  If its >0, something
     * else bad happened, so report that as an error too.
     */
    if (rc)
    {
        /* Write a log message with the error info */
        SendFileioErr(LOG_FILEIO_ERR, FILEIO_WRITE, rc, mrpStat, fileID, wr_good, wr_err);
    }

    return rc;
}


/**********************************************************************
*                                                                     *
*  Name:        ReadFileBaseFunc()                                    *
*                                                                     *
*  Description: Reads a file from the file system.                    *
*                                                                     *
*  Input:       fileID - the "file identifier" that is its index in   *
*                        directory.                                   *
*               blkOffset - block offset within file to start xfer    *
*                           0 = start with header                     *
*               buffer - pointer to the buffer where the read data    *
*                        will be written to.                          *
*               lengthOfBuf - the length of the provided buffer.      *
*               useMyBuffer - flag indicating that the read data      *
*                        should be placed directly into the provided  *
*                        buffer instead of copied into a temp buffer  *
*                        and then out.  With this flag set only whole *
*                        blocks are returned, so the given buffer     *
*                        MUST BE >= a block multiple of the amount of *
*                        data you want returned.                      *
*                                                                     *
*  Returns:     0 on success, -1..-8 on error.                        *
*               Refer to the code for error locations/reasons.        *
*                                                                     *
**********************************************************************/
INT32 ReadFileBaseFunc(UINT32 fileID, UINT32 blkOffset, void *buffer,
                       UINT32 lengthOfBuf, UINT32 useMyBuffer)
{
    FS_FILE_HEADER *fileHdr = NULL;
    MRFSYSOP_REQ *ptrInPkt = NULL;
    MRFSYSOP_RSP *ptrOutPkt = NULL;
    char       *fileData = NULL;
    INT32       fileBlkCnt;
    INT32       rc = 0;
    INT32       mrpRc = PI_GOOD;
    INT32       mutexLocked = 0;
    UINT8       mrpStat = 0;
    UINT8       useHeader = FALSE;

    /*
     * Acquire the mutex to ensure the file system is in a usable
     * state. Immediately free the mutex to allow overlapped file
     * operations.
     */
    (void)LockMutex(&fileSystemMutex, MUTEX_WAIT);
    UnlockMutex(&fileSystemMutex);

    do
    {
        /*
         * Make sure the caller provided a buffer to write to. Other input
         * parameters are not range checked, but its a good idea to check
         * pointers...
         */
        if (buffer == NULL)
        {
            rc = FS_ERROR_READ_NULL_BUFFER;
            break;
        }

        /*
         * Make sure the directory has been initialized. Note: if the fileID
         * requested is FS_FID_DIRECTORY, go do the read anyway, even if
         * fileDirCached has not been set.
         */
        if (fileID != FS_FID_DIRECTORY && fileDirCached == 0)
        {

            InitFileSystem();

            /*
             * If it didn't work, abort the read.
             */
            if (!fileDirCached)
            {
                rc = FS_ERROR_READ_DIRECT_INIT;
                break;
            }
        }

        /*
         * Lock the fileio mutex
         */
        (void)LockMutex(&fileIOMutex, MUTEX_WAIT);
        mutexLocked = 1;

        /*
         * Allocate memory for the file header & MRP output packet.
         */
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));
        fileHdr = MallocSharedWC(sizeof(*fileHdr));

#ifdef ELECTION_SIMULATE_SLOW_FILE_SYSTEM
#define INJECT_SLOW_READ_COUNTER        4
#define INJECT_BIG_READ_DELAY_COUNTER   10

        if (1)
        {
            static UINT32 slowReadInjectionCounter = INJECT_SLOW_READ_COUNTER;
            static UINT32 bigReadDelayInjectionCounter = INJECT_BIG_READ_DELAY_COUNTER;

            if (EL_TestInProgress() == TRUE)
            {
                if (slowReadInjectionCounter > 0)
                {
                    if (K_timel & 1)
                    {
                        slowReadInjectionCounter--;

                        dprintf(DPRINTF_ELECTION, "RFBF: ***** Slow read injection counter is now %d *****\n",
                                slowReadInjectionCounter);
                    }
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "RFBF: Injecting slow read\n");
                    TaskSleepMS(1000);
                    slowReadInjectionCounter = INJECT_SLOW_READ_COUNTER;

                    /*
                     * Inject a much bigger delay - less often than the little delay
                     */
                    if (bigReadDelayInjectionCounter > 0)
                    {
                        if (K_timel & 1)
                        {
                            bigReadDelayInjectionCounter--;

                            dprintf(DPRINTF_ELECTION, "RFBF: ***** Big read delay injection counter is now %d *****\n",
                                    bigReadDelayInjectionCounter);
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_ELECTION, "RFBF: Injecting big read delay\n");
                        TaskSleepMS(4000);
                        bigReadDelayInjectionCounter = INJECT_BIG_READ_DELAY_COUNTER;
                    }
                }
            }
        }
#endif /* ELECTION_SIMULATE_SLOW_FILE_SYSTEM */

        /*
         * If the block Offset is zero, then read in and process the
         * file header
         */
        if (blkOffset == 0)
        {

            useHeader = TRUE;

            /*
             * Allocate memory for MRP input packet.
             */
            ptrInPkt = MallocWC(sizeof(*ptrInPkt));

            /*
             * Construct and issue the fsysop READ mrp to read the header.
             */
            ptrInPkt->op = MFSOPRD;
            ptrInPkt->fid = fileID;
            ptrInPkt->confirm = 1;
            ptrInPkt->offset = blkOffset;       /* The file header is at offset */
            ptrInPkt->bcount = 1;       /* 0, and is 1 block long.      */
            ptrInPkt->buffptr = fileHdr;

            if (FIO_GetReadableDiskMap(&readMap) == GOOD)
            {
                ptrInPkt->pGoodMap = &readMap;

                /*
                 * If we have less than 2 disks, we do not need to confirm.
                 */
                if (FIO_GetNumDiskMapDisks(&readMap) < 2)
                {
                    ptrInPkt->confirm = 0;
                }
            }

            /*
             * Send the request to Thunderbolt.  This function handles timeout
             * conditions and task switches while waiting.
             */
            mrpRc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFSYSOP,
                               ptrOutPkt, sizeof(*ptrOutPkt), MRP_FSYSOP_TIMEOUT);

            /*
             * Free the input packet
             */
            if (mrpRc != PI_TIMEOUT)
            {
                if (ptrInPkt)
                {
                    Free(ptrInPkt);
                }
            }

            /*
             * At this point, we should have received the return packet.
             */
            mrpStat = ptrOutPkt->header.status;
            if (mrpRc != PI_GOOD)
            {
                if (mrpRc == PI_TIMEOUT)
                {
                    rc = FS_ERROR_READ_HEADER_PI_TIMEOUT;
                }
                else
                {
                    rc = FS_ERROR_READ_HEADER;
                }
                break;
            }

            /*
             * Check the header's CRC
             */
            if (CRC32((char *)fileHdr, sizeof(*fileHdr) - sizeof(UINT32)) != fileHdr->hdrCRC)
            {
                rc = FS_ERROR_READ_CRC_CHECK_HEADER;
                break;
            }

            /*
             * Adjust the block offset out past the header
             */
            ++blkOffset;
        }

        /*
         * Read the actual file data.  Round down and/or truncate what's read
         * to fit in the buffer provided.
         */
        if (useMyBuffer)
        {
            INT32       sz2 = 0;

            fileBlkCnt = lengthOfBuf / BLOCK_SZ;

            if (useHeader == TRUE)
            {
                sz2 = (fileHdr->length + BLOCK_SZ - 1) / BLOCK_SZ;

                if (sz2 < fileBlkCnt)
                {
                    fileBlkCnt = sz2;
                }
            }
        }

        /*
         * Otherwise we're using an internal buffer
         */
        else
        {

            if (useHeader == TRUE)
            {
                /*
                 * fileHdr is valid here since we just read it up
                 */
                fileBlkCnt = (fileHdr->length + BLOCK_SZ - 1) / BLOCK_SZ;
            }
            else
            {
                /*
                 * No file header was read up so use the length provided
                 */
                fileBlkCnt = (lengthOfBuf + BLOCK_SZ - 1) / BLOCK_SZ;
            }

            /*
             * Get memory to read the file data into
             */
            fileData = MallocSharedW(fileBlkCnt * BLOCK_SZ);
        }

        /*
         * Construct and issue the fsysop READ mrp to read the file data.
         * Note: the previous ptrInPkt (if one was malloc'd) was freed by ExecMRP().
         */
        ptrInPkt = MallocWC(sizeof(*ptrInPkt));
        if (ptrInPkt == NULL)
        {
            rc = FS_ERROR_READ_MALLOC_DATA;
            break;
        }

        ptrInPkt->op = MFSOPRD;
        ptrInPkt->fid = fileID;
        ptrInPkt->confirm = 1;
        ptrInPkt->offset = blkOffset;
        ptrInPkt->bcount = fileBlkCnt;

        if (FIO_GetReadableDiskMap(&readMap) == GOOD)
        {
            ptrInPkt->pGoodMap = &readMap;

            /*
             * If we have less than 2 disks, we do not need to confirm.
             */
            if (FIO_GetNumDiskMapDisks(&readMap) < 2)
            {
                ptrInPkt->confirm = 0;
            }
        }

        if (useMyBuffer)
        {
            ptrInPkt->buffptr = buffer;
        }
        else
        {
            ptrInPkt->buffptr = fileData;
        }

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        mrpRc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFSYSOP,
                           ptrOutPkt, sizeof(*ptrOutPkt), MRP_FSYSOP_TIMEOUT);

        /*
         * Free the input packet
         */
        if (mrpRc != PI_TIMEOUT)
        {
            if (ptrInPkt)
            {
                Free(ptrInPkt);
            }
        }

        /*
         * At this point, we should have received the return packet.
         */
        mrpStat = ptrOutPkt->header.status;
        if (mrpRc != PI_GOOD)
        {
            if (mrpRc == PI_TIMEOUT)
            {
                rc = FS_ERROR_READ_DATA_PI_TIMEOUT;
            }
            else
            {
                rc = FS_ERROR_READ_DATA;
            }
            break;
        }

        /*
         * Check the file's CRC, if the header was read  (blkOffset == 0)
         */
        if (useHeader == TRUE)
        {

            if (CRC32(useMyBuffer ? buffer : fileData, fileHdr->length) != fileHdr->dataCRC)
            {
                rc = FS_ERROR_READ_CRC_CHECK_DATA;
                break;
            }
        }

    } while (0);

    /*
     * Unlock the fileio mutex
     */
    if (mutexLocked)
    {
        UnlockMutex(&fileIOMutex);
    }

    /*
     * Copy out the file data and free the buffer if one was allocated
     */
    if (fileData)
    {
        /*
         * Copy data to user's buffer if required.
         */
        if (useMyBuffer == 0)
        {

            /*
             * Only return the actual file data even if more space was provided
             */
            if (useHeader == TRUE)
            {
                memcpy(buffer, fileData,
                       lengthOfBuf > fileHdr->length ? fileHdr->length : lengthOfBuf);
            }
            /*
             * If we didn't read up a file header, then just copy out however
             * much was requested.
             */
            else
            {
                memcpy(buffer, fileData, lengthOfBuf);
            }
        }
    }

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (ptrOutPkt && mrpRc != PI_TIMEOUT)
    {
        /* Note: the input packet is freed by ExecMRP() */
        Free(ptrOutPkt);
    }

    /*
     * Free the malloced storage.  Use the delayed free method here
     * since these pointers were passed to the BE by the MRP's.
     */
    DelayedFree(MRFSYSOP, fileHdr);
    DelayedFree(MRFSYSOP, fileData);

    /*
     * 'rc' should only be 0 or negative here.  If its >0, something
     * else bad happened, so report that as an error too.
     */
    if (rc)
    {
        /*
         * Write a log message with the error info
         */
        SendFileioErr(LOG_FILEIO_ERR, FILEIO_READ, rc, mrpStat, fileID, 0, 0);
    }

    return rc;
}


/**********************************************************************
*                                                                     *
*  Name:        GetFileSize()                                         *
*                                                                     *
*  Description: Returns the size of a file (in bytes) from the        *
*               directory.                                            *
*                                                                     *
*  Input:       fid - the "file identifier" that is its index in      *
*                     directory.                                      *
*                                                                     *
*  Returns:     size of a file (in bytes) or 0 if the directory is    *
*               not valid.                                            *
*                                                                     *
**********************************************************************/
UINT32 GetFileSize(INT32 fid)
{
    UINT32      fidSize = 0;

    /*
     * If the file directory is not cached, initialize the file system.
     */
    if (!fileDirCached)
    {
        InitFileSystem();
    }

    /*
     * Check again to see if the file directory is cached.  If it is the
     * size of the requested file is returned.  If not a size of 0
     * is returned.
     */
    if (fileDirCached)
    {
        fidSize = (fileDir[fid].lbaCount - 1) * BLOCK_SZ;
    }

    return (fidSize);
}

UINT32 GetFileSizeInBlocksIncHeader(INT32 fid)
{
    UINT32      fidSize = 0;

    /*
     * If the file directory is not cached, initialize the file system.
     */
    if (!fileDirCached)
    {
        InitFileSystem();
    }

    /*
     * Check again to see if the file directory is cached.  If it is the
     * size of the requested file is returned.  If not a size of 0
     * is returned.
     */
    if (fileDirCached)
    {
        fidSize = fileDir[fid].lbaCount;
    }

    return fidSize;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
