#-----------------------------------------------------------------------------
SONGS  =
SONGS += b-6
# Measures do not have meter corrections.
SONGS += inv15-example
# Measures do not have meter corrections.
SONGS += inv15-example.working
SONGS += loudness.mcomp
SONGS += promenade
SONGS += promenade-musicomp2abc
#?? SONGS += tc
SONGS += Aflatmajor
#-----------------------------------------------------------------------------
TESTS  =
TESTS += KEYg-
TESTS += KEYS
TESTS += DECODE
TESTS += DECODE-2
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
TESTS += SHARPFLAT
TESTS += MiddleC
TESTS += VARIABLES
TESTS += CSTOP
#-----------------------------------------------------------------------------
# SONGS_GCS = ${SONGS:=.gcs}
#+ SONGS_CSV = ${SONGS:=.csv}
#+ SONGS_MID = ${SONGS:=.mid}
#+ SONGS_PS = ${SONGS:=.ps}
#+ SONGS_WAV = ${SONGS:=.wav}
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PDF= ${SONGS:=.pdf}
SONGS_H= ${SONGS:=.h}
SONGS_V= ${SONGS:=.v}
SONGS_2ABC = ${SONGS:=_2.abc}
SONGS_2FS = ${SONGS:=_2.fs}
SONGS_M2_PS = ${SONGS:=_m2.ps}
SONGS_2PDF= ${SONGS:=_2.pdf}
SONGS_2H= ${SONGS:=_2.h}
SONGS_2V= ${SONGS:=_2.v}
#-----------------------------------------------------------------------------
TESTS_E = ${TESTS:=.E}
TESTS_ABC = ${TESTS:=.abc}
#+ TESTS_CSV = ${TESTS:=.csv}
TESTS_FS = ${TESTS:=.fs}
#+ TESTS_MID = ${TESTS:=.mid}
TESTS_M_PS = ${TESTS:=_m.ps}
TESTS_PDF= ${TESTS:=.pdf}
#+ TESTS_PS = ${TESTS:=.ps}
#+ TESTS_WAV = ${TESTS:=.wav}
#-----------------------------------------------------------------------------
MUSICOMP = ./musicomp2abc
IMSCOMP = ~/ims/imscomp
MC_ARGS =
#++ MC_ARGS = --warn_octave_accidental
#-- MC_ARGS = --noxpose --voices=9
#+ YAPS	     = yaps
ABCM2PS	     = abcm2ps
PS2PDF       = ps2pdf
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
.PRECIOUS: %.abc %.E %.csv %_m.ps
.SUFFIXES: .abc _m.ps .E .fs .csv .mid .pdf
.PRECIOUS: %_2.abc %_2.csv %_m2.ps
.SUFFIXES: _2.abc _m2.ps _2.fs _2.csv _2.mid _2.pdf
# output $@   -- input $<
%.fs:   %.E
	${MUSICOMP} ${MC_ARGS} --two --fluidsynth $< $@ || rm -f $@
%_2.fs: %.E
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@ || rm -f $@
%.csv:  %.E
	${MUSICOMP} ${MC_ARGS} --midi1csv $< $@ || rm -f $@
%_2.csv: %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@ || rm -f $@
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%_2.mid: %_2.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_2.abc: %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%_m2.ps: %_2.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%_2.pdf: %_m2.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.v:    %.E ${MUSICOMP}
	${MUSICOMP} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%_2.v:  %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%.h:    %.E ${MUSICOMP}
	${MUSICOMP} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
%_2.h:  %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
#+ %.ps:   %.abc
#+ 	${YAPS} $< -o $@ -k || (rm -f $@ ; exit 1)
#+ %.wav:  %.mid
#+ 	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 $<
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
#- all songs ${SONGS}: ${SONGS_MID} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_WAV} ${SONGS_FS}
#- all songs ${SONGS}: ${SONGS_M_PS} ${SONGS_FS} ${SONGS_PDF}
all songs ${SONGS}: ${SONGS_M_PS} ${SONGS_M2_PS} ${SONGS_FS} ${SONGS_2FS} ${SONGS_PDF} ${SONGS_2PDF}
#-----------------------------------------------------------------------------
.PHONY: v h
v: ${SONGS_2V} ${SONGS_V}
h: ${SONGS_2H} ${SONGS_H}
#-----------------------------------------------------------------------------
.PHONY: tests ${TESTS}
#- tests ${TESTS}: ${TESTS_MID} ${TESTS_PS} ${TESTS_M_PS} ${TESTS_WAV} ${TESTS_FS}
#- tests ${TESTS}: ${TESTS_M_PS} ${TESTS_FS} ${TESTS_PDF}
tests ${TESTS}: ${TESTS_M_PS} ${TESTS_M2_PS} ${TESTS_FS} ${TESTS_2FS} ${TESTS_PDF} ${TESTS_2PDF}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS} ${TESTS_FS}
	rm -f ${SONGS_2FS} ${TESTS_2FS}
	rm -f ${SONGS_ABC} ${TESTS_ABC}
	rm -f ${SONGS_2ABC} ${TESTS_2ABC}
	rm -f ${SONGS_M_PS} ${TESTS_M_PS}
	rm -f ${SONGS_M2_PS} ${TESTS_M2_PS}
	rm -f ${SONGS_PDF} ${TESTS_PDF}
	rm -f ${SONGS_2PDF} ${TESTS_2PDF}
	rm -f ${SONGS_E} ${TESTS_E}
	rm -f *.stderr *.stdout
	rm -f *.h *.v *.csv *.mid *.E
	rm -f *_2.h *_2.v *_2.csv *_2.mid
#-----------------------------------------------------------------------------
#+	rm -f ${SONGS_CSV} ${TESTS_CSV}
#+	rm -f ${SONGS_MID} ${TESTS_MID}
#+	rm -f ${SONGS_PS} ${TESTS_PS}
#+	rm -f ${SONGS_WAV} ${TESTS_WAV}
#-----------------------------------------------------------------------------
.PHONY: check
check:
	echo "SONGS=${SONGS}"
	echo "SONGS_E=${SONGS_E}"
	echo "SONGS_ABC=${SONGS_ABC}"
	echo "SONGS_2ABC=${SONGS_2ABC}"
	echo "SONGS_FS=${SONGS_FS}"
	echo "SONGS_2FS=${SONGS_2FS}"
	echo "SONGS_M_PS=${SONGS_M_PS}"
	echo "SONGS_M2_PS=${SONGS_M2_PS}"
	echo "TESTS=${TESTS}"
	echo "TESTS_E=${TESTS_E}"
	echo "TESTS_ABC=${TESTS_ABC}"
	echo "TESTS_2ABC=${TESTS_2ABC}"
	echo "TESTS_FS=${TESTS_FS}"
	echo "TESTS_2FS=${TESTS_2FS}"
	echo "TESTS_M_PS=${TESTS_M_PS}"
	echo "TESTS_M2_PS=${TESTS_M2_PS}"
	echo "MUSICOMP=${MUSICOMP}"
	echo "IMSCOMP=${IMSCOMP}"
	echo "MC_ARGS=${MC_ARGS}"
	echo "ABCM2PS=${ABCM2PS}"
	echo "PS2PDF=${PS2PDF}"
	echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP} calculate.py
#-----------------------------------------------------------------------------
# End of file Makefile for musicomp2abc.
