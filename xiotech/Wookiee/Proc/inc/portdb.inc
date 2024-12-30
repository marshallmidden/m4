# $Id: portdb.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: portdb.inc
#
#  PURPOSE:
#
#       To define the data structure for the QLogic Get Port DB data.
#
#  Copyright (c) 1996 - 1998 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- begin structure
#
        .set    pdbopt,0                # Options                    <b>
        .set    pdbcnt,1                # Control                    <b>
        .set    pdbmst,2                # Master State               <b>
        .set    pdbsst,3                # Slave State                <b>
        .set    pdbhad,4                # Hard address from ADISC    <w>
        .set    pdbpid,8                # Port ID                    <w>
        .set    pdbndn,0xC              # Node Name                  <l>
        .set    pdbpdn,0x14             # Port Name                  <l>
        .set    pdbprliw0,0x4c          # prli ser parm word 0       <s>
        .set    pdbprliw3,0x4e          # prli ser parm word 3       <s>
#
# --- end structure
#
        .set    PORTDBSIZ,88            # Size of QLogic FC-AL port database
                                        #  in bytes
        .set    PORTDALLOC,2048*PORTDBSIZ # allocation for port databases
                                        #   for all possible devices on loop
                                        #   Including fabric (upper 128)
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
