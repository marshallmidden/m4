GCC=gcc
CFLAGS += ${ON64FLAGS}
CFLAGS += -I../../Shared/Inc
CFLAGS += -I../../CCB/Inc
CFLAGS += -g
CFLAGS += -c -g -Wall -Werror

ifeq (${RUNON64},RUNON64)
CFLAGS += -DRUNON64 -Wno-pointer-sign
endif


CCBFLAGS = -DLINUX_VER -DHYPERNODE -DCCB_RUNTIME_CODE
LDFLAGS += -Wl,-Map,$@.map

OBJDIR := obj_${MODEL}

SRV_SRC = SystemSrv.c
SRV_OBJ = ${OBJDIR}/SystemSrv.o

# ----------------------------------------------------------------------------
.PHONY: all
all:	${OBJDIR} ${OBJDIR}/syssrv

${OBJDIR}:
	mkdir -p ${OBJDIR}

${OBJDIR}/%.o : %.c Makefile
	# $@
	@${GCC} $(CFLAGS) -o $@ $<

${SRV_OBJ}: ${OBJDIR} ${SRV_SRC}

${OBJDIR}/syssrv: ${SRV_OBJ}
	# $@
	@${GCC} ${ON64FLAGS} $(LDFLAGS) -o $@ $<

# ----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf syssrv *.map *.o ${OBJDIR}
# ----------------------------------------------------------------------------
