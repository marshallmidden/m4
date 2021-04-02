.PHONY: help
help:
	@echo "Targets:"
	@echo "  A      - run script A"
	@grep -v '^#' A | sed -e 's/^/            /'
	@echo "  B      - run script B"
	@grep -v '^#' B | sed -e 's/^/            /'
	@echo "  V      - run script V"
	@grep -v '^#' V | sed -e 's/^/            /'
	@echo "  all"
	@echo "  mcomp  - Compile mcomp program."
	@echo "  clean  - Remove objects, postscript (ps), and executable mcomp."
	@echo "  tags   - Create tags for 'vi'."
	@echo ""
	@echo "Use 'yaps file.abc -k' to get file.ps."
	@echo "Use 'midicsv file.mid >file.csv'."
	@echo "Use 'csvmidi file.csv >file.mid'."
	@echo "Use 'midi2abc file.mid >file.abc'."
#=============================================================================
.PHONY: A
A:	mcomp
	./A
#-----------------------------------------------------------------------------
.PHONY: B
B:	mcomp
	./B
#-----------------------------------------------------------------------------
.PHONY: V
V:	mcomp
	./V
#=============================================================================
CC=gcc
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
#--too many to fix--	CFLAGS += -Wswitch-enum
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
CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers
#-----------------------------------------------------------------------------
LD = ${CC}
# LDLIBS += -lncurses
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
#=============================================================================
.PHONY: all
all : mcomp
#-----------------------------------------------------------------------------
mcomp : mcomp.o
	$(LD) ${LDFLAGS} -o mcomp mcomp.o ${LDLIBS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.o
	rm -f mcomp
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f A.csv A.ABC
	rm -f B.mid
	rm -f V.csv V.ABC
#-----------------------------------------------------------------------------
.PHONY: tags tag ctags
tag ctags tags:
#--	ctags -d -t mcomp.c
	ctags mcomp.c
#-----------------------------------------------------------------------------
testmidi.abc: testmidi
	/Users/m4/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc < testmidi >testmidi.abc
#-----------------------------------------------------------------------------
testmidi1.mid: testmidi.abc
	abc2midi testmidi.abc
#-----------------------------------------------------------------------------
testmidi1.csv: testmidi1.mid
	midicsv testmidi1.mid > testmidi1.csv
#-----------------------------------------------------------------------------
.PHONY: test
test T:	mcomp testmidi1.mid testmidi1.csv
#	./mcomp version2_b6-1.mid > version2_b6
	./mcomp testmidi1.mid > T
#-----------------------------------------------------------------------------
.PHONY: b6
b6:	mcomp version2_b6-1.mid
	./mcomp version2_b6-1.mid > version2_b6
#-----------------------------------------------------------------------------
