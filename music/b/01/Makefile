V1 = v1-1
V2 = v1-2
V3 = v1-3
V4 = v1-4
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
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
.PHONY: help
help:
	@echo "Example:"
	@echo "  Use 'musicomp2abc ${V1} to get ${V1}.abc."
	@echo "  Use 'yaps ${V1}.abc -k' to get ${V1}.ps."
	@echo "  Use 'abcm2ps ${V1}.abc > ${V1}_m.ps'."
	@echo "  Use 'csvmidi ${V1}.csv > ${V1}.mid'."
	@echo ""
	@echo "Targets:"
	@echo "  all    - Create music files for ${V1}, ${V2}, ${V3}, and ${V4}."
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  s1     - create ${V1}.abc, ${V1}.ps, ${V1}_m.ps, and ${V1}.mid from ${V1}."
	@echo "  s2     - create ${V2}.abc, ${V2}.ps, ${V2}_m.ps, and ${V2}.mid from ${V2}."
	@echo "  s3     - create ${V3}.abc, ${V3}.ps, ${V3}_m.ps, and ${V3}.mid from ${V3}."
	@echo "  s4     - create ${V4}.abc, ${V4}.ps, ${V4}_m.ps, and ${V4}.mid from ${V4}."
#=============================================================================
.PHONY: all
all: s1 s2 s3 s4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps *.A_ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${V1}.mid ${V1}.ps ${V1}_m.ps
#-----------------------------------------------------------------------------
.PHONY: s2
s2: ${V2}.mid ${V2}.ps ${V2}_m.ps
#-----------------------------------------------------------------------------
.PHONY: s3
s3: ${V3}.mid ${V3}.ps ${V3}_m.ps
#-----------------------------------------------------------------------------
.PHONY: s4
s4: ${V4}.mid ${V4}.ps ${V4}_m.ps
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
