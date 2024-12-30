#!/usr/bin/perl -w
# $Header$
#====================================================================
#
# FILE NAME:    colorize.pl
#
# AUTHOR:       Michael McMaster
#
# DATE:         02/25/2005
#
# DESCRIPTION:  Formats and colors console output based upon the
#               regular expressions in the settings file.
#
#====================================================================
use strict;
use Getopt::Std;
use File::Basename;
use IO::Handle;

##
# Version control
##
$Getopt::Std::STANDARD_HELP_VERSION = 1;
use constant VERSION => "1.0";

STDOUT->autoflush( 1 );
STDERR->autoflush( 1 );

# Foreground colors
use constant FG_BLACK         => "\e[30m";
use constant FG_RED           => "\e[31m";
use constant FG_RED_BOLD      => "\e[31;1m";
use constant FG_GREEN         => "\e[32m";  
use constant FG_GREEN_BOLD    => "\e[32;1m";  
use constant FG_YELLOW        => "\e[33m";  
use constant FG_YELLOW_BOLD   => "\e[33;1m";  
use constant FG_BLUE          => "\e[34m";  
use constant FG_BLUE_BOLD     => "\e[34;1m";  
use constant FG_MAGENTA       => "\e[35m";  
use constant FG_MAGENTA_BOLD  => "\e[35;1m";  
use constant FG_CYAN          => "\e[36m";  
use constant FG_CYAN_BOLD     => "\e[36;1m";  
use constant FG_WHITE         => "\e[37m";  
use constant FG_WHITE_BOLD    => "\e[37;1m";  

# Background colors
use constant BG_BLACK         => "\e[40m";
use constant BG_RED           => "\e[41m";
use constant BG_RED_BOLD      => "\e[41;1m";
use constant BG_GREEN         => "\e[42m";  
use constant BG_GREEN_BOLD    => "\e[42;1m";  
use constant BG_YELLOW        => "\e[43m";  
use constant BG_YELLOW_BOLD   => "\e[43;1m";  
use constant BG_BLUE          => "\e[44m";  
use constant BG_BLUE_BOLD     => "\e[44;1m";  
use constant BG_MAGENTA       => "\e[45m";  
use constant BG_MAGENTA_BOLD  => "\e[45;1m";  
use constant BG_CYAN          => "\e[46m";  
use constant BG_CYAN_BOLD     => "\e[46;1m";  
use constant BG_WHITE         => "\e[47m";  
use constant BG_WHITE_BOLD    => "\e[47;1m";  

# Generic color settings
use constant RESET_COLOR      => "\e[0m";
use constant FG_DEFAULT       => FG_WHITE;
use constant BG_DEFAULT       => BG_BLACK;
use constant START_COLOR      => RESET_COLOR;

# Flags
use constant FLAG_WHOLE_LINE  => (1 << 0);
use constant FLAG_NO_SUB_LINE => (1 << 1);
use constant FLAG_IGNORE_CASE => (1 << 2);

##
# Declare globals
##
my $iniFile = $ENV{HOME} . "/.colorize.ini";
my $debug = 0;
my @colorArray;
my $currentColor = START_COLOR;

##
# Parse the input parameters
##
our $opt_h;
our $opt_f;
our $opt_x;
getopts('hf:x');

if( $opt_h )
{
    HELP_MESSAGE( );
}    

if( $opt_f )
{
    $iniFile = $opt_f;
}    

if( $opt_x )
{
    $debug = 1;
}    

##
# Open and read the ini file 
##
if( -r $iniFile )
{
    my $line = "";
    my ($colors, $flags, $expression);

    open INI, "$iniFile";
    print "Reading $iniFile...\n";

    while( $line = <INI> )
    {
        ##
        # Strip any newlines
        ##
        chomp $line;

        ##
        # Skip any comment or blank lines
        ##
        if( ($line =~ m/^#/) ||
            ($line !~ m/([^\s])/ig) )
        {
            if( $debug != 0 )
            {
                print( "Skipping: [$line]\n" );
            }

            next;
        }
        
        ##
        # Split the line into the color, flags, and regular expression
        ##
        ($colors, $line) = split( /\s*:\s*/, $line, 2 );
        ($flags, $expression) = split( /\s*=\s*/, $line, 2 );

        if( $colors && $flags && $expression )
        {
            my $colorString = "";
            my $flagBits = 0;

            if( $debug != 0 )
            {
                print( "*** New ini line ***\n" );
                print( "colors:     $colors\n" );
                print( "flags:      $flags\n" );
                print( "expression: $expression\n" );
            }

            ##
            # Decode the color(s) to be used for the regular expression
            ##
            $colorString .= FG_BLACK         if ($colors =~ s/\bFG_BLACK\b//gi);
            $colorString .= FG_RED           if ($colors =~ s/\bFG_RED\b//gi);
            $colorString .= FG_RED_BOLD      if ($colors =~ s/\bFG_RED_BOLD\b//gi);
            $colorString .= FG_GREEN         if ($colors =~ s/\bFG_GREEN\b//gi);
            $colorString .= FG_GREEN_BOLD    if ($colors =~ s/\bFG_GREEN_BOLD\b//gi);
            $colorString .= FG_YELLOW        if ($colors =~ s/\bFG_YELLOW\b//gi);
            $colorString .= FG_YELLOW_BOLD   if ($colors =~ s/\bFG_YELLOW_BOLD\b//gi);
            $colorString .= FG_BLUE          if ($colors =~ s/\bFG_BLUE\b//gi);
            $colorString .= FG_BLUE_BOLD     if ($colors =~ s/\bFG_BLUE_BOLD\b//gi);
            $colorString .= FG_MAGENTA       if ($colors =~ s/\bFG_MAGENTA\b//gi);
            $colorString .= FG_MAGENTA_BOLD  if ($colors =~ s/\bFG_MAGENTA_BOLD\b//gi);
            $colorString .= FG_CYAN          if ($colors =~ s/\bFG_CYAN\b//gi);
            $colorString .= FG_CYAN_BOLD     if ($colors =~ s/\bFG_CYAN_BOLD\b//gi);
            $colorString .= FG_WHITE         if ($colors =~ s/\bFG_WHITE\b//gi);
            $colorString .= FG_WHITE_BOLD    if ($colors =~ s/\bFG_WHITE_BOLD\b//gi);

            $colorString .= BG_BLACK         if ($colors =~ s/\bBG_BLACK\b//gi);
            $colorString .= BG_RED           if ($colors =~ s/\bBG_RED\b//gi);
            $colorString .= BG_RED_BOLD      if ($colors =~ s/\bBG_RED_BOLD\b//gi);
            $colorString .= BG_GREEN         if ($colors =~ s/\bBG_GREEN\b//gi);
            $colorString .= BG_GREEN_BOLD    if ($colors =~ s/\bBG_GREEN_BOLD\b//gi);
            $colorString .= BG_YELLOW        if ($colors =~ s/\bBG_YELLOW\b//gi);
            $colorString .= BG_YELLOW_BOLD   if ($colors =~ s/\bBG_YELLOW_BOLD\b//gi);
            $colorString .= BG_BLUE          if ($colors =~ s/\bBG_BLUE\b//gi);
            $colorString .= BG_BLUE_BOLD     if ($colors =~ s/\bBG_BLUE_BOLD\b//gi);
            $colorString .= BG_MAGENTA       if ($colors =~ s/\bBG_MAGENTA\b//gi);
            $colorString .= BG_MAGENTA_BOLD  if ($colors =~ s/\bBG_MAGENTA_BOLD\b//gi);
            $colorString .= BG_CYAN          if ($colors =~ s/\bBG_CYAN\b//gi);
            $colorString .= BG_WHITE         if ($colors =~ s/\bBG_WHITE\b//gi);
            $colorString .= BG_WHITE_BOLD    if ($colors =~ s/\bBG_WHITE_BOLD\b//gi);

            ##
            # Decode the flag(s) to be used for the regular expression
            ##
            $flags =~ s/\bNONE//gi;
            $flagBits |= FLAG_WHOLE_LINE     if ($flags =~ s/\bWHOLE_LINE\b//gi);
            $flagBits |= FLAG_NO_SUB_LINE    if ($flags =~ s/\bNO_SUB_LINE\b//gi);
            $flagBits |= FLAG_IGNORE_CASE    if ($flags =~ s/\bIGNORE_CASE\b//gi);

            ##
            # Check for any unprocessed colors or flags
            ##
            if( (length($colors) > 0) &&
                ($colors =~ m/([^\s])/ig) )
            {
                print( "Unknown color specified: [$colors]\n" );
                exit;
            }

            if( (length($flags) > 0) &&
                ($flags =~ m/([^\s])/ig) )
            {
                print( "Unknown flags specified: [$flags]\n" );
                exit;
            }

            push( @colorArray, [$colorString, $flagBits, $expression] );
        }
        else
        {
            print( "Unformatted line: [$line]\n" );
        }
    }

    close INI;
}
else
{
    print "Could not read $iniFile\n";
    exit;
}

##
# Start out with the reset coloring
##
SetColor( START_COLOR );

##
# Print out a sample formatting for the specified ini file
##
if( $debug != 0 )
{
    foreach my $arrayItem( @colorArray )
    {
        my ($color, $flagBits, $expression) = @$arrayItem;

        printf( "Example: (%d) ", $flagBits );
        my $restoreColor = SetColor( $color );
        printf( "$expression" );
        SetColor( $restoreColor );
        printf( "\n" );
    }
}

##
# Process anything coming in through stdin by replacing anything
# that matches the sequence with the colored version of the string.
##
my $line = "";
my $searchItem = "";
my $previousColor = "";
my $preventSubLine = 0;
my ($color, $flagBits, $expression);
my %matchList;

while( 1 )
{
    ##
    # Grab a new line from the standard input
    ##
    while( $line = <STDIN> )
    {
        ##
        # Reset the list of string matches
        ##
        %matchList = ( );

        ##
        # We're starting on a new line - no coloring yet
        ##
        $preventSubLine = 0;

        ##
        # Go through "whole line" items and insert the colors into the line
        ##
        foreach $searchItem( @colorArray )
        {
            ($color, $flagBits, $expression) = @$searchItem;

            if( $flagBits & FLAG_WHOLE_LINE )
            {
                if( $flagBits & FLAG_IGNORE_CASE )
                {
                    if( $line =~ m/$expression/gi )
                    {
                        MatchAdd( 0, length($line), $color, \%matchList );

                        if( $flagBits & FLAG_NO_SUB_LINE )
                        {
                            $preventSubLine = 1;
                        }
                    }
                }
                else
                {
                    if( $line =~ m/$expression/g )
                    {
                        MatchAdd( 0, length($line), $color, \%matchList );

                        if( $flagBits & FLAG_NO_SUB_LINE )
                        {
                            $preventSubLine = 1;
                        }
                    }
                }
            }
        }

        if( $preventSubLine == 0 )
        {
            ##
            # Go through "sub-line" items and insert the colors into the line
            ##
            foreach $searchItem( @colorArray )
            {
                ($color, $flagBits, $expression) = @$searchItem;

                if( !($flagBits & FLAG_WHOLE_LINE) )
                {
                    if( $flagBits & FLAG_IGNORE_CASE )
                    {
                        while( $line =~ m/($expression)/gi )
                        {
                            MatchAdd( pos($line) - length($1), length($1), $color, \%matchList );
                        }
                    }
                    else
                    {
                        while( $line =~ m/($expression)/g )
                        {
                            MatchAdd( pos($line) - length($1), length($1), $color, \%matchList );
                        }
                    }
                }
            }
        }

        ##
        # Print out the color formatted line
        ##
        MatchPrint( \$line, \%matchList );
    }
}


################################################################################
## Subroutines
################################################################################
##
# VERSION_MESSAGE subroutine
##
sub VERSION_MESSAGE
{
    SetColor( RESET_COLOR );
    printf( "colorize.pl version %s\n", VERSION );
}

##
# HELP_MESSAGE subroutine
##
sub HELP_MESSAGE

{
    SetColor( RESET_COLOR );
    print "\nUsage: colorize.pl [-h] [-f ColorFile] [-x]\n";
    print "  -h           = This help text\n";
    print "  -f ColorFile = Color settings (default: $iniFile)\n";
    print "  -x           = Debug mode\n";
    exit;
}

##
# SetColor subroutine
#   Change the current console color and remember the previous color
##
sub SetColor
{
    my( $color ) = @_;
    my $returnColor = $currentColor;

    ##
    # Change the current color
    ##
    print $color;

    ##
    # Remember the previous color
    ##
    $currentColor = $color;

    ##
    # Return the color that we're changing from
    ##
    return( $returnColor );
}


##
# MatchAdd subroutine
#   Manage the colorization matches
##
sub MatchAdd
{
    my($start, $length, $color, $pMatchList) = @_;
    my $counter = 0;
    my $matchEntry;

    if( $debug != 0 )
    {
        print( "Before\n" );
        foreach $matchEntry (sort keys(%$pMatchList))
        {
            printf( "  Match: [$matchEntry] to [%d]\n", $matchEntry + $$pMatchList{$matchEntry}{LENGTH} );
        }
    }

    ##
    # Check if the hash is empty.  If it is, then inserting the
    # the new match is very easy.  If the hash is not empty, then
    # we must determine how to insert the new match within the
    # existing matches.
    ##
    if( scalar(keys(%$pMatchList)) > 0 )
    {
        if( $debug != 0 )
        {
            printf( "Insert %d to %d\n", $start, $start + $length );
        }

        ##
        # Find the key where the color will be inserted
        ##
        foreach $matchEntry ( sort keys %$pMatchList )
        {
            ##
            # Check if we're 'left' of the current entry
            ##
            if( $start < $matchEntry )
            {
                ##
                # We need to insert the new match here
                ##
                if( $debug != 0 )
                {
                    print( "  Insert $start before $matchEntry\n" );
                }

                if( $start + $length < $matchEntry )
                {
                    if( $debug != 0 )
                    {
                        print( "    No overlap\n" );
                    }

                    $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
                    last;
                }
                else
                {
                    if( ($start + $length) <
                        ($matchEntry + $$pMatchList{$matchEntry}{LENGTH}) )
                    {
                        if( $debug != 0 )
                        {
                            print( "    Partial overlap\n" );
                        }
                        
                        ##
                        # Modify the start and length of the existing match
                        ##
                        $$pMatchList{$start + $length} =
                            { LENGTH => $$pMatchList{$matchEntry}{LENGTH} - ($start + $length - $matchEntry),
                              COLOR => $$pMatchList{$matchEntry}{COLOR} };

                        ##
                        # Add the new match to the hash
                        ##
                        $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
                        last;
                    }
                    else
                    {
                        if( $debug != 0 )
                        {
                            print( "    Entire overlap\n" );
                        }

                        ##
                        # Remove the existing match from the hash
                        ##
                        delete $$pMatchList{$matchEntry};

                        ##
                        # Add the new match to the hash
                        ##
                        $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
                        last;
                    }
                }
            }
            elsif( $start == $matchEntry )
            {
                ##
                # We've matched a current entry
                ##
                if( $debug != 0 )
                {
                    print( "  Insert $start at $matchEntry\n" );
                }

                if( ($start + $length) <
                    ($matchEntry + $$pMatchList{$matchEntry}{LENGTH}) )
                {
                    ##
                    # Create a new entry for the partial overlap
                    ##
                    if( $debug != 0 )
                    {
                        print( "    Partial overlap\n" );
                    }

                    $$pMatchList{ ($start + $length) } =
                        { LENGTH => $$pMatchList{$matchEntry}{LENGTH} - $length,
                          COLOR => $$pMatchList{$matchEntry}{COLOR} };
                }
                else
                {
                    if( $debug != 0 )
                    {
                        print( "    Entire overlap\n" );
                    }
                }

                $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
                last;
            }
            elsif( $start < ($matchEntry + $$pMatchList{$matchEntry}{LENGTH}) )
            {
                ##
                # We're 'right' the current entry, but the current entry's
                # length overlaps with the new entry's start.
                ##
                if( $debug != 0 )
                {
                    print( "  Insert $start into $matchEntry\n" );
                }

                if( ($start + $length) < ($matchEntry + $$pMatchList{$matchEntry}{LENGTH}) )
                {
                    if( $debug != 0 )
                    {
                        print( "  New entry on right\n" );
                    }

                    ##
                    # Create a new entry for the 'right side'
                    ##
                    $$pMatchList{$start + $length} =
                        { LENGTH => $$pMatchList{$matchEntry}{LENGTH} - ($start + $length) - $matchEntry,
                          COLOR => $$pMatchList{$matchEntry}{COLOR} };
                }

                ##
                # Shorten the length of the existing match
                ##
                $$pMatchList{$matchEntry} =
                    { LENGTH => $start - $matchEntry,
                      COLOR => $$pMatchList{$matchEntry}{COLOR} };

                ##
                # Add the new entry to the hash
                ##
                $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
                last;
            }
        }
    }
    else
    {
        $$pMatchList{$start} = { LENGTH => $length, COLOR => $color };
    }

    if( $debug != 0 )
    {
        print "After\n";
        foreach $matchEntry (sort keys(%$pMatchList))
        {
            printf( "  Match: [$matchEntry] to [%d]\n",
                $matchEntry + $$pMatchList{$matchEntry}{LENGTH} );
        }
        print "\n";
    }
}


##
# MatchPrint subroutine
#   Colorize and print the line
##
sub MatchPrint
{
    my($pLine, $pMatchList) = @_;
    my $currentPosition = 0;
    my $matchEntry;
    my $previousColor = $currentColor;

    foreach $matchEntry (sort keys %$pMatchList)
    {
        if( $currentPosition < $matchEntry )
        {
            printf( substr($$pLine, $currentPosition, $matchEntry - $currentPosition) );
            $currentPosition += $matchEntry - $currentPosition;
        }

        if( $currentPosition == $matchEntry )
        {
            $previousColor = $currentColor;
            SetColor( $$pMatchList{$matchEntry}{COLOR} );
            print( substr($$pLine, $currentPosition, $$pMatchList{$matchEntry}{LENGTH}) );
            $currentPosition += $$pMatchList{$matchEntry}{LENGTH};
            SetColor( $previousColor );
        }
    }

    if( $currentPosition < length($$pLine) )
    {
        print( substr($$pLine, $currentPosition, length($$pLine) - $currentPosition) );
        $currentPosition += length($$pLine) - $currentPosition;
    }
}
