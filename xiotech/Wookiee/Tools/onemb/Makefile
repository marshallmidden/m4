PROG = onemb

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
# Add these warnings.
CFLAGS += -Wconversion
#-- CFLAGS += -Wunreachable-code	# fflush() and sleep() yell.
# Marshall's standard additional.
CFLAGS += -Wno-system-headers
CFLAGS += -Waggregate-return
CFLAGS += -Wbad-function-cast
#-- CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wchar-subscripts
CFLAGS += -Wcomments
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wendif-labels
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
CFLAGS += -Wformat-nonliteral
CFLAGS += -Wformat-security
CFLAGS += -Wimplicit-int
CFLAGS += -Wimport
CFLAGS += -Winline
CFLAGS += -Wlong-long
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
CFLAGS += -Wnested-externs
CFLAGS += -Wparentheses
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-aliasing
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wswitch-enum
CFLAGS += -Wtrigraphs
CFLAGS += -Wundef
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
#-----------------------------------------------------------------------------
all:	${PROG}
#-----------------------------------------------------------------------------
${PROG}: ${PROG}.o
	${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${PROG}.o ${LIBS}
#-----------------------------------------------------------------------------
clean:
	rm -f *.o ${PROG} tags
#-----------------------------------------------------------------------------
test:	${PROG} Makefile
	root ./${PROG} /dev/sdb 1G 8589934589
#-----------------------------------------------------------------------------
.PHONY: tag tags ctags
tag tags ctags:
	ctags *.[hc]
# ----------------------------------------------------------------------------
.PHONY: gdb
gdb:    ${PROG} core
	gdb ${PROG} core
# ----------------------------------------------------------------------------
# vi:sw=8 ts=8 noexpandtab
# End of Makefile
# ----------------------------------------------------------------------------
