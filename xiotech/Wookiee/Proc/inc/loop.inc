# $Id: loop.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: loop.inc
#
#  DESCRIPTION:
#
#       To provide definitions for generic FC-AL equates, structures and
#       bit definitions.
#
#  Copyright (c) 1996 - 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
#
# --- Equates -----------------------------------------------------------------
#

#
# --- Structures --------------------------------------------------------------
#

#
# --- Bit Definitions ---------------------------------------------------------
#

#
# --- XIOTECH OUI (IEEE address)
#
        .set    XIOOUI,0x00D0B2
#
# --- World wide names constants
#
#     The world wide names follow this format...
#
#         Node names:  21e0oooo:oossssss  where e = 0 for front end
#                                                   1 for back end
#                                              oooooo = OUI
#                                              ssssss = controller serial number
#
#         Front end Port names:  22c0oooo:oossssss  where c = channel number
#
#         Back end Port names:  23c0oooo:oossssss  where c = channel number
#
        .set    WWNFNode,0x20200000     # MSW of front end node name
        .set    WWNCNode,0x20600000     # MSW of front end control port name
        .set    WWNBNode,0x20A00000     # MSW of back end node name
        .set    WWNENode,0x20E00000     # MSW of disk enclosure node name
        .set    WWNFPort,0x21200000     # MSW of front end port name
        .set    WWNCPort,0x21600000     # MSW of front end control port name
        .set    WWNBPort,0x21A00000     # MSW of back end port name
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
