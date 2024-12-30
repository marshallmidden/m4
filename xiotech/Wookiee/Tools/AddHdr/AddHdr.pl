#!/usr/bin/perl -w

#====================================================================
#
# FILE NAME:    AddHdr.pl
#
# AUTHOR:       Randy Rysavy
#
# DATE:         2/1/01
#
# DESCRIPTION:  Adds a Firmware Header to the specified file. This
#               makes the file "email-able", at least in the sense
#               the CCB will not reject it based upon the header
#               CRC's etc.
#
#====================================================================

#
# Set file names
#
($script = $0) =~ s/^.*\\//;
if (@ARGV == 3) {
	($fwbin, $cfgfile, $ver) = @ARGV; $rev = "0000";
}
else {
	if (@ARGV == 4) {

		($fwbin, $cfgfile, $ver, $rev) = @ARGV;
	}
	else {
	 	die "\nUsage: perl $script fw-binary cfg-file version [revcount] ".
        "<- optional\nExample: perl $script MyNewDriveFW.bin DiskDrive.cfg V101\n\n"
	}
}
$fwtmp="$fwbin".".tmp";
$fwtmp2="$fwbin".".tmp2";
($fwout = $fwbin) =~ s/\..*/\.ima/i;


#
# Initialize the header template (structure)
#
$hdr_t = "                  # offset
L8  # the branch + rsvd0        0-7

L   # magicNumber               8
L   # rsvd1                     9
L   # productID                 10
L   # target                    11

A4  # revision                  12
A4  # revCount                  13
A4  # buildID                   14
L   # vendorID                  15

S   # timestamp.year            16
C   # timestamp.month           17
C   # timestamp.date            18
C   # timestamp.day             19
C   # timestamp.hours           20
C   # timestamp.minutes         21
C   # timestamp.seconds         22
L   # rsvd2                     23
L   # burnSequence              24

L   # loadID.emcAddrA           25
L   # loadID.emcAddrB           26
L   # loadID.targAddr           27
L   # loadID.length             28

L   # loadID.checksum           29
L   # loadID.compatibilityID    30
L   # rsvd3[0]                  31
L   # rsvd3[1]                  32

L   # rsvd3[2]                  33
L   # rsvd3[3]                  34
L   # rsvd3[4]                  35
L   # hdrCksum                  36
";

#
# Assign all of the offsets as constants
#
*magicNumber = \8;
*productID   = \10;
*target      = \11;
*revision    = \12;
*revCount    = \13;
*buildID     = \14;
*vendorID    = \15;
*year        = \16;
*month       = \17;
*date        = \18;
*day         = \19;
*hours       = \20;
*minutes     = \21;
*seconds     = \22;
#*burnSequence = \24;
*ccbAddrA    = \25;
*ccbAddrB    = \26;
*targAddr    = \27;
*flength     = \28;
*checksum    = \29;
*compatibilityID = \30;
*hdrCksum    = \36;


#
# Initialize a firmware header
#
@header = (0) x 37;
$hdrlen = 128;

#
# Declare the 'getval' subroutine.
# Input:   The line containing our constant definition
# Returns: Just the constant, as a word value/offset.
#
sub getval {
	my ($name, $value);
	($name, $value) = split("=", $_);
    chomp $value;
	$value =~ s/^\s*"*\s*//;		# strip	beginning and
	$value =~ s/\s*"*\s*$//;		# ending whitespace
    $value = hex $value if /0[xX]/; # if hex number, convert to dec
    return $value;
}

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
# Declare the 'crc32' subroutine.
# Input:   $file, $offset, $length
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
	my ($p, $n, $l, $nread, $start, $nbytes, $file, $crc, $tmpl, $chnk);

    $file = $_[0];
    $start = $_[1];
    $nbytes = $_[2];

    $crc = 0xFFFFFFFF;
    $chnk = 512;  # number of bytes to process at a time.
                  # A number greater than this doesn't appear
                  # to improve performance any.

    open CRCF, "$file" or die "\nAbort: Can't open $file...\n";
    binmode CRCF;

    # create the template
    for($n=0; $n<$chnk; $n++) {
        $tmpl .= "C";
    }

    # first position to the 'offset'
    seek CRCF, $start, 0 or die "\nAbort: Can't seek in $file...\n";

    $nread=0;
    while($nread<$nbytes) {

        # read & unpack a chunk of data
        $n=read CRCF, $p, $chnk;
        if($n==0) {
            die "\nAbort: $file too short...\n";
        }
        $nread+=$n;
        @pp=unpack $tmpl, $p;
        $l=$#pp+1;

        # adjust length to process if we read too much.
        if($nread>$nbytes) {
            $l -= $nread-$nbytes
        }

        # calculate running crc
        for($i=0; $i<$l; $i++) {
            $p=$pp[$i];
            $crc = (($crc>>8) & 0x00FFFFFF) ^ $CRCtable[($crc ^ $p) & 0xFF];
        }
    }
    close CRCF;

    return ($crc ^ 0xFFFFFFFF);
}


#
# Function: Convert short integer to BCD
#
# short CvtToBCD ( short Num )
#
# Input:   The short to convert to BCD
# Returns: The converted short
#
sub CvtToBCD {
	my $Num = $_[0];
    my $BCD = 0;
    my $sh;

    for($sh=0; $sh<16; $sh+=4) {
        $BCD |= ($Num % 10) << $sh;
        $Num /= 10;
    }

	return $BCD;
}


#
# Function: Extract value from cfg file line
#
# Input:   the line where with the name/value pair
# Returns: the value
#
sub ExtractVal {
    my @fields;
    my $value;

    @fields = split /\s+/, $_[0];
    $value = $fields[1];
    $value = hex $value if /0[xX]/; # if hex number, convert to dec

    return $value;
}


###################################################################
#                                                                 #
#  Main execution starts here                                     #
#                                                                 #
###################################################################

#
# Read up the binary that is to be patched. Determine if there is
# already a firmware header tacked on.
#
$fwbinlen = -s $fwbin;
if($fwbinlen>=$hdrlen) {
    open FWBIN, "$fwbin" or die "\nAbort: Can't open $fwbin...\n";
    binmode FWBIN;
    $n=0;
    while($n<$hdrlen) {
        $n+=read FWBIN, $buffer, $hdrlen-$n, $n;
    }
    close FWBIN;
    @tsthdr=unpack $hdr_t, $buffer;

    if($tsthdr[$magicNumber] == 0x0FFCCADE) {
        die "\nAbort: $fwbin appears to already have been patched...\n";
    }
}

#
# Open the config file and pull out the configuration information
#
$header[$productID] = $header[$target] = $header[$ccbAddrA] =
$header[$ccbAddrB] = $header[$targAddr] = $header[$vendorID] =
$header[$compatibilityID] = 0xFFFFFFFF;

open CFG, "$cfgfile" or die "\nAbort: Can't open $cfgfile...\n";
while(<CFG>) {
    if(/^[ \t]*ProductID[ \t]+/) {
        $header[$productID] = ExtractVal $_;
    }

    if(/^[ \t]*TargetID[ \t]+/) {
        $header[$target] = ExtractVal $_;
    }

    if(/^[ \t]*CCBAddrA[ \t]+/) {
        $header[$ccbAddrA] = ExtractVal $_;
    }

    if(/^[ \t]*CCBAddrB[ \t]+/) {
        $header[$ccbAddrB] = ExtractVal $_;
    }

    if(/^[ \t]*TargetAddr[ \t]+/) {
        $header[$targAddr] = ExtractVal $_;
    }

    if(/^[ \t]*VendorID[ \t]+/) {
        $header[$vendorID] = ExtractVal $_;
    }

    if(/^[ \t]*CompatID[ \t]+/) {
        $header[$compatibilityID] = ExtractVal $_;
    }

}
close CFG;

#
# Make sure all values were found in the config file
#
if(($header[$productID] == 0xFFFFFFFF) ||
($header[$target] == 0xFFFFFFFF) ||
($header[$ccbAddrA] == 0xFFFFFFFF) ||
($header[$ccbAddrB] == 0xFFFFFFFF) ||
($header[$targAddr] == 0xFFFFFFFF) ||
($header[$vendorID] == 0xFFFFFFFF) ||
($header[$compatibilityID] == 0xFFFFFFFF)) {
    die "\nAbort: Something wrong with $cfgfile (missing entry?)...\n";
}

#
#  Initialize the individual items in the firmware header
#
$header[$magicNumber] = 0x0FFCCADE;

$header[$revision] = $ver;
$header[$revCount] = $rev;

# Get username of caller
$builder = getlogin;
if($?) {
    $builder="????";
}
$builder .= "    ";
$header[$buildID]=substr $builder, 0, 4;

# Get current time
@ltime = localtime;
$ltime[4]+=1;    # mon  -> 1-12
$ltime[5]+=1900; # year -> 2000-??
$ltime[6]+=1;    # wday -> 1-7

for($i=0; $i<7; $i++) {
    $ltime[$i] = CvtToBCD( $ltime[$i] );
}

$header[$year   ] = $ltime[5];
$header[$month  ] = $ltime[4];
$header[$date   ] = $ltime[3];
$header[$day    ] = $ltime[6];
$header[$hours  ] = $ltime[2];
$header[$minutes] = $ltime[1];
$header[$seconds] = $ltime[0];


#
# Patch the header values.
#
print "\nPatching the firmware header in $fwout:\n";
$fwbinlen += $hdrlen;
$header[$flength]=$fwbinlen;
printf "file length = %u bytes\n", $header[$flength];

# Handle Drive Bay code seperatly - XOR every 2nd byte with 0x80
if($header[$target] == 0x0000010F) {

    # write the XOR'd data out to a temp file
    open FWBIN, "$fwbin" or die "\nAbort: Can't open $fwbin...\n";
    binmode FWBIN;
    open FWTMP2, ">$fwtmp2" or die "\nAbort: Can't open $fwtmp2...\n";
    binmode FWTMP2;

    $first = 0;
    while(read FWBIN, $buffer, 2) {
        @buf2=unpack "CC", $buffer;

        if($first == 0 && ($buf2[0] != 0x53 || $buf2[1] != 0x30)) {
            die "\n\n===> You're drive bay firmware format has changed!\n===> Please talk to Randy Rysavy about what to do now...\n\n";
        }
        $first = 1;

        $buf2[1] ^= 0x80;
        $buffer=pack "CC", @buf2;
        print FWTMP2 $buffer;
    }
    close FWTMP2;
    close FWBIN;

    # calculate the crc of the fw body
    $header[$checksum]=crc32 $fwtmp2, 0, $fwbinlen-$hdrlen;
}
else {
    # calculate the crc of the fw body
    $header[$checksum]=crc32 $fwbin, 0, $fwbinlen-$hdrlen;
}

# write the header out to a temp file
open FWTMP, ">$fwtmp" or die "\nAbort: Can't open $fwtmp...\n";
binmode FWTMP;
print FWTMP pack $hdr_t, @header;
close FWTMP;

# now calculate the crc of the header
$header[$hdrCksum]=crc32 $fwtmp, 0, $hdrlen-4;
printf "hdr crc32   = 0x%08X\n", $header[$hdrCksum];
printf "body crc32  = 0x%08X\n", $header[$checksum];

#
# Write the binary back out
#
if($header[$target] == 0x0000010F) {
    open FWBIN, "$fwtmp2" or die "\nAbort: Can't open $fwtmp2...\n";
}
else {
    open FWBIN, "$fwbin" or die "\nAbort: Can't open $fwbin...\n";
}
binmode FWBIN;

open FWOUT, ">$fwout" or die "\nAbort: Can't open $fwout...\n";
binmode FWOUT;
print FWOUT pack $hdr_t, @header;

while(read FWBIN, $buffer, 0x10000) {
    print FWOUT $buffer;
}

close FWBIN;
close FWOUT;

unlink $fwtmp;
if(-r $fwtmp2) {
    unlink $fwtmp2;
}


