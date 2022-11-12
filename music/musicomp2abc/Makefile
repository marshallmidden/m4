#-----------------------------------------------------------------------------
SONGS  =
SONGS += b-6
# SONGS += inv15-example			# Measures do not have meter corrections.
# SONGS += inv15-example.working		# Measures do not have meter corrections.
SONGS += loudness.mcomp
SONGS += promenade
SONGS += promenade-musicomp2abc
#-----------------------------------------------------------------------------
TESTS  =
TESTS += KEYg-
TESTS += DECODE
TESTS += LEGATO1
TESTS += TIES
TESTS += DRUMS
TESTS += WRONGTIES
TESTS += TRILLS
TESTS += TEMPO
TESTS += RANGE1
TESTS += RANGE2
TESTS += CRESC1
TESTS += CRESC2
TESTS += XPOSE1
TESTS += XPOSE2
#-----------------------------------------------------------------------------
# SONGS_GCS = ${SONGS:=.gcs}
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
#+ SONGS_CSV = ${SONGS:=.csv}
SONGS_FS = ${SONGS:=.fs}
#+ SONGS_MID = ${SONGS:=.mid}
SONGS_M_PS = ${SONGS:=_m.ps}
#+ SONGS_PS = ${SONGS:=.ps}
#+ SONGS_WAV = ${SONGS:=.wav}
#-----------------------------------------------------------------------------
TESTS_E = ${TESTS:=.E}
TESTS_ABC = ${TESTS:=.abc}
#+ TESTS_CSV = ${TESTS:=.csv}
TESTS_FS = ${TESTS:=.fs}
#+ TESTS_MID = ${TESTS:=.mid}
TESTS_M_PS = ${TESTS:=_m.ps}
#+ TESTS_PS = ${TESTS:=.ps}
#+ TESTS_WAV = ${TESTS:=.wav}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
#++ MC_ARGS =
MC_ARGS = --warn_octave_accidental
#-- MC_ARGS = --noxpose --voices=9
#+ YAPS	     = yaps
ABCM2PS	     = abcm2ps
#-----------------------------------------------------------------------------
# -E			Only run the preprocessor.
# -x c			Treat input file(s) as having <language> 'c'.
# -Wno-invalid-pp-token
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
#+.PRECIOUS: %.abc %.csv %.mid %.E
#+ .SUFFIXES: .mid .csv .ps .abc _m.ps .wav .E .fs
.PRECIOUS: %.abc %.E %.csv
.SUFFIXES: .abc _m.ps .E .fs .csv .mid
# output $@   -- input $<
%.fs:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --two --fluidsynth $< $@ || rm -f $@
%.csv:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --midi1csv $< $@ || rm -f $@
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
#+ %.wav:  %.mid
#+ 	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 $<
%.abc:  %.E
	${MUSICOMP2ABC} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
#+ %.ps:   %.abc
#+ 	${YAPS} $< -o $@ -k || (rm -f $@ ; exit 1)
%_m.ps: %.abc
#-	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
%.v:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%.h:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
	@echo '  tags      - Create tags file for python sources.'
	@echo '  tests     - Convert TESTS.gcs files to .mid files, etc.'
	@echo "SONGS: ${SONGS}"
	@echo "TESTS: ${TESTS}"
#-----------------------------------------------------------------------------
.PHONY: all songs ${SONGS}
#+ all songs ${SONGS}: ${SONGS_MID} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_WAV} ${SONGS_FS}
all songs ${SONGS}: ${SONGS_M_PS} ${SONGS_FS}
#-----------------------------------------------------------------------------
.PHONY: tests ${TESTS}
#+ tests ${TESTS}: ${TESTS_MID} ${TESTS_PS} ${TESTS_M_PS} ${TESTS_WAV} ${TESTS_FS}
tests ${TESTS}: ${TESTS_M_PS} ${TESTS_FS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
#+	rm -f ${SONGS_CSV} ${TESTS_CSV}
	rm -f ${SONGS_FS} ${TESTS_FS}
#+	rm -f ${SONGS_MID} ${TESTS_MID}
	rm -f ${SONGS_ABC} ${TESTS_ABC}
#+	rm -f ${SONGS_PS} ${TESTS_PS}
	rm -f ${SONGS_M_PS} ${TESTS_M_PS}
#+	rm -f ${SONGS_WAV} ${TESTS_WAV}
	rm -f ${SONGS_E} ${TESTS_E}
	rm -f *.stderr
	rm -f *.h *.v *.csv *.mid
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP2ABC} calculate.py
#-----------------------------------------------------------------------------
