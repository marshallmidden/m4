V            = b-6
#-----------------------------------------------------------------------------
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc
.SUFFIXES: .mid .csv .ps .abc _m.ps
# output $@   -- input $<
%.mid:  %.csv
	csvmidi $< $@
%.csv:  %
	${MUSICOMP2ABC} --midi1csv $< $@
%.abc:  %
	${MUSICOMP2ABC} --abc $< $@
%.ps:   %.abc
	${YAPS} $< -o $@ -k
%_m.ps: %.abc
	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
#-----------------------------------------------------------------------------
.PRECIOUS: %.A_abc
.SUFFIXES: _A.ps .A_abc _m_A.ps
# output $@   -- input $<
%.A_abc:  %
	${MUSICOMP2ABC} --abc $< $@
%_A.ps:   %.A_abc
	${YAPS} $< -o $@ -k
%_m_A.ps: %.A_abc
	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
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
	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "YAPS=${YAPS}"
	@echo "ABCM2PS=${ABCM2PS}"
#=============================================================================
all: ${V}.mid ${V}.ps ${V}_m.ps
	#++ open ${V}.ps ${V}_m.ps
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V}.mid
	#-- fluidsynth -F "${V}.wav" -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 "${V}.mid"
#=============================================================================
.PHONY: clean
clean::
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc *.A_abc
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
