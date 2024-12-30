/* $Id: PI_FileSys.c 143020 2010-06-22 18:35:56Z m4 $ */
/*===========================================================================
** FILE NAME:       PI_FileSys.c
** MODULE TITLE:    Packet Interface for File System Commands
**
** DESCRIPTION:     These functions handle requests to the file system.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "debug_files.h"
#include "FIO.h"
#include "LargeArrays.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PortServer.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 FileSystemRead(UINT32 fileId, UINT8 *pReadBuf);
static INT32 FileSystemWrite(UINT32 fileId, UINT32 writeDataLength, UINT8 *pWriteData);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_FileSystemRead
**
** Description: Read a file ID (FID) from the file system.
**              NOTE: Not all file IDs will be available via this interface
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FileSystemRead(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT32      fileId;
    UINT32      responseLength = 0;
    INT32       rc = PI_GOOD;
    void       *pReadBuf = NULL;

    /*
     * Get the fileId from the input packet.  Determine the size of the
     * response data.
     */
    fileId = ((PI_MISC_FILE_SYSTEM_READ_REQ *)(pReqPacket->pPacket))->fileID;
    responseLength = GetFileSize(fileId);

    if (responseLength > 0)
    {
        /*
         * Allocate memory and read the requested FID.
         */
        pReadBuf = MallocSharedWC(responseLength);

        rc = FileSystemRead(fileId, pReadBuf);

        /*
         * If the read was successful set up the response packet.
         */
        if (rc == PI_GOOD)
        {
            pRspPacket->pPacket = (UINT8 *)pReadBuf;
            pRspPacket->pHeader->length = responseLength;
            pRspPacket->pHeader->status = rc;
            pRspPacket->pHeader->errorCode = rc;
        }
        else
        {
            /*
             * Indicate an error condition and no return data in the header.
             */
            pRspPacket->pHeader->length = 0;
            pRspPacket->pHeader->status = PI_ERROR;
            pRspPacket->pHeader->errorCode = rc;
        }
    }
    else
    {
        /*
         * Indicate an error condition and no return data in the header.
         * If responseLength == 0 set the return status to PI_ERROR.
         */
        if (responseLength == 0)
        {
            rc = PI_ERROR;
        }

        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = responseLength;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_FileSystemWrite
**
** Description: Write a file ID (FID) to the file system
**              NOTE: Not all file IDs will be available via this interface
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FileSystemWrite(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT32      fileId;
    UINT32      writeDataLength = 0;
    UINT8      *pWriteData;
    INT32       rc = PI_GOOD;

    /*
     * Put input parms into local variables for readability.
     */
    fileId = ((PI_MISC_FILE_SYSTEM_WRITE_REQ *)(pReqPacket->pPacket))->fileID;
    pWriteData = ((PI_MISC_FILE_SYSTEM_WRITE_REQ *)(pReqPacket->pPacket))->data;
    writeDataLength = ((PI_MISC_FILE_SYSTEM_WRITE_REQ *)(pReqPacket->pPacket))->length;

    /*
     * Set up common return information. We override this below if necessary.
     */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    /*
     * Call the private function to write the data to the file system.
     */
    rc = FileSystemWrite(fileId, writeDataLength, pWriteData);

    /*
     * If the write failed update the status and error code.
     */
    if (rc != GOOD)
    {
        pRspPacket->pHeader->status = PI_ERROR;
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    FileSystemRead
**
** Description: Read a file ID (FID) from the file system.
**              NOTE: Not all file IDs will be available via this interface
**
** Inputs:      UINT32 fileId   - FID to read
**              UINT8 *pReadBuf - pointer to buffer allocate by the caller
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 FileSystemRead(UINT32 fileId, UINT8 *pReadBuf)
{
    UINT32      responseLength = 0;
    UINT32      blockOffset = 0;
    INT32       rc = PI_GOOD;

    /*
     * Use the file ID to determine the size of the response data.
     * The switch is used as an easy way to list which FIDs are valid
     * and which are not.  Normally a blockOffset of 0 is used which
     * will read the entire FID including the header and validate the CRC.
     * Since the back end FIDs don't have CRC, start reading at
     * blockOffset=1 which will skip the CRC check.
     */
    switch (fileId)
    {
            /* Back end files */
        case FS_FID_DEVLABEL:
        case FS_FID_BE_NVRAM:
        case FS_FID_FE_NVRAM:
        case FS_FID_CCB_NVRAM:
        case FS_FID_BE_SCRATCH:
            blockOffset = 1;
            responseLength = GetFileSize(fileId);
            break;

            /* CCB files */
        case FS_FID_DIRECTORY:
        case FS_FID_QM_MASTER_CONFIG:
        case FS_FID_QM_CONTROLLER_MAP:
        case FS_FID_QM_COMM_AREA:
        case FS_FID_CKP_DIRECTORY:
            blockOffset = 0;
            responseLength = GetFileSize(fileId);
            break;

        default:
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    /*
     * If the FID was valid read it.
     */
    if ((rc == PI_GOOD) && (responseLength > 0))
    {
        rc = ReadFileAtOffset(fileId, blockOffset, pReadBuf, responseLength);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    FileSystemWrite
**
** Description: Write a file ID (FID) to the file system
**              NOTE: Not all file IDs will be available via this interface
**
** Inputs:      UINT32 fileId           - FID to write
**              UINT32 writeDataLength  - # of bytes to write
**              UINT8 *pWriteData       - pointer to write data buffer
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 FileSystemWrite(UINT32 fileId, UINT32 writeDataLength, UINT8 *pWriteData)
{
    UINT32      writeBufSize = 0;
    UINT8      *pWriteBuf = NULL;
    UINT8      *pNameCache = NULL;
    INT32       rc = PI_GOOD;

    /*
     * Use the file ID to determine the size of the write buffer required
     * for this FID.  The entire FID is written even if data for only
     * one name entry is supplied.
     * Some files are cached to DRAM in addition to being written
     * to the file system.
     */
    switch (fileId)
    {
        default:
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    /*
     * If the FID was valid write it.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Always write the entire FID even if the requested data size is
         * smaller.  Allocate memory based on the FID size.
         */
        pWriteBuf = MallocWC(writeBufSize);

        /*
         * Preset the buffer to 0xFF and copy the write data into
         * the buffer.
         */
        memset(pWriteBuf, 0xFF, writeBufSize);
        memcpy(pWriteBuf, pWriteData, writeDataLength);

        /*
         * If there is a valid name cache pointer, copy the
         * write data to the name cache.
         */
        if (pNameCache != NULL)
        {
            memcpy(pNameCache, pWriteBuf, writeBufSize);
        }

        /*
         * Write the buffer to the file system and free the write buffer.
         */
        rc = WriteFile(fileId, pWriteBuf, writeBufSize);
        Free(pWriteBuf);
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
