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
# SFZ (VPO sampler) pipeline. s14 is a solo piano: it has no VPO mapping, so
# gcs2sfz renders it through fluidsynth + GeneralUser.sf2 (GM fallback).
#   make sfz       - render ${V} to sfz-mix/${V}-sfz-mix.wav
#   make sfz-mp4   - encode to ${V}-sfz.mp4 video
#   make sfz-clean - remove sfz-csv/ and sfz-mix/
#-----------------------------------------------------------------------------
sfz-csv/${V}/.done: ${V}.E ${IMSCOMP}
	@mkdir -p $(dir $@)
	@rm -f $(dir $@)*.csv
	cd $(dir $@) && $(CURDIR)/../../ims/imscomp --sfzpipecsv $(CURDIR)/${V}.E
	@touch $@
#-----------------------------------------------------------------------------
sfz-mix/${V}-sfz-mix.wav: sfz-csv/${V}/.done
	@mkdir -p $(dir $@)
	"$(CURDIR)/$(GCS2SFZ)" "$(CURDIR)/$(SFZ_CSVDIR)/${V}" \
		--outdir "$(CURDIR)/$(SFZ_MIXDIR)" \
		--sfzdir "$(CURDIR)/$(SFZ_LIBDIR)" \
		--sf2 "$(SOUNDFONT)"
#-----------------------------------------------------------------------------
${V}-sfz.mp4: sfz-mix/${V}-sfz-mix.wav ${V}.E ${GCS2YOUTUBE} ${WAV2WAVEFORM} ${FS2ASS}
	@DURATION=$$(ffprobe -v error -show_entries format=duration \
		-of default=noprint_wrappers=1:nokey=1 "$<"); \
	TITLE=$$(grep '^title ' ${V}.E | sed 's/^title[[:space:]]*//' | tail -1); \
	TITLE=$${TITLE:-"${V}"}; \
	ASS_FLAG=""; \
	WAVEFORM_FLAG=""; \
	if [ -x "$(CURDIR)/$(FS2ASS)" ]; then \
		ASS_FILE="$${TMPDIR:-/tmp}/${V}_sfz.ass"; \
		if python3 "$(CURDIR)/$(FS2ASS)" "${V}.E" "$$ASS_FILE" --title "$$TITLE" 2>/dev/null; then \
			ASS_FLAG="ass=$$ASS_FILE"; \
		fi; \
	fi; \
	if [ -x "$(CURDIR)/$(WAV2WAVEFORM)" ]; then \
		WAVEFORM_FILE="$${TMPDIR:-/tmp}/${V}_sfz_waveform.mp4"; \
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
