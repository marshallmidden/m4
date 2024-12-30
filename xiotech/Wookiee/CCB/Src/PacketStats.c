/* $Id: PacketStats.c 141094 2010-05-17 15:40:32Z m4 $ */
/*============================================================================
** FILE NAME:       PacketStats.c
** MODULE TITLE:    Packet Statistics
**
** DESCRIPTION: Packet statistics information gathered for PI, X1 and MRP packets.
**
** Copyright (c) 2002-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "PacketStats.h"

#include "rtc.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define MAX_RANGE_PI_IDS      0x300
#define MAX_RANGE_X1_IDS      0x100
#define MAX_RANGE_X1_VDC_IDS  0x100
#define MAX_RANGE_X1_BF_IDS   MAX_RANGE_PI_IDS
#define MAX_RANGE_MRP_IDS     0x600
#define MAX_RANGE_IPC_IDS     0x100

#define PACKET_STATS_SCHEMA   0x31544B50

/*****************************************************************************
** Private variables
*****************************************************************************/
typedef struct
{
    char        name[12];
    UINT32      nextBlk;
} BLOCK_HDR;

typedef struct
{
    /* First Quad */
    UINT32      schema;
    TIMESTAMP   startTime;

    /* Second Quad */
    UINT32      initialized;
    TIMESTAMP   endTime;

    /* Third Quad */
    UINT32      unknownPI;
    UINT32      unknownX1;
    UINT32      unknownX1_VDC;
    UINT32      unknownX1_BF;

    /* Fourth Quad */
    UINT32      unknownMRP;
    UINT32      unknownIPC;
    UINT32      unknownType;
    UINT32      reserved;

    /* Fifth Quad */
    BLOCK_HDR   pi_start;
    UINT32      pi_packet_stats[MAX_RANGE_PI_IDS];

    BLOCK_HDR   x1_start;
    UINT32      x1_packet_stats[MAX_RANGE_X1_IDS];

    BLOCK_HDR   x1_vdc_start;
    UINT32      x1_vdc_packet_stats[MAX_RANGE_X1_VDC_IDS];

    BLOCK_HDR   x1_bf_start;
    UINT32      x1_bf_packet_stats[MAX_RANGE_X1_BF_IDS];

    BLOCK_HDR   mrp_start;
    UINT32      mrp_packet_stats[MAX_RANGE_MRP_IDS];

    BLOCK_HDR   ipc_start;
    UINT32      ipc_packet_stats[MAX_RANGE_IPC_IDS];

    BLOCK_HDR   the_end;
} COMBINED_PACKET_STATS;

/* NOTE: this is in the bss section, and is thus zero upon startup. */
static COMBINED_PACKET_STATS cpstats;

/*****************************************************************************
** Code Start
*****************************************************************************/

void IncPacketCount(UINT32 type, UINT32 packetID)
{
    if (!cpstats.initialized)
    {
        memset(&cpstats, 0, sizeof(cpstats));
        RTC_GetTimeStamp(&cpstats.startTime);

        strncpy(cpstats.pi_start.name, "PI START", 12);
        cpstats.pi_start.nextBlk = (MAX_RANGE_PI_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.x1_start.name, "X1 START", 12);
        cpstats.x1_start.nextBlk = (MAX_RANGE_X1_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.x1_vdc_start.name, "X1_VDC START", 12);
        cpstats.x1_vdc_start.nextBlk = (MAX_RANGE_X1_VDC_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.x1_bf_start.name, "X1_BF START", 12);
        cpstats.x1_bf_start.nextBlk = (MAX_RANGE_X1_BF_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.mrp_start.name, "MRP START", 12);
        cpstats.mrp_start.nextBlk = (MAX_RANGE_MRP_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.ipc_start.name, "IPC START", 12);
        cpstats.ipc_start.nextBlk = (MAX_RANGE_IPC_IDS * sizeof(UINT32)) + sizeof(BLOCK_HDR);
        strncpy(cpstats.the_end.name, "THE END!", 12);
        cpstats.the_end.nextBlk = 0;

        cpstats.initialized = 1;
        cpstats.schema = PACKET_STATS_SCHEMA;
    }

    switch (type)
    {
        /* PI packets map directly into the array */
        case PACKET_TYPE_PI:
            if (packetID < MAX_RANGE_PI_IDS)
            {
                cpstats.pi_packet_stats[packetID]++;
            }
            else
            {
                cpstats.unknownPI++;
            }
            break;

        /* X1 packets map directly into the array */
        case PACKET_TYPE_X1:
            if (packetID < MAX_RANGE_X1_IDS)
            {
                cpstats.x1_packet_stats[packetID]++;
            }
            else
            {
                cpstats.unknownX1++;
            }
            break;

        /* X1_VDC packets map directly into the array */
        case PACKET_TYPE_X1_VDC:
            if (packetID < MAX_RANGE_X1_VDC_IDS)
            {
                cpstats.x1_vdc_packet_stats[packetID]++;
            }
            else
            {
                cpstats.unknownX1_VDC++;
            }
            break;

        /* X1_BF packets map directly into the array */
        case PACKET_TYPE_X1_BF:
            if (packetID < MAX_RANGE_X1_BF_IDS)
            {
                cpstats.x1_bf_packet_stats[packetID]++;
            }
            else
            {
                cpstats.unknownX1_BF++;
            }
            break;

        /* MRP packets map directly into the array */
        case PACKET_TYPE_MRP:
            if (packetID < MAX_RANGE_MRP_IDS)
            {
                cpstats.mrp_packet_stats[packetID]++;
            }
            else
            {
                cpstats.unknownMRP++;
            }
            break;

        /* IPC packets need to be shifted */
        case PACKET_TYPE_IPC:
            if (packetID >= 600 && (packetID - 600) < MAX_RANGE_IPC_IDS)
            {
                cpstats.ipc_packet_stats[packetID - 600]++;
            }
            else
            {
                cpstats.unknownIPC++;
            }
            break;

        /* Don't know what this is, count it in the unknown "type" category */
        default:
            cpstats.unknownType++;
            break;
    }
}

/**********************************************************************
*                                                                     *
*  Name:        GetPacketStatsPointer()                               *
*                                                                     *
*  Description: Return pointer to packet stats structure              *
*                                                                     *
*  Input:       UINT32* - pointer where length should be stored       *
*                                                                     *
*  Returns:     pointer to stats structure                            *
*                                                                     *
**********************************************************************/
UINT8      *GetPacketStatsPointer(UINT32 *statsLength)
{
    if (statsLength != NULL)
    {
        *statsLength = sizeof(cpstats);
    }

    return ((UINT8 *)&cpstats);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
