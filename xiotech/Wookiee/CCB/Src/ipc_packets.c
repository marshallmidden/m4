/* $Id: ipc_packets.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_packets.c
** MODULE TITLE:    IPC packet handling functions
**
** DESCRIPTION:     Implementation functions that create and destroy packets
**                  and any other packet specfic functions.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "XIO_Std.h"
#include "ipc_packets.h"
#include "ipc_session_manager.h"
#include "misc.h"
#include "quorum_utils.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define SetPacketHeaderProtocolLevel(pHdr) (pHdr->protocolLevel = IPC_PROTOCOL_LEVEL)
#define SetPacketHeaderVersion(pHdr) (pHdr->packetVersion = IPC_PACKET_VERSION_0)

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: CreateHeader
**
**  Description:    Correctly creates a header, i.e. allocates the memory
**                  and sets the seqnum and ccb serial number up.
**                  Should always use this function for creating headers
**
**  Inputs: None
**
**  Returns:    Pointer to a new packet header
**--------------------------------------------------------------------------*/
static IPC_PACKET_HEADER *CreateHeader(const char *file, const UINT32 line)
{
    IPC_PACKET_HEADER *p = p_XK_MallocWC(sizeof(*p), file, line);

    /*
     * We want to clear out the entire header and then make sure to set
     * the session number and serial number up correctly as these are
     * very important to determine if a packet is a response to a request
     * or a new packet request
     */
    SetSeqNum(p, NextSequenceNumber());
    SetCCBSerialNumber(p, GetMyControllerSN());

    /*
     * Set up the packet protocol level and the default packet version
     */
    SetPacketHeaderProtocolLevel(p);
    SetPacketHeaderVersion(p);

    return p;
}

/*----------------------------------------------------------------------------
**  Function Name: CreatePacket
**
**  Description:    Correctly creates a packet, i.e. allocates the memory
**
**  Inputs:     UINT16  commandCode     Yes the command code
**              size_t  dataPacketSize  Size of the data portion of the packet
**
**  Returns:    Pointer to a new packet header, NULL if we encounter errors
**--------------------------------------------------------------------------*/
IPC_PACKET *CreatePacket(UINT16 commandCode, size_t dataPacketSize, const char *file, const UINT32 line)
{
    IPC_PACKET *p = p_XK_MallocWC(sizeof(*p), file, line);

    p->header = CreateHeader(file, line);

    /*
     * Set the command code and size of the data.
     */
    p->header->commandCode = commandCode;
    p->header->length = dataPacketSize;

    if (dataPacketSize)
    {
        p->data = s_XK_MallocWC(dataPacketSize, file, line);
    }

    return p;
}

/*----------------------------------------------------------------------------
**  Function Name:  CreateResponsePacket
**
**  Description:    Correctly creates a response packet based off of an
**                  existing packet
**
**  Inputs: UINT16          commandCode     newly created packet cc
**          IPC_PACKET     *pattern         Packet we are using to build a
**                                          response
**          size_t          dataPacketSize  Size of the data portion of the
**                                          packet
**
**  Returns:    Pointer to a new packet header, NULL if we encounter errors
**--------------------------------------------------------------------------*/
IPC_PACKET *CreateResponsePacket(UINT16 commandCode,
                                 IPC_PACKET *pattern, size_t dataPacketSize,
                                 const char *file, const UINT32 line)
{
    IPC_PACKET *rc = NULL;

    ccb_assert(pattern != NULL, pattern);
    ccb_assert(pattern->header != NULL, pattern->header);

    rc = CreatePacket(commandCode, dataPacketSize, file, line);

    /*
     * Copy over the vital data needed for the ipc layer
     */
    rc->header->ccbSerialNumber = pattern->header->ccbSerialNumber;
    rc->header->sequenceNumber = pattern->header->sequenceNumber;
    rc->header->packetVersion = pattern->header->packetVersion;

    return rc;
}

/*----------------------------------------------------------------------------
**  Function Name: FreePacket
**
**  Description:    Correctly free a packet
**
**  Inputs: Packet ot be free'd
**
*** Outputs: Pointer that is passed in is set to NULL
**
**  Returns:    None
**--------------------------------------------------------------------------*/
void FreePacket(IPC_PACKET **packet, const char *file, const UINT32 line)
{
    IPC_PACKET *p = *packet;
    ccb_assert(packet != NULL, packet);

    if (p)
    {
        if (p->data)
        {
            Free_Memory_Ptr(p->data, p->header->length, file, line);
        }
        if (p->header)
        {
            Free_Memory_Ptr(p->header, sizeof(IPC_PACKET_HEADER), file, line);
        }
        Free_Memory_Ptr(p, sizeof(IPC_PACKET), file, line);
        *packet = 0;
    }
}


void FreePacketStaticPacketPointer(IPC_PACKET *p, const char *file, const UINT32 line)
{
    ccb_assert(p != NULL, p);

    if (p->data)
    {
        Free_Memory_Ptr(p->data, p->header->length, file, line);
        p->data = 0;
    }
    if (p->header)
    {
        Free_Memory_Ptr(p->header, sizeof(IPC_PACKET_HEADER), file, line);
        p->header = 0;
    }
}


bool GotCmdInProgress(IPC_PACKET *rx)
{
    bool        rc = FALSE;

    ccb_assert(rx != NULL, rx);
    ccb_assert(rx->header != NULL, rx->header);

    if (rx && rx->header && rx->data)
    {
        if ((PACKET_IPC_COMMAND_STATUS == rx->header->commandCode) &&
            (IPC_COMMAND_IN_PROGRESS == rx->data->commandStatus.status))
        {
            rc = TRUE;
        }
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name:  CopyPacket
**
**  Description:    Correctly copy a packet
**
**  Inputs:     Packet to be copied
**
*** Outputs:    Packet is copied
**
**  Returns:    Pointer to copy of packet
**--------------------------------------------------------------------------*/
IPC_PACKET *CopyPacket(IPC_PACKET *sourcePacketPtr)
{
    IPC_PACKET *newPacketPtr = NULL;

    ccb_assert(sourcePacketPtr != NULL, sourcePacketPtr);

    /*
     * Make a new packet pointer
     */
    newPacketPtr = MallocSharedWC(sizeof(*newPacketPtr));

    /*
     * Check that sourcePacketPtr has a valid header pointer
     */
    if (sourcePacketPtr->header)
    {
        /*
         * Make a new header pointer
         */
        newPacketPtr->header = MallocSharedWC(sizeof(*newPacketPtr->header));

        /*
         * Copy the sourcePacket's header
         */
        memcpy(newPacketPtr->header, sourcePacketPtr->header,
               sizeof(*newPacketPtr->header));
    }

    /*
     * Check that sourcePacketPtr has a valid data pointer
     */
    if (sourcePacketPtr->data)
    {
        /*
         * Make a new data pointer
         */
        newPacketPtr->data = MallocSharedWC(sourcePacketPtr->header->length);

        /*
         * Copy the sourcePacket's data
         */
        memcpy(newPacketPtr->data,
               sourcePacketPtr->data, sourcePacketPtr->header->length);
    }

    return (newPacketPtr);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
