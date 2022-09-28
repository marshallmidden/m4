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
MEASURES = --measures 0,1,2
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
	rm -f *.fs *.abc *.ps *.E *.mid
	rm -f *.stderr
	rm -f Y.* Z.*
#-----------------------------------------------------------------------------
s1.gcs:	b1.csv Makefile
	# -n not in the built-in echo. The following to to not have a "vim" in the Makefile.
	(/bin/echo -n "* v" ; /bin/echo "im: tabstop=8 expandtab shiftwidth=4 softtabstop=4") > s1.gcs
	echo "*-----------------------------------------------------------------------------" >> s1.gcs
	echo "key     c" >> s1.gcs
	echo "meter   4/4" >> s1.gcs
	echo "tempo   88,8" >> s1.gcs
	echo "bars    6+1" >> s1.gcs
	echo "voice   31" >> s1.gcs
	echo "*-----------------------------------------------------------------------------" >> s1.gcs
	echo "putd    =VOICE2=1=" >> s1.gcs
	echo "putd    =VOICE2A=2=" >> s1.gcs
	echo "putd    =VOICE2B=3=" >> s1.gcs
	echo "putd    =VOICE3=4=" >> s1.gcs
	echo "putd    =VOICE3A=5=" >> s1.gcs
	echo "putd    =VOICE3B=6=" >> s1.gcs
	echo "putd    =VOICE4=7=" >> s1.gcs
	echo "putd    =VOICE4A=8=" >> s1.gcs
	echo "putd    =VOICE4B=9=" >> s1.gcs
	echo "putd    =VOICE5=10=" >> s1.gcs
	echo "putd    =VOICE5A=11=" >> s1.gcs
	echo "putd    =VOICE5B=12=" >> s1.gcs
	echo "putd    =VOICE6=13=" >> s1.gcs
	echo "putd    =VOICE7=14=" >> s1.gcs
	echo "putd    =VOICE8=15=" >> s1.gcs
	echo "putd    =VOICE8B=16=" >> s1.gcs
	echo "putd    =VOICE9=17=" >> s1.gcs
	echo "putd    =VOICE10=18=" >> s1.gcs
	echo "putd    =VOICE10A=19=" >> s1.gcs
	echo "putd    =VOICE10B=20=" >> s1.gcs
	echo "putd    =VOICE10C=21=" >> s1.gcs
	echo "putd    =VOICE10D=22=" >> s1.gcs
	echo "putd    =VOICE10E=23=" >> s1.gcs
	echo "putd    =VOICE11=24=" >> s1.gcs
	echo "putd    =VOICE11A=25=" >> s1.gcs
	echo "putd    =VOICE11B=26=" >> s1.gcs
	echo "putd    =VOICE11C=28=" >> s1.gcs
	echo "putd    =VOICE12=29=" >> s1.gcs
	echo "putd    =VOICE13=30=" >> s1.gcs
	echo "putd    =VOICE14=31=" >> s1.gcs
	echo "*-----------------------------------------------------------------------------" >> s1.gcs
	./notes < b1.csv >> s1.gcs 2> Y.stderr
#-----------------------------------------------------------------------------
s1.v:	s1.gcs Makefile
	../musicomp2abc --vert ${MEASURES} s1.gcs s1.v >AAA 2>&1
#-----------------------------------------------------------------------------
