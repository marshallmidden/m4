#-----------------------------------------------------------------------------
SONGS  = G21clar
SONGS += G21flugel
SONGS += G22bassn
SONGS += G22clar
SONGS += G22flugel
SONGS += G22flute
SONGS += G22horn
SONGS += G22oboe
SONGS += G22perc
SONGS += G22strng
SONGS += Gershwin_Concerto_move_2.1
SONGS += Gershwin_Concerto_move_2.1_
SONGS += Gershwin_Concerto_move_2.2
SONGS += Gershwin_Concerto_move_2.2_
SONGS += Gershwin_Concerto_move_3
#-----------------------------------------------------------------------------
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_FS = ${SONGS:=.fs}
SONGS_PS = ${SONGS:=.ps}
SONGS_PDF= ${SONGS:=.pdf}
#-----------------------------------------------------------------------------
IMSCOMP = ../../imscomp
#-- MC_ARGS ?= --platochars --lines
MC_ARGS ?= --platochars
ABCM2PS	     = abcm2ps
PS2PDF       = ps2pdf
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
#
CPP=gcc -E -x c -nostdinc -C -CC -Wno-error -Wno-extra -traditional-cpp -Wno-invalid-pp-token
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.E %.csv
.SUFFIXES: .abc .E .fs .csv .mid .pdf
# output $@   -- input $<
%.E: %.gcs
	${CPP} $< -o $@
%.fs:  %.E
	${IMSCOMP} ${MC_ARGS} --two --fluidsynth $< $@
%.csv:  %.E
	${IMSCOMP} ${MC_ARGS} --midi1csv $< $@
%.mid:  %.csv
	csvmidi $< $@
%.abc:  %.E
	${IMSCOMP} ${MC_ARGS} --abc $< $@
%.ps: %.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
%.pdf:	%.ps
	${PS2PDF} $< $@
%.v:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --vert $< $@
%.h:    %.E ${IMSCOMP}
	${IMSCOMP} ${MC_ARGS} --hori $< $@
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  check     - Print out the Makefile Variables.'
	@echo '  clean     - Remove intermediate .csv, .mid, .abc, .ps, and .fs files.'
	@echo '  fs        - Make SONGS as fluidsynth (.fs) output.'
	@echo '  ps        - Make SONGS as postscript (.ps) output.'
	@echo '  pdf       - Make SONGS as postscript (.pdf) output.'
	@echo "songs: ${SONGS}"
#-----------------------------------------------------------------------------
.PHONY: all songs SONGS ${SONGS}
all songs SONGS ${SONGS}: ${SONGS_PS} ${SONGS_FS} ${SONGS_PDF}
#-----------------------------------------------------------------------------
.PHONY: fs
fs: ${SONGS_FS}
.PHONY: ps
ps: ${SONGS_PS}
.PHONY: pdf
pdf: ${SONGS_PDF}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_FS}
	rm -f ${SONGS_ABC}
	rm -f ${SONGS_PS}
	rm -f ${SONGS_PDF}
	rm -f ${SONGS_E}
	rm -f *.stderr
	rm -f *.h *.v *.csv *.mid *.E *.fs *.pdf *.ps *.abc
	rm -f AAA.[0-9][0-9] BBB.[0-9][0-9]
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "SONGS=${SONGS}"
	@echo "SONGS_E=${SONGS_E}"
	@echo "SONGS_ABC=${SONGS_ABC}"
	@echo "SONGS_FS=${SONGS_FS}"
	@echo "SONGS_PS=${SONGS_PS}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "ABCM2PS=${ABCM2PS}"
	@echo "PS2PDF=${PS2PDF}"
	@echo "CPP=${CPP}"
#-----------------------------------------------------------------------------
new-g3.ps: new-g3.abc
# Unique options for new-g3.abc. Grumble.
	(${ABCM2PS} -M -b 0 $< -i -N 2 -j 1 ; mv Out.ps $@)
#-----------------------------------------------------------------------------
