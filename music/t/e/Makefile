V            = e
M            = instruments.include
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
# Limit processing... some done in compiling, others with output.
#-DOC-	PAGES = --pages 6..15			# cstop processing
#-DOC-	MEASURES = --measures 48..70		# cstop processing
#-DOC-	VOICES = --voices 43..50,55..62		# output limiting.
PAGES =
MEASURES =
VOICES =
# All voices & staves.
#-- WHICH = "-D WHICH=0"
#-- PAGES = --pages 0,12..14
#-- PAGES = --pages 0,69..100
#-----------------------------------------------------------------------------
# [canon cloches]
#? WHICH = "-D WHICH=12"
#? VOICES = --voices 41..42
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Stringed instruments only.
#? WHICH = "-D WHICH=1"
#? VOICES += --voices 43..63
#+ PAGES += --pages 12..75
#-- MEASURES += --measure 73..422
#..............................................................................
# "xx" Below edited, and reviewed by viewing e_m.ps compared to 1812.pdf.
#..............................................................................
# Horned instruments only, except for brass.
#xx WHICH = "-D WHICH=2"
#xx VOICES += --voices 13..34
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Clarinet A/B and English Horn A.  -- Done fixing, and reviewing (viewing).
#xx WHICH = "-D WHICH=3"
#xx VOICES = --voices 18..20
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# All brass section (top two staves).
#xx WHICH = "-D WHICH=4"
#xx VOICES = --voices 1..12
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# piccolo, flute1, flute2 -- 3rd, 4th, 5th staves.
#xx WHICH = "-D WHICH=5"
#xx VOICES = --voices 13..15
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(oboeA oboeB) (bassoonA bassoonB bassoonC)]
#xx WHICH = "-D WHICH=6"
#xx VOICES = --voices 16..17,21..23
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(french_horn1A french_horn1B) (french_horn2A french_horn2B)]
# Needs fixing via viewing.
#xx WHICH = "-D WHICH=7"
#xx VOICES = --voices 23..26
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(cornet1A cornet1A) (trumpet1A trumpet1A)]
#xx WHICH = "-D WHICH=8"
#xx VOICES = --voices 27..30
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(trombone1A trombone1B) (tuba1A tuba1B)]
#xx WHICH = "-D WHICH=9"
#xx VOICES = --voices 31..32,33..34
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# timpaniA [(timpaniA tambourineA triangleA snaredrumA)]
#xx WHICH = "-D WHICH=10"
#xx VOICES = --voices 35..38
# [cymbalA bassdrumA]
#xx WHICH = "-D WHICH=11"
#xx VOICES = --voices 39..40
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-- MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES}
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
%.E: %.gcs ${M} Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
# Manually invokable rules follow.
.SUFFIXES: .v
%.v: %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --vertical $< $@ || (rm -f $@ ; exit 1)
.SUFFIXES: .horizontal
%.horizontal: %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --horizontal $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create ${V}.E from ${V}.gcs (via 'cpp')"
	@echo "         - and ${V}.abc from ${V}.E (via 'musicomp2abc --abc')"
	@echo "         - and ${V}_2.abc from ${V}.E (via 'imscomp --abc')"
	@echo "         - and ${V}_m.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}_m2.ps from ${V}_2.abc (via 'abcm2ps')"
	@echo "         - and ${V}.fs from ${V}.E (via 'musicomp2abc --fluidsynth')"
	@echo "         - and ${V}_2.fs from ${V}.E (via 'imscomp --fluidsynth')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
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
#--	@echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all		#-- There is a bug in gmake. I don't understand.
all: ${V}_m.ps ${V}.fs ${V}.mid ${V}.pdf
all: ${V}_m2.ps ${V}_2.fs ${V}_2.mid ${V}_2.pdf
#-----------------------------------------------------------------------------
fs: ${V}.fs ${V}_2.fs
#=============================================================================
.PHONY: clean
clean:
	rm -f *.abc *.E
	rm -f *.ps
	rm -f *.fs
	rm -f *.mid
	rm -f *.csv
	rm -f *.v *.h *.wav
	rm -f ${V}.pdf ${V}_2.pdf
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
