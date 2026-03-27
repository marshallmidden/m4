# FILE = AAA	# Piano
# FILE = BBB	# Violin
# FILE = CCC	# Viola
# FILE = DDD	# Cello
# FILE = EEE	# Contrabass
# FILE = FFF	# Timpani
# FILE = GGG	# Trombone
# FILE = HHH	# French Horn
# FILE = III	# Oboe
# FILE = JJJ	# Bassoon
# FILE = JJJ	# Clarinet
# FILE = KKK	# Flute
CSV = ${FILE}.csv
MIDI = ${FILE}.mid
WAV = ${FILE}.wav
# ----------------------------------------------------------------------------
# OS-specific settings for fluidsynth
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SOUNDFONT_GS = /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2
    FLUID_AUDIO  = -a coreaudio
else
    SOUNDFONT_GS = /home/m4/src/GeneralUser_GS/GeneralUser.sf2
    FLUID_AUDIO  =
endif
# ----------------------------------------------------------------------------
.PHONY: all
all:
	./c11.pl > ${CSV}
	csvmidi ${CSV} ${MIDI}
	fluidsynth -F "${WAV}" ${FLUID_AUDIO} -i -n -q -g 0.5 -K 64 ${SOUNDFONT_GS} "${MIDI}"
	cp ${WAV} ~/Downloads/

# ----------------------------------------------------------------------------
