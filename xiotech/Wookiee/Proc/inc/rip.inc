# $Id: rip.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: rip.inc (Raid Initialization Packet)
#
#  DESCRIPTION:
#
#       Provides a common structure definition for the Raid
#       Initialization Packet
#
#  Copyright (c) 1996-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#       general definitions
#
        .set    RIPMAXACT,8             # maximum number of active raid
                                        #  inits allowed at a time
#
# --- Begin structure -------------------------------------------------
#                                                                      *****
        .set    rip_link,0              # Link to next rip              <w>
        .set    rip_rdd,rip_link+4      # RDD of Raid to get inited     <w>
        .set    rip_psd,rip_rdd+4       # PSD to get inited             <w>
        .set    rip_rid,rip_psd+4       # Raid ID                       <w>
#
        .set    ripsiz,rip_rid+4        # Size of Raid Initialization Packet structure
#
# --- End Raid Initialization Packet data structure -----------------------------------
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
