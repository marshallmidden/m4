# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy, Mark Schibilla
#
# Purpose:
#   Wrapper for all the different XIOTech X1 Server commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;

# X1_SERVER_MAP
use constant X1_SERVER_MAP =>
           "C          # loopNum 
            a32        # bitMap
            C          # rsvd1
            C          # zoneMode - not used
            C          # defMode - not used
            C          # needKey - not used
            a32        # activeServerMap
            C";        # rsvd2

# X1_SERVER_STATS
use constant X1_SERVER_STATS =>
           "S          # serverNumber 
            C          # serverIndex
            L          # queueDepth
            LL         # Read requests
            LL         # Write requests
            L          # Requests per second
            L          # Average request size (avgSC)
            a10";      # rsvd2

# X1_HAB_STATS
use constant X1_HAB_STATS =>
           "C          # habNumber 
            L          # queueDepth
            L          # Requests per second
            a30";      # rsvd2

# X1_MASK_INFO
use constant X1_MASK_INFO =>
           "S          # serverId
            NN         # wwn  
            a2         # targetBitMap
            C          # selectedTarget
            C          # status
            a16        # sName
            C";        # lookupIndex





##############################################################################
# Name:     X1ServerGetMap
#
# Desc:     Get the Server Map
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1ServerGetMap
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_SERVER_MAP, undef);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_SERVER_MAP,
            \&_handleResponseX1GetServerMap);

    if (%rsp)
    {
        logMsg("X1ServerGetMap successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1ServerGetMap failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1ServerGetStats
#
# Desc:     Get the Server Stats
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1ServerGetStats
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1ServerGetStats"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_SERVER_STATS, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_SERVER_STATS,
            \&_handleResponseX1GetServerStats);

    if (%rsp)
    {
        logMsg("X1ServerGetStats successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1ServerGetStats failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1HABGetStats
#
# Desc:     Get the HAB Stats
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1HABGetStats
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1HABGetStats"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_HAB_STATS, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_HAB_STATS,
            \&_handleResponseX1GetHABStats);

    if (%rsp)
    {
        logMsg("X1HABGetStats successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1HABGetStats failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetMaskInfo
#
# Desc:     Get the Server Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetMaskInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetMaskInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_MASK_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_MASK_INFO,
            \&_handleResponseX1GetMaskInfo);

    if (%rsp)
    {
        logMsg("X1GetMaskInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetMaskInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetLunsInfo
#
# Desc:     Get the LUNs Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetLunsInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetLunsInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_LUNS_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_LUNS_INFO,
            \&_handleResponseX1GetLunsInfo);

    if (%rsp)
    {
        logMsg("X1GetLunsInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetMaskInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GetVPortInfo
#
# Desc:     Get the VPort (Target) Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetVPortInfo
{
    my ($self, $id) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["X1GetVPortInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    my $data = pack("S",
                    $id);

    my $packet = assembleX1Packet(X1PKT_GET_VPORT_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_GET_VPORT_INFO,
            \&_handleResponseX1GetVPortInfo);

    if (%rsp)
    {
        logMsg("X1GetVPortInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GetVPortInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1WorksetGetInfo
#
# Desc:     Get the X1 Workset Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1WorksetGetInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_WORKSET_INFO, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
							            X1RPKT_GET_WORKSET_INFO,
							            \&_handleResponseX1WorksetGetInfo);

    if (%rsp)
    {
        logMsg("X1WorksetGetInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1WorksetGetInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GeoPoolGetInfo
#
# Desc:     Get the X1 GeoPool Info
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GeoPoolGetInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

    my $packet = assembleX1Packet(X1PKT_GET_GEOPOOL_INFO, undef);

    %rsp = $self->_handleX1SyncResponse($packet,
							            X1RPKT_GET_GEOPOOL_INFO,
							            \&_handleResponseX1GeoPoolGetInfo);

    if (%rsp)
    {
        logMsg("X1GeoPoolGetInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GeoPoolGetInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     X1GeoPoolSetInfo
#
# Desc:     Send a canned set of data to test the X1 Set GeoPool interface.
#			User data can be entered using the PI interface to Set GeoPool.
#
# Input:
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GeoPoolSetInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    my $rc;
    my %rsp;

	# Build up 4 pools of data to send via X1
    my $data = pack("S SSA16 SSA16 SSA16 SSA16",
                    0xAAAA,			# primaryControllerBitmap

                    0x00C0,			# controllerBitmap
                    0x0300,			# bayBitmap
					"GeoPool_0",	# name

                    0x0300,			# controllerBitmap
                    0x0C00,			# bayBitmap
					"GeoPool_1",	# name

                    0x0C00,			# controllerBitmap
                    0x3000,			# bayBitmap
					"GeoPool_2",	# name

                    0x3000,			# controllerBitmap
                    0xC000,			# bayBitmap
					"GeoPool_3"		# name
                    );

    my $packet = assembleX1Packet(X1PKT_SET_GEOPOOL_INFO, $data);

    %rsp = $self->_handleX1SyncResponse(
							            $packet,
							            X1RPKT_SET_GEOPOOL_INFO,
							            \&_handleResponseX1StatusOnlyGood0);

    if (%rsp)
    {
        logMsg("X1GeoPoolSetInfo successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1GeoPoolSetInfo failed\n");
        $rc = 0;
    }

    return %rsp;
}

##############################################################################
# Name:     DisplayX1ServerMap
#
# Desc:     Display an X1 Server map
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1ServerMap
{
    my (%info) = @_;

    my @servers = ParseBitmap($info{SERVERMAP});
    my @activeServers = ParseBitmap($info{ACTIVESERVERMAP});

    print "Server Bitmap: @servers\n";
    print "Active Server Bitmap: @activeServers\n";

    print "\n";
}

##############################################################################
# NAME:     DisplayX1ServerStats
#
# DESC:     Print the X1 server statistics
#
# INPUT:    Server Statistics Hash
##############################################################################
sub DisplayX1ServerStats
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf " X1 Server Statistics:\n";
    $msg .= sprintf "  Server Number:        %d\n", $info{SERVERNUM};
    $msg .= sprintf "  Server Index:         %d\n", $info{SERVERINDEX};
    $msg .= sprintf "  Queue Depth:          $info{QDEPTH}\n";
    $msg .= sprintf "  Read Requests:        $info{RDREQ}\n";
    $msg .= sprintf "  Write Requests:       $info{WRTREQ}\n";
    $msg .= sprintf "  Requests per Second:  $info{REQPERSEC}\n";
    $msg .= sprintf "  Average Request Size: $info{AVGSC}\n";
    $msg .= sprintf "\n";

    return $msg;
}


##############################################################################
# NAME:     DisplayX1HABStats
#
# DESC:     Print the X1 HAB statistics
#
# INPUT:    HAB Statistics Hash
##############################################################################
sub DisplayX1HABStats
{
    my ($self, %info) = @_;

    my $msg = "";

    logMsg("begin\n");

    $msg .= sprintf " X1 HAB Statistics:\n";
    $msg .= sprintf "  HAB Number:           %d\n", $info{HABNUM};
    $msg .= sprintf "  Queue Depth:          $info{QDEPTH}\n";
    $msg .= sprintf "  Requests per Second:  $info{REQPERSEC}\n";
    $msg .= sprintf "\n";

    return $msg;
}

##############################################################################
# Name:     DisplayX1MaskInfo
#
# Desc:     Display an X1 Server map
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1MaskInfo
{
    my ($self, %info) = @_;

    my @targets = ParseBitmap($info{TARGETBITMAP});

    print "Mask Info (Server Info):\n";
    printf "  Server ID:        %hu\n", $info{SERVERID};
    printf "  WWN:              %8.8x%8.8x\n", $info{WWN_LO}, $info{WWN_HI};
    print  "  Target bit map:   @targets \n";
    printf "  Selected target:  %hu\n", $info{SELECTEDTARGET};
    printf "  Status:           0x%02x\n", $info{SERVERSTATUS};
    printf "  Name:             %s\n", $info{SERVERNAME};
    printf "  Lookup Index:     %d\n", $info{LOOKUPINDEX};

    print "\n";
}

##############################################################################
# Name:     DisplayX1VPortInfo
#
# Desc:     Display an X1 VPort (Target) Info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1VPortInfo
{
    my ($self, %info) = @_;

    print "VPort (Target) Info:\n";
    printf "  VPort ID:                     %u\n", $info{VPORTID};
    printf "  HBA number:                   %u\n", $info{HBAID};
    printf "  Requested fibre port ID:      %u\n", $info{REQFCID};
    printf "  Actual fibre port ID:         %u\n", $info{ACTFCID};
    printf "  Port WWN:                     %8.8X%8.8X\n", $info{PORT_WWN_LO}, $info{PORT_WWN_HI};
    printf "  Node WWN:                     %8.8X%8.8X\n", $info{NODE_WWN_LO}, $info{NODE_WWN_HI};
    printf "  Controller node SN:           %u\n", $info{CTRLNODESN};
    printf "  Loop status:                  0x%02x\n", $info{LOOPSTATUS};

    print "\n";

}

##############################################################################
# Name:     DisplayX1LunsInfo
#
# Desc:     Display an X1 LUNs info
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1LunsInfo
{
    my ($self, %info) = @_;

    printf "LUN Info for Server ID: %d  Number of LUNs: %d  Lookup Index: %d\n\n", 
           $info{SERVERID}, $info{NUMLUNS}, $info{LOOKUPINDEX};

    print  "  LUN     VID      TID\n";
    print  "  ---     ---      ---\n";
    for (my $i = 0; $i < $info{NUMLUNS}; $i++)
    {
        print  "  ";
        printf "%3d   ", $info{LUNMAP}[$i]{LUN};
        printf "%5d    ", $info{LUNMAP}[$i]{VID};
        printf "%5d", $info{LUNMAP}[$i]{TID};
        print  "\n";
    }

    print "\n";
}

##############################################################################
# Name: DisplayX1WorksetInfo
#
# Desc: Print the Workset information
#
# In:   Workset Information Hash
##############################################################################
sub DisplayX1WorksetInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

	my @vBlkMap;
	my @serverMap;

    printf "Workset Information:\n\n";

    for (my $i = 0; $i < 16; $i++)
    {
    	printf "ID: %d\n", $i;

        printf "Name         : %s\n", $info{X1WORKSET}[$i]{NAME};

		@vBlkMap = ParseBitmap($info{X1WORKSET}[$i]{VBLKBITMAP});
    	printf "VBlock Bitmap: @vBlkMap\n";

		@serverMap = ParseBitmap($info{X1WORKSET}[$i]{SERVERBITMAP});
    	printf "Server Bitmap: @serverMap\n";

        printf "Default VPort: %d\n\n", $info{X1WORKSET}[$i]{DEFAULTVPORT};
    }
}

##############################################################################
# Name: DisplayX1GeoPoolInfo
#
# Desc: Print the GeoPool information
#
# In:   GeoPool Information Hash
##############################################################################
sub DisplayX1GeoPoolInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";
    my @ctrlMap;
    my @bayMap;
    my @pcMap;

    printf "X1 GeoPool Information:\n\n";

    @pcMap = ParseBitmap($info{PCBITMAP});
    printf "Primary controller bitmap: @pcMap\n\n";

    for (my $i = 0; $i < 4; $i++)
    {
        printf "ID: %d\n", $i;

        @ctrlMap = ParseBitmap($info{GEOPOOL}[$i]{CTRLBITMAP});
        printf "Controller Bitmap: @ctrlMap\n";

        @bayMap = ParseBitmap($info{GEOPOOL}[$i]{BAYBITMAP});
        printf "Disk Bay Bitmap: @bayMap\n";

        printf "Name: %s\n\n", $info{GEOPOOL}[$i]{NAME};
    }
}

##############################################################################
# Name:     _handleResponseX1GetServerMap
#
# Desc:     Handle an X1 Get Server Map return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetServerMap
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{LOOPNUM},
      $info{SERVERMAP},
      $info{RSVD1},
      $info{ZONEMODE},
      $info{DEFMODE},
      $info{NEEDKEY},
      $info{ACTIVESERVERMAP}) = unpack X1_SERVER_MAP, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetServerStats
#
# Desc:     Handle an X1 Get Server Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetServerStats
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %rdReq;
    my %wrtReq;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{SERVERNUM},
      $info{SERVERINDEX},
      $info{QDEPTH},
      $rdReq{LO_LONG}, $rdReq{HI_LONG},
      $wrtReq{LO_LONG}, $wrtReq{HI_LONG},
      $info{REQPERSEC},
      $info{AVGSC},
      $rsvd) = unpack X1_SERVER_STATS, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{RDREQ} = longsToBigInt(%rdReq);
    $info{RDREQ_HI} = $rdReq{HI_LONG};
    $info{RDREQ_LO} = $rdReq{LO_LONG};
    
    $info{WRTREQ} = longsToBigInt(%wrtReq);
    $info{WRTREQ_HI} = $wrtReq{HI_LONG};
    $info{WRTREQ_LO} = $wrtReq{LO_LONG};
    
    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetHABStats
#
# Desc:     Handle an X1 Get Server Stats return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetHABStats
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %rdReq;
    my %wrtReq;
    my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{HABNUM},
      $info{QDEPTH},
      $info{REQPERSEC},
      $rsvd) = unpack X1_HAB_STATS, $parts{DATA};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1GetMaskInfo
#
# Desc:     Handle an X1 Get Mask Info (i.e. Server Info) return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetMaskInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wwn;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{SERVERID},
      $wwn{LO_LONG}, $wwn{HI_LONG},
      $info{TARGETBITMAP},
      $info{SELECTEDTARGET},
      $info{SERVERSTATUS},
      $info{SERVERNAME},
      $info{LOOKUPINDEX}) = unpack X1_MASK_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{WWN} = longsToBigInt(%wwn);
    $info{WWN_HI} = $wwn{HI_LONG};
    $info{WWN_LO} = $wwn{LO_LONG};

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetLunsInfo
#
# Desc:     Handle an X1 Get LUNs Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GetLunsInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %wwn;
    my $i;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{SERVERID},
      $info{NUMLUNS},
      $info{LOOKUPINDEX}) = unpack "SSC", $parts{DATA};

    my @lunmap;

    for ($i = 0; $i < $info{NUMLUNS}; $i++)
    {
        my $start = 5 + (6 * $i);

        (
        $lunmap[$i]{VID},
        $lunmap[$i]{LUN},
        $lunmap[$i]{TID}
        ) = unpack("SSS", substr($parts{DATA}, $start));
    }

    $info{LUNMAP} = [@lunmap];

    return %info;
}


##############################################################################
# Name:     _handleResponseX1GetVPortInfo
#
# Desc:     Handle an X1 Get VPort Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
# X1_VPORT_INFO
use constant X1_VPORT_INFO =>
           "S           # vportNumber
            C           # feHBANumber
            C           # requestedFCID
            C           # actualFCID
            NN          # portWwn
            NN          # nodeWwn
            L           # ctrlNodeSN
            C";         # loopStatus

sub _handleResponseX1GetVPortInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my %portwwn;
    my %nodewwn;
    my $i;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{VPORTID},
      $info{HBAID},
      $info{REQFCID},
      $info{ACTFCID},
      $portwwn{LO_LONG}, $portwwn{HI_LONG},
      $nodewwn{LO_LONG}, $nodewwn{HI_LONG},
      $info{CTRLNODESN},
      $info{LOOPSTATUS}) = unpack X1_VPORT_INFO, $parts{DATA};

    # Now fixup all the 64 bit  numbers
    $info{PORTWWN} = longsToBigInt(%portwwn);
    $info{PORT_WWN_HI} = $portwwn{HI_LONG};
    $info{PORT_WWN_LO} = $portwwn{LO_LONG};

    $info{NODEWWN} = longsToBigInt(%nodewwn);
    $info{NODE_WWN_HI} = $nodewwn{HI_LONG};
    $info{NODE_WWN_LO} = $nodewwn{LO_LONG};

    return %info;
}

##############################################################################
# Name:     _handleResponseX1WorksetGetInfo
#
# Desc:     Handle an X1 Get Workset Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1WorksetGetInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
	my $rsvd;

    my %parts = disassembleX1Packet($recvPacket);

    my @x1Workset;

    for ($i = 0; $i < 16; $i++)
    {
        my $start = 51 * $i;

        (
        $x1Workset[$i]{NAME},
        $x1Workset[$i]{VBLKBITMAP},
        $x1Workset[$i]{SERVERBITMAP},
        $x1Workset[$i]{DEFAULTVPORT},
        ) = unpack("a16 a2 a32 C", substr($parts{DATA}, $start));
    }

    $info{X1WORKSET} = [@x1Workset];

    return %info;
}



##############################################################################
# Name:     _handleResponseX1GeoPoolGetInfo
#
# Desc:     Handle an X1 Get GeoPool Info return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1GeoPoolGetInfo
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    my %parts = disassembleX1Packet($recvPacket);

    # Unpack the primary controller bitmap
    (
    $info{PCBITMAP}
    ) = unpack("a2", $parts{DATA});

    my @geoPools;

    for ($i = 0; $i < 4; $i++)
    {
        my $start = 2 + (20 * $i);

        (
        $geoPools[$i]{CTRLBITMAP},
        $geoPools[$i]{BAYBITMAP},
        $geoPools[$i]{NAME}
        ) = unpack("a2a2a16", substr($parts{DATA}, $start));
    }

    $info{GEOPOOL} = [@geoPools];

    return %info;
}



1;

##############################################################################
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.11  2004/06/15 18:43:05  SchibillaM
# TBolt00010632: Add support for X1 Server and HBA stats.  New Stats Manager
# component.  HBA stats framework done, waiting for proc support.  Reviewed by Chris.
#
# Revision 1.10  2003/08/22 17:46:30  SchibillaM
# TBolt00008030: Remove wasteful reserved byte from workset struct.  Approved
# by Jeff.
#
# Revision 1.9  2003/08/19 20:58:44  SchibillaM
# TBolt00008962: Add defaultVPort to workset.
#
# Revision 1.8  2003/07/25 18:32:55  SchibillaM
# TBolt00008793: X1GeoPool get and set support.  Reviewed by Randy.
#
# Revision 1.7  2003/07/18 19:16:15  SchibillaM
# TBolt00008030: Complete X1 Workset support.  Reviewed by Randy.
#
# Revision 1.6  2003/07/16 13:08:19  SchibillaM
# TBolt00008030: Initial workset support.  X1 changes to set worksets not complete.
# Reviewed by Chris.
#
# Revision 1.5  2003/04/22 13:39:32  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.4  2003/02/27 22:49:10  NigburC
# TBolt00007472 - Changed the server bitmap to be 32 bytes, large enough to
# support 256 servers.  Added a new lookup table to kludge a packed server
# bitmap index value to a semi-real server ID which is then translated into the
# PROC server ID of the selected target.  Added additional fields in the mask
# and luns info to return the bitmap index value.  Added lookups in the get
# mask and get luns code to take a lookup value instead of a real SID.
# Bumped the version number for XSSA compatibility to 0x0D.
# Reviewed by Mark Schibilla.
#
# Revision 1.3  2003/01/13 16:50:40  SchibillaM
# TBolt00006514: Add support for VPortInfo
#
# Revision 1.2  2002/12/13 16:32:46  SchibillaM
# TBolt00006408: Add support for X1GetLunsInfo.  Fix bug that prevented
# Zone Changed Notification from being sent on associate - disassociate.
# Reviewed by Chris.
#
# Revision 1.1  2002/12/09 17:42:50  SchibillaM
# TBolt00006408: Initial server changes.  Report only unique (physical) servers
# in the server map.  Mask Info (server info) target bit map combines all server
# records with the same wwn.  Reviewed by Chris.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
