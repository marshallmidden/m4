#-----------------------------------------------------------------------------
SONGS  =
SONGS += 5-dollar-fuga
SONGS += BigBad
SONGS += BlueBird
SONGS += Castillo
SONGS += Choral
SONGS += Coke
SONGS += ForAllWeKnow
SONGS += Inv12
SONGS += Inv3a
SONGS += JC
SONGS += Madrigal
SONGS += SixtyFour
SONGS += SuperStar
SONGS += WaltzOfTheFlowers
SONGS += andante
SONGS += bbb-huh
SONGS += courante
SONGS += fuga-vii
SONGS += fuga-vii-2
SONGS += inv3
SONGS += inventio-15
SONGS += invention8
SONGS += macros-abstraction
SONGS += minuet-in-G-major
SONGS += promenade
SONGS += promenade2
SONGS += promenade3
SONGS += promenade4
SONGS += promenade5
SONGS += sonic-squeak
SONGS += wonderland-winter

# Needs triplets(4?) put in
SONGS += bellanotte
SONGS += boughs
SONGS += caroling
SONGS += children
SONGS += fatetowind
SONGS += fellow
SONGS += field
SONGS += frosty
SONGS += fugue-in-c-minor
SONGS += heighho
SONGS += holiday
SONGS += hop
SONGS += indiana
SONGS += jinglerock
SONGS += largo-nws
SONGS += letitsnow
SONGS += littletown
SONGS += macros-fugue-in-c
SONGS += mary
SONGS += merry
SONGS += minuet-in-g
SONGS += moscows
SONGS += nowell
SONGS += raindeer
SONGS += rednose
SONGS += russian-folk-song
SONGS += shepherds
SONGS += silentnite
SONGS += sol
SONGS += sonatina
SONGS += trokiaride
SONGS += uponastar
SONGS += waltz-in-a-minor
SONGS += wassailing
SONGS += wondersong

SONGS += m1a
SONGS += m1b
SONGS += chopin
SONGS += david
SONGS += handel
SONGS += kleine-suite
SONGS += JC1
SONGS += amerika
SONGS += bbs
SONGS += caisson
SONGS += gooch
SONGS += lenore
SONGS += lindsay
SONGS += mag
SONGS += moment
SONGS += morgan
SONGS += partita
SONGS += roses
SONGS += stripes
SONGS += taps
SONGS += warren1
SONGS += brand3
SONGS += chicks

SONGS += dream
SONGS += little
SONGS += pete
SONGS += rand_rock
SONGS += sympathy
SONGS += turkish
SONGS += three_king
SONGS += winterwind
SONGS += double_rat
SONGS += kije
SONGS += putd-test

SONGS += cradle
SONGS += macro-aer-ber

SONGS += gnome

# NOTDONEYET Figure out GOTOs and VOLUMEs
SONGS += nachtmusik
#				# Missing rests, voice in the middle of a measure, goto 5 -- no 5??
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# A few bits that I wanted to know what they were...
SONGS += amadeus-cafe
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Following will not create postscript files correctly - printable note length problems.
SONGS_NO_PS += inv1
SONGS_NO_PS += vinci
SONGS_NO_PS += sonata
SONGS_NO_PS += macros-complicated
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Cannot do this in python or MIDI.
# MAYBE could do this with PTE and portamento.
BROKEN += slidall

#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../musicomp2abc/musicomp2abc
IMSCOMP = ../ims/imscomp
#-- YAPS         = yaps
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
MC_ARGS ?= --nofillwarning --nooncewarning --norestswarning --noleftoverwarnings --notrillwarnings
#-- MC_ARGS += --one
#-- MC_ARGS += --two
#-- MC_ARGS += --sf2=/Users/m4/thisisthesoundfont.sf2
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
# -E                    Only run the preprocessor.
# -x c                  Treat input file(s) as having <language> 'c'.
# -Wno-invalid-pp-token
# -undef
# -Wundef
# -Werror               Turn warnings into errors.
# -nostdinc             No standard include directory.
# -P                    Disable linemarker output in -E mode.
# -C                    Include comments in preprocessed output.
# -CC                   Include comments from within macros in preprocessed output.
# -traditional-cpp      Enable some traditional CPP emulation.
#
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
#-----------------------------------------------------------------------------
# SONGS_GCS = ${SONGS:=.gcs}
SONGS_E = ${SONGS:=.E} ${SONGS_NO_PS:=.E}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
SONGS_ABC = ${SONGS:=.abc}
SONGS_CSV = ${SONGS:=.csv} ${SONGS_NO_PS:=.csv}
SONGS_MID = ${SONGS:=.mid} ${SONGS_NO_PS:=.mid}
SONGS_FS = ${SONGS:=.fs} ${SONGS_NO_PS:=.fs}
SONGS_M_PS = ${SONGS:=_m.ps}
#-- SONGS_PS = ${SONGS:=.ps}
SONGS_PDF= ${SONGS:=.pdf}
SONGS_WAV = ${SONGS:=.wav} ${SONGS_NO_PS:=.wav}
SONGS_V= ${SONGS:=.v}
SONGS_H= ${SONGS:=.h}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
BROKEN_MID = ${BROKEN:=.mid}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
SONGS_2ABC = ${SONGS:=_2.abc}
SONGS_2CSV = ${SONGS:=_2.csv} ${SONGS_NO_PS:=_2.csv}
SONGS_2MID = ${SONGS:=_2.mid} ${SONGS_NO_PS:=_2.mid}
SONGS_2FS = ${SONGS:=_2.fs} ${SONGS_NO_PS:=_2.fs}
SONGS_M2_PS = ${SONGS:=_m2.ps}
#-- SONGS_2PS = ${SONGS:=_2.ps}
SONGS_2PDF= ${SONGS:=_2.pdf}
SONGS_2WAV = ${SONGS:=_2.wav} ${SONGS_NO_PS:=_2.wav}
SONGS_2V= ${SONGS:=_2.v}
SONGS_2H= ${SONGS:=_2.h}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
BROKEN_2MID = ${BROKEN:=_2.mid}
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PRECIOUS: %.E
.SUFFIXES: .E
.PRECIOUS: %.csv %.abc %.mid %_m.ps
.SUFFIXES: .csv .abc .mid _m.ps .fs .wav .pdf .ps
.PRECIOUS: %_2.csv %_2.abc %_2.mid %_m2.ps
.SUFFIXES: _2.csv _2.abc _2.mid _m2.ps _2.fs _2.wav _2.pdf _2.ps
# output $@   -- input $<
%.fs:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%_2.fs:  %.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@ || (rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%_2.csv:  %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || (rm -f $@ ; exit 1)
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid:  %_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.wav:  %.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
%_2.wav:  %_2.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc:  %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
#-- %.ps:   %.abc
#-- 	${YAPS} $< -o $@ -k || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%_m2.ps: %_2.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%_2.pdf:	%_m2.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.v:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%_2.v:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%.h:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
%_2.h:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert all working (?) .gcs songs to .mid files.'
	@echo '  broken    - Attempt to convert all broken .gcs songs to .mid files.'
#-- 	@echo "  all_ps    - Use ${YAPS} to create .ps (postscript) files from .gcs files."
	@echo "  all_m_ps  - Use ${ABCM2PS} to create .ps (postscript) files from .gcs files."
	@echo '  all_wav   - Use fluidsynth to create .wav (audio) files from .gcs files.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps, and .wav files.'
	@echo "SONGS: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all all_mid
all all_mid: ${SONGS_MID} all_fs all_pdf ${SONGS_2MID}
#-----------------------------------------------------------------------------
.PHONY: broken
broken: ${BROKEN_MID} ${BROKEN_2MID}
#-----------------------------------------------------------------------------
#-- .PHONY: all_ps
#-- all_ps: ${SONGS_PS} ${SONGS_2PS}
#-----------------------------------------------------------------------------
.PHONY: all_m_ps
all_m_ps: ${SONGS_M_PS} ${SONGS_M2_PS}
#-----------------------------------------------------------------------------
.PHONY: all_pdf
all_pdf: ${SONGS_PDF} ${SONGS_2PDF}
#-----------------------------------------------------------------------------
.PHONY: all_pdf2
all_pdf2: ${SONGS_2PDF}
#-----------------------------------------------------------------------------
.PHONY: all_wav
all_wav: ${SONGS_WAV} ${SONGS_2WAV}
#-----------------------------------------------------------------------------
.PHONY: fs all_fs
fs all_fs: ${SONGS_FS} ${SONGS_2FS}
#-----------------------------------------------------------------------------
.PHONY: fs2 all_fs2
fs2 all_fs2: ${SONGS_2FS}
#-----------------------------------------------------------------------------
.PHONY: v all_v
v all_v: ${SONGS_V} ${SONGS_2V}
#-----------------------------------------------------------------------------
.PHONY: h all_h
h all_h: ${SONGS_H} ${SONGS_2H}
#-----------------------------------------------------------------------------
.PHONY: test
test:
	@echo SONGS_MID=${SONGS_MID}
	@echo SONGS_2MID=${SONGS_2MID}
	@echo BROKEN_MID=${BROKEN_MID}
	@echo BROKEN_2MID=${BROKEN_2MID}
	@echo SONGS_PS=${SONGS_PS}
	@echo SONGS_2PS=${SONGS_2PS}
	@echo SONGS_M_PS=${SONGS_M_PS}
	@echo SONGS_M2_PS=${SONGS_M2_PS}
	@echo SONGS_PDF=${SONGS_PDF}
	@echo SONGS_2PDF=${SONGS_2PDF}
	@echo SONGS_WAV=${SONGS_WAV}
	@echo SONGS_2WAV=${SONGS_2WAV}
	@echo SONGS_FS=${SONGS_FS}
	@echo SONGS_2FS=${SONGS_2FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
#--	rm -f ${SONGS_E}
#--	rm -f ${SONGS_CSV} ${SONGS_MID} ${SONGS_WAV} ${SONGS_FS}
#--	rm -f ${SONGS_ABC} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_PDF}
#--	rm -f ${SONGS_V} ${SONGS_H}

#--	rm -f ${SONGS_2CSV} ${SONGS_2MID} ${SONGS_2WAV} ${SONGS_2FS}
#--	rm -f ${SONGS_2ABC} ${SONGS_2PS} ${SONGS_M2_PS} ${SONGS_2PDF}
#--	rm -f ${SONGS_2V} ${SONGS_2H}

	rm -f ${SONGS_2PDF} ${SONGS_PDF}
	rm -f *.v *.h *.abc *.ps *.mid *.wav *.fs *.E *.csv
	rm -f *.stderr
#-----------------------------------------------------------------------------
# End of Makefile
