###############################################################################
#                         CONFIDENTIAL AND PROPRIETARY
#          Copyright (C) 2008 Xiotech Corporation, All Rights Reserved
###############################################################################
#
# \file     ExportRules.mk
# \author   Mark Rustad
#
# Wookiee tool rules.
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

ifndef DEPS
$(error DEPS variable is required)
endif

# Self dependency

ALL_EXPORT_RULES += $(WOOKIEEDIR)/ExportRules.mk

# Rules

$(DEPS): $(WOOKIEE_HEADERS_GENERATED)

$(WOOKIEE_HEADERS_GENERATED):    $(WOOKIEE_DEFS)
	$(PreBuild)
	$(Build_DEF)
	$(PostBuild)


# vi:sw=4 ts=4 noexpandtab
