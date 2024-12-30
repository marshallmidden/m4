# $Id: cmX1Account.pm 146064 2010-08-20 21:15:33Z m4 $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2002  Xiotech
# ======================================================================
#
# Purpose:
#   Wrapper for all the different XIOTech X1 Account commands
#   that can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::error;

use XIOTech::logMgr;

use strict;


##############################################################################
# Name:     _parseX1GetEnviro
#
# Desc:     Handle an X1 Get Environmental Info return packet.
#           This is a common function to handle the data from both X1 and PI
#           packets.
#
# Input:    self
#           rx packet
#
# Returns:  a hash with the following fields
#
#
##############################################################################
sub _parseX1GetEnviro
{
# X1_ENVIRO_INFO
use constant X1_ENVIRO_INFO_1 =>
           "C                   # Controller temperature - Host
            C                   # Controller temperature - Store
            C                   # Controller AC 1
            C                   # Controller AC 2
            C                   # Controller DC 1
            C                   # Controller DC 2
            C                   # Controller Fan 1
            C                   # Controller Fan 2
            C                   # Controller Buffer Host
            C                   # Controller Buffer Store
            a2";                # Fibre Bay Map

use constant X1_ENVIRO_INFO_2 =>
           "L                   # Server MB/sec
            L                   # Server IO/sec
            S                   # RESERVED
            L                   # Back-end proc heartbeat
            L                   # Front-end proc heartbeat
            a2";                # SATA Bay Map
            
    my (%parts) = @_;

    my %info;
    my $i;
    my $reserved; 
    my $start = 0; 
    
    # Load first part of X1_ENVIRO_INFO into the hash
    ( $info{CTRLTEMPHOST},
      $info{CTRLTEMPSTORE},
      $info{CTRLAC1},
      $info{CTRLAC2},
      $info{CTRLDC1},
      $info{CTRLDC2},
      $info{CTRLFAN1},
      $info{CTRLFAN2},
      $info{CTRLBUFHOST},
      $info{CTRLBUFSTORE},
      $info{FIBREBAYMAP}) = unpack X1_ENVIRO_INFO_1, $parts{DATA};

    # Move start past the data just unpacked
    $start = 12; 

    my @fibreBayTempIn1;

    # Get the array of Fibre Bay Input 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempIn1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempIn1 array into the info hash
    $info{FIBREBAYTEMPIN1} = [@fibreBayTempIn1];

    my @fibreBayTempIn2;
    # Get the array of Fibre Bay Input 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempIn2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempIn2 array into the info hash
    $info{FIBREBAYTEMPIN2} = [@fibreBayTempIn2];

    my @fibreBayTempOut1;
    # Get the array of Fibre Bay Output 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempOut1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempOut1 array into the info hash
    $info{FIBREBAYTEMPOUT1} = [@fibreBayTempOut1];

    my @fibreBayTempOut2;
    # Get the array of Fibre Bay Output 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayTempOut2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the bayTempOut2 array into the info hash
    $info{FIBREBAYTEMPOUT2} = [@fibreBayTempOut2];

    my @fibreBayPSFan;
    # Get the array of Fibre Bay Power Supply Fans
    for ($i = 0; $i < 16; $i++)
    {
        (
        $fibreBayPSFan[$i]
        ) = unpack("x$start C", $parts{DATA});
        $start = $start + 1;
    }

    # Put the bayPSFan array into the info hash
    $info{FIBREBAYPSFAN} = [@fibreBayPSFan];


    # Load last part of X1_ENVIRO_INFO into the hash
    ( $info{SERVERMB},
      $info{SERVERIO},
      $reserved,
      $info{BEPROCHB},
      $info{FEPROCHB},
      $info{SATABAYMAP}) = unpack("x$start LL S LL a2", $parts{DATA});


    #
    # Additions for SATA bays
    #
    my @sataBayTempOut1;
    $start = $start + 20;   # Accounts for the unpack above 
    
    # Get the array of SATA Bay Output 1 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayTempOut1[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayTempOut1 array into the info hash
    $info{SATABAYTEMPOUT1} = [@sataBayTempOut1];

    my @sataBayTempOut2;
    # Get the array of SATA Bay Output 2 temperatures
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayTempOut2[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayTempOut2 array into the info hash
    $info{SATABAYTEMPOUT2} = [@sataBayTempOut2];

    my @sataBayPS;
    # Get the array of SATA Bay Power Supplies 
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayPS[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayPS array into the info hash
    $info{SATABAYPS} = [@sataBayPS];

    my @sataBayFan;
    # Get the array of SATA Bay Fans 
    for ($i = 0; $i < 16; $i++)
    {
        (
        $sataBayFan[$i]
        ) = unpack("x$start C", $parts{DATA});

        $start = $start + 1;
    }

    # Put the sataBayfan array into the info hash
    $info{SATABAYFAN} = [@sataBayFan];

    return %info;
}


##############################################################################
# Name:     DisplayX1EnvironmentalInfo
#
# Desc:     Display the X1 Controller and Disk Bay Environmental Info
#
# Input:    hash containing X1 account data
#
##############################################################################
sub DisplayX1EnvironmentalInfo
{
    my (%info) = @_;

    my $msg;

    FmtX1EnvironmentalInfo(undef, \$msg, %info);

    printf "%s", $msg;

}

##############################################################################
# Name:     FmtX1EnvironmentalInfo
#
# Desc:     Format X1 Controller and Disk Bay Environmental Info
#
# Input:    hash containing X1 account data
#
##############################################################################
sub FmtX1EnvironmentalInfo
{
    my ($self, $msgPtr, %info ) = @_;

    my @fibreBays = ParseBitmap($info{FIBREBAYMAP});
    my @sataBays =  ParseBitmap($info{SATABAYMAP});

    my $msg;

    $msg = sprintf("X1 Environmental Info:\n");
    $msg .= sprintf("  Controller temperature - Host:      %d\n", $info{CTRLTEMPHOST});
    $msg .= sprintf("  Controller temperature - Store:     %d\n", $info{CTRLTEMPSTORE});
    $msg .= sprintf("  Controller AC 1:                  0x%02X\n", $info{CTRLAC1});
    $msg .= sprintf("  Controller AC 2:                  0x%02X\n", $info{CTRLAC2});
    $msg .= sprintf("  Controller DC 1:                  0x%02X\n", $info{CTRLDC1});
    $msg .= sprintf("  Controller DC 2:                  0x%02X\n", $info{CTRLDC2});
    $msg .= sprintf("  Controller Fan 1:                 0x%02X\n", $info{CTRLFAN1});
    $msg .= sprintf("  Controller Fan 2:                 0x%02X\n", $info{CTRLFAN2});
    $msg .= sprintf("  Controller buffer - Host:         0x%02X\n", $info{CTRLBUFHOST});
    $msg .= sprintf("  Controller buffer - Store:        0x%02X\n", $info{CTRLBUFSTORE});
    $msg .= sprintf("  Fibre Bay Map:                    @fibreBays\n");


    # Array of Fibre Bay Input 1 temperatures
    
    $msg .= sprintf("\n  Bay number:                                  ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $i);
    }

    $msg .= sprintf("\n                                               ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("---  ");
    }

    $msg .= sprintf("\n  Fibre Bay Input 1 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPIN1}[$i]);
    }

    # Print the array of Fibre Bay Input 2 temperatures
    $msg .= sprintf("\n  Fibre Bay Input 2 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPIN2}[$i]);
    }

    # Print the array of Fibre Bay Output 1 temperatures
    $msg .= sprintf("\n  Fibre Bay Output 1 temps (bay0 - bay15):     ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPOUT1}[$i]);
    }

    # Print the array of Fibre Bay Output 2 temperatures
    $msg .= sprintf("\n  Fibre Bay Output 2 temps (bay0 - bay15):     ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{FIBREBAYTEMPOUT2}[$i]);
    }

    # Print the array of Fibre Bay Power Supply Fan values 
    $msg .= sprintf( "\n  Fibre Bay Power Supply Fans (bay0 - bay15): ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{FIBREBAYPSFAN}[$i]);
    }

    $msg .= sprintf("\n  Server MB/Sec:                    %0.2f\n", $info{SERVERMB}/100.0);
    $msg .= sprintf("  Server IO/Sec:                    %d\n", $info{SERVERIO});
    $msg .= sprintf("  Back-end processor heartbeat:     %d\n", $info{BEPROCHB});
    $msg .= sprintf("  Front-end processor heartbeat:    %d\n\n", $info{FEPROCHB});


    # Changes below for SATA bays
    $msg .= sprintf("  SATA Bay Map:                     @sataBays");

    # Print the array of SATA Bay Output 1 temperatures
    $msg .= sprintf("\n  SATA Bay Output 1 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{SATABAYTEMPOUT1}[$i]);
    }

    # Print the array of SATA Bay Output 2 temperatures
    $msg .= sprintf("\n  SATA Bay Output 2 temps (bay0 - bay15):      ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("%3d  ", $info{SATABAYTEMPOUT2}[$i]);
    }

    # Print the array of Fibre Bay Power Supply values 
    $msg .= sprintf( "\n  SATA Bay Power Supply (bay0 - bay15):       ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{SATABAYPS}[$i]);
    }

    # Print the array of Fibre Bay Power Supply Fan values 
    $msg .= sprintf( "\n  SATA Bay  Fans (bay0 - bay15):              ");
    for (my $i = 0; $i < 16; $i++)
    {
        $msg .= sprintf("0x%02X ", $info{SATABAYFAN}[$i]);
    }

    $$msgPtr .= $msg;

}

##############################################################################
# Name: _fixString
#
# Desc: Remove certain annoying characters and replace with .
#
# In:   string
#
# Out:  String
##############################################################################
sub _fixString
{
    my ($str) = @_;

    ###
    ## The negated list is ' ' through '~' (which is the class of printables)
    ##   is to be replaced with a question mark symbol.
    ###
    $str =~ s/[^ -~]/\./g;

    return $str;
}


1;

##############################################################################
# Modelines:
# vi: sw=4 ts=4 expandtab
#
