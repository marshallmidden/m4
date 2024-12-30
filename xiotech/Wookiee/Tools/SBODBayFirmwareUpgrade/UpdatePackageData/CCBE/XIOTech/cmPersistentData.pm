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

use Time::Local;
use POSIX qw(strftime);
use strict;

##############################################################################
# Name:		persistentDataControl
#
# Desc:     Write Persistent data.
#
# In:       None.
#
# Returns:  
#
##############################################################################
sub persistentDataControl
{
    my ($self, $option, $offset, $length, $data) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x03],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['s'],
                ["persistentDataControl"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %retData;
    my $cmd = PI_PERSISTENT_DATA_CONTROL_CMD;
    
    if (($option == PERSISTENT_DATA_OPTION_WRITE) ||
        ($option == PERSISTENT_DATA_OPTION_RESET) ||
        ($option == PERSISTENT_DATA_OPTION_CHECKSUM))
    {
        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $reqData = pack("Ca3LL", 
                       $option, 
                       0,
                       $offset, 
                       $length);
        
        $reqData .= $data; # tack on the data payload.
        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $reqData,
                                            $self->{PORT}, VERSION_1);

        %retData = $self->_handleSyncResponse($seq,
                                            $packet,
                                            \&_pDataControlResponsePacket);
    }
    else #option == PERSISTENT_DATA_OPTION_READ
    {
        my $allData;
        my $orgLength = $length;
        
        while ($length) {
            my $seq = $self->{SEQ}->nextId();
            my $ts = $self->{SEQ}->nextTimeStamp();
            my $reqData = pack("Ca3LL", 
                               $option, 
                               0,
                               $offset, 
                               $length);
    
            my $packet = assembleXiotechPacket($cmd,
                    $seq,
                    $ts,
                    $reqData,
                    $self->{PORT}, VERSION_1);

            %retData = $self->_handleSyncResponse($seq,
                    $packet,
                    \&_pDataControlResponsePacket);
                
            if($retData{STATUS} == 0) {
                $allData .= $retData{RD_DATA};
            }
            else {
                last;
            }

            $length -= $retData{LENGTH};
            $offset += $retData{LENGTH};
        }
    
        $retData{LENGTH} = $orgLength;
        $retData{RD_DATA} = $allData;
    }
    
    return %retData;
}

##############################################################################
# Name:		ewokDataControl
#
# Desc:     Write Persistent data.
#
# In:       None.
#
# Returns:  
#
##############################################################################
sub ewokDataControl
{
    my ($self, $option, $recname, $startByte, $length, $trunc, $data) = @_;
    my %retData;
    logMsg("begin\n");
    
    my $cmd = PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD;
        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $reqData = pack("SSLa256LLa256", 
                       $option, 
                       $trunc,
                       0,
                       $recname,
                       $startByte,  
                       $length,
                       $data);
        $reqData .= $data; # tack on the data payload.
        my $packet = assembleXiotechPacket($cmd,
                                            $seq,
                                            $ts,
                                            $reqData,
                                            $self->{PORT});

        %retData = $self->_handleSyncResponse($seq,
                                            $packet,
                                            \&_ewokDataControlResponsePacket);
}    
    
##############################################################################
# Name:  _pDataControlResponsePacket
#
# Desc: Handles a response packet from a persistent Data Control request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               OPTION
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _pDataControlResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %pData;
    
    logMsg("_pDataControlResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $pData{STATUS} = $parts{STATUS};
        $pData{ERROR_CODE} = $parts{ERROR_CODE};
        
        if ($pData{STATUS} == 0)
        {
            (
             $pData{OPTION},
             $pData{CHECK_SUM},
             $pData{RSVD1},
             $pData{LENGTH},
             $pData{RSVD2}
            ) = unpack("CCa2La8", $parts{DATA});
        
            if($pData{OPTION} == PERSISTENT_DATA_OPTION_READ)
            {
                $pData{RD_DATA} = substr $parts{DATA}, 16;
            }
        }
    }

    return %pData;
}

##############################################################################
# Name:  _ewokDataControlResponsePacket
#
# Desc: Handles a response packet from a persistent Data Control request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               OPTION
#               LENGTH
#               RD_DATA
#
##############################################################################
sub _ewokDataControlResponsePacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %eData;
    my $rsvd;
    
    logMsg("_ewokDataControlResponsePacket...begin\n");

    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $eData{STATUS} = $parts{STATUS};
        $eData{ERROR_CODE} = $parts{ERROR_CODE};
        
        if ($eData{STATUS} == 0)
        {
            (
             $eData{OPTION},
             $rsvd,
             $eData{REC_NAME},
             $eData{OFFSET},
             $eData{LENGTH}
            ) = unpack("Ca3a256LL", $parts{DATA});
        
            if($eData{OPTION} == 3)
            {
                $eData{RD_DATA} = substr $parts{DATA}, 268, $eData{LENGTH};
            }
            elsif($eData{OPTION} == 5)
            {
                (
                    $eData{REC_COUNT},
                    $eData{MAX_SPACE},
                    $eData{FREE_SPACE}
                ) = unpack("LLL", substr $parts{DATA}, 268, 12);

                for (my $index = 0; $index < $eData{REC_COUNT}; ++$index)
                {
                    my $mgmtstruct = substr $parts{DATA}, (280 + ($index * 268));

                    (
                        $eData{$index}{RC_NAME},
                        $eData{$index}{RC_LENGTH},
                        $eData{$index}{RC_LOCKED},
                        $eData{$index}{RC_TIMESTAMP},
                    ) = unpack("a256LLL", $mgmtstruct);
                }
                
            }

#            printf("\nEWOK Record total data %s length %u\n",$parts{DATA},$eData{LENGTH});
#            my $start = 5;
#            my $c;
#            for(my $i=0;$i<25;$i++)
#            {
#                $c = substr $parts{DATA}, $start,1;
#                $start++;
#                printf("%u",$c);
#            }
        }
    }

    return %eData;
}

##############################################################################
# Name:  _ewokDataControlResponsePacket
#
# Desc: Handles a response packet from a persistent Data Control request.
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: hash with the follwing (empty if failure):
#               STATUS
#               OPTION
#               LENGTH
#               RD_DATA
#
##############################################################################
sub displayewokDataControlResponsePacket
{
    my ($self, %eData) = @_;

    if (%eData)
    {
        print "Persistent Data for option $eData{OPTION}\n";

        
        if ($eData{STATUS} == 0)
        {
            if($eData{OPTION} == 5)
            {
                printf "List of %d records\n", $eData{REC_COUNT};
                for (my $index = 0; $index < $eData{REC_COUNT}; ++$index)
                {
                    printf "  Record %d\n", $index;
                    printf "  Record Name:      %s\n", $eData{$index}{RC_NAME};
                    printf "  Record Length:    %u\n", $eData{$index}{RC_LENGTH};
                    printf "  Record Locked:    %u\n", $eData{$index}{RC_LOCKED};
                    printf "  Record Timestamp: %u\n", $eData{$index}{RC_TIMESTAMP};
                    printf "  \n";
                }
            }

            else
            {
                printf "Record %s\n", $eData{REC_NAME};
                printf "  Offset %u, Length %u\n", $eData{OFFSET}, $eData{LENGTH};
                print " SUCCESS\n";
            }
        }
    }
}

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.5  2006/07/26 17:08:14  HoltyB
# TBolt00000000
# Fixed response handler for ewok pdata.
#
# Revision 1.4  2006/07/21 20:17:51  HoltyB
# TBolt00000000
# Fixed persistent data inmformation to show correctly.
#
# Revision 1.3  2006/07/21 14:53:03  HoltyB
# TBolt00000000
# Initial checkin of ewok persistent data ccbe library and ccbcl access.
#
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
# Revision 1.2  2002/10/30 15:30:17  HoltyB
# TBolt00006236: Added checksum to persistent data
#
# Revision 1.1  2002/10/21 12:01:46  HoltyB
# TBolt00006201:  Added persistent data functionality to CCB.
#
# Revision 1.24  2002/10/17 14:00:52  HoltyB
# TBolt00005229: Split customer and debug logs into different flash space.
#
# Revision 1.23  2002/07/10 01:28:54  HoltyB
# TBolt00005254: Changed Loginfo to display in the current computers local
# time zone.  Also added an option to still display the GMT time.
# TBolt00005248: Added new interface to write a debug message to the
# CCB logs.
#
# Revision 1.22  2002/06/17 19:51:02  SchibillaM
# TBolt00003454: Add the binary event data at the end of the ASCII log message.
# This is required to be able to decode the async event in the UMC.  Needed
# for VLink Change Name and other events.  Reviewed by Chris.
#
# Revision 1.21  2002/06/04 17:45:34  SchibillaM
# TBolt00004537: Final logging fixes and style clean-up (I hope).
# TBolt00004609: Add the event code to the ASCII log data.
# Reviewed by Tim.
#
# Revision 1.20  2002/05/07 16:56:33  SchibillaM
# TBolt00003506: Decode the event type field for binary log events.
#
# Revision 1.19  2002/05/07 16:43:05  SchibillaM
# TBolt00003506: Changes to binary log event structures and binary log handling.
# This is the first step to handle binary log data.  Requires corresponding changes
# in CCBE and Java code.
# Reviewed by Tim.
#
# Revision 1.18  2002/04/09 19:32:54  SwatoshT
# Tbolt00003509: Added support to retrieve and display binary log events.
#
# Revision 1.17  2002/02/15 19:13:28  SwatoshT
# TBolt00002744: Added controller SN and VCG ID to the log packet, to support
# aggregate logging. Reviewed by Chris N.
#
# Revision 1.16  2002/02/13 15:56:34  SwatoshT
# TBolt00002744: Changed Async client to send Log packets, rather than
# text strings.
#
# Revision 1.15  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.14  2002/01/29 21:47:38  HoltyB
# changed status and errorcode returned for empty hash in loginfo
#
# Revision 1.13  2001/12/19 20:16:22  SwatoshT
# Added support for sequence number when requesting logs.
# Reveiwed by Randy R.
#
# Revision 1.12  2001/11/27 22:16:56  RysavyR
# Added [-f filename] to LOGINFO to write logs to a file
#
# Revision 1.11  2001/11/15 17:04:21  NigburC
# Made sure logInfo and envStats added the STATUS and ERROR_CODE
# fields to the hash returned.
#
# Revision 1.10  2001/11/14 19:28:20  SwatoshT
# Add support for log info packet in reduced format.
#
# Revision 1.9  2001/11/14 13:03:04  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.8  2001/11/13 23:04:59  SwatoshT
# Reversed order of display and added newline.
#
# Revision 1.7  2001/11/13 18:49:17  SwatoshT
# Added support for displaying log info on port 3100.
#
# Revision 1.6  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
#
# Revision 1.5  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.4  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.3  2001/11/01 14:01:43  NigburC
# Added the displayLogInfo routine.
# Added numerical sorting to the keys in the logInfo hash so they are sorted
# correctly.
#
# Revision 1.2  2001/10/31 17:00:04  NigburC
# Added the PACKET_LOG_CLEAR packet and handlers.
#
# Revision 1.1  2001/10/31 15:40:04  NigburC
# Initial integration of logging command manager code.
#
# Revision 1.1.1.1  2001/10/31 12:51:30  NigburC
# Initial integration of Bigfoot command line.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
