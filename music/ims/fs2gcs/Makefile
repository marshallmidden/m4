#-----------------------------------------------------------------------------
FS = b2m4
#-----------------------------------------------------------------------------
FS2CSV = ./fs2gcs
#-----------------------------------------------------------------------------
IMSCOMP = ../imscomp
MC_ARGS ?=
ABCM2PS      = abcm2ps
PS2PDF       = ps2pdf
CSV2FS  = ../csv2fs/csv2fs
#-----------------------------------------------------------------------------
%.gcs:  %.fs
	${FS2CSV} $< $@
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo "  all       - Convert ${FS}.fs to output file ${FS}.gcs."
	@echo "  clean     - Remove ${FS}.fs."
	@echo "  tags      - Create tags file for python sources for ${FS2CSV}."
	@echo "  testing   - csv2fs, fs2gcs, and imscomp --fs / --csv / --abc"
#-----------------------------------------------------------------------------
.PHONY: all
all: ${FS}.gcs
#-----------------------------------------------------------------------------
${FS}.gcs:  ${FS}.fs ${FS}.hints
	${FS2CSV} ${FS}.fs ${FS}.gcs ${FS}.hints
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${FS}.gcs
	rm -f test.abc                test.pdf test.ps        tmp_test.gcs
	rm -f t.abc             t.fs  t.pdf    t.ps    t.gcs  tmp_t.gcs
	rm -f u.abc    u.csv    u.fs  u.pdf    u.ps    u.gcs  tmp_u.gcs
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${FS2CSV}
#-----------------------------------------------------------------------------
# Test a whole bunch of stuff!
testing: test.pdf t.pdf u.pdf
#-----------------------------------------------------------------------------
tmp_test.gcs: HEADER.gcs test.gcs
	cat HEADER.gcs test.gcs > tmp_test.gcs
test.abc: tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@
test.pdf: test.ps
	${PS2PDF} $< $@
test.ps: test.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
#-----------------------------------------------------------------------------
t.fs: tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --fs $< $@
# ............................................................................
tmp_t.gcs: HEADER.gcs t.gcs
	cat HEADER.gcs t.gcs > tmp_t.gcs
t.abc: tmp_t.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@
t.pdf: t.ps
	${PS2PDF} $< $@
t.ps: t.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
t.gcs:	t.fs
	./fs2gcs t.fs t.gcs
#-----------------------------------------------------------------------------
u.csv:	tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --csv $< $@
# ............................................................................
tmp_u.gcs: HEADER.gcs u.gcs
	cat HEADER.gcs u.gcs > tmp_u.gcs
u.abc: tmp_u.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@
u.fs:	u.csv
	${CSV2FS} u.csv u.fs
u.gcs:	u.fs
	./fs2gcs u.fs u.gcs
u.pdf: u.ps
	${PS2PDF} $< $@
u.ps: u.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
#-----------------------------------------------------------------------------
