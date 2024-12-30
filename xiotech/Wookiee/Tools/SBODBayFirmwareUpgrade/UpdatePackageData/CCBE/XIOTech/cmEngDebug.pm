#ifdef ENGINEERING
# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
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
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.3  2002/09/03 14:32:52  SchibillaM
# TBolt00005858: Changes to CCBCL files to allow a subset of function to be built
# for field use.  The tool BuildCCBCLSubset.pl builds the subset.  These changes
# also remove ENVSTATS which is replaced by STATSENV.
# Reviewed by Randy and Craig.
#
# Revision 1.2  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.1  2001/12/07 21:47:12  NigburC
# Added the ENGDEBUG command.
#
# Revision 1.1  2001/12/07 17:11:04  NigburC
# Added VLINK commands.
# Added DEVSTATUS command.
# Added RAIDCOUNT and RAIDLIST commands.
# Reverted the byte swapping done on capacity and count 64 bit integer values
# since they really did not need to be swapped.  Only WWNs should be
# swapped.
# Fixed other bugs found during debugging.
#
##############################################################################
#endif
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
