/* $Id: X1_Workset.c 143845 2010-07-07 20:51:58Z mdr $*/
/**
******************************************************************************
**
**  @file       X1_Workset.c
**
**  @brief      X1 Workset functions
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "X1_Workset.h"

#include "CmdLayers.h"
#include "debug_files.h"
#include "DEF_Workset.h"
#include "PacketInterface.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      GetWorkset
**
**              Get the Workset based on the input ID
**
**  @param      id          - Workset ID
**  @param      pWorkset    - Pointer to buffer where workset will be copied
**
**  @return     GOOD or ERROR
**
**  @attention  Caller must allocate enough memory for the request.
**
******************************************************************************
**/
static INT32 GetWorkset(UINT16 id, DEF_WORKSET *pWorkset)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    INT32       rc = PI_GOOD;
    UINT16      worksetCount = 0;

    /*
     * Determine the number of worksets being requested based on the
     * input ID.
     */
    if (id == GET_ALL_WORKSETS)
    {
        worksetCount = MAX_WORKSETS;
    }
    else
    {
        worksetCount = 1;
    }

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPkt.pHeader = MallocWC(sizeof(PI_PACKET_HEADER));
    reqPkt.pPacket = MallocWC(sizeof(PI_MISC_GET_WORKSET_INFO_REQ));
    rspPkt.pHeader = MallocWC(sizeof(PI_PACKET_HEADER));
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPkt.pHeader->commandCode = PI_MISC_GET_WORKSET_INFO_CMD;
    reqPkt.pHeader->length = sizeof(PI_MISC_GET_WORKSET_INFO_REQ);

    /*
     * Fill in the request parms.
     */
    ((PI_MISC_GET_WORKSET_INFO_REQ *)reqPkt.pPacket)->id = id;

    /*
     * Issue the command through the top-level command handler
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    /*
     * If the request was successful copy the data into the buffer.
     */
    if (rc == PI_GOOD)
    {
        memcpy(pWorkset,
               ((PI_MISC_GET_WORKSET_INFO_RSP *)rspPkt.pPacket)->workset,
               (worksetCount * sizeof(DEF_WORKSET)));
    }

    /*
     * Free the request and response headers.  If a timeout occurred keep
     * the response packet around - the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    return (rc);
}

/**
******************************************************************************
**
**  @brief      SetWorkset
**
**              Set the Workset info based on the input ID
**
**  @param      id          - Workset ID
**  @param      pWorkset    - Pointer to workset
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static INT32 SetWorkset(UINT16 id, DEF_WORKSET *pWorkset)
{
    XIO_PACKET  reqPkt = { NULL, NULL };
    XIO_PACKET  rspPkt = { NULL, NULL };
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPkt.pHeader = MallocWC(sizeof(PI_PACKET_HEADER));
    reqPkt.pPacket = MallocWC(sizeof(PI_MISC_SET_WORKSET_INFO_REQ));
    rspPkt.pHeader = MallocWC(sizeof(PI_PACKET_HEADER));
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPkt.pHeader->commandCode = PI_MISC_SET_WORKSET_INFO_CMD;
    reqPkt.pHeader->length = sizeof(PI_MISC_SET_WORKSET_INFO_REQ);
    reqPkt.pHeader->flags |= PI_HDR_FLG_RESTRAIN_LOG;

    /*
     * Fill in the request parms.
     */
    ((PI_MISC_SET_WORKSET_INFO_REQ *)reqPkt.pPacket)->id = id;

    memcpy(&(((PI_MISC_SET_WORKSET_INFO_REQ *)reqPkt.pPacket)->workset),
           pWorkset, sizeof(DEF_WORKSET));

    /*
     * Issue the command through the top-level command handler
     */
    rc = PortServerCommandHandler(&reqPkt, &rspPkt);

    /*
     * Free the request and response headers.  If a timeout occurred keep
     * the response packet around - the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    return (rc);
}


/**
******************************************************************************
**
**  @brief      InitWorksetDefaultVPort
**
**              Initialize the default VPort value for ALL worksets
**
**  @param      wid             - Workset ID
**  @param      defaultVPort    - New default VPort value
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
INT32 InitWorksetDefaultVPort(void)
{
    DEF_WORKSET *pWorkset;
    INT32       rc = GOOD;
    UINT16      i = 0;

    /*
     * Allocate memory for all the worksets.
     */
    pWorkset = MallocWC(MAX_WORKSETS * sizeof(DEF_WORKSET));

    /*
     * Get all the workset info.
     */
    rc = GetWorkset(GET_ALL_WORKSETS, pWorkset);

    /*
     * If the request was successful initialize the default VPort value
     * in each workset and write it back.
     */
    while ((rc == PI_GOOD) && (i < MAX_WORKSETS))
    {
        (pWorkset + i)->defaultVPort = DEF_WS_DEFAULT_VPORT_INIT;

        /*
         * Save the modified workset.
         */
        rc = SetWorkset(i, (pWorkset + i));

        /*
         * Advance to the next workset.
         */
        i++;
    }

    dprintf(DPRINTF_DEFAULT, "InitWorksetDefaultVPort - lastWorksetInitialized=%d  status=0x%02X\n",
            i - 1, rc);

    /*
     * Free memory used for worksets.
     */
    Free(pWorkset);

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
