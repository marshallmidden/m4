PROGS = aio_read_test aio_write_test
LDLIBS  = -lrt
CC = gcc
#-----------------------------------------------------------------------------
#-- CFLAGS += -march=native -mtune-native

#-- CFLAGS += -m64

CFLAGS +=-g
CFLAGS +=-O3
#-- CFLAGS +=-O0
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
#-- LDFLAGS += -m64
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
	@ ${LD} -o $@ ${LDFLAGS} $< ${LDLIBS}
#-----------------------------------------------------------------------------
.PHONY: all
all: ${PROGS}
#-----------------------------------------------------------------------------
aio_read_test: aio_read_test.o
aio_read_test.o: aio_read_test.c Makefile
#-----------------------------------------------------------------------------
aio_write_test: aio_write_test.o
aio_write_test.o: aio_write_test.c Makefile
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o ${PROGS}
#-----------------------------------------------------------------------------
.PHONY: test
test:	${PROGS}
	./aio_read_test
	./aio_write_test
#-----------------------------------------------------------------------------
# End of Makefile
