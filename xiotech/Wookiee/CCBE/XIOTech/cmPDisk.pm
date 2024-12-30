# $Id: cmPDisk.pm 162892 2014-03-18 16:45:14Z marshall_midden $
##############################################################################
# Copyright (c) 2001-2008  Xiotech Corporation. All rights reserved.
# ======================================================================
# Author: Anthony Asleson
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the Xiotech SAN system
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
# Name:     physicalDisks
#
# Desc:     Retrieves physical disk information for all physical disks.
#
# In:       NONE
#
# Returns:
##############################################################################
sub physicalDisks
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["physicalDisks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISKS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_physicalDisksPacket);
}


##############################################################################
# Name:     physicalDisksCache
#
# Desc:     Retrieves physical disk information for all physical disks from cache
#
# In:       NONE
#
# Returns:
##############################################################################
sub physicalDisksCache
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["physicalDisksCache"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISKS_FROM_CACHE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_physicalDisksCachePacket);
}


##############################################################################
# Name:     physicalDiskBeacon
#
# Desc:     Beacon a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskBeacon
{
    my ($self, $pid, $dur) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["physicalDiskBeacon"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_BEACON_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
                    $dur);

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
# Name:     physicalDiskCount
#
# Desc:     Retrieves the number of physical disks.
#
# Input:    None
#
# Returns:
##############################################################################
sub physicalDiskCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_PDISK_COUNT_CMD);
}

##############################################################################
# Name:     physicalDiskDefrag
#
# Desc:     Defragement a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskDefrag
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["physicalDiskDefrag"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_DEFRAG_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
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
# Name:     physicalDiskDefragStatus
#
# Desc:     Defragementation status of physical disk(s)
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskDefragStatus
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_PDISK_DEFRAG_STATUS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    
    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_pDiskDefragStatusPacket);
}

##############################################################################
# Name:     physicalDiskFail
#
# Desc:     Fail a physical disk
#
# In:       ID of the physical disk
#           FORCE option
#
# Returns:
##############################################################################
sub physicalDiskFail
{
    my ($self, $id, $hspid, $force) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0F],
                ["physicalDiskFail"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_FAIL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSCCCC",
                    $id,
                    $hspid,
                    $force,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskAutoFailBack
#
# Desc:     Enables/Disables the auto failback feature 
#
# In:          option
#
# Returns:
##############################################################################
sub physicalDiskAutoFailBack
{
    my ($self, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x0F],
                ["physicalDiskAutoFailBack"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $option,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
#                                        \&_statusResponsePacket);
                                        \&_pDiskAutoFailbackPacket);
}
#SERVICEABILITY42


#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskFailBack
#
# Desc:     Failback an used hotspare physical disk
#
# In:       ID of the used hotspare physical disk
#           option
#
# Returns:
##############################################################################
sub physicalDiskFailBack
{
    my ($self, $hspid, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0F],
                ["physicalDiskFailBack"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_FAILBACK_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCCCC",
                    $hspid,
                    $option,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}
#SERVICEABILITY42
 
##############################################################################
# Name:     physicalDiskInfo
#
# Desc:     Get information about a physical disk
#
# In:       ID of the physical disk
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPMAP
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub physicalDiskInfo
{
    my ($self, $pid, $options) = @_;

    logMsg("begin\n");

    my $bNewArgs = 0;
    my @newArgs;

    if (!defined($options))
    {
        $options = 0;
        $bNewArgs = 1;
    }

    if ($bNewArgs == 1)
    {
        push @newArgs, $self;
        push @newArgs, $pid;
        push @newArgs, $options;
        
        @_ =  @newArgs;
    }

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['i'],
                ["physicalDiskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $pid,
                    $options,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_physicalDiskInfoPacket);
}


##############################################################################
# Name:     physicalDiskLabel
#
# Desc:     Label/Unlabel physical disk(s)
#
# In:       scalar $physicaldiskOrdinal
#
# Returns:  1 on success, 0 on error
##############################################################################
sub physicalDiskLabel
{
    my ($self,
        $pids,
        $type) = @_;

    logMsg("begin\n");

    my $rc = 0;

    # verify parameters
    my $args = [['i'],
                ['a'],
                ['d', 0, 0xFF],
                ["physicalDiskLabel"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my @disks = @$pids;
    my $pidcnt = scalar(@disks);
    my %rsp;

    for (my $i = 0; $i < $pidcnt; $i++)
    {
        my $pid = $disks[$i];

        %rsp = $self->physicalDiskLabelEx($pid, $type);

        if (%rsp)
        {
            if ($rsp{STATUS} != PI_GOOD)
            {
                return %rsp;
            }
        }
        else
        {
            $rsp{STATUS} = PI_ERROR;
            return %rsp;
        }
    }

    $rsp{STATUS} = PI_GOOD;
    return %rsp;
}

##############################################################################
# Name:     physicalDiskLabelEx
#
# Desc:     Label/Unlabel a physical disk and set the device name
#
# In:       scalar $physicaldiskOrdinal
#
# Returns:  1 on success, 0 on error
##############################################################################
sub physicalDiskLabelEx
{
    my ($self,
        $pid,
        $type) = @_;

    logMsg("begin\n");

    my $rc = 0;

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["physicalDiskLabelEx"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_LABEL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $pid,
                    $type,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     physicalDiskList
#
# Desc:     Retrieves an array containing the identifiers of the physical disks.
#
# Input:    None
#
# Returns:  List of physical disk identifiers or UNDEF if an error occurred.
##############################################################################
sub physicalDiskList
{
    my ($self) = @_;
    return $self->getObjectList(PI_PDISK_LIST_CMD);
}

##############################################################################
# Name:     physicalDiskRestore
#
# Desc:     Restore a physical disk
#
# In:       ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskRestore
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["physicalDiskRestore"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_UNFAIL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

##############################################################################
# Name:     physicalDiskDelete
#
# Desc:     deletes  a physical disk.
#
#           pid   -  Device ID
#
# Return:   Status Response Hash
##############################################################################
sub physicalDiskDelete
{
    my ($self, $pid) = @_;
    
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["physicalDiskDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_DELETE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCS",
                    DELETE_DISK_DRIVE,
                    0,
                    $pid);

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
# Name:     physicalDiskBypass
#
# Desc:     Bypass a physical disk
#
# In:       pid - ID of the physical disk
#           setting - Bypass setting
#
# Returns:
##############################################################################
sub physicalDiskBypass
{
    my ($self, $ses, $slot, $setting) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xF],
                ["physicalDiskBypass"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISK_BYPASS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $ses,
                    $slot,
                    $setting);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}

#SERVICEABILITY42
##############################################################################
# Name:     physicalDiskSpindown
#
# Desc:     Spin down a physical disk
#
# In:       pid - ID of the physical disk
#
# Returns:
##############################################################################
sub physicalDiskSpindown
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    my $cmd = PI_PDISK_SPINDOWN_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("S",
                    $pid);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}
#SERVICEABILITY42

##############################################################################
# Name:     AsciiHexToBin
#
# Desc:     Converts an ASCII hex string to packed binary data
#
# Input:    data - hex string representing the MRP input data
#           format - byte|short|word  (default: word)
##############################################################################
sub AsciiHexToBin
{
    my ($data, $format) = @_;

    $data =~ s/0x//i;

    if (!defined $data) {
        print "AsciiHexToBin: No input data.\n";
        return undef;
    }

    if (!defined $format) {
        $format = "word";
    }

    if ($format !~ /^byte$|^short$|^word$/i) {
        print "AsciiHexToBin: format incorrect ($format).\n";
        return undef;
    }

    # setup to parse the input data string
    my $cCnt;
    my $cTpl;

    if ($format =~ /^byte$/i) {
        $cCnt = 2;
        $cTpl = "C";
    }
    elsif ($format =~ /^short$/i) {
        $cCnt = 4;
        $cTpl = "S";
    }
    else { # word
        $cCnt = 8;
        $cTpl = "L";
    }

    my @wData;
    my $i;
    my $template = "";
    my $length = length $data;

    if ($length % $cCnt) {
        print "AsciiHexToBin: Input data string length not correct for the\n" .
              " format selected ($format).\n";
        return undef;
    }

    # parse the input data string
    for($i=0; $i<$length; $i+=$cCnt) {
        push @wData, oct("0x" . substr $data, 0, $cCnt);
        $data = substr $data, $cCnt;
        $template .= $cTpl;
    }

    $data = pack $template, @wData;
    return $data;
}
##############################################################################
# Name: displayPhysicalDisks
#
# Desc: Print the physical disks
#
# In:   Physical Disks Information Hash
##############################################################################
sub displayPhysicalDisks
{
    my ($self, $dsptype, %info) = @_;
    my $wrong;
    my $ses;
    my $slot;
    my $tas_ne_las;

    my $msg = "";

    logMsg("begin\n");

    if (uc($dsptype) eq "STD")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID  PORT    DNAME      SES   SLOT  %REM    RBREMAIN          LAS            TAS        HSDNAME     GEOLOCATION\n";
        $msg .=  "  ---  ----  ----------  -----  ----  ----  -------------  -------------  -------------  ----------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
            $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;

            if ($ses != $info{PDISKS}[$i]{SES} or
                $slot != $info{PDISKS}[$i]{SLOT})
            {
                $wrong = "*";
            }
            else
            {
                $wrong = " ";
            }

            if($info{PDISKS}[$i]{LAS} == $info{PDISKS}[$i]{TAS})
            {
                $wrong .= " ";
            }
            else
            {
                $wrong .= "!";
            }

            $msg .= sprintf "%s%3hu  %4d  %10s  %5d  %4d  %4d  %13s  %13s  %13s  %10s      %3d\n",
                    $wrong,
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{PD_CHANNEL},
                    _getString_DNAME($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{PD_DNAME}),
                    $info{PDISKS}[$i]{SES},
                    $info{PDISKS}[$i]{SLOT},
                    $info{PDISKS}[$i]{PCTREM},
                    $info{PDISKS}[$i]{RBREMAIN},
                    $info{PDISKS}[$i]{LAS},
                    $info{PDISKS}[$i]{TAS},
                    _getString_DNAME($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{HSDNAME}),
#GR_GEORAID - Following is added for GEORaid
                    $info{PDISKS}[$i]{GL_ID},
        }
    }
    elsif (uc($dsptype) eq "BB")
    {
        $msg .= sprintf  "PDisks by Bay ($info{COUNT} disks)  CL=0-U,1-D,2-HS,3=N   DT=1-ENT,2-SATA,4=ECON:\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID DT CL PP GEO STATUS   DNAME    SES SLT CAP(MB) LAS(MB) TAS(MB) TO_RBLD/%REM  IOPS  MBPS  HSDNAME  \n";
        $msg .=  "  --- -- -- -- --- ------ ---------- --- --- ------- ------- ------- ------------ ----- ----- ----------\n";

        for (my $j = 0; $j <= 66; $j++)
        {
               if($j == 66)
               {
                  $j = 255;
               }
           for(my $k = 0; $k <= 66; $k++)
           {
             if($k == 66)
             {
               $k = 255;
             }
           for (my $i = 0; $i < $info{COUNT}; $i++)
           {
               if ($j == $info{PDISKS}[$i]{SES})
               {
                 if($k == $info{PDISKS}[$i]{SLOT})
                 {
               $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
               $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;
   
               if ($ses != $info{PDISKS}[$i]{SES} or
                   $slot != $info{PDISKS}[$i]{SLOT})
               {
                   $wrong = "*";
               }
               else
               {
                   $wrong = " ";
               }
   
               if($info{PDISKS}[$i]{LAS} == $info{PDISKS}[$i]{TAS})
               {
                   $wrong .= " ";
               }
               else
               {
                   $wrong .= "!";
               }
   
               $msg .= sprintf "%s%3hu %2hu %2hu %2d %2d  %02x/%02x  %-10s %3d %3d %7s %7s %7s %7s/%4d %5d %5.2f %10s\n",
                       $wrong,
                       $info{PDISKS}[$i]{PD_PID},
                       $info{PDISKS}[$i]{PD_DEVTYPE},
                       $info{PDISKS}[$i]{PD_CLASS},
                       $info{PDISKS}[$i]{PD_CHANNEL},
                       $info{PDISKS}[$i]{GL_ID},
                       $info{PDISKS}[$i]{PD_DEVSTAT},
                       $info{PDISKS}[$i]{PD_POSTSTAT},
                       _getString_DNAME2($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{PD_DNAME}),
                       $info{PDISKS}[$i]{SES},
                       $info{PDISKS}[$i]{SLOT},
                       $info{PDISKS}[$i]{CAPACITY}/2048,
                       $info{PDISKS}[$i]{LAS}/2048,
                       $info{PDISKS}[$i]{TAS}/2048,
                       $info{PDISKS}[$i]{RBREMAIN}/2048,
                       $info{PDISKS}[$i]{PCTREM},
                       $info{PDISKS}[$i]{PD_RPS},
                       $info{PDISKS}[$i]{PD_AVGSC}*$info{PDISKS}[$i]{PD_RPS}/2048,
                       _getString_DNAME2($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{HSDNAME}),
               }
               }
               }
           }
        }
    }
    elsif (uc($dsptype) eq "SMART")
    {
        $msg .= sprintf  "PDisks by Bay & SATA SMART ($info{COUNT} disks)  CL=0-U,1-D,2-HS,3=N   DT=1-ENT,2-SATA,4=ECON:\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID DT CL PP GEO STATUS   DNAME    SES SLT CAP(MB) LAS(MB) TAS(MB) TO_RBLD/%REM  IOPS  MBPS  HSDNAME  \n";
        $msg .=  "  --- -- -- -- --- ------ ---------- --- --- ------- ------- ------- ------------ ----- ----- ----------\n";

        for (my $j = 0; $j <= 66; $j++)
        {
           if($j == 66)
           {
              $j = 255;
           }
           for(my $k = 0; $k <= 66; $k++)
           {
             if($k == 66)
             {
               $k = 255;
             }
             for (my $i = 0; $i < $info{COUNT}; $i++)
             {
               if ($j == $info{PDISKS}[$i]{SES})
               {
                 if($k == $info{PDISKS}[$i]{SLOT})
                 {
                   $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
                   $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;
       
                   if ($ses != $info{PDISKS}[$i]{SES} or
                       $slot != $info{PDISKS}[$i]{SLOT})
                   {
                       $wrong = "*";
                   }
                   else
                   {
                       $wrong = " ";
                   }
   
                   if($info{PDISKS}[$i]{LAS} == $info{PDISKS}[$i]{TAS})
                   {
                       $wrong .= " ";
                   }
                   else
                   {
                       $wrong .= "!";
                   }
   
                   $msg .= sprintf "%s%3hu %2hu %2hu %2d %2d  %02x/%02x  %-10s %3d %3d %7s %7s %7s %7s/%4d %5d %5.2f %10s\n",
                       $wrong,
                       $info{PDISKS}[$i]{PD_PID},
                       $info{PDISKS}[$i]{PD_DEVTYPE},
                       $info{PDISKS}[$i]{PD_CLASS},
                       $info{PDISKS}[$i]{PD_CHANNEL},
                       $info{PDISKS}[$i]{GL_ID},
                       $info{PDISKS}[$i]{PD_DEVSTAT},
                       $info{PDISKS}[$i]{PD_POSTSTAT},
                       _getString_DNAME2($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{PD_DNAME}),
                       $info{PDISKS}[$i]{SES},
                       $info{PDISKS}[$i]{SLOT},
                       $info{PDISKS}[$i]{CAPACITY}/2048,
                       $info{PDISKS}[$i]{LAS}/2048,
                       $info{PDISKS}[$i]{TAS}/2048,
                       $info{PDISKS}[$i]{RBREMAIN}/2048,
                       $info{PDISKS}[$i]{PCTREM},
                       $info{PDISKS}[$i]{PD_RPS},
                       $info{PDISKS}[$i]{PD_AVGSC}*$info{PDISKS}[$i]{PD_RPS}/2048,
                       _getString_DNAME2($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{HSDNAME});
                   if($info{PDISKS}[$i]{PD_DEVTYPE} == 2)
                   {
                      if($info{PDISKS}[$i]{CAPACITY}/2048 > 715000)
                      {
                         my @deviceID;
                         ##
                         # Use the smart read data page from the drive to get the smart data
                         ##
                         $deviceID[0]{WWN_LO} = $info{PDISKS}[$i]{WWN_LO};
                         $deviceID[0]{WWN_HI} = $info{PDISKS}[$i]{WWN_HI};
                         $deviceID[0]{PD_LUN} = $info{PDISKS}[$i]{PD_LUN};
                  
                         my $logSenseCDB = "4d003000000000100000";
                         my $cdb = AsciiHexToBin($logSenseCDB, "byte");
                         my %rspInfo = $self->scsiCmd( $cdb, undef, @deviceID );
                  
                         if( %rspInfo )
                         {
                             if( $rspInfo{STATUS} == PI_GOOD )
                             {
                                 ##
                                 # SCSI command succeeded - grab the counters from the returned data
                                 ##
    #                             $self->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256);
                  
                                 my $currentByte = 6;
                                 my $smartid = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                 my $smartflags = 0;
                                 my $smartcurrent = 0;
                                 my $smartworst = 0;
                                 my $smartraw0 = 0;
                                 my $smartraw1 = 0;
                                 my $smartraw2 = 0;
                                 my $smartraw3 = 0;
                                 my $smartraw4 = 0;
                                 my $smartraw5 = 0;
                                 $currentByte += 1;
                                 $msg .= sprintf "     SMART INFORMATION:    CUR WORST THOLD  RAW DATA     PRODID=%12s SN=%s REV=%s\n", $info{PDISKS}[$i]{PS_PRODID},$info{PDISKS}[$i]{PS_SERIAL},$info{PDISKS}[$i]{PD_REV};
                                 while( $smartid != 0x0 )
                                 {
                                     $smartflags = unpack( "n", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 2;
                                     $smartcurrent = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartworst = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw4 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 1;
                                     $smartraw5 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );  
                                     $currentByte += 2;
        
                                     if( $smartid == 0x01 )
                                     {
                                         my $smartre = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         if( $smartworst <= 6 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         $msg .= sprintf "       -> read error rate: %03d   %03d    6 - Number of Sectors=%d\n", $smartcurrent, $smartworst,$smartre;
                                     }
                                     elsif( $smartid == 0x03 )
                                     {
                                         $msg .= sprintf "       -> spinup time:     %03d   %03d    0 - 0x%02x%02x%02x%02x%02x%02x\n",$smartcurrent,$smartworst,$smartraw5,$smartraw4,$smartraw3,$smartraw2,$smartraw1,$smartraw0;
                                     }
                                     elsif( $smartid == 0x04 )
                                     {
                                         if( $smartworst <= 20 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         my $smartsscnt = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Start Stop CNT:  %03d   %03d   20 - Cnt=%d\n",$smartcurrent,$smartworst,$smartsscnt;
                                     }
                                     elsif( $smartid == 0x05 )
                                     {
                                         if( $smartworst <= 36 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         my $smartreall = $smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Retired SECTORS: %03d   %03d   36 - Cnt=%d\n",$smartcurrent,$smartworst,$smartreall;
                                     }
                                     elsif( $smartid == 0x07 )
                                     {
                                         if( $smartworst <= 30 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         my $smartscnt = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         my $smartse = $smartraw5*256+$smartraw4;
                                         $msg .= sprintf "       -> Seek Error Rate: %03d   %03d   30 - #Seeks=%d, SeekErrs=%d\n",$smartcurrent,$smartworst,$smartscnt,$smartse;
                                     }
                                     elsif( $smartid == 0x09 )
                                     {
                                         my $smarthrs = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         my $smartdays = $smarthrs/24;
                                         $msg .= sprintf "       -> POWER-ON Time:   %03d   %03d    0 - %d hrs (%d days)\n",$smartcurrent,$smartworst,$smarthrs,$smartdays
                                     }
                                     elsif( $smartid == 0x0a )
                                     {
                                         if( $smartworst <= 97 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         my $smartsrcnt = $smartraw5*256*256*256*256*256+$smartraw4*256*256*256*256+$smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Spin Retry CNT:  %03d   %03d   97 - Cnt=%d\n",$smartcurrent,$smartworst,$smartsrcnt;
                                     }
                                     elsif( $smartid == 0x0c )
                                     {
                                         if( $smartworst <= 20 )
                                         {
                                           $msg .= sprintf "       [SMART FAIL]"
                                         }
                                         my $smartpccnt = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Power Cycle CNT: %03d   %03d   20 - Cnt=%d\n",$smartcurrent,$smartworst,$smartpccnt;
                                     }
                                     elsif( $smartid == 0xb8 )  #184
                                     {
                                         $msg .= sprintf "       -> Rpt IOEDC errs:  %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                     }
                                     elsif( $smartid == 0xbb )  #187
                                     {
                                         $msg .= sprintf "       -> Rpt Uncorrected: %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartraw1*256+$smartraw0;
                                     }
                                     elsif( $smartid == 0xbc )  #188
                                     {
                                         $msg .= sprintf "       -> Command Timeout: %03d   %03d    0 - Cnt=%d, >5secs=%d, >7.5secs=%d\n",$smartcurrent,$smartworst,$smartraw1*256+$smartraw0,$smartraw3*256+$smartraw2,$smartraw5*256+$smartraw4;
                                     }
                                     elsif( $smartid == 0xbd )  #189
                                     {
                                         $msg .= sprintf "       -> High Fly Writes: %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartraw1*256+$smartraw0;
                                     }
                                     elsif( $smartid == 0xbe )  #190
                                     {
                                         if( $smartworst <= 45 )
                                         {
                                           $msg .= sprintf "       [SMART WARN]"
                                         }
                                         $msg .= sprintf "       -> AirFlow Temp:    %03d   %03d   45 - Now=%d, Since PO=%d-%d (degF), too high cnt=%d\n",$smartcurrent,$smartworst,$smartraw0*9/5+32,$smartraw2*9/5+32,$smartraw3*9/5+32,$smartraw5*256+$smartraw4;
                                     }
                                     elsif( $smartid == 0xc2 )  #194
                                     {
                                         $msg .= sprintf "       -> Temperature:     %03d   %03d    0 - Now=%d, All Time=%d-%d (degF)\n",$smartcurrent,$smartworst,$smartraw0*9/5+32,$smartraw4*9/5+32,$smartworst*9/5+32;
                                     }
                                     elsif( $smartid == 0xc3 )  #195
                                     {
                                         my $smarteccnt = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> ECC OTF Count:   %03d   %03d    0 - #Sectors=%d\n",$smartcurrent,$smartworst,$smarteccnt;
                                     }
                                     elsif( $smartid == 0xc5 )  #197
                                     {
                                         my $smartreallcnt = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Realloc Evt CNT: %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartreallcnt;
                                     }
                                     elsif( $smartid == 0xc6 )  #198
                                     {
                                         my $smartou = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Offline Uncorr:  %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartou;
                                     }
                                     elsif( $smartid == 0xc7 )  #199
                                     {
                                         my $smartudma = $smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> UDMA R-Errs CNT: %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartudma;
                                     }
                                     elsif( $smartid == 0xc8 )  #200
                                     {
                                         my $smartfh = $smartraw5*256*256*256*256*256+$smartraw4*256*256*256*256+$smartraw3*256*256*256+$smartraw2*256*256+$smartraw1*256+$smartraw0;
                                         $msg .= sprintf "       -> Flying Height:   %03d   %03d    0 - Cnt=%d\n",$smartcurrent,$smartworst,$smartfh;
                                     }
                                     elsif( $smartid == 0xca )  #202
                                     {
                                         $msg .= sprintf "       -> U-Area Offline:  %03d   %03d    0 - 0x%02x%02x%02x%02x%02x%02x\n",$smartcurrent,$smartworst,$smartraw5,$smartraw4,$smartraw3,$smartraw2,$smartraw1,$smartraw0;
                                     }
                                     else
                                     {
                                         $msg .= sprintf "       -> Unknown id (%d): %03d   %03d    0 - 0x%02x%02x%02x%02x%02x%02x\n",$smartid,$smartcurrent,$smartworst,$smartraw5,$smartraw4,$smartraw3,$smartraw2,$smartraw1,$smartraw0;
                                     }
                                     $smartid = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                 }
                                 if($info{PDISKS}[$i]{CAPACITY}/2048 > 915000)
                                 {
                                     my $smartbiterr =  unpack( "V", substr($rspInfo{DATA}, 4+388) );
                                     
                                     my $smartspinidx = unpack( "C", substr($rspInfo{DATA}, 4+392) );
                                     
                                     my $smartsuhist0 = unpack( "V", substr($rspInfo{DATA}, 4+393) );
                                     my $smartsuhist1 = unpack( "V", substr($rspInfo{DATA}, 4+397) );
                                     
                                     my $smartsretry0 = unpack( "V", substr($rspInfo{DATA}, 4+401) );
                                     my $smartsretry1 = unpack( "V", substr($rspInfo{DATA}, 4+405) );
                                     my $smarten =      unpack( "C", substr($rspInfo{DATA}, 4+406) );
                                     
                                     my $smartshock =         unpack( "v", substr($rspInfo{DATA}, 4+420) );
                                     my $smartnumrawretries = unpack( "v", substr($rspInfo{DATA}, 4+422) );
                                     my $smartaggscancnt =    unpack( "v", substr($rspInfo{DATA}, 4+432) );
                                     my $smartpendtimerset =  unpack( "v", substr($rspInfo{DATA}, 4+434) );
                                     my $smartsyswritefails = unpack( "v", substr($rspInfo{DATA}, 4+438) );
                                     
                                     my $smarthostwrites_h = unpack( "V", substr($rspInfo{DATA}, 4+440) );
                                     my $smarthostwrites_l = unpack( "V", substr($rspInfo{DATA}, 4+444) );
                                     my $smarthostreads_h =  unpack( "V", substr($rspInfo{DATA}, 4+448) );
                                     my $smarthostreads_l =  unpack( "V", substr($rspInfo{DATA}, 4+452) );
                                     my $smartpohrslastlba = unpack( "V", substr($rspInfo{DATA}, 4+456) );
                                     my $smartbiterr_195 =   unpack( "V", substr($rspInfo{DATA}, 4+460) );
                                     my $smartprevattrsts =  unpack( "v", substr($rspInfo{DATA}, 4+464) );
                                     
                                     my $smartcmdTOintcnt =   unpack( "v", substr($rspInfo{DATA}, 4+472) );
                                     my $smartcmdTOerrcnt =   unpack( "v", substr($rspInfo{DATA}, 4+474) );
                                     my $smartbmspctdone =    unpack( "v", substr($rspInfo{DATA}, 4+490) );
                                     my $smartloadretries =   unpack( "V", substr($rspInfo{DATA}, 4+496) );
                                     my $smartunloadretries = unpack( "V", substr($rspInfo{DATA}, 4+500) );
                                     my $smartmaxspinup =     unpack( "C", substr($rspInfo{DATA}, 4+488) );
                                     my $smartbmsstatus =     unpack( "C", substr($rspInfo{DATA}, 4+489) );
                                     
                                     $msg .= sprintf "       [EN=%02x, smartbits=%08x, uhist=%08x%08x\n",
                                                     $smarten,$smartbiterr,$smartsuhist1,$smartsuhist0;
                                     $msg .= sprintf "       [maxspinup/shock=%d/%d,      sretry=%08x%08x\n",
                                                     $smartmaxspinup,$smartshock,$smartsretry1,$smartsretry0;
                                     $msg .= sprintf "       [bms_sts/bms_pct=%d/%d, cmdintcnt/errcnt=%d/%d\n",
                                                     $smartbmsstatus,$smartbmspctdone,$smartcmdTOintcnt,$smartcmdTOerrcnt;
                                     $msg .= sprintf "       [rawretry=%d, scancnt=%d, pendtimer=%d]\n",
                                                     $smartnumrawretries,$smartaggscancnt,$smartpendtimerset;
                                     $msg .= sprintf "       [hostwr=%08x%08x, hostrd=%08x%08x, pollba=0x%x, syswritefails=%d]\n",
                                                     $smarthostwrites_l,$smarthostwrites_h,$smarthostreads_l,$smarthostreads_h,$smartpohrslastlba,$smartsyswritefails;
                                     $msg .= sprintf "       [be_195=0x%x, pattrsts=%d, unloadretries=%d, loadretries=%d]\n",
                                                     $smartbiterr_195,$smartprevattrsts,$smartunloadretries,$smartloadretries;
                                 }
                                 else
                                 {
                                     $currentByte = 390;
                                     my $smartbiterr0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartbiterr1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartbiterr2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartbiterr3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     
                                     my $smartspinidx = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     
                                     my $smartsuhist0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist4 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist5 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist6 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsuhist7 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     
                                     my $smartsretry0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry4 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry5 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry6 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartsretry7 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smarten = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     
                                     $currentByte=422;
                                     my $smartticks0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks4 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks5 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks6 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     $currentByte += 1;
                                     my $smartticks7 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                     
                                     my $smartiffysectorcnt = unpack( "V", substr($rspInfo{DATA}, 4+426) );
                                     my $smartshock = unpack( "v", substr($rspInfo{DATA}, 4+430) );
                                     my $smartnumrawretries = unpack( "v", substr($rspInfo{DATA}, 4+432) );
                                     my $smartaggscancnt = unpack( "v", substr($rspInfo{DATA}, 4+434) );
                                     my $smartlastlba = unpack( "V", substr($rspInfo{DATA}, 4+436) );
                                     my $smartpendtimerset = unpack( "v", substr($rspInfo{DATA}, 4+440) );
                                     my $smartnumpo2sweep = unpack( "v", substr($rspInfo{DATA}, 4+442) );
                                     my $smartnumburps = unpack( "v", substr($rspInfo{DATA}, 4+444) );
                                     my $smartnumaburps = unpack( "v", substr($rspInfo{DATA}, 4+446) );
                                     my $smartrestartkey = unpack( "v", substr($rspInfo{DATA}, 4+448) );
                                     my $smartinitreason = unpack( "v", substr($rspInfo{DATA}, 4+450) );
                                     my $smarttrackfails = unpack( "v", substr($rspInfo{DATA}, 4+452) );
                                     my $smartcrcerrors = unpack( "V", substr($rspInfo{DATA}, 4+454) );
                                     my $smarthostwrites_h = unpack( "V", substr($rspInfo{DATA}, 4+458) );
                                     my $smarthostwrites_l = unpack( "V", substr($rspInfo{DATA}, 4+462) );
                                     my $smarthostreads_h = unpack( "V", substr($rspInfo{DATA}, 4+466) );
                                     my $smarthostreads_l = unpack( "V", substr($rspInfo{DATA}, 4+470) );
                                     my $smartpohrslastlba = unpack( "V", substr($rspInfo{DATA}, 4+474) );
                                     my $smartrwtempevnts = unpack( "v", substr($rspInfo{DATA}, 4+478) );
                                     my $smartbiterr_195 = unpack( "V", substr($rspInfo{DATA}, 4+480) );
                                     my $smartprevattrsts = unpack( "v", substr($rspInfo{DATA}, 4+484) );
                                     my $smartamdetectretrycnt = unpack( "v", substr($rspInfo{DATA}, 4+486) );
                                     my $smartxingdeltatripcnt = unpack( "v", substr($rspInfo{DATA}, 4+488) );
                                     my $smartunloadretrycnt = unpack( "V", substr($rspInfo{DATA}, 4+490) );
                                     my $smartloadretrycnt = unpack( "V", substr($rspInfo{DATA}, 4+494) );
                                     
                                     $msg .= sprintf "       [EN=%02x, smartbits=%02x%02x%02x%02x, uhist=%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                                     $smarten,$smartbiterr3,$smartbiterr2,$smartbiterr1,$smartbiterr0,
                                                     $smartsuhist7,$smartsuhist6,$smartsuhist5,$smartsuhist4,$smartsuhist3,$smartsuhist2,$smartsuhist1,$smartsuhist0;
                                     $msg .= sprintf "       [burps/aburps=%d/%d,         sretry=%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                                     $smartnumburps,$smartnumaburps,
                                                     $smartsretry7,$smartsretry6,$smartsretry5,$smartsretry4,$smartsretry3,$smartsretry2,$smartsretry1,$smartsretry0;
                                     $msg .= sprintf "       [iffy/shock=%d/%d,           sticks=%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                                     $smartiffysectorcnt,$smartshock,
                                                     $smartticks7,$smartticks6,$smartticks5,$smartticks4,$smartticks3,$smartticks2,$smartticks1,$smartticks0;
                                     $msg .= sprintf "       [rawretry=%d, scancnt=%d, lastlba=0x%x, pendtimer=%d, numpo2swp=%d,restartkey=%d]\n",
                                                     $smartnumrawretries,$smartaggscancnt,$smartlastlba,$smartpendtimerset,
                                                     $smartnumpo2sweep,$smartrestartkey;
                                     $msg .= sprintf "       [initrsn=%d, trackfails=%d, crcerrs=%d, hostwr=%08x%08x, hostrd=%08x%08x, pollba=0x%x]\n",
                                                     $smartinitreason,$smarttrackfails,$smartcrcerrors,$smarthostwrites_l,$smarthostwrites_h,$smarthostreads_l,$smarthostreads_h,
                                                     $smartpohrslastlba;
                                     $msg .= sprintf "       [rwevnts=%d, be_195=0x%x, pattrsts=%d, amretries=%d, xingdelta=%d, unloadretries=%d, loadretries=%d]\n",
                                                     $smartrwtempevnts,$smartbiterr_195,$smartprevattrsts,$smartamdetectretrycnt,$smartxingdeltatripcnt,$smartunloadretrycnt,$smartloadretrycnt;
                                 }
                             }
                         }
                         $logSenseCDB = "4d003200000000100000";
                         $cdb = AsciiHexToBin($logSenseCDB, "byte");
                         %rspInfo = $self->scsiCmd( $cdb, undef, @deviceID );
                  
                         if( %rspInfo )
                         {
                             if( $rspInfo{STATUS} == PI_GOOD )
                             {
                                 ##
                                 # SCSI command succeeded - grab the counters from the returned data
                                 ##
    #                             $self->FormatData($rspInfo{DATA}, 0x00000000, "byte", undef, 256);
                  
                                 my $currentByte = 5;
                                 my $smartlastindex = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                 $currentByte = 456;
                                 my $smartb1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                 $currentByte += 1;
                                 my $smartb2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                 
                                 my $smarterrorcnt = $smartb1+$smartb2*256;
                                 
                                 $msg .= sprintf "     Error Log: %d errors, lastindex=%d\n",$smarterrorcnt,$smartlastindex;
                                 my $ii = $smarterrorcnt;
                                 my $jj = $smartlastindex;
                                 while($ii!=0 && $smarterrorcnt<$ii+5)
                                 {
                                    $currentByte = 6+($jj-1)*90+61;
                                    my $smerr = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smcnt = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smlba0 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smlba8 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smlba16 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smlba = $smlba16+$smlba8*256+$smlba0*256*256;
                                    my $smdh = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smsts = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smhextmode = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 2;
                                    my $smierrnum = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smrwerrnum = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smdiskseqsts = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 15;
                                    my $smstate = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smb1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    $currentByte += 1;
                                    my $smb2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                    my $smt2e = $smb2+256*$smb1;
                                    $msg .= sprintf "     ->Error #%d: ERR=%02x, CNT=%d, LBA=%x, DH=%02x, STS/STATE=%02x/%02x, T2E=%d, HEEM=%d, IERR#=%d, RWERR#=%d, SEQSTS=%x\n",
                                        $ii,$smerr,$smcnt,$smlba,$smdh,$smsts,$smstate,$smt2e,$smhextmode,$smierrnum,$smrwerrnum,$smdiskseqsts;
                                    # now for the last 5 commands (if needed)
                                    if($smarterrorcnt!=0)
                                    {
                                       for(my $mm = 1; $mm <=5; $mm++)
                                       {
                                         $currentByte = 6+($jj-1)*90+($mm-1)*12;
                                         my $smdr = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smfr = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smsc = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smsn = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smcl = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smch = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         my $lba = $smch+$smcl*256+$smsn*256*256;
                                         $currentByte += 1;
                                         $smdh = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smcmd = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         $smb1 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         $smb2 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smb3 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         $currentByte += 1;
                                         my $smb4 = unpack( "C", substr($rspInfo{DATA}, $currentByte) );
                                         my $smms = $smb4*256*256*256+$smb3*256*256+$smb2*256+$smb1;
                                         my $smdesc="UNKNOWN CMD  ";
                                         if($smcmd==0x35)
                                         {
                                           $smdesc="WRITE DMA EXT";
                                         }
                                         elsif($smcmd==0x25)
                                         {
                                           $smdesc="READ DMA EXT ";
                                         }
                                         elsif($smcmd==0x2f)
                                         {
                                           $smdesc="READ LOG EXT ";
                                         }
                                         elsif($smcmd==0xb0)
                                         {
                                           $smdesc="S.M.A.R.T    ";
                                         }
                                         elsif($smcmd==0x61)
                                         {
                                           $smdesc="WRT FPDMA Q'd";
                                         }
                                         elsif($smcmd==0x60)
                                         {
                                           $smdesc="RD FPDMA Q'd ";
                                         }
                                         elsif($smcmd==0x08)
                                         {
                                           $smdesc="SOFT RESET   ";
                                         }
                                         elsif($smcmd==0xe4)
                                         {
                                           $smdesc="READ BUFFER  ";
                                         }
                                         elsif($smcmd==0xe5)
                                         {
                                           $smdesc="CHK PWR MODE ";
                                         }
                                         elsif($smcmd==0xec)
                                         {
                                           $smdesc="ID DEVICE    ";
                                         }
                                         elsif($smcmd==0xef)
                                         {
                                           $smdesc="SET FEATURES ";
                                         }
                                         elsif($smcmd>=0x10 && $smcmd<=0x1f)
                                         {
                                           $smdesc="RECALIBRATE  ";
                                         }
                                         $msg .= sprintf "       CMD #%d: CMD=%02x(%s), DC=%02x, FR=%02x, SC=%02x, LBA=%08x, DH=%02x, TS=%d ms\n",$mm,$smcmd,$smdesc,$smdr,$smfr,$smsc,$smlba,$smdh,$smms;
                                       }
                                    }
                                    $ii--;
                                    $jj--;
                                    if($jj==0)
                                    {
                                      $jj=5;
                                    }
                                 }
                             }
                         }
                      }
                    }
                  }
               }
             }
           }
        }
    }
    elsif (uc($dsptype) eq "LOOP")
    {
        my %portHash;
        my $portHashKey;

        my %sesHash;
        my $sesHashKey;

        my %slotHash;
        my $slotHashKey;

        my %lidHash;
        my $lidHashKey;

        my $debug = 1;

        ##
        # Make a new hash of hashes, using the PORT, SES, and SLOT as the keys
        ##
        for( my $i = 0; $i < $info{COUNT}; $i++ )
        {
            ##
            # Hash by PORT, SES, and SLOT
            ##
            my $portAChannelString = "?";
            my $portBChannelString = "?";
            my $portAString = "?";
            my $portBString = "?";
            my $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} & ~1;
            my $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} |  1;

            if( $info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_UNKNOWN )
            {
                next;
            }
            elsif ($info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_FC_DISK ||
                   $info{PDISKS}[$i]{PD_DEVTYPE} == PD_DT_ECON_ENT)
            {
                if( $info{PDISKS}[$i]{CIP} =~ /N\/A/ )
                {
                    next;
                }
                elsif( $info{PDISKS}[$i]{CIP} == 0 )
                {
                    ##
                    # CIP is zero, meaning the current port number is 'channel A'
                    ##
                    $portAChannelString = "A";
                    $portBChannelString = "B";

                    $portANumber = $info{PDISKS}[$i]{PD_CHANNEL};
                    $portAString = sprintf( "*%01d", $info{PDISKS}[$i]{PD_CHANNEL} );

                    if( $info{PDISKS}[$i]{PD_CHANNEL} & 0x01 )
                    {
                        # Port number is odd, so this port is one less (even)
                        $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} - 1;
                    }
                    else
                    {
                        # Port number is even, so this port is one more (odd)
                        $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL} + 1;
                    }
                }
                else
                {
                    ##
                    # CIP is one, meaning the current port number is 'channel B'
                    ##
                    $portAChannelString = "A";
                    $portBChannelString = "B";

                    $portBNumber = $info{PDISKS}[$i]{PD_CHANNEL};
                    $portBString = sprintf( "*%01d", $info{PDISKS}[$i]{PD_CHANNEL} );

                    if( $info{PDISKS}[$i]{PD_CHANNEL} & 0x01 )
                    {
                        # Port number is odd, so this port is one less (even)
                        $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} - 1;
                    }
                    else
                    {
                        # Port number is even, so this port is one more (odd)
                        $portANumber = $info{PDISKS}[$i]{PD_CHANNEL} + 1;
                    }
                }
            }

            ##
            # Stringify the port numbers
            ##
            $portAString = sprintf( "%01d", $portANumber );
            $portBString = sprintf( "%01d", $portBNumber );

            ##
            # Add channel A to the hash
            ##
            $portHash{$portANumber}{$info{PDISKS}[$i]{SES}}{$info{PDISKS}[$i]{SLOT}}{$info{PDISKS}[$i]{PD_ID}} = {
                PD_PID   => $info{PDISKS}[$i]{PD_PID},
                PD_DNAME => $info{PDISKS}[$i]{PD_DNAME},
                SES      => $info{PDISKS}[$i]{SES},
                SLOT     => $info{PDISKS}[$i]{SLOT},
                LID      => $info{PDISKS}[$i]{PD_ID},
                LMAP     => $info{PDISKS}[$i]{PD_LOOPMAP},
                PORT     => $portAString,
                CHAN     => $portAChannelString,
                LFCNT    => $info{PDISKS}[$i]{LFCNT_A},
                LSCNT    => $info{PDISKS}[$i]{LSCNT_A},
                ITCNT    => $info{PDISKS}[$i]{ITCNT_A},
                ICCNT    => $info{PDISKS}[$i]{ICCNT_A},
                LIPF7I   => $info{PDISKS}[$i]{LIPF7I_A},
                LIPF7R   => $info{PDISKS}[$i]{LIPF7R_A},
                LIPF8I   => $info{PDISKS}[$i]{LIPF8I_A},
                LIPF8R   => $info{PDISKS}[$i]{LIPF8R_A},
                POM      => $info{PDISKS}[$i]{POM} };

            ##
            # Add channel B to the hash
            ##
            $portHash{$portBNumber}{$info{PDISKS}[$i]{SES}}{$info{PDISKS}[$i]{SLOT}}{$info{PDISKS}[$i]{PD_ID}} = {
                PD_PID   => $info{PDISKS}[$i]{PD_PID},
                PD_DNAME => $info{PDISKS}[$i]{PD_DNAME},
                SES      => $info{PDISKS}[$i]{SES},
                SLOT     => $info{PDISKS}[$i]{SLOT},
                LID      => $info{PDISKS}[$i]{PD_ID},
                LMAP     => $info{PDISKS}[$i]{PD_LOOPMAP},
                PORT     => $portBString,
                CHAN     => $portBChannelString,
                LFCNT    => $info{PDISKS}[$i]{LFCNT_B},
                LSCNT    => $info{PDISKS}[$i]{LSCNT_B},
                ITCNT    => $info{PDISKS}[$i]{ITCNT_B},
                ICCNT    => $info{PDISKS}[$i]{ICCNT_B},
                LIPF7I   => $info{PDISKS}[$i]{LIPF7I_B},
                LIPF7R   => $info{PDISKS}[$i]{LIPF7R_B},
                LIPF8I   => $info{PDISKS}[$i]{LIPF8I_B},
                LIPF8R   => $info{PDISKS}[$i]{LIPF8R_B},
                POM      => $info{PDISKS}[$i]{POM} };
        }

        ##
        # Display the data, sorted first on SES number, then on slot number
        ##
        $msg .= sprintf "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf "  PID   DNAME    SES   SLOT  LID   ALPA LMAP PORT LINK_FAIL  LOST_SYNC   INV_XMIT   INV_CRC   LIPF7_Init LIPF7_Recv LIPF8_Init LIPF8_Recv PowerOnMin\n";
        $msg .= sprintf "  --- ---------- ----- ---- ------ ---- ---- ---- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n";

        foreach $portHashKey( sort _sortNumerically keys %portHash )
        {
            foreach $sesHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}} )
            {
                foreach $slotHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}{$sesHashKey}} )
                {
                    foreach $lidHashKey( sort _sortNumerically keys %{$portHash{$portHashKey}{$sesHashKey}{$slotHashKey}} )
                    {
                        $msg .= sprintf "  %3hu %s %5d  %3d 0x%04x 0x%02x  %3d %2s-%1s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PD_PID},
                            _getString_DNAME(0,$portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PD_DNAME}),
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{SES},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{SLOT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LID},
                            _getALPA($portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LID}),
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LMAP},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{PORT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{CHAN},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LFCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LSCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{ITCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{ICCNT},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF7I},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF7R},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF8I},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{LIPF8R},
                            $portHash{$portHashKey}{$sesHashKey}{$slotHashKey}{$lidHashKey}{POM};
                    }
                }

                $msg .= sprintf "\n";
            }

            $msg .= sprintf "\n";
        }
    }
    elsif (uc($dsptype) eq "FWV")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID  VendorID    TYPE     REV   Product ID        Serial #      CAPACITY (blocks)     WWN\n";
        $msg .= sprintf "  ---  --------  --------  -----  ----------------  ------------  --------------------  ----------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "  %3hu  %8s  %-8s  %-5s  %16s  %12s  %-20s  %8.8x%8.8x\n", 
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{VENDID},
                    _getString_PDDT($info{PDISKS}[$i]{PD_DEVTYPE}),
                    $info{PDISKS}[$i]{PD_REV},
                    $info{PDISKS}[$i]{PS_PRODID},
                    $info{PDISKS}[$i]{PS_SERIAL},
                    $info{PDISKS}[$i]{CAPACITY},
                    
                    $info{PDISKS}[$i]{WWN_LO}, $info{PDISKS}[$i]{WWN_HI};
                    
        }
    }
    elsif (uc($dsptype) eq "SES")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID   SES    SLOT      DNAME  \n";
        $msg .= sprintf "  ---  -----  ------  ----------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $ses = ($info{PDISKS}[$i]{PD_DNAME} & 0x00FF0000) >> 16;
            $slot = ($info{PDISKS}[$i]{PD_DNAME} & 0xFF000000) >> 24;

            if ($ses != $info{PDISKS}[$i]{SES} or
                $slot != $info{PDISKS}[$i]{SLOT})
            {
                $wrong = "*";
            }
            else
            {
                $wrong = " ";
            }

            $msg .= sprintf "%s %3hu  %5d  %6d  %s\n",
                    $wrong,
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{SES},
                    $info{PDISKS}[$i]{SLOT},
                    _getString_DNAME($self->{CONTROLLER_TYPE},$info{PDISKS}[$i]{PD_DNAME});
        }
    }
    elsif (uc($dsptype) eq "CMPL")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "Physical Disk Information:\n";
            $msg .= sprintf "  CLASS:                %s\n", _getString_CLASS($info{PDISKS}[$i]{PD_CLASS});
            $msg .= sprintf "  DEVTYPE:              %hu\n", $info{PDISKS}[$i]{PD_DEVTYPE};
            $msg .= sprintf "  MISCSTAT:             0x%02x\n", $info{PDISKS}[$i]{PD_MISCSTAT};
            $msg .= sprintf "  PORT:                 %hu\n", $info{PDISKS}[$i]{PD_CHANNEL};
            $msg .= sprintf "  LOOPMAP:              %hu\n",$info{PDISKS}[$i]{PD_LOOPMAP};
            $msg .= sprintf "  LUN:                  %hu\n", $info{PDISKS}[$i]{PD_LUN};
            $msg .= sprintf "  LID:                  0x%x\n", $info{PDISKS}[$i]{PD_ID};
            $msg .= sprintf "  ALPA:                 0x%x\n", _getALPA($info{PDISKS}[$i]{PD_ID});
            $msg .= sprintf "  DEV:                  0x%x\n", $info{PDISKS}[$i]{PD_DEV};
            $msg .= sprintf "  PID:                  %hu\n", $info{PDISKS}[$i]{PD_PID};
            $msg .= sprintf "  POSTSTAT:             0x%02x\n", $info{PDISKS}[$i]{PD_POSTSTAT};
            $msg .= sprintf "  DEVSTAT:              0x%02x\n", $info{PDISKS}[$i]{PD_DEVSTAT};
            $msg .= sprintf "  FLED:                 0x%x\n", $info{PDISKS}[$i]{PD_FLED};
            $msg .= sprintf "  PCTREM:               %hu\n", $info{PDISKS}[$i]{PCTREM};
            $msg .= sprintf "  CAPACITY:             $info{PDISKS}[$i]{CAPACITY}\n";
            $msg .= sprintf "  QD:                   %lu\n", $info{PDISKS}[$i]{PD_QD};
            $msg .= sprintf "  RPS:                  %lu\n", $info{PDISKS}[$i]{PD_RPS};
            $msg .= sprintf "  AVGSC:                %lu\n", $info{PDISKS}[$i]{PD_AVGSC};
            $msg .= sprintf "  SSERIAL:              0x%x\n", $info{PDISKS}[$i]{PD_SSERIAL};
            $msg .= sprintf "  RREQ:                 $info{PDISKS}[$i]{RREQ}\n";
            $msg .= sprintf "  WREQ:                 $info{PDISKS}[$i]{WREQ}\n";
            $msg .= sprintf "  VENDID:               $info{PDISKS}[$i]{VENDID}\n";
            $msg .= sprintf "  REV:                  $info{PDISKS}[$i]{PD_REV}\n";
            $msg .= sprintf "  ERR:                  %lu\n", $info{PDISKS}[$i]{PD_ERR};
            $msg .= sprintf "  PRODID:               $info{PDISKS}[$i]{PS_PRODID}\n";
            $msg .= sprintf "  SERIAL:               $info{PDISKS}[$i]{PS_SERIAL}\n";
            $msg .= sprintf "  DAML:                 0x%x\n", $info{PDISKS}[$i]{PD_DAML};
            $msg .= sprintf "  TAS:                  $info{PDISKS}[$i]{TAS}\n";
            $msg .= sprintf "  LAS:                  $info{PDISKS}[$i]{LAS}\n";
            $msg .= sprintf "  WWN:                  %8.8x%8.8x\n", $info{PDISKS}[$i]{WWN_LO}, $info{PDISKS}[$i]{WWN_HI};
            $msg .= sprintf "  R10_MISCOMP:          %lu\n", $info{PDISKS}[$i]{R10_MISCOMP};
            $msg .= sprintf "  DNAME:                0x%x\n", $info{PDISKS}[$i]{PD_DNAME};
            $msg .= sprintf "  LFCNT:                %lu\n", $info{PDISKS}[$i]{LFCNT};
            $msg .= sprintf "  LSCNT:                %lu\n", $info{PDISKS}[$i]{LSCNT};
            $msg .= sprintf "  LGCNT:                %lu\n", $info{PDISKS}[$i]{LGCNT};
            $msg .= sprintf "  PSCNT:                %lu\n", $info{PDISKS}[$i]{PSCNT};
            $msg .= sprintf "  ITCNT:                %lu\n", $info{PDISKS}[$i]{ITCNT};
            $msg .= sprintf "  ICCNT:                %lu\n", $info{PDISKS}[$i]{ICCNT};
            $msg .= sprintf "  MISCOMP:              %lu\n", $info{PDISKS}[$i]{MISCOMP};
            $msg .= sprintf "  DEVMISCOMP:           %lu\n", $info{PDISKS}[$i]{DEVMISCOMP};
            $msg .= sprintf "  RBTOTAL:              $info{PDISKS}[$i]{RBTOTAL}\n";
            $msg .= sprintf "  RBREMAIN:             $info{PDISKS}[$i]{RBREMAIN}\n";
            $msg .= sprintf "  SES:                  $info{PDISKS}[$i]{SES}\n";
            $msg .= sprintf "  SLOT:                 $info{PDISKS}[$i]{SLOT}\n";
            $msg .= sprintf "  HSDNAME:              0x%x\n", $info{PDISKS}[$i]{HSDNAME};
            $msg .= sprintf "  GEO-LOCATION:         %u\n", $info{PDISKS}[$i]{GL_ID};
            $msg .= sprintf "  GEOFLAGS:             0x%x\n", $info{PDISKS}[$i]{GEOFLAGS};
            $msg .= sprintf "  HANGCNT:              %u\n", $info{PDISKS}[$i]{HANGCNT};
            $msg .= sprintf "  LASTLIP:              %u\n", $info{PDISKS}[$i]{LASTLIP};
            $msg .= sprintf "\n";
        }
    }
    elsif (uc($dsptype) eq "PORTS")
    {
        $msg .= sprintf  "Physical Disks ($info{COUNT} disks):\n";
        $msg .= sprintf  "\n";

        $msg .= sprintf "  PID  PORT  LOOPMAP  LUN    LID   ALPA\n";
        $msg .= sprintf "  ---  ----  -------  ----  -----  ----\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            $msg .= sprintf "  %3hu  %4d  %7d  0x%2.2x  0x%3.3x  0x%2.2x\n",
                    $info{PDISKS}[$i]{PD_PID},
                    $info{PDISKS}[$i]{PD_CHANNEL},
                    $info{PDISKS}[$i]{PD_LOOPMAP},
                    $info{PDISKS}[$i]{PD_LUN},
                    $info{PDISKS}[$i]{PD_ID},
                    _getALPA($info{PDISKS}[$i]{PD_ID});
        }
    }
    elsif (uc($dsptype) eq "DEFRAG")
    {
        $msg .= sprintf  "Physical Disks defragging or requiring defrag:\n";
        $msg .= sprintf  "\n";

        $msg .=  "  PID  %REM    RBREMAIN          LAS            TAS     \n";
        $msg .=  "  ---  ----  -------------  -------------  -------------\n";

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $defragging = $info{PDISKS}[$i]{PD_MISCSTAT} & 0x4;

            if ($info{PDISKS}[$i]{LAS} != $info{PDISKS}[$i]{TAS} or
                $defragging > 0)
            {
                $msg .= sprintf "  %3hu  %4d  %13s  %13s  %13s\n",
                        $info{PDISKS}[$i]{PD_PID},
                        $info{PDISKS}[$i]{PCTREM},
                        $info{PDISKS}[$i]{RBREMAIN},
                        $info{PDISKS}[$i]{LAS},
                        $info{PDISKS}[$i]{TAS};
            }
        }
    }
    else
    {
        $msg .= sprintf  "Unrecognized pdisks argument type ($dsptype).\n";
    }
    
    return $msg;
}


##############################################################################
# Name: displayPhysicalDiskInfo
#
# Desc: Print the physical disk information
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPhysicalDiskInfo
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf "Physical Disk Information:\n";
    $msg .= sprintf "  CLASS:                %s\n", _getString_CLASS($info{PD_CLASS});
    $msg .= sprintf "  DEVTYPE:              0x%x - %s\n", $info{PD_DEVTYPE}, _getString_PDDT($info{PD_DEVTYPE});
    $msg .= sprintf "  MISCSTAT:             0x%02x\n", $info{PD_MISCSTAT};
    $msg .= sprintf "  FLAGS:                0x%02x\n", $info{PD_FLAGS};
    $msg .= sprintf "  PORT:                 %hu\n", $info{PD_CHANNEL};
    $msg .= sprintf "  LOOPMAP:              %hu\n",$info{PD_LOOPMAP};
    $msg .= sprintf "  LUN:                  %hu\n", $info{PD_LUN};
    $msg .= sprintf "  LID:                  0x%x\n", $info{PD_ID};
    $msg .= sprintf "  ALPA:                 0x%x\n", _getALPA($info{PD_ID});
    $msg .= sprintf "  DEV:                  0x%x\n", $info{PD_DEV};
    $msg .= sprintf "  PID:                  %hu\n", $info{PD_PID};
    $msg .= sprintf "  POSTSTAT:             0x%02x\n", $info{PD_POSTSTAT};
    $msg .= sprintf "  DEVSTAT:              0x%02x\n", $info{PD_DEVSTAT};
    $msg .= sprintf "  FLED:                 0x%x\n", $info{PD_FLED};
    $msg .= sprintf "  PCTREM:               %hu\n", $info{PCTREM};
    $msg .= sprintf "  CAPACITY:             $info{CAPACITY}\n";
    $msg .= sprintf "  QD:                   %lu\n", $info{PD_QD};
    $msg .= sprintf "  RPS:                  %lu\n", $info{PD_RPS};
    $msg .= sprintf "  AVGSC:                %lu\n", $info{PD_AVGSC};
    $msg .= sprintf "  SSERIAL:              0x%x\n", $info{PD_SSERIAL};
    $msg .= sprintf "  RREQ:                 $info{RREQ}\n";
    $msg .= sprintf "  WREQ:                 $info{WREQ}\n";
    $msg .= sprintf "  VENDID:               $info{VENDID}\n";
    $msg .= sprintf "  REV:                  $info{PD_REV}\n";
    $msg .= sprintf "  ERR:                  %lu\n", $info{PD_ERR};
    $msg .= sprintf "  PRODID:               $info{PS_PRODID}\n";
    $msg .= sprintf "  SERIAL:               $info{PS_SERIAL}\n";
    $msg .= sprintf "  DAML:                 0x%x\n", $info{PD_DAML};
    $msg .= sprintf "  TAS:                  $info{TAS}\n";
    $msg .= sprintf "  LAS:                  $info{LAS}\n";
    $msg .= sprintf "  WWN:                  %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
    $msg .= sprintf "  R10_MISCOMP:          %lu\n", $info{R10_MISCOMP};
    $msg .= sprintf "  DNAME:                0x%x\n", $info{PD_DNAME};
    $msg .= sprintf "  LFCNT:                %lu\n", $info{LFCNT};
    $msg .= sprintf "  LSCNT:                %lu\n", $info{LSCNT};
    $msg .= sprintf "  LGCNT:                %lu\n", $info{LGCNT};
    $msg .= sprintf "  PSCNT:                %lu\n", $info{PSCNT};
    $msg .= sprintf "  ITCNT:                %lu\n", $info{ITCNT};
    $msg .= sprintf "  ICCNT:                %lu\n", $info{ICCNT};
    $msg .= sprintf "  MISCOMP:              %lu\n", $info{MISCOMP};
    $msg .= sprintf "  DEVMISCOMP:           %lu\n", $info{DEVMISCOMP};
    $msg .= sprintf "  RBTOTAL:              $info{RBTOTAL}\n";
    $msg .= sprintf "  RBREMAIN:             $info{RBREMAIN}\n";
    $msg .= sprintf "  SES:                  $info{SES}\n";
    $msg .= sprintf "  SLOT:                 $info{SLOT}\n";
    $msg .= sprintf "  HSDNAME:              0x%x\n", $info{HSDNAME};
# GR_GEORAID - Following is added for GeoRaid
    $msg .= sprintf "  LOCATIONID:           %hu\n", $info{GL_ID};
    $msg .= sprintf "  GEOFLAGS:             0x%x\n", $info{GEOFLAGS};
    $msg .= sprintf "  HANGCNT:              %u\n", $info{HANGCNT};
    $msg .= sprintf "  LASTLIP:              %u\n", $info{LASTLIP};
    $msg .= sprintf "\n";

    return $msg;
}


##############################################################################
# Name: displayPDiskDefragStatus
#
# Desc: Print the physical disk information
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPDiskDefragStatus
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if ($info{FLAGS} == 0)
    {
        $msg .= sprintf "Physical Disk Defrag Status:  INACTIVE\n";
    }
    else
    {
        $msg .= sprintf "Physical Disk Defrag Status:  ACTIVE ";

        if ($info{PDISKID} == 0xFFFF)
        {
            $msg .= sprintf "(ALL)\n";
        }
        else
        {
            $msg .= sprintf "(%u)\n", $info{PDISKID};
        }

        $msg .= sprintf "\n";
    }

    return $msg;
}

#SERVICEABILITY
##############################################################################
# Name: displayPDiskAutoFailback
#
# Desc: Print the physical disk autofailback response
#
# In:   Physical Disk Information Hash
##############################################################################
sub displayPDiskAutoFailback
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    if ($info{MODE} == 0)
    {
        $msg .= sprintf "PDisk Auto Failback Disabled\n";
    }
    else
    {
        $msg .= sprintf "PDisk Auto Failback Enabled\n";
    }

    return $msg;
}
#SERVICEABILITY

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _physicalDisksPacket
#
# Desc:     Parses the physical disks packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub _physicalDisksPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISKS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @pdisks;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (216 * $i);

            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;
            my %tas;
            my %las;
            my %wwn;
            my %rbtotal;
            my %rbremain;

            # Unpack the data
            (
            $rsvd,
            $pdisks[$i]{STATUS_MRP},
            $pdisks[$i]{LEN},
            $pdisks[$i]{PD_CLASS},
            $pdisks[$i]{PD_DEVTYPE},
            $pdisks[$i]{PD_MISCSTAT},
            $rsvd,
            $pdisks[$i]{PD_CHANNEL},
            $pdisks[$i]{PD_LOOPMAP},
            $pdisks[$i]{PD_LUN},
            $pdisks[$i]{PD_ID},

            $pdisks[$i]{PD_DEV},
            $pdisks[$i]{PD_PID},
            $rsvd,
            $pdisks[$i]{PD_POSTSTAT},
            $pdisks[$i]{PD_DEVSTAT},
            $pdisks[$i]{PD_FLED},
            $pdisks[$i]{PCTREM},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $pdisks[$i]{PD_QD},
            $pdisks[$i]{PD_RPS},

            $pdisks[$i]{PD_AVGSC},
            $pdisks[$i]{PD_SSERIAL},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $pdisks[$i]{VENDID},

            $pdisks[$i]{PD_REV},
            $pdisks[$i]{PD_ERR},
            $pdisks[$i]{PS_PRODID},
            $pdisks[$i]{PS_SERIAL},
            $pdisks[$i]{PD_DAML},
            $tas{LO_LONG}, $tas{HI_LONG},

            $las{LO_LONG}, $las{HI_LONG},
            $wwn{LO_LONG}, $wwn{HI_LONG},

            $pdisks[$i]{R10_MISCOMP},
            $pdisks[$i]{PD_DNAME},
            $pdisks[$i]{LFCNT},
            $pdisks[$i]{LSCNT},

            $pdisks[$i]{LGCNT},
            $pdisks[$i]{PSCNT},
            $pdisks[$i]{ITCNT},
            $pdisks[$i]{ICCNT},

            $pdisks[$i]{MISCOMP},
            $pdisks[$i]{DEVMISCOMP},
            $rbtotal{LO_LONG}, $rbtotal{HI_LONG},
            $rbremain{LO_LONG}, $rbremain{HI_LONG},

            $pdisks[$i]{SES},
            $pdisks[$i]{SLOT},
            $rsvd,
            $pdisks[$i]{HSDNAME},
# GR_GEORAID - Following is added for GEORaid
            $pdisks[$i]{GL_ID},
            $pdisks[$i]{GEOFLAGS},
            $pdisks[$i]{HANGCNT},
            $rsvd,
            $pdisks[$i]{LASTLIP}
            ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL 
                        LLNN LLLL LLLL LLLL LLSCCLCCCCL", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $pdisks[$i]{CAPACITY} = longsToBigInt(%capacity);
            $pdisks[$i]{RREQ} = longsToBigInt(%rreq);
            $pdisks[$i]{WREQ} = longsToBigInt(%wreq);
            $pdisks[$i]{TAS} = longsToBigInt(%tas);
            $pdisks[$i]{LAS} = longsToBigInt(%las);
            $pdisks[$i]{WWN} = longsToBigInt(%wwn);
            $pdisks[$i]{WWN_HI} = $wwn{HI_LONG};
            $pdisks[$i]{WWN_LO} = $wwn{LO_LONG};
            $pdisks[$i]{RBTOTAL} = longsToBigInt(%rbtotal);
            $pdisks[$i]{RBREMAIN} = longsToBigInt(%rbremain);
            
        }

        $info{PDISKS} = [@pdisks];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a physical disk info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _physicalDisksCachePacket
#
# Desc:     Parses the physical disks packet from cache and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub _physicalDisksCachePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISKS_FROM_CACHE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @pdisks;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (216 * $i);

            my $rsvd;
            my %capacity;
            my %rreq;
            my %wreq;
            my %tas;
            my %las;
            my %wwn;
            my %rbtotal;
            my %rbremain;

            # Unpack the data
            (
            $rsvd,
            $pdisks[$i]{STATUS_MRP},
            $pdisks[$i]{LEN},
            $pdisks[$i]{PD_CLASS},
            $pdisks[$i]{PD_DEVTYPE},
            $pdisks[$i]{PD_MISCSTAT},
            $rsvd,
            $pdisks[$i]{PD_CHANNEL},
            $pdisks[$i]{PD_LOOPMAP},
            $pdisks[$i]{PD_LUN},
            $pdisks[$i]{PD_ID},

            $pdisks[$i]{PD_DEV},
            $pdisks[$i]{PD_PID},
            $rsvd,
            $pdisks[$i]{PD_POSTSTAT},
            $pdisks[$i]{PD_DEVSTAT},
            $pdisks[$i]{PD_FLED},
            $pdisks[$i]{PCTREM},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $pdisks[$i]{PD_QD},
            $pdisks[$i]{PD_RPS},

            $pdisks[$i]{PD_AVGSC},
            $pdisks[$i]{PD_SSERIAL},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $pdisks[$i]{VENDID},

            $pdisks[$i]{PD_REV},
            $pdisks[$i]{PD_ERR},
            $pdisks[$i]{PS_PRODID},
            $pdisks[$i]{PS_SERIAL},
            $pdisks[$i]{PD_DAML},
            $tas{LO_LONG}, $tas{HI_LONG},

            $las{LO_LONG}, $las{HI_LONG},
            $wwn{LO_LONG}, $wwn{HI_LONG},

            $pdisks[$i]{R10_MISCOMP},
            $pdisks[$i]{PD_DNAME},
            $pdisks[$i]{LFCNT},
            $pdisks[$i]{LSCNT},

            $pdisks[$i]{LGCNT},
            $pdisks[$i]{PSCNT},
            $pdisks[$i]{ITCNT},
            $pdisks[$i]{ICCNT},

            $pdisks[$i]{MISCOMP},
            $pdisks[$i]{DEVMISCOMP},
            $rbtotal{LO_LONG}, $rbtotal{HI_LONG},
            $rbremain{LO_LONG}, $rbremain{HI_LONG},

            $pdisks[$i]{SES},
            $pdisks[$i]{SLOT},
            $rsvd,
            $pdisks[$i]{HSDNAME},
# GR_GEORAID - Following is added for GEORaid
            $pdisks[$i]{GL_ID},
            $pdisks[$i]{GEOFLAGS},
            $pdisks[$i]{HANGCNT},
            $rsvd,
            $pdisks[$i]{LASTLIP}
            ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL 
                        LLNN LLLL LLLL LLLL LLSCCLCCCCL", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $pdisks[$i]{CAPACITY} = longsToBigInt(%capacity);
            $pdisks[$i]{RREQ} = longsToBigInt(%rreq);
            $pdisks[$i]{WREQ} = longsToBigInt(%wreq);
            $pdisks[$i]{TAS} = longsToBigInt(%tas);
            $pdisks[$i]{LAS} = longsToBigInt(%las);
            $pdisks[$i]{WWN} = longsToBigInt(%wwn);
            $pdisks[$i]{WWN_HI} = $wwn{HI_LONG};
            $pdisks[$i]{WWN_LO} = $wwn{LO_LONG};
            $pdisks[$i]{RBTOTAL} = longsToBigInt(%rbtotal);
            $pdisks[$i]{RBREMAIN} = longsToBigInt(%rbremain);
            
        }

        $info{PDISKS} = [@pdisks];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a physical disk info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _physicalDiskInfoPacket
#
# Desc:     Parses the physical disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       DISK_ID                 Physical disk id
#       NAME                    Name of disk
#       CLASS                   Values
#                               0 = Unlabeled
#                               1 = Labeled device with redundancy
#                               2 = hotspare
#
#       SYSTEM_SERIAL_NUMBER    San box serial number   (perl BigInt)
#       CURRENT_POST_STATUS
#       DRIVE_STATUS            Values
#                               0 = Nonexistent device
#                               1 = Operational drive
#                               2 = Inoperative drive
#       CAPACITY                Capacity in bytes (perl BigInt)
#       PRODUCT_ID
#       VENDER_ID
#       REVISION
#       SERIAL_NUMBER
#       AVAILABLE_SPACE         In bytes (perl BigInt)
#       LARGEST_AVAIL_SEG       Largest available segment
#       WWN                     Disk WWN    (perl BigInt)
#       LUN
#       CHANNEL
#       LOOPID
#       CONTAINER_ID
##############################################################################
sub _physicalDiskInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        # Temp values for 64 values
        my %capacity;
        my %rreq;
        my %wreq;
        my %tas;
        my %las;
        my %wwn;
        my %rbtotal;
        my %rbremain;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{PD_CLASS},
        $info{PD_DEVTYPE},
        $info{PD_MISCSTAT},
        $info{PD_FLAGS},
        $info{PD_CHANNEL},
        $info{PD_LOOPMAP},
        $info{PD_LUN},

        $info{PD_ID},
        $info{PD_DEV},
        $info{PD_PID},
        $rsvd,
        $info{PD_POSTSTAT},
        $info{PD_DEVSTAT},
        $info{PD_FLED},
        $info{PCTREM},

        $capacity{LO_LONG}, $capacity{HI_LONG},
        $info{PD_QD},
        $info{PD_RPS},

        $info{PD_AVGSC},
        $info{PD_SSERIAL},
        $rreq{LO_LONG}, $rreq{HI_LONG},

        $wreq{LO_LONG}, $wreq{HI_LONG},
        $info{VENDID},

        $info{PD_REV},
        $info{PD_ERR},
        $info{PS_PRODID},
        $info{PS_SERIAL},
        $info{PD_DAML},
        $tas{LO_LONG}, $tas{HI_LONG},

        $las{LO_LONG}, $las{HI_LONG},
        $wwn{LO_LONG}, $wwn{HI_LONG},

        $info{R10_MISCOMP},
        $info{PD_DNAME},
        $info{LFCNT},
        $info{LSCNT},

        $info{LGCNT},
        $info{PSCNT},
        $info{ITCNT},
        $info{ICCNT},

        $info{MISCOMP},
        $info{DEVMISCOMP},
        $rbtotal{LO_LONG}, $rbtotal{HI_LONG},

        $rbremain{LO_LONG}, $rbremain{HI_LONG},
        $info{SES},
        $info{SLOT},
        $rsvd,
        $info{HSDNAME},
# GR_GEORAID - Following is added for GEORaid
        $info{GL_ID},
        $info{GEOFLAGS},
        $info{HANGCNT},
        $rsvd,
        $info{LASTLIP}
        ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL
                    LLNN LLLL LLLL LLLL LLSCCLCCCCL", $parts{DATA});

        # Now fixup all the 64 bit numbers
        $info{CAPACITY} = longsToBigInt(%capacity);
        $info{RREQ} = longsToBigInt(%rreq);
        $info{WREQ} = longsToBigInt(%wreq);
        $info{TAS} = longsToBigInt(%tas);
        $info{LAS} = longsToBigInt(%las);
        $info{WWN} = longsToBigInt(%wwn);
        $info{WWN_HI} = $wwn{HI_LONG};
        $info{WWN_LO} = $wwn{LO_LONG};
        $info{RBTOTAL} = longsToBigInt(%rbtotal);
        $info{RBREMAIN} = longsToBigInt(%rbremain);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a physical disk info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _pDiskDefragStatusPacket
#
# Desc:     Parses the physical disk defrag status packet and places the 
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
##############################################################################
sub _pDiskDefragStatusPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_DEFRAG_STATUS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        $info{STATUS_MRP} = $parts{STATUS};     # Make the same.
        $info{LEN} = $parts{PAYLOAD_LENGTH};    # Make the same.

        # Unpack the data
        (
        $info{PDISKID},
        $info{FLAGS},
        $rsvd
        ) = unpack("SCC", $parts{DATA});

    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a PDisk Defrag Status packet\n");
    }

    return %info;
}

#SERVICEABILITY42
##############################################################################
# Name:     _pDiskAutoFailbackPacket
#
# Desc:     Parses the physical disk autofailback response packet and places the 
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
##############################################################################
sub _pDiskAutoFailbackPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        # Temp values for 64 values

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};
        $info{STATUS_MRP} = $parts{STATUS};     # Make the same.
        $info{LEN} = $parts{PAYLOAD_LENGTH};    # Make the same.

        # Unpack the data
        (
        $info{MODE},
        $rsvd
        ) = unpack("Ca3", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a PDisk AutoFailback Response packet\n");
    }

    return %info;
}
#SERVICEABILITY42


##############################################################################
# Name: _getString_CLASS
#
# Desc: Gets a string with detailed information given a CLASS number
#       (device class number).
#
# In:   Device Class
#
##############################################################################
sub _getString_CLASS
{
    my ($class) = @_;
    my $fmt;

    if ($class == 0)
    {
        $fmt = "0x%x - UNLABELED";
    }
    elsif ($class == 1)
    {
        $fmt = "0x%x - DATA";
    }
    elsif ($class == 2)
    {
        $fmt = "0x%x - HOTSPARE";
    }
    elsif ($class == 3)
    {
        $fmt = "0x%x - UNSAFE";
    }
    elsif ($class == 4)
    {
        $fmt = "0x%x - MAG LINKED DEVIVCE";
    }
    elsif ($class == 5)
    {
        $fmt = "0x%x - FOREIGN TARGET";
    }
    else
    {
        $fmt = "0x%x";
    }

    return sprintf $fmt, $class;
}

##############################################################################
# Name: _getString_PDDT
#
# Desc: Gets a string with detailed information given a CLASS number
#       (device class number).
#
# In:   Device Type
#
##############################################################################
sub _getString_PDDT
{
    my ($type) = @_;
    my $strType;

    #
    # Based on the device type, return the correct string value.
    # NOTE: The string must be less than 8 characters unless
    #       other code changes to support more than that limit.
    #
    if ($type == PD_DT_FC_DISK)
    {
        $strType = "FC";
    }
    elsif ($type == PD_DT_SATA)
    {
        $strType = "SATA";
    }
    elsif ($type == PD_DT_SSD)
    {
        $strType = "SSD";
    }
    elsif ($type == PD_DT_SAS)
    {
        $strType = "SAS";
    }
    elsif ($type == PD_DT_ECON_ENT)
    {
        $strType = "ECON ENT";
    }
    elsif ($type == PD_DT_FC_SES)
    {
        $strType = "FC SES";
    }
    elsif ($type == PD_DT_SATA_SES)
    {
        $strType = "SATA SES";
    }
    elsif ($type == PD_DT_SBOD_SES)
    {
        $strType = "SBOD SES";
    }
    elsif ($type == PD_DT_SBOD_SAS_EXP)
    {
        $strType = "SBOD SAS EXP";
    }
    elsif ($type == PD_DT_ISE_SES)
    {
        $strType = "ISE";
    }
    elsif ($type == PD_DT_ISE_HIGH_PERF)
    {
        $strType = "FC HIGH";
    }
    elsif ($type == PD_DT_ISE_PERF)
    {
        $strType = "FC PERF";
    }
    elsif ($type == PD_DT_ISE_BALANCE)
    {
        $strType = "FC BAL";
    }
    elsif ($type == PD_DT_ISE_CAPACITY)
    {
        $strType = "FC CAP";
    }
    else
    {
        $strType = "UNKNOWN";
    }

    return $strType;
}

##############################################################################
# Name: _getString_DNAME
#
# Desc: Gets a string with detailed information given a DNAME (device name).
#
# In:   Device Name
#
##############################################################################
sub _getString_DNAME
{
    my ($controller_type,$dname) = @_;
    my $fmt;
    my $bay;
    my $slot;

    if (!defined($controller_type))
    {
        $controller_type = CTRL_TYPE_7000;
    }
    # For names that have 'PD' output the bay and slot
    if (($dname & 0x0000FFFF) == 0x00004450)
    {

        $slot = ($dname & 0xFF000000) >> 24;
        $bay = (($dname & 0x00FF0000) >> 16);

        if ($controller_type == CTRL_TYPE_7000 || $controller_type == CTRL_TYPE_4700)
        {
            $fmt = sprintf(' PD:%02d-%%02d ', $bay);
        }
        elsif ($bay >= 0 && $bay < (26*2))
        {
            $fmt = sprintf('  PD-%s%%02d  ', substr('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz', $bay, 1));
        }
        else
        {
            # Output the name in hex if the bay is invalid - shouldn't happen
            $fmt = "0x%8.8x";
            return sprintf $fmt, $dname;
        }

        return sprintf $fmt, $slot;
    } 

    # For names without a 'PD' output the name in hex
    else
    {
        $fmt = "0x%8.8x";
        return sprintf $fmt, $dname;
    } 

}

##############################################################################
# Name: _getString_DNAME2
#
# Desc: Gets a string with detailed information given a DNAME (device name).
#       NOTE: special version that just returns '   -   ' for empties
#
# In:   Device Name
#
##############################################################################
sub _getString_DNAME2
{
    my ($controller_type,$dname) = @_;
    my $fmt;
    my $bay;
    my $slot;

    # For names that have 'PD' output the bay and slot
    if (($dname & 0x0000FFFF) == 0x00004450)
    {

        $slot = ($dname & 0xFF000000) >> 24;
        $bay = (($dname & 0x00FF0000) >> 16);

        if ($controller_type == CTRL_TYPE_7000 || $controller_type == CTRL_TYPE_4700)
        {
            $fmt = sprintf(' PD:%02d-%%02d ', $bay);
        }
        elsif ($bay >= 0 && $bay < (26*2))
        {
            $fmt = sprintf('  PD-%s%%02d  ', substr('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz', $bay, 1));
        }
        else
        {
            # Output the name in hex if the bay is invalid - shouldn't happen
            $fmt = "0x%8.8x";
            return sprintf $fmt, $dname;
        }

        return sprintf $fmt, $slot;
    } 

    # For names without a 'PD' output the name in hex
    else
    {
        if($dname == 0x00000000)
        {
          return '    -    ';
        }
        $fmt = "0x%8.8x";
        return sprintf $fmt, $dname;
    } 

}

##############################################################################
# Name: _getALPA
#
# Desc: Gets an ALPA based upon the LID value.
#
# In:   LID value
#
##############################################################################
sub _getALPA
{
    my ($lid) = @_;
    my $alpa = 0;

    my @lid_to_alpa = (
        0xef,0xe8,0xe4,0xe2,0xe1,0xe0,0xdc,0xda,
        0xd9,0xd6,0xd5,0xd4,0xd3,0xd2,0xd1,0xce,
        0xcd,0xcc,0xcb,0xca,0xc9,0xc7,0xc6,0xc5,
        0xc3,0xbc,0xba,0xb9,0xb6,0xb5,0xb4,0xb3,
        0xb2,0xb1,0xae,0xad,0xac,0xab,0xaa,0xa9,
        0xa7,0xa6,0xa5,0xa3,0x9f,0x9e,0x9d,0x9b,
        0x98,0x97,0x90,0x8f,0x88,0x84,0x82,0x81,
        0x80,0x7c,0x7a,0x79,0x76,0x75,0x74,0x73,
        0x72,0x71,0x6e,0x6d,0x6c,0x6b,0x6a,0x69,
        0x67,0x66,0x65,0x63,0x5c,0x5a,0x59,0x56,
        0x55,0x54,0x53,0x52,0x51,0x4e,0x4d,0x4c,
        0x4b,0x4a,0x49,0x47,0x46,0x45,0x43,0x3c,
        0x3a,0x39,0x36,0x35,0x34,0x33,0x32,0x31,
        0x2e,0x2d,0x2c,0x2b,0x2a,0x29,0x27,0x26,
        0x25,0x23,0x1f,0x1e,0x1d,0x1b,0x18,0x17,
        0x10,0x0f,0x08,0x04,0x02,0x01,0x00,0xff);

    if ($lid < 0x80)
    {
        $alpa = $lid_to_alpa[$lid];
    }

    return $alpa;
}

##############################################################################
# Name:     pdiskEmulateQlogicTimeout
#
# Desc:     Emulate Qlogic timeout for given pdisk.
#
# In:       ID of the physical disk
#
# Returns:  Nothing.
##############################################################################
sub pdiskEmulateQlogicTimeout
{
    my ($self, $pid, $setting) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0x0001],
                ["pdiskEmulateQlogicTimeout"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_PDISKS_QLOGIC_TIMEOUT_EMULATE;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SC", $pid, $setting);

    my $packet = assembleXiotechPacket($cmd, $seq, $ts, $data, $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericResponsePacket);
}

##############################################################################
1;

#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
