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
our $opt_l;
getopts('l:');

if($opt_l) {
    $linkmap = $opt_l;
}
else {
    $linkmap = "CCBRun.map";
}
    
$intsPerSec = 8; # default: BE/FE 125mS Timer 0 
    
($script = $0) =~ s/^.*\\//;
unless (@ARGV == 2) { die "\nUsage: $script [-l link-map] file-to-decode ops-per-sec\n\n" }
my ($dumpf, $opsPerSec)=@ARGV;
$outfile = "$dumpf-out";

#
# Setup timing constants
#
$numSample = 250000;
$busFreq = 100.0e+6; # Zion
$samplesPerSec = 4125; # Timer 1
$reload = int($busFreq/$intsPerSec);

#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to \"$outfile\"\n";

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
read F, $buffer, 5*4; # throw the first 5 ints out (FW stuff)
read F, $buffer, 16;
($sT, $sR, $fT, $fR) = unpack "LLLL", $buffer;

$readcnt = 0;
print STDERR "working";
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
    if($funcs[$i][1] =~ /^IDLE[0-9]*/) {
        $idleCnt += $funcs[$i][3];
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


print OUT "\nDebug Info:\n";
print OUT "------------------------------------\n";
print OUT "reload:           $reload\n";
print OUT "numSample:        $numSample\n";
print OUT "idleCnt:          $idleCnt\n";
print OUT "numNonIdle:       $numNonIdle\n";
print OUT "opsPerSec:        $opsPerSec\n";
print OUT "start:            $start\n";
print OUT "finish:           $finish\n";
print OUT "actualRunTime:    $actualRunTime\n";
print OUT "expectedRunTime:  $expectedRunTime\n";
print OUT "interruptTime:    $interruptTime\n";
print OUT "interruptPercent: $interruptPercent\n";
print OUT "idlePercent:      $idlePercent\n";


# rel%  act%  Function                       Module               Relative Use
#--------------------------------------------------------------------------------
# 0.05  0.00  _TaskReadyByState              kernel_as.o
# 4.23  0.37  .pr10                          kernel_as.o          ****

printf OUT "\nTotal Idle Time = %.1f%%\n", $idlePercent;
printf OUT "Total Interrupt Time = %.1f%%\n", $interruptPercent;
printf OUT "Percentage of remaining time used, by user:\n\n";
print  OUT " Rel%  Act%    uS/Op  Function/Label                 Module               Relative Use\n";
print  OUT  "-" x 80 . "\n";

#
# Walk the link map array and print out used functions
#
my $percSum = 0.0;
my $timeSum = 0.0;
my $relPerc;
my $actPerc;
my $uSecPerOp;
my $uSecPerMod = 0.0;
my $lastMod = "";
my $relPercPerMod;
for($i=0; $i<$totFuncs; $i++) 
{
    $relPerc = $funcs[$i][3]/$numNonIdle*100;
    $actPerc = $funcs[$i][3]/$numSample*100;
    $uSecPerOp = (($actPerc/100)/$opsPerSec)*1000000;

    if($funcs[$i][3] && ($funcs[$i][1] !~ /^IDLE[0-9]*/))
    {
        if($lastMod eq "")
        {
            $uSecPerMod = $uSecPerOp;
            $relPercPerMod = $relPerc;
            $lastMod = $funcs[$i][2];
        }
        elsif($funcs[$i][2] eq $lastMod)
        {
            $uSecPerMod += $uSecPerOp;
            $relPercPerMod += $relPerc;
        }
        else
        {
            printf OUT "          => %7.3f, %5.2f%%<=\n\n", $uSecPerMod, $relPercPerMod;
            $uSecPerMod = $uSecPerOp;
            $relPercPerMod = $relPerc;
            $lastMod = $funcs[$i][2];
        }

        # limit the displayed list to something reasonable
        if ($relPerc >= .01) 
        {
            printf OUT  "%5.2f %5.2f  %7.3f  %-30s %-20s %s\n", 
            $relPerc, $actPerc, $uSecPerOp, 
            $funcs[$i][1], $funcs[$i][2], 
#            "*" x (int(($relPerc*4) + 0.5));
            "*" x (int(($relPerc/2) + 0.5));
            $percSum += $relPerc;
        }

        # sum this over all points
        $timeSum += $uSecPerOp;
    }
}
printf OUT "          => %7.3f <=\n\n", $uSecPerMod;

printf OUT "\nTotal Relative Percent: %.2f%%\n\n", $percSum;
printf OUT "Total uS/Op:            %.3f uS/Op\n\n", $timeSum;
print OUT "\n";

close F;
close OUT;

print "\n\nResults:\n";
print "------------------------------------\n";
open OUT, "$outfile" or die "\nAbort: Can't open $outfile...\n";
while(<OUT>) {
    print;
}
close OUT;

