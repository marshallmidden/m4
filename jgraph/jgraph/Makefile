# Start of Makefile for jgraph.
#-----------------------------------------------------------------------------
PROG = jgraph
#-----------------------------------------------------------------------------
OBJS =  draw.o \
	edit.o \
	list.o \
	printline.o \
	prio_list.o \
	process.o \
	show.o \
	token.o
#=============================================================================
CC = gcc
#-----------------------------------------------------------------------------
CFLAGS += -m64

#-- CFLAGS += -ansi
CFLAGS += -pedantic

CFLAGS +=-g
CFLAGS +=-O3
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
#-NOT-CLANG-	CFLAGS += -Wlogical-op
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
#-NOT-CLANG-	CFLAGS += -Wunsafe-loop-optimizations
CFLAGS += -Wunused
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
CFLAGS += -Wwrite-strings
CFLAGS += -Wvolatile-register-var
#-NOT-CLANG-	CFLAGS += -fbounds-check
#-----------------------------------------------------------------------------
LD = ${CC}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# We are on a 64 bit machine.
LDFLAGS += -m64
# Do not allow unresolved references in object files
#-- LDFLAGS += -Xlinker --no-undefined
# Warn about duplicate common symbols
#-- LDFLAGS += -Xlinker --warn-common
# Warn if global constructors/destructors are seen
#-- LDFLAGS += -Xlinker --warn-constructors
# Warn if the multiple GP values are used
#-- LDFLAGS += -Xlinker --warn-multiple-gp
# Warn only once per undefined symbol
#-- LDFLAGS += -Xlinker --warn-once
# Report unresolved symbols as errors
#-- LDFLAGS += -Xlinker --error-unresolved-symbols
# Treat warnings as errors
#-- LDFLAGS += -Xlinker --fatal-warnings
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Check section addresses for overlaps (default)
#-- LDFLAGS += -Xlinker --check-sections
#=============================================================================
%.o: %.c
	@echo "  CC $@"
	${CC} -c ${CFLAGS} $<
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%: %.o
	@echo " LD $@"
	${LD} -o $@ ${CFLAGS} ${LDFLAGS} $<
#=============================================================================
.PHONY:	all
all:	${PROG}
#=============================================================================
${PROG}: ${PROG}.o Makefile ${OBJS}
	${CC} ${CFLAGS} -Wall -o ${PROG} ${PROG}.o ${OBJS} ${LDFLAGS} -lm
#=============================================================================
${PROG}.o: ${PROG}.c Makefile
#=============================================================================
draw.o:	     draw.c jgraph.h list.h prio_list.h Makefile
#-----------------------------------------------------------------------------
edit.o:      edit.c jgraph.h list.h prio_list.h Makefile
#-----------------------------------------------------------------------------
jgraph.o:    jgraph.c jgraph.h list.h prio_list.h  Makefile
#-----------------------------------------------------------------------------
list.o:      list.c list.h Makefile
#-----------------------------------------------------------------------------
printline.o: printline.c jgraph.h list.h prio_list.h  Makefile
#-----------------------------------------------------------------------------
prio_list.o: prio_list.c list.h prio_list.h Makefile
#-----------------------------------------------------------------------------
process.o:   process.c jgraph.h list.h prio_list.h  Makefile
#-----------------------------------------------------------------------------
show.o:      show.c jgraph.h list.h prio_list.h  Makefile
#-----------------------------------------------------------------------------
token.o:     token.c list.h Makefile
#=============================================================================
.PHONY: clean
clean:
	rm -f *.o a.out tags ${PROG} ${PROG}.exe ${PROG}.stackdump ${PROG}.exe.stackdump 
#=============================================================================
.PHONY: ctags tags
ctags tags:
	ctags ${PROG}.c $(patsubst %.o,%.c,$(OBJS)) *.h
#-----------------------------------------------------------------------------
.PHONY: install
install: jgraph
	install -d /usr/local/bin/
	install jgraph /usr/local/bin/jgraph
	install jgraph.1 /usr/local/share/man/man1/
#-----------------------------------------------------------------------------
# End of Makefile for jgraph.
