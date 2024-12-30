/*
 * iSCSI Administrator Utility Socket Interface
 *
 * Copyright (C) 2004 Dmitry Yusupov, Alex Aizman
 * maintained by open-iscsi@googlegroups.com
 *
 * Originally based on:
 * (C) 2004 FUJITA Tomonori <tomof@acm.org>
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
#include <errno.h>
#include <unistd.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdio.h>

#include "iscsid.h"
#include "idbm.h"
#include "mgmt_ipc.h"
#include "iscsi_ipc.h"
#include "log.h"
#include "strings.h"

#define PEERUSER_MAX	64
extern int sendtargets_discovery(struct mgmt_ipc_db *dbt, struct iscsi_sendtargets_config *config,
                                 struct string_buffer *info);
static mgmt_ipc_err_e mgmt_ipc_target_add(struct mgmt_ipc_db *dbt, queue_task_t *qtask, 
                                           iscsiadm_req_t req, iscsiadm_rsp_t *rsp);
static mgmt_ipc_err_e mgmt_ipc_x_target_remove(struct mgmt_ipc_db *dbt, queue_task_t *qtask, 
                                               iscsiadm_req_t req, iscsiadm_rsp_t *rsp);
int find_record_id(struct mgmt_ipc_db *dbt, char *target_name, int pgtag);
int
mgmt_ipc_listen(void)
{
	int fd, err;
	struct sockaddr_un addr;

	fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		log_error("can not create IPC socket");
		return fd;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	memcpy((char *) &addr.sun_path + 1, ISCSIADM_NAMESPACE,
		strlen(ISCSIADM_NAMESPACE));

	if ((err = bind(fd, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
		log_error("can not bind IPC socket");
		close(fd);
		return err;
	}

	if ((err = listen(fd, 32)) < 0) {
		log_error("can not listen IPC socket");
		close(fd);
		return err;
	}

	log_debug(1, "IPC socket is listening...");

	return fd;
}

void
mgmt_ipc_close(int fd)
{
}

static mgmt_ipc_err_e
mgmt_ipc_node_read(struct mgmt_ipc_db *dbt, int rid, node_rec_t *rec)
{
	idbm_t *db = NULL;

	if (dbt->init) {
		db = dbt->init(dconfig->config_file);
		if (!db) {
			return MGMT_IPC_ERR_IDBM_FAILURE;
		}
	}

	if (dbt->node_read(db, rid, rec)) {
		log_error("node record [%06x] not found!", rid);
		return MGMT_IPC_ERR_NOT_FOUND;
	}

	if (dbt->terminate)
		dbt->terminate(db);
	return 0;
}


int int_to_string(int num, char *string)
{
    int i=0;
    int length=0;
    char *p=string;

    if(num == 0)
    {
        *string = '0';
        return 1;
    }

    while(num > 0)
    {
        i=num%10;
        num=num/10;
        *string++='0'+i;
        length++;
    }
    *string--=0;
    /*
     *reverse the string
     */
    while(p<string)
    {
        i=*p;
        *p = *string;
        *string=i;
        string--;
        p++;
    }
    return length;

}
int ip_to_string(unsigned int ip, char *string)
{
    int byte = ip & 0x000000ff ;
    int len;
    int i = 0;

    while(i < 4)
    {
        byte = (ip >> i*8) & 0x000000ff;
        len = int_to_string(byte, string);
        string = string + len;
        *string++ = '.';
        i++;
    }
    string--;
    *string = 0;
    return 1;
}



static mgmt_ipc_err_e mgmt_ipc_x_target_remove(struct mgmt_ipc_db *dbt, queue_task_t *qtask, iscsiadm_req_t req, iscsiadm_rsp_t *rsp)
{
    idbm_t *db = NULL;
    int rec_id=-1;
    node_rec_t rec;
    int err = MGMT_IPC_OK;

	if (dbt->init) {
		db = dbt->init(dconfig->config_file);
    }
    if(db == NULL)
	{
	    log_error("mgmt_ipc_target_remove db pointer is null\n");
	    return MGMT_IPC_ERR_INTERNAL;
	}
    
    rec_id = idbm_find_node(db, (char *)req.u.target.name, req.u.target.pgtag);
	if (rec_id != -1 && !dbt->node_read(db, rec_id, &rec))
    {
         if(idbm_delete_node(db, &rec) < 0)
         {
             log_error("NODE not deleted");
             err =  MGMT_IPC_ERR_RECORD_NOT_DELETED;
         } 
    }else
    {
        log_error("NODE not found");
    }
	if (dbt->terminate)
		dbt->terminate(db);
    return err;
}
/*
* This function adds a target record in database. The target record is added in discovery as well as node database.
*
*/
static mgmt_ipc_err_e mgmt_ipc_target_add(struct mgmt_ipc_db *dbt, queue_task_t *qtask, iscsiadm_req_t req, iscsiadm_rsp_t *rsp)
{
    char address[NI_MAXHOST]; /*aaa.bbb.ccc.ddd*/
	char buffer[1024]; /*enough space for a target information*/
    idbm_t *db = NULL;
	discovery_rec_t *drec;
	mgmt_ipc_err_e retval = MGMT_IPC_OK;
        int rec_id = -1;  
        node_rec_t rec;

    memset(address, 0, NI_MAXHOST);
	if (dbt->init) {
		db = dbt->init(dconfig->config_file);
    }

	if(db == NULL)
	{
	    log_error("mgmt_ipc_target_add db pointer is null\n");
	    return MGMT_IPC_ERR_INTERNAL;
	}				
    /*
	** convert uint32 ip to string
	*/
    ip_to_string(req.u.target.ip, address);
    

    /*
	** create the buffer which is accepted by idbm_new_discovery
    ** DTN=iqn.2001-04.com.example:storage.disk2.sys1.xyz
	 * TT=1
	 * TP=3260
	 * TA=10.16.16.227
	 * ;
	 * !
    */
         rec_id = idbm_find_node(db, (char *)req.u.target.name, req.u.target.pgtag);
         if (rec_id != -1 && !dbt->node_read(db, rec_id, &rec)) 
         {
             /*
             **  if record exist and ip and port does not match then update in the database
             */
             if((strncmp(rec.conn[0].address, address, NI_MAXHOST) == 0) && (rec.conn[0].port == req.u.target.port))
             {
                 if (dbt->terminate)
                     dbt->terminate(db);
                 return MGMT_IPC_OK;
             } 
             else if(idbm_delete_node(db, &rec) < 0)
             {
                 log_error("NODE not deleted");
                 if (dbt->terminate)
                    dbt->terminate(db);
                 return MGMT_IPC_ERR_RECORD_NOT_DELETED;
             }
         }
    /*
    ** record does not exist.. we can add it in db
    */

	sprintf(buffer, "DTN=%s\nTT=%d\nTP=%d\nTA=%s\n;\n!", req.u.target.name, req.u.target.pgtag, req.u.target.port, address);
	if ((drec = idbm_new_discovery(db, address, req.u.target.port,
					DISCOVERY_TYPE_SENDTARGETS, buffer))) {
		idbm_print_nodes(db, drec);
		free(drec);
		retval = 0;
	}
    log_debug(1, "target %s tag %d ip %s port %d added in db", req.u.target.name, req.u.target.pgtag, address, req.u.target.port);
 	
 if (dbt->terminate)
     dbt->terminate(db);
 return retval;
}
static mgmt_ipc_err_e
mgmt_ipc_session_login(struct mgmt_ipc_db *dbt, queue_task_t *qtask, int rid)
{
	mgmt_ipc_err_e rc;
	node_rec_t rec;

	if ((rc = mgmt_ipc_node_read(dbt, rid, &rec)))
		return rc;
	return session_login_task(&rec, qtask);
}
/*
 ** find the record id from the target name and pgtag
 */
int find_record_id(struct mgmt_ipc_db *dbt, char *target_name, int pgtag)
{
	idbm_t *db=NULL;
	int rec_id;
	if (dbt->init) {
		db = dbt->init(dconfig->config_file);
    }
	if(db == NULL)
	{
		log_error("mgmt_ipc_x_session_login db pointer is null\n");
		return -1;
	}				
	rec_id = idbm_find_node(db, target_name, pgtag);
	if(rec_id == -1)
	{
		log_error("find_record_id no record target=%s pgtag=%d\n",
				target_name, pgtag);			
	}
    if (dbt->terminate)
        dbt->terminate(db);
	return rec_id;
}

/*
** This function does login to the given target.
** The information provided here is target name and portal group tag.
** With this information a record in database is found and existing functions are used to do login.
*/
static mgmt_ipc_err_e
mgmt_ipc_x_session_login(struct mgmt_ipc_db *dbt, queue_task_t *qtask, iscsiadm_req_t req, iscsiadm_rsp_t *rsp)
{
   int rec_id;
   int rc;
   node_rec_t rec;
   iscsi_session_t *session;
   rec_id = find_record_id(dbt, req.u.o_session.target_name, req.u.o_session.pgtag);
   if(rec_id == -1)
   {
        return MGMT_IPC_ERR_INTERNAL;
   } 
   if ((rc = mgmt_ipc_node_read(dbt, rec_id, &rec)))
   {
        return rc;
   }
   log_debug(1, "trying login  i_name %s t_name %s pgt %d \n",
                         req.u.o_session.initiator_name, req.u.o_session.target_name, req.u.o_session.pgtag);
   if (!(session = session_find_by_rec(&rec, req.u.o_session.initiator_name))) {
   	   return mgmt_ipc_session_login(dbt, qtask, rec_id);
   }
   log_error("session i_name %s t_name %s pgt %d exist ============== first logout\n",
              req.u.o_session.initiator_name, req.u.o_session.target_name, req.u.o_session.pgtag);
    return MGMT_IPC_ERR_EXISTS;
}
static mgmt_ipc_err_e mgmt_ipc_x_discovery(struct mgmt_ipc_db *dbt, queue_task_t *qtask, discovery_info dinfo, iscsiadm_rsp_t *rsp)
{
	int rc = 0;
	struct string_buffer info;
    struct iscsi_sendtargets_config cfg;
	idbm_t *db = NULL;

	if (dbt->init) {
		db = dbt->init(dconfig->config_file);
    }
	if(db == NULL)
	{
	    log_error("mgmt_ipc_x_discovery db pointer is null\n");
	    return MGMT_IPC_ERR_INTERNAL;
	}				

	idbm_sendtargets_defaults(db, &cfg);
    /*
	** convert the discovery info into sendtarget config information
	** 
	*/
    inet_ntop(AF_INET, &(dinfo.target_ip), cfg.address, 16);

	cfg.port = dinfo.port;
	cfg.continuous = 0;
	cfg.auth.username_in[0]=0;
	cfg.auth.password_in_length = 0;
        strcpy(cfg.initiator_name, (char *)dinfo.initiator_name);
	log_error("mgmt_ipc_x_session_login target_ip = %x string=%s , initiator=%s\n", dinfo.target_ip, cfg.address, cfg.initiator_name);


	init_string_buffer(&info, 8 * 1024);

	rc =  sendtargets_discovery(dbt, &cfg, &info);
	if (!rc) {
		discovery_rec_t *drec;
		if ((drec = x_idbm_new_discovery(db, cfg.address, cfg.port,
		    DISCOVERY_TYPE_SENDTARGETS, info.buffer, rsp))) {
			//idbm_print_nodes(db, drec);
			log_error("x_idbm_new_discovery returns %d\n", rsp->u.targetlist.cnt);
			free(drec);
		}
	}
	truncate_buffer(&info, 0);
    if (dbt->terminate)
        dbt->terminate(db);
	return MGMT_IPC_OK;

}
static mgmt_ipc_err_e
mgmt_ipc_session_activelist(queue_task_t *qtask, iscsiadm_rsp_t *rsp)
{
	iscsi_session_t *session;
	struct qelem *item;
	int i;

	rsp->u.activelist.cnt = 0;
	for (i = 0; i < num_providers; i++) {
		item = provider[i].sessions.q_forw;
		while (item != &provider[i].sessions) {
			session = (iscsi_session_t *)item;
			rsp->u.activelist.rids[rsp->u.activelist.cnt]= session->nrec.id;
			rsp->u.activelist.sids[rsp->u.activelist.cnt]= session->id;
			rsp->u.activelist.cnt++;
			item = item->q_forw;
		}
	}

	return MGMT_IPC_OK;
}

static mgmt_ipc_err_e
mgmt_ipc_session_getstats(queue_task_t *qtask, int rid, int sid,
		iscsiadm_rsp_t *rsp)
{
	iscsi_session_t *session;
	struct qelem *item;
	int i;

	for (i = 0; i < num_providers; i++) {
		item = provider[i].sessions.q_forw;
		while (item != &provider[i].sessions) {
			session = (iscsi_session_t *)item;
			if (session->id == sid) {
				int rc;

				rc = ipc->get_stats(session->transport_handle,
					session->id, session->conn[0].id,
					(void*)&rsp->u.getstats,
					MGMT_IPC_GETSTATS_BUF_MAX);
				if (rc) {
					log_error("get_stats(): IPC error %d "
						"session [%02d:%06x]", rc, sid, rid);
					return MGMT_IPC_ERR_INTERNAL;
				}
				return MGMT_IPC_OK;
			}
			item = item->q_forw;
		}
	}

	return MGMT_IPC_ERR_NOT_FOUND;
}

static mgmt_ipc_err_e
mgmt_ipc_session_logout(struct mgmt_ipc_db *dbt, queue_task_t *qtask, int rid, char *initiator_name)
{
	mgmt_ipc_err_e rc;
	node_rec_t rec;
	iscsi_session_t *session;

	if ((rc = mgmt_ipc_node_read(dbt, rid, &rec)))
		return rc;

	if (!(session = session_find_by_rec(&rec, initiator_name))) {
		log_error("session with corresponding record [%06x] "
			  "not found!", rid);
		return MGMT_IPC_ERR_NOT_FOUND;
	}

	return session_logout_task(session, qtask);
}

static mgmt_ipc_err_e
mgmt_ipc_session_sync(struct mgmt_ipc_db *dbt, queue_task_t *qtask, int rid,
		      int sid)
{
	mgmt_ipc_err_e rc;
	node_rec_t rec;

	if ((rc = mgmt_ipc_node_read(dbt, rid, &rec)))
		return rc;
	return iscsi_sync_session(&rec, qtask, sid);
}

static mgmt_ipc_err_e
mgmt_ipc_cfg_initiatorname(queue_task_t *qtask, iscsiadm_rsp_t *rsp)
{
	strcpy(rsp->u.config.var, dconfig->initiator_name);

	return MGMT_IPC_OK;
}

static mgmt_ipc_err_e
mgmt_ipc_cfg_initiatoralias(queue_task_t *qtask, iscsiadm_rsp_t *rsp)
{
//	strcpy(rsp->u.config.var, dconfig->initiator_alias);
	strcpy(rsp->u.config.var, "No unique initiator_alias. ");

	return MGMT_IPC_OK;
}

static mgmt_ipc_err_e
mgmt_ipc_cfg_filename(queue_task_t *qtask, iscsiadm_rsp_t *rsp)
{
	strcpy(rsp->u.config.var, dconfig->config_file);

	return MGMT_IPC_OK;
}

static mgmt_ipc_err_e
mgmt_ipc_conn_add(queue_task_t *qtask, int rid, int cid)
{
	return MGMT_IPC_ERR;
}

static mgmt_ipc_err_e
mgmt_ipc_conn_remove(queue_task_t *qtask, int rid, int cid)
{
	return MGMT_IPC_ERR;
}

static int
mgmt_peeruser(int sock, char *user)
{
#if defined(SO_PEERCRED)
	/* Linux style: use getsockopt(SO_PEERCRED) */
	struct ucred peercred;
	socklen_t so_len = sizeof(peercred);
	struct passwd *pass;

	errno = 0;
	if (getsockopt(sock, SOL_SOCKET, SO_PEERCRED, &peercred,
		&so_len) != 0 || so_len != sizeof(peercred)) {
		/* We didn't get a valid credentials struct. */
		log_error("peeruser_unux: error receiving credentials: %m");
		return 0;
	}

	pass = getpwuid(peercred.uid);
	if (pass == NULL) {
		log_error("peeruser_unix: unknown local user with uid %d",
				(int) peercred.uid);
		return 0;
	}

	strncpy(user, pass->pw_name, PEERUSER_MAX);
	return 1;

#elif defined(SCM_CREDS)
	struct msghdr msg;
	typedef struct cmsgcred Cred;
#define cruid cmcred_uid
	Cred *cred;

	/* Compute size without padding */
	/* for NetBSD */
	char cmsgmem[_ALIGN(sizeof(struct cmsghdr)) + _ALIGN(sizeof(Cred))];

	/* Point to start of first structure */
	struct cmsghdr *cmsg = (struct cmsghdr *) cmsgmem;

	struct iovec iov;
	char buf;
	struct passwd *pw;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = (char *) cmsg;
	msg.msg_controllen = sizeof(cmsgmem);
	memset(cmsg, 0, sizeof(cmsgmem));

	/*
	 * The one character which is received here is not meaningful; its
	 * purposes is only to make sure that recvmsg() blocks long enough for
	 * the other side to send its credentials.
	 */
	iov.iov_base = &buf;
	iov.iov_len = 1;

	if (recvmsg(sock, &msg, 0) < 0 || cmsg->cmsg_len < sizeof(cmsgmem) ||
			cmsg->cmsg_type != SCM_CREDS) {
		log_error("ident_unix: error receiving credentials: %m");
		return 0;
	}

	cred = (Cred *) CMSG_DATA(cmsg);

	pw = getpwuid(cred->cruid);
	if (pw == NULL) {
		log_error("ident_unix: unknown local user with uid %d",
				(int) cred->cruid);
		return 0;
	}

	strncpy(user, pw->pw_name, PEERUSER_MAX);
	return 1;

#else
	log_error("'mgmg_ipc' auth is not supported on local connections "
		"on this platform");
	return 0;
#endif
}

static int
mgmt_ipc_handle(struct mgmt_ipc_db *dbt, int accept_fd)
{
	struct sockaddr addr;
	int fd, rc = 0, immrsp = 0;
	iscsiadm_req_t req;
	iscsiadm_rsp_t rsp;
	queue_task_t *qtask = NULL;
	char user[PEERUSER_MAX];
	socklen_t len;

	memset(&rsp, 0, sizeof(rsp));
	len = sizeof(addr);
	if ((fd = accept(accept_fd, (struct sockaddr *) &addr, &len)) < 0) {
		if (errno == EINTR)
			rc = -EINTR;
		else
			rc = -EIO;
		return rc;
	}
	if (!mgmt_peeruser(accept_fd, user) ||
	    strncmp(user, "root", PEERUSER_MAX)) {
		rsp.err = MGMT_IPC_ERR_ACCESS;
		log_error("user is not root:\n");
		rc = EINVAL;
		goto err;
	}

	if (read(fd, &req, sizeof(req)) != sizeof(req)) {
		rc = -EIO;
		close(fd);
		return rc;
	}
	rsp.command = req.command;

	qtask = calloc(1, sizeof(queue_task_t));
	if (!qtask) {
		rsp.err = MGMT_IPC_ERR_NOMEM;
		rc = -ENOMEM;
		goto err;
	}
	memcpy(&qtask->u.login.req, &req, sizeof(iscsiadm_req_t));
	qtask->u.login.mgmt_ipc_fd = fd;

	switch(req.command) {

	case X_MGMT_IPC_TARGET_ADD:
		rsp.err = mgmt_ipc_target_add(dbt, qtask, req, &rsp);
                immrsp = 1;
		break;
        
	case X_MGMT_IPC_SESSION_LOGIN:
		log_debug(2, "X_MGMT_IPC_SESSION_LOGIN: i_name = %s t_name = %s \n",
                 req.u.o_session.initiator_name, req.u.o_session.target_name);
		rsp.err = mgmt_ipc_x_session_login(dbt, qtask, req, &rsp);
		break;
	case X_MGMT_IPC_SESSION_CLOSE:
		{
			int rec_id;
			/* 
			 ** find the record id from given target name and pgtag
			 **/
			rec_id = find_record_id(dbt, req.u.o_session.target_name, req.u.o_session.pgtag);
			if(rec_id == -1)
			{  
				rsp.err = MGMT_IPC_ERR_NOT_FOUND;
			}else
			{
				rsp.err = mgmt_ipc_session_logout(dbt, qtask, rec_id, req.u.o_session.initiator_name);
			}

			break;
		}

	case X_MGMT_IPC_DISCOVERY:
		rsp.err = mgmt_ipc_x_discovery(dbt, qtask, req.u.discovery, &rsp);
             immrsp = 1;

	break;

	case X_MGMT_IPC_TARGET_REMOVE:
                rsp.err = mgmt_ipc_x_target_remove(dbt, qtask, req, &rsp);
                immrsp = 1; 
	    break;
	case MGMT_IPC_SESSION_LOGIN:
		rsp.err = mgmt_ipc_session_login(dbt, qtask, req.u.session.rid);
		break;
	case MGMT_IPC_SESSION_LOGOUT:
		rsp.err = mgmt_ipc_session_logout(dbt, qtask, req.u.session.rid, dconfig->initiator_name);
		break;
	case MGMT_IPC_SESSION_SYNC:
		rsp.err = mgmt_ipc_session_sync(dbt, qtask, req.u.session.rid,
						req.u.session.sid);
		break;
	case MGMT_IPC_SESSION_ACTIVELIST:
		rsp.err = mgmt_ipc_session_activelist(qtask, &rsp);
		immrsp = 1;
		break;
	case MGMT_IPC_SESSION_STATS:
		rsp.err = mgmt_ipc_session_getstats(qtask, req.u.session.rid,
				req.u.session.sid, &rsp);
		immrsp = 1;
		break;
	case MGMT_IPC_CONN_ADD:
		rsp.err = mgmt_ipc_conn_add(qtask, req.u.conn.rid,
					    req.u.conn.cid);
		break;
	case MGMT_IPC_CONN_REMOVE:
		rsp.err = mgmt_ipc_conn_remove(qtask, req.u.conn.rid,
					       req.u.conn.cid);
		break;
	case MGMT_IPC_CONFIG_INAME:
		rsp.err = mgmt_ipc_cfg_initiatorname(qtask, &rsp);
		immrsp = 1;
		break;
	case MGMT_IPC_CONFIG_IALIAS:
		rsp.err = mgmt_ipc_cfg_initiatoralias(qtask, &rsp);
		immrsp = 1;
		break;
	case MGMT_IPC_CONFIG_FILE:
		rsp.err = mgmt_ipc_cfg_filename(qtask, &rsp);
		immrsp = 1;
		break;
	case MGMT_IPC_IMMEDIATE_STOP:
		rsp.err = MGMT_IPC_OK;
		immrsp = 1;
		rc = 1;
		break;
	default:
		log_error("unknown request: %s(%d) %u",
			  __FUNCTION__, __LINE__, req.command);
		break;
	}

	if (rsp.err == MGMT_IPC_OK && !immrsp)
		return 0;

err:
    
	if (write(fd, &rsp, sizeof(rsp)) != sizeof(rsp))
    {
	    log_error("sending response failed \n");
        rc = -EIO;
    }
	close(fd);
	if (qtask)
		free(qtask);
	return rc;
}
void  send_msg2_proc(struct async_msg *msg2proc)
{
	int fd, err;
	struct sockaddr_in addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		log_error("can not create IPC socket for msg to proc!");
		return ;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; 
	addr.sin_port        = htons(3261);

	if ((err = connect(fd, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
		log_error("can not connect to xiotech PROC!");
		close(fd);
		return;
	}

	if ((err = write(fd, msg2proc, sizeof(*msg2proc))) != sizeof(*msg2proc)) {
		log_error("iscsid: write error (%d) on async msg %d", err, msg2proc->event_type);
	}
	/*
	** close the socket
	*/
	close(fd);
}
static int reap_count;

void
need_reap(void)
{
	reap_count++;
}

static void
reaper(void)
{
	int rc;

	/*
	 * We don't really need reap_count, but calling wait() all the
	 * time seems execessive.
	 */
	if (reap_count) {
		rc = waitpid(0, NULL, WNOHANG);
		if (rc > 0) {
			reap_count--;
			log_debug(6, "reaped pid %d, reap_count now %d",
				  rc, reap_count);
		}
	}
}

#define POLL_CTRL	0
#define POLL_IPC	1
#define POLL_MAX	2

/* TODO: this should go somewhere else */
void event_loop(struct iscsi_ipc *ipc, int control_fd, int mgmt_ipc_fd,
		struct mgmt_ipc_db *dbt)
{
	struct pollfd poll_array[POLL_MAX];
	int res;

	poll_array[POLL_CTRL].fd = control_fd;
	poll_array[POLL_CTRL].events = POLLIN;
	poll_array[POLL_IPC].fd = mgmt_ipc_fd;
	poll_array[POLL_IPC].events = POLLIN;

	while (1) {
		res = poll(poll_array, POLL_MAX, ACTOR_RESOLUTION);
		if (res > 0) {
			log_debug(6, "poll result %d", res);
			if (poll_array[POLL_CTRL].revents)
				ipc->ctldev_handle();

			if (poll_array[POLL_IPC].revents)
				if (mgmt_ipc_handle(dbt, mgmt_ipc_fd) == 1)
					break;
		} else if (res < 0) {
			if (errno == EINTR) {
				log_debug(1, "event_loop interrupted");
			} else {
				log_error("got poll() error (%d), errno (%d), "
					  "exiting", res, errno);
				break;
			}
		} 
        /*
         ** actor_poll should be called in all cases whenever poll returns.
         ** otherwise some async requests could get starved if there is continous request from PROC.
         */
		actor_poll();
		reaper();
	}
}
