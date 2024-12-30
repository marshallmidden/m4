/* $Id: PortServer.h 162888 2014-03-18 15:20:25Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       PortServer.h
**
**  @brief      CCB Port Server
**
**  Server that handles packet communication between the
**  CCB and the XMC.
**
**  Copyright (c) 2001 - 2008 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __PORTSERVER_H__
#define __PORTSERVER_H__

#include "XIO_Types.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Version constants
**/

/* @{ */
#define PI_MAJOR_VERSION        0x0000
#define PI_MINOR_VERSION        0x0001

/* @} */

/**
**  @name   Constants used for handling large data transfers on the X1 port
**          (in multiple packets)
**/

/* @{ */
/* #define SIZEOF_TXBUFFER         (SIZE_5MEG + sizeof(DDR_FID_HEADER)) */
/* Above not implemented. The fidread 299 is about 5mb if 64 bays exist. This
   FCM information is not filled in, because there is no way to get the data
   from the ISE. As such, fixing fidread 299 would not provide anything useful.
   To prevent taking away 3mb of memory from the CCB, for "nothing useful" is
   considered a bad thing at this time. 2014-03-18@10:18:32.241803286 */
#define SIZEOF_TXBUFFER         (SIZE_2MEG + sizeof(DDR_FID_HEADER))
#define SIZEOF_RXBUFFER         (SIZE_16MEG)

#define MPX_MAX_TX_DATA_SIZE    0xF800  /* 62K */
#define MPX_MAX_OVERALL_SIZE    (MPX_MAX_TX_DATA_SIZE + 0x100)

#define MPX_MAX_NUM_PACKETS     255
#define MPX_BIT_MAP_SIZE        ((MPX_MAX_NUM_PACKETS + 8 - 1) / 8)

/* @} */

/**
**  @name   Command completion status constants
**/

/* @{ */

#define PI_GOOD                 GOOD    /**< 0 - Command conpleted successfully */

#define PI_ERROR                ERROR   /**< 1 - Command completed with an error.
                                         **  examine the errorCode for more
                                         **  information                    */

#define PI_IN_PROGRESS              2   /**< Command is in progress         */

#define PI_TIMEOUT                  4   /**< Command has timed out          */

#define PI_INVALID_CMD_CODE         5   /**< Requested command code is not
                                         **  supported.                     */

#define PI_MALLOC_ERROR             6   /**< Memory allocation error        */

#define PI_PARAMETER_ERROR          7   /**< Command can not be completed
                                         **  because of an error in one or
                                         **  more of the input parameters.  */

#define PI_MASTER_CNT_ERROR         8   /**< This command can only be executed
                                         **  from the master controller     */

#define PI_POWER_UP_REQ_ERROR       9   /**< This command can not be executed
                                         **  until power up is complete.    */

#define PI_ELECTION_ERROR           10  /**< This command can not be executed
                                         **  because an election is in
                                         **  progress                       */

#define PI_TUNNEL_ERROR             11  /**< Error attempting to tunnel a
                                         **  request from one controller to
                                         **  another.                       */

#define PI_R5_STRIPE_RESYNC_ERROR   12  /**< This command can not be executed
                                         **  until all RAID 5 stripe resync
                                         **  operations have completed.     */

#define PI_LOCAL_RAID_RESYNC_ERROR  13  /**< This command can not be executed
                                         **  until all local/DSC resync
                                         **  operations have completed.     */

#define PI_INVALID_PACKETVERSION_ERROR   14   /**< Command version not supported  */

#define PI_COMPAT_INDEX_NOT_SUPPORTED    15   /**< Codebase level not supported   */

/* @} */

/**
**  @name   Port defintions and related constants
**/

/* @{ */
#define EWOK_PORT_NUMBER                3000
#define TEST_PORT_NUMBER                3100
#define DEBUG_PORT_NUMBER               3200

#define DEBUGCON_PORT_NUMBER            3102    /* DebugPrintf data (UDP)  */
#define DEBUGCON_MAX_CHANNELS           20      /* 20 channels 3102 - 3121 */

#define PI_SOCKET_ERROR                 -1

#define LISTENING_QUEUE_SIZE            1
#define SOCKET_RETRY_DELAY              1000    /* Delay between retries in ms  */

#define X1_PORT_TIMEOUT_DEFAULT         60
#define PI_PORT_TIMEOUT_DEFAULT         60
#define UNKNOWN_PORT_TIMEOUT_DEFAULT    60

#define TMO_PI_SEND                     10
#define TMO_PI_RECEIVE_DATA             10

#define WAIT_FOR_DIAG_ENABLE            (15 * 1000)     /* 15 seconds */

/**
** 8 ticks/sec * 60 sec/min * 60 min/hr * 24 hr => 24 hrs
**/
#define DIAG_PORT_ENABLE_TIMEOUT    ((8 * 60 * 60) * 24 /* hrs */)

/*
** Socket Errno's defined in Treck by non-standard names (e.g. "TM_Exxx")
*/
#define TM_EPERM            EPERM
#define TM_ENOENT           ENOENT
#define TM_ESRCH            ESRCH
#define TM_EINTR            EINTR
#define TM_EIO              EIO
#define TM_ENXIO            ENXIO
#define TM_EBADF            EBADF
#define TM_ECHILD           ECHILD
#define TM_ENOMEM           ENOMEM
#define TM_EACCES           EACCES
#define TM_EFAULT           EFAULT
#define TM_EEXIST           EEXIST
#define TM_ENODEV           ENODEV
#define TM_ENOTDIR          ENOTDIR
#define TM_EISDIR           EISDIR
#define TM_EINVAL           EINVAL
#define TM_EMFILE           EMFILE
#define TM_ENOSPC           ENOSPC
#define TM_EWOULDBLOCK      EWOULDBLOCK
#define TM_EINPROGRESS      EINPROGRESS
#define TM_EALREADY         EALREADY
#define TM_ENOTSOCK         ENOTSOCK
#define TM_EDESTADDRREQ     EDESTADDRREQ
#define TM_EMSGSIZE         EMSGSIZE
#define TM_EPROTOTYPE       EPROTOTYPE
#define TM_ENOPROTOOPT      ENOPROTOOPT
#define TM_EPROTONOSUPPORT  EPROTONOSUPPORT
#define TM_ESOCKTNOSUPPORT  ESOCKTNOSUPPORT
#define TM_EOPNOTSUPP       EOPNOTSUPP
#define TM_EPFNOSUPPORT     EPFNOSUPPORT
#define TM_EAFNOSUPPORT     EAFNOSUPPORT
#define TM_EADDRINUSE       EADDRINUSE
#define TM_EADDRNOTAVAIL    EADDRNOTAVAIL
#define TM_ENETDOWN         ENETDOWN
#define TM_ENETUNREACH      ENETUNREACH
#define TM_ENETRESET        ENETRESET
#define TM_ECONNABORTED     ECONNABORTED
#define TM_ECONNRESET       ECONNRESET
#define TM_ENOBUFS          ENOBUFS
#define TM_EISCONN          EISCONN
#define TM_ENOTCONN         ENOTCONN
#define TM_ESHUTDOWN        ESHUTDOWN
#define TM_ETOOMANYREFS     ETOOMANYREFS
#define TM_ETIMEDOUT        ETIMEDOUT
#define TM_ECONNREFUSED     ECONNREFUSED
#define TM_EHOSTDOWN        EHOSTDOWN
#define TM_EHOSTUNREACH     EHOSTUNREACH

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct _XIO_PACKET;
struct _TASK_PARMS;

/**
** typedef for function pointer used by PacketCommandHandler()
**/
typedef INT32 (*PI_CommandHandler_t)(struct _XIO_PACKET *, struct _XIO_PACKET *);

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 diagPortTimeout;
extern UINT32 gSysIP;
extern MUTEX gPICommandMutex;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern void DebugServer(struct _TASK_PARMS *parms);
extern void ShutdownPortServer(void);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PORTSERVER_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
