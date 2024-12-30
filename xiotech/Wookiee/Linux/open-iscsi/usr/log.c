/*
 * Copyright (C) 2002-2003 Ardis Technolgies <roman@ardistech.com>
 *
 * Released under the terms of the GNU GPL v2.0.
 */

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<errno.h>

#include "util.h"
#include "log.h"

#define SEMKEY	0xA7L
#define LOGDBG 1

#if LOGDBG
#define logdbg(file, fmt, args...) fprintf(file, fmt, ##args)
#else
#define logdbg(file, fmt, args...) do {} while (0)
#endif

char *log_name;
int log_daemon = 1;
int log_level = 0;

static int log_stop_daemon = 0;

static int logarea_init (int size)
{
	int shmid;

	logdbg(stderr,"enter logarea_init\n");

	if ((shmid = shmget(IPC_PRIVATE, sizeof(struct logarea),
			    0644 | IPC_CREAT | IPC_EXCL)) == -1)
		return 1;

	la = shmat(shmid, NULL, 0);
	if (!la)
		return 1;

	if (size < MAX_MSG_SIZE)
		size = DEFAULT_AREA_SIZE;

	if ((shmid = shmget(IPC_PRIVATE, size,
			    0644 | IPC_CREAT | IPC_EXCL)) == -1) {
		shmdt(la);
		return 1;
	}

	la->start = shmat(shmid, NULL, 0);
	if (!la->start) {
		shmdt(la);
		return 1;
	}
	memset(la->start, 0, size);

	la->empty = 1;
	la->end = la->start + size;
	la->head = la->start;
	la->tail = la->start;

	if ((shmid = shmget(IPC_PRIVATE, MAX_MSG_SIZE + sizeof(struct logmsg),
			    0644 | IPC_CREAT | IPC_EXCL)) == -1) {
		shmdt(la->start);
		shmdt(la);
		return 1;
	}
	la->buff = shmat(shmid, NULL, 0);
	if (!la->buff) {
		shmdt(la->start);
		shmdt(la);
		return 1;
	}

	if ((la->semid = semget(SEMKEY, 1, 0666 | IPC_CREAT)) < 0) {
		shmdt(la->buff);
		shmdt(la->start);
		shmdt(la);
		return 1;
	}

	la->semarg.val=1;
	if (semctl(la->semid, 0, SETVAL, la->semarg) < 0) {
		shmdt(la->buff);
		shmdt(la->start);
		shmdt(la);
		return 1;
	}

	return 0;

}

static void free_logarea (void)
{
	semctl(la->semid, 0, IPC_RMID, la->semarg);
	shmdt(la->buff);
	shmdt(la->start);
	shmdt(la);
	return;
}


int log_enqueue (int prio, const char * fmt, va_list ap)
{
	int len, fwd;
	char buff[MAX_MSG_SIZE];
	struct logmsg * msg;
	struct logmsg * lastmsg;

	/* print to a local buffer */
	len = vsnprintf(buff, MAX_MSG_SIZE, fmt, ap);
	if (len <= 0)
		return len;
	else if (len < MAX_MSG_SIZE) {
		len++; /* account for trailing NUL in the memcpys */
	}
	else  {
		len = MAX_MSG_SIZE; /* truncate */
		buff[MAX_MSG_SIZE - 1] = '\0'; 
	}
	
	/* la->tail is the last logmsg */
	lastmsg = (struct logmsg *)la->tail;

	/* if non-empty, move the tail past the last message in the buffer */
	if (!la->empty) {
		fwd = sizeof(struct logmsg) +
		      strlen((char *)&lastmsg->str) * sizeof(char) + 1;
		la->tail += fwd;
	}

	/* not enough space on tail : rewind */
	if (la->head <= la->tail &&
	    (len + sizeof(struct logmsg)) > (la->end - la->tail)) {
		logdbg(stderr, "enqueue: rewind tail to %p\n", la->tail);
		la->tail = la->start;
	}

	/* not enough space on head : drop msg */
	if (la->head > la->tail &&
	    (len + sizeof(struct logmsg)) > (la->head - la->tail)) {
		logdbg(stderr, "enqueue: log area overrun, drop msg\n");

		if (!la->empty)
			la->tail = lastmsg;

		return 1;
	}

	/* ok, we can stage the msg in the area */
	la->empty = 0;
	msg = (struct logmsg *)la->tail;
	msg->prio = prio;
	msg->length = len + sizeof(struct logmsg);
	memcpy((void *)&msg->str, buff, len);
	lastmsg->next = la->tail;
	msg->next = la->head;

	logdbg(stderr, "enqueue: %p, %p, %i, %s\n", (void *)msg, msg->next,
		msg->prio, (char *)&msg->str);

	return 0;
}

int log_dequeue (void * buff)
{
	struct logmsg * src = (struct logmsg *)la->head;
	struct logmsg * dst = (struct logmsg *)buff;
	struct logmsg * lst = (struct logmsg *)la->tail;
	int len;

	if (la->empty)
		return 1;

	len = src->length;
	dst->prio = src->prio;
	memcpy(dst, src,  len);

	if (la->tail == la->head)
		la->empty = 1; /* purge the last log msg */
	else {
		la->head = src->next;
		lst->next = la->head;
	}
	logdbg(stderr, "dequeue: %p, %p, %i, %s\n",
	       (void *)src, src->next, src->prio, (char *)&src->str);

	memset((void *)src, 0, len);

	return la->empty;
}

/*
 * this one can block under memory pressure
 */
static void log_syslog (void * buff)
{
	struct logmsg * msg = (struct logmsg *)buff;

	syslog(msg->prio, "%s", (char *)&msg->str);
}

static void dolog(int prio, const char *fmt, va_list ap)
{
	if (log_daemon) {
		struct sembuf ops[1];

		memset(&ops, 0x0, sizeof(ops));
		ops[0].sem_op = -1;
		if (semop(la->semid, ops, 1) < 0) {
			syslog(LOG_ERR, "semop down failed");
			return;
		}

		log_enqueue(prio, fmt, ap);

		ops[0].sem_op = 1;
		if (semop(la->semid, ops, 1) < 0) {
			syslog(LOG_ERR, "semop up failed");
			return;
		}
	} else {
		fprintf(stderr, "%s: ", log_name);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		fflush(stderr);
	}
}

void log_warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dolog(LOG_WARNING, fmt, ap);
	va_end(ap);
}

void log_error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dolog(LOG_ERR, fmt, ap);
	va_end(ap);
}

void log_debug(int level, const char *fmt, ...)
{
	if (log_level > level) {
		va_list ap;
		va_start(ap, fmt);
		dolog(LOG_DEBUG, fmt, ap);
		va_end(ap);
	}
}



static void log_flush(void)
{
	struct sembuf ops[1];

	memset(&ops, 0x0, sizeof(ops));
	while (!la->empty) {
		ops[0].sem_op = -1;
		if (semop(la->semid, ops, 1) < 0) {
			syslog(LOG_ERR, "semop down failed");
			exit(1);
		}
		log_dequeue(la->buff);
		ops[0].sem_op = 1;
		if (semop(la->semid, ops, 1) < 0) {
			syslog(LOG_ERR, "semop up failed");
			exit(1);
		}
		log_syslog(la->buff);
	}
}

static void catch_signal(int signo)
{
	switch (signo) {
	case SIGSEGV:
		log_flush();
		break;
	case SIGTERM:
		log_stop_daemon = 1;
		break;
	}

	log_warning("pid %d caught signal -%d", getpid(), signo);
}

static void __log_close(void)
{
	log_flush();
	closelog();
	free_logarea();
}

int log_init(char *program_name, int size)
{
	logdbg(stderr,"enter log_init\n");
	log_name = program_name;

	if (log_daemon) {
		struct sigaction sa_old;
		struct sigaction sa_new;
		pid_t pid;

		openlog(log_name, 0, LOG_DAEMON);
		setlogmask (LOG_UPTO (LOG_DEBUG));

		if (logarea_init(size))
			return -1;

		pid = fork();
		if (pid < 0) {
			syslog(LOG_ERR, "starting logger failed");
			exit(1);
		} else if (pid) {
			syslog(LOG_WARNING,
			       "iSCSI logger with pid=%d started!", pid);
			return pid;
		}

		daemon_init();

		/* flush on daemon's crash */
		sa_new.sa_handler = (void*)catch_signal;
		sigemptyset(&sa_new.sa_mask);
		sa_new.sa_flags = 0;
		sigaction(SIGSEGV, &sa_new, &sa_old );
		sigaction(SIGTERM, &sa_new, &sa_old );

		while(1) {
			log_flush();
			sleep(1);

			if (log_stop_daemon)
				break;
		}

		__log_close();
		exit(0);
	}

	return 0;
}

void log_close(pid_t pid)
{
	int status;

	if (!log_daemon || pid <= 0) {
		__log_close();
		return;
	}

	kill(pid, SIGTERM);
	waitpid(pid, &status, 0);
}
