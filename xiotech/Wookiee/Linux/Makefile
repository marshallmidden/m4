# $Id: Makefile 67755 2008-11-04 19:25:01Z m4 $
#
# Makefile for GPL source release tarball.
#

MODEL ?= 3000
GPLSRCTAR := 1

include xio3d/KERNEL_VERSION-${MODEL}

-include xio3d/${KERNBASEVER}/mkdefs-${KMODEL}

KERNREV	?= -0
CDDIR := gpl-src-release-${MODEL}
IMGDIR := /home/image-store

all:
	@echo "Please specify build target:"
	@echo "  gplsrc"

test:
	@echo KERNVER=${KERNVER} KERNBASEVER=${KERNBASEVER}

.PHONY:	all test gplsrc help clean

gplsrc:
	test -d /home/image-store || { \
		echo "${IMGDIR} is not available. Either arrange to"; \
		echo "provide it, or do this on a build system that has it"; \
		exit 1; }
	${MAKE} -C xio3d allclean	# Clean up kernel build area
	rm -rf ${CDDIR}
	mkdir -p ${CDDIR}/{rpms,src}
	${MAKE} -C open-iscsi/usr clean
	${MAKE} -C gpl-packages clean
	cp ${IMGDIR}/Wookiee-gpl-src/linux-${KERNBASEVER}.tar.bz2 ${CDDIR}
	test ! -f ${IMGDIR}/Wookiee-gpl-src/patch-${KERNVER}.bz2 || \
		cp ${IMGDIR}/Wookiee-gpl-src/patch-${KERNVER}.bz2 ${CDDIR}
	cp ${IMGDIR}/Wookiee-gpl-src/* ${CDDIR}/src
	cp ${IMGDIR}/Wookiee-sles9-rpms/* ${CDDIR}/rpms
	tar cjf ${CDDIR}/gplrel.tar.bz2 --exclude=.svn \
		--exclude=.cvsignore --exclude=bld*.out \
		xio3d/{[D-Za-z]*,${KERNBASEVER}} open-iscsi gpl-packages
	@echo "This is the GPL source release for the Xiotech 3D ${MODEL}" \
		"system. The system" > ${CDDIR}/README
	@echo "is based on the SuSE SLES9 distribution, with source" \
		"modifications to some" >> ${CDDIR}/README
	@echo "components." >> ${CDDIR}/README
	@echo >> ${CDDIR}/README
	@echo "The kernel release version is ${KERNVER}-${KMODEL}${KERNREV}." \
		>> ${CDDIR}/README
	@echo >> ${CDDIR}/README
	@echo "The remainder of the source, including the patches made to" \
		"the kernel, " >> ${CDDIR}/README
	@echo "the kernel configuration and other components are included in " \
		"the gplrel.tar.bz2 tarball." >> ${CDDIR}/README
	@echo "SLES9 source packages are in the rpms directory." \
		>> ${CDDIR}/README

	cp /usr/src/linux-${KERNVER}/COPYING ${CDDIR}
	rm -rf gplsrc-${KMODEL}${KERNREV}.iso
	mkisofs -o gplsrc-${KMODEL}${KERNREV}.iso -r -R -V \
		gplsrc-${KMODEL}${KERNREV} ${CDDIR}
	@echo GPL source release iso built as gplsrc-${KMODEL}${KERNREV}.iso

clean:
	rm -rf gpl-src-release-*
	rm -rf gplsrc-*.iso
	rm -rf *.out

help:
	@echo 'This Makefile is used to build a GPL source release CD.'
	@echo 'It is called by "make MODEL=<model> gplsrc".'
	@echo 'A "test" target is provided to display the values of'
	@echo 'some variables for debugging.'

# vi:sw=8 ts=8 noexpandtab
