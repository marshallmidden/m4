#-----------------------------------------------------------------------------
TESTS  = p
#.............................................................................
TESTS_E = ${TESTS:=.E}
TESTS_FS = ${TESTS:=.fs}
TESTS_CSV = ${TESTS:=.csv}
TESTS_MID = ${TESTS:=.mid}
#+ TESTS_WAV = ${TESTS:=.wav}
TESTS_ABC = ${TESTS:=.abc}
#+ TESTS_PS = ${TESTS:=.ps}
TESTS_M_PS = ${TESTS:=_m.ps}
TESTS_PDF= ${TESTS:=.pdf}
TESTS_V= ${TESTS:=.v}
TESTS_H= ${TESTS:=.h}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
MC_ARGS =
# MC_ARGS = --xpose '0 b-'
# MC_ARGS = --xpose '0 b-'
# MC_ARGS = --xpose '1 10 b-'
# MC_ARGS = --xpose '1 0 c'
#? MC_ARGS = --xpose '1 12 c'
# MC_ARGS = --warn_octave_accidental
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
CPP=gcc -E -x c -nostdinc -C -CC -Wno-error -Wno-extra -traditional-cpp -Wno-invalid-pp-token
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv %_m.ps
.SUFFIXES: .abc _m.ps .E .fs .csv .mid .pdf
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
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
%.E: %.gcs Makefile ${MUSICOMP2ABC}
	${CPP} $< -o $@ || (rm -f $@ ; exit 1)
%.v:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --vert $< $@ || (rm -f $@ ; exit 1)
%.h:    %.E ${MUSICOMP2ABC}
	${MUSICOMP2ABC} ${MC_ARGS} --hori $< $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert TESTS.gcs files to .mid files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
	@echo "TESTS: ${TESTS}"
#-----------------------------------------------------------------------------
.PHONY: all tests ${TESTS}
all tests ${TESTS}: ${TESTS_M_PS} ${TESTS_FS} ${TESTS_PDF} ${TESTS_MID} ${TESTS_V} ${TESTS_H}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${TESTS_CSV}
	rm -f ${TESTS_FS}
	rm -f ${TESTS_MID}
	rm -f ${TESTS_ABC}
#+	rm -f ${TESTS_PS}
	rm -f ${TESTS_M_PS}
	rm -f ${TESTS_PDF}
#+	rm -f ${TESTS_WAV}
	rm -f ${TESTS_E}
	rm -f ${TESTS_V}
	rm -f ${TESTS_H}
	rm -f [0-9]*.v [0-9]*.csv nested-[0-9]*.v
#-----------------------------------------------------------------------------
.PHONY: check
check:
	echo "SONGS=${SONGS}"
	echo "SONGS_E=${SONGS_E}"
	echo "SONGS_ABC=${SONGS_ABC}"
	echo "SONGS_FS=${SONGS_FS}"
	echo "SONGS__M_PS=${SONGS__M_PS}"
	echo "TESTS=${TESTS}"
	echo "TESTS_E=${TESTS_E}"
	echo "TESTS_ABC=${TESTS_ABC}"
	echo "TESTS_FS=${TESTS_FS}"
	echo "TESTS_M_PS=${TESTS_M_PS}"
	echo "MUSICOMP2ABC=${MUSICOMP2ABC}"
	echo "MC_ARGS=${MC_ARGS}"
	echo "ABCM2PS=${ABCM2PS}"
	echo "PS2PDF=${PS2PDF}"
	echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
