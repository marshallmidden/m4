/*
 * iSCSI Initiator Daemon
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
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/utsname.h>
#include <sys/signal.h>

#include "iscsid.h"
#include "mgmt_ipc.h"
#include "iscsi_ipc.h"
#include "log.h"
#include "util.h"
#include "initiator.h"
#include "transport.h"
#include "idbm.h"
#include "version.h"

char initiator_name[TARGET_NAME_MAXLEN];
char initiator_alias[TARGET_NAME_MAXLEN];
char config_file[TARGET_NAME_MAXLEN];
/* global config info */
struct iscsi_daemon_config daemon_config;
struct iscsi_daemon_config *dconfig = &daemon_config;
iscsi_provider_t *provider = NULL;
int num_providers = 0;

static char program_name[] = "iscsid";
int control_fd, mgmt_ipc_fd;

extern char sysfs_file[];

static struct mgmt_ipc_db mgmt_ipc_db = {
	.init		= idbm_init,
	.terminate	= idbm_terminate,
	.node_read	= idbm_node_read,
};

static struct option const long_options[] = {
	{"config", required_argument, NULL, 'c'},
	{"initiatorname", required_argument, NULL, 'i'},
	{"foreground", no_argument, NULL, 'f'},
	{"debug", required_argument, NULL, 'd'},
	{"uid", required_argument, NULL, 'u'},
	{"gid", required_argument, NULL, 'g'},
	{"pid", required_argument, NULL, 'p'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0},
};

static void usage(int status)
{
	if (status != 0)
		fprintf(stderr, "Try `%s --help' for more information.\n",
			program_name);
	else {
		printf("Usage: %s [OPTION]\n", program_name);
		printf("\
Open-iSCSI initiator daemon.\n\
  -c, --config=[path]     Execute in the config file (" CONFIG_FILE ").\n\
  -i, --initiatorname=[path]     read initiatorname from file (" INITIATOR_NAME_FILE ").\n\
  -f, --foreground        make the program run in the foreground\n\
  -d, --debug debuglevel  print debugging information\n\
  -u, --uid=uid           run as uid, default is current user\n\
  -g, --gid=gid           run as gid, default is current user group\n\
  -p, --pid=pidfile       use pid file (default " PID_FILE ").\n\
  -h, --help              display this help and exit\n\
  -v, --version           display version and exit\n\
");
	}
	exit(status == 0 ? 0 : -1);
}
#if 0
static int sync_session(iscsi_provider_t *provider, char *sys_session)
{
	idbm_t *db;
	int rc;
	int sid, rec_id;
	iscsiadm_req_t req;
	iscsiadm_rsp_t rsp;
	int fd;

	log_debug(7, "sync session %s", sys_session);

	db = idbm_init(daemon_config.config_file);
	if (!db) {
		log_error("could not open node database");
		return -1;
	}

	rc = idbm_find_ids_by_session(db, &sid, &rec_id, sys_session);
	if (rc < 0) {
		log_error("could not find record for %s", sys_session);
		return -1;
	}
	idbm_terminate(db);

	memset(&req, 0, sizeof(req));
	req.command = MGMT_IPC_SESSION_SYNC;
	req.u.session.rid = rec_id;
	req.u.session.sid = sid;
	return do_iscsid(&fd, &req, &rsp);
}
static void sync_sessions(iscsi_provider_t *prv)
{
	DIR *dirfd;
	struct dirent *dent;

	sprintf(sysfs_file, "/sys/class/iscsi_session");
	dirfd = opendir(sysfs_file);
	while ((dent = readdir(dirfd))) {
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
			continue;
		if (sync_session(prv, dent->d_name))
			log_error("Could not sync %s\n", dent->d_name);
	}
	closedir(dirfd);
}
/*
 * synchronize with existing sessions/connections
 */
static int sync_provider_sessions(void)
{
	int i;

	for (i = 0; i < num_providers; i++)
		if (provider[i].handle)
			sync_sessions(&provider[i]);

	return 0;
}
#endif
int logger_pid=0;
static void catch_signal(int signo) {
    if(signo == SIGINT)
    {
        if(logger_pid)
        {
            fprintf(stderr, "iscsidaemon caught signal %d\n", signo);
            log_warning("caught signal -%d,killing logger %d\n ", signo,logger_pid);
            kill(logger_pid,SIGKILL);
        }
        exit(0);
    }

	log_warning("caught signal -%d, ignoring...", signo);
}

static void iscsid_exit(void)
{
	if (num_providers > 0) {
		free(provider);
	}
}

int main(int argc, char *argv[])
{
	struct utsname host_info; /* will use to compound initiator alias */
	char *config_file = CONFIG_FILE;
	char *initiatorname_file = INITIATOR_NAME_FILE;
	char *pid_file = PID_FILE;
	int ch, longindex;
	uid_t uid = 0;
	gid_t gid = 0;
	struct sigaction sa_old;
	struct sigaction sa_new;

	/* do not allow ctrl-c for now... */
	sa_new.sa_handler = catch_signal;
	sigemptyset(&sa_new.sa_mask);
	sa_new.sa_flags = 0;
	sigaction(SIGINT, &sa_new, &sa_old );
	sigaction(SIGPIPE, &sa_new, &sa_old );
	sigaction(SIGTERM, &sa_new, &sa_old );

	while ((ch = getopt_long(argc, argv, "c:i:fd:u:g:p:vh", long_options,
				 &longindex)) >= 0) {
		switch (ch) {
		case 'c':
			config_file = optarg;
			break;
		case 'i':
			initiatorname_file = optarg;
			break;
		case 'f':
			log_daemon = 0;
			break;
		case 'd':
			log_level = atoi(optarg);
			break;
		case 'u':
			uid = strtoul(optarg, NULL, 10);
			break;
		case 'g':
			gid = strtoul(optarg, NULL, 10);
			break;
		case 'p':
			pid_file = optarg;
			break;
		case 'v':
			printf("%s version %s\n", program_name,
				ISCSI_VERSION_STR);
			exit(0);
		case 'h':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}

	/* make sure we have initiatorname config file first */
//	if (access(initiatorname_file, R_OK)) {
//		fprintf(stderr, "Error: initiatorname file doesn't exist! we will use default initiator name");
//		fprintf(stderr, " default [%s]\n", INITIATOR_NAME_FILE);
	//	usage(0);
//	}

	/* initialize logger */
    logger_pid = log_init(program_name, DEFAULT_AREA_SIZE);
#ifdef MODEL_750
        /*  turned this off so code will load on the 2.6.19 kernels*/
	check_class_version();
#endif
	if (atexit(iscsid_exit)) {
		log_error("failed to set exit function\n");
		exit(1);
	}

	if ((mgmt_ipc_fd = mgmt_ipc_listen()) < 0) {
		exit(-1);
	}

	if (log_daemon) {
		char buf[64];
		int fd;

		fd = open(pid_file, O_WRONLY|O_CREAT, 0644);
		if (fd < 0) {
			log_error("unable to create pid file");
			exit(1);
		}
        /*
         ** original daemon used to fork from here. But we need this daemon to be child of 
         ** pam so we will not fork
         */
		if ((control_fd = ipc->ctldev_open()) < 0) {
			exit(-1);
		}

		chdir("/");
		if (lockf(fd, F_TLOCK, 0) < 0) {
			log_error("unable to lock pid file");
			exit(1);
		}
		ftruncate(fd, 0);
		sprintf(buf, "%d\n", getpid());
		write(fd, buf, strlen(buf));

		daemon_init();
	} else {
		if ((control_fd = ipc->ctldev_open()) < 0) {
			exit(-1);
		}
	}

	if (uid && setuid(uid) < 0)
		perror("setuid\n");

	if (gid && setgid(gid) < 0)
		perror("setgid\n");

	memset(&daemon_config, 0, sizeof (daemon_config));
	daemon_config.pid_file = pid_file;
	daemon_config.config_file = config_file;
#if 0
	daemon_config.initiator_name_file = initiatorname_file;
	daemon_config.initiator_name =
	    get_iscsi_initiatorname(daemon_config.initiator_name_file);
	if (daemon_config.initiator_name == NULL) {
		log_warning("exiting due to configuration error");
		exit(1);
	}

	/* optional InitiatorAlias */
	daemon_config.initiator_alias =
	    get_iscsi_initiatoralias(daemon_config.initiator_name_file);
	if (!daemon_config.initiator_alias) {
		memset(&host_info, 0, sizeof (host_info));
		if (uname(&host_info) >= 0) {
			daemon_config.initiator_alias = 
				strdup(host_info.nodename);
		}
	}

	log_debug(1, "InitiatorName=%s", daemon_config.initiator_name);
	log_debug(1, "InitiatorAlias=%s", daemon_config.initiator_alias);
#endif
       /*
       ** create default initiator name using system node name if initiatorname file is not provided
	*/ 
	daemon_config.initiator_name =
	    get_iscsi_initiatorname(daemon_config.initiator_name_file);
	if(daemon_config.initiator_name == NULL)
	{
		memset(&host_info, 0, sizeof (host_info));
		if (uname(&host_info) >= 0) {
			sprintf(initiator_name,"iqn.%s-%s",host_info.sysname,host_info.nodename) ;
			daemon_config.initiator_name = initiator_name;
		}else
        {
          log_warning("exiting due to configuration error");
          exit(1);
        }
	}

	/* oom-killer will not kill us at the night... */
	if (oom_adjust())
		log_debug(1, "can not adjust oom-killer's pardon");

	/* in case of transports/sessions/connections been active
	 * and we've been killed or crashed. update states.
	 */
	if (sync_transports()) {
		log_error("failed to get transport list, exiting...");
		exit(-1);
	}

	/* we don't want our active sessions to be paged out... */
	if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
		log_error("failed to mlockall, exiting...");
		exit(1);
	}

#if 0
	pid = fork();
	if (pid == 0) {
		/* child */
		sync_provider_sessions();
		exit(0);
	} else if (pid < 0) {
		log_error("fork failed error %d: existing sessions"
			  " will not be synced", errno);
	} else {
		/* parent continues */
		log_warning("iSCSI sync pid=%d started", pid);
		need_reap();
	}
#endif
        
    log_warning("iSCSI daemon pid=%d started", getpid());
	need_reap();

	log_debug(1, "daemon started");
	actor_init();
	event_loop(ipc, control_fd, mgmt_ipc_fd, &mgmt_ipc_db);
	log_debug(1, "daemon stopping");
	return 0;
}
