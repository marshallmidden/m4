V1 = v1-1
V2 = v1-2
V3 = v1-3
V4 = v1-4
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "Use 'musicomp2abc ${V1} to get ${V1}.abc."
	@echo "Use 'yaps ${V1}.abc -k' to get ${V1}.ps."
	@echo "Use 'abcm2ps ${V1}.abc > Out.ps'."
	@echo "Use 'csvmidi ${V1}.csv > ${V1}.mid'."
	@echo
	@echo "  s1     - create ${V1}.abc from ${V1}"
	@echo "  s2     - create ${V2}.abc from ${V2}"
	@echo "  s3     - create ${V3}.abc from ${V3}"
	@echo "  s4     - create ${V4}.abc from ${V4}"
#=============================================================================
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
.PHONY: s1
s1:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V1} >${V1}.abc
	yaps ${V1}.abc -o ${V1}.ps -k
	abcm2ps -l -s 0.42 -M -b 1 ${V1}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V1} >${V1}.csv
	csvmidi ${V1}.csv ${V1}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V1}.mid
#-----------------------------------------------------------------------------
.PHONY: s2
s2:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V2} >${V2}.abc
	yaps ${V2}.abc -o ${V2}.ps -k
	abcm2ps -l -s 0.42 -M -b 1 ${V2}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V2} >${V2}.csv
	csvmidi ${V2}.csv ${V2}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V2}.mid
#-----------------------------------------------------------------------------
.PHONY: s3
s3:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V3} >${V3}.abc
	yaps ${V3}.abc -o ${V3}.ps -k
	abcm2ps -l -s 0.42 -M -b 1 ${V3}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V3} >${V3}.csv
	csvmidi ${V3}.csv ${V3}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V3}.mid
#-----------------------------------------------------------------------------
.PHONY: s4
s4:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V4} >${V4}.abc
	yaps ${V4}.abc -o ${V4}.ps -k
	abcm2ps -l -s 0.42 -M -b 1 ${V4}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V4} >${V4}.csv
	csvmidi ${V4}.csv ${V4}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V4}.mid
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
