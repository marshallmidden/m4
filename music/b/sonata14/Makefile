V            = s14
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
#++ MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
MUSICOMP2ABC = ../../ims/musicomp2abc
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
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
#-- MC_ARGS += --one
#-- MC_ARGS += --two
#-- MC_ARGS += --sf2=/Users/m4/thisisthesoundfont.sf2
#-----------------------------------------------------------------------------
.PRECIOUS: %.E
.PRECIOUS: %.abc %.csv %_m.ps
.PRECIOUS: %_2.abc %_2.csv %_m2.ps
.SUFFIXES: .E
.SUFFIXES: .abc _m.ps .fs .csv .mid .pdf
.SUFFIXES: _2.abc _m2.ps _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.fs:  %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs: %.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:  %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:        %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:  %.csv Makefile
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid:        %_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.wav:  %.mid Makefile
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
%.abc:  %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc:        %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps: %.abc Makefile
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:	%_m.ps Makefile
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.v:    %.E
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%_m.v:    %.E
	${MUSICOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E
	${IMSCOMP} ${MC_ARGS} --hori $< $@
%_m.h:    %.E
	${MUSICOMP} ${MC_ARGS} --hori $< $@
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
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "ABCM2PS=${ABCM2PS}"
#=============================================================================
all:	${V}.fs ${V}.mid ${V}.pdf ${V}.wav
#-----------------------------------------------------------------------------
fs: ${V}.fs
pdf: ${V}.pdf
#=============================================================================
S14 = s14-orig.mid
#=============================================================================
s14:
	${MCOMP} ${S14} vertical > s14.pre-gcs
#=============================================================================
.PHONY: clean
clean:
	rm -f ${V}.abc ${V}_m.ps ${V}.pdf
	rm -f ${V}.stdout ${V}.stderr
	rm -f ${V}.csv ${V}.mid ${V}.wav
	rm -f ${V}.v ${V}.h
	rm -f ${V}.fs
	rm -f ${V}.E
	rm -f ${V}_2.abc ${V}_2.pdf
	rm -f ${V}_2.csv ${V}_2.mid ${V}.wav
	rm -f ${V}_2.v ${V}_2.h
	rm -f ${V}_2.fs
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
