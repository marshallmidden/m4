#-----------------------------------------------------------------------------
SONGS  = 18notes
SONGS += 1963
SONGS += Chromatic
#-- DOESN'T WORK	SONGS += Djokja
SONGS += Entroid
SONGS += Entroid_
SONGS += Horizons
SONGS += Kanon
SONGS += Miditest
SONGS += Poppers
SONGS += Roundoff
SONGS += changes-idea
SONGS += char-print
#-- DOESN'T WORK	SONGS += e-minor
SONGS += fru-uhlingserwachen
SONGS += idea
SONGS += joy-piano-part
SONGS += ma-solitude
SONGS += maple-leaf-rag
SONGS += miscellaneous-ideas
SONGS += mozart
SONGS += opus10-chopin
SONGS += opus8-scriabin
SONGS += random-chords
SONGS += replot
SONGS += revolution-midi
SONGS += song-without-words
SONGS += theme
#-----------------------------------------------------------------------------
BROKEN += Joy
BROKEN += Phantasm
BROKEN += clock-tower
BROKEN += etude-1-chopin
BROKEN += invention-4
BROKEN += oh-lady-be-good
BROKEN += poj-prelude
BROKEN += true-faith
BROKEN += what-a-fool-believes
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_PDF= ${SONGS:=.pdf}

BROKEN_E = ${BROKEN:=.E}
BROKEN_ABC = ${BROKEN:=.abc}
BROKEN_FS = ${BROKEN:=.fs}
BROKEN_PS = ${BROKEN:=.ps}
BROKEN_PDF= ${BROKEN:=.pdf}
#-----------------------------------------------------------------------------
IMSCOMP = ../imscomp
MC_ARGS ?= --platochars
ABCM2PS	     = abcm2ps
PS2PDF       = ps2pdf
#-----------------------------------------------------------------------------
# -E			Only run the preprocessor.
# -x c			Treat input file(s) as having <language> 'c'.
# -Wno-invalid-pp-token
# -undef
# -Wundef
# -Werror		Turn warnings into errors.
# -nostdinc		No standard include directory.
# -P			Disable linemarker output in -E mode.
# -C			Include comments in preprocessed output.
# -CC			Include comments from within macros in preprocessed output.
# -traditional-cpp	Enable some traditional CPP emulation.
#
CPP=gcc -E -x c -nostdinc -C -CC -Wno-error -Wno-extra -traditional-cpp -Wno-invalid-pp-token
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv
.SUFFIXES: .abc .E .fs .csv .mid .pdf
# output $@   -- input $<
%.E: %.gcs
	${CPP} $< -o $@
%.fs:  %.E
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@
%.csv:  %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@
%.mid:  %.csv
	csvmidi $< $@
%.abc:  %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@
%.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%.ps
	${PS2PDF} $< $@
%.v:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  check     - Print out the Makefile Variables.'
	@echo '  clean     - Remove intermediate .csv, .mid, .abc, .ps, and .fs files.'
	@echo '  fs        - Make SONGS as fluidsynth (.fs) output.'
	@echo '  ps        - Make SONGS as postscript (.ps) output.'
	@echo '  pdf       - Make SONGS as postscript (.pdf) output.'
	@echo "songs: ${SONGS}"
	@echo "broken: ${BROKEN}"
#-----------------------------------------------------------------------------
.PHONY: all songs SONGS ${SONGS}
all songs SONGS ${SONGS}: ${SONGS_PS} ${SONGS_FS} ${SONGS_PDF}
#-----------------------------------------------------------------------------
broken BROKEN ${BROKEN}: ${BROKEN_PS} ${BROKEN_FS} ${BROKEN_PDF}
#-----------------------------------------------------------------------------
.PHONY: fs
fs: ${SONGS_FS}
.PHONY: ps
ps: ${SONGS_PS}
.PHONY: pdf
pdf: ${SONGS_PDF}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS} ${BROKEN_FS}
	rm -f ${SONGS_ABC} ${BROKEN_ABC}
	rm -f ${SONGS_PS} ${BROKEN_PS}
	rm -f ${SONGS_PDF} ${BROKEN_PDF}
	rm -f ${SONGS_E} ${BROKEN_E}
	rm -f *.stderr
	rm -f *.h *.v *.csv *.mid *.E *.fs *.pdf *.ps *.abc
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "SONGS=${SONGS}"
	@echo "SONGS_E=${SONGS_E}"
	@echo "SONGS_ABC=${SONGS_ABC}"
	@echo "SONGS_FS=${SONGS_FS}"
	@echo "SONGS_PS=${SONGS_PS}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "PS2PDF=${PS2PDF}"
	@echo "CPP=${CPP}"
	@echo "BROKEN=${BROKEN}"
	@echo "BROKEN_E=${BROKEN_E}"
	@echo "BROKEN_ABC=${BROKEN_ABC}"
	@echo "BROKEN_FS=${BROKEN_FS}"
	@echo "BROKEN_PS=${BROKEN_PS}"
#-----------------------------------------------------------------------------
