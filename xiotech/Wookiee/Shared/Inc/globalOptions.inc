# $Id: globalOptions.inc 159129 2012-05-12 06:25:16Z marshall_midden $
    .title  "globalOptions.inc"
#******************************************************************************
# NAME:         globalOptions.inc
#
# PURPOSE:      Conditional assembly equates and options
#               This file contains the toggles for many of the conditional
#               assembly options for the platform.
#
# Copyright (c) 2001-2008  Xiotech Corporation.  All rights reserved.
#******************************************************************************

###############################################################################
# Public defines
###############################################################################
    .set ISCSI_CODE,TRUE
    .set CM_IM_DEBUG,FALSE
    .set GR_GEORAID15_DEBUG,FALSE

.ifdef FRONTEND
    .set FE_ISCSI_CODE,ISCSI_CODE
.else
    .set FE_ISCSI_CODE,FALSE
.endif


.ifdef FRONTEND
.set FE_ICL,FALSE
.set ICL_DEBUG,FALSE  # Enable or Disable Debug messages for ICL port
.else # BACKEND
.set FE_ICL,FALSE
.set ICL_DEBUG,FALSE  # Enable or Disable  Debug messages for ICL port
.endif

.ifndef  MODEL_3000
.ifndef  MODEL_7400
.set DISABLE_WRITE_SAME,1
.endif  # MODEL_7400
.endif  # MODEL_3000

###############################################################################
# Public variables
###############################################################################


###############################################################################
# Public function prototypes
###############################################################################

#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
