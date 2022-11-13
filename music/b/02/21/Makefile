V1 = v2-1
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
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
#? WHICH = "-D WHICH=11"
#? VOICES = --voices 24..26
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS = ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-- MC_ARGS = --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv %_m.ps
.SUFFIXES: .abc _m.ps .E .fs .csv .mid .pdf
# output $@   -- input $<
%.fs:	%.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:	%.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:	%.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:	%.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs ../../instruments.include Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use 'musicomp2abc ${V1} to get ${V1}.abc."
	@echo "  Use 'abcm2ps ${V1}.abc > ${V1}_m.ps'."
	@echo ""
	@echo "Targets:"
	@echo "  all    - create music files for ${V1}."
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  s1     - create ${V1}.abc, ${V1}_m.ps, and ${V1}.fs from ${V1}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V1=${V1}"
	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PAGES=${PAGES}"
	@echo "MEASURES=${MEASURES}"
	@echo "VOICES=${VOICES}"
	@echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all
all: s1
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V1}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
#-- .PHONY: s1
s1: ${V1}.fs ${V1}.pdf
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
