.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
#-----------------------------------------------------------------------------
.PHONY: v
v:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m b-6 >b-6.csv
	csvmidi b-6.csv b-6.mid
#	fluidsynth -a sdl2 -i -n -q --reverb=off /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 b-6.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 b-6.mid
#-----------------------------------------------------------------------------
# End of Makefile
