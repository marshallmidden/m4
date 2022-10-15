V            = v1-4
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
#-- YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -P -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.csv %.mid %.E .fs
.SUFFIXES: .mid .csv .ps .abc _m.ps .wav .E .fs
%.fs:	%.E
	${MUSICOMP2ABC} --two --fluidsynth $< $@ \
		|| (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} --midi1csv $< $@ \
		|| (rm -f $@ ; exit 1)
%.mid:	%.csv
	csvmidi $< $@ \
		|| (rm -f $@ ; exit 1)
%.wav:	%.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
%.abc:	%.E
	${MUSICOMP2ABC} --abc $< $@ \
		|| (rm -f $@ ; exit 1)
#-- %.ps:	%.abc
#-- 	${YAPS} $< -o $@ -k \
#-- 		|| (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
#-- %.E: %.gcs beethoven.macros
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PRECIOUS: %.A_abc
.SUFFIXES: _A.ps .A_abc _m_A.ps
# output $@   -- input $<
%.A_abc:  %
	${MUSICOMP2ABC} --abc $< $@
#-- %_A.ps:   %.A_abc
#-- 	${YAPS} $< -o $@ -k
%_m_A.ps: %.A_abc
	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@
#-----------------------------------------------------------------------------
As = 01 02 03 04 05 06 07 08 09 10 11 12
w_A := ${words $(As)}
s := $(shell seq 1 $(w_A))
#-----------------------------------------------------------------------------
define many_targets
    help::
	@echo "  a${word $1,${As}}    - Use '${SEPARATE} to create A.${word $1,${As}}, and yaps&abcm2ps for staff $1"
    A.${word $1,${As}}: ${V} ${SEPARATE}
	${SEPARATE}
    .PHONY: a.${word $1,${As}}
    a${word $1,${As}}: A.${word $1,${As}} a.${word $1,${As}}_A.ps a.${word $1,${As}}_m_A.ps
	open a.${word $1,${As}}_A.ps a.${word $1,${As}}_m_A.ps
    a.${word $1,${As}}: A.${word $1,${As}}
	ln -sf A.${word $1,${As}} a.${word $1,${As}}
    clean::
	rm -f a.${word $1,${As}}
endef
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create ${V}.abc from ${V} (via 'musicomp2abc')"
	@echo "         - and ${V}.ps from ${V}.abc (via 'yaps')"
	@echo "         - and Out.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}.mid from ${V} (via 'musicomp2abc --midicsv')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
	@echo "MCOMP=${MCOMP}"
	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
#-- 	@echo "YAPS=${YAPS}"
	@echo "ABCM2PS=${ABCM2PS}"
#=============================================================================
all: ${V}.mid ${V}.ps ${V}_m.ps ${V}.wav ${V}.fs
	#++ open ${V}.ps ${V}_m.ps
	#-- fluidsynth -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 ${V}.mid
	#-- fluidsynth -F "${V}.wav" -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 "${V}.mid"
#=============================================================================
#-- S14 = b14-orig
#=============================================================================
#-- s14:
#-- 	${MCOMP} ${S14} vertical > s14
#=============================================================================
.PHONY: clean
clean::
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid *.fs
	rm -f *.abc *.A_abc
	rm -f *.wav
	rm -f *.E
	rm -f A.[0123]* B.[0123]* C.[0123]*
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
