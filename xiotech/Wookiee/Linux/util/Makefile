# $Id: Makefile 145166 2010-08-05 21:40:17Z m4 $
#
# Makefile for utilities
#
# Mark D. Rustad, 2009/06/30
#
# Copyright 2009 Xiotech Corporation. All rights reserved.

MODEL ?= 3000
GCC=gcc

# ----------------------------------------------------------------------------
# Set locations of shared memory segments, and other mmap-ed areas.
include ../../Makefile-${MODEL}.defs
# ----------------------------------------------------------------------------

CFLAGS += -c -g -Wall -Werror
# CFLAGS += -O2			# Does not work with -O2 set! 2007/05/01
CFLAGS += ${ON64FLAGS}
ifeq (${RUNON64},RUNON64)
CFLAGS += -DRUNON64 -Wno-pointer-sign
endif

OBJDIR := obj_${MODEL}
CFLAGS += -I ${OBJDIR}
LDFLAGS += -Wl,-Map,$@.map

tmp-dep = $(dir $@).$(notdir $@).d
dep-flags = -Wp,-MD,${tmp-dep}

TARGETS := subst-elf shmdump cvtcore_${MODEL}

all: ${OBJDIR} $(addprefix ${OBJDIR}/,${TARGETS})

${OBJDIR}:
	mkdir -p $@

${OBJDIR}/cvtcore_${MODEL}:	cvtcore.src ../../Makefile-${MODEL}.defs
	sed -e 's/FESTART/${MAKE_DEFS_FE_start}/' \
		-e 's/CCBSTART/${MAKE_DEFS_CCB_start}/' \
		-e 's/BESTART/${MAKE_DEFS_BE_start}/' \
		< $< > $@
	chmod +x $@

${OBJDIR}/%.o: %.c
	@echo $@
	@${GCC} ${dep-flags} ${CFLAGS} -o $@ $<

${OBJDIR}/%: ${OBJDIR}/%.o
	@echo $@
	@${GCC} ${ON64FLAGS} ${LDFLAGS} -o $@ $^

clean:
	rm -rf *.core core.* ${OBJDIR}

.PHONY: all clean

#-----------------------------------------------------------------------------
test:
	echo CFLAGS=${CFLAGS}
	echo MODEL=${MODEL}
#-----------------------------------------------------------------------------
# Include the compile dependencies.
-include ${OBJDIR}/.*.d
