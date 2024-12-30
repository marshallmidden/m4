# $Id: cmEngDebug.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
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
# Name:     engDebug
#
# Desc:     Engineering debug pass-through command.
#
# Input:    [ARG1]
#           [ARG2]
#           .
#           .
#           .
#           [ARGN]
#
# Returns:  NOTHING
##############################################################################
sub engDebug
{
    my ($self, $in_args) = @_;

    logMsg("begin\n");

    my @args;

    if (defined($in_args))
    {
        @args = @$in_args;
    }

    if (scalar(@args) > 0)
    {
        for (my $i = 0; $i < scalar(@args); ++$i)
        {
            my $arg = "$args[$i]";
            print "arg[$i]: $arg\n";
        }
    }
    else
    {
        print "no arguments\n";
    }
}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
