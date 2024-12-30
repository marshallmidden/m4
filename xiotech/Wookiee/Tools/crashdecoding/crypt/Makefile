PROG = crypt
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
CFLAGS += -O4
CFLAGS += -fstrict-aliasing
CFLAGS += -frename-registers
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wno-system-headers
CFLAGS += -Waggregate-return
CFLAGS += -Warray-bounds			# added in gcc 4.3.1
CFLAGS += -Wattributes				# added in gcc 4.3.1
CFLAGS += -Wbad-function-cast
#-- CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wchar-subscripts
CFLAGS += -Wcomments
CFLAGS += -Wconversion
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wendif-labels
CFLAGS += -Wextra				# added in gcc 4.3.1
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
CFLAGS += -Wformat-nonliteral
CFLAGS += -Wformat-security
CFLAGS += -Wimplicit-int
CFLAGS += -Wimport
CFLAGS += -Winline
CFLAGS += -Wlong-long
CFLAGS += -Wlogical-op				# added in gcc 4.3.1
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
CFLAGS += -Wnested-externs
CFLAGS += -Woverflow				# added in gcc 4.3.1
CFLAGS += -Wpadded
CFLAGS += -Wparentheses
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-aliasing
CFLAGS += -Wstrict-overflow			# added in gcc 4.3.1
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wswitch-enum
CFLAGS += -Wtrigraphs
CFLAGS += -Wundef
CFLAGS += -Wuninitialized
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunsafe-loop-optimizations		# added in gcc 4.3.1
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
CFLAGS += -Wvolatile-register-var		# added in gcc 4.3.1
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
OBJS = Crypt.o bf_enc.o bf_skey.o
#-----------------------------------------------------------------------------
.PHONY: all
all: ${PROG}
#-----------------------------------------------------------------------------
${OBJS}: Makefile
${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS}
#-----------------------------------------------------------------------------
Crypt.o: blowfish.h
bf_enc.o: blowfish.h bf_locl.h
bf_skey.o: blowfish.h bf_locl.h bf_pi.h
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o ${PROG} tags TAGS
	${MAKE} -C test clean
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags *.c *.h
#-----------------------------------------------------------------------------
.PHONY: test
test:
	${MAKE} -C test
#-----------------------------------------------------------------------------
SPLINT	= /home/marshall_midden/bin/splint -I.

.PHONY: splint lint
lint splint:
	${SPLINT} *.c
#-----------------------------------------------------------------------------
.PHONY: install
install:
	cp crypt ../bin/
#-----------------------------------------------------------------------------
