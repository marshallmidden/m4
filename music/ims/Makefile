#-----------------------------------------------------------------------------
SONGS  = A-Flat-Prelude-Chopin
#-----------------------------------------------------------------------------
TESTS  = A0
TESTS += A1
TESTS += A2
TESTS += A4
TESTS += A5
TESTS += A6
TESTS += A7
TESTS += B.err
TESTS += B3
TESTS += B4
TESTS += B5
TESTS += B6
TESTS += DECODE
TESTS += ENCODE
TESTS += L
TESTS += L1
TESTS += N
TESTS += P1
TESTS += P2
TESTS += P3
TESTS += P4
TESTS += P5
TESTS += P6
TESTS += P7
TESTS += P8
TESTS += P9
TESTS += S1
TESTS += SIMPLE
TESTS += STAFF
TESTS += T1
TESTS += T2
TESTS += T3
TESTS += T4
TESTS += U1
TESTS += U2
TESTS += U3
TESTS += U4
TESTS += V1
TESTS += V2
TESTS += W1
TESTS += W2
TESTS += W3
TESTS += W3.v.no.10
TESTS += W4
TESTS += W5
TESTS += W6
TESTS += W6.p.no.10
TESTS += W7
TESTS += W8
TESTS += W8.i.no.10
TESTS += W9
TESTS += debugstaffvol
TESTS += s14
TESTS += sequence
TESTS += test-cresc-chord
TESTS += test-cresc-staff
TESTS += test-cresc
TESTS += trills
TESTS += vc1-test
TESTS += vc2-test
TESTS += vc3-test
TESTS += vc4-test
TESTS += vars-test
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_M_ABC = ${SONGS:=.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_M_FS = ${SONGS:=.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PDF= ${SONGS:=.pdf}
SONGS_M_PDF= ${SONGS:=.pdf}
#-----------------------------------------------------------------------------
TESTS_E = ${TESTS:=.E}
TESTS_ABC = ${TESTS:=.abc}
TESTS_M_ABC = ${TESTS:=_m.abc}
TESTS_FS = ${TESTS:=.fs}
TESTS_M_FS = ${TESTS:=_m.fs}
TESTS_PS = ${TESTS:=.ps}
TESTS_M__PS = ${TESTS:=_m.ps}
TESTS_PDF= ${TESTS:=.pdf}
TESTS_M_PDF= ${TESTS:=_m.pdf}
#-----------------------------------------------------------------------------
IMSCOMP = ./imscomp
MUSICOMP = ~/musicomp2abc/musicomp2abc
MC_ARGS ?=
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
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@
%_m.fs:  %.E
	${MUSICOMP} ${MC_ARGS} --two --fluidsynth $< $@
%.csv:  %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@
%_m.csv:  %.E
	${MUSICOMP} ${MC_ARGS} --midi1csv $< $@
%.mid:  %.csv
	csvmidi $< $@
%_m.mid:  %_m.csv
	csvmidi $< $@
%.abc:  %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@
%_m.abc:  %.E
	${MUSICOMP} ${MC_ARGS} --abc $< $@
%.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%_m.ps: %_m.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%.ps
	${PS2PDF} $< $@
%_m.pdf:	%_m.ps
	${PS2PDF} $< $@
%.E: %.gcs
	${CPP} $< -o $@
%.v:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%_m.v:    %.E ${MUSICOMP}
	${MUSICOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@
%_m.h:    %.E ${MUSICOMP}
	${MUSICOMP} ${MC_ARGS} --hori $< $@
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
all songs ${SONGS}: ${SONGS_PS} ${SONGS_M_PS} ${SONGS_FS} ${SONGS_M_FS} ${SONGS_PDF} ${SONGS_M_PDF}
#-----------------------------------------------------------------------------
.PHONY: tests ${TESTS}
tests ${TESTS}: ${TESTS_PS} ${TESTS_FS} ${TESTS_PDF}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS} ${TESTS_FS} ${SONGS_M_FS} ${TESTS_M_FS}
	rm -f ${SONGS_ABC} ${TESTS_ABC} ${SONGS_M_ABC} ${TESTS_M_ABC}
	rm -f ${SONGS_PS} ${TESTS_PS} ${SONGS_M_PS} ${TESTS_M_PS}
	rm -f ${SONGS_PDF} ${TESTS_PDF} ${SONGS_M_PDF} ${TESTS_M_PDF}
	rm -f ${SONGS_E} ${TESTS_E}
	rm -f *.stderr
	rm -f *.h *.v *.csv *.mid *.E *.fs *.pdf *.ps *.abc
	rm -f Y.0*
#-----------------------------------------------------------------------------
.PHONY: check
check:
	echo "SONGS=${SONGS}"
	echo "SONGS_E=${SONGS_E}"
	echo "SONGS_ABC=${SONGS_ABC}"
	echo "SONGS_FS=${SONGS_FS}"
	echo "SONGS_PS=${SONGS_PS}"
	echo "SONGS_M_ABC=${SONGS_M_ABC}"
	echo "SONGS_M_FS=${SONGS_M_FS}"
	echo "SONGS_M_PS=${SONGS_M_PS}"
	echo "TESTS=${TESTS}"
	echo "TESTS_E=${TESTS_E}"
	echo "TESTS_ABC=${TESTS_ABC}"
	echo "TESTS_FS=${TESTS_FS}"
	echo "TESTS_PS=${TESTS_PS}"
	echo "TESTS_M_ABC=${TESTS_M_ABC}"
	echo "TESTS_M_FS=${TESTS_M_FS}"
	echo "TESTS_M_PS=${TESTS_M_PS}"
	echo "IMSCOMP=${IMSCOMP}"
	echo "MUSICOMP=${MUSICOMP}"
	echo "MC_ARGS=${MC_ARGS}"
	echo "ABCM2PS=${ABCM2PS}"
	echo "PS2PDF=${PS2PDF}"
	echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${IMSCOMP} calculate.py
#-----------------------------------------------------------------------------
