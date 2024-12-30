# $Id: system.inc 161368 2013-07-29 14:53:10Z marshall_midden $
#**********************************************************************
#
#  NAME: system.inc (SYSTEM)
#
#  DESCRIPTION:
#
#       To provide a common means of supplying system definitions.
#
#  Copyright (c) 1996-2008 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- General equates -------------------------------------------------
#
        .set    BIT31,0x80000000        # Sign bit constant
        .set    TRUE,1                  # True condition
        .set    FALSE,0                 # False condition
        .set    GOOD,0                  # Good condition
        .set    ERROR,1                 # Error condition
#
# --- Kernel/misc layer assigments ------------------------------------
#
        .set    TIMERPRI,10             # k$timer priority
        .set    DFMPRI,11               # k$dfmexec priority
        .set    STATSPRI,20             # k$stats priority
        .set    HBEATPRI,PCOMPPRI-1     # Heartbeat exec priorities - very high
        .set    MISCPRI,160             # m$exec priority
#
        .set    VERS,1                  # Version
        .set    REV,0                   # Revision
#
        .set    QUANTUM,125             # Time quantum in ms - must be
                                        #  evenly divisible into 1000
        .set    MAXTWAIT,0x7fffffff     # Maximum delay for K$twait
#
        .set    MAXGPDELAY,500          # Maximum M$gpdelay in ms
.ifdef Perf
        .set    HBEATTIME,5000          # Time between heartbeat checks in ms (5 seconds)
.else   # Perf
        .set    HBEATTIME,50000         # Time between heartbeat checks in ms (50 seconds)
.endif  # Perf
#
# --- Minimum memory resources
#
        .set    MINCDRAM,262144         # Minimum cached DRAM
        .set    MINNCDRAM,1048576*4     # Minimum non-cached DRAM
#
# --- Memory granularity
#
        .set    MGRAN,63                # Memory granularity - 1
#;        .set    BGRAN,63                # Buffer granularity - 1
        .set    BGRAN,15                # Buffer granularity - 1
#
# --- Initial packet allocations
#
        .set    IILTS,8192              # Initial ILT allocation
        .set    IPRPS,512               # Initial PRP allocation
        .set    IRRPS,8                 # Initial RRP allocation
        .set    ISCBS,256               # Initial SCB allocation
        .set    IRPNS,256               # Initial RPN allocation
        .set    IRRBS,256               # Initial RRB allocation
        .set    MAXSRVPRI,127           # Maximum server priority
.ifdef FRONTEND
        .set    MAXCDRAMILTS,2*IILTS    # Maximum number of Cacheable DRAM ILTs
        .set    MAXCDRAMVRPS,4096       # Maximum number of Cacheable DRAM VRPs
.else   # BACKEND
        .set    MAXCDRAMILTS,4*IILTS    # Maximum number of Cacheable DRAM ILTs
        .set    MAXCDRAMVRPS,8192       # Maximum number of Cacheable DRAM VRPs
.endif
#
# --- NVRAM Layer assignments -----------------------------------------
#
        .set    NV_SCRUB_PRIORITY,220   # NVRAM Scrub Priority
        .set    NV_DMA_EXEC_PRIORITY,26 # NV Memory DMA Executor priority
        .set    NV_DMA_COMP_PRIORITY,25 # NV Memory DMA Completor priority
#
# --- ISP (Front end) layer assignments -------------------------------
#
        .set    ISPMONCPRI,22           # _isp_monitorFwhb priority
        .set    ISPPONPRI,23            # _isp_portOnlineHandler priority
.ifdef FRONTEND
        .set    ISPMONPRI,205           # isp$monitor priority
        .set    ISPRCVIOEXECPRI,200     # rcv io exec priority
.else
        .set    ISPMONPRI,30            # isp$monitor priority
.endif

.ifdef FRONTEND
        .set    ISPMONAPRI,200          # isp$monitor_async priority
        .set    ISPREGVP,168            # isp_registerVports priority
.ifdef PORT_NUMBER
        .set    MAXISP,PORT_NUMBER      # Maximum ISP devices supported
.else  /* PORT_NUMBER */
/* Following defined in Shared/Inc/XIO_Const.h, and chn.inc. --NOTE: number FE must equal BE */
        .set    MAXISP,4                # Maximum ISP devices supported
.endif /* PORT_NUMBER */

        .set     ICL_PORT,4
.else
        .set    ISPMONAPRI,18           # isp$monitor_async priority
        .set    ISPMONLPRI,19           # isp$monitor_loop priority
/* Following defined in chn.inc. --NOTE: number FE must equal BE */
        .set    MAXISP,MAXCHN           # Maximum ISP devices supported
.endif
.if FE_ICL
        .set    MAXICL,1
.else
        .set    MAXICL,0
.endif

#
# --- CDriver layer assignments ---------------------------------------
#
        .set    CDEXECPRI,190           # cd$exec priority
        .set    CIMTMAX,MAXISP+MAXICL   # Max. # CIMTs to support + ICL
# NOTE: MAX FE LUNS change needed below.
        .set    LUNMAX,64               # Max. # LUNs per host
#
# --- APL layer assignments -------------------------------------------
#
        .set    APLABPRI,182            # Abort task priority
        .set    APLTIMERPRI,180         # apl$timer priority
        .set    OIL2TOVAL,8             # OIL2 time out in ticks(250ms)
#
# --- IDriver layer assignments ---------------------------------------
#
        .set    ITCPRI,246              # i$tmt_cleanup priority
        .set    IRESCANPRI,176          # i$rescan priority
        .set    ISCANPRI,174            # Ploop and Fabric Scan priorities
        .set    ISDISCPRI,172           # i$start_discovery priority
        .set    IEXECPRI,170            # i$exec priority
        .set    MAXIF,MAXISP            # Max. # of interfaces
        .set    ICIMTMAX,MAXIF+MAXICL   # Max. # ICIMT to support
#
# --- MagDriver layer assignments -------------------------------------
#
        .set    MAGEXECPRI,165          # mag$exec priority
        .set    MAGFUPRI,160            # mag$fu priority
        .set    MAGICPRI,245            # mag$imt_cleanup priority
#
# --- Link-level Driver layer assignments -----------------------------
#
        .set    LLDSRBCR,156            # .snddg_rbcr_250 priority
        .set    LLDSWBCR,154            # .snddg_wbcr_250 priority
        .set    LLDSEXECPRI,152         # lld$srpx priority
        .set    LLDEXECPRI,150          # lld$exec priority
        .set    LLDRTYIOPRI,158         # lld$retryio priority
#
# --- Target assignments ----------------------------------------------
#
# NOTE: MAX FE LUNS change needed below.
        .set    MAXTARGETS,64           # 16 controllers with 4 targets per
        .set    MAXPORTS,4              # 4 ports per DSC
#
# --- Op Throttling assignments ---------------------------------------
#
        .set    VTV_MAX,128            # VDisk Throttle Value Maximum before
                                        #  checking Controller Throttle Value
        .set    CTV_MAX,16384           # Controller Throttle Value Maximum
                                        #  to throttle any more ops
        .set    READ_THROTTLE_VALUE,1   # Read Type Ops Throttle Value
        .set    WRITE_THROTTLE_VALUE,2  # Write Type Ops Throttle Value
#
# --- Cache layer assignments -----------------------------------------
#
        .set    CEXECPRI,148            # c$exec priority
        .set    CBFTPRI,147             # wc$BackgroundFlushTask priority
        .set    CVCDQUEUECHECKPRI,146   # c$VCDQueueCheck priority
        .set    COPRETRYPRI,145         # c$OpRetry task Priority
        .set    CIOEXECPRI,144          # c$ioexec priority
        .set    CFLUSHPRI,142           # wc$FlushTask priority
        .set    CDRPEXECPRI,141         # c$drpexec priority
        .set    CMIRRORPRI,140          # wc$Mirror_exec priority
        .set    MAXBAT,2                # Number of batteries
#
# --- Link 960 layer assignments --------------------------------------
#
.ifdef FRONTEND
        .set    LEORPPRI,124            # l$exec_orp priority
        .set    LEIRPPRI,122            # l$exec_irp priority
        .set    LECOMPPRI,120           # l$exec_comp priority
.else
        .set    LEORPPRI,90             # l$exec_orp priority
        .set    LEIRPPRI,89             # l$exec_irp priority
        .set    LECOMPPRI,88            # l$exec_comp priority
.endif
#
# --- Multi-controller assignments ------------------------------------
#
        .set    MAXCTRL,16              # Maximum controllers in a system
#
# --- Virtual layer assignments ---------------------------------------
#
        .set    VEXECPRI,110            # v$exec priority
        .set    VUPDFESTATUS,120        # v$updFEStatus priority
        .set    VSTATSPRI,20            # v$stats priority
        .set    VSECCOPYPRI,152         # v$seccopy priority
        .set    VSCHEDCOPYPRI,155       # v$sched_copy priority
#
        .set    MAXSEGMENTS,16          # Max RAID devices/virtual device
# NOTE: MAX FE LUNS change needed below.
        .set    MAXVLINKS,64            # Max virtual link devices
#
# --- RAID layer assignments ------------------------------------------
#
        .set    REXECPRI,92             # r$exec priority
        .set    R5EXECPRI,91            # r$r5exec priority
        .set    RXEXECPRI,90            # r$xexec priority
        .set    RCEXECPRI,89            # r$cexec priority
        .set    RSTATSPRI,20            # r$stats priority
        .set    RSCRUBPRI,160           # r$scrub priority
        .set    RCHECKERPRI,95          # r$checker priority
        .set    RERRORPRI,88            # o$rerror exec priority
        .set    RBLDWRITEPRI,105        # RB_UpdateRebuildWriteStateTask prty
        .set    RLLOCALIMGIPPRI,94      # RL_LocalImageIPCheck priority
#
        .set    SCRUBSCALE,31           # Scrub scale factor (ms) per
                                        #  outstanding I/O
        .set    SCRUBMIN,375+1          # Scrub delay minimum (ms)
        .set    SCRUBMAX,8000           # Scrub delay maximum (ms)
#
        .set    SECSIZE,512             # Disk sector size
        .set    DSKBALLOC,1048576       # Disk allocation unit (bytes)
        .set    DSKSALLOC,DSKBALLOC/SECSIZE# Disk allocation unit (sectors)
#
        .set    MIN0STRIPE,128          # Minimum RAID 0/10 stripe size
        .set    MAX0STRIPE,512          # Maximum RAID 0/10 stripe size
#
        .set    MIN5STRIPE,8            # Minimum RAID 5 stripe size
        .set    MAX5STRIPE,64           # Maximum RAID 5 stripe size
#
        .set    MAXIO,4096              # Maximum sector count
#
# Also change XIO_Const.h.
.ifdef  MODEL_3000
        .set    MAXDRIVES,512           # Max number of physical drives
        .set    MAXVIRTUALS,4000        # Max virtual devices
        .set    MAXRAIDS,8000           # Max RAID devices
        .set    MAXSERVERS,1024         # Maximum number of servers
        .set    MAXPDISKSPERRAID,512    # Maximum pdisks that can be striped over
.endif  # MODEL_3000
.ifdef  MODEL_7000
        .set    MAXDRIVES,512           # Max number of physical drives
        .set    MAXVIRTUALS,4000        # Max virtual devices
        .set    MAXRAIDS,8000           # Max RAID devices
        .set    MAXSERVERS,1024         # Maximum number of servers
        .set    MAXPDISKSPERRAID,512    # Maximum pdisks that can be striped over
.endif  # MODEL_7000
.ifdef  MODEL_4700
        .set    MAXDRIVES,512           # Max number of physical drives
        .set    MAXVIRTUALS,4000        # Max virtual devices
        .set    MAXRAIDS,8000           # Max RAID devices
        .set    MAXSERVERS,1024         # Maximum number of servers
        .set    MAXPDISKSPERRAID,512    # Maximum pdisks that can be striped over
.endif  # MODEL_4700
.ifdef  MODEL_7400
        .set    MAXDRIVES,512           # Max number of physical drives
        .set    MAXVIRTUALS,4000        # Max virtual devices
        .set    MAXRAIDS,8000           # Max RAID devices
        .set    MAXSERVERS,1024         # Maximum number of servers
        .set    MAXPDISKSPERRAID,512    # Maximum pdisks that can be striped over
.endif  # MODEL_7400
.ifndef MAXDRIVES
.error "Unknown MODEL"
.endif
#
        .set    MAXSES,64               # Max SES enclosures devices
        .set    MAXMISC,64              # Max MISC devices
        .set    MAXOPDDLISTCOUNT,MAXDRIVES+MAXSES+MAXMISC     # Maximum number of devices in pdd list
#
# --- Define layer assignments ----------------------------------------
#
.ifdef FRONTEND
        .set    DEXECPRI,130            # d$exec priority
.else
        .set    DEXECPRI,90             # d$exec priority
        .set    FEXECPRI,89             # FS$exec priority
.endif
        .set    DLABELPRI,50            # d$label priority
        .set    DINITRAIDPRIO,90        # d$initalize_raid priority
        .set    DPSDREBUILDPRIO,150     # d$psd_rebuild priority
        .set    DSCSIPRI,150            # d$multi_scsiio priority
        .set    DSCHEDIRAIDPRIO,150     # d$exec_rip priority
        .set    DRIPMAXACT,5            # Max number of raid inits at a time
        .set    DERRTRAPPRIO,90         # d$call_errtrap priority
#
# --- Online layer assigments -----------------------------------------
#
        .set    OINITDRVPRI,150         # O$init_drv priority
        .set    ONLINEPRI,50            # o$online priority
        .set    OHOTSWAPPRIO,150        # o$hotswap priority
        .set    OINQPRIO,150            # o$inquire priority
        .set    OSPINDOWNPRIO,150       # O$spindowndrive(x) priority
        .set    OHOTSPARECHECKPRIO,200  # o$hotsparecheck priority
#
# --- Physical layer assignments --------------------------------------
#
        .set    PEXECPRI,50             # p$exec priority
        .set    PINITPRI,32             # p$init priority
        .set    PCOMPPRI,20             # p$comp priority
        .set    PESCPRI,21              # p$escalate priority
#
        .set    SENSESIZ,32             # Size of SCSI request sense
                                        #  return buffer
# NOTE: system.h needs changing too.
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .set    BTIMEOUT,25             # Base I/O timeout (seconds)
        .set    IORETRY,3               # task retry counter (3 retries plus original I/O)
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .set    BTIMEOUT,5              # Base I/O timeout (seconds)
        .set    IORETRY,5               # task retry counter (5 retries plus original I/O)
.endif  # MODEL_4700
.endif  # MODEL_7000
        .set    MAXDEV,2048             # Maximum number of FC targets (LIDs)
                                        #  per channel
        .set    NO_LID,0xffff           # invalid lid
#     .set    MINTAG,1                  # Minimum tagged commands
       .set    MAXTAG,31                # Maximum tagged commands
                                        #  (63 is absolute max)
#       .set    MAX_TAG_SATA,8          # Maximum tagged commands for SATA Drive
        .set    MAXSIMPLE,16            # Maximum consecutive simple
                                        #  tagged commands
        .set    DPOWRITE,TRUE           # Disable Page Out (DPO) for
                                        #  SCSI writes
        .set    DPOVERIFY,TRUE          # Disable Page Out (DPO) for
                                        #  SCSI verifies
        .set    INQRETRY,2              # SCSI Inquiry Count
        .set    FINDIMT_INCONSISTENT,0xFFFFFFFF #Findimt had a mismatch between cimtdir and srvdb
#
# --- ISP (Back end) layer assignments -------------------------------
#
        .set    ONLINE_DELAY,2500/QUANTUM
        .set    OFFLINE_DELAY,10000/QUANTUM
#
# --- File system constants -------------------------------------------
#
        .set    FSYSPRI,35              # fs$writeproc priority
#
# --- Data-link Manager layer assignments -----------------------------
#
.ifdef BACKEND
        .set    DLMVEXECPRI,59          # dlm$vrpx priority
        .set    DLMLRPIOPRI,60          # dlm$lrpio priority
        .set    DLMLRPRTYPRI,60         # dlm$lrprty priority
        .set    DLMLRPAGEPRI,60         # dlm$lrpage priority
        .set    DLMLRPCRPRI,61          # dlm$lrpcr priority
        .set    DLMLDDXPRI,80           # dlm$lddx priority
        .set    DLMTPMTECPRI,81         # dlm$tpmtec priority
        .set    DLMVLOPENPRI,88         # dlm$vlopen priority
        .set    DLMVLMOVEPRI,88         # dlm$vlmove priority
        .set    DLMVLOCKPRI,89          # dlm$vlock priority
        .set    DLMVLCHKPRI,89          # dlm$vlchk priority
        .set    DLMVLAGEPRI,89          # dlm$vlage priority
        .set    DLMRETRYDGPRI,90        # dlm$retrydg priority
#
        .set    MAXLDDS,512             # max. # LDDs to support
.else   # FRONTEND
        .set    DLMVEXECPRI,130         # dlm$vrpx priority
        .set    DLMDRPPRI,132           # dlm$drp priority
        .set    DLMBESRPPRI,133         # dlm$besrp priority
        .set    DLMRETRYDGPRI,137       # dlm$retrydg priority
        .set    DLMPOLLCNTRL,139        # dlm$pollcntrl priority
.endif  # ifdef BACKEND
#
# @@@ FINISH @@@ need to talk to rochester about these values
#
# --- Copy Manager layer assignments ----------------------------------
#
        .set    CMPCMEXECPRI,240        # pcm$exec priority
        .set    CMSPEXECPRI,220         # cm$spexec priority
        .set    CMPOLLEXECPRI,250       # cm$pollexec priority
#
# --- MicroMemory assignments -----------------------------------------
#
        .set    MMMONITORPRI,160        # MicroMemory Monitor priority
        .set    MMTESTPRI,250           # MicroMemory Test Driver priority
#
#
# --- Base memory addresses -------------------------------------------
#
.ifdef FRONTEND
        .set    SHARELOC,      FE_BASEADDR  # Start of FE shared memory
.endif /* FRONTEND */
.ifdef BACKEND
        .set    SHARELOC,      BE_BASEADDR  # Start of BE shared memory
.endif /* BACKEND */
        .set    CCBCOMM,       SHARELOC+0x1000 # CCB Communications area
        .set    DDR_TABLE_ADDR,SHARELOC+0x1000 # The Debug Data Retrieval table

        .set    CCB_COMM_SIZE,0x1000    # Size of the CCB Communications Area
        .set    MAX_DDR_ENTRIES,127     # Maximum number of entries
        .set    MAX_CCB_COM_ENTRIES,128 # Maximum CCB Communications entries
        .set    MAX_DDR_CCB_COM_ENTIES,MAX_DDR_ENTRIES+MAX_CCB_COM_ENTRIES
                                        # Number of entries beyond DDR Header

.ifdef FRONTEND
        # FE CCB Communications area (outside of the DDR) needs to start at
        #   0xA0001FFFF and works backwards (DDR will work forward).  Each entry
        #   must be like a DDR entry (8 byte ID, 4 Byte Address 4 Byte Length)
        .set    FE_NVRAM_P4_ID,SHARELOC+0x1ff0 # The FE NVRAM Part 4 ID
        .set    FE_NVRAM_P4_ADDR,SHARELOC+0x1ff8 # The FE NVRAM Part 4 Address
        .set    FE_NVRAM_P4_LENGTH,SHARELOC+0x1ffC # THe FE NVRAM Part 4 Length

        .set    FE_NVRAM_P4_ID_ASCII_1,0x4E5F4546 # ASCII FE_NV_P4
        .set    FE_NVRAM_P4_ID_ASCII_2,0x34505F56

.else   # BACKEND
        # BE CCB Communications area (outside of the DDR) needs to start at
        #   0xA0001FFFF and works backwards (DDR will work forward).  Each entry
        #   must be like a DDR entry (8 byte ID, 4 Byte Address 4 Byte Length)
.endif  # FRONTEND else BACKEND
#
#
        .set    DIMMSIZEADDR,SHARELOC+0x20d0 # Diag location of size of DIMMs
        .set    TRAPADDR,SHARELOC+0x10000     # start of register storage on errtrap

        .set    DIAGMEMSIZE,0xe000      # Diag size (A0002000 - A0010000)
#
        .set    TRAPADDR_R3,TRAPADDR+0x1C # r3 storage on errtrap
        .set    TRAPADDR_R4,TRAPADDR_R3+4 # r4 storage on nmi errtrap
#
        .set    NVSRAMSIZ,0x00400000    # Size of mapped NVRAM area including
                                        #  MACH and other chips
        .set    NVSRAMONLYSIZE,0x00200000 # Size of just the NVRAM
        .set    NVSRAMSTARTNMI,0x7800   # Start addr of NMI area
        .set    NVSRAMNMISIZ,0x0800     # Size of NMI area
        .set    NVSRAMNMISTATS,0x7F40   # Location of NMI stat area
        .set    NVSRAMNMISTART,NVSRAM+NVSRAMNMISTATS   # Address of NMI stats
                                        # off of the base NVRAM address
        .set    NVSRAMSTARTSN,0x6200    # Start addr of serial number area
        .set    NVSRAMSNSIZ,0x0200      # Size of serial number area
        .set    NVSRAMOLDCSER,0x7F70    # Old location of the controller serial number
        .set    NVSRAMCSER,0x6210       # Location of controller serial number
        .set    NVSRAMRES,0x8000        # Diagnostics area (32K)
        .set    NVSRAMSIZVAR,SHARELOC+0x20ec # Location of total NVRAM size

# Size of Write Record Area MUST be a multiple of 32*nvasiz (0x200)
        .set    NVRAMWRITERECORDAREASIZE,0x40000    # 256kB

.ifdef  FRONTEND

        #   Part 4 NVRAM for RAID 5 Write Records at the FE Mirror Partner (48K)
        .set    NVSRAMP4START,NVSRAM+NVSRAMRES          # Part 4 NVRAM start
        .set    NVSRAMP4SIZ,NVRAMWRITERECORDAREASIZE    # Part 4 NVRAM size

        #   Part 5 NVRAM for Diagnostic Data Area (1968K)
        .set    NVSRAMP5START,NVSRAMP4START+NVSRAMP4SIZ # FE Part 5 start
        .set    NVSRAMP5SIZ,0x1EC000    # Size of Part 5 diagnostic data area

.else   # BACKEND

        #   Part 2 NVRAM for Configuration records (1472K)
        .set    NVSRAMP2START,NVSRAM+NVSRAMRES  # Part 2 NVRAM start addr
        .set    NVSRAMP2SIZ,0x170000            # Part 2 NVRAM size
        .set    NVRP2SSIZ,NVSRAMP2SIZ/SECSIZE # Size of NVRAM part 2 in sectors

        #   Part 3 NVRAM for RAID 5 Write Records in progress at BE
        .set    NVSRAMP3START,NVSRAMP2START+NVSRAMP2SIZ # Part 3 NVRAM start
        .set    NVSRAMP3SIZ,NVRAMWRITERECORDAREASIZE    # Part 3 NVRAM size

        #   Part 4 NVRAM for RAID 5 Write Records from FE P4 Area
        .set    NVSRAMP4START,NVSRAMP3START+NVSRAMP3SIZ # Part 4 NVRAM start
        .set    NVSRAMP4SIZ,NVRAMWRITERECORDAREASIZE    # Part 4 NVRAM size

        #   Part 5 NVRAM for Diagnostic Data Area  (300K)
        .set    NVSRAMP5START,NVSRAMP4START+NVSRAMP4SIZ # Part 5 NVRAM start
        .set    NVSRAMP5SIZ,0x4B000                     # Part 5 NVRAM size

        #   Part 6 NVRAM for Resync Copy Save Area  (64K)
        .set    NVSRAMP6START,NVSRAMP5START+NVSRAMP5SIZ # Part 6 NVRAM start
        .set    NVSRAMP6SIZ,0x10000                     # Part 6 NVRAM size

.endif  # FRONTEND and else BACKEND
        .set    SIZE_OF_PART, NVRAMWRITERECORDAREASIZE # size of part-3 or 4 is 256kB here.
        .set    LOCAL_NVRAM_SIZE, 3*SIZE_OF_PART    # BE p-3 & p-4 and FE p-4
        .set    LOCAL_BE_NVRAMSIZ,2*SIZE_OF_PART
        .set    LOCAL_FE_NVRAMSIZ,SIZE_OF_PART

        .set    NV_ADMIN_START,        0
        .set    NV_ADMIN_SIZE,         0x8000
        .set    MICRO_MEM_BE_P3_START, NV_ADMIN_START+NV_ADMIN_SIZE # BE part-3 is the 2nd block in the NV memory
        .set    MICRO_MEM_BE_P4_START, MICRO_MEM_BE_P3_START+SIZE_OF_PART # BE part-4 is next to BE part-3
        .set    MICRO_MEM_FE_P4_START, MICRO_MEM_BE_P4_START+SIZE_OF_PART # FE part-4 is next to BE part-4
.ifdef LINUX_VER_NVP6_MM
.ifdef  BACKEND
        .set    MICRO_MEM_FE_P4_END,   MICRO_MEM_FE_P4_START+SIZE_OF_PART
        .set    MICRO_MEM_BE_P6_START, MICRO_MEM_FE_P4_END
        .set    NVSRAMP6SIZ,           0x10000      # part 6 NVRAM size;;64K
.endif # BACKEND
.endif # LINUX_VER_NVP6_MM

# Flag to indicate whql is disabled
        .set    DISABLE,0xff

#
#
        .set    SIZE_4MEG,0x00400000    # Present size of flash
#
# --- Device offset for Secondary bus scan - normalize IDSEL position to zero
#     (11 - 31 -> 0 - 20)
#
        .set    devoffset,11
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
