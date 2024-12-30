# ----------------------------------------------------------------------------
# Set locations of shared memory segments, and other mmap-ed areas.
include ../../Makefile-${MODEL}.defs
CFLAGS += ${MAKE_DEFS_CC}
# ----------------------------------------------------------------------------
CFLAGS += -I../../Shared/Inc
CFLAGS += -I../../Proc/obj_${MODEL}Debug
CFLAGS += -I../../Proc/obj_${MODEL}Perf
CFLAGS += -I../xio3d
CFLAGS += -g
CFLAGS += -DPAM -DPROC_CODE
CFLAGS += ${ON64FLAGS}
ifeq (${RUNON64},RUNON64)
CFLAGS += -DRUNON64
endif
# LDFLAGS += -Wl,-Map,$@.map
LDFLAGS += -static

all:	dumpshared zero

zero:  zero.o memorder.o
	gcc ${CFLAGS} ${LDFLAGS} -o $@ $^
zero.o: ../../Shared/Src/L_XIO3D.c

dumpshared:  dumpshared.o memorder.o
	gcc ${CFLAGS} ${LDFLAGS} -o $@ $^
dumpshared.o: ../../Shared/Src/L_XIO3D.c

clean cleandir:
	rm -f dumpshared zero *.core core.* *.o
