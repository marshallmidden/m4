V            = s14
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
#++ MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
MUSICOMP2ABC = ../../musicomp2abc/musicomp2abc
GCS2YOUTUBE  = ../../gcs2youtube
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
FLUIDSYNTH   = ~/bin/fluidsynth
#-- SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
GCS2SFZ      = ../../music/sfz/gcs2sfz
SFZ_LIBDIR   = ../../music/sfz/library/VPO
SFZ_CSVDIR   = sfz-csv
SFZ_MIXDIR   = sfz-mix
WAV2WAVEFORM = ../../wav2waveform
FS2ASS       = ../../fs2ass
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
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
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
%.wav:  %.fs Makefile
	${FLUIDSYNTH} -q -K 256 -F $@ ${SOUNDFONT} < $< > /dev/null \
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
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
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
	@echo "  sfz    - render ${V} to SFZ VPO mix WAV (sfz-mix/; GM piano)."
	@echo "  sfz-mp4- encode the SFZ mix to MP4 video."
	@echo "  sfz-clean- remove SFZ artifacts (sfz-csv/, sfz-mix/)."
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
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${V}.mp4
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${V}_2.mp4
#-----------------------------------------------------------------------------
.PHONY: sfz
sfz: sfz-mix/${V}-sfz-mix.wav
#-----------------------------------------------------------------------------
.PHONY: sfz-mp4
sfz-mp4: ${V}-sfz.mp4
#-----------------------------------------------------------------------------
.PHONY: sfz-clean
sfz-clean:
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V}-sfz.mp4
#-----------------------------------------------------------------------------
# SFZ (VPO sampler) pipeline. s14 is a solo piano: it has no VPO mapping, so
# gcs2sfz renders it through fluidsynth + GeneralUser.sf2 (GM fallback).
#   make sfz       - render ${V} to sfz-mix/${V}-sfz-mix.wav
#   make sfz-mp4   - encode to ${V}-sfz.mp4 video
#   make sfz-clean - remove sfz-csv/ and sfz-mix/
#-----------------------------------------------------------------------------
sfz-csv/${V}/.done: ${V}.E ${IMSCOMP}
	@mkdir -p $(dir $@)
	@rm -f $(dir $@)*.csv
	cd $(dir $@) && ${IMSCOMP} ${MC_ARGS} --sfzpipecsv $(CURDIR)/${V}.E
	@touch $@
#-----------------------------------------------------------------------------
sfz-mix/${V}-sfz-mix.wav: sfz-csv/${V}/.done
	@mkdir -p $(dir $@)
	"$(CURDIR)/$(GCS2SFZ)" "$(CURDIR)/$(SFZ_CSVDIR)/${V}" \
		--outdir "$(CURDIR)/$(SFZ_MIXDIR)" \
		--sfzdir "$(CURDIR)/$(SFZ_LIBDIR)" \
		--sf2 "$(SOUNDFONT)"
#-----------------------------------------------------------------------------
${V}-sfz.mp4: sfz-mix/${V}-sfz-mix.wav ${V}_2.fs ${GCS2YOUTUBE} ${WAV2WAVEFORM} ${FS2ASS}
	${GCS2YOUTUBE} -a "$<" -o "$@" "${V}_2.fs" || (rm -f $@ ; exit 1)
#=============================================================================
S14 = s14-orig.mid
#=============================================================================
s14:
	${MCOMP} ${S14} vertical > s14.pre-gcs
#=============================================================================
.PHONY: clean
clean: sfz-clean
	rm -f ${V}.abc ${V}_m.ps ${V}.pdf
	rm -f ${V}.stdout ${V}.stderr
	rm -f ${V}.csv ${V}.mid ${V}.wav
	rm -f ${V}.v ${V}.h
	rm -f ${V}.fs
	rm -f ${V}.mp4 ${V}_2.mp4
	rm -f ${V}.E
	rm -f ${V}_2.abc ${V}_2.pdf
	rm -f ${V}_2.csv ${V}_2.mid ${V}.wav
	rm -f ${V}_2.v ${V}_2.h
	rm -f ${V}_2.fs
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V}-sfz.mp4
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
