# $Id: Makefile 145451 2010-08-11 16:50:09Z m4 $
#
# Makefile for logsim

PROG := logsim
OBJDIR := ../obj_${MODEL}
WOBJDIR := ../obj_w${MODEL}
#-----------------------------------------------------------------------------
OBJ :=
OBJD :=
WOBJ :=
WOBJD :=
OBJL :=
OBJR :=

export PATH := ${PATH}:/opt/mingw32/bin
WCC := i386-mingw32-gcc -I ${WOBJDIR}
CC := gcc -g -O0 -I ${OBJDIR}

# Following are in this directory.
OBJL += LogSimFuncs.o
#OBJL += logtest.o
OBJL += logging.o
OBJL += main.o

# Following are in CCB/Src directory.
OBJR += codeburn.o
OBJR += convert.o
OBJR += EL_Strings.o
OBJR += errorCodes.o
OBJR += FCM_Strings.o
OBJR += HWM_ConditionStrings.o
OBJR += HWM_StatusStrings.o
OBJR += i82559_Strings.o
OBJR += logview.o
OBJR += realtime.o
OBJR += xk_rtc.o

# Following are in Shared/Src directory.
OBJR += L_Misc.o

OBJ += $(addprefix ${OBJDIR}/,${OBJL} ${OBJR})
OBJD += $(addprefix ${OBJDIR}/.,${OBJL} ${OBJR})
WOBJ += $(addprefix ${WOBJDIR}/,${OBJL} ${OBJR})
WOBJD += $(addprefix ${WOBJDIR}/.,${OBJL} ${OBJR})
#-----------------------------------------------------------------------------
CFLAGS += -MD -MF ${@D}/${@F:%.o=.%.d}
#-----------------------------------------------------------------------------
CFLAGS += -I../Inc/
CFLAGS += -I../../Inc/
CFLAGS += -I../../../Shared/Inc/
#-----------------------------------------------------------------------------
# CFLAGS += -E
#-----------------------------------------------------------------------------
CFLAGS += -DMODEL_${MODEL}
CFLAGS += -DCCB_RUNTIME_CODE
CFLAGS += -DLOG_SIMWOOKIEE
CFLAGS += -DLOG_SIMULATOR
CFLAGS += -DHYPERNODE
CFLAGS += -DLINUX_VER

CFLAGS += ${ON64FLAGS}
ifeq (${RUNON64},RUNON64)
CFLAGS += -DRUNON64 -Wno-pointer-sign
endif

#-- CFLAGS += -W -Wall -Werror -Wbad-function-cast -Wcast-align -Wcast-qual
CFLAGS += -W -Wall -Werror -Wbad-function-cast -Wcast-qual
CFLAGS += -Wchar-subscripts -Wcomments -Wdeprecated-declarations
CFLAGS += -Wendif-labels -Wfloat-equal
CFLAGS += -Wformat -Wformat-nonliteral -Wformat-security
CFLAGS += -Wimplicit-int -Wimport -Winline -Wlong-long
CFLAGS += -Wmissing-braces -Wmissing-declarations -Wmissing-format-attribute
CFLAGS += -Wmissing-noreturn -Wmissing-prototypes -Wmultichar -Wnested-externs
CFLAGS += -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type
CFLAGS += -Wshadow -Wsign-compare -Wstrict-aliasing -Wstrict-prototypes
CFLAGS += -Wswitch -Wswitch-enum -Wtrigraphs -Wundef -Wunknown-pragmas
CFLAGS += -Wunused -Wunused-function -Wunused-label -Wunused-parameter
CFLAGS += -Wunused-value -Wunused-variable -Wwrite-strings

#-----------------------------------------------------------------------------
all: ${WOBJDIR}/${PROG}.exe ${OBJDIR}/${PROG}
#-----------------------------------------------------------------------------
${OBJDIR}/${PROG}: ${OBJ}
	# $@
	@${CC} ${ON64FLAGS} -o $@ ${OBJ}

${WOBJDIR}/${PROG}.exe: ${WOBJ}
	# $@
	@${WCC} ${ON64FLAGS} -o $@ ${WOBJ}
#-----------------------------------------------------------------------------
${OBJ}: ${OBJDIR}/.gen
${WOBJ}: ${WOBJDIR}/.gen

${OBJDIR} ${WOBJDIR}:
	mkdir -p $@

# These get done before the *.def files.
${OBJDIR}/.gen: ${OBJDIR}
${WOBJDIR}/.gen: ${WOBJDIR}

${OBJDIR}/.gen ${WOBJDIR}/.gen:	$(wildcard ../../../Shared/Inc/*.def)
	# $@
	for f in $?; do ../../../Bin/makeinc.pl -o ${@D} $$f; done
	touch $@

%/logview.o: override CFLAGS += -Wno-format-nonliteral

${OBJDIR}/%.o: %.c
	# $@
	@${CC} ${CFLAGS} -o $@ -c $<

${WOBJDIR}/%.o: %.c
	# $@
	@${WCC} ${CFLAGS} -o $@ -c $<

${OBJDIR}/%.o: ../../Src/%.c
	# $@
	@${CC} ${CFLAGS} -o $@ -c $<

${WOBJDIR}/%.o: ../../Src/%.c
	# $@
	@${WCC} ${CFLAGS} -o $@ -c $<

${OBJDIR}/%.o: ../../../Shared/Src/%.c
	# $@
	@${CC} ${CFLAGS} -o $@ -c $<

${WOBJDIR}/%.o: ../../../Shared/Src/%.c
	# $@
	@${WCC} ${CFLAGS} -o $@ -c $<
#-----------------------------------------------------------------------------
clean:
	rm -f ${OBJ} ${PROG} ${OBJD:%.o=.%.d}
	rm -f ${WOBJ} ${PROG} ${WOBJD:%.o=.%.d}
	rm -rf ${OBJDIR}
	rm -rf ${WOBJDIR}
#-----------------------------------------------------------------------------
# Include the compile dependencies.
sinclude ${OBJD:%.o=%.d}
sinclude ${WOBJD:%.o=%.d}

# End of Makefile
