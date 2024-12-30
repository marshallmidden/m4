#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    SerDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         6/7/2002
#
# DESCRIPTION:  Decodes a CCB serial dump file.
#
#====================================================================


($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script file-to-decode\n\n" }
($infile) = @ARGV;

$outfile = "$infile-out";

#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to $outfile...\n";

#
# Open the input file
#
open IN, "$infile" or die "\nAbort: Can't open $infile...\n";
binmode IN;

read IN, $buffer, 24 or die "\nAbort: Can't read $infile...\n";

($beginP, $inP, $outP, $endP, $mutex1, $mutex2) = unpack "LLLLLL", $buffer;
printf "beginP = 0x%08X\n", $beginP;
printf "inP    = 0x%08X\n", $inP;
printf "outP   = 0x%08X\n", $outP;
printf "endP   = 0x%08X\n", $endP;

$offset = $outP - $beginP + 24;
$firstReadLen = $endP - $outP;
$secondReadLen = $outP - $beginP;

#printf "Seeking to file offset %u/0x%08X (0x%08X)\n", $offset, $offset, $outP;
seek IN, $offset, 0 or die "\nAbort: Can't seek to $offset...\n"; 

#print "Reading $firstReadLen bytes\n";
read IN, $buffer, $firstReadLen or die "\nAbort: Can't read $infile...\n";
$buffer =~ s/\r//g;
$buffer =~ s/\0//g;
print OUT $buffer;

#printf "Seeking to file offset %u/0x%08X (0x%08X)\n", 24, 24, $beginP;
seek IN, 24, 0 or die "\nAbort: Can't seek to $beginP...\n"; 

#print "Reading $secondReadLen bytes\n";
read IN, $buffer, $secondReadLen or die "\nAbort: Can't read $infile...\n";
$buffer =~ s/\r//g;
$buffer =~ s/\0//g;
print OUT $buffer;

close IN;
close OUT;


$inP = $inP;
$mutex1 = $mutex1;
$mutex2 = $mutex2;
