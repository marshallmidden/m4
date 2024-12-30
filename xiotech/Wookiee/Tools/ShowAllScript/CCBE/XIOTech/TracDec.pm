# $Id: TracDec.pm 145456 2010-08-11 20:33:28Z m4 $
#====================================================================
# FILE NAME:    TracDec.pm
#
# DESCRIPTION:  Decodes/annotates a CCB trace dump file.
#====================================================================

package XIOTech::TracDec;

use warnings;
use strict;

use XIOTech::CmdCodeHashes;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;

    @ISA         = qw(Exporter);
    @EXPORT      = qw(
                        &CCBDecoder
                     );

    #%EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],

    # TestLibs::Logging::logVersion(__PACKAGE__, q$Revision: 145456 $);
}
    our @EXPORT_OK;


########################################################################
#                         exported function(s)
########################################################################

my $gVersion = 0; # global version num


sub CCBDecoder
{
    my ($buffer, $offset, $version) = @_;

    my $outStr;
    my @dataArray;
    my $busFreq;
    my $intsPerSec;
    my $reload;
    my $firstT;
    my $firstO;
    my $length;
    my $i;
    my $daIndex;
    my @rowData;
    my $padLen;
    my $unpackTpl;
    my $total_uS;
    my $totalLines;
    my $count;
    my $lineCount;
    my $id;
    my $data;
    my $tCourse;
    my $tFine;
    my $arrayOffset;
    my $lastTot;
    my $deltaT;
    my $absT;
    my $idDesc;
    my $dataDesc;
    my $rowSize;

    # Set the global version num if one was passed in.
    $gVersion = $version if defined($version);
    print "Decoding MRP Trace v$gVersion\n";

    $rowSize = 16;          # bytes per 'row' in the data

    my %AllHashes = BuildCmdCodeHashTables();


    #
    # Setup timing constants
    #
    # $busFreq = 33.333333333333e+6;  # RD processor
    $busFreq = 100.0e+6; # Zion
    $intsPerSec = 48;

    $reload = int($busFreq/$intsPerSec);

#    $outStr = "Reload = $reload\n\n";


    #
    # Work through the input buffer and build the data array we'll process
    # 
    # We also find the starting point for the data
    #
    $firstT = 1e+100;
    $firstO = 0;
    
    # Handle a starting offset into the buffer.  If none was passed in
    # set a default value.
    if (!defined($offset))
    {
        $offset = 0;
    }
   
    # Start processing after the offset.   
    $buffer = substr $buffer, $offset;      # MRS


    # get the overall length of the data buffer
    $length = length $buffer;           
    
    # Read data from the buffer a row at a time.  Place each row
    # in dataArray
    for ($i=0; $i<$length; $i+=$rowSize) 
    {
        # data array $rowSize (each row is 16 bytes
        $daIndex = $i / $rowSize;
        
        # init arrays
        @rowData = ();

        # guarantees a full row of something
        if ($i + $rowSize > $length) 
        {
            $padLen = ($i + $rowSize) - $length;
            $buffer .= pack "C$padLen", 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00;
        }


        # get/unpack data from buffer and put in array
        $unpackTpl = "L L L L";
        push @rowData, unpack $unpackTpl, $buffer;

        #
        # Calculate timestamp. Save the "earliest/smallest" 
        #
        $total_uS = CalcUSec($rowData[2], $rowData[3], $reload, $busFreq);
        if($total_uS and $total_uS < $firstT) 
        {
            $firstT = $total_uS;
            $firstO = $daIndex;         # this is the beginning offset
        }

        # save this row in the data array
        $dataArray[$daIndex] = [ @rowData ];

        # removes the processed data from the input buffer
        if (length($buffer) > $rowSize) 
        {
            $buffer = substr $buffer, $rowSize;
        }
    }

    # number of lines in the array
    $totalLines = 1 + $daIndex;
    print "offset = $firstO, totalLines = $totalLines\n";

    #
    # Initialize variables for reading the data
    #
    $count = 1;
    $total_uS = 0;
    my $skippedEntries = 0;

    $outStr .= "Line      Absolute Time    ";
    $outStr .= "       " if ($gVersion);
    $outStr .= "Relative Time    Type         Sub-Type (text/numeric)\n\n";

    # Process that data in dataArray
    for ( $lineCount = 0; $lineCount < $totalLines; $lineCount++ )
    {

        # Get the data from the array.  This handles the correct 
        # starting point and the wrap.  
        $arrayOffset = ($lineCount + $firstO) % $totalLines;

        # Get a row of data
        ($id, $data, $tCourse, $tFine) = @{$dataArray[$arrayOffset]};


        #
        # MRS: If this is an invalid entry skip it and go to the next row.
        # Count the number of entries that are skipped.
        # We are only looking at one value here, we could validate others...
        #
        if($tCourse == 0 || $tCourse == 0xFFFFFFFF) 
        {
            $skippedEntries++;
            next;
        }

        #
        # Calculate the timestamp. Use the previous one to calculate a difference.
        #
        $lastTot = $total_uS;
        $total_uS = CalcUSec($tCourse, $tFine, $reload, $busFreq);
        $total_uS -= $firstT if (!$gVersion);

        #
        # Format the timestamp data for display.
        #
        $absT = ( !$gVersion ) ? FormatTime($total_uS) : FormatTime2($total_uS);
        if ($lastTot == 0)
        {
            $lastTot = $total_uS;
        }
        $deltaT = FormatTime($total_uS - $lastTot);

        #
        # Figure out which class trace point this is; decode if possible.
        #
        $idDesc = sprintf "0x%08X", $id;
        $dataDesc = sprintf "0x%08X", $data;
       
        # MRP's use bit 31
        if($id & 0x80000000) {
            if( $AllHashes{MRP_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{MRP_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{MRP_DATA}{$data} ) {
                $dataDesc = $AllHashes{MRP_DATA}{$data};
            }
        }

        # Port Server Packets use bit 30
        elsif($id & 0x40000000) {
            if( $AllHashes{PI_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{PI_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{PI_DATA}{$data} ) {
                $dataDesc = $AllHashes{PI_DATA}{$data};
            }
        }

        # IPC uses bit 29
        elsif($id & 0x20000000) {
            if( $AllHashes{IPC_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{IPC_ID}{$id & 0x0000FFFF};
            }
            if ((($id & 0x0000FFFF) == 93) or (($id & 0x0000FFFF) == 94))
            {
                if( $AllHashes{PI_DATA}{$data} ) {
                    $dataDesc = $AllHashes{PI_DATA}{$data};
                }
            }
            elsif( $AllHashes{IPC_DATA}{$data} ) {
                $dataDesc = $AllHashes{IPC_DATA}{$data};
            }
        }

        # LOG uses bit 28
        elsif($id & 0x10000000) {
            use constant LOG_SEV_MASK        => 0xC000;
            use constant LOG_INFO            => 0x0000;
            use constant LOG_WARNING         => 0x4000;
            use constant LOG_ERROR           => 0x8000;
            use constant LOG_FATAL           => 0xC000;

            use constant LOG_PROP_MASK       => 0x3000;
            use constant LOG_HIDDEN          => 0x2000;
            use constant LOG_DEBUG           => 0x1000;
            use constant LOG_CUSTOMER        => 0x0000;

            use constant LOG_CODE_MASK       => 0x0FFF;

            if( $AllHashes{LOG_ID}{$id & 0x0000FFFF} ) {
                if (($data & LOG_PROP_MASK) == LOG_CUSTOMER) 
                {
                    $idDesc = "LOG Info"  if (($data & LOG_SEV_MASK) == LOG_INFO);
                    $idDesc = "LOG Warn"  if (($data & LOG_SEV_MASK) == LOG_WARNING);
                    $idDesc = "LOG Error" if (($data & LOG_SEV_MASK) == LOG_ERROR);
                    $idDesc = "LOG Fatal" if (($data & LOG_SEV_MASK) == LOG_FATAL);
                }
                elsif (($data & LOG_PROP_MASK) == LOG_DEBUG) 
                {
                    $idDesc = "LOG Debug I" if (($data & LOG_SEV_MASK) == LOG_INFO);
                    $idDesc = "LOG Debug W" if (($data & LOG_SEV_MASK) == LOG_WARNING);
                    $idDesc = "LOG Debug E" if (($data & LOG_SEV_MASK) == LOG_ERROR);
                    $idDesc = "LOG Debug F" if (($data & LOG_SEV_MASK) == LOG_FATAL);
                }
                elsif (($data & LOG_PROP_MASK) == LOG_HIDDEN) 
                {
                    $idDesc = "LOG Hidden I" if (($data & LOG_SEV_MASK) == LOG_INFO);
                    $idDesc = "LOG Hidden W" if (($data & LOG_SEV_MASK) == LOG_WARNING);
                    $idDesc = "LOG Hidden E" if (($data & LOG_SEV_MASK) == LOG_ERROR);
                    $idDesc = "LOG Hidden F" if (($data & LOG_SEV_MASK) == LOG_FATAL);
                }
                elsif (($data & LOG_PROP_MASK) == (LOG_HIDDEN|LOG_DEBUG)) 
                {
                    $idDesc = "LOG DebHid I" if (($data & LOG_SEV_MASK) == LOG_INFO);
                    $idDesc = "LOG DebHid W" if (($data & LOG_SEV_MASK) == LOG_WARNING);
                    $idDesc = "LOG DebHid E" if (($data & LOG_SEV_MASK) == LOG_ERROR);
                    $idDesc = "LOG DebHid F" if (($data & LOG_SEV_MASK) == LOG_FATAL);
                }
            }
            if( $AllHashes{LOG_DATA}{$data & LOG_CODE_MASK} ) {
                $dataDesc = $AllHashes{LOG_DATA}{$data & LOG_CODE_MASK};
            }
        }

        # X1 uses bit 27
        elsif($id & 0x08000000) {
            if( $AllHashes{X1_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{X1_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{X1_DATA}{$data} ) {
                $dataDesc = $AllHashes{X1_DATA}{$data};
            }
        }

        # X1 VDisk Cfg uses bit 26
        elsif($id & 0x04000000) {
            if( $AllHashes{X1_VDC_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{X1_VDC_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{X1_VDC_DATA}{$data} ) {
                $dataDesc = $AllHashes{X1_VDC_DATA}{$data};
            }
        }

        # X1 BF Passthru uses bit 25
        # Same data as PI_DATA
        elsif($id & 0x02000000) {
            if( $AllHashes{X1_BF_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{X1_BF_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{PI_DATA}{$data} ) {
                $dataDesc = $AllHashes{PI_DATA}{$data};
            }
        }

        # RM uses bit 24
        elsif($id & 0x01000000) {
            if( $AllHashes{RM_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{RM_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{RM_DATA}{$data} ) {
                $dataDesc = $AllHashes{RM_DATA}{$data};
            }
        }
        
        # Signals use bit 23
        elsif($id & 0x00800000) {
            if( $AllHashes{SG_ID}{$id & 0x0000FFFF} ) {
                $idDesc = $AllHashes{SG_ID}{$id & 0x0000FFFF};
            }
            if( $AllHashes{SG_DATA}{$data} ) {
                $dataDesc = $AllHashes{SG_DATA}{$data};
            }
            else {
                $dataDesc = $AllHashes{SG_DATA}{$data};
                if (!defined($dataDesc)) {
                    $dataDesc = "Not Defined";
                }
            }
        }

        #
        # Write the formatted data out
        #

        $outStr .= sprintf( "%-5u %17s %15s    %-12s %-23s / 0x%X\n",
                            $count++, $absT, $deltaT, $idDesc, 
                            $dataDesc, $data);

    }  # end of for $lineCount

    $outStr .= sprintf("\nSkipped %d empty or invalid entries\n", 
                       $skippedEntries);
        
    return $outStr;

}   # end of CCBDecoder()


##################################################################
################## other functions ( local) ######################
##################################################################

#
# Given a course and fine time, calculate total uS
#
sub CalcUSec
{
    my ($tc, $tf, $reload, $busFreq) = @_;

    # this works, trust me :-)
    return ( !$gVersion ) ?
        ((($tc * $reload) + $reload - $tf) * 1000000 / $busFreq) :
        $tc * 1000000 + $tf;
}

#
# Format a suitable output string
#
sub FormatTime
{
    my ($uSin) = @_;
    my ($uS, $mS, $s);
    my $sign = "";

    if($uSin < 0) 
    {
        $uSin = -$uSin;
        $sign = "(-)";
    }

    $uS   = $uSin - ( int($uSin / 1000) * 1000 );
    $uSin = int($uSin / 1000);
    $mS   = $uSin % 1000;
    $s    = int($uSin / 1000);
    
    return sprintf("%4us %03u_%06.2f%s", $s, $mS, $uS, $sign);
}

sub FormatTime2
{
    my ($uSin) = @_;
    my ($uS, $mS);
    my $sign = "";

    if($uSin < 0) 
    {
        $uSin = -$uSin;
        $sign = "(-)";
    }

    my ($secPart, $uSecPart);

    $secPart = $uSin / 1000000;
    $uSecPart = $uSin % 1000000;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday) = gmtime $secPart;

    $secPart = $sec * 1000000 + $uSecPart;

    return sprintf("%02u/%02u/%02u %02u:%02u:%02u.%06u%s",
    $mon+1, $mday, $year % 100, $hour, $min, $sec, $uSecPart, $sign);
}

#################################################################

1;

#################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
