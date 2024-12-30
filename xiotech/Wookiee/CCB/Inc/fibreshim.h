/* $Id: fibreshim.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       fibreshim.h
** MODULE TITLE:    Header file for fibreshim.c
**
** DESCRIPTION:     The fibre channel shim is responsible for interprocessor
**                  communincations using the Front End Fibre connection.
**                  This is meant as a back-up to the ethernet communications.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _FIBRESHIM_H_
#define _FIBRESHIM_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define DLM_SEND_ETHERNET   0x00

struct _PACKET;

typedef struct _DLM_HEADER
{
    UINT8       serverProcCode; /* Server Processor Code                */
    UINT8       headerLength;   /* Length of Header                     */
    UINT16      sequenceNumber; /* Message Sequence Number              */
    UINT8       functionCode;   /* Request Function Code                */
    UINT8       pathNumber;     /* Path Number                          */
    UINT16      rsvd1;          /* Reserved                             */
    UINT32      srcSerialNumber;        /* Source serial number                 */
    UINT32      destSerialNumber;       /* Destination serial number            */
    char        destServerName[4];      /* Destination server name              */
    UINT32      dataLength;     /* Length of the following data         */
    UINT32      userSpecific;   /* user Specific                        */
    UINT32      headerCRC;      /* Header CRC - filled in by LLD        */
} DLM_PACKET_HEADER;

typedef struct _DLM_PACKET
{
    DLM_PACKET_HEADER header;   /* Packet header                        */
    char        data[0];        /* Packet Data                          */
} DLM_PACKET;

typedef struct _DLM_REQUEST_PACKET
{
    UINT16      functionCode;   /* Funciton Code                        */
    UINT8       rsvd1;          /* Reserved                             */
    UINT8       status;         /* Status                               */
    UINT32      sglPtr;         /* Scatter/ Gather Pointer              */
    UINT32      rsvd2;          /* Reserved                             */
    UINT32      packetPtr;      /* Reserved  (l960)                     */
    UINT32      responseLength; /* Response length                      */
    UINT32      responseAddr;   /* Response address                     */
    UINT32      requestAddr;    /* request address                      */
    UINT32      requestLength;  /* request length                       */
} DLM_REQUEST_PACKET;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 CheckFibrePath(UINT32 controllerSN);
extern INT32 FibreShimHandler(DLM_REQUEST_PACKET *dlmRequestPtr);

/*----------------------------------------------------------------------------
**  Function Name: SendDLMDirect
**
**  Description:
**      This routine sends an IPC packet directly through the DLM, bypassing
**      the TCP/IP stack.
**
**
**
**  Inputs:    destinationSN    = destination Serial Number
**             tx               = transmit packet
**  Outputs:
**
**  Returns:   status           - 0 = Good
**
**
**--------------------------------------------------------------------------*/
extern int  SendDLMDirect(UINT32 destinationSN, struct _PACKET *tx);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FIBRESHIM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
