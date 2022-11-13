V1 = v1-1
V2 = v1-2
V3 = v1-3
V4 = v1-4
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
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS = ${PAGES} ${MEASURES} ${VOICES}
#++ MC_ARGS = --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES}
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv %_m.ps
.SUFFIXES: .abc _m.ps .E .fs .csv .mid .pdf
# output $@   -- input $<
%.fs:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:	%.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs ../instruments.include
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use 'musicomp2abc ${V1} to get ${V1}.abc."
	@echo "  Use 'abcm2ps ${V1}.abc > ${V1}_m.ps'."
	@echo ""
	@echo "Targets:"
	@echo "  all    - Create music files for ${V1}, ${V2}, ${V3}, and ${V4}."
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  s1     - create ${V1}.abc, ${V1}_m.ps, and ${V1}.fs from ${V1}."
	@echo "  s2     - create ${V2}.abc, ${V2}_m.ps, and ${V2}.fs from ${V2}."
	@echo "  s3     - create ${V3}.abc, ${V3}_m.ps, and ${V3}.fs from ${V3}."
	@echo "  s4     - create ${V4}.abc, ${V4}_m.ps, and ${V4}.fs from ${V4}."
#=============================================================================
.PHONY: all
all: s1 s2 s3 s4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V1}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f ${V2}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f ${V3}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f ${V4}.{csv,mid,fs,pdf,abc,wav,E}
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${V1}_m.ps ${V1}.fs ${V1}.pdf      Makefile ${MUSICOMP2ABC}
#-----------------------------------------------------------------------------
.PHONY: s2
s2: ${V2}_m.ps ${V2}.fs ${V2}.pdf      Makefile ${MUSICOMP2ABC}
#-----------------------------------------------------------------------------
.PHONY: s3
s3: ${V3}_m.ps ${V3}.fs ${V3}.pdf      Makefile ${MUSICOMP2ABC}
#-----------------------------------------------------------------------------
.PHONY: s4
s4: ${V4}_m.ps ${V4}.fs ${V4}.pdf      Makefile ${MUSICOMP2ABC}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
