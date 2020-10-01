PROG	= diskpattern
NOMAN	=
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Wshadow
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-qual
CFLAGS += -Winline
CFLAGS += -O4
CFLAGS += -g

SRCS    = diskpattern.c
#-----------------------------------------------------------------------------
LDFLAGS += -static

#-----------------------------------------------------------------------------
all:	${PROG}
${PROG}: ${PROG}.o
	${CC} ${CFLAGS} -o ${PROG} ${PROG}.o

#-----------------------------------------------------------------------------
clean:
	rm -f *.o ${PROG} Z DISK.sd* DISKDRIVES RUN.DISKDRIVES

#-----------------------------------------------------------------------------
test:	${PROG}
	./${PROG} -s 30 -n 5 >Z
	wc Z
	./printblocks Z
#-----------------------------------------------------------------------------
