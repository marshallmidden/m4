/* $Id: PI_Snapshot.c 143020 2010-06-22 18:35:56Z m4 $ */
/*===========================================================================
** FILE NAME:       PI_Snapshot.c
** MODULE TITLE:    Packet Interface for snapshot commands
**
** DESCRIPTION:     Handler functions for SNapshot request packets.
**
** Copyright (c) 2002-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIOPacket.h"
#include "XIO_Macros.h"
#include "PacketInterface.h"
#include "Snapshot.h"
#include "PI_CmdHandlers.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_TakeSnapshot()
**
** Description: Take a snapshot of controller configuration.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_TakeSnapshot(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = 0;
    PI_SNAPSHOT_REQ *snapReqP = (PI_SNAPSHOT_REQ *)pReqPacket->pPacket;

    /*
     * Go take the snapshot
     */
    rc = TakeSnapshot(snapReqP->type, snapReqP->description);

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->length = 0;

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    PI_LoadSnapshot()
**
** Description: Load a snapshot of controller configuration.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_LoadSnapshot(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = 0;
    PI_SNAPSHOT_REQ *snapReqP = (PI_SNAPSHOT_REQ *)pReqPacket->pPacket;

    /*
     * Go load the snapshot
     */
    rc = LoadSnapshot(snapReqP->index, snapReqP->flags);

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->length = 0;

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    PI_ChangeSnapshot()
**
** Description: Change a snapshot description or status in the snapshot dir.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ChangeSnapshot(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = 0;

    PI_SNAPSHOT_REQ *snapReqP = (PI_SNAPSHOT_REQ *)pReqPacket->pPacket;

    /*
     * Go modify the snapshot
     */
    rc = ChangeSnapshot(snapReqP->index, snapReqP->status,
                        snapReqP->description[0] ? snapReqP->description : NULL);

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->length = 0;

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    PI_ReadSnapshotDirectory()
**
** Description: Change a snapshot description or status in the snapshot dir.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ReadSnapshotDirectory(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = 0;
    PI_SNAPSHOTDIR_RSP *rspP = NULL;
    UINT32      rspLen = sizeof(*rspP) + SizeofSnapshotDirectory();

    /*
     * Allocate the response packet
     */
    rspP = MallocWC(rspLen);
    rspP->dirLen = SizeofSnapshotDirectory();

    /*
     * Go read the snapshot directory
     */
    rc = ReadSnapshotDirectory(rspP->dir, rspP->dirLen);

    /*
     * Fill out response packet before returning
     */
    pRspPacket->pPacket = (UINT8 *)rspP;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->length = rspLen;

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
