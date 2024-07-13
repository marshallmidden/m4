#-----------------------------------------------------------------------------
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------

A = 01 02 03 04 05 06 07 08 09 10 11 12 

w_A := ${words $(A)}

s := $(shell seq 1 $(w_A))

#-----------------------------------------------------------------------------
.SUFFIXES: .mid .csv .ps .abc _m.ps .E
# output $@   -- input $<
%.mid:  %.csv
	echo csvmidi $< $@
%.csv:  %.E
	echo ${MUSICOMP2ABC} --midi1csv $< $@
%.abc:  %.E
	echo ${MUSICOMP2ABC} --abc $< $@
%.ps:   %.abc
	echo ${YAPS} $< -o $@ -k
%_m.ps: %.abc
	echo ${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
%.E: % beethoven.macros
	echo ${CPP} $< -o $@
#-----------------------------------------------------------------------------
.SUFFIXES: .mid .csv .ps .abc _m.ps .E
# output $@   -- input $<
%.A_abc:  %.E
	echo ${MUSICOMP2ABC} --abc $< $@
	touch $@
%.A_ps:   %.A_abc
	echo ${YAPS} $< -o $@ -k
	touch $@
%_m.A_ps: %.A_abc
	touch Out.ps
	echo ${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
#-----------------------------------------------------------------------------
define many_targets
    help::
	@echo "  a${word $1,${A}} -- do yaps and abcm2ps for staff $1"
    A.${word $1,${A}}: ${V}.E ${SEPARATE}
    	${SEPARATE}
    .PHONY: a.${word $1,${A}}
    a${word $1,${A}}: A.${word $1,${A}} a.${word $1,${A}}.A_ps a.${word $1,${A}}_m.A_ps
	open $>.A_ps $>_m.A_ps
    a.${word $1,${A}}.E: A.${word $1,${A}}
	ln -sf A.${word $1,${A}} a.${word $1,${A}}.E
endef
#-----------------------------------------------------------------------------
help::
	echo "First line of help"
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
.PHONY:	test
test:
	echo "A='${A}'"
	echo "w_A='${w_A}'"
	echo "s='${s}'"
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc
	rm -f *.E
	rm -f X X.v
