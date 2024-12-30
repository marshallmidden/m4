# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile: CmdCodeHashes.pm
# Author: Randy Rysavy
#
# Purpose: Build the various Command Code hashes and return to caller in
#          one large, all encompassing hash.
# 
# IMPORTANT NOTE: "CmdCodeHashes.pm" is an AUTO-GENERATED FILE.  DO NOT EDIT
# THAT FILE OR THE CHANGES YOU MAKE WILL BE OVERWRITTEN THE NEXT TIME IT IS
# GENERATED.  Instead, make your changes to CmdCodeHashes.tpl, as those
# changes then get reflected in the generated file.
#   
##############################################################################
package XIOTech::CmdCodeHashes;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    BuildCmdCodeHashTables
);

use strict;

sub BuildCmdCodeHashTables 
{
    my %AllHashes;


    #====================================================================
    #                      M R P ' s
    #====================================================================
    my %mrpIdH;
    my %mrpDataH;

    #
    # MRP id description table
    #
    $mrpIdH{ 99 }  = "MRP Start";
    $mrpIdH{ 100 } = "MRP TOCallBk";
    $mrpIdH{ 0 }   = "MRP Good";
    $mrpIdH{ 1 }   = "MRP Error";
    $mrpIdH{ 2 }   = "MRP InProg";
    $mrpIdH{ 3 }   = "MRP N/A (?)";
    $mrpIdH{ 4 }   = "MRP TimeOut";
    $mrpIdH{ 5 }   = "MRP InvCmd";
    $mrpIdH{ 6 }   = "MRP MallocEr";
    $mrpIdH{ 7 }   = "MRP ParmVEr";
    $mrpIdH{ 8 }   = "MRP NotMastr";
    $mrpIdH{ 9 }   = "MRP PwrUpNC";
    $mrpIdH{ 10 }  = "MRP ElInProg";
    $mrpIdH{ 11 }  = "MRP TunnelEr";
    $mrpIdH{ 12 }  = "MRP R5Resync";
    $mrpIdH{ 128 } = "MRPQ BE Free";
    $mrpIdH{ 129 } = "MRPQ BE M Bl";
    $mrpIdH{ 130 } = "MRPQ BE F Bl";
    $mrpIdH{ 131 } = "MRPQ BE A Bl";
    $mrpIdH{ 256 } = "MRPQ FE Free";
    $mrpIdH{ 257 } = "MRPQ FE M Bl";
    $mrpIdH{ 512 } = "MRPG Start";
    $mrpIdH{ 513 } = "MRPG Finish";

    #
    # MRP data description table
    # 
    ###MRP_DATA###

    # Add these hashes to the main hash
    $AllHashes{MRP_ID} = \%mrpIdH;
    $AllHashes{MRP_DATA} = \%mrpDataH;




    #====================================================================
    #                      Packet ID's
    #====================================================================
    my %pktIdH;
    my %pktDataH;

    #
    # Packet id description table
    #
    $pktIdH{ 99 } = "Pkt Start";
    $pktIdH{ 0 }  = "Pkt Good";
    $pktIdH{ 1 }  = "Pkt Error";
    $pktIdH{ 2 }  = "Pkt InProg";
    $pktIdH{ 3 }  = "Pkt N/A (?)";
    $pktIdH{ 4 }  = "Pkt TimeOut";
    $pktIdH{ 5 }  = "Pkt InvCmd";
    $pktIdH{ 6 }  = "Pkt MallocEr";
    $mrpIdH{ 7 }  = "Pkt ParmVEr";
    $mrpIdH{ 8 }  = "Pkt NotMastr";
    $mrpIdH{ 9 }  = "Pkt PwrUpNC";
    $mrpIdH{ 10 } = "Pkt ElInProg";
    $mrpIdH{ 11 } = "Pkt TunnelEr";
    $mrpIdH{ 12 } = "Pkt R5Resync";

    #
    # Packet data description table
    #
    ###PI_DATA###

    # Add these hashes to the main hash
    $AllHashes{PI_ID} = \%pktIdH;
    $AllHashes{PI_DATA} = \%pktDataH;




    #====================================================================
    #                      I P C
    #====================================================================
    my %ipcIdH;
    my %ipcDataH;

    #
    # IPC id description table
    #
    $ipcIdH{ 99 } = "IPC Start";
    $ipcIdH{ 98 } = "IPC Callback";
    $ipcIdH{ 97 } = "IPC DspStart";
    $ipcIdH{ 96 } = "IPC DspDone";
    $ipcIdH{ 95 } = "IPC DspNull";
    $ipcIdH{ 94 } = "IPC DspTunSt";
    $ipcIdH{ 93 } = "IPC TunStart";
    $ipcIdH{ 0 }  = "IPC TimeOut";
    $ipcIdH{ 1 }  = "IPC NoPath";
    $ipcIdH{ 2 }  = "IPC AnyPath";
    $ipcIdH{ 3 }  = "IPC Ethernet";
    $ipcIdH{ 4 }  = "IPC Fibre";
    $ipcIdH{ 5 }  = "IPC Quorum";

    #
    # Packet data description table
    #
    ###IPC_DATA###

    # Add these hashes to the main hash
    $AllHashes{IPC_ID} = \%ipcIdH;
    $AllHashes{IPC_DATA} = \%ipcDataH;




    #====================================================================
    #                      Logs
    #====================================================================
    my %logIdH;
    my %logDataH;

    #
    # Log Message id description table
    #							 
    $logIdH{ 0 } = "Log Event";

    #
    # Log Message data description table
    #							 
    ###LOG_DATA###

    # Add these hashes to the main hash
    $AllHashes{LOG_ID} = \%logIdH;
    $AllHashes{LOG_DATA} = \%logDataH;





    #====================================================================
    #                      X1 Packets
    #====================================================================
    my %x1IdH;
    my %x1DataH;

    #
    # X1 id description table
    #							 
    $x1IdH{ 99 } = "X1  Start";
    $x1IdH{ 0 }  = "X1  Good";

    #
    # X1 data description table
    #							 
    ###X1_DATA###
    
    # Add these hashes to the main hash
    $AllHashes{X1_ID} = \%x1IdH;
    $AllHashes{X1_DATA} = \%x1DataH;




    #====================================================================
    #                      X1 VDisk Cfg Packets
    #====================================================================
    my %x1VdcIdH;
    my %x1VdcDataH;

    #
    # X1 VDC id description table
    #							 
    $x1VdcIdH{ 99 } = "X1VDC Start";
    $x1VdcIdH{ 0 }  = "X1VDC Good";

    #
    # X1 VDC data description table
    #							 
    ###X1_VDC_DATA###
    
    # Add these hashes to the main hash
    $AllHashes{X1_VDC_ID} = \%x1VdcIdH;
    $AllHashes{X1_VDC_DATA} = \%x1VdcDataH;




    #====================================================================
    #                      X1 BF Passthru Packets
    #====================================================================
    my %x1BFIdH;

    #
    # X1 BF id description table
    #							 
    $x1BFIdH{ 99 } = "X1BF Start";
    $x1BFIdH{ 0 }  = "X1BF Good";

    #
    # X1 BF data description table
    #							 
    # Note: uses same data as PI_DATA
    
    # Add these hashes to the main hash
    $AllHashes{X1_BF_ID} = \%x1BFIdH;




    #====================================================================
    #                      RM Data Codes
    #====================================================================
    my %rmIdH;
    my %rmDataH;

    #
    # RM id description table
    #
    $rmIdH{ 0 } = "RM Trace";

    #
    # RM data description table
    #							 
    ###RM_DATA###
    
    # Add these hashes to the main hash
    $AllHashes{RM_ID} = \%rmIdH;
    $AllHashes{RM_DATA} = \%rmDataH;


    #====================================================================

    return %AllHashes;
}

##############################################################################


1;

