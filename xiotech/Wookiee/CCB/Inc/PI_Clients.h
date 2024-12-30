/* $Id: PI_Clients.h 143007 2010-06-22 14:48:58Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_Clients.h
**
**  @brief      Platform Interface Client Information
**
**  This file contains the public interfaces for the client information
**  functionality of the Platform Interface.  The client information
**  module handles the list of current client connections.
**
**  Copyright (c) 2005-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __PI_CLIENTS_H__
#define __PI_CLIENTS_H__

#include "slink.h"
#include "XIO_Types.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/**
**  @ingroup    PI_IMPLEMENTATION
**  @defgroup   PI_CLIENTS Client Connections
**
**  @brief      This information helps associate a socket file descriptor
**              with its connection nformation.
**/

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  @ingroup    PI_CLIENTS
**
**  Client connection information
**/
typedef struct pi_client_info
{
    UINT32      ipaddr;         /**< Client IP Address                      */
    INT32       port;           /**< Client Port Number                     */
    INT32       sockfd;         /**< Client socket file descriptor          */
    UINT8       berror;         /**< Client error state indicator.          */
    UINT8       client_type;    /**< Client type defined in PacketInterface.h*/
    UINT16      pad1;           /* Available.                               */
    UINT64      regEvents;      /**< Event Registration Inforamtion         */
    UINT32      tmo_receive;    /**< Timeout to use for the calls to receive*/
                                /**< data.  This determines how long the    */
                                /**< platform keeps the socket open if it   */
                                /**< does not receive any requests.         */
    UINT32      pad2;           /* Available.                               */
} pi_client_info;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern MUTEX gClientsMutex;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void pi_clients_initialize(void);
extern S_LIST *pi_clients_get_list(void);
extern void pi_clients_release_list(void);
extern pi_client_info *pi_clients_add(UINT32 ipaddr, INT32 port, INT32 sockfd,
                                      INT32 tmo_receive);
extern pi_client_info *pi_clients_remove(pi_client_info *pinfo);
extern void pi_clients_close(pi_client_info *pinfo);
extern void pi_clients_error(pi_client_info *pinfo);
extern UINT64 pi_clients_register_events_get(INT32 sockfd);
extern void pi_clients_register_events_set(INT32 sockfd, UINT64 events);
extern UINT32 pi_clients_tmo_receive_get(INT32 sockfd);
extern void pi_clients_tmo_receive_set(INT32 sockfd, UINT32 tmo_receive);
extern UINT8 pi_clients_count_client_type(UINT8 client_type);
extern void pi_clients_register_client_type(UINT8 client_type, UINT32 sockfd);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_CLIENTS_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
