SONGS        = b9m1
SONGS       += b9m2
SONGS       += b9m3

#++ SONGS       += b9m4
INCLUDES     = instruments.include
INCLUDES    += macros.include
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_2_ABC = ${SONGS:=_2.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_2_FS = ${SONGS:=_2.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_2_PS = ${SONGS:=_2.ps}
SONGS_PDF= ${SONGS:=.pdf}
SONGS_2_PDF= ${SONGS:=_2.pdf}
SONGS_MP4= ${SONGS:=.mp4}
SONGS_2_MP4= ${SONGS:=_2.mp4}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../../musicomp2abc/musicomp2abc
IMSCOMP      = ../../ims/imscomp
GCS2YOUTUBE  = ../../gcs2youtube
FLUIDSYNTH   = ~/bin/fluidsynth
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
#-- SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
GCS2SFZ     = ../../music/music/sfz/gcs2sfz
SFZ_LIBDIR  = ../../music/music/sfz/library/VPO
SFZ_CSVDIR  = sfz-csv
SFZ_MIXDIR  = sfz-mix
WAV2WAVEFORM = ../../wav2waveform
FS2ASS      = ../../fs2ass
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
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
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
%.E: %.gcs ${M} ${INCLUDES} %.starting Makefile
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
%.wav:  %.fs
	${FLUIDSYNTH} -q -K 256 -F $@ ${SOUNDFONT} < $< > /dev/null \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Example:"
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --midi1csv ${SONGS} to get ${SONGS}.abc."
	@echo "  Use '${ABCM2PS} ${SONGS_ABC} > ${SONGS_2_PS}"
	@echo "  Use '${PS2PDF} ${SONGS_2_PS} > ${SONGS_2_PDF}'."
	@echo "  Use '${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth ${SONGS} to get ${SONGS_FS}"
	@echo ""
	@echo "Targets:"
	@echo "  all    - create music files for ${SONGS}."
	@echo "  clean  - Remove postscript (ps), pdf, csv, and fs files."
	@echo "  sfz    - render all pieces to SFZ VPO mix WAV (sfz-mix/)."
	@echo "  sfz-mp4- encode all SFZ mixes to MP4 video."
	@echo "  sfz-clean- remove SFZ artifacts (sfz-csv/, sfz-mix/)."
	@echo ""
	@echo "  s1     - create ${SONGS_ABC}, ${SONGS_2_PS}, and ${SONGS_FS} from ${SONGS}."
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "SONGS=${SONGS}"
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
fs: ${SONGS_FS} ${SONGS_2_FS}
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${SONGS_MP4}
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${SONGS_2_MP4}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f ${SONGS_E} ${SONGS_ABC} ${SONGS_2_ABC} ${SONGS_FS} ${SONGS_2_FS} ${SONGS_MP4} ${SONGS_2_MP4}
	rm -f ${SONGS_PS} ${SONGS_2_PS} ${SONGS_PS} ${SONGS_2_PS} ${SONGS_PDF} ${SONGS_2_PDF}
	rm -f *.wav *.csv *.h *.v
	rm -f *.E
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${SONGS:%=%-sfz.mp4}
#-----------------------------------------------------------------------------
.PHONY: s1
s1: ${SONGS_FS} ${SONGS_2_FS}    Makefile ${MUSICOMP2ABC}
s1: ${SONGS_PDF} ${SONGS_2_PDF}  Makefile ${IMSCOMP}
#-----------------------------------------------------------------------------
# SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
#   make sfz          - render all pieces to sfz-mix/*.wav
#   make sfz-mp4      - encode all pieces to *-sfz.mp4 video
#   make sfz-clean    - remove sfz-csv/ and sfz-mix/ directories
#
# Per-piece flow:
#   .gcs -> .E -> [imscomp --sfzpipecsv] -> sfz-csv/<piece>/*.csv
#                                          -> [gcs2sfz]      -> sfz-mix/<piece>-sfz-mix.wav
#                                          -> [ffmpeg]        -> <piece>-sfz.mp4
#-----------------------------------------------------------------------------
# 1) Generate per-instrument CSVs via imscomp --sfzpipecsv
#    Runs in the target CSV directory (absolute paths via CURDIR).
sfz-csv/%/.done: %.E ${IMSCOMP}
	@mkdir -p $${@D}
	@rm -f $${@D}/*.csv
	cd $${@D} && $(CURDIR)/../../ims/imscomp --sfzpipecsv $(CURDIR)/$*.E
	@touch $@
#-----------------------------------------------------------------------------
# 2) Render instrument CSVs to a mixed WAV via gcs2sfz + sfizz_render
sfz-mix/%-sfz-mix.wav: sfz-csv/%/.done
	@mkdir -p $${@D}
	"$(CURDIR)/$(GCS2SFZ)" "$(CURDIR)/$(SFZ_CSVDIR)/$*" \
		--outdir "$(CURDIR)/$(SFZ_MIXDIR)" \
		--sfzdir "$(CURDIR)/$(SFZ_LIBDIR)"
#-----------------------------------------------------------------------------
# 3) Encode SFZ mix to MP4 (same visual style as gcs2youtube: title card + waveform)
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
sfz: ${SONGS:%=${SFZ_MIXDIR}/%-sfz-mix.wav}
#-----------------------------------------------------------------------------
.PHONY: sfz-mp4
sfz-mp4: ${SONGS:%=%-sfz.mp4}
#-----------------------------------------------------------------------------
.PHONY: sfz-clean
sfz-clean:
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${SONGS:%=%-sfz.mp4}
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
