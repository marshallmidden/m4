# $Id: Makefile 159214 2012-06-04 00:11:40Z marshall_midden $
#
# Makefile for install CDs.
#
# Copyright 2006-2010 XIOtech Corporation. All rights reserved.
#
# Mark D. Rustad, 9/11/2006

MODEL ?= 3000
VER ?= ENGR
MODELS := 7000 3000 4700 7400

include ../../Bin/MMODEL.mk

# Get release info from FWRun-${MODEL}.ver
include ../../FWRun-${MODEL}.ver
MAJ_REL := $(subst 0x00,,${FW_MAJOR_RELEASE})
MIN_REL := $(patsubst 0x%00,%,${FW_MINOR_RELEASE})
RELEASE := ${MAJ_REL}${MIN_REL}

ROOTDIR := tmp-${MODEL}-root
GPLWD := ../gpl-packages
IMGDIR := /home/image-store/Wookiee-blobs

UVER := $(shell echo ${VER}|tr [:lower:] [:upper:])

include ${MODEL}-base/mkdefs

PTYPE := $(shell z=${VER};zu=${UVER}; test $${z:0:1} = $${zu:0:1} && echo Perf)

ifeq (${PTYPE},)
PTYPE := Debug
endif

PLATFORMDIR := $(shell cd ../..;pwd)/built-${MODEL}${PTYPE}

IMODEL ?= ${MODEL}

include ../xio3d/KERNEL_VERSION-${IMODEL}

KERNEL := ${KERNVER}-${KMODEL}${KERNREV}

VOLID := ${MODEL}-${RELEASE}-${VER}
ISOID := PS-install-${MMODEL}-${RELEASE}-${VER}
ISOLINUXFILES := memtest
ISOLINUXFILES += initrd.gz isolinux.bin
ISOLINUXFILES += $(filter-out .svn,$(notdir $(wildcard ${MODEL}-base/isolinux/*)))
ISOLINUXFILES += message.msg linux
PLRPMPATH := $(wildcard ${PLATFORMDIR}/platformApps-${MMODEL}-*-${VER}.rpm)
PLRPM := $(notdir ${PLRPMPATH})
ROOTFILES := ${VOLID} VERSION rd-base.img.bz2
ROOTFILES += specialize.tar.bz2
ROOTFILES += ${PLRPM}
ROOTFILES += ${TARS}
ROOTFILES += COPYRT
SPECFILES := etc
APPPATH := eng_apps_build/XWS_MAIN_INFO_Build_STABLE
TMPLIB := tmp-${MODEL}-lib

DOMAKE = ${MAKE} VER=${VER} ${1}
DOALL = set -e; for m in ${MODELS}; do $(call DOMAKE,MODEL=$$m ${1}); done

.DELETE_ON_ERROR:

.PHONY: all
all: cd

.PHONY: test
test:
	@echo ROOTFILES=${ROOTFILES}
	@echo ISOLINUXFILES=${ISOLINUXFILES}
	@echo ROOTDIR=${ROOTDIR} PLRPM=${PLRPM}
	@echo PTYPE=${PTYPE} UVER=${UVER}

.PHONY: cd
cd: ${ISOID}.iso

${ISOID}.iso: $(addprefix ${ROOTDIR}/isolinux/,${ISOLINUXFILES}) \
	$(addprefix ${ROOTDIR}/,${ROOTFILES})
	@test "${PLRPM}" != "" || \
		{ echo No platform RPM found in ${PLATFORMDIR}!; exit 1; }
	mkisofs -J -r -T -P "Xiotech Corporation, www.xiotech.com" \
		-V "${VOLID}" -o $@ -b isolinux/isolinux.bin \
		-c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
		-copyright COPYRT -boot-info-table ${ROOTDIR}
	@echo "Checking image..."
	isovfy $@

${ROOTDIR}/isolinux/linux: ../kernels/kern-${KERNEL}.tar.gz
	tar xzf $< -C ${ROOTDIR} ./boot/vmlinuz-${KERNEL}
	mv ${ROOTDIR}/boot/vmlinuz-${KERNEL} $@
	rm -rf ${ROOTDIR}/boot

${ROOTDIR}/isolinux/memtest: ${GPLWD}/${MTVER}/memtest.bin
	mkdir -p ${@D}
	cp $< $@

${ROOTDIR}/isolinux/message.msg: ${MODEL}-base/pieces/message1.msg \
		${APPS} ${MODEL}-base/pieces/message2.msg
	sed -e 's/%%%%/${VER}/' < $< > $@
ifneq (${APPS},)
	sed -e 's/^Xiotech-//' -e 's/\\.rpm//' < ${APPS} >> $@
endif
	cat ${MODEL}-base/pieces/message2.msg >> $@

${ROOTDIR}/isolinux/initrd.gz:	${IMGDIR}/${INITRD}
	cp $< $@

${ROOTDIR}/rd-base.img.bz2:	${IMGDIR}/${RDBASE}
	cp $< $@

${ROOTDIR}/%: ${IMGDIR}/${MODEL}-cd-${TARREV}/%
	cp $< $@

${ROOTDIR}/%: ${IMGDIR}/%
	cp $< $@

${ROOTDIR}/%: ${MODEL}-base/%
	cp $< $@

${ROOTDIR}/%: common-base/%
	cp $< $@

${ROOTDIR}/COPYRT::
	@echo -n "XIOtech model ${MMODEL} install CD, " > $@
	@echo "Release ${RELEASE}, Version ${VER}." >>$@
	@echo -n "Copyright 2008-2010 XIOtech Corporation. " >>$@
	@echo "All rights reserved." >>$@
	@echo "www.xiotech.com" >>$@

${ROOTDIR}/specialize.tar.bz2: $(addprefix ${MODEL}-base/,${SPECFILES}) \
		$(addprefix common-base/,${SPECFILES}) ${TMPLIB}
	tar cjf $@ --exclude=.svn --owner=root --group=root \
		-C common-base ${SPECFILES} -C ../${MODEL}-base ${SPECFILES} \
		-C ../${TMPLIB} .

${TMPLIB}:: ../kernels/kern-${KERNEL}.tar.gz ${EXTRAFILES}
	rm -rf $@
	mkdir -p $@/lib/modules $@/etc/rc.d
	for d in $(addprefix $@,$(filter-out ${<D},${^D})); do mkdir -p $$d; done
	for f in $(filter-out $<,$^); do cp $$f ${@}/`dirname $$f`; done
	tar xzf $< -C $@ ./lib/modules || rm -rf ${@}/lib/modules
	echo "VER=${VER}" >> $@/etc/rc.d/rc.inits

${ROOTDIR}/%:	${PLATFORMDIR}/%
	rm -rf platformApps-*.rpm
	cp $< $@

ifneq (${APPS},)
${ROOTDIR}/app-versions:
	echo 'Type password to public@epitengd01 if prompted'
	rm -rf ${ROOTDIR}/Xiotech-*.rpm
	if scp public@epitengd01:${APPPATH}/Xiotech-\*.rpm ${ROOTDIR}; then \
		echo 'App rpms loaded:'; cd ${ROOTDIR}; ls Xiotech-*.rpm; \
	else echo 'scp failed'; exit 1; fi
	(cd ${ROOTDIR}; ls Xiotech-*.rpm) > $@
endif

${ROOTDIR}/${VOLID}:
	rm -rf ${ROOTDIR}/PS-install-${MMODEL}-*
	: > $@

${ROOTDIR}/VERSION:
	echo "${ISOID}" > $@

${GPLWD}/${MTVER}/memtest.bin::
	${MAKE} -C ${GPLWD} PKG=${MTVER}

.PHONY: allcd
allcd:
	$(call DOALL,cd)
	@echo "Make allcd complete."

.PHONY: clean
clean:
	-rm -f *.iso
	-root rm -rf tmp-*

.PHONY: allclean
allclean:
	$(call DOALL,clean)
	@echo "Make allclean complete."

.PHONY: help
help:
	@echo 'Targets in this Makefile:'
	@echo '  help:	Shows this help'
	@echo '  clean: Removes built files for all MODELs'
	@echo '  allcd: Make CD for all models'
	@echo '  allclean: Removes built files for all models'
	@echo '  all (default): Build install CD, see below:'
	@echo '      To build an install CD, specify value for the MODEL'
	@echo '      and VER variables. For example:'
	@echo '          make MODEL=7000 VER=M009'
	@echo '      will make an install CD for the 7000 model with the M009'
	@echo '      version of the platform code.'

# vi:sw=8 ts=8 noexpandtab
