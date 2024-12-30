# $Id: cmPersistentData.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
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
# Name:     persistentDataControl
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
# Name:     ewokDataControl
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

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
