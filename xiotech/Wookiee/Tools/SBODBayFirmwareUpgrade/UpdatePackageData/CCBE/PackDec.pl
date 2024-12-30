#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    PackDec.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         5/12/2001
#
# DESCRIPTION:  Decodes/annotates a CCB packet dump file.
#
#====================================================================
use XIOTech::CmdCodeHashes;


($script = $0) =~ s/^.*\\//;
unless (@ARGV == 1) { die "\nUsage: $script file-to-decode\n\n" }
($tracefile)=@ARGV;

$outfile = "$tracefile-out";


#
# Initialize hash tables
#
my %AllHashes = BuildCmdCodeHashTables();



#
# Open the output file
#
open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
print "Output being written to $outfile...\n";

#
# Open the input trace file
#
open F, "$tracefile" or die "\nAbort: Can't open $tracefile...\n";
binmode F;

sub parseCommands
{
    my ($pattern, $firstCmd, $hash) = @_;

    print "Pattern to match is: $pattern\n";
    
    #
    # seek back to the start of the file
    #
    seek F, 0, 0 or die;  

    #
    # Run through the file and find the PI START
    #
    while(read F, $buffer, 16) {

        ($text, $space, $commands) = unpack "a8a4L", $buffer;

        # print " $text \n";

        #
        # If we are at the end of the data or suspect a bad entry, exit.
        # We are only looking at one value here, we could validate others...
        #
        if($text eq $pattern) {
            print "$pattern found\n";
            printf  OUT "\nStart of $pattern data \n";
            print OUT "Cmd \t\tDescription \t\t\t\tCount \n";
            last;

        }

    }


    # Initialize variables for reading the data

    $count = 0;
    $firstCmd = 0;
    $commands = ($commands-16) / 4;


    while(read F, $buffer, 4) {

        ($cmdCount) = unpack "L", $buffer;


        if ($cmdCount)
        {
            my $id = $firstCmd+$count;
            if( $AllHashes{$hash}{$id} ) 
            {
                $dataDesc = $AllHashes{$hash}{$id};

                #
                # Write the formatted data out
                #
                printf OUT "0x%-5x \t%-25s \t%-5u\n", 
                      $id, $dataDesc, $cmdCount;
            }
            else
            {
                $dataDesc = " ";
                #
                # Write the unformatted data out
                #
                printf OUT "0x%-5x \t%-25s \t%-5u\n", 
                    $id, $dataDesc, $cmdCount;
            }
        }

        ++$count;

        if ($count >= $commands)
        {
            last;
        }

    }
}

parseCommands("PI START", 0, PI_DATA);
parseCommands("X1 START", 0, X1_DATA);
parseCommands("X1 VDC S", 0, X1_VDC_DATA);
parseCommands("X1_BF ST", 0, PI_DATA);
parseCommands("MRP STAR", 0, MRP_DATA);
parseCommands("IPC STAR", 600, IPC_DATA);



#
# Close files and exit
#
close F;
close OUT;
