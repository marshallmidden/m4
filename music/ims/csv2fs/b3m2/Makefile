MIDICSV = midicsv
CSV2FS  = ../csv2fs
FS2GCS  = ../../fs2gcs/fs2gcs
# ----------------------------------------------------------------------------
MID	 =
#-- BAD --	MID	+= ScoreMvtII
#-- Complicated --	MID	+= Symphonie3_Opus55_Mvt2_Heroique
MID	+= beeth3_2
# ----------------------------------------------------------------------------
MID_CSV = ${MID:=.csv}
MID_FS = ${MID:=.fs}
MID_GCS = ${MID:=.gcs}
MID_HINTS = ${MID_GCS:.gcs=.hints}
# ----------------------------------------------------------------------------
all:	${MID_GCS}
# ----------------------------------------------------------------------------
.PRECIOUS: %.csv %.fs %.gcs %.mid
.SUFFIXES: .csv .fs .gcs .mid
# ----------------------------------------------------------------------------
# output $@   -- input $<
%.csv:  %.mid
	${MIDICSV} $< $@ 
%.fs:  %.csv
	${CSV2FS} $< $@
%.gcs:  %.fs
	${FS2GCS} $< $@ ${@:.gcs=.hints}
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
