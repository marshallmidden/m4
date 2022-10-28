#-----------------------------------------------------------------------------
SONGS  =
SONGS = testvoices
#-----------------------------------------------------------------------------
# SONGS_GCS = ${SONGS:=.gcs}
SONGS_E = ${SONGS:=.E}
SONGS_ABC = ${SONGS:=.abc}
SONGS_CSV = ${SONGS:=.csv}
SONGS_MID = ${SONGS:=.mid}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PS = ${SONGS:=.ps}
SONGS_WAV = ${SONGS:=.wav}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS	     = yaps
ABCM2PS	     = abcm2ps
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.csv %.mid %.E
.SUFFIXES: .mid .csv .ps .abc _m.ps .wav .E
# output $@   -- input $<
%.csv:  %.E
	${MUSICOMP2ABC} --midi1csv $< $@ || rm -f $@
%.mid:  %.csv
	csvmidi $< $@ || (rm -f $@ ; exit 1)
%.wav:  %.mid
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 64 $<
%.abc:  %.E
	${MUSICOMP2ABC} --abc $< $@ || (rm -f $@ ; exit 1)
%.ps:   %.abc
	${YAPS} $< -o $@ -k || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@) || (rm -f $@ ; exit 1)
#-- %.E: %.gcs beethoven.macros
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all       - Convert SONGS.gcs files to .mid files, etc.'
	@echo '  clean     - Remove intermediate .csv,.mid,.abc,.ps,_m.ps, and .wav files.'
	@echo '  tags      - Create tags file for python sources.'
	@echo '  tests     - Convert TESTS.gcs files to .mid files, etc.'
	@echo "SONGS: ${SONGS}"
	@echo "TESTS: ${TESTS}"
#-----------------------------------------------------------------------------
.PHONY: all songs
all songs: ${SONGS_MID} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_WAV}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${SONGS_CSV} ${SONGS_MID} ${SONGS_ABC} ${SONGS_PS} ${SONGS_M_PS} ${SONGS_WAV} ${SONGS_E}
	rm -f *.stderr tags ctags
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP2ABC} calculate.py
#-----------------------------------------------------------------------------
