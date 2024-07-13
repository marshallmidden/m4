# Start of Makefile for singlechar.
#-----------------------------------------------------------------------------
PROG1 = ternary
#-----------------------------------------------------------------------------
OS := ${shell uname}
#-----------------------------------------------------------------------------
CFLAGS   +=-g
#-- CFLAGS   +=-O0
ifneq (${OS},Darwin)
CFLAGS   +=-O4
else
CFLAGS   +=-O3
endif
CFLAGS += -mtune=corei7
CFLAGS += -march=corei7

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
ifneq (${OS},Darwin)
CFLAGS += -Wlogical-op
endif
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
ifneq (${OS},Darwin)
CFLAGS += -Wunsafe-loop-optimizations
endif
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
CFLAGS += -Wvolatile-register-var
ifneq (${OS},Darwin)
CFLAGS += -fbounds-check
endif
#-----------------------------------------------------------------------------
LD = ${CC}
LDLIBS += -lncurses
# LDLIBS += -lcurses
# LDLIBS += -lncurses++
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
LDFLAGS += -m64
ifneq (${OS},Darwin)
# Optimize output file
LDFLAGS += -Xlinker -O
# Strip symbols in discarded sections
LDFLAGS += -Xlinker --strip-discarded
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
# Remove unused sections (on some targets)
LDFLAGS += -Xlinker --gc-sections
#- # List removed unused sections on stderr
#- LDFLAGS += -Xlinker --print-gc-sections
# ignore-all, report-all, ignore-in-object-files, ignore-in-shared-libs.
LDFLAGS += -Xlinker --unresolved-symbols=report-all
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Check section addresses for overlaps (default)
LDFLAGS += -Xlinker --check-sections
# Discard temporary local symbols (default)
LDFLAGS += -Xlinker --discard-locals
endif
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Do not link against shared libraries.
#-- LDFLAGS += -Xlinker -Bstatic
#-----------------------------------------------------------------------------
%.o: %.c
	@ echo "  CC $@"
	@${CC} -c ${CFLAGS} $<
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%: %.o
	@ echo " LD $@"
	@${LD} -o $@ ${CFLAGS} ${LDFLAGS} $< ${LDLIBS}
#-----------------------------------------------------------------------------
.PHONY:	all
all:	${PROG1}
#-----------------------------------------------------------------------------
${PROG1}: ${PROG1}.o Makefile
#-----------------------------------------------------------------------------
${PROG1}.o: ${PROG1}.c Makefile
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o tags ${PROG1} ${PROG1}.exe ${PROG1}.stackdump ${PROG1}.exe.stackdump
#-----------------------------------------------------------------------------
.PHONY: ctags tags
ctags tags:
	ctags ${PROG1}.c
#-----------------------------------------------------------------------------
.PHONY:	test m4 run
test m4 run:	${PROG1}
	./${PROG1}
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo
	@echo "make all         Compile program ${PROG1}."
	@echo "make clean       Remove executable and potential core/dump files."
	@echo "make tags        Create tags file for vi editor."
	@echo "make m4          Rum the program."
	@echo
#-----------------------------------------------------------------------------
# vi: sw=4 ts=8 expandtab
# End of Makefile
