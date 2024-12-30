###############################################################################
#                         CONFIDENTIAL AND PROPRIETARY
#          Copyright (C) 2008 Xiotech Corporation, All Rights Reserved
###############################################################################
#
# \file     ExportTools.mk
# \author   Mark Rustad
#
# Wookiee tool exports.
#
# A project unlucky enough to need to include Wookiee definitions is
# encouraged to include this file in its Makefile.
#
###############################################################################

# Input checks

ifndef WOOKIEEDIR
$(error WOOKIEEDIR variable is required)
endif

ifndef LOBJDIR
$(error LOBJDIR variable is required)
endif

# Exported variables

# Self dependency

ALL_EXPORT_TOOLS += $(WOOKIEEDIR)/ExportTools.mk

# Include path

INCPATH +=  -I${WOOKIEEDIR}/CCB/Inc
INCPATH +=  -I${WOOKIEEDIR}/Shared/Inc -I${WOOKIEEDIR}/Proc/inc
INCPATH +=  -I${LOBJDIR}

# Defined symbols

DEFSYMB +=  -DHYPERNODE -DLINUX_VER -DCCB_RUNTIME_CODE

WOOKIEE_DEFS := $(wildcard ${WOOKIEEDIR}/Shared/Inc/*.def)
WOOKIEE_DEFS += $(wildcard ${WOOKIEEDIR}/Proc/inc/*.def)

# No longer a dotfile, since that prevents rm $(LOBJDIR)/* from regenerating headers.
WOOKIEE_HEADERS_GENERATED = $(LOBJDIR)/wookiee_headers_generated

define Build_DEF
@for f in $?; do ${WOOKIEEDIR}/Bin/makeinc.pl -o ${@D} $$f; done
@echo "Built $(DISPLAY_CWD)/$@"
@touch $@
endef

# vi:sw=4 ts=4 noexpandtab
