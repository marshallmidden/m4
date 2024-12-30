/* $Id: PI_Pr.c 162797 2014-02-27 19:30:40Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       PI_Pr.c
** MODULE TITLE:    Packet Interface and Ipc for persistent reserve feature.
**
** DESCRIPTION:     Allows Persistent reserve parameters are handled and
**                  send ipc packet if required
**
** Copyright (c) 2006-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "PacketInterface.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "ipc_common.h"
#include "ipc_sendpacket.h"
#include "ipc_packets.h"
#include "logview.h"
#include "quorum_utils.h"
#include "rm_val.h"
#include "LOG_Defs.h"
#include "PI_ClientPersistent.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "slink.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

/*****************************************************************************
 ** Public functions not in any header file.
 *****************************************************************************/
extern void SendPresPacketToMaster(LOG_PRES_EVENT_PKT *pLOG);

/**
******************************************************************************
**
**  @brief
**
******************************************************************************
**/
void SendPresPacketToMaster(LOG_PRES_EVENT_PKT *pLOG)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc;
    UINT32      reqSize = pLOG->dataSize;
    MRPRCONFIGCOMPLETE_REQ *pInPkt;
    MRPRCONFIGCOMPLETE_RSP *pOutPkt;

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocSharedWC(reqSize);
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_SET_PR_CMD;
    reqPacket.pHeader->length = reqSize;

    /* Store the MR packet in the PI packet */
    memcpy(reqPacket.pPacket, pLOG->data, reqPacket.pHeader->length);

    /* Setup the completion struct from the log msg. */
    pInPkt = MallocWC(sizeof(*pInPkt));
    pOutPkt = MallocSharedWC(sizeof(*pOutPkt));
    /*
     * Copy the sid vid and lun to MRPRCONFIGCOMPLETE request packet
     */
    pInPkt->vid = pLOG->vid;
    pInPkt->sid = pLOG->sid;
    pInPkt->data = pLOG->data;
    pInPkt->dataSize = pLOG->dataSize;
    pInPkt->rc = PI_ERROR;

    if (TestforMaster(GetMyControllerSN()))
    {
        pInPkt->rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
        do
        {
            if (pInPkt->rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            pInPkt->rc = TunnelRequest(Qm_GetMasterControllerSN(), &reqPacket, &rspPacket);
        } while (pInPkt->rc != GOOD && (retries--) > 0);
    }
    /*
     * Send the required data and return code (rc)
     * to FE the ITL Nexus completed the PR config complete.
     */
    rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRPRCONFIGCOMPLETE,
                    pOutPkt, sizeof(*pOutPkt), GetGlobalMRPTimeout());

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (pInPkt->rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }
    Free(pInPkt);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
