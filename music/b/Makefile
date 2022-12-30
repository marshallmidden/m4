DIRS = 01 02/21 02/22 06
#   02
# - 03
#   04
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
