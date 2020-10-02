# Start of Makefile for mls.
#-----------------------------------------------------------------------------
PROG = mls
#=============================================================================
#-- DEFINES  = -DHAVE_CONFIG_H 
#-- DEFINES += -I.
#-- DEFINES += -D_LARGEFILE64_SOURCE
#-- DEFINES += -D_FILE_OFFSET_BITS=64
#=============================================================================
CFLAGS += -m64

CFLAGS +=-g
# CFLAGS +=-O4
CFLAGS +=-O0
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wno-system-headers
# CFLAGS += -Wtraditional		## broken are many typical usages.
CFLAGS += -Waggregate-return
CFLAGS += -Warray-bounds
CFLAGS += -Wattributes
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wchar-subscripts
CFLAGS += -Wcomments
#-- CFLAGS += -Wconversion       ## cannot have -- broken conversions with unsigned char.
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wdisabled-optimization
CFLAGS += -Wendif-labels
CFLAGS += -Wextra
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
CFLAGS += -Wformat-nonliteral
CFLAGS += -Wformat-security
CFLAGS += -Wimplicit-int
CFLAGS += -Wimport
CFLAGS += -Winline
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
CFLAGS += -Wnested-externs
CFLAGS += -Woverflow
CFLAGS += -Wparentheses
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-aliasing
CFLAGS += -Wstrict-overflow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wswitch-enum
CFLAGS += -Wtrigraphs
CFLAGS += -Wundef
CFLAGS += -Wuninitialized
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunreachable-code
CFLAGS += -Wunsafe-loop-optimizations
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
CFLAGS += -Wvolatile-register-var
CFLAGS += -fbounds-check
#-----------------------------------------------------------------------------
LD = ${CC}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# We are on a 64 bit machine.
LDFLAGS += -m64
# Do not allow unresolved references in object files
LDFLAGS += -Xlinker --no-undefined
# Warn about duplicate common symbols
LDFLAGS += -Xlinker --warn-common
# Warn if global constructors/destructors are seen
LDFLAGS += -Xlinker --warn-constructors
# Warn if the multiple GP values are used
LDFLAGS += -Xlinker --warn-multiple-gp
# Warn only once per undefined symbol
LDFLAGS += -Xlinker --warn-once
# Report unresolved symbols as errors
LDFLAGS += -Xlinker --error-unresolved-symbols
# Treat warnings as errors
LDFLAGS += -Xlinker --fatal-warnings
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Check section addresses for overlaps (default)
LDFLAGS += -Xlinker --check-sections
#-----------------------------------------------------------------------------
%.o: %.c
	@echo "  CC $@"
	@ ${CC} -c ${CFLAGS} ${DEFINES} $<
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%: %.o
	@echo "  LD $@"
	@ ${LD} -o $@ ${CFLAGS} ${LDFLAGS} $<
#=============================================================================
.PHONY:	all
all:	${PROG}
#-----------------------------------------------------------------------------
${PROG}: ${PROG}.o Makefile
#-----------------------------------------------------------------------------
${PROG}.o: ${PROG}.c Makefile
#-----------------------------------------------------------------------------
.PHONY: install
install: ${PROG}
	install -D ${PROG} ../../PX/bin/${PROG}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o tags
	rm -f ${PROG} ${PROG}.exe ${PROG}.stackdump ${PROG}.exe.stackdump
#	rm -f /var/tmp/cores/core.${PROG}.*
#-----------------------------------------------------------------------------
.PHONY: ctags tags
ctags tags:
	ctags ${PROG}.c
#-----------------------------------------------------------------------------
.PHONY:	gdb
gdb:	${PROG}
	gdb ./${PROG} /var/tmp/cores/core.simulator.*
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo
	@echo "make all          Compile programs ${PROG}."
	@echo "make clean        Remove executables and potential core/dump files."
	@echo "make tags         Create tags file for vi editor."
#-----------------------------------------------------------------------------
# End of Makefile for mls.
