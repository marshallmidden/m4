V1 = v1-1
V2 = v1-2
V3 = v1-3
V4 = v1-4
#-----------------------------------------------------------------------------
SEPARATE     = ./A.separate
MUSICOMP2ABC = ../../musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
GCS2YOUTUBE  = ../../gcs2youtube
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
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES}
#++ MC_ARGS ?= --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES}
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
%.fs:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs:	%.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:	%.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
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
%.E:	%.gcs ../instruments.include
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.mp4:	%.gcs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
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
	@echo "         - create ${V1}_2.abc, ${V1}_m2.ps, and ${V1}_2.fs from ${V1}."
	@echo "  s2     - create ${V2}.abc, ${V2}_m.ps, and ${V2}.fs from ${V2}."
	@echo "         - create ${V2}_2.abc, ${V2}_m2.ps, and ${V2}_2.fs from ${V2}."
	@echo "  s3     - create ${V3}.abc, ${V3}_m.ps, and ${V3}.fs from ${V3}."
	@echo "         - create ${V3}_2.abc, ${V3}_m2.ps, and ${V3}_2.fs from ${V3}."
	@echo "  s4     - create ${V4}.abc, ${V4}_m.ps, and ${V4}.fs from ${V4}."
	@echo "         - create ${V4}_2.abc, ${V4}_m2.ps, and ${V4}_2.fs from ${V4}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V1=${V1}"
	@echo "V2=${V2}"
	@echo "V3=${V3}"
	@echo "V4=${V4}"
	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
#=============================================================================
.PHONY: all
all: s1 s2 s3 s4
#-----------------------------------------------------------------------------
fs: ${V1}.fs ${V2}.fs ${V3}.fs ${V4}.fs
fs2: ${V1}_2.fs ${V2}_2.fs ${V3}_2.fs ${V4}_2.fs
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${V1}.mp4 ${V2}.mp4 ${V3}.mp4 ${V4}.mp4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V1}.csv ${V1}.mid ${V1}.fs ${V1}.pdf ${V1}.abc ${V1}.wav ${V1}.E ${V1}.h ${V1}.v ${V1}.mp4
	rm -f ${V1}_2.csv ${V1}_2.mid ${V1}_2.fs ${V1}_2.pdf ${V1}_2.abc ${V1}_2.wav ${V1}_2.E ${V1}_2.h ${V1}_2.v
	rm -f ${V2}.csv ${V2}.mid ${V2}.fs ${V2}.pdf ${V2}.abc ${V2}.wav ${V2}.E ${V2}.h ${V2}.v ${V2}.mp4
	rm -f ${V2}_2.csv ${V2}_2.mid ${V2}_2.fs ${V2}_2.pdf ${V2}_2.abc ${V2}_2.wav ${V2}_2.E ${V2}_2.h ${V2}_2.v
	rm -f ${V3}.csv ${V3}.mid ${V3}.fs ${V3}.pdf ${V3}.abc ${V3}.wav ${V3}.E ${V3}.h ${V3}.v ${V3}.mp4
	rm -f ${V3}_2.csv ${V3}_2.mid ${V3}_2.fs ${V3}_2.pdf ${V3}_2.abc ${V3}_2.wav ${V3}_2.E ${V3}_2.h ${V3}_2.v
	rm -f ${V4}.csv ${V4}.mid ${V4}.fs ${V4}.pdf ${V4}.abc ${V4}.wav ${V4}.E ${V4}.h ${V4}.v ${V4}.mp4
	rm -f ${V4}_2.csv ${V4}_2.mid ${V4}_2.fs ${V4}_2.pdf ${V4}_2.abc ${V4}_2.wav ${V4}_2.E ${V4}_2.h ${V4}_2.v
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${V1}.fs ${V1}.pdf     ${MUSICOMP2ABC} Makefile
s1: ${V1}_2.fs ${V1}_2.pdf ${IMSCOMP} Makefile
#-----------------------------------------------------------------------------
.PHONY: s2
s2: ${V2}.fs ${V2}.pdf     Makefile ${MUSICOMP2ABC}
s2: ${V2}_2.fs ${V2}_2.pdf Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
.PHONY: s3
s3: ${V3}.fs ${V3}.pdf     Makefile ${MUSICOMP2ABC}
s3: ${V3}_2.fs ${V3}_2.pdf Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
.PHONY: s4
s4: ${V4}.fs ${V4}.pdf      Makefile ${MUSICOMP2ABC}
s4: ${V4}_2.fs ${V4}_2.pdf Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
