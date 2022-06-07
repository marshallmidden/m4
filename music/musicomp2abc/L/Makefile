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
.PHONY: all
all:
	./c11.pl > ${CSV}
	csvmidi ${CSV} ${MIDI}
	fluidsynth -F "${WAV}" -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 "${MIDI}"
	cp ${WAV} ~/Downloads/

# ----------------------------------------------------------------------------
