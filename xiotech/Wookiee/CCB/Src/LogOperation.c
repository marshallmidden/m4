/* $Id: LogOperation.c 160950 2013-04-22 21:10:28Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       LogOperation.c
** MODULE TITLE:    Log operations initiated through the packet interface
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "LogOperation.h"

#include "CacheBay.h"
#include "CachePDisk.h"
#include "CacheServer.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "quorum_utils.h"
#include "X1_Structs.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 GetWWNFromPid(UINT16 pid, UINT64 *wwn);
static INT32 GetWWNFromBid(UINT16 bid, UINT64 *wwn);
#if defined(MODEL_7000) || defined(MODEL_4700)
static INT32 GetWWNandSlotFromPid(UINT16 pid, UINT64 *wwn, UINT16 *slot);
#endif /* MODEL_7000 || MODEL_4700 */

/*****************************************************************************
** Code Start
*****************************************************************************/

#if defined(MODEL_7000) || defined(MODEL_4700)
/*----------------------------------------------------------------------------
**  Function:   GetWWNandSlotFromPid
**
**  Comments:   Take an input pid retrieves the wwn and slot.
**
**  Parameters: UINT16  pid         - pDisk ID
**              UINT64 *wwn         - world wide name output.
**              UINT16 *slot        - Slot output
**
**  Returns:    GOOD or ERROR
**--------------------------------------------------------------------------*/
static INT32 GetWWNandSlotFromPid(UINT16 pid, UINT64 *wwn, UINT16 *slot)
{
    MRGETPINFO_RSP  data;

    /* Retrieve the pdisk information. */
    if (GetPDiskInfoFromPid(pid, &data) == GOOD)
    {
        *wwn = data.pdd.wwn;
        *slot = data.pdd.slot;
        return GOOD;
    }
    *wwn = 0;
    *slot = 0xFF;
    return ERROR;
}
#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
**  Function:   GetWWNFromPid
**
**  Comments:   Take an input pid retrieves the wwn.
**
**  Parameters: UINT16  pid         - pDisk ID
**              UINT64 *wwn         - world wide name output.
**
**  Returns:    GOOD or ERROR
**--------------------------------------------------------------------------*/
static INT32 GetWWNFromPid(UINT16 pid, UINT64 *wwn)
{
    MRGETPINFO_RSP data;

    /* Retrieve the pdisk information. */
    if (GetPDiskInfoFromPid(pid, &data) == GOOD)
    {
        *wwn = data.pdd.wwn;
        return GOOD;
    }
    *wwn = 0;
    return ERROR;
}

/*----------------------------------------------------------------------------
**  Function:   GetWWNFromBid
**
**  Comments:   Take an input bid retrieves the wwn.
**
**  Parameters: UINT16  bid         - diskbay ID
**              UINT64* wwn         - world wide name output.
**
**  Returns:    GOOD or ERROR
**--------------------------------------------------------------------------*/
static INT32 GetWWNFromBid(UINT16 bid, UINT64 *wwn)
{
    MRGETEINFO_RSP data;

    /* Retrieve the pdisk information. */
    if (GetDiskBayInfoFromBid(bid, &data) == GOOD)
    {
        *wwn = data.pdd.wwn;
        return GOOD;
    }
    *wwn = 0;
    return ERROR;
}

/*----------------------------------------------------------------------------
** Function:    LogOperation
**
** Description: Log each request that is made through
**              PacketCommandHandlerImpl().  This will log all requests that
**              are made through the packet interface.  Other log events
**              must be made "manually" in the appropriate places in the code.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or PI_INVALID_CMD_CODE
**
** NOTES:      PLEASE KEEP EVENT CODES IN ORDER AS THEY APPEAR IN DEF.H!
**
**--------------------------------------------------------------------------*/
INT32 LogOperation(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    LOG_MRP    *opLog = NULL;
    void       *pData;
    UINT8       makeLogEntry = TRUE;

    /*
     * Allocate memory for the operation log entry.
     */
    opLog = MallocWC(sizeof(*opLog));

    /*
     * Get a pointer to the data portion of the log event.  This will be
     * cast to each specific event type below.
     */
    pData = opLog->mleData;

    /*
     * If the response is not PI_GOOD, set the log message to an error.
     */
    if (pRspPacket->pHeader->status != PI_GOOD)
    {
        LOG_SetError(opLog->mleEvent);
    }
    else if (pReqPacket->pHeader->flags & PI_HDR_FLG_RESTRAIN_LOG)
    {
        LOG_SetDebug(opLog->mleEvent);
    }
    else
    {
        LOG_SetInfo(opLog->mleEvent);
    }

    /*
     * Create the op log entry based on the command code.
     * Cases where makeLogEntry = FALSE are requests that do not
     * need a log entry.  These are generally information requests.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_CONNECT_CMD:
        case PI_DISCONNECT_CMD:
        case PI_PING_CMD:

        case PI_PDISK_COUNT_CMD:
        case PI_PDISK_LIST_CMD:
        case PI_PDISK_INFO_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_PDISK_LABEL_CMD:
            {
                UINT64      wwn = 0;
#if defined(MODEL_7000) || defined(MODEL_4700)
                UINT16 slot = 0;
#endif /* MODEL_7000 || MODEL_4700 */
                LOG_SetCode(opLog->mleEvent, LOG_PDISK_LABEL_OP);
                opLog->mleLength = sizeof(LOG_PDISK_LABEL_OP_PKT);

                ((LOG_PDISK_LABEL_OP_PKT *)pData)->pid = ((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->pid;

                ((LOG_PDISK_LABEL_OP_PKT *)pData)->labtype = ((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->labtype;

                ((LOG_PDISK_LABEL_OP_PKT *)pData)->option = ((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->option;

                ((LOG_PDISK_LABEL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;
                ((LOG_PDISK_LABEL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
#if defined(MODEL_7000) || defined(MODEL_4700)
                GetWWNandSlotFromPid(((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->pid, &wwn, &slot);
                ((LOG_PDISK_LABEL_OP_PKT *)pData)->slot = slot;
#else  /* MODEL_7000 || MODEL_4700 */
                GetWWNFromPid(((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->pid, &wwn);
#endif /* MODEL_7000 || MODEL_4700 */
                ((LOG_PDISK_LABEL_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_PDISK_DEFRAG_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_PDISK_DEFRAG_OP);
                opLog->mleLength = sizeof(LOG_PDISK_DEFRAG_OP_PKT);

                ((LOG_PDISK_DEFRAG_OP_PKT *)pData)->id = ((PI_PDISK_DEFRAG_REQ *)(pReqPacket->pPacket))->id;

                ((LOG_PDISK_DEFRAG_OP_PKT *)pData)->status = pRspPacket->pHeader->status;
                ((LOG_PDISK_DEFRAG_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                GetWWNFromPid(((PI_PDISK_DEFRAG_REQ *)(pReqPacket->pPacket))->id, &wwn);

                ((LOG_PDISK_DEFRAG_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_PDISK_FAIL_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_PDISK_FAIL_OP);
                opLog->mleLength = sizeof(LOG_PDISK_FAIL_OP_PKT);

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->pid = ((PI_PDISK_FAIL_REQ *)(pReqPacket->pPacket))->pid;

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->hspid = ((PI_PDISK_FAIL_REQ *)(pReqPacket->pPacket))->hspid;

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->options = ((PI_PDISK_FAIL_REQ *)(pReqPacket->pPacket))->options;

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                GetWWNFromPid(((PI_PDISK_FAIL_REQ *)(pReqPacket->pPacket))->pid, &wwn);

                ((LOG_PDISK_FAIL_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_PDISK_SPINDOWN_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_PDISK_SPINDOWN_OP);
                opLog->mleLength = sizeof(LOG_PDISK_SPINDOWN_OP_PKT);

                ((LOG_PDISK_SPINDOWN_OP_PKT *)pData)->pid = ((PI_PDISK_SPINDOWN_REQ *)(pReqPacket->pPacket))->pid;

                ((LOG_PDISK_SPINDOWN_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_PDISK_SPINDOWN_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                GetWWNFromPid(((PI_PDISK_SPINDOWN_REQ *)(pReqPacket->pPacket))->pid, &wwn);

                ((LOG_PDISK_SPINDOWN_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_PDISK_FAILBACK_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_PDISK_FAILBACK_OP);
                opLog->mleLength = sizeof(LOG_PDISK_FAILBACK_OP_PKT);

                ((LOG_PDISK_FAILBACK_OP_PKT *)pData)->pid = ((PI_PDISK_FAILBACK_REQ *)(pReqPacket->pPacket))->hspid;

                ((LOG_PDISK_FAILBACK_OP_PKT *)pData)->options = ((PI_PDISK_FAILBACK_REQ *)(pReqPacket->pPacket))->options;

                ((LOG_PDISK_FAILBACK_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_PDISK_FAILBACK_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                GetWWNFromPid(((PI_PDISK_FAILBACK_REQ *)(pReqPacket->pPacket))->hspid,
                              &wwn);

                ((LOG_PDISK_FAILBACK_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_SET_GEO_LOCATION_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_SET_GEO_LOCATION);
                opLog->mleLength = sizeof(LOG_SET_GEO_LOCATION_OP_PKT);

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->bayId = ((PI_SET_GEO_LOCATION_REQ *)(pReqPacket->pPacket))->bayId;

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->locationId = ((PI_SET_GEO_LOCATION_REQ *)(pReqPacket->pPacket))->locationId;

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->anyHotSpares = ((PI_SET_GEO_LOCATION_RSP *)(pRspPacket->pPacket))->anyHotSpares;

                GetWWNFromBid(((PI_SET_GEO_LOCATION_REQ *)(pReqPacket->pPacket))->bayId,
                              &wwn);

                ((LOG_SET_GEO_LOCATION_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_CLEAR_GEO_LOCATION_CMD:
            {
                LOG_SetCode(opLog->mleEvent, LOG_CLEAR_GEO_LOCATION);
                opLog->mleLength = sizeof(LOG_CLEAR_GEO_LOCATION_OP_PKT);

                ((LOG_CLEAR_GEO_LOCATION_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_CLEAR_GEO_LOCATION_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            }
            break;

/*
        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
            {
            LOG_SetCode(opLog->mleEvent, LOG_PDISK_AUTO_FAILBACK_OP);
            opLog->mleLength = sizeof(LOG_PDISK_AUTO_FAILBACK_OP_PKT);

            ((LOG_PDISK_AUTO_FAILBACK_OP_PKT*)pData)->status = pRspPacket->pHeader->status;

            ((LOG_PDISK_AUTO_FAILBACK_OP_PKT*)pData)->errorCode = pRspPacket->pHeader->errorCode;
            }
            break;
*/

        case PI_PDISK_BEACON_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_PDISK_UNFAIL_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_PDISK_UNFAIL_OP);
                opLog->mleLength = sizeof(LOG_PDISK_UNFAIL_OP_PKT);

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->id = ((PI_PDISK_UNFAIL_REQ *)(pReqPacket->pPacket))->id;

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                GetWWNFromPid(((PI_PDISK_UNFAIL_REQ *)(pReqPacket->pPacket))->id, &wwn);

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->wwn = wwn;
            }
            break;

        case PI_PDISK_DELETE_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_DEVICE_DELETE_OP);
                opLog->mleLength = sizeof(LOG_DEVICE_DELETE_OP_PKT);

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->id = ((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->type = ((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->type;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                if (((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->type == DELETE_DEVICE_DRIVE)
                {
                    GetWWNFromPid(((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did, &wwn);
                }
                else
                {
                    GetWWNFromBid(((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did, &wwn);
                }

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->wwn = wwn;
            }
            break;


        case PI_PDISKS_CMD:
        case PI_PDISKS_FROM_CACHE_CMD:

        case PI_VDISK_COUNT_CMD:
        case PI_VDISK_LIST_CMD:
        case PI_VDISK_INFO_CMD:
            makeLogEntry = FALSE;
            break;

            /*
             * These 3 requests share a common request packet at the MRP
             * level.
             */
        case PI_VDISK_CREATE_CMD:
            if (pReqPacket->pHeader->commandCode == PI_VDISK_CREATE_CMD)
            {
                LOG_SetCode(opLog->mleEvent, LOG_VDISK_CREATE_OP);
            }

        case PI_VDISK_EXPAND_CMD:
            if (pReqPacket->pHeader->commandCode == PI_VDISK_EXPAND_CMD)
            {
                LOG_SetCode(opLog->mleEvent, LOG_VDISK_EXPAND_OP);
            }

        case PI_VDISK_PREPARE_CMD:
            if (pReqPacket->pHeader->commandCode == PI_VDISK_PREPARE_CMD)
            {
                LOG_SetDebug(opLog->mleEvent);
                LOG_SetCode(opLog->mleEvent, LOG_VDISK_PREPARE_OP);
            }

            opLog->mleLength = sizeof(LOG_VDISK_PREPARE_OP_PKT);

            /* Values from request packet */
            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->raidType = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->rtype;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->mirrorDepth = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->depth;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->parity = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->parity;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->drives = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->drives;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->stripe = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->stripe;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->requestCapacity = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->devcap;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->vid = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->vid;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->flags = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->flags;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->minPD = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->minPD;

            /* Values from response packet */
            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->actualCapacity = ((PI_VDISK_CREATE_RSP *)(pRspPacket->pPacket))->devcap;
            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->crossLocation = (UINT8)(((PI_VDISK_CREATE_RSP *)(pRspPacket->pPacket))->clFlag);

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_PREPARE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VDISK_DELETE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VDISK_DELETE_OP);
            opLog->mleLength = sizeof(LOG_VDISK_DELETE_OP_PKT);

            ((LOG_VDISK_DELETE_OP_PKT *)pData)->id = ((PI_VDISK_DELETE_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_VDISK_DELETE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_DELETE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VDISK_CONTROL_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VDISK_CONTROL_OP);
            opLog->mleLength = sizeof(LOG_VDISK_CONTROL_OP_PKT);

            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->subOpType = ((PI_VDISK_CONTROL_REQ *)(pReqPacket->pPacket))->subtype;

            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->srcVid = ((PI_VDISK_CONTROL_REQ *)(pReqPacket->pPacket))->svid;

            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->destVid = ((PI_VDISK_CONTROL_REQ *)(pReqPacket->pPacket))->dvid;

            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            ((LOG_VDISK_CONTROL_OP_PKT *)pData)->mirrorSetType = ((PI_VDISK_CONTROL_RSP *)(pRspPacket->pPacket))->mirrorSetType;
            break;

        case PI_VDISK_SET_PRIORITY_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VDISK_SET_PRIORITY);
            opLog->mleLength = sizeof(LOG_VDISK_SET_PRIORITY_PKT);

            ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

            ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->count = ((MRSETVPRI_REQ *)(pReqPacket->pPacket))->count;
            ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->vid1 = ((MRSETVPRI_REQ *)(pReqPacket->pPacket))->lst[0].vid;
            ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->pri1 = ((MRSETVPRI_REQ *)(pReqPacket->pPacket))->lst[0].pri;
            if (((MRSETVPRI_REQ *)(pReqPacket->pPacket))->count > 1)
            {
                ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->vid2 = ((MRSETVPRI_REQ *)(pReqPacket->pPacket))->lst[1].vid;
                ((LOG_VDISK_SET_PRIORITY_PKT *)pData)->pri2 = ((MRSETVPRI_REQ *)(pReqPacket->pPacket))->lst[1].pri;
            }
            break;

        case PI_VDISK_PR_CLR_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VDISK_PR_CLR);
            opLog->mleLength = sizeof(LOG_VDISK_PR_CLR_PKT);

            ((LOG_VDISK_PR_CLR_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_PR_CLR_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

            ((LOG_VDISK_PR_CLR_PKT *)pData)->id = ((PI_PR_CLR_REQ *)(pReqPacket->pPacket))->id;
            break;

        case PI_VDISK_PR_GET_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_VDISK_OWNER_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_VDISK_SET_ATTRIBUTE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VDISK_SET_ATTRIBUTE_OP);
            opLog->mleLength = sizeof(LOG_VDISK_SET_ATTRIBUTE_OP_PKT);

            ((LOG_VDISK_SET_ATTRIBUTE_OP_PKT *)pData)->mode = ((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->change;

            ((LOG_VDISK_SET_ATTRIBUTE_OP_PKT *)pData)->vid = ((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->attrRequest.vid;

            ((LOG_VDISK_SET_ATTRIBUTE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VDISK_SET_ATTRIBUTE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_VDISKS_CMD:
        case PI_VDISKS_FROM_CACHE_CMD:

        case PI_SERVER_COUNT_CMD:
        case PI_SERVER_LIST_CMD:
        case PI_SERVER_INFO_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_SERVER_CREATE_CMD:
            {
                MRGETSINFO_RSP srvr;

                /*
                 * If we already have a record of this server,
                 * log it as a debug message
                 */
                if (GetServerInfoFromWwn(((PI_SERVER_CREATE_REQ *)(pReqPacket->pPacket))->wwn, &srvr) == GOOD)
                {
                    LOG_SetDebug(opLog->mleEvent);
                }
                else
                {
                    /*
                     * Seems like overkill, but we have to refresh the cache NOW!
                     * to prevent other servers with the same WWN from being logged.
                     */
                    InvalidateCacheServers(true);
                }

                LOG_SetCode(opLog->mleEvent, LOG_SERVER_CREATE_OP);
                opLog->mleLength = sizeof(LOG_SERVER_CREATE_OP_PKT);

                /* Values from request packet */
                ((LOG_SERVER_CREATE_OP_PKT *)pData)->targetId = ((PI_SERVER_CREATE_REQ *)(pReqPacket->pPacket))->targetId;

                ((LOG_SERVER_CREATE_OP_PKT *)pData)->owner = ((PI_SERVER_CREATE_REQ *)(pReqPacket->pPacket))->owner;

                ((LOG_SERVER_CREATE_OP_PKT *)pData)->wwn = ((PI_SERVER_CREATE_REQ *)(pReqPacket->pPacket))->wwn;

                /* Values from response packet */
                ((LOG_SERVER_CREATE_OP_PKT *)pData)->sid = ((PI_SERVER_CREATE_RSP *)(pRspPacket->pPacket))->sid;

                ((LOG_SERVER_CREATE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_SERVER_CREATE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            }
            break;

        case PI_SERVER_DELETE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_SERVER_DELETE_OP);
            opLog->mleLength = sizeof(LOG_SERVER_DELETE_OP_PKT);

            ((LOG_SERVER_DELETE_OP_PKT *)pData)->id = ((PI_SERVER_DELETE_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_SERVER_DELETE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_SERVER_DELETE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_SERVER_ASSOCIATE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_SERVER_ASSOC_OP);
            opLog->mleLength = sizeof(LOG_SERVER_ASSOC_OP_PKT);

            ((LOG_SERVER_ASSOC_OP_PKT *)pData)->sid = ((PI_SERVER_ASSOCIATE_REQ *)(pReqPacket->pPacket))->sid;

            ((LOG_SERVER_ASSOC_OP_PKT *)pData)->lun = ((PI_SERVER_ASSOCIATE_REQ *)(pReqPacket->pPacket))->lun;

            ((LOG_SERVER_ASSOC_OP_PKT *)pData)->vid = ((PI_SERVER_ASSOCIATE_REQ *)(pReqPacket->pPacket))->vid;

            ((LOG_SERVER_ASSOC_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_SERVER_ASSOC_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_SERVER_DISASSOCIATE_CMD:
            /*
             * If the disassociate op is from the default LUN make this a
             * debug message.
             */
            if ((pReqPacket->pHeader->flags & PI_HDR_FLG_RESTRAIN_LOG) &&
                (pRspPacket->pHeader->status == GOOD))

            {
                LOG_SetDebug(opLog->mleEvent);
            }

            LOG_SetCode(opLog->mleEvent, LOG_SERVER_DISASSOC_OP);
            opLog->mleLength = sizeof(LOG_SERVER_DISASSOC_OP_PKT);

            ((LOG_SERVER_DISASSOC_OP_PKT *)pData)->sid = ((PI_SERVER_DISASSOCIATE_REQ *)(pReqPacket->pPacket))->sid;

            ((LOG_SERVER_DISASSOC_OP_PKT *)pData)->lun = ((PI_SERVER_DISASSOCIATE_REQ *)(pReqPacket->pPacket))->lun;

            ((LOG_SERVER_DISASSOC_OP_PKT *)pData)->vid = ((PI_SERVER_DISASSOCIATE_REQ *)(pReqPacket->pPacket))->vid;

            ((LOG_SERVER_DISASSOC_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_SERVER_DISASSOC_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_SERVER_SET_PROPERTIES_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_SERVER_SET_PROPERTIES_OP);
            opLog->mleLength = sizeof(LOG_SERVER_SET_PROPERTIES_OP_PKT);

            ((LOG_SERVER_SET_PROPERTIES_OP_PKT *)pData)->sid = ((PI_SERVER_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->sid;

            ((LOG_SERVER_SET_PROPERTIES_OP_PKT *)pData)->priority = ((PI_SERVER_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->priority;

            ((LOG_SERVER_SET_PROPERTIES_OP_PKT *)pData)->attr = ((PI_SERVER_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->attr;

            ((LOG_SERVER_SET_PROPERTIES_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_SERVER_SET_PROPERTIES_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_SERVER_LOOKUP_CMD:
        case PI_SERVERS_CMD:

        case PI_VLINK_REMOTE_CTRL_COUNT_CMD:
        case PI_VLINK_REMOTE_CTRL_INFO_CMD:
        case PI_VLINK_REMOTE_CTRL_VDISKS_CMD:
        case PI_VLINK_INFO_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_VLINK_CREATE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VLINK_CREATE_OP);
            opLog->mleLength = sizeof(LOG_VLINK_CREATE_OP_PKT);

            /* Values from request packet */
            ((LOG_VLINK_CREATE_OP_PKT *)pData)->cid = ((PI_VLINK_CREATE_REQ *)(pReqPacket->pPacket))->ctrlIndex;

            ((LOG_VLINK_CREATE_OP_PKT *)pData)->vdo = ((PI_VLINK_CREATE_REQ *)(pReqPacket->pPacket))->vdiskOrd;

            ((LOG_VLINK_CREATE_OP_PKT *)pData)->vid = ((PI_VLINK_CREATE_REQ *)(pReqPacket->pPacket))->vid;

            /* Values from response packet */
            memcpy(((LOG_VLINK_CREATE_OP_PKT *)pData)->ctrlName,
                   ((PI_VLINK_CREATE_RSP *)(pRspPacket->pPacket))->ctrlName,
                   NAMES_CONTROLLER_LEN_MAX);

            memcpy(((LOG_VLINK_CREATE_OP_PKT *)pData)->vdName,
                   ((PI_VLINK_CREATE_RSP *)(pRspPacket->pPacket))->vdName,
                   NAMES_VDEVICE_LEN_MAX);

            ((LOG_VLINK_CREATE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VLINK_CREATE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VLINK_BREAK_LOCK_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VLINK_BREAK_LOCK_OP);
            opLog->mleLength = sizeof(LOG_VLINK_BREAK_LOCK_OP_PKT);

            ((LOG_VLINK_BREAK_LOCK_OP_PKT *)pData)->id = ((PI_VLINK_BREAK_LOCK_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_VLINK_BREAK_LOCK_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VLINK_BREAK_LOCK_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VLINK_NAME_CHANGED_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VLINK_NAME_CHANGED);
            opLog->mleLength = sizeof(LOG_VLINK_NAME_CHANGED_OP_PKT);

            ((LOG_VLINK_NAME_CHANGED_OP_PKT *)pData)->id = ((PI_VLINK_NAME_CHANGED_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_VLINK_NAME_CHANGED_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VLINK_NAME_CHANGED_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_TARGET_COUNT_CMD:
        case PI_TARGET_LIST_CMD:
        case PI_TARGET_INFO_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_TARGET_SET_PROPERTIES_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_TARGET_SET_PROPERTIES_OP);
            opLog->mleLength = sizeof(LOG_TARGET_SET_PROPERTIES_OP_PKT);

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->tid = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->tid;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->port = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->port;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->opt = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->opt;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->fcid = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->fcid;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->lock = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->lock;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->pname = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->pname;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->nname = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->nname;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->powner = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->powner;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->owner = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->owner;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->cluster = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->cluster;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->modMask = ((PI_TARGET_SET_PROPERTIES_REQ *)(pReqPacket->pPacket))->modMask;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_TARGET_SET_PROPERTIES_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;
#if ISCSI_CODE
        case PI_ISCSI_SET_TGTPARAM_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_ISCSI_SET_INFO);
            opLog->mleLength = sizeof(LOG_ISCSI_SET_INFO_PKT);

            ((LOG_ISCSI_SET_INFO_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_ISCSI_SET_INFO_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

            ((LOG_ISCSI_SET_INFO_PKT *)pData)->tid = ((MRSETTGINFO_REQ *)(pReqPacket->pPacket))->i_tgd.tid;
            ((LOG_ISCSI_SET_INFO_PKT *)pData)->setmap = ((MRSETTGINFO_REQ *)(pReqPacket->pPacket))->setmap;
            break;

        case PI_ISCSI_SET_CHAP_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_ISCSI_SET_CHAP);
            opLog->mleLength = sizeof(LOG_ISCSI_SET_CHAP_PKT);

            ((LOG_ISCSI_SET_CHAP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_ISCSI_SET_CHAP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

            ((LOG_ISCSI_SET_CHAP_PKT *)pData)->count = ((MRSETCHAP_REQ *)(pReqPacket->pPacket))->count;

            ((LOG_ISCSI_SET_CHAP_PKT *)pData)->option = ((MRSETCHAP_REQ *)(pReqPacket->pPacket))->option;
            break;
#endif  /* ISCSI_CODE */


        case PI_TARGET_RESOURCE_LIST_CMD:
        case PI_TARGETS_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_STATS_GLOBAL_CACHE_CMD:
        case PI_STATS_CACHE_DEVICE_CMD:
        case PI_STATS_FRONT_END_PROC_CMD:
        case PI_STATS_BACK_END_PROC_CMD:
        case PI_STATS_FRONT_END_LOOP_CMD:
        case PI_STATS_BACK_END_LOOP_CMD:
        case PI_STATS_FRONT_END_PCI_CMD:
        case PI_STATS_BACK_END_PCI_CMD:
        case PI_STATS_SERVER_CMD:
        case PI_STATS_VDISK_CMD:
        case PI_STATS_PROC_CMD:
        case PI_STATS_PCI_CMD:
        case PI_STATS_CACHE_DEVICES_CMD:

        case PI_RAID_COUNT_CMD:
        case PI_RAID_LIST_CMD:
        case PI_RAID_INFO_CMD:
        case PI_RAIDS_CMD:
        case PI_RAIDS_FROM_CACHE_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_RAID_INIT_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_RAID_INIT_OP);
            opLog->mleLength = sizeof(LOG_RAID_INIT_OP_PKT);

            ((LOG_RAID_INIT_OP_PKT *)pData)->id = ((PI_RAID_INIT_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_RAID_INIT_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_RAID_INIT_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_RAID_CONTROL_CMD:
            /*
             * Only generate an op log if scrubbing or parity are changed.
             */
            if ((((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->scrubcontrol & SCRUB_CHANGE) ||
                (((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->paritycontrol & PARITY_CHANGE))
            {
                LOG_SetCode(opLog->mleEvent, LOG_RAID_CONTROL_OP);
                opLog->mleLength = sizeof(LOG_RAID_CONTROL_OP_PKT);

                /* Values from request packet */
                ((LOG_RAID_CONTROL_OP_PKT *)pData)->scrubcontrol = ((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->scrubcontrol;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->raidid = ((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->raidid;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->paritycontrol = ((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->paritycontrol;

                /* Values from response packet */
                ((LOG_RAID_CONTROL_OP_PKT *)pData)->sstate = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->sstate;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->pstate = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->pstate;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->scrubpid = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->scrubp;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->scanrid = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->scanr;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->scrubblock = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->scrubb;

                ((LOG_RAID_CONTROL_OP_PKT *)pData)->scanblock = ((MRSCRUBCTRL_RSP *)(pRspPacket->pPacket))->scanb;
            }
            else
            {
                makeLogEntry = FALSE;
            }
            break;


        case PI_ADMIN_FW_VERSIONS_CMD: /* Is an op log entry needed ? */
        case PI_ADMIN_LEDCNTL_CMD:     /* Is an op log entry needed ? */
            makeLogEntry = FALSE;
            break;


        case PI_ADMIN_SETTIME_CMD:     /* Is an op log entry needed ? */
            LOG_SetCode(opLog->mleEvent, LOG_ADMIN_SETTIME_OP);
            opLog->mleLength = sizeof(LOG_ADMIN_SETTIME_OP_PKT);

            /* Values from request packet */
            ((LOG_ADMIN_SETTIME_OP_PKT *)pData)->time = ((PI_SETTIME_REQ *)(pReqPacket->pPacket))->sysTime;

            ((LOG_RAID_CONTROL_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_RAID_CONTROL_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_ADMIN_SET_IP_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_ADMIN_SET_IP_OP);
            opLog->mleLength = sizeof(LOG_ADMIN_SET_IP_OP_PKT);

            /* Values from request packet */
            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->serNum = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->serialNum;

            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->ipAdr = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->ipAddress;

            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->subMaskAdr = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->subnetMask;

            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->gatewayAdr = ((PI_SET_IP_REQ *)(pReqPacket->pPacket))->gatewayAddress;

            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_ADMIN_SET_IP_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_DEBUG_MEM_RDWR_CMD:
        case PI_DEBUG_REPORT_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
            LOG_SetDebug(opLog->mleEvent);
            LOG_SetCode(opLog->mleEvent, LOG_INIT_PROC_NVRAM_OP);
            opLog->mleLength = sizeof(LOG_INIT_PROC_NVRAM_OP_PKT);

            ((LOG_INIT_PROC_NVRAM_OP_PKT *)pData)->type = ((PI_DEBUG_INIT_PROC_NVRAM_REQ *)(pReqPacket->pPacket))->type;

            ((LOG_INIT_PROC_NVRAM_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_INIT_PROC_NVRAM_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_INIT_CCB_NVRAM_OP);
            opLog->mleLength = 0;
            break;

        case PI_DEBUG_GET_SER_NUM_CMD:
        case PI_DEBUG_STRUCT_DISPLAY_CMD:
        case PI_DEBUG_GET_ELECTION_STATE_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_DEBUG_SCSI_COMMAND_CMD:        /* Is an op log entry needed ? */
            makeLogEntry = FALSE;
            break;

        case PI_DEBUG_READWRITE_CMD:            /* Is an op log entry needed ? */
            makeLogEntry = FALSE;
            break;

        case PI_DEBUG_BE_LOOP_PRIMITIVE_CMD:
            if (pReqPacket->pHeader->commandCode == PI_DEBUG_BE_LOOP_PRIMITIVE_CMD)
            {
                ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->proc = 1;
            }
        case PI_DEBUG_FE_LOOP_PRIMITIVE_CMD:
            if (pReqPacket->pHeader->commandCode == PI_DEBUG_FE_LOOP_PRIMITIVE_CMD)
            {
                ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->proc = 0;
            }

            LOG_SetCode(opLog->mleEvent, LOG_LOOP_PRIMITIVE_DEBUG);
            opLog->mleLength = sizeof(LOG_LOOP_PRIMITIVE_OP_PKT);

            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->option = ((PI_LOOP_PRIMITIVE_REQ *)(pReqPacket->pPacket))->option;

            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->id = ((PI_LOOP_PRIMITIVE_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->port = ((PI_LOOP_PRIMITIVE_REQ *)(pReqPacket->pPacket))->port;

            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->lid = ((PI_LOOP_PRIMITIVE_REQ *)(pReqPacket->pPacket))->lid;


            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_LOOP_PRIMITIVE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_PING_CMD:
        case PI_VCG_INFO_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_VCG_PREPARE_SLAVE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_PREPARE_SLAVE_OP);
            opLog->mleLength = sizeof(LOG_VCG_PREPARE_SLAVE_OP_PKT);

            ((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->vcgID = ((PI_VCG_PREPARE_SLAVE_REQ *)(pReqPacket->pPacket))->vcgID;

            ((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->controllerSN = ((PI_VCG_PREPARE_SLAVE_REQ *)(pReqPacket->pPacket))->controllerSN;

            ((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->ipAddress = ((PI_VCG_PREPARE_SLAVE_REQ *)(pReqPacket->pPacket))->ipEthernetAddress;

            memcpy(((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->communicationsKey,
                   ((PI_VCG_PREPARE_SLAVE_REQ *)(pReqPacket->pPacket))->communicationsKey,
                   16);

            ((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_PREPARE_SLAVE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_ADD_SLAVE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_PREPARE_SLAVE_OP);
            opLog->mleLength = sizeof(LOG_VCG_ADD_SLAVE_OP_PKT);

            ((LOG_VCG_ADD_SLAVE_OP_PKT *)pData)->ipAddress = ((PI_VCG_ADD_SLAVE_REQ *)(pReqPacket->pPacket))->ipAddress;

            ((LOG_VCG_ADD_SLAVE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_ADD_SLAVE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_APPLY_LICENSE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_APPLY_LICENSE_OP);
            opLog->mleLength = sizeof(LOG_VCG_APPLY_LICENSE_OP_PKT);

            ((LOG_VCG_APPLY_LICENSE_OP_PKT *)pData)->vcgID = ((PI_VCG_APPLY_LICENSE_REQ *)(pReqPacket->pPacket))->vcgID;

            ((LOG_VCG_APPLY_LICENSE_OP_PKT *)pData)->vcgMaxNumControllers = ((PI_VCG_APPLY_LICENSE_REQ *)(pReqPacket->pPacket))->vcgMaxNumControllers;

            ((LOG_VCG_APPLY_LICENSE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_APPLY_LICENSE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_UNFAIL_CONTROLLER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_UNFAIL_CONTROLLER_OP);
            opLog->mleLength = sizeof(LOG_VCG_UNFAIL_CONTROLLER_OP_PKT);

            ((LOG_VCG_UNFAIL_CONTROLLER_OP_PKT *)pData)->serialNumber = ((PI_VCG_UNFAIL_CONTROLLER_REQ *)(pReqPacket->pPacket))->serialNumber;

            ((LOG_VCG_UNFAIL_CONTROLLER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_UNFAIL_CONTROLLER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_INACTIVATE_CONTROLLER_OP);
            opLog->mleLength = sizeof(LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT);

            ((LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT *)pData)->serialNumber = ((PI_VCG_INACTIVATE_CONTROLLER_REQ *)(pReqPacket->pPacket))->serialNumber;

            ((LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_ACTIVATE_CONTROLLER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_ACTIVATE_CONTROLLER_OP);
            opLog->mleLength = sizeof(LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT);

            ((LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT *)pData)->serialNumber = ((PI_VCG_ACTIVATE_CONTROLLER_REQ *)(pReqPacket->pPacket))->serialNumber;

            ((LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_SET_CACHE_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_SET_CACHE_OP);
            opLog->mleLength = sizeof(LOG_VCG_SET_CACHE_OP_PKT);

            ((LOG_VCG_SET_CACHE_OP_PKT *)pData)->mode = ((PI_VCG_SET_CACHE_REQ *)(pReqPacket->pPacket))->mode;

            ((LOG_VCG_SET_CACHE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_SET_CACHE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            SendAsyncEvent(LOG_VCG_SET_CACHE_OP, 0, NULL);
            break;

        case PI_VCG_FAIL_CONTROLLER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_FAIL_CONTROLLER_OP);
            opLog->mleLength = sizeof(LOG_VCG_FAIL_CONTROLLER_OP_PKT);

            ((LOG_VCG_FAIL_CONTROLLER_OP_PKT *)pData)->serialNumber = ((PI_VCG_FAIL_CONTROLLER_REQ *)(pReqPacket->pPacket))->serialNumber;

            ((LOG_VCG_FAIL_CONTROLLER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_FAIL_CONTROLLER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_REMOVE_CONTROLLER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_REMOVE_CONTROLLER_OP);
            opLog->mleLength = sizeof(LOG_VCG_FAIL_CONTROLLER_OP_PKT);

            ((LOG_VCG_REMOVE_CONTROLLER_OP_PKT *)pData)->serialNumber = ((PI_VCG_REMOVE_CONTROLLER_REQ *)(pReqPacket->pPacket))->serialNumber;

            ((LOG_VCG_REMOVE_CONTROLLER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_VCG_REMOVE_CONTROLLER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_VCG_SHUTDOWN_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_VCG_SHUTDOWN_OP);
            opLog->mleLength = sizeof(LOG_STATUS_ONLY_PKT);

            ((LOG_STATUS_ONLY_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_STATUS_ONLY_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_DISK_BAY_COUNT_CMD:
        case PI_DISK_BAY_LIST_CMD:
        case PI_DISK_BAY_INFO_CMD:
        case PI_DISK_BAYS_CMD:
            makeLogEntry = FALSE;
            break;


        case PI_DISK_BAY_DELETE_CMD:
            {
                UINT64      wwn = 0;

                LOG_SetCode(opLog->mleEvent, LOG_DEVICE_DELETE_OP);
                opLog->mleLength = sizeof(LOG_DEVICE_DELETE_OP_PKT);

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->id = ((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->type = ((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->type;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

                ((LOG_DEVICE_DELETE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;

                if (((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->type == DELETE_DEVICE_DRIVE)
                {
                    GetWWNFromPid(((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did, &wwn);
                }
                else
                {
                    GetWWNFromBid(((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->did, &wwn);
                }

                ((LOG_PDISK_UNFAIL_OP_PKT *)pData)->wwn = wwn;
            }
            break;


        case PI_ENVIRO_DATA_DISK_BAY_CMD:
        case PI_FIRMWARE_DOWNLOAD_CMD:
        case PI_LOG_INFO_CMD:
        case PI_LOG_CLEAR_CMD:
        case PI_WRITE_BUFFER_MODE5_CMD:
        case PI_TRY_CCB_FW_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_LOG_TEXT_MESSAGE_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_GENERIC_CMD:
        case PI_GENERIC2_CMD:
        case PI_GENERIC_MRP_CMD:
            makeLogEntry = FALSE;
            break;

        case PI_PROC_RESTORE_NVRAM_CMD:
            /*
             * There is no longer an op log entry for this operation.
             * The proc will send an event when the NVRAM is restored.
             */
            makeLogEntry = FALSE;
            break;

        case PI_PROC_RESET_FE_QLOGIC_CMD:
            if (((LOG_FE_QLOGIC_RESET_OP_PKT *)pData)->status == GOOD)
            {
                LOG_SetDebug(opLog->mleEvent);
            }

            LOG_SetCode(opLog->mleEvent, LOG_FE_QLOGIC_RESET_OP);
            opLog->mleLength = sizeof(LOG_FE_QLOGIC_RESET_OP_PKT);

            ((LOG_FE_QLOGIC_RESET_OP_PKT *)pData)->port = ((MRRESETFEPORT_REQ *)(pReqPacket->pPacket))->port;

            ((LOG_FE_QLOGIC_RESET_OP_PKT *)pData)->option = ((MRRESETFEPORT_REQ *)(pReqPacket->pPacket))->option;

            ((LOG_FE_QLOGIC_RESET_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_FE_QLOGIC_RESET_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_PROC_RESET_BE_QLOGIC_CMD:
            if (((LOG_BE_QLOGIC_RESET_OP_PKT *)pData)->status == GOOD)
            {
                LOG_SetDebug(opLog->mleEvent);
            }

            LOG_SetCode(opLog->mleEvent, LOG_BE_QLOGIC_RESET_OP);
            opLog->mleLength = sizeof(LOG_BE_QLOGIC_RESET_OP_PKT);

            ((LOG_BE_QLOGIC_RESET_OP_PKT *)pData)->port = ((MRRESETBEPORT_REQ *)(pReqPacket->pPacket))->port;

            ((LOG_BE_QLOGIC_RESET_OP_PKT *)pData)->option = ((MRRESETBEPORT_REQ *)(pReqPacket->pPacket))->option;

            ((LOG_BE_QLOGIC_RESET_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_BE_QLOGIC_RESET_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_PROC_START_IO_CMD:
            LOG_SetDebug(opLog->mleEvent);
            LOG_SetCode(opLog->mleEvent, LOG_PROC_START_IO_OP);
            opLog->mleLength = sizeof(LOG_STATUS_ONLY_PKT);

            ((LOG_STATUS_ONLY_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_STATUS_ONLY_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_PROC_STOP_IO_CMD:
            LOG_SetDebug(opLog->mleEvent);
            LOG_SetCode(opLog->mleEvent, LOG_PROC_STOP_IO_OP);
            opLog->mleLength = sizeof(LOG_STOP_IO_OP_PKT);

            ((LOG_STOP_IO_OP_PKT *)pData)->waitForFlush = ((PI_PROC_STOP_IO_REQ *)(pReqPacket->pPacket))->operation;

            ((LOG_STOP_IO_OP_PKT *)pData)->waitForShutdown = ((PI_PROC_STOP_IO_REQ *)(pReqPacket->pPacket))->intent;

            ((LOG_STOP_IO_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_STOP_IO_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_PROC_ASSIGN_MIRROR_PARTNER_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_PROC_ASSIGN_MIRROR_PARTNER_OP);
            opLog->mleLength = sizeof(LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT);

            ((LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT *)pData)->newSerialNumber = ((PI_PROC_ASSIGN_MIRROR_PARTNER_REQ *)(pReqPacket->pPacket))->serialNumber;

            /* Values from response packet */
            ((LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT *)pData)->oldSerialNumber = ((PI_PROC_ASSIGN_MIRROR_PARTNER_RSP *)(pRspPacket->pPacket))->serialNumber;

            ((LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;


        case PI_PROC_NAME_DEVICE_CMD:
            if (((PI_PROC_NAME_DEVICE_REQ *)pReqPacket->pPacket)->option == MNDRETVCG)
            {
                LOG_SetDebug(opLog->mleEvent);
                LOG_SetCode(opLog->mleEvent, LOG_PROC_NAME_DEVICE_OP);
            }
            else
            {
                LOG_SetCode(opLog->mleEvent, LOG_PROC_NAME_DEVICE_OP);
            }
            opLog->mleLength = sizeof(LOG_PROC_NAME_DEVICE_OP_PKT);

            ((LOG_PROC_NAME_DEVICE_OP_PKT *)pData)->option = ((PI_PROC_NAME_DEVICE_REQ *)(pReqPacket->pPacket))->option;

            /* Values from response packet */
            ((LOG_PROC_NAME_DEVICE_OP_PKT *)pData)->id = ((PI_PROC_NAME_DEVICE_REQ *)(pReqPacket->pPacket))->id;

            memcpy(((LOG_PROC_NAME_DEVICE_OP_PKT *)pData)->name,
                   ((PI_PROC_NAME_DEVICE_REQ *)(pReqPacket->pPacket))->name,
                   NAME_DEVICE_NAME_LEN);

            ((LOG_PROC_NAME_DEVICE_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_PROC_NAME_DEVICE_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_MISC_SET_WORKSET_INFO_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_WORKSET_CHANGED);
            opLog->mleLength = sizeof(LOG_WORKSET_CHANGED_OP_PKT);

            ((LOG_WORKSET_CHANGED_OP_PKT *)pData)->id = ((PI_MISC_SET_WORKSET_INFO_REQ *)(pReqPacket->pPacket))->id;

            ((LOG_WORKSET_CHANGED_OP_PKT *)pData)->status = pRspPacket->pHeader->status;

            ((LOG_WORKSET_CHANGED_OP_PKT *)pData)->errorCode = pRspPacket->pHeader->errorCode;
            break;

        case PI_SETISNSINFO_CMD:
            LOG_SetCode(opLog->mleEvent, LOG_ISNS_CHANGED);
            opLog->mleLength = 0;
            break;

        case PI_PROC_BE_PORT_LIST_CMD:
        case PI_PROC_FE_PORT_LIST_CMD:
        case PI_MISC_GET_DEVICE_COUNT_CMD:
        case PI_MISC_RESCAN_DEVICE_CMD:
        case PI_MISC_GET_BE_DEVICE_LIST_CMD:
        case PI_MISC_GET_FE_DEVICE_LIST_CMD:
        case PI_MISC_FILE_SYSTEM_READ_CMD:
        case PI_MISC_FILE_SYSTEM_WRITE_CMD:
        case PI_MISC_FAILURE_STATE_SET_CMD:
        case PI_MISC_GET_MODE_CMD:
        case PI_MISC_SET_MODE_CMD:
        case PI_MISC_COUNT_CMD:
        default:
            /*
             * Set a flag to indicate a log entry should not be made.
             */
            makeLogEntry = FALSE;
            break;
    }

    /*
     * If the opLog structure was loaded above log the operation.
     */
    if (makeLogEntry == TRUE)
    {
        AsyncEventHandler(NULL, opLog);
    }

    if (opLog != NULL)
    {
        Free(opLog);
    }

    return (PI_GOOD);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
