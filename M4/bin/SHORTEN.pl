#!/usr/bin/perl -w
use strict;
use warnings;

#-----------------------------------------------------------------------------
# Script has 0 arguments.
#-----------------------------------------------------------------------------
my $previous = '';
my $current = '';
my $line;
my $flag = 0;

while ($line = <>) {
    if ($flag == 0)
    {
	if ($line =~ /^ACTION=/)
	{
	    $current = $line;
	    $flag = 1;
	}
	else
	{
	    print $line;
	}
    }
    else
    {
	if ($line =~ /^-----/)
	{
	    if ($previous ne $current)
	    {
		print $current;
		$previous = $current;
	    }
	    $current = '';
	    $flag = 0;
	    print $line;
	}
	else
	{
	    $current .= $line;
	}
    }
}

if ($current ne '')
{
    print $current;
}

exit 0;

#-----------------------------------------------------------------------------
# End of file SHORTEN.pl
