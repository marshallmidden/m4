# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Keith House
#
# Purpose:
#   Error messages for packets coming from the tbolt.
##############################################################################
package XIOTech::error;

use Exporter();
@ISA = qw(Exporter);
@EXPORT = qw(
    verifyParameters
);

use XIOTech::xiotechPackets;
use XIOTech::logMgr;
use XIOTech::cmdMgr;

use strict;

##############################################################################
# Name: verifyParameters
#
# Desc: Verifies parameters are the correct type and within a range if
#       applicable.
#
# In:   reference to an array - @_ from some function
#       reference to an array of arrays - list of correct parameter types
#
# Returns:  nothing on no error
#           prints message string and then dies
#
#   'd'  = digit
#   's'  = string
#   'sc' = scalar
#   'a'  = array
#   'h'  = hash
#   'c'  = code
#   'g'  = glob
#   'b'  = BigInt
#   'o'  = Object, 2nd parameter is the object name
#   'i'  = ignore, don't care what type it is
#
#   Example:
#
#   !usr/bin/perl -w
#
#   use XIOTech::error;
#   use strict;
#
#   my ($s);
#
#   $s = blah(0,2,"hi",[1,2,3]);
#
#   if (defined($s))
#   {
#       print "$s\n";
#   }
#   else
#   {
#       print "all parameters check ok\n";
#   }
#
#   sub blah
#   {
#       # first parameter is a digit with a min of 0 and a max of 2 inclusive
#       # second parameter is the same
#       # third parameter is a string
#       # fourth parameter is a hash
#       # last item in the array is the name of the function
#       my $array = [['d',0,2],['d',0,2],['s'],['h'],["blah"]];
#
#       return(verifyParameters(\@_, $array));
#   }
##############################################################################
sub verifyParameters
{
    my ($parms, $specifics, $values, $i, $j, $msg, $valid);

    use constant TYPE        => 0;
    use constant MIN         => 1;
    use constant MAX         => 2;
    use constant OBJECT_TYPE => 1;

    $values = shift; # parameter list @_
    $parms = shift; # definitions of what parameter list should be

    for ($i = 0; $i < scalar(@$parms) - 1; $i++)
    {
        $j = $i + 1;
        $specifics = $parms->[$i];

        if (defined($values->[$i]))
        {
            # Check all the expected types against the values
            if ($specifics->[TYPE] eq 'd')
            {
                if ($values->[$i] !~ /^-?\d+$/)
                {
                    $msg .= "Parameter $j can only be a digit.\n";
                }
                elsif (defined($specifics->[MIN]) &&
                        defined($specifics->[MAX]))
                {
                    if ($values->[$i] < $specifics->[MIN] ||
                        $values->[$i] > $specifics->[MAX])
                    {
                        $msg .= "Parameter $j is out of range: ";
                        $msg .= "$specifics->[MIN] < $values->[$i] ";
                        $msg .= "< $specifics->[MAX].\n";
                    }
                }
            }
            elsif ($specifics->[TYPE] eq 's')
            {
                if ($values->[$i] !~ /^.+$/)
                {
                    $msg .= "Parameter $j can only be a string.\n";
                }
                elsif (defined($specifics->[MIN]) &&
                        defined($specifics->[MAX]))
                {
                    if (length($values->[$i]) < $specifics->[MIN] ||
                        length($values->[$i]) > $specifics->[MAX])
                    {
                        $msg .= "Parameter $j is out of range: ";
                        $msg .= "$specifics->[MIN] < $values->[$i] ";
                        $msg .= "< $specifics->[MAX].\n";
                    }
                }
            }
            elsif ($specifics->[TYPE] eq 'sc')
            {
                my $type = ref($values->[$i]);

                print "Scalar ref = $type\n";

                if (ref($values->[$i]) ne 'SCALAR')
                {
                    $msg .= "Parameter $j can only be a scalar.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'a')
            {
                if (ref($values->[$i]) ne 'ARRAY')
                {
                    $msg .= "Parameter $j can only be an array.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'h')
            {
                if (ref($values->[$i]) ne 'HASH')
                {
                    $msg .= "Parameter $j can only be a hash.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'c')
            {
                if (ref($values->[$i]) ne 'CODE')
                {
                    $msg .= "Parameter $j can only be code.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'g')
            {
                if (ref($values->[$i]) ne 'GLOB')
                {
                    $msg .= "Parameter $j can only be a glob.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'b')
            {
                if ($values->[$i] !~ /^[\+\-]{1}\d+$/ &&
                    ref($values->[$i]) ne 'Math::BigInt')
                {
                    $msg .= "Parameter $j can only be a BigInt.\n";
                }
            }
            elsif ($specifics->[TYPE] eq 'o')
            {
                if ($values->[$i] !~ /^[\+\-]{1}\d+$/ &&
                    ref($values->[$i]) ne $specifics->[OBJECT_TYPE])
                {
                    $msg .= "Parameter $j can only be a $specifics->[OBJECT_TYPE].\n";
                }
            }
            elsif ($specifics->[TYPE] ne 'i')
            {
                $msg .= "Unknown parameter type '$specifics->[TYPE]', ";
                $msg .= "invalid use of sub verifyParameters.\n";
            }
        }
        else
        {
            $msg .= "Invalid number of parameters.\n";
        }
    }

    # was there a bad parameter?

    if (defined($msg))
    {
        my $function = @$parms[scalar(@$parms) - 1]->[0];
        my $number = scalar(@$parms) - 1;

        $msg = "Error in parameter list for function $function\n".$msg;
        $msg .= "This function takes $number parameters:\n$function(";

        for ($i = 0; $i < scalar(@$parms) - 1; $i++)
        {
            $specifics = $parms->[$i];
            if ($specifics->[TYPE] eq 'd')
            {
                $msg .= "digit,";
            }
            elsif ($specifics->[TYPE] eq 's')
            {
                $msg .= "string,";
            }
            elsif ($specifics->[TYPE] eq 'sc')
            {
                $msg .= "scalar,";
            }
            elsif ($specifics->[TYPE] eq 'a')
            {
                $msg .= "array,";
            }
            elsif ($specifics->[TYPE] eq 'h')
            {
                $msg .= "hash,";
            }
            elsif ($specifics->[TYPE] eq 'c')
            {
                $msg .= "code,";
            }
            elsif ($specifics->[TYPE] eq 'g')
            {
                $msg .= "glob,";
            }
            elsif ($specifics->[TYPE] eq 'b')
            {
                $msg .= "BigInt,";
            }
            elsif ($specifics->[TYPE] eq 'i')
            {
                $msg .= "ignore,";
            }
            elsif ($specifics->[TYPE] eq 'o')
            {
                $msg .= $specifics->[OBJECT_TYPE] . ",";
            }
        }

        $msg .= "\b)\n";
        logMsg($msg);
    }

    my %hash;

    if (defined($msg))
    {
        $hash{STATUS} = PI_ERROR;
        $hash{ERROR_CODE} = 0;
        $hash{MESSAGE} = $msg;
        
        $hash{STATUS_MSG} = "CCBCL Parameter error:";
		$hash{ERROR_MSG} = "\n" . $msg;
    }

    return %hash;
}

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:54  RysavyR
# Initial revision
#
# Revision 1.10  2002/02/08 12:55:41  HoltyB
# added error code and status messages on return from ccbcl invalid parameter
#
# Revision 1.9  2002/01/09 13:28:48  HoltyB
# moved getStatusMsg to cmUtils.pm
# removed getErrorMsg
#
# Revision 1.8  2001/11/30 14:26:39  NigburC
# Added getStatusMsg function to get message text for a given STATUS code.
#
# Revision 1.7  2001/11/14 12:57:37  NigburC
# Modified verify parameters to return a hash instead of just the message.
# This will allow the code to return it to the command line where it will be
# processed like any other error, except there is a MESSAGE in the
# hash which will be displayed describing the parameter error that occurred.
#
# Revision 1.6  2001/11/12 21:23:22  NigburC
# Added reference to the log manager module.
#
# Revision 1.5  2001/11/08 12:26:39  NigburC
# Modified verifyParameters to return a message string on failure,
# undef on success.  It was calling "die" before which would end the
# command line.
#
# Revision 1.4  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
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
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
