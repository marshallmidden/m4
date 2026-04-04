SONGS        = b2m1
SONGS       += b2m2
SONGS       += b2m3
SONGS       += b2m4
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_2_ABC = ${SONGS:=_2.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_2_FS = ${SONGS:=_2.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_2_PS = ${SONGS:=_2.ps}
SONGS_PDF= ${SONGS:=.pdf}
SONGS_2_PDF= ${SONGS:=_2.pdf}
SONGS_MP4= ${SONGS:=.mp4}
SONGS_2_MP4= ${SONGS:=_2.mp4}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../../musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
GCS2YOUTUBE  = ../../gcs2youtube
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
#-----------------------------------------------------------------------------
# -b nn = starting bar number
# -i = put red circle around errors.
# -j nn = number every this many bars.
# -l = landscape mode
# -M = suppress lyrics
# -N 2 = number pages in upper right.
# -s nn = page scale to (default 0.75) = %%scale.
ABCM2PS_ARGS = -M -b 1 $< -i -N 2 -j 1
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
MC_ARGS ?=
#-- MC_ARGS ?= --warn_octave_accidental
#-- MC_ARGS += --one
#-- MC_ARGS += --two
#-- MC_ARGS += --sf2=/Users/m4/thisisthesoundfont.sf2
#-----------------------------------------------------------------------------
.PRECIOUS: %.E
.PRECIOUS: %.abc %.csv %_m.ps
.PRECIOUS: %_2.abc %_2.csv %_m2.ps
.SUFFIXES: .E
.SUFFIXES: .abc _m.ps .fs .csv .mid .pdf
.SUFFIXES: _2.abc _m2.ps _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.fs:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs:	%.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:	%.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:	%.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid:	%_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc:	%.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%_m2.ps:	%_2.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%_2.pdf:	%_m2.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs ../instruments.include instruments.include Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Extra #include dependencies for specific movements.
b2m2.E: music-2-2.gcs
b2m3.E: music-2-3.gcs
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  all    - create music files for ${SONGS}."
	@echo "  clean  - Remove postscript (ps), pdf, csv, and fs files."
	@echo ""
	@echo "  s1     - create .fs, _2.fs, .pdf, _2.pdf for ${SONGS}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "SONGS=${SONGS}"
	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
#=============================================================================
#-- .PHONY: all         #-- There is a bug in gmake. I don't understand.
all: s1
#-----------------------------------------------------------------------------
fs: ${SONGS_FS} ${SONGS_2_FS}
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${SONGS_MP4}
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${SONGS_2_MP4}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${SONGS_E} ${SONGS_ABC} ${SONGS_2_ABC} ${SONGS_FS} ${SONGS_2_FS} ${SONGS_MP4} ${SONGS_2_MP4}
	rm -f ${SONGS_PS} ${SONGS_2_PS} ${SONGS_PDF} ${SONGS_2_PDF}
	rm -f *.wav *.csv *.h *.v *.mid
	rm -f *.E
	rm -f A.[012]* B.[012]*
	rm -f ZZZ.*
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${SONGS_FS} ${SONGS_2_FS}    Makefile ${MUSICOMP2ABC}
s1: ${SONGS_PDF} ${SONGS_2_PDF}  Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
