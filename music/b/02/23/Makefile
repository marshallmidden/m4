V3           = v2-3
M3           = music-2-3
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
IMSCOMP      = ~/ims/imscomp
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
PAGES =
MEASURES =
VOICES =
# All voices & staves.
WHICH = "-D WHICH=0"
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#++ WHICH = "-D WHICH=1"
#++ VOICES = --voices 1..2
#++ WHICH = "-D WHICH=2"
#++ VOICES = --voices 3..4
#++ WHICH = "-D WHICH=3"
#++ VOICES = --voices 5..6
#++ WHICH = "-D WHICH=4"
#++ VOICES = --voices 7..8
#++ WHICH = "-D WHICH=5"
#++ VOICES = --voices 9..10
#++ WHICH = "-D WHICH=6"
#++ VOICES = --voices 11..12
#++ WHICH = "-D WHICH=7"
#++ VOICES = --voices 13
#++ WHICH = "-D WHICH=8"
#++ VOICES = --voices 14..17
#++ WHICH = "-D WHICH=9"
#++ VOICES = --voices 18..20
#++ WHICH = "-D WHICH=10"
#++ VOICES = --voices 21..23
#++ WHICH = "-D WHICH=11"
#++ VOICES = --voices 24..26
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-- MC_ARGS ?= --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-----------------------------------------------------------------------------
.PRECIOUS: %.E
.PRECIOUS: %.abc %.csv %_m.ps
.PRECIOUS: %_2.abc %_2.csv %_m2.ps
.SUFFIXES: .E
.SUFFIXES: .abc _m.ps .fs .csv .mid .pdf
.SUFFIXES: _2.abc _m2.ps _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.fs:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs:	%.E
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:	%.E
	${IMSCOMP} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
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
%.E: %.gcs ../../instruments.include ${M} Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --midi1csv ${V3} to get ${V3}.abc."
	@echo "  Use '${ABCM2PS} ${V3}.abc > ${V3}_m.ps'."
	@echo "  Use '${PS2PDF} ${V3}_m.ps > ${V3}.pdf'."
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth ${V3} to get ${V3}.fs."
	@echo ""
	@echo "Targets:"
	@echo "  all    - create music files for ${V3}."
	@echo "  clean  - Remove postscript (ps), pdf, csv, and fs files."
	@echo ""
	@echo "  s1     - create ${V3}.abc, ${V3}_m.ps, and ${V3}.fs from ${V3}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V3=${V3}"
	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
	@echo "PAGES=${PAGES}"
	@echo "MEASURES=${MEASURES}"
	@echo "VOICES=${VOICES}"
	@echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all		#-- There is a bug in gmake. I don't understand.
all: s1
#-----------------------------------------------------------------------------
fs: ${V3}.fs ${V3}_2.fs
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V3}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f ${V3}_2.{csv,mid,fs,pdf,abc,wav}
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${V3}.fs ${V3}.pdf      Makefile ${MUSICOMP2ABC}
s1: ${V3}_2.fs ${V3}_2.pdf  Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
