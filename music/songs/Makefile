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
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -P -C -CC
#-----------------------------------------------------------------------------
# SONGS_GCS = ${SONGS:=.gcs}
SONGS_E = ${SONGS:=.E} ${SONGS_NO_PS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_CSV = ${SONGS:=.csv} ${SONGS_NO_PS:=.csv}
SONGS_MID = ${SONGS:=.mid} ${SONGS_NO_PS:=.mid}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PS = ${SONGS:=.ps}
SONGS_WAV = ${SONGS:=.wav} ${SONGS_NO_PS:=.wav}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
BROKEN_MID = ${BROKEN:=.mid}
#-----------------------------------------------------------------------------
.PRECIOUS: %.csv %.abc %.mid %.E
.SUFFIXES: .mid .csv .ps .abc _m.ps .wav .E
# output $@   -- input $<
%.csv:  %.E
	${MUSICOMP2ABC} --midi1csv $< $@ \
		|| (rm -f $@ ; exit 1)
%.mid:  %.csv
	csvmidi $< $@ \
		|| (rm -f $@ ; exit 1)
%.wav:  %.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 $< \
		|| (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} --abc $< $@ \
		|| (rm -f $@ ; exit 1)
%.ps:   %.abc
	${YAPS} $< -o $@ -k \
		|| (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
#-- %.E: %.gcs beethoven.macros
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert all working (?) .gcs songs to .mid files.'
	@echo '  broken    - Attempt to convert all broken .gcs songs to .mid files.'
	@echo "  all_ps    - Use ${YAPS} to create .ps (postscript) files from .gcs files."
	@echo "  all_m_ps  - Use ${ABCM2PS} to create .ps (postscript) files from .gcs files."
	@echo '  all_wav   - Use fluidsynth to create .wav (audio) files from .gcs files.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps, and .wav files.'
	@echo "SONGS: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all all_mid
all all_mid: ${SONGS_MID}
#-----------------------------------------------------------------------------
.PHONY: broken
broken: ${BROKEN_MID}
#-----------------------------------------------------------------------------
.PHONY: all_ps
all_ps: ${SONGS_PS}
#-----------------------------------------------------------------------------
.PHONY: all_m_ps
all_m_ps: ${SONGS_M_PS}
#-----------------------------------------------------------------------------
.PHONY: all_wav
all_wav: ${SONGS_WAV}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_CSV} ${SONGS_MID} ${SONGS_ABC} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_WAV} ${SONGS_E}
	rm -f *.stderr
#-----------------------------------------------------------------------------
# End of Makefile
