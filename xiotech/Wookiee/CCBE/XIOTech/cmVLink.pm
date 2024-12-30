# $Id: cmVLink.pm 161427 2013-08-23 16:32:05Z marshall_midden $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
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
# Name:     virtualLinks
#
# Desc:     Retrieves virtual link information for all virtual links.
#
# In:       NONE
#
# Returns:
#
##############################################################################
sub virtualLinks
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["virtualLinks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %info;
    my $i;
    my $sIndex;
    my $lIndex;
    my $count = 0;
    my %vdisks = $self->virtualDisks();
    my %servers = $self->servers();

    $info{STATUS} = PI_GOOD;
    $info{ERROR_CODE} = 0;

    if (%vdisks &&
        %servers &&
        $vdisks{STATUS} == PI_GOOD &&
        $servers{STATUS} == PI_GOOD)
    {
        for ($i = 0; $i < $vdisks{COUNT}; $i++)
        {
            my %vlinkinfo;

            if ($vdisks{VDISKS}[$i]{ATTR} & 0x0080)
            {
                %vlinkinfo = $self->virtualLinkInfo($vdisks{VDISKS}[$i]{VID});

                if (%vlinkinfo && $vlinkinfo{STATUS} == PI_GOOD)
                {
                    $info{LIST}[$count]{VID} = $vdisks{VDISKS}[$i]{VID};
                    $info{LIST}[$count]{ATTR} = $vdisks{VDISKS}[$i]{ATTR};
                    $info{LIST}[$count]{CAPACITY} = $vdisks{VDISKS}[$i]{CAPACITY};
                    $info{LIST}[$count]{LD_BASESN} = $vlinkinfo{LD_BASESN};
                    $info{LIST}[$count]{LD_BASECL} = $vlinkinfo{LD_BASECL};
                    $info{LIST}[$count]{LD_BASEVD} = $vlinkinfo{LD_BASEVD};
                    $info{LIST}[$count]{LD_LUN} = $vlinkinfo{LD_LUN};

                    $count = $count + 1;
                }
            }
            elsif ($vdisks{VDISKS}[$i]{ATTR} & 0x0040)
            {
                $info{LIST}[$count]{VID} = $vdisks{VDISKS}[$i]{VID};
                $info{LIST}[$count]{ATTR} = $vdisks{VDISKS}[$i]{ATTR};
                $info{LIST}[$count]{CAPACITY} = $vdisks{VDISKS}[$i]{CAPACITY};

                for ($sIndex = 0; $sIndex < $servers{COUNT}; $sIndex++)
                {
                    for ($lIndex = 0; $lIndex < $servers{SERVERS}[$sIndex]{NLUNS}; ++$lIndex)
                    {
                        if ($servers{SERVERS}[$sIndex]{LUNMAP}[$lIndex]{VID} == $vdisks{VDISKS}[$i]{VID})
                        {
                            $info{LIST}[$count]{WWN_LO} = $servers{SERVERS}[$sIndex]{WWN_LO};
                            $info{LIST}[$count]{WWN_HI} = $servers{SERVERS}[$sIndex]{WWN_HI};
                            $info{LIST}[$count]{TARGET_ID} = $servers{SERVERS}[$sIndex]{TARGETID};
                        }
                    }
                }
                $count = $count + 1;
            }
        }
        $info{COUNT} = $count;
    }
    else
    {
        $info{STATUS} = PI_ERROR;
        $info{ERROR_CODE} = 0;
        $info{MESSAGE} = "Failed to get virtual disks or servers.";
    }
    return %info;
}

##############################################################################
# Name:     virtualLinkCtrlCount
#
# Desc:     Retrieves the number of remote controllers.
#
# Input:    None
#
# Returns:  Number of targets or UNDEF if an error occurred.
##############################################################################
sub virtualLinkCtrlCount
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["virtualLinkCtrlCount"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_REMOTE_CTRL_COUNT_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualLinkCtrlCountPacket);
}

##############################################################################
# Name:     virtualLinkCtrlInfo
#
# Desc:     Get information about a remote controller
#
# In:       ID of a remote controller
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#       STATUS                  Status of the command
#
##############################################################################
sub virtualLinkCtrlInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualLinkCtrlInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_REMOTE_CTRL_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualLinkCtrlInfoPacket);
}

##############################################################################
# Name:     virtualLinkCtrlVDisks
#
# Desc:     Get information about a remote controller
#
# In:       ID of a remote controller
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#       STATUS                  Status of the command
#
##############################################################################
sub virtualLinkCtrlVDisks
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualLinkCtrlVDisks"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_REMOTE_CTRL_VDISKS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualLinkCtrlVDisksPacket);
}

##############################################################################
# Name:     virtualLinkCreate
#
# Desc:     Create a virtual link.
#
# In:       CONTROLLER_ID   - Ordinal of the remote controller
#           VIRTUAL_DISK_ID - Ordinal of the virtual disk.
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#       STATUS                  Status of the command
#
##############################################################################
sub virtualLinkCreate
{
    my ($self, $cid, $vid, $rvid) = @_;
    my @newArgs;
    
    logMsg("begin\n");

    if (!defined($rvid))
    {
        $rvid = $self->_virtualDiskGetNextVid();
        
        if (defined($rvid))
        {
#            print "Pushing VID: $vid\n";
            push @newArgs, $self;
            push @newArgs, $cid;
            push @newArgs, $vid;
            push @newArgs, $rvid;
            
            @_ =  @newArgs;
        }
        else
        {
            print "Could not generate a valid VID!!\n";
        }
    }
    
#    print "Using CID=$cid, VID=$vid, RVID=$rvid\n";

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 4000],         # 7000 it is 4000 for MAX virtual disks.
                ["virtualLinkCreate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_CREATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCCSS",
                    $cid,
                    $vid,
                    0,
                    $rvid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualLinkCreatePacket);
}

##############################################################################
# Name:     virtualLinkInfo
#
# Desc:     Get information about a virtual link
#
# In:       ID  - Virtual link identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#       STATUS                  Status of the command
#
##############################################################################
sub virtualLinkInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualLinkInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_virtualLinkInfoPacket);
}

##############################################################################
# Name:     virtualLinkBreakLock
#
# Desc:     Check for Virtual Links for the input vid.  Clean them up if 
#           inactive or return an error if in use.
#
# In:       ID  - Virtual disk identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#       STATUS                  Status of the command
#
##############################################################################
sub virtualLinkBreakLock
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["virtualLinkBreakLock"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_BREAK_LOCK_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
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
# Name:     getVLinkRelatedName
#
# Desc:     Get the name associated with a VLink related component
#
# In:       COMPONENT_TYPE  - VDISK, VLINKRMTCTRL, CONTROLLER, VLINKRMTCTRL
#           COMPONENT_ID    - 
#
# Returns:  ASCII name for this component
#           
##############################################################################
sub getVLinkRelatedName
{
    my ($self, $type, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 10, 18],
                ['d', 0, 0xFFFF],
                ["getVLinkRelatedName"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_GET_NAME_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL",
                    $type,
                    $id);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                       $packet,
                                       \&_getNameResponsePacket);
}

##############################################################################
# Name:     writeVLinkRelatedName
#
# Desc:     Write the name associated with a VLink related component
#           Actually it should work for any system component if you
#           have the right file ID.
#
# In:       COMPONENT_TYPE  - VDISK, VLINKRMTCTRL, CONTROLLER, VLINKRMTCTRL
#           COMPONENT_ID    - ID for the component
#           NAME            - ASCII name for this component    
#
# Returns:  none
#
##############################################################################
sub writeVLinkRelatedName
{
    my ($self, $type, $id, $name) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 10, 18],
                ['d', 0, 0xFFFF],
                ['s'],
                ["writeName"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DEBUG_WRITE_NAME_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LLa62",
                    $type,
                    $id,
                    $name);

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
# Name:     GetVLinkDLinkInfo
#
# Desc:     Get information about a virtual link (DLink)
#
# In:       ID  - Virtual link identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub GetVLinkDLinkInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["GetVLinkDLinkInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # Do old 32 bit version.
    my $cmd = PI_VLINK_DLINK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_VLinkDLinkInfoPacket);
}

##############################################################################
# Name:     GetVLinkDLinkInfoGT2TB
#
# Desc:     Get information about a virtual link (DLink)
#
# In:       ID  - Virtual link identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub GetVLinkDLinkInfoGT2TB
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["GetVLinkDLinkInfoGT2TB"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    # Do old 32 bit version.
    my $cmd = PI_VLINK_DLINK_GT2TB_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_VLinkDLinkInfoPacket_GT2TB);
}

##############################################################################
# Name:     GetVLinkDLockInfo
#
# Desc:     Get information about a virtual link - DLock
#
# In:       ID  - Virtual link identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub GetVLinkDLockInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["GetVLinkDLockInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_VLINK_DLOCK_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $id,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_VLinkDLockInfoPacket);
}

##############################################################################
# Name: displayVirtualLinks
#
# Desc: Print the virtual link information for all virtual links.
#
# In:   Virtual Links Hash
##############################################################################
sub displayVirtualLinks
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;
    my $msg = "";

    $msg .= sprintf("Virtual Links ($info{COUNT} disks):\n");
    $msg .= sprintf("\n");

    $msg .= "REMOTE STORAGE -->\n";
    $msg .= sprintf("\n");
    $msg .= " VID    ATTR      CAPACITY     SERIAL NUM  CLUSTER  VDISK  LUN\n";
    $msg .= " ----  ------  --------------  ----------  -------  -----  ---\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        if ($info{LIST}[$i]{ATTR} & 0x0080)
        {
            $msg .= sprintf(" %4hu  0x%4.4x  %14s  %10hu  %7hu  %5hu  %3hu\n", 
                            $info{LIST}[$i]{VID},
                            $info{LIST}[$i]{ATTR},
                            $info{LIST}[$i]{CAPACITY},
                            $info{LIST}[$i]{LD_BASESN},
                            $info{LIST}[$i]{LD_BASECL},
                            $info{LIST}[$i]{LD_BASEVD},
                            $info{LIST}[$i]{LD_LUN});
        }
    }

    $msg .= sprintf("\n");
    $msg .= sprintf("\n");

    $msg .= "<-- LOCAL STORAGE\n";
    $msg .= sprintf("\n");
    $msg .= " VID    ATTR      CAPACITY        Server WWN     TARGET\n";
    $msg .= " ----  ------  --------------  ----------------  ------\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        if ($info{LIST}[$i]{ATTR} & 0x0040)
        {
            $msg .= sprintf(" %4hu  0x%4.4x  %14s  %8.8x%8.8x  %6hu\n", 
                            $info{LIST}[$i]{VID},
                            $info{LIST}[$i]{ATTR},
                            $info{LIST}[$i]{CAPACITY},
                            $info{LIST}[$i]{WWN_LO}, 
                            $info{LIST}[$i]{WWN_HI},
                            $info{LIST}[$i]{TARGET_ID});  
        }
    }
    return $msg;
}

##############################################################################
# Name: displayVirtualLinkCtrlInfo
#
# Desc: Print the remote controller information
#
# In:   Remote Controller Information Hash
##############################################################################
sub displayVirtualLinkCtrlInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Virtual Link Controller Information:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  WWN:                   %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
    printf "  CONTROLLER NAME:       %s\n", $info{CNAME};
    printf "  LUNS:                  %hu\n", $info{LUNS};
    printf "  CONTROLLER TYPE:       0x%x\n", $info{SCTYPE};
    printf "  CLUSTER:               %hu\n", $info{CLUSTER};
    printf "  IP ADDRESS:            %lu\n", $info{IPADDR};
    printf "  SERIAL NUMBER:         %lu\n", $info{SERIAL_NUM};
    print "\n";
}

##############################################################################
# Name: displayVirtualLinkCtrlVDisks
#
# Desc: Print the remote controller virtual disks
#
# In:   Remote Controller Virtual Disks Hash
##############################################################################
sub displayVirtualLinkCtrlVDisks
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $i;

    print "Virtual Link Controller Virtual Disks:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  COUNT:                 %hu\n", $info{COUNT};
    print  "\n";

    print  "ORDINAL  LUN  RTYPE  CLUSTER  ATTR  CAPACITY  SERIAL_NUM  VID1  VID2  SCNT  VLCNT  VDNAME\n";
    print  "-------  ---  -----  -------  ----  --------  ----------  ----  ----  ----  -----  ------\n";

    for ($i = 0; $i < $info{COUNT}; ++$i)
    {
        printf "%7u  %3u   0x%02X      %3u  0x%02X",
                $i,
                $info{VDDS}[$i]{LUN},
                $info{VDDS}[$i]{RTYPE},
                $info{VDDS}[$i]{CLUSTER},
                $info{VDDS}[$i]{ATTR};
        print  "  " . $info{VDDS}[$i]{DEVCAP};
        printf "    %8lu   %4u   %4u   %4u    %4u",
                $info{VDDS}[$i]{SSERIAL},
                $info{VDDS}[$i]{VID1},
                $info{VDDS}[$i]{VID2},
                $info{VDDS}[$i]{SCNT},
                $info{VDDS}[$i]{VLCNT};
        printf "  " . $info{VDDS}[$i]{VDNAME};
        print  "\n";
    }

    print "\n";
}

##############################################################################
# Name: displayVirtualLinkCreate
#
# Desc: Print the virtual link create information
#
# In:   Virtual Link Create Information Hash
##############################################################################
sub displayVirtualLinkCreate
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Virtual Link Create Information:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  COUNT:                 %hu\n", $info{COUNT};
    printf "  VID:                   %hu\n", $info{VID};
    printf "  CONTROLLER_NAME:       %s\n", $info{CTRLNAME};
    printf "  VDISK_NAME:            %s\n", $info{VDNAME};
    print "\n";
}

##############################################################################
# Name: displayVirtualLinkInfo
#
# Desc: Print the virtual link information
#
# In:   Virtual Link Information Hash
##############################################################################
sub displayVirtualLinkInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print  "Virtual Link Information:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    print  "\n";
    print  "  VDD Data:\n";
    printf "    VD_VID:              %hu\n", $info{VD_VID};
    printf "    VD_ATTR:             0x%x\n", $info{VD_ATTR};
    printf "    VD_DEVSTAT:          0x%x\n", $info{VD_DEVSTAT};
    printf "    VD_SCORVID:          %hu\n", $info{VD_SCORVID};
    printf "    VD_SCPCOMP:          %hu\n", $info{VD_SCPCOMP};
    printf "    VD_RAIDCNT:          %hu\n", $info{VD_RAIDCNT};
    printf "    VD_DEVCAP:           $info{VD_DEVCAP}\n";
    printf "    VD_ERROR:            %lu\n", $info{VD_ERROR};
    printf "    VD_QD:               %lu\n", $info{VD_QD};
    printf "    VD_RPS:              %lu\n", $info{VD_RPS};
    printf "    VD_AVGSC:            %lu\n", $info{VD_AVGSC};
    printf "    VD_RREQ:             $info{VD_RREQ}\n";
    printf "    VD_WREQ:             $info{VD_WREQ}\n";
    printf "    VD_CACHEEN:          0x%x\n", $info{VD_CACHEEN};
    printf "    VD_MIRROR:           0x%x\n", $info{VD_MIRROR};
    printf "    VD_SPRC:             %lu\n", $info{VD_SPRC};
    printf "    VD_SPSC:             %lu\n", $info{VD_SPSC};
    printf "    VD_SCHEAD:           %lu\n", $info{VD_SCHEAD};
    printf "    VD_SCTAIL:           %lu\n", $info{VD_SCTAIL};
    printf "    VD_CPSCMT:           %lu\n", $info{VD_CPSCMT};
    printf "    VD_VLINKS:           0x%8.8x\n", $info{VD_VLINKS};
    printf "    VD_NAME:             %s\n", $info{VD_NAME};
    print  "\n";

    print  "  RDD Data:\n";
    printf "    RD_RID:              %hu\n", $info{RD_RID};
    printf "    RD_TYPE:             0x%x\n", $info{RD_TYPE};
    printf "    RD_DEVSTATUS:        0x%x\n", $info{RD_DEVSTATUS};
    printf "    RD_DEPTH:            %hu\n", $info{RD_DEPTH};
    printf "    RD_PCTREM:           %hu\n", $info{RD_PCTREM};
    printf "    RD_PSDCNT:           %hu\n", $info{RD_PSDCNT};
    printf "    RD_SPS:              %lu\n", $info{RD_SPS};
    printf "    RD_SPU:              %lu\n", $info{RD_SPU};
    printf "    RD_DEVCAP:           $info{RD_DEVCAP}\n";
    printf "    RD_NVRDD:            %lu\n", $info{RD_NVRDD};
    printf "    RD_VID:              %hu\n", $info{RD_VID};
    printf "    RD_FRCNT:            %hu\n", $info{RD_FRCNT};
    printf "    RD_ERROR:            %lu\n", $info{RD_ERROR};
    printf "    RD_QD:               %lu\n", $info{RD_QD};
    printf "    RD_RPS:              %lu\n", $info{RD_RPS};
    printf "    RD_AVGSC:            %lu\n", $info{RD_AVGSC};
    printf "    RD_RREQ:             $info{RD_RREQ}\n";
    printf "    RD_WREQ:             $info{RD_WREQ}\n";
    printf "    RD_LLSDA:            $info{RD_LLSDA}\n";
    printf "    RD_LLEDA:            $info{RD_LLEDA}\n";
    printf "    RD_IPROCS:           %lu\n", $info{RD_IPROCS};
    printf "    RD_IERRORS:          %lu\n", $info{RD_IERRORS};
    printf "    RD_ISECTORS:         $info{RD_ISECTORS}\n";
    printf "    RD_MISCOMP:          %lu\n", $info{RD_MISCOMP};
    printf "    RD_PARDRV:           %lu\n", $info{RD_PARDRV};
    printf "    RD_DEFLOCK:          %lu\n", $info{RD_DEFLOCK};
    printf "    RD_ASTATUS:          %lu\n", $info{RD_ASTATUS};
    printf "    RD_SPRC:             %lu\n", $info{RD_SPRC};
    printf "    RD_SPSC:             %lu\n", $info{RD_SPSC};
    printf "    RD_RPNHEAD:          %lu\n", $info{RD_RPNHEAD};
    print  "\n";

    print  "  LDD Data:\n";
    printf "    LD_CLASS:            0x%x\n", $info{LD_CLASS},
    printf "    LD_STATE:            0x%x\n", $info{LD_STATE},
    printf "    LD_PMASK:            0x%x\n", $info{LD_PMASK},
    printf "    LD_PPRI:             %hu\n", $info{LD_PPRI},
    printf "    LD_OWNER:            %lu\n", $info{LD_OWNER},
    printf "    LD_DEVCAP:           $info{LD_DEVCAP}\n";
    printf "    LD_VENDID:           %s\n", $info{LD_VENDID},
    printf "    LD_REV:              %s\n", $info{LD_REV},
    printf "    LD_PRODID:           %s\n", $info{LD_PRODID},
    printf "    LD_SERIAL:           %s\n", $info{LD_SERIAL},
    printf "    LD_LUN:              %hu\n", $info{LD_LUN},
    printf "    LD_BASEVD:           %hu\n", $info{LD_BASEVD},
    printf "    LD_BASECL:           %hu\n", $info{LD_BASECL},
    printf "    LD_BASESN:           %lu\n", $info{LD_BASESN},
    printf "    LD_BASENAME:         %s\n", $info{LD_BASENAME},
    printf "    LD_BASENODE:         %8.8x%8.8x\n", $info{LD_BASENODE_LO}, $info{LD_BASENODE_HI};
    print  "\n";
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# NAME:     _getNameResponsePacket
#
# DESC:     Handle the response packet from a get name request
#
# INPUT:    scalar  $sequenceID     Sequence id
#           scalar  $recvPacket     Packet to parse
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS      status of the command
#       ERROR_CODE  error code associated withthe command
#       ID          ID of the system component
#       NAME        name of the system component    
#
##############################################################################
sub _getNameResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %parts = disassembleXiotechPacket($recvPacket);

    my $rsvd;
    my %info;

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    (
        $info{ID},
        $info{NAME},
        $rsvd
    ) = unpack("La62a62", $parts{DATA});
    return %info;
}

##############################################################################
# NAME:     _virtualLinkCtrlCountPacket
#
# DESC:     Handles a virtual link remote controller count response packet
#
# INPUT:    scalar  $sequenceID         Sequence id
#           scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
#       STATUS                  status of the command
#       COUNT                   count of remote controllers
#
##############################################################################
sub _virtualLinkCtrlCountPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_VLINK_REMOTE_CTRL_COUNT_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd1;
        my $rsvd2;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $rsvd1,
        $info{STATUS_MRP},
        $info{LEN},
        $info{COUNT},
        $rsvd2
        ) = unpack("a3CLSa2", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual link controller count packet\n");
    }
    return %info;
}

##############################################################################
# Name:     _virtualLinkCtrlInfoPacket
#
# Desc:     Parses the virtual link remote controller information packet and
#           places the information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualLinkCtrlInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VLINK_REMOTE_CTRL_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd1;
        my $rsvd2;
        my %wwn;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd1,
        $info{STATUS_MRP},
        $info{LEN},
        $wwn{LO_LONG}, $wwn{HI_LONG},
        $info{CNAME},
        $info{LUNS},
        $info{SCTYPE},
        $info{CLUSTER},
        $rsvd2,
        $info{IPADDR},
        $info{SERIAL_NUM}
        ) = unpack("a3CLNNa20CCCCLL", $parts{DATA});

        # Now fixup all the 64 bit  numbers
        $info{WWN} = longsToBigInt(%wwn);
        $info{WWN_LO} = $wwn{LO_LONG};
        $info{WWN_HI} = $wwn{HI_LONG};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual link controller info packet\n");
    }
    return %info;
}

##############################################################################
# Name:     _virtualLinkCtrlVDisksPacket
#
# Desc:     Parses the remote controller virtual disks packet and places the
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualLinkCtrlVDisksPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VLINK_REMOTE_CTRL_VDISKS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd1;
        my $rsvd2;
        my $rsvd3;
        my @vdds;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $info{COUNT},
        $rsvd1,
        $info{STATUS_MRP},
        $info{LEN}
        ) = unpack("Ca2CL", $parts{DATA});

        for ($i = 0; $i < $info{COUNT}; ++$i)
        {
            my $start = 8 + (96 * $i);

            my %vddout;
            my %devcap;

            # Unpack the virtual disk information
            (
            $vdds[$i]{LUN},
            $vdds[$i]{RTYPE},
            $vdds[$i]{CLUSTER},
            $vdds[$i]{ATTR},
            $rsvd1,
            $devcap{LO_LONG}, $devcap{HI_LONG},
            $vdds[$i]{SSERIAL},
            $vdds[$i]{VID1},
            $vdds[$i]{VID2},
            $vdds[$i]{SCNT},
            $vdds[$i]{VLCNT},
            $rsvd2,
            $vdds[$i]{VDNAME},
            $rsvd3
            ) = unpack("SCCCa3LLLSSSSa4a52a12", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $vdds[$i]{DEVCAP} = longsToBigInt(%devcap);
            $vdds[$i]{DEVCAP_LO} = $devcap{LO_LONG};
            $vdds[$i]{DEVCAP_HI} = $devcap{HI_LONG};
        }
        $info{VDDS} = [@vdds];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a remote controller virtual disks packet\n");
    }
    return %info;
}

##############################################################################
# Name:     _virtualLinkCreatePacket
#
# Desc:     Parses the virtual link create information packet and
#           places the information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualLinkCreatePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VLINK_CREATE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd1;
        my $rsvd2;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $info{COUNT},
        $rsvd1,
        $info{STATUS_MRP},
        $info{LEN},
        $info{VID},
        $rsvd2,
        $info{CTRLNAME},
        $info{VDNAME}
        ) = unpack("CSCLSa2a20a52", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual link create packet\n");
    }
    return %info;
}

##############################################################################
# Name:     _virtualLinkInfoPacket
#
# Desc:     Parses the virtual link information packet and
#           places the information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _virtualLinkInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_VLINK_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);
        my $vdd = substr($parts{DATA}, 8, 96);
        my $rdd = substr($parts{DATA}, 104, 128);
        my $ldd = substr($parts{DATA}, 232, 112);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN}
        ) = unpack("a3CL", $parts{DATA});

        my %vd_devcap;
        my %vd_rreq;
        my %vd_wreq;

        # Unpack the VDD
        (
        $info{VD_VID},
        $info{VD_ATTR},
        $info{VD_DEVSTAT},
        $info{VD_SCORVID},
        $info{VD_SCPCOMP},
        $info{VD_RAIDCNT},
        $vd_devcap{LO_LONG}, $vd_devcap{HI_LONG},

        $info{VD_ERROR},
        $info{VD_QD},
        $info{VD_RPS},
        $info{VD_AVGSC},

        $vd_rreq{LO_LONG}, $vd_rreq{HI_LONG},
        $vd_wreq{LO_LONG}, $vd_wreq{HI_LONG},

        $info{VD_CACHEEN},
        $info{VD_MIRROR},
        $rsvd,
        $rsvd,
        $info{VD_SPRC},
        $info{VD_SPSC},

        $info{VD_SCHEAD},
        $info{VD_SCTAIL},
        $info{VD_CPSCMT},
        $info{VD_VLINKS},

        $info{VD_NAME},
        ) = unpack("SCCSCCLL LLLL LLLL CCSLLL LLLL a16", $vdd);
# NOTE: 2013-08-22 The above is not the vdisk structure. It is wrong. But not used in ccbcl.

        # Now fixup all the 64 bit  numbers
        $info{VD_DEVCAP} = longsToBigInt(%vd_devcap);
        $info{VD_RREQ} = longsToBigInt(%vd_rreq);
        $info{VD_WREQ} = longsToBigInt(%vd_wreq);

        my %rd_devcap;
        my %rd_rreq;
        my %rd_wreq;
        my %rd_llsda;
        my %rd_lleda;
        my %rd_isectors;

        # Unpack the RDD
        (
        $info{RD_RID},
        $info{RD_TYPE},
        $info{RD_DEVSTATUS},
        $info{RD_DEPTH},
        $info{RD_PCTREM},
        $info{RD_PSDCNT},
        $info{RD_SPS},
        $info{RD_SPU},

        $rd_devcap{LO_LONG}, $rd_devcap{HI_LONG},
        $info{RD_NVRDD},
        $info{RD_VID},
        $info{RD_FRCNT},

        $info{RD_ERROR},
        $info{RD_QD},
        $info{RD_RPS},
        $info{RD_AVGSC},

        $rd_rreq{LO_LONG}, $rd_rreq{HI_LONG},
        $rd_wreq{LO_LONG}, $rd_wreq{HI_LONG},

        $rd_llsda{LO_LONG}, $rd_llsda{HI_LONG},
        $rd_lleda{LO_LONG}, $rd_lleda{HI_LONG},

        $info{RD_IPROCS},
        $info{RD_IERRORS},
        $rd_isectors{LO_LONG}, $rd_isectors{HI_LONG},

        $info{RD_MISCOMP},
        $info{RD_PARDRV},
        $info{RD_DEFLOCK},
        $info{RD_ASTATUS},
        $rsvd,

        $info{RD_SPRC},
        $info{RD_SPSC},
        $info{RD_RPNHEAD},
        $rsvd
        ) = unpack("SCCCCSLL LLLSS LLLL LLLL LLLL LLLL  LLSCa5  LLLa4", $rdd);

        # Now fixup all the 64 bit  numbers
        $info{RD_DEVCAP} = longsToBigInt(%rd_devcap);
        $info{RD_RREQ} = longsToBigInt(%rd_rreq);
        $info{RD_WREQ} = longsToBigInt(%rd_wreq);
        $info{RD_LLSDA} = longsToBigInt(%rd_llsda);
        $info{RD_LLEDA} = longsToBigInt(%rd_lleda);
        $info{RD_ISECTORS} = longsToBigInt(%rd_isectors);

        my %ld_devcap;
        my %ld_basenode;

        # Unpack the LDD
        (
        $info{LD_CLASS},
        $info{LD_STATE},
        $info{LD_PMASK},
        $info{LD_PPRI},
        $info{LD_OWNER},
        $ld_devcap{LO_LONG}, $ld_devcap{HI_LONG},

        $info{LD_VENDID},
        $info{LD_REV},
        $rsvd,

        $info{LD_PRODID},

        $info{LD_SERIAL},
        $rsvd,

        $info{LD_LUN},
        $info{LD_BASEVD},
        $info{LD_BASECL},
        $rsvd,
        $info{LD_BASESN},
        $rsvd,

        $info{LD_BASENAME},

        $ld_basenode{LO_LONG}, $ld_basenode{HI_LONG},

        $rsvd
        ) = unpack("CCCCLLL a8a4a4 a16 a12a4 SSCa3La4 a16 NNa8", $ldd);

        # Now fixup all the 64 bit  numbers
        $info{LD_DEVCAP} = longsToBigInt(%ld_devcap);
        $info{LD_BASENODE} = longsToBigInt(%ld_basenode);
        $info{LD_BASENODE_HI} = $ld_basenode{HI_LONG};
        $info{LD_BASENODE_LO} = $ld_basenode{LO_LONG};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a virtual link info packet\n");
    }
    return %info;
}

##############################################################################
# Name:     _VLinkDLinkInfoPacket
#
# Desc:     Handle a VLink DLink Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
##############################################################################

# VLINK_DLINK_INFO
use constant VLINK_DLINK_INFO =>
           "a3          # rsvd
            C           # mrpStatus
            L           # length

            S           # dIndex
            C           # linkStatus
            C           # type
            C           # rsvd1
            L           # devCap
            a8          # srcVName
            L           # srcSerial
            C           # srcVBlock
            C           # srcVid
            C           # srcVPort
            C           # srcNumConn

            NN          # srcNodeWwn
            NN          # srcPortWwn1
            NN          # srcPortWwn2
            L           # baseSerial
            C           # baseVBlock
            C           # baseVid
            a10         # rsvd2
            a16         # product ID
            a8          # vendId
            a8          # srcName
            a4          # firmware revision
            a12";       # serial number

sub _VLinkDLinkInfoPacket
{
    my ($self, $seq, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %nodeWwn;
    my %portWwn1;
    my %portWwn2;
    my $rsvd;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    ( $rsvd,
      $info{STATUS_MRP},
      $info{LENGTH},
      $info{DINDEX},
      $info{LINKSTATUS},
      $info{TYPE},
      $rsvd,
      $info{DEVCAP},
      $info{SRCVNAME},
      $info{SRCSN},
      $info{SRCVBLK},
      $info{SRCVID},
      $info{SRCVPORT},
      $info{SRCNUMCONN},
      $nodeWwn{LO_LONG}, $nodeWwn{HI_LONG},
      $portWwn1{LO_LONG}, $portWwn1{HI_LONG},
      $portWwn2{LO_LONG}, $portWwn2{HI_LONG},
      $info{BASESN},
      $info{BASEVBLK},
      $info{BASEVID},
      $rsvd,
      $info{PRODID},
      $info{VENDID},
      $info{SRCNAME},
      $info{FWREV},
      $info{SN}) = unpack VLINK_DLINK_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{NODEWWN} = longsToBigInt(%nodeWwn);
    $info{NODEWWN_HI} = $nodeWwn{HI_LONG};
    $info{NODEWWN_LO} = $nodeWwn{LO_LONG};

    $info{PORTWWN1} = longsToBigInt(%portWwn1);
    $info{PORTWWN1_HI} = $portWwn1{HI_LONG};
    $info{PORTWWN1_LO} = $portWwn1{LO_LONG};

    $info{PORTWWN2} = longsToBigInt(%portWwn2);
    $info{PORTWWN2_HI} = $portWwn2{HI_LONG};
    $info{PORTWWN2_LO} = $portWwn2{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _VLinkDLinkInfoPacket_GT2TB
#
# Desc:     Handle a VLink DLink Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
##############################################################################

# VLINK_DLINK_INFO_GT2TB
use constant VLINK_DLINK_INFO_GT2TB =>
           "a3          # rsvd
            C           # mrpStatus
            L           # length

            S           # dIndex
            C           # linkStatus
            C           # type
            C           # rsvd1
            LL          # devCap
            a8          # srcVName
            L           # srcSerial
            C           # srcVBlock
            C           # srcVid
            C           # srcVPort
            C           # srcNumConn

            NN          # srcNodeWwn
            NN          # srcPortWwn1
            NN          # srcPortWwn2
            L           # baseSerial
            C           # baseVBlock
            C           # baseVid
            a10         # rsvd2
            a16         # product ID
            a8          # vendId
            a8          # srcName
            a4          # firmware revision
            a12";       # serial number

sub _VLinkDLinkInfoPacket_GT2TB 
{
    my ($self, $seq, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %nodeWwn;
    my %portWwn1;
    my %portWwn2;
    my $rsvd;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    my %vd_devcap;
    ( $rsvd,
      $info{STATUS_MRP},
      $info{LENGTH},
      $info{DINDEX},
      $info{LINKSTATUS},
      $info{TYPE},
      $rsvd,
      $vd_devcap{LO_LONG}, $vd_devcap{HI_LONG},
      $info{SRCVNAME},
      $info{SRCSN},
      $info{SRCVBLK},
      $info{SRCVID},
      $info{SRCVPORT},
      $info{SRCNUMCONN},
      $nodeWwn{LO_LONG}, $nodeWwn{HI_LONG},
      $portWwn1{LO_LONG}, $portWwn1{HI_LONG},
      $portWwn2{LO_LONG}, $portWwn2{HI_LONG},
      $info{BASESN},
      $info{BASEVBLK},
      $info{BASEVID},
      $rsvd,
      $info{PRODID},
      $info{VENDID},
      $info{SRCNAME},
      $info{FWREV},
      $info{SN}) = unpack VLINK_DLINK_INFO_GT2TB, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{DEVCAP} = longsToBigInt(%vd_devcap);
    $info{NODEWWN} = longsToBigInt(%nodeWwn);
    $info{NODEWWN_HI} = $nodeWwn{HI_LONG};
    $info{NODEWWN_LO} = $nodeWwn{LO_LONG};

    $info{PORTWWN1} = longsToBigInt(%portWwn1);
    $info{PORTWWN1_HI} = $portWwn1{HI_LONG};
    $info{PORTWWN1_LO} = $portWwn1{LO_LONG};

    $info{PORTWWN2} = longsToBigInt(%portWwn2);
    $info{PORTWWN2_HI} = $portWwn2{HI_LONG};
    $info{PORTWWN2_LO} = $portWwn2{LO_LONG};

    return %info;
}   # End of _VLinkDLinkInfoPacket_GT2TB

##############################################################################
# Name:     _VLinkDLockInfoPacket
#
# Desc:     Handle a VLink DLock Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
##############################################################################
# VLINK_DLOCK_INFO
use constant VLINK_DLOCK_INFO =>
           "a3          # rsvd
            C           # mrpStatus
            L           # length

            S           # vid
            L           # lockSN
            C           # lockVBlock
            C           # lockVid
            S           # rsvd1
            a8          # lockVDiskName
            a8";        # lockSUName

sub _VLinkDLockInfoPacket
{
    my ($self, $seq, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $rsvd;

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    ( $rsvd,
      $info{STATUS_MRP},
      $info{LENGTH},
      $info{VID},
      $info{LOCKSN},
      $info{LOCKVBLOCK},
      $info{LOCKVID},
      $rsvd,
      $info{VDISKNAME},
      $info{SUNAME}) = unpack VLINK_DLOCK_INFO, $parts{DATA};
    return %info;
}

##############################################################################
1;
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
