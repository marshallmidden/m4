V = b-6
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
	abcm2ps -l -s 0.5 -M -b 0 ${V}.abc -i -N 2 -j 1
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V} >${V}.csv
	csvmidi ${V}.csv ${V}.mid
	#-- fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V}.mid
#-----------------------------------------------------------------------------
# End of Makefile
