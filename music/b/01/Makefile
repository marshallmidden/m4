V = v1-1
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "Use 'musicomp2abc ${V} to get ${V}.abc."
	@echo "Use 'yaps ${V}.abc -k' to get ${V}.ps."
	@echo "Use 'abcm2ps ${V}.abc > Out.ps'."
	@echo "Use 'csvmidi ${V}.csv > ${V}.mid'."
	@echo
	@echo "  s      - create ${V}.abc from ${V}"
#=============================================================================
S1 = 'symphony_1_1_(c)lucarelli.mid'
S2 = 'symphony_1_2_(c)lucarelli.mid'
S3 = 
S4 = 'symphony_1_4_(c)lucarelli.mid'
#=============================================================================
s1-1:
	../../abc/mcomp/mcomp ${S1} vertical  > s1-1
#-----------------------------------------------------------------------------
s1-2:
	../../abc/mcomp/mcomp ${S2} vertical  > s1-2
#-----------------------------------------------------------------------------
s1-3:
	../../abc/mcomp/mcomp ${S3} vertical  > s1-3
#-----------------------------------------------------------------------------
s1-4:
	../../abc/mcomp/mcomp ${S4} vertical  > s1-4
#=============================================================================
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc
	rm -f A.[012]*
#-----------------------------------------------------------------------------
.PHONY: s
s:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V} >${V}.abc
	yaps ${V}.abc -o ${V}.ps -k
	abcm2ps -l -s 0.42 -M -b 1 ${V}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V} >${V}.csv
	csvmidi ${V}.csv ${V}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V}.mid
#-----------------------------------------------------------------------------
.PHONY: a01
a01:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.01 >a01.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.01 >a01.abc
	yaps a01.abc -o a01.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a01.abc -i -N 2 -j 1
	#-- csvmidi a01.csv a01.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a01.mid
#-----------------------------------------------------------------------------
.PHONY: a02
a02:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.02 >a02.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.02 >a02.abc
	yaps a02.abc -o a02.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a02.abc -i -N 2 -j 1
	#-- csvmidi a02.csv a02.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a02.mid
#-----------------------------------------------------------------------------
.PHONY: a03
a03:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.03 >a03.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.03 >a03.abc
	yaps a03.abc -o a03.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a03.abc -i -N 2 -j 1
	#-- csvmidi a03.csv a03.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a03.mid
#-----------------------------------------------------------------------------
.PHONY: a04
a04:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.04 >a04.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.04 >a04.abc
	yaps a04.abc -o a04.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a04.abc -i -N 2 -j 1
	#-- csvmidi a04.csv a04.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a04.mid
#-----------------------------------------------------------------------------
.PHONY: a05
a05:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.05 >a05.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.05 >a05.abc
	yaps a05.abc -o a05.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a05.abc -i -N 2 -j 1
	#-- csvmidi a05.csv a05.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a05.mid
#-----------------------------------------------------------------------------
.PHONY: a06
a06:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.06 >a06.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.06 >a06.abc
	yaps a06.abc -o a06.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a06.abc -i -N 2 -j 1
	#-- csvmidi a06.csv a06.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a06.mid
#-----------------------------------------------------------------------------
.PHONY: a07
a07:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.07 >a07.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.07 >a07.abc
	yaps a07.abc -o a07.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a07.abc -i -N 2 -j 1
	#-- csvmidi a07.csv a07.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a07.mid
#-----------------------------------------------------------------------------
.PHONY: a08
a08:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.08 >a08.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.08 >a08.abc
	yaps a08.abc -o a08.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a08.abc -i -N 2 -j 1
	#-- csvmidi a08.csv a08.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a08.mid
#-----------------------------------------------------------------------------
.PHONY: a09
a09:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.09 >a09.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.09 >a09.abc
	yaps a09.abc -o a09.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a09.abc -i -N 2 -j 1
	#-- csvmidi a09.csv a09.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a09.mid
#-----------------------------------------------------------------------------
.PHONY: a10
a10:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.10 >a10.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.10 >a10.abc
	yaps a10.abc -o a10.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a10.abc -i -N 2 -j 1
	#-- csvmidi a10.csv a10.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a10.mid
#-----------------------------------------------------------------------------
.PHONY: a11
a11:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.11 >a11.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.11 >a11.abc
	yaps a11.abc -o a11.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a11.abc -i -N 2 -j 1
	#-- csvmidi a11.csv a11.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a11.mid
#-----------------------------------------------------------------------------
.PHONY: a12
a12:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.12 >a12.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.12 >a12.abc
	yaps a12.abc -o a12.ps -k
	abcm2ps -l -s 0.8 -M -b 1 a12.abc -i -N 2 -j 1
	#-- csvmidi a12.csv a12.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a12.mid
#-----------------------------------------------------------------------------
# End of Makefile
