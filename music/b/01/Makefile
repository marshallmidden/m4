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
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid
	rm -f *.abc
#-----------------------------------------------------------------------------
.PHONY: s
s:
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc ${V} >${V}.abc
	yaps ${V}.abc -o ${V}.ps -k
	~/Desktop/plato-stuff/music/musicomp2abc/musicomp2abc -m ${V} >${V}.csv
	csvmidi ${V}.csv ${V}.mid
	fluidsynth -a sdl2 -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 ${V}.mid
#-----------------------------------------------------------------------------
# End of Makefile
