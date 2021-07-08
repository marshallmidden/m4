#-----------------------------------------------------------------------------
SONG = b-6
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ./musicomp2abc
#-----------------------------------------------------------------------------
# SONG = promenade
# SONG += tv
#-- SONG_abc = ${SONG:=.abc}
#-- SONG_ps = ${SONG:=.ps}
# .SUFFIXES: .abc 1.mid .ps
#-- %1.mid:	%.abc
#-- 	abc2midi $<
#-- %.ps:	%.abc
#-- 	yaps $< -k
#-- %.abc:	%
#-- 	${MUSICOMP2ABC} $< > $@ || (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: all
all: ${SONG}.mid
${SONG}.mid: ${SONG}.csv
	csvmidi $< $@
${SONG}.csv: ${SONG}
	${MUSICOMP2ABC} --midi1csv $< > $@
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.mid *.ps *.abc *.stderr tags ctags *.csv
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP2ABC} calculate.py
#-----------------------------------------------------------------------------
