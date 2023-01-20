#-----------------------------------------------------------------------------
SONGS  = abc.testing
#-----------------------------------------------------------------------------
SONGS_FS = ${SONGS:=.fs}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ./tokens
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -nostdinc -C -CC -Wno-error -Wno-extra -traditional-cpp
#-----------------------------------------------------------------------------
.SUFFIXES: fs
# output $@   -- input $<
%.fs:  %.gcs
	${MUSICOMP2ABC} --two --fluidsynth $< $@ || rm -f $@
	rm -f $@
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
	@echo '  tags      - Create tags file for python sources.'
	@echo "SONGS: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all songs ${SONGS}
all songs ${SONGS}: ${SONGS_FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS}
#-----------------------------------------------------------------------------
.PHONY: check
check:
	echo "SONGS=${SONGS}"
	echo "SONGS_FS=${SONGS_FS}"
	echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python tokens calculate.py
#-----------------------------------------------------------------------------
