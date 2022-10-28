V            = b-6
#-----------------------------------------------------------------------------
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
#- YAPS         = yaps
ABCM2PS      = abcm2ps
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
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv
.SUFFIXES: .abc _m.ps .E .fs .csv .mid
# output $@   -- input $<
%.fs:	%.E
	${MUSICOMP2ABC} --two --fluidsynth $< $@ ${PAGES} ${MEASURES} ${VOICES} \
		|| ( rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} --midi1csv $< $@ ${PAGES} ${MEASURES} ${VOICES} || rm -f $@
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:	%.E
	${MUSICOMP2ABC} --abc $< $@ ${PAGES} ${MEASURES} ${VOICES} \
		|| (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
%.E: %.gcs ../instruments.include
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
As = 01 02 03 04 05 06 07 08 09 10
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
	@echo "  clean  - Remove postscript (ps), csv, fs, and mid files."
	@echo ""
	@echo "  all    - create ${V}.abc from ${V} (via 'musicomp2abc')"
	@echo "         - and ${V}_m.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}.fs from ${V} (via 'musicomp2abc --fluidsynth')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
#-- 	@echo "YAPS=${YAPS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
#=============================================================================
all: ${V}_m.ps ${V}.fs ${V}.mid
#=============================================================================
.PHONY: clean
clean::
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid *.fs
	rm -f *.abc *.A_abc
	rm -f *.wav
	rm -f *.E
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
