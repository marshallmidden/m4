V1 = v1-1
V2 = v1-2
V3 = v1-3
V4 = v1-4
#-----------------------------------------------------------------------------
MCOMP        = ~/src/abc/mcomp/mcomp
SEPARATE     = ./A.separate
MUSICOMP2ABC = ~/musicomp2abc/musicomp2abc
YAPS         = yaps
ABCM2PS      = abcm2ps
#-----------------------------------------------------------------------------
CPP=gcc -E -x c -Wno-invalid-pp-token -undef -Wundef -Werror -nostdinc -P -C -CC
#-----------------------------------------------------------------------------
.PRECIOUS: %.abc %.csv %.mid %.E
.SUFFIXES: .mid .csv .ps .abc _m.ps .wav .E
# output $@   -- input $<
%.fs:	%.E
	${MUSICOMP2ABC} --two --fluidsynth $< $@ \
		|| (rm -f $@ ; exit 1)
%.csv:	%.E
	${MUSICOMP2ABC} --midi1csv $< $@ \
		|| (rm -f $@ ; exit 1)
%.mid:	%.csv
	csvmidi $< $@ \
		|| (rm -f $@ ; exit 1)
%.wav:	%.mid
#--	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser_GS/GeneralUser_GS.sf2 -g 0.5 -K 128 $< \
	fluidsynth -F $@ -a coreaudio -i -n -q /Users/m4/src/GeneralUser/GeneralUser.sf2 -K 128 $< \
		|| (rm -f $@ ; exit 1)
%.abc:	%.E
	${MUSICOMP2ABC} --abc $< $@ \
		|| (rm -f $@ ; exit 1)
%.ps:	%.abc
	${YAPS} $< -o $@ -k \
		|| (rm -f $@ ; exit 1)
%_m.ps:	%.abc
	(${ABCM2PS} -l -s 0.42 -M -b 1 $< -i -N 2 -j 1 ; mv Out.ps $@) \
		|| (rm -f $@ ; exit 1)
#-- %.E: %.gcs beethoven.macros
%.E: %.gcs
	${CPP} $< -o $@ \
		|| (rm -f $@ ; exit 1)
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Example:"
	@echo "  Use 'musicomp2abc ${V1} to get ${V1}.abc."
	@echo "  Use 'yaps ${V1}.abc -k' to get ${V1}.ps."
	@echo "  Use 'abcm2ps ${V1}.abc > ${V1}_m.ps'."
	@echo "  Use 'csvmidi ${V1}.csv > ${V1}.mid'."
	@echo ""
	@echo "Targets:"
	@echo "  all    - Create music files for ${V1}, ${V2}, ${V3}, and ${V4}."
	@echo "  clean  - Remove postscript (ps), csv, and mid files."
	@echo ""
	@echo "  s1     - create ${V1}.abc, ${V1}.ps, ${V1}_m.ps, ${V1}.mid, and ${V1}.wav from ${V1}."
	@echo "  s2     - create ${V2}.abc, ${V2}.ps, ${V2}_m.ps, ${V2}.mid, and ${V2}.wav from ${V2}."
	@echo "  s3     - create ${V3}.abc, ${V3}.ps, ${V3}_m.ps, ${V3}.mid, and ${V3}.wav from ${V3}."
	@echo "  s4     - create ${V4}.abc, ${V4}.ps, ${V4}_m.ps, ${V4}.mid, and ${V4}.wav from ${V4}."
#=============================================================================
.PHONY: all
all: s1 s2 s3 s4
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f *.ps
	rm -f *.stdout *.stderr
	rm -f *.csv *.mid *.fs
	rm -f *.abc
	rm -f *.wav
	rm -f *.E
	rm -f A.[012]* B.[012]*
#-----------------------------------------------------------------------------
.PHONY: s1
# s1: ${V1}.mid ${V1}.ps ${V1}_m.ps ${V1}.wav ${V1}.fs
s1: ${V1}_m.ps ${V1}.fs
#-----------------------------------------------------------------------------
.PHONY: s2
# s2: ${V2}.mid ${V2}.ps ${V2}_m.ps ${V2}.wav ${V2}.fs
s2: ${V2}_m.ps ${V2}.fs
#-----------------------------------------------------------------------------
.PHONY: s3
# s3: ${V3}.mid ${V3}.ps ${V3}_m.ps ${V3}.wav ${V3}.fs
s3: ${V3}_m.ps ${V3}.fs
#-----------------------------------------------------------------------------
.PHONY: s4
# s4: ${V4}.mid ${V4}.ps ${V4}_m.ps ${V4}.wav ${V4}.fs
s4: ${V4}_m.ps ${V4}.fs
#-----------------------------------------------------------------------------
# End of Makefile
#-----------------------------------------------------------------------------
