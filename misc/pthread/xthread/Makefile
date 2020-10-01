PROG := xtest

FILES := dbg_xprintf.c multi-reader.c xthread.c xthread_memory.c

CFLAGS += -pthread

CFLAGS += -I/home/m4/nextgen/include
CFLAGS += -I/home/m4/nextgen/odin

OBJS := ${FILES:.c=.o}

all: ${PROG}
	${CC} -o ${PROG} ${OBJS} -lpthread

${PROG}: ${OBJS}

clean:
	rm -f *.o ${PROG} core.[0-9]*
