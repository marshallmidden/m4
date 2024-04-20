MIDICSV = midicsv
CSV2FS  = ../csv2fs
FS2GCS  = ../../fs2gcs/fs2gcs
# ----------------------------------------------------------------------------
MID	 =
#-- MID	+= ScoreMvtIV
MID	+= sym3_4
# ----------------------------------------------------------------------------
MID_CSV = ${MID:=.csv}
MID_FS = ${MID:=.fs}
MID_GCS = ${MID:=.gcs}
MID_HINTS = ${MID_GCS:.gcs=.hints}
# ----------------------------------------------------------------------------
.PRECIOUS: %.csv %.fs %.gcs %.mid
.SUFFIXES: .csv .fs .gcs .mid
# ----------------------------------------------------------------------------
# output $@   -- input $<
%.csv:  %.mid
	${MIDICSV} $< | (unset LANG __CF_USER_TEXT_ENCODING ; sed -f SED-csv) > $@  || true
%.fs:  %.csv
	${CSV2FS} $< $@ || true
%.gcs:  %.fs
	${FS2GCS} $< $@ ${@:.gcs=.hints}
	sed -f SED-b3m4 $@ >A ; mv A $@
# ----------------------------------------------------------------------------
all:	${MID_GCS}
# ----------------------------------------------------------------------------
#-- ${MID_GCS}: ${MID_FS}
#-- ${MID_FS}: ${MID_CSV}
# ----------------------------------------------------------------------------
.phony: test tests
test tests:
	@echo MID_CSV=${MID_CSV}
	@echo MID_FS=${MID_FS}
	@echo MID_GCS=${MID_GCS}
	@echo MID_HINTS=${MID_HINTS}
# ----------------------------------------------------------------------------
.phony: clean
clean:
	rm -f ${MID_CSV}
	rm -f ${MID_FS}
	rm -f ${MID_GCS}
# ----------------------------------------------------------------------------
# End of Makefile
# ----------------------------------------------------------------------------
