#-----------------------------------------------------------------------------
# Things to change:
#     MODEL   = 3000 or 7000
#     PD      = Perf or Debug
#     SRCS    = The base directory where the sources for this release.
#     MASTER  = Where the .GDBINIT.xxx files are located. How many directories
#               away are you? I.e. is it one level up, two, or in Neverland.
#     APPS    = Should be correct if SRCS is correct. The unpacking of cores...
#     CORES   = Where the core files are located -- sometimes in subdirectories.
#     RELEASE = Not really important, some "new" gdb macros need this to be
#               able to handle really old executables -- typically the .gdbinit
#               files are gotten from the SRCS (release) directory.
#-----------------------------------------------------------------------------
# MODEL:=3000
MODEL:=7000
PD:=Perf
# PD:=Debug
# RELEASE:=830
# RELEASE:=840
# RELEASE:=850
# RELEASE:=860
RELEASE:=922
#
# SRCS := /home/Builds/storage/Wookiee/ByVersion/S013/Input/eng/storage/Wookiee/
# SRCS := /home/m4/b/
# SRCS := /home/m4/tmp/development_wookiee/
# SRCS := /home/m4/tmp/HEAD/
# U045 is 0922
SRCS := /home/Builds/storage/Wookiee/ByVersion/U045/Input/eng/storage/Wookiee/
# U042 is 0921
# SRCS := /home/Builds/storage/Wookiee/ByVersion/U042/Input/eng/storage/Wookiee/
# U006 is 0901
# SRCS := /home/Builds/storage/Wookiee/ByVersion/U006/Input/eng/storage/Wookiee/
# U004 is 0900
# T011 is 0881
# SRCS := /home/Builds/storage/Wookiee/ByVersion/R011/Input/eng/storage/Wookiee/
# T007 is 0880
# SRCS := /home/Builds/storage/Wookiee/ByVersion/T007/Input/eng/storage/Wookiee/
# S059 is 0877
# S058 is 0876
# S056 is 0875
# SRCS := /home/Builds/storage/Wookiee/ByVersion/S056/Input/eng/storage/Wookiee/
# S055 is 0874
# SRCS := /home/Builds/storage/Wookiee/ByVersion/S055/Input/eng/storage/Wookiee/
# S054 is 0873
# SRCS := /home/Builds/storage/Wookiee/ByVersion/S054/Input/eng/storage/Wookiee/
# S053 is 0872
# S049 is 0871
# S044 is 0870
# R042 is 0861
# SRCS := /home/Builds/storage/Wookiee/ByVersion/R042/Input/eng/storage/Wookiee/
# R034 is 0860
# SRCS := /home/Builds/storage/Wookiee/ByVersion/R034/Input/eng/storage/Wookiee/
# Q039 is 0851
# SRCS := /home/Builds/storage/Wookiee/ByVersion/Q039/Input/eng/storage/Wookiee/
# P037 is 0830
# SRCS := /home/Builds/storage/Wookiee/ByVersion/P037/Input/eng/storage/Wookiee/
# L126 is 0740
# SRCS := /home/Builds/storage/Wookiee/ByVersion/L126/Input/eng/storage/Wookiee/
# SRCS := /home/Builds/storage/Wookiee/ByRelease/Platform_8.3.0/Input/eng/storage/Wookiee/
# SRCS := /home/eng_build/Nite/Wookiee/
#----
CORES := ./
APPS := ${SRCS}/built-${MODEL}${PD}/
#-----------------------------------------------------------------------------
# The MASTER variable below points to where the "good" files .GDBINIT.BE,
# .GDBINIT.CCB, .GDBINIT.FE are located.
MASTER := ../../
#----
# Script output file to fix/change labels.
TS := typescript
LENGTH ?= 100
#----
RUNENV = runenv sles9-32-Wookiee
#-----------------------------------------------------------------------------
.PHONY:	all help
all help:
	@echo ' '
	@echo See Makefile.
	@echo ' '
	@echo "  MODEL=${MODEL} PD=${PD} SRCS=${SRCS}"
	@echo "  MASTER=${MASTER} CORES=${CORES} APPS=${APPS}"
	@echo ' '
	@echo '  make be         - to look at Back.t.core'
	@echo '  make fe         - to look at Front.t.core'
	@echo '  make ccb        - to look at ccbrun.core'
	@echo '  make A.be       - to do tracelog $${LENGTH} on Back.t.core'
	@echo '  make A.fe       - to do tracelog $${LENGTH} on Front.t.core'
	@echo '  make A.ccb      - to do tracelog $${LENGTH} on ccbrun.core'
	@echo '  make A.beall    - to do tracelog 800000 on Back.t.core'
	@echo '  make A.feall    - to do tracelog 800000 on Front.t.core'
	@echo '  make A.ccball   - to do tracelog 800000 on ccbrun.core'
	@echo '  make FIND.ilts  - to find missing ILTs'
	@echo '  make verifybe   - verify vdisk/raid/psd tables in the backend.'
	@echo '  make clean      - delete existing cores, apps.xiolog, etc.'
	@echo '  make extradelete - delete *.txt, *.hist, ql* files.'
	@echo '  make unmake     - undo make be/fe/ccb (and/or) to try again.'
	@echo '  make procsrc    - cd ${SRCS}/Proc/src'
	@echo '  make procinc    - cd ${SRCS}/Proc/src'
	@echo '  make ccbsrc     - cd ${SRCS}/CCB/Src'
	@echo '  make ccbinc     - cd ${SRCS}/CCB/Inc'
	@echo '  make sharedsrc  - cd ${SRCS}/Shared/Src'
	@echo '  make sharedinc  - cd ${SRCS}/Shared/Inc'
	@echo ' '
#=============================================================================
# Include local (this directory) additions.
sinclude .SPECIAL-Makefile
#=============================================================================
.PHONY:	get
get:
	./GET
#=============================================================================
.PHONY:	procsrc ct
procsrc ct:
	cd ${SRCS}/Proc/src ; bash -i
.PHONY:	procinc
procinc:
	cd ${SRCS}/Proc/inc ; bash -i
.PHONY:	ccbsrc
ccbsrc:
	cd ${SRCS}/CCB/Src ; bash -i
.PHONY:	ccbinc
ccbinc:
	cd ${SRCS}/CCB/Inc ; bash -i
.PHONY:	sharedsrc shared
sharedsrc shared:
	cd ${SRCS}/Shared/Src ; bash -i
.PHONY:	sharedinc
sharedinc:
	cd ${SRCS}/Shared/Inc ; bash -i
#=============================================================================
${CORES}/Back.t.core: ${CORES}/be.shm ${CORES}/ccb.shm ${CORES}/fe.shm Back.t cvtcore_${MODEL} subst-elf
	-if [ -e ${CORES}/Back.t.core ]; then exit 0; fi; \
	 if [ -e ${CORES}/Back.t.core.gz ]; then gunzip ${CORES}/Back.t.core.gz; fi; \
	 if [ -e ${CORES}/Back.t.core.bz2 ]; then bunzip2 ${CORES}/Back.t.core.bz2; fi; \
	 if [ \! -e ${CORES}/Back.t.core ]; then exit 1; fi; \
	 if [ -e ${CORES}/be.shm ]; then cd ${CORES}; ${RUNENV} ./cvtcore_${MODEL} Back.t.core; mv Back.t.core Back.t.core.orig; mv Back.t.core.merged Back.t.core; fi
#-----------------------------------------------------------------------------
.PHONY:	${CORES}/be.shm
${CORES}/be.shm:
	-if [ -e ${CORES}/be.shm.gz ]; then gunzip ${CORES}/be.shm.gz; fi
#-----------------------------------------------------------------------------
.PHONY:	${CORES}/fe.shm
${CORES}/fe.shm:
	-if [ -e ${CORES}/fe.shm.gz ]; then gunzip ${CORES}/fe.shm.gz; fi
#-----------------------------------------------------------------------------
.PHONY:	${CORES}/ccb.shm
${CORES}/ccb.shm:
	-if [ -e ${CORES}/ccb.shm.gz ]; then gunzip ${CORES}/ccb.shm.gz; fi
#-----------------------------------------------------------------------------
.PHONY:	be
be:	.gdbinit .GDBINIT.BE Back.t ${CORES}/Back.t.core
	${RUNENV} gdb -nx -x .GDBINIT.BE Back.t
#=============================================================================
# Get executables local -- in case they change as you are debugging.
Back.t:
	cp --preserve=timestamps ${APPS}/Back.t Back.t
#-----------------------------------------------------------------------------
Front.t:
	cp --preserve=timestamps ${APPS}/Front.t Front.t
#-----------------------------------------------------------------------------
ccbrun:
	cp --preserve=timestamps ${APPS}/ccbrun ccbrun
#-----------------------------------------------------------------------------
cvtcore_${MODEL}:
	cp --preserve=timestamps ${APPS}/cvtcore_${MODEL} cvtcore_${MODEL}
#-----------------------------------------------------------------------------
subst-elf:
	cp --preserve=timestamps ${APPS}/subst-elf subst-elf
#=============================================================================
${CORES}/Front.t.core: ${CORES}/be.shm ${CORES}/ccb.shm ${CORES}/fe.shm Front.t cvtcore_${MODEL} subst-elf
	-if [ -e ${CORES}/Front.t.core ]; then exit 0; fi; \
	 if [ -e ${CORES}/Front.t.core.gz ]; then gunzip ${CORES}/Front.t.core.gz; fi; \
	 if [ -e ${CORES}/Front.t.core.bz2 ]; then bunzip2 ${CORES}/Front.t.core.bz2; fi; \
	 if [ \! -e ${CORES}/Front.t.core ]; then exit 1; fi; \
	 if [ -e ${CORES}/be.shm ]; then cd ${CORES}; ${RUNENV} ./cvtcore_${MODEL} Front.t.core; mv Front.t.core Front.t.core.orig; mv Front.t.core.merged Front.t.core; fi
#-----------------------------------------------------------------------------
.PHONY:	fe
fe:	.gdbinit .GDBINIT.FE Front.t ${CORES}/Front.t.core
	${RUNENV} gdb -nx -x .GDBINIT.FE Front.t
#=============================================================================
${CORES}/ccbrun.core: ${CORES}/be.shm ${CORES}/ccb.shm ${CORES}/fe.shm ccbrun cvtcore_${MODEL} subst-elf
	-if [ -e ${CORES}/ccbrun.core ]; then exit 0; fi; \
	 if [ -e ${CORES}/ccbrun.core.gz ]; then gunzip ${CORES}/ccbrun.core.gz; fi; \
	 if [ -e ${CORES}/ccbrun.core.bz2 ]; then bunzip2 ${CORES}/ccbrun.core.bz2; fi; \
	 if [ \! -e ${CORES}/ccbrun.core ]; then exit 1; fi; \
	 if [ -e ${CORES}/be.shm ]; then cd ${CORES}; ${RUNENV} ./cvtcore_${MODEL} ccbrun.core; mv ccbrun.core ccbrun.core.orig; mv ccbrun.core.merged ccbrun.core; fi
#-----------------------------------------------------------------------------
.PHONY:	ccb
ccb:	.gdbinit.ccb .GDBINIT.CCB ccbrun ${CORES}/ccbrun.core
	gdb -nx -x .GDBINIT.CCB ccbrun
#=============================================================================
.GDBINIT.CCB: .gdbinit.ccb
	sed -e "s;SRCS;${SRCS};" -e "s;MODEL;${MODEL};" -e "s;DEBUGPERF;${PD};" ${MASTER}/.GDBINIT.CCB > ./.GDBINIT.CCB
.GDBINIT.FE: .gdbinit
	sed -e "s;SRCS;${SRCS};" -e "s;MODEL;${MODEL};" -e "s;DEBUGPERF;${PD};" ${MASTER}/.GDBINIT.FE > ./.GDBINIT.FE
.GDBINIT.BE: .gdbinit
	sed -e "s;SRCS;${SRCS};" -e "s;MODEL;${MODEL};" -e "s;DEBUGPERF;${PD};" ${MASTER}/.GDBINIT.BE > ./.GDBINIT.BE
.gdbinit:
	ln -s ${SRCS}/Proc/src/.gdbinit .gdbinit
.gdbinit.ccb:
	ln -s ${SRCS}/CCB/Src/.gdbinit .gdbinit.ccb
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	cp
cp:
#	-rm -f /var/log/dump/*.bz2
	-bzip2 -9 /var/log/dump/Back.t.core
	-bzip2 -9 /var/log/dump/Front.t.core
	-bzip2 -9 /var/log/dump/ccbrun.core
	-cp --preserve=timestamps /var/log/dump/Back.t.core* ./
	-cp --preserve=timestamps /var/log/dump/Front.t.core* ./
	-cp --preserve=timestamps /var/log/dump/ccbrun.core* ./
	-rm -f /var/log/dump/*.bz2
	-cp --preserve=timestamps /var/log/xiotech/* ./
	-cp --preserve=timestamps /opt/xiotech/apps/Front.t* ./
	-cp --preserve=timestamps /opt/xiotech/apps/Back.t* ./
	-cp --preserve=timestamps /opt/xiotech/apps/ccbrun* ./
#-----------------------------------------------------------------------------
.PHONY:	scp
scp:
# scp -p /opt/xiotech/apps/{Front.t,Back.t,ccbrun} /var/log/xiotech/* /var/log/dump/*.core* marshall_midden@10.64.99.254/crash/save/
	scp -p root@10.64.132.45:'/var/log/xiotech/*' ./
#-----------------------------------------------------------------------------
.PHONY:	apps
apps:
	cp --preserve=timestamps -r ${SRCS}/built-${MODEL}${PD}/{Front.*,Back.*,ccbrun*} ./
#-----------------------------------------------------------------------------
.PHONY:	zip
zip:
	-bzip2 -9 B* F* c* a*log p*log q*out
#-----------------------------------------------------------------------------
.PHONY:	cleancontroller cc
cleancontroller cc:
	-rm -f /var/log/dump/Back.t.core /var/log/dump/Front.t.core /var/log/xiotech/*
#-----------------------------------------------------------------------------
.PHONY:	clean
clean:
	-rm -f Back.t Back.t.core* Front.t Front.t.core* ccbrun ccbrun.core* *.shm*
	-rm -f apps.xiolog* pam.log* ql* cvtcore_${MODEL} subst-elf
	-rm -f .GDBINIT.BE .GDBINIT.FE .GDBINIT.CCB .gdbinit .gdbinit.ccb
	-rm -f A.fe A.be ${TS} A.[bf]e.W BE[12].ilts* BE3.prps* ILTs.* PRPs.* PILTs.lost I1
	-rm -f *.hist *.txt
#-----------------------------------------------------------------------------
.PHONY:	extradelete
extradelete:
	-rm -f Back.t Back.t.core Front.t Front.t.core ccbrun ccbrun.core *.shm
	-rm -f apps.xiolog.* pam.log.* ql* 2400* *.hist *.txt kernel* pam
	-rm -f .GDBINIT.BE .GDBINIT.FE .GDBINIT.CCB .gdbinit .gdbinit.ccb
	-rm -f A.fe A.be ${TS} A.[bf]e.W BE[12].ilts* BE3.prps* ILTs.* PRPs.*
	-rm -f *.hist *.txt
	-rm -f 2400* MD5sums bvm chgnetcfg gzshm iscsid kernel* pam shmdump syssrv xioFidScript
#-----------------------------------------------------------------------------
.PHONY:	src
src:
	tar jcf src.tbz2 -C ${SRCS}/ {.svn,Makefile*,Proc,CCB,Shared}
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	ccbscriptfix ccbfix
ccbscriptfix ccbfix: ${TS} ${CORES}/ccbrun.core ccbrun
	${MASTER}/bin/DEBUGTRACELOGCCB ${TS} A.ccb ${APPS}/ccbrun.nm
	${MASTER}/bin/DEBUGTRACELOGEXITTO A.ccb ccbrun ${CORES}/ccbrun.core
#-----------------------------------------------------------------------------
.PHONY:	A.ccb ccbscriptget
A.ccb ccbscriptget: ${CORES}/ccbrun.core .GDBINIT.CCB .gdbinit.ccb ccbrun
	script -f -c "${MASTER}/bin/DEBUGTRACELOG-ccb .GDBINIT.CCB ccbrun ${CORES}/ccbrun.core ${LENGTH}" ${TS} > /dev/null
	make ccbscriptfix
#-----------------------------------------------------------------------------
.PHONY:	A.ccball
A.ccball: ${CORES}/ccbrun.core .GDBINIT.CCB .gdbinit.ccb ccbrun
	script -f -c "${MASTER}/bin/DEBUGTRACELOG-ccb .GDBINIT.CCB ccbrun ${CORES}/ccbrun.core 800000" ${TS} > /dev/null
	make ccbscriptfix
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	bescriptfix befix
bescriptfix befix: ${TS} ${CORES}/Back.t.core Back.t
	${MASTER}/bin/DEBUGTRACELOGBE ${TS} A.be ${APPS}/Back.nm
	${MASTER}/bin/DEBUGTRACELOGEXITTO A.be Back.t ${CORES}/Back.t.core
#-----------------------------------------------------------------------------
.PHONY:	A.be bescriptget
A.be bescriptget: ${CORES}/Back.t.core .GDBINIT.BE .gdbinit Back.t
	script -f -c "${MASTER}/bin/DEBUGTRACELOG .GDBINIT.BE Back.t ${CORES}/Back.t.core ${LENGTH}" ${TS} > /dev/null
	make bescriptfix
#-----------------------------------------------------------------------------
.PHONY:	A.beall
A.beall: ${CORES}/Back.t.core .GDBINIT.BE .gdbinit
	script -f -c "${MASTER}/bin/DEBUGTRACELOG .GDBINIT.BE Back.t ${CORES}/Back.t.core 800000" ${TS} > /dev/null
	make bescriptfix
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	fescriptfix fefix
fescriptfix fefix: ${TS} ${CORES}/Front.t.core Front.t
	${MASTER}/bin/DEBUGTRACELOGFE ${TS} A.fe ${APPS}/Front.nm
	${MASTER}/bin/DEBUGTRACELOGEXITTO A.fe Front.t ${CORES}/Front.t.core
#-----------------------------------------------------------------------------
.PHONY:	A.feall
A.feall: ${CORES}/Front.t.core .GDBINIT.FE .gdbinit Front.t
	script -f -c "${MASTER}/bin/DEBUGTRACELOG .GDBINIT.FE Front.t ${CORES}/Front.t.core 800000" ${TS} > /dev/null
	make fescriptfix
#-----------------------------------------------------------------------------
.PHONY:	A.fe fescriptget
A.fe fescriptget: ${CORES}/Front.t.core .GDBINIT.FE .gdbinit Front.t
	script -f -c "${MASTER}/bin/DEBUGTRACELOG .GDBINIT.FE Front.t ${CORES}/Front.t.core ${LENGTH}" ${TS} > /dev/null
	make fescriptfix
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	verifybe
verifybe: ${CORES}/Back.t.core .GDBINIT.BE .gdbinit Back.t
	script -f -c "${MASTER}/bin/VERIFYBE .GDBINIT.BE Back.t ${CORES}/Back.t.core" ${TS} > /dev/null
	${MASTER}/bin/verifybe <${TS}  > V.BE 2>&1
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
.PHONY:	FIND.ilts find.ilts
# Located in bin -- uses "make be", "make fe", sed and sort several times, wc, and bin/joinem.
FIND.ilts find.ilts:
	${MASTER}/bin/FIND.ilts ${RELEASE}
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
unmake UNMAKE:
	${MAKE} unmakebe
	${MAKE} unmakefe
	${MAKE} unmakeccb

unmakebe:
	-rm -f Back.t Back.t.core cvtcore_3000 subst-elf
	-rm -f .gdbinit .GDBINIT.BE
	-mv Back.t.core.orig Back.t.core
	-gzip Back.t.core

unmakefe:
	-rm -f Front.t Front.t.core cvtcore_3000 subst-elf
	-rm -f .gdbinit .GDBINIT.FE
	-mv Front.t.core.orig Front.t.core
	-gzip Front.t.core

unmakeccb:
	-rm -f ccbrun.t ccbrun.t.core cvtcore_3000 subst-elf
	-rm -f .gdbinit .GDBINIT.CCB
	-mv ccbrun.t.core.orig ccbrun.t.core
	-gzip ccbrun.t.core
#-----------------------------------------------------------------------------
