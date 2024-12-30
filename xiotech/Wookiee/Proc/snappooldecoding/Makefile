MODEL = 7000
TYPE ?= Debug
# ----------------------------------------------------------------------------
MAKEFILE = Makefile
include ../../Makefile-${MODEL}.defs
B_CFLAGS += ${MAKE_DEFS_CC} -DSNAPPOOLDECODE -DBACKEND
# ----------------------------------------------------------------------------
cwd := $(shell pwd)
WHERE := ${cwd}
# -------------
# Must have obj_${MODEL} directory for fe.ld and be.ld to find file f_front.CT.o and b_back.Ct.o.
OBJDIRNAME := ../obj_${MODEL}${TYPE}
OBJ := $(shell if [ -d ${OBJDIRNAME} ]; then echo "${OBJDIRNAME}/" ; \
	else mkdir ${OBJDIRNAME} && echo "${OBJDIRNAME}/" ; fi)
# -------------
CC = gcc
C_INCLUDES += -I${WHERE}
C_INCLUDES += -I${WHERE}/../inc
C_INCLUDES += -I${WHERE}/../../Shared/Inc
C_INCLUDES += -I${WHERE}/../../Linux/xio3d
C_INCLUDES += -I${WHERE}/${OBJ}
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Default VER to ENG -- note, command line overrides such assignments!
VER = ENGR
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C_GCC_FLAGS += -g
C_GCC_FLAGS += -O0
#......................................
C_GCC_FLAGS += -fno-strict-aliasing
#++++++++++++++++++
#--2024-12-12 C_GCC_FLAGS += -fno-rename-registers
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Lots of warning messages.
C_GCC_FLAGS += -W
C_GCC_FLAGS += -Wall
C_GCC_FLAGS += -Werror
C_GCC_FLAGS += -Wno-system-headers
C_GCC_FLAGS += -Waggregate-return
C_GCC_FLAGS += -Wbad-function-cast
C_GCC_FLAGS += -Wcast-align
C_GCC_FLAGS += -Wcast-qual
C_GCC_FLAGS += -Wchar-subscripts
C_GCC_FLAGS += -Wcomments
C_GCC_FLAGS += -Wdeprecated-declarations
C_GCC_FLAGS += -Wendif-labels
C_GCC_FLAGS += -Wfloat-equal
C_GCC_FLAGS += -Wformat
C_GCC_FLAGS += -Wformat-nonliteral
C_GCC_FLAGS += -Wformat-security
C_GCC_FLAGS += -Wimplicit-int
C_GCC_FLAGS += -Wimport
C_GCC_FLAGS += -Winline
C_GCC_FLAGS += -Wlong-long
C_GCC_FLAGS += -Wmissing-braces
C_GCC_FLAGS += -Wmissing-declarations
C_GCC_FLAGS += -Wmissing-format-attribute
C_GCC_FLAGS += -Wmissing-noreturn
C_GCC_FLAGS += -Wmissing-prototypes
C_GCC_FLAGS += -Wmultichar
C_GCC_FLAGS += -Wnested-externs
C_GCC_FLAGS += -Wparentheses
C_GCC_FLAGS += -Wpointer-arith
C_GCC_FLAGS += -Wredundant-decls
C_GCC_FLAGS += -Wreturn-type
C_GCC_FLAGS += -Wshadow
C_GCC_FLAGS += -Wsign-compare
C_GCC_FLAGS += -Wstrict-aliasing
C_GCC_FLAGS += -Wstrict-prototypes
C_GCC_FLAGS += -Wswitch
C_GCC_FLAGS += -Wswitch-enum
C_GCC_FLAGS += -Wtrigraphs
C_GCC_FLAGS += -Wundef
C_GCC_FLAGS += -Wunknown-pragmas
C_GCC_FLAGS += -Wunused
C_GCC_FLAGS += -Wunused-function
C_GCC_FLAGS += -Wunused-label
C_GCC_FLAGS += -Wunused-parameter
C_GCC_FLAGS += -Wunused-value
C_GCC_FLAGS += -Wunused-variable
C_GCC_FLAGS += -Wwrite-strings
# Set the architecture type to pentium4 (Dell 2600, 7501's, Nocona Lindenhurst).
#-- C_GCC_FLAGS += -march=pentium4
# -------------
LD = gcc
LIBS =
LD += -Xlinker -E
LD += -Xlinker -d
LD += -Xlinker --no-strip-discarded
LD += -Xlinker --discard-none
LD += -Xlinker --no-undefined
LD += -Xlinker --warn-common
LD += -Xlinker --warn-constructors
LD += -Xlinker --warn-multiple-gp
LD += -Xlinker --warn-once
LD += -Xlinker --error-unresolved-symbols
LD += -Xlinker -z -Xlinker defs
LD += -static
#-----
# Following is only for "c" files, NOT i960 assembler generated (*.CT.c.) files.
C_CODEFLAGS += -Wno-cast-qual
# ============================================================================
# Following is for Link Layer located in different directory.
VPATH = ${WHERE}/../../Shared/Src
#-----
B_CFLAGS += ${B_C_DEFINES} ${C_INCLUDES} ${C_GCC_FLAGS}
${OBJ}snapshotwork.o: override CFLAGS = ${B_CFLAGS} -fpack-struct
${OBJ}main.o: override CFLAGS = ${B_CFLAGS}
# ============================================================================
.PHONY: all
all: testsnapshot
# ============================================================================
${OBJ}.gen:	$(wildcard ../inc/*.def) $(wildcard ../../Shared/Inc/*.def)
	# $@
	mkdir -p ${@D}
	for f in $?; do ../../Bin/makeinc.pl -o ${@D} $$f; done
	touch $@
#-----
${OBJ}snapshotwork.o: snapshotwork.c Makefile
	# $@
	@${CC} -o $@ ${CFLAGS} -c $<
${OBJ}main.o: main.c Makefile
	# $@
	@${CC} -o $@ ${CFLAGS} -c $<
# ============================================================================
testsnapshot: ${OBJ}main.o ${OBJ}snapshotwork.o
	# $@
	@${LD} -o $@ ${OBJ}main.o ${OBJ}snapshotwork.o ${LIBS}
# ============================================================================
.PHONY: clean
clean:
	rm -f ${OBJ}main.o ${OBJ}snapshotwork.o testsnapshot tags core
	rm -f AAA.out AAA.err BBB.out BBB.err CCC.out CCC.err
# ----------------------------------------------------------------------------
.PHONY: obj
obj:
	@if [ -d ${OBJDIRNAME} ]; then echo "directory ${OBJDIRNAME} exists" ; \
		else mkdir ${OBJDIRNAME} ; echo "created directory ${OBJDIRNAME}" ; fi
# ----------------------------------------------------------------------------
.PHONY: tag tags ctags
tag tags ctags:
	ctags *.[hc] ../inc/*.h ../../Shared/*/*.h ${OBJDIRNAME}/*.h
# ----------------------------------------------------------------------------
.PHONY: run
run:	testsnapshot
	./testsnapshot SNAPPOOL_100 > AAA.out 2> AAA.err
# ----------------------------------------------------------------------------
.PHONY: gdb
gdb:	testsnapshot core
	gdb testsnapshot core
# ----------------------------------------------------------------------------
.PHONY: run1
run1:	testsnapshot
	./testsnapshot SNAPPOOL_100before > AAA.out 2> AAA.err
# ----------------------------------------------------------------------------
.PHONY: run2
run2:	testsnapshot
	./testsnapshot SNAPPOOL_100after1stwrite > BBB.out 2> BBB.err
# ----------------------------------------------------------------------------
.PHONY: run3
run3:	testsnapshot
	./testsnapshot SNAPPOOL_100after2ndwrite > CCC.out 2> CCC.err
# ----------------------------------------------------------------------------
# vi:sw=8 ts=8 noexpandtab
# End of Makefile
# ----------------------------------------------------------------------------
