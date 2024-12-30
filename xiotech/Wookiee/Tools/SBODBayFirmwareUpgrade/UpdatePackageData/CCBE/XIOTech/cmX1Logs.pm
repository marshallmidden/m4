# $Header$
##############################################################################
# Xiotech
# Copyright (c) 2002  Xiotech
# ======================================================================
# $RCSfile$
# Author: Bryan Holty
#
# Purpose:
#   X1 Logging interface
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use Time::Local;
use POSIX qw(strftime);
use strict;

# X1_SERVER_MAP
use constant X1_LOG_ENTRY_REQ =>
           "S          # someNum 
            L";        # sequence number


# X1_MASK_INFO
use constant X1_LOG_ENTRY_RSP =>
           "S          # someNum 
            L          # sequence number
            L          # timestamp
            C          # type
            C          # extended type
            C          # flags
            C          # status
            a40";      # message




##############################################################################
# Name:     X1AnnotateLog
#
# Desc:     Annotate a log.
#
# Input:    severity, message.
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1AnnotateLog
{
    my ($self, $severity, $message) = @_;

    logMsg("begin\n");

        # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ['s'],
                ["X1AnnotateLog"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    
    my $data = pack("CCa40", 0xb4, $severity, $message);
    
    my $packet = assembleX1Packet(X1PKT_SET_LOG, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_SET_LOG,
            \&_handleX1AnnotateLog);

    if (%rsp)
    {
        logMsg("X1AnnotateLog successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1AnnotateLog failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     X1GetLogEntry
#
# Desc:     Get a log entry.
#
# Input:    seqNbr - Sequence number to retrieve.
#
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information:
#
#       STATUS                  Status of the command
#
##############################################################################
sub X1GetLogEntry
{
    my ($self, $seqNbr) = @_;

    logMsg("begin\n");

        # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFFFFFF],
                ["X1LogEntry"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc;
    my %rsp;
    
    my $someNum = int(rand 0xFFFF);
    
    my $data = pack(X1_LOG_ENTRY_REQ,
                    $someNum, $seqNbr);

    my $packet = assembleX1Packet(X1PKT_LOG_ENTRY, $data);

    %rsp = $self->_handleX1SyncResponse(
            $packet,
            X1RPKT_LOG_ENTRY,
            \&_handleResponseX1LogEntry);

    if (%rsp)
    {
        logMsg("X1LogEntry successful\n");
        $rc = 1;
    }
    else
    {
        logMsg("X1LogEntry failed\n");
        $rc = 0;
    }

    return %rsp;
}


##############################################################################
# Name:     DisplayX1LogEntry
#
# Desc:     Display an X1 Log Entry
#
# Input:    hash containing 
#
##############################################################################
sub DisplayX1LogEntry
{
    my ($self, $gm_time, %info) = @_;
    my $tmstmp = 0;
    
    if (!defined($gm_time))
    {
        $gm_time = 0;
    }
    
    if ($gm_time == 1)
    {
        $tmstmp = sprintf "%s", 
            (strftime "%X %m/%d/%Y", gmtime($info{TIMESTAMP}));
    }
    else
    {
        $tmstmp = sprintf "%s", 
            (strftime "%X %m/%d/%Y", localtime($info{TIMESTAMP}));
    }

    print "Log Entry\n";
    printf "  Event Number:     %lu\n", $info{EVENTNUM};
    printf "  TimeStamp:        (%lu), %s\n", $info{TIMESTAMP}, $tmstmp;

    printf "  Type:             (0x%02X), %s\n", $info{TYPE}, 
            $self->_DisplayX1LogEntryGetLogType($info{TYPE});
    
    printf "  EType:            (0x%02X), %s\n", $info{ETYPE},
            $self->_DisplayX1LogEntryGetLogEtype($info{ETYPE});
            
    printf "  Flags:            (0x%02X), %s\n", $info{FLAGS},
            $self->_DisplayX1LogEntryGetFlags($info{FLAGS});
            
    printf "  Status:           (0x%02X)\n", $info{STATUS};
    printf "  Message:          %s\n", $info{MESSAGE};

    print "\n";
}

##############################################################################
# Name:     _handleX1AnnotateLog
#
# Desc:     Handle an X1 Annotate log.
#
# Input:    self
#           rx packet
#
# Returns:  Always good.
#
##############################################################################
sub _handleX1AnnotateLog
{
    my ($self,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    $info{STATUS} = PI_GOOD;

    return %info;
}


##############################################################################
# Name:     _handleResponseX1LogEntry
#
# Desc:     Handle an X1 Log Entry return packet.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _handleResponseX1LogEntry
{
    my ($self, $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    my %parts = disassembleX1Packet($recvPacket);

    ( $info{SOMENUM},
      $info{EVENTNUM},
      $info{TIMESTAMP},
      $info{TYPE},
      $info{ETYPE},
      $info{FLAGS},
      $info{STATUS},
      $info{MESSAGE}) = unpack X1_LOG_ENTRY_RSP, $parts{DATA};

    if ($info{EVENTNUM} == 0xFFFFFFFF)
    {
        $info{STATUS}       = PI_ERROR;
        $info{STATUS_MSG}   = "Invalid Sequence Number";
        $info{ERROR_CODE}   = 0;
        $info{ERROR_MSG}    = 
            "The sequence number given was not a valid sequence number";
    }

    
    return %info;
}


##############################################################################
# Name:     _DisplayX1LogEntryGetLogType
#
# Desc:     Determines String representation of type of log.
#
# Input:    type - numeral type
#
# Returns:  String type.
##############################################################################
sub _DisplayX1LogEntryGetLogType
{
    my ($self, $type) = @_;

    if      (!defined($type))   {return "Unknown";}
    elsif   ($type == 0xB0)     {return "Text Event";}
    elsif   ($type == 0xB1)     {return "MemDump Event";}
    elsif   ($type == 0xB2)     {return "User Logged In Or Out Event";}

    elsif   ($type == 0xE0)     {return "HUB - Miscellaneous Event";}
    elsif   ($type == 0xE1)     {return "HUB - Physical Drive Event";}
    elsif   ($type == 0xE2)     {return "HUB - RAID Event";}
    elsif   ($type == 0xE3)     {return "HUB - Virtual Disk Event";}
    elsif   ($type == 0xE4)     {return "Local HAB Event";}
    elsif   ($type == 0xE5)     {return "System HAB Event";}
    elsif   ($type == 0xE6)     {return "Server Driver Event";}
    elsif   ($type == 0xE7)     {return "Write Cache Event";}
    elsif   ($type == 0xE8)     {return "ISA Card Event";}
    elsif   ($type == 0xE9)     {return "EPC Event";}
    elsif   ($type == 0xEA)     {return "LAN Event";}
    elsif   ($type == 0xEB)     {return "Serial Event";}
    elsif   ($type == 0xEC)     {return "Environmental Event";}
    elsif   ($type == 0xED)     {return "System Event";}
    elsif   ($type == 0xEE)     {return "Debug Event";}
    elsif   ($type == 0xEF)     {return "Power Up Test Failure";}

    return "Unknown";
}

##############################################################################
# Name:     _DisplayX1LogEntryGetFlags
#
# Desc:     Determines String representation of flag of log.
#
# Input:    flag - numeral flag
#
# Returns:  String flag.
##############################################################################
sub _DisplayX1LogEntryGetFlags
{
    my ($self, $flag) = @_;
    my $fflag;

    if      (!defined($flag))               {$fflag = "Unknown";}
    elsif   ((($flag >> 2) & 0x01) == 0)    {$fflag = "Unacknowledged";}
    elsif   ((($flag >> 2) & 0x01) == 1)    {$fflag = "Acknowledged";}
    else                                    {$fflag = "Unknown";}
    
    if      (!defined($flag))       {$fflag .= " - Unknown";}
    elsif   (($flag & 0x03) == 0)   {$fflag .= " - Info";}
    elsif   (($flag & 0x03) == 1)   {$fflag .= " - Warning";}
    elsif   (($flag & 0x03) == 2)   {$fflag .= " - Error";}
    elsif   (($flag & 0x03) == 3)   {$fflag .= " - Fatal";}
    else                            {$fflag .= " - Unknown";}
    
    

    return $fflag;
}

##############################################################################
# Name:     _DisplayX1LogEntryGetLogEtype
#
# Desc:     Determines String representation of etype of log.
#
# Input:    etype - numeral etype
#
# Returns:  String etype.
##############################################################################
sub _DisplayX1LogEntryGetLogEtype
{
    my ($self, $etype) = @_;

    #Define the different classes of MMC extended log defs
    #define MMC_ELOG_NORMAL         0x00
    #define MMC_ELOG_FATAL_BIT      0x80
    #define MMC_ELOG_POWERUP_BIT    0x40
    #define MMC_ELOG_ENV_BIT        0x20
    #define MMC_ELOG_USER_MSG       0
    #define MMC_ELOG_PS             1
    #define MMC_ELOG_HAB            2
    #define MMC_ELOG_UPS            3
    #define MMC_ELOG_DEBUG          4
    #define MMC_ELOG_DIAG           5
    #define MMC_ELOG_SCSI           6
    #define MMC_ELOG_MEMPROB        7
    #define MMC_ELOG_FAN            8
    #define MMC_ELOG_TEMP           9
    #define MMC_ELOG_PRED           10
    #define MMC_ELOG_CONTROLLER     14

    my $eetype;

    if      (!defined($etype))      {$eetype = "Unknown";}
    elsif   (($etype & 0x0F) == 0)  {$eetype = "User Message";}
    elsif   (($etype & 0x0F) == 1)  {$eetype = "Power Supply";}
    elsif   (($etype & 0x0F) == 2)  {$eetype = "Hab";}
    elsif   (($etype & 0x0F) == 3)  {$eetype = "Ups";}
    elsif   (($etype & 0x0F) == 4)  {$eetype = "Debug";}
    elsif   (($etype & 0x0F) == 5)  {$eetype = "Diag";}
    elsif   (($etype & 0x0F) == 6)  {$eetype = "Scsi";}
    elsif   (($etype & 0x0F) == 7)  {$eetype = "MemProb";}
    elsif   (($etype & 0x0F) == 8)  {$eetype = "Fan";}
    elsif   (($etype & 0x0F) == 9)  {$eetype = "Temp";}
    elsif   (($etype & 0x0F) == 10) {$eetype = "Pred";}
    elsif   (($etype & 0x0F) == 14) {$eetype = "Controller";}
    else                            {$eetype = "Unknown";}
    
    if      (!defined($etype))              {$eetype .= " - Unknown";}
    elsif   ((($etype >> 4) & 0x0F) == 0xb) {$eetype .= " - XSSA";}
    elsif   ((($etype >> 4) & 0x0F) == 8)   {$eetype .= " - Fatal";}
    elsif   ((($etype >> 4) & 0x0F) == 4)   {$eetype .= " - Powerup";}
    elsif   ((($etype >> 4) & 0x0F) == 2)   {$eetype .= " - Environmental";}
    elsif   ((($etype >> 4) & 0x0F) == 0)   {$eetype .= " - Normal";}
    else                                    {$eetype .= " - Unknown";}
    
    return $eetype    
}

1;

##############################################################################
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.2  2003/01/30 22:29:28  HoltyB
# TBolt00000000: Changes for X1 log acknowledged bit.
#
# Revision 1.1  2003/01/02 17:10:44  HoltyB
# TBolt00006385:  Added X1LogEntry command to retrieve log entries through
# the X1 packet interface.
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
