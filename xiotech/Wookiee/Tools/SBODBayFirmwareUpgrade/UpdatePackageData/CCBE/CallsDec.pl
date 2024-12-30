#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    CallsDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         5/12/2001
#
# DESCRIPTION:  Annotate a dumped callstack with associated
#               function names.
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
unless (@ARGV == 1) { die "\nUsage: $script [-l link-map] file-to-decode.dmp\n\n" }
($dumpf)=@ARGV;
$outfile = "$dumpf-out";

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
        push @funcs, [(oct $line[0], $line[1], $curModule)];
    }
}
close F;
$totFuncs = @funcs;

#
# Process the dump file, adding function and module names to the RIP ptrs.
#
open F, "$dumpf" or die "\nAbort: Can't open $dumpf...\n";

while(<F>) {
    s/\x0//g;
    if(/RIP:/) {
        chomp;
        @line = split;
        $last = @line-1;
        ($funcAddr = $line[$last]) =~ s/RIP://;
        $funcAddr = oct $funcAddr;
        
        for($i=0; $i<$totFuncs-1; $i++) {
            if($funcAddr>=$funcs[$i][0] && $funcAddr<$funcs[$i+1][0]) {
                if($funcs[$i][1] eq ".text") {
                    $i--;
                }
                $fn = sprintf ("%s+%d", $funcs[$i][1],
                    $funcAddr-$funcs[$i][0]);
                $_ .= sprintf (" %-28s %s\n", $fn, $funcs[$i][2]);
                last;
            }
        }
    }
    print OUT $_;
}
close F;
close OUT;
