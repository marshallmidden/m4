C_DEFS  = -D_REENTRANT -DCCB_RUNTIME_CODE -DCCB_RELEASE=3 -DDISABLE_NAGLE \
	  -DNONFATAL_ASSERT -DHYPERNODE -DLINUX_VER
REALCFLAGS  = ${C_DEFS} ${INCDIRS} -g -c ${CFLG}
REALCFLAGS += -Wall
REALCFLAGS += -Werror
#+++
REALCFLAGS += -W
REALCFLAGS += -Wno-system-headers
#--- Many legitimate uses for aggregate return.  REALCFLAGS += -Waggregate-return
REALCFLAGS += -Wbad-function-cast
REALCFLAGS += -Wcast-align
REALCFLAGS += -Wcast-qual
REALCFLAGS += -Wchar-subscripts
REALCFLAGS += -Wcomments
REALCFLAGS += -Wdeprecated-declarations
REALCFLAGS += -Wendif-labels
REALCFLAGS += -Wfloat-equal
REALCFLAGS += -Wformat
REALCFLAGS += -Wformat-nonliteral
REALCFLAGS += -Wformat-security
REALCFLAGS += -Wimplicit-int
REALCFLAGS += -Wimport
REALCFLAGS += -Winline
REALCFLAGS += -Wlong-long
REALCFLAGS += -Wmissing-braces
REALCFLAGS += -Wmissing-declarations
REALCFLAGS += -Wmissing-format-attribute
REALCFLAGS += -Wmissing-noreturn
REALCFLAGS += -Wmissing-prototypes
REALCFLAGS += -Wmultichar
REALCFLAGS += -Wnested-externs
# REALCFLAGS += -Wpacked           ## cannot have this.
# REALCFLAGS += -Wpadded           ## cannot have this warning turned on, link layer yells.
REALCFLAGS += -Wparentheses
REALCFLAGS += -Wpointer-arith
REALCFLAGS += -Wredundant-decls
REALCFLAGS += -Wreturn-type
REALCFLAGS += -Wshadow
REALCFLAGS += -Wsign-compare
REALCFLAGS += -Wstrict-aliasing
REALCFLAGS += -Wstrict-prototypes
REALCFLAGS += -Wswitch
# REALCFLAGS += -Wswitch-default   ## memset() gives warning about missing default on case.
REALCFLAGS += -Wswitch-enum
REALCFLAGS += -Wtrigraphs
REALCFLAGS += -Wundef
ifeq (${PERF},PERF)
REALCFLAGS += -Wuninitialized
endif
REALCFLAGS += -Wunknown-pragmas
REALCFLAGS += -Wunused
REALCFLAGS += -Wunused-function
REALCFLAGS += -Wunused-label
REALCFLAGS += -Wunused-parameter
REALCFLAGS += -Wunused-value
REALCFLAGS += -Wunused-variable
REALCFLAGS += -Wwrite-strings
REALCFLAGS += -Wno-cast-qual
#+++
# -- xk_kernel.c and PortServ.c have problems with -O2,3,4. -fstrict-aliasing.
REALCFLAGS += -Wno-strict-aliasing

REALCFLAGS += -O3

REALCFLAGS += -march=pentium4

CFLAGS := ${CFLAGS} ${REALCFLAGS}

all:	size sizeof
