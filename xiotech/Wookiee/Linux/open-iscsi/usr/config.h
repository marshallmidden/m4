/*
 * iSCSI Configuration
 *
 * Copyright (C) 2002 Cisco Systems, Inc.
 * maintained by linux-iscsi-devel@lists.sourceforge.net
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

#ifndef CONFIG_H
#define CONFIG_H

#include <netdb.h>
#include "types.h"
#include "auth.h"	/* for the username and password sizes */

#define PORTAL_GROUP_TAG_UNKNOWN -1

/* default iSCSI port number */
#define ISCSI_DEFAULT_PORT 3260

#define CHAP_AUTHENTICATION     1

/* ISIDs now have a typed naming authority in them.  We use an OUI */
#define DRIVER_ISID_0  0x00
#define DRIVER_ISID_1  0xD0
#define DRIVER_ISID_2  0xB2

/* default window size */
#define TCP_WINDOW_SIZE (256 * 1024)

/* number of possible connections per session */
#define ISCSI_CONN_MAX		1

/* database version control */
#define IDBM_VERSION		0x05

/* the following structures store the options set in the config file.
 * a structure is defined for each logically-related group of options.
 * if you are adding a new option, first check if it should belong
 * to one of the existing groups.  If it does, add it.  If not, define
 * a new structure.
 */

/* all authentication-related options should be added to this structure.
 * this structure is per-session, and can be configured
 * by TargetName but not Subnet.
 */
struct iscsi_auth_config {
	unsigned int authmethod;
	char username[AUTH_STR_MAX_LEN];
	unsigned char password[AUTH_STR_MAX_LEN];
	unsigned int password_length;
	char username_in[AUTH_STR_MAX_LEN];
	unsigned char password_in[AUTH_STR_MAX_LEN];
	unsigned int password_in_length;
};

/* all per-connection timeouts go in this structure.
 * this structure is per-portal, and can be configured
 * both by TargetName and Subnet.
 */
struct iscsi_connection_timeout_config {
	int login_timeout;
	int auth_timeout;
	int active_timeout;
	int idle_timeout;
	int ping_timeout;
	int noop_out_interval;
	int noop_out_timeout;
};

/* all per-connection timeouts go in this structure.
 * this structure is per-session, and can be configured
 * by TargetName but not by Subnet.
 */
struct iscsi_session_timeout_config {
	int replacement_timeout;
};

/* all error handling timeouts go in this structure.
 * this structure is per-portal, and can be configured
 * both by TargetName and Subnet.
 */
struct iscsi_error_timeout_config {
	int abort_timeout;
	int reset_timeout;
};

/* all TCP options go in this structure.
 * this structure is per-portal, and can be configured
 * both by TargetName and Subnet.
 */
struct iscsi_tcp_config {
	int window_size;
	int type_of_service;	/* try to set IP TOS bits */
};

struct iscsi_conn_operational_config {
	int MaxRecvDataSegmentLength;
	int HeaderDigest;
	int DataDigest;
	int IFMarker;
	int OFMarker;
};

/* all iSCSI operational params go in this structure.
 * this structure is per-portal, and can be configured
 * both by TargetName and Subnet.
 */
struct iscsi_session_operational_config {
	int protocol;
	int InitialR2T;
	int ImmediateData;
	int FirstBurstLength;
	int MaxBurstLength;
	int DefaultTime2Wait;
	int DefaultTime2Retain;
	int MaxConnections;
	int MaxOutstandingR2T;
	int ERL;
};

#define CONFIG_DIGEST_NEVER  0
#define CONFIG_DIGEST_ALWAYS 1
#define CONFIG_DIGEST_PREFER_ON 2
#define CONFIG_DIGEST_PREFER_OFF 3

struct iscsi_sendtargets_config {
	char address[NI_MAXHOST];
	int port;
	int continuous;
	int send_async_text;
	int reopen_max;
	struct iscsi_auth_config auth;
	struct iscsi_connection_timeout_config conn_timeo;
	char initiator_name[260];
};

struct iscsi_slp_config {
	char address[NI_MAXHOST];	/* for unicast */
	int port;		/* for unicast */
	char *scopes;
	char *interfaces;	/* for multicast, list of interfaces names,
				 * "all", or "none" */
	int poll_interval;
	struct iscsi_auth_config auth;
};

typedef enum iscsi_startup {
	ISCSI_STARTUP_MANUAL,
	ISCSI_STARTUP_AUTOMATIC,
} iscsi_startup_e;

typedef enum discovery_type {
	DISCOVERY_TYPE_SENDTARGETS,
	DISCOVERY_TYPE_SLP,
	DISCOVERY_TYPE_ISNS,
} discovery_type_e;

typedef struct conn_rec {
	iscsi_startup_e				startup;
	char					address[NI_MAXHOST];
	int					port;
	struct iscsi_tcp_config			tcp;
	struct iscsi_connection_timeout_config	timeo;
	struct iscsi_conn_operational_config	iscsi;
} conn_rec_t;

typedef struct session_rec {
	int					initial_cmdsn;
	int					reopen_max;
	struct iscsi_auth_config		auth;
	struct iscsi_session_timeout_config	timeo;
	struct iscsi_error_timeout_config	err_timeo;
	struct iscsi_session_operational_config	iscsi;
} session_rec_t;

#define ISCSI_TRANSPORT_NAME_MAXLEN 16

typedef struct node_rec {
	int			id;
	int			dbversion;
	char			name[TARGET_NAME_MAXLEN];
	char			transport_name[ISCSI_TRANSPORT_NAME_MAXLEN];
	int			tpgt;
	int			active_conn;
	iscsi_startup_e		startup;
	session_rec_t		session;
	conn_rec_t		conn[ISCSI_CONN_MAX];
} node_rec_t;

typedef struct discovery_rec {
	int					id;
	int					dbversion;
	iscsi_startup_e				startup;
	discovery_type_e			type;
	union {
		struct iscsi_sendtargets_config	sendtargets;
		struct iscsi_slp_config		slp;
	} u;
} discovery_rec_t;

#endif /* CONFIG_H */
