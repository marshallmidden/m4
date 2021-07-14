DIRS = 01 06
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
	@echo "  s      - create .mid from source(s)."
#=============================================================================
.PHONY: clean
clean:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d clean ; \
	done
#-----------------------------------------------------------------------------
.PHONY: s
s:
	for d in ${DIRS} ; do \
	    ${MAKE} -C $$d s ; \
	done
#-----------------------------------------------------------------------------
# End of Makefile
