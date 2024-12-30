#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    ProfDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         5/24/2001, updated 5/15/2002
#
# DESCRIPTION:  Analyze a "profile" dump to determine top CPU users.
#
#====================================================================

use	Getopt::Std;

#
# Get the command line args
#
$opt_l=0; 
getopts('l:');
if($opt_l) {
    $linkmap = $opt_l;
}
else {
    $linkmap = "CCBRun.map";
}
    
($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script [-l link-map] file-to-decode\n\n" }
($dumpf)=@ARGV;
$outfile = "$dumpf-out";

#
# Setup timing constants
#
$numSample = 250000;
$busFreq = 100.0e+6; # Zion
$intsPerSec = 48; # Timer 0
$samplesPerSec = 4125; # Timer 1

$reload = int($busFreq/$intsPerSec);
print "Reload = $reload\n";

#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to $outfile...\n";

$curModule = "";

#
# Build an array of all of the function names/addresses from the link map.
#
open F, "$linkmap" or die "\nAbort: Can't open $linkmap...\n";

while(<F>) {
    if(/text.*\.o/) {

        @line = split;
        $last = @line-1;
        ($curModule = $line[$last]) =~ s/\[.*\]//;
    }
    if(/ +0xa0[0-9a-f]{6} {16}/) {
        @line = split;
        # array def:  0:funcAddr  1:funcName  2:moduleName  3:count
        push @funcs, [(oct $line[0], $line[1], $curModule, 0)];
    }
}
close F;
push @funcs, [(0xFFFFFFFF, 0, "", 0)];
$totFuncs = @funcs;

#
# Given a course and fine time, calculate total uS
#
sub CalcUSec
{
    my ($tc, $tf) = @_;

    # this works, trust me :-)
    return ((($tc * $reload) + $reload - $tf) * 1000000 / $busFreq);
}

#
# Process the dump file, counting rip occurances in the link map array.
#
open F, "$dumpf" or die "\nAbort: Can't open $dumpf...\n";
binmode F;
read F, $buffer, 16;
($sT, $sR, $fT, $fR) = unpack "LLLL", $buffer;

$readcnt = 0;
while(read F, $buffer, 4) {
    $rip = unpack "L", $buffer;

    $readcnt++;
    if($readcnt%3500 == 0) {
        print STDERR ".";
    }

    #
    # Do a Binary Search to find the proper slot to add it to
    #
    $lowIdx = 0;
    $hiIdx = $totFuncs-1;
    while($lowIdx < $hiIdx) {
        $midIdx = int (($lowIdx+$hiIdx)/2);
        if($funcs[$midIdx][0] <= $rip) {
            $lowIdx = $midIdx+1;
        }
        else {
            $hiIdx = $midIdx;
        }
    }
    $lowIdx--; # back up one
    
    if($funcs[$lowIdx][1] eq ".text") {
        $lowIdx--;
    }
    $funcs[$lowIdx][3]++;
}
print STDERR "\n";


for($i=0; $i<$totFuncs; $i++) {
    if($funcs[$i][1] eq "IDLE") {
        $idleCnt = $funcs[$i][3];
        last;
    }
}

$start  = CalcUSec($sT, $sR);
$finish = CalcUSec($fT, $fR);

$actualRunTime    = $finish-$start;
$expectedRunTime  = $numSample/$samplesPerSec*1e+6;
$interruptTime    = $actualRunTime - $expectedRunTime;
$interruptPercent = $interruptTime/$actualRunTime*100;

$numNonIdle  = $numSample - $idleCnt;
$idlePercent = $idleCnt/$numSample*(100.0-$interruptPercent);

printf OUT "\nTotal Idle Time = %.1f%%\n", $idlePercent;
printf OUT "Total Interrupt Time = %.1f%%\n", $interruptPercent;
printf OUT "Percentage of remaining time used, by user:\n\n";
printf OUT "%%  Function                       Module               Relative Use\n";
print OUT  "-" x 80 . "\n";

#
# Walk the link map array and print out used functions
#
for($i=0; $i<$totFuncs; $i++) {
    $percent = int $funcs[$i][3]/$numNonIdle*100+0.5;
    if($funcs[$i][3] && ($funcs[$i][1] ne "IDLE") && ($percent >= 1) ) {
        printf OUT  "%-2u %-30s %-20s %s\n", 
            $percent, $funcs[$i][1], $funcs[$i][2], "*" x ($percent/2);
    }
}

print OUT "\n";

close F;
close OUT;
