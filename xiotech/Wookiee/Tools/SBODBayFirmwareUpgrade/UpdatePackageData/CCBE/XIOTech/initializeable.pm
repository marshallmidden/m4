# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
#
# Purpose:
#   Base class used for all classed that are to be inheritable
##############################################################################
package XIOTech::initializeable;
use strict;

##############################################################################
#Base class for any class that wants to be inheritable
##############################################################################
sub new
{
    my ($class, @args) = @_;
    my $self = bless{}, ref($class) || $class;
    $self->_init(@args);
    return $self;
}

sub _init
{
    print STDERR "DEBUG- Missing _init for a dervied class.\n";
}

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:56  RysavyR
# Initial revision
#
# Revision 1.3  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.2  2001/10/31 15:42:02  NigburC
# Updated the command line to include the "logInfo" command to display
# the last N log messages.
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
