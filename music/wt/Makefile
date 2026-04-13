V1 = v11
#-----------------------------------------------------------------------------
IMSCOMP      = ../ims/imscomp
#-----------------------------------------------------------------------------
PAGES =
MEASURES =
VOICES =
AUTO_STAVES ?= yes
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -undef -Wundef -Werror -nostdinc -C -CC -traditional-cpp
MC_ARGS ?= ${PAGES} ${MEASURES} ${VOICES}
#++ MC_ARGS ?= --warn_octave_accidental ${PAGES} ${MEASURES} ${VOICES}
#-- MC_ARGS += --one
#-- MC_ARGS += --two
#-- MC_ARGS += --sf2=/Users/m4/thisisthesoundfont.sf2
#-----------------------------------------------------------------------------
ifeq ($(AUTO_STAVES),yes)
STAVES_ARGS = --auto-staves --staves
STAVES_ARGS += --keep "14,15,16,17;18,19,20"       # Violin1 ; Violin2
else
STAVES_ARGS = --staves
endif
#-----------------------------------------------------------------------------
.PRECIOUS: %.E %.staves.gcs
.SUFFIXES: _2.fs
.SUFFIXES: .staves.gcs
.SUFFIXES: .staves_2.fs
# output $@   -- input $<
%.E:	%.gcs ../instruments.include
	${CPP} $< -o $@
%.staves.gcs:	%.E
	${IMSCOMP} ${MC_ARGS} ${STAVES_ARGS} $< $@
%.staves_2.fs:	%.staves.gcs
	${IMSCOMP} ${MC_ARGS} -fs $< $@
%_2.fs:	%.E
	${IMSCOMP} ${MC_ARGS} -fs $< $@
#-----------------------------------------------------------------------------
.PHONY: help
help::
	@echo "Targets:"
	@echo "  all    - Create music files for ${V1}."
	@echo "  clean  - Remove postscript *.staves file."
	@echo ""
	@echo "Options:"
	@echo "  AUTO_STAVES=yes  - Group voices by instrument with chord notation (default: yes)"
	@echo "  AUTO_STAVES=no   - Use individual voices without grouping"
#-----------------------------------------------------------------------------
.PHONY: check
check:
	@echo "V1=${V1}"
	@echo "IMSCOMP=${IMSCOMP}"
	@echo "MC_ARGS=${MC_ARGS}"
	@echo "AUTO_STAVES=${AUTO_STAVES}"
	@echo "STAVES_ARGS=${STAVES_ARGS}"
#=============================================================================
.PHONY: all
all: staves fs
#-----------------------------------------------------------------------------
.PHONY: staves
staves: ${V1}.staves.gcs
#-----------------------------------------------------------------------------
.PHONY: fs
fs: ${V1}.staves_2.fs ${V1}_2.fs
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f ${V1}.staves.gcs
	rm -f ${V1}.staves_2.fs
	rm -f ${V1}_2.fs
	rm -f ${V1}.E ${V1}.staves.E
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
