#!perl -w
#====================================================================
#
# FILE NAME:    Bin2Txt.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         11/21/2003
#
# DESCRIPTION:  Formats a binary dump file into a text readable
#               file with a choice of endian swapping formats.
#
#====================================================================

use strict;
use	Getopt::Std;

my $start = time();

#
# Output templates
#
my $fmtWord  = "%08X:  "."%08X %08X  %08X %08X"."  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";
my $fmtShort = "%08X:  "."%04X %04X %04X %04X  %04X %04X %04X %04X".
                                                "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";
my $fmtByte  = "%08X:  ".
   "%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X". 
                                                "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";
my $formatTpl = $fmtWord;

#
# Input templates
#
my $unpWord = "L4";
my $unpShort = "S8";
my $unpByte = "C16";
my $unpackTpl = $unpWord;

#
# Misc tamplates
#
my $byteTpl = "CCCC CCCC CCCC CCCC";

#
# Process command line args
#
our $opt_t; 
getopts('t:');

my $script;
($script = $0) =~ s/^.*\\//;
unless (@ARGV >= 1) { die "\nUsage: $script [-t word|short|byte] file-to-decode [starting-address]\n\n" }
my ($infile, $address) = @ARGV;
my $outfile = "$infile-out";

if (defined($address))
{
    if ($address =~ /^0x/i)
    {
        $address = oct $address;
    }
}
else
{
    $address = 0;
}

if ($opt_t)
{
    if ($opt_t !~ /^word$|^short$|^byte$/i)
    {
        print "Type must be WORD, SHORT or BYTE\n";
        exit;
    }
    else
    {
        if ($opt_t =~ /^short$/i)
        {
            $formatTpl = $fmtShort;
            $unpackTpl = $unpShort;
        }
        if ($opt_t =~ /^byte$/i)
        {
            $formatTpl = $fmtByte;
            $unpackTpl = $unpByte;
        }
    }
}

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

#
# Process the data
#
my $processed = 0;
my $buffer;

while (read IN, $buffer, 16)
{
    my @rowData = ();
    my @asciiData = ();
    my $padLen = 0;
    my $bufLen = length($buffer);
    $processed += $bufLen;
    
    # Getting the final line to display properly when not an even multiple
    # of 16 bytes is not easy given the way that this routine has been 
    # structured. So as a kludge, we pad the data out to the proper length
    # with 0xAA's, and then warn the caller that we did this.
    if ($bufLen < 16) 
    {
        $padLen = 16 - $bufLen;
        $buffer .= pack "C$padLen", 
            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA;
    }
    
    push @rowData, unpack $unpackTpl, $buffer;

    my $tmp;
    foreach $tmp (unpack $byteTpl, $buffer) 
    {
        if ($tmp < 0x20 or $tmp >= 0x7f) 
        {
            $tmp = 0x2e; # '.'
        }
        push @asciiData, $tmp;
    }

    my $str = sprintf  $formatTpl, $address, @rowData, @asciiData;
    
    $address += 16;

    print OUT $str;
    if ($padLen) 
    {
        print OUT "\nWarning: The final $padLen bytes (AA's) ARE NOT YOUR DATA!\n";
    }
    
    if (($processed & 0xFFFF) == 0)
    {
        print "=> $processed\r";
    }
}

close IN;
close OUT;

my $diff = time() - $start;
if ($diff == 0)
{
    $diff++;
}
 
#
# Print output statistics
#
printf "$processed bytes processed in %d seconds (%.1f KB/Sec)\n",
    $diff, $processed/$diff/1024;
