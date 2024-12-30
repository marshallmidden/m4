# $Id$
#
# MMODEL.mk - Map model number to marketing name
#
# Copyright 2008 Xiotech Corporation. All rights reserved.
#
# Mark D. Rustad, 2008/01/29.
#
# MMODEL.mk is included in Makefiles to map the value of ${MODEL} into
# a marketing name ${MMODEL}. This also sets MODEL_TYP appropriately.

ifndef MODEL
$(error Variable MODEL must be defined)
endif

# Check for models that need change.

ifeq (${MODEL},7000)
MMODEL:=Emprise_${MODEL}
endif

ifeq (${MODEL},3000)
MMODEL:=4000
endif

# If MMODEL is not yet defined, default to using MODEL.

ifndef MMODEL
MMODEL:=${MODEL}
endif

# Handle setting MODELTYP

ifeq (${MODEL},750)
MODELTYP := MAG_750
endif

ifeq (${MODEL},7000)
MODELTYP := EMPRISE_${MODEL}
endif

# If not yet set, default to MAG_3D_${MMODEL}

ifndef MODELTYP
MODELTYP:=MAG_3D_${MMODEL}
endif

# vi:sw=4 ts=4 noexpandtab
