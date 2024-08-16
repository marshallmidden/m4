#-- SONGS        = v4-1
#-- SONGS       += v4-2
SONGS       += v4-3
#-- SONGS       += v4-4
INCLUDES     = instruments.include
INCLUDES    += macros.include
v4-1_INCLUDES = v4-1.starting
v4_2_INCLUDES = v4-2.starting
v4_3_INCLUDES = v4-3.starting
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
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
IMSCOMP = ~/ims/imscomp
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
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS ?=
#-- MC_ARGS ?= --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-----------------------------------------------------------------------------
.PRECIOUS: %.E
.PRECIOUS: %.abc %.csv %_m.ps
.PRECIOUS: %_2.abc %_2.csv %_m2.ps
.SUFFIXES: .E
.SUFFIXES: .abc _m.ps .fs .csv .mid .pdf
.SUFFIXES: _2.abc _m2.ps _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.fs:   %.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs: %.E
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:        %.E
	${IMSCOMP} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid:        %_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc:        %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%_m2.ps:        %_2.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:  %_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%_2.pdf:        %_m2.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs ${M} ${INCLUDES} ${%_INCLUDES} Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.v:    %.E
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%_m.v:    %.E
	${MUSICOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E
	${IMSCOMP} ${MC_ARGS} --hori $< $@
%_m.h:    %.E
	${MUSICOMP} ${MC_ARGS} --hori $< $@
#-----------------------------------------------------------------------------
%.wav:  %.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --midi1csv ${SONGS} to get ${SONGS}.abc."
	@echo "  Use '${ABCM2PS} ${SONGS_ABC} > ${SONGS_2_PS}"
	@echo "  Use '${PS2PDF} ${SONGS_2_PS} > ${SONGS_2_PDF}'."
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth ${SONGS} to get ${SONGS_FS}"
	@echo ""
	@echo "Targets:"
	@echo "  all    - create music files for ${SONGS}."
	@echo "  clean  - Remove postscript (ps), pdf, csv, and fs files."
	@echo ""
	@echo "  s1     - create ${SONGS_ABC}, ${SONGS_2_PS}, and ${SONGS_FS} from ${SONGS}."
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
#--	@echo "PAGES=${PAGES}"
#--	@echo "MEASURES=${MEASURES}"
#--	@echo "VOICES=${VOICES}"
#--     @echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all         #-- There is a bug in gmake. I don't understand.
all: s1
#-----------------------------------------------------------------------------
fs: ${SONGS_FS} ${SONGS_2_FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${SONGS_E} ${SONGS_ABC} ${SONGS_2_ABC} ${SONGS_FS} ${SONGS_2_FS}
	rm -f ${SONGS_PS} ${SONGS_2_PS} ${SONGS_PS} ${SONGS_2_PS} ${SONGS_PDF} ${SONGS_2_PDF}
	rm -f *.wav *.csv *.h *.v
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${SONGS_FS} ${SONGS_2_FS}    Makefile ${MUSICOMP2ABC}
s1: ${SONGS_PDF} ${SONGS_2_PDF}  Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
