# $Id: Makefile 73133 2009-01-12 17:19:24Z mdr $
#
# Makefile for the BIOS update CD
#
# Copyright (c) 2005-2008 Xiotech Corporation. All rights reserved.
#
# Mark D. Rustad, 11/16/2005

MODEL ?= 7000
BOARDDIR := boards
ISODIR := iso-${MODEL}
MBTMPDIR := tmp-${MODEL}
ISDIR := /home/image-store
HWDIR := ${ISDIR}/Wookiee-hardware
DOSDIR := ${HWDIR}/dos
ISOFILE := BIOSCD-${MODEL}-$(shell date +%Y%m%d).iso

include mkdefs-${MODEL}

BRDPATHS := $(shell find ${BOARDDIR}/${MBID} -name .svn -prune -o -name .\* \
		-prune -o -type f -print)
BRDFILES := $(patsubst ${BOARDDIR}/%,${MBTMPDIR}/%,${BRDPATHS})
BRDFILES += ${MBTMPDIR}/${MBID}/${BIOSROM}
PROGS := boardid.exe phlash16.exe
PROGSDEP := $(addprefix ${MBTMPDIR}/bin/, ${PROGS})

VOLID := XIOBIOS${MODEL}
DEPS := ${ISODIR}/BootImage.ima .boot-${MODEL} ${ISODIR}/${VOLID}
DEPS += ${PROGSDEP}
BOOTIMG := ${DOSDIR}/BootImage288.ima
APPLICATION := "Xiotech ${MODEL} BIOS setup CD"
PUBLISHER := "Xiotech Corporation, www.xiotech.com"

all:	${ISOFILE}

test:
	@echo ISODIR=${ISODIR}
	@echo DEPS=${DEPS}
	@echo BRDPATHS=${BRDPATHS}
	@echo BRDFILES=${BRDFILES}
	@echo MBTMPDIR=${MBTMPDIR}
	@echo PROGSDEP=${PROGSDEP}
	@echo BIOSROM=${BIOSROM}

${ISOFILE}:	${DEPS}
	rm -f $@ ${ISODIR}/boot.catalog
	mkisofs -J -r -R -T -A ${APPLICATION} -P ${PUBLISHER} -V ${VOLID} \
		-o $@ -b BootImage.ima -c boot.catalog ${ISODIR} || \
			{ rm -f $@; echo mkisofs failed; exit 255; }

${MBTMPDIR}/bin/%.exe:	src/%.c ${MBTMPDIR}/bin
	gcc-dos -Wall -Werror -O2 -o $@ $<

${MBTMPDIR}/bin/phlash16.exe: ${BIOSZIP} ${MBTMPDIR}/bin
	cd ${@D}; unzip -LL -C -j ${BIOSZIP} ${@F}

${MBTMPDIR}/${MBID}/${BIOSROM}: ${BIOSZIP} ${MBTMPDIR}/${MBID}
	rm -f ${@F} ${BIOSROM}
	cd ${@D}; unzip -LL -C -j ${BIOSZIP} ${BIOSROM}
#	cd ${@D}; mv ${BIOSROM} ${@F}

${MBTMPDIR}/bin:
	mkdir -p $@

${ISODIR}/BootImage.ima: ${BOOTIMG}
	./cp-if-change $< $@
	chmod u+w $@
	-rm -f .boot-${MODEL}

.boot-${MODEL}:	${ISODIR}/BootImage.ima ${BRDFILES} doupdate.bat ${PROGSDEP}
	mcopy -D o -i ${ISODIR}/BootImage.ima -s ${MBTMPDIR}/${MBID} ::
	mcopy -D o -i ${ISODIR}/BootImage.ima -s ${MBTMPDIR}/bin ::
	mcopy -D o -i ${ISODIR}/BootImage.ima doupdate.bat ::\\bin
	mlabel -i ${ISODIR}/BootImage.ima -nv ::XIO${MODEL}
	touch $@

${ISODIR}/${VOLID}:
	mkdir -p ${@D}
	touch $@

${MBTMPDIR}/${MBID}/bios.bat: ${BOARDDIR}/${MBID}/bios.bat
	mkdir -p ${@D}
	sed s/BIOS.ROM/${BIOSROM}/ < $< > $@

${MBTMPDIR}/%: ${BOARDDIR}/%
	./cp-if-change $< $@

clean:
	rm -rf iso-* BIOSCD-*.iso tmp-* .boot-*

.PHONY: all test clean


# vi:ts=4 sw=4 noexpandtab
