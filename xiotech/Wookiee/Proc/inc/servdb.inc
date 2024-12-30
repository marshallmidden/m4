# $Id: servdb.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: servdb.inc
#
#  PURPOSE:
#
#       To define the data structure necessary for mapping FC-AL IEEE port names
#       to a Server name and a unique Server ordinal.
#
#  Copyright (c) 1996 - 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
#
# --- Equates
#
#
        .set    SERVDBALLOC,MAXDEV*srvdblen # Size of Server database table
#
# --- Begin Port ID/Server Name table structure -------------------------------
#
        .set    ieeeaddr,0              # IEEE address location      <8b>
#
# --- End structure
#
        .set    srvdblen,ieeeaddr+8     # sizeof a table entry
        .set    srvdbshift,3            # shift ordinal by 3 to get offset
                                        #   into Port ID/Server Name table
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
