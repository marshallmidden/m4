#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
#++ = to do yet.
#-  = macros -- they are not implemented yet.
#   = converted, probably (maybe) okay.
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

SONGS += bellanotte		# Needs triplets(4?) put in
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
SONGS += vinci
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
SONGS += gnome
SONGS += inv1
SONGS += little
SONGS += pete
SONGS += rand_rock
SONGS += sonata
SONGS += sympathy
SONGS += turkish
SONGS += three_king
SONGS += winterwind
SONGS += double_rat
SONGS += kije
SONGS += putd-test

SONGS += cradle
SONGS += macro-aer-ber
SONGS += macros-complicated

# SONGS += nachtmusik		# NOTDONEYET Figure out GOTOs and VOLUMEs
#				# Missing rests, voice in the middle of a measure, goto 5 -- no 5??
BROKEN += nachtmusik

#++ SONGS += slidall		# Cannot do this in python or MIDI.


#-----------------------------------------------------------------------------
SONGS_mid = ${SONGS:=.mid}
BROKEN_mid = ${BROKEN:=.mid}
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc
.SUFFIXES: .mid .csv .ps .abc _m.ps
# output $@   -- input $<
%.mid:  %.csv
	csvmidi $< $@
%.csv:  %
	${MUSICOMP2ABC} --midi1csv $< $@
%.abc:  %
	${MUSICOMP2ABC} --abc $< $@
%.ps:   %.abc
	${YAPS} $< -o $@ -k
%_m.ps: %.abc
	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
#-----------------------------------------------------------------------------
.PHONY: all
all: ${SONGS_mid}
#-----------------------------------------------------------------------------
.PHONY: broken
broken: ${BROKEN_mid}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.csv *.mid
	rm -f *.abc
#-----------------------------------------------------------------------------
# End of Makefile
