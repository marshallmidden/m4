/* $Id: PI_PersistentData.c 143020 2010-06-22 18:35:56Z m4 $*/
/**
******************************************************************************
**
**  @file   PI_PersistentData.c
**
**  @brief  Packet Interface for Persistent Data commands
**
**  Allows persistent data to be saved and retrieved from the CCB.
**
** Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PacketInterface.h"

#include "debug_files.h"
#include "errorCodes.h"
#include "ipc_common.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#include "xssa_structure.h"
#include "PI_CmdHandlers.h"

#include "xk_mapmemfile.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define PERSISTENT_DATA_SIZE        (SIZE_64K)
#define PERSISTENT_DATA_RSP_SIZE    (sizeof(PI_PERSISTENT_DATA_CONTROL_RSP))
#define PERSISTENT_DATA_MAX_TX_SIZE (MPX_MAX_TX_DATA_SIZE - PERSISTENT_DATA_RSP_SIZE)

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_PersistentDataControl()
**
** Description: Write or read to persistent Data
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PersistentDataControl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       error = 0;
    UINT32      returnlen = 0;
    UINT32      length = 0;
    UINT32      offset = 0;
    UINT32      count = 0;
    UINT8       checksum = 0;
    UINT8      *nvramPdata = NULL;
    PI_PERSISTENT_DATA_CONTROL_REQ *pDataReqP = NULL;
    PI_PERSISTENT_DATA_CONTROL_RSP *pDataRspP = NULL;

    dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: Enter\n");

    nvramPdata = ((UINT8 *)((UINT32)(&XSSAData)));

    pDataReqP = (PI_PERSISTENT_DATA_CONTROL_REQ *)pReqPacket->pPacket;
    offset = pDataReqP->offset;
    length = pDataReqP->length;

    if ((length > PERSISTENT_DATA_MAX_TX_SIZE) &&
        ((pDataReqP->option == PERSISTENT_DATA_OPTION_READ) ||
         (pDataReqP->option == PERSISTENT_DATA_OPTION_WRITE)))
    {
        rc = PI_ERROR;
        error = PDATA_TOO_MUCH_DATA;
    }

    if (rc == PI_GOOD)
    {
        /*
         * Check to make sure we do not overstep our bounds.
         */
        if ((offset > PERSISTENT_DATA_SIZE) || ((offset + length) > PERSISTENT_DATA_SIZE))
        {
            rc = PI_ERROR;
            error = PDATA_OUT_OF_RANGE;
        }
    }

    if (rc == PI_GOOD)
    {
        /*
         * Check to see what the option is.  Should be READ or WRITE.
         */
        switch (pDataReqP->option)
        {
                /*
                 * We are going to read and return the data read.
                 */
            case PERSISTENT_DATA_OPTION_CHECKSUM:
                dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: PERSISTENT_DATA_OPTION_CHECKSUM, offset: %d length %d\n",
                        offset, length);

                /*
                 * Get the response packet length.
                 */
                returnlen = (sizeof(*pDataRspP));

                /*
                 * Allocate the memory for the response.
                 */
                pDataRspP = MallocWC(returnlen);

                /*
                 * Set the response packet option and length.
                 */
                pDataRspP->option = pDataReqP->option;
                pDataRspP->length = 0;

                /*
                 * Checksum the data.
                 */
                for (count = offset; count < (offset + length); ++count)
                {
                    checksum += nvramPdata[count];
                }

                pDataRspP->checksum = checksum;

                break;

                /*
                 * We are going to read and return the data read.
                 */
            case PERSISTENT_DATA_OPTION_RESET:
                dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: PERSISTENT_DATA_OPTION_RESET, offset: %d length %d\n",
                        offset, length);

                /*
                 * Get the response packet length.
                 */
                returnlen = (sizeof(*pDataRspP));

                /*
                 * Allocate the memory for the response.
                 */
                pDataRspP = MallocWC(returnlen);

                /*
                 * Set the response packet option and length.
                 */
                pDataRspP->option = pDataReqP->option;
                pDataRspP->length = 0;
                pDataRspP->checksum = 0;
                /*
                 * Clear the range of persistent data.
                 */
                for (count = offset; count < (offset + length); ++count)
                {
                    nvramPdata[count] = 0x00;
                }

                /* Flush the NVRAM */

                MEM_FlushMapFile(&nvramPdata[offset], length);
                break;

                /*
                 * We are going to read and return the data read.
                 */
            case PERSISTENT_DATA_OPTION_READ:
                dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: PERSISTENT_DATA_OPTION_READ, offset: %d length %d\n",
                        offset, length);

                /*
                 * Get the response packet length.
                 */
                returnlen = (sizeof(*pDataRspP) + length);

                /*
                 * Allocate the memory for the response.
                 */
                pDataRspP = MallocWC(returnlen);

                /*
                 * Set the response packet option.
                 */
                pDataRspP->option = pDataReqP->option;
                pDataRspP->checksum = 0;

                /*
                 * Copy the data to the response packet.
                 */
                memcpy(pDataRspP->buffer, (nvramPdata + offset), length);

                pDataRspP->length = length;

                break;

                /*
                 * We are going to write the data sent to us.
                 */
            case PERSISTENT_DATA_OPTION_WRITE:
                dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: PERSISTENT_DATA_OPTION_WRITE, offset: %d length %d\n",
                        offset, length);

                /*
                 * Get the response packet length.
                 */
                returnlen = (sizeof(*pDataRspP));

                /*
                 * Allocate the memory for the response.
                 */
                pDataRspP = MallocWC(returnlen);

                /*
                 * Set the response packet option.
                 */
                pDataRspP->option = pDataReqP->option;
                pDataRspP->checksum = 0;

                /*
                 * Copy the data from the request packet.
                 */
                for (count = offset; count < (offset + length); ++count)
                {
                    nvramPdata[count] = pDataReqP->buffer[count - offset];
                }

                /* Flush the NVRAM */

                MEM_FlushMapFile(&nvramPdata[offset], length);

                pDataRspP->length = 0;
                break;

                /*
                 * Invalid option.
                 */
            default:
                rc = PI_ERROR;
                error = PDATA_INVALID_OPTION;
                break;
        }
    }

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = error;
    pRspPacket->pHeader->length = returnlen;
    pRspPacket->pPacket = (UINT8 *)pDataRspP;

    dprintf(DPRINTF_XSSA_DEBUG, "PI_PersistentDataControl: Exit. status: %d  error: %d  returnlen: %d\n",
            rc, error, returnlen);

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
