#-----------------------------------------------------------------------------
# Input is .csv format file.
TESTS  =
# TESTS += b1
# TESTS += b1m1
# TESTS += 1812
TESTS += e
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
	# -n not in the built-in echo. The following to not have a "vim" in the Makefile.
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
.PHONY: b1m1
b1m1:	b1m1.v
#.............................................................................
# Beethoven's 1st, movement 1 - midi input. (To get transpositions correct for instruments.)
b1m1.gcs:	b1m1.csv Makefile notes ${MUSICOMP2ABC}
	# -n not in the built-in echo. The following to not have a "vim" in the Makefile.
	(/bin/echo -n "* v" ; /bin/echo "im: tabstop=8 expandtab shiftwidth=4 softtabstop=4") > b1m1.gcs
	echo "*-----------------------------------------------------------------------------" >> b1m1.gcs
	echo "key     c" >> b1m1.gcs
	echo "meter   4/4" >> b1m1.gcs
	echo "tempo   88,8" >> b1m1.gcs
	echo "bars    6+1" >> b1m1.gcs
	echo "voice   30" >> b1m1.gcs
	echo "*-----------------------------------------------------------------------------" >> b1m1.gcs
	echo "putd    =VOICE2A=1=" >> b1m1.gcs
	echo "putd    =VOICE2B=2=" >> b1m1.gcs
	echo "putd    =VOICE3A=3=" >> b1m1.gcs
	echo "putd    =VOICE3B=4=" >> b1m1.gcs
	echo "putd    =VOICE4A=5=" >> b1m1.gcs
	echo "putd    =VOICE4B=6=" >> b1m1.gcs
	echo "putd    =VOICE5A=7=" >> b1m1.gcs
	echo "putd    =VOICE5B=8=" >> b1m1.gcs
	echo "putd    =VOICE6A=9=" >> b1m1.gcs
	echo "putd    =VOICE7A=10=" >> b1m1.gcs
	echo "putd    =VOICE8A=11=" >> b1m1.gcs
	echo "putd    =VOICE8B=12=" >> b1m1.gcs
	echo "putd    =VOICE9A=13=" >> b1m1.gcs
	echo "putd    =VOICE10A=14=" >> b1m1.gcs
	echo "putd    =VOICE10B=15=" >> b1m1.gcs
	echo "putd    =VOICE10C=16=" >> b1m1.gcs
	echo "putd    =VOICE10D=17=" >> b1m1.gcs
	echo "putd    =VOICE10E=18=" >> b1m1.gcs
	echo "putd    =VOICE10F=19=" >> b1m1.gcs
	echo "putd    =VOICE11A=20=" >> b1m1.gcs
	echo "putd    =VOICE11B=21=" >> b1m1.gcs
	echo "putd    =VOICE11C=22=" >> b1m1.gcs
	echo "putd    =VOICE11D=23=" >> b1m1.gcs
	echo "putd    =VOICE12A=24=" >> b1m1.gcs
	echo "putd    =VOICE12B=25=" >> b1m1.gcs
	echo "putd    =VOICE12C=26=" >> b1m1.gcs
	echo "putd    =VOICE13A=27=" >> b1m1.gcs
	echo "putd    =VOICE13B=28=" >> b1m1.gcs
	echo "putd    =VOICE14A=29=" >> b1m1.gcs
	echo "putd    =VOICE14B=30=" >> b1m1.gcs
	echo "*-----------------------------------------------------------------------------" >> b1m1.gcs
	./notes < b1m1.csv >> b1m1.gcs 2> Y.stderr
	echo "*-----------------------------------------------------------------------------" >> b1m1.gcs
	echo "measure TheLast" >> b1m1.gcs
	echo "cstop*" >> b1m1.gcs
#-----------------------------------------------------------------------------
.PHONY: 1812
1812:	1812.v
#.............................................................................
1812.gcs:	1812.csv Makefile notes ${MUSICOMP2ABC}
	# -n not in the built-in echo. The following to not have a "vim" in the Makefile.
	(/bin/echo -n "* v" ; /bin/echo "im: tabstop=8 expandtab shiftwidth=4 softtabstop=4") > 1812.gcs
	echo "*-----------------------------------------------------------------------------" >> 1812.gcs
	echo "key     c" >> 1812.gcs		# Many keys, going to be interesting!
	echo "meter   3/4" >> 1812.gcs
	echo "tempo   60,4" >> 1812.gcs
	echo "bars    7+1" >> 1812.gcs
	echo "voice   76" >> 1812.gcs
	echo "volumes 1..76 mf" >> 1812.gcs
	echo "page    3" >> 1812.gcs
	echo "*-----------------------------------------------------------------------------" >> 1812.gcs
	echo "putd    =VOICE1A=1=" >> 1812.gcs
	echo "putd    =VOICE1B=2=" >> 1812.gcs
	echo "putd    =VOICE1C=3=" >> 1812.gcs
	echo "putd    =VOICE1D=4=" >> 1812.gcs
	echo "putd    =VOICE1E=5=" >> 1812.gcs
	echo "putd    =VOICE1F=6=" >> 1812.gcs
	echo "putd    =VOICE2A=7=" >> 1812.gcs
	echo "putd    =VOICE2B=8=" >> 1812.gcs
	echo "putd    =VOICE2C=9=" >> 1812.gcs
	echo "putd    =VOICE2D=10=" >> 1812.gcs
	echo "putd    =VOICE2E=11=" >> 1812.gcs
	echo "putd    =VOICE2F=12=" >> 1812.gcs
	echo "putd    =VOICE3A=13=" >> 1812.gcs
	echo "putd    =VOICE4A=14=" >> 1812.gcs
	echo "putd    =VOICE5A=15=" >> 1812.gcs
	echo "putd    =VOICE6A=16=" >> 1812.gcs
	echo "putd    =VOICE6B=17=" >> 1812.gcs
	echo "putd    =VOICE7A=18=" >> 1812.gcs
	echo "putd    =VOICE7B=19=" >> 1812.gcs
	echo "putd    =VOICE8A=20=" >> 1812.gcs
	echo "putd    =VOICE9A=21=" >> 1812.gcs
	echo "putd    =VOICE9B=22=" >> 1812.gcs
	echo "putd    =VOICE9C=23=" >> 1812.gcs
	echo "putd    =VOICE10A=24=" >> 1812.gcs
	echo "putd    =VOICE10B=25=" >> 1812.gcs
	echo "putd    =VOICE11A=26=" >> 1812.gcs
	echo "putd    =VOICE11B=27=" >> 1812.gcs
	echo "putd    =VOICE12A=28=" >> 1812.gcs
	echo "putd    =VOICE12B=29=" >> 1812.gcs
	echo "putd    =VOICE13A=30=" >> 1812.gcs
	echo "putd    =VOICE13B=31=" >> 1812.gcs
	echo "putd    =VOICE14A=32=" >> 1812.gcs
	echo "putd    =VOICE14B=33=" >> 1812.gcs
	echo "putd    =VOICE15A=34=" >> 1812.gcs
	echo "putd    =VOICE15B=35=" >> 1812.gcs
	echo "putd    =VOICE16A=36=" >> 1812.gcs
	echo "putd    =VOICE16B=37=" >> 1812.gcs
	echo "putd    =VOICE16C=38=" >> 1812.gcs
	echo "putd    =VOICE16D=39=" >> 1812.gcs
	echo "putd    =VOICE16E=40=" >> 1812.gcs
	echo "putd    =VOICE16F=41=" >> 1812.gcs
	echo "putd    =VOICE16G=42=" >> 1812.gcs
	echo "putd    =VOICE16H=43=" >> 1812.gcs
	echo "putd    =VOICE16I=44=" >> 1812.gcs
	echo "putd    =VOICE16J=45=" >> 1812.gcs
	echo "putd    =VOICE16K=46=" >> 1812.gcs
	echo "putd    =VOICE16L=47=" >> 1812.gcs
	echo "putd    =VOICE16M=48=" >> 1812.gcs
	echo "putd    =VOICE17A=49=" >> 1812.gcs
	echo "putd    =VOICE18A=50=" >> 1812.gcs
	echo "putd    =VOICE19A=51=" >> 1812.gcs
	echo "putd    =VOICE19B=52=" >> 1812.gcs
	echo "putd    =VOICE20A=53=" >> 1812.gcs
	echo "putd    =VOICE21A=54=" >> 1812.gcs
	echo "putd    =VOICE23A=55=" >> 1812.gcs
	echo "putd    =VOICE23B=56=" >> 1812.gcs
	echo "putd    =VOICE23C=57=" >> 1812.gcs
	echo "putd    =VOICE23D=58=" >> 1812.gcs
	echo "putd    =VOICE24A=59=" >> 1812.gcs
	echo "putd    =VOICE24B=60=" >> 1812.gcs
	echo "putd    =VOICE24C=61=" >> 1812.gcs
	echo "putd    =VOICE24D=62=" >> 1812.gcs
	echo "putd    =VOICE25A=63=" >> 1812.gcs
	echo "putd    =VOICE25B=64=" >> 1812.gcs
	echo "putd    =VOICE25C=65=" >> 1812.gcs
	echo "putd    =VOICE25D=66=" >> 1812.gcs
	echo "putd    =VOICE26A=67=" >> 1812.gcs
	echo "putd    =VOICE26B=68=" >> 1812.gcs
	echo "putd    =VOICE26C=69=" >> 1812.gcs
	echo "putd    =VOICE26D=70=" >> 1812.gcs
	echo "putd    =VOICE27A=71=" >> 1812.gcs
	echo "putd    =VOICE27B=72=" >> 1812.gcs
	echo "putd    =VOICE27C=73=" >> 1812.gcs
	echo "putd    =VOICE27D=74=" >> 1812.gcs
	echo "putd    =VOICE28A=75=" >> 1812.gcs
	echo "putd    =VOICE28B=76=" >> 1812.gcs
	./notes < 1812.csv >> 1812.gcs 2> Y.stderr
	echo "*-----------------------------------------------------------------------------" >> 1812.gcs
	echo "measure TheLast" >> 1812.gcs
	echo "cstop*" >> 1812.gcs
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
