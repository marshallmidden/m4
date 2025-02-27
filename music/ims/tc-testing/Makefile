#-----------------------------------------------------------------------------
SONGS  = 18notes
SONGS += 1963

# Chromatic.gcs - Lots of tied note errors.
SONGS += Chromatic

SONGS += Entroid
SONGS += Entroid_
SONGS += Horizons
SONGS += Joy

# Kanon.gcs - Lots of tied note errors.
SONGS += Kanon

SONGS += Miditest

# Phantasm.gcs - Lots of tied note errors.
SONGS += Phantasm

SONGS += Poppers
SONGS += Roundoff
SONGS += changes-idea
SONGS += char-print
SONGS += etude-1-chopin

# fru-uhlingserwachen.gcs - Lots of tied note errors.
SONGS += fru-uhlingserwachen

SONGS += idea
SONGS += joy-piano-part

# ma-solitude.gcs - Lots of tied note errors.
SONGS += ma-solitude

SONGS += maple-leaf-rag

# miscellaneous-ideas.gcs - Lots of tied note errors.
SONGS += miscellaneous-ideas

# mozart.gcs - One tied note error.
SONGS += mozart

# oh-lady-be-good.gcs - Two tied note errors.
SONGS += oh-lady-be-good

# opus10-chopin.gcs - Two tied note errors.
SONGS += opus10-chopin

# opus8-scriabin.gcs - Lots of tied note errors.
SONGS += opus8-scriabin

# poj-prelude.gcs - Lots of tied note errors.
SONGS += poj-prelude

SONGS += random-chords
SONGS += replot
SONGS += revolution-midi
SONGS += song-without-words
SONGS += theme

# what-a-fool-believes.gcs - Lots of tied note errors.
SONGS += what-a-fool-believes
#-----------------------------------------------------------------------------
# Runs a VERY long time. Needs debugging as to what is wrong.
BROKEN += Djokja

# ERROR - Second goto in measure not allowed.
BROKEN += clock-tower

# No notes.
BROKEN += e-minor

# linecount=39 line='one:    DN 2. 4d16 DN 1. e f g a b'
#   read_next_line: tline="" \/s2A""
# linecount=39 l=1 macro i line#1 line='" \/s2A"'
#   read_next_line: tline=" 4d16 i 1. e f g a b"
# linecount=39 line=' 4d16 i 1. e f g a b'
#   read_next_line: tline="encode DEF=A;explicit"
# linecount=39 l=2 process invert line#1 line='encode DEF=A;explicit'
#   read_next_line: tline="decode A;type,typ;octave,oct;notename,name;sharp,shr;flat,flt"
# linecount=39 l=2 process invert line#2 line='decode A;type,typ;octave,oct;notename,name;sharp,shr;flat,flt'
#   read_next_line: tline="if typ == 0"
# linecount=39 l=2 process invert line#3 line='if typ == 0'
#   read_next_line: tline="if in(0, name, 2)"
# linecount=39 l=2 process invert line#4 line='if in(0, name, 2)'
# ERROR - if unable to determine if value (in(0, d, 2)).
#.............................................................................
#  read_next_line: tline="calc name = mod((9 - name), 7)"
#linecount=39 l=2 process invert line#9 line='calc name = mod((9 - name), 7)'
#ERROR - calc error - ERROR - function 'mod' called with wrong argument type ERROR - get_value - unrecognized variable='['ID', 'd']' vs ['COMMA']
BROKEN += invention-4

# ERROR - m:1 v['right'] Note is not legal 'vmf<' unknown
# ERROR - decode note had error with note vmf<
# Traceback (most recent call last):
#   File "/Users/m4/ims/tc-testing/../imscomp", line 9163, in do_encode
#     if (strng[0] != '' and strng[0] in 'vp') or strng[0:5] in ['cresc', 'dimin']:
# IndexError: string index out of range
BROKEN += true-faith

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
MC_ARGS ?= --platochars --lines
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
