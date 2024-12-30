PROG = r5check

LDFLAGS += -g
LDFLAGS += -static

LIBS += -lm

CFLAGS += -g
# CFLAGS += -O4		# False error on compile gcc 4.4.1
# CFLAGS += -O3		# False error on compile gcc 4.4.1
CFLAGS += -O2
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wshadow
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-qual
CFLAGS += -Winline
CFLAGS += -Waggregate-return
#-- CFLAGS += -Wcast-align

CFLAGS += -Wno-unused-parameter
CFLAGS += -Wundef
CFLAGS += -Wendif-labels

CFLAGS += -Wbad-function-cast
CFLAGS += -Wwrite-strings
CFLAGS += -Wconversion
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wnested-externs
CFLAGS += -Wunreachable-code
# CFLAGS += -Wlong-long

#-----------------------------------------------------------------------------
all:	${PROG}
#-----------------------------------------------------------------------------
${PROG}: ${PROG}.o
	${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${PROG}.o ${LIBS}
#-----------------------------------------------------------------------------
clean:
	rm -f *.o ${PROG}
#-----------------------------------------------------------------------------
test:	${PROG} Makefile
	root ./${PROG} /dev/sdb 1G 8589934589
#-----------------------------------------------------------------------------
tags:
	ctags *.c
