V = v1-1
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Targets:"
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "Use 'yaps file.abc -k' to get file.ps."
	@echo "Use 'midicsv file.mid >file.csv'."
	@echo "Use 'csvmidi file.csv >file.mid'."
	@echo "Use 'midi2abc file.mid >file.abc'."
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
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V} >${V}.csv
	#-- csvmidi ${V}.csv ${V}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V}.mid
#-----------------------------------------------------------------------------
.PHONY: a1
a1:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.01 >a1.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.01 >a1.abc
	yaps a1.abc -o a1.ps -k
	#-- csvmidi a1.csv a1.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a1.mid
#-----------------------------------------------------------------------------
.PHONY: a2
a2:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.02 >a2.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.02 >a2.abc
	yaps a2.abc -o a2.ps -k
	#-- csvmidi a2.csv a2.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a2.mid
#-----------------------------------------------------------------------------
.PHONY: a3
a3:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.03 >a3.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.03 >a3.abc
	yaps a3.abc -o a3.ps -k
	#-- csvmidi a3.csv a3.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a3.mid
#-----------------------------------------------------------------------------
.PHONY: a4
a4:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.04 >a4.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.04 >a4.abc
	yaps a4.abc -o a4.ps -k
	#-- csvmidi a4.csv a4.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a4.mid
#-----------------------------------------------------------------------------
.PHONY: a5
a5:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.05 >a5.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.05 >a5.abc
	yaps a5.abc -o a5.ps -k
	#-- csvmidi a5.csv a5.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a5.mid
#-----------------------------------------------------------------------------
.PHONY: a6
a6:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.06 >a6.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.06 >a6.abc
	yaps a6.abc -o a6.ps -k
	#-- csvmidi a6.csv a6.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a6.mid
#-----------------------------------------------------------------------------
.PHONY: a7
a7:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.07 >a7.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.07 >a7.abc
	yaps a7.abc -o a7.ps -k
	#-- csvmidi a7.csv a7.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a7.mid
#-----------------------------------------------------------------------------
.PHONY: a8
a8:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.08 >a8.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.08 >a8.abc
	yaps a8.abc -o a8.ps -k
	#-- csvmidi a8.csv a8.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a8.mid
#-----------------------------------------------------------------------------
.PHONY: a9
a9:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.09 >a9.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.09 >a9.abc
	yaps a9.abc -o a9.ps -k
	#-- csvmidi a9.csv a9.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a9.mid
#-----------------------------------------------------------------------------
.PHONY: a10
a10:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m A.10 >a10.csv
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc A.10 >a10.abc
	yaps a10.abc -o a10.ps -k
	#-- csvmidi a10.csv a10.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 a10.mid
#-----------------------------------------------------------------------------
# End of Makefile
