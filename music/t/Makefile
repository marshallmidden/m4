DIRS = e
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create .mid from source(s)."
	@echo "  fs     - create .fs from source(s)."
	@echo "  mp4    - create .mp4 YouTube videos from source(s)."
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
# End of Makefile
