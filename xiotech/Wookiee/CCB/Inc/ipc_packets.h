/* $Id: ipc_packets.h 143020 2010-06-22 18:35:56Z m4 $ */
/*===========================================================================
** FILE NAME:       ipc_packets.h
** MODULE TITLE:    Header file for ipc related packet stuff
**
** DESCRIPTION:     IPC command definitions and data structures
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_PACKETS_H_
#define _IPC_PACKETS_H_

#include "EL_DiskMap.h"
#include "FIO.h"
#include "slink.h"
#include "XIOPacket.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** IPC packet protocol defines
*/
#define IPC_PACKET_VERSION_0    0
#define IPC_PROTOCOL_LEVEL      IPC_PACKET_VERSION_0

/*
** IPC command codes
*/
#define PACKET_IPC_CONFIGURATION_UPDATE         601         // 0x259
/* UNUSED                                       602 */      // 0x25a
#define PACKET_IPC_REPORT_CONTROLLER_FAILURE    603         // 0x25b
#define PACKET_IPC_COMMAND_STATUS               604         // 0x25c
#define PACKET_IPC_ELECT                        605         // 0x25d
/* UNUSED                                       606 */      // 0x25e
#define PACKET_IPC_HEARTBEAT                    607         // 0x25f
/* UNUSED                                       608 */      // 0x260
#define PACKET_IPC_OFFLINE                      609         // 0x261
/* UNUSED                                       610 */      // 0x262
/* UNUSED                                       611 */      // 0x263
/* UNUSED                                       612 */      // 0x264
/* UNUSED                                       613 */      // 0x265
#define PACKET_IPC_TUNNEL                       614         // 0x266
/* UNUSED                                       615 */      // 0x267
/* UNUSED                                       616 */      // 0x268
/* UNUSED                                       617 */      // 0x269
#define PACKET_IPC_PING                         618         // 0x26a
#define PACKET_IPC_ONLINE                       619         // 0x26b
#define PACKET_IPC_SIGNAL                       620         // 0x26c
/* UNUSED                                       621 */      // 0x26d
/* UNUSED                                       622 */      // 0x26e
/* UNUSED                                       623 */      // 0x26f
/* UNUSED                                       624 */      // 0x270
/* UNUSED                                       625 */      // 0x271
/* UNUSED ** (see below)                        626 */      // 0x272
#define PACKET_IPC_FLUSH_BE_CACHE               627         // 0x273
/* UNUSED                                       628 */      // 0x274
#define PACKET_IPC_ENABLE_DISABLE_CACHE         629         // 0x275
#define PACKET_IPC_SET_MIRROR_PARTNER           630         // 0x276
/* UNUSED ** (see below)                        631 */      // 0x277
#define PACKET_IPC_CONTINUE_WO_MIRROR           632         // 0x278
/* UNUSED                                       633 */      // 0x279
#define PACKET_IPC_RESCAN_DEVICES               634         // 0x27a
/* UNUSED                                       635 */      // 0x27b
#define PACKET_IPC_SET_DLM_HEARTBEAT_LIST       636         // 0x27c
/* UNUSED                                       637 */      // 0x27d
#define PACKET_IPC_ELECT_QUORUM                 638         // 0x27e
#define PACKET_IPC_BROADCAST                    639         // 0x27f
#define PACKET_IPC_GET_MIRROR_PARTNER           640         // 0x280
#define PACKET_IPC_GET_MIRROR_PARTNER_RESPONSE  641         // 0x281
#define PACKET_IPC_ADD_CONTROLLER               642         // 0x282
#define PACKET_IPC_FLUSH_COMPLETED              643         // 0x283
#define PACKET_IPC_LED_CHANGE                   644         // 0x284
/* UNUSED                                       645 */      // 0x285
/* UNUSED                                       646 */      // 0x286
/* UNUSED * (see below)                         647 */      // 0x287
/* UNUSED * (see below)                         648 */      // 0x288
#define PACKET_IPC_CLIENT_PERSISTENT_DATA_CMD   649         // 0x289
#define PACKET_IPC_RESYNC_CLIENT_CMD            650         // 0x28a
#define PACKET_IPC_RESYNC_CLIENT_RECORD         651         // 0x28b
#define PACKET_IPC_LATEST_PERSISTENT_DATA       652         // 0x28c
#define PACKET_IPC_SETPRES_DATA_CMD             653         // 0x28d

/*
** "*" denotes items declared in R1 but not used
** and declared unused in R2.
**
** "**" denotes items declared in R2 but not used
** and declared unused in R3.
*/

/*
** Structures for all the data portions of all the IPC Packets
*/

typedef struct _IPC_CONFIGURATION_UPDATE
{
    UINT8       rsvd0[3];       /* RESERVED                                 */
    UINT8       restoreOption;  /* Restore option - See MR_Defs.h           */
    UINT32      reason;         /* Any combination of:                      */
    /*      X1_ASYNC_PCHANGED                   */
    /*      X1_ASYNC_RCHANGED                   */
    /*      X1_ASYNC_VCHANGED                   */
    /*      X1_ASYNC_HCHANGED                   */
    /*      X1_ASYNC_ACHANGED                   */
    /*      X1_ASYNC_ZCHANGED                   */
    /*      X1_ASYNC_VCG_ELECTION_STATE_CHANGE  */
    /*      X1_ASYNC_VCG_ELECTION_STATE_ENDED   */
    /*      X1_ASYNC_VCG_POWERUP                */
    /*      X1_ASYNC_VCG_CFG_CHANGED            */
    UINT8       rsvd8[4];       /* RESERVED                                 */
} IPC_CONFIGURATION_UPDATE;

/* Failure Reporting Packet */
#define SIZEOF_FAILURE_PACKET   (sizeof(UINT32))

typedef struct _IPC_ELECTION_COMPLETE
{
    UINT32      ElectionState;  /* ?? */
} IPC_ELECTION_COMPLETE;

#define SIZEOF_IPC_ELECTION_COMPLETE (sizeof(IPC_ELECTION_COMPLETE)+SIZEOF_FAILURE_PACKET)

typedef struct _IPC_CONTROLLER_FAILURE
{
    UINT32      DetectedBySN;   /* Controller that found the failure */
    UINT32      FailedControllerSN;     /* Controller that is failed */
    UINT32      ErrorType;      /* Error */
} IPC_CONTROLLER_FAILURE;

#define CONTROLLER_FAILURE_UNSPECIFIED_FAILURE  0
#define CONTROLLER_FAILURE_CONFIGURATION_FAILED 1

#define SIZEOF_IPC_CONTROLLER_FAILURE (sizeof(IPC_CONTROLLER_FAILURE)+SIZEOF_FAILURE_PACKET)

typedef struct _IPC_INTERFACE_FAILURE
{
    UINT32      DetectedBySN;   /* Controller that found the failure */
    UINT32      FailedInterfaceID;      /* ID of the interface that has failed */
    UINT32      ControllerSN;   /* SN of the controller this interface is in */
    UINT32      InterfaceFailureType;   /* Type of the interface failure */
} IPC_INTERFACE_FAILURE;

#define SIZEOF_IPC_INTERFACE_FAILURE (sizeof(IPC_INTERFACE_FAILURE)+SIZEOF_FAILURE_PACKET)

/* Constants for InterfaceFailureType */
#define INTERFACE_FAIL_OK               0       /* The interface has recovered */
#define INTERFACE_FAIL_FIRMWARE_ERROR   1       /* A firmware fault has occured */
#define INTERFACE_FAIL_HARDWARE_ERROR   2       /* A hardware fault has occured */
#define INTERFACE_FAIL_OTHER_ERROR      3       /* Some other non-recoverable error has occured */

typedef struct _IPC_BATTERY_FAILURE
{
    UINT32      DetectedBySN;   /* Controller that found the battery state change */
    UINT32      BatteryState;   /* State of the battery */
    UINT8       BatteryBank;    /* Which bank 1, or 2 */
    UINT8       Reserved[3];
} IPC_BATTERY_FAILURE;

#define SIZEOF_IPC_BATTERY_FAILURE (sizeof(IPC_BATTERY_FAILURE)+SIZEOF_FAILURE_PACKET)

/* Constants for BatteryState */
#define BATTERY_FAIL_OK         0       /* Battery is ok and charged */
#define BATTERY_FAIL_MISSING    1       /* Battery is not present */
#define BATTERY_FAIL_OTHER      2       /* Some other battery error */
#define BATTERY_FAIL_LOW_CHARGE 3       /* Battery has low charge */

typedef struct _IPC_COMMUNICATIONS_LINK_FAILURE
{
    UINT32      DetectedBySN;   /* Controller that found the link failure */
    UINT32      DestinationSN;  /* Destination of the failed link */
    UINT8       LinkType;       /* Type of the link that has failed */
    UINT8       Processor;      /* Which Processor for PCI Failure */
    char        Reserved[2];
    UINT32      LinkError;      /* Error found on this link */
} IPC_COMMUNICATIONS_LINK_FAILURE;

#define SIZEOF_IPC_COMMUNICATIONS_LINK_FAILURE (sizeof(IPC_COMMUNICATIONS_LINK_FAILURE)+SIZEOF_FAILURE_PACKET)

/* Constants for LinkType */
#define IPC_LINK_TYPE_ETHERNET  0
#define IPC_LINK_TYPE_FIBRE     1
#define IPC_LINK_TYPE_QUORUM    2
#define IPC_LINK_TYPE_PCI       3

/* Constants for LinkError */
#define IPC_LINK_ERROR_OK           0   /* Link is ok now */
#define IPC_LINK_ERROR_CREATE_LINK  1   /* Link does not exist or cannot be created */
#define IPC_LINK_ERROR_FAILED       2   /* Link has failed for an unspecified reason */
#define IPC_LINK_ERROR_NO_LOOP      3   /* Link has failed because no loop exists to the destination */
#define IPC_LINK_ERROR_NO_LINK      4   /* Link has failed because there is no ethernet link */
#define IPC_LINK_ERROR_NO_HEARTBEAT 5   /* Link has failed because no heartbeat has been received or responded to */
#define IPC_LINK_ERROR_HB_LIST      6   /* Can not create heartbeat list */

/* THIS PACKET MUST NOT EXCEED 28 BYTES, AND MUST BE OF EVEN WORD LENGTH (multiples of 4 bytes)*/
typedef struct _IPC_REPORT_CONTROLLER_FAILURE
{
    UINT32      Type;           /* Type of the failure */
    union _FailureData
    {
        IPC_CONTROLLER_FAILURE ControllerFailure;
        IPC_INTERFACE_FAILURE InterfaceFailure;
        IPC_COMMUNICATIONS_LINK_FAILURE CommunicationsLinkFailure;
        IPC_ELECTION_COMPLETE ElectionComplete;
        IPC_BATTERY_FAILURE BatteryFailure;
    } FailureData;
} IPC_REPORT_CONTROLLER_FAILURE;

/* Constants for Type */
#define IPC_FAILURE_TYPE_INTERFACE_FAILED       1       /* A single interface has failed */
#define IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED  2       /* A communications link has failed */
#define IPC_FAILURE_TYPE_CONTROLLER_FAILED      3       /* Massive controller failure has occured */
#define IPC_FAILURE_TYPE_ELECTION_COMPLETE      4       /* Used internally for the elector to tell us the election is complete */
#define IPC_FAILURE_TYPE_BATTERY_FAILURE        6       /* Battery state changed */
#define IPC_FAILURE_TYPE_REBUILD_STATUS         7       /* Rebuild status has changed */

/* Packet used by SM for enabling and disabling cache */

/* PACKET_IPC_ENABLE_DISABLE_CACHE */
typedef struct _IPC_ENABLE_DISABLE_CACHE
{
    UINT16      TargetID;       /* Target ID for target to enable or disable cache */
    UINT8       Enable;         /* TRUE = Enable cache, FALSE = Disable cache */
    UINT8       Wait;           /* TRUE = Wait for flush, FALSE = Do not wait for flush */
} IPC_ENABLE_DISABLE_CACHE;

/* Packet used by SM for flushing BE cache */
/* PACKET_IPC_FLUSH_BE_CACHE */
typedef struct _IPC_FLUSH_BE_CACHE
{
    UINT16      TargetID;       /* Target ID for target to flush IPC_FLUSH_BE_CACHE_ALL_TARGETS = All Targets */
} IPC_FLUSH_BE_CACHE;

#define IPC_FLUSH_BE_CACHE_ALL_TARGETS  0xFFFF

/* Packet used by SM for setting the mirror partner of a controller */
/* PACKET_IPC_SET_MIRROR_PARTNER */
typedef struct _IPC_SET_MIRROR_PARTNER
{
    UINT32      ControllerSN;   /* Controller SN to use as a mirror partner */
} IPC_SET_MIRROR_PARTNER;

/* Packet used by SM for requesting a device rescan */
/* PACKET_IPC_RESCAN_DEVICES */
typedef struct _IPC_RESCAN_DEVICES
{
    UINT32      reserved;
} IPC_RESCAN_DEVICES;

/* Packet used by SM for continuing cache w/o a mirror partner */
typedef struct _IPC_CONTINUE_WO_MIRROR
{
    UINT32      reserved;
} IPC_CONTINUE_WO_MIRROR;

/* Packet used by SM for setting the DLM heartbeat list */
typedef struct _IPC_SET_DLM_HEARTBEAT_LIST
{
    UINT32      Count;          /* number of controllers in list */
    UINT32      List[1];        /* List of controller SN's to use for DLM heartbeats */
} IPC_SET_DLM_HEARTBEAT_LIST;

/* Packet used by SM to get the taret controllers mirror partner */
typedef struct _IPC_GET_MIRROR_PARTNER
{
    UINT32      reserved;
} IPC_GET_MIRROR_PARTNER;

typedef struct _IPC_GET_MIRROR_PARTNER_RESPONSE
{
    UINT32      PartnerSN;      /* Partner to this controller */
    UINT32      MySN;           /* My SN */
    UINT32      Status;         /* PI_STATUS of this request */
} IPC_GET_MIRROR_PARTNER_RESPONSE;

/* Packet used by SM to signal that a flush has completed */
typedef struct _IPC_FLUSH_COMPLETED
{
    UINT32      ControllerSN;   /* SN of controller sending packet */
    UINT8       Success;
} IPC_FLUSH_COMPLETED;

typedef struct _IPC_COMMAND_STATUS
{
    UINT8       status;
    UINT32      errorCodePrimary;
    UINT32      errorCodeSecondary;
} IPC_COMMAND_STATUS;

#define IPC_COMMAND_IN_PROGRESS 0
#define IPC_COMMAND_SUCCESSFUL  1
#define IPC_COMMAND_FAILED      2

typedef struct
{
    UINT32      starting;
    UINT32      current;
} ELECTION_SERIAL;

typedef struct
{
    ELECTION_SERIAL electionSerial;
    UINT8       messageType;
    UINT8       electionTaskState;
} IPC_ELECT_R1;

typedef struct
{
    IPC_ELECT_R1 ipcElectR1;
    ELECTION_DISK_MAP diskMap;
} IPC_ELECT_R3;

typedef union
{
    IPC_ELECT_R1 ipcElectR1;
    IPC_ELECT_R3 ipcElectR3;
} IPC_ELECT, IPC_ELECT_QUORUM;

typedef struct _CHILD_STATUS
{
    UINT32      controllerSN;
    UINT32      timeStamp;
    UINT32      reserved[2];
} CHILD_STATUS;

typedef struct _IPC_HEARTBEAT
{
    UINT32      numChildrenAggregated;
    UINT32      reseverd[3];
    CHILD_STATUS childStatus[1];
} IPC_HEARTBEAT;

typedef struct _IPC_OFFLINE
{
    UINT32      WaitTime;       /* Time to wait in ms before going offline */
} IPC_OFFLINE;

/*
** IPC_TUNNEL
**
** This packet is used to send a Packet Interface (PI) packet from one
** controller to another.  This packet will need to include the
** PI_PACKET_HEADER and if the PI request requires data to be sent it
** needs to include that also.
**
** Packet Size Calculation:
**      Before calling the CreatePacket method the total packet size needs
**      to be calculated.  This size needs to include the size of a
**      IPC_TUNNEL structure, the size of a PI_PACKET_HEADER structure
**      and if there is data for the PI request, the size of that request
**      structure.
**
** The final packet looks something like the following where the "packet"
** field in the IPC_TUNNEL structure is used as the offset to the start
** of the PI packet header.  If there is request data it is calculated
** using the "packet" field plus the size of a PI_PACKET_HEADER.
**              |-----------------------------------|
**              |  RSVD  |  RSVD  |  RSVD  | STATUS |
**              |-----------------------------------|
**      packet  | PI_PACKET_HEADER                  |
**              |-----------------------------------|
**              | OPTIONAL PACKET DATA              |
**              |-----------------------------------|
*/
typedef struct _IPC_TUNNEL
{
    UINT8       rsvd[3];        /* RESERVED */
    UINT8       status;         /* Status of the IPC request */
    UINT8       packet[0];      /* Place holder for the start of the PI packet */
} IPC_TUNNEL;

typedef struct _IPC_PING
{
    UINT32      tbd;
} IPC_PING;

typedef struct _IPC_ONLINE
{
    UINT32      tbd;
} IPC_ONLINE;

typedef struct _IPC_SIGNAL
{
    UINT16      signalEvent;
} IPC_SIGNAL;

#define IPC_SIGNAL_RUN_BE           0x0001
#define IPC_SIGNAL_RUN_FE           0x0002
#define IPC_SIGNAL_LOAD_CONFIG      0x0004
#define IPC_SIGNAL_POWERUP_COMPLETE 0x0008
#define IPC_SIGNAL_POWERUP_BE_READY 0x0010
#define IPC_SIGNAL_RUN_P2INIT       0x0020
#define IPC_SIGNAL_FE_PORT_GO       0x0040

#define IPC_SIGNAL_MIRROR_PARTNER_INFO       0x0080

/*
** IPC_LOCAL_IMAGE
**
** This packet is used for both the PACKET_IPC_LOCAL_IMAGE_READY and
** PACKET_IPC_GET_LOCAL_IMAGE commands.  In both cases there is a local
** NVRAM image that needs to be sent from one controller to another.
**
** PACKET_IPC_LOCAL_IMAGE_READY
**      The local image ready command will be used when a slave controller
**      receives the LOCAL_IMAGE_READY async event and needs to send
**      the local image to the master controller.
**
** PACKET_IPC_GET_LOCAL_IMAGE
**      The get local image command will be used when a master controller
**      needs to retrieve the local images from each of the slaves.
*/
typedef struct _IPC_LOCAL_IMAGE
{
    UINT8       rsvd[3];        /* RESERVED */
    UINT8       status;         /* Status of the IPC request */
    UINT32      imageSize;      /* Size of the local image being processed */
    UINT8       image[0];       /* Place holder for the start of the local image */
} IPC_LOCAL_IMAGE;

/*
** IPC_BROADCAST
**
** This packet is to send an event to one or more controllers in the
** virtual controller group.
*/
typedef struct _IPC_BROADCAST
{
    UINT16      eventType;      /* Event to broadcast */
    UINT16      bcastType;      /* Type of broadcast */
    UINT32      serialNum;      /* Serial number of a specific controller */
    UINT32      dataSize;       /* Size of the following data */
    UINT8       data[0];        /* Place holder for the start of the data */
} IPC_BROADCAST;

/*
** Broadcast event types
*/
#define IPC_BROADCAST_PUTSOS        0x0000
#define IPC_BROADCAST_PUTSCMT       0x0001
#define IPC_BROADCAST_PUTLCLIMAGE   0x0002
#define IPC_BROADCAST_PUTDG         0x0003
#define IPC_BROADCAST_FSYS          0x0004
#define IPC_BROADCAST_PUTLDD        0x0005
#define IPC_BROADCAST_ISID          0x0006

/*
** This is used to broadcast message to all active DCNs,
** when all BackEnd devices, before this DCN dies.
*/
#define IPC_BROADCAST_ALLDEVMISSING 0x0007
#define IPC_BROADCAST_PRR           0x0008

/*
** Broadcast destination types - Bit fields, can set multiple values.
**
**  MASTER      - Send to the current master controller, even this controller.
**  SLAVES      - Send to all slave controllers, except this controller.
**  SELF        - Send to this controller only.
**  SPECIFIC    - Send to a specific controller, even this controller.
**  OTHERS      - Send to all controllers other than this controller.
*/
#define IPC_BROADCAST_MASTER        0x0001
#define IPC_BROADCAST_SLAVES        0x0002
#define IPC_BROADCAST_SELF          0x0004
#define IPC_BROADCAST_SPECIFIC      0x0008
#define IPC_BROADCAST_OTHERS        0x0010

/*
** Broadcast copy handler constants, used when decoding a PUTSCMT
** broadcast event.
**
** The CTYPE and PERCENT are just UINT8 values at the 4 and 6
** offsets respectively.  The structure definition can be found
** in PROC\INC\NVR.H (nrc structure).
*/
#define COPY_HAND_CTYPE(a)          (((UINT8*)((a)->data))[4])
#define COPY_HAND_PERCENT(a)        (((UINT8*)((a)->data))[6])
#define COPY_HAND_MIRROR            0   /* Copy mirror completion handler const */
#define COPY_HAND_SWAP              1   /* Copy swap completion handler const   */
#define COPY_HAND_COMP              2   /* Copy completion handler const        */

/*
** Broadcast local image constants, used when decoding a PUTLCLIMAGE
** broadcast event.
*/
#define PUTLCLIMAGE_LEN(a)          (((UINT32*)((a)->data))[0])
#define PUTLCLIMAGE_SN(a)           (((UINT32*)((a)->data))[1])
#define PUTLCLIMAGE_MP(a)           (((UINT32*)((a)->data))[2])
#define PUTLCLIMAGE_SEQ(a)          (((UINT32*)((a)->data))[3])

/*
** Broadcast datagram constants, used when decoding a PUTDG
** broadcast event.
**
** The defined fields and offsets are defined in the CCSM_E structure
** in PROC\INC\CCSM.H and PROC\INC\CCSM.INC.  The fields referenced
** are a portion of the CCSM_E structure that gets sent in the
** broadcast event.
**
** The defined type codes (DEFINE, CLIENT and CCBG) are defined in
** PROC\INC\CCSM.H and PROC\INC\CCSM.INC.
*/
#define PUTDG_LEN(a)                (((UINT32*)((a)->data))[0])
#define PUTDG_TYPE(a)               (((UINT8*) ((a)->data))[4])
#define PUTDG_FC(a)                 (((UINT8*) ((a)->data))[5])
#define PUTDG_SEQ(a)                (((UINT16*)((a)->data))[3])
#define PUTDG_SN(a)                 (((UINT32*)((a)->data))[2])
#define PUTDG_USERSWAP(a)           (((UINT8*) ((a)->data))[12])
#define PUTDG_DEFINE                0   /* define event type                */
#define PUTDG_CLIENT                1   /* client interface event type      */
#define PUTDG_CCBG                  5   /* CCBGram event type               */

/*
** IPC_ADD_CONTROLLER
**
** This packet is used to tell another controller that it is being
** added to the group.
*/
typedef struct _IPC_ADD_CONTROLLER
{
    UINT32      cserial;        /* Controller Serial Number */
} IPC_ADD_CONTROLLER;

/*
** IPC_LED_CHANGE
**
** This packet is to send a LED change request to the master
*/
#define LED_BOTH_OFF        0
#define LED_FAIL_ON         1
#define LED_FAIL_OFF        2
#define LED_ID_ON           3
#define LED_ID_OFF          4

typedef struct _IPC_LED_CHANGE
{
    UINT32      serialNum;      /* Serial number of a specific controller */
    UINT64      wwn;            /* WWN of device to change                */
    UINT32      ledReq;         /* State to set LED                       */
} IPC_LED_CHANGE;

/*
** IPC_RESYNC_CLIENT_RECORD
**
** This packet is to send for resync client record to slave
** from the master.
*/
typedef struct _IPC_RESYNC_CLIENT_RECORD
{
    char        recordName[256];
    UINT32      startOffset;
    UINT32      dataSize;       /* Size of the following data */
    UINT8       data[0];        /* Place holder for the start of the data */
} IPC_RESYNC_CLIENT_RECORD;

/*
** IPC_LATEST_PERSISTENT_DATA
**
** This packet is to send from the master to get the slave who has
** latest persistent data. This same packet will be sent from the
** controller who has latest data.
*/
typedef struct _IPC_LATEST_PERSISTENT_DATA
{
    UINT32      timeStamp;
    UINT32      controllerSN;
} IPC_LATEST_PERSISTENT_DATA;

/*
** IPC_RESYNC_CLIENT_CMD
**
** This packet is to send for resync request to controller who
** has latest persistent data from the controller  during its bringup.
*/
typedef struct _IPC_RESYNC_CLIENT_CMD
{
    UINT32      slaveSN;        /* Slave serial number */
} IPC_RESYNC_CLIENT_CMD;

/*
** IPC_CLIENT_PERSISTENT_DATA
**
** This packet is to send an event to one or more controllers in the
** virtual controller group.
*/
typedef struct _IPC_CLIENT_PERSISTENT_DATA
{
    UINT16      eventType;      /* Persistent event to send  */
    UINT32      serialNum;      /* Serial number of a specific controller */
    UINT32      dataSize;       /* Size of the following data */
    UINT8       packet[0];      /* Place holder for the start of the data */
} IPC_CLIENT_PERSISTENT_DATA;

/*
** Persistent data event types
*/
#define IPC_PERSISTENT_NOP          0x0000
#define IPC_PERSISTENT_CREATE       0x0001
#define IPC_PERSISTENT_REMOVE       0x0002
#define IPC_PERSISTENT_WRITE        0x0003
#define IPC_PERSISTENT_READ         0x0004

/*
** IPC_SETPRES_DATA
**
** This packet is to send set persistent reserve cmd to the
** the master in the VCG.
*/
typedef struct _IPC_SETPRES_DATA
{
    UINT32      serialNum;      /* Serial number of a specific controller */
    UINT32      pktSize;        /* Size of the following data */
    UINT8       packet[0];      /* Place holder for the start of the data */
} IPC_SETPRES_DATA;

/*
** NOTE: The structure definition for IPC packet header is identical to that
** which is used by the PI (Packet Interface) protocol, and is found in
** XIOPacket.h.
**
** The following is a union of all of the IPC data packet types.
*/
typedef union _IPC_PACKET_DATA
{
    IPC_CONFIGURATION_UPDATE configurationUpdate;
    IPC_REPORT_CONTROLLER_FAILURE reportControllerFailure;
    IPC_COMMAND_STATUS commandStatus;
    IPC_ELECT   elect;
    IPC_ELECT_QUORUM electQuorum;
    IPC_HEARTBEAT heartBeat;
    IPC_OFFLINE offline;
    IPC_TUNNEL  tunnel;
    IPC_PING    ping;
    IPC_ONLINE  online;
    IPC_SIGNAL  signal;
    IPC_LOCAL_IMAGE localImage;
    IPC_ENABLE_DISABLE_CACHE sequencerEnableDisableCache;       /* Enable or disable cache */
    IPC_FLUSH_BE_CACHE sequencerFlushBECache;   /* Flush BE cache */
    IPC_SET_MIRROR_PARTNER sequencerSetMirrorPartner;   /* Set the mirror partner */
    IPC_CONTINUE_WO_MIRROR sequencerContinueWithoutMirror;      /* Continue without a mirror partner */
    IPC_RESCAN_DEVICES sequencerRescanDevices;  /* Rescan devices */
    IPC_SET_DLM_HEARTBEAT_LIST sequencerDLMHeartbeatList;       /* Set the DLM heartbeat list */
    IPC_GET_MIRROR_PARTNER sequencerGetMirrorPartner;   /* Get the mirror partner */
    IPC_GET_MIRROR_PARTNER_RESPONSE sequencerGetMirrorPartnerResponse;  /* Mirror Partner Response */
    IPC_FLUSH_COMPLETED sequencerFlushCompleted;        /* A flush has completed */
    IPC_BROADCAST broadcast;
    IPC_ADD_CONTROLLER addController;
    IPC_LED_CHANGE ledChange;
    IPC_CLIENT_PERSISTENT_DATA persistent;
    IPC_SETPRES_DATA setpr;
    IPC_RESYNC_CLIENT_RECORD resyncClientRecord;
    IPC_RESYNC_CLIENT_CMD resyncClientCmd;
    IPC_LATEST_PERSISTENT_DATA latestClientData;
} IPC_PACKET_DATA;

/*
** Structure for packet, pointer for header and pointer for data packet
*/
typedef struct _PACKET
{
    IPC_PACKET_HEADER *header;  /* Pointer to header structure */
    IPC_PACKET_DATA *data;      /* Pointer to data structure */
} IPC_PACKET;

/*
** This is the difference between the ENUM  SENDPACKET_ANY_PATH and INTERFACE_NONE
*/
#define ENUM_DIFFERENCE ((SENDPACKET_ANY_PATH) - (INTERFACE_NONE))

/*****************************************************************************
** Public macros
*****************************************************************************/

#define SetReaderInterface(packet, a) ( (packet)->readInterface = (UINT16)(a))
#define GetReaderInterface(packet) ( (PATH_TYPE)((packet)->readInterface) )

#define SetSenderInterface(packet, a) ( (packet)->senderInterface = (UINT16)(a))
#define GetSenderInterface(packet) ( (PATH_TYPE)((packet)->senderInterface) )

#define GetPacketDataLength(a)  ((a)->length)
#define GetPacketSeqNum(a) ((a)->sequenceNumber)
#define SetSeqNum(packet, num) ((packet)->sequenceNumber = (num))

#define GetCCBSerialNumber(a) ((a)->ccbSerialNumber)
#define SetCCBSerialNumber(packet, num) ((packet)->ccbSerialNumber = (num))

/*
** void    SetPacketVersion(IPC_PACKET* p, UINT16 version);
*/
#define SetPacketVersion(p,version) (p->header->packetVersion = version)

/*
** UINT16  GetPacketVersion(IPC_PACKET* p);
*/
#define GetPacketVersion(p) (p->header->packetVersion)

/*
** UINT16  SetPacketProtocolLevel(IPC_PACKET* p);
*/
#define SetPacketProtocolLevel(p) (p->header->protocolLevel = IPC_PROTOCOL_LEVEL)

/*
** UINT16  GetPacketProtocolLevel(IPC_PACKET* p);
*/
#define GetPacketProtocolLevel(p) (p->header->protocolLevel)

/*
** bool    CheckPacketCompatibility(IPC_PACKET* p);
*/
#define CheckPacketCompatibility(p) (p->header->packetVersion <= IPC_PROTOCOL_LEVEL)

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern IPC_PACKET *CreatePacket(UINT16, size_t, const char *, const UINT32);
extern IPC_PACKET *CreateResponsePacket(UINT16, IPC_PACKET *, size_t, const char *, const UINT32);
extern void FreePacket(IPC_PACKET **, const char *, const UINT32 line);
extern void FreePacketStaticPacketPointer(IPC_PACKET *, const char *, const UINT32);
extern bool GotCmdInProgress(IPC_PACKET *rx);
extern IPC_PACKET *CopyPacket(IPC_PACKET *sourcePacketPtr);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_PACKETS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
