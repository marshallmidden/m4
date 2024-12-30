# $Id: Makefile 16265 2006-12-27 22:28:00Z mdr $
#
# Makefile for gpl-packages
#
# Copyright 2006 Xiotech Corporation. All rights reserved.
#
# Mark Rustad, 12/15/2006
#

PKGDIR := /home/image-store/Wookiee-gpl-src
PATCH := scripts/${PKG}.patch
BUILD := scripts/${PKG}.build
BLDDEP := $(wildcard scripts/${PKG}.build)

all:
	@test "${PKG}" || { echo 'PKG undefined'; exit 1; }
	${MAKE} PKG=${PKG} .bld-${PKG}

.bld-${PKG}: .patch-${PKG} ${BLDDEP}
	if [ -x ${BUILD} ]; then ${BUILD} ${PKGDIR}; else \
		cd ${PKG} && if [[ ! -f Makefile ]]; then ./configure; fi; \
		${MAKE}; fi
	touch $@

.patch-${PKG}:	${PATCH}
	rm -rf .patch-${PKG} .bld-${PKG} ${PKG}
	@test -d ${PKGDIR} || { echo "PKGDIR (${PKGDIR}) missing"; exit 1; }
	${PATCH} ${PKGDIR}
	touch $@

.PHONY: test
test:
	@echo MAKECMDGOALS=${MAKECMDGOALS} PKG=${PKG} PATCH=${PATCH}

.PHONY: clean
clean:
	rm -rf .bld-* .patch-* *-*.* built-*

#
# vi:ts=8 sw=8 noexpandtab
