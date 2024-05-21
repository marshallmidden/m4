# ----------------------------------------------------------------------------
MIDICSV = midicsv
CSV2FS  = ../csv2fs
FS2GCS  = ../../fs2gcs/fs2gcs
# ----------------------------------------------------------------------------
MID	 =
# UGH	MID	+= b4m1.mid
MID	+= sym4_1.mid
# UGH	MID	+= symphony_4_1_cvikl.mid
# ----------------------------------------------------------------------------
MID_HINTS = This.hints
# ----------------------------------------------------------------------------
MID_CSV = ${MID:.mid=.csv}
MID_FS = ${MID:.mid=.fs}
MID_GCS = ${MID:.mid=.gcs}
# ----------------------------------------------------------------------------
.PRECIOUS: %.csv %.fs %.gcs
.SUFFIXES: .csv .fs .gcs .mid
# ----------------------------------------------------------------------------
# output $@   -- input $<
%.csv:  %.mid
	${MIDICSV} "$<" | (unset LANG __CF_USER_TEXT_ENCODING ; sed -f SED-csv) > "$@" || true
%.fs:  %.csv
	${CSV2FS} "$<" "$@" || true
%.gcs:  %.fs
	${FS2GCS} "$<" "$@" "${MID_HINTS}" || true
	sed -f SED-b4m1 "$@" >A ; mv A "$@"
# ----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo "  all       - Convert ${MID} to gcs input files:"
	@echo "                       ${MID_GCS}"
	@echo "  clean     - Remove files:"
	@echo "                ${MID_CSV}"
	@echo "                ${MID_FS}"
	@echo "                ${MID_GCS}"
	@echo '  tests     - See what various variables values are.'
#-----------------------------------------------------------------------------
all:	${MID_GCS}
# ----------------------------------------------------------------------------
.phony: test tests
test tests:
	@echo "MIDICSV=${MIDICSV}"
	@echo "CSV2FS=${CSV2FS}"
	@echo "FS2GCS=${FS2GCS}"
	@echo "MID=${MID}"
	@echo "MID_CSV=${MID_CSV}"
	@echo "MID_FS=${MID_FS}"
	@echo "MID_GCS=${MID_GCS}"
	@echo "MID_HINTS=${MID_HINTS}"
# ----------------------------------------------------------------------------
.phony: clean
clean:
	rm -f ${MID_CSV}
	rm -f ${MID_FS}
	rm -f ${MID_GCS}
# ----------------------------------------------------------------------------
# End of Makefile
# ----------------------------------------------------------------------------
