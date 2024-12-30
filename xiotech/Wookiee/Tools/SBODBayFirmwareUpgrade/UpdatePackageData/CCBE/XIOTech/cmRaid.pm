# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
#
# Purpose:
#   Wrapper for all the different XIOTech virtual disk commands that
#   can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use strict;

##############################################################################
# Name:     raids
#
# Desc:     Retrieves raid information for all raids.
#
# In:       NONE
#
# Returns:
##############################################################################
sub raids
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["raids"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAIDS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_raidsPacket);
}

##############################################################################
# Name:     raidCount
#
# Desc:     Retrieves the number of raid devices.
#
# Input:    None
#
# Returns:  Number of virtual disks or UNDEF if an error occurred.
##############################################################################
sub raidCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_RAID_COUNT_CMD);
}

##############################################################################
# Name:     raidList
#
# Desc:     Retrieves an array containing the identifiers of the raid devices.
#
# Input:    None
#
# Returns:
##############################################################################
sub raidList
{
    my ($self) = @_;
    return $self->getObjectList(PI_RAID_LIST_CMD);
}

##############################################################################
# Name: raidBeacon
#
# Desc: Beacon Raid Device
#
# In:    Duration
#        Raid Information Hash
##############################################################################
sub raidBeacon
{
    my ($self, $dur, %info) = @_;

    logMsg("begin\n");

    my $i;
    my %rsp;

    for ($i = 0; $i < $info{PSDCNT}; $i++)
    {
        %rsp = $self->physicalDiskBeacon($info{PIDS}[$i]{PID}, $dur);

        if (%rsp)
        {
            if ($rsp{STATUS} == PI_GOOD)
            {
                print "Physical disk ($info{PIDS}[$i]{PID}) is beaconing.\n";
            }
            else
            {
                print "Unable to beacon physical disk ($info{PIDS}[$i]{PID}).";
                $rsp{STATUS} = PI_ERROR;
                return %rsp;
            }
        }
        else
        {
            print "ERROR: Did not receive a response packet.\n";
            $rsp{STATUS} = PI_ERROR;
            return %rsp;
        }
   }
   $rsp{STATUS} = PI_GOOD;
   return %rsp;
   print "\n";
}

##############################################################################
# Name:     virtualDiskRaidInfo
#
# Desc:     Get information about a raid
#
# In:       ID of a raid
#
# Returns:
##############################################################################
sub virtualDiskRaidInfo
{
    my ($self, $rid) = @_;

    my %pdisks;
    my %geoPools;
    my %info;
    my $iPSD;
    my $iPDD;
    my $iPool;
    my $iBay;
    my @bayMap;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualDiskRaidInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    %pdisks = $self->physicalDisks();

    if (!%pdisks || $pdisks{STATUS} != PI_GOOD)
    {
        return undef;
    }

    %geoPools = $self->getGeoPoolInfo();

    if (!%geoPools || $geoPools{STATUS} != PI_GOOD)
    {
        return undef;
    }

    my $cmd = PI_RAID_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $rid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    %info = $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualDiskRaidInfoPacket);

    if (%info && $info{STATUS} == PI_GOOD)
    {
        for ($iPSD = 0; $iPSD < $info{PSDCNT}; $iPSD++)
        {
            for ($iPDD = 0; $iPDD < $pdisks{COUNT}; $iPDD++)
            {
                if ($info{PIDS}[$iPSD]{PID} == $pdisks{PDISKS}[$iPDD]{PD_PID})
                {
                    $info{PIDS}[$iPSD]{SES} = $pdisks{PDISKS}[$iPDD]{SES};
                    last;
                }
            }

            $info{PIDS}[$iPSD]{POOL} = 0xFF;

            for ($iPool = 0; $iPool < 4; $iPool++)
            {
                @bayMap = ParseBitmap($geoPools{GEOPOOL}[$iPool]{BAYBITMAP});

                for ($iBay = 0; $iBay < scalar(@bayMap); $iBay++)
                {
                    if ($info{PIDS}[$iPSD]{SES} == $bayMap[$iBay])
                    {
                        $info{PIDS}[$iPSD]{POOL} = $iPool;
                        last;
                    }
                }
            }
        }
    }

    return %info;
}

##############################################################################
# Name: displayRaids
#
# Desc: Print the raids
#
# In:   Raids Information Hash
##############################################################################
sub displayRaids
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "Raids ($info{COUNT} raids):\n";
    $msg .= sprintf  "\n";

    $msg .= "                                                      INIT  FAIL   MISC      NOT    \n";
    $msg .= " RID  TYPE  SPS  DEVSTAT  ASTAT  VID     CAPACITY     %REM  COUNT  COUNT  MIRRORING \n";
    $msg .= " ---  ----  ---  -------  -----  ---  --------------  ----  -----  -----  ----------\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf " %3hu    %2hu  %3d     0x%2.2x   0x%2.2x  %3d  %14s  %4d  %5d  %5d  0x%8.8x\n", 
                $info{RAIDS}[$i]{RID},
                $info{RAIDS}[$i]{TYPE},
                $info{RAIDS}[$i]{SPS},
                $info{RAIDS}[$i]{DEVSTAT},
                $info{RAIDS}[$i]{ASTATUS},
                $info{RAIDS}[$i]{VID},
                $info{RAIDS}[$i]{CAPACITY},
                $info{RAIDS}[$i]{PCTREM},
                $info{RAIDS}[$i]{FRCNT},
                $info{RAIDS}[$i]{MISCOMP},
                $info{RAIDS}[$i]{NOT_MIRRORING_CSN};
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayVirtualDiskRaidInfo
#
# Desc: Print the raid information
#
# In:   Raid Information Hash
##############################################################################
sub displayVirtualDiskRaidInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my $pool;

    my $msg = "";

    $msg .= sprintf "Raid Information:\n";
    $msg .= sprintf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                   %lu\n", $info{LEN};
    $msg .= sprintf "  RID:                   %hu\n", $info{RID};
    $msg .= sprintf "  TYPE:                  %lu\n", $info{TYPE};
    $msg .= sprintf "  DEVSTAT:               0x%02x\n", $info{DEVSTAT};
    $msg .= sprintf "  DEPTH:                 %hu\n", $info{DEPTH};
    $msg .= sprintf "  PCTREM:                %hu\n", $info{PCTREM};
    $msg .= sprintf "  PSDCNT:                %hu\n", $info{PSDCNT};
    $msg .= sprintf "  SPS:                   %lu\n", $info{SPS};
    $msg .= sprintf "  SPU:                   %lu\n", $info{SPU};
    $msg .= sprintf "  CAPACITY:              $info{CAPACITY}\n";
    $msg .= sprintf "  NVRDD:                 %lu\n", $info{NVRDD};
    $msg .= sprintf "  VID:                   %hu\n", $info{VID};
    $msg .= sprintf "  FRCNT:                 %hu\n", $info{FRCNT};
    $msg .= sprintf "  ERROR:                 %lu\n", $info{ERROR};
    $msg .= sprintf "  QD:                    %lu\n", $info{QD};
    $msg .= sprintf "  RPS:                   %lu\n", $info{RPS};
    $msg .= sprintf "  AVGSC:                 %lu\n", $info{AVGSC};
    $msg .= sprintf "  RREQ:                  $info{RREQ}\n";
    $msg .= sprintf "  WREQ:                  $info{WREQ}\n";
    $msg .= sprintf "  LLSDA:                 $info{LLSDA}\n";
    $msg .= sprintf "  LLEDA:                 $info{LLEDA}\n";
    $msg .= sprintf "  IPROCS:                %lu\n", $info{IPROCS};
    $msg .= sprintf "  IERRORS:               %lu\n", $info{IERRORS};
    $msg .= sprintf "  ISECTORS:              $info{ISECTORS}\n";
    $msg .= sprintf "  MISCOMP:               %lu\n", $info{MISCOMP};
    $msg .= sprintf "  PARDRV:                %lu\n", $info{PARDRV};
    $msg .= sprintf "  DEFLOCK                %lu\n", $info{DEFLOCK};
    $msg .= sprintf "  ASTATUS                0x%02x\n", $info{ASTATUS};
    $msg .= sprintf "  R5SROUT:               %lu\n", $info{R5SROUT};
    $msg .= sprintf "  NOT_MIRRORING_CSN:     0x%8.8x\n", $info{NOT_MIRRORING_CSN};
    $msg .= sprintf "  SPRC:                  %lu\n", $info{SPRC};
    $msg .= sprintf "  SPSC:                  %lu\n", $info{SPSC};
    $msg .= sprintf "  RPNHEAD:               %lu\n", $info{RPNHEAD};

    $msg .= sprintf "\n";

    $msg .= "  PSD  PID  STATUS  ASTATUS  % COMP   ENCL  POOL\n";
    $msg .= "  ---  ---  ------  -------  ------   ----  ----\n";

    for ($i = 0; $i < $info{PSDCNT}; $i++)
    {
        if ($info{PIDS}[$i]{POOL} == 0xFF)
        {
            $pool = "-";
        }
        else
        {
            $pool = sprintf "%d", $info{PIDS}[$i]{POOL};
        }

        $msg .= sprintf "  %3hu  %3hu    0x%02x     0x%02x  %6d  %4d  %4s\n",
                $i,
                $info{PIDS}[$i]{PID},
                $info{PIDS}[$i]{PSD_STATUS},
                $info{PIDS}[$i]{PSD_ASTATUS},
                $info{PIDS}[$i]{PSD_RPC},
                $info{PIDS}[$i]{SES},
                $pool;
    }

    $msg .= sprintf "\n";

    return $msg;
    
}

##############################################################################
# Name:     raidRecover
#
# Desc:     Recover an inoperative raid
#
# In:       raid ID 
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub raidRecover
{
    my ($self, $rid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["raidRecover"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAID_RECOVER_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $rid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _raidsPacket
#
# Desc:     Parses the raids packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _raidsPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_RAIDS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @raids;

        my $start = 4;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;
            my %llsda;
            my %lleda;
            my %isectors;

            # Unpack the data
            (
            $rsvd,
            $raids[$i]{STATUS_MRP},
            $raids[$i]{LEN},
            $raids[$i]{RID},
            $raids[$i]{TYPE},
            $raids[$i]{DEVSTAT},
            $raids[$i]{DEPTH},
            $raids[$i]{PCTREM},
            $raids[$i]{PSDCNT},
            $raids[$i]{SPS},
            $raids[$i]{SPU},
            $capacity{LO_LONG}, $capacity{HI_LONG},
            $raids[$i]{NVRDD},
            $raids[$i]{VID},
            $raids[$i]{FRCNT},
            $raids[$i]{ERROR},
            $raids[$i]{QD},
            $raids[$i]{RPS},
            $raids[$i]{AVGSC},
            $rreq{LO_LONG}, $rreq{HI_LONG},
            $wreq{LO_LONG}, $wreq{HI_LONG},
            $llsda{LO_LONG}, $llsda{HI_LONG},
            $lleda{LO_LONG}, $lleda{HI_LONG},
            $raids[$i]{IPROCS},
            $raids[$i]{IERRORS},
            $isectors{LO_LONG}, $isectors{HI_LONG},
            $raids[$i]{MISCOMP},
            $raids[$i]{PARDRV},
            $raids[$i]{DEFLOCK},
            $raids[$i]{ASTATUS},
            $raids[$i]{R5SROUT},
            $raids[$i]{NOT_MIRRORING_CSN},
            $raids[$i]{SPRC},
            $raids[$i]{SPSC},
            $raids[$i]{RPNHEAD}
            ) = unpack("a3CLSCCCCSLLLLLSSLLLLLLLLLLLLLLLLLLSCCLLLL", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $raids[$i]{CAPACITY} = longsToBigInt(%capacity);
            $raids[$i]{RREQ} = longsToBigInt(%rreq);
            $raids[$i]{WREQ} = longsToBigInt(%wreq);
            $raids[$i]{LLSDA} = longsToBigInt(%llsda);
            $raids[$i]{LLEDA} = longsToBigInt(%lleda);
            $raids[$i]{ISECTORS} = longsToBigInt(%isectors);

            $start += 132;

            my @pids;
            for (my $j = 0; $j < $raids[$i]{PSDCNT}; $j++)
            {
                (
                $pids[$j]{PID},
                $pids[$j]{PSD_STATUS},
                $pids[$j]{PSD_RPC},
                $pids[$j]{PSD_ASTATUS},
                $rsvd,
                ) = unpack("SCCCa3", substr($parts{DATA}, $start, MIRXSIZ));

                $start += MIRXSIZ;
            }

            $raids[$i]{PIDS} = [@pids];
        }

        $info{RAIDS} = [@raids];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a raids packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _virtualDiskRaidInfoPacket
#
# Desc:     Parses the raid info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualDiskRaidInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_RAID_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %capacity;
        my %rreq;
        my %wreq;
        my %llsda;
        my %lleda;
        my %isectors;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},

        $info{RID},
        $info{TYPE},
        $info{DEVSTAT},
        $info{DEPTH},
        $info{PCTREM},
        $info{PSDCNT},

        $info{SPS},
        $info{SPU},

        $capacity{LO_LONG}, $capacity{HI_LONG},

        $info{NVRDD},
        $info{VID},
        $info{FRCNT},

        $info{ERROR},
        $info{QD},

        $info{RPS},
        $info{AVGSC},

        $rreq{LO_LONG}, $rreq{HI_LONG},

        $wreq{LO_LONG}, $wreq{HI_LONG},

        $llsda{LO_LONG}, $llsda{HI_LONG},

        $lleda{LO_LONG}, $lleda{HI_LONG},

        $info{IPROCS},
        $info{IERRORS},

        $isectors{LO_LONG}, $isectors{HI_LONG},

        $info{MISCOMP},
        $info{PARDRV},

        $info{DEFLOCK},
        $info{ASTATUS},
        $info{R5SROUT},
        $info{NOT_MIRRORING_CSN},

        $info{SPRC},
        $info{SPSC},

        $info{RPNHEAD}

        ) = unpack("a3CL SCCCCS LL LL LSS LL LL LL LL LL LL LL LL LL SCCL LL L", $parts{DATA});

        my $pid = substr($parts{DATA}, 132);
        my @pids;

        logMsg("PSDCNT: " . $info{PSDCNT} . "\n");

        for ($i = 0; $i < $info{PSDCNT}; $i++)
        {
            my $start = MIRXSIZ * $i;

            (
            $pids[$i]{PID},
            $pids[$i]{PSD_STATUS},
            $pids[$i]{PSD_RPC},
            $pids[$i]{PSD_ASTATUS},
            $rsvd,
            ) = unpack("SCCCa3", substr($pid, $start));
        }

        $info{PIDS} = [@pids];

        # Now fixup all the 64 bit  numbers
        $info{CAPACITY} = longsToBigInt(%capacity);
        $info{RREQ} = longsToBigInt(%rreq);
        $info{WREQ} = longsToBigInt(%wreq);
        $info{LLSDA} = longsToBigInt(%llsda);
        $info{LLEDA} = longsToBigInt(%lleda);
        $info{ISECTORS} = longsToBigInt(%isectors);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual disk info packet\n");
    }

    return %info;
}

1;

##############################################################################
# Change log:
# $Log$
# Revision 1.2  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.1  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.19  2004/08/25 16:14:18  WilliamsJ
# TBolt00000000 - Corrected rebuild percent label.  Reviewed by Chris.
#
# Revision 1.18  2004/03/08 22:17:15  RysavyR
# TBolt00010199: Add MRP - MRRAIDRECOVER to recover an inoperative raid.
# Rev by Tim Swatosh..
#
# Revision 1.17  2004/03/01 14:16:34  NigburC
# TBolt00000000 - Added the NOT MIRRORING field to the RAIDS display.
# Reivewed by Mark Jurgensen.
#
# Revision 1.16  2003/09/09 21:02:55  NigburC
# TBolt00008575 - Added the new fields in the RAID to the RAIDINFO display
# information.
# Reviewed by Jim Snead.
#
# Revision 1.15  2003/08/05 18:58:57  SchibillaM
# TBolt00008793: Complete GeoPool support in CCB and CCBE.
# Reviewed by Randy.
#
# Revision 1.14  2003/08/01 15:09:48  NigburC
# TBolt00000000 - Added code to display the enclosure and geoPool for the
# PSD in a raid when displaying the raid information.
# Reviewed by Jeff Williams.
#
# Revision 1.13  2003/06/03 19:46:18  MenningC
# TBOLT00000000: Changed many of the 'display' functions to fill a string rather than print to the screen. This supports use by the test scripts. Reviewed by Randy R.
#
# Revision 1.12  2003/05/05 21:33:50  TeskeJ
# tbolt00008227 - scrubbing changes
# rev by Bryan
#
# Revision 1.11  2003/04/15 16:43:27  TeskeJ
# tbolt00007720 - added rebuild % in raidinfo command and format for pdiskinfo
# tbolt00008060 - added SPS in 'raids' command
# rev by Tim
#
# Revision 1.10  2003/03/04 17:15:46  TeskeJ
# tbolt00006866 - CCBE changes to display PSD# along with PID & astatus
# rev by Tim
#
# Revision 1.9  2002/11/14 15:20:03  SchibillaM
# TBolt00004962: Add support for Loop Primitive MRP.  This function is useful
# for validation testing.
#
# Revision 1.8  2002/11/11 22:05:45  TeskeJ
# tbolt00000000 - raid heading for %REM is just initialization %REM
# rev by Chris
#
# Revision 1.7  2002/10/31 20:48:05  HoltyB
# TBolt00006238:  Fixed displayRaidInfo - PCTREM.
#
# Revision 1.6  2002/07/17 13:08:45  ThiemannE
# Tbolt00004640: Separated PSD rebuild percent complete and PSD astat fields in Get RAID Device Info MRP
# Reviewed by Mark Schibilla.
#
# Revision 1.5  2002/04/16 14:30:53  NigburC
# TBolt00003594 - Added VDISKS, RAIDS and PORTLIST commands.
# Modified the DEVSTAT calls to use the new VDISKS and RAIDS commands
# to make them faster.
#
# Revision 1.4  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.3  2002/01/09 13:37:11  HoltyB
# Added virtualDiskRaidInfo , _virtualDiskRaidInfoPacket, and
# displayVirtualDiskRaidInfo
#
# Added raidBeacon to beacon all physical disks in the raid device
#
# Revision 1.2  2001/12/20 22:33:53  NigburC
# Modified the CCBCL to handle the checking for active connection in the
# big switch statement instead of in each of the command handlers.  This way
# we could use the code from just one area and print the same message in the
# case a command needs an active connection and one is not available.
#
# Added getObjectCount and getObjectList functions to the CCBE and now
# use them for all count and list operations.  These functions now match the
# changes to the packets where LIST no longer takes any input and returns
# a count value along with the list of values.
#
# Revision 1.1  2001/12/07 17:11:04  NigburC
# Added VLINK commands.
# Added DEVSTATUS command.
# Added RAIDCOUNT and RAIDLIST commands.
# Reverted the byte swapping done on capacity and count 64 bit integer values
# since they really did not need to be swapped.  Only WWNs should be
# swapped.
# Fixed other bugs found during debugging.
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
