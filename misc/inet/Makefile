CC	= gcc
CFLAGS += -O3
CFLAGS += -W -Wall -Wshadow -Wpointer-arith -Wcast-qual -Winline
CFLAGS += -Waggregate-return -Wcast-align
LDLIBS += -lresolv
# BINDIR	= /usr/local/bin
BINDIR	= /Users/m4/bin
# --------------------------------------------------
all:	inet
# --------------------------------------------------
inet:	inet.o
	${CC} ${CFLAGS} inet.o -o inet ${LDLIBS}
# --------------------------------------------------
inet.o:	
	${CC} -c $(CFLAGS) inet.c
# --------------------------------------------------
clean:
	rm -f *.o
	rm -f inet
# --------------------------------------------------
install:	inet
	install -c -s -o root -g bin  -m 555 inet ${BINDIR}

