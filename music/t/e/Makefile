V            = e
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
# YAPS         = yaps
ABCM2PS      = abcm2ps
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
#-DOC-	PAGES = --pages 6..75
#-DOC-	MEASURES = --measures 48..422
#-DOC-	VOICES = --voices 42..62
PAGES =
MEASURES =
VOICES =
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# All voices & staves.
WHICH = "-D WHICH=0"
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Stringed instruments only.
#-- WHICH = "-D WHICH=1"
#-- PAGES += --pages 13..15
#-- MEASURES += --measure 73..422
#-- VOICES += --voices 42..62
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Horned instruments only, except for brass.
#-- WHICH = "-D WHICH=2"
# VOICES += --voices 13..35
#-- MEASURES += --measure 0,23..422
#-- MEASURES += --measure 34..422
#++ MEASURES += --measure 49..422
#-- MEASURES += --measure 1..422
#-- PAGES += --pages 4
#-- PAGES += --pages 5..75
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
#? WHICH = "-D WHICH=5"
#? VOICES = --voices 13..15
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(oboeA oboeB) (bassoonA bassoonB bassoonC)]
#? WHICH = "-D WHICH=6"
#? VOICES = --voices 16..17,21..23
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(french_horn1A french_horn1B) (french_horn2A french_horn2B)]
#? WHICH = "-D WHICH=7"
#? VOICES = --voices 24..27
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(cornet1A cornet1A) (trumpet1A trumpet1A)]
#? WHICH = "-D WHICH=8"
#? VOICES = --voices 28..31
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(trombone1A trombone1B) (trombone2A trombone2B)]
#? WHICH = "-D WHICH=9"
#? VOICES = --voices 32..35
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# timpaniA [(tambourineA triangleA snaredrumA)]
#? WHICH = "-D WHICH=10"
#? VOICES = --voices 36..39
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [cymbalA bassdrumA]
#? WHICH = "-D WHICH=11"
#? VOICES = --voices 40..41
#-----------------------------------------------------------------------------
#-- CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -P -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.csv %.mid
#- .SUFFIXES: .mid .csv .ps .abc _m.ps
.SUFFIXES: .mid .csv .abc _m.ps
# Output $@ -- input $< (first dependency -- after the :).
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#-- %.E: %.gcs Makefile
#-- 	${CPP} $< -o $@ \
#-- 		|| (rm -f $@ ; exit 1)
#-- # . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%.fs: %.gcs
	${MUSICOMP2ABC} --fluidsynth $< $@ ${PAGES} ${MEASURES} ${VOICES} ${WHICH} \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%.abc: %.gcs Makefile
	${MUSICOMP2ABC} --abc $< $@ ${PAGES} ${MEASURES} ${VOICES} ${WHICH} \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# %.ps: %.abc
# 	${YAPS} $< -o $@ -k \
# 		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%_m.ps: %.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
# Manually invokable rules follow.
%.csv: %.gcs Makefile
	${MUSICOMP2ABC} --midi1csv $< $@ \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
.SUFFIXES: .v
%.v: %.gcs Makefile
	${MUSICOMP2ABC} --vertical $< $@ ${PAGES} ${MEASURES} ${VOICES} ${WHICH} \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
.SUFFIXES: .horizontal
%.horizontal: %.gcs Makefile
	${MUSICOMP2ABC} --horizontal $< $@ ${PAGES} ${MEASURES} ${VOICES} ${WHICH} \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%.mid: %.csv
	csvmidi $< $@ \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%.wav: %.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 $< \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create ${V}.E from ${V}.gcs (via 'cpp')"
	@echo "         - and ${V}.abc from ${V}.E (via 'musicomp2abc --abc')"
#	@echo "         - and ${V}.ps from ${V}.abc (via 'yaps')"
	@echo "         - and ${V}_m.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}.fs from ${V}.E (via 'musicomp2abc --fluidsynth')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
#-- 	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
# 	@echo "YAPS=${YAPS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PAGES=${PAGES}"
	@echo "MEASURES=${MEASURES}"
	@echo "VOICES=${VOICES}"
	@echo "WHICH=${WHICH}"
#=============================================================================
# all: ${V}.ps ${V}_m.ps ${V}.abc ${V}.fs
all: ${V}_m.ps ${V}.abc ${V}.fs
#=============================================================================
.PHONY: clean
clean:
	rm -f *.abc
	rm -f *.ps
	rm -f *.fs
#--	rm -f *.v *.h *.csv *.mid *.wav
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
