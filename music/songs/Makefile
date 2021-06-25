#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../musicomp2abc/musicomp2abc
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
SONGS += macro-aer-ber
# SONGS += macros-complicated	# NOTDONEYET SOMETHING WRONG WITH "decode m22,A" and then "r(m22)".
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
# SONGS += nachtmusik		# NOTDONEYET Figure out GOTOs and VOLUMEs
SONGS += nowell
SONGS += raindeer
SONGS += rednose
SONGS += russian-folk-song
SONGS += shepherds
SONGS += silentnite
#++ SONGS += slidall
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
SONGS += cradle
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

#-----------------------------------------------------------------------------
#-- SONGS_abc = ${SONGS:=.abc}
#-- SONGS_ps = ${SONGS:=.ps}
#-- SONGS_csv = ${SONGS:=.csv}
SONGS_mid = ${SONGS:=.mid}
#-----------------------------------------------------------------------------
.PHONY: all
#-- all: ${SONGS_ps} ${SONGS_mid} ${SONGS_abc}
all: ${SONGS_mid}
#-- ${SONGS_mid}: ${SONGS_csv}
#-----------------------------------------------------------------------------
.SUFFIXES:		# delete the default suffixes
#-- .SUFFIXES:  .abc 1.mid .ps
#-- .SUFFIXES:  .mid .csv
.SUFFIXES:  .mid
#-----------------------------------------------------------------------------
#-- %1.mid: %.abc
#-- 	abc2midi $<
%.mid: %
	${MUSICOMP2ABC} -m < $< > $<.csv || (rm -f $@ ; exit 1)
	csvmidi $<.csv > $@
#-- %.ps: %.abc
#-- 	yaps $< -k
#-- %.abc: %
#-- 	${MUSICOMP2ABC} < $< > $@ || (rm -f $@ ; exit 1)
#-- %.csv: %
#-----------------------------------------------------------------------------
#-- ${SONGS_ps}: ${SONGS_abc}
#-- ${SONGS_mid}: ${SONGS_abc}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.mid *.ps *.abc *.csv
#-----------------------------------------------------------------------------
