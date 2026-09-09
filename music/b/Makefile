DIRS  = 01
DIRS += 02
DIRS += 03
DIRS += 04
DIRS += 06
DIRS += sonata14
# - 05
# ? 07 dyna.abc
#   08
DIRS += 09
#   10
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create .mid from source(s)."
	@echo "  fs     - create .fs from source(s)."
	@echo "  mp4    - create .mp4 YouTube videos from source(s)."
	@echo "  mp4_2  - create _2.mp4 YouTube videos (imscomp variants)."
	@echo "  sfz    - render to SFZ VPO mix WAV (sfz-mix/)."
	@echo "  sfz-mp4 - encode all SFZ mixes to MP4 video."
	@echo "  sfz-clean - remove SFZ artifacts (sfz-csv/, sfz-mix/)."
#=============================================================================
.PHONY: clean
clean:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d clean ; \
	done
#-----------------------------------------------------------------------------
.PHONY: all
all:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d all ; \
	done
#-----------------------------------------------------------------------------
.PHONY: fs
fs:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d fs ; \
	done
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d mp4 ; \
	done
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d mp4_2 ; \
	done
#-----------------------------------------------------------------------------
.PHONY: sfz
sfz:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d sfz ; \
	done
#-----------------------------------------------------------------------------
.PHONY: sfz-mp4
sfz-mp4:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d sfz-mp4 ; \
	done
#-----------------------------------------------------------------------------
.PHONY: sfz-clean
sfz-clean:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d sfz-clean ; \
	done
#-----------------------------------------------------------------------------
# End of Makefile
