# This Makefile will work only with GNU make.

OSNAME=$(shell uname -s)

ifeq ($(OSNAME),Linux)
IPC_CFLAGS=-DNETLINK_ISCSI=8
IPC_OBJ=netlink.o
DBM_LIB=-ldb
else
ifeq ($(OSNAME),FreeBSD)
IPC_CFLAGS=
IPC_OBJ=ioctl.o
DBM_LIB=
endif
endif

OPTFLAGS ?= -O2 -g
ifeq (${RUNON64},RUNON64)
OPTFLAGS += -DRUNON64 ${ON64FLAGS} -Wno-pointer-sign
endif
WARNFLAGS ?= -Wall -Wstrict-prototypes
CFLAGS += $(OPTFLAGS) $(WARNFLAGS) -I../include -D$(OSNAME) $(IPC_CFLAGS)
PROGRAMS = iscsid iscsiadm #iscsistart

# sources shared between iscsid, iscsiadm and iscsistart
ISCSI_LIB_SRCS = util.o io.o auth.o login.o log.o md5.o sha1.o
# sources shared between iscsid and iscsiadm
COMMON_SRCS =  $(ISCSI_LIB_SRCS) idbm.o
# core initiator files
INITIATOR_SRCS = initiator.o queue.o actor.o mgmt_ipc.o transport.o

#-----------------------------------------------------------------------------
# Compile rule. I didn't like the very long line for each compile,
# so I replaced it with a simple display of the file being compiled.
%.o: %.c
	# $@
	@$(CC) ${CFLAGS} -o $@ -c $<

# ----------------------------------------------------------------------------
.PHONY: all
all: $(PROGRAMS)

iscsid: $(COMMON_SRCS) $(IPC_OBJ) $(INITIATOR_SRCS) iscsid.o \
				  discovery.o strings.o
	# $@
	@$(CC) $(CFLAGS) $^ $(DBM_LIB) -lpthread -o $@

iscsiadm: $(COMMON_SRCS) strings.o discovery.o iscsiadm.o
	# $@
	@$(CC) $(CFLAGS) $^ $(DBM_LIB) -lpthread -o $@

iscsistart: $(IPC_OBJ) $(ISCSI_LIB_SRCS) $(INITIATOR_SRCS) iscsistart.o \
		statics.o idbm.o
	# $@
	@$(CC) $(CFLAGS) -static $^ $(DBM_LIB) -o $@

.PHONY: clean
clean:
	rm -f *.o $(PROGRAMS)
# ----------------------------------------------------------------------------
