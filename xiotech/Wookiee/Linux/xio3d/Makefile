# $Id: Makefile 159129 2012-05-12 06:25:16Z marshall_midden $
# Makefile for the Linux kernel with the Xiotech 3D device driver.
# Copyright 2004-2007 Xiotech Corporation. All rights reserved.
# Mark D. Rustad, 2004/03/18
#
# This file uses Linux kernel conventions, which is hard, 8-space tabs,
# do not change that!

##
# Variables that may be passed from the make command line

override CDPATH=
MODEL ?= 3000
include ../../Bin/MMODEL.mk
include KERNEL_VERSION-${MODEL}
J ?= -j 2
KERNPATH ?=/usr/src
KERNBASEVER ?= ${KERNVER}
include ${KERNBASEVER}/mkdefs-${KERNELMODEL}
-include ${KERNBASEVER}/prepatches
KERNREV ?= -x
SBINPROGS ?= logtrim
EXTRAFILES ?= boot/memtest
IMGDIR := /home/image-store
TARDIR := ${IMGDIR}/Wookiee-gpl-src
KERNSTORE := ${IMGDIR}/Wookiee-kernel-store
TARPATH := ${TARDIR}/linux-${KERNBASEVER}.tar.bz2
PATCHPATH := ${TARDIR}/patch-${KERNVER}.bz2
MTVER ?= memtest86+-4.10
KXVER ?= kexec-tools-2.0.0
MODELS := 7000 i7000 3000 i3000 4700 i4700 7400 i7400

##
# Generated variables

space := ${empty} ${empty}
KERNSEP := $(subst -, ,${KERNVER})
KERNPATCH := $(addprefix -,$(subst ${space},-,$(wordlist 2,9,${KERNSEP})))
KERNSUBLVL := $(word 3,$(subst ., ,$(word 1,${KERNSEP})))
KERNSUB := $(addprefix .,$(word 4,$(subst ., ,$(word 1,${KERNSEP}))))
KERNCFG := -${KMODEL}${KERNREV}
KPATCHES := $(addprefix ${KERNBASEVER}/,${PATCHES})
XWD := $(shell pwd)
UWD := $(shell cd ..;pwd)
CMPWD := ${UWD}/components
GPLWD := ${UWD}/gpl-packages
MEMWD := ${GPLWD}/${MTVER}
SCSIWD := ${GPLWD}/${SCSIRASVER}
MDWD := ${GPLWD}/${MDADMVER}
KXWD := ${GPLWD}/${KXVER}/build/sbin
LOGTRIMWD := ${UWD}/logtrim
EUWD := ${CMPWD}/expander-utils

KERNSRC := ${KERNPATH}/linux-${KERNVER}
KERNBASESRC := ${KERNPATH}/linux-${KERNBASEVER}
KERNOBJ := ${XWD}/${KERNBASEVER}/obj-${KERNELMODEL}
CONFINC := ${KERNOBJ}/include/linux/autoconf.h
KERNINST := ${XWD}/${KERNBASEVER}/inst-${KERNELMODEL}
KERNINIT := ${XWD}/${KERNBASEVER}/init-${KERNELMODEL}
FILEDIR-${KMODEL} ?= files-${MODEL}
FILEDIR := ${XWD}/${FILEDIR-${KMODEL}}

XIO3DSRC := ${XWD}
EXTRA  := ${KERNSUB}${KERNPATCH}${KERNCFG}
EXTRAVERSION := EXTRAVERSION=${EXTRA} SUBLEVEL=${KERNSUBLVL}
USRBIN := $(shell echo ~)/bin

EXTRAMAKES := ${LOGTRIMWD} ${EUWD}

GPLMAKES := ${MDADMVER} ${MTVER} ${SCSIRASVER}

##
# Environment variables

KERNID := ${KERNVER}${KERNCFG}
TARNAME := kern-${KERNID}.tar.gz
RAWTARNAME := kern-${KERNID}.tar
SYMTAR := syms-${KERNID}.tar.gz
INSTARGS := INSTALL_PATH=${KERNINST}/boot INSTALL_MOD_PATH=${KERNINST}
KPARM := ${J} -C ${KERNSRC} O=${KERNOBJ} ${EXTRAVERSION}
IPARM := ${KPARM} ${INSTARGS}
MODBUILD :=$(shell grep 'CONFIG_MODULES=y' ${KERNBASEVER}/config-${KERNELMODEL} &> /dev/null && echo modules)

INSTFILES := $(patsubst ./%,%,	\
	$(shell cd ${FILEDIR};find . -name .svn -prune -o -name \\.\#\* -prune \
		-o -type f -print))
INSTFILES := $(filter-out %~ %.bak,${INSTFILES})
ORDIR := $(wildcard ${KERNBASEVER}/override)
ORFILES :=
ifneq (${ORDIR},)
ORFILES += $(patsubst ./%,%,	\
	$(shell cd ${ORDIR};find . -name .svn -prune -o -name \\.\#\* -prune \
		-o -type f -print))
ORFILES := $(filter-out %~ %.bak,${ORFILES})
endif

FILES := $(addprefix ${KERNINST}/sbin/,${SBINPROGS}) \
	$(addprefix ${KERNINST}/,${INSTFILES} ${EXTRAFILES})
DOALL = set -e; for m in ${MODELS}; do ${MAKE} MODEL=$$m ${1}; done

KMODULES := $(shell if grep ^CONFIG_MODULES=y ${KERNBASEVER}/config-${KERNELMODEL} \
	&>/dev/null; then echo yes; else echo no; fi)
ifeq (${KMODULES},yes)
MODDEP := ${KERNINST}/lib/modules/${KERNID}/modules.dep
else
MODDEP :=
endif


##
# Build targets

.PHONY: all
all: tar
	@echo Make all complete.

.PHONY: alltar
alltar:
	$(call DOALL,tar)
	@echo Make alltar complete.

.PHONY: allpatch
allpatch:
	$(call DOALL,patch)
	@echo Make allpatch complete.

.PHONY: allfix_config
allfix_config:
	$(call DOALL,fix_config)
	@echo Make allfix_config complete.

.PHONY: alltest
alltest:
	$(call DOALL,test)
	@echo Make alltest complete.

.PHONY: test
test:
	@echo KERNVER=${KERNVER} KERNPATCH=${KERNPATCH} KERNSUB=${KERNSUB}
	@echo KERNBASEVER=${KERNBASEVER} KERNSUBLVL=${KERNSUBLVL}
	@echo KERNSRC=${KERNSRC} KERNBASESRC=${KERNBASESRC}
	@echo KMODEL=${KMODEL} EXTRA=${EXTRA} EXTRAVERSION=${EXTRAVERSION}
	@echo XIO3DSRC=${XIO3DSRC} XWD=${XWD} TARNAME=${TARNAME}
	@echo INSTARGS=${INSTARGS}
	@echo IPARM=${IPARM}
	@echo EXTRADEPS=${EXTRADEPS}
	@echo INSTFILES=${INSTFILES}
	@echo ORFILES=${ORFILES}
	@echo FILES=${FILES}
	@echo MODBUILD=${MODBUILD}
	@echo TARDIR=${TARDIR}
	@echo SBINPROGS=${SBINPROGS}
	@echo SCSIRASVER=${SCSIRASVER} SCSIWD=${SCSIWD}
	@echo MDADMVER=${MDADMVER} MDWD=${MDWD}
	@echo KMODULES=${KMODULES}


# menuconfig: Use menuconfig to reconfigure kernel
# xconfig: Use xconfig to reconfigure kernel
# oldconfig: Use oldconfig to reconfigure kernel

.PHONY: menuconfig xconfig oldconfig silentoldconfig
menuconfig xconfig oldconfig silentoldconfig:  ${KERNOBJ}/.setup \
		${KERNOBJ}/.patched \
	$(addprefix ${KERNOBJ}/, $(filter %/Kconfig %/Makefile,${ORFILES}))
	./cp-if-change ${KERNBASEVER}/config-${KERNELMODEL} ${KERNOBJ}/.config
	cd ${KERNSRC}; make O=${KERNOBJ} ${EXTRAVERSION} $@
	./cp-if-change ${KERNOBJ}/.config ${KERNBASEVER}/config-${KERNELMODEL}

# fix_config: Feed yes "" to config to get a consistent kernel configuration

.PHONY: fix_config
fix_config:  ${KERNOBJ}/.setup ${KERNOBJ}/.patched \
	$(addprefix ${KERNOBJ}/, $(filter %/Kconfig %/Makefile,${ORFILES}))
	./cp-if-change ${KERNBASEVER}/config-${KERNELMODEL} ${KERNOBJ}/.config
	cd ${KERNSRC}; yes "" | make O=${KERNOBJ} ${EXTRAVERSION} config
	./cp-if-change ${KERNOBJ}/.config ${KERNBASEVER}/config-${KERNELMODEL}


# setup_src - install and prepatch kernel source.

ifneq (${KERNSRC},${KERNBASESRC})
${KERNSRC}/.prepatched: ${KERNBASESRC}/.prepatched
	@if [ ! -d ${TARDIR} ]; then \
		echo Directory ${TARDIR} not found!; exit 1; \
	fi
	@if [ ! -f ${PATCHPATH} ]; then \
		echo File ${PATCHPATH} not found!; exit 1; \
	fi
	root rm -rf ${KERNSRC}
	root cp -rl ${KERNBASESRC} ${KERNSRC}
	cd ${KERNSRC}; bzcat ${PATCHPATH} | root patch -p1
	root touch $@
endif

.PHONY: setup_src
setup_src: ${KERNSRC}/.prepatched
	@echo Kernel source set up in ${KERNSRC}

${KERNBASESRC}/.prepatched:
	rm -f $@
	@if [ ! -d ${TARDIR} ]; then \
		echo Directory ${TARDIR} not found!; exit 1; \
	fi
	@if [ ! -f ${TARPATH} ]; then \
		echo File ${TARPATH} not found!; exit 1; \
	fi
	cd ${KERNBASEVER}; ./prepatch ${KERNBASEVER} ${TARDIR} ${PREPATCHES}
	root touch $@


# patch - patch kernel to allow configuration

.PHONY: patch
patch: ${KERNOBJ}/.setup ${KERNOBJ}/.patched
	@echo Kernel3d patches applied.


# psrc: Build a private source tree containing the entire kernel tree with Xiotech patches 
#       and overrides applied.
#
# Mark's original sequence is:
# make setup_src: prepatch unpacks linux tarball in /usr/src, applies "prepatches"
# make setup_src: stable kernel patch is applied to /usr/src
# make: Xiotech patches are applied to files copied into the object directory
# make: Xiotech overrides are copied into the object directory.
# make: during kernel compile files in the object dir take precedence over files in /usr/src
PSRC_DIR:=$(KERNBASEVER)/linux-$(KERNVER)-$(KMODEL)$(KERNREV)

psrc: $(PSRC_DIR)/.setup

.PHONY: $(PSRC_DIR)/.setup
$(PSRC_DIR)/.setup:
	@if [ -d ${PSRC_DIR} ] ; then \
		echo "Directory ${PSRC_DIR} already exists.  " ; \
		echo "Manually rename or delete if there are no unsaved changes." ; \
		exit 1; \
	fi
	@if [ ! -f ${TARPATH} ]; then \
		echo File ${TARPATH} not found! You may need to update; \
		echo RustadM/linux-kernels in `cd ../../../..;pwd` ; \
		exit 1; \
	fi
	@if [ ! -f ${PATCHPATH} ]; then \
		echo File ${PATCHPATH} not found! You may need to update; \
		echo RustadM/linux-kernels in `cd ../../../..;pwd` ; \
		exit 1; \
	fi
	mkdir -p -m 755 $(PSRC_DIR)
	mkdir -p -m 775 $(PSRC_DIR)/tmp
	bzcat $(TARPATH) | pax -r linux-$(KERNBASEVER) -s :linux-$(KERNBASEVER):$(PSRC_DIR):
	TMPDIR=tmp \
	bzcat ${PATCHPATH} | patch -d $(PSRC_DIR) -p1
	for p in $(PREPATCHES) ; do \
		echo Applying prepatch $(KERNBASEVER)/$$p ; \
		TMPDIR=tmp \
		patch -d $(PSRC_DIR) -p1 < $(KERNBASEVER)/$$p ; \
	done
	for p in $(PATCHES) ; do \
		echo Applying patch $(KERNBASEVER)/$$p ; \
		TMPDIR=tmp \
		patch -d $(PSRC_DIR) -p1 < $(KERNBASEVER)/$$p ; \
	done ;
	sed -e "s/^EXTRAVERSION.*$$/EXTRAVERSION=$(EXTRA)/" < $(PSRC_DIR)/Makefile > $(PSRC_DIR)/Makefile.extra.$$$$ && \
		mv -f $(PSRC_DIR)/Makefile.extra.$$$$ $(PSRC_DIR)/Makefile
	@echo Applying overrides in $(CWD)/$(KERNBASEVER)/override
	cp -ar $(KERNBASEVER)/override/* $(PSRC_DIR)
	find $(PSRC_DIR) -depth -type d -name CVS -exec rm -rf {} \;
	cd $(PSRC_DIR)/drivers/char && ln -sf ../../../../xio3d.c xio3d.c
	cd $(PSRC_DIR)/drivers/char && ln -sf ../../../../xio3d.h xio3d.h
	cp -a ${KERNBASEVER}/config-${KERNELMODEL} $(PSRC_DIR)/.config
	touch $@

# tar: Produce kernel tarball

.PHONY: tar
tar: ${TARNAME}
	echo Kernel3d tarball built.

${TARNAME}: ${KERNINST}/boot/vmlinuz-${KERNID} \
	${KERNBASEVER}/config-${KERNELMODEL} ${FILES} ${MODDEP}
	chmod g-w -R ${KERNINST}
	rm -f ${RAWTARNAME} ${TARNAME}
	tar -cf ${RAWTARNAME} -C ${KERNINST} --group=root --owner=root \
		--exclude boot/System.map-\* --exclude ./opt/xiotech .
	test ! -d ${KERNINST}/opt/xiotech || \
		tar -rf ${RAWTARNAME} -C ${KERNINST} \
		--group=xiotech --owner=xiotech ./opt/xiotech
	gzip -9 ${RAWTARNAME}
	tar -czf ${SYMTAR} -C ${KERNOBJ} --group=root --owner=root \
		--exclude usr --exclude scripts \
		--exclude .\*.cmd --exclude include --exclude include2 \
		--exclude *.txt --exclude \*.[ch] --exclude bzImage \
		--exclude \*.bin --exclude Makefile --exclude Kconfig . \
		-C .. ./config-${KERNELMODEL}


# kernel: Build kernel

.PHONY: kernel
kernel:  ${KERNINST}/boot/vmlinuz-${KERNID}
	@echo Kernel build complete.

${KERNINST}/boot/vmlinuz-${KERNID}: ${KERNOBJ}/.setup ${KERNOBJ}/.patched \
	${XIO3DSRC}/xio3d.c ${XIO3DSRC}/xio3d.h ${EXTRADEPS} \
	$(addprefix ${KERNOBJ}/, ${ORFILES}) \
	${CONFINC}
	mkdir -p ${@D}
	./cp-if-change ${KERNBASEVER}/config-${KERNELMODEL} ${KERNOBJ}/.config
	./cp-if-change ${KERNBASEVER}/installkernel ${USRBIN}/installkernel
	cd ${KERNOBJ}; ${MAKE} CC=${CC} ${IPARM} all
	cd ${KERNOBJ}; ${MAKE} CC=${CC} ${IPARM} install
ifneq (${MODBUILD},)
	${MAKE} CC=${CC} ${IPARM} modules_install
endif
	-rm -f ${@D}/*.old

${CONFINC}:	${KERNOBJ}/.setup ${KERNOBJ}/.patched
	./cp-if-change ${KERNBASEVER}/config-${KERNELMODEL} ${KERNOBJ}/.config
	cd ${KERNOBJ}; ${MAKE} ${IPARM} prepare

${KERNOBJ}/usr/initfiles-${KERNELMODEL}: ${KERNBASEVER}/initfiles-${KERNELMODEL}
	./cp-if-change $< $@

${KERNOBJ}/.patched:
	rm -f $@
ifdef	SUSEKVER
	echo Doing SuSE setup, ${KERNSRC}-obj/${SUSEKVER}
	cp -r ${KERNSRC}-obj/${SUSEKVER}/* ${@D}
	rm -f ${@D}/.config
endif
	cd ${KERNBASEVER}; ./patch ${KERNVER} ${KERNELMODEL} ${PATCHES}
	touch $@

${KERNOBJ}/.setup: ${KERNSRC}/.prepatched ${KERNBASEVER}/patch \
		$(wildcard ${KERNBASEVER}/patches) ${KPATCHES} \
		${KERNBASEVER}/mkdefs-${KERNELMODEL} KERNEL_VERSION-${MODEL}
	rm -f $@
	rm -rf ${@D}
	rm -rf ${KERNINST}
	mkdir -p ${@D}
	touch $@

${KERNINST}/sbin/mdadm: ${MDWD}/mdadm
	./cp-if-change $< $@

${KERNINST}/sbin/sgraidmon: ${SCSIWD}/src/sgraidmon
	echo sgaraidmon SCSIWD="${SCSIWD}"
	./cp-if-change $< $@

${KERNINST}/sbin/logtrim: ${LOGTRIMWD}/logtrim
	./cp-if-change $< $@

${KERNINST}/sbin/shmalloc: shmalloc.c xio3d.h
	gcc -Wall -O2 -m32 -o $@ $<

${KERNINST}/sbin/kexec: ${KXWD}/kexec
	./cp-if-change $< $@

${KERNINST}/sbin/rc%:
	mkdir -p ${@D}
	ln -sf /etc/init.d/$(@:${@D}/rc%=%) $@

${KERNINST}/%: ${FILEDIR}/%
	./cp-if-change $< $@

${KERNOBJ}/%: ${ORDIR}/%
	./cp-if-change $< $@

${KERNINST}/sbin/%: ${EUWD}/%
	./cp-if-change $< $@

${KERNINST}/boot/memtest: ${MEMWD}/memtest
	./cp-if-change $< $@

${SCSIWD}/src/sgraidmon::
	${MAKE} -C ${GPLWD} PKG=${SCSIRASVER}

${MDWD}/mdadm::
	${MAKE} -C ${GPLWD} PKG=${MDADMVER}

${MEMWD}/memtest::
	${MAKE} -C ${GPLWD} PKG=${MTVER}

${KXWD}/kexec::
	${MAKE} -C ${GPLWD} PKG=${KXVER}

${LOGTRIMWD}/logtrim::
	${MAKE} -C ${@D}

${EUWD}/dtemp::
	${MAKE} -C ${@D} ${@F}

${EUWD}/smpgetphydata::
	${MAKE} -C ${@D} ${@F}

${EUWD}/smpsetspeed::
	${MAKE} -C ${@D} ${@F}

${EUWD}/smptest::
	${MAKE} -C ${@D} ${@F}

${EUWD}/sg_fwupdate::
	${MAKE} -C ${@D} ${@F}


##
#
.PHONY: allinstall
allinstall:
	$(call DOALL,install)
	@echo Make allinstall complete

.PHONY: install
install: ${TARNAME}
	test -d ${KERNSTORE} || { echo "${KERNSTORE} not found"; exit 1; }
	root mv ${SYMTAR} ${KERNSTORE}
	root chown root:root ${KERNSTORE}/${SYMTAR}
	root chmod -w ${KERNSTORE}/${SYMTAR}
	mv -f ${TARNAME} ../kernels/${TARNAME}
	@echo ''
	@echo "Don't forget to do:"
	@echo '   "cd ../kernels"'
	@echo '   "svn add '${TARNAME}'"'
	@echo '   "svn commit '${TARNAME}'"'
	@echo 'if that is appropriate at this time.'
	@echo ''

##
#
.PHONY: doc
doc: HOWTO-build-kernels.pdf

%.pdf: %.ps
	ps2pdf $< $@

%.ps: %.ms
	groff -ms $< > $@

##
# Utility targets

.PHONY: allclean
allclean:
	$(call DOALL,clean)
	@echo Make allclean complete.

.PHONY: clean
clean:
	-rm -rf ${KERNOBJ}
	-rm -rf ${KERNINST}
	-rm -rf ${KERNINIT}
	-rm -f ${TARNAME}
	-rm -f ${SYMTAR}
	-for d in ${EXTRAMAKES}; do ${MAKE} -C $${d} clean || \
		echo "$${d} probably already clean"; done
	-for d in ${GPLMAKES}; do ${MAKE} -C ${GPLWD} PKG=$${d} clean || \
		echo "$${d} probably already clean"; done

.PHONY: cleandir
cleandir:
	-rm -rf */obj-*
	-rm -rf */inst-*
	-rm -rf */init-*
	-rm -f kern-*.tar.gz
	-rm -f syms-*.tar.gz
	-rm -f *.pdf

.PHONY: distclean
distclean: cleandir
#	root rm -rf ${KERNSRC}

.PHONY: cscope
cscope:
	$(MAKE) -C $(KERNOBJ) cscope

.PHONY: TAGS
TAGS:
	$(MAKE) -C $(KERNOBJ) TAGS

.PHONY: help
help:
	@echo 'Cleaning targets:'
	@echo '  clean          - remove generated files for one MODEL'
	@echo '  allclean       - remove generated files for all MODELs'
	@echo '  cleandir       - remove most generated files'
	@echo '  distclean      - remove kernel source as well'
	@echo
	@echo 'Configuration targets:'
	@echo '  setup_src      - install and prepatch kernel source tree'
	@echo '  patch          - patch kernel for one MODEL'
	@echo '  allpatch       - patch kernel for all MODELs'
	@echo '  menuconfig     - reconfigure the kernel'
	@echo '  oldconfig      - to do an oldconfig on the configuration'
	@echo '  fix_config     - make config consistent for one MODEL'
	@echo '  allfix_config  - make config consistent for all MODELs'
	@echo '  psrc      	- create a private kernel source tree for one MODEL'
	@echo
	@echo 'Compilation targets:'
	@echo '  tar        - Build kernel tarball for one MODEL'
	@echo '  alltar     - Build kernel tarball for all MODELs'
	@echo '  kernel     - build (and compile) kernel'
	@echo '  install    - Install tarballs into ../kernels for one MODEL'
	@echo '  allinstall - Install tarballs into ../kernels for all MODELs'
	@echo
	@echo 'Documentation:'
	@echo '  doc     - Generate documentation as pdf'
	@echo
	@echo 'Suggested targets:'
	@echo '  clean patch fix_config menuconfig tar'
	@echo
	@echo 'Target model is set using MODEL variable, as in:'
	@echo '  make MODEL=7000 tar'

# vi:sw=8 ts=8 noexpandtab
