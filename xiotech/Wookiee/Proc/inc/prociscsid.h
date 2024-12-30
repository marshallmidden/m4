/* $Id: prociscsid.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       prociscsid.h
**
**  @brief      This is a daemon interface from PROC
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef __PROCISCSID_H
#define __PROCISCSID_H

/*
*  interface file between PROC and iscsid (daemon)
* --- Functionality covered------
*  1.    discovery
*  2.    add a target
*  3.    remove a target
*  4.    open a session to a target
*  5.    close a session to a target
*/

#include <netinet/in.h>
#include <stdint.h>
#include <sys/types.h>

typedef uint16_t __be16;
typedef uint32_t __be32;

#define ISCSI_NAME_LENGTH 260
#define SECRET_LENGTH 20

typedef struct iscsi_target
{
    char name[ISCSI_NAME_LENGTH];
    unsigned int ip;
    int port;
    int pgtag;

} iscsi_target;

typedef struct session_handle
{
    uint64_t transport_handle;
    uint64_t handle;
    uint32_t session_id;
    char     device_name[ISCSI_NAME_LENGTH];
} session_handle;

/*
request and response structure
 Any change here should also reflect in mgmt_ipc.h
*/
typedef enum iscsid_err {
    MGMT_IPC_OK         = 0,
    MGMT_IPC_ERR            = 1,
    MGMT_IPC_ERR_NOT_FOUND      = 2,
    MGMT_IPC_ERR_NOMEM      = 3,
    MGMT_IPC_ERR_TCP_FAILURE    = 4,
    MGMT_IPC_ERR_LOGIN_FAILURE  = 5,
    MGMT_IPC_ERR_IDBM_FAILURE   = 6,
    MGMT_IPC_ERR_INVAL      = 7,
    MGMT_IPC_ERR_TCP_TIMEOUT    = 8,
    MGMT_IPC_ERR_INTERNAL       = 9,
    MGMT_IPC_ERR_LOGOUT_FAILURE = 10,
    MGMT_IPC_ERR_PDU_TIMEOUT    = 11,
    MGMT_IPC_ERR_TRANS_NOT_FOUND    = 12,
    MGMT_IPC_ERR_ACCESS     = 13,
    MGMT_IPC_ERR_TRANS_CAPS     = 14,
    MGMT_IPC_ERR_SESSON_EXIST       = 15,
    MGMT_IPC_ERR_DUPLICATE_RECORD    = 16,
    MGMT_IPC_ERR_RECORD_NOT_DELETED = 17,
} iscsid_err;

/*command type*/
#define X_MGMT_IPC_TARGET_ADD       11
#define X_MGMT_IPC_TARGET_SHOW      12
#define X_MGMT_IPC_SESSION_LOGIN    13
#define X_MGMT_IPC_TARGET_REMOVE    14
#define X_MGMT_IPC_DISCOVERY        15
#define X_MGMT_IPC_SESSION_CLOSE    16

typedef struct discovery_info {
    unsigned char initiator_name[ISCSI_NAME_LENGTH];
    unsigned int initiator_ip;
    unsigned int target_ip;
    int port;
    char chap_secret[SECRET_LENGTH];
} discovery_info;

typedef struct open_session {
    char target_name[ISCSI_NAME_LENGTH];
    int pgtag;
    char initiator_name[ISCSI_NAME_LENGTH];
    unsigned int initiator_ip;
    char chap_secret[SECRET_LENGTH];
} open_session;

typedef struct add_target_info
{
    unsigned char  name[ISCSI_NAME_LENGTH];
    int pgtag;
    unsigned int  ip;
    int port;
} add_target_info;

typedef struct request {
    uint32_t command;
    union {
        /* messages */
        discovery_info discovery;
        open_session o_session;
        add_target_info target;
        /*
        ** for close session and remove target
        ** 1st 3 fields of open_session structure is used
        */
    } u;
} request;

typedef struct response {
    uint32_t command;
    uint32_t status;

    union {
        struct discovery_targetlist {
#define TARGETLIST_MAX        16
            iscsi_target target[TARGETLIST_MAX];
            int cnt;
        } targetlist;
        session_handle session;

    } u;
} response;

typedef struct response_state
{
    int fd;
    ILT* pILT;
    int bytes_read;
    response rsp;
    iscsi_target *target_list;
    int logged_in;
    int total_target;
    int i_port;
} response_state;

#endif  /* __PROCISCSID_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
