#-----------------------------------------------------------------------------
SONGS  =
SONGS += loudness.bassdrum
SONGS += loudness.bassoon
SONGS += loudness.brass
SONGS += loudness.cello
SONGS += loudness.clarinet
SONGS += loudness.contrabass
SONGS += loudness.cymbal
SONGS += loudness.englishhorn
SONGS += loudness.flute
SONGS += loudness.frenchhorn
SONGS += loudness.gunshot
SONGS += loudness.oboe
SONGS += loudness.piccolo
SONGS += loudness.pizz
SONGS += loudness.snaredrum
SONGS += loudness.tambourine
SONGS += loudness.timpani
SONGS += loudness.triangle
SONGS += loudness.trombone
SONGS += loudness.tuba
SONGS += loudness.viola
SONGS += loudness.violin
#-----------------------------------------------------------------------------
SONGS_FS = ${SONGS:=.fs}
#-----------------------------------------------------------------------------
FOUR = 
FOUR += loudness.piano
FOUR += loudness.piano.intensity
FOUR += loudness.piano.velocity
FOUR += loudness.piano.3
#-----------------------------------------------------------------------------
FOUR_FS = ${FOUR:=.fs}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
#-----------------------------------------------------------------------------
.SUFFIXES: .fs
%.fs:  %.gcs
	${MUSICOMP2ABC} --fluidsynth $< $@ || rm -f $@
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .fs files, etc.'
	@echo '  four      - Convert FOUR.gcs files to .fs files, etc.'
	@echo '  clean     - Remove resulting files: *.fs'
	@echo "SONGS: ${SONGS}"
	@echo "FOUR: ${FOUR}"
#-----------------------------------------------------------------------------
.PHONY: all songs ${SONGS}
all songs ${SONGS}: ${SONGS_FS}
#-----------------------------------------------------------------------------
.PHONY: songs ${FOUR}
four ${FOUR}: ${FOUR_FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS} ${FOUR_FS}
	rm -f *.stderr
#-----------------------------------------------------------------------------
