/*
 * iSCSI Daemon/Admin Management IPC
 *
 * Copyright (C) 2004 Dmitry Yusupov, Alex Aizman
 * maintained by open-iscsi@googlegroups.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * See the file COPYING included with this distribution for more details.
 */
#ifndef MGMT_IPC_H
#define MGMT_IPC_H

#include "types.h"
#include "iscsi_if.h"

#define ISCSIADM_NAMESPACE	"ISCSIADM_ABSTRACT_NAMESPACE"
#define ISCSI_NAME_LENGTH 260
#define PEERUSER_MAX		64
#define SECRET_LENGTH 20


typedef enum mgmt_ipc_err {
	MGMT_IPC_OK			= 0,
	MGMT_IPC_ERR			= 1,
	MGMT_IPC_ERR_NOT_FOUND		= 2,
	MGMT_IPC_ERR_NOMEM		= 3,
	MGMT_IPC_ERR_TRANS_FAILURE	= 4,
	MGMT_IPC_ERR_LOGIN_FAILURE	= 5,
	MGMT_IPC_ERR_IDBM_FAILURE	= 6,
	MGMT_IPC_ERR_INVAL		= 7,
	MGMT_IPC_ERR_TRANS_TIMEOUT	= 8,
	MGMT_IPC_ERR_INTERNAL		= 9,
	MGMT_IPC_ERR_LOGOUT_FAILURE	= 10,
	MGMT_IPC_ERR_PDU_TIMEOUT	= 11,
	MGMT_IPC_ERR_TRANS_NOT_FOUND	= 12,
	MGMT_IPC_ERR_ACCESS		= 13,
	MGMT_IPC_ERR_TRANS_CAPS		= 14,
	MGMT_IPC_ERR_EXISTS		= 15,
    MGMT_IPC_ERR_DUPLICATE_RECORD    = 16,
    MGMT_IPC_ERR_RECORD_NOT_DELETED = 17,
} mgmt_ipc_err_e;

typedef enum iscsiadm_cmd {
	MGMT_IPC_UNKNOWN		= 0,
	MGMT_IPC_SESSION_LOGIN		= 1,
	MGMT_IPC_SESSION_LOGOUT		= 2,
	MGMT_IPC_SESSION_ACTIVELIST	= 3,
	MGMT_IPC_SESSION_ACTIVESTAT	= 4,
	MGMT_IPC_CONN_ADD		= 5,
	MGMT_IPC_CONN_REMOVE		= 6,
	MGMT_IPC_SESSION_STATS		= 7,
	MGMT_IPC_CONFIG_INAME		= 8,
	MGMT_IPC_CONFIG_IALIAS		= 9,
	MGMT_IPC_CONFIG_FILE		= 10,
	X_MGMT_IPC_TARGET_ADD       = 11,
	X_MGMT_IPC_TARGET_SHOW      = 12,
	X_MGMT_IPC_SESSION_LOGIN    = 13,
	X_MGMT_IPC_TARGET_REMOVE    = 14,
	X_MGMT_IPC_DISCOVERY        = 15,
	X_MGMT_IPC_SESSION_CLOSE    = 16,
	MGMT_IPC_IMMEDIATE_STOP		= 17,
	MGMT_IPC_SESSION_SYNC		= 18,
} iscsiadm_cmd_e;
typedef struct discovery_info {
	unsigned char initiator_name[ISCSI_NAME_LENGTH];
	unsigned int initiator_ip;
	unsigned int target_ip;
	int port;
	char chap_secret[SECRET_LENGTH];
}discovery_info;

typedef struct iscsi_target
{
	char name[ISCSI_NAME_LENGTH];
	unsigned int ip;
	int port;
	int pgtag;
	
}iscsi_target;
typedef struct add_target_info
{
	unsigned char name[ISCSI_NAME_LENGTH];
	int         pgtag;
	unsigned int  ip;
	int         port;
}add_target_info;

typedef struct open_session {
    char target_name[ISCSI_NAME_LENGTH];
    int pgtag;
    char initiator_name[ISCSI_NAME_LENGTH];
    unsigned int initiator_ip;
    char chap_secret[SECRET_LENGTH];
}open_session;

/* IPC Request */
typedef struct iscsiadm_req {
	iscsiadm_cmd_e command;

	union {
		/* messages */
		struct msg_session {
			int rid;
			int sid;
		} session;
		struct msg_conn {
			int rid;
			int sid;
			int cid;
		} conn;
		discovery_info discovery;
		open_session o_session;
                add_target_info target;
	} u;
} iscsiadm_req_t;

/* IPC Response */
typedef struct iscsiadm_rsp {
	iscsiadm_cmd_e command;
	mgmt_ipc_err_e err;

	union {
		struct msg_activelist {
#define MGMT_IPC_ACTIVELIST_MAX		64
			int sids[MGMT_IPC_ACTIVELIST_MAX];
			int rids[MGMT_IPC_ACTIVELIST_MAX];
			int cnt;
		} activelist;
#define MGMT_IPC_GETSTATS_BUF_MAX	(sizeof(struct iscsi_uevent) + \
					sizeof(struct iscsi_stats) + \
					sizeof(struct iscsi_stats_custom) * \
						ISCSI_STATS_CUSTOM_MAX)
		struct msg_getstats {
			struct iscsi_uevent ev;
			struct iscsi_stats stats;
			char custom[sizeof(struct iscsi_stats_custom) *
					ISCSI_STATS_CUSTOM_MAX];
		} getstats;
		struct msg_config {
			char var[VALUE_MAXLEN];
		} config;
		struct discovery_targetlist {
#define TARGETLIST_MAX		16
			iscsi_target target[TARGETLIST_MAX];
			int cnt;
		} targetlist;
		struct session {
       		uint64_t transport_handle;
		uint64_t handle;
                uint32_t session_id;
                char device_name[ISCSI_NAME_LENGTH];
		}session;
	} u;
} iscsiadm_rsp_t;

struct idbm;
struct node_rec;

struct mgmt_ipc_db {
	struct idbm *(*init)(char *configfile);
	int (* node_read)(struct idbm *db, int rec_id, struct node_rec *rec);
	void (* terminate)(struct idbm *db);
};

struct iscsi_ipc *ipc;

void need_reap(void);
void event_loop(struct iscsi_ipc *ipc, int control_fd, int mgmt_ipc_fd,
		struct mgmt_ipc_db *db);
int mgmt_ipc_listen(void);
void mgmt_ipc_close(int fd);
/*
 ** following structure is to notify PROC about 
 ** async event
 */
typedef enum ASYNC_EVENT
{
     ASYNC_CLOSE_SESSION=0,
     
}ASYNC_EVENT;

typedef struct async_msg 
{
    ASYNC_EVENT event_type;
    char initiator_name[ISCSI_NAME_LENGTH];
    char target_name[ISCSI_NAME_LENGTH];
    int pgt;
    int error_no;
    uint64_t session_handle;

}async_msg;  
#define XIOPROC_NAMESPACE   "XIOPROC_NAMESPACE"
void  send_msg2_proc(struct async_msg *msg2proc);

#endif /* MGMT_IPC_H */
