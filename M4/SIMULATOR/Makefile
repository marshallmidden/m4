# Start of Makefile for simulator.
#-----------------------------------------------------------------------------
#-- WHERE	= "/mnt/m4_bigdst1"
#-- DIR1     = "${WHERE}/input_files/"
#-- DIR_LIVE = "${WHERE}/output_files/"
#-----------------------------------------------------------------------------
WHERE    = .
DIR1     = "/mnt/m4_nfs1/input_files"
DIR_LIVE = "/mnt/m4_nfs2/output_files"
#-----------------------------------------------------------------------------
PWD:="${shell pwd}"
#-----------------------------------------------------------------------------
PROG = simulator
#-----------------------------------------------------------------------------
OS := ${shell uname}
#-----------------------------------------------------------------------------
#OLD	CFLAGS += -mtune=corei7
#OLD	CFLAGS += -march=corei7

CFLAGS += -m64
#-- CFLAGS += -march=core-avx2
#-- CFLAGS += -mtune=core-avx2

CFLAGS +=-g
CFLAGS +=-O4
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
	${CC} -c ${CFLAGS} $<
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%: %.o
	@echo " LD $@"
	${LD} -o $@ ${CFLAGS} ${LDFLAGS} $<
#-----------------------------------------------------------------------------
.PHONY:	all
all:	${PROG}
#-----------------------------------------------------------------------------
${PROG}: ${PROG}.o Makefile
	${CC} ${CFLAGS} -Wall -lpthread -o ${PROG} ${PROG}.o ${LDFLAGS}
#-----------------------------------------------------------------------------
${PROG}.o: ${PROG}.c Makefile
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o tags \
	    ${PROG} ${PROG}.exe ${PROG}.stackdump ${PROG}.exe.stackdump
#-----------------------------------------------------------------------------
.PHONY: ctags tags
ctags tags:
	ctags ${PROG}.c
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	reallyclean
reallyclean:
	${MAKE} clean
	rm -rf ${DIR1} ${DIR_LIVE}
#-----------------------------------------------------------------------------
.PHONY:	m4 run
m4 run:	${PROG}
	rm -f /var/tmp/cores/core.simulator.*
	rm -rf ${DIR1}/*/*/*.dat
	rm -rf ${DIR_LIVE}/*/*/*.dat
	# Start simulator program.
	${WHERE}/${PROG} ${DIR1} ${DIR_LIVE}
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
	@echo "make m4           Run the program."
	@echo "make reallyclean  "make clean" and remove the test files"
	@echo
	@echo "Variables:"
	@echo "  PWD     =${PWD}"
	@echo "  WHERE   =${WHERE}"
	@echo "  DIR1    =${DIR1}"
	@echo "  DIR_LIVE=${DIR_LIVE}"
#-----------------------------------------------------------------------------
# End of Makefile
