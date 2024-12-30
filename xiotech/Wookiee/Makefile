#
# NOTE: Makefile-${MODEL}.defs defines many constants and definitions.
#
JOBS =
#-- JOBS = --jobs
#-- JOBS = --jobs=1
MAKEFLAGS += ${JOBS}
#-----------------------------------------------------------------------------
atime := ${shell date '+%Y-%m-%d,%H:%M:%S'}
SEC_STARTED := ${shell date '+%s'}
#-----------------------------------------------------------------------------
MODELS=3000 7000 4700 7400
export MODEL ?= 3000
include Bin/MMODEL.mk
# Get release info from FWRun-${MODEL}.ver
include FWRun-${MODEL}.ver
include Build/BuildNum
MAJ_REL := $(subst 0x00,,${FW_MAJOR_RELEASE})
MIN_REL := $(patsubst 0x%00,%,${FW_MINOR_RELEASE})
export RELEASE := ${MAJ_REL}${MIN_REL}
export VER := ENGR
VOLID := PS-install-${MMODEL}-${RELEASE}-${VER}
#---------------
# Get kernel version information.
include Linux/xio3d/KERNEL_VERSION-${MODEL}
#---------------
KERNEL := ${KERNVER}-${KMODEL}${KERNREV}
#---------------
# Wookiee Build PC.
WBPC = 10.64.99.244
# ----------------------------------------------------------------------------
export ON64FLAGS = -m32
UNAMEI := ${shell head -1 /etc/SuSE-release | sed 's/.*(//;s/)//' | tr -- - _}
ifeq (${UNAMEI},x86_64)
# Following will change xio3d structures to 64 bit.
# Comment it out if doing cross-compiling on 64 bit to run on 32 bit.
export RUNON64 = RUNON64
endif
#-----------------------------------------------------------------------------
# Proc cannot compile f_front.CT.c and b_back.CT.c at same time in 2gb memory.
ifeq (x${PERF},xPERF)
TYPE := Perf
override VER := ${shell echo ${VER}|tr [:lower:] [:upper:]}
# don't know if I need to export these variables again, but doesn't hurt.
export VER	
export PERF	
export TYPE
BUILD_MATCH := [A-Z]
else
TYPE := Debug
override VER := ${shell echo ${VER}|tr [:upper:] [:lower:]}
export VER	
export PERF	
export TYPE
BUILD_MATCH := [a-z]
endif
BUILT_DIR := built-${MODEL}${TYPE}
OBJ_DIR := obj_${MODEL}${TYPE}
RELVER := ${RELEASE}-${VER}
MRELVER := ${MMODEL}-${RELVER}
#---------------
# Force the default target to be "all".  This allows targets to be put in the
# ~/.wookieemake or ~/.wookieemake-${MODEL} file.
all:
#---------------
# Include local (user) default modifications.
sinclude ~/.wookieemake
# Include local (user) model dependant modifications.
sinclude ~/.wookieemake-${MODEL}
#=============================================================================
.PHONY: all setpermissions proccode ccbcode pamcode iscsicode syssrvcode logsim
.PHONY:	utilcode
all: setpermissions check_for_bad_tabs proccode ccbcode pamcode \
	iscsicode syssrvcode logsim utilcode

# Some people use WinCVS to check out files, and check out read-only.
# Fix up some files here before building anything.
setpermissions:
	root chmod 755 Linux/makeutil/hc

proccode: MKOPTS := MODEL="${MODEL}" VER="${VER}" RELEASE="${RELEASE}"
proccode: setpermissions check_for_bad_tabs
	${MAKE} -C Proc/src ${MKOPTS} obj && \
		${MAKE} -C Proc/src ${MKOPTS} all

ccbcode: MKOPTS := MODEL="${MODEL}" VER="${VER}" RELEASE="${RELEASE}"
ccbcode: setpermissions check_for_bad_tabs
	${MAKE} -C CCB/Src ${JOBS} ${MKOPTS} all

pamcode: setpermissions check_for_bad_tabs
# NOTDONEYET ${JOBS}   pam's Makefile does not have dependancies correct. Messy.
	${MAKE} --jobs=1 -C Linux/pam all

# open-iscsi doesn't have it's dependencies correct, so we always clean it first
iscsicode: setpermissions check_for_bad_tabs
	${MAKE} ${JOBS} -C Linux/open-iscsi/usr all

syssrvcode: setpermissions check_for_bad_tabs
	${MAKE} ${JOBS} -C Linux/syssrv all

logsim: setpermissions check_for_bad_tabs
	${MAKE} --jobs=1 -C CCB/LogSim/Src all

utilcode: setpermissions check_for_bad_tabs
	${MAKE} --jobs=1 -C Linux/util all

# insert push/popls in all C files
# remove existing push/pops and start afresh.
dbgstk:
	perl Proc/src/dbgstk.deletepushpops
	perl Proc/src/dbgstk.preconversion
#	make clean all
	make proccode
	make dbgstk_ancillary

# SHOULD NOT BE USED DIRECTLY.
# insert push/pops in all *CT.c files
dbgstk_ancillary: Proc/${OBJ_DIR}/b_back.CT.c \
		Proc/${OBJ_DIR}/f_front.CT.c
	perl Proc/src/dbgstk.postconversion
#	make proccode
	make yeti

#-----------------------------------------------------------------------------
BUILDLETTER := $(shell v=${FW_BUILDNAME}; echo -n $${v:0:1})
BLD_VER := ${BUILDLETTER}$(shell printf %03d ${BUILD_NUM})

.PHONY: output output-links target_name version version_us

output: MKOPTS := VER=${BLD_VER}
output:
	echo MKOPTS=${MKOPTS} MODELS=${MODELS}
	set -e; for m in ${MODELS}; do \
		${MAKE} MODEL=$${m} ${MKOPTS} perf 2>&1 | tee bld$${m}p.out; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit $${PIPESTATUS[0]}; fi; \
		${MAKE} MODEL=$${m} ${MKOPTS} PERF=PERF cd 2>&1 | tee bld$${m}cd.out; \
		if [[ $${PIPESTATUS[0]} -ne 0 ]]; then exit $${PIPESTATUS[0]}; fi; \
	done

output-links:
	set -e; for m in ${MODELS}; do \
		blt=${PWD}/built-$${m}Perf; \
		mkdir -p ${OUTPUT_DIR}/$${m}_bin || exit 1; \
		ln -sfn $${blt}/*.{rpm2,xzp,zip} ${OUTPUT_DIR} || exit 1; \
		ln -sfn $${blt}/cvtcore_* ${OUTPUT_DIR} || exit 1; \
		ln -sfn $${blt}/{ccbrun,*.t} ${OUTPUT_DIR}/$${m}_bin || exit 1; \
		ln -sfn $${blt}/logsim* ${OUTPUT_DIR}/$${m}_bin || exit 1; \
	done
	ln -sfn ${PWD}/Linux/install-cd/PS-install-*.iso ${OUTPUT_DIR}
	ln -sfn ${PWD}/CCBE ${OUTPUT_DIR}
	ln -sfn ${PWD}/Linux/util/obj_7000/subst-elf ${OUTPUT_DIR}
	scp ${PWD}/built-7000Perf/*.rpm2 \
		root@10.64.102.56:/bldtest/TEST/engbuild.rpm2
	ssh -a -f -x root@10.64.102.56 "(cd /bldtest/TEST ; \
		perl XTC.pl BVT1S SMOKE) &>Test.run.output1" &
	ssh -a -f -x root@10.64.102.56 "(cd /bldtest/TEST ; \
		perl XTC.pl BVT2S SMOKE) &>Test.run.output2" &

target_name:
	@echo Wookiee

version:
	@echo "${BLD_VER}"

version_us:
	@echo "${BLD_VER}"

#-----------------------------------------------------------------------------
.PHONY: nocr
# Grep for possible coding standard violations.
nocr:
	${MAKE} -i cleandir
	echo 'tabs'
	fgrep -nr '	' Makefile Proc/src Proc/inc Shared CCB/Inc CCB/Src 1>&2

#-----------------------------------------------------------------------------
.PHONY: check_for_bad_tabs
check_for_bad_tabs:	setpermissions
	@echo '********************************************************'
	@echo '**********       Checking for Tabs:           **********'
	@echo '********************************************************'
	@(find Proc/src Proc/inc CCB/Src CCB/Inc Shared/Src Shared/Inc -maxdepth 1 | \
		grep -E -i '\.c$$|\.h$$|\.as$$|\.inc$$' | \
		xargs grep -l '	') && exit 1 || true
	@echo '********************************************************'
	@echo '**********       Tab check done               **********'
	@echo '********************************************************'
#-----------------------------------------------------------------------------
.PHONY: nogpl
# Grep for possible GPL violations.
nogpl:
	${MAKE} -i cleandir
	fgrep -r linux/ . | grep include
	fgrep -r GNU . | grep include
#-----------------------------------------------------------------------------
.PHONY: clean cleandir
clean cleandir:
#	${MAKE} -C Linux/xio3d clean
	rm -f ${LASTBLD_FN} *.rpm2 *.xzp *.zip
	rm -rf built-*
	rm -rf CCB/Build
	${MAKE} -C CCB/Src cleandir
	${MAKE} -C Proc/src cleandir
	rm -rf Proc/BuildFE
	rm -rf Proc/BuildBE
	rm -rf Proc/Release
	${MAKE} -C Linux/sharedmemory clean
	${MAKE} -C Linux/pam clean
	${MAKE} -C Linux/open-iscsi/usr clean
	${MAKE} -C Linux/syssrv clean
	${MAKE} -C Linux/util clean
	${MAKE} -C CCB/LogSim/Src clean
#-----------------------------------------------------------------------------
.PHONY: debug Debug DEBUG
debug Debug DEBUG:
ifeq (x${PERF},xPERF)
	@echo
	@echo 'Please do not use the PERF=PERF syntax anymore. Use "make PERF" instead.'
	@echo
else
	${MAKE} PERF= MODEL=${MODEL} BuildAndPackage
endif
#-----------------------------------------------------------------------------
.PHONY: perf Perf PERF 
perf Perf PERF:
	${MAKE} PERF=PERF BuildAndPackage
#-----------------------------------------------------------------------------
.PHONY: yeti YetiApps yetiapps
yeti YetiApps yetiapps:
	@echo
	@echo 'The "yeti" target is obsolete; Please specify either "DEBUG" or "PERF"!'
	@echo
	@false
#-----------------------------------------------------------------------------
.PHONY: BuildAndPackage
BuildAndPackage:
	@echo BuildAndPackage: VER=${VER}  PERF=${PERF}
	${MAKE} MODEL=${MODEL} PERF=${PERF} all
	mkdir -p ${BUILT_DIR}
# Firstly: copy in the CCB, BE, FE executable applications.
	cp -f CCB/${OBJ_DIR}/ccbrun ${BUILT_DIR}/
	cp -f CCB/${OBJ_DIR}/ccbrun.{map,nm} ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/Back.t ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/Back.{map,nm} ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/Front.t ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/Front.{map,nm} ${BUILT_DIR}/
#	chmod a+x ${BUILT_DIR}/ccbrun ${BUILT_DIR}/Back.t ${BUILT_DIR}/Front.t
# Secondly: copy in the FE QLogic hba code, then BE code.
	cp -f Proc/${OBJ_DIR}/2400mid.bin ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/2400.bin ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/2500mid.bin ${BUILT_DIR}/
	cp -f Proc/${OBJ_DIR}/2500.bin ${BUILT_DIR}/
# Fourthly: copy in Platform Application Management (pam) program & open-iscsi executable.
	cp -f Linux/pam/obj_${MODEL}/pam ${BUILT_DIR}/
	cp -f Linux/open-iscsi/usr/iscsid ${BUILT_DIR}/
# Fifthly: copy in "System Call Server" program.
	cp -f Linux/syssrv/obj_${MODEL}/syssrv ${BUILT_DIR}/
# Sixthly: copy in 'chgnetcfg', 'bvm', and 'xioFidScript' perl scripts.
	cp -f Linux/util/{chgnetcfg,bvm,xioFidScript} ${BUILT_DIR}/
	cp -f Linux/util/{procana,snapana}.pl ${BUILT_DIR}/
	cp -f Linux/util/gzshm ${BUILT_DIR}/
	cp -f Linux/util/obj_${MODEL}/{cvtcore_${MODEL},shmdump,subst-elf} ${BUILT_DIR}
	cp -f CCB/LogSim/obj_${MODEL}/logsim ${BUILT_DIR}/
	cp -f CCB/LogSim/obj_w${MODEL}/logsim.exe ${BUILT_DIR}/
# Sixthly-part2: Copy in the kernel
	cp -f Linux/kernels/kern-${KERNEL}.tar.gz ${BUILT_DIR}/kernel.tgz
	echo ${KERNEL} > ${BUILT_DIR}/kernelver
# Patch, create rpm & ICON zip file.
	${MAKE} MODEL=${MODEL} package
#-----------------------------------------------------------------------------
.PHONY: package relabel
package relabel:
# Seventhly: create the RPM
	${MAKE} -C Linux/rpm all VER=${VER} MODEL=${MODEL}
# Lastly: create the icon loadable 'zip' file.
	rm -f ${BUILT_DIR}/*.zip
	perl Tools/FWKBuild/CrtFWZipLinux ${MODEL} ${MMODEL} ${RELEASE} ${VER} \
		"${TYPE}"
	if [ -x Bin/UpdateSigner/UpdateSigner.pl ]; then \
		${MAKE} VER=${VER} MODEL=${MODEL} ewokpkg; fi
#-----------------------------------------------------------------------------
.PHONY: ewokpkg
RPM2PATH := XIOTECH-PLATFORM-${MRELVER}/Firmware ${RELVER}/${MRELVER}.rpm2
ewokpkg:
	test -x Bin/UpdateSigner/UpdateSigner.pl || \
		{ echo "Bin/UpdateSigner not available"; exit 1; }
	cd Bin/UpdateSigner; \
		./UpdateSigner.pl -o ${PWD}/${BUILT_DIR} -t ${MODELTYP} \
			-v ${RELVER} -p "${RPM2PATH}" \
			Templates/Xiotech-Platform.xml ${PWD}/${BUILT_DIR}/*.zip
#-----------------------------------------------------------------------------
.PHONY: install
install:
# if ${BUILT_DIR} does not exist, build the entire codebase and create the RPM
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
# create the 'xiotech' home directory if it doesn't exist
	if [ ! -d ~xiotech ]; then \
	  root mkdir ~xiotech && \
	  root chown xiotech.xiotech ~xiotech; \
	  fi
	${MAKE} xiotech_directories
# install the RPM sitting in ${BUILT_DIR}
	${MAKE} -C Linux/rpm apply
# copy the 'bvm' perl script to /opt/xiotech
	root install -m 755 -o 0 -g 0 Linux/util/bvm /opt/xiotech/
	root install -m 755 -o 0 -g 0 Linux/util/xioFidScript /opt/xiotech/
# Set the release directory permissions and ownership
	if [ -d ~xiotech/release ]; then \
		root chown -R root:xiotech ~xiotech/release && \
		root chmod -R 775 ~xiotech/release; \
	fi
#-----------------------------------------------------------------------------
.PHONY: installdebug
installdebug:
# copy the 'ccb.cfg' config file, but don't overwrite if it exists
	@if [ ! -r /opt/xiotech/ccbdata/ccb.cfg ]; then \
		root install -m 666 -o 0 -g 0 Linux/util/ccb.cfg /opt/xiotech/ccbdata/; \
		echo " ccb.cfg successfully installed"; \
	else \
		echo " ********* WARNING!! ccb.cfg already exists. **********";\
		echo " *********       No changes made.            **********";\
	fi
#-----------------------------------------------------------------------------
.PHONY: zero
zero:
	cd Linux/sharedmemory && make zero
	root ./Linux/sharedmemory/zero
.PHONY: fakepam
fakepam:
	cd Linux/pam/obj_${MODEL} && root ./fakepam
#-----------------------------------------------------------------------------
.PHONY: run
run:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} zero
	(cd ${BUILT_DIR} && root ./ccbrun 2>AAA.CCB_`../Linux/makeutil/getipaddr` 1>&2 &)
	sleep 2
	(cd ${BUILT_DIR} && root ./Front.t 2>AAA.Front_`../Linux/makeutil/getipaddr` 1>&2 &)
	sleep 2
	(cd ${BUILT_DIR} && root ./Back.t 2>AAA.Back_`../Linux/makeutil/getipaddr` 1>&2 &)
#-----------------------------------------------------------------------------
.PHONY: everything
everything: clean yeti install
#=============================================================================
.PHONY: nvrsetup nvramsetup nvram
nvrsetup nvramsetup nvram:
	${MAKE} xiotech_directories
	root rm -f /opt/xiotech/ccbdata/CCB_FLASH.mmf /opt/xiotech/ccbdata/CCB_FLASH.mmf
	root rm -f /opt/xiotech/procdata/shared_memory_NVSRAM_BE /opt/xiotech/procdata/shared_memory_NVSRAM_FE
	root ./Linux/nvrsetup/nvrsetup `./Linux/makeutil/getipaddr` `./Linux/makeutil/getnetmask` `./Linux/makeutil/getgateway` /opt/xiotech/ccbdata/CCB_NVRAM.mmf 10234 0
#=============================================================================
# This builds an iso of an install CD for the given model and version.
# Example: make PERF=PERF VER=H004 MODEL=3000 cd
.PHONY: cd
cd:
	cd ./Linux/install-cd && \
		${MAKE} --jobs=1 MODEL=${MODEL} VER=${VER} cd
#=============================================================================
.PHONY: help
help:
	@echo ''
	@echo 'Cleaning target:'
	@echo '   clean          - remove generated files'
	@echo 'Compilation target:'
	@echo '   all            - Debug build, NO package'
	@echo 'Full Build targets:'
	@echo '   debug          - Debug build & package.'
	@echo '   perf           - Performance build & package.'
	@echo 'Installation to user xiotech target:'
	@echo '   install        - Create required dirs and install the RPM from ${BUILT_DIR}.'
	@echo 'Miscellaneous target:'
	@echo '   everything     - Do suggested targets (see below).'
	@echo 'Running targets:'
	@echo '   run            - Enter ${BUILT_DIR} directory and run it in 3 X windows.'
	@echo '   nvrsetup       - Delete the NVRAM and FLASH files and re-create one.'
	@echo 'Suggested targets:'
	@echo '   "clean debug" - Do a clean build.'
	@echo 'Try "make morehelp" for more complicated targets and information.'
	@echo ''

.PHONY: morehelp
morehelp:
	@echo 'Debugging targets:'
	@echo '   gdbrun         - Three X windows, each with running gdb.'
	@echo '   gdb            - gdb NOT running in three X windows.'
	@echo '   installdebug   - Install the ccb.cfg file.'
	@echo 'Fixing things up targets:'
	@echo '   zero           - Zero out the kernel shared memory.'
	@echo 'Variables possibly passed into the Makefile:'
	@echo "   MODEL=MMMM     - application and rpm target model (optional; default: ${MODEL})"
	@echo "   RELEASE=RRRR   - application and rpm release version (optional; default: ${RELEASE})"
	@echo "   VER=VVVV       - application and rpm build version (optional; default: ${VER})"
	@echo 'Speciality:'
	@echo '   target_name   - Build system used to output target name'
	@echo '   output        - Build system used to do a complete build'
	@echo '   version       - Build system used to output version'
	@echo '   version_us    - Build system used to output version w/o periods'
	@echo '   cd            - Create the CD'
	@echo 'Specific users:'
	@echo '   m4_gdb        - Run 3 xterms having gdb positioned as m4 wants them.'
	@echo ''
#=============================================================================
# Following are some perl scripts that might be interesting to run.
# .PHONY: mfg_clean mfgclean mfg-clean mc
# mfg_clean mfgclean mfg-clean mc:
# 	cd Test && perl -I../CCBE mfg_clean.pl `../Linux/makeutil/getipaddr`
#-----------------------------------------------------------------------------
# .PHONY: wipe_clean wipeclean wipe-clean wc
# wipe_clean wipeclean wipe-clean wc:
# 	cd Test && perl -I../CCBE wipe_clean.pl `../Linux/makeutil/getipaddr` -l
#-----------------------------------------------------------------------------
# .PHONY: regression_single regressionsingle regression-single rs
# regression_single regressionsingle regression-single rs:
# 	cd Test && perl -I../CCBE regression_single.pl `../Linux/makeutil/getipaddr` -r
#-----------------------------------------------------------------------------
# .PHONY: SCM scm
# SCM scm:
# 	cd Test && perl -I../CCBE -IPerlLibs XTC.pl Yeti_`../Linux/makeutil/whichmachine` SCM
#-----------------------------------------------------------------------------
# .PHONY: ICTest ictest
# ICTest ictest:
# 	# cd Test && perl -I../CCBE -IPerlLibs XTC.pl Yeti_`../Linux/makeutil/whichmachine` ICTest
#-----------------------------------------------------------------------------
# .PHONY: BEStressAllCases bestress
# BEStressAllCases bestress:
# 	cd Test && perl -I../CCBE -IPerlLibs XTC.pl Yeti_`../Linux/makeutil/whichmachine` BEStressAllCases
#-----------------------------------------------------------------------------
# .PHONY: BFDump
# BFDump:
# 	cd Test && perl -I../CCBE BFDump.pl `../Linux/makeutil/getipaddr`
#=============================================================================
.PHONY: debugconsole DebugConsole
debugconsole DebugConsole:
	cd CCBE && perl debugconsole -s 10 ~/debugconsole.txt
#=============================================================================
# .PHONY: snapdump SnapDump
# snapdump SnapDump:
# 	rm -rf /tmp/snapdump
# 	- mkdir /tmp/snapdump
# 	cd CCBE && perl SnapDump.pl -o /tmp/snapdump 10.64.102.5
# ... below same as above on linux.
###cd CCBE && perl SnapDump.pl -l ../CCB/LogSim/Src/logsim -o /tmp/snapdump 10.64.102.5
#	cd Linux/util && ./snapana /tmp/snapdump
###cd ${BUILD_DIR} && ./snapana /tmp/snapdump
#=============================================================================
# Following are for debugging via gdb.
.PHONY: gdbrun
gdbrun:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
	cd ${BUILT_DIR} && xterm -title GDB_CCB     -e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNCCB ./ccbrun' AAA.CCB_`../Linux/makeutil/getipaddr` & \
	  sleep 2; \
	  cd ${BUILT_DIR} && xterm -title GDB_FrontEnd  -e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNFE ./Front.t' AAA.Front_`../Linux/makeutil/getipaddr` & \
	  cd ${BUILT_DIR} && xterm -title GDB_BackEnd -e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNBE ./Back.t' AAA.Back_`../Linux/makeutil/getipaddr` &
#-----------------------------------------------------------------------------
.PHONY: gdb
gdb:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
	cd ${BUILT_DIR} && xterm -title GDB_CCB     -e script -f -c 'root gdb ./ccbrun' AAA.CCB_`../Linux/makeutil/getipaddr` & \
	  cd ${BUILT_DIR} && xterm -title GDB_FrontEnd  -e script -f -c 'root gdb ./Front.t' AAA.Front_`../Linux/makeutil/getipaddr` & \
	  cd ${BUILT_DIR} && xterm -title GDB_BackEnd -e script -f -c 'root gdb ./Back.t' AAA.Back_`../Linux/makeutil/getipaddr` &
#=============================================================================
.PHONY: xiotech_directories
xiotech_directories:
	if [ ! -d /opt/xiotech/ccbdata ]; then \
	  root mkdir -p /opt/xiotech/ccbdata; \
	  root chown xiotech.xiotech /opt/xiotech/ccbdata; \
	  fi
	if [ ! -d /opt/xiotech/procdata ]; then \
	  root mkdir -p /opt/xiotech/procdata; \
	  root chown xiotech.xiotech /opt/xiotech/procdata; \
	  fi
	if [ ! -d /var/log/xiotech ]; then \
	  root mkdir -p /var/log/xiotech; \
	  fi
#=============================================================================
# DE -- bring up 3 xterm's running gdb, nicely position as DE wants them.
.PHONY: de_gdb
de_gdb:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} zero
	cd ${BUILT_DIR} && xterm -fn "10x20" -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_CCB_`../Linux/makeutil/getipaddr`  \
		-e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNCCB ./ccbrun' AAA.CCB_`../Linux/makeutil/getipaddr` & \
	  sleep 2; \
	  cd ${BUILT_DIR} && xterm -fn "10x20" -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_FrontEnd_`../Linux/makeutil/getipaddr`  \
		-e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNFE ./Front.t' AAA.Front_`../Linux/makeutil/getipaddr` & \
	  sleep 2; \
	  cd ${BUILT_DIR} && xterm -fn "10x20" -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_BackEnd_`../Linux/makeutil/getipaddr`  \
		-e script -f -c 'root gdb -q -n -x ../Linux/makeutil/GDBRUNBE ./Back.t' AAA.Back_`../Linux/makeutil/getipaddr` &

#-----------------------------------------------------------------------------
ATIME := ${shell date '+%Y-%m-%d@%H:%M:%S.%N'}
MYIP := ${shell ./Linux/makeutil/getipaddr}
# M4 -- bring up 3 xterm's running gdb, nicely position as M4 wants them (upper right virtual).
.PHONY: m4_gdb
m4_gdb:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
# windows are 80 columns wide, 10 pixels per character, 800 dots wide, plus 24 for scroll bar.
	cd ${BUILT_DIR} && \
	  echo "MYIP=${MYIP}  ATIME=${ATIME}"; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_CCB_"${MYIP}"_"${ATIME}" -geometry "85x90+2664+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./ccbrun' AAA.CCB_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_FrontEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+1599+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNFE ./Front.t' AAA.Front_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_BackEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+2132+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNBE ./Back.t' AAA.Back_"${MYIP}"_"${ATIME}" &
# \
#	  sleep 2; \
#	  xterm -rw -vb -sb -sl 20480 -si -sk \
#	    -title GDB_SYSSRV_"${MYIP}"_"${ATIME}" -geometry "85x90+797+0" \
#		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./syssrv' AAA.SYSSRV_"${MYIP}"_"${ATIME}"
	
# M4 -- bring up 3 xterm's running gdb, nicely position as M4 wants them (upper left virtual).
.PHONY: m4_gdb2
m4_gdb2:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
	cd ${BUILT_DIR} && \
	  echo "MYIP=${MYIP}  ATIME=${ATIME}"; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_CCB_"${MYIP}"_"${ATIME}" -geometry "85x90+0+0" \
		-e script -f -c 'root gdb -q -n -x /ohome/m4/r/GDB/GDBRUNCCB ./ccbrun' AAA.CCB_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_FrontEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+1066+0" \
		-e script -f -c 'root gdb -q -n -x /ohome/m4/r/GDB/GDBRUNFE ./Front.t' AAA.Front_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_BackEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+533+0" \
		-e script -f -c 'root gdb -q -n -x /ohome/m4/r/GDB/GDBRUNBE ./Back.t' AAA.Back_"${MYIP}"_"${ATIME}" &
# \
#	  sleep 2; \
#	  xterm -rw -vb -sb -sl 20480 -si -sk \
#	    -title GDB_SYSSRV_"${MYIP}"_"${ATIME}" -geometry "85x90+2472+0" \
#		-e script -f -c 'root gdb -q -n -x /ohome/m4/r/GDB/GDBRUNCCB ./syssrv' AAA.SYSSRV_"${MYIP}"_"${ATIME}"

# M4 -- bring up 3 xterm's running gdb, nicely position as M4 wants them (lower right virtual).
.PHONY: m4_gdb3
m4_gdb3:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
# windows are 80 columns wide, 10 pixels per character, 800 dots wide, plus 24 for scroll bar.
	cd ${BUILT_DIR} && \
	  echo "MYIP=${MYIP}  ATIME=${ATIME}"; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_CCB_"${MYIP}"_"${ATIME}" -geometry "85x90+0+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./ccbrun' AAA.CCB_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_FrontEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+1066+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNFE ./Front.t' AAA.Front_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_BackEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+533+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNBE ./Back.t' AAA.Back_"${MYIP}"_"${ATIME}" &
# \
#	  sleep 2; \
#	  xterm -rw -vb -sb -sl 20480 -si -sk \
#	    -title GDB_SYSSRV_"${MYIP}"_"${ATIME}" -geometry "85x90+797+0" \
#		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./syssrv' AAA.SYSSRV_"${MYIP}"_"${ATIME}"

# M4 -- bring up 3 xterm's running gdb, nicely position as M4 wants them (lower left virtual).
.PHONY: m4_gdb4
m4_gdb4:
	if [ ! -d ${BUILT_DIR} ]; then ${MAKE} MODEL=${MODEL} debug; fi
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
	cd ${BUILT_DIR} && \
	  echo "MYIP=${MYIP}  ATIME=${ATIME}"; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_CCB_"${MYIP}"_"${ATIME}" -geometry "85x90+0+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./ccbrun' AAA.CCB_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_FrontEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+1066+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNFE ./Front.t' AAA.Front_"${MYIP}"_"${ATIME}" & \
	  sleep 2; \
	  xterm -rw -vb -sb -sl 20480 -si -sk \
	    -title GDB_BackEnd_"${MYIP}"_"${ATIME}" -geometry "85x90+533+0" \
		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNBE ./Back.t' AAA.Back_"${MYIP}"_"${ATIME}" &
# \
#	  sleep 2; \
#	  xterm -rw -vb -sb -sl 20480 -si -sk \
#	    -title GDB_SYSSRV_"${MYIP}"_"${ATIME}" -geometry "85x90+2472+0" \
#		-e script -f -c 'root gdb -q -n -x /home/m4/r/GDB/GDBRUNCCB ./syssrv' AAA.SYSSRV_"${MYIP}"_"${ATIME}"
#-----------------------------------------------------------------------------
# M4 on controller!
.PHONY: m4controller
MYIP := ${shell ./Linux/makeutil/getipaddr}
MYMAKEUTIL := ${shell pwd}/Linux/makeutil
m4controller:
	${MAKE} MODEL=${MODEL} fakepam
	${MAKE} MODEL=${MODEL} zero
	cd /opt/xiotech/apps && xterm -title GDB_CCB_${MYIP} -geometry "80x90+1095+0" \
		-e root script -f -c 'root gdb -q -n -x ${MYMAKEUTIL}/GDBRUNCCB ./ccbrun' AAA.CCB_${MYIP} & \
	  sleep 2; \
	  cd /opt/xiotech/apps && xterm -title GDB_FrontEnd_${MYIP} -geometry "80x90+88+0" \
		-e root script -f -c 'root gdb -q -n -x ${MYMAKEUTIL}/GDBRUNFE ./Front.t' AAA.Front_${MYIP} & \
	  sleep 2; \
	  cd /opt/xiotech/apps && xterm -title GDB_BackEnd_${MYIP} -geometry "80x90+592+0" \
		-e root script -f -c 'root gdb -q -n -x ${MYMAKEUTIL}/GDBRUNBE ./Back.t' AAA.Back_${MYIP}
#-----------------------------------------------------------------------------
.PHONY: remote
remote:
	ssh ${WBPC} "cd ${shell pwd} && make $(filter-out remote, ${MAKECMDGOALS})"
	@echo ""
	@echo "IGNORE ERROR BELOW"
	@false
#-----------------------------------------------------------------------------
.PHONY: TAGS
TAGS:
	@ctags -R --c++-types=+px --excmd=pattern --exclude=Makefile --exclude=*.CT.c -f wookiee.tags Proc/ CCB/ Shared/
#@etags Proc/src/*.[ch] Proc/inc/*.[ch] Shared/Src/*.[ch] Shared/Inc/*.[ch] CCB/Src/*.[ch] CCB/Inc/*.[ch] \
#	Linux/open-iscsi/usr/*.[ch] Linux/pam/*.[ch]  Linux/syssrv/*.[ch] Linux/logtrim/*.[ch]
#-----------------------------------------------------------------------------
.PHONY: test
test:
	${MAKE} -C Proc/src test
#-----------------------------------------------------------------------------
ifndef CSCOPE_ARGS
CSCOPE_ARGS=-q
endif
.PHONY: cscope
cscope:
	@cscope -b $(CSCOPE_ARGS) -IProc/inc -IShared/Inc -I CCB/Inc -sProc/src -sShared/Src -sCCB/Src
#-----------------------------------------------------------------------------
# vi:ts=4 sw=4 noexpandtab
# End of Makefile
