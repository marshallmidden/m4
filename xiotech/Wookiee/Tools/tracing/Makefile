# $Id: Makefile 148621 2010-10-05 17:49:17Z m4 $
#
##############################################################################
##
##  Copyright 2006 - 2010 Xiotech Corporation. All rights reserved.
##    
##############################################################################

##############################################################################
# Force the default target to be "all".  This allows targets to be put in the
# ~/.xiomake file.
all:

# remove default suffix rules
.SUFFIXES:

##############################################################################
# Files that if they change imply a remake of everything.
USERXIOMAKE := $(wildcard ~/.xiomake )
USERXIOMAKE += $(wildcard ~/.xiomake.tracing )
USERXIOMAKE += Makefile
USERXIOMAKE += $(wildcard Makefile.CFLAGS)
##############################################################################
## Include local (user) modifications.
sinclude ~/.xiomake
sinclude ~/.xiomake.tracing

##############################################################################
## Type of build.
BTYPE ?= DEBUG

##############################################################################
## Object location
OBJ_DEBUG ?= obj-debug
OBJ_PERF  ?= obj-perf

ifeq ($(BTYPE),DEBUG)
    OBJ_DIR:=$(OBJ_DEBUG)
else
    OBJ_DIR:=$(OBJ_PERF)
endif

ifneq ($(OBJ),)
   OBJ_DIR:=$(OBJ)
endif

# make sure OBJ_DIR ends with a slash
ifeq ($(OBJ_DIR),$(OBJ_DIR:%/=%))
    OBJ_DIR:=$(OBJ_DIR)/
endif

##############################################################################
## Defines
ifeq ($(BTYPE),DEBUG)
DEFINES += -DXASSERT_FATAL
else
DEFINES += -DXASSERT_NON_FATAL
endif

##############################################################################
# Libraries not default with the system.
LIBRARIES += 

##############################################################################
## Include Directories
INCLUDES += -I./
INCLUDES += -I../../Proc/inc
INCLUDES += -I../../Shared/Inc
INCLUDES += -I../../Proc/obj_3000Debug/

##############################################################################
## Source directories
#VPATH += ../common/debug

##############################################################################
## Source Files
## (this) directory
FRT_SOURCES += frtview.cpp

IST_SOURCES += istview.cpp

##############################################################################
## Create object files from source .c filenames.
FRT_OBJ_FILES := $(FRT_SOURCES:%.cpp=${OBJ_DIR}%.o)
IST_OBJ_FILES := $(IST_SOURCES:%.cpp=${OBJ_DIR}%.o)
OBJ_FILES := $(FRT_OBJ_FILES) $(IST_OBJ_FILES)
INCLUDE_DEPEND := $(OBJ_FILES:${OBJ_DIR}%.o=${OBJ_DIR}.%.d)

##############################################################################
## Compiler Flags
CC := g++
CFLAGS += -c
CFLAGS += -g
ifeq ($(BTYPE),DEBUG)
OPT ?= -O0
else
# OPT ?= -O0
# OPT ?= -O1
# OPT ?= -O2
# OPT ?= -O3
OPT ?= -O4
endif
CFLAGS += $(OPT) -Wall
CFLAGS += -DFLIGHTRECORDER -DXIO_XWS

## The -W flags are in a separate file (to shorten this one).
sinclude Makefile.CFLAGS

##############################################################################
## Linker Flags
LDFLAGS += -Wl,-Map,$@.map -static

##############################################################################
## Targets
##############################################################################
all: frtview

$(OBJ_DIR): 
	@mkdir -pv $(OBJ_DIR)

# intended for use in .xiomake
.PHONY: pre-frtview post-frtview
pre-frtview:
post-frtview:

.PHONY: frtview
frtview: $(OBJ_DIR) pre-frtview $(OBJ_DIR)frtview post-frtview

# intended for use in .xiomake
.PHONY: pre-istview post-istview
pre-istview:
post-istview:

.PHONY: istview
istview: $(OBJ_DIR) pre-istview $(OBJ_DIR)istview post-istview

##############################################################################
$(OBJ_DIR)frtview: $(FRT_OBJ_FILES) ${LIBRARIES}
	@echo Linking: $@
	@$(CC) $(LDFLAGS) -o $@ $^
	@nm -n -l $@ > $@.nm
	@echo SUCCESS: $(@:${OBJ_DIR}%=%)

$(OBJ_DIR)istview: $(IST_OBJ_FILES) ${LIBRARIES}
	@echo Linking: $@
	@$(CC) $(LDFLAGS) -o $@ $^
	@nm -n -l $@ > $@.nm
	@echo SUCCESS: $(@:${OBJ_DIR}%=%)

##############################################################################
.PHONY: clean cleandir
cleandir:
	@rm -rf -- $(OBJ_DEBUG) $(OBJ_PERF) $(OBJ_DIR)
	@rm -rf -- $(DOXY_OBJ)
	@echo "frtview: make $@ complete for $(OBJ_DEBUG) $(OBJ_PERF) $(OBJ_DIR)"

clean:
	@rm -f $(OBJ_FILES) $(OBJ_DIR)frtview $(INCLUDE_DEPEND) $(OBJ_DIR)frtview.map $(OBJ_DIR)frtview.nm
	@rm -f $(OBJ_FILES) $(OBJ_DIR)istview $(INCLUDE_DEPEND) $(OBJ_DIR)istview.map $(OBJ_DIR)istview.nm
	@rm -rf -- $(DOXY_OBJ)
	@echo "frtview: make $@ complete for $(OBJ_DIR)"

##############################################################################
$(DOXY_OBJ):
	@mkdir -pv $(DOXY_OBJ)

.PHONY: doc
doc: $(DOXY_OBJ)
	@export DOXY_OBJ=$(DOXY_OBJ); doxygen doxyfile

##############################################################################
# print out variable expansions (e.g. `make expand VAR=CFLAGS`)
# note that some special vars such as $@ won't be set,
# and thus won't be expanded properly.
.PHONY: expand
expand: EXPANDED_VAR:=$($(VAR))
expand:
	@echo $(EXPANDED_VAR)

##############################################################################
## Rules
##############################################################################
$(OBJ_DIR)%.o : %.c ${USERXIOMAKE}
	@echo Compiling: $(@:${OBJ_DIR}%.o=%.c)
	@$(CC) -MD -MF $(@:${OBJ_DIR}%.o=${OBJ_DIR}.%.d) \
	       $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

$(OBJ_DIR)%.o : %.cpp ${USERXIOMAKE}
	@echo Compiling: $(@:${OBJ_DIR}%.o=%.cpp)
	@$(CC) -MD -MF $(@:${OBJ_DIR}%.o=${OBJ_DIR}.%.d) \
	       $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

##############################################################################
## Include the compile dependencies.
##############################################################################
sinclude ${INCLUDE_DEPEND}

# ----------------------------------------------------------------------------
# Modelines (Makefiles must have tabs.)
# vi:sw=4 ts=4 noexpandtab
# End of Makefile
