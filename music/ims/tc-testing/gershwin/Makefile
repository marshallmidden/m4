#-----------------------------------------------------------------------------
# SONGS  = G21clar
# SONGS += G21flugel
# SONGS += G22bassn
# SONGS += G22clar
# SONGS += G22flugel
# SONGS += G22flute
# SONGS += G22horn
# SONGS += G22oboe
# SONGS += G22perc
# SONGS += G22strng
# SONGS += Gershwin_Concerto_move_2.1
# SONGS += Gershwin_Concerto_move_2.1_
# SONGS += Gershwin_Concerto_move_2.2
# SONGS += Gershwin_Concerto_move_2.2_
# SONGS += Gershwin_Concerto_move_3
SONGS += new-g3
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_PDF= ${SONGS:=.pdf}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
SONGS_2ABC = ${SONGS:=_2.abc}
SONGS_2FS = ${SONGS:=_2.fs}
SONGS_2PS = ${SONGS:=_2.ps}
SONGS_2PDF= ${SONGS:=_2.pdf}
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
SONGS_MP4 = ${SONGS:=.mp4}
SONGS_2_MP4 = ${SONGS:=_2.mp4}
#-----------------------------------------------------------------------------
IMSCOMP = ../../imscomp
MUSICOMP2ABC = ../../../musicomp2abc/musicomp2abc
GCS2YOUTUBE  = ../../../gcs2youtube
#-- MC_ARGS ?= --platochars --lines
MC_ARGS ?= --platochars
#-- Default soundfont is whatever is with the system.
#-- MC_ARGS += --one		# Defined in imscomp/musicomp2abc
#-- MC_ARGS += --two		# Defined in imscomp/musicomp2abc
#-- MC_ARGS += --sf /Users/m4/thisisthesoundfont.sf2
#-----------------------------------------------------------------------------
ABCM2PS	     = abcm2ps
PS2PDF       = ps2pdf
#-----------------------------------------------------------------------------
# -E			Only run the preprocessor.
# -x c			Treat input file(s) as having <language> 'c'.
#
# -undef
# -Wundef
# -Werror		Turn warnings into errors.
# -nostdinc		No standard include directory.
# -P			Disable linemarker output in -E mode.
# -C			Include comments in preprocessed output.
# -CC			Include comments from within macros in preprocessed output.
# -traditional-cpp	Enable some traditional CPP emulation.
#
CPP=gcc -E -x c -nostdinc -C -CC -Wno-error -Wno-extra -traditional-cpp
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv
.PRECIOUS: %_2.abc %_2.csv
.SUFFIXES: .abc .E .fs .csv .mid .pdf
.SUFFIXES: _2.abc _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.E: %.gcs
	${CPP} $< -o $@
%.fs:  %.E
	${IMSCOMP} ${MC_ARGS} --fluidsynth $< $@
%.csv:  %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@
%.mid:  %.csv
	csvmidi $< $@
%.abc:  %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@
%.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%.ps
	${PS2PDF} $< $@
%.v:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%_2.fs:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --fluidsynth $< $@
%_2.csv:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@
%_2.mid:  %_2.csv
	csvmidi $< $@
%_2.abc:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@
%_2.ps: %_2.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%_2.pdf:	%_2.ps
	${PS2PDF} $< $@
%_2.v:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --vert $< $@
%_2.h:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --hori $< $@
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
%.mp4:	%.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
%_2.mp4:	%_2.fs
	${GCS2YOUTUBE} $< || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  check     - Print out the Makefile Variables.'
	@echo '  clean     - Remove intermediate .csv, .mid, .abc, .ps, .fs, and .mp4 files.'
	@echo '  fs        - Make SONGS as fluidsynth (.fs) output.'
	@echo '  fs2       - Make SONGS as fluidsynth (_2.fs) output (musicomp2abc).'
	@echo '  mp4       - Make SONGS as YouTube (.mp4) output.'
	@echo '  mp4_2     - Make SONGS as YouTube (_2.mp4) output (musicomp2abc).'
	@echo '  ps        - Make SONGS as postscript (.ps) output.'
	@echo '  pdf       - Make SONGS as postscript (.pdf) output.'
	@echo "songs: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all songs SONGS ${SONGS}
all songs SONGS ${SONGS}: ${SONGS_PS} ${SONGS_FS} ${SONGS_PDF} ${SONGS_2PS} ${SONGS_2FS} ${SONGS_2PDF}
#-----------------------------------------------------------------------------
.PHONY: fs
fs: ${SONGS_FS}
.PHONY: fs2
fs2: ${SONGS_2FS}
.PHONY: ps
ps: ${SONGS_PS}
.PHONY: pdf
pdf: ${SONGS_PDF}
#-----------------------------------------------------------------------------
.PHONY: mp4
mp4: ${SONGS_MP4}
#-----------------------------------------------------------------------------
.PHONY: mp4_2
mp4_2: ${SONGS_2_MP4}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS} ${SONGS_2FS}
	rm -f ${SONGS_ABC} ${SONGS_2ABC}
	rm -f ${SONGS_PS} ${SONGS_2PS}
	rm -f ${SONGS_PDF} ${SONGS_2PDF}
	rm -f ${SONGS_E}
	rm -f *.stderr
	rm -f *.h *.v *.csv *.mid *.E *.fs *.pdf *.ps *.abc *.wav *.mp4
	rm -f AAA.[0-9][0-9] BBB.[0-9][0-9]
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "SONGS=${SONGS}"
	@echo "SONGS_E=${SONGS_E}"
	@echo "SONGS_ABC=${SONGS_ABC}"
	@echo "SONGS_FS=${SONGS_FS}"
	@echo "SONGS_PS=${SONGS_PS}"
	@echo "SONGS_2ABC=${SONGS_2ABC}"
	@echo "SONGS_2FS=${SONGS_2FS}"
	@echo "SONGS_2PS=${SONGS_2PS}"
	@echo "SONGS_2PDF=${SONGS_2PDF}"
	@echo "SONGS_MP4=${SONGS_MP4}"
	@echo "SONGS_2_MP4=${SONGS_2_MP4}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	@echo "GCS2YOUTUBE=${GCS2YOUTUBE}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "PS2PDF=${PS2PDF}"
	@echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
new-g3.ps: new-g3.abc
# Unique options for new-g3.abc. Grumble.
	(${ABCM2PS} -M -b 0 $< -i -N 2 -j 1 ; mv Out.ps $@)
#-----------------------------------------------------------------------------
