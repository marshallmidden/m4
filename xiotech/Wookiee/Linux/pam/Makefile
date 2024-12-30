# ----------------------------------------------------------------------------
# Set locations of shared memory segments, and other mmap-ed areas.
include ../../Makefile-${MODEL}.defs
CFLAGS += ${MAKE_DEFS_CC}
# ----------------------------------------------------------------------------

GCC=gcc

CFLAGS += -c -g -Wall -Werror -pthread
CFLAGS += -D_REENTRANT -DPAM
# CFLAGS += -O2			# Does not work with -O2 set! 2007/05/01
CFLAGS += ${ON64FLAGS}
ifeq (${RUNON64},RUNON64)
CFLAGS += -DRUNON64 -Wno-pointer-sign
endif

OBJDIR := obj_${MODEL}
CFLAGS += -I ${OBJDIR}

# import files
include makefile-${MODEL}.files

# Uncomment for PAM DEADLOOP
PAMDEF += -DPAM_DEADLOOP

LDFLAGS += -pthread -Wl,-Map,$@.map

# ----------------------------------------------------------------------------
.PHONY: all
all: deps ${OBJDIR}/pam ${OBJDIR}/fakepam

.PHONY: deps
deps: ${OBJDIR} ${OBJDIR}/.gen depspre depspam depsccb depsshr depsprc
	@touch ${OBJDIR}/.deps

${OBJDIR}:
	mkdir -p ${OBJDIR}

${OBJDIR}/.gen:	$(wildcard ../../Shared/Inc/*.def ../../Proc/inc/*.def)
	# $@
	for f in $?; do ../../Bin/makeinc.pl -o ${@D} $$f; done
	touch $@

.PHONY: depspre
depspre:
	@echo "Checking dependencies"
	@rm -f $@ ${OBJDIR}/makefile.deps ${OBJDIR}/.deps

.PHONY: depspam
depspam: $(PAMFILES)
	@${GCC} $(PAMFLAGS) $(PAMINC) $(PAMDEF) -MM $^ | \
		sed -e "s/^\([^ ][^ ]*\)/$$\(OBJ\)\1 Makefile/" >> \
		${OBJDIR}/makefile.deps

.PHONY: depsccb
depsccb: $(CCBFILES)
	@${GCC} $(CCBFLAGS) $(CCBINC) $(CCBDEF) -MM $^ | \
		sed -e "s/^\([^ ][^ ]*\)/$$\(OBJ\)\1 Makefile/" >> \
		${OBJDIR}/makefile.deps

.PHONY: depsshr
depsshr: $(SHRFILES)
	@${GCC} $(SHRFLAGS) $(SHRINC) $(SHRDEF) -MM $^ | \
		sed -e "s/^\([^ ][^ ]*\)/$$\(OBJ\)\1 Makefile/" >> \
		${OBJDIR}/makefile.deps

.PHONY: depsprc
depsprc: $(PROCFILES)
	@${GCC} $(PROCFLAGS) $(PROCINC) $(PROCDEF) -MM $^ | \
		sed -e "s/^\([^ ][^ ]*\)/$$\(OBJ\)\1 Makefile/" >> \
		${OBJDIR}/makefile.deps

${OBJDIR}/%.o : %.c
	# $@
	@${GCC} $(PAMFLAGS) ${PAMINC} $(PAMDEF) -o $@ $<

${OBJDIR}/%.o : ../../Shared/Src/%.c
	# $@
	@${GCC} $(SHRFLAGS) ${SHRINC} $(SHRDEF) -o $@ $<

${OBJDIR}/%.o : ../../CCB/Src/%.c
	# $@
	@${GCC} $(CCBFLAGS) $(CCBINC) ${CCBDEF} -o $@ $<

${OBJDIR}/%.o : ../../Proc/src/%.c
	# $@
	@${GCC} $(PROCFLAGS) $(PROCINC) ${PROCDEF} -o $@ $<

${OBJDIR}/pam: ${PAM_OBJ} ${SHR_OBJ} ${CCB_OBJ} ${PROC_OBJ} 
	# $@
	@${GCC} ${ON64FLAGS} $(LDFLAGS) -o $@ $^

# ----------------------------------------------------------------------------
.PHONY: tags
tags: ${PAMFILES} ${CCBFILES} ${SHRFILES} ${PROCFILES} \
		../../CCB/Inc/*.h ../../Shared/Inc/*.h
	ctags ${PAMFILES} ${CCBFILES} ${SHRFILES} ${PROCFILES} \
		../../CCB/Inc/*.h ../../Shared/Inc/*.h

#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf *.core core.* *.o *.map ${OBJDIR} tags #makefile.deps .deps

#-----------------------------------------------------------------------------
.PHONY: test
test:
	echo CFLAGS=${CFLAGS}
	echo MAKE_DEFS_CC=${MAKE_DEFS_CC}
	echo MODEL=${MODEL}
#-----------------------------------------------------------------------------

# ${OBJDIR}/fakepam: ${FAKEPAM_OBJ} ${SHR_OBJ} ${PROC_OBJ} 
${OBJDIR}/fakepam: ${FAKEPAM_OBJ} ${PROC_OBJ} 
	# $@
	@${GCC} ${ON64FLAGS} $(LDFLAGS) -o $@ $^
#-----------------------------------------------------------------------------
# Include the compile dependencies.
sinclude ${OBJDIR}/makefile.deps
# ----------------------------------------------------------------------------
