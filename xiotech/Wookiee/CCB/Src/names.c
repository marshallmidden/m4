/* $Id: names.c 143845 2010-07-07 20:51:58Z mdr $ */
/*============================================================================
** FILE NAME:       names.c
** MODULE TITLE:    Naming for CCB
**
** DESCRIPTION:     Names are associated with physical disks, virtual disks,
**                  servers, disk sets, and containers.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "CmdLayers.h"
#include "cps_init.h"
#include "debug_files.h"
#include "FIO.h"
#include "LargeArrays.h"
#include "mode.h"
#include "names.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PortServer.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** DEFAULT value of a CNC name if it has not been set.
*/
#define CNC_NO_NAME             "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

/*****************************************************************************
** Private functions
*****************************************************************************/
static INT32 ControllerNodeCluster_GetName(char *buf);
static INT32 NameDevice(UINT16 option, UINT16 id, char *buf);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ControllerNodeCluster_IsNameSet
**
** Description: Checks if the CNC name has been initialized.
**
** Inputs:      NONE
**
** Returns:     true if the name has been initialized, false othewise.
**
**--------------------------------------------------------------------------*/
bool ControllerNodeCluster_IsNameSet(void)
{
    char        cncName[NAME_DEVICE_NAME_LEN];
    INT32       rc = PI_GOOD;
    bool        bSet = false;

    rc = ControllerNodeCluster_GetName(cncName);

    if (rc == PI_GOOD)
    {
        if (memcmp(cncName, CNC_NO_NAME, NAME_DEVICE_NAME_LEN) != 0)
        {
            bSet = true;
        }
    }

    return bSet;
}

/*----------------------------------------------------------------------------
** Function:    ControllerNodeCluster_GetName
**
** Description: Get the controller node cluster name.
**
** Inputs:      UINT8* buf
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     The variable "buf" must be defined as a NAME_DEVICE_NAME_LEN
**              byte buffer.
**--------------------------------------------------------------------------*/
static INT32 ControllerNodeCluster_GetName(char *buf)
{
    return NameDevice(MNDRETVCG, 0, buf);
}

/*----------------------------------------------------------------------------
** Function:    ControllerNodeCluster_SetDefault
**
** Description: Set the controller node cluster default name (CNCnnnnnnnnnn).
**
** Inputs:      NONE
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 ControllerNodeCluster_SetDefaultName(void)
{
    char        cncName[NAME_DEVICE_NAME_LEN];

    /*
     * Clear the CNC name buffer and then set the default
     * name into the buffer.
     */
    memset(cncName, 0x00, NAME_DEVICE_NAME_LEN);
    sprintf(cncName, "DSC%d", CntlSetup_GetSystemSN());

    return NameDevice(MNDVCG, 0, cncName);
}

/*----------------------------------------------------------------------------
** Function:    NameDevice
**
** Description: Get or set a device name (vdisk, server or CNC).
**
** Inputs:      UINT16 option
**              UINT16 id
**              UINT8* buf
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     The variable "buf" must be defined as a NAME_DEVICE_NAME_LEN
**              byte buffer.
**--------------------------------------------------------------------------*/
static INT32 NameDevice(UINT16 option, UINT16 id, char *buf)
{
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    INT32       rc = PI_GOOD;

    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_NAME_DEVICE_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_PROC_NAME_DEVICE_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_NAME_DEVICE_REQ);

    ((PI_PROC_NAME_DEVICE_REQ *)reqPacket.pPacket)->option = option;
    ((PI_PROC_NAME_DEVICE_REQ *)reqPacket.pPacket)->id = id;

    /*
     * For all set name requests, copy the name into the request packet.
     */
    if (option != MNDRETVCG)
    {
        memcpy(((PI_PROC_NAME_DEVICE_REQ *)reqPacket.pPacket)->name,
               buf, NAME_DEVICE_NAME_LEN);
    }

    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        /*
         * If this was a get name request copy the name into the
         * response packet.
         */
        if (option == MNDRETVCG)
        {
            memcpy(buf,
                   ((PI_PROC_NAME_DEVICE_RSP *)rspPacket.pPacket)->name,
                   NAME_DEVICE_NAME_LEN);
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "NameDevice - Failed to set or retrieve device name (rc: 0x%x, status: 0x%x, errorCode: 0x%x).\n",
                rc, rspPacket.pHeader->status, rspPacket.pHeader->errorCode);
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

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
