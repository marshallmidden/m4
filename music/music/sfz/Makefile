#-----------------------------------------------------------------------------
# music/sfz holds the SFZ render scripts (gcs2sfz, instruments.py), the VPO
# sample library, and scratch benchmark dirs.  The scratch dirs have no
# Makefile of their own, so their compiler output was invisible to the piece
# Makefiles' clean rules.  This target removes the generated files only --
# .gcs sources and the instruments.include/macros.include symlinks stay.
#
#   make -C music/sfz clean
#-----------------------------------------------------------------------------
SCRATCH_DIRS = dynprobe
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  clean - Remove generated files from music/sfz scratch dirs (${SCRATCH_DIRS}).'
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	@for d in ${SCRATCH_DIRS} ; do \
	    if [ -d $$d ] ; then \
	        echo "=== clean $$d ===" ; \
	        rm -f $$d/*.E $$d/*.fs $$d/*.abc $$d/*.csv $$d/*.mid $$d/*.h $$d/*.v ; \
	        rm -f $$d/*.ps $$d/*.pdf $$d/*.wav $$d/*.mp4 $$d/*.stdout $$d/*.stderr ; \
	        rm -rf $$d/out $$d/sfz-csv $$d/sfz-mix ; \
	    fi ; \
	done
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------