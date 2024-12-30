# ----------------------------------------------------------------------------
MODEL ?= 3000
TYPE ?= Debug
# ----------------------------------------------------------------------------
# Set locations of shared memory segments, and other mmap-ed areas.
include ../../Makefile-${MODEL}.defs
CFLG += ${MAKE_DEFS_CC}
# ----------------------------------------------------------------------------
# The following turns off cpu affinity and real-time scheduling -- bad if fatal loops happen.
ifdef NOAFFINITY
CFLG += -DNOAFFINITY
endif # NOAFFINITY
#-----------------------------------------------------------------------------
#-- Turned off due to messing up the copy manager.
# CFLG += -DNO_RAID_CACHE
# CFLG += -DNO_VDISK_CACHE
# CFLG += -DNO_PDISK_CACHE	# Not done ?
# CFLG += -DNO_VCD_CACHE	# Not done ?
# CFLG += -DNO_SERVER_CACHE	# Not done ?
# CFLG += -DNO_TARGET_CACHE	# Not done ?
# CFLG += -DNO_VCG_CACHE	# Not done ?	NOTE "G"
# CFLG += -DNO_HAB_CACHE	# Not done ?
#-----------------------------------------------------------------------------
ifeq (${PERF},PERF)
CFLG += -DPERF
endif # PERF
# ----------------------------------------------------------------------------
ifneq (${PERF},PERF)
# This makes heartbeat timeouts very long, and does abort() instead of DeadLoop().
# CFLG += -DM4_ABORT
# To allow following execution of MRP or PI commands.
#-- CFLG += -DFOLLOW_MRP_EXECUTION
#-- CFLG += -DFOLLOW_PI_EXECUTION
# Enable all dprintf messages (not just a handful).
#-- CFLG += -DALL_DPRINTF
# Memory pool checking.
# CFLG += -DM4_DEBUG_MEMORY_WITH_PATTERNS
# CFLG += -DMEMSET_PATTERN_FREE
#-- CFLG += -DCHECK_MEMORY_ALL
# CFLG += -DREALLY_DELAY_MEMORY_FREES
# ----------------------------------------------------------------------------
#-- KEEP_HISTORY = KEEP_HISTORY
ifeq (${MODEL},3000)
# Keep a history tracelog (.gdbinit "tracelog" macro).
#-- KEEP_HISTORY = KEEP_HISTORY
# Add in HISTORY_KEEPING of each routine entry/exit to tracelog.
# NOTE: KEEP_CYG only works when running in gdb. It will not work in normal scenerio.
#-- KEEP_CYG = KEEP_CYG
endif	# MODEL==3000
endif	# !PERF
#-----------------------------------------------------------------------------
ifeq (${KEEP_HISTORY},KEEP_HISTORY)
CFLG += -DHISTORY_KEEP
CFLG += -DHISTORY_TSC_KEEP
CT_HISTORY += CT_history.c
endif
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
ifeq (${KEEP_CYG},KEEP_CYG)
CYG = -finstrument-functions
CT_HISTORY += cyg.c
CYGFILES = AsyncClient.c AsyncEventHandler.c AsyncQueue.c \
	CacheBay.c CacheLoop.c CacheManager.c CacheMisc.c CachePDisk.c CacheRaid.c \
	CacheServer.c CacheTarget.c CacheVDisk.c CmdLayers.c \
	EL.c EL_BayMap.c EL_Disaster.c EL_DiskMap.c EL_KeepAlive.c EL_Strings.c \
	FCM.c FCM_Counters.c FCM_Strings.c FIO.c FIO_Maps.c \
	HWM.c HWM_Analyze.c HWM_ConditionStrings.c HWM_Happy.c HWM_StatusStrings.c \
	LargeArrays.c LogOperation.c MM_mgr.c \
	PI_Admin.c PI_BatteryBoard.c PI_BlkXfr.c PI_ClientPersistentData.c PI_Clients.c \
	PI_Debug.c PI_DiskBay.c PI_FileSys.c PI_GenFunc.c PI_Generic.c PI_Logs.c \
	PI_Misc.c PI_PDisk.c PI_PersistentData.c PI_Pr.c PI_RegisterClients.c \
	PI_Server.c PI_Snapshot.c PI_Stats.c PI_Target.c PI_Utils.c \
	PI_VCG.c PI_VCGFWUpdate.c PI_VDisk.c PI_WCache.c PR.c PacketStats.c \
	ParmVal.c PktCmdHdl.c PortSrv.c PortSrvUtils.c RMCmdHdl.c RM_Raids.c \
	SerBuff.c SerCon.c SerConLogs.c SerConNetwork.c SerConTime.c SerJournal.c \
	Snapshot.c StatsManager.c StatsServer.c \
	X1_AsyncEventHandler.c X1_Packets.c X1_Utils.c X1_VDisk.c X1_VLink.c X1_Workset.c \
	ccb_hw.c ccb_statistics.c codeburn.c convert.c cps_init.c crc32.c \
	ddr.c debug_files.c debug_struct.c errorCodes.c error_handler.c \
	fibreshim.c fm_master.c fm_slave.c heap.c \
	hw_common.c hw_mon_x6dh8_xg2.c hw_mon_x7dwe.c hw_monitor.c i82559.c i82559_Strings.c \
	idr_structure.c ipc_cmd_dispatcher.c ipc_heartbeat.c ipc_listener.c ipc_packets.c \
	ipc_security.c ipc_sendpacket.c ipc_server.c ipc_session_manager.c ipc_socket_io.c \
	kernel.c led.c logging.c low_memory.c mach.c \
	md5.c misc.c mode.c names.c nvram.c nvram_structure.c \
	quorum.c quorum_comm.c quorum_utils.c \
	realtime.c rm.c rm_misc.c rm_realloc.c rm_val.c \
	serial.c serial_num.c ses.c slink.c sm.c \
	timer.c trace.c xssa_structure.c xk_flash.c \
	LL_LinuxLinkLayer.c \
	xk_kernel.c xk_raidmon.c xk_rtc.c led_control.c led_pca9551.c \
 	xk_init.c L_Clone.c L_Misc.c L_Signal.c L_StackDump.c L_XIO3D.c \
	hw_mon_x6dhr_8g2__x6dhr_tg.c li_evt.c
# These slow things up way too much -- trust them to be OK
# 	mem_pool.c memory.c
# This doesn't work with mutex in CT_history.c: main.c code_start.c
# Following not instrumented -- does not work: xk_mapmemfile.c
# NOTE -- SPECIAL = ${OBJ}/xk_mapmemfile.o ${OBJ}/logview.o ${CYG_FILES}
CYG_FILES = ${CYGFILES:%.c=${OBJ}/%.o}
endif
#-----------------------------------------------------------------------------
export VER = ENGR

include makefile-${MODEL}.files
#-----------------------------------------------------------------------------
OBJ := ../obj_${MODEL}${TYPE}

GCC = gcc

INCDIRS := -I ../Inc -I ../../Shared/Inc -I ../../Linux/xio3d
INCDIRS += -I ${OBJ}
#-----------------------------------------------------------------------------
C_DEFS  = -D_REENTRANT -DCCB_RUNTIME_CODE -DCCB_RELEASE=3 -DDISABLE_NAGLE \
	  -DNONFATAL_ASSERT -DHYPERNODE -DLINUX_VER
REALCFLAGS  = ${C_DEFS} ${INCDIRS} -g -c ${CFLG}
REALCFLAGS += ${ON64FLAGS}
ifeq (${RUNON64},RUNON64)
REALCFLAGS += -DRUNON64 -Wno-pointer-sign
endif
REALCFLAGS += -Wall
REALCFLAGS += -Werror
#+++
REALCFLAGS += -W
REALCFLAGS += -Wno-system-headers
#--- Many legitimate uses for aggregate return.  REALCFLAGS += -Waggregate-return
ifeq (${GCCVER},431)
REALCFLAGS += -Warray-bounds                           # added in gcc 4.3.1
REALCFLAGS += -Wattributes                             # added in gcc 4.3.1
endif
REALCFLAGS += -Wbad-function-cast
#-- REALCFLAGS += -Wcast-align
REALCFLAGS += -Wcast-qual
REALCFLAGS += -Wchar-subscripts
REALCFLAGS += -Wcomments
REALCFLAGS += -Wdeprecated-declarations
REALCFLAGS += -Wendif-labels
ifeq (${GCCVER},431)
REALCFLAGS += -Wextra                                  # added in gcc 4.3.1
endif
REALCFLAGS += -Wfloat-equal
REALCFLAGS += -Wformat
REALCFLAGS += -Wformat-nonliteral
REALCFLAGS += -Wformat-security
REALCFLAGS += -Wimplicit-int
REALCFLAGS += -Wimport
REALCFLAGS += -Winline
REALCFLAGS += -Wlong-long
ifeq (${GCCVER},431)
REALCFLAGS += -Wlogical-op                             # added in gcc 4.3.1
endif
REALCFLAGS += -Wmissing-braces
REALCFLAGS += -Wmissing-declarations
REALCFLAGS += -Wmissing-format-attribute
REALCFLAGS += -Wmissing-noreturn
REALCFLAGS += -Wmissing-prototypes
REALCFLAGS += -Wmultichar
REALCFLAGS += -Wnested-externs
ifeq (${GCCVER},431)
REALCFLAGS += -Woverflow                               # added in gcc 4.3.1
endif
# REALCFLAGS += -Wpacked           ## cannot have this.
# REALCFLAGS += -Wpadded           ## cannot have this warning turned on, link layer yells.
REALCFLAGS += -Wparentheses
REALCFLAGS += -Wpointer-arith
REALCFLAGS += -Wredundant-decls
REALCFLAGS += -Wreturn-type
REALCFLAGS += -Wshadow
REALCFLAGS += -Wsign-compare
REALCFLAGS += -Wstrict-aliasing
ifeq (${GCCVER},431)
REALCFLAGS += -Wstrict-overflow                        # added in gcc 4.3.1
endif
REALCFLAGS += -Wstrict-prototypes
REALCFLAGS += -Wswitch
# REALCFLAGS += -Wswitch-default   ## memset() gives warning about missing default on case.
REALCFLAGS += -Wswitch-enum
REALCFLAGS += -Wtrigraphs
REALCFLAGS += -Wundef
ifeq (${PERF},PERF)
REALCFLAGS += -Wuninitialized
endif
REALCFLAGS += -Wunknown-pragmas
# ifeq (${GCCVER},431)
# REALCFLAGS += -Wunreachable-code ## 2008-10-08 -- gives false positives with gcc 3.3.3 and 4.3.1.
# endif
# ifeq (${GCCVER},431)
# REALCFLAGS += -Wunsafe-loop-optimizations                # added in gcc 4.3.1
# endif
REALCFLAGS += -Wunused
REALCFLAGS += -Wunused-function
REALCFLAGS += -Wunused-label
REALCFLAGS += -Wunused-parameter
REALCFLAGS += -Wunused-value
REALCFLAGS += -Wunused-variable
REALCFLAGS += -Wwrite-strings
ifeq (${GCCVER},431)
REALCFLAGS += -Wvolatile-register-var                  # added in gcc 4.3.1
endif
REALCFLAGS += -Wno-cast-qual
#+++
# -- xk_kernel.c and PortServ.c have problems with -O2,3,4. -fstrict-aliasing.
REALCFLAGS += -Wno-strict-aliasing
ifeq (${PERF},PERF)
REALCFLAGS += -O3
# REALCFLAGS += -O4	# This does compile and work.
else
REALCFLAGS += -O0
endif
REALCFLAGS += -march=pentium4

PACKED := -fpack-struct

AFLAGS	= -g -c

# Where to find the source files
VPATH = .:../../Shared/Src
#-----------------------------------------------------------------------------
# Compile rule. I didn't like the very long line for each compile,
# so I replaced it with a simple display of the file being compiled.
${OBJ}/%.o: %.c
	# $@
	@$(GCC) ${CFLAGS} -o $@ $<
${OBJ}/%.o: %.S
	# $@
	@gcc -Wa,--32 ${AFLAGS} -o $@ $<
#-----------------------------------------------------------------------------
CFILES = ${CFILES1} ${CT_HISTORY}
OFILES = ${CFILES:%.c=${OBJ}/%.o}
DEP_FILES = ${CFILES:%.c=${OBJ}/.%.c.depend}

SPECIAL = ${OBJ}/xk_mapmemfile.o ${OBJ}/logview.o ${CYG_FILES}

$(filter-out ${SPECIAL},${OFILES}): override CFLAGS = ${REALCFLAGS} ${PACKED}
${OBJ}/xk_mapmemfile.o: override CFLAGS = ${REALCFLAGS}
${OBJ}/logview.o: override CFLAGS = ${REALCFLAGS} -Wno-format-nonliteral ${PACKED}

${CYG_FILES}: override CFLAGS = ${REALCFLAGS} ${PACKED} ${CYG}

#-----------------------------------------------------------------------------
.PHONY: all
all: ccbrun
#-----------------------------------------------------------------------------
${OBJ}/ccbrun.a: ${OFILES}
	# $@
	@ar cr $@ ${OFILES}
#-----------------------------------------------------------------------------
.PHONY: ccbrun
ccbrun: ${OBJ}/ccbrun ${DEP_FILES}
#-----------------------------------------------------------------------------
${OBJ}/ccbrun: ${OBJ}/ccbrun.a ccb.ld
	# $@
	@gcc ${ON64FLAGS} -o $@ -Xlinker --warn-common -Xlinker -Map \
	    -Xlinker $@.map -Xlinker --defsym \
	    -Xlinker MAKE_DEFS_CCB_LOAD=${MAKE_DEFS_CCB_LOAD} \
	    -Xlinker --defsym -Xlinker MAKE_DEFS_CCB_ONLY_MEMORY=${MAKE_DEFS_CCB_ONLY_MEMORY} \
	    -Xlinker --defsym -Xlinker MAKE_DEFS_CCB_DATA_MEMORY=${MAKE_DEFS_CCB_DATA_MEMORY} \
	    -Xlinker -T -Xlinker ccb.ld -pthread -lm ${OBJ}/code_start.o $<
	@nm -n -l $@ > ${OBJ}/ccbrun.nm
#-----------------------------------------------------------------------------
# Create generated include files
${OBJ}/.gen:	$(wildcard ../Inc/*.def) $(wildcard ../../Shared/Inc/*.def)
	# $@
	mkdir -p ${@D}
	for f in $?; do ../../Bin/makeinc.pl -o ${@D} $$f; done
	touch $@
#-----------------------------------------------------------------------------
# Create the dependencies file included below.
${OBJ}/.%.c.depend: %.c ${OBJ}/.gen
	# $@
	@mkdir -p ${@D}
	@echo -n "$@" > $@
	@gcc ${REALCFLAGS} -MM $< | sed -e "s=^\([^ ][^ ]*\)= ${OBJ}/\1 Makefile=" >> $@
#-----------------------------------------------------------------------------
# Create TAGS file for us EMACS users
.PHONY: tag tags TAGS ctags etags
tag tags TAGS ctags etags: ${CFILES}
	ctags $^ ../Inc/*.h ../../Shared/Inc/*.h ${OBJ}/*.[hc]
	etags $^ ../Inc/*.h ../../Shared/Inc/*.h ${OBJ}/*.[hc]
#-----------------------------------------------------------------------------
# Special case for FW_HeaderSrc. This is built by requiring
# a perl script on FW_HeaderGen.c before compiling.
${OBJ}/FW_HeaderSrc.o: FW_HeaderSrc.c ../../Tools/updfwhdr.pl \
	$(filter-out $(OBJ)/.FW_HeaderSrc.c.depend, ${DEP_FILES}) ${OBJ}/.gen
	# $@
	@mkdir -p ${@D}
	@perl ../../Tools/updfwhdr.pl -v${VER} -i ../../FWRun-${MODEL}.ver  $< ${OBJ}/FW_HeaderGen.c
	@$(GCC) -o $@ ${CFLAGS} ${OBJ}/FW_HeaderGen.c
	@rm -f ${OBJ}/FW_HeaderGen.c
#-----------------------------------------------------------------------------
# Special rules to generate devices.gen from the
# DeviceConfiguration${MODEL}.txt file.
${OBJ}/devices.gen: ../../Tools/DeviceConfiguration${MODEL}.txt
	mkdir -p ${@D}
	./convdev.sh $< > $@
#-----------------------------------------------------------------------------
${OBJ}/.misc.c.depend: ${OBJ}/devices.gen
#-----------------------------------------------------------------------------
.PHONY: clean cleandir
clean:
	if [ -n "${OBJ}" ]; then rm -f ${OBJ}/* ${OBJ}/.*.depend; fi
	rm -f *.o
#	rm -f makefile.files makefile.deps .deps
	root rm -f /dev/shm/CCB_*.mmf
#-----------------------------------------------------------------------------
cleandir: clean
	rm -rf ../obj_${MODEL}*
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Clean target:'
	@echo '  clean           - remove contents of object directory.'
	@echo '  cleandir        - same as "clean"'
	@echo ''
	@echo 'Configuration and compilation targets:'
	@echo '  ccbrun          - rebuild CCB image'
	@echo '  all             - same as "ccbrun"'
	@echo ''
	@echo 'Suggested targets:'
	@echo '  make all'
#-----------------------------------------------------------------------------
# Include the compile dependencies.
ifeq '' "$(strip $(filter clean cleandir obj Build,${MAKECMDGOALS}))"
ifneq '' "$(strip ${DEP_FILES})"
sinclude ${DEP_FILES}
endif
endif
#-----------------------------------------------------------------------------
