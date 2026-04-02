V4           = v2-4
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../../../musicomp2abc/musicomp2abc
IMSCOMP      = ../../../ims/imscomp
GCS2YOUTUBE  = ../../../gcs2youtube
FLUIDSYNTH   = ~/bin/fluidsynth
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
#-----------------------------------------------------------------------------
# OS-specific settings for fluidsynth
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SOUNDFONT  = /Users/m4/src/GeneralUser/GeneralUser.sf2
    FLUID_AUDIO = -a coreaudio
else
    SOUNDFONT  = /home/m4/src/GeneralUser_GS/GeneralUser.sf2
    FLUID_AUDIO =
endif
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
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC
MC_ARGS ?=
#-- MC_ARGS ?= --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
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
%.fs:   %.E
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs: %.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:        %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid:        %_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc:        %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%_m2.ps:        %_2.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:  %_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%_2.pdf:        %_m2.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs ../../instruments.include ${M} Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
%.wav:  %.fs
	${FLUIDSYNTH} -q -F $@ ${SOUNDFONT} < $< \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --midi1csv ${V4} to get ${V4}.abc."
	@echo "  Use '${ABCM2PS} ${V4}.abc > ${V4}_m.ps'."
	@echo "  Use '${PS2PDF} ${V4}_m.ps > ${V4}.pdf'."
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth ${V4} to get ${V4}.fs."
	@echo ""
	@echo "Targets:"
	@echo "  all    - create music files for ${V4}."
	@echo "  clean  - Remove postscript (ps), pdf, csv, and fs files."
	@echo ""
	@echo "  s1     - create ${V4}.abc, ${V4}_m.ps, and ${V4}.fs from ${V4}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V4=${V4}"
	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
#--	@echo "PAGES=${PAGES}"
#--	@echo "MEASURES=${MEASURES}"
#--	@echo "VOICES=${VOICES}"
#--     @echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all         #-- There is a bug in gmake. I don't understand.
all: s1
#-----------------------------------------------------------------------------
fs: ${V4}.fs ${V4}_2.fs
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${V4}.mp4
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${V4}_2.mp4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V4}.csv ${V4}.mid ${V4}.fs ${V4}.pdf ${V4}.abc ${V4}.wav ${V4}.E ${V4}.h ${V4}.v ${V4}.mp4 ${V4}_2.mp4
	rm -f ${V4}_2.csv ${V4}_2.mid ${V4}_2.fs ${V4}_2.pdf ${V4}_2.abc ${V4}_2.wav ${V4}_2.E ${V4}_2.h ${V4}_2.v
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${V4}.fs ${V4}.pdf      Makefile ${MUSICOMP2ABC}
s1: ${V4}_2.fs ${V4}_2.pdf  Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
