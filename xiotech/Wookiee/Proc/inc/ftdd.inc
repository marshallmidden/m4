# $Id: ftdd.inc 159730 2012-08-27 22:03:32Z marshall_midden $
#*****************************************************************************
#
#  NAME: ftdd.inc  (Foreign Target Device Database)
#
#  DESCRIPTION:
#       This module defines the Foreign Target Device Database used to pass
#       information about the devices located on an identified Foreign
#       Target between the Link-level driver and the Data-link manager.
#
#  Copyright (c) 1999-2001 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- FTDD header data structure ---------------------------------------------
        .set    ftdd_luns,0             # # LUNs defined                  <b>
        .set    ftdd_lunrecs,ftdd_luns+1 # # LUN records defined          <b>
        .set    ftdd_rsize,ftdd_lunrecs+1 # LUN record size               <b>
# --- Reserved 1
        .set    ftdd_hdr,ftdd_rsize+2   # size of FTDD header
# --- End of FTDD header data structure -------------------------------------
#
# --- LUN record data structure ---------------------------------------------
        .set    ftdd_lnum,0             # LUN #                           <b>
        .set    ftdd_dtype,ftdd_lnum+1  # Device type code                <b>
        .set    ftdd_snlen,ftdd_dtype+1 # Serial # length                 <b>
# --- Reserved 3                                                         3<b>
        .set    ftdd_secsz,ftdd_snlen+4 # Sector size                     <s>
        .set    ftdd_size,ftdd_secsz+2  # Disk size (# sectors)           <l>
        .set    ftdd_sn,ftdd_size+8     # LUN serial number             16<b>
        .set    ftdd_recsz,ftdd_sn+16   # size of LUN record
# --- End of LUN record data structure --------------------------------------
#
# --- Equates ---------------------------------------------------------------
        .set    ftdd_maxluns,LUNMAX     # max. # LUNs supported
        .set    ftdd_alloc,ftdd_hdr+(ftdd_recsz*ftdd_maxluns)
                                        # memory allocation for FTDD
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
