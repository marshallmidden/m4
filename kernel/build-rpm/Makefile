#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all           - Get sources (if needed), configure, install, clean.'
	@echo '  distclean     - Remove sources.'
	@echo '  clean         - Remove source for tools (not gcc and friends).'
	@echo '  unpack        - Unpack sources (not gcc and friends).'
	@echo '  compile       - Compile sources (not gcc and friends).'
#-----------------------------------------------------------------------------
.PHONY: all
all:
	./01.new-gcc+rpm+etc
#-----------------------------------------------------------------------------
unpack:
	./03.unpack.tools
#-----------------------------------------------------------------------------
compile:
	./20-libgcrypt
	./21-help2man
	./22-libarchive
	./23-libmicrohttpd
	./24-elfutils
	./25-debugedit
	./26-file
	./27-lua
	./28-rpm
#-----------------------------------------------------------------------------
clean:
	./98.tools-clean
#-----------------------------------------------------------------------------
.PHONY: distclean
distclean:
	./99.remove-sources
#-----------------------------------------------------------------------------
# End of file Makefile
#-----------------------------------------------------------------------------
