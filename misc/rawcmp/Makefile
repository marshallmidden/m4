PROG	= rawcmp
# CFLAGS += -O9 -W -Wall -Wtraditional -Wshadow -Wpointer-arith -Wcast-qual -Winline -g
CFLAGS += -O4
CFLAGS += -Werror
SRCS    = ${PROG}.c
#-----------------------------------------------------------------------------
LDFLAGS += -static
#-----------------------------------------------------------------------------

all:	${PROG}
${PROG}: ${PROG}.o
	${CC} ${LDFLAGS} -o ${PROG} ${PROG}.o
${PROG.o}: Makefile ${SRCS}
#-----------------------------------------------------------------------------
clean:
	rm -f ${PROG} *.o
#-----------------------------------------------------------------------------
# End of Makefile
