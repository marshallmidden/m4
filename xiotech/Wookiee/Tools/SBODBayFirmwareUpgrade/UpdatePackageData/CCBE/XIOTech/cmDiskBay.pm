# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Bryan Holty
#
# Purpose:
#   Wrapper for all the different XIOTech commands that can be sent
#   to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use strict;


#GR_GEORAID
##############################################################################
# Name:     setBayGeoLocation
#
# Desc:     Sets the Geo location of the specified disk bay.
#
# In:       Bay ID
#           Location ID
#
# Returns:
##############################################################################
sub setBayGeoLocation
{
    my ($self, $bid, $locationId) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ["setGeoLocation"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SET_GEO_LOCATION_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCS",
                    $bid,
                    $locationId,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
#                                        \&_setGeoLocationPacket);
}
#GR_GEORAID

#GR_GEORAID
##############################################################################
# Name:     clearGeoLocation
#
# Desc:     Clears the Geo location of all the bays and its associated
#           disks.
#
# In:       NONE 
#
# Returns:
##############################################################################
sub clearGeoLocation
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["clearGeoLocation"]];

    my %vp = verifyParameters(\@_, $args);

    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_CLEAR_GEO_LOCATION_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
#                                        \&_clearGeoLocationPacket);
}


##############################################################################
# Name:     diskBays
#
# Desc:     Retrieves disk bay information for all disk bays.
#
# In:       NONE
#
# Returns:
##############################################################################
sub diskBays
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["diskBays"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DISK_BAYS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_diskBaysPacket);
}

#GR_GEORAID
##############################################################################
# Name:     geoLocationStatus
#
# Desc:     display geo location info of the bay(s) and associated PDisks.
#
# In:       NONE
#
# Returns:
##############################################################################
sub geoLocationStatus
{
    my ($self) = @_;

    my %bays;
    my %disks;
    my @sortBays;
    my @sortDisks;
    my @pdisks;
    my @dbays;
    print "\n";

    %bays = $self->diskBays();

    if (%bays)
    {
        if ($bays{STATUS} == PI_GOOD)
        {
            %disks = $self->physicalDisks();

            if (%disks)
            {
                if ($disks{STATUS} == PI_GOOD)
                {
                    my $i;
                    my $j;

                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {

                        $dbays[$i]{PD_PID} = $bays{BAYS}[$i]{PD_BID};
                        $dbays[$i]{WWN_HI} = $bays{BAYS}[$i]{WWN_HI};
                        $dbays[$i]{WWN_LO} = $bays{BAYS}[$i]{WWN_LO};
                        $dbays[$i]{PD_REV} = $bays{BAYS}[$i]{PD_REV};
                        my $count = 0;
                        for ($j = 0; $j < $disks{COUNT}; ++$j)
                        {
                            if($bays{BAYS}[$i]{PD_BID} == $disks{PDISKS}[$j]{SES})
                            {

                                $pdisks[$i][$count]{PD_PID} = $disks{PDISKS}[$j]{PD_PID};
                                $pdisks[$i][$count]{WWN_HI} = $disks{PDISKS}[$j]{WWN_HI};
                                $pdisks[$i][$count]{WWN_LO} = $disks{PDISKS}[$j]{WWN_LO};
                                $pdisks[$i][$count]{SLOT}   = $disks{PDISKS}[$j]{SLOT};
# GR_GEORAID - Following is added for GeoRaid
                                $pdisks[$i][$count]{GL_ID}   = $disks{PDISKS}[$j]{GL_ID};
                                $count++;
                            }
                        }

                        $dbays[$i]{PD_COUNT} = $count;
                        $dbays[$i]{PDISKS} = $pdisks[$i];
                    }

                    my $i2;
                    my $j2 = -1;
                    my $loc = 0;
                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {
                        $i2 = 9999999;
                        for ($j = 0; $j < $bays{COUNT}; ++$j)
                        {
                            if($dbays[$j]{PD_PID} < $i2 && $dbays[$j]{PD_PID} > $j2)
                            {
                                $loc = $j;
                                $i2 = $dbays[$j]{PD_PID};
                            }
                        }
                        $sortBays[$i] = $dbays[$loc];
                        $j2 = $i2;

                        my $k;
                        my $k1;
                        my $l;
                        my $l1 = -1;
                        my $loc1;
                        for ($k = 0; $k < $sortBays[$i]{PD_COUNT}; ++$k)
                        {
                            $k1 = 9999999;
                            for ($l = 0; $l < $sortBays[$i]{PD_COUNT}; ++$l)
                            {
                                if($sortBays[$i]{PDISKS}[$l]{SLOT} < $k1 && $sortBays[$i]{PDISKS}[$l]{SLOT} > $l1)
                                {
                                    $loc1 = $l;
                                    $k1 = $sortBays[$i]{PDISKS}[$l]{SLOT};
                                }
                            }
                            $pdisks[$k] = $sortBays[$i]{PDISKS}[$loc1];
                            $l1 = $k1;
                        }
                        $sortBays[$i]{PDISKS} = [@pdisks];
                    }

                    printf " BayID       BayWWN      BayRev  PDiskSlot    PDiskID       PDiskWWN         PDiskGeoLocation\n";
                    printf "------- ---------------- ------ ----------- ----------- ------------------ --------------------\n";

                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {
                        printf " %3d    %8.8x%8.8x  %3s", $sortBays[$i]{PD_PID},
                                                          $sortBays[$i]{WWN_LO},
                                                          $sortBays[$i]{WWN_HI},
                                                          $sortBays[$i]{PD_REV};

                        for ($j = 0; $j < $sortBays[$i]{PD_COUNT}; ++$j)
                        {
                            if ($j > 0)
                            {
                                printf "                              ";
                            }
                            printf "     %3d         %3d       %8.8x%8.8x          %3d\n", $sortBays[$i]{PDISKS}[$j]{SLOT},
                                                                     $sortBays[$i]{PDISKS}[$j]{PD_PID},
                                                                     $sortBays[$i]{PDISKS}[$j]{WWN_LO},
                                                                     $sortBays[$i]{PDISKS}[$j]{WWN_HI},
                                                                     $sortBays[$i]{PDISKS}[$j]{GL_ID};
                        }
                        printf "\n";
                    }
                }
                else
                {
                    return %disks;
                }
            }
            else
            {
                return;
            }

        }
        else
        {
            return %bays;
        }
    }
    else
    {
        return;
    }

    return %bays;
}



##############################################################################
# Name:     diskBayStatus
#
# Desc:     display diskbay Status.
#
# In:       NONE
#
# Returns:
##############################################################################
sub diskBayStatus
{
    my ($self) = @_;

    my %bays;
    my %disks;
    my @sortBays;
    my @sortDisks;
    my @pdisks;
    my @dbays;
    print "\n";

    %bays = $self->diskBays();

    if (%bays)
    {
        if ($bays{STATUS} == PI_GOOD)
        {
            %disks = $self->physicalDisks();

            if (%disks)
            {
                if ($disks{STATUS} == PI_GOOD)
                {
                    my $i;
                    my $j;
                
                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {
                        
                        $dbays[$i]{PD_PID} = $bays{BAYS}[$i]{PD_BID};
                        $dbays[$i]{WWN_HI} = $bays{BAYS}[$i]{WWN_HI};
                        $dbays[$i]{WWN_LO} = $bays{BAYS}[$i]{WWN_LO};
                        $dbays[$i]{PD_REV} = $bays{BAYS}[$i]{PD_REV};
                        my $count = 0;
                        for ($j = 0; $j < $disks{COUNT}; ++$j)
                        {
                            if($bays{BAYS}[$i]{PD_BID} == $disks{PDISKS}[$j]{SES})
                            {

                                $pdisks[$i][$count]{PD_PID} = $disks{PDISKS}[$j]{PD_PID};
                                $pdisks[$i][$count]{WWN_HI} = $disks{PDISKS}[$j]{WWN_HI};
                                $pdisks[$i][$count]{WWN_LO} = $disks{PDISKS}[$j]{WWN_LO};
                                $pdisks[$i][$count]{SLOT}   = $disks{PDISKS}[$j]{SLOT};
#GR_GEORAID - Following is added for GEORaid
                                $pdisks[$i][$count]{GL_ID}   = $disks{PDISKS}[$j]{GL_ID};
          
                                $count++;
                            }
                        }

                        $dbays[$i]{PD_COUNT} = $count;
                        $dbays[$i]{PDISKS} = $pdisks[$i];
                    }
                    
                    my $i2;
                    my $j2 = -1;
                    my $loc = 0;
                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {
                        $i2 = 9999999;
                        for ($j = 0; $j < $bays{COUNT}; ++$j)
                        {
                            if($dbays[$j]{PD_PID} < $i2 && $dbays[$j]{PD_PID} > $j2)
                            {
                                $loc = $j;
                                $i2 = $dbays[$j]{PD_PID};
                            }
                        }
                        $sortBays[$i] = $dbays[$loc];
                        $j2 = $i2;
                        
                        my $k;
                        my $k1;
                        my $l;
                        my $l1 = -1;
                        my $loc1;
                        for ($k = 0; $k < $sortBays[$i]{PD_COUNT}; ++$k)
                        {
                            $k1 = 9999999;
                            for ($l = 0; $l < $sortBays[$i]{PD_COUNT}; ++$l)
                            {
                                if($sortBays[$i]{PDISKS}[$l]{SLOT} < $k1 && $sortBays[$i]{PDISKS}[$l]{SLOT} > $l1)
                                {
                                    $loc1 = $l;
                                    $k1 = $sortBays[$i]{PDISKS}[$l]{SLOT};
                                }
                            }
                            $pdisks[$k] = $sortBays[$i]{PDISKS}[$loc1];
                            $l1 = $k1;
                        }
                        $sortBays[$i]{PDISKS} = [@pdisks];
                    }
                    
                    printf " BayID       BayWWN      BayRev  PDiskSlot    PDiskID       PDiskWWN        PDiskLocation\n";
                    printf "------- ---------------- ------ ----------- ----------- ------------------ ----------------\n";
                    
                    for ($i = 0; $i < $bays{COUNT}; ++$i)
                    {
                        printf " %3d    %8.8x%8.8x  %3s", $sortBays[$i]{PD_PID}, 
                                                          $sortBays[$i]{WWN_LO},
                                                          $sortBays[$i]{WWN_HI},
                                                          $sortBays[$i]{PD_REV};
                    
                        for ($j = 0; $j < $sortBays[$i]{PD_COUNT}; ++$j)
                        {
                            if ($j > 0)
                            {
                                printf "                              ";
                            }
                            printf "     %3d         %3d       %8.8x%8.8x       %3d\n", $sortBays[$i]{PDISKS}[$j]{SLOT},
                                                                     $sortBays[$i]{PDISKS}[$j]{PD_PID},
                                                                     $sortBays[$i]{PDISKS}[$j]{WWN_LO},
                                                                     $sortBays[$i]{PDISKS}[$j]{WWN_HI},
#GR_GEORAID - Following is added for GEORaid
                                                                     $sortBays[$i]{PDISKS}[$j]{GL_ID};
                        }
                        printf "\n";
                    }
                }
                else
                {
                    return %disks;
                }
            }
            else
            {
                return;
            }    
            
        }
        else
        {
            return %bays;
        }
    }
    else
    {
        return;
    }
    
    return %bays;
}

##############################################################################
# Name:     DiskBayCount
#
# Desc:     Retrieves the number of disk bays.
#
# Input:    None
#
# Returns:
##############################################################################
sub diskBayCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_DISK_BAY_COUNT_CMD);
}

##############################################################################
# Name:     DiskBayInfo
#
# Desc:     Get information about a disk bay
#
# In:       ID of the disk bay
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       BAY_ID                  disk bay id
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
sub diskBayInfo
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["physicalDiskBayInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DISK_BAY_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $pid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_diskBayInfoPacket);
}

##############################################################################
# Name:     diskBayList
#
# Desc:     Retrieves an array containing the identifiers of the disk bays.
#
# Input:    None
#
# Returns:  List of physical disk identifiers or UNDEF if an error occurred.
##############################################################################
sub diskBayList
{
    my ($self) = @_;
    return $self->getObjectList(PI_DISK_BAY_LIST_CMD);
}

##############################################################################
# Name:     diskBayAlarmControl
#
# Desc:     Sends an alarm control byte to a disk Bay
#
# In:       bid      - ID of the disk bay.
#           settings - alarm control settings
#
# Returns:
##############################################################################
sub diskBayAlarmControl
{
    my ($self, $bid, $settings) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['i', 0, 0xFF],
                ["diskBayAlarmControl"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DISK_BAY_ALARM_CTRL_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCC",
                    $bid,
                    $settings,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_genericResponsePacket);
}


##############################################################################
# Name:     diskBayDelete
#
# Desc:     deletes  a disk bay.
#
#           bid   -  Device ID
#
# Return:   Status Response Hash
##############################################################################
sub diskBayDelete
{
    my ($self, $bid) = @_;
    
    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFF],
                ["diskBayDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DISK_BAY_DELETE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCS",
                    DELETE_DISK_BAY,
                    0,
                    $bid);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT},
                                        VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name: displayDiskBays
#
# Desc: Print the disk bays
#
# In:   Disk Bays Information Hash
##############################################################################
sub displayDiskBays
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "Disk Bays ($info{COUNT} bays):\n";
    $msg .= sprintf  "\n";

    $msg .= sprintf "  BID  Port   Vendor     Type       Product ID     Revision    Serial #    Shelf ID        WWN         GEOLOCATION \n";
    $msg .= sprintf "  ---  ----  --------  --------  ----------------  --------  ------------  --------  ----------------  ------------\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf "  %3hu    %2d  %8s  %-8s  %16s  %8s  %12s      0x%02x  %8.8x%8.8x  %12d\n", 
                $info{BAYS}[$i]{PD_BID},
                $info{BAYS}[$i]{PD_CHANNEL},
                $info{BAYS}[$i]{VENDID},
                _getString_PDDT($info{BAYS}[$i]{PD_DEVTYPE}),
                $info{BAYS}[$i]{PS_PRODID},
                $info{BAYS}[$i]{PD_REV},
                $info{BAYS}[$i]{PS_SERIAL},
                $info{BAYS}[$i]{SLOT},
                $info{BAYS}[$i]{WWN_LO},
                $info{BAYS}[$i]{WWN_HI},
# GR_GEORAID - Following is added for GEORaid
                $info{BAYS}[$i]{GL_ID};
    }

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name: displayDiskBayInfo
#
# Desc: Print the physical disk bay information
#
# In:   Physical Disk Bay Information Hash
##############################################################################
sub displayDiskBayInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Physical Disk Bay Information:\n";
    $msg .= sprintf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                   %lu\n", $info{LEN};
    $msg .= sprintf "  PD_CLASS:              %s\n", _getString_CLASS($info{PD_CLASS});
    $msg .= sprintf "  PD_DEVTYPE:            0x%x - %s\n", $info{PD_DEVTYPE}, _getString_PDDT($info{PD_DEVTYPE});
    $msg .= sprintf "  PD_MISCSTAT:           0x%x\n", $info{PD_MISCSTAT};
    $msg .= sprintf "  PD_PORT:               %hu\n", $info{PD_CHANNEL};
    $msg .= sprintf "  PD_LOOPMAP:            %hu\n", $info{PD_LOOPMAP};
    $msg .= sprintf "  PD_LUN:                %hu\n", $info{PD_LUN};

    $msg .= sprintf "  PD_LID:                %lu\n", $info{PD_ID};
    $msg .= sprintf "  PD_DEV:                0x%x\n", $info{PD_DEV};
    $msg .= sprintf "  PD_BID:                %hu\n", $info{PD_BID};
    $msg .= sprintf "  PD_POSTSTAT:           0x%x\n", $info{PD_POSTSTAT};
    $msg .= sprintf "  PD_DEVSTAT:            0x%x\n", $info{PD_DEVSTAT};
    $msg .= sprintf "  PD_FLED:               0x%x\n", $info{PD_FLED};
    $msg .= sprintf "  PCTREM:                %hu\n", $info{PCTREM};

    $msg .= sprintf "  CAPACITY:              $info{CAPACITY}\n";
    $msg .= sprintf "  PD_QD:                 %lu\n", $info{PD_QD};
    $msg .= sprintf "  PD_RPS:                %lu\n", $info{PD_RPS};

    $msg .= sprintf "  PD_AVGSC:              %lu\n", $info{PD_AVGSC};
    $msg .= sprintf "  PD_SSERIAL:            %lu\n", $info{PD_SSERIAL};
    $msg .= sprintf "  RREQ:                  $info{RREQ}\n";

    $msg .= sprintf "  WREQ:                  $info{WREQ}\n";
    $msg .= sprintf "  VENDID:                $info{VENDID}\n";

    $msg .= sprintf "  PD_REV:                $info{PD_REV}\n";
    $msg .= sprintf "  PD_ERR:                %lu\n", $info{PD_ERR};
    $msg .= sprintf "  PD_PRODID:             $info{PS_PRODID}\n";
    $msg .= sprintf "  PS_SERIAL:             $info{PS_SERIAL}\n";
    $msg .= sprintf "  PD_DAML:               0x%x\n", $info{PD_DAML};
    $msg .= sprintf "  TAS:                   $info{TAS}\n";

    $msg .= sprintf "  LAS:                   $info{LAS}\n";
    $msg .= sprintf "  WWN:                   %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};

    $msg .= sprintf "  R10_MISCOMP:           %lu\n", $info{R10_MISCOMP};
    $msg .= sprintf "  PD_DNAME:              0x%x\n", $info{PD_DNAME};
    $msg .= sprintf "  LFCNT:                 %lu\n", $info{LFCNT};
    $msg .= sprintf "  LSCNT:                 %lu\n", $info{LSCNT};

    $msg .= sprintf "  LGCNT:                 %lu\n", $info{LGCNT};
    $msg .= sprintf "  PSCNT:                 %lu\n", $info{PSCNT};
    $msg .= sprintf "  ITCNT:                 %lu\n", $info{ITCNT};
    $msg .= sprintf "  ICCNT:                 %lu\n", $info{ICCNT};

    $msg .= sprintf "  MISCOMP:               %lu\n", $info{MISCOMP};
    $msg .= sprintf "  DEVMISCOMP:            %lu\n", $info{DEVMISCOMP};
    $msg .= sprintf "  RBTOTAL:               $info{RBTOTAL}\n";

    $msg .= sprintf "  RBREMAIN:              $info{RBREMAIN}\n";
    $msg .= sprintf "  SES:                   $info{SES}\n";
    $msg .= sprintf "  SLOT:                  $info{SLOT}\n";
# GR_GEORAID - Following is added for GEORaid
    $msg .= sprintf "  LOCATION:              $info{GL_ID}\n";

    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _diskBaysPacket
#
# Desc:     Parses the disk bays packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
##############################################################################
sub _diskBaysPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_DISK_BAYS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @bays;

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
            $bays[$i]{STATUS_MRP},
            $bays[$i]{LEN},
            $bays[$i]{PD_CLASS},
            $bays[$i]{PD_DEVTYPE},
            $bays[$i]{PD_MISCSTAT},
            $rsvd,
            $bays[$i]{PD_CHANNEL},
            $bays[$i]{PD_LOOPMAP},
            $bays[$i]{PD_LUN},
            $bays[$i]{PD_ID},

            $bays[$i]{PD_DEV},
            $bays[$i]{PD_BID},
            $rsvd,
            $bays[$i]{PD_POSTSTAT},
            $bays[$i]{PD_DEVSTAT},
            $bays[$i]{PD_FLED},
            $bays[$i]{PCTREM},

            $capacity{LO_LONG}, $capacity{HI_LONG},
            $bays[$i]{PD_QD},
            $bays[$i]{PD_RPS},

            $bays[$i]{PD_AVGSC},
            $bays[$i]{PD_SSERIAL},
            $rreq{LO_LONG}, $rreq{HI_LONG},

            $wreq{LO_LONG}, $wreq{HI_LONG},
            $bays[$i]{VENDID},

            $bays[$i]{PD_REV},
            $bays[$i]{PD_ERR},
            $bays[$i]{PS_PRODID},
            $bays[$i]{PS_SERIAL},
            $bays[$i]{PD_DAML},
            $tas{LO_LONG}, $tas{HI_LONG},

            $las{LO_LONG}, $las{HI_LONG},
            $wwn{LO_LONG}, $wwn{HI_LONG},

            $bays[$i]{R10_MISCOMP},
            $bays[$i]{PD_DNAME},
            $bays[$i]{LFCNT},
            $bays[$i]{LSCNT},

            $bays[$i]{LGCNT},
            $bays[$i]{PSCNT},
            $bays[$i]{ITCNT},
            $bays[$i]{ICCNT},

            $bays[$i]{MISCOMP},
            $bays[$i]{DEVMISCOMP},
            $rbtotal{LO_LONG}, $rbtotal{HI_LONG},
            $rbremain{LO_LONG}, $rbremain{HI_LONG},

            $bays[$i]{SES},
            $bays[$i]{SLOT},
#GR_GEORAID - Following is added for GEORaid
            $rsvd,
#GR_GEORAID - Following is added for GEORaid
            $bays[$i]{GL_ID},
            $rsvd
#GR_GEORAID - Following is added for GEORaid
            ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL 
                        LLNN LLLL LLLL LLLL LLSCa5Ca7", substr($parts{DATA}, $start));

#            ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL 
#                        LLNN LLLL LLLL LLLL LLSCCa13", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $bays[$i]{CAPACITY} = longsToBigInt(%capacity);
            $bays[$i]{RREQ} = longsToBigInt(%rreq);
            $bays[$i]{WREQ} = longsToBigInt(%wreq);
            $bays[$i]{TAS} = longsToBigInt(%tas);
            $bays[$i]{LAS} = longsToBigInt(%las);
            $bays[$i]{WWN} = longsToBigInt(%wwn);
            $bays[$i]{WWN_HI} = $wwn{HI_LONG};
            $bays[$i]{WWN_LO} = $wwn{LO_LONG};
            $bays[$i]{RBTOTAL} = longsToBigInt(%rbtotal);
            $bays[$i]{RBREMAIN} = longsToBigInt(%rbremain);
        }

        $info{BAYS} = [@bays];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a disk bays info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _diskBayInfoPacket
#
# Desc:     Parses the disk bay info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       BAY_ID                  disk bay id
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
sub _diskBayInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_DISK_BAY_INFO_CMD)
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
        $rsvd,
        $info{PD_CHANNEL},
        $info{PD_LOOPMAP},
        $info{PD_LUN},

        $info{PD_ID},
        $info{PD_DEV},
        $info{PD_BID},
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
# GR_GEORAID - Follwing is added for GEORaid
        $rsvd,
# GR_GEORAID - Follwing is added for GEORaid
        $info{GL_ID},
        $rsvd
# GR_GEORAID - Follwing is added for GEORaid
        ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL
                    LLNN LLLL LLLL LLLL LLSCa5Ca7", $parts{DATA});

#        ) = unpack("a3CLCCCCCCS LLSSCCCC LLLL LLLL LLa8 a4La16a12LLL
#                    LLNN LLLL LLLL LLLL LLSCCa13", $parts{DATA});

        # Now fixup all the 64 bit  numbers
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
        logMsg("Unexpected packet: We expected a disk bay info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     DiskBayEnviro
#
# Desc:     Get Environmental (SES) information about a disk bay
#
# In:       ID of the disk bay
#
# Returns:  An empty hash on error, else returns a hash with data.
##############################################################################
sub diskBayEnviro
{
    my ($self, $pid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'], ['d', 0, 0xFFFF], ["physicalDiskBayEnvironmental"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_ENVIRO_DATA_DISK_BAY_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS", $pid, 0);

    my $packet = assembleXiotechPacket($cmd, $seq, $ts, $data, $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_diskBayEnviroPacket);
}

##############################################################################
# Display %parts, for debugging.
##############################################################################
sub display_parts($)
{
    my $msg = '';
    my ($parts) = @_;
    foreach my $i (sort keys(%$parts))
    {
        if ($i eq 'HEADER_MD5') {next};
        if ($i eq 'DATA_MD5') {next};
        if (defined($$parts{$i}))
        {
            if ($i eq 'DATA')
            {
                $msg .= sprintf("    parts->%s[%u]=", $i, $$parts{DATA_LENGTH});
                my @k = unpack('C' x $$parts{DATA_LENGTH}, $$parts{DATA});
                for (my $j = 0; $j < $$parts{DATA_LENGTH}; $j++)
                {
                    if (($j % 4) == 0 && $j != 0)
                    {
                        $msg .= sprintf(" ");
                    }
                    $msg .= sprintf("%-2.2x", $k[$j]);
                }
                $msg .= sprintf("\n");
            }
            else
            {
                $msg .= sprintf("    parts->%s=", $i);
                $msg .= sprintf("'%u' [0x%x]\n", $$parts{$i}, $$parts{$i});
            }
        }
        else
        {
            $msg .= sprintf("    parts->$i=undefined\n");
        }
    }
#    $msg .= ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n";
    return($msg);
}

##############################################################################
use constant SES_WWN_TO_SES =>
   "NN      # WWN;                /* World wide name */
    L       # SES;                /* SES device pointer */
    S       # Slot;               /* Slot number */
    S";     # PID;                /* Dev ID as seen from BEP */

##############################################################################
# Display %info, for debugging.
##############################################################################
sub display_info($)
{
    my ($info) = @_;
    my $msg;

    $msg = '';
    foreach my $i (sort keys(%$info))
    {
        if (defined($$info{$i}))
        {
            if ($i eq 'PAGE2_Control'
#                || $i eq 'SES_MapType'
#                || $i eq 'SES_Slots'
                )
            {
                next;
            }
            if ($i eq 'SES_revision')
            {
                $msg .= sprintf("    info->%s=%s\n", $i, $$info{$i});
                next;
            }
            elsif ( $i eq 'devMap')
            {
                my %wwn;
                my $SES;
                my $Slot;
                my $PID;
                my $offset = 0;
                
                $msg .= sprintf("    info->%s[%u]=\n", $i, length($$info{$i}));
                $msg .= sprintf("        PID: Slot WWN\n");
                while ($offset < length($$info{$i}))
                {
                    ( 
                        $wwn{LO_LONG}, $wwn{HI_LONG}, $SES, $Slot, $PID
                    ) = unpack("x$offset" . SES_WWN_TO_SES , $$info{$i});
                    $msg .= sprintf("        %3u: %4u %08x%08x\n", $PID, $Slot, $wwn{LO_LONG}, $wwn{HI_LONG});

                    $offset += 2*4 + 4 + 2 + 2;
                }
            }
            elsif ($i eq 'SES_MapType' || $i eq 'SES_Slots' || $i eq 'devMap')
            {
               my $l = length($$info{$i});
               $msg .= sprintf("    info->%s[%u]=", $i, $l);
               my @k = unpack('C' x $l, $$info{$i});
               for (my $j = 0; $j < $l; $j++)
               {
                 if (($j % 4) == 0 && $j != 0)
                 {
                   $msg .= sprintf(" ");
                 }
                 $msg .= sprintf("%-2.2x", $k[$j]);
               }
               $msg .= sprintf("\n");
            }
            else
            {
                $msg .= sprintf("    info->$i='%u' [0x%x]\n", $$info{$i}, $$info{$i});
            }
        }
        else
        {
            $msg .= sprintf("undefined\n");
        }
    }
#    $msg .= "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
    return($msg);
}

##############################################################################
use constant SES_DEVICE_PI =>        # Does have devType in it.
   "L       # *NextSES;           /* Next SES device          */
    NN      # WWN;                /* WWN of the SES device    */
    L       # SupportedPages;     /* Bit significant support  */
    L       # FCID;               /* Fibre channel ID         */
    L       # Generation;         /* Generation code          */
    C       # Channel;            /* Fibre channel adapter    */
    C       # devStat;            /* Device status            */
    S       # PID;                /* PID of the SES device    */
    S       # LUN;                /* Logical unit number      */
    S       # TotalSlots;         /* Total element slots      */
    L       # *OldPage2;          /* Previous page 2 reading  */
    a256    # Map[SES_ET_MAX_VAL+1];/* Map of type area       */
    a256    # Slots[SES_ET_MAX_VAL+1];/* Number of slots      */
    A4      # pd_rev[4];          /* revision                 */
    C       # devType;            /* Device type              */
# Following is structure PI_DISK_BAY_ENVIRO_PART2_RSP.
    S       # mapLength;          /* Length of device map to follow */
    S";     # page2Length;        /* Length of page 2 data to follow */

##############################################################################
# Name:     _diskBayEnviroPacket
#
# Desc:     Parses the disk bay environment (SES) packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash.
#
##############################################################################
sub _diskBayEnviroPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_ENVIRO_DATA_DISK_BAY_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $ptr;
        my %wwn;
        my $map;
        my $slots;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        ( # struct SES_DEVICE
            $ptr, $wwn{HI_LONG}, $wwn{LO_LONG}, $info{SES_SupportedPages},
            $info{SES_FCID}, $info{SES_Generation}, $info{SES_Channel},
            $info{SES_devStat}, $info{SES_PID}, $info{SES_LUN}, $info{SES_TotalSlots},
            $ptr, $info{SES_MapType}, $info{SES_Slots}, $info{SES_revision},
            $info{SES_devType}, $info{PART2_mapLength}, $info{PART2_page2Length}
        ) = unpack( SES_DEVICE_PI , $parts{DATA});

        $info{WWN} = longsToBigInt(%wwn);
        $info{WWN_HI} = $wwn{HI_LONG};
        $info{WWN_LO} = $wwn{LO_LONG};

        my $offset2 = 4*4 + 4*4 + 4 + 256 + 256 + 1;
        $info{devMap} = substr($parts{DATA}, $offset2 + 4 + 4, $info{PART2_mapLength});

        my $offset3 = $offset2 + 4 + 4 + $info{PART2_mapLength};
        (   $info{PAGE2_PageCode}, $info{PAGE2_Status}, $info{PAGE2_Length},
            $info{PAGE2_Generation}
        ) = unpack("x$offset3 CCSL", $parts{DATA});

        # Number of elements is in $info{SES_TotalSlots}.

        my $offset4 = $offset3 + 8;
        $info{PAGE2_Control} = substr($parts{DATA}, $offset4);
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a disk bay environmental info packet\n");
    }

    return %info;
}

##############################################################################
# Name: displayDiskBayEnviro
#
# Desc: Print the physical disk bay environmental information
#
# In:   Physical Disk Bay Environmental nformation Hash
##############################################################################
sub displayDiskBayEnviro
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = '';

    my $asciiType = "UNKN";
    if ($info{SES_devType} == PD_DT_FC_SES)   { $asciiType = "Fibre Channel"; }
    if ($info{SES_devType} == PD_DT_SATA_SES) { $asciiType = "SATA"; }
    if ($info{SES_devType} == PD_DT_SBOD_SES) { $asciiType = "SBOD"; }
    $msg .= sprintf("Disk Bay Environmental Information for PID %u of type %s [0x%x].\n",
		    $info{SES_PID}, $asciiType, $info{SES_devType});

# Decode the Device Mapping within the Bay.
    my %wwn;
    my $SES;
    my $Slot;
    my $PID;
    my $offset = 0;
    
    $msg .= sprintf("    Device Mapping within Bay:\n");
    $msg .= sprintf("    PID Slot WWN\n");
    while ($offset < length($info{devMap}))
    {
	( 
	    $wwn{LO_LONG}, $wwn{HI_LONG}, $SES, $Slot, $PID
	) = unpack("x$offset" . SES_WWN_TO_SES , $info{devMap});
	$msg .= sprintf("    %3u %4u %08x%08x\n", $PID, $Slot, $wwn{LO_LONG}, $wwn{HI_LONG});

	$offset += 2*4 + 4 + 2 + 2;
    }
    $msg .= "\n";

# Now decode the enviromentals.
    my $m;
    ($m) = unpack("a$info{PART2_page2Length}", $info{PAGE2_Control});
    $msg .= sprintf(XIOTech::fmtFIDs::fmt_display_elements($info{SES_MapType},$info{SES_Slots},$m));

    return $msg;
}

##############################################################################
# Following is a true response for exit from this file.
1;

##############################################################################
# Change log:
# $Log$
# Revision 1.4  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.2.2.2  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.2.2.1  2006/02/24 14:17:23  MiddenM
#
# Merge from WOOKIEE_EGGS_GA_BR into MODEL750_BR
#
# Revision 1.3  2006/01/23 14:04:50  BoddukuriV
# TBolt00000000 - Removed CRLFs
#
# Revision 1.2  2005/12/23 08:13:48  BalemarthyS
# Merged ISCSI & GEORAID related changes
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.24  2004/07/23 18:19:22  NigburC
# TBolt00000000 - Changed the dev pointer display to be in hex.
# Reviewed by Jeff Williams.
#
# Revision 1.23  2004/02/26 15:44:41  SchibillaM
# TBolt00010148:  Changes to display text for DEV_TYPE.
#
# Revision 1.22  2004/02/24 19:43:49  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests and to the DISKBAYS request.
# Reviewed by Jeff Williams.
#
# Revision 1.21  2004/02/24 19:34:19  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests and to the DISKBAYS request.
# Reviewed by Jeff Williams.
#
# Revision 1.20  2003/06/03 19:46:18  MenningC
# TBOLT00000000: Changed many of the 'display' functions to fill a string rather than print to the screen. This supports use by the test scripts. Reviewed by Randy R.
#
# Revision 1.19  2003/04/25 19:39:12  TeskeJ
# tbolt00008122 - added the fcal loopmap index into pdisk info and displays
#
# Revision 1.18  2003/04/22 13:39:32  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.17  2003/04/16 19:08:20  SchibillaM
# TBolt00007922: Remove disk bay naming and name FID init code.
#
# Revision 1.16  2002/12/12 16:56:21  NigburC
# TBolt00006457 - Updated the disk bay packet handling so it matches the
# recent changes in the CCB and PROC.
# Reviewed by Mark Schibilla (virtually).
#
# Revision 1.15  2002/11/21 21:35:04  HoltyB
# TBolt00006364:  Fixed up diskBayStatus.
#
# Revision 1.14  2002/11/19 21:14:55  NigburC
# TBolt00006343 - Modified the PDISK INFO response packet definition to
# match what the PROC is now returning.  The size of the structure did not
# change but the contents did.  The SES and SLOT information changed
# wherein two values became reserved and the names of the others changed.
# Reviewed by Jeff Williams.
#
# Revision 1.13  2002/09/13 15:57:45  HoltyB
# TBolt00000000: Fixed diskBaystatus for changes from xiotech to eurolgic bays.
#
# Revision 1.12  2002/09/03 14:32:52  SchibillaM
# TBolt00005858: Changes to CCBCL files to allow a subset of function to be built
# for field use.  The tool BuildCCBCLSubset.pl builds the subset.  These changes
# also remove ENVSTATS which is replaced by STATSENV.
# Reviewed by Randy and Craig.
#
# Revision 1.11  2002/07/01 20:19:03  SchibillaM
# TBolt00004285: Implement disk bay audible alarm control.
# Review by Randy Rysavy (C, Perl),  Chris (Java)
#
# Revision 1.10  2002/04/29 21:59:27  HoltyB
# TBolt00004068: Separated DEVDELETE into two seperate commands
# PDISKDELETE and DISKBAYDELETE and removed DEVDELETE
#
# Revision 1.9  2002/04/15 19:20:06  NigburC
# TBolt00003771 - Modified the display routines for PDISKS and DISKBAYS to
# provide useful information in a readable format.
# Reviewed by Mark Schibilla.
#
# Revision 1.8  2002/04/11 16:27:19  HoltyB
# Added Disk Bay Revision to diskBayStatus
#
# Revision 1.7  2002/03/27 13:11:13  SchibillaM
# TBolt00003487: Add support for Get BE Port List and Break VLink lock.
# Reviewed by Randy.
#
# Revision 1.6  2002/03/26 22:32:54  HoltyB
# Added diskBayStatus command to dislpay disk bays and their associated
# physical disks
#
# Revision 1.5  2002/03/20 19:24:36  SchibillaM
# TBolt00003359: Correct CCBE to show additional fields recently added to
# disk bay info.  Reviewed by Chris.
#
# Revision 1.4  2002/03/18 20:34:53  NigburC
# TBolt00003338, TBolt00002733 - Added CCBE side of the command to
# retrieve the information for all disk bays in one request.
# Added the first basic implementation of controller validation.  This includes
# the VCGVALIDATESLAVE command handler and the framework to add
# additional validation steps.
#
# Revision 1.3  2002/02/19 21:13:58  SchibillaM
# TBolt00003096: Add DiskBaySetName command to CCBE.
#
# Revision 1.2  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.1  2002/01/11 22:30:55  HoltyB
# Initial Integration
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
