# $Id: cmServer.pm 148937 2010-10-12 16:06:50Z m4 $
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
# Name:     servers
#
# Desc:     Retrieves server information for all servers.
#
# In:       NONE
#
# Returns:
##############################################################################
sub servers
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["servers"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVERS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_serversPacket);
}

##############################################################################
# Name:     serverProperties
#
# Desc:     Associate a server to a virtual disk
#
# In:       SID - server identifier
#           PRI - Priority
#               0
#               1
#               2
#           ATTR - Attributes
#               bit 0 = 0   nothing set
#               bit 0 = 1   default
#               bit 1 = 0   nothing set
#               bit 1 = 1   new
#               bit 2 = 0   nothing set
#               bit 2 = 1   hide
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverProperties
{
    my ($self, $sid, $pri, $attr) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFFFFFFFF],
                ["serverProperties"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVER_SET_PROPERTIES_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("Sa1CL",
                    $sid,
                    0,
                    $pri,
                    $attr);

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
# Name:     serverAssociate
#
# Desc:     Associate a server to a virtual disk
#
# In:       SID - server identifier
#           LUN - Lun to use for the map
#           VID - virtual disk identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverAssociate
{
    my ($self, $sid, $lun, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["serverAssociate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    return $self->serverAssociateEx($sid, $lun, $vid, 0, 0);
}

##############################################################################
# Name:     serverAssociateEx
#
# Desc:     Associate a server to a virtual disk
#
# In:       SID - server identifier
#           LUN - Lun to use for the map
#           VID - virtual disk identifier
#           OPTION - Associate option
#           DSID - Destination server identifier
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverAssociateEx
{
    my ($self, $sid, $lun, $vid, $option, $dsid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 4],
                ['d', 0, 0xFFFF],
                ["serverAssociate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVER_ASSOCIATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSSSSS",
                    $sid,
                    $option,
                    $lun,
                    $dsid,
                    $vid,
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
# Name:     serverCount
#
# Desc:     Retrieves the number of servers.
#
# Input:    None
#
# Returns:  Number of servers or UNDEF if an error occurred.
##############################################################################
sub serverCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_SERVER_COUNT_CMD);
}

#ifdef ENGINEERING
##############################################################################
# Name:     serverCreate
#
# Desc:     Create a server
#
# In:       ID of a target
#           WWN
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverCreate
{
    my ($self, $tid, $owner, $mwwn, $iname) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['s'],
                ["serverCreate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }
    my $msg;

    my $wwn1 = (oct ("0x" . (substr $mwwn,0,8)));
    my $wwn2 = (oct ("0x" . (substr $mwwn,8)));

    my %wwn;
    $wwn{HI_LONG} = $wwn2;
    $wwn{LO_LONG} = $wwn1;


    if (!defined($wwn{HI_LONG}))
    {
        $wwn{HI_LONG} = 0;
    }
    if (!defined($wwn{LO_LONG}))
    {
        $wwn{LO_LONG} = 0;
    }

    my $cmd = PI_SERVER_CREATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSLNNa256",
                    $tid,
                    0,
                    $owner,
                    $wwn{LO_LONG},
                    $wwn{HI_LONG},
                    $iname);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_serverCreatePacket);
}
#endif

##############################################################################
# Name:     serverDelete
#
# Desc:     Delete a server
#
# In:       ID of a server
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverDelete
{
    my ($self, $sid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["serverDelete"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    return $self->serverDeleteEx($sid, 0);
}

##############################################################################
# Name:     serverDeleteEx
#
# Desc:     Delete a server
#
# In:       ID of a server
#           Option for the command.
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverDeleteEx
{
    my ($self, $sid, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 3],
                ["serverDeleteEx"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVER_DELETE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $sid,
                    $option);

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
# Name:     serverDisassociate
#
# Desc:     Disassociate a server
#
# In:       SID - server identifier
#           LUN - Lun to use for the map
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverDisassociate
{
    my ($self, $sid, $lun, $vid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["serverDisassociate"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVER_DISASSOCIATE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SSSS",
                    $sid,
                    0,
                    $lun,
                    $vid);

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
# Name:     serverInfo
#
# Desc:     Get information about a server
#
# In:       ID of a server
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverInfo
{
    my ($self, $sid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["serverInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_SERVER_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SS",
                    $sid,
                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_serverInfoPacket);
}

##############################################################################
# Name:     serverWwnToTargetMap
#
# Desc:     Get the list of active servers and associated Target map
#
# In:       none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub serverWwnToTargetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_SERVER_WWN_TO_TARGET_MAP_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_serverWwnToTargetPacket);
}


##############################################################################
# Name:     serverList
#
# Desc:     Retrieves an array containing the identifiers of the servers.
#
# Input:    None
#
# Returns:
##############################################################################
sub serverList
{
    my ($self) = @_;
    return $self->getObjectList(PI_SERVER_LIST_CMD);
}


##############################################################################
# Name:     getWorksetInfo
#
# Desc:     Get workset information
#
# In:       ID of a workset.  0xFFFF=all worksets are returned
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub getWorksetInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["worksetInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_MISC_GET_WORKSET_INFO_CMD;
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
                                        \&_getWorksetInfoPacket);
}

##############################################################################
# Name:     setWorksetInfo
#
# Desc:     Set workset information
#
# In:       ID of a workset.  
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub setWorksetInfo
{
    my ($self, $id, $name, $vBlkList, $serverList, $clearWorksets, $defaultVPort) = @_;

    logMsg("begin\n");

    # Set up and send the command
    my $cmd = PI_MISC_SET_WORKSET_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data;

    # If the request was to clear a workset,  write 0's to all fields.
    if ($clearWorksets == 1)
    {
        my $args = [['i'],
                    ['d', 0, 0x000F],
                    ["setWorksetInfo"]];

        my %vp = verifyParameters(\@_, $args);
        if (%vp)
        {
            return %vp;
        }

        printf "CLEAR workset %d\n", $id;

        $data = pack("S a16 S LLLL LLLL C",
                     $id,
                     "empty",
                     0,
                     0,0,0,0,0,0,0,0,
                     0xFF
                     );
    }
    else
    {
        # verify parameters
        my $args = [['i'],
                    ['d', 0, 0x000F],
                    ['s'],
                    ['a'],
                    ['a'],
                    ['d', 0, 0xFF],
                    ["setWorksetInfo"]];

        my %vp = verifyParameters(\@_, $args);
        if (%vp)
        {
            return %vp;
        }

        printf "Setting info for workset %d", $id;

        # Get local copies of the arrays which were passed by reference
        my @vBlks = @$vBlkList;
        my @servers = @$serverList;

        # Build bitmaps from the arrays
        my $vBlkMap = BuildBitmap(16, \@vBlks);
        my $serverMap = BuildBitmap(256, \@servers);

            $data = pack("S a16 a2 a32 C",
                        $id,
                        $name,
                        $vBlkMap,
                        $serverMap,
                        $defaultVPort
                        );
    }

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
# Name: displayServers
#
# Desc: Print the servers
#
# In:   Servers Information Hash
##############################################################################
sub displayServers
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "Servers ($info{COUNT} servers):\n";
    $msg .= sprintf  "\n";

    $msg .= sprintf " SID  TID  STATUS   PRI   ATTRIBUTES  SESSION     REQCNT       OWNER      SERVER NAME           WWN          INITIATOR NAME \n";
    $msg .= sprintf " ---  ---  ------  -----  ----------  -------  ------------  ---------  ----------------  ----------------  ----------------\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
         $msg .= sprintf " %3hu  %3hu    0x%2.2x   0x%2.2x  0x%8.8x  %7lu  %12s  %9lu  %16s  %8.8x%8.8x  %s\n",
                $info{SERVERS}[$i]{SID},
                $info{SERVERS}[$i]{TARGETID},
                $info{SERVERS}[$i]{SSTATUS},
                $info{SERVERS}[$i]{PRI},
                $info{SERVERS}[$i]{ATTRIB},
                $info{SERVERS}[$i]{SESSION},
                $info{SERVERS}[$i]{REQCNT},
                $info{SERVERS}[$i]{OWNER},
                $info{SERVERS}[$i]{NAME},
                $info{SERVERS}[$i]{WWN_LO}, $info{SERVERS}[$i]{WWN_HI},
                $info{SERVERS}[$i]{INAME};
    }

    $msg .= sprintf "\n";

    return $msg;
}

#ifdef ENGINEERING
##############################################################################
# Name: displayServerCreate
#
# Desc: Print the server creation information.
#
# In:   Server Create Information Hash
##############################################################################
sub displayServerCreate
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    print "Server Create Results:\n";
    printf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    printf "  LEN:                   %lu\n", $info{LEN};
    printf "  SID:                   %hu\n", $info{SID};
    print "\n";
}
#endif

##############################################################################
# Name: displayServerInfo
#
# Desc: Print the server information
#
# In:   Server Information Hash
##############################################################################
sub displayServerInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf "Server Information:\n";
    $msg .= sprintf "  STATUS:                0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                   %lu\n", $info{LEN};
    $msg .= sprintf "  SID:                   %hu\n", $info{SID};
    $msg .= sprintf "  NLUNS:                 %hu\n", $info{NLUNS};
    $msg .= sprintf "  TARGETID:              %hu\n", $info{TARGETID};
    $msg .= sprintf "  SSTATUS:               %hu\n", $info{SSTATUS};
    $msg .= sprintf "  PRI:                   0x%02x\n", $info{PRI};
    $msg .= sprintf "  ATTRIB:                0x%08x\n", $info{ATTRIB};
    $msg .= sprintf "  SESSION:               %lu\n", $info{SESSION};
    $msg .= sprintf "  REQCNT:                $info{REQCNT}\n";
    $msg .= sprintf "  OWNER:                 %lu\n", $info{OWNER};
    $msg .= sprintf "  SERVER NAME:           $info{NAME}\n";
    $msg .= sprintf "  WWN:                   %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
#    $msg .= sprintf "  WWN:                   $info{WWN}\n";
    $msg .= sprintf "  INITIATOR NAME:        $info{INAME}\n";

    $msg .= sprintf "\n";

    $msg .= sprintf  "  LUN   VID\n";
    $msg .= sprintf  "  ---   ----\n";
    for (my $i = 0; $i < $info{NLUNS}; $i++)
    {
        $msg .= sprintf  "  ";
        $msg .= sprintf "%3d", $info{LUNMAP}[$i]{LUN};
        $msg .= sprintf  "   ";
        $msg .= sprintf "%4d", $info{LUNMAP}[$i]{VID};
        $msg .= sprintf  "\n";
    }

    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
# Name: displayServerWwnToTargetMap
#
# Desc: Print the Server WWN to Target Map information
#
# In:   Server WWN to Port Map Hash
##############################################################################
sub displayServerWwnToTargetMap
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    printf "Active Server count: %d\n", $info{COUNT};
    print "\n";

    print  "  WWN                Target Bit Map\n";
    print  "  ---                --------------\n";
    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        print  "  ";
        printf "%8.8x%8.8x", $info{WWNTARGETMAP}[$i]{WWN_LO}, $info{WWNTARGETMAP}[$i]{WWN_HI};
        print  "   ";
        printf "0x%02x\n", $info{WWNTARGETMAP}[$i]{MAP};
        print  "\n";
    }

    print "\n";
}

##############################################################################
# Name: displayWorksetInfo
#
# Desc: Print the Workset information
#
# In:   Workset Information Hash
##############################################################################
sub displayWorksetInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";
    my @vBlkMap;
    my @serverMap;

    $msg .= sprintf "Workset Information for %d workset(s):\n", $info{COUNT};
    $msg .= sprintf "\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        if ($info{COUNT} > 1)
        {
            $msg .= sprintf "ID: %d\n", $i;
        }

        $msg .= sprintf "Name         : %s\n", $info{WORKSET}[$i]{NAME};

        @vBlkMap = ParseBitmap($info{WORKSET}[$i]{VBLKBITMAP});
        $msg .= sprintf "VBlock Bitmap: @vBlkMap\n";

        @serverMap = ParseBitmap($info{WORKSET}[$i]{SERVERBITMAP});
        $msg .= sprintf "Server Bitmap: @serverMap\n";

        $msg .= sprintf "Default VPort: %d\n\n", $info{WORKSET}[$i]{DEFAULTVPORT};
    }

    return $msg;

}


##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

##############################################################################
# Name:     _serversPacket
#
# Desc:     Parses the servers packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _serversPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_SERVERS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @servers;

        my $start = 4;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $rsvd;
            my %reqcnt;
            my %wwn;

            # Unpack the data
            (
            $rsvd,
            $servers[$i]{STATUS_MRP},
            $servers[$i]{LEN},
            $servers[$i]{SID},
            $servers[$i]{NLUNS},
            $servers[$i]{TARGETID},
            $servers[$i]{SSTATUS},
            $servers[$i]{PRI},

            $servers[$i]{ATTRIB},
            $servers[$i]{SESSION},
            $reqcnt{LO_LONG}, $reqcnt{HI_LONG},

            $rsvd,
            $servers[$i]{OWNER},
            $wwn{LO_LONG}, $wwn{HI_LONG},

            $rsvd,
            $servers[$i]{NAME},
            $servers[$i]{INAME},
#            ) = unpack("a3CLSSSCC LLLL a4LNN a8a16", substr($parts{DATA}, $start));
            ) = unpack("a3CLSSSCC LLLL a4LNN a8A16A256", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $servers[$i]{REQCNT} = longsToBigInt(%reqcnt);
#            $servers[$i]{WWN} = longsToBigInt(%wwn);
            $servers[$i]{WWN_HI} = $wwn{HI_LONG};
            $servers[$i]{WWN_LO} = $wwn{LO_LONG};

            $servers[$i]{WWN} = $servers[$i]{INAME};

#            $start += 72;
            $start += 328;

            my @lunmap;
            my $luns = $servers[$i]{NLUNS};

            if ($luns > 0)
            {
                for (my $j = 0; $j < $luns; $j++)
                {
                    (
                    $lunmap[$j]{VID},
                    $lunmap[$j]{LUN}
                    ) = unpack("SS", substr($parts{DATA}, $start, 4));

                    $start += 4;
                }

                $servers[$i]{LUNMAP} = [@lunmap];
            }
        }

        $info{SERVERS} = [@servers];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a servers packet\n");
    }

    return %info;
}

#ifdef ENGINEERING
##############################################################################
# Name:     _serverCreatePacket
#
# Desc:     Parses the virtual disk info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _serverCreatePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_SERVER_CREATE_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{SID},
        $rsvd
        ) = unpack("a3CLSa2", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a create server packet\n");
    }

    return %info;
}
#endif

##############################################################################
# Name:     _serverInfoPacket
#
# Desc:     Parses the server info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _serverInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_SERVER_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %reqcnt;
        my %wwn;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{SID},
        $info{NLUNS},
        $info{TARGETID},
        $info{SSTATUS},
        $info{PRI},

        $info{ATTRIB},
        $info{SESSION},
        $reqcnt{LO_LONG}, $reqcnt{HI_LONG},

        $rsvd,
        $info{OWNER},
        $wwn{LO_LONG}, $wwn{HI_LONG},

        $rsvd,
        $info{NAME},
        $info{INAME}
#        ) = unpack("a3CLSSSCC LLLL a4LNN a8a16", $parts{DATA});
        ) = unpack("a3CLSSSCC LLLL a4LNN a8a16a256", $parts{DATA});

        my @lunmap;

        for ($i = 0; $i < $info{NLUNS}; $i++)
        {
            my $start = 72 + 256 + (4 * $i);

            (
            $lunmap[$i]{VID},
            $lunmap[$i]{LUN}
            ) = unpack("SS", substr($parts{DATA}, $start));
        }

        $info{LUNMAP} = [@lunmap];

        # Now fixup all the 64 bit  numbers
        $info{REQCNT} = longsToBigInt(%reqcnt);
#        $info{WWN} = longsToBigInt(%wwn);
        $info{WWN_HI} = $wwn{HI_LONG};
        $info{WWN_LO} = $wwn{LO_LONG};
        
        $info{WWN} = $info{INAME};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a server info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _serverWwnToTargetPacket
#
# Desc:     Parses the Server WWN to Target packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _serverWwnToTargetPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_SERVER_WWN_TO_TARGET_MAP_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %reqcnt;
        my %wwn;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
            $info{COUNT},
            $rsvd
        ) = unpack("SCC", $parts{DATA});

        my @targetMap;

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (12 * $i);

            (
                $wwn{LO_LONG}, $wwn{HI_LONG},
                $targetMap[$i]{MAP},
                $rsvd
            ) = unpack("NNCa3", substr($parts{DATA}, $start));

            $targetMap[$i]{WWN} = longsToBigInt(%wwn);
            $targetMap[$i]{WWN_HI} = $wwn{HI_LONG};
            $targetMap[$i]{WWN_LO} = $wwn{LO_LONG};
        }

        $info{WWNTARGETMAP} = [@targetMap];

    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a server info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _getWorksetInfoPacket
#
# Desc:     Parses the workset info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _getWorksetInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_MISC_GET_WORKSET_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the count to determine how many worksets follow
        (
        $info{COUNT},
        $rsvd
        ) = unpack("SS", $parts{DATA});

        my @workset;

        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (51 * $i);

            (
            $workset[$i]{NAME},
            $workset[$i]{VBLKBITMAP},
            $workset[$i]{SERVERBITMAP},
            $workset[$i]{DEFAULTVPORT}
            ) = unpack("a16a2a32CC", substr($parts{DATA}, $start));
        }

        $info{WORKSET} = [@workset];
        
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a workset info packet\n");
    }

    return %info;
}
1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
