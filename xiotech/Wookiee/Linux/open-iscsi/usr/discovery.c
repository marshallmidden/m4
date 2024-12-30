/*
 * iSCSI Discovery
 *
 * Copyright (C) 2002 Cisco Systems, Inc.
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "strings.h"
#include "types.h"
#include "iscsi_proto.h"
#include "initiator.h"
#include "iscsiadm.h"
#include "log.h"

#ifdef SLP_ENABLE
#include "iscsi-slp-discovery.h"
#endif

#define DISCOVERY_NEED_RECONNECT 0xdead0001

static int rediscover = 0;
static int record_begin;

static int
send_nop_reply(iscsi_session_t *session, struct iscsi_nopin *nop,
	       char *data, int timeout)
{
	struct iscsi_nopout out;

	memset(&out, 0, sizeof (out));
	out.opcode = ISCSI_OP_NOOP_OUT | ISCSI_OP_IMMEDIATE;
	out.flags = ISCSI_FLAG_CMD_FINAL;
	memcpy(out.lun, nop->lun, sizeof (out.lun));
	out.itt = nop->itt;
	out.ttt = nop->ttt;
	memcpy(out.dlength, nop->dlength, sizeof (out.dlength));
	out.cmdsn = htonl(session->cmdsn);	/* don't increment after
						 * immediate cmds
						 */
	out.exp_statsn = htonl(session->conn[0].exp_statsn);

	log_debug(4, "sending nop reply for ttt %u, cmdsn %u, dlength %d",
		 ntohl(out.ttt), ntohl(out.cmdsn), ntoh24(out.dlength));

	return iscsi_io_send_pdu(&session->conn[0], (struct iscsi_hdr *)&out,
			ISCSI_DIGEST_NONE, data, ISCSI_DIGEST_NONE, timeout);
}

static int
iscsi_make_text_pdu(iscsi_session_t *session, struct iscsi_hdr *hdr,
		    char *data, int max_data_length)
{
	struct iscsi_text *text_pdu = (struct iscsi_text *)hdr;

	/* initialize the PDU header */
	memset(text_pdu, 0, sizeof (*text_pdu));

	text_pdu->opcode = ISCSI_OP_TEXT;
	text_pdu->itt = htonl(session->itt);
	text_pdu->ttt = ISCSI_RESERVED_TAG;
	text_pdu->cmdsn = htonl(session->cmdsn++);
	text_pdu->exp_statsn = htonl(session->conn[0].exp_statsn);

	return 1;
}

static int
request_targets(iscsi_session_t *session)
{
	char data[64];
	struct iscsi_text text;
	struct iscsi_hdr *hdr = (struct iscsi_hdr *) &text;

	memset(&text, 0, sizeof (text));
	memset(data, 0, sizeof (data));

	/* make a text PDU with SendTargets=All */
	if (!iscsi_make_text_pdu(session, hdr, data, sizeof (data))) {
		log_error("failed to make a SendTargets PDU");
		return 0;
	}

	if (!iscsi_add_text(hdr, data, sizeof (data), "SendTargets", "All")) {
		log_error("failed to add SendTargets text key");
		exit(1);
	}

	text.ttt = ISCSI_RESERVED_TAG;
	text.flags = ISCSI_FLAG_CMD_FINAL;

	if (++session->itt == ISCSI_RESERVED_TAG)
		session->itt = 1;

	if (!iscsi_io_send_pdu(&session->conn[0], hdr, ISCSI_DIGEST_NONE, data,
		    ISCSI_DIGEST_NONE, session->conn[0].active_timeout)) {
		log_error("failed to send SendTargets PDU");
		return 0;
	}

	return 1;
}

static int
iterate_targets(iscsi_session_t *session, uint32_t ttt)
{
	char data[64];
	struct iscsi_text text;
	struct iscsi_hdr *pdu = (struct iscsi_hdr *) &text;

	memset(&text, 0, sizeof (text));
	memset(data, 0, sizeof (data));

	/* make an empty text PDU */
	if (!iscsi_make_text_pdu(session, pdu, data, sizeof (data))) {
		log_error("failed to make an empty text PDU");
		return 0;
	}

	text.ttt = ttt;
	text.flags = ISCSI_FLAG_CMD_FINAL;

	if (++session->itt == ISCSI_RESERVED_TAG)
		session->itt = 1;

	if (!iscsi_io_send_pdu(&session->conn[0], pdu, ISCSI_DIGEST_NONE, data,
		    ISCSI_DIGEST_NONE, session->conn[0].active_timeout)) {
		log_error("failed to send empty text PDU");
		return 0;
	}

	return 1;
}

int
add_portal(struct string_buffer *info, char *address, char *port, char *tag)
{
	struct sockaddr_storage ss;
	char host[NI_MAXHOST];

	/* resolve the address, in case it was a DNS name */
	if (resolve_address(address, port, &ss)) {
		log_error("cannot resolve %s", address);
		return 0;
	}

	/* convert the resolved name to text */
	getnameinfo((struct sockaddr *) &ss, sizeof(ss),
		    host, sizeof(host), NULL, 0, NI_NUMERICHOST);

	if (tag && *tag) {
		if (!append_sprintf(info, "TT=%s\n", tag)) {
			log_error("couldn't add portal tag %s",
				  tag);
			return 0;
		}
	}

	if (port && *port) {
		if (!append_sprintf(info, "TP=%s\n", port)) {
			log_error("couldn't add port %s", port);
			return 0;
		}
	}

	if (strcmp(host, address))
		/* if the resolved name doesn't match the original,
		 * send an RA line as well as a TA line
		 */
		return append_sprintf(info, "RA=%s\nTA=%s\n",
				      host, address);
	else
		/* don't need the RA line */
		return append_sprintf(info, "TA=%s\n", address);
}

int
add_target_record(struct string_buffer *info, char *name, char *end,
		  int lun_inventory_changed, char *default_address,
		  char *default_port)
{
	char *text = NULL;
	char *nul = name;
	size_t length;
	size_t original = data_length(info);

	/* address = IPv4
	 * address = [IPv6]
	 * address = DNSname
	 * address = IPv4:port
	 * address = [IPv6]:port
	 * address = DNSname:port
	 * address = IPv4,tag
	 * address = [IPv6],tag
	 * address = DNSname,tag
	 * address = IPv4:port,tag
	 * address = [IPv6]:port,tag
	 * address = DNSname:port,tag
	 */

	log_debug(7, "adding target record %p, end %p", name, end);

	/* find the end of the name */
	while ((nul < end) && (*nul != '\0'))
		nul++;

	length = nul - name;
	if (length > TARGET_NAME_MAXLEN) {
		log_error("TargetName %s too long, ignoring", name);
		return 0;
	}

	if (!record_begin) {
		if (!append_sprintf
		    (info, lun_inventory_changed ? "DLC=%s\n" : "DTN=%s\n",
		     name)) {
			log_error("couldn't report target %s", name);
			truncate_buffer(info, original);
			return 0;
		}
		record_begin = 1;
	} else {
		if (!append_sprintf
		    (info, lun_inventory_changed ? "LC=%s\n" : "TN=%s\n",
		     name)) {
			log_error("couldn't report target %s", name);
			truncate_buffer(info, original);
			return 0;
		}
	}

	text = name + length;

	/* skip NULs after the name */
	while ((text < end) && (*text == '\0'))
		text++;

	/* if no address is provided, use the default */
	if (text >= end) {
		if (default_address == NULL) {
			log_error(
			       "no default address known for target %s", name);
			truncate_buffer(info, original);
			return 0;
		} else
		    if (!add_portal(info, default_address, default_port, NULL))
		{
			log_error(
			       "failed to add default portal, ignoring "
			       "target %s", name);
			truncate_buffer(info, original);
			return 0;
		} else if (!append_string(info, ";\n")) {
			log_error(
			       "failed to terminate target record, "
			       "ignoring target %s", name);
			truncate_buffer(info, original);
			return 0;
		}

		/* finished adding the default */
		return 1;
	}

	/* process TargetAddresses */
	while (text < end) {
		char *next = text + strlen(text) + 1;

		log_debug(7, "text %p, next %p, end %p, %s", text, next, end,
			 text);

		if (strncmp(text, "TargetAddress=", 14) == 0) {
			char *port = NULL;
			char *tag = NULL;
			char *address = text + 14;
			char *temp;

			if ((tag = strrchr(text, ','))) {
				*tag = '\0';
				tag++;
			}
			if ((port = strrchr(text, ':'))) {
				*port = '\0';
				port++;
			}

			if (*address == '[') {
				address++;
				if ((temp = strrchr(text, ']')))
					*temp = '\0';
			}

			if (!add_portal(info, address, port, tag)) {
				log_error(
				       "failed to add default portal, "
				       "ignoring target %s", name);
				truncate_buffer(info, original);
				return 0;
			}
		} else {
			log_error("unexpected SendTargets data: %s",
			       text);
		}

		text = next;
	}

	/* indicate the end of the target record */
	if (!append_string(info, ";\n")) {
		log_error(
		       "failed to terminate target record, ignoring target %s",
		       name);
		truncate_buffer(info, original);
		return 0;
	}

	return 1;
}

static int
process_sendtargets_response(struct string_buffer *sendtargets,
			     struct string_buffer *info, int final,
			     int lun_inventory_changed, char *default_address,
			     char *default_port)
{
	char *start = buffer_data(sendtargets);
	char *text = start;
	char *end = text + data_length(sendtargets);
	char *nul = end - 1;
	char *record = NULL;
	int num_targets = 0;
	size_t valid_info = 0;

	if (start == end) {
		/* no SendTargets data */
		goto done;
	}

	/* scan backwards to find the last NUL in the data, to ensure we
	 * don't walk off the end.  Since key=value pairs can span PDU
	 * boundaries, we're not guaranteed that the end of the data has a
	 * NUL.
	 */
	while ((nul > start) && *nul)
		nul--;

	if (nul == start) {
		/* couldn't find anything we can process now,
		 * it's one big partial string
		 */
		goto done;
	}

	/* find the boundaries between target records (TargetName or final PDU)
	 */
	for (;;) {
		/* skip NULs */
		while ((text < nul) && (*text == '\0'))
			text++;

		if (text == nul)
			break;

		log_debug(7,
			 "processing sendtargets record %p, text %p, line %s",
			 record, text, text);

		/* look for the start of a new target record */
		if (strncmp(text, "TargetName=", 11) == 0) {
			if (record) {
				/* send the last record, which we just found
				 * the end of. don't bother passing the
				 * "TargetName=" prefix.
				 */
				if (!add_target_record(info, record + 11, text,
				        lun_inventory_changed, default_address,
				        default_port)) {
					log_error(
					       "failed to add target record");
					truncate_buffer(sendtargets, 0);
					truncate_buffer(info, valid_info);
					goto done;
				}
				num_targets++;
				valid_info = data_length(info);
			}
			record = text;
		}

		/* everything up til the next NUL must be part of the
		 * current target record
		 */
		while ((text < nul) && (*text != '\0'))
			text++;
	}

	if (record) {
		if (final) {
			/* if this is the last PDU of the text sequence,
			 * it also ends a target record
			 */
			log_debug(7,
				 "processing final sendtargets record %p, "
				 "line %s",
				 record, record);
			if (add_target_record (info, record + 11, text,
					lun_inventory_changed, default_address,
					default_port)) {
				num_targets++;
				record = NULL;
				truncate_buffer(sendtargets, 0);
			} else {
				log_error("failed to add target record");
				truncate_buffer(sendtargets, 0);
				truncate_buffer(info, valid_info);
				goto done;
			}
		} else {
			/* remove the parts of the sendtargets buffer we've
			 * processed, and move the parts we haven't to the
			 * beginning of the buffer.
			 */
			log_debug(7,
				 "processed %d bytes of sendtargets data, "
				 "%d remaining",
				 (int)(record - buffer_data(sendtargets)),
				 (int)(buffer_data(sendtargets) +
				 data_length(sendtargets) - record));
			remove_initial(sendtargets,
				       record - buffer_data(sendtargets));
		}
	}

      done:
	/* send all of the discovered targets to the fd ("stdout" currently) */
	if (append_string(info, final ? "!\n" : ".\n")) {
		if (final) {
			record_begin = 0;
		}
		log_debug(4, "sent %d targets to the parent", num_targets);
		return 1;
	} else {
		log_error("couldn't send %d targets to the parent",
		       num_targets);
		return 0;
	}

	return 1;
}

static int
add_async_record(struct string_buffer *info, char *record, int targetoffline)
{
	int length = strlen(record);
	size_t original = data_length(info);

	log_debug(7, " adding async record for %s", record);

	if (targetoffline) {
		/* We have received targetoffline event */
		if (length > TARGET_NAME_MAXLEN) {
			log_error("Targetname %s too long, ignoring",
			       record);
			return 0;
		}
	}
	if (!append_sprintf
	    (info, targetoffline ? "ATF=%s\n" : "APF=%s\n", record)) {
		log_error("couldn't report the record\n");
		truncate_buffer(info, original);
		return 0;
	} else if (!append_string(info, ";\n")) {
		log_error(
		       "failed to terminate target record, ignoring target %s",
		       record);
		truncate_buffer(info, original);
	}
	return 1;
}

static void
clear_timer(struct timeval *timer)
{
	memset(timer, 0, sizeof (*timer));
}

static int
process_async_event_text(struct string_buffer *sendtargets,
			 struct string_buffer *info, struct timeval *timer)
{
	char *text = buffer_data(sendtargets);
	int targetoffline = 0;
	int slen = (ntohs(*(short *) (text)));
	text = text + 2 + slen;

	if (strncmp(text, "X-com.cisco.targetOffline=", 26) == 0) {
		targetoffline = 1;
		if (!add_async_record(info, text + 26, targetoffline)) {
			log_error("failed to add async record");
			return 0;
		}
		clear_timer(timer);
	} else if (strncmp(text, "X-com.cisco.portalOffline=", 26) == 0) {
		targetoffline = 0;
		if (!add_async_record(info, text + 26, targetoffline)) {
			log_error("failed to add async record");
			return 0;
		}
		clear_timer(timer);
	} else {
		log_debug(1, "sendtargets for the other events");
		return 1;
	}

	if (append_string(info, "!\n")) {
		log_debug(4, "sent async event record to the caller");
		return 1;
	} else {
		log_error("couldn't send async event record to the caller");
		return 0;
	}
}

/* set timer to now + seconds */
static void
set_timer(struct timeval *timer, int seconds)
{
	if (timer) {
		memset(timer, 0, sizeof (*timer));
		gettimeofday(timer, NULL);

		timer->tv_sec += seconds;
	}
}

static int
timer_expired(struct timeval *timer)
{
	struct timeval now;

	/* no timer, can't have expired */
	if ((timer == NULL) || ((timer->tv_sec == 0) && (timer->tv_usec == 0)))
		return 0;

	memset(&now, 0, sizeof (now));
	gettimeofday(&now, NULL);

	if (now.tv_sec > timer->tv_sec)
		return 1;
	if ((now.tv_sec == timer->tv_sec) && (now.tv_usec >= timer->tv_usec))
		return 1;
	return 0;
}

static int
msecs_until(struct timeval *timer)
{
	struct timeval now;
	int msecs;
	long partial;

	/* no timer, can't have expired, infinite time til it expires */
	if ((timer == NULL) || ((timer->tv_sec == 0) && (timer->tv_usec == 0)))
		return -1;

	memset(&now, 0, sizeof (now));
	gettimeofday(&now, NULL);

	/* already expired? */
	if (now.tv_sec > timer->tv_sec)
		return 0;
	if ((now.tv_sec == timer->tv_sec) && (now.tv_usec >= timer->tv_usec))
		return 0;

	/* not expired yet, do the math */
	partial = timer->tv_usec - now.tv_usec;
	if (partial < 0) {
		partial += 1000 * 1000;
		msecs = (partial + 500) / 1000;
		msecs += (timer->tv_sec - now.tv_sec - 1) * 1000;
	} else {
		msecs = (partial + 500) / 1000;
		msecs += (timer->tv_sec - now.tv_sec) * 1000;
	}

	return msecs;
}

static int
soonest_msecs(struct timeval *t1, struct timeval *t2, struct timeval *t3)
{
	int m1 = msecs_until(t1);
	int m2 = msecs_until(t2);
	int m3 = msecs_until(t3);

	/* infinity is -1, handle it specically */
	if ((m1 == -1) && (m2 == -1))
		return m3;

	if ((m1 == -1) && (m3 == -1))
		return m2;

	if ((m2 == -1) && (m3 == -1))
		return m1;

	if (m1 == -1)
		return (m2 < m3) ? m2 : m3;

	if (m2 == -1)
		return (m1 < m3) ? m1 : m3;

	if (m3 == -1)
		return (m1 < m2) ? m1 : m2;

	if (m1 < m2)
		return (m1 < m3) ? m1 : m3;
	else
		return (m2 < m3) ? m2 : m3;
}

static iscsi_session_t *
init_new_session(struct iscsi_sendtargets_config *config)
{
	iscsi_session_t *session;

	session = calloc(1, sizeof (*session));
	if (session == NULL) {
		log_error("discovery process to %s:%d failed to "
		       "allocate a session", config->address, config->port);
		goto done;
	}

	/* initialize the session's leading connection */
	session->conn[0].socket_fd = -1;
	session->conn[0].login_timeout = config->conn_timeo.login_timeout;
	session->conn[0].auth_timeout = config->conn_timeo.auth_timeout;
	session->conn[0].active_timeout = config->conn_timeo.active_timeout;
	session->conn[0].idle_timeout = config->conn_timeo.idle_timeout;
	session->conn[0].ping_timeout = config->conn_timeo.ping_timeout;
	session->send_async_text = config->continuous ?
						config->send_async_text : -1;
	session->conn[0].hdrdgst_en = ISCSI_DIGEST_NONE;
	session->conn[0].datadgst_en = ISCSI_DIGEST_NONE;
	session->conn[0].max_recv_dlength = DEFAULT_MAX_RECV_DATA_SEGMENT_LENGTH;
	session->conn[0].max_xmit_dlength = DEFAULT_MAX_RECV_DATA_SEGMENT_LENGTH;

	session->reopen_cnt = config->reopen_max;

	/* OUI and uniqifying number */
	session->isid[0] = DRIVER_ISID_0;
	session->isid[1] = DRIVER_ISID_1;
	session->isid[2] = DRIVER_ISID_2;
	session->isid[3] = 0;
	session->isid[4] = 0;
	session->isid[5] = 0;

	/* initialize the session */
	strcpy(session->initiator_name,config->initiator_name);
	session->initiator_alias = initiator_alias;
	session->portal_group_tag = PORTAL_GROUP_TAG_UNKNOWN;
	session->type = ISCSI_SESSION_TYPE_DISCOVERY;

	log_debug(4, "sendtargets discovery to %s:%d using "
		 "isid 0x%02x%02x%02x%02x%02x%02x",
		 config->address, config->port, session->isid[0],
		 session->isid[1], session->isid[2], session->isid[3],
		 session->isid[4], session->isid[5]);
done:
	return(session);
}


static int
setup_authentication(iscsi_session_t *session,
		     struct iscsi_sendtargets_config *config)
{
	int rc;

	rc = 1;

	/* if we have any incoming credentials, we insist on authenticating
	 * the target or not logging in at all
	 */
	if (config->auth.username_in[0]
	    || config->auth.password_in_length) {
		session->bidirectional_auth = 1;

		/* sanity check the config */
		if ((config->auth.username[0] == '\0')
		    || (config->auth.password_length == 0)) {
			log_error(
			       "discovery process to %s:%d has incoming "
			       "authentication credentials but has no outgoing "
			       "credentials configured",
			       config->address, config->port);
			log_error(
			       "discovery process to %s:%d exiting, bad "
			       "configuration",
			       config->address, config->port);
			rc = 0;
			goto done;
		}
	} else {
		/* no or 1-way authentication */
		session->bidirectional_auth = 0;
	}

	/* copy in whatever credentials we have */
	strncpy(session->username, config->auth.username,
		sizeof (session->username));
	session->username[sizeof (session->username) - 1] = '\0';
	if ((session->password_length = config->auth.password_length))
		memcpy(session->password, config->auth.password,
		       session->password_length);

	strncpy(session->username_in, config->auth.username_in,
		sizeof (session->username_in));
	session->username_in[sizeof (session->username_in) - 1] = '\0';
	if ((session->password_in_length =
	     config->auth.password_in_length))
		memcpy(session->password_in, config->auth.password_in,
		       session->password_in_length);

	if (session->password_length || session->password_in_length) {
		/* setup the auth buffers */
		session->auth_buffers[0].address = &session->auth_client_block;
		session->auth_buffers[0].length =
		    sizeof (session->auth_client_block);
		session->auth_buffers[1].address =
		    &session->auth_recv_string_block;
		session->auth_buffers[1].length =
		    sizeof (session->auth_recv_string_block);

		session->auth_buffers[2].address =
		    &session->auth_send_string_block;
		session->auth_buffers[2].length =
		    sizeof (session->auth_send_string_block);

		session->auth_buffers[3].address =
		    &session->auth_recv_binary_block;
		session->auth_buffers[3].length =
		    sizeof (session->auth_recv_binary_block);

		session->auth_buffers[4].address =
		    &session->auth_send_binary_block;
		session->auth_buffers[4].length =
		    sizeof (session->auth_send_binary_block);

		session->num_auth_buffers = 5;
	} else {
		session->num_auth_buffers = 0;
	}
 done:
	return(rc);
}

static int
process_recvd_pdu(struct iscsi_hdr *pdu,
		  struct iscsi_sendtargets_config *config,
		  iscsi_session_t *session,
		  struct string_buffer *sendtargets,
		  struct string_buffer *info,
		  int *lun_inventory_changed,
		  char *default_port,
		  int *active,
		  int *long_lived,
		  struct timeval *async_timer,
		  char *data)
{
	int rc=0;

	switch (pdu->opcode) {
		case ISCSI_OP_TEXT_RSP:{
			struct iscsi_text_rsp *text_response =
				(struct iscsi_text_rsp *) pdu;
			int dlength = ntoh24(pdu->dlength);
			int final =
				(text_response->flags & ISCSI_FLAG_CMD_FINAL) ||
				(text_response-> ttt == ISCSI_RESERVED_TAG);

			log_debug(4, "discovery session to %s:%d received text"
				 " response, %d data bytes, ttt 0x%x, "
				 "final 0x%x",
				 config->address,
				 config->port,
				 dlength,
				 ntohl(text_response->ttt),
				 text_response->flags & ISCSI_FLAG_CMD_FINAL);

			/* mark how much more data in the sendtargets
			 * buffer is now valid
			 */
			enlarge_data (sendtargets, dlength);

			/* process as much as we can right now */
			process_sendtargets_response (sendtargets,
						      info, final,
						      *lun_inventory_changed,
						      config->address,
						      default_port);

			if (final) {
				/* SendTargets exchange is now complete
				 */
				*active = 0;
				*lun_inventory_changed = 1;
				/* from now on, after any reconnect,
				 * assume LUNs may have changed
				 */
			} else {
				/* ask for more targets */
				if (!iterate_targets(session,
						     text_response->ttt)) {
					rc = DISCOVERY_NEED_RECONNECT;
					goto done;
				}
			}
			break;
		}
		case ISCSI_OP_ASYNC_EVENT:{
			struct iscsi_async *async_hdr =
				(struct iscsi_async *) pdu;
			int dlength = ntoh24(pdu->dlength);
			short senselen;
			char logbuf[128];
			int i;

			/*
			 * If we receive an async message stating
			 * the target wants to close the connection,
			 * then don't try to reconnect anymore.
			 * This is reasonable, so we don't log
			 * anything here.
			 */
			if ((async_hdr->async_event ==
			      ISCSI_ASYNC_MSG_REQUEST_LOGOUT)||
			     (async_hdr->async_event ==
			      ISCSI_ASYNC_MSG_DROPPING_CONNECTION)||
			     (async_hdr->async_event ==
			      ISCSI_ASYNC_MSG_DROPPING_ALL_CONNECTIONS)) {
				*long_lived=0;
				break;
			}

			/*
			 * Log info about the async message.
			 */
			log_warning(
			       "Received Async Msg from target, Event = %d, "
			       "Code = %d, Data Len = %d",
			       async_hdr->async_event,
			       async_hdr->async_vcode, dlength);

			/*
			 * If there was data, print out the first 8 bytes
			 */
			if (dlength > 0) {
				memset(logbuf, 0, sizeof(logbuf));
				for (i=0; i<8 && i<dlength; ++i) {
					sprintf(logbuf+i*5, "0x%02x ",
						data[i]);
				}
				log_warning(" Data[0]-[%d]: %s",
					i<dlength ? dlength-1 : i-1,
					logbuf);
			}


			if (dlength > (sizeof (short))) {
				senselen = (ntohs(*(short *) (data)));

				log_debug(1, " senselen = %d", senselen);
				if (dlength > senselen + 2) {
					log_debug(1, "recvd async event : %s",
						 data + 2 + senselen);
				}
			}
			*long_lived = 1;

			/*
			 * Arrange for a rediscovery to  occur in the near
			 * future.  We use a timer so that we merge
			 * multiple events that occur in rapid succession,
			 * and only rediscover once for each  burst of
			 * Async events.
			 */
			set_timer(async_timer, 1);
			if (*active) {
				log_debug(4, "discovery process received Async "
					 "event while active");
			} else {
				log_debug(4, "discovery process received Async "
					 "event while idle");
			}
			process_async_event_text(sendtargets, info,
						 async_timer);
			break;
		}
		case ISCSI_OP_NOOP_IN:{
			struct iscsi_nopin *nop =
				(struct iscsi_nopin *) pdu;

			/*
			 * The iSCSI spec doesn't allow Nops on
			 * discovery sessions, but some targets
			 * use them anyway.  If we receive one, we
			 * can safely assume that the target
			 * supports long-lived discovery sessions
			 * (otherwise it wouldn't be sending nops
			 * to verify the connection is still
			 * working).
			 */
			*long_lived = 1;
			log_debug(4,"discovery session to %s:%d  received"
				 " Nop-in with itt %u, ttt %u, dlength %u",
				 config->address, config->port,
				 ntohl(nop->itt), ntohl(nop->ttt),
				 ntoh24(nop->dlength));

			if (nop->ttt != ISCSI_RESERVED_TAG) {
				/* reply to the  Nop-in */
				if (!send_nop_reply(session, nop, data,
					    session->conn[0].active_timeout)) {
					log_error(
						"discovery session to %s:%d "
						"failed to send Nop reply, "
						"ttt %u, reconnecting",
						 config->address,
						 config->port,
						 ntohl(nop->ttt));
					rc = DISCOVERY_NEED_RECONNECT;
					goto done;
				}
			}
			break;
		}
		case ISCSI_OP_REJECT:{
			struct iscsi_reject *reject =
				(struct iscsi_reject *) pdu;
			int dlength = ntoh24(pdu->dlength);

			log_error("reject, dlength=%d, "
			       "data[0]=0x%x",
			       dlength, data[0]);
			log_error(
			       "Received a reject from the target "
			       "with reason code = 0x%x",
			       reject->reason);
			/*
			 * Just attempt to reconnect if we receive a reject
			 */
			rc = DISCOVERY_NEED_RECONNECT;
			goto done;
			break;
		}
		default:{
			log_warning(
			       "discovery session to %s:%d received "
			       "unexpected opcode 0x%x",
			       config->address, config->port, pdu->opcode);
			rc = DISCOVERY_NEED_RECONNECT;
			goto done;
		}
	}
 done:
	return(rc);
}

/*
 * Make a best effort to logout the session, then disconnect the
 * socket.
 */
static void
iscsi_logout_and_disconnect(iscsi_session_t * session)
{
	struct iscsi_logout logout_req;
	struct iscsi_logout_rsp logout_resp;
	int rc;

	/*
	 * Build logout request header
	 */
	memset(&logout_req, 0, sizeof (logout_req));
	logout_req.opcode = ISCSI_OP_LOGOUT | ISCSI_OP_IMMEDIATE;
	logout_req.flags = ISCSI_FLAG_CMD_FINAL |
		(ISCSI_LOGOUT_REASON_CLOSE_SESSION &
				ISCSI_FLAG_LOGOUT_REASON_MASK);
	logout_req.itt = htonl(session->itt);
	if (++session->itt == ISCSI_RESERVED_TAG)
		session->itt = 1;
	logout_req.cmdsn = htonl(session->cmdsn);
	logout_req.exp_statsn = htonl(++session->conn[0].exp_statsn);

	/*
	 * Send the logout request
	 */
	rc = iscsi_io_send_pdu(&session->conn[0],(struct iscsi_hdr *)&logout_req,
			    ISCSI_DIGEST_NONE, NULL, ISCSI_DIGEST_NONE, 3);
	if (!rc) {
		log_error(
		       "iscsid: iscsi_logout - failed to send logout PDU.");
		goto done;
	}

	/*
	 * Read the logout response
	 */
	memset(&logout_resp, 0, sizeof(logout_resp));
	rc = iscsi_io_recv_pdu(&session->conn[0],
		(struct iscsi_hdr *)&logout_resp, ISCSI_DIGEST_NONE, NULL,
		0, ISCSI_DIGEST_NONE, 1);
	if (!rc) {
		log_error("iscsid: logout - failed to receive logout resp");
		goto done;
	}
	if (logout_resp.response != ISCSI_LOGOUT_SUCCESS) {
		log_error("iscsid: logout failed - response = 0x%x",
		       logout_resp.response);
	}

done:
	/*
	 * Close the socket.
	 */
	iscsi_io_disconnect(&session->conn[0]);
}

int
sendtargets_discovery(struct iscsi_sendtargets_config *config,
		      struct string_buffer *info)
{
	iscsi_session_t *session;
	struct pollfd pfd;
	struct iscsi_hdr pdu_buffer;
	struct iscsi_hdr *pdu = &pdu_buffer;
	char *data = NULL;
	char *end_of_data;
	int long_lived = (config->continuous > 0) ? 1 : 0;
	int lun_inventory_changed = 0;
	int active = 0;
	struct timeval connection_timer, async_timer;
	int timeout;
	int rc;
	struct string_buffer sendtargets;
	uint8_t status_class = 0, status_detail = 0;
	unsigned int login_failures = 0;
	int login_delay = 0;
	struct sockaddr_storage ss;
	char host[NI_MAXHOST], serv[NI_MAXSERV], default_port[NI_MAXSERV];

	/* initial setup */
	log_debug(1, "starting sendtargets discovery, address %s:%d, "
		 "continuous %d", config->address, config->port,
		 config->continuous);
	memset(&pdu_buffer, 0, sizeof (pdu_buffer));
	clear_timer(&connection_timer);
	clear_timer(&async_timer);

	/* allocate data buffers for SendTargets data and discovery pipe info */
	init_string_buffer(&sendtargets, 32 * 1024);

	/* allocate a new session, and initialize default values */
	session = init_new_session(config);
	if (session == NULL) {
		return 1;
	}

	sprintf(default_port, "%d", config->port);
	/* resolve the DiscoveryAddress to an IP address */
	if (resolve_address(config->address, default_port, &ss)) {
		log_error("cannot resolve host name %s", config->address);
		return 1;
	}

	log_debug(4, "discovery timeouts: login %d, reopen_cnt %d, auth %d, "
		 "active %d, idle %d, ping %d",
		 session->conn[0].login_timeout, session->reopen_cnt,
		 session->conn[0].auth_timeout, session->conn[0].active_timeout,
		 session->conn[0].idle_timeout, session->conn[0].ping_timeout);

	/* setup authentication variables for the session*/
	rc = setup_authentication(session, config);
	if (rc == 0)
		return 1;

set_address:
	/*
	 * copy the saved address to the session,
	 * undoing any temporary redirect
	 */
	session->conn[0].saddr = ss;

reconnect:

	if (--session->reopen_cnt < 0) {
		log_error("connection login retries (reopen_max) %d exceeded",
			  config->reopen_max);
		return 1;
	}

redirect_reconnect:

	iscsi_io_disconnect(&session->conn[0]);

	session->cmdsn = 1;
	session->itt = 1;
	session->portal_group_tag = PORTAL_GROUP_TAG_UNKNOWN;

	/* slowly back off the frequency of login attempts */
       login_delay = 0;
	if (login_delay) {
		log_debug(4, "discovery session to %s:%d sleeping for %d "
			 "seconds before next login attempt",
			 config->address, config->port, login_delay);
		sleep(login_delay);
	}

	getnameinfo((struct sockaddr *) &session->conn[0].saddr,
		    sizeof(session->conn[0].saddr), host,
		    sizeof(host), serv, sizeof(serv),
		    NI_NUMERICHOST|NI_NUMERICSERV);
        log_debug(4,"check if sesison is null %p",session->conn[0].session);
        if(session->conn[0].session == NULL)
        {
           session->conn[0].session = session;
        }
	if (!iscsi_io_connect(&session->conn[0])) {
		log_error("connection to discovery address %s "
			  "failed", host);

		login_failures++;
		/* If a temporary redirect sent us to something unreachable,
		 * we want to go back to the original IP address, so make sure
		 * we reset the session's IP.
		 */
                /*
                ** in case it is failing we will return from here. Let idriver do the retry
                */ 
                return 1;
	}

	log_debug(1, "connected to discovery address %s", host);

	log_debug(4, "discovery session to %s:%d starting iSCSI login on fd %d",
		 config->address, config->port, session->conn[0].socket_fd);

	/* In case of discovery, we using socket's descriptor as ctrl. */
	session->ctrl_fd = session->conn[0].socket_fd;
	session->conn[0].session = session;

	status_class = 0;
	status_detail = 0;
	rc = iscsi_login(session, 0, buffer_data(&sendtargets),
			 unused_length(&sendtargets),
			 &status_class, &status_detail);

	switch (rc) {
	case LOGIN_OK:
	case LOGIN_REDIRECT:
		break;

	case LOGIN_IO_ERROR:
	case LOGIN_WRONG_PORTAL_GROUP:
	case LOGIN_REDIRECTION_FAILED:
		/* try again */
		log_warning("retrying discovery login to %s", host);
		iscsi_io_disconnect(&session->conn[0]);
		login_failures++;
		goto set_address;

	default:
	case LOGIN_FAILED:
	case LOGIN_NEGOTIATION_FAILED:
	case LOGIN_AUTHENTICATION_FAILED:
	case LOGIN_VERSION_MISMATCH:
	case LOGIN_INVALID_PDU:
		log_error("discovery login to %s failed, giving up", host);
		iscsi_io_disconnect(&session->conn[0]);
		return 1;
	}

	/* check the login status */
	switch (status_class) {
	case ISCSI_STATUS_CLS_SUCCESS:
		log_debug(4, "discovery login success to %s", host);
		login_failures = 0;
		break;
	case ISCSI_STATUS_CLS_REDIRECT:
		switch (status_detail) {
			/* the session IP address was changed by the login
			 * library, so just try again with this portal
			 * config but the new address.
			 */
		case ISCSI_LOGIN_STATUS_TGT_MOVED_TEMP:
			log_warning(
				"discovery login temporarily redirected to "
				"%s port %s", host, serv);
			goto redirect_reconnect;
		case ISCSI_LOGIN_STATUS_TGT_MOVED_PERM:
			log_warning(
				"discovery login permanently redirected to "
				"%s port %s", host, serv);
			/* make the new address permanent */
			ss = session->conn[0].saddr;
			goto redirect_reconnect;
		default:
			log_error(
			       "discovery login rejected: redirection type "
			       "0x%x not supported",
			       status_detail);
			goto set_address;
		}
		break;
	case ISCSI_STATUS_CLS_INITIATOR_ERR:
		log_error(
			"discovery login to %s rejected: "
			"initiator error (%02x/%02x), non-retryable, giving up",
			host, status_class, status_detail);
		iscsi_io_disconnect(&session->conn[0]);
		return 1;
	case ISCSI_STATUS_CLS_TARGET_ERR:
		log_error(
			"discovery login to %s rejected: "
			"target error (%02x/%02x)",
			host, status_class, status_detail);
		iscsi_io_disconnect(&session->conn[0]);
		login_failures++;
		goto reconnect;
	default:
		log_error(
			"discovery login to %s failed, response "
			"with unknown status class 0x%x, detail 0x%x",
			host,
			status_class, status_detail);
		iscsi_io_disconnect(&session->conn[0]);
		login_failures++;
		goto reconnect;
	}

      rediscover:
	/* reinitialize */
	truncate_buffer(&sendtargets, 0);
	truncate_buffer(info, 0);

	/* we're going to do a discovery regardless */
	clear_timer(&async_timer);

	/* ask for targets */
	if (!request_targets(session)) {
		goto reconnect;
	}
	active = 1;

	/* set timeouts */
	if (long_lived) {
		clear_timer(&connection_timer);
	} else {
		set_timer(&connection_timer,
		 session->conn[0].active_timeout + session->conn[0].ping_timeout);
	}

	/* prepare to poll */
	memset(&pfd, 0, sizeof (pfd));
	pfd.fd = session->conn[0].socket_fd;
	pfd.events = POLLIN | POLLPRI;

	/* check timers before blocking */
	if (timer_expired(&connection_timer)) {
		if (long_lived || !lun_inventory_changed) {
			/* long-lived, or never finished the first
			 * exchange (might be long-lived)
			 */
			clear_timer(&connection_timer);
			log_debug(1,
			       "discovery session to %s:%d  "
			       "reconnecting after connection timeout",
			       config->address, config->port);
			goto reconnect;
		} else {
			log_warning(
			       "discovery session to %s:%d session "
			       "logout, connection timer expired",
			       config->address, config->port);
			iscsi_logout_and_disconnect(session);
			return 1;
		}
	}

	if (active) {
		/* ignore the async timer, we're in the middle
		 * of a discovery
		 */
		timeout = msecs_until(&connection_timer);
	} else {
		/* to avoid doing LUN probing repeatedly, try to merge
		 * multiple Async PDUs into one rediscovery by
		 * deferring discovery until a timeout expires.
		 */
		if (timer_expired(&async_timer)) {
			log_debug(4,
				 "discovery session to %s:%d async "
				 "timer expired, rediscovering",
				 config->address, config->port);
			clear_timer(&async_timer);
			goto rediscover;
		} else
			timeout =
				soonest_msecs(NULL,
					  &connection_timer,
					  &async_timer);
	}

	/* block until we receive a PDU, a TCP FIN, a TCP RST,
	 * or a timeout
	 */
	log_debug(4,
		 "discovery process  %s:%d polling fd %d, "
		 "timeout in %f seconds",
		 config->address, config->port, pfd.fd,
		 timeout / 1000.0);

	pfd.revents = 0;
	rc = poll(&pfd, 1, timeout);

	log_debug(7,
		 "discovery process to %s:%d returned from poll, rc %d",
		 config->address, config->port, rc);

	if (rc > 0) {
		if (pfd.revents & (POLLIN | POLLPRI)) {
			/* put any PDU data into the
			 * sendtargets buffer for now
			 */
			data = buffer_data(&sendtargets) +
			    data_length(&sendtargets);
			end_of_data =
			    data + unused_length(&sendtargets);
			timeout = msecs_until(&connection_timer);

			if (iscsi_io_recv_pdu(&session->conn[0],
			     pdu, ISCSI_DIGEST_NONE, data,
			     end_of_data - data, ISCSI_DIGEST_NONE,
			     timeout)) {
				/*
				 * process iSCSI PDU received
				 */
				rc = process_recvd_pdu(
					pdu,
					config,
					session,
					&sendtargets,
					info,
					&lun_inventory_changed,
					default_port,
					&active,
					&long_lived,
					&async_timer,
					data);
				if (rc == DISCOVERY_NEED_RECONNECT)
					goto reconnect;

				/* reset timers after receiving a PDU */
				if (long_lived) {
					clear_timer(&connection_timer);
				} else {
					if (active)
						set_timer
						    (&connection_timer,
						     session->conn[0].
						     active_timeout);
					else
						/*
						 * 3 minutes to try
						 * to go long-lived
						 */
						set_timer(&connection_timer,
							  3 * 60);
				}
			} else {
				if (long_lived) {
					log_debug(1,
					       "discovery session to  "
					       "%s:%d failed to recv a "
					       "PDU response, "
					       "reconnecting",
					       config->address,
					       config->port);
					goto reconnect;
				} else {
					log_debug(1,
					       "discovery session to "
					       "%s:%d failed to recv a "
					       "PDU response, "
					       "terminating",
					       config->address,
					       config->port);
					iscsi_io_disconnect(&session->conn[0]);
					return 1;
				}
			}
		}
		if (pfd.revents & POLLHUP) {
			if (long_lived) {
				log_warning(
				       "discovery session to  %s:%d "
				       "reconnecting after hangup",
				       config->address, config->port);
				goto reconnect;
			} else {
				log_warning(
				       "discovery session to %s:%d "
				       "terminating after hangup",
				       config->address, config->port);
				iscsi_io_disconnect(&session->conn[0]);
				return 1;
			}
		}

		if (pfd.revents & POLLNVAL) {
			log_warning("discovery POLLNVAL");
			sleep(1);
			goto reconnect;
		}

		if (pfd.revents & POLLERR) {
			log_warning("discovery POLLERR");
			sleep(1);
			goto reconnect;
		}
	} else if (rc < 0) {
		if (errno == EINTR) {
			/* if we got SIGHUP, reconnect and rediscover */
			if (rediscover) {
				rediscover = 0;
				log_debug(1, "rediscovery requested");
				goto reconnect;
			}
		} else {
			log_error("poll error");
			return 1;
		}
	}

	log_debug(1, "discovery process to %s:%d exiting",
		 config->address, config->port);

	return 0;
}

#ifdef SLP_ENABLE
int
slp_discovery(struct iscsi_slp_config *config)
{
	struct sigaction action;
	char *pl;
	unsigned short flag = 0;

	memset(&action, 0, sizeof (struct sigaction));
	action.sa_sigaction = NULL;
	action.sa_flags = 0;
	action.sa_handler = SIG_DFL;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGPIPE, &action, NULL);

	action.sa_handler = sighup_handler;
	sigaction(SIGHUP, &action, NULL);

	if (iscsi_process_should_exit()) {
		log_debug(1, "slp discovery process %p exiting", discovery);
		exit(0);
	}

	discovery->pid = getpid();

	pl = generate_predicate_list(discovery, &flag);

	while (1) {
		if (flag == SLP_MULTICAST_ENABLED) {
			discovery->flag = SLP_MULTICAST_ENABLED;
			slp_multicast_srv_query(discovery, pl, GENERIC_QUERY);
		}

		if (flag == SLP_UNICAST_ENABLED) {
			discovery->flag = SLP_UNICAST_ENABLED;
			slp_unicast_srv_query(discovery, pl, GENERIC_QUERY);
		}

		sleep(config->poll_interval);
	}

	exit(0);
}

#endif
