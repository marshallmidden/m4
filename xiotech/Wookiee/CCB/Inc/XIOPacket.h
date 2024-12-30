/* $Id: XIOPacket.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/*===========================================================================
**
** FILE NAME:       XIOPacket.h
** MODULE TITLE:    Generic Xiotech packet
**
** DESCRIPTION:     This generic packet is used by the Port Server and
**                  the Async Client
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __XIOPACKET_H__
#define __XIOPACKET_H__

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
** Current protocol version
*/
#define PKT_SERVER_PROTOCOL_VER     1

/*
** Controller Type
*/
#define CONTROLLER_TYPE_BIGFOOT     0
#define CONTROLLER_TYPE_WOOKIEE     1
#define CONTROLLER_TYPE_750         2
#define CONTROLLER_TYPE_3100        3
#define CONTROLLER_TYPE_4000        4
#define CONTROLLER_TYPE_7000        5
#define CONTROLLER_TYPE_4700        6
#define CONTROLLER_TYPE_7400        7

/*
** flags definition.
*/
#define PI_HDR_FLG_NONE             0x00000000
#define PI_HDR_FLG_RESTRAIN_LOG     0x00000001
#define PI_HDR_FLG_KILL_IPC_TASK    0x00000002

/*
** Data structures
** NOTE: All packets must be a multiple of 16 bytes long.
*/
typedef struct _PI_PACKET_HEADER
{
    /*
     ** These first 16 bytes MUST NEVER CHANGE! If fields following these
     ** change, then bump the protocolVersion in this quad and then
     ** make the required changes.  In this way, the client can always
     ** assume these first 16 bytes to have consistent meaning and be able
     ** to be de-crypted properly etc.
     **
     ** Note: From an application standpoint, you need only worry about
     ** the "length" field below. Only the Portserver itself and lower
     ** level routines care about the "payloadLength".
     */
    UINT32      headerLength;   /* Length of this header (must be x16)  */
    UINT32      length;         /* Requested data length                */
    UINT32      payloadLength;  /* Actual data length (must be x16)     */
    UINT16      protocolLevel;  /* Packet Interface protocol level      */
    UINT16      packetVersion;  /* Version of individual packet         */
    /* No changes allowed above this point! */

    UINT32      commandCode;    /* Command code                         */
    UINT32      sequenceNumber; /* Sequence number                      */
    UINT64      timeStamp;      /* Time stamp                           */

    UINT16      rsvd1;          /* RESERVED                             */
    UINT8       controllerType; /* Controller Type (0==BF / default)    */
    /*  Only returned on CONNECT.           */
    UINT8       status;         /* Completion status - see below        */
    INT32       errorCode;      /* Add'l error info beyond status       */
    UINT32      timeout;        /* Timeout (MS) for MRP requests        */
    UINT32      flags;          /* Flags to send through                */

    UINT16      senderInterface;        /* IPC: User requested path             */
    UINT16      readInterface;  /* IPC: Interface we read the packet on */
    UINT32      ccbSerialNumber;        /* IPC: CCB serial number               */
    UINT32      socket;         /* socket request came in on            */
    UINT8       rsvd3[4];       /* RESERVED                             */
    UINT8       rsvd4[32];      /* RESERVED                             */
    UINT8       dataMD5[16];    /* MD5 Signature for data packet        */
    UINT8       headerMD5[16];  /* MD5 Signature for header incl. key   */
} PI_PACKET_HEADER, IPC_PACKET_HEADER, *PIPC_PACKET_HEADER;

/*
** The struct below is used for both request and response packets.
*/
typedef struct _XIO_PACKET
{
    PI_PACKET_HEADER *pHeader;  /* Ptr to packet header defined above   */
    UINT8      *pPacket;        /* Ptr to data payload                  */
} XIO_PACKET;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __XIOPACKET_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
