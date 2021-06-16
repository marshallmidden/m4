.PHONY: help
help:
	@echo "Targets:"
#--	@echo "  A      - run script A"
#--	@grep -v '^#' A | sed -e 's/^/            /'
#--	@echo "  B      - run script B"
#--	@grep -v '^#' B | sed -e 's/^/            /'
#--	@echo "  V      - run script V"
#--	@grep -v '^#' V | sed -e 's/^/            /'
	@echo "  all"
	@echo "  mcomp  - Compile mcomp program."
	@echo "  clean  - Remove objects, postscript (ps), and executable mcomp."
	@echo "  tags   - Create tags for 'vi'."
	@echo ""
	@echo "Use 'yaps file.abc -k' to get file.ps."
	@echo "Use 'midicsv file.mid >file.csv'."
	@echo "Use 'csvmidi file.csv >file.mid'."
	@echo "Use 'midi2abc file.mid >file.abc'."
	@echo
	@echo "  v      - create v.mid from v-0."
#=============================================================================
#- .PHONY: A
#- A:	mcomp
#- 	./A
#-----------------------------------------------------------------------------
#- .PHONY: B
#- B:	mcomp
#- 	./B
#-----------------------------------------------------------------------------
#- .PHONY: V
#- V:	mcomp
#- 	./V
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
	rm -f v.csv v.mid
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
.PHONY: v
v:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m v-0 >v.csv
	csvmidi v.csv v.mid
#	fluidsynth -a sdl2 -i -n -q --reverb=off /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 v.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 v.mid
#-----------------------------------------------------------------------------
.PHONY: a1
a1:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.01 >a1.csv
	#-- ~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.01 >a1.abc
	#-- yaps a1.abc -o a1.ps -k
	csvmidi a1.csv a1.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a1.mid
#-----------------------------------------------------------------------------
.PHONY: a2
a2:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.02 >a2.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.02 >a2.abc
	yaps a2.abc -o a2.ps -k
	csvmidi a2.csv a2.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a2.mid
#-----------------------------------------------------------------------------
.PHONY: a3
a3:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.03 >a3.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.03 >a3.abc
	yaps a3.abc -o a3.ps -k
	csvmidi a3.csv a3.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a3.mid
#-----------------------------------------------------------------------------
.PHONY: a4
a4:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.04 >a4.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.04 >a4.abc
	yaps a4.abc -o a4.ps -k
	csvmidi a4.csv a4.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a4.mid
#-----------------------------------------------------------------------------
.PHONY: a5
a5:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.05 >a5.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.05 >a5.abc
	yaps a5.abc -o a5.ps -k
	csvmidi a5.csv a5.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a5.mid
#-----------------------------------------------------------------------------
.PHONY: a6
a6:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.06 >a6.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.06 >a6.abc
	yaps a6.abc -o a6.ps -k
	csvmidi a6.csv a6.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a6.mid
#-----------------------------------------------------------------------------
.PHONY: a7
a7:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.07 >a7.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.07 >a7.abc
	yaps a7.abc -o a7.ps -k
	csvmidi a7.csv a7.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a7.mid
#-----------------------------------------------------------------------------
.PHONY: a8
a8:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.08 >a8.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.08 >a8.abc
	yaps a8.abc -o a8.ps -k
	csvmidi a8.csv a8.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a8.mid
#-----------------------------------------------------------------------------
.PHONY: a9
a9:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.09 >a9.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.09 >a9.abc
	yaps a9.abc -o a9.ps -k
	csvmidi a9.csv a9.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a9.mid
#-----------------------------------------------------------------------------
.PHONY: a10
a10:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.10 >a10.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.10 >a10.abc
	yaps a10.abc -o a10.ps -k
	csvmidi a10.csv a10.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a10.mid
#-----------------------------------------------------------------------------
# End of Makefile
