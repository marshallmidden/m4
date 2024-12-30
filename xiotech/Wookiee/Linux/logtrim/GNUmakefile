# $Id$
#
# GNUmakefile
#
# Mark D. Rustad	2009/02/20
#
# Copyright 2009-2010 XIOtech Corporation. All rights reserved.
#
# This makefile acts as a wrapper around a normal makefile. If build
# environments exist, it arranges to chroot into the appropriate
# environment, otherwise it proceeds as usual.

MAKECMDGOALS ?= all
TARGET_ENV := sles9-32-Wookiee
ENV_DIR := $(wildcard /env/${TARGET_ENV})

.PHONY:	${MAKECMDGOALS}
ifneq (,${ENV_DIR})
${MAKECMDGOALS}:
	runenv "${TARGET_ENV}" ${MAKE} -f Makefile ${MAKECMDGOALS}
else
${MAKECMDGOALS}:
	@${MAKE} -f Makefile ${MAKECMDGOALS}
endif

