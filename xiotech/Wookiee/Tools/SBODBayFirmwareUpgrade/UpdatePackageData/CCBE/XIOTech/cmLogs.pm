# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris NIgbur
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
# Name:     logClear
#
# Desc:     Clear the log entries on the current connection.
#
# In:       None.
#
# Returns:
#
##############################################################################
sub logClear
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["logClear"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket(PI_LOG_CLEAR_CMD,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericResponsePacket);
}

##############################################################################
# Name:     logTextMessage
#
# Desc:     Sends a text log command
#
# Input:    Which command
##############################################################################
sub logTextMessage
{
    my ($self, $msg, $type) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['i'],
                ["logTextMessage"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $msgType = LOG_TYPE_DEBUG;

    if (defined($type))
    {
        $msgType = $type;
    }


    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LZ128",
                    $msgType,
                    $msg);

    my $packet = assembleXiotechPacket(PI_LOG_TEXT_MESSAGE_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericResponsePacket);
}

##############################################################################
# Name: logInfo
#
# Desc:     Get the last N log messages
#
# In:       count - Number of log messages to retrieve.
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       SEQUENCE_NUMBER         Log event sequence number
#       TIME_AND_DATE           Time and date string
#       MESSAGE_TYPE            Message type string
#       MESSAGE_DESC            Message description string
#
##############################################################################
sub logInfo
{
    my ($self, $ttlCount, $mode, $sequence) = @_;

    logMsg("begin\n");

    my %info;
    my %logData;
    my $getAll = 0;
    my $firstLoop = 1;

    if (defined($ttlCount) && (uc($ttlCount) eq "ALL"))
    {
        $_[1] = 0xFFFF;
        $getAll = 1;
    }

    my $totalCount =  $_[1];

    #
    # Attemp to recieve all events in one request
    #
    my $count = $totalCount;
    $info{EVENT_COUNT} = 0;


    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["logInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    #
    # Loop through until all requested log events have been recieved
    #
    do
    {
#        printf "ISSUED:   ";
#        printf "Event Count = %5lu", $count;
#        printf " Sequence = %5lu \n", $sequence;

        #
        # Build the request
        #
        my $seq = $self->{SEQ}->nextId();
        my $ts = $self->{SEQ}->nextTimeStamp();
        my $data = pack("SSL", $count, $mode, $sequence);

        my $packet = assembleXiotechPacket(PI_LOG_INFO_CMD,
                                            $seq,
                                            $ts,
                                            $data,
                                            $self->{PORT}, VERSION_1);

        #
        # Recieve the Results
        #
        %logData = $self->_handleSyncResponse($seq, $packet, \&_logInfoPacket);

        if (%logData)
        {
            #
            # Save away the status and error code
            #
            $info{STATUS} = $logData{STATUS};
            $info{ERROR_CODE} = $logData{ERROR_CODE};

#            printf "RECEIVED:   ";
#            printf "Event Count = %5lu", $logData{EVENT_COUNT};
#            printf " Sequence = %5lu \n", $logData{SEQUENCE_NUMBER};
        }
        else
        {
            $info{STATUS} = PI_TIMEOUT;
            $info{STATUS_MSG} = $self->getStatusMsg($info{STATUS});
            $info{ERROR_CODE} = 0;

        }
        #
        # If we received valid data, accumulate the result. Keep the results
        # in the order they were recieved.
        #
        if (%logData)
        {
            #
            # Copy each new event
            #
            my $j;

            for ( my $i=0; ($i < $logData{EVENT_COUNT}); $i++)
            {
                if ($getAll)
                {
                    $j =  $info{EVENT_COUNT} + $i;
                }

                else
                {
                    $j =  $totalCount - $count + $i;
                }

                #
                # If binary mode, save binary data, else save strings
                #
                if ($mode & 0x01)
                {
                    # Binary Mode
                    $info{$j}->{EVENT_LEN} =  $logData{$i}->{EVENT_LEN};
                    $info{$j}->{EVENT_TYPE} =  $logData{$i}->{EVENT_TYPE};
                    $info{$j}->{MESSAGE} =    $logData{$i}->{MESSAGE};
                }
                else
                {
                    $info{$j}->{MESSAGE_LEN_TOTAL} = $logData{$i}->{MESSAGE_LEN_TOTAL};

                    # ASCII Mode
                    $info{$j}->{MESSAGE_LEN_ASCII} = $logData{$i}->{MESSAGE_LEN_ASCII};
                    $info{$j}->{EVENT_CODE}      =  $logData{$i}->{EVENT_CODE};
                    $info{$j}->{MASTER_SEQUENCE_NUMBER} =  $logData{$i}->{MASTER_SEQUENCE_NUMBER};
                    $info{$j}->{SEQUENCE_NUMBER} =  $logData{$i}->{SEQUENCE_NUMBER};
                    $info{$j}->{STATUS_WORD}     =  $logData{$i}->{STATUS_WORD};
                    $info{$j}->{TIME_AND_DATE}   =  $logData{$i}->{TIME_AND_DATE};
                    $info{$j}->{MESSAGE_TYPE}    =  $logData{$i}->{MESSAGE_TYPE};
                    $info{$j}->{MESSAGE_DESC}    =  $logData{$i}->{MESSAGE_DESC};

                    # Additional binary data
                    $info{$j}->{MESSAGE_LEN_BINARY} = $logData{$i}->{MESSAGE_LEN_BINARY};
                    $info{$j}->{EVENT_TYPE_BINARY}  = $logData{$i}->{EVENT_TYPE_BINARY};
                    $info{$j}->{MESSAGE_BINARY}     = $logData{$i}->{MESSAGE_BINARY};
                }

            }

            #
            # Update the count and starting sequence number
            #

            $count -= $logData{EVENT_COUNT};
            $sequence = $logData{SEQUENCE_NUMBER} - 1;

            #
            # If we didn't get all the entries, use the sequence mode
            #
            if ($count)
            {
                $mode |= 0x04;
            }

            #
            # Copy the "header" information
            #
            if ($getAll)
            {
                if ($firstLoop)
                {
                    $info{EVENT_COUNT} = $logData{EVENT_COUNT};
                }
                else
                {
                    $info{EVENT_COUNT} += $logData{EVENT_COUNT};
                }

#                printf "Total Event Count = %7lu\n", $info{EVENT_COUNT};
            }
            else
            {
                $info{EVENT_COUNT} = $totalCount - $count;
            }

            $info{MODE} = $logData{MODE};
            $info{SEQUENCE_NUMBER}  = $sequence;

            if ($getAll)
            {
                $count = 0xFFFF;
                $firstLoop = 0;
            }

        }


    } while (($count > 0) && (%logData) && ($logData{EVENT_COUNT}) &&
             (($logData{MODE} & 0x8000) == 0));



    return %info;
}

##############################################################################
# Name:     logAcknowledge
#
# Desc:     Acknowledges a log
#
# Input:    log sequence number
##############################################################################
sub logAcknowledge
{
    my ($self, $seqNum) = @_;

    logMsg("begin\n");

    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("LL",
                    1,$seqNum);

    my $packet = assembleXiotechPacket(PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq, $packet, \&_genericResponsePacket);
}

##############################################################################
# Name:     displayLogInfo
#
# Desc:     Displays the log information.
#
# In:       None.
#
# Returns:  None.
#
##############################################################################
sub displayLogInfo
{
    my ($self, $filen, $mode, $gmt, $showAckStatus, %info) = @_;

    return displayLogInfo2($self, $filen, $mode, $gmt, $showAckStatus, \%info);
}

sub displayLogInfo2
{
    my ($self, $filen, $mode, $gmt, $showAckStatus, $pInfo, $regX) = @_;

    my $key = 0;
    my %info = %$pInfo;

    # open the output file if one was requested
    my $fh = *STDOUT;
    if (defined $filen) {
        $fh = *FH;
        open $fh, ">$filen" or $fh = *STDOUT;
    }

    my $count = $info{EVENT_COUNT};

    #
    # Print the logs in reverse order
    #
    if (!($mode & 0x01))   # ASCII mode
    {
        if($gmt)
        {
            if($showAckStatus)
            {
                print $fh "  MSeq     SeqN      Time (GMT) Date     LogID   N/Ack  Type    Description\n";
            }
            else
            {
                print $fh "  MSeq     SeqN      Time (GMT) Date     LogID    Type    Description\n";
            }
        }
        else
        {
            if($showAckStatus)
            {
                print $fh "  MSeq     SeqN    Time (Local) Date     LogID   N/Ack  Type    Description\n";
            }
            else
            {
                print $fh "  MSeq     SeqN    Time (Local) Date     LogID    Type    Description\n";
            }
        }
    }

    my $address = 0;
    my $format = "word";

    if ($count)
    {
        for ($key = $count-1; $key >= 0; $key-- )
        {
            if ($mode & 0x01)   # Binary mode
            {
                printf $fh "Event Type = %2lu ", $info{$key}->{EVENT_TYPE};

                # Decode the event type to English
                #define LOG_TYPE_INFO        0
                #define LOG_TYPE_WARNING     1
                #define LOG_TYPE_ERROR       2
                #define LOG_TYPE_DEBUG       3
                #define LOG_TYPE_FATAL       4
                if ($info{$key}->{EVENT_TYPE} == 0)
                {
                    printf $fh "(Info) \n";
                }
                elsif ($info{$key}->{EVENT_TYPE} == 1)
                {
                    printf $fh "(Warning) \n";
                }
                elsif ($info{$key}->{EVENT_TYPE} == 2)
                {
                    printf $fh "(Error) \n";
                }
                elsif ($info{$key}->{EVENT_TYPE} == 3)
                {
                    printf $fh "(Debug) \n";
                }
                elsif ($info{$key}->{EVENT_TYPE} == 4)
                {
                    printf $fh "(Fatal) \n";
                }
                else
                {
                    printf $fh "(Unknown) \n";
                }

                printf $fh "Event Length (includes Event Type field) = %2lu \n",
                           $info{$key}->{EVENT_LEN};
                my $str = FormatDataString($self, $info{$key}->{MESSAGE},
                                           $address, $format, $fh,
                                           $info{$key}->{MESSAGE_LEN_TOTAL});
                print $fh $str."\n";
            }
            else        # ASCII mode
            {
                my $msg = "";

                # MSeq
                $msg .= sprintf "%7lu", $info{$key}->{MASTER_SEQUENCE_NUMBER};
                $msg .= sprintf "  ";

                # SeqN
                $msg .= sprintf "%7lu", $info{$key}->{SEQUENCE_NUMBER};
                $msg .= sprintf "  ";

                # GMT time conversion to local time
                if (!$gmt)
                {
                    my $tmp = _localTime($info{$key}->{TIME_AND_DATE});
                    $tmp =~ s/ AM/am/;
                    $tmp =~ s/ PM/pm/;
                    $info{$key}->{TIME_AND_DATE} = $tmp;
                }

                # Time and Date
                $msg .= sprintf $info{$key}->{TIME_AND_DATE};
                $msg .= sprintf "  ";

                # LogID
                $msg .= sprintf "0x%04X", $info{$key}->{EVENT_CODE};
                $msg .= sprintf "  ";

                # Flags
                if ($showAckStatus)
                {
                    $msg .= sprintf "%s", _logInfoStatusFlags($info{$key}->{STATUS_WORD});
                    $msg .= sprintf "  ";
                }

                # Type
                $msg .= sprintf $info{$key}->{MESSAGE_TYPE};
                $msg .= sprintf "  ";

                # Description
                $msg .= sprintf $info{$key}->{MESSAGE_DESC};
                my $c = substr($info{$key}->{MESSAGE_DESC}, -1, 1);
                if ($c eq "\r")
                {
                    chop($info{$key}->{MESSAGE_DESC});
                    $c = substr($info{$key}->{MESSAGE_DESC}, -1, 1);
                }
                if ($c ne "\n")
                {
                    $msg .= sprintf "\n";
                }

                # Binary data following the ASCII data
                # Leave this code in for debug purposes.
                #$msg .= sprintf "Binary Event Length (does NOT include Event Type field): %2lu \n",
                #           $info{$key}->{MESSAGE_LEN_BINARY} - 2;
                #
                #my $str = FormatDataString($self, $info{$key}->{MESSAGE_BINARY},
                #                           $address, $format, $fh,
                #                           $info{$key}->{MESSAGE_LEN_BINARY} - 2);
                #$msg .= sprintf $str."\n";

                # If reg exp passed in, use it now.
                if ( !defined($regX) or $msg =~ /$regX/i )
                {
                    print $fh $msg;
                }
            }
        }
    }

    if($fh ne *STDOUT) {
        close $fh;
    }
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################


##############################################################################
# Name: _logInfoStatusFlags
#
# Desc: Parses the statusWord and places the information in a string
#
# In:   scalar  $statusWord
#
# Returns:
#       Empty string if we have errors else a string with the following:
#       S: Started
#       C: Complete
#       A: Acknowledged
#       D: Deleted
#
##############################################################################
sub _logInfoStatusFlags
{
    my ($statusFlags) = @_;
    my $string = "";

    # LE_STARTED
    #if( !($statusFlags & 0x0001) )
    #{
    #    $string .= "S";
    #}
    #else
    #{
    #    $string .= " ";
    #}

    # LE_COMPLETE
    #if( !($statusFlags & 0x0002) )
    #{
    #    $string .= "C";
    #}
    #else
    #{
    #    $string .= " ";
    #}

    # LE_ACKED
    if( !($statusFlags & 0x4000) )
    {
        $string .= "Ack ";
    }
    else
    {
        $string .= "Nack";
    }

    # LE_DELETED
    #if( !($statusFlags & 0x8000) )
    #{
    #    $string .= "D";
    #}
    #else
    #{
    #    $string .= " ";
    #}
}

##############################################################################
# Name: _logInfoPacket
#
# Desc: Parses the log info packet and places the information
#       in a hash
#
# In:   scalar  $packet to be parsed (If no packet then will attempt to read
#               one from the SAN box
#
# Returns:
#
#       Empty hash if we have errors else a hash with the following:
#
#       SEQUENCE_NUMBER         Log event sequence number
#       TIME_AND_DATE           Time and date string
#       MESSAGE_TYPE            Message type string
#       MESSAGE_DESC            Message description string
#
##############################################################################
sub _logInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("_logInfoPacket...begin\n");

    my %info;

    if (!(defined($recvPacket)))
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

    if (defined($recvPacket))
    {
        if (commandCode($recvPacket) == PI_LOG_INFO_CMD)
        {
            my %parts = disassembleXiotechPacket($recvPacket);

            %info = logInfoData(%parts);

            $info{STATUS} = $parts{STATUS};
            $info{ERROR_CODE} = $parts{ERROR_CODE};

        }
        else
        {
            $self->_handleError($recvPacket);
            logMsg("Unexpected packet: We expected a log info packet\n");
        }
    }

    return %info;
}

##############################################################################
# Name: changedEventData
#
# Desc: Parses the PI changed event packet returns the event type
#
# In:   hash   $packet data to be parsed
#
# Returns:
#       Event type (quad).
#
##############################################################################
sub changedEventData
{
    my (%parts) = @_;

    my $event1;
    my $event2;

    # Unpack the log header
    ($event1, $event2) = unpack('LL', $parts{DATA});
    return(($event2 << 32) | $event1);
}

##############################################################################
# Name: changedEventString
#
# Desc: Converts event to printable string.
#
# In:   event (quad).
#
# Returns:
#       String representing event.
#
##############################################################################
sub changedEventString
{
    my ($event) = @_;

    my $string;

    # Convert to event string.
    if ($event & X1_ASYNC_PCHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_PCHANGED';
    }
    if ($event & X1_ASYNC_RCHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_RCHANGED';
    }
    if ($event & X1_ASYNC_VCHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCHANGED';
    }
    if ($event & X1_ASYNC_HCHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_HCHANGED';
    }

    if ($event & X1_ASYNC_ACHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_ACHANGED';
    }
    if ($event & X1_ASYNC_ZCHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_ZCHANGED';
    }
    if ($event & ASYNC_ENV_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_ENV_CHANGE';
    }
    if ($event & ASYNC_DEFRAG_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_DEFRAG_CHANGE';
    }

    if ($event & ASYNC_PDATA_CREATE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_PDATA_CREATE';
    }
    if ($event & ASYNC_PDATA_REMOVE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_PDATA_REMOVE';
    }
    if ($event & ASYNC_PDATA_MODIFY) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_PDATA_MODIFY';
    }
    if ($event & ASYNC_ISNS_MODIFY) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_ISNS_MODIFY';
    }

    if ($event & ASYNC_BUFFER_BOARD_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_BUFFER_BOARD_CHANGE';
    }
    if ($event & ASYNC_GLOBAL_CACHE_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_GLOBAL_CACHE_CHANGE';
    }
    if ($event & ASYNC_PRES_CHANGED) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_PRES_CHANGED';
    }
    if ($event & ASYNC_APOOL_CHANGED) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_APOOL_CHANGED';
    }

    if ($event & X1_ASYNC_VCG_ELECTION_STATE_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_ELECTION_STATE_CHANGE';
    }
    if ($event & X1_ASYNC_VCG_ELECTION_STATE_ENDED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_ELECTION_STATE_ENDED';
    }
    if ($event & X1_ASYNC_VCG_POWERUP) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_POWERUP';
    }
    if ($event & X1_ASYNC_VCG_CFG_CHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_CFG_CHANGED';
    }

    if ($event & X1_ASYNC_VCG_WORKSET_CHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_WORKSET_CHANGED';
    }
    if ($event & X1_ASYNC_VCG_GEOPOOL_CHANGED) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_VCG_GEOPOOL_CHANGED';
    }
    if ($event & 0x00400000) {
      $string .= ((defined($string))?' ':'') . 'UNUSED(0x00400000)';
    }
    if ($event & 0x00800000) {
      $string .= ((defined($string))?' ':'') . 'UNUSED(0x00800000)';
    }

    if ($event & X1_ASYNC_BE_PORT_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_BE_PORT_CHANGE';
    }
    if ($event & X1_ASYNC_FE_PORT_CHANGE) {
      $string .= ((defined($string))?' ':'') . 'X1_ASYNC_FE_PORT_CHANGE';
    }
    if ($event & 0x04000000) {
      $string .= ((defined($string))?' ':'') . 'UNUSED(0x04000000)';
    }
    if ($event & 0x08000000) {
      $string .= ((defined($string))?' ':'') . 'UNUSED(0x08000000)';
    }

    if ($event & ASYNC_PING_EVENT) {
      $string .= ((defined($string))?' ':'') . 'ASYNC_PING_EVENT';
    }
    if ($event & LOG_STD_MSG) {
      $string .= ((defined($string))?' ':'') . 'LOG_STD_MSG';
    }
    if ($event & LOG_XTD_MSG) {
      $string .= ((defined($string))?' ':'') . 'LOG_XTD_MSG';
    }
    if ($event & LOG_BIN_MSG) {
      $string .= ((defined($string))?' ':'') . 'LOG_BIN_MSG';
    }

# Note: 2007-07-09 ... no upper bits are currently used.
    if ($event & ~0xFFFFFFFF) {
      $string .= ((defined($string))?' ':'') . sprintf("UNUSED(0x%qx)", $event & ~0xFFFFFFFF);
    }

    return($string);
}

##############################################################################
# Name: logInfoData
#
# Desc: Parses the log info packet data and places the information
#       in a hash
#
# In:   hash   $packet data to be parsed
#
# Returns:
#
#       Empty hash if we have errors else a hash with the following:
#
#       SEQUENCE_NUMBER         Log event sequence number
#       TIME_AND_DATE           Time and date string
#       MESSAGE_TYPE            Message type string
#       MESSAGE_DESC            Message description string
#       STATUS_WORD             Log statusWord flags (ack, del, etc)
#
##############################################################################
sub logInfoData
{
    my (%parts) = @_;

    my %info;
    my $count;

    # Unpack the log header
    (
    $info{EVENT_COUNT},
    $info{MODE},
    $info{SEQUENCE_NUMBER},
    $info{CONTROLLER_SN},
    $info{VCG_ID},
    $info{RSVD_1},
    $info{RSVD_2},
    $info{RSVD_3},
    $info{RSVD_4}
    ) = unpack("SSLLLLLLL", $parts{DATA});

#    print "Event count = $info{EVENT_COUNT} \n";
#    print "Mode = $info{MODE} \n";
#    print "Seq Num = $info{SEQUENCE_NUMBER} \n";
#    print "Controller SN = $info{CONTROLLER_SN} \n";
#    print "VCG ID = $info{VCG_ID} \n";
#    print "RSVD 1 = $info{RSVD_1} \n";
#    print "RSVD 2 = $info{RSVD_2} \n";
#    print "RSVD 3 = $info{RSVD_3} \n";
#    print "RSVD 4 = $info{RSVD_4} \n\n";

    $count = $info{EVENT_COUNT};
    my $start = 32;     # adjust start for the log header above

    # Loop through each log event
    for (my $i = 0; $i < $count; $i++)
    {
        my $data = substr $parts{DATA}, $start;

        if ($info{MODE} & 0x01)     # Binary Mode
        {
            # Unpack the binary  data
            (
                $info{$i}{EVENT_LEN},
                $info{$i}{EVENT_TYPE}
            ) = unpack("SS", $data);

            #print "MESSAGE_LEN = $info{$i}{MESSAGE_LEN} \n";
            #print "EVENT_TYPE = $info{$i}{EVENT_TYPE} \n";

            # Adjust start by the fixed part of log event
            $start += 4;

            # Adjust the length of the message data to account for the
            # EVENT_TYPE field
            my $len =  $info{$i}{EVENT_LEN} - 2;
            $data = substr $parts{DATA}, $start, $len;

            # Get the variable length of the message
            ($info{$i}{MESSAGE}) = $data;

            # Adjust start by the variable length message
            $start += $len;
        }
        else        # ASCII Mode
        {
            # Unpack the string data
            (
            $info{$i}{MESSAGE_LEN_TOTAL},
            $info{$i}{MESSAGE_LEN_ASCII},
            $info{$i}{EVENT_CODE},
            $info{$i}{MASTER_SEQUENCE_NUMBER},
            $info{$i}{SEQUENCE_NUMBER},
            $info{$i}{STATUS_WORD},
            $info{$i}{TIME_AND_DATE},
            $info{$i}{MESSAGE_TYPE}
            ) = unpack("LSSLLLa25a10", $data);

#           print "MESSAGE_LEN_TOTAL      = $info{$i}{MESSAGE_LEN_TOTAL} \n";
#           print "MESSAGE_LEN_ASCII      = $info{$i}{MESSAGE_LEN_ASCII} \n";
#           print "MASTER_SEQUENCE_NUMBER = $info{$i}{MASTER_SEQUENCE_NUMBER} \n";
#           print "SEQUENCE_NUMBER        = $info{$i}{SEQUENCE_NUMBER} \n";
#           print "STATUS_WORD            = $info{$i}{STATUS_WORD} \n";
#           print "TIME_AND_DATE          = $info{$i}{TIME_AND_DATE} \n";
#           print "MESSAGE_TYPE           = $info{$i}{MESSAGE_TYPE} \n";
#           print "\n";

            # Adjust start by the fixed part of log event
            $start += 55;
            $data = substr $parts{DATA}, $start;

            my $len =  $info{$i}{MESSAGE_LEN_ASCII};
            # MESSAGE_LEN_ASCII includes the fixed part of the message plus
            # the variable length ASCII message string.  Subtract the fixed
            # portion to get the message string length.
            $len -= 49;

            # Get the variable length of the message
            ($info{$i}{MESSAGE_DESC}) = unpack("a$len", $data);

            # Adjust start by the variable length message
            $start += $len;

            # Fix the strings
            $info{$i}{TIME_AND_DATE} =~ s/\x00//g;
            $info{$i}{MESSAGE_TYPE} =~ s/\x00//g;
            $info{$i}{MESSAGE_DESC} =~ s/\x00//g;

            # Reset $data to point to the next data section - the
            # binary data.
            $data = substr $parts{DATA}, $start;

            # Get the binary portion of the log event
            (
            $info{$i}{MESSAGE_LEN_BINARY},  # length of binary data to follow
            $info{$i}{EVENT_TYPE_BINARY}
            ) = unpack("SS", $data);

            #print "MESSAGE_LEN_BINARY = $info{$i}{MESSAGE_LEN_BINARY} \n";
            #print "EVENT_TYPE_BINARY = $info{$i}{EVENT_TYPE_BINARY} \n";

            # Adjust start for the 2 fields above
            $start += 4;

            # MESSAGE_LEN_BINARY includes the length of the EVENT_TYPE field.
            # Subtract this off to get the actual length of the binary data.
            my $lenBinary =  $info{$i}{MESSAGE_LEN_BINARY} - 2;

            $data = substr $parts{DATA}, $start, $lenBinary;

            # Get the binary log event data
            ($info{$i}{MESSAGE_BINARY}) = $data;

            # Adjust start by the length of the log event
            $start += $lenBinary;
        }
    }

    return %info;

}

##############################################################################
# Name:     _localTime
#
# Desc:     Helper function for converting GMT to localtime.
#
# Input:    tm  -   Date and time returned in loginfo.
##############################################################################
sub _localTime
{
    my ($tm) = @_;

    my $newTm;
    my $tmDf = 0;
    my @splitTmDt = split / /, $tm;
    my @spltTm = split /:/, $splitTmDt[0];

    my ($lhour, $lmin, $lsec) = @spltTm;
    $lsec = substr($lsec,0,2);

    my ($lmon, $lmday, $lyear)= split /\//, $splitTmDt[1];

    my $gmTm = timegm($lsec,$lmin,$lhour,$lmday,($lmon-1),$lyear);

    if ((substr($spltTm[2],2,3) eq "am") && ($lhour == 12))
    {
        $gmTm -= 43200; #12 hours
    }
    elsif ((substr($spltTm[2],2,3) eq "pm") && ($lhour != 12))
    {
        $gmTm += 43200; #12 hours
    }

    $newTm = sprintf "%s", (strftime "%X %m/%d/%Y", localtime($gmTm));

    return $newTm;
}

##############################################################################
# Name:     _sortNumerically
#
# Desc:     Helper function for sorting values numerically.
#
# Input:    Two values to be compared.
##############################################################################
sub _sortNumerically
{
    $a <=> $b;
}

1;

##############################################################################
# Name:     registerAsyncEvents
#
# Desc:     register for async events.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub registerAsyncEvents
{
    my ($self,
        $opt,$registerEvents) = @_;

    logMsg("begin\n");

    my $cmd = PI_REGISTER_EVENTS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("LLCC",
                    $registerEvents,0,$opt, 0);


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT});

#    return $self->_handleSyncResponse($seq,
#                                        $packet,
#                                        \&_registerEventsPacket);
    $self->_receivePacketASync($seq,
                               $opt,
                               $packet,
                               \&_registerEventsPacket);
}

##############################################################################
# Name:     _registerEventsPacket
#
# Desc:     Parses the event registration response packet and places the
#           information in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _registerEventsPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $rsvd1;
    my $rsvd2;
    my $i;

    if (commandCode($recvPacket) == PI_REGISTER_EVENTS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my $start = 8;

            (
            $info{REGISTERED},
            $rsvd1)
            = unpack("SS", $parts{DATA});
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a registerEvents response packet\n");
    }

    return %info;
}

##############################################################################
#
# Change log:
# $Log$
# Revision 1.4  2006/07/21 14:53:03  HoltyB
# TBolt00000000
# Initial checkin of ewok persistent data ccbe library and ccbcl access.
#
# Revision 1.3  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.2.28.2  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.2.28.1  2006/04/17 06:07:40  BharadwajS
# Log Acknowledgement
#
# Revision 1.2  2005/05/19 21:52:35  RysavyR
# TBolt00000000: Added '-g reg-exp' option to DSPLOGS per Jeff Williams request. Refer to the DSPLOGS help text for details.
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.29  2004/04/28 18:17:38  HoltyB
# TBolt00000000:  Added Wookiee andling for the CCBE amd CCBCL.
#
# Revision 1.28  2003/06/02 16:17:00  RysavyR
# Misc fix to line up log messages on output.
#
# Revision 1.27  2003/02/11 19:58:37  McmasterM
# TBolt00007228: Display log event flags in the CCBE loginfo command
# Added [-A] option to loginfo and dsplogs commands to show the
# acknowledgement status of each log entry.  Default is to not display the data.
# Reviewed by Tim Swatosh
#
# Revision 1.26  2003/02/05 17:15:59  RysavyR
# Added DSPLOGS as an improved interface to LOGINFO.
#
# Revision 1.25  2002/11/11 20:56:55  HoltyB
# TBolt00006300:  Added functionality to log a generic text message and
# assign the severity as well.  Also changed the logTextMessage packet
# interface to allow the severity to be passed through.
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
