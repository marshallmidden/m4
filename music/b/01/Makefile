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
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
# SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
#   make sfz       - render V1..V4 to sfz-mix/*.wav
#   make sfz-mp4   - encode to *-sfz.mp4 video
#   make sfz-clean - remove sfz-csv/ and sfz-mix/
# Instruments with no VPO mapping (e.g. default piano) are rendered through
# fluidsynth + GeneralUser.sf2 (gcs2sfz --sf2) -- see music/sfz/gcs2sfz.
#-----------------------------------------------------------------------------
sfz-csv/%/.done: %.E ${IMSCOMP}
	@mkdir -p $(dir $@)
	@rm -f $(dir $@)*.csv
	cd $(dir $@) && $(CURDIR)/../../ims/imscomp --sfzpipecsv $(CURDIR)/$*.E
	@touch $@
#-----------------------------------------------------------------------------
sfz-mix/%-sfz-mix.wav: sfz-csv/%/.done
	@mkdir -p $(dir $@)
	"$(CURDIR)/$(GCS2SFZ)" "$(CURDIR)/$(SFZ_CSVDIR)/$*" \
		--outdir "$(CURDIR)/$(SFZ_MIXDIR)" \
		--sfzdir "$(CURDIR)/$(SFZ_LIBDIR)" \
		--sf2 "$(SOUNDFONT)"
#-----------------------------------------------------------------------------
%-sfz.mp4: sfz-mix/%-sfz-mix.wav %.E ${GCS2YOUTUBE} ${WAV2WAVEFORM} ${FS2ASS}
	@DURATION=$$(ffprobe -v error -show_entries format=duration \
		-of default=noprint_wrappers=1:nokey=1 "$<"); \
	TITLE=$$(grep '^title ' $*.E | sed 's/^title[[:space:]]*//' | tail -1); \
	TITLE=$${TITLE:-"$*"}; \
	ASS_FLAG=""; \
	WAVEFORM_FLAG=""; \
	if [ -x "$(CURDIR)/$(FS2ASS)" ]; then \
		ASS_FILE="$${TMPDIR:-/tmp}/$*_sfz.ass"; \
		if python3 "$(CURDIR)/$(FS2ASS)" "$*.E" "$$ASS_FILE" --title "$$TITLE" 2>/dev/null; then \
			ASS_FLAG="ass=$$ASS_FILE"; \
		fi; \
	fi; \
	if [ -x "$(CURDIR)/$(WAV2WAVEFORM)" ]; then \
		WAVEFORM_FILE="$${TMPDIR:-/tmp}/$*_sfz_waveform.mp4"; \
		if uv run --script "$(CURDIR)/$(WAV2WAVEFORM)" "$<" "$$WAVEFORM_FILE" \
				--fps 15 --window 10.0 2>/dev/null || \
		   python3 "$(CURDIR)/$(WAV2WAVEFORM)" "$<" "$$WAVEFORM_FILE" \
				--fps 15 --window 10.0 2>/dev/null; then \
			WAVEFORM_FLAG="$$WAVEFORM_FILE"; \
		fi; \
	fi; \
	if [ -n "$$ASS_FLAG" ] && [ -n "$$WAVEFORM_FLAG" ]; then \
		ffmpeg -y \
			-f lavfi -i "color=c=black:s=1920x1080:r=15:d=$$DURATION" \
			-i "$<" -i "$$WAVEFORM_FLAG" \
			-filter_complex "[0:v]$$ASS_FLAG[bg]; [bg][2:v]overlay=960:540[out]" \
			-map "[out]" -map "1:a" \
			-c:v libx264 -pix_fmt yuv420p -c:a aac -b:a 192k \
			-shortest -movflags +faststart "$@"; \
	elif [ -n "$$ASS_FLAG" ]; then \
		ffmpeg -y \
			-f lavfi -i "color=c=black:s=1920x1080:r=15:d=$$DURATION" \
			-i "$<" -vf "$$ASS_FLAG" \
			-c:v libx264 -pix_fmt yuv420p -c:a aac -b:a 192k \
			-shortest -movflags +faststart "$@"; \
	elif [ -n "$$WAVEFORM_FLAG" ]; then \
		ffmpeg -y \
			-f lavfi -i "color=c=black:s=1920x1080:r=15:d=$$DURATION" \
			-i "$<" -i "$$WAVEFORM_FLAG" \
			-filter_complex "[0:v]drawtext=text='$$TITLE':fontcolor=white:fontsize=48:x=(w-text_w)/2:y=(h-text_h)/2-40[bg]; [bg][2:v]overlay=960:540[out]" \
			-map "[out]" -map "1:a" \
			-c:v libx264 -pix_fmt yuv420p -c:a aac -b:a 192k \
			-shortest -movflags +faststart "$@"; \
	else \
		ffmpeg -y \
			-f lavfi -i "color=c=black:s=1920x1080:r=1:d=$$DURATION" \
			-i "$<" \
			-vf "drawtext=text='$$TITLE':fontcolor=white:fontsize=48:x=(w-text_w)/2:y=(h-text_h)/2-40" \
			-c:v libx264 -pix_fmt yuv420p -c:a aac -b:a 192k \
			-shortest -movflags +faststart "$@"; \
	fi
#-----------------------------------------------------------------------------
.PHONY: sfz
sfz: ${V1:%=${SFZ_MIXDIR}/%-sfz-mix.wav} ${V2:%=${SFZ_MIXDIR}/%-sfz-mix.wav} ${V3:%=${SFZ_MIXDIR}/%-sfz-mix.wav} ${V4:%=${SFZ_MIXDIR}/%-sfz-mix.wav}
#-----------------------------------------------------------------------------
.PHONY: sfz-mp4
sfz-mp4: ${V1:%=%-sfz.mp4} ${V2:%=%-sfz.mp4} ${V3:%=%-sfz.mp4} ${V4:%=%-sfz.mp4}
#-----------------------------------------------------------------------------
.PHONY: sfz-clean
sfz-clean:
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V1:%=%-sfz.mp4} ${V2:%=%-sfz.mp4} ${V3:%=%-sfz.mp4} ${V4:%=%-sfz.mp4}
#-----------------------------------------------------------------------------
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
	@echo "  sfz    - render ${V1}, ${V2}, ${V3}, ${V4} to SFZ VPO mix WAV (sfz-mix/)."
	@echo "  sfz-mp4- encode all SFZ mixes to MP4 video."
	@echo "  sfz-clean- remove SFZ artifacts (sfz-csv/, sfz-mix/)."
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
.PHONY: mp4_2
mp4_2: ${V1}_2.mp4 ${V2}_2.mp4 ${V3}_2.mp4 ${V4}_2.mp4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${V1}.csv ${V1}.mid ${V1}.fs ${V1}.pdf ${V1}.abc ${V1}.wav ${V1}.E ${V1}.h ${V1}.v ${V1}.mp4 ${V1}_2.mp4
	rm -f ${V1}_2.csv ${V1}_2.mid ${V1}_2.fs ${V1}_2.pdf ${V1}_2.abc ${V1}_2.wav ${V1}_2.E ${V1}_2.h ${V1}_2.v
	rm -f ${V2}.csv ${V2}.mid ${V2}.fs ${V2}.pdf ${V2}.abc ${V2}.wav ${V2}.E ${V2}.h ${V2}.v ${V2}.mp4 ${V2}_2.mp4
	rm -f ${V2}_2.csv ${V2}_2.mid ${V2}_2.fs ${V2}_2.pdf ${V2}_2.abc ${V2}_2.wav ${V2}_2.E ${V2}_2.h ${V2}_2.v
	rm -f ${V3}.csv ${V3}.mid ${V3}.fs ${V3}.pdf ${V3}.abc ${V3}.wav ${V3}.E ${V3}.h ${V3}.v ${V3}.mp4 ${V3}_2.mp4
	rm -f ${V3}_2.csv ${V3}_2.mid ${V3}_2.fs ${V3}_2.pdf ${V3}_2.abc ${V3}_2.wav ${V3}_2.E ${V3}_2.h ${V3}_2.v
	rm -f ${V4}.csv ${V4}.mid ${V4}.fs ${V4}.pdf ${V4}.abc ${V4}.wav ${V4}.E ${V4}.h ${V4}.v ${V4}.mp4 ${V4}_2.mp4
	rm -f ${V4}_2.csv ${V4}_2.mid ${V4}_2.fs ${V4}_2.pdf ${V4}_2.abc ${V4}_2.wav ${V4}_2.E ${V4}_2.h ${V4}_2.v
	rm -f A.[012]* B.[012]*
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V1:%=%-sfz.mp4} ${V2:%=%-sfz.mp4} ${V3:%=%-sfz.mp4} ${V4:%=%-sfz.mp4}
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
