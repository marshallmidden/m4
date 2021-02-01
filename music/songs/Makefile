#-----------------------------------------------------------------------------
MUSICOMP2ABC = ../python/musicomp2abc
#-----------------------------------------------------------------------------
#++ = to do yet.
#-  = macros -- they are not implemented yet.
#   = converted, probably (maybe) okay.
# SONGS += 5-dollar-fuga
# SONGS += BigBad
# SONGS += BlueBird
# SONGS += Castillo
# SONGS += Choral
# SONGS += Coke
# SONGS += ForAllWeKnow
# SONGS += Inv12
# SONGS += Inv3a
# SONGS += JC
# SONGS += Madrigal
# SONGS += SixtyFour
# SONGS += SuperStar
# SONGS += WaltzOfTheFlowers
# SONGS += andante 
# SONGS += bbb-huh
# SONGS += courante
# SONGS += fuga-vii 
# SONGS += fuga-vii-2 
# SONGS += inv3
# SONGS += inventio-15
# SONGS += invention8
#- SONGS += macros-abstraction
#- SONGS += macro-aer-ber
#- SONGS += macros-complicated
# SONGS += minuet-in-G-major
# SONGS += promenade
# SONGS += promenade2
# SONGS += promenade3
# SONGS += promenade4
# SONGS += promenade5
# SONGS += sonic-squeak
# SONGS += wonderland-winter

# SONGS += bellanotte		# Needs triplets(4?) put in
# SONGS += boughs
# SONGS += caroling
# SONGS += children
# SONGS += fatetowind
# SONGS += fellow
# SONGS += field
# SONGS += frosty
# SONGS += fugue-in-c-minor
# SONGS += heighho
# SONGS += holiday
# SONGS += hop
# SONGS += indiana
# SONGS += jinglerock
# SONGS += largo-nws
# SONGS += letitsnow
# SONGS += littletown
#++ SONGS += macros-fugue-in-c
# SONGS += mary
# SONGS += merry
# SONGS += minuet-in-g
# SONGS += moscows
# SONGS += nachtmusik
# SONGS += nowell
# SONGS += raindeer
# SONGS += rednose
# SONGS += russian-folk-song
# SONGS += shepherds
# SONGS += silentnite
#++ SONGS += slidall
# SONGS += sol
# SONGS += sonatina
# SONGS += trokiaride
# SONGS += uponastar
# SONGS += waltz-in-a-minor
# SONGS += wassailing
# SONGS += wondersong
#-----------------------------------------------------------------------------
SONGS_abc = ${SONGS:=.abc}
SONGS_ps = ${SONGS:=.ps}
SONGS_mid = ${SONGS:=1.mid}
#-----------------------------------------------------------------------------
.PHONY: all
all: ${SONGS_ps} ${SONGS_mid} ${SONGS_abc}
#-----------------------------------------------------------------------------
.SUFFIXES:		# delete the default suffixes
.SUFFIXES:  .abc 1.mid .ps
#-----------------------------------------------------------------------------
%1.mid: %.abc
	abc2midi $<
%.ps: %.abc
	yaps $< -k
%.abc: %
	${MUSICOMP2ABC} < $< > $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
${SONGS_ps}: ${SONGS_abc}
${SONGS_mid}: ${SONGS_abc}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.mid *.ps *.abc
#-----------------------------------------------------------------------------
