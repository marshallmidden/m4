#-----------------------------------------------------------------------------
SONGS =
SONGS = b-6
#- SONGS += inv15-example
#- SONGS += inv15-example.working
SONGS += loudness.mcomp
SONGS += promenade
SONGS += promenade-musicomp2abc
#-----------------------------------------------------------------------------
SONGS_MID = ${SONGS:=.mid}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PS = ${SONGS:=.ps}
#-----------------------------------------------------------------------------
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS	     = yaps
ABCM2PS	     = abcm2ps
#-----------------------------------------------------------------------------
.SUFFIXES: .mid .csv .ps .abc _m.ps
# output $@   -- input $<
%.mid:	%.csv
	csvmidi $< $@
%.csv:	%
	${MUSICOMP2ABC} --midi1csv $< $@
%.abc:	%
	${MUSICOMP2ABC} --abc $< $@
%.ps:	%.abc
	${YAPS} $< -o $@ -k
%_m.ps:	%.abc
	${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 && mv Out.ps $@
#-----------------------------------------------------------------------------
.PHONY: all
all: ${SONGS_MID} ${SONGS_PS} ${SONGS_M_PS}
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.mid *.ps *.abc *.stderr tags ctags *.csv
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${MUSICOMP2ABC} calculate.py
#-----------------------------------------------------------------------------

#	yaps ${V}.abc -o ${V}.ps -k
#	abcm2ps -l -s 0.42 -M -b 1 ${V}.abc -i -N 2 -j 1
#	open Out.ps

