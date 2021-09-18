#-----------------------------------------------------------------------------
SONGS =
SONGS = b-6
#- SONGS += inv15-example
#- SONGS += inv15-example.working
SONGS += loudness.mcomp
SONGS += promenade
SONGS += promenade-musicomp2abc

SONGS_MID = ${SONGS:=.mid}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ./musicomp2abc
#-----------------------------------------------------------------------------
.SUFFIXES: .mid .csv
%.csv:	%.mid
%.mid:	%
	${MUSICOMP2ABC} --midi1csv $< $<.csv
	csvmidi $<.csv $@
#-----------------------------------------------------------------------------
.PHONY: all
all: ${SONGS_MID}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.mid *.ps *.abc *.stderr tags ctags *.csv
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP2ABC} calculate.py
#-----------------------------------------------------------------------------
