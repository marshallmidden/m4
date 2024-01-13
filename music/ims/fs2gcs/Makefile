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
.PHONY: help
help:
	@echo 'Targets:'
	@echo "  all       - Convert ${FS}.fs to output file ${FS}.gcs."
	@echo "  clean     - Remove ${FS}.fs."
	@echo "  tags      - Create tags file for python sources for ${FS2CSV}."
	@echo "  testing   - csv2fs, fs2gcs, and imscomp --fs / --csv / --abc"
#-----------------------------------------------------------------------------
.PHONY: all ${FS}
all ${FS}: ${FS}.gcs
#-----------------------------------------------------------------------------
.PHONY: phase2
phase2: phase2.fs phase2.pdf
#-----------------------------------------------------------------------------
phase2.gcs: ${FS}.gcs phase2-header
	cat phase2-header ${FS}.gcs > $@
phase2.fs: phase2.gcs
	${IMSCOMP} ${MC_ARGS} --fs $< $@
phase2.abc: phase2.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || true
phase2.ps: phase2.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps phase2.ps)
phase2.pdf: phase2.ps
	${PS2PDF} $< $@
#-----------------------------------------------------------------------------
${FS}.gcs:  ${FS}.fs ${FS}.hints
	${FS2CSV} ${FS}.fs ${FS}.gcs ${FS}.hints
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${FS}.gcs
	rm -f test.{abc,csv,fs,pdf,ps}
	rm -f t.{abc,csv,fs,pdf,ps,gcs}
	rm -f u.{abc,csv,fs,pdf,ps,gcs}
	rm -f tmp_{test,t,u}.gcs
	rm -f typescript
	rm -f phase2.{fs,csv,abc,ps,pdf,gcs}
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags --language-force=python ${FS2CSV}
#-----------------------------------------------------------------------------
# Test a whole bunch of stuff!
testing: test.pdf  t.pdf u.pdf   t.gcs u.gcs
#-----------------------------------------------------------------------------
tmp_test.gcs: HEADER.gcs test.gcs
	cat HEADER.gcs test.gcs > tmp_test.gcs
test.abc: tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || true
test.ps: test.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
test.pdf: test.ps
	${PS2PDF} $< $@
#-----------------------------------------------------------------------------
tmp_t.gcs: HEADER.gcs t.gcs
	cat HEADER.gcs t.gcs > tmp_t.gcs
t.abc: tmp_t.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || true
t.ps: t.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
t.pdf: t.ps
	${PS2PDF} $< $@
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
t.fs: tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --fs $< $@
t.gcs:	t.fs
	./fs2gcs t.fs t.gcs
#-----------------------------------------------------------------------------
tmp_u.gcs: HEADER.gcs u.gcs
	cat HEADER.gcs u.gcs > tmp_u.gcs
u.abc: tmp_u.gcs
	${IMSCOMP} ${MC_ARGS} --abc $< $@ || true
u.ps: u.abc
	(${ABCM2PS} -l -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@)
u.pdf: u.ps
	${PS2PDF} $< $@
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
u.csv:	tmp_test.gcs
	${IMSCOMP} ${MC_ARGS} --csv $< $@
u.fs:	u.csv
	${CSV2FS} u.csv u.fs
u.gcs:	u.fs
	./fs2gcs u.fs u.gcs
#-----------------------------------------------------------------------------
