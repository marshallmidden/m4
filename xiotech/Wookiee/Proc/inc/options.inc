# $Id: options.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: options.inc (Conditional assembly equates and options)
#
#  DESCRIPTION:
#
#               This file contains the toggles for many of the conditional
#               assembly options.
#
#
#  Copyright (c) 1996 - 2005 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
        .set    FALSE,0
        .set    TRUE,1
#
# --- Software Detected Fault Options -----------------------------------------
#
        .set    SFTDEBUG,TRUE           # Software Detected Fault Debug
                                        #   FALSE = Firmware Alert Log Event
                                        #   TRUE = Error Trap
        .set    LINUX_VER_NVP6_MM,TRUE

        .set    LINUX_VER_NEWMP,TRUE    # To Enable the code for MP Info MRPs
#        .set    DEBUG_NEWMP,TRUE       # To Enable the Debug prints for MP Info MRPs

#       .set    LINUX_R5DEBUG,TRUE
#
# --- Error Trap Fault Options -----------------------------------------
#
        .set    ERRTRAP_DEBUG,TRUE      # How to handle jumps to .errtrap
                                        #   FALSE = Jump to boot code on errtrap
                                        #   TRUE = Stay in proc code
#
# --- Kernel options ----------------------------------------------------------
#
        .set    KDEBUG,TRUE             # Enable debug code
        .set    KDEBUG1,TRUE            # Enable electric fence
        .set    KDEBUG3,TRUE            # Enable memory check on context switch
        .set    KDEBUG4,TRUE            # Enable FMM Options to check free
                                        #   memory on large memory regions:
                                        #       Write Cache Buffer
#
# --- Flight recorder - see fr.inc for description
#
        .set    DEBUG_FLIGHTREC,TRUE          # Enable flight recorder basics
        .set    fr_asize,fr_size*4000         # sizeof allocated flight recorder
#
.ifdef PERF
        .set    DEBUG_FLIGHTREC_D,TRUE        # Define
        .set    DEBUG_FLIGHTREC_O,TRUE        # Online
        .set    DEBUG_FLIGHTREC_OHSPARE,TRUE  # Hot spare
#
        .set    DEBUG_FLIGHTREC_CD,FALSE      # Cdriver
        .set    DEBUG_FLIGHTREC_C,FALSE       # Cache
        .set    DEBUG_FLIGHTREC_L,FALSE       # Link layer
        .set    DEBUG_FLIGHTREC_V,FALSE       # Virtual layer
        .set    DEBUG_FLIGHTREC_R,FALSE       # RAID layer
        .set    DEBUG_FLIGHTREC_DLM,FALSE     # DLM layer
        .set    DEBUG_FLIGHTREC_P,FALSE       # Physical layer
        .set    DEBUG_FLIGHTREC_PD,TRUE       # Physical discovery layer
        .set    DEBUG_FLIGHTREC_PR,TRUE       # Physical recovery
        .set    DEBUG_FLIGHTREC_I,FALSE       # ISP layer
        .set    DEBUG_FLIGHTREC_M,FALSE       # Misc layer
#
        .set    DEBUG_FLIGHTREC_MEMORY,FALSE  # Malloc and mrel tracing
        .set    DEBUG_FLIGHTREC_XCHANG,FALSE  # Kernel context switch timing
        .set    DEBUG_FLIGHTREC_TIME,FALSE    # Time stamping
#
# --- These enable tracing for the particular feature or layer
#
.else
        .set    DEBUG_FLIGHTREC_D,FALSE        # Define
        .set    DEBUG_FLIGHTREC_O,TRUE        # Online
        .set    DEBUG_FLIGHTREC_OHSPARE,TRUE  # Hot spare
#
        .set    DEBUG_FLIGHTREC_CD,FALSE       # Cdriver
        .set    DEBUG_FLIGHTREC_C,FALSE        # Cache
        .set    DEBUG_FLIGHTREC_L,FALSE        # Link layer
        .set    DEBUG_FLIGHTREC_V,FALSE        # Virtual layer
        .set    DEBUG_FLIGHTREC_R,FALSE        # RAID layer
        .set    DEBUG_FLIGHTREC_DLM,FALSE      # DLM layer
        .set    DEBUG_FLIGHTREC_P,FALSE        # Physical layer
        .set    DEBUG_FLIGHTREC_PD,TRUE       # Physical discovery layer
        .set    DEBUG_FLIGHTREC_PR,TRUE       # Physical recovery
        .set    DEBUG_FLIGHTREC_I,FALSE        # ISP layer
        .set    DEBUG_FLIGHTREC_M,FALSE        # Misc layer
#
        .set    DEBUG_FLIGHTREC_MEMORY,FALSE   # Malloc and mrel tracing
        .set    DEBUG_FLIGHTREC_XCHANG,FALSE  # Kernel context switch timing -- CAREFUL!
        .set    DEBUG_FLIGHTREC_TIME,FALSE    # Time stamping
#
.endif
#
# --- Cache Options -----------------------------------------------------------
#
        .set    WC_ENABLE,TRUE          # FALSE = Write Cache will Not Enable
                                        # TRUE  = WC can be enabled by user
#
        .set    rbidebug,FALSE          # RBI Debug code
        .set    rbidebug_NodeMax,FALSE  # RBI Node Max Debug
#
        .set    WC_MIRROR_ERROR_DISABLE,FALSE # TRUE = Disable Errors being seen
                                             #      on Mirror Operations
                                             # FALSE = Enable Errors being seen
                                             #      on Mirror Operations
#
# --- RAID Options ------------------------------------------------------------
#
        .set    CHKDEBUG,TRUE           # Checker process debug option
#
# --- Initiator Options -------------------------------------------------------
#
.ifdef  BACKEND
        .set    MULTI_ID,FALSE          # Define multi-target support
        .set    INITIATOR,TRUE          # Initiator code always enabled on BE
        .set    MAG2MAG,TRUE            # MAGNITUDE to MAGNITUDE support
        .set    ITRACES,FALSE           # Always for the Backend
        .set    DLMFE_DRIVER,FALSE      # Disable FE DLM Driver Tester
.else   # FRONTEND

        #***************************************#
        #                                       #
        # WHQL SCSI Complicance                 #
        #                                       #
        # This option changes the configuration #
        # of the inquiry, mode sense, and mode  #
        # select commands.                      #
        #                                       #
        # Caution:                              #
        # Changes to the inquiry command while  #
        # servers are active may cause servers  #
        # to fault.                             #
        #                                       #
        # NOTE:                                 #
        # The WHQL tests can not be run against #
        # an uninitialized RAID 10. The test    #
        # does not issues writes before reading #
        # from the Vdisk.                       #
        #                                       #
        #***************************************#
        .set    WHQL_SCSI_COMPLIANCE,FALSE

        .set    MULTI_ID,TRUE           # Define multi-target support
        .set    INITIATOR,TRUE          # Initiator code for FE

        .if     INITIATOR

        .set    MAG2MAG,TRUE            # TRUE = Enable SAN Links support
        .set    VALIDATE,TRUE           # Validate PSF calls (initiator function)
        .set    ITRACES,FALSE            # Initiator traces
        .set    DLMFE_DRIVER,FALSE      # Enable FE DLM Driver Tester

        .else
#
# do mot modify these values - these REQUIRE initiator support
#
        .set    MAG2MAG,FALSE           # MAGNITUDE to MAGNITUDE support
        .set    VALIDATE,FALSE          # Validate PSF calls
        .set    ITRACES,FALSE           # Enable Initiator traces
        .set    debug_tracestop2,FALSE  # disable trace when path is lost to self
        .set    debug_tracestop3,FALSE  # disable trace when there are no more alpa'a
        .set    debug_tracestop4,FALSE  # disable traces when any target is lost
        .set    debug_tracestop5,FALSE  # disable traces on port unavailable
        .set    DLMFE_DRIVER,FALSE      # Disable FE DLM Driver Tester

        .endif

        .if     ITRACES
        .set    debug_tracestop2,FALSE  # disable trace when path is lost to self
        .set    debug_tracestop3,FALSE  # disable trace when there are no more alpa'a
        .set    debug_tracestop4,FALSE  # disable traces when any target is lost
        .set    debug_tracestop5,FALSE  # disable traces on port unavailable
        .else
        .set    debug_tracestop2,FALSE  # disable trace when path is lost to self
        .set    debug_tracestop3,FALSE  # disable trace when there are no more alpa'a
        .set    debug_tracestop4,FALSE  # disable traces when any target is lost
        .set    debug_tracestop5,FALSE  # disable traces on port unavailable
        .endif

.endif
#
# --- DLM module conditional assembly equates ---------------------------------
#
        .set    MAGNITUDE,FALSE
#
# --- Online processing.
#
        .set    NO_DELAY_ONLINE,FALSE   # Do not wait for CCB in online processing
        .set    DRIVE_POLLING,TRUE      # Periodically poll drives
#
# --- MAGDRVR/CDRIVER  processing.
#
        .set    CDRIVER_MSGS,TRUE
#
# --- MAGDRVR/CDRIVER module trace definitions --------------------------------
#     This saves code trace data by channel #. The CIMT has pointers to the
#     trace area. See traces.inc
#
.ifndef PERF
        .set    TRACES,TRUE             # traces
.endif # PERF
        .set    DEF_TFLG,0x069e         # default trace flag setting
#
# --- MAGDRVR/CDRIVER module error log definitions ----------------------------
#
        .set    ERRLOG,FALSE            # error log
        .set    TRACE_EL,FALSE          # save traces in error log record
        .set    DEF_EFLG,0x0000000f     # default error log flag setting
#
# --- Physical Options -------------------------------------------------------------
#
        .set    PHY_LOG_RETRY,TRUE
#
# --- ISP Options -------------------------------------------------------------
#
        .set    ISP_CP_DESC_LID,TRUE    # Control port descending Loop ID search
        .set    ISP_DEBUG,TRUE
        .set    ISP_RESET_ERRTRAP,TRUE # Enables errtrap on port reset
        .set    ISP_INIT_MSG,TRUE       # Log msg when port init completes ok
.ifdef  BACKEND
        .set    ISP_ERROR_INJECT,TRUE   # Enable Error Inject feature for BE
.else
        .set    ISP_ERROR_INJECT,FALSE  # Disable Error Inject feature for FE
.endif
        .set    ISP_GAN_DEBUG,TRUE      # GAN (SNS) debug in ISP & Idriver
        .set    ISP_RESET_FAIL_DEBUG,TRUE # ISP Chip Reset Failure debug

#
# --- Heartbeat Tracing Options -----------------------------------------------
#
        .set    DEBUG_HBEATREC,TRUE     # Enable heart beat tracing
        .set    hbt_asize,16*1024       # sizeof allocated heartbeat recorder
#
# --- Doug K. & Larry D. Options ----------------------------------------------
#
# --- Special INQUIRY data format option flags
#
        .set    spec_INQ_new,TRUE       # New style special INQUIRY data
                                        #  format
#
        .if     spec_INQ_new
#
        .set    spec_INQ_old,FALSE      # Old style special INQUIRY data format
#
        .else
#
        .set    spec_INQ_old,TRUE       # Old style special INQUIRY data format
#
        .endif
#
# --- End Special INQUIRY data format option flags
#
# --- Start Performance Build, flags to be disabled
#
.ifdef PERF
        .set    SFTDEBUG,FALSE          # Software Detected Fault Debug
#
        .set    ERRTRAP_DEBUG,FALSE     # How to handle jumps to .errtrap
#
        .set    KDEBUG,FALSE            # Disable debug code
        .set    KDEBUG1,FALSE           # Disable electric fence
        .set    KDEBUG3,FALSE           # Disable memory check on context switch
        .set    KDEBUG4,FALSE           # Disable FMM Options to check free mem
#
        .set    CHKDEBUG,FALSE          # Checker process debug option
        .set    ITRACES,FALSE           # Disable Initiator traces
        .set    CDRIVER_MSGS,FALSE
        .set    PHY_LOG_RETRY,FALSE
        .set    ISP_DEBUG,FALSE
        .set    ISP_RESET_ERRTRAP,FALSE # Disable errtrap on port reset
        .set    ISP_ERROR_INJECT,FALSE  # Disable Error Inject feature
        .set    ISP_GAN_DEBUG,FALSE     # GAN (SNS) debug in ISP & Idriver
        .set    ISP_RESET_FAIL_DEBUG,FALSE # ISP Chip Reset Failure debug
        .set    DEBUG_HBEATREC,FALSE    # Disable heart beat tracing
.endif
#
# --- End Performance Build, flags to be disabled
#
#
#
# --- Kernel options ----------------------------------------------------------
#
.if KDEBUG1
        .set    EFSZ,MGRAN+1            # Electric fence size
.endif # KDEBUG1
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
