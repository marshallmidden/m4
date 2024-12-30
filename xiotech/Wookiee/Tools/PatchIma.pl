#!/perl/bin/perl -w

# $Header$
#====================================================================
#
# FILE NAME:    PatchIma.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         8/21/2000
#
# DESCRIPTION:  Patches the Firmware Header in the binary firmware 
#               image with the file length and CRC32 over the header
#               and over the entire file.  Also allows update of several
#               other pieces of data in the header.
#
#====================================================================

use Getopt::Std;

use constant GOOD          => 0;
use constant ERROR         => 1;

#
# Set file names
#
($script = $0) =~ s/^.*\\//;
unless (@ARGV > 0) { die "\nUsage: $script [-s sysRel (4 ch)] [-v version (4 ch)] ".
    "[-i index-file] file.ima [file2.ima ...]\n\n" }

#
# Parse the command line
#
getopts('v:s:i:');

#
# define header structure offsets:
#
my $headerLength   = 128;

# CRC table used by 'crc32'
@CRCtable = (
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
);


#
# Declare the 'run' subroutine.
# Input:   The entire system call is passed as arg 1.
# Returns: output from system call
#
sub run($)
{
	my ($line) = @_;
    my $output = "";
    my $returnCode = GOOD;
    my $tempFile = "PatchIma.tmp";

	print "=> $line\n";

    ##
    # Hijack stdout
    ##
    open SAVEOUT, ">&STDOUT";
    open STDOUT,  ">$tempFile" or die "Can't redirect stdout to $tempFile";
    select STDOUT; $| = 1;      # Enable autoflush

    ##
    # Do the command and capture the STDIO and STDERR output
    ##
    $returnCode = (system("$line 2>&1")) >> 8;
    close STDOUT or die "Can't close command output file $tempFile";

    ##
    # Restore previous stdout
    ##
    open STDOUT, ">&SAVEOUT" or die "Can't restore standard output";
    close SAVEOUT or die "Can't close backup standard output";

    ##
    # Pass the command output back to the caller
    ##
    open OUTPUT, "<$tempFile" or die "Can't open command output file $tempFile";
    while( <OUTPUT> )
    {
        $output .= $_;
    }
    close( OUTPUT ) or die "Can't close command output file $tempFile";
    unlink( "$tempFile" ) or die "Can't remove command output file $tempFile";

    ##
    # Die on error
    ##
    if( $returnCode != GOOD )
    {
        print( $output );
        print( "\nAbort: \"$line\" failed, return code = $returnCode\n" );
        exit( $returnCode );
    }

    return( $output );
}

#############################################################################
#
# SUB: ELF_GetFirmwareHeaderOffset
# Input:   ($fileName)
# Returns: ($errorCode, $offset)
#    $errorCode = GOOD or ERROR
#    $offset    = Byte offset of header from beginning of $file    
#
#############################################################################
sub ELF_GetFirmwareHeaderOffset
{
    my ($fileName) = @_;
    my ($returnCode, $offset) = (ERROR, 0);

    ##
    # Call objdump to get the section headers
    ##
    my $output = run( "objdump -h $fileName" );

    ##
    # Parse the objdump output to get the file offset of the firmware header
    ##
    if( $output =~ m/(\.fw\.header)[\s]+([A-Fa-f0-9]+)[\s]+([A-Fa-f0-9]+)[\s]+([A-Fa-f0-9]+)[\s]+([A-Fa-f0-9]+)/gi )
    {
        #print "1: $1\n"; # .fw.header
        #print "2: $2\n"; # Size
        #print "3: $3\n"; # VMA
        #print "4: $4\n"; # LMA
        #print "5: $5\n"; # File offset

        ##
        # Verify that the header sizes match between the image and PatchIma
        ##
        if( oct("0x".$2) == $headerLength )
        {
            ##
            # Match found - convert the offset string ($5) into binary ($offset)
            ##
            $returnCode = GOOD;
            $offset = oct( "0x" . $5 );
        }
        else
        {
            print( "ERROR: Header size sanity check failed\n" );
        }
    }

    return( $returnCode, $offset );
}


#
# Declare the 'crc32' subroutine.
# Input:   $file, $crcStart, $crcEnd, $skipStart, $skipEnd
# Returns: $crc32
#
#    crc = 0xFFFFFFFF;
#    for (Index = 0; Index < length; Index ++ )
#    {
#            crc = ((crc>>8) & 0x00FFFFFF) ^ crc_table[ (crc^Buffer[Index]) & 0xFF ];
#    }
#    return( crc^0xFFFFFFFF );
#
sub crc32 {
	my ($buffer, $byteValue, $chunkSize, $crcLength, $currentFileOffset, $crc, $template, $maxChunk);
    my ($file, $crcStart, $crcEnd, $skipStart, $skipEnd) = @_;

    $crc = 0xFFFFFFFF;
    $maxChunk = 512;  # number of bytes to process at a time.
                      # A number greater than this doesn't appear
                      # to improve performance any.
    
    #
    # Open the file and seek to the point where CRC should begin
    #
    open CRCF, "$file" or die "\nAbort: Can't open file $file...\n";
    binmode CRCF;
    seek CRCF, $crcStart, 0 or die "\nAbort: Can't seek in $file...\n";
    $currentFileOffset = $crcStart;
   
    if( $crcEnd > (-s $file) )
    {
        print( "ERROR: CRC endpoint is beyond the end of the input file\n" );
        exit( ERROR );
    }

    #
    # Create the template
    #
    $template = sprintf( "C%d", $maxChunk );
   
    while( $currentFileOffset < $crcEnd )
    {
        #
        # Read & unpack a chunk of data
        #
        $chunkSize = read CRCF, $buffer, $maxChunk;

        if( $chunkSize == 0 )
        {
            die "\nAbort: $file too short...\n";
        }

        @chunkBytes = unpack $template, $buffer;
        $crcLength = $#chunkBytes + 1;

        if( scalar(@chunkBytes) != $chunkSize )
        {
            printf( "\n" );
            printf( "chunkSize:  %d\n", $chunkSize );
            printf( "chunkBytes: %d\n", scalar(@chunkBytes) );
        }

        #
        # Adjust length to process if we read too much
        #
        if( ($currentFileOffset + $chunkSize) > $crcEnd )
        {
            $crcLength -= ($currentFileOffset + $chunkSize) - $crcEnd;
        }

        #
        # Calculate running crc, skipping over firmware header section
        #
        for( $byteCounter = 0; $byteCounter < $crcLength; $byteCounter++ )
        {
            $currentOffset = $currentFileOffset + $byteCounter;

            #
            # Treat the skip-area region as zeros
            #
            if( ($currentOffset < $skipStart) ||
                ($currentOffset >= $skipEnd) )
            {
                $byteValue = $chunkBytes[$byteCounter];
                $crc = (($crc>>8) & 0x00FFFFFF) ^ $CRCtable[($crc ^ $byteValue) & 0xFF];
            }
        }

        #
        # The bytes read from the file have been processed - continue
        #
        $currentFileOffset += $chunkSize;
    }

    close CRCF;
    return ($crc ^ 0xFFFFFFFF);
}


#
# Read up the compatibility and backlevel index file
#
if(defined($opt_i)) 
{
    open IDX, "$opt_i" or die "\nAbort: Can't open opt_i $opt_i...\n";
    my ($var, $val);

    while(<IDX>)
    {
        if(!/=/)
        { 
            next; 
        }

        ($var, $val) = split /\s*=\s*/;
        $var =~ s/\s+//g;
        $val =~ s/\s+//g;
        chomp $val;
        
        if($var eq "FW_COMPAT_INDEX")
        {
            $newCompatIdx=$val;
        }

        if($var eq "FW_BACKLEVEL_INDEX")
        {
            $newBlIdx=$val;
        }

        if($var eq "FW_SEQUENCING_INDEX")
        {
            $newSeqIdx=$val;
        }
        if($var eq "FW_MAJOR_RELEASE") 
        {
            if ($val =~ /0x/i)
            {
                $newMajRel = oct $val;
            }
            else
            {
                $newMajRel = $val;
            }
        }
        if($var eq "FW_MINOR_RELEASE") 
        {
            if ($val =~ /0x/i)
            {
                $newMinRel = oct $val;
            }
            else
            {
                $newMinRel = $val;
            }
        }
    }

    close IDX;
}

while (@ARGV)
{
    my $returnCode = GOOD;
    $parm = shift @ARGV;
    @files = glob($parm);

    while (@files)
    {
        my $headerOffset = 0;
        my $buffer = 0;

        $fwbin = shift @files;
        $fwtmp = "$fwbin".".tmp";

        $newSysRel = substr($opt_s . "    ", 0, 4) if defined($opt_s);
        $newVersion = substr($opt_v . "    ", 0, 4) if defined($opt_v);

        #
        # Read & unpack the the firmware header
        #
        open FWBIN, "+<$fwbin" or die "\nAbort: Can't open $fwbin...\n";
        binmode FWBIN;

        #
        # Detect the image file type (i960 COFF or x86 ELF)
        #
	    read FWBIN, $buffer, 4;
        $buffer =~ s/[^A-Z]//g;

        if( $buffer eq "ELF" )
        {
            #
            # x86 ELF files have the firmware header located at .fw.header section
            #
            print "x86 ELF file found\n";

            #
            # This is where the objdump work will go - locate the section
            #
            ($returnCode, $headerOffset) = ELF_GetFirmwareHeaderOffset( $fwbin );

            if( $returnCode != GOOD )
            {
                print( "ERROR: Could not find firmware header offset\n" );
                exit( ERROR );
            }
        }
        else
        {
            #
            # i960 COFF files have the firmware header at the beginning
            #
            print "i960 COFF file found\n";
            $headerOffset = 0;
        }

        #
        # Read the firmware header from the image file
        #
        seek FWBIN, $headerOffset, 0 or die "\nAbort: Can't seek in $fwbin...\n";;
        $buffer = "";
        $readCount = 0;

        while($readCount < $headerLength)
        {
	        my $chunkSize = read FWBIN, $buffer, $headerLength - $readCount;

	        if ($chunkSize <= 0)
	        {
	            last;
	        }

            $readCount += $chunkSize;
        }

        $template = "
            a32     # misc1
            N       # magicNumber
            a12     # misc2
            A4      # version
            A4      # buildCount
            A4      # buildID
            A4      # sysRelease
            a28     # misc3
            L       # length
            L       # checksum
            C       # compatIndex
            C       # backLevelIndex
            C       # sequencingIndex
            a1      # misc4
            S       # majRel
            S       # minRel
            a16     # misc5
            L";     # hdrCksum
            
        ($misc1, $magicNumber, $misc2, $version, $buildCount, $buildID, $sysRel, 
            $misc3, $fileLength, $bodyChecksum, 
            $compatIdx, $blIdx, $seqIdx, $misc4, $majRel, $minRel, $misc5, $hdrCksum) = 
            unpack $template, $buffer;

        #
        # Verify the magic number is correct, otherwise something is wrong
        #
        if( $magicNumber != 0xDECAFC0F )
        {
            printf( "\n" );
            printf( "ERROR - Firmware header is missing magic number (read 0x%08x)\n", 
                    $magicNumber );
            printf( "  headerOffset: 0x%08x\n", $headerOffset );
            printf( "  length:     %d bytes\n", bytes::length($buffer) );

            for( $printCount = 0; $printCount < $headerLength; $printCount++ )
            {
                my $formatString = sprintf( "A%dC", $printCount );
                my ($discard, $data) = unpack($formatString, $buffer);
                printf( "Buffer[%03d]: 0x%02x\n", $printCount, $data );
            }

            exit( ERROR );
        }

        #
        # Patch the header values.
        #
        print "Patching the firmware header in $fwbin\n";
        $fileLength = -s $fwbin;

        $sysRel    = $newSysRel    if defined($newSysRel);
        $version   = $newVersion   if defined($newVersion);
        $compatIdx = $newCompatIdx if defined($newCompatIdx);
        $blIdx     = $newBlIdx     if defined($newBlIdx);
        $seqIdx    = $newSeqIdx    if defined($newSeqIdx);
        $majRel    = $newMajRel    if defined($newMajRel);
        $minRel    = $newMinRel    if defined($newMinRel);

        #
        # Calculate the crc of the fw body
        #
        $bodyChecksum = crc32( $fwbin, 0, $fileLength, $headerOffset, 
                        $headerOffset + $headerLength );

        #
        # Write only the header out to a temp file
        #
        open FWTMP, ">$fwtmp" or die "\nAbort: Can't open $fwtmp...\n";
        binmode FWTMP;
        print FWTMP pack $template, $misc1, $magicNumber, $misc2, 
                $version, $buildCount, $buildID, $sysRel, $misc3, 
                $fileLength, $bodyChecksum, $compatIdx, $blIdx, $seqIdx, $misc4, 
                $majRel, $minRel, $misc5, $hdrCksum;
        close FWTMP;

        #
        # Calculate the crc of the header
        #
        $hdrCksum = crc32( $fwtmp, 0, $headerLength - 4, 0, 0 );

        #
        # Print out changed fields
        #
        printf "File Length    = %u bytes\n", $fileLength;
        printf "Data CRC       = 0x%08X\n", $bodyChecksum;
        printf "Hdr CRC        = 0x%08X\n", $hdrCksum;
        print  "Version        = $newVersion\n" if defined($newVersion);
        print  "Sys Release    = $newSysRel\n" if defined($newSysRel);
        print  "Compat Idx     = $newCompatIdx\n" if defined($newCompatIdx);
        print  "Backlevel Idx  = $newBlIdx\n" if defined($newBlIdx);
        print  "Sequencing Idx = $newSeqIdx\n" if defined($newSeqIdx);
        printf "Major Release  = %04X\n", $newMajRel if defined($newMajRel);
        printf "Minor Release  = %04X\n", $newMinRel if defined($newMinRel);

        #
        # Overwrite the firmware header in the output file with the new data
        #
        seek FWBIN, $headerOffset, 0 or die "\nAbort: Can't seek in $fwbin...\n";
        print FWBIN pack $template, $misc1, $magicNumber, $misc2, 
                $version, $buildCount, $buildID, $sysRel, $misc3, 
                $fileLength, $bodyChecksum, $compatIdx, $blIdx, $seqIdx, $misc4, 
                $majRel, $minRel, $misc5, $hdrCksum;
        close FWBIN;

        unlink $fwtmp;
    }
}

print "\n";
exit( GOOD );

# vi:sw=4 ts=4 expandtab
