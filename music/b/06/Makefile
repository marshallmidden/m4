#-----------------------------------------------------------------------------
.DEFAULT_GOAL := help
#-----------------------------------------------------------------------------
V            = b-6
#-----------------------------------------------------------------------------
SEPARATE     = ./A.separate
MUSICOMP2ABC = ../../musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
GCS2YOUTUBE  = ../../gcs2youtube
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
#-- SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
GCS2SFZ     = ../../music/sfz/gcs2sfz
SFZ_LIBDIR  = ../../music/sfz/library/VPO
SFZ_CSVDIR  = sfz-csv
SFZ_MIXDIR  = sfz-mix
WAV2WAVEFORM = ../../wav2waveform
FS2ASS      = ../../fs2ass
#-----------------------------------------------------------------------------
# OS-specific settings for the GM fallback soundfont
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SOUNDFONT  = /Users/m4/src/GeneralUser/GeneralUser.sf2
else
    SOUNDFONT  = /home/m4/src/GeneralUser_GS/GeneralUser.sf2
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
PAGES =
MEASURES =
VOICES =
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
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
%_2.mid:	%._2csv
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
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
# SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
#   make sfz       - render ${V} to sfz-mix/${V}-sfz-mix.wav
#   make sfz-mp4   - encode to ${V}-sfz.mp4 video
#   make sfz-clean - remove sfz-csv/ and sfz-mix/
# Instruments with no VPO mapping (e.g. default piano) are rendered through
# fluidsynth + GeneralUser.sf2 (gcs2sfz --sf2) -- see music/sfz/gcs2sfz.
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
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, fs, and mid files."
	@echo "  sfz    - render ${V} to SFZ VPO mix WAV (sfz-mix/)."
	@echo "  sfz-mp4- encode the SFZ mix to MP4 video."
	@echo "  sfz-clean- remove SFZ artifacts (sfz-csv/, sfz-mix/)."
	@echo ""
	@echo "  all    - create ${V}.abc from ${V} (via 'musicomp2abc')"
	@echo "         - and ${V}_2.abc from ${V} (via 'imscomp')"
	@echo "         - and ${V}_m.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}_m2.ps from ${V}_.abc (via 'abcm2ps')"
	@echo "         - and ${V}.fs from ${V} (via 'musicomp2abc --fluidsynth')"
	@echo "         - and ${V}_2.fs from ${V} (via 'imscomp --fluidsynth')"
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
	@echo "SEPARATE=${SEPARATE}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
#=============================================================================
all: ${V}_m.ps ${V}.fs ${V}.mid ${V}.pdf
all: ${V}_m2.ps ${V}_2.fs ${V}_2.mid ${V}_2.pdf
#-----------------------------------------------------------------------------
fs: ${V}.fs
fs2: ${V}_2.fs
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
#=============================================================================
.PHONY: clean
clean: sfz-clean
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid *.fs
	rm -f *.abc *.A_abc
	rm -f *.wav
	rm -f *.mp4
	rm -f *.E
	rm -f *.v *.h
	rm -f A.[012]* B.[012]*
	rm -f ${V}.pdf ${V}_2.pdf
	rm -f ZZZ.*
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V}-sfz.mp4
#-----------------------------------------------------------------------------
$(foreach e,$(s),$(eval $(call many_targets,$(e))))
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
