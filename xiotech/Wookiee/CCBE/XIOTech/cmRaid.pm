# $Id: cmRaid.pm 75992 2009-02-13 16:33:34Z mdr $
##############################################################################
#   Xiotech Corporation
#   Copyright (c) 2001,2009  Xiotech Corporation. All rights reserved.
# ======================================================================
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
# Name:     raidscache
#
# Desc:     Retrieves raid information for all raids.
#
# In:       NONE
#
# Returns:
##############################################################################
sub raidsCache
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["raidscache"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_RAIDS_FROM_CACHE_CMD;
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
    my %info;
    my $iPSD;
    my $iPDD;
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

    $msg .= "                                                        INIT  FAIL   MISC      NOT    \n";
    $msg .= " RID   TYPE  SPS  DEVSTAT  ASTAT  VID      CAPACITY     %REM  COUNT  COUNT  MIRRORING \n";
    $msg .= " ----  ----  ---  -------  -----  ----  --------------  ----  -----  -----  ----------\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf " %4hu    %2hu  %3d     0x%2.2x   0x%2.2x  %4d  %14s  %4d  %5d  %5d  0x%8.8x\n", 
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
        $pool = "-";

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

    if ((commandCode($recvPacket) == PI_RAIDS_CMD) ||
        (commandCode($recvPacket) == PI_RAIDS_FROM_CACHE_CMD))
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
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
