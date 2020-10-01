PROG1	= writep2
PROG2	= readp2

# CC      = /usr/local/bin/gcc-43
CFLAGS	+= -O4
CFLAGS += -g
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Waggregate-return
# CFLAGS += -Warray-bounds
# CFLAGS += -Wattributes
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wchar-subscripts
CFLAGS += -Wcomments
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wendif-labels
CFLAGS += -Werror
# CFLAGS += -Wextra
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
CFLAGS += -Wformat-nonliteral
CFLAGS += -Wformat-security
CFLAGS += -Wimplicit-int
CFLAGS += -Wimport
CFLAGS += -Winline
# CFLAGS += -Wlogical-op
CFLAGS += -Wlong-long
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
CFLAGS += -Wnested-externs
CFLAGS += -Wno-cast-qual
# CFLAGS += -Wno-pointer-sign
CFLAGS += -Wno-system-headers
# CFLAGS += -Woverflow
CFLAGS += -Wparentheses
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-aliasing
# CFLAGS += -Wstrict-overflow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wswitch-enum
CFLAGS += -Wtrigraphs
CFLAGS += -Wundef
CFLAGS += -Wuninitialized
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
# CFLAGS += -Wvolatile-register-var
CFLAGS += -Wwrite-strings
#-----------------------------------------------------------------------------
# LDFLAGS += -g
#-----------------------------------------------------------------------------
# CFLAGS += -fmudflap
# LIB += /usr/local/lib/libmudflap.a
# LIB += /usr/lib/libc.a
# LIB += -lmudflap
#-----------------------------------------------------------------------------
#-- LDFLAGS += -static
#-----------------------------------------------------------------------------
# CFLAGS += -fprofile-arcs -ftest-coverage
#
#-----------------------------------------------------------------------------
all:	${PROG1} ${PROG2}
#-------
${PROG1}:	${PROG1}.o
	${CC} ${LDFLAGS} ${CFLAGS} ${PROG1}.o -o ${PROG1} ${LIB}
#-------
${PROG2}:	${PROG2}.o
	${CC} ${LDFLAGS} ${CFLAGS} ${PROG2}.o -o ${PROG2} ${LIB}
#-------
clean:
	rm -f *.o ${PROG1} ${PROG2}
#-----------------------------------------------------------------------------
# End of Makefile
