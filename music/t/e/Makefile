V            = e
#-----------------------------------------------------------------------------
#-- SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
# -b nn = starting bar number
# -i = put red circle around errors.
# -j nn = number every this many bars.
# -l = landscape mode
# -M = suppress lyrics
# -N 2  = number pages in upper right.
# -s nn = page scale to (default 0.75) = %%scale.
#-- ABCM2PS_ARGS = "-l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
#-- ABCM2PS_ARGS = -s 0.40 -M -b 1 $< -i -N 2 -j 1
#-- ABCM2PS_ARGS = -s 0.41 -M -b 1 $< -i -N 2 -j 1
ABCM2PS_ARGS = -M -b 1 $< -i -N 2 -j 1
#-- ABCM2PS_ARGS = "-s 0.42 -M -b 1 $< -i -N 2 -j 1"
#-- ABCM2PS_ARGS = -s 0.43 -M -b 1 $< -i -N 2 -j 1
#-----------------------------------------------------------------------------
# Limit processing... some done in compiling, others with output.
#-DOC-	PAGES = --pages 6..75
#-DOC-	MEASURES = --measures 48..422
#-DOC-	VOICES = --voices 42..62
PAGES =
MEASURES =
VOICES =
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Stringed instruments only.
#-- PAGES += --pages 13..15
#-- MEASURES += --measure 73..422
#-- VOICES += --voices 42..62
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Clarinet A/B and English Horn A.
#-- VOICES += --voices 18..20
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Horned instruments only, except for brass.
# VOICES += --voices 13..35
#++ VOICES += --voices 18..20
#-- MEASURES += --measure 0,23..422
#-- MEASURES += --measure 34..422
#++ MEASURES += --measure 49..422
#-- MEASURES += --measure 1..422
#-- PAGES += --pages 4
#-- PAGES += --pages 5..75
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -P -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.csv %.mid %.E
.SUFFIXES: .mid .csv .ps .abc _m.ps .E .wav
# output $@   -- input $<
%.fs:  %.E
	${MUSICOMP2ABC} --fluidsynth $< $@ ${PAGES} ${MEASURES} ${VOICES} \
		|| (rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} --midi1csv $< $@ \
		|| (rm -f $@ ; exit 1)
#-- %.mid:  %.csv
#-- 	csvmidi $< $@ \
#-- 		|| (rm -f $@ ; exit 1)
#-- %.wav:  %.mid
#-- 	fluidsynth -F $@ -a coreaudio -d -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 $< \
#-- 		|| (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} --abc $< $@ ${PAGES} ${MEASURES} ${VOICES} \
		|| (rm -f $@ ; exit 1)
%.ps:   %.abc
	${YAPS} $< -o $@ -k \
		|| (rm -f $@ ; exit 1)
%_m.ps: %.abc
#--	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
#--		|| (rm -f $@ ; exit 1)
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
.SUFFIXES: .v
%.v:  %.E
	${MUSICOMP2ABC} --vertical $< $@ ${PAGES} ${MEASURES} ${VOICES} \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
#-- .PRECIOUS: %.A_abc
#-- .SUFFIXES: _A.ps .A_abc _m_A.ps
# output $@   -- input $<
#-- %.A_abc:  %.E
#-- 	${MUSICOMP2ABC} --abc $< $@
#-- %_A.ps:   %.A_abc
#-- 	${YAPS} $< -o $@ -k
#-- %_m_A.ps: %.A_abc
#-- 	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@
#-----------------------------------------------------------------------------
#-- As = 01 02 03 04 05 06 07 08 09 10 11 12
#-- w_A := ${words $(As)}
#-- s := $(shell seq 1 $(w_A))
#-----------------------------------------------------------------------------
#-- define many_targets
#--     help::
#-- #	@echo "  a${word $1,${As}}    - Use '${SEPARATE} to create A.${word $1,${As}}, and yaps&abcm2ps for staff $1"
#-- #    A.${word $1,${As}}: ${V}.E ${SEPARATE}
#-- #	${SEPARATE}
#--     .PHONY: a.${word $1,${As}}
#--     a${word $1,${As}}: A.${word $1,${As}} a.${word $1,${As}}_A.ps a.${word $1,${As}}_m_A.ps
#-- 	open a.${word $1,${As}}_A.ps a.${word $1,${As}}_m_A.ps
#--     a.${word $1,${As}}.E: A.${word $1,${As}}
#-- 	ln -sf A.${word $1,${As}} a.${word $1,${As}}.E
#-- endef
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create ${V}.abc from ${V} (via 'musicomp2abc')"
	@echo "         - and ${V}.ps from ${V}.abc (via 'yaps')"
	@echo "         - and Out.ps from ${V}.abc (via 'abcm2ps')"
#--	@echo "         - and ${V}.mid from ${V} (via 'musicomp2abc --midicsv')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
#	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "YAPS=${YAPS}"
	@echo "ABCM2PS=${ABCM2PS}"
#=============================================================================
#-- all: ${V}.mid ${V}.ps ${V}_m.ps ${V}.wav ${V}.fs
#-- all: ${V}.fs
all: ${V}.ps ${V}_m.ps ${V}.abc ${V}.fs
#=============================================================================
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
#--	rm -f *.csv *.mid *.fs
	rm -f *.csv *.fs
	rm -f *.abc *.A_abc
#--	rm -f *.wav
	rm -f *.E
	rm -f A.[0123]* B.[0123]* C.[0123]*
	rm -f *.v
#-----------------------------------------------------------------------------
#-- $(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
