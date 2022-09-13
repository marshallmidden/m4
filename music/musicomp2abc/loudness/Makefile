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
SONGS += loudness.piano
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
SONGS += loudness.piano.atten
#-----------------------------------------------------------------------------
SONGS_FS = ${SONGS:=.fs}
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
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
	@echo "SONGS: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all songs ${SONGS}
all songs ${SONGS}: ${SONGS_FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS}
	rm -f *.stderr
#-----------------------------------------------------------------------------
