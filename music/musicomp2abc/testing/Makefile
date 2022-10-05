#-----------------------------------------------------------------------------
# Input is .csv format file.
TESTS  =
# TESTS += b1
TESTS += b1m1
#-----------------------------------------------------------------------------
TESTS_GCS = ${TESTS:=.gcs}
TESTS_FS = ${TESTS:=.fs}
TESTS_v = ${TESTS:=.v}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
MEASURES =
# MEASURES = --measures 0,1,2
#-----------------------------------------------------------------------------
.SUFFIXES: .v .gcs .fs
# output $@   -- input $<
%.fs:  %.v ${MUSICOMP2ABC}
	${MUSICOMP2ABC} --two --fluidsynth $< $@               || (rm -f $@ ; exit 1)
%.v:	%.gcs ${MUSICOMP2ABC}
	${MUSICOMP2ABC} --vert --multiplemeasures ${MEASURES} $< $@
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
	rm -f *.fs *.v *.gcs
	rm -f *.stderr
	rm -f Y.* Z.* Y Z
#-----------------------------------------------------------------------------
.PHONY: ${TESTS}
${TESTS}:	${TESTS}.v
#.............................................................................
${TESTS}.gcs:	${TESTS}.csv Makefile notes ${MUSICOMP2ABC}
	# -n not in the built-in echo. The following to to not have a "vim" in the Makefile.
	(/bin/echo -n "* v" ; /bin/echo "im: tabstop=8 expandtab shiftwidth=4 softtabstop=4") > ${TESTS}.gcs
	echo "*-----------------------------------------------------------------------------" >> ${TESTS}.gcs
	echo "key     c" >> ${TESTS}.gcs
	echo "meter   4/4" >> ${TESTS}.gcs
	echo "tempo   88,8" >> ${TESTS}.gcs
	echo "bars    6+1" >> ${TESTS}.gcs
	echo "voice   30" >> ${TESTS}.gcs
	echo "*-----------------------------------------------------------------------------" >> ${TESTS}.gcs
	echo "putd    =VOICE2A=1=" >> ${TESTS}.gcs
	echo "putd    =VOICE2B=2=" >> ${TESTS}.gcs
	echo "putd    =VOICE3A=3=" >> ${TESTS}.gcs
	echo "putd    =VOICE3B=4=" >> ${TESTS}.gcs
	echo "putd    =VOICE4A=5=" >> ${TESTS}.gcs
	echo "putd    =VOICE4B=6=" >> ${TESTS}.gcs
	echo "putd    =VOICE5A=7=" >> ${TESTS}.gcs
	echo "putd    =VOICE5B=8=" >> ${TESTS}.gcs
	echo "putd    =VOICE6A=9=" >> ${TESTS}.gcs
	echo "putd    =VOICE7A=10=" >> ${TESTS}.gcs
	echo "putd    =VOICE8A=11=" >> ${TESTS}.gcs
	echo "putd    =VOICE8B=12=" >> ${TESTS}.gcs
	echo "putd    =VOICE9A=13=" >> ${TESTS}.gcs
	echo "putd    =VOICE10A=14=" >> ${TESTS}.gcs
	echo "putd    =VOICE10B=15=" >> ${TESTS}.gcs
	echo "putd    =VOICE10C=16=" >> ${TESTS}.gcs
	echo "putd    =VOICE10D=17=" >> ${TESTS}.gcs
	echo "putd    =VOICE10E=18=" >> ${TESTS}.gcs
	echo "putd    =VOICE10F=19=" >> ${TESTS}.gcs
	echo "putd    =VOICE11A=20=" >> ${TESTS}.gcs
	echo "putd    =VOICE11B=21=" >> ${TESTS}.gcs
	echo "putd    =VOICE11C=22=" >> ${TESTS}.gcs
	echo "putd    =VOICE11D=23=" >> ${TESTS}.gcs
	echo "putd    =VOICE12A=24=" >> ${TESTS}.gcs
	echo "putd    =VOICE12B=25=" >> ${TESTS}.gcs
	echo "putd    =VOICE12C=26=" >> ${TESTS}.gcs
	echo "putd    =VOICE13A=27=" >> ${TESTS}.gcs
	echo "putd    =VOICE13B=28=" >> ${TESTS}.gcs
	echo "putd    =VOICE14A=29=" >> ${TESTS}.gcs
	echo "putd    =VOICE14B=30=" >> ${TESTS}.gcs
	echo "*-----------------------------------------------------------------------------" >> ${TESTS}.gcs
	./notes < ${TESTS}.csv >> ${TESTS}.gcs 2> Y.stderr
	echo "*-----------------------------------------------------------------------------" >> ${TESTS}.gcs
	echo "measure TheLast" >> ${TESTS}.gcs
	echo "cstop*" >> ${TESTS}.gcs
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python notes
#-----------------------------------------------------------------------------
a1.gcs:	a.csv Makefile notes
	(/bin/echo -n "* v" ; /bin/echo "im: tabstop=8 expandtab shiftwidth=4 softtabstop=4") > a1.gcs
	echo "*-----------------------------------------------------------------------------" >> a1.gcs
	echo "key     c" >> a1.gcs
	echo "meter   4/4" >> a1.gcs
	echo "tempo   88,8" >> a1.gcs
	echo "bars    1+1" >> a1.gcs
	echo "*-----------------------------------------------------------------------------" >> a1.gcs
	echo "putd    =VOICE2A=1=" >> a1.gcs
	echo "putd    =VOICE2B=2=" >> a1.gcs
	echo "*-----------------------------------------------------------------------------" >> a1.gcs
	./notes < a.csv >> a1.gcs 2> Y.stderr
	echo "*-----------------------------------------------------------------------------" >> a1.gcs
	echo "measure TheLast" >> a1.gcs
	echo "cstop*" >> a1.gcs
#-----------------------------------------------------------------------------
