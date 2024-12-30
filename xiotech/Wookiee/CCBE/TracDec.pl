#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    TraceDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         5/12/2001
#
# DESCRIPTION:  Decodes/annotates a CCB trace dump file.
#
#====================================================================
use XIOTech::fmtFIDs;

($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script file-to-decode\n\n" }
($tracefile)=@ARGV;

$outfile = "$tracefile-out";

#
# Open the input trace file
#
open F, "$tracefile" or die "\nAbort: Can't open $tracefile...\n";
binmode F;
while (read F, $buffer, 0x10000)
{
    $inData .= $buffer;
}

print "Output being written to $outfile...\n"; 
CCBEDecodeFids ($inData, 256, $outfile);

#
# Close files and exit
#
close F;
