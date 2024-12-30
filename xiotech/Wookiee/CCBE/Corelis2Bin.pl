#!/bin/perl -w
#====================================================================
#
# FILE NAME:    Corelis2Bin.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         6/7/02
#
# DESCRIPTION:  Converts a Corelis memory dump file to an equivalent
#               binary data file.
#
#====================================================================

($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script file-to-parse\n\n" }
($parsef)=@ARGV;
$outfile = "$parsef-bin";

#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to $outfile...\n";
open OUT, ">$outfile";
binmode OUT;

while(<>) 
{
    # Eat any lines that look like this:
    # "#Thu Jun 06 12:01:47 2002"
    next if /^#[A-Z][a-z][a-z] [A-Z][a-z][a-z] [0-9][0-9] /;
    # Eat any lines that look like this:
    # "#**** mem **** t=i960RN_RM **** w=1.0 ****"
    next if /^#\*\*\*\*/;
    # and eat blank lines
    next if /^\s*$/;

    @f = split;

    $n = pack "NNNN", unpack "LLLL", pack "H8H8H8H8", $f[1], $f[2], $f[3], $f[4];
    
    print OUT $n;
}

close OUT;

