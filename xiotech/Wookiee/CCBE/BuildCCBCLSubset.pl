#!/bin/perl -w 

# $Header$
#====================================================================
#
# FILE NAME:    BuildCCBCLSubset.pl
#
# AUTHOR:       Randy Rysavy, Mark Schibilla
#
# DATE:         8/27/2002
#
# DESCRIPTION:  Process the files that make up the CCBE/CCBCL removing
#               code which is between the #ifdef ENFGINEERING and #endif
#               tags.  These tags appear as comments to perl.
#
# NOTES:        Sample command line - 
#                   BuildCCBCLSubset.pl subset ccbcl.pl xiotech\*.pm
#
#                   subset       = output dir for processed files
#                   ccbcl.pl     = the main CCBCL file
#                   xiotech\*.pm = the dir and file spec defining the
#                                  additional perl modules required
#                                  by ccbcl.pl
#
#               THE USER MUST CREATE SUBSET AND SUBSET\XIOTECH BEFORE
#               RUNNING THE PROGRAM.
#
#====================================================================

my $printThisLine = 1;
my $nextFile = "";


#
# Validate input parms
#
($script = $0) =~ s/^.*\\//;
unless (@ARGV > 1) { die "\nUsage: $script Output_Directory Input_Files...\n\n" }

# First arg is the output file directory
$outDir = shift @ARGV;

if (! -d $outDir) 
{
    die "Can't open directory \"$outDir\" for output...\n";
}


# Get the file list from the command line.  Handle wildcards 
while (@ARGV) 
{
    $parm = shift @ARGV;    # Get 1 file at a time from the arg list
    @files = glob($parm);   # expand wildcards e.g. *.pm

    # Process each file in the argument list
    while (@files) 
    {
        $nextFile = shift @files;
        
        # Open the input and output files
        open(FILEIN, "$nextFile");
        # open(FILEOUT, ">$outDir/$nextFile"); #or die "\nAbort: Can't open $outDir/$nextFile...\n");
        open FILEOUT, ">$outDir/$nextFile" or die "\nAbort: Can't open $outDir/$nextFile...\n";


        select(FILEOUT);

        print STDOUT "Processing $nextFile...\n";

        #
        # Process each line in the input file.  Print all lines that are not 
        # between the #ifdef ENGINEERING and #endif "directives" to the
        # output file.
        #
        while(<FILEIN>) 
        {
            # When #ifdef ENGINEERING is encountered, turn print off
            if (/^#ifdef\s+ENGINEERING/) 
            {
                die "#ifdef/#endif mismatch" if (!$printThisLine);
                $printThisLine = 0;
            }

            # When #endif is encountered, turn print back on
            if (/^#endif/) 
            {
                die "#ifdef/#endif mismatch" if ($printThisLine);
                $printThisLine = 1;
                next;
            }

            # Print a line to the file in the output directory
            print if ($printThisLine);
        }

        # Close the input and output files before opening new ones
        close(FILEIN);
        close(FILEOUT);
    
    }
}

#**********************************************************************
