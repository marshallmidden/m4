# $Id: fr.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: fr.inc (Flight Recorder)
#
#  DESCRIPTION:
#
#       To define operation types and queueing structure for a flight recorder.
#
#       The flight recorder allows the ILTs and VRPs/RRPs/PRPs be tracked
#       as they move across the layers. Generally these are saved at the
#       execs and completions routines in a layer. The entries are saved in
#       a circular queue.
#
#       Usage:
#       - enable the DEBUG_FLIGHTREC flag in options.inc.
#       - adjust the queue depth if needed in fr.inc (fr_asize).
#       - enable any other DEBUG_FLIGHTREC flags:
#           - MEMORY traces malloc, mrel, and deferred mrel
#           - TIME adds 4 additional words of data described below.
#
#       Saving Data to the queue:
#       - enclose the entry in the DEBUG_FLIGHTREC assembler flag
#       - fill in the fr_parmN fields with the values you want saved
#         - byte 0 of fr_parm0 is the type defined below.
#         - fr_parm1 should be the ILT when appropriate
#         - fr_parm2 should be the VRP/RRP/SRP/PRP when appropriate
#         - fr_parm3 is user defined
#
#       Viewing the data:
#       - data variable fr_queue points to the circular queue pointers
#       - the 3 words of queue pointers are BEGIN, NEXT, BEGIN, and END
#       - the data immediately follows the queue pointers
#           - words 0-3 are parms 0-3
#           If DEBUG_FLIGHTREC_TIME is enabled 4 additional words are added:
#           - word 4 is the timer interrupt tick counter from the Kii struct
#           - word 5 is the incremental time in useconds
#           - word 6-7 are spares
#
#  Copyright (c) 2000-2005 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
#
# --- Flight Recorder Types
#     These go in byte 0 of fr_parm0.
#

        .set    frt_dlm_rrp,    0xe0        # DLM - rrp received
        .set    frt_dlm_msge,   0xe1        #   - message execution
        .set    frt_dlm_dgsend, 0xe2        #   - datagram send
        .set    frt_dlm_qsrp,   0xe3        #   - Queued to BE SRP Executive
        .set    frt_dlm_besrp,  0xe4        #   - BE SRP Executive
        .set    frt_dlm_qdrp,   0xe5        #   - Queue to DRP Executive
        .set    frt_dlm_drp,    0xe6        #   - DRP Executive
        .set    frt_dlm_novlop, 0xe7        #   - No VLOP
        .set    frt_dlm_prec,   0xe8        #   - Starting precedence
        .set    frt_dlm_resync, 0xe9        #   - Resync
        .set    frt_dlm_snddg2, 0xea        #   - Send DG 2 ILT Comp Routine
# Seq #<s>/Func Code<b>/e7<b>; ILT; SRP; DG Stat<b>/EC1<b>/EC2<b>/Retry Cnt<b>

#
#  frt_h_xx codes:
#
#                                     O$hotswap
#  00d0,   0,     0,   0                Start of hot swap task invocation
#  01d0,   0,     0,   0                At top of hot swap
#  02d0,   0/pid, pdd, old dev status   Missing drive
#  03d0,   0,     0,   0                All inquiries completed
#
#                                     O$inquire
#  00d1,   0/pid, pdd, 0                Start of inquire
#  01d1,   0/pid, pdd, pdx addr         Prior to init_drv call
#  02d1,   0/pid, pdd, new status       After init_drv for drive previously operable
#  03d1,   0/pid, pdd, new status       After init_drv for drive previously not operable
#
#                                       o$init_drv
#  00d2,   0/pid, pdd, test type        o$init_drv - start
#  01d2,   0/pid, pdd, status           o$init_drv - end (0|misc|dev|post status)
#
#                                     Misc rebuild and device status
#  00d3, rid/pid, psd, stat|astat       O$hsparepsd - hot spare PSD call
#  01d3,   0,     0,   0,               d$p2update
#  02d3, rid/pid, psd, True/False       rb$canrebuild
#  03d3, rid/pid, psd, answer           RB$canspare - (answer is yes=0/errcode)
#  04d3, caph,   capl, hspare pdd       rb$findhotspare - find hspare call
#  05d3, rid/0,   rdd, rd|rda|0|0       RB_setraidstat - set raid status/astatus
#  06d3, rid/pid, rdd, 0|0|ps|psa       RB_setpsdstat - set PSD status/astatus
#  07d3, rid/0,retstat,rd|rda|0|0       rb$getraiderrorstat - get raid error status
#  08d3, rid/pid, rdd, psd              D_startinitraid - start RAID initialization
#  09d3, rid/pid, pdd, True/False       d$inopchk - inoperable check
#  0ad3, rid/pid, 0,   True/False       D$defchk - active definition check
#  0bd3, failpid, hspid, options        RB$faildev - fail a device MRP
#  0cd3, pNVRSOS, 0,   True/False       N_processSOS
#  0dd3,   0,     0,   0                N_p2update_nvram
#  0ed3, pNVRII,  pPDX,True/False       N_restorenvram
#
#                                     Rebuild
#  00d4, rid/pid, psd, rdd              rb$rebuildpsd - start
#  01d4, rid/pid, psd, rbr              rb$rebuildpsd - rebuild will take place
#  02d4, rid/pid, psd, hspare pid       rb$redirectpsd - redirect PSD
#  03d4, rid/pid, psd, rbr              rb$psd_rebuilder - start of PSD rebuild
#  04d4, rid/pid, psd, rbr              rb$psd_rebuilder - done with PSD rebuild - good
#  05d4, rid/pid, psd, errcode          rb$psd_rebuilder - done with PSD rebuild - all
#  06d4,   0/pid, pdd, rbr              rb$psd_rebuilder - done with PID rebuild
#  07d4, rid/0,   0,   rbr              RB$cancel_rebld - cancel rebuild
#  08d4,     0,   0,    0               rb$SearchForFailedPSDs - start
#  09d4,     0,   0,    0               rb$SearchForFailedPSDs - end

#
#                                     RAID error handler
#  00d5, rid/pid, psd, rd|rda|ps|psa    rb$rerror - end - RAID error detected
#  01d5,   0,       0, start/end        rb$rerror_exec - RAID error exec
#  02d5, rid/0,   rdd, ilt              rb$rerror_exec - RAID error exec - each ILT
#  03d5,  ilt,    prp, #/q/r/s prpstat  rb$rerror - start - RAID error detected
#
        .set    frt_h_hswap0,   0x00d0      # Hotswap
        .set    frt_h_hswap1,   0x01d0
        .set    frt_h_hswap2,   0x02d0
        .set    frt_h_hswap3,   0x03d0

        .set    frt_h_inquire0, 0x00d1      # Inquire
        .set    frt_h_inquire1, 0x01d1
        .set    frt_h_inquire2, 0x02d1
        .set    frt_h_inquire3, 0x03d1

        .set    frt_h_initdrv0, 0x00d2      # Init drive
        .set    frt_h_initdrv1, 0x01d2

        .set    frt_h_misc0,    0x00d3      # Misc functions
        .set    frt_h_misc1,    0x01d3
        .set    frt_h_misc2,    0x02d3
        .set    frt_h_misc3,    0x03d3
        .set    frt_h_misc4,    0x04d3
        .set    frt_h_misc5,    0x05d3
        .set    frt_h_misc6,    0x06d3
        .set    frt_h_misc7,    0x07d3
        .set    frt_h_misc8,    0x08d3
        .set    frt_h_misc9,    0x09d3
        .set    frt_h_misca,    0x0ad3
        .set    frt_h_miscb,    0x0bd3
        .set    frt_h_miscc,    0x0cd3
        .set    frt_h_miscd,    0x0dd3
        .set    frt_h_misce,    0x0ed3
        .set    frt_h_miscf,    0x0fd3
        .set    frt_h_misc10,   0x10d3      # WriteFailedLabel - start
        .set    frt_h_misc11,   0x11d3      # WriteFailedLabel - end

        .set    frt_h_rbld0,    0x00d4      # Rebuild functions
        .set    frt_h_rbld1,    0x01d4
        .set    frt_h_rbld2,    0x02d4
        .set    frt_h_rbld3,    0x03d4
        .set    frt_h_rbld4,    0x04d4
        .set    frt_h_rbld5,    0x05d4
        .set    frt_h_rbld6,    0x06d4
        .set    frt_h_rbld7,    0x07d4
        .set    frt_h_rbld8,    0x08d4      # SearchForFailedPSDs - start
        .set    frt_h_rbld9,    0x09d4      # SearchForFailedPSDs - end
        .set    frt_h_rblda,    0x0Ad4      # Update Raid rebuild write status

        .set    frt_h_rerr0,    0x00d5      # Raid error
        .set    frt_h_rerr1,    0x01d5
        .set    frt_h_rerr2,    0x02d5
        .set    frt_h_rerr3,    0x03d5

        .set    frt_k_malloc,   0xc0        # Kernel - memory allocation
        .set    frt_k_mrel,     0xc1        #   - memory release
        .set    frt_k_dmrel,    0xc2        #   - deferred memory release
        .set    frt_k_xchang,   0xc3        #   - context switch timing

        .set    frt_m_rip,      0xb0        # Misc
        .set    frt_m_aipw,     0xb1        #
        .set    frt_m_ailt,     0xb2        #   - allocate an ILT
        .set    frt_m_rilt,     0xb3        #   - release an ILT

        .set    frt_a_,         0xa0        # AplDriver

        .set    frt_i_,         0x90        # IDriver

        .set    frt_cd_findimt, 0x80        # CDriver/MagDriver

        .set    frt_wc_wrpexec, 0x70        # Write Cache - WRP Executive
        .set    frt_wc_wrpque,  0x71        #  - WRP Queue Function
        .set    frt_wc_tdexec,  0x72        #  - TDis Executive
        .set    frt_wc_vdisktdis, 0x73      #  - VDisk Temporary Disable func
        .set    frt_wc_vddiscmp, 0x74       #  - VDisk Disable Complete

        .set    frt_c_que,      0x60        # Cache - Queued to Cache Overlapped
        .set    frt_c_execb,    0x61        #   - Initial Overlapped Check
        .set    frt_c_qio,      0x62        #   - Queued to I/O Executive
        .set    frt_c_execio,   0x63        #   - I/O Executive
        .set    frt_c_ncr1,     0x64        #   - Non-cached Read Complete 1
        .set    frt_c_ncr2,     0x65        #   - Non-cached Read Complete 2
        .set    frt_c_ncw1,     0x66        #   - Non-cached Write Complete 1
        .set    frt_c_ncw2,     0x67        #   - Non-cached Write Complete 2
        .set    frt_c_nlc,      0x68        #   - Next Level Completion
        .set    frt_c_lower,    0x69        #   - Call lower function
        .set    frt_c_upper,    0x6A        #   - Call Upper function
        .set    frt_c_wdcomp,   0x6B        #   - Write Cache Data complete
        .set    frt_c_iocomp,   0x6C        #   - I/O Complete (or Write Comp)
        .set    frt_c_quedrp,   0x6D        #   - Que to the DRP Executive
        .set    frt_c_drpexec,  0x6E        #   - DRP Executive
        .set    frt_c_drpcomp,  0x6F        #   - DRP Completion


        .set    frt_l_que,      0x50        # Link
        .set    frt_l_execo,    0x51        #   - outbound VRP exec
        .set    frt_l_execi,    0x52        #   - inbound VRP exec
        .set    frt_l_comp,     0x53        #   - completion routine
        .set    frt_l_execc,    0x54        #   - completion exec
        .set    frt_l_sync,     0x55        #   - processor sync

        .set    frt_v_exec,     0x40        # Virtual
        .set    frt_v_vscomp,   0x41
        .set    frt_v_vmcomp,   0x42

        .set    frt_r_exec,     0x30        # Raid - main exec
        .set    frt_r_comp,     0x31        #   - completion
        .set    frt_r_parity,   0x32        #   - parity scan bad
        .set    frt_r_rrb,      0x33        #   - raid 5 insert RRB into RPN
        .set    frt_r_rrbcomp,  0x34        #   - raid 5 complete RRB

        .set    frt_p_exec,     0x20        # Physical
        .set    frt_p_comp,     0x21
        .set    frt_p_cancel,   0x22
        .set    frt_p_join,     0x23
        .set    frt_p_getilt,   0x24
        .set    frt_p_crtdev,   0x1026
        .set    frt_p_fnddev,   0x2026
        .set    frt_p_init_drv, 0x3026
        .set    frt_p_mvdev,    0x4026
        .set    frt_p_rmdev,    0x5026
        .set    frt_p_findalt,  0x6026
        .set    frt_p_purge,    0x7026
        .set    frt_p_detach,   0x9026
        .set    frt_p_requp,    0xA026
        .set    frt_p_reqdown,  0xB026
        .set    frt_p_discovery,0xC026
        .set    frt_p_rescan,   0xD026
        .set    frt_p_rscn,     0xE026
        .set    frt_f_reqdn,    0xE126
        .set    frt_f_gpnid,    0xE226
        .set    frt_f_nst,      0xE326
        .set    frt_f_fablogin, 0xE426
        .set    frt_f_nolid,    0xE526
        .set    frt_f_logout,   0xE726
        .set    frt_f_rscnend,  0xE926
        .set    frt_p_lundisc,  0xF026
        .set    frt_p_retry,    0x27
        .set    frt_p_chk_cond, 0x28
        .set    frt_p_recovery, 0x0029
        .set    frt_p_lipff,    0x1029
        .set    frt_p_login,    0x4729
        .set    frt_p_retry2,   0x002A

        .set    frt_isp_io,     0x10        # ISP
        .set    frt_isp_cdb,    0x11
        .set    frt_isp_buffer, 0x12
        .set    frt_isp_pdbc,   0x13
        .set    frt_isp_thread, 0x14
        .set    frt_isp_unth,   0x15
        .set    frt_isp_marker, 0x16
        .set    frt_isp_vpicb,  0x17

        .set    frt_isp_reset,  0x1F
        .set    frt_isp_vpctrl, 0x101F
        .set    frt_isp_vpidacq,0x301F
        .set    frt_isp_regfc4, 0x9201F
        .set    frt_isp_aen,    0xAE1F
#
# --- Entry Constants
#
        .set    fr_parms,0                  # parms 0-3
                                            #   This must match the number of
                                            #   parms in misc.as.
.if     DEBUG_FLIGHTREC_TIME
        .set    fr_time,fr_parms+16         # absolute time
        .set    fr_xpcb,fr_time+8           # executing PCB
        .set    fr_xpcb_rip,fr_xpcb+4       # latest rip of executing PCB
        .set    fr_size,fr_xpcb_rip+4       # Sizeof a flight recorder entry
.else
        .set    fr_size,fr_parms+16         # Sizeof a flight recorder entry
.endif

#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
