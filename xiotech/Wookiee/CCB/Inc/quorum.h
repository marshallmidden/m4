/* $Id: quorum.h 143020 2010-06-22 18:35:56Z m4 $*/
/*============================================================================
** FILE NAME:       quorum.h
** MODULE TITLE:    Header file for quorum.c
**
** DESCRIPTION:  The quorum manager is responsible for two main functions:
**                  1. Providing an interface to allow access to quorum area
**                     data
**                  2. Providing a mechanism to transfer and monitor packets
**                     through the quorum communications area, both for a slave
**                     controller and the master controller.
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _QUORUM_H_
#define _QUORUM_H_

#include "kernel.h"
#include "EL_KeepAlive.h"
#include "ipc_packets.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** Constant Definitions
*/

/*
** NOTE: THE QUORUM AREA HAS A FIXED LIMITATION OF 16 CONTROLLERS, AS
** DETERMINED BY THE FIXED LOCATIONS OF THE ESTABLISHED COMMUNICATIONS AREAS.
** THEREFOR, THE MAXIMUM NUMBER OF CONTROLLERS SHOULD NOT EXCEED 16 CONTROLLERS
** W/O ADDRESSING A CHANGE TO QUORUM COMMUNICATIONS AND SUBSEQUENT COMPATIBLITY
** ISSUES.
*/
#define QM_MAX_CONTROLLERS      16      /* Max controllers allocated in quorum  */
#if (QM_MAX_CONTROLLERS != 16)
#error FAIL -- QUORUM is fixed at 16 controllers
#endif
#if (QM_MAX_CONTROLLERS  < MAX_CONTROLLERS)
#error FAIL -- Max controllers exeeds quorum size
#endif

#define QUORUM_POLL_PERIOD      1000

#define SQP_PARM_ERROR          1       /* Send Quorun Packet - Parameter Error */
#define SQP_PROTOCOL_ERROR      2       /* Send Quorun Packet - Protocol Error  */
#define SQP_ALLOCATE_ERROR      3       /* Send Quorun Packet - Allocate Error  */
#define SQP_TIMEOUT_ERROR       4       /* Send Quorun Packet - Timeout Error   */
#define SQP_WRITE_ERROR         5       /* Send Quorun Packet - Write Error     */

#define NVRAM_MAGIC_NUMBER      0x6731261
#define SCHEMA          2       /* Bump SCHEMA if you want to force a       */
                                /* NVRAM incompatibility on the next        */
                                /* release.                                 */

/* Controller config Map schema        */
#define CONTROLLER_MAP_SCHEMA   1

#define ACM_NODE_UNDEFINED      ( (UINT8)-1 )

/*
** Quorum data definitions
*/
enum
{
    UNLOCKED,
    LOCKED
};

typedef enum
{
    QM_IDLE,
    QM_NEW_MESSAGE,
    QM_MESSAGE_RECEIVED,
    QM_MESSAGE_COMPLETE,
    QM_UNUSED = (UINT32)-1      /* Pad enum values out to 32 bits */
} MessageState;

typedef enum
{
    INBOUND,
    OUTBOUND
} MessageDirection;

/*
** Quorum data types
*/

/* Quorum Communications sector - as it exist in the Quorum                 */
typedef struct
{
    MessageState messageState;
    UINT8       data[BYTES_PER_SECTOR - sizeof(MessageState)];
} QM_COMM_SECTOR;

typedef struct
{
    QM_COMM_SECTOR commSector[QM_MAX_CONTROLLERS];
} QM_IPC_MAILBOX;

/* Quorum Election state sector - as it exist in the Quorum                 */
typedef struct
{
    UINT32      state;
    ELECTION_SERIAL electionSerial;
    UINT8       mastershipAbility;
    UINT8       iconConnectivity;
    UINT8       contactMap[128];
    UINT32      portCount;
} QM_ELECTION_DATA;

/* QM_ELECTION_DATA.state value definitions */
typedef enum _ELECTION_DATA_STATE
{
    ED_STATE_END_TASK = 0,
    ED_STATE_BEGIN_ELECTION = 1,
    ED_STATE_CHECK_MASTERSHIP_ABILITY = 2,
    ED_STATE_TIMEOUT_CONTROLLERS = 3,
    ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE = 4,
    ED_STATE_CONTACT_ALL_CONTROLLERS = 5,
    ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE = 6,
    ED_STATE_WAIT_FOR_MASTER = 7,
    ED_STATE_CHECK_MASTER = 8,
    ED_STATE_NOTIFY_SLAVES = 9,
    ED_STATE_FAILED = 10,
    ED_STATE_FINISHED = 11
} ELECTION_DATA_STATE;

/* QM_ELECTION_DATA.masterCapabilityField value definitions */
typedef enum _ELECTION_DATA_MASTERSHIP_ABILITY
{
    ED_MASTERSHIP_ABILITY_NOT_TESTED = 0,
    ED_MASTERSHIP_ABILITY_QUALIFIED = 1,
    ED_MASTERSHIP_ABILITY_NOT_QUALIFIED = 2
} ELECTION_DATA_MASTERSHIP_ABILITY;

/* QM_ELECTION_DATA.iconConnectivity value definitions */
typedef enum _ELECTION_DATA_ICON_CONNECTIVITY
{
    ED_ICON_CONNECTIVITY_NOT_TESTED = 0,
    ED_ICON_CONNECTIVITY_CONNECTED = 1,
    ED_ICON_CONNECTIVITY_NOT_CONNECTED = 2
} ELECTION_DATA_ICON_CONNECTIVITY;

/* QM_ELECTION_DATA.contactMap value definitions */
typedef enum _ELECTION_DATA_CONTACT_MAP_ITEM
{
    ED_CONTACT_MAP_NO_ACTIVITY = 0,
    ED_CONTACT_MAP_CONTACTING = 1,
    ED_CONTACT_MAP_CONTACT_FAILED = 2,
    ED_CONTACT_MAP_CONTACT_TIMEOUT = 3,
    ED_CONTACT_MAP_CONTACTED_QUORUM = 4,
    ED_CONTACT_MAP_CONTACTED_FIBRE = 5,
    ED_CONTACT_MAP_CONTACTED_ETHERNET = 6,
    ED_CONTACT_MAP_NOTIFY_SLAVE = 7,
    ED_CONTACT_MAP_SLAVE_NOTIFIED = 8,
    ED_CONTACT_MAP_NOTIFY_SLAVE_FAILED = 9,
    ED_CONTACT_MAP_NOTIFY_SLAVE_TIMEOUT = 10,
    ED_CONTACT_MAP_MASTER_CONTROLLER = 11,
    ED_CONTACT_MAP_TIMEOUT_CONTROLLER = 12,
    ED_CONTACT_MAP_CONTROLLER_TIMED_OUT = 13,
    ED_CONTACT_MAP_TIMEOUT_CONTROLLER_FAILED = 14
} ELECTION_DATA_CONTACT_MAP_ITEM;

typedef struct
{
    QM_ELECTION_DATA electionData;
    UINT8       rsvd[BYTES_PER_SECTOR - sizeof(QM_ELECTION_DATA)];
} QM_ELECTION_SECTOR;

/* Quorum Failure state sector - as it exist in the Quorum                  */
typedef struct
{
    UINT32      state;
} QM_FAILURE_DATA;

/* QM_FAILURE_DATA.state value definitions */
typedef enum _FAILURE_DATA_STATE
{
    FD_STATE_UNUSED = 0,
    FD_STATE_FAILED = 1,
    FD_STATE_OPERATIONAL = 2,
    FD_STATE_POR = 3,
    FD_STATE_ADD_CONTROLLER_TO_VCG = 4,
    FD_STATE_STRANDED_CACHE_DATA = 5,
    FD_STATE_FIRMWARE_UPDATE_INACTIVE = 6,
    FD_STATE_FIRMWARE_UPDATE_ACTIVE = 7,
    FD_STATE_UNFAIL_CONTROLLER = 8,
    FD_STATE_VCG_SHUTDOWN = 9,
    FD_STATE_INACTIVATED = 10,
    FD_STATE_ACTIVATE = 11,
    FD_STATE_DISASTER_INACTIVE = 12,
} FAILURE_DATA_STATE;

typedef struct
{
    QM_FAILURE_DATA failureData;
    UINT8       rsvd[BYTES_PER_SECTOR - sizeof(QM_FAILURE_DATA)];
} QM_FAILURE_SECTOR;

/* Quorum reserved sector   - as it exist in the Quorum                     */
typedef struct
{
    UINT8       data[BYTES_PER_SECTOR];
} QM_RESERVED_SECTOR;

/*
** The quorum communications fid consists of 2 distinct areas. These areas
** are read and written independently through the block offset methods within
** the fileio component. The 2 areas are the Controller communications
** (original method of communications) and the Mailbox area. The layout of the
** Fid is as follows:
**
**  #--------------------------------------------#
**  # 1 Block Header (common to all Fids)        #
**  #--------------------------------------------#
**  # 128 Blocks ( 8 Blocks x 16)                #
**  # Controller Comm Area                       #
**  #--------------------------------------------#
**  # 256 Blocks ( 16 Blocks x 16)               #
**  # Controller Mailboxes                       #
**  #--------------------------------------------#
**  # 640 Blocks Free                            #
**  #--------------------------------------------#
*/

/* Per Controller communications area - as it exist in the Quorum           */
typedef struct
{
    QM_RESERVED_SECTOR rsvd1[2];
    QM_ELECTION_SECTOR electionStateSector;
    QM_FAILURE_SECTOR failStateSector;
    QM_RESERVED_SECTOR rsvd2[4];
} QM_CONTROLLER_COMM_AREA;

/*
** Size of the entire communications area.
*/
#define SIZE_ENTIRE_COMM_AREA   (MAX_CONTROLLERS * sizeof(QM_CONTROLLER_COMM_AREA))

/*
** Offset and size of the mailbox sectors within the communications fid.
*/
#define NUM_IPC_MAILBOXES   (QM_MAX_CONTROLLERS * QM_MAX_CONTROLLERS)
#define IPC_MAILBOXES_SECTOR_OFFSET  \
  (((QM_MAX_CONTROLLERS * sizeof(QM_CONTROLLER_COMM_AREA))/BYTES_PER_SECTOR)+1)

/* Quorum Communications slot                                               */
typedef struct
{
    MessageState state;
    UINT8       sendLock;
    UINT8       rsvd[3];
    QM_COMM_SECTOR *sectorPtr;
} QM_COMM_PORT;

/* Active controller map   - as it exist in the Quorum                      */
typedef struct
{
    UINT8 node[MAX_CONTROLLERS];
    UINT8       reserved[SIZE_128 - MAX_CONTROLLERS];
} QM_ACTIVE_CONTROLLER_MAP;

/*
** Cached copy of this controller's failure state
*/
extern FAILURE_DATA_STATE cachedControllerFailureState;

/*
** Accessor functions for controller failure state
*/
#define GetControllerFailureState() (cachedControllerFailureState)
#define SetControllerFailureState(s) (cachedControllerFailureState=(s))

/* Master Configuration Record                         */

/*
 * The Master configuration record is stored in the quorum area of the virtual
 * controller. A copy is also maintained in the NVRAM as a back-up if the file
 * system is unavailable at boot. A checksum is maintained and checked on the
 * NVRAM copy. A seperate checksum is also maintained within the file system.
 *
 * IMPORTANT GUIDELINES:
 *
 * 1) This structure is exactly 3616 bytes long.   I don't know where that
 *    number came from, but if you add a field to this structure, make sure
 *    sure to decrease the "pad" field by the same amount. Failure to do this
 *    will most likely can cause us to exceed the storage allocated in the
 *    quorum and change the checksum loacation in the NVRAM (causing a
 *    checksum error).
 *
 * 2) When adding a field, add it IMMEDIATELY BEFORE the "pad" field, and
 *    then, decrease "pad," as noted above.  This will help maintain backward
 *    compatibility with configuration records currently configured in the emc's.
 *
 * 3) Maintain proper alignment -- words on word boundaries, shorts on short
 *    boundaries etc.  Although this is not strictly necessary (the 960 handles
 *    non-aligned accesses automatically), it is good proctice!
 *
 * 4) Again, when adding a field, try to make it so that its default value
 *    is '\0'.  That way, when you start up your new code, it will already
 *    be set to zero (your default) in the NVRAM.
 *
 * 5) The idea behind all of this is to cause the least amount of pain to
 *    everyone when a new field is added to this system area.  Following
 *    these rules, new stuff will be added to a currently unused area, and
 *    will default to zero. This will cause no compatibility issues with
 *    prior NVRAM configurations (i.e. they won't be wiped out upon startup
 *    of the new firmware).
 *
 * 6) If you absolutely must force an incompatibility, bump the SCHEMA
 *    value -- this will force a miscompare of the schema's and
 *    will cause the NVRAM to be re-initialized.
*/
typedef struct _QM_MASTER_CONFIG
{
    UINT32      schema;         /* schema version for quorum        */
    UINT32      magicNumber;    /* magic number for quorum          */
    UINT32      virtualControllerSN;    /* VCG System ID (licensed)         */
    UINT32      electionSerial; /* election serial number           */
    /* QUAD */
    UINT32      currentMasterID;        /* Id of current Master cntrl       */
    UINT32      numControllersInVCG;    /* # of controllers in group        */
    UINT8       communicationsKey[16];  /* signature key                    */
    IP_ADDRESS  ipAddress;      /* IP address for Master            */
    IP_ADDRESS  gatewayAddress; /* Gateway address - system wide    */
    /* QUADS (2) */
    IP_ADDRESS  subnetMask;     /* Subnet Mask - system wide        */

    /* Active Controller map and padding for additional controllers */
    QM_ACTIVE_CONTROLLER_MAP activeControllerMap;

    UINT8       rsvd180[12];    /* RESERVED                         */
    /* QUADS (9) */
    UINT8       rsvd192[1];     /* RESERVED                         */

    UINT8       defragActive;   /* Defrag operation active          */
    UINT16      defragPID;      /* Defrag operation PID             */
    UINT32      ownedDriveCount;        /* Number of drives owned by VCG    */
    KEEP_ALIVE  keepAlive;      /* Keep-alive flags and slot        */

    /* Pad */
    UINT8       pad[3616 -      /* pad out crc to end of sector -16 */
                    sizeof      (UINT32) -      /* schema                           */
                    sizeof      (UINT32) -      /* magicNumber                      */
                    sizeof      (UINT32) -      /* virtualControllerSN              */
                    sizeof      (UINT32) -      /* electionSerial                   */
                    sizeof      (UINT32) -      /* currentMasterID                  */
                    sizeof      (UINT32) -      /* numControllersInVCG              */
                                (sizeof(UINT8) * 16) -  /* communicationsKey                */
                    sizeof      (IP_ADDRESS) -  /* ipAddress                        */
                    sizeof      (IP_ADDRESS) -  /* gatewayAddress                   */
                    sizeof      (IP_ADDRESS) -  /* subnetMask                       */
                    sizeof      (QM_ACTIVE_CONTROLLER_MAP) -    /* activeControllerMap              */
                                (sizeof(UINT8) * 12) -  /* rsvd180                          */
                                (sizeof(UINT8) * 4) -   /* rsvd192                          */
                    sizeof      (UINT32) -      /* ownedDriveCount                  */
                    sizeof      (KEEP_ALIVE) -  /* keepAlive                        */
                    sizeof      (UINT32)];      /* CRC (below)                      */

    UINT32      CRC;            /* CRC for master config record     */
} QM_MASTER_CONFIG;

/*
** Accessor functions for master configuration data
*/
#define SaveMasterConfig()                  StoreMasterConfigToNVRAM(); \
                WriteMasterConfiguration(&masterConfig, FS_FID_QM_MASTER_CONFIG)

#define SaveControllerConfigMap()           WriteControllerMap(&cntlConfigMap)

#define Qm_GetVirtualControllerSN()         ( masterConfig.virtualControllerSN )
#define Qm_SetVirtualControllerSN(a)        ( masterConfig.virtualControllerSN = a )

#define Qm_GetElectionSerial()              ( masterConfig.electionSerial )
#define Qm_SetElectionSerial(a)             ( masterConfig.electionSerial = a )

#define Qm_GetMasterControllerSN()          ( masterConfig.currentMasterID )
#define Qm_SetMasterControllerSN(a)         ( masterConfig.currentMasterID = a )

#define Qm_GetNumControllersAllowed()       ( masterConfig.numControllersInVCG )
#define Qm_SetNumControllersAllowed(val)    ( masterConfig.numControllersInVCG = val )

#define Qm_ActiveCntlMapPtr()               ( &masterConfig.activeControllerMap )
#define Qm_SetActiveCntlMap(i,a)            ( masterConfig.activeControllerMap.node[i] = a )
#define Qm_GetActiveCntlMap(i)              ( masterConfig.activeControllerMap.node[i] )

#define Qm_GetCommKeyPtr()                  ( masterConfig.communicationsKey )

#define Qm_GetIPAddress()                   ( masterConfig.ipAddress )
#define Qm_SetIPAddress(val)                ( masterConfig.ipAddress = val )

#define Qm_GetGateway()                     ( masterConfig.gatewayAddress )
#define Qm_SetGateway(a)                    ( masterConfig.gatewayAddress = a )

#define Qm_GetSubnet()                      ( masterConfig.subnetMask )
#define Qm_SetSubnet(a)                     ( masterConfig.subnetMask = a )

#define Qm_GetccbFwDirPtr()                 ( masterConfig.ccbFwDir )

#define Qm_GetMagicNumber()                 ( masterConfig.magicNumber )
#define Qm_SetMagicNumber(val)              ( masterConfig.magicNumber = val )

#define Qm_GetCRC()                         ( masterConfig.CRC )
#define Qm_SetCRC(val)                      ( masterConfig.CRC = val )

#define Qm_GetSchema()                      ( masterConfig.schema)
#define Qm_SetSchema(val)                   ( masterConfig.schema = val )

#define Qm_GetDefragActive()                ( masterConfig.defragActive )
#define Qm_SetDefragActive(val)             ( masterConfig.defragActive = val )

#define Qm_GetDefragPID()                   ( masterConfig.defragPID )
#define Qm_SetDefragPID(val)                ( masterConfig.defragPID = val )

#define Qm_GetOwnedDriveCount()             ( masterConfig.ownedDriveCount )
#define Qm_SetOwnedDriveCount(val)          ( masterConfig.ownedDriveCount = val )

#define Qm_VCGIDFromSerial(a)               ((a) >> 4)
#define Qm_SerialFromVCGID(a,i)             (((a) << 4) + i)
#define Qm_SlotFromSerial(a)                ((a) & 0xF)

/* Master Configuration controller Map Sectors                              */
typedef struct
{
    UINT32      controllerSN;   /* controller serial number         */
    UINT8       rsvd1[8];       /* reserved                         */
    IP_ADDRESS  ipEthernetAddress;      /* Ethernet IP address              */
    IP_ADDRESS  newIpEthernetAddress;   /* Changed Ethernet IP address      */
    IP_ADDRESS  gatewayAddress; /* Gateway address - Fibre          */
    IP_ADDRESS  newGatewayAddress;      /* Changed Gateway address          */
    IP_ADDRESS  subnetMask;     /* Subnet Mask - Fibre              */
    IP_ADDRESS  newSubnetMask;  /* Changed Subnet Mask              */
    UINT8       rsvd3[28];      /* reserved pad                     */
} QM_CONTROLLER_CONFIG;

typedef struct
{
    UINT32      schema;
    QM_CONTROLLER_CONFIG cntlConfigInfo[MAX_CONTROLLERS];
} QM_CONTROLLER_CONFIG_MAP;

#define CCM_Schema()                        ( cntlConfigMap.schema )
#define CCM_ControllerSN(a)                 ( cntlConfigMap.cntlConfigInfo[a].controllerSN )
#define CCM_IPAddress(a)                    ( cntlConfigMap.cntlConfigInfo[a].ipEthernetAddress )
#define CCM_IPAddressNew(a)                 ( cntlConfigMap.cntlConfigInfo[a].newIpEthernetAddress )
#define CCM_Gateway(a)                      ( cntlConfigMap.cntlConfigInfo[a].gatewayAddress )
#define CCM_GatewayNew(a)                   ( cntlConfigMap.cntlConfigInfo[a].newGatewayAddress )
#define CCM_Subnet(a)                       ( cntlConfigMap.cntlConfigInfo[a].subnetMask )
#define CCM_SubnetNew(a)                    ( cntlConfigMap.cntlConfigInfo[a].newSubnetMask )

/*****************************************************************************
** Public variables
*****************************************************************************/
extern QM_MASTER_CONFIG masterConfig;
extern QM_CONTROLLER_CONFIG_MAP cntlConfigMap;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern INT32 ReadMasterConfiguration(QM_MASTER_CONFIG *configPtr);
extern void WriteMasterConfiguration(QM_MASTER_CONFIG *configPtr, INT32 fid);
extern INT32 ReadControllerMap(QM_CONTROLLER_CONFIG_MAP *mapPtr);
extern INT32 WriteControllerMap(QM_CONTROLLER_CONFIG_MAP *mapPtr);
extern UINT8 SetControllerMapAddresses(UINT32 sn, UINT32 ip, UINT32 subnet, UINT32 gateway, bool bChangeIP);
extern INT32 WriteElectionData(UINT32 controllerSN, QM_ELECTION_DATA *electionPtr);
extern INT32 ReadFailureData(UINT32 controllerSN, QM_FAILURE_DATA *failurePtr);
extern INT32 WriteFailureData(UINT32 controllerSN, QM_FAILURE_DATA *failurePtr);
extern INT32 WriteFailureDataState(UINT32 controllerSN, UINT32 state);
extern INT32 WriteMailbox(UINT16 slotID, MessageDirection direction, QM_COMM_SECTOR *sectorPtr);
extern INT32 ReadCommArea(UINT16 slotID, QM_CONTROLLER_COMM_AREA *commPtr);
extern INT32 ReadAllCommunications(QM_CONTROLLER_COMM_AREA commPtr[]);
extern INT32 ReadAllMailboxes(QM_IPC_MAILBOX pComm[]);

extern UINT8 ACM_GetActiveControllerCount(QM_ACTIVE_CONTROLLER_MAP *acmPtr);
extern UINT32 ACM_GetNodeBySN(QM_ACTIVE_CONTROLLER_MAP *acmPtr, UINT16 *acmNodePtr, UINT32 serialNumber);
extern UINT32 ACM_GetNodeBySlot(QM_ACTIVE_CONTROLLER_MAP *acmPtr, UINT16 *acmNodePtr, UINT16 slotNumber);
extern UINT32 ACM_GetParent(UINT16 currentNode, UINT16 *parentNode);
extern UINT32 ACM_GetChildren(UINT16 currentNode, UINT16 *leftChildNode, UINT16 *rightChildNode);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _QUORUM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
