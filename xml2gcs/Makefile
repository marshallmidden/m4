SONGS  =
SONGS += Beethoven_Symphony_4_Movement_III
# ----------------------------------------------------------------------------
SONGS_GCS = ${SONGS:=.gcs}
SONGS_ABC = ${SONGS:=.abc}
SONGS_M_PS = ${SONGS:=_m.ps}
SONGS_PDF= ${SONGS:=.pdf}
# ----------------------------------------------------------------------------
# -b nn = starting bar number
# -i = put red circle around errors.
# -j nn = number every this many bars.
# -l = landscape mode
# -M = suppress lyrics
# -N 2 = number pages in upper right.
# -s nn = page scale to (default 0.75) = %%scale.
ABCM2PS_ARGS = -M -b 1 $< -i -N 2 -j 1
ABCM2PS      = abcm2ps
# ----------------------------------------------------------------------------
PS2PDF       = ps2pdf
# ----------------------------------------------------------------------------
XML2GCS      = ./xml2gcs.py
# ----------------------------------------------------------------------------
IMSCOMP = ~/ims/imscomp
#-- MC_ARGS ?= --nofillwarning --nooncewarning --norestswarning --noleftoverwarnings --notrillwarnings
# ----------------------------------------------------------------------------
.PRECIOUS: %.gcs %.abc %_m.ps %.pdf
%.gcs: %.mxl
	${XML2GCS} $<  > $@ || (rm -f $@ ; exit 1)
%.abc:  %.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || (rm -f $@ ; exit 1)
%_m.ps: %.abc
	(${ABCM2PS} ${ABCM2PS_ARGS} ; mv Out.ps $@) || (rm -f $@ ; exit 1)
%.pdf: %_m.ps
	${PS2PDF} $< $@ || (rm -f $@ ; exit 1)
# ----------------------------------------------------------------------------
.PHONY: all
all: ${SONGS_PDF}
# ----------------------------------------------------------------------------
#-- .PHONY: all_pdf
#-- all_pdf: Beethoven_Symphony_4_Movement_III.pdf
# ----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf ${SONGS_GCS} ${SONGS_ABC} ${SONGS_M_PS} ${SONGS_PDF}
# ----------------------------------------------------------------------------
.PHONY: tags
tags:
	ctags --language-force=python ${XML2GCS}
# ----------------------------------------------------------------------------
# End of Makefile
