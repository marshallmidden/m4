#-----------------------------------------------------------------------------
TESTS  =
TESTS += M1
#-----------------------------------------------------------------------------
TESTS_E = ${TESTS:=.E}
TESTS_ABC = ${TESTS:=.abc}
TESTS_FS = ${TESTS:=.fs}
TESTS_M_PS = ${TESTS:=_m.ps}
TESTS_CSV = ${TESTS:=.csv}
TESTS_MID = ${TESTS:=.mid}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
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
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -nostdinc -P -C -CC -Wno-error -Wno-extra -traditional-cpp
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv
.SUFFIXES: .abc _m.ps .E .fs .csv
# output $@   -- input $<
%.fs:  %.E
	${MUSICOMP2ABC} --two --fluidsynth $< $@               || (rm -f $@ ; exit 1)
%.mid:  %.csv
	csvmidi $< $@                                          || (rm -f $@ ; exit 1)
%.csv:  %.E
	${MUSICOMP2ABC} --midi1csv $< $@                       || (rm -f $@ ; exit 1)
%.abc:  %.E
	${MUSICOMP2ABC} --abc $< $@                            || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@) || (rm -f $@ Out.ps ; exit 1)
%.E: %.gcs
	${CPP} $< -o $@                                        || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert TESTS '.gcs' files to output format files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps,.fs, and .wav files.'
	@echo "TESTS: ${TESTS}"
#-----------------------------------------------------------------------------
.PHONY: all tests ${TESTS}
all tests ${TESTS}: ${TESTS_M_PS} ${TESTS_FS} ${TESTS_MID}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.fs *.abc *.ps *.E *.csv *.mid
	rm -f *.stderr
#-----------------------------------------------------------------------------
