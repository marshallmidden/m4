V            = e
M            = instruments.include
#-----------------------------------------------------------------------------
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
# -b nn = starting bar number
# -i = put red circle around errors.
# -j nn = number every this many bars.
# -l = landscape mode
# -M = suppress lyrics
# -N 2 = number pages in upper right.
# -s nn = page scale to (default 0.75) = %%scale.
ABCM2PS_ARGS = -M -b 1 $< -i -N 2 -j 1
#-----------------------------------------------------------------------------
# Limit processing... some done in compiling, others with output.
#-DOC-	PAGES = --pages 6..15			# cstop processing
#-DOC-	MEASURES = --measures 48..70		# cstop processing
#-DOC-	VOICES = --voices 43..50,55..62		# output limiting.
PAGES =
MEASURES =
VOICES =
# All voices & staves.
#-- WHICH = "-D WHICH=0"
#-- PAGES = --pages 0,12..14
#-- PAGES = --pages 0,69..100
#-----------------------------------------------------------------------------
# [canon cloches]
#? WHICH = "-D WHICH=12"
#? VOICES = --voices 41..42
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Stringed instruments only.
#? WHICH = "-D WHICH=1"
#? VOICES += --voices 43..63
#+ PAGES += --pages 12..75
#-- MEASURES += --measure 73..422
#..............................................................................
# "xx" Below edited, and reviewed by viewing e_m.ps compared to 1812.pdf.
#..............................................................................
# Horned instruments only, except for brass.
#xx WHICH = "-D WHICH=2"
#xx VOICES += --voices 13..34
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Clarinet A/B and English Horn A.  -- Done fixing, and reviewing (viewing).
#xx WHICH = "-D WHICH=3"
#xx VOICES = --voices 18..20
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# All brass section (top two staves).
#xx WHICH = "-D WHICH=4"
#xx VOICES = --voices 1..12
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# piccolo, flute1, flute2 -- 3rd, 4th, 5th staves.
#xx WHICH = "-D WHICH=5"
#xx VOICES = --voices 13..15
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(oboeA oboeB) (bassoonA bassoonB bassoonC)]
#xx WHICH = "-D WHICH=6"
#xx VOICES = --voices 16..17,21..23
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(french_horn1A french_horn1B) (french_horn2A french_horn2B)]
# Needs fixing via viewing.
#xx WHICH = "-D WHICH=7"
#xx VOICES = --voices 23..26
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(cornet1A cornet1A) (trumpet1A trumpet1A)]
#xx WHICH = "-D WHICH=8"
#xx VOICES = --voices 27..30
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# [(trombone1A trombone1B) (tuba1A tuba1B)]
#xx WHICH = "-D WHICH=9"
#xx VOICES = --voices 31..32,33..34
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# timpaniA [(timpaniA tambourineA triangleA snaredrumA)]
#xx WHICH = "-D WHICH=10"
#xx VOICES = --voices 35..38
# [cymbalA bassdrumA]
#xx WHICH = "-D WHICH=11"
#xx VOICES = --voices 39..40
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES} ${WHICH}
#-- MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES}
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
%.E: %.gcs ${M} Makefile
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
# Manually invokable rules follow.
.SUFFIXES: .v
%.v: %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --vertical $< $@ || (rm -f $@ ; exit 1)
.SUFFIXES: .horizontal
%.horizontal: %.E Makefile
	${MUSICOMP2ABC} ${MC_ARGS} --horizontal $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  all    - create ${V}.E from ${V}.gcs (via 'cpp')"
	@echo "         - and ${V}.abc from ${V}.E (via 'musicomp2abc --abc')"
	@echo "         - and ${V}_2.abc from ${V}.E (via 'imscomp --abc')"
	@echo "         - and ${V}_m.ps from ${V}.abc (via 'abcm2ps')"
	@echo "         - and ${V}_m2.ps from ${V}_2.abc (via 'abcm2ps')"
	@echo "         - and ${V}.fs from ${V}.E (via 'musicomp2abc --fluidsynth')"
	@echo "         - and ${V}_2.fs from ${V}.E (via 'imscomp --fluidsynth')"
	@echo "  sfz    - render ${V} to SFZ VPO mix WAV (sfz-mix/)."
	@echo "  sfz-mp4- encode the SFZ mix to MP4 video."
	@echo "  sfz-clean- remove SFZ artifacts (sfz-csv/, sfz-mix/)."
	@echo ""
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V=${V}"
	@echo "CPP=${CPP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "ABCM2PS_ARGS=${ABCM2PS_ARGS}"
	@echo "PS2PDF=${PS2PDF}"
	@echo "GCS2SFZ=${GCS2SFZ}"
	@echo "SFZ_LIBDIR=${SFZ_LIBDIR}"
	@echo "SFZ_CSVDIR=${SFZ_CSVDIR}"
	@echo "SFZ_MIXDIR=${SFZ_MIXDIR}"
	@echo "PAGES=${PAGES}"
	@echo "MEASURES=${MEASURES}"
	@echo "VOICES=${VOICES}"
#--	@echo "WHICH=${WHICH}"
#=============================================================================
#-- .PHONY: all		#-- There is a bug in gmake. I don't understand.
all: ${V}_m.ps ${V}.fs ${V}.mid ${V}.pdf
all: ${V}_m2.ps ${V}_2.fs ${V}_2.mid ${V}_2.pdf
#-----------------------------------------------------------------------------
fs: ${V}.fs ${V}_2.fs
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${V}.mp4
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${V}_2.mp4
#-----------------------------------------------------------------------------
# SFZ (VPO sampler) pipeline: real orchestral samples via sfizz_render
#   make sfz          - render to sfz-mix/${V}-sfz-mix.wav
#   make sfz-mp4      - encode to ${V}-sfz.mp4 video
#   make sfz-clean    - remove sfz-csv/ and sfz-mix/ directories
#
# Per-piece flow:
#   .gcs -> .E -> [imscomp --sfzpipecsv] -> sfz-csv/*.csv
#                                          -> [gcs2sfz]      -> sfz-mix/${V}-sfz-mix.wav
#                                          -> [ffmpeg]        -> ${V}-sfz.mp4
#-----------------------------------------------------------------------------
# 1) Generate per-instrument CSVs via imscomp --sfzpipecsv
#    Runs in the target CSV directory (absolute paths via CURDIR).
sfz-csv/${V}/.done: ${V}.E ${IMSCOMP}
	@mkdir -p $(dir $@)
	@rm -f $(dir $@)*.csv
	cd $(dir $@) && $(CURDIR)/../../ims/imscomp --sfzpipecsv $(CURDIR)/${V}.E
	@touch $@
#-----------------------------------------------------------------------------
# 2) Render instrument CSVs to a mixed WAV via gcs2sfz + sfizz_render
sfz-mix/${V}-sfz-mix.wav: sfz-csv/${V}/.done
	@mkdir -p $(dir $@)
	"$(CURDIR)/$(GCS2SFZ)" "$(CURDIR)/$(SFZ_CSVDIR)/${V}" \
		--outdir "$(CURDIR)/$(SFZ_MIXDIR)" \
		--sfzdir "$(CURDIR)/$(SFZ_LIBDIR)"
#-----------------------------------------------------------------------------
# 3) Encode SFZ mix to MP4 (same visual style as gcs2youtube: title card + waveform)
${V}-sfz.mp4: sfz-mix/${V}-sfz-mix.wav ${V}_2.fs ${GCS2YOUTUBE} ${WAV2WAVEFORM} ${FS2ASS}
	${GCS2YOUTUBE} -a "$<" -o "$@" "${V}_2.fs" || (rm -f $@ ; exit 1)
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
clean:
	rm -f *.abc *.E
	rm -f *.ps
	rm -f *.fs
	rm -f *.mid
	rm -f *.csv
	rm -f *.mp4
	rm -f *.v *.h *.wav
	rm -f ${V}.pdf ${V}_2.pdf
	rm -rf ${SFZ_CSVDIR} ${SFZ_MIXDIR}
	rm -f ${V}-sfz.mp4
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
