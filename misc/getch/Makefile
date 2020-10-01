PROG = test_get_single_ch.exe
#-----------------------------------------------------------------------------
OBJS = get_single_ch.o test_get_single_ch.o
SRCS = get_single_ch.c test_get_single_ch.c
#-----------------------------------------------------------------------------
CFLAGS   +=-g
CFLAGS   +=-O4
# CFLAGS   +=-O3
# CFLAGS   +=-O2
#-- CFLAGS += -msse4.2
#-- CFLAGS += -mtune=nehalem
#-- CFLAGS += -march=nehalem
CFLAGS += -mtune=corei7
CFLAGS += -march=corei7

CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wno-system-headers
# CFLAGS += -Wtraditional		## broken are many typical usages.
CFLAGS += -Waggregate-return
CFLAGS += -Warray-bounds                           # added in gcc 4.3.1
CFLAGS += -Wattributes                             # added in gcc 4.3.1
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wchar-subscripts
CFLAGS += -Wcomments
#-- CFLAGS += -Wconversion       ## cannot have -- broken conversions with unsigned char.
CFLAGS += -Wdeprecated-declarations
CFLAGS += -Wdisabled-optimization    ## GCSE disabled, and VRP disabled in FrontEnd.
CFLAGS += -Wendif-labels
CFLAGS += -Wextra                                  # added in gcc 4.3.1
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
CFLAGS += -Wformat-nonliteral
CFLAGS += -Wformat-security
CFLAGS += -Wimplicit-int
CFLAGS += -Wimport
CFLAGS += -Winline
CFLAGS += -Wlogical-op                             # added in gcc 4.3.1
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
CFLAGS += -Wnested-externs
CFLAGS += -Woverflow                               # added in gcc 4.3.1
CFLAGS += -Wparentheses
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-aliasing
CFLAGS += -Wstrict-overflow                        # added in gcc 4.3.1
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wswitch-enum
CFLAGS += -Wtrigraphs
CFLAGS += -Wundef
CFLAGS += -Wuninitialized
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunreachable-code ## 2008-10-08 -- gives false positives with gcc 3.3.3 and 4.3.1.
CFLAGS += -Wunsafe-loop-optimizations                # added in gcc 4.3.1
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
CFLAGS += -Wvolatile-register-var                  # added in gcc 4.3.1
CFLAGS += -fbounds-check

#-----------------------------------------------------------------------------
LD = ${CC}
LD += -m64
# Optimize output file
LD += -Xlinker -O
# Strip symbols in discarded sections
LD += -Xlinker --strip-discarded
# Do not allow unresolved references in object files
LD += -Xlinker --no-undefined
# Warn about duplicate common symbols
LD += -Xlinker --warn-common
# Warn if global constructors/destructors are seen
LD += -Xlinker --warn-constructors
# Warn if the multiple GP values are used
LD += -Xlinker --warn-multiple-gp
# Warn only once per undefined symbol
LD += -Xlinker --warn-once
# Report unresolved symbols as errors
LD += -Xlinker --error-unresolved-symbols
# Treat warnings as errors
LD += -Xlinker --fatal-warnings
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Remove unused sections (on some targets)
LD += -Xlinker --gc-sections
# List removed unused sections on stderr
LD += -Xlinker --print-gc-sections
# ignore-all, report-all, ignore-in-object-files, ignore-in-shared-libs.
LD += -Xlinker --unresolved-symbols=report-all
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Check section addresses for overlaps (default)
LD += -Xlinker --check-sections
# Discard temporary local symbols (default)
LD += -Xlinker --discard-locals
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Do not link against shared libraries.
#-- LD += -Xlinker -Bstatic
#-----------------------------------------------------------------------------
%.o: %.c
	# $@
	@${CC} -o $@ ${CFLAGS} -c $<
#-----------------------------------------------------------------------------
.PHONY:	all
all:	${PROG}
#-----------------------------------------------------------------------------
${PROG}: ${OBJS} Makefile
	# $@
	@${LD} -o $@ ${CFLAGS} ${OBJS}
#-----------------------------------------------------------------------------
get_single_ch.o: get_single_ch.c Makefile
#-----------------------------------------------------------------------------
test_single_ch.o: test_single_ch.c Makefile
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o tags ${PROG} ${PROG}.stackdump
#-----------------------------------------------------------------------------
.PHONY: ctags tags
ctags tags:
	ctags ${SRCS}
#-----------------------------------------------------------------------------
