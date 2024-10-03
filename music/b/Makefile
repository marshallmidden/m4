DIRS  = 01
DIRS += 02/21 02/22 02/23 02/24
DIRS += 03
DIRS += 04
DIRS += 06
DIRS += sonata14
# - 05
# ? 07 dyna.abc
#   08
# ? 09
#   10
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create .mid from source(s)."
	@echo "  fs     - create .fs from source(s)."
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
# End of Makefile
