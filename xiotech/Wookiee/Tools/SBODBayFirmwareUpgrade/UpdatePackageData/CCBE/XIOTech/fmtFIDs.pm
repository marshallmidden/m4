#! /usr/bin/perl
# $Header$
##############################################################################
#
#   CCBE Integration test library - format routines for FIDs
#
#   5/9/2003  XIOtech   Craig Menning
#
#   A set of library functions for formatting the FIDs that are extracted
#   fromt the controller for debug.
#
#   It is expected that the user will write a perl script that calls
#   these.
#
#   Copyright 2002-2003 XIOtech
#
#   For XIOtech internal use only.
#
##############################################################################
=head1 NAME

TestLibs::fmtFIDs

$Id: fmtFIDs.pm 29886 2007-08-20 19:48:28Z m4 $

=head1 SUPPORTED PLATFORMS

=begin html

 <UL>
     <LI>Linux</LI>
     <LI>Windows</LI>
 </UL>

=end html

=head1 SYNOPSIS

A buffer is passed to this module and the functions in this module decode
it and format it to ASCII strings. These strings are then returned to the
caller for printing.

Entry points will be provided so that the user may call individual
decoder functions that take the buffer to an ASCII string only. This will
facilitate sharing of code among groups.

=head1 DESCRIPTION

Test Functions Available (exported)

        The more significant ones

                       CCBEDecodeFids
                       FmtCCBHeapStatsFID
                       FmtCCBHeapStatsFID2
                       FmtCCBPCBDumpFID
                       FmtCCProfileFID
                       FmtCCBTraceFID
                       FmtCCBNVR1FID

                       FmtDiskBays
                       FmtExecQueFID
                       FmtFCALCounters
                       FmtFWHeadersFID
                       FmtFCMCountersFID

                       FmtLogFid
                       FmtPdisksFID
                       FmtRaidsFID
                       FmtServersFID

                       FmtStatsCacheFID
                       FmtStatsEnvFID
                       FmtStatsI2CFID
                       FmtStatsLoopFID
                       FmtStatsLoopFIDNC
                       FmtStatsPciFID
                       FmtStatsProcFID
                       FmtStatsServerFID
                       FmtStatsVdiskFID

                       FmtTargetFID

                       FmtVdisksFID
                       FmtiSCSIStatsFID

        The less significant ones

=head1 PARAMETERS

Most functions have the same parameters. All functions will return text data in
the supplied pointer. Some also return the parsed data in a hash. The
output pointers are not required and may be set to 0. Here are the
input parameters for the functions

    FnName( $destPtr, $hashPtr, $bufferPtr, $offset, $reqLength, $processor, $address);

    Input:    $destPtr - pointer to a scalar for the ascii results ( may be 0 )
              $hashPtr - pointer to return a hash with the data ( may be 0 )
              $bufferPtr - pointer to a buffer scalar with the data
              $offset - offset into the buffer to the start of the data
                        (may be zero )

              The following may be optional. This varies by function.

              $reqLength - The number of bytes to process. There must be
                           this many bytes in the buffer and it should be
                           a multiple of 16 bytes. Set to 0 if not used.
              $processor - text string with the processor or another title.
                           Is used by some functions to alter the nature of
                           the decode. If use the string is scanned for
                           'BE', 'FE', or 'CCB', as needed. Set to "" if
                           not used.
              $address - Memory address where the data came from. Set to
                         0 if not used.


    Output:   String data is appended to the scalar referenced by $destPtr.

              Hash data replaces the item referenced by $hashPtr. The hash
              data is a 'work in progess' and may not be complete.

              Functions return GOOD.


=cut


#
# - what I am
#

package XIOTech::fmtFIDs;

#
# - other modules used
#

use warnings;
use lib "../CCBE";


use XIOTech::cmdMgr;
use XIOTech::cmPDisk;
use XIOTech::cmUtils;
use XIOTech::cmStats;
use XIOTech::constants;
use XIOTech::errorCodes;
use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::logMgr;

#use TestLibs::Logging;
#####################################use TestLibs::Constants;
use constant GOOD => 0;
use constant ERROR => 1;

#####################################use TestLibs::utility;
use XIOTech::decodeSupport;
use XIOTech::decodeRings;

use XIOTech::TracDec;


#
# - perl compiler/interpreter flags 'n' things
#

use Cwd;
use strict;

#
# - Constants used
#

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    #$VERSION = do { my @r = (q$Revision: 29886 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker

                        #




    @ISA         = qw(Exporter);
    @EXPORT      = qw(


                       &CCBEDecodeFids

                       &DumpNVR2

                       &FmtCCBHeapStatsFID
                       &FmtCCBHeapStatsFID2
                       &FmtCCBPCBDumpFID
                       &FmtCCProfileFID
                       &FmtCCBTraceFID
                       &FmtCCBNVR1FID
                       &FmtCCBNVR1FIDNew
                       &FmtCCBNVR1FlashCopies

                       &FmtDiskBays
                       &FmtExecQueFID
                       &FmtFCALCounters
                       &FmtFWHeadersFID
                       &FmtFWHeadersFIDSummary
                       &FmtFCMCountersFID

                       &FmtLogFid
                       &FmtMirrorPartnerList
                       &FmtPdisksFID
                       &FmtRaidsFID
                       &FmtServersFID

                       &FmtStatsCacheFID
                       &FmtStatsEnvFID
                       &FmtStatsI2CFID
                       &FmtStatsLoopFID
                       &FmtStatsLoopFIDNC
                       &FmtStatsPciFID
                       &FmtStatsProcFID
                       &FmtStatsServerFID
                       &FmtStatsVdiskFID

                       &FmtTargetFID
                       &FmtBEDevicePaths
                       &FmtVdisksFID
                       &FmtiSCSIStatsFID

                      );
    #%EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],

    # your exported package globals go here,
    # as well as any optionally exported functions
    #@EXPORT_OK   = qw($Var1 %Hashit &func3);

  #  TestLibs::Logging::logVersion(__PACKAGE__, q$Revision: 29886 $);
  #  TestLibs::Logging::logVersion(__PACKAGE__, q$Name$);
}
    our @EXPORT_OK;


##############################################################################
#
#               Public Functions
#
##############################################################################

##############################################################################
sub CCBEDecodeFids
{
    my ($buffer, $fid, $filen) = @_;

    my $ret;
    my $msg = "";
    my $fidStr = "FID " . $fid ." ";
    my $fidName = "";
    my $version = 0;
    my $startAddr = 0;

    #
    # Look for a FID header if it exists.  If it does, use the meta data
    # within to help select the proper decoder.
    #

    # Don't look for the FID header in the Linux files
    if ( !(($fid >= 0x100) && ($fid < 0x200) &&
           (($fid & 0xFF) >= 0x31) &&
           (($fid & 0xFF) <= 0x60)) )
    {
        # Don't bother looking if we don't have at least the length of the
        # header (32 bytes).
        if (length($buffer) >= 32)
        {
            my ($hdrMagicNum, $hdrFid, $hdrVer, $hdrStartAddr, $hdrId) = unpack
            DDR_FID_HEADER, $buffer;

            if ($hdrMagicNum == DDR_FID_HEADER_MAGIC_NUM)
            {
                $buffer = substr($buffer, 32);

                if ($fid != $hdrFid)
                {
                    print "Specified FID ($fid) does not match FID in header ($hdrFid). ".
                    "Using $hdrFid!\n";
                    $fid = $hdrFid;
                    $fidStr = "FID " . $fid ." ";
                }

                $fidName = $hdrId;
                $version = $hdrVer;
                $startAddr = $hdrStartAddr;
            }
        }
    }

    print "Decoding FID $fid v$version $fidName\n\n";

    # this function maps a fid number to the appropriate decode routine
    # All routines either return a string or fill a string reference.
    # $msg is that string.


    if  (
            $fid eq 2          # NVRAM part 2
        )
    {
#        print ( $fidStr );

        $msg = DumpNVR2( $buffer, 0 );

    }
    elsif (
            $fid eq 256
          )
    {
        $ret = FmtCCBTraceFID(  \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );

    }
    elsif (  $fid eq 257  )
    {
        $msg = "Begin Decode. Note: this is a circular buffer. START and END are in the middle.";

        $msg .= "Here are some pointers to help.\n\n";

        $msg .= FmtDataString( \$buffer, $startAddr, "word", 24, 0);

        $msg .= "\nThe Serial data....\n\n";

        my $lBuffer = substr($buffer, 24);

        $msg .= FmtDataString( \$lBuffer, $startAddr, "text", length($lBuffer), 0);

    }
    elsif (
            $fid eq 258
          )
    {
        # Call the new decoder, it will call the old decoder if necessary.
        $ret = FmtCCBHeapStatsFID2(  \$msg, 0, \$buffer, 0, 0, $fidStr,
            $startAddr, 1 );

    }
    elsif (
            $fid eq 259
          )
    {
        $ret = FmtCCProfileFID(  \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr  );

    }
    elsif (
            $fid eq 260
          )
    {
        $ret = FmtCCBPCBDumpFID(  \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr  );

    }
    elsif (
            $fid eq        264  ||
            $fid eq        268
           )
    {
        $ret = FmtBEDevicePaths( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr  );

    }
    elsif (
            $fid eq        263
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache DiskBayMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 8, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        265
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache PDiskMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 128, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        266
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache PDiskFailMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 128, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        267
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache PDiskRebuildMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 128, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        269
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache VDiskMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 64, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        270
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache VDiskCopyMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 64, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        271
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache VDiskMirrorMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 64, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        272
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache RAIDMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 128, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        273
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache ServerMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 32, $fidStr, $startAddr );
    }
    elsif (
            $fid eq        274
          )
    {
        # $reqLength is the number of bytes in the map
        $msg = "\nCache TargetMap: ";
        FmtBitmap( \$msg, 0, \$buffer, 0, 8, $fidStr, $startAddr );
    }

    elsif (
            $fid eq 275
            #||
            #$fid eq PDISKFID
          )
    {
        $ret = FmtDiskBays( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }

    elsif (
            $fid eq 276           ||
            $fid eq "Other Target"
          )
    {
        $ret = FmtTargetFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (
            $fid eq 277      ||
            $fid eq 278
          )
    {
        $ret = FmtStatsLoopFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif ( $fid eq 279  )
    {
        $ret = FmtPdisksFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 280  )
    {
        $ret = FmtVdisksFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 281  )
    {
        $ret = FmtRaidsFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (   $fid eq 282  )
    {
        $ret = FmtServersFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 283  )
    {
        $ret = FmtStatsI2CFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 284  )
    {
        $ret = FmtStatsProcFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 285  )
    {
        $ret = FmtStatsPciFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 286  )
    {
        if ( $version == 1 )
        {
            my %hdata = XIOTech::cmdMgr::_envIIdataFmt(0, $buffer);
            $msg = "\nEnvironmental Statistics:\n";
            $msg .= XIOTech::cmdMgr::envIIDisplay(0, %hdata);
            $ret = GOOD;
        }
        else
        {
            $ret = FmtStatsEnvFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );
        }

    }
    elsif (   $fid eq 287  )
    {
        $ret = FmtStatsServerFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 288  )
    {
        $ret = FmtStatsVdiskFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 289  )
    {
        $ret = FmtStatsCacheFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 290  )
    {
        $ret = FmtStatsLoopFIDNC( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 291  )
    {
        $ret = FmtFCALCounters( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (
            $fid eq 293          # CCB NVRAM
          )
    {
#        print ( $fidStr );

        $ret = FmtCCBNVR1FIDNew( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 296  )
    {
        $ret = FmtFWHeadersFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (
            $fid eq 298          # CCB NVRAM
          )
    {
#        print ( $fidStr );

        $ret = FmtCCBNVR1FlashCopies( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 299  )
    {
        # This is a hack so that CCBE won't blow up if this FID is
        # not implemented
        if (length($buffer) < 100)
        {
            $msg .= FmtDataString( \$buffer, $startAddr, "word", length($buffer), 0);
        }
        else
        {
            $ret = FmtFCMCountersFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );
        }
    }
    elsif (  $fid eq 300  )
    {
        $ret = FmtVCGInfoFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 301  )
    {
        $ret = FmtTargetResListFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 302  )
    {
        $ret = FmtFIDListFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 303  )
    {
        $ret = FmtCCBStats( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (  $fid eq 304  )
    {
        $ret = FmtMirrorPartnerList( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    # Linux FileRead rsvd fids
    elsif (  ($fid >= 305) && ($fid <= 352)  )
    {
        # Since we pass the filename in, return directly from the decoder.
        return FmtLinuxFileReads( \$msg, 0, \$buffer, 0, 0, $fidStr,
            $startAddr, $fid, $filen );
    }
    elsif (  $fid eq 353  )
    {
        $ret = FmtSESData( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );
    }
    elsif (  $fid eq 563 ||
             $fid eq 819 )
    {
        $ret = FmtFICB( \$msg, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (
            $fid eq 512 ||
            $fid eq 768
          )
    {
#        print ( $fidStr );

        $msg = FmtProcData( 0, $fidStr, $buffer, $startAddr,  "fltRec" );

    }
    elsif (
            $fid eq 514 ||
            $fid eq 770
          )
    {
#        print ( $fidStr );

        $msg = FmtProcData( 0, $fidStr, $buffer, $startAddr,  "mrpTrace" );

    }
    elsif (
            $fid eq 520      ||
            $fid eq 776
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtKii( \$msg,  \$buffer, 0, length($buffer), $fidStr, $startAddr );

    }
    elsif (
            $fid eq 521      ||
            $fid eq 522      ||
            $fid eq 523      ||
            $fid eq 524
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtItrace( \$msg,  \$buffer, 0, length($buffer), $fidStr, $startAddr );

    }
    elsif (
            $fid eq 525      ||
            $fid eq 526      ||
            $fid eq 527      ||
            $fid eq 528
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtTrace( \$msg,  \$buffer, 0, length($buffer), $fidStr, $startAddr );

    }
    elsif (
            $fid eq 529     ||
            $fid eq 530     ||
            $fid eq 531     ||
            $fid eq 532     ||
            $fid eq 533     ||
            $fid eq 534     ||
            $fid eq 535     ||
            $fid eq 536     ||
            $fid eq 546     ||
            $fid eq 548     ||
            $fid eq 560     ||
            $fid eq 785     ||
            $fid eq 786     ||
            $fid eq 787     ||
            $fid eq 788     ||
            $fid eq 789     ||
            $fid eq 790     ||
            $fid eq 791     ||
            $fid eq 792     ||
            $fid eq 802     ||
            $fid eq 804     ||
            $fid eq 816
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtExecQueFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );

    }
    elsif (
            $fid eq 537  ||
            $fid eq 538  ||
            $fid eq 539  ||
            $fid eq 540  ||
            $fid eq 541  ||
            $fid eq 542  ||
            $fid eq 543  ||
            $fid eq 544  ||
            $fid eq 793  ||
            $fid eq 794  ||
            $fid eq 795  ||
            $fid eq 796  ||
            $fid eq 797  ||
            $fid eq 798  ||
            $fid eq 799  ||
            $fid eq 800
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtIspRspQ( \$msg,  \$buffer, 0, length($buffer), $fidStr, $startAddr );

    }
    elsif (
            $fid eq 545 ||         # NVRAM part 5
            $fid eq 801
          )
    {
#        print ( $fidStr );

        DiagProc( $buffer, 0, \$msg, $version );

    }

    elsif (
            $fid eq 550      ||
            $fid eq 806
           )
    {
        $fidStr = "FID " . $fid ." FE IRAM";
        $ret = FmtIRAM( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );

    }

    elsif (
            $fid eq 551      ||
            $fid eq 807
           )
    {
        $fidStr = "FID " . $fid ." BE IRAM";
        $ret = FmtIRAM( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );
    }

    elsif (
            $fid eq    556    ||
            $fid eq    803    ||
            $fid eq    805    ||
            $fid eq    808    ||
            $fid eq    809    ||
            $fid eq    810    ||
            $fid eq    811    ||
            $fid eq    812    ||
            $fid eq    813    ||
            $fid eq    814    ||
            $fid eq    815    ||
            $fid eq    817
           )
    {
        $msg = "\n$fid:\n";

        $ret = FmtPcb( \$msg,  \$buffer, 0, length($buffer), $fidStr, $startAddr );

    }
# ISCSI_CODE
    elsif (  $fid eq 354 )
    {
        $ret = FmtiSCSIStatsFID( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr );
    }
# ISCSI_CODE

    elsif (  $fid eq 355  )
    {
        $ret = FmtAsyncData( \$msg, 0, \$buffer, 0, 0, $fidStr, $startAddr, $version );
    }


    elsif (
            $fid eq 562 ||         # NVRAM part 1
            $fid eq 818
          )
    {
#        print ( $fidStr );

        Part1Decode( \$buffer, 0, \$msg);

    }
    else
    {
        # all others has hex
        $msg .= FmtDataString( \$buffer, $startAddr, "word", length($buffer), 0);

    }

    # open the output file if one was requested

    my $fh = *STDOUT;

    if (defined $filen)
    {
        $fh = *FH;
#        open $fh, ">$filen" or $fh = *STDOUT;
        open $fh, ">$filen" or return ERROR;
    }

    print $fh $msg;


    if($fh ne *STDOUT)
    {
        close $fh;
    }


    return GOOD;

}
##############################################################################




##############################################################################
=head2 FmtDiskBays() function

Formats disk bays information from FID 275. Output is similar to the
DISKBAYINFO ccbcl command. Returns GOOD upon completion. The data is a
concatenation of physical device records. Hash data is not complete.

=cut
##############################################################################


sub FmtDiskBays
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;

    ##########################################
    # Disk bays
    ##########################################

    # log what we are decoding
    $msg = "\nDisk Bays:\n$processor Information. \n";

    $len = length $$bufferPtr;    # get length of structure


    # walk the buffer, do each physical device record

    while ( $offset < $len - 4 )
    {

        # the FID is a concatenation of physical device records, so we can call
        # the FmtPhysDevice() function. Since the number of records appears to be
        # unknown, we peek into each record to see if there is a length field.

        $fmt = sprintf("x%d L L ",$offset);      # generate the format string
        ($item1, $item2 ) =  unpack $fmt , $$bufferPtr;

        # the field length is in item2.

        # if we have a length, assume the rest of the structure is there
        # and process it

        if ( $item2 > 0 )
        {

            $msg .= "\nDevice index $idx \n";
            $ret = FmtPhysDevice( \$msg, \%info, $bufferPtr, $offset, 0, " ", 0 );
            $offset += $item2;      # move to next one
            $idx++;
        }
        else
        {
            # a bad length says we are done
            last;
        }

    }


    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;

}




##############################################################################
=head2 FmtTargetFID() function

Formats disk bays information from FID 276. Returns GOOD upon completion. Hash
data is not complete.

=cut
##############################################################################
##############################################################################
sub FmtTargetFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;

    ##########################################
    # Target FID
    ##########################################

    $msg = "\nTarget records:\n$processor Information \n";

    $len = length $$bufferPtr;    # get length of structure

    while ( $offset < $len - 4 )
    {

        # the FID is a concatenation of target records, so we can call
        # the FmtTarget() function. Since the number of records appears to be
        # unknown, we peek into each record to see if there is a length field.

        $fmt = sprintf("x%d L L ",$offset);      # generate the format string
        ($item1, $item2 ) =  unpack $fmt , $$bufferPtr;

        # the field length is in item2.

        if ( $item2 > 0 )
        {

            $msg .= "\nTarget index $idx \n";
            $ret = FmtTargetStruct( \$msg, \%info, $bufferPtr, $offset, 0, " ", 0 );
            $offset += $item2;      # move to next one
            $idx++;
        }
        else
        {
            last;
        }

    }


    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;

}


##############################################################################
##############################################################################
=head2 FmtExecQueFID() function

Formats Executive queue information from multiple FIDs. Returns GOOD upon completion.
Hash data is returned.

This supports FIDs 0x?11 0x?12 0x?13 0x?14 0x?15 0x?16 0x?17 0x?18.
                   (529-536, 785-792)

Hash data is returned.

=cut
##############################################################################
##############################################################################
sub FmtExecQueFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;

    ##########################################
    # Exec Queue FID
    ##########################################


    $msg = "\nExec Queue:\n$processor Information \n\n";

    $len = length $$bufferPtr;    # get length of structure


    #typedef struct qu_t
    #{
    #    struct ilt_t *      qu_head;
    #    struct ilt_t *      qu_tail;
    #    UINT32              qu_qcnt;
    #    struct pcb_t *      qu_pcb;
    #};

    $fmt = sprintf("x%d LL LL", $offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" qu_head:        0x%08x    \n", $item1);
    $msg .= sprintf(" qu_tail:        0x%08x    \n", $item2);
    $msg .= sprintf(" qu_qcnt:        0x%08x    \n", $item3);
    $msg .= sprintf(" qu_pcb:         0x%08x    \n\n", $item4);

    $info{QU_HEAD}  = $item1;
    $info{QU_TAIL}  = $item2;
    $info{QU_QCNT}  = $item3;
    $info{QU_PCB}   = $item4;

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }


    return GOOD;

}


##############################################################################
##############################################################################
=head2 FmtFIDListFID() function

Formats list of FIDs FID (FID 302). Returns GOOD upon completion.
Hash data is NOT returned.


=cut
##############################################################################
##############################################################################
sub FmtFIDListFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $fidCount;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $feTypeCount = 0;
    my $beTypeCount = 0;


    $len = length $$bufferPtr;    # get length of structure


    #   typedef struct FID_LIST
    #   {
    #       UINT16  count;          # of FIDs in fidList[]
    #       UINT16  fidList[0];     # Array of FID numbers.
    #   } FID_LIST;
    #
    $fmt = sprintf("x%d S", $offset);      # generate the format string
    ($fidCount) = unpack $fmt , $$bufferPtr;
    $offset += 2;

    $msg .= sprintf("\nFID List contains %d FIDs\n", $fidCount);
    $msg .= sprintf("---CCB FIDs---");

    for (my $i = 0; $i < $fidCount; $i++)
    {
        $fmt = sprintf("x%d S", $offset);      # generate the format string
        ($item1) = unpack $fmt , $$bufferPtr;
        $offset += 2;

        # Group the FIDs in the display to match the layout in ddr.c.
        # This makes it easier to see any differences.
        if (($item1 < 512) && (($i % 10) == 0))
        {
            $msg .= "\n";
        }
        # Break at the next FID type
        elsif ($item1 == 512)
        {
            $msg .= sprintf("\n---Front End FIDs---\n");
            $feTypeCount++;
        }
        elsif (($item1 > 512) && ($item1 < 768))
        {
            if (($feTypeCount % 10) == 0)
            {
                $msg .= "\n";
            }

            $feTypeCount++;
        }
        # Break at the next FID type
        elsif ($item1 == 768)
        {
            $msg .= sprintf("\n---Back End FIDs---\n");
            $beTypeCount++;
        }
        elsif ($item1 > 768)
        {
            if (($beTypeCount % 10) == 0)
            {
                $msg .= "\n";
            }

            $beTypeCount++;
        }

        $msg .= sprintf("%3d  ", $item1);
    }


    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}


##############################################################################
##############################################################################
=head2 FmtStatsEnvFID() function

Formats Environmental stats information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 286.

=cut
##############################################################################
##############################################################################
sub FmtStatsEnvFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;

    $msg = "\nEnvironmental Statistics:\n";

    $len = length($$bufferPtr);

    if ( $len > 2680 )
    {
        # decode for R2 and beyond

        # we have to build a packet to use the CCBCL.PL decoders. This packet
        # consists of a hash with a 'header' and 'data' key. Note, in some cases
        # the key will be 'data' in others 'DATA'. The content we are using
        # are mostly ignored, so a set of data from a typical header is user.
        # the important items are the COMMAND item, The Length of the header
        # (128) and the length of the data packet. Wherever we use this
        # method, these three fields must be correct.The header is always 128
        # bytes. Many of the 0 fields have been defaulted.

        $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                        128, length($$bufferPtr), 0, 1,
                        PI_STATS_ENVIRONMENTAL_CMD, 45657, 0, 1053011891,
                        '   ', 0, 0, '        ',
                        0, 0, 0, '        ',
                        '                                ',
                        '-¶|a+—MxT+Q4?Á?F',
                        '+¶¶+i^S—?ÈV‹=d•  ');

        # now create the hash to call the CCBCl functions

        $rp{'header'} = $header;
        $rp{'data'} = $$bufferPtr;

        # the first call parses the two-item hash into the info hash
        # that is displayed. The info hash has a separate key for each
        # data item. We can alos return the infor hash in %hPtr if
        # desired.

        %info = XIOTech::cmdMgr::_envStatsExtPacket(0, 0, \%rp);
        $msg .= XIOTech::cmdMgr::displayEnvironmentalStatsExtended(0, %info);

        # if the CL function returns a string, then we pront the string. In
        # orer to get the string, we must have modified the CL function to
        # fill a string, rather than just print the data as most fcns
        # currently do.

        # Now we can copy the output back to where the pointers say.
        # Copy the data to the callers pointers

        if ( $destPtr )
        {
            $$destPtr .= $msg;         # append to callers (string) item
        }

        if ( $hPtr )
        {
            %$hPtr = %info;            # hash will overwrite
        }
    }
    else
    {
        # decode for R1  (currently same except for fcn calls)

        # we have to build a packet to use the CCBCL.PL decoders. This packet
        # consists of a hash with a 'header' and 'data' key. Note, in some cases
        # the key will be 'data' in others 'DATA'. The content we are using
        # are mostly ignored, so a set of data from a typical header is user.
        # the important items are the COMMAND item, The Length of the header
        # (128) and the length of the data packet. Wherever we use this
        # method, these three fields must be correct.The header is always 128
        # bytes. Many of the 0 fields have been defaulted.

        $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                        128, length($$bufferPtr), 0, 1,
                        PI_STATS_ENVIRONMENTAL_CMD, 45657, 0, 1053011891,
                        '   ', 0, 0, '        ',
                        0, 0, 0, '        ',
                        '                                ',
                        '-¶|a+—MxT+Q4?Á?F',
                        '+¶¶+i^S—?ÈV‹=d•  ');

        # now create the hash to call the CCBCl functions

        $rp{'header'} = $header;
        $rp{'data'} = $$bufferPtr;

        # the first call parses the two-item hash into the info hash
        # that is displayed. The info hash has a separate key for each
        # data item. We can alos return the infor hash in %hPtr if
        # desired.

        %info = R1_envStatsExtPacket(0, 0, \%rp);
        $msg .= R1displayEnvironmentalStatsExtended(0, %info);

        # if the CL function returns a string, then we pront the string. In
        # orer to get the string, we must have modified the CL function to
        # fill a string, rather than just print the data as most fcns
        # currently do.

        # Now we can copy the output back to where the pointers say.
        # Copy the data to the callers pointers

        if ( $destPtr )
        {
            $$destPtr .= $msg;         # append to callers (string) item
        }

        if ( $hPtr )
        {
            %$hPtr = %info;            # hash will overwrite
        }
    }

    # just return GOOD

    return GOOD;
}
##############################################################################
##############################################################################
=head2 FmtPdisksFID() function

Formats Pdisk information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. Its output represents
three of the four modes of the PDISKS command in the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 279.

=cut
##############################################################################
##############################################################################
sub FmtPdisksFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nPDisks:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    # 4 is added to the length to account for the 'count' we need to add.

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    PI_PDISKS_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    # For some FIDs, the packet is missing a count of items that the
    # call to the CL needs to have. In those cases, we must walk the
    # structure to generate this count.


    # need to walk the buffer to count the number of pdisks present

    $disks = 0;
    for ( $i = 4; $i < $length; $i += 216 )
    {
        $fmt = sprintf("x%d L",$i);
        ($j) = unpack($fmt, $$bufferPtr );
        if ( $j == 0xd8 ) { $disks++; }
    }

    $length = pack("L", $disks);

    # print " Counted $disks pdisks in the data \n";

    # Now when we build the data key, we put the generated count at the
    # beginning so that the CL can properly digest the packet.

    $rp{'data'} = $length . $$bufferPtr;


    %info = XIOTech::cmdMgr::_physicalDisksPacket(0, 0, \%rp);

    # this hash can be printed three ways in the CL, So, we do all three
    # methods here. There is also a fourth method, but the hash does not
    # contain the data for that method.

    $msg .= "\n-----------------------------  STD ----------------------------------\n";
    $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "STD", %info);

    #   print "-----------------------------  LOOP ----------------------------------\n";
    #   $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "LOOP", %info);

    $msg .= "\n-----------------------------  FWV ----------------------------------\n";
    $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "FWV", %info);

    $msg .= "\n-----------------------------  SES ----------------------------------\n";
    $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "SES", %info);

    $msg .= "\n-----------------------------  CMPL ----------------------------------\n";
    $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "CMPL", %info);

    # we would fill the output variables here.

    # Now we can copy the output back to where the pointers say.
    # Copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers (string) item
    }

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }


    return GOOD;
}


##############################################################################
##############################################################################
=head2 FmtVdisksFID() function

Formats Vdisk information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 280.

=cut
##############################################################################
##############################################################################
sub FmtVdisksFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nVDisks:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    PI_VDISKS_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # Need to walk the buffer to count the number of vdisks present.
    # Unfortunately, we do not have a vdisk count, and the data we are
    # presented with is likely much greater than the vdisk population,
    # so we traverse through and guess when we are at the end.
    $disks = 0;
    for ( $i = 0; $i < $length; $i += $j )
    {
        my $rcnt;
        $fmt = sprintf("x%d x4 L x7 C",$i); # extract the length and raid count
        ($j, $rcnt) = unpack($fmt, $$bufferPtr);
        if ($j != 0x78 + (2 * $rcnt))   # 0x78 is the length of VDD excluding raid list
        {
            last;
        }
        $disks++;
    }

    $length = pack("L", $disks);

    # print " Counted $disks vdisks in the data \n";


    $rp{'data'} = $length . $$bufferPtr;


    %info = XIOTech::cmdMgr::_virtualDisksPacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayVirtualDisks(0, "STD",  %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}


##############################################################################
##############################################################################
=head2 FmtRaidsFID() function

Formats Raids information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 281.

=cut
##############################################################################
##############################################################################
sub FmtRaidsFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nRaids:\n";;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    PI_RAIDS_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # need to walk the buffer to count the number of pdisks present

    $disks = 0;
    for ( $i = 4; $i < $length; $i += $j )
    {
        $fmt = sprintf("x%d L",$i);
        ($j) = unpack($fmt, $$bufferPtr );
        if ( $j != 0 ) { $disks++; }
        if ( $j == 0 ) { last; }
    }

    $length = pack("L", $disks);

    print " Counted $disks raids in the data \n";


    $rp{'data'} = $length . $$bufferPtr;


    %info = XIOTech::cmdMgr::_raidsPacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayRaids(0,  %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtServersFID() function

Formats Servers information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 282.

=cut
##############################################################################
##############################################################################
sub FmtServersFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nServers\n";;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    PI_SERVERS_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # need to walk the buffer to count the number of pdisks present

    $disks = 0;
    for ( $i = 4; $i < $length; $i += $j )
    {
        $fmt = sprintf("x%d L",$i);
        ($j) = unpack($fmt, $$bufferPtr );
        if ( $j != 0 ) { $disks++; }
        if ( $j == 0 ) { last; }
    }

    $length = pack("L", $disks);

    print " Counted $disks servers in the data \n";


    $rp{'data'} = $length . $$bufferPtr;


    %info = XIOTech::cmdMgr::_serversPacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayServers(0,  %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }


    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsServerFID() function

Formats Server statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSSERVER on each server using the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 287.

=cut
##############################################################################
##############################################################################
sub FmtStatsServerFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStatsServer:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;
    my $count;
    my $server;
    my $offset1;
    my $offset2;
    my $packet;

    ( $count) = unpack("S", $$bufferPtr);
    $offset1 = 2;

    $offset2 = 2 + (2 * $count);


    # this FID actually contains the equvalent of multiple calls to the CL.
    # Since the CL only processes one server in a cll, we need to break
    # up the packet and call the CL multiple times. Note that we build the
    # header each time as the length of a server data block may vary.

    for ( $i = 0; $i < $count; $i++ )
    {
        $fmt = sprintf("x%d S", (2 + (2 * $i)) );      # generate the format string

        ( $server) = unpack($fmt, $$bufferPtr);

        $msg .= "Statistics for server $server. \n";

        $fmt = sprintf("x%d S", ($offset2 + 4 ));      # generate the format string
        ( $length ) = unpack($fmt, $$bufferPtr);

        if ( $length )
        {
            # print " Packet start: $offset2  length: $length \n";

            $packet = substr( $$bufferPtr, $offset2, $length);

            $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                            128, ( $length ), 0, 1,
                            PI_STATS_SERVER_CMD, 45657, 0, 1053011891,
                            '   ', 0, 0, '        ',
                            0, 0, 0, '        ',
                            '                                ',
                            '-¶|a+—MxT+Q4?Á?F',
                            '+¶¶+i^S—?ÈV‹=d•  ');

            $rp{'header'} = $header;

            $rp{'data'} = $packet;


            %info = XIOTech::cmdMgr::_statsServer(0, 0, \%rp);

            $msg .= XIOTech::cmdMgr::statsServerDisplay(0,  %info);

            $offset2 += $length;
        }
        else
        {
            $msg .= "      Server not present/no data available. \n";
            $offset2 += 0x84;      # assumed length
        }


     }

    $msg .= "\n";

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # no hash pointer here.

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsVdiskFID() function

Formats Vdisk statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSVDISK using the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 288.

=cut
##############################################################################
##############################################################################
sub FmtStatsVdiskFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStatsVdisk:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_STATS_VDISK_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # need to walk the buffer to count the number of pdisks present


    print " Stats Vdisk handler \n";


    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_statsVDisk(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsVDiskDisplay(0,  %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsLoopFID() function

Formats loop statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSLOOP using the ccbcl for BE or FE processors.

No hash data is returned. No string data is returned.

This is for FIDs 277 and 278.

=cut
##############################################################################
##############################################################################
sub FmtStatsLoopFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStatsLoop:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;
    my $oLen;
    my $proc;
    my $cmd;

    $length =  length($$bufferPtr);

    # Here we are using one function to handle two different FIDs. The
    # content of the processor variable determines how we behave. We
    # look for the string "BE" to select the processor.

    if ( index( uc($processor),"BE") > -1 )
    {
        $cmd = PI_STATS_BACK_END_LOOP_CMD;
        $proc = "BE";
    }
    else
    {
        $cmd = PI_STATS_FRONT_END_LOOP_CMD;
        $proc = "FE";
    }

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    $cmd, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # need to walk the buffer to count the number of blocks present
    # This is based upon the number of QL caards in the system.

    $disks = 0;
    $oLen = 0;

    for ( $i = 0; $i < $length; $i += $oLen )
    {
        $fmt = sprintf("x%d S",$i);
        ($j) = unpack($fmt, $$bufferPtr );
        if ( $j > 0 and $j <= 256 )
        {
            $disks++;
            # J is the length of data that follows
            # add two bytes to include $j
            $oLen = ($j + 2);       # assume padded to 16 bytes
        }
        else { last; }
    }

    $length = pack("L", $disks);

    print " Counted $disks loops in the data \n";


    $rp{'data'} = $length . $$bufferPtr;


    %info = XIOTech::cmdMgr::_statsLoop(0, 0, \%rp);


    $msg .= XIOTech::cmdMgr::statsLoopDisplay(0,  $proc, %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsLoopFIDNC() function

Formats loop statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSLOOP using the ccbcl for BE and FE processors.

No hash data is returned. No string data is returned.

This is for FID 290.

=cut
##############################################################################
##############################################################################
sub FmtStatsLoopFIDNC
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;


    my $msg = "\nStatsLoopNC:\n";
    my %info;

    my $header;
    my %rp;
    my $length;
    my $cmd;
    my $bePart = 0;
    my $FELength;

    $length =  length($$bufferPtr);


    # This FID contains data for both FE and BE processors, As a result
    # We ned to do CL calls for each half.

    #    FE part
    $cmd = PI_STATS_FRONT_END_LOOP_CMD;

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length - 4 ), 0, 1,
                    $cmd, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = substr($$bufferPtr, 4);   # skip 1st 4 bytes

    %info = XIOTech::cmdMgr::_statsLoop(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsLoopDisplay(0,  "FE", %info);

    #--------------------------------

    #    BE part

    # To Do the second half, we get the length of the first part and
    # use the substr function to remove the first part and the length
    # bytes for each part. This just leaves us with the second part.

    ($FELength) = unpack("L", $$bufferPtr );
    $bePart = substr($$bufferPtr, 8 + $FELength);   # skip length bytes
    $length =  length($bePart);


    $cmd = PI_STATS_BACK_END_LOOP_CMD;

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length ), 0, 1,
                    $cmd, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = $bePart;

    %info = XIOTech::cmdMgr::_statsLoop(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsLoopDisplay(0, "BE", %info);


    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsI2CFID() function

Formats I2C statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSI2C using the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 283.

=cut
##############################################################################
##############################################################################
sub FmtStatsI2CFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStats I2C:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_STATS_I2C_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_i2cStatsExtPacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayI2cStatsExtended(0,  %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsProcFID() function

Formats processor statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSPROC using the ccbcl for FE and BE processors.

No hash data is returned. No string data is returned.

This is for FID 284.

=cut
##############################################################################
##############################################################################
sub FmtStatsProcFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStatsProc:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_STATS_PROC_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_statsProc(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsProcDisplay(0, "ALL", %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsPciFID() function

Formats PCI statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSPCI using the ccbcl for FE and BE processors.

No hash data is returned. No string data is returned.

This is for FID 285.

=cut
##############################################################################
##############################################################################
sub FmtStatsPciFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStats PCI:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_STATS_PCI_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_statsPCI(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsPCIDisplay(0, "ALL", %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
=head2 FmtVCGInfoFID() function

Formats VCG information. Returns GOOD upon completion.

No hash data is returned. No string data is returned.

This is for FID 300.

=cut
##############################################################################
##############################################################################
sub FmtVCGInfoFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_VCG_INFO_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_vcgInfoPacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayVCGInfo(0, %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}


##############################################################################
=head2 FmtBEDevicePaths() function

Formats Back End Device Path info for Disk Bays and PDisks.
Returns GOOD upon completion.

This is for FIDs 264 and 268

=cut
##############################################################################
##############################################################################
sub FmtBEDevicePaths
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;

    my $fmt;
    my $msg = "";
    my %info;
    my $header;
    my %rp;
    my $length;


    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_PROC_BE_DEVICE_PATH_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_devicePathResponsePacket(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::displayDevicePath(0, %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}


##############################################################################
=head2 FmtTargetResListFID() function

Formats Target Resource List information. Returns GOOD upon completion.
NOTE: This ONLY handles the Active Server List in WWN format.

No hash data is returned. No string data is returned.

This is for FID 301.

=cut
##############################################################################
##############################################################################
sub FmtTargetResListFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_TARGET_RESOURCE_LIST_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;

    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_targetResListPacket(0, 0, \%rp);

    # This function currently only handles the Active Server List in
    # WWN format.  This is the only format generated by the CCB code
    # for this FID.
    $msg .= XIOTech::cmdMgr::displayTargetResList(0, 39, %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtStatsCacheFID() function

Formats cache statistics information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl. This is similar to
performing STATSCACHE using the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 289.

=cut
##############################################################################
##############################################################################
sub FmtStatsCacheFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nStats Cache Device:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length  ), 0, 1,
                    PI_STATS_CACHE_DEVICES_CMD, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;



    $rp{'data'} = $$bufferPtr;


    %info = XIOTech::cmdMgr::_statsCacheDevices(0, 0, \%rp);

    $msg .= XIOTech::cmdMgr::statsCacheDevicesDisplay(0, 0xFFFF, %info);

    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtLogFid() function

Formats FLASH log information. Returns GOOD upon completion.

The processing for this data is incomplete. The logsim.exe program
should be used.

No hash data is returned.


This is for FIDs 261 or 262.

=cut
##############################################################################
##############################################################################
sub FmtLogFid
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $msg = "\nLOG (unsupported):\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $msg .= "Length of buffer: $length \n";
    print  "Length of buffer: $length \n";


    $fmt = sprintf("x%d L",$offset);      # generate the format string
    ($item1 ) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" reserved: 0x%08x    \n",$item1);

    $offset += 4;                         #  bytes processed


#    while (( $offset < $length) && ($item4 < 8192 ))
    while (( $offset < $length) )
    {
        #typedef struct
        #{
        #     union
        #     {
        #        struct
        #        {
        #            UINT16  flags;
        #            UINT16  reserved;
        #        }status;
        #        UINT32 statusWord;
        #     } le;
        #
        #     UINT32     masterSequence;
        #     UINT32     sequence;
        #     UINT16     length;
        #     UINT16     eventCode;
        #     UINT32     timeStamp;
        #} LOG_HDR, *LE_PTR;



        $fmt = sprintf("x%d LLL SSL",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6 ) =
                            unpack $fmt , $$bufferPtr;

  #      $msg .= sprintf("Offset: 0x%08x  ", $offset);
  #      $msg .= sprintf(" StatusWord: 0x%08x  ",$item1);
  #      $msg .= sprintf(" masterSequence: 0x%08x  ",$item2);
  #      $msg .= sprintf(" sequence: 0x%08x  ",$item3);
  #      $msg .= sprintf(" addtl length: 0x%04x  ",$item4);
  #      $msg .= sprintf(" event code: 0x%04x  ",$item5);
  #      $msg .= sprintf(" timestamp: 0x%08x  \n",$item6);

        $msg .= sprintf(" [0x%08x] 0x%08x: %8d (%8d): 0x%04x %6d bytes \n",
                     $offset, $item6, $item2, $item3, $item5, $item4);


        if ( $item1 == 0xffffffff )
        {
            # try to get to next 256KBlock

            $offset = 4 + (($offset + 0x40000) & 0xfffc0000);

        }
        else
        {
            $offset += 20;                         #  bytes processed

            $offset += $item4;    # skip additional data
        }
    }

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }


    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtCCBTraceFID() function

Formats CCB trace information. Returns GOOD upon completion.

The data has a short header and is then the same as the MRP trace.

No hash data is returned.

This is for FID 256.

=cut
##############################################################################
##############################################################################
sub FmtCCBTraceFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address,
                $version) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $version = 0 if !defined($version);

    $length =  length($$bufferPtr);

    $msg = "\nCCBE Trace:\n$processor \n";


    # FmtFwhShort ( \$msg, $bufferPtr, $offset, 20, $processor, 0 );

    # FmtTrace ( $destPtr, $bufferPtr, 20 , $length - 20, $processor, 20 );

    #FmtMrpTrcP( \$msg, $bufferPtr, 20, $length - 20, $processor, 20);


    # Call the TracDec decoder instead.  Get a local copy of the buffer.
    # Pass in an offset so TracDec will skip past the FW Header info.
    my $localBuffer = $$bufferPtr;
    $msg .= CCBDecoder($localBuffer, 20, $version);

    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtCCBPCBDumpFID() function

Formats CCB PCB Dump information. Returns GOOD upon completion.

The data has a short header and is then the same as the MRP trace.

No hash data is returned.

This is for FID 260.

=cut
##############################################################################
##############################################################################
sub FmtCCBPCBDumpFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $msg = "\nCCB PCB Dump:\n$processor \n";


    FmtFwhShort ( \$msg, $bufferPtr, $offset, 20, $processor, 0 );

    $offset += 20;                         # add bytes processed
    $length -= 20;
    $address +=20;

    $msg .= substr( $$bufferPtr, $offset, $length);
    # FmtString( $bufferPtr, $offset, $length );

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtCCBHeapStatsFID() function

Formats CCB PCB Dump information. Returns GOOD upon completion.

No hash data is returned.

This is for FID 258.

=cut
##############################################################################
##############################################################################
sub FmtCCBHeapStatsFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor,
                                                     $address, $fwh) = @_;
    my $fmt;
    my $msg;
    my $bufLen = length ($$bufferPtr);

    $msg = "\nCCB Heap Stats:\n\n";

    #
    # When called as part of a NVRAM decode, $reqLength will be set to -1.
    # If this is the case, exit, as this older format is not copied to NVRAM.
    # This path should never be called, but in case it is, just exit.
    #
    if ($reqLength == -1)
    {
        $msg .= sprintf( "None Found!\n");
        return GOOD;
    }

    #
    # If there is a "mini" FW header, read it first.
    #
    if ($fwh)
    {
        FmtFwhShort ( \$msg, $bufferPtr, $offset, 20, $processor, 0 );
        $offset += 20;                         # add bytes processed
        $bufLen -= 20;
    }

    #
    # First read the heap control block
    #
    $fmt = sprintf("x%d L9",$offset);      # generate the format string
    $offset += 36;
    $bufLen -= 36;

    my ($fm_org, $fm_s0len, $fm_align, $fm_cur_avl, $fm_max_avl, $fm_min_avl,
     $fm_waits, $fm_wait_stat, $fm_count) = unpack $fmt, $$bufferPtr;

    #
    # Other local vars
    #
    my @hits;
    my @hits2;
    my $total;
    my $i;

    #
    # The read each allocation record
    #
    while($bufLen >= 16) {

        $fmt = sprintf("x%d L A8 SS", $offset);      # generate the format string
        my ($length, $fileName, $lineNum, $tag) = unpack $fmt, $$bufferPtr;

        $fileName =~ s/\0.*//;
        $fileName =~ s/\..*//;

        # round up to multiple of 16 and add 16. This
        # gets us to the actual allocation in the malloc code.
#        $length = ((int(($length + 15) / 16)) * 16) + 16;

        # Post-process the parameters even further than what is
        # done in PI_generic.c.
        if (($fileName =~ /^RIP=/) or
                    ($fileName =~ /^[a-zA-Z]{1}[\w]{0,7}[ ]{0,7}$/))
        {

            if ($fileName =~ /^RIP=/) {
                my $rip;
                $fmt = sprintf("x%d L A4 L SS", $offset);    # generate the format string
                ($length, $fileName, $rip, $lineNum, $tag) = unpack $fmt, $$bufferPtr;
                $lineNum = $rip;
            }

            my $found = 0;
            for($i=0; $i<@hits; $i++) {
                # array def:  0:fileName  1:lineNum  2:count 3:totAlloc
                if($hits[$i][0] eq $fileName  &&  $hits[$i][1] == $lineNum) {
                    $hits[$i][2]++;
                    $hits[$i][3] += $length;
                    $found = 1;
                    last;
                }
            }

            if($found == 0) {
                push @hits, [($fileName, $lineNum, 1, $length)];
            }

            $total++;
        }

        $offset += 16;
        $bufLen -= 16;
    }

    #
    # Create another array that can now be sorted
    #
    for($i=0; $i<@hits; $i++) {
        my $templ;
        if($hits[$i][1] < 0xa0000000) {
            $templ = "%-8s %8u %5u %8u";
        }
        else {
            $templ = "%-8s %8x %5u %8u";
        }
        push @hits2, sprintf $templ,
        $hits[$i][0], $hits[$i][1],
        $hits[$i][2], $hits[$i][3];
    }
    @hits2 = sort @hits2;

    #
    # print out the results
    #
    $msg .= sprintf "\n";
    $msg .= sprintf "Base Addr       = 0x%08X\n", $fm_org;
    $msg .= sprintf "Heap Size         = %8u\n", $fm_max_avl;
    $msg .= sprintf "Cur Available     = %8u\n", $fm_cur_avl;
    $msg .= sprintf "Low Water Mark    = %8u\n", $fm_min_avl;
    $msg .= sprintf "Wait Count        = %8u\n", $fm_waits;
    $msg .= sprintf "Wait State        = %8u\n", $fm_wait_stat;
    $msg .= sprintf "Malloc's Reported = %8u\n", $fm_count;
    $msg .= sprintf "Malloc's Found    = %8u\n", $total;
    $msg .= sprintf "Total Malloc'd    = %8u\n", $fm_max_avl-$fm_cur_avl;
    $msg .= sprintf "\n";

    $msg .= sprintf "filename     line  count   total\n";
    $msg .= sprintf "--------------------------------\n";
    for($i=0; $i<@hits2; $i++) {
        $msg .= sprintf "$hits2[$i]\n";
    }
    $msg .= sprintf "\n";

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     FmtCCBHeapStatsFID2
#
# Desc:     Formats CCB heap statistics into a string
#
#
#  typedef struct {
#      UINT32  name0;
#      UINT32  name1;
#      UINT32  lineNum;
#      UINT32  count;
#      UINT32  total;
#  } HEAP_STATS_ENTRY;
#
#  typedef struct {
#      CHAR        eyecatcher[12];
#      KERNEL_HEAP heapCB;
#      TIMESTAMP   ts;
#      UINT32      numEntries;
#
#      HEAP_STATS_ENTRY entry[NUM_HEAP_STATS_ENTRIES];
#  } HEAP_STATS;
#
#
##############################################################################
sub FmtCCBHeapStatsFID2
{
    my ($destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor,
                                                        $address, $fwh) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $item9;
    my $msg = "\nCCB Heap Statistics:\n\n";
    my $str;
    my %info;

    #
    # If there is a "mini" FW header, read it first.
    #
    if ($fwh)
    {
        FmtFwhShort ( \$msg, $bufferPtr, $offset, 20, $processor, 0 );
        $offset += 20;                         # add bytes processed
    }

#   CHAR        eyecatcher[12];
    $fmt = sprintf("x%d A12", $offset);      # generate the format string
    ($item1) = unpack $fmt , $$bufferPtr;

    # Could be 0 if reading out of NVRAM or 0xFFFFFFFF if reading out of FLASH.
    # Then just return.
    $fmt = sprintf("x%d LL", $offset);      # generate the format string
    ($item2, $item3) = unpack $fmt , $$bufferPtr;

    if (($item2 == 0 and $item3 == 0) or
        ($item2 == 0xFFFFFFFF and $item3 == 0xFFFFFFFF))
    {
        $msg .= sprintf( "None Found!\n");
    }
    elsif($item1 =~ /HEAPSTATS 01/)
    {
        $msg .= sprintf( "Eyecatcher:       \"%s\"\n\n", $item1);
        $offset += 12;

        #  typedef struct KERNEL_HEAP_STRUCT
        #  {
        #       UINT32 fm_org;
        #       UINT32 fm_s0len;
        #       UINT32 fm_align;
        #       UINT32 fm_cur_avl;
        #       UINT32 fm_max_avl;
        #       UINT32 fm_min_avl;
        #       UINT32 fm_waits;
        #       UINT32 fm_wait_stat;
        #       UINT32 fm_count;
        #  } KERNEL_HEAP, *KERNEL_HEAP_PTR;

        $fmt = sprintf("x%d L9",$offset);      # generate the format string
        my (@heap) = unpack $fmt, $$bufferPtr;

        $msg .= sprintf( "Base Address:      0x%08X\n", $heap[0]);
        $msg .= sprintf( "Heap Size:         %u\n", $heap[4]);
        $msg .= sprintf( "Cur Available:     %u\n", $heap[3]);
        $msg .= sprintf( "Low Water Mark:    %u\n", $heap[5]);
        $msg .= sprintf( "Wait Count:        %u\n", $heap[6]);
        $msg .= sprintf( "Wait State:        %u\n", $heap[7]);
        $msg .= sprintf( "Malloc Count:      %u\n", $heap[8]);
        $msg .= sprintf( "Bytes Malloc'd:    %u\n\n", $heap[4]-$heap[3]);

        $offset += 36;                         # add bytes processed

        #  typedef struct TIMESTAMP
        #  {
        #      UINT16  year;               /**< Year 0 -9999                           */
        #      UINT8   month;              /**< Month 1 -12                            */
        #      UINT8   date;               /**< Day of the month 1 - 31                */
        #      UINT8   day;                /**< Day of the week 1 - 7 (1 = Sunday)     */
        #      UINT8   hours;              /**< Hour 0 - 23     (0 = midnight)         */
        #      UINT8   minutes;            /**< Minutes 0 - 59                         */
        #      UINT8   seconds;            /**< Seconds 0 - 59                         */
        #      UINT32  systemSeconds;      /**< Seconds the system has been running    */
        #  } TIMESTAMP, *TIMESTAMP_PTR;

        $fmt = sprintf("x%d S C6 L",$offset);   # generate the format string
        my (@ts) = unpack $fmt, $$bufferPtr;

        $msg .= sprintf( "Heap Data Collected at:\n%X/%02X/%04X %02X:%02X:%02X, ".
            "up for %s\n\n",
            $ts[1], $ts[2], $ts[0], $ts[4], $ts[5], $ts[6], Uptime($ts[7]));

        $offset += 12;                     # add bytes processed

        # Get numEntries
        $fmt = sprintf("x%d L",$offset);   # generate the format string
        my ($numEntries) = unpack $fmt, $$bufferPtr;
        $offset += 4;                     # add bytes processed

        #  typedef struct {
        #      UINT32  name0;
        #      UINT32  name1;
        #      UINT32  lineNum;
        #      UINT32  count;
        #      UINT32  total;
        #  } HEAP_STATS_ENTRY;

        $msg .= sprintf( "filename     line  count   total\n");
        $msg .= sprintf( "--------------------------------\n");
        my $i;
        my $numFound;
        for ($i = 0; $i < $numEntries; $i++)
        {
            $fmt = sprintf("x%d A8 L L L", $offset);   # generate the format string
            my ($name, $line, $count, $total) = unpack $fmt, $$bufferPtr;

            if ($name =~ /RIP=/)
            {
                $msg .= sprintf( "%-8s %8X %5u %8u\n", $name, $line, $count, $total);
            }
            else
            {
                $msg .= sprintf( "%-8s %8u %5u %8u\n", $name, $line, $count, $total);
            }

            $numFound += $count;
            $offset += 20;
        }

        $msg .= sprintf( "\nMalloc's Reported: %4u\n\n", $numFound);
    }
    else
    {
        # Call the old decoder
        return FmtCCBHeapStatsFID(@_);
    }

    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;      # append to callers item
    }

    return GOOD;
}

##############################################################################
##############################################################################
=head2 FmtCCProfileFID() function

Formats CCB profile information. Returns GOOD upon completion.

No hash data is returned.

This is for FID 259.

This function is incomplete, do not use. The FID will not be extracted under
normal conditions.

=cut
##############################################################################
##############################################################################
sub FmtCCProfileFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $msg = "\nCCB Profile (only process header):\n$processor \n";

    FmtFwhShort ( \$msg, $bufferPtr, $offset, 20, $processor, 0 );

    $offset += 20;                         # add bytes processed
    $length -= 20;
    $address +=20;



    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}
##############################################################################

##############################################################################
=head2 FmtCCBNVR1FID() function

Formats CCB NVRAM FID information. Returns GOOD upon completion.

No hash data is returned.

This is for FID 293.

This function is incomplete, do not use. The FID will not be extracted under
normal conditions.

=cut
##############################################################################
##############################################################################
sub FmtCCBNVR1FID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $msg = "\n--------------------------------------------------------------\n";
    $msg .= "\n                    CCB NVRAM 1\n";
    $msg .= "\n--------------------------------------------------------------\n";

    my $startOffset = $offset;
    my $startAddress = $address;

    #    typedef struct _NVRAM_STRUCTURE
    #    {
    #        /*
    #        ** Standard defines for the 128kB NVRAM device
    #        */
    #
    #        /* 8 kB chunk */
    #        UINT32 schema;                       /* Overall CCB NVRAM schema        */
    #        UINT32 reservedSpace0[3];            /* Pad back to 16-byte boundary    */


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;
    $fmt = sprintf("x%d A4",$offset);      # generate the format string
    ($item5) =
                        unpack $fmt , $$bufferPtr;


    $msg .= sprintf("  \n0x%08x:          schema:  0x%08x (%s) ", $address, $item1, $item5);
    $msg .= sprintf("       reservedSpace0:  0x%08x  \n",$item2);
    $msg .= sprintf("             reservedSpace0:  0x%08x  ",$item3);
    $msg .= sprintf("             reservedSpace0:  0x%08x  \n",$item4);


    $offset += 16;                         #   bytes processed
    $address += 16;


    #        QM_MASTER_CONFIG masterConfigRecord; /* NVRAM copy of MasterConfig      */

    $ret = FmtCCBQmMc ( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 0xE20;                         #   bytes processed
    $address += 0xE20;


    #        MISC_NON_CRC_DATA miscData;          /* Misc non-crc'd data             */
    #        typedef struct
    #        {
    #            UINT32 ipcSequenceNumber;
    #            UINT32 reserved[31];
    #        } MISC_NON_CRC_DATA,


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= "\nMISC_NON_CRC_DATA:\n\n";

    $msg .= sprintf("0x%08x: ipcSequenceNumber:  0x%08x \n", $address, $item1);
    $msg .= sprintf("                reserved(0-2):  0x%08x 0x%08x 0x%08x  \n",$item2, $item3, $item4);


    $offset += 16;                         #   bytes processed
    $address += 16;

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:     reserved(3-6):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:    reserved(7-10):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:   reserved(11-14):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:   reserved(15-18):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:   reserved(19-22):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:   reserved(23-26):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("0x%08x:   reserved(27-30):  0x%08x 0x%08x 0x%08x 0x%08x \n", $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;



    #        CONTROLLER_SETUP cntlSetup;          /* NVRAM copy of ip configuration  */

    $ret = FmtCCBCtlrSU ( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 0x1dc;                         #   bytes processed
    $address += 0x1dc;



    #        ASYNC_EVENT_QUEUE asyncQueue;        /* NVRAM copy of log queue         */
    #        typedef struct _ASYNC_EVENT_QUEUE
    #        {
    #            UINT32  seqNum;    /* Sequence # of first event in the "Queue"      */
    #            UINT32  count;     /* Number of events remaining on the "Queue"     */
    #            UINT32  rsvd;      /* Resevered                                     */
    #            UINT32  crc;       /* crc                                           */
    #        } ASYNC_EVENT_QUEUE;
    #

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= "\nASYNC_EVENT_QUEUE:\n\n";
    $msg .= sprintf("  \n0x%08x:        seqNum:  0x%08x  ", $address, $item1);
    $msg .= sprintf("                    count:  0x%08x  \n",$item2);
    $msg .= sprintf("                     rsvd:  0x%08x  ",$item3);
    $msg .= sprintf("                      crc:  0x%08x  \n",$item4);


    $offset += 16;                         #   bytes processed
    $address += 16;




    #        UINT8  reservedSpace1[NVR_RSVD1];    /* (Used to be snmp addresses)     */

    $msg .= "\nreservedSpace1; (1612 bytes skipped)\n\n";

    $offset += 0x064C;                         #   bytes processed
    $address += 0x064C;


    #        I2C_NVRAM_STORAGE i2cNVRAMStorage;   /* NVRAM storage for I2C code      */
    $ret = FmtCCBbty ( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 128;                         #   bytes processed
    $address += 128;



#        UINT8 reservedSpace2[SIZE_8K -       /* Space for future growth         */
#            (4 * sizeof(UINT32)) -           /* schema and reserved0 */
#            sizeof(QM_MASTER_CONFIG) -
#            sizeof(MISC_NON_CRC_DATA) -
#            sizeof(CONTROLLER_SETUP) -
#            sizeof(ASYNC_EVENT_QUEUE) -
#            NVR_RSVD1 -
#            sizeof(I2C_NVRAM_STORAGE)];
#

    $offset = $startOffset + 8192;                         #   bytes processed
    $address = $startAddress + 8192;


#        /* 8 kB chunk */
#        UINT8 reservedSpace3[SIZE_8K];


    $msg .= "\n-------------- reserved space  ( 8K )\n";

    $msg .= "(skipped)\n\n";

    #$msg .= FmtDataString( $bufferPtr, $address, "word", 8192, $offset);

    $offset += 8192;                         #   bytes processed
    $address += 8192;



    #
    #        /* 42 kB chunk */
    #        ERRORTRAP_DATA_RUN errortrapDataRun;
    $ret = FmtCCBETDataRun( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 43008;                         #   bytes processed
    $address += 43008;



#
#        /* 6 kB chunk - minus space dedicated to the RTC */
#        ERRORTRAP_DATA_BOOT errortrapDataBoot;
#
#    } NVRAM_STRUCTURE,


    $ret = FmtCCBETBoot( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}
##############################################################################

##############################################################################
sub FmtCCBNVR1FIDNew
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;
    my $timeStamp;


    $header =  "\n--------------------------------------------------------------\n";
    $header .= "\n                    CCB NVRAM 1\n";
    $header .= "\n--------------------------------------------------------------\n\n";

    my $startOffset = $offset;
    my $startAddress = $address;

    #   typedef struct _NVRAM_STRUCTURE
    #   {
    #       /*
    #       ** Standard defines for the 128kB NVRAM device
    #       */
    #
    #       /* 8 kB chunk */
    #       UINT32 schema;                       /* Overall CCB NVRAM schema        */
    #       UINT32 reservedSpace0[3];            /* Pad back to 16-byte boundary    */
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;
    $fmt = sprintf("x%d A4",$offset);      # generate the format string
    ($item5) =
                        unpack $fmt , $$bufferPtr;

    my $masterCfgMsg;
    $masterCfgMsg .= sprintf("schema                 0x%08X (%s) ", $item1, $item5);

    $offset += 16;                         #   bytes processed
    $address += 16;


    #   QM_MASTER_CONFIG masterConfigRecord; /* NVRAM copy of MasterConfig      */
    $ret = FmtCCBQmMc (\$masterCfgMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 0xE20;                         #   bytes processed
    $address += 0xE20;


    #   MISC_NON_CRC_DATA miscData;          /* Misc non-crc'd data             */
    #       typedef struct
    #       {
    #           UINT32 ipcSequenceNumber;
    #           UINT32 reserved[31];
    #       } MISC_NON_CRC_DATA,
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $masterCfgMsg .= "\nMISC_NON_CRC_DATA:\n\n";
    $masterCfgMsg .= sprintf("ipcSequenceNumber      0x%08X \n", $item1);


    # Skip all reserved bytes
    $offset += 128;                         #   bytes processed
    $address += 128;


    #   CONTROLLER_SETUP cntlSetup;          /* NVRAM copy of ip configuration  */
    my $ctrlSetupMsg;
    $ret = FmtCCBCtlrSU(\$ctrlSetupMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 0x1dc;                         #   bytes processed
    $address += 0x1dc;

    #   ASYNC_EVENT_QUEUE asyncQueue;        /* NVRAM copy of log queue      */
    #       typedef struct _ASYNC_EVENT_QUEUE
    #       {
    #           UINT32  seqNum;    /* Sequence # of first event in the "Queue"   */
    #           UINT32  count;     /* Number of events remaining on the "Queue"  */
    #           UINT32  rsvd;      /* Resevered                                  */
    #           UINT32  crc;       /* crc                                        */
    #       } ASYNC_EVENT_QUEUE;
    #
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $ctrlSetupMsg .= "\nASYNC_EVENT_QUEUE:\n\n";
    $ctrlSetupMsg .= sprintf("seqNum                 0x%08X \n", $item1);
    $ctrlSetupMsg .= sprintf("count                  0x%08X \n", $item2);
    $ctrlSetupMsg .= sprintf("crc                    0x%08X \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;


    # UINT8  reservedSpace1[NVR_RSVD1];    /* (Used to be snmp addresses)   */

    $offset += 0x064C;                         #   bytes processed
    $address += 0x064C;


    #   I2C_NVRAM_STORAGE i2cNVRAMStorage;   /* NVRAM storage for I2C code    */
    my $batteryMsg;
    $ret = FmtCCBbty(\$batteryMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 128;                         #   bytes processed
    $address += 128;


    #   DISASTER_DATA disasterData;          /* Disaster data for elections */
    #
    #   #define MMC_MESSAGE_SIZE    40
    #
    #   typedef union
    #   {
    #       UINT32 value;
    #       DISASTER_DATA_FLAGS_BITS bits;
    #   } DISASTER_DATA_FLAGS;
    #
    #   typedef struct
    #   {
    #       UINT32 schema;                      /**< Schema for disaster data version   */
    #       DISASTER_DATA_FLAGS flags;          /**< Flags used in election logic       */
    #       INT8 reasonString[MMC_MESSAGE_SIZE];/**< String to remember disaster reason */
    #
    #       /* Pad so structure fits into 16 byte alignment (see nvram_structure.h) */
    #       UINT8 pad[SIZE_256 -                /**< Future growth - pad to 256 bytes   */
    #           sizeof(UINT32) -                /* schema                               */
    #           sizeof(DISASTER_DATA_FLAGS) -   /* flags                                */
    #           sizeof(MMC_MESSAGE_SIZE) -      /* reasonString                         */
    #           sizeof(UINT32)];                /* crc (below)                          */
    #
    #       UINT32 crc;                         /**< CRC to validate disaster data      */
    #   } DISASTER_DATA;
    #
    # Need to decode this at some point
    my $disasterMsg = "\nDISASTER_DATA\n\n";
    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) = unpack $fmt , $$bufferPtr;

    $disasterMsg .= sprintf("schema                 0x%08X \n", $item1);
    $disasterMsg .= sprintf("flags                  0x%08X \n", $item2);
    $disasterMsg .= sprintf("reasonString           ");

    $offset  += 8;      #   bytes processed
    $address += 8;

    $disasterMsg .= substr($$bufferPtr, $offset, 40);

    $offset  += 40;      #   bytes processed
    $address += 40;

    $disasterMsg .= "\n";


    #   UINT8 reservedSpace2[SIZE_8K -       /* Space for future growth           */
    #         (4 * sizeof(UINT32)) -           /* schema and reserved0              */
    #         sizeof(QM_MASTER_CONFIG) -
    #         sizeof(MISC_NON_CRC_DATA) -
    #         sizeof(CONTROLLER_SETUP) -
    #         sizeof(ASYNC_EVENT_QUEUE) -
    #         NVR_RSVD1 -
    #         sizeof(I2C_NVRAM_STORAGE)];
    #

    # Note that up until now we have kept a running value for offset
    # and address by adding the size of the struct that was just processed.
    # Here we add 8K to the starting offset to get to the start of
    # reservedSpace3 (which also happens to be 8k big).
    $offset = $startOffset + 8192;      #   bytes processed
    $address = $startAddress + 8192;


    #   /* 8 kB chunk */
    #   UINT8 reservedSpace3[SIZE_8K];

    $offset += 8192;                         #   bytes processed
    $address += 8192;

    #
    #   /* 42 kB chunk */
    #   ERRORTRAP_DATA_RUN errortrapDataRun;
    my $errTrapRunMsg;
    $timeStamp = FmtCCBETDataRun(\$errTrapRunMsg, 0, $bufferPtr, $offset,
                            0, $processor, $address);

    $offset += 43008;                         #   bytes processed
    $address += 43008;

    #
    #   /* 6 kB chunk - minus space dedicated to the RTC */
    #   ERRORTRAP_DATA_BOOT errortrapDataBoot;
    #
    # } NVRAM_STRUCTURE,

    my $errTrapBootMsg;
    $ret = FmtCCBETBoot(\$errTrapBootMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        # $$destPtr .= $msg;         # append to callers item

        # Re-order major sections to taste
        $$destPtr .= $header;
        $$destPtr .= $errTrapRunMsg;
        $$destPtr .= "--------------------------------------------------------------\n";
        $$destPtr .= $masterCfgMsg;
        $$destPtr .= "--------------------------------------------------------------\n";
        $$destPtr .= $ctrlSetupMsg;
        $$destPtr .= "--------------------------------------------------------------\n";
        $$destPtr .= $batteryMsg;
        $$destPtr .= "--------------------------------------------------------------\n";
        $$destPtr .= $errTrapBootMsg;
        $$destPtr .= "--------------------------------------------------------------\n";
        $$destPtr .= $disasterMsg;
    }

    return $timeStamp;
}
##############################################################################

##############################################################################
sub FmtCCBNVR1FlashCopies
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;

    my $header;
    my @msg;
    my @timeStamp;
    my @sortedTimeStamp;
    my $sep = "==================================================\n";
    my $completeMsg = "";


    $header =  "\n--------------------------------------------------------------\n";
    $header .= "\n                    CCB NVRAM Flash Copies\n";
    $header .= "\n--------------------------------------------------------------\n\n";

    my $startOffset = $offset;
    my $startAddress = $address;


    # There are 8 copies of NVRAM 1 in the NVRAM Flash Copies FID.  Each is
    # 64KB and is decoded using the same decoder as the CCB NVRAM Fid 293.
    #
    # An Epoch timestamp for each NVRAM copy is passed up through the
    # function call tree.  This timeStamp and the msg string for each
    # copy are stored in separate arrays.
    #
    for (my $i = 0; $i < 8; $i++)
    {
        $timeStamp[$i] = FmtCCBNVR1FIDNew(\$msg[$i], 0, $bufferPtr, $offset,
                                          0, $processor, $address );

#        printf "FmtCCBNVR1FlashCopies: timeStamp[$i]=$timeStamp[$i]\n";

        # Move pointers by 64K
        $offset += 0x10000;           #   bytes processed
        $address += 0x10000;
    }


    # Sort time stamps numerically descending (i.e. reverse chronological
    # order.
    @sortedTimeStamp = sort {$b <=> $a} @timeStamp;

#    print "\nSorted list: @sortedTimeStamp\n";


    # Find the message corresponding to the sorted time stamp and build
    # the complete message in the proper order.
    for (my $i = 0; $i < 8; $i++)
    {
        for (my $j = 0; $j < 8; $j++)
        {
            if ($sortedTimeStamp[$i] == $timeStamp[$j])
            {
                # Add a separator for each NVRAM image
                my $headMsg = sprintf("\n%sCCB_NVRAM_Flash_Copy_%d(){}\n%s",
                                      $sep, $i + 1, $sep);

                $completeMsg .= $headMsg;
                $completeMsg .= $msg[$j];
            }
        }
    }


    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $header;
        $$destPtr .= $completeMsg;
    }

    return GOOD;
}



##############################################################################


##############################################################################
#
#     This fcn handles the files that are data dumps from the BE
#     processor NVRAM part 2.
#
#
#
##############################################################################
sub DumpNVR2
{

    my ( $buffer, $flags ) = @_;

    my $available;

    my $msg;
    my $ret = GOOD;

    $available = length( $buffer ) ;   # bytes available to do

    FmtNvramDumpBEParts( \$msg, \$buffer, 0, $available, "  ", 0 );

    return $msg;
}

##############################################################################
sub FmtFCALCounters
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\npdisk FCAL counters:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;
    my $numPdisks;
    my @pdisks;

    my $startingOffset = $offset;


    $length =  length($$bufferPtr);

    # count = *(UINT16 *)bufP = pDevList->count;
    # bufP += (sizeof(UINT16));

    #
    # get the pdisk count
    #

    $fmt = sprintf("x%d S", $offset);      # generate the format string
    ($item1) =     unpack $fmt , $$bufferPtr;

    $numPdisks = $item1;

    $msg .= sprintf(" Number of Pdisks:        %4d    \n", $item1);

    $offset += 2;

    #
    # pull out the list of pdisks
    #

    $msg .= sprintf("     These Pdisks: ", $item1);

    for ( $i = 0; $i < $numPdisks; $i++ )
    {

        $fmt = sprintf("x%d S", $offset);      # generate the format string
        ($item1) =     unpack $fmt , $$bufferPtr;

        $pdisks[$i] = $item1;

        $msg .= sprintf(" %3d,", $item1);

        $offset += 2;

        if ( ($i + 1 ) % 16 == 0 )
        {
            $msg .= "\n                   ";
        }

    }

    $msg .= " \n";


    #
    # Loop thru each pdisk
    #

    for ( $i = 0; $i < $numPdisks; $i++ )
    {
        $msg .= "\nFor pdisk $pdisks[$i]:\n\n";


        FmtPhysDevice( \$msg, 0, $bufferPtr, $offset, $reqLength, "BE PdiskInfo", $address);


        $offset += 216;

        #typedef struct _PI_DEBUG_SCSI_CMD_RSP
        #{
        #    UINT8   sense;          /* Sense key                    */
        #    UINT8   asc;            /* Addional sense code          */
        #    UINT8   ascq;           /* Addional sense code qualifier */
        #    UINT32  length;         /* actual response data length  */
        #    ZeroArray(UINT8, data); /* response data                */
        #} PI_DEBUG_SCSI_CMD_RSP;

        $fmt = sprintf("x%d CCC L", $offset);      # generate the format string
        ($item1, $item2, $item3, $item4 ) =   unpack $fmt , $$bufferPtr;

        $msg .= " \n";

        $msg .= sprintf("        Sense key:   %02x    \n", $item1);
        $msg .= sprintf(" Addtl Sense Code:   %02x    \n", $item2);
        $msg .= sprintf("    ASC Qualifier:   %02x    \n", $item3);
        $msg .= sprintf("      Data Length:   %d    \n\n", $item4);


        $offset += 7;

        $msg .= FmtDataString( $bufferPtr, 0, "byte", $item4, $offset);



        $msg .= "\n----------------------------------------------------------------- \n";

        $offset += $item4;

    }

    #
    # We've completed the output of the individual pdisk data.
    # Now make a call to genetare the 'PDISKS LOOP' style table.
    # This just makes another pass thru the FID looking at the data
    # differently
    #

    $ret = FmtLoopSenseData( \$msg,
                             0,
                             $bufferPtr,
                             $startingOffset,
                             $reqLength,
                             "BE",
                             0 );


    $$destPtr .= $msg;


}

##############################################################################
sub FmtFWHeadersFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\nList of all Firmware Headers (Running and Flash):\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;
    my $numPdisks;
    my @pdisks;

    my $startingOffset = $offset;


    $length =  length($$bufferPtr);

    # count = *(UINT16 *)bufP = pDevList->count;
    # bufP += (sizeof(UINT16));

    #
    # get the pdisk count
    #
    $fmt = sprintf("x%d L", $offset);      # generate the format string
        ($item2) =     unpack $fmt , $$bufferPtr;

    $offset += 16;

    $msg.= "-" x 97 . "\n";
    for($i = 0; $i < $item2; $i++)
    {
        $fmt = sprintf("x%d A16", $offset);      # generate the format string
            ($item1) =     unpack $fmt , $$bufferPtr;

        $msg .= sprintf("Firmware Header: %16s\n\n", $item1);

        $offset += 16;

        #
        # pull out the list of pdisks
        #

        FmtFwh( \$msg, $bufferPtr, $offset, $reqLength, $item1, $address);

        $offset += 128;

        $msg.= "-" x 97 . "\n";
    }


    $$destPtr .= $msg;


}

##############################################################################
sub FmtFWHeadersFIDSummary
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;

    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $i;
    my $startingOffset = $offset;

    #
    $length =  length($$bufferPtr);


    $msg = "Firmware            Vers    Count    BldId    SysRls    Timestamp   (CST/CDT)\n";

    #
    # Not sure what this is for - skip the first 16 bytes?
    #
    $fmt = sprintf("x%d L", $offset);      # generate the format string

    # What does this do?
    ($item2) = unpack $fmt , $$bufferPtr;

    $offset += 16;

    # Create a separator line of dashes
    $msg.= "-" x 78 . "\n";

    for($i = 0; $i < $item2; $i++)
    {
        $fmt = sprintf("x%d A16", $offset);      # generate the format string
            ($item1) =     unpack $fmt , $$bufferPtr;

        # Firmware type string
        $msg .= sprintf("%16s    ", $item1);

        $offset += 16;

        #
        # Format the data for this firmware header
        #
        FmtFwhCCBEFormat( \$msg, $bufferPtr, $offset, $reqLength, $item1, $address);

        # Create a separator line of dashes at certain spots
        if ($i == 2 || $i == 5 || $i == 8 || $i == 10 || $i == 16 || $i == 22)
        {
            $msg.= "-" x 78 . "\n";
        }

        $offset += 128;             # Move to start of next header
    }


    $$destPtr .= $msg;


}


##############################################################################
sub FmtFCMCounterTimestamp
{
    my ( $bufferPtr, $offsetPtr ) = @_;
    my $fmt = sprintf( "x%d SCCCCCCL", $$offsetPtr );
    my ( $year, $month, $date, $day, $hours, $minutes, $seconds, $systemSeconds ) =
        unpack( $fmt, $$bufferPtr );

    $$offsetPtr += 12;
    my $msg = sprintf( "%02X/%02X/%04X %02X:%02X:%02X",
        $month, $date, $year, $hours, $minutes, $seconds );

    return( $msg );
}

##############################################################################
sub FCM_CountersSignedToString
{
    my ( $counterValue ) = @_;
    my $msg = "";

    if( $counterValue == -1 )
    {
#        $msg = sprintf( "N/A" );
        $msg = sprintf( "   " );
    }
    else
    {
        $msg = sprintf( "%10d", $counterValue );
    }

    return( $msg );
}

##############################################################################
sub FCM_CountersToString
{
    my ( $counterValue ) = @_;
    my $msg = "";

    if( $counterValue == 0xFFFFFFFF )
    {
#        $msg = sprintf( "N/A" );
        $msg = sprintf( "   " );
    }
    else
    {
        $msg = sprintf( "%10u", $counterValue );
    }

    return( $msg );
}

##############################################################################
sub FmtFCMCounterMapHeader
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr ) = @_;
    my $fmt = sprintf( "x%d LLLLLLLLL", $$offsetPtr );
    my ( $version, $busyMutex, $numberOfBackEndHabs, $numberOfBays, $numberOfSlotsInBay,
         $sizeCounterMap, $sizeErrorData, $sizeHabData, $sizeBayData ) =
        unpack( $fmt, $$bufferPtr );

    $$offsetPtr += 36;
    $$destPtr .= sprintf( "  version:             0x%08x\n", $version );
    $$destPtr .= sprintf( "  busyMutex:           0x%08x\n", $busyMutex );
    $$destPtr .= sprintf( "  numberOfBackEndHabs: %u\n", $numberOfBackEndHabs );
    $$destPtr .= sprintf( "  numberOfBays:        %u\n", $numberOfBays );
    $$destPtr .= sprintf( "  numberOfSlotsInBay:  %u\n", $numberOfSlotsInBay );
    $$destPtr .= sprintf( "  sizeCounterMap:      %u\n", $sizeCounterMap );
    $$destPtr .= sprintf( "  sizeErrorData:       %u\n", $sizeErrorData );
    $$destPtr .= sprintf( "  sizeHabData:         %u\n", $sizeHabData );
    $$destPtr .= sprintf( "  sizeBayData:         %u\n", $sizeBayData );

    $$counterHeaderPtr{VERSION}                 = $version;
    $$counterHeaderPtr{NUMBER_OF_BACK_END_HABS} = $numberOfBackEndHabs;
    $$counterHeaderPtr{NUMBER_OF_BAYS}          = $numberOfBays;
    $$counterHeaderPtr{NUMBER_OF_SLOTS_IN_BAY}  = $numberOfSlotsInBay;
    $$counterHeaderPtr{SIZE_COUNTER_MAP}        = $sizeCounterMap;
    $$counterHeaderPtr{SIZE_ERROR_DATA}         = $sizeErrorData;
    $$counterHeaderPtr{SIZE_HAB_DATA}           = $sizeHabData;
    $$counterHeaderPtr{SIZE_BAY_DATA}           = $sizeBayData;
}

##############################################################################
sub FmtFCMCounterMapFlags
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr ) = @_;
    my $fmt = sprintf( "x%d L", $$offsetPtr );
    my ( $flags ) = unpack( $fmt, $$bufferPtr );

    $$offsetPtr += 4;
    $$destPtr .= sprintf( "  flags:               0x%08x\n", $flags );
    $$counterHeaderPtr{FLAGS} = $flags;
}

##############################################################################
sub FmtFCMCounterErrorData
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version ) = @_;
    my $startingOffset = $$offsetPtr;

    $$destPtr .= sprintf( "  Timestamps:          %s  -to-  %s\n",
        FmtFCMCounterTimestamp($bufferPtr, $offsetPtr),
        FmtFCMCounterTimestamp($bufferPtr, $offsetPtr) );

    $$destPtr .= sprintf( "\n" );
    FmtFCMCountersHabDataList( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr );

    ##
    # Baydatalist and BayInfo work on the same data, so save and restore the
    # offset pointer
    ##
    my $savedOffset = $$offsetPtr;
    $$destPtr .= sprintf( "\n" );
    FmtFCMCountersBayDataList( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version );

    $$offsetPtr = $savedOffset;
    $$destPtr .= sprintf( "\n" );
    FmtFCMBayInfo( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version );

    $$offsetPtr = $startingOffset + $$counterHeaderPtr{SIZE_ERROR_DATA};
}

##############################################################################
sub FmtFCMCountersHabDataList
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr ) = @_;
    my $habCounter = 0;
    my $fmt = "";
    my ( $linkFail, $lostSync, $lostSignal, $sequenceError, $invalidTransmit, $invalidCRC );
    my ( $flags ) = 0;
    my $startingOffset = $$offsetPtr;

    $$destPtr .= sprintf( "                                       PORT  LINK_FAIL  LOST_SYNC   INV_XMIT    INV_CRC   LOST_SIG  SEQ_ERROR\n" );
    $$destPtr .= sprintf( "                                       ---- ---------- ---------- ---------- ---------- ---------- ----------\n" );

    for( $habCounter = 0; $habCounter < $$counterHeaderPtr{NUMBER_OF_BACK_END_HABS}; $habCounter++ )
    {
        ##
        # FCM_HAB_DATA flags
        ##
        $fmt = sprintf( "x%d L", $$offsetPtr );
        $$offsetPtr += 4;
        ( $flags ) = unpack( $fmt, $$bufferPtr );

        ##
        # FCM_HAB_DATA counters
        ##
        $fmt = sprintf( "x%d LLLLLL", $$offsetPtr );
        $$offsetPtr += 24;
        ( $linkFail, $lostSync, $lostSignal, $sequenceError, $invalidTransmit, $invalidCRC ) =
            unpack( $fmt, $$bufferPtr );

        if( $flags & (1 << 0) )         # habPresent
        {
            if( $flags & (1 << 1) )     # countersValid
            {
                $$destPtr .= sprintf( "                                          %1d %10s %10s %10s %10s %10s %10s\n",
                    $habCounter,
                    FCM_CountersToString($linkFail),
                    FCM_CountersToString($lostSync),
                    FCM_CountersToString($invalidTransmit),
                    FCM_CountersToString($invalidCRC),
                    FCM_CountersToString($lostSignal),
                    FCM_CountersToString($sequenceError) );
            }
            else
            {
                $$destPtr .= sprintf( "                                          %1d\n",
                    $habCounter );
            }
        }
        else
        {
            $$destPtr .= sprintf( "                                       None\n" );
        }
    }

    $$offsetPtr = $startingOffset + $$counterHeaderPtr{SIZE_HAB_DATA};
}

##############################################################################
sub FmtFCMBayInfo
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version ) = @_;
    my $portCounter = 0;
    my $bayCounter = 0;
    my $flags = 0;
    my $hdrPrinted = 0;
    my $fmt = "";
    my ( $linkFail, $lostSync, $lostSignal, $sequenceError, $invalidTransmit, $invalidCRC );
    my ( $WordError, $CRCError, $ClockDelta, $LoopUp, $Insertion, $Stall, $Utilization );
    my ( $ses, $devType, $valid, $wwnhi, $wwnlo);
    my $startingOffset = $$offsetPtr;
    my $bayCounterSize =  $$counterHeaderPtr{SIZE_BAY_DATA} / $$counterHeaderPtr{NUMBER_OF_BAYS}; # sizeof single bay counter structure
    my $lastSes = -1;


    ###
    ## Scan through all bays
    ###
    for( $bayCounter = 0; $bayCounter < $$counterHeaderPtr{NUMBER_OF_BAYS}; $bayCounter++ )
    {
        $$offsetPtr = $startingOffset + ($bayCounter * ($bayCounterSize));

        ###
        ## Unpack another bay
        ## FCM_BAY_DATA flags
        ###
        $fmt = sprintf( "x%d L", $$offsetPtr );
        $$offsetPtr += 4;
        ( $flags ) = unpack( $fmt, $$bufferPtr );

        ###
        ## Check if the bay is present before attempting to decode all
        ## of the slot information.
        ###
        if( $flags & (1 << 0) )                                         # bayPresent
        {
            ##
            # If the bay counter is valid (SATA type bay), it has data.
            ##
            # Skip past the slot data
            # FCM_BAY_FLAGS = 4                                        <****
            # FCM_SLOT_DATA = xxx
            # FCM_BAY_INFO = 2 + 1 + 1 + 8 = 12                        <****
            # FCM_BAY_ERROR_COUNTERS =
            #   FCM_BAY_PORT_ERROR_COUNTERS [FCM_NUM_BAY_PORTS][2] =
            #     2 * 4 * FCM_BAY_PORT_ERROR_COUNTERS =
            #     2 * 4 * (4 * 6 + 4 * 7) = 416                        <****
            if ($version < 1)
            {
                $$offsetPtr += ($bayCounterSize - (4 + 48 + 12));
            }
            else
            {
                $$offsetPtr += ($bayCounterSize - (4 + 416 + 12));
            }
            ##
            # Get the Bay Information
            ##
            $fmt = sprintf( "x%d SCCNN", $$offsetPtr );
            $$offsetPtr += 12;
            ( $ses, $devType, $valid, $wwnhi, $wwnlo) =
                unpack( $fmt, $$bufferPtr );

            if( (($flags & (1 << 1)) || $devType == PD_DT_SBOD_SES ||   # portsUnknown
                ((($portCounter | 1) == 1) && ($flags & (1 << 2))) ||   # ports0and1
                ((($portCounter | 1) == 3) && ($flags & (1 << 3)))) )   # ports2and3
            {
                if (($flags & (1 << 4)) || $devType == PD_DT_SBOD_SES)
                {
                    # Add a header line here
                    if ($hdrPrinted == 0 and $lastSes != $ses)
                    {
                        FmtFCMAddHeader2(@_, $devType);
                        $hdrPrinted = 1;
                        $lastSes = $ses;
                    }

                    my $inc = 1;
                    my $limit = ($version < 1) ? 2 : 4*2;
                    if ($devType != PD_DT_SBOD_SES)
                    {
                        $inc = ($version <1) ? 1 : 2;
                        $limit = ($version < 1) ? 2 : 2*2;
                    }
                    my $portprint;
                    for( $portCounter = 0; $portCounter <  $limit; $portCounter += $inc )
                    {

                        ##
                        # FCM_BAY_COUNTERS counters
                        ##
                        $fmt = sprintf( "x%d LLLLLL", $$offsetPtr );
                        $$offsetPtr += 24 * $inc;
                        ( $linkFail, $lostSync, $lostSignal, $sequenceError, $invalidTransmit, $invalidCRC ) =
                            unpack( $fmt, $$bufferPtr );
                        if ($version >= 1) {
                            $fmt = sprintf( "x%d LLlLLLL", $$offsetPtr );
                            $$offsetPtr += 28 * $inc;
                            ( $WordError, $CRCError, $ClockDelta, $LoopUp, $Insertion, $Stall, $Utilization ) =
                                unpack( $fmt, $$bufferPtr );
                        }
                        if ($version < 1 || $devType != PD_DT_SBOD_SES)
                        {
                            $portprint = sprintf(" %1d",$portCounter/$inc);
                        }
                        else
                        {
                            $portprint = sprintf("%1d%s", $portCounter>>1, ($portCounter & 1)==0?'A':'B'),
                        }

                        my $asciiType = "UNKN";
                        if ($devType == PD_DT_FC_SES)   { $asciiType = "FC"; }
                        if ($devType == PD_DT_SATA_SES) { $asciiType = "SATA"; }
                        if ($devType == PD_DT_SBOD_SES) { $asciiType = "SBOD"; }

                        if ($devType != PD_DT_SBOD_SES)
                        {
                            $$destPtr .= sprintf( "  %08x%08x  %4s(0x%02x) %4d   %2s %10s %10s %10s %10s %10s %10s\n",
                                $wwnhi,
                                $wwnlo,
                                $asciiType, $devType,
                                $ses,
                                $portprint,
                                FCM_CountersToString($linkFail),
                                FCM_CountersToString($lostSync),
                                FCM_CountersToString($invalidTransmit),
                                FCM_CountersToString($invalidCRC),
                                FCM_CountersToString($lostSignal),
                                FCM_CountersToString($sequenceError) );
                        }
                        if ($version >= 1 && $devType == PD_DT_SBOD_SES)
                        {
                            $$destPtr .= sprintf( "  %08x%08x  %4s(0x%02x) %4d   %2s %10s %10s %10s %10s %10s %10s  %10s\n",
                                $wwnhi,
                                $wwnlo,
                                $asciiType, $devType,
                                $ses,
                                $portprint,
                                FCM_CountersToString($WordError),
                                FCM_CountersToString($CRCError),
                                FCM_CountersSignedToString($ClockDelta),
                                FCM_CountersToString($LoopUp),
                                FCM_CountersToString($Insertion),
                                FCM_CountersToString($Stall),
                                FCM_CountersToString($Utilization));
                        }
                    }
                    $hdrPrinted = 0;
                }
            }
        }
    }

    ###
    ## Move the offset to the end of the bayList
    ###
    $$offsetPtr = $startingOffset + $$counterHeaderPtr{SIZE_BAY_DATA};

}

##############################################################################
sub FmtFCMAddHeader1
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version, $devType ) = @_;
    $$destPtr .= "        WWN          DNAME    SES SLOT PORT  LINK_FAIL  LOST_SYNC   INV_XMIT    INV_CRC LIPF7_Init LIPF7_Recv LIPF8_Init LIPF8_Recv PowerOnMin";
    if ($version >= 1 and $devType == PD_DT_SBOD_SES)
    {
        $$destPtr .= "  WordError   CRCError ClockDelta     LoopUp Insertions      Stall Utilization";
    }
    $$destPtr .= "\n";

    $$destPtr .= "  ---------------- ---------- --- ---- ---- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------";
    if ($version >= 1 and $devType == PD_DT_SBOD_SES)
    {
        $$destPtr .= " ---------- ---------- ---------- ---------- ---------- ---------- -----------";
    }
    $$destPtr .= "\n";
}

##############################################################################
sub FmtFCMAddHeader2
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version, $devType ) = @_;
    $$destPtr .= "\n";
    if ($devType != PD_DT_SBOD_SES)
    {
        $$destPtr .= "        WWN            TYPE     SES PORT  LINK_FAIL  LOST_SYNC   INV_XMIT    INV_CRC   LOST_SIG  SEQ_ERROR\n";
        $$destPtr .= "  ----------------  ----------  --- ---- ---------- ---------- ---------- ---------- ---------- ----------\n";
    }
    if ($version >= 1 && $devType == PD_DT_SBOD_SES)
    {
        $$destPtr .= "        WWN            TYPE     SES PORT  WordError   CRCError ClockDelta     LoopUp Insertions      Stall Utilization\n";
        $$destPtr .= "  ----------------  ----------  --- ---- ---------- ---------- ---------- ---------- ---------- ---------- -----------\n";
    }
}

##############################################################################
sub FmtFCMCountersBayDataList
{
    my ( $destPtr, $counterHeaderPtr, $bufferPtr, $offsetPtr, $version ) = @_;
    my $portCounter = 0;
    my $slotCounter = 0;
    my $bayCounter = 0;
    my $fmt = "";
    my ( $flags ) = 0;
    my ( $linkFail_A, $lostSync_A, $invalidTransmit_A, $invalidCRC_A, $lipF7Init_A, $lipF7Recv_A, $lipF8Init_A, $lipF8Recv_A );
    my ( $linkFail_B, $lostSync_B, $invalidTransmit_B, $invalidCRC_B, $lipF7Init_B, $lipF7Recv_B, $lipF8Init_B, $lipF8Recv_B );
    my ( $WordError_A, $CRCError_A, $ClockDelta_A, $LoopUp_A, $Insertion_A, $Stall_A, $Utilization_A );
    my ( $WordError_B, $CRCError_B, $ClockDelta_B, $LoopUp_B, $Insertion_B, $Stall_B, $Utilization_B );
    my ( $powerOnMinutes, $commandInitiatePort );
    my ( $channel, $devName );
    my $portLinePrinted = 0;
    my $bayLinePrinted = 0;
    my $startingOffset = $$offsetPtr;
    my ( $wwnLow, $wwnHigh ) = 0;
    my $activePort = ' ';
    my $portLetter = 'A';
    my $hdrPrinted = 0;

    my $linkFailString = "";
    my $lostSyncString = "";
    my $invXmitString = "";
    my $invCrcString = "";
    my $lipf7InitString = "";
    my $lipf7RecvString = "";
    my $lipf8InitString = "";
    my $lipf8RecvString = "";
    my $powerOnMinString = "";
    my $WordErrorString ="";
    my $CRCErrorString ="";
    my $ClockDeltaString ="";
    my $LoopUpString ="";
    my $InsertionString ="";
    my $StallString ="";
    my $UtilizationString = "";

    my $lastPort = -1;
    my $thisPort = -1;

    my $bayCounterSize =  $$counterHeaderPtr{SIZE_BAY_DATA} / $$counterHeaderPtr{NUMBER_OF_BAYS}; # sizeof single bay counter structure
    ###
    ## Scan through all ports
    ###
    for( $portCounter = 0; $portCounter < $$counterHeaderPtr{NUMBER_OF_BACK_END_HABS}; $portCounter++ )
    {
        $portLinePrinted = 0;

        ###
        ## Scan through all bays
        ###
        for( $bayCounter = 0; $bayCounter < $$counterHeaderPtr{NUMBER_OF_BAYS}; $bayCounter++ )
        {
            $bayLinePrinted = 0;
            $$offsetPtr = $startingOffset + ($bayCounter * ($bayCounterSize));
            my $devoffset;
            ##
            # If the bay counter is valid (SATA type bay), it has data.
            ##
            # Skip past the slot data
            # FCM_BAY_FLAGS = 4                                        <****
            # FCM_SLOT_DATA = xxx
            # FCM_BAY_INFO = 2 + 1 + 1 + 8 = 12                        <****
            # FCM_BAY_ERROR_COUNTERS =
            #   FCM_BAY_PORT_ERROR_COUNTERS [FCM_NUM_BAY_PORTS][2] =
            #     2 * 4 * FCM_BAY_PORT_ERROR_COUNTERS =
            #     2 * 4 * (4 * 6 + 4 * 7) = 416                        <****
            if ($version < 1)
            {
                $devoffset = $$offsetPtr + ($bayCounterSize - (48 + 12));
            }
            else
            {
                $devoffset = $$offsetPtr + ($bayCounterSize - (416 + 12));
            }
            $fmt = sprintf( "x%d SCCNN", $devoffset );
            my ( $ses, $devType, $valid, $wwnhi, $wwnlo) =
                unpack( $fmt, $$bufferPtr );

            ###
            ## Unpack another bay
            ## FCM_BAY_DATA flags
            ###
            $fmt = sprintf( "x%d L", $$offsetPtr );
            $$offsetPtr += 4;
            ( $flags ) = unpack( $fmt, $$bufferPtr );

            ###
            ## Check if the bay is present before attempting to decode all
            ## of the slot information.
            ###
            if( $flags & (1 << 0) )                                         # bayPresent
            {
                if( (($flags & (1 << 1)) ||                                 # portsUnknown
                    ((($portCounter | 1) == 1) && ($flags & (1 << 2))) ||  # ports0and1
                    ((($portCounter | 1) == 3) && ($flags & (1 << 3)))) )  # ports2and3
                {

                    if ($flags & (1 << 1))
                    {
                        $thisPort = 0;
                    }   # portsUnknown
                    elsif ((($portCounter | 1) == 1) && ($flags & (1 << 2)))
                    {
                        $thisPort = 1;
                    }   # ports0and1
                    elsif ((($portCounter | 1) == 3) && ($flags & (1 << 3)))
                    {
                        $thisPort = 2;
                    }  # ports2and3

                    # Add a header line here
                    if ($hdrPrinted == 0 and $lastPort != $thisPort)
                    {
                        FmtFCMAddHeader1(@_, $devType);
                        $hdrPrinted = 1;
                        $lastPort = $thisPort;
                    }

                    ##
                    # If the Bay counters are valid (Sata bays only for now),
                    # the skip the drives. It will be processed in Bay info.
                    # Otherwise process the slot data.
                    ##
                    if ($flags & (1 << 4))
                    {
                        # SATA Drive.
                    }
                    else
                    {
                        # The following few are because SBOD has 16, EUROLOGIC has 14.
                        my $numberofports = $$counterHeaderPtr{NUMBER_OF_SLOTS_IN_BAY};
                        if ($version >= 1 && $devType == PD_DT_FC_SES)
                        {
                            $numberofports = 14;
                        }

                        ###
                        ## Scan through all slots in this bay
                        ###
                        for( $slotCounter = 0; $slotCounter < $numberofports; $slotCounter++ )
                        {
                            ###
                            ## Unpack another slot
                            ## FCM_SLOT_DATA flags
                            ###
                            $fmt = sprintf( "x%d L", $$offsetPtr );
                            $$offsetPtr += 4;
                            ( $flags ) = unpack( $fmt, $$bufferPtr );

                            ###
                            ## Check if the drive is present before attempting to decode all
                            ## of the counter information.
                            ###
                            if( $flags & (1 << 0) )                             # drivePresent
                            {
                                ###
                                ## FCM_SLOT_DATA_COUNTERS
                                ###
                                $fmt = sprintf( "x%d LLLLLLLL", $$offsetPtr );
                                $$offsetPtr += 32;
                                ( $linkFail_A, $lostSync_A, $invalidTransmit_A, $invalidCRC_A, $lipF7Init_A, $lipF7Recv_A, $lipF8Init_A, $lipF8Recv_A ) =
                                    unpack( $fmt, $$bufferPtr );

                                if ($version >= 1)
                                {
                                  $$offsetPtr += 28;
                                  $fmt = sprintf( "x%d LLlLLLL", $$offsetPtr );
                                  ( $WordError_A, $CRCError_A, $ClockDelta_A, $LoopUp_A, $Insertion_A, $Stall_A, $Utilization_A ) =
                                      unpack( $fmt, $$bufferPtr );
                                }

                                $fmt = sprintf( "x%d LLLLLLLL", $$offsetPtr );
                                $$offsetPtr += 32;
                                ( $linkFail_B, $lostSync_B, $invalidTransmit_B, $invalidCRC_B, $lipF7Init_B, $lipF7Recv_B, $lipF8Init_B, $lipF8Recv_B ) =
                                    unpack( $fmt, $$bufferPtr );

                                if ($version >= 1)
                                {
                                  $fmt = sprintf( "x%d LLlLLLL", $$offsetPtr );
                                  $$offsetPtr += 28;
                                  ( $WordError_B, $CRCError_B, $ClockDelta_B, $LoopUp_B, $Insertion_B, $Stall_B, $Utilization_B ) =
                                      unpack( $fmt, $$bufferPtr );
                                }

                                $fmt = sprintf( "x%d LC", $$offsetPtr );
                                $$offsetPtr += 5;
                                ( $powerOnMinutes, $commandInitiatePort ) =
                                    unpack( $fmt, $$bufferPtr );

                                ###
                                ## WWN
                                ###
                                $fmt = sprintf( "x%d NN", $$offsetPtr );
                                $$offsetPtr += 8;
                                ( $wwnHigh, $wwnLow ) = unpack( $fmt, $$bufferPtr );

                                ###
                                ## channel and devName
                                ###
                                $fmt = sprintf( "x%d CL", $$offsetPtr );
                                $$offsetPtr += 5;
                                ( $channel, $devName ) = unpack( $fmt, $$bufferPtr );

                                ###
                                ## Check if the $portCounter is looking at the same
                                ## port as the SCSI command was issued on.
                                ## Figure out the drive port-to-BE port mapping
                                ###
                                if( ($channel & 1) == ($portCounter & 1) )
                                {
                                    $activePort = '*';

                                    if( $commandInitiatePort == 0 )
                                    {
                                        $portLetter = 'A';
                                    }
                                    else
                                    {
                                        $portLetter = 'B';
                                    }
                                }
                                else
                                {
                                    $activePort = ' ';

                                    if( $commandInitiatePort == 0 )
                                    {
                                        $portLetter = 'B';
                                    }
                                    else
                                    {
                                        $portLetter = 'A';
                                    }
                                }

                                if( ($channel | 1) == ($portCounter | 1) )
                                {
                                    if( $portLetter eq 'A' )
                                    {
                                        $linkFailString   = FCM_CountersToString( $linkFail_A );
                                        $lostSyncString   = FCM_CountersToString( $lostSync_A );
                                        $invXmitString    = FCM_CountersToString( $invalidTransmit_A );
                                        $invCrcString     = FCM_CountersToString( $invalidCRC_A );
                                        $lipf7InitString  = FCM_CountersToString( $lipF7Init_A );
                                        $lipf7RecvString  = FCM_CountersToString( $lipF7Recv_A );
                                        $lipf8InitString  = FCM_CountersToString( $lipF8Init_A );
                                        $lipf8RecvString  = FCM_CountersToString( $lipF8Recv_A );

                                        if ($version >= 1)
                                        {
                                            $WordErrorString   =  FCM_CountersToString( $WordError_A );
                                            $CRCErrorString    =  FCM_CountersToString( $CRCError_A );
                                            $ClockDeltaString  =  FCM_CountersSignedToString( $ClockDelta_A );
                                            $LoopUpString      =  FCM_CountersToString( $LoopUp_A );
                                            $InsertionString   =  FCM_CountersToString( $Insertion_A );
                                            $StallString       =  FCM_CountersToString( $Stall_A );
                                            $UtilizationString =  FCM_CountersToString( $Utilization_A );
                                        }
                                    }
                                    else
                                    {
                                        $linkFailString   = FCM_CountersToString( $linkFail_B );
                                        $lostSyncString   = FCM_CountersToString( $lostSync_B );
                                        $invXmitString    = FCM_CountersToString( $invalidTransmit_B );
                                        $invCrcString     = FCM_CountersToString( $invalidCRC_B );
                                        $lipf7InitString  = FCM_CountersToString( $lipF7Init_B );
                                        $lipf7RecvString  = FCM_CountersToString( $lipF7Recv_B );
                                        $lipf8InitString  = FCM_CountersToString( $lipF8Init_B );
                                        $lipf8RecvString  = FCM_CountersToString( $lipF8Recv_B );

                                        if ($version >= 1)
                                        {
                                            $WordErrorString   =  FCM_CountersToString( $WordError_B );
                                            $CRCErrorString    =  FCM_CountersToString( $CRCError_B );
                                            $ClockDeltaString  =  FCM_CountersSignedToString( $ClockDelta_B );
                                            $LoopUpString      =  FCM_CountersToString( $LoopUp_B );
                                            $InsertionString   =  FCM_CountersToString( $Insertion_B );
                                            $StallString       =  FCM_CountersToString( $Stall_B );
                                            $UtilizationString =  FCM_CountersToString( $Utilization_B );
                                        }
                                    }

                                    $powerOnMinString = FCM_CountersToString( $powerOnMinutes );

                                    $$destPtr .= sprintf( "  %08x%08x %s %3d %4d %s%1d-%s %10s %10s %10s %10s %10s %10s %10s %10s %10s",
                                        $wwnHigh, $wwnLow,
                                        XIOTech::cmdMgr::_getString_DNAME($devName),
                                        $bayCounter,
                                        $slotCounter,
                                        $activePort,
                                        $portCounter,
                                        $portLetter,
                                        $linkFailString,
                                        $lostSyncString,
                                        $invXmitString,
                                        $invCrcString,
                                        $lipf7InitString,
                                        $lipf7RecvString,
                                        $lipf8InitString,
                                        $lipf8RecvString,
                                        $powerOnMinString );

                                    if ($version >= 1 && $devType == PD_DT_SBOD_SES)
                                    {
                                        $$destPtr .= sprintf( " %10s %10s %10s %10s %10s %10s  %10s",
                                            $WordErrorString,
                                            $CRCErrorString,
                                            $ClockDeltaString,
                                            $LoopUpString,
                                            $InsertionString,
                                            $StallString,
                                            $UtilizationString);
                                    }
                                    $$destPtr .= "\n";

                                    $hdrPrinted = 0;
                                    $portLinePrinted = 1;
                                    $bayLinePrinted = 1;
                                }
                            }
                            else
                            {
# Don't print anything for non-populated slots (per Steve Nowakowski)
#                                $$destPtr .= sprintf( "                              %3d %4d",
#                                    $bayCounter, $slotCounter );

#                                $$destPtr .= "\n";

                                $$offsetPtr += 82;
                                if ($version >= 1)
                                {
                                    $$offsetPtr += (28 + 28);
                                }
                            }
                        }
                    }
                }
            }

            if( $bayLinePrinted == 1 )
            {
                $$destPtr .= sprintf( "\n" );
            }
        }

        if( $portLinePrinted == 1 )
        {
            $$destPtr .= sprintf( "\n" );
        }
    }

    ###
    ## Move the offset to the end of the bayList
    ###
    $$offsetPtr = $startingOffset + $$counterHeaderPtr{SIZE_BAY_DATA};
}

sub FmtFCMCountersFID
{
    use constant LINE_SEP_LEN => 220;
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $fidStr, $address, $version) = @_;
    my %counterHeader;

    $$destPtr .= ( "-"  x LINE_SEP_LEN ) . "\nFCM Counters\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
    FmtFCMCounterMapHeader( $destPtr, \%counterHeader, $bufferPtr, \$offset );
    FmtFCMCounterMapFlags( $destPtr, \%counterHeader, $bufferPtr, \$offset );

    ## baselineMapValid
    if( $counterHeader{FLAGS} & (1 << 0) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nBaseline\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## deltaMapValid
    if( $counterHeader{FLAGS} & (1 << 1) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nUpdate\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## updateMapValid
    if( $counterHeader{FLAGS} & (1 << 2) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nDelta (current)\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## backup0MapValid
    if( $counterHeader{FLAGS} & (1 << 3) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nDelta (backup 0)\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## backup1MapValid
    if( $counterHeader{FLAGS} & (1 << 4) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nDelta (backup 1)\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## backup2MapValid
    if( $counterHeader{FLAGS} & (1 << 5) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nDelta (backup 2)\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }

    ## backup3MapValid
    if( $counterHeader{FLAGS} & (1 << 6) )
    {
        $$destPtr .= "\n\n" . ( "-"  x LINE_SEP_LEN ) . "\nDelta (backup 3)\n" . ( "-" x LINE_SEP_LEN ) . "\n\n";
        FmtFCMCounterErrorData( $destPtr, \%counterHeader, $bufferPtr, \$offset, $version );
    }
    else
    {
        $offset += $counterHeader{SIZE_ERROR_DATA};
    }
}

##############################################################################
#
#          Name: FmtMirrorPartnerList
# Call:
#   FmtMirrorPartnerList ( $destPtr, $hptr, $bufferPtr, $offset, $reqLength,
#                          $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
#           $hashPtr - pointer to return a hash with the data ( may be 0 )
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, cyurrently not
#                        used, but may be used if the data differs between
#                        processors
#           $address - Memory address where the data came from.
#
#  Return: GOOD or ERROR
#
# The data structure being decoded...
#
#
# typedef struct MR_HDR_RSP
# {
#    UINT8   rsvd0[3];       /**< RESERVED                                   */
#    UINT8   status;         /**< MRP Completion Status                      */
#    UINT32  len;            /**< Length of this return packet in bytes      */
# } MR_HDR_RSP;
#
# typedef struct MRGETMPLIST_RSP_INFO
# {
#    UINT32  source;         /**< Source mirror partner                      */
#    UINT32  dest;           /**< Destination mirror partner                 */
# } MRGETMPLIST_RSP_INFO;
#
# typedef struct MRGETMPLIST_RSP
# {
#    MR_HDR_RSP  header;     /**< MRP Response Header - 8 bytes              */
#    UINT16      count;      /**< Number of devices in the dynamic list      */
#    UINT8       rsvd10[2];  /**< RESERVED                                   */
#    MRGETMPLIST_RSP_INFO list[0]; /* Mirror partner list           */
# } MRGETMPLIST_RSP;
#
##############################################################################
sub FmtMirrorPartnerList
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;

    my $item1;
    my $item2;
    my $count;
    my $fmt;
    my $msg = "";

    ##########################################
    # Mirror Partner List
    ##########################################

    # --- Begin structure -------------------------------------------------

    # MR_HDR_RSP:

    $fmt = sprintf( "x%d C L", $offset+3 );      # generate the format string
    ($item1, $item2) =  unpack $fmt, $$bufferPtr;

    if ($item1 != 0)
    {
        $msg .= sprintf( "MRP Status: $item1\n");
        $msg .= sprintf( "Pkt Length: $item2\n");
    }

    $offset += 8;                         # add bytes processed

    # count & rsvd10

    $fmt = "x$offset S";
    ($count) = unpack $fmt, $$bufferPtr;

    if ($item1 != 0)
    {
        $msg .= sprintf( "List Count: $count\n\n");
    }

    $offset += 4;                         # add bytes processed

    # loop through the list
    $msg .= sprintf "Assigned Mirror Partners:\n";
    $msg .= sprintf "Source => Destination\n\n";

    while($count--)
    {
        $fmt = "x$offset L L";
        ($item1, $item2) = unpack $fmt, $$bufferPtr;

        $msg .= sprintf( "%u => %u   (0x%X => 0x%X)\n", $item1, $item2, $item1, $item2);

        $offset += 8;                         # add bytes processed
    }

    # finished
    $$destPtr .= $msg;

    return GOOD;
}

# ISCSI_CODE
##############################################################################
##############################################################################
=head2 FmtiSCSIStatsFID() function

Formats iSCSI Stats information. Returns GOOD upon completion. Data is
currently only printed to the screen as supporting functions do not save to
a string. This function uses the decoders from the ccbcl.

No hash data is returned. No string data is returned.

This is for FID 354.

=cut
##############################################################################
##############################################################################
sub FmtiSCSIStatsFID
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg = "\niSCSI Stats:\n";
    my %info;
    my $len;
    my $ret;
    my $idx = 0;
    my $header;
    my $buff;
    my %rp;
    my $pkt;
    my $length;
    my $disks;
    my $i;
    my $j;

    $length =  length($$bufferPtr);

    $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16",
                    128, ( $length + 4 ), 0, 1,
                    PI_ISCSI_SESSION_INFO, 45657, 0, 1053011891,
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¶|a+—MxT+Q4?Á?F',
                    '+¶¶+i^S—?ÈV‹=d•  ');

    $rp{'header'} = $header;


    # Need to walk the buffer to count the number of vdisks present.
    # Unfortunately, we do not have a vdisk count, and the data we are
    # presented with is likely much greater than the vdisk population,
    # so we traverse through and guess when we are at the end.
#    $disks = 0;
#    for ( $i = 0; $i < $length; $i += $j )
#    {
#        my $rcnt;
##        $fmt = sprintf("x%d x4 L x7 C",$i); # extract the length and raid count
#        ($j, $rcnt) = unpack($fmt, $$bufferPtr);
#        if ($j != 0x68 + (2 * $rcnt))
#        {
#            last;
#        }
#        $disks++;
#    }

#    $length = pack("L", $disks);

    # print " Counted $disks vdisks in the data \n";


    $rp{'data'} = $$bufferPtr;


#        %info = XIOTech::cmdMgr::_envStatsExtPacket(0, 0, \%rp);
#        $msg .= XIOTech::cmdMgr::displayEnvironmentalStatsExtended(0, %info);

    %info = XIOTech::cmdMgr::_iSCSIStatsPacket(0, 0, \%rp);
    $msg .= XIOTech::cmdMgr::displayiSCSIStats(0,  %info);


    # copy the data to the callers pointers

    # if there is a valid pointer, append the test to the item
    # referenced by the pointer

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    # if the hash pointer is valid update the contents with the
    # new hash (overwrite)

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    return GOOD;
}
# ISCSI_CODE

##############################################################################
#
# Name: FmtLinuxFileReads
# Call:
#   FmtLinuxFileReads ( $destPtr, $hptr, $bufferPtr, $offset, $reqLength,
#                          $processor, $address, $fid, $filen);
#
# Input:    $pDest     - pointer to a scalar for the ascii results
#           $pHash     - pointer to return a hash with the data ( may be 0 )
#           $pBuffer   - pointer to a buffer scalar with the data
#           $offset    - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, cyurrently not
#                        used, but may be used if the data differs between
#                        processors
#           $address   - Memory address where the data came from.
#           $fid       - fid number.
#           $outputFileName - file name.
#
#  Return: GOOD or ERROR
#
##############################################################################
sub FmtLinuxFileReads
{
    my ($pDest, $pHash, $pBuffer, $offset, $reqLength, $processor, $address, $fid, $outputFileName) = @_;
    my $length;
    my $fileBuf;
    my $inputHandle;
    my $outputHandle = *STDOUT;
    my $isArchiveFid = 0;
    my $currentDir = cwd;
    my $returnCode = GOOD;

    ##
    # Fids to fully decode
    ##
    if ( ($fid == 306) ||
         ($fid == 307) ||
         ($fid == 308) ||
         ($fid == 309) ||
         ($fid == 311) ||
         ($fid == 312) )
    {
        $isArchiveFid = 1;
    }

    ##
    # Archive FIDs must be untarred/uncompressed before being output
    ##
    if( $isArchiveFid )
    {
        ##
        # Warn if the archive file is being overwritten
        ##
        if( -e "$fid.archive" )
        {
            warn( "Overwriting existing file $fid.archive\n" );
        }

        $outputHandle = ();
        open($outputHandle, ">$fid.archive") or return ERROR;
        binmode $outputHandle, ":raw";
    }
    else
    {
        if( defined $outputFileName )
        {
            ##
            # Warn if the output file is being overwritten
            ##
            if( -e $outputFileName )
            {
                warn( "Overwriting existing file $outputFileName\n" );
            }

            $outputHandle = ();

            if( open($outputHandle, ">$outputFileName") )
            {
                binmode $outputHandle, ":raw";
            }
            else
            {
                printf STDERR "Unable to open $outputFileName for output\n";
                return ERROR;
            }
        }
    }

    ##
    # We need to remove the FID header, if one exists.  Start by
    # opening the file (buffer) that was created during the fidRead.
    ##
    open($inputHandle, "$$pBuffer") or return ERROR;
    binmode $inputHandle, ":raw";

    $length = read( $inputHandle, $fileBuf, 0x80000 );

    if( $length )
    {
        ##
        # Strip off the FID header, if one is found
        ##
        my( $hdrMagicNum, $hdrFid, $hdrVer, $hdrStartAddr, $hdrId ) =
            unpack( DDR_FID_HEADER, $fileBuf );

        if ($hdrMagicNum == DDR_FID_HEADER_MAGIC_NUM)
        {
            $fileBuf = substr( $fileBuf, 32 );
        }

        ##
        # Copy the file but leave off the header.  For non-archvive files,
        # this will go directly to the output file, but for tarred files,
        # this will go to a temp file to be processed below.
        ##
        do
        {
            $length = print $outputHandle $fileBuf;
            $length = read( $inputHandle, $fileBuf, 0x80000 );
        } while ( $length );
    }

    ##
    # Close the current file handles
    ##
    unlink( $$pBuffer );
    close $inputHandle;

    if( $outputHandle ne *STDOUT )
    {
        close $outputHandle;
        $outputHandle = *STDOUT;
    }

    ##
    # Untar the FID, if needed
    ##
    if( $isArchiveFid )
    {
        my $tarFd;
        my $tarDir = "tmpTar";
        my @tarFiles;

        ##
        # Remove the temp directory if it exists (fail quietly)
        ##
        if( -d $tarDir )
        {
            unlink( glob("$tarDir/*") );
            unlink( glob("$tarDir/*.*") );
            rmdir( $tarDir );
        }

        ##
        # Create the directory
        ##
        if( !(mkdir $tarDir) &&
            !(-d $tarDir) )
        {
            print "FAILED: mkdir $tarDir\n";
            return ERROR;
        }

        if( !(-e "$fid.archive") )
        {
            print "FAILED: -e $fid.archive\n";
            return ERROR;
        }

        ##
        # Decompress the file (two methods GZIP and BZIP2)
        # NOTE: At the end of this section, the file will be named $fid.tar
        ##
        print STDERR "Decompressing the archive...\n";
        if( rename("$fid.archive", "$fid.tgz") &&
            system("gzip -dvf $fid.tgz") == GOOD )
        {
            print STDERR "Decompressed (GZIP) the archive\n";
        }
        else
        {
            if( rename("$fid.tgz", "$fid.tbz") &&
                system("bzip2 -dvf $fid.tbz") == GOOD )
            {
                print STDERR "Decompressed (BZIP2) the archive\n";
            }
            else
            {
                print STDERR "Treating the archive as uncompressed\n";
                rename("$fid.tbz", "$fid.tar")
            }
        }

        ##
        # Untar the file
        ##
        if( system("tar -xf $fid.tar -C $tarDir") == GOOD )
        {
            ##
            # Open the final output file
            ##
            if( defined $outputFileName )
            {
                ##
                # Warn if the output file is being overwritten
                ##
                if( -e $outputFileName )
                {
                    warn( "Overwriting existing file $outputFileName\n" );
                }

                $outputHandle = ();

                if( open($outputHandle, ">$outputFileName") )
                {
                    binmode $outputHandle, ":raw";
                }
                else
                {
                    printf STDERR "Unable to open $outputFileName for output\n";
                    return ERROR;
                }
            }

            ##
            # Gather the list of files in the directory
            ##
            if( opendir(TAR, $tarDir) )
            {
                @tarFiles = readdir( TAR );
                closedir(TAR) or warn ("Bad directory close - ($!)\n");
            }
            else
            {
                printf "ERROR: opendir\n";
                return ERROR;
            }

            ##
            # Change into the tarball directory and process the individual files
            ##
            if( chdir $tarDir )
            {
                my @fileList;
                my @filesToWrite;
                my $fileHandle;

                ##
                # Decompress any compressed files (GZip and BZip2)
                ##
                foreach my $file (@tarFiles)
                {
                    ##
                    # Only attempt to decompress files (ignore directories)
                    ##
                    if( -f $file )
                    {
                        if( $file =~ s/\.gz$// )
                        {
                            if( system("gzip -df $file\.gz") != GOOD )
                            {
                                print( "GZip error unzipping $file\n" )
                            }
                        }
                        elsif( $file =~ s/\.bz2$// )
                        {
                            if( system("bzip2 -df $file\.bz2") != GOOD )
                            {
                                print( "BZip2 error unzipping $file\n" )
                            }
                        }

                        push @fileList, $file;
                    }
                    else
                    {
                        print( "Ignoring file: [$file]\n" );
                    }
                }

                ##
                # Read the files in the specified order, and write them to
                # the desired output file (or STDOUT).
                ##
                @filesToWrite = reverse sort @fileList;
                chdir $currentDir;

                foreach my $file (@filesToWrite)
                 {
                    my $fileBuf = ();

                    if( open $fileHandle, "$tarDir/$file" )
                    {
                        print "Recording information from file : $file\n";
                        print $outputHandle "\n******   File Start: $file   ******\n";

                        while( read($fileHandle, $fileBuf, 0x80000) )
                        {
                            print $outputHandle $fileBuf;
                        }

                        close $fileHandle;
                        $fileHandle = ();

                        print $outputHandle "\n******   File End: $file   ******\n";
                    }
                }
            }

            ##
            # Switch back to the starting directory and clean up the mess
            ##
            chdir $currentDir;
            unlink( "$fid.tar" );

            ##
            # Close output file handle
            ##
            if( $outputHandle ne *STDOUT )
            {
                close $outputHandle;
                $outputHandle = ();
            }
        }
        else
        {
            print "FAILED: Cannot decompress the archive\n";

            if( defined $outputFileName )
            {
                ##
                # Mutate the file name into something unique
                ##
                ($outputFileName) = split( m/\./, $outputFileName );
                $outputFileName =~ s/>//g;
                $outputFileName = "$outputFileName.$fid.archive";

                rename("$fid.tar", $outputFileName) or
                    warn( "Couldn't rename $fid.tar to $outputFileName\n" );
            }
            else
            {
                rename("$fid.tar", "$fid.archive") or
                    warn( "Couldn't rename $fid.tar to $fid.archive\n" );
            }

            $returnCode = ERROR;
        }

        ##
        # Remove the temp directory and working files
        ##
        unlink( glob("$tarDir/*") );
        unlink( glob("$tarDir/*.*") );
        rmdir( $tarDir ) or warn "Can't remove $tarDir ($!)";
    }

    return( $returnCode );
}

##############################################################################
#
# Name: FmtSESData
# Call:
#   FmtSESData ( $destPtr, $hptr, $bufferPtr, $offset, $reqLength,
#                   $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
#           $hashPtr - pointer to return a hash with the data ( may be 0 )
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, cyurrently not
#                        used, but may be used if the data differs between
#                        processors
#           $address - Memory address where the data came from.
#
#  Return: GOOD or ERROR
#
#  /*
#  **  Format of input buffer:
#  **
#  **  UINT32         count;           // number of bays found
#  **
#  **  -- The following two sections are repeated for each bay found --
#  **
#  **  UINT32         length;          // length of next section (eyecatcher + data)
#  **  CHAR           eyecatcher1[8];  // "SES_DEV "
#  **  SES_DEVICE     data1;           // the SES_DEVICE data structure
#  **
#  **  UINT32         length;          // length of next section (eyecatcher + data)
#  **  CHAR           eyecatcher2[8];  // "PAGE_02 "
#  **  SES_PAGE_02    data2;           // the PAGE 2 data
#  */
##############################################################################

#
# Element Types
#
use constant SES_ET_UNSPECIFIED => 0x00;        # Unspecified element type
use constant SES_ET_DEVICE      => 0x01;        # Device element type
use constant SES_ET_POWER       => 0x02;        # Power supply element type
use constant SES_ET_COOLING     => 0x03;        # Cooling element type
use constant SES_ET_TEMP_SENSOR => 0x04;        # Temp sensor element type
use constant SES_ET_DOOR_ALARM  => 0x05;        # Door alarm element type
use constant SES_ET_AUD_ALARM   => 0x06;        # Audible alarm element typ
use constant SES_ET_SES_ELEC    => 0x07;        # SES electronics element
use constant SES_ET_SCC_ELEC    => 0x08;        # SCC electronics element
use constant SES_ET_NV_CACHE    => 0x09;        # NV cache element type
                                                # 0x0A is reserved
use constant SES_ET_UPS         => 0x0B;        # UPS element type
use constant SES_ET_DISPLAY     => 0x0C;        # Display element type
use constant SES_ET_KEY_PAD     => 0x0D;        # Key pad element type
                                                # 0x0E is reserved
use constant SES_ET_SCSI_PORT   => 0x0F;        # SCSI port element type
use constant SES_ET_LANGUAGE    => 0x10;        # Language element type
use constant SES_ET_COMM_PORT   => 0x11;        # Comm port element type
use constant SES_ET_VOLT_SENSOR => 0x12;        # Volt sensor element type
use constant SES_ET_CURR_SENSOR => 0x13;        # Curr sensor element type
use constant SES_ET_TARGET      => 0x14;        # SCSI target element type
use constant SES_ET_INIT        => 0x15;        # SCSI intiator element type
use constant SES_ET_SUBENCL     => 0x16;        # Subenclosure element type
use constant SES_ET_LOOP_STAT   => 0x80;        # Loop status element type
use constant SES_ET_CTRL_STAT   => 0x82;        # Ctrlr status element type

use constant SES_EC_OTHER => # Type: anything other than those decoded below,
                             # including types 0x80 and 0x82
   "C       # CommonCtrl    /* Common control           */
    C       # byte1;        /* Byte 1                   */
    C       # byte2;        /* Byte 2                   */
    C";     # byte3;        /* Byte 3                   */

use constant SES_EC_DEVICE =>  # Type 0x01
   "C       # CommonCtrl    /* Common control           */
    C       # Slot;         /* Slot address             */
    C       # Ctrl1;        /* Control 1                */
    C";     # Ctrl2;        /* Control 2                */

use constant SES_EC_POWER =>  # Type 0x02
   "C       # CommonCtrl    /* Common control           */
    C       # rsvd;         /* Reserved                 */
    C       # Ctrl1;        /* Control 1                */
    C";     # Ctrl2;        /* Control 2                */

use constant SES_EC_COOLING =>  # Type 0x03
   "C       # CommonCtrl    /* Common control           */
    n       # FanSp;        /* Fan Speed (mask high bit)*/
    C";     # Ctrl3;        /* Control                  */

use constant SES_EC_TEMP =>     # Type 0x04
   "C       # CommonCtrl    /* Common control           */
    C       # Rsvd;         /* Reserved                 */
    C       # Temp;         /* Temperature              */
    C";     # Ctrl2;        /* Control                  */

use constant SES_EC_AUD_ALARM => # Type 0x06
   "C       # CommonCtrl    /* Common control           */
    C       # Rsvd;         /* Reserved                 */
    C       # Rsvd;         /* Reserved                 */
    C";     # Ctrl1;        /* Control                  */

use constant SES_EC_SES_ELEC => # Type 0x07
   "C       # CommonCtrl    /* Common control           */
    C       # Rsvd;         /* Reserved                 */
    C       # Select;       /* Select                   */
    C";     # Rsvd;         /* Reserved                 */

use constant SES_EC_VOLTAGE =>  # Type 0x12
   "C       # CommonCtrl    /* Common control           */
    C       # Ctrl;         /* Control                  */
    n";     # Voltage;      /* Voltage                  */

use constant SES_EC_CURRENT =>  # Type 0x13
   "C       # CommonCtrl    /* Common control           */
    C       # Ctrl;         /* Control                  */
    n";     # Current;      /* Current                  */

use constant SES_EC_GENERIC =>  # Types 0x80 and 0x82
   "C       # CommonCtrl    /* Common control           */
    C       # Slot;         /* Slot address             */
    C       # Ctrl1;        /* Control 1                */
    C";     # Ctrl2;        /* Control 2                */


use constant SES_PAGE_02 =>
   "C       # PageCode;     /* Page code (0x02)         */
    C       # Status;       /* Status - constants below */
    n       # Length;       /* Length to follow         */
    L";     # Generation;   /* Generation code          */
#   a*      # Control       /* Actual control elements  */

use constant SES_DEVICE_V0 =>
   "L       # *NextSES;           /* Next SES device          */
    NN      # WWN;                /* WWN of the SES device    */
    L       # SupportedPages;     /* Bit significant support  */
    L       # FCID;               /* Fibre channel ID         */
    L       # Generation;         /* Generation code          */
    C       # Channel;            /* Fibre channel adapter    */
    C       # devStat;            /* Device status            */
    S       # PID;                /* PID of the SES device    */
    S       # LUN;                /* Logical unit number      */
    S       # TotalSlots;         /* Total element slots      */
    L       # *OldPage2;          /* Previous page 2 reading  */
    a256    # Map[SES_ET_MAX_VAL+1];/* Map of type area       */
    a256    # Slots[SES_ET_MAX_VAL+1];/* Number of slots      */
    A4      # pd_rev[4];          /* revision                 */
    C";     # devType;            /* Device type              */

use constant SES_DEVICE_V1 =>
   "L       # *NextSES;           /* Next SES device          */
    NN      # WWN;                /* WWN of the SES device    */
    L       # SupportedPages;     /* Bit significant support  */
    L       # FCID;               /* Fibre channel ID         */
    L       # Generation;         /* Generation code          */
    C       # Channel;            /* Fibre channel adapter    */
    C       # devStat;            /* Device status            */
    S       # PID;                /* PID of the SES device    */
    S       # LUN;                /* Logical unit number      */
    S       # TotalSlots;         /* Total element slots      */
    L       # *OldPage2;          /* Previous page 2 reading  */
    a256    # Map[SES_ET_MAX_VAL+1];/* Map of type area       */
    a256    # Slots[SES_ET_MAX_VAL+1];/* Number of slots      */
    A4      # pd_rev[4];          /* revision                 */
    C       # devType;            /* Device type              */
    L       # *OldPage80;         /* Previous page 80 reading */
    L";     # *OldPage81;         /* Previous page 81 reading */

use constant SES_P80XTEXPort =>
   "C       # StateCode;          /* Overall Port Event Flags */ /* 14 */
    C       # WordErrorCount;     /* Word Error Count         */ /* 15 */
    C       # CRCErrorCount;      /* CRC Error Count          */ /* 16 */
    c       # ClockDelta;         /* Clock Delta              */ /* 17 */
    C       # LoopUpCount;        /* Loop Up Count            */ /* 18 */
    C       # InsertionCount;     /* Insertion Count          */ /* 19 */
    C       # StallCount;         /* Stall Count              */ /* 20 */
    C";     # Utilization;        /* Utilization              */ /* 21 */

use constant SES_P80_XTEX =>
   "C       # PageCode;           /* Page code (0x80 / 0x81)  */ /* 0 */
    C       # Status;             /* Status - constants below */ /* 1 */
    n       # Length;             /* Length to follow         */ /* 2-3 */
    L       # Generation;         /* Generation code          */ /* 4-7 */
    C       # SBODStatusPageSeq;  /* SBOD Status Page Sequence*/ /* 8 */
    C       # SystemStatus;       /* System Status bits       */ /* 9 */
    C       # HWRev;              /* Hardware Revision Level  */ /* 10 */
    C       # AllPortEventFlags;  /* Overall Port Event Flags */ /* 11 */
    C       # LIPPortValue;       /* LIP Port Value           */ /* 12 */
    C";     # LoopUpCount;        /* Loop up Count            */ /* 13 */
    # 20 SES_P80XTEXPort structs follow here

sub FmtSESData
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor,
         $address, $version ) = @_;
    my ($count, $bayn, $length, $eyecatcher);
    my $msg = "";

    ($count) = unpack "x$offset L", $$bufferPtr;
    $offset += 4;

    # First the count of bays
    $msg .= sprintf "Found $count Disk Bay(s) in the data.\n";

    $bayn = 0;
    while ($count--)
    {
        ###################################################################
        ##### SES_DEVICE
        ###################################################################
        # Get the length of this section.
        # length = eyecatcher + SES_DEVICE
        ($length) = unpack "x$offset L", $$bufferPtr;
        $offset += 4;

        # The length will always be at least 8 (for the eyecatcher),
        # but if it is exactly 8, there is nothing more to decode for this
        # section.
        if ($length < 8)
        {
            $msg .= sprintf "Something is wrong ... bailing out.\n";
            return 0;
        }

        # Next is the eyecatcher
        ($eyecatcher) = unpack "x$offset A8", $$bufferPtr;
        $offset += 8;
#        $msg .= sprintf "$eyecatcher".": $length bytes\n";

        my ($nextSes, $wwnhi, $wwnlo, $supportedPages, $fcid, $generation,
            $channel, $devStat, $pid, $lun, $totalSlots, $oldPage2,
            $map, $slots, $pd_rev, $devType, $oldPage80, $oldPage81);

        # If we got more bytes in this section, assume we got it all and
        # decode it.
        if ($length > 8)
        {
            if ($version == 0)
            {
                ($nextSes, $wwnhi, $wwnlo, $supportedPages, $fcid,
                    $generation, $channel, $devStat, $pid, $lun, $totalSlots,
                    $oldPage2, $map, $slots, $pd_rev, $devType) =
                unpack "x$offset".SES_DEVICE_V0, $$bufferPtr;
            }
            else
            {
                ($nextSes, $wwnhi, $wwnlo, $supportedPages, $fcid,
                    $generation, $channel, $devStat, $pid, $lun, $totalSlots,
                    $oldPage2, $map, $slots, $pd_rev, $devType,
                    $oldPage80, $oldPage81) =
                unpack "x$offset".SES_DEVICE_V1, $$bufferPtr;
            }

            $msg .= sprintf "\n";
            $msg .= sprintf "#####################################################\n";
            $msg .= sprintf "###############      Bay $bayn Data       ###############\n";
            $msg .= sprintf "#####################################################\n";
            $msg .= sprintf "\n";
            $msg .= sprintf "WWN:            %08X%08X\n", $wwnhi, $wwnlo;
            $msg .= sprintf "supportedPages: 0x%X\n", $supportedPages;
            $msg .= sprintf "FCID:           0x%X\n", $fcid;
            $msg .= sprintf "Generation:     0x%X\n", $generation;
            $msg .= sprintf "Channel:        0x%X\n", $channel;
            $msg .= sprintf "DevStat:        0x%X\n", $devStat;
            $msg .= sprintf "Pid:            %u\n", $pid;
            $msg .= sprintf "Lun:            %u\n", $lun;
            $msg .= sprintf "Total Slots     %u\n", $totalSlots;
            $msg .= sprintf "FW Version:     $pd_rev\n";
            $msg .= sprintf "Device Type:    0x%X\n", $devType;
            $msg .= sprintf "\n";
        }
        $offset += ($length - 8);

        ###################################################################
        ##### PAGE 02
        ###################################################################
        # Get the length of this section.
        # length = eyecatcher + PAGE2 data
        $length = unpack "x$offset L", $$bufferPtr;
        $offset += 4;

        # The length will always be at least 8 (for the eyecatcher),
        # but if it is exactly 8, there is nothing more to decode for this
        # section.
        if ($length < 8)
        {
            $msg .= sprintf "Something is wrong ... bailing out.\n";
            return 0;
        }

        # Next is the eyecatcher
        ($eyecatcher) = unpack "x$offset A8", $$bufferPtr;
        $offset += 8;
#        $msg .= sprintf "$eyecatcher".": $length bytes\n";

        # If we got more bytes in this section, assume we got it all and
        # decode it.
        if ($length > 8)
        {
            # Unpack the page 2 data (the first 8 bytes)
            my ($p2PageCode, $p2Status, $p2Length, $p2Generation) =
                unpack "x$offset".SES_PAGE_02, $$bufferPtr;
            $offset += 8;

            $msg .= sprintf "Page2 PageCode: 0x%X\n", $p2PageCode;
            $msg .= sprintf "Page2 Status:   0x%X\n", $p2Status;
            $msg .= sprintf "Page2 Length:   $p2Length\n";
            $msg .= sprintf "Page2 Generation: 0x%X\n", $p2Generation;
            $msg .= sprintf "\n";

            # Now suck in the control elements
            $length -= 16;
            my ($elements) = unpack "x$offset a$length", $$bufferPtr;
            $offset += $length;

            $msg .= fmt_display_elements($map, $slots, $elements);

        }
        else
        {
            $offset += 8;
        }

        if ($version == 0)
        {
            # finished
            $$destPtr .= $msg;

            return GOOD;
        }

        ###################################################################
        ##### PAGE 80 / 81
        ###################################################################
        my $loopCount = 0;

        while ($loopCount < 2)
        {
            # bail if out of data
            if (($offset + 4) > length($$bufferPtr))
            {
                $msg .= sprintf "Something is wrong ... bailing out.\n";
                return 0;
            }

            # Get the length of this section.
            # length = eyecatcher + PAGE80/81 data
            $length = unpack "x$offset L", $$bufferPtr;
            $offset += 4;

            # The length will always be at least 8 (for the eyecatcher),
            # but if it is exactly 8, there is nothing more to decode for this
            # section.
            if ($length < 8)
            {
                $msg .= sprintf "Something is wrong ... bailing out.\n";
                return 0;
            }

            # Next is the eyecatcher
            ($eyecatcher) = unpack "x$offset A8", $$bufferPtr;
            $offset += 8;
            $length -= 8;
#            $msg .= sprintf "$eyecatcher".": $length bytes\n";

            if ($length > 0)
            {
                $msg .= "\n". "#" x 75 ."\n";
                $msg .= "#### PAGE 8$loopCount DATA\n";
                $msg .= "#" x 75 . "\n\n";

                # Unpack the page 80 data (the first 14 bytes)
                my ($p80PageCode, $p80Status, $p80Length, $p80Generation,
                    $p80SystemStatus, $p80HWRev, $p80AllPortEventFlags,
                    $p80LIPPortValue, $p80LoopUpCount) =
                    unpack "x$offset".SES_P80_XTEX, $$bufferPtr;

                $msg .= sprintf "Page8%u PageCode:          0x%02X\n",
                    $loopCount, $p80PageCode;
                $msg .= sprintf "Page8%u Status:            0x%02X\n",
                    $loopCount, $p80Status;
                $msg .= sprintf "Page8%u Length:            %u\n",
                    $loopCount, $p80Length;
                $msg .= sprintf "Page8%u SBODStatusPageSeq: 0x%02X\n",
                    $loopCount, $p80Generation;
                $msg .= sprintf "Page8%u SystemStatus:      0x%02X\n",
                    $loopCount, $p80SystemStatus;
                $msg .= sprintf "Page8%u HWRev:             0x%02X\n",
                    $loopCount, $p80HWRev;
                $msg .= sprintf "Page8%u AllPortEventFlags: 0x%02X\n",
                    $loopCount, $p80AllPortEventFlags;
                $msg .= sprintf "Page8%u LIPPortValue:      0x%02X\n",
                    $loopCount, $p80LIPPortValue;
                $msg .= sprintf "Page8%u LoopUpCount:       0x%02X\n",
                    $loopCount, $p80LoopUpCount;
                $msg .= sprintf "\n";

                $offset += 14;
                $length -= 14;

                #
                # Now decode just the status byte for each port
                #
                $msg .= "Port Statistics:\n\n";
                $msg .= "Port      WordErrCnt        ClockDelta       InsertionCnt       Utilization\n";
                $msg .= "   StateCode      CRCErrorCnt       LoopUpCnt          StallCnt\n";
                $msg .= "-" x 75 . "\n";
            }

            my $i = 0;
            while ($length > 0)
            {
                # Unpack the port data
                my ($stateCode, $WordErrorCnt, $CRCErrorCount, $ClockDelta,
                    $LoopUpCount, $InsertionCnt, $StallCount, $Utilization) =
                        unpack "x$offset".SES_P80XTEXPort, $$bufferPtr;

                $msg .= sprintf
                    "%-4u 0x%02X %6u   %6u   %6d   %6u   %6u   %6u    %6u\n",
                    $i, $stateCode,
                    sbufp2int($WordErrorCnt),
                    sbufp2int($CRCErrorCount),
                    ($ClockDelta * 8),
                    $LoopUpCount,
                    $InsertionCnt,
                    $StallCount,
                    $Utilization;

                if ($i == 3)
                {
                    $msg .= "- " x 38 . "\n";
                }

                $offset += 8;
                $length -= 8;
                $i++;
            }

            $loopCount++;
        }

        $bayn++;
    }

    # finished
    $$destPtr .= $msg;

    return GOOD;
}

sub fmt_display_elements
{
    my ($map, $slots, $elements) = @_;
    my $msg = '';
    # Done with the easy part, now put it all together...
    for (my $eType=0; $eType<256; $eType++)
    {
        my ($eStart, $eCount, $eOffset);
        my @eData;
        
        $eStart = unpack "x$eType C", $map;
        next if ($eStart == 0xFF);

        $eCount = unpack "x$eType C", $slots;
        $eCount++;

        $msg .= sprintf "########################################\n";
        if ($eType == SES_ET_DEVICE)
        {
            $msg .= sprintf "######   Device Sensors (0x%02X)    ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_POWER)
        {
            $msg .= sprintf "##### Power Supply Sensors (0x%02X) ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_COOLING)
        {
            $msg .= sprintf "######   Cooling Sensors (0x%02X)   ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_AUD_ALARM)
        {
            $msg .= sprintf "##### Audible Alarm Sensors (0x%02X) #####\n",
            $eType;
        }
        elsif ($eType == SES_ET_SES_ELEC)
        {
            $msg .= sprintf "#### SES Electronics Sensors (0x%02X) ####\n",
            $eType;
        }
        elsif ($eType == SES_ET_TEMP_SENSOR)
        {
            $msg .= sprintf "###### Temperature Sensors (0x%02X) ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_VOLT_SENSOR)
        {
            $msg .= sprintf "######   Voltage Sensors (0x%02X)   ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_CURR_SENSOR)
        {
            $msg .= sprintf "######   Current Sensors (0x%02X)   ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_LOOP_STAT)
        {
            $msg .= sprintf "###### Loop Status Sensors (0x%02X) ######\n",
            $eType;
        }
        elsif ($eType == SES_ET_CTRL_STAT)
        {
            $msg .= sprintf "### Controller Status Sensors (0x%02X) ###\n",
            $eType;
        }
        elsif ($eType == SES_ET_DISPLAY)
        {
            $msg .= sprintf "######## Display Sensors (0x%02X) ########\n",
            $eType;
        }
        else
        {
            $msg .= sprintf "##### Other/Unknown Sensor (0x%02X)  #####\n",
            $eType;
        }
        $msg .= sprintf "########################################\n\n";


        $eOffset = $eStart * 4;
        for (my $i=0; $i<$eCount; $i++, $eOffset+=4)
        {
        
            if(length($elements) < ($eOffset + 4))
            {
                # If we are bailing out for this reason, something is
                # wrong, print an error message.
                $msg .= "\n\nPossible Error: we ran out of data before ".
                "we should have...bailing out!\n\n";
                last;
            }
        
            if ($eType == SES_ET_DEVICE)
            {
                @eData = unpack "x$eOffset".SES_EC_DEVICE, $elements;
                
                $msg .= sprintf "%s "."Device Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  slot:    0x%X\n".
                    "  ctrl1:   0x%X\n".
                    "  ctrl2:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }
        
            elsif ($eType == SES_ET_POWER)
            {
                @eData = unpack "x$eOffset".SES_EC_POWER, $elements;
                
                $msg .= sprintf "%s "."Power Supply Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  rsvd:    0x%X\n".
                    "  ctrl1:   0x%X\n".
                    "  ctrl2:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }
        
            elsif ($eType == SES_ET_COOLING)
            {
                @eData = unpack "x$eOffset".SES_EC_COOLING, $elements;
                
                $msg .= sprintf "%s "."Cooling Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  fanSp:   0x%X (%u RPM)\n".
                    "  ctrl:    0x%X\n".
                    "\n",
                        $eData[0],
                        $eData[1], ($eData[1] & ~0x8000) * 10,
                        $eData[2];
            }

            elsif ($eType == SES_ET_TEMP_SENSOR)
            {
                @eData = unpack "x$eOffset".SES_EC_TEMP, $elements;
                
                my ($degC, $degF);
                if ($eData[2])
                {
                    $degC = $eData[2] - 20;
                    $degF = $degC * 9/5 +32;
                }
                else
                {
                    $degC = $degF = 0;
                }
                
                $msg .= sprintf "%s "."Temp Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  rsvd:    0x%X\n".
                    "  temp:    0x%X (%d C, %d F)\n".
                    "  ctrl2:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $degC, $degF,
                        $eData[3];
            }
        
            elsif ($eType == SES_ET_AUD_ALARM)
            {
                @eData = unpack "x$eOffset".SES_EC_AUD_ALARM, $elements;
                
                $msg .= sprintf "%s "."Audible Alarm Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  rsvd:    0x%X\n".
                    "  rsvd:    0x%X\n".
                    "  ctrl1:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }
        
            elsif ($eType == SES_ET_SES_ELEC)
            {
                @eData = unpack "x$eOffset".SES_EC_SES_ELEC, $elements;
                
                $msg .= sprintf "%s "."SES Electronics Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  rsvd:    0x%X\n".
                    "  select:  0x%X\n".
                    "  rsvd:    0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }
        
            elsif ($eType == SES_ET_VOLT_SENSOR)
            {
                @eData = unpack "x$eOffset".SES_EC_VOLTAGE, $elements;
                
                $msg .= sprintf "%s "."Voltage Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  ctrl:    0x%X\n".
                    "  voltage: 0x%X (%.2f V)\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], ($eData[2] + 0.0)/100,
                        $eData[3];
            }
        
            elsif ($eType == SES_ET_CURR_SENSOR)
            {
                @eData = unpack "x$eOffset".SES_EC_CURRENT, $elements;
                
                $msg .= sprintf "%s "."Current Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  ctrl:    0x%X\n".
                    "  current: 0x%X (%.2f A)\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], ($eData[2] + 0.0)/100,
                        $eData[3];
            }
        
            elsif ($eType == SES_ET_LOOP_STAT)
            {
                @eData = unpack "x$eOffset".SES_EC_OTHER, $elements;
                
                $msg .= sprintf "%s "."Loop Status $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  byte1:   0x%X\n".
                    "  byte2:   0x%X\n".
                    "  byte3:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }

            elsif ($eType == SES_ET_CTRL_STAT)
            {
                @eData = unpack "x$eOffset".SES_EC_OTHER, $elements;
                
                $msg .= sprintf "%s "."Controller Status $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  byte1:   0x%X\n".
                    "  byte2:   0x%X\n".
                    "  byte3:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }

            elsif ($eType == SES_ET_DISPLAY)
            {
                @eData = unpack "x$eOffset".SES_EC_OTHER, $elements;
                
                $msg .= sprintf "%s "."Display Status $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  byte1:   0x%X\n".
                    "  byte2:   0x%X\n".
                    "  byte3:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }

            else
            {
                @eData = unpack "x$eOffset".SES_EC_OTHER, $elements;
                
                $msg .= sprintf "%s "."Non-decoded Element $i:\n",
                    $i?"Individual":"Overall";
                $msg .= sprintf
                    "  cmCtl:   0x%X\n".
                    "  byte1:   0x%X\n".
                    "  byte2:   0x%X\n".
                    "  byte3:   0x%X\n".
                    "\n",
                        $eData[0], $eData[1],
                        $eData[2], $eData[3];
            }
        }
    }
    return($msg);
}

sub sbufp2int
{
    my ($num) = @_;
    my $exp = ($num & 0xF0) >> 4;
    my $mant = $num & 0x0F;
    return $mant * (2 ** $exp);
}

##############################################################################
# The number of raids in the APOOL vdisk is 5.
use constant APOOL_MAX_ELEMENTS => 5;

# struct APOOL below.
use constant APOOL =>
   "S       # id;                     // ID of this apool.
    S       # status;                 // Status bit field.
    C       # percent_full;           // Set with fidread 355 is done.
    C       # version;                // Version of this record (firmware only)
    CC      # reserved[2]; 
    S       # cur_head_element;       // Element containing current head pointer.
    S       # cur_tail_element;       // Element containing current tail pointer.
    S       # cur_head_shadow_element; // Element containing current head shadow pointer.
    S       # cur_tail_shadow_element; // Element containing current tail shadow pointer.
    LL      # head_shadow;            // Head shadow pointer in sectors.
    LL      # tail_shadow;            // Tail shadow pointer in sectors.
    L       # *get_record_head;       // Pointer to first (oldest) get record.
    L       # *get_record_tail;       // Pointer to last (newest) get record.
    L       # *put_record_head;       // Pointer to first (oldest) put record.
    L       # *put_record_tail;       // Pointer to last (newest) put record.
    LL      # length;                 // Total size of this apool in sectors.
    LL      # sequence_count;         // Current sequence count.
    L       # time_threshold;         // Time bursting threshold.
    L       # mb_threshold;           // Mega Byte bursting threshold.
    L       # *mover_task_pcb;        // PCB of the mover for this apool.
    S       # alink_count;            // Number of alinks using this apool.
    S       # element_count;          // Number of elements in this apool.
    L       # *alink_head;            // Pointer to first alink struct.
    L";     # *alink_tail;            // Pointer to last alink struct.
# The MAX_ELEMENTS array of APOOL_ELEMENT's follow here.
use constant APOOL_length => 2+2+1+1+2+2+2+2+2 + 8+8 + 4+4+4+4 + 8+8 + 4+4+4+2+2 + 4+4;

# struct APOOL_ELEMENT below.
use constant APOOL_ELEMENT =>
    "S      # apool_id;               // ID of the owning apool.
     S      # status;                 // Current status bits .
     S      # vid;                    // VID where this element resides.
     LL     # length;                 // Length of this emement in sectors.
     LL     # sda;                    // SDA for this element.
     LL     # head;                   // Head pointer in sectors.
     LL";   # tail;                   // Tail pointer in sectors.
use constant APOOL_ELEMENT_length => 2+2+2 + 8+8+8+8;

# struct APOOL_LINK below.
use constant APOOL_LINK =>
    "L      # *next;                  // Pointer to next alink.
     S      # vid;                    // Associated vid .
     S      # status;
     LL     # sectors_outstanding;    // Number of sectors consumed in apool.
     LL     # last_sequence_count;    // Seq count of the last IO.
     LL     # last_time_stamp;        // Last time stamp to be removed from apool.
     S      # apool_id;               // ID of the owning apool.
     C";    # apool_percent_consumed; // Percent of apool consumed by this alink.
use constant APOOL_LINK_length => 4+2+2 + 8+8+8+2+1;

##############################################################################
#
# Name: FmtAsyncData
# Call:
#   FmtAsyncData ( $destPtr, $hptr, $bufferPtr, $offset, $reqLength,
#                   $processor, $address, $version);
#   fidread 355
#
# Input:    $destPtr - pointer to a scalar for the ascii results
#           $hashPtr - pointer to return a hash with the data ( may be 0 )
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, cyurrently not
#                        used, but may be used if the data differs between
#                        processors
#           $address - Memory address where the data came from.
#           $version - currently not used.
#
#  Return: GOOD or ERROR
#
#  /*
#  **  Format of input buffer:
#  **
#  **  UINT32         count;           // number of apools found
#  **
#  **  -- Following section repeated for each apool found --
#  **
#  **  UINT32         length;          // length of data.
#  **  CHAR           data[length];    // Apool data for async replication.
#  */
##############################################################################

sub FmtAsyncData
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor,
         $address, $version ) = @_;
    my ($apooln, $length, $data, $count);
    my $msg = "";

    ($count) = unpack("x$offset L", $$bufferPtr);
    $offset += 4;

    # First the count of apools.
    $msg .= sprintf "FmtAsyncData: $count Apool async replication(s) in the data.\n";

    $apooln = 0;
    while ($apooln < $count)
    {
        # length = amount of data for async replication.
        ($length) = unpack("x$offset L", $$bufferPtr);
        $offset += 4;

        # Unpack the APOOL structure.
        my ($id, $status, $percent_full, $apool_version, $r1, $r2,
            $cur_head_element, $cur_tail_element,
            $cur_head_shadow_element, $cur_tail_shadow_element,
            $head_shadow_lower, $head_shadow_upper,
            $tail_shadow_lower, $tail_shadow_upper,
            $get_record_head, $get_record_tail, $put_record_head, $put_record_tail,
            $length_lower, $length_upper, $sequence_count_lower, $sequence_count_upper,
            $time_threshold, $mb_threshold, $mover_task_pcb,
            $alink_count, $element_count, $alink_head, $alink_tail
           ) = unpack("x$offset" . APOOL, $$bufferPtr);
        $offset += APOOL_length;

        # We are up to APOOL structure's APOOL_ELEMENT element[MAX_ELEMENTS];
        my (@ape_apool_id, @ape_status, @ape_vid,
            @ape_length_lower, @ape_length_upper,
            @ape_sda_lower, @ape_sda_upper,
            @ape_head_lower, @ape_head_upper,
            @ape_tail_lower, @ape_tail_upper);

        for (my $i = 0; $i < APOOL_MAX_ELEMENTS; $i++)
        {
            ($ape_apool_id[$i], $ape_status[$i], $ape_vid[$i],
             $ape_length_lower[$i], $ape_length_upper[$i],
             $ape_sda_lower[$i], $ape_sda_upper[$i], 
             $ape_head_lower[$i], $ape_head_upper[$i],
             $ape_tail_lower[$i], $ape_tail_upper[$i]
            ) = unpack("x$offset" . APOOL_ELEMENT, $$bufferPtr);
            $offset += APOOL_ELEMENT_length;
        }

        # The ALINKs follow, we have $alink_count of them.
        my (@apl_next, @apl_vid, @apl_status,
            @apl_sectors_outstanding_lower, @apl_sectors_outstanding_upper,
            @apl_last_sequence_count_lower, @apl_last_sequence_count_upper,
            @apl_last_time_stamp_lower, @apl_last_time_stamp_upper,
            @apl_apool_id, @apl_apool_percent_consumed);

        for (my $i = 0; $i < $alink_count; $i++)
        {
            ($apl_next[$i], $apl_vid[$i], $apl_status[$i],
             $apl_sectors_outstanding_lower[$i], $apl_sectors_outstanding_upper[$i],
             $apl_last_sequence_count_lower[$i], $apl_last_sequence_count_upper[$i], 
             $apl_last_time_stamp_lower[$i], $apl_last_time_stamp_upper[$i],
             $apl_apool_id[$i], $apl_apool_percent_consumed[$i]
            ) = unpack("x$offset" . APOOL_LINK, $$bufferPtr);
            $offset += APOOL_LINK_length;
        }

        #..............................................................................
        $msg .= sprintf("  %u: Data of length (%u)\n", $apooln, $length);
        $msg .= sprintf("     id=%u status=%u percent_full=%u, length=%u mover_task_pcb=0x%8.8x\n",
                              $id, $status, $percent_full,
                              ($length_upper << 32) | $length_lower,
                              $mover_task_pcb);
        $msg .= sprintf("     cur_head_element=%u cur_tail_element=%u\n",
                              $cur_head_element, $cur_tail_element);
        $msg .= sprintf("     cur_head_shadow_element=%u cur_tail_shadow_element=%u\n",
                              $cur_head_shadow_element, $cur_tail_shadow_element);
        $msg .= sprintf("     head_shadow=%u tail_shadow=%u\n",
                              ($head_shadow_upper << 32) | $head_shadow_lower,
                              ($tail_shadow_upper << 32) | $tail_shadow_lower);
        $msg .= sprintf("     sequence_count=%u time_threshold=%u mb_threshold=%u\n",
                              ($sequence_count_upper << 32) | $sequence_count_lower,
                              $time_threshold, $mb_threshold);
        $msg .= sprintf("     alink_count=%u, element_count=%u, alink_head=%u, alink_tail=%u\n",
                              $alink_count, $element_count, $alink_head, $alink_tail);
        #..............................................................................
        if ($element_count == 0)
        {
            $msg .= sprintf("  No elements in use.\n");
        }
        for (my $i = 0; $i < $element_count; $i++)
        {
            $msg .= sprintf("  Element %u: apool_id=%u, status=%u, vid=%u, length=%u\n",
                            $i, $ape_apool_id[$i], $ape_status[$i], $ape_vid[$i],
                            ($ape_length_upper[$i] << 32) | $ape_length_lower[$i]);
            $msg .= sprintf("             sda=%u, head=%u, tail=%u\n",
                            ($ape_sda_upper[$i] << 32) | $ape_sda_lower[$i],
                            ($ape_head_upper[$i] << 32) | $ape_head_lower[$i],
                            ($ape_tail_upper[$i] << 32) | $ape_tail_lower[$i]);
        }
        #..............................................................................
        if ($alink_count == 0)
        {
            $msg .= sprintf("  No alinks in use.\n");
        }
        for (my $i = 0; $i < $alink_count; $i++)
        {
            $msg .= sprintf("  Alink %u: next=0x%8.8x, vid=%u, status=%u\n",
                            $i, $apl_next[$i], $apl_vid[$i], $apl_status[$i]);
            $msg .= sprintf("           sectors_outstanding=%u, last_sequence_count=%u\n",
        ($apl_sectors_outstanding_upper[$i] << 32) | $apl_sectors_outstanding_lower[$i],
        ($apl_last_sequence_count_upper[$i] << 32) | $apl_last_sequence_count_lower[$i]);
            $msg .= sprintf("           last_time_stamp=%u, apool_percent_consumed=%u, id=%u\n",
        ($apl_last_time_stamp_upper[$i] << 32) | $apl_last_time_stamp_lower[$i],
        $apl_apool_percent_consumed[$i], $apl_apool_id[$i]);
        }
        #..............................................................................
        $apooln++;
    }

    # finished
    $$destPtr .= $msg;

    return GOOD;
}

##############################################################################

#
#   The following functions represent StatsEnv for R1 only. These are here
#   to support decoding R1 dumps
#

##############################################################################




# I2C Monitor Status Codes
#use constant I2C_MONITOR_STATUS_CODE_UNKNOWN               => 0;
#use constant I2C_MONITOR_STATUS_CODE_NOT_PRESENT           => 1;
#use constant I2C_MONITOR_STATUS_CODE_VALID                     => 2;
#use constant I2C_MONITOR_STATUS_CODE_BUSY                  => 3;
#use constant I2C_MONITOR_STATUS_CODE_NOT_READY                 => 4;
#use constant I2C_MONITOR_STATUS_CODE_ERROR                 => 5;

#use constant I2C_MONITOR_TEMPERATURE_CONDITION_UNKNOWN         => 0;
#use constant I2C_MONITOR_TEMPERATURE_CONDITION_NORMAL      => 1;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_COOL         => 2;
#use constant I2C_MONITOR_TEMPERATURE_CONDITION_HOT             => 3;
#use constant I2C_MONITOR_TEMPERATURE_CONDITION_HOT_CRITICAL => 4;

#use constant I2C_MONITOR_LIMIT_MONITOR_UNKNOWN                 => 0;
#use constant I2C_MONITOR_LIMIT_MONITOR_GOOD                => 1;
#use constant I2C_MONITOR_LIMIT_MONITOR_TRIPPED                 => 2;

#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_UNKNOWN    => 0;
#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_GOOD       => 1;
#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE => 2;
#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_DC_FAILED  => 3;
#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_AC_FAILED  => 4;
#use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_NOT_PRESENT => 5;

#use constant I2C_MONITOR_COOLING_FAN_CONDITION_UNKNOWN         => 0;
#use constant I2C_MONITOR_COOLING_FAN_CONDITION_GOOD        => 1;
#use constant I2C_MONITOR_COOLING_FAN_CONDITION_FAILED      => 2;
#use constant I2C_MONITOR_COOLING_FAN_CONDITION_NOT_PRESENT     => 3;
#
#use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_UNKNOWN     => 0;
#use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_RUNNING     => 1;
#use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_RESET   => 2;
#
#use constant I2C_MONITOR_BATTERY_CONDITION_UNKNOWN             => 0;
#use constant I2C_MONITOR_BATTERY_CONDITION_GOOD            => 1;
#use constant I2C_MONITOR_BATTERY_CONDITION_LOW_CAPACITY    => 2;
#use constant I2C_MONITOR_BATTERY_CONDITION_UNDER_VOLTAGE   => 3;
#use constant I2C_MONITOR_BATTERY_CONDITION_OVER_VOLTAGE    => 4;
#use constant I2C_MONITOR_BATTERY_CONDITION_NOT_PRESENT         => 5;
#
#use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_UNKNOWN    => 0;
#use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_GOOD       => 1;
#use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_ABNORMAL   => 2;
#
#use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_UNKNOWN      => 0;
#use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_GOOD             => 1;
#use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_SHUTDOWN         => 2;
#
#use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_UNKNOWN  => 0;
#use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_OPERATIONAL => 1;
#use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR => 2;
#use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD => 3;

#use constant I2C_MONITOR_CHARGER_CONDITION_UNKNOWN             => 0;
#use constant I2C_MONITOR_CHARGER_CONDITION_IDLE            => 1;
#use constant I2C_MONITOR_CHARGER_CONDITION_TRICKLE             => 2;
#use constant I2C_MONITOR_CHARGER_CONDITION_BULK            => 3;
#use constant I2C_MONITOR_CHARGER_CONDITION_OVER            => 4;
#use constant I2C_MONITOR_CHARGER_CONDITION_TOPOFF          => 5;
#
#use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_UNKNOWN   => 0;
#use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_GOOD      => 1;
#use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_FAILED    => 2;
#
#use constant I2C_MONITOR_EEPROM_CONDITION_UNKNOWN          => 0;
#use constant I2C_MONITOR_EEPROM_CONDITION_GOOD                 => 1;
#use constant I2C_MONITOR_EEPROM_CONDITION_BAD_CRC          => 2;
#use constant I2C_MONITOR_EEPROM_CONDITION_NOT_READABLE      => 3;
#
#use constant PERSISTENT_DATA_OPTION_READ                    => 0x00;
#use constant PERSISTENT_DATA_OPTION_WRITE                   => 0x01;
#use constant PERSISTENT_DATA_OPTION_RESET                   => 0x02;
#use constant PERSISTENT_DATA_OPTION_CHECKSUM                => 0x03;
#






##############################################################################

##############################################################################
# Name:  R1_envStatsExtPacket
#
# Desc: Parses the system information packet and places the information in a
#       hash
#
# In:   scalar  $sequenceID         Sequence id
#       scalar  $recvPacket         Packet to parse (otherwise we go get one)
#
# Returns: Empty hash on error, else a hash with the following elements:
#
##############################################################################
sub R1_envStatsExtPacket
{
    my ($self, $seq, $recvPacket) = @_;

    my %envStats;

    if (!(defined($recvPacket)))
    {
        $recvPacket = $self->_receivePacketSync($seq);
    }

################################################################################
##  /* Things pertaining to condition reporting of the object */
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##
##  /* Things pertaining to condition of the object */
##    CCB_STATUS ccbStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      NVRAM_BATTERY_STATUS nvramBatteryStatus;
##      EEPROM_STATUS ccbBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##      EEPROM_STATUS ccbMemoryModuleEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    PROC_BOARD_STATUS procBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      POWER_SUPPLY_VOLTAGES_STATUS powerSupplyVoltagesStatus;
##      PROC_BOARD_PROCESSOR_STATUS frontEndProcessorStatus;
##      PROC_BOARD_PROCESSOR_STATUS backEndProcessorStatus;
##      EEPROM_STATUS chassisEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##      EEPROM_STATUS procBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    POWER_SUPPLY_STATUS frontEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      UINT8 powerSupplyConditionValue;
##      UINT8 coolingFanConditionValue;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    POWER_SUPPLY_STATUS backEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      UINT8 powerSupplyConditionValue;
##      UINT8 coolingFanConditionValue;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      TEMPERATURE_STATUS temperatureStatus;
##      BATTERY_STATUS batteryStatus;
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
##      CHARGER_STATUS chargerStatus;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
##    BUFFER_BOARD_STATUS backEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##      TEMPERATURE_STATUS temperatureStatus;
##      BATTERY_STATUS batteryStatus;
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
##      CHARGER_STATUS chargerStatus;
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
##        UINT8 eepromCondition;
##        XCI_DATA xciData;
################################################################################
    my $I2C_MONITOR_STATUS_CODE                     = "L";
    my $I2C_MONITOR_EVENT_STATISTICS                = "LLL";
    my $I2C_MONITOR_EVENT_PROPERTIES_FLAG           = "C";

    my $I2C_MONITOR_EVENT_PROPERTIES                = $I2C_MONITOR_STATUS_CODE .
                                                      $I2C_MONITOR_EVENT_STATISTICS .
                                                      $I2C_MONITOR_EVENT_PROPERTIES_FLAG;

    my $XCI_DATA                                    = "a64";

    my $EEPROM_STATUS                               = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C" .             # eepromCondition
                                                      $XCI_DATA;

    my $NVRAM_BATTERY_STATUS                        = $I2C_MONITOR_EVENT_PROPERTIES .
                                                       "C";             # nvramBatteryCondition

    my $CCB_STATUS                                  = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $NVRAM_BATTERY_STATUS .
                                                      $EEPROM_STATUS .  # ccbBoardEEPROMStatus
                                                      $EEPROM_STATUS;   # ccbMemoryModuleEEPROMStatus

    my $MILLIVOLTS                                  = "S";
    my $DEGREES_CELSIUS                             = "c";

    my $TEMPERATURE_STATUS                          = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $DEGREES_CELSIUS .
                                                      $DEGREES_CELSIUS .
                                                      $DEGREES_CELSIUS .
                                                      "C";              # conditionValue

    my $VOLTAGE_INPUT_READING                       = $MILLIVOLTS .
                                                      $MILLIVOLTS .
                                                      $MILLIVOLTS .
                                                      "C";              # limitMonitorValue

    my $POWER_SUPPLY_STATUS                         = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C" .             # powerSupplyConditionValue
                                                      "C" .             # coolingFanConditionValue
                                                      $EEPROM_STATUS .  # assemblyEEPROMStatus
                                                      $EEPROM_STATUS;   # interfaceEEPROMStatus

    my $POWER_SUPPLY_VOLTAGES_STATUS                = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING;

    my $PROC_BOARD_PROCESSOR_STATUS                 = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $TEMPERATURE_STATUS .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # processorResetConditionValue

    my $PROC_BOARD_STATUS                           = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $POWER_SUPPLY_VOLTAGES_STATUS .
                                                      $PROC_BOARD_PROCESSOR_STATUS .
                                                      $PROC_BOARD_PROCESSOR_STATUS .
                                                      $EEPROM_STATUS .  # chassisEEPROMStatus
                                                      $EEPROM_STATUS;   # procBoardEEPROMStatus

    my $BATTERY_STATUS                              = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # batteryCondition

    my $FUEL_GAUGE_STATUS                           = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "L" .             # currentFlowRate
                                                      $VOLTAGE_INPUT_READING .
                                                      "C" .             # fuelGaugeCondition
                                                      "C";              # currentFlowCondition

    my $MAIN_REGULATOR_STATUS                       = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      $VOLTAGE_INPUT_READING .
                                                      "C";              # mainRegulatorCondition

    my $CHARGER_STATUS                              = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      "C";              # chargerCondition

    my $BATTERY_SDIMM_STATUS                        = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $TEMPERATURE_STATUS .
                                                      $BATTERY_STATUS .
                                                      $FUEL_GAUGE_STATUS .
                                                      $MAIN_REGULATOR_STATUS .
                                                      $CHARGER_STATUS .
                                                      $EEPROM_STATUS;

    my $I2C_MONITOR_STATUS                          = $I2C_MONITOR_EVENT_PROPERTIES .
                                                      $CCB_STATUS .
                                                      $PROC_BOARD_STATUS .
                                                      $POWER_SUPPLY_STATUS .
                                                      $POWER_SUPPLY_STATUS .
                                                      $BATTERY_SDIMM_STATUS .
                                                      $BATTERY_SDIMM_STATUS;
    if (defined($recvPacket))
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        if (commandCode($recvPacket) == PI_STATS_ENVIRONMENTAL_CMD)
        {
            $envStats{STATUS} = $parts{STATUS};
            $envStats{ERROR_CODE} = $parts{ERROR_CODE};
##            print "Data Length: $parts{DATA_LENGTH}\n";
##            print "I2C_MONITOR_STATUS: $I2C_MONITOR_STATUS\n";

            (
##    I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{MONITOR_EVENT_PROP_STATUS},
            $envStats{MONITOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{MONITOR_EVENT_PROP_STATS_WARNING},
            $envStats{MONITOR_EVENT_PROP_STATS_ERROR},
            $envStats{MONITOR_EVENT_PROP_FLAG_VALUE},
##    CCB_STATUS ccbStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_EVENT_PROP_STATUS},
            $envStats{CCB_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_EVENT_PROP_FLAG_VALUE},
##      NVRAM_BATTERY_STATUS nvramBatteryStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATUS},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_WARNING},
            $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_ERROR},
            $envStats{NVRAM_BATTERY_EVENT_PROP_FLAG_VALUE},
##        UINT8 nvramBatteryCondition;
            $envStats{NVRAM_BATTERY_CONDITION},
##      EEPROM_STATUS ccbBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{CCB_BOARD_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{CCB_BOARD_EEPROM_XCI_DATA},
##      EEPROM_STATUS ccbMemoryModuleEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATUS},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{CCB_MEMORY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{CCB_MEMORY_EEPROM_XCI_DATA},
##    PROC_BOARD_STATUS procBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_EVENT_PROP_FLAG_VALUE},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_FLAG_VALUE},
##      POWER_SUPPLY_VOLTAGES_STATUS powerSupplyVoltagesStatus;
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_LMV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MAX_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MIN_MV},
            $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_LMV},
##      PROC_BOARD_PROCESSOR_STATUS frontEndProcessorStatus;
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_FLAG_VALUE},

            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_FLAG_VALUE},

            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MAX_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CURRENT_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MIN_CELSIUS},
            $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CONDITION},

            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MAX_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MIN_MV},
            $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_LMV},

            $envStats{PROC_BOARD_STATUS_FE_PROC_PRCV},
##      PROC_BOARD_PROCESSOR_STATUS backEndProcessorStatus;
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_FLAG_VALUE},

            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_FLAG_VALUE},

            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MAX_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CURRENT_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MIN_CELSIUS},
            $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CONDITION},

            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MAX_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_CURRENT_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MIN_MV},
            $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_LMV},

            $envStats{PROC_BOARD_STATUS_BE_PROC_PRCV},
##      EEPROM_STATUS chassisEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{PROC_CHASSIS_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{PROC_CHASSIS_EEPROM_XCI_DATA},
##      EEPROM_STATUS procBoardEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATUS},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{PROC_BOARD_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{PROC_BOARD_EEPROM_XCI_DATA},
##    POWER_SUPPLY_STATUS frontEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE},
##      UINT8 powerSupplyConditionValue;
            $envStats{FE_POWER_SUPPLY_STATUS_PSCV},
##      UINT8 coolingFanConditionValue;
            $envStats{FE_POWER_SUPPLY_STATUS_CFCV},
##      EEPROM_STATUS assemblyEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA},
##      EEPROM_STATUS interfaceEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA},
##    POWER_SUPPLY_STATUS backEndPowerSupply;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE},
##      UINT8 powerSupplyConditionValue;
            $envStats{BE_POWER_SUPPLY_STATUS_PSCV},
##      UINT8 coolingFanConditionValue;
            $envStats{BE_POWER_SUPPLY_STATUS_CFCV},
##      EEPROM_STATUS assemblyEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA},
##      EEPROM_STATUS interfaceEEPROMStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA},
##    BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE},
##      TEMPERATURE_STATUS temperatureStatus;
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE},

            $envStats{FE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
            $envStats{FE_BATT_SDIMM_STATUS_TEMP_CONDITION},
##      BATTERY_STATUS batteryStatus;
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE},

            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_BATT_CONDITION},
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV},

            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION},
            $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION},
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE},

            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV},
            $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION},
##      CHARGER_STATUS chargerStatus;
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE},

            $envStats{FE_BATT_SDIMM_STATUS_CHARGER_CONDITION},
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{FE_BATT_SDIMM_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{FE_BATT_SDIMM_EEPROM_XCI_DATA},
##    BUFFER_BOARD_STATUS backEndBufferBoardStatus;
##      I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE},
##      TEMPERATURE_STATUS temperatureStatus;
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE},

            $envStats{BE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
            $envStats{BE_BATT_SDIMM_STATUS_TEMP_CONDITION},
##      BATTERY_STATUS batteryStatus;
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE},

            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_BATT_CONDITION},
##      FUEL_GAUGE_STATUS fuelGaugeStatus;
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV},

            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION},
            $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION},
##      MAIN_REGULATOR_STATUS mainRegulatorStatus;
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE},

            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV},
            $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION},
##      CHARGER_STATUS chargerStatus;
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE},

            $envStats{BE_BATT_SDIMM_STATUS_CHARGER_CONDITION},
##      EEPROM_STATUS eepromStatus;
##        I2C_MONITOR_EVENT_PROPERTIES eventProperties;
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_COMPONENT_ID},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR},
            $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE},
##        UINT8 eepromCondition;
            $envStats{BE_BATT_SDIMM_EEPROM_CONDITION},
##        XCI_DATA xciData;
            $envStats{BE_BATT_SDIMM_EEPROM_XCI_DATA}) =
                unpack($I2C_MONITOR_STATUS, $parts{DATA});
        }
        else
        {
            $self->_handleError($recvPacket);
            logMsg("Unexpected packet: We expected a system info. packet\n");
        }
    }

    return %envStats;
}

##############################################################################
# Name: displayEnvironmentalStatsExtended
#
# Desc: Print the environmental statistics information
#
# In:   Environmental Statistics Hash
#
##############################################################################
sub R1displayEnvironmentalStatsExtended
{
    my ($self, %envStats) = @_;

    my $msg = "";

    logMsg( "displayEnvironentalStatsExtended...begin\n" );

    $msg .= sprintf( "Monitor Status Information ----------------------\n" );
    $msg .= sprintf( "  Event Status Code:                        %s\n", R1_getI2CMonitorStatusCodeString($envStats{MONITOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "    Warning:                                %d\n", $envStats{MONITOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "    Error:                                  %d\n", $envStats{MONITOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "    Flags:                                  %s\n", R1_getI2CMonitorEventFlagsString($envStats{MONITOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- CCB Status -----------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{CCB_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{CCB_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "---- NVRAM Battery Status -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{NVRAM_BATTERY_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{NVRAM_BATTERY_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{NVRAM_BATTERY_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorNVRAMBatteryConditionString($envStats{NVRAM_BATTERY_CONDITION}) );
    $msg .= sprintf( "---- Board EEPROM Status ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{CCB_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{CCB_BOARD_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{CCB_BOARD_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Memory EEPROM Status -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{CCB_MEMORY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{CCB_MEMORY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{CCB_MEMORY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{CCB_MEMORY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Processor Board ------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "---- Power Supply Voltages ----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Voltage Levels\n" );
    $msg .= sprintf( "      12 Volt: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_12VOLT_MAX_MV} );
    $msg .= sprintf( "      5 Volt: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_MAX_MV} );
    $msg .= sprintf( "      3.3 Volt: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_3VOLT_MAX_MV} );
    $msg .= sprintf( "      5 Volt Standby: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_LMV}),
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_POWER_SUPPLY_5VOLT_STANDBY_MAX_MV} );
    $msg .= sprintf( "---- Front End Processor ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_FE_PROC_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature Event Status Code:          %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CURRENT_CELSIUS},
                    R1_getI2CMonitorTemperatureConditionString($envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_CONDITION}),
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MIN_CELSIUS},
                    $envStats{PROC_BOARD_STATUS_FE_PROC_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "    DIMM Socket Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_LMV}),
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_FE_PROC_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Reset Condition: %s\n", R1_getI2CMonitorProcessorResetConditionString($envStats{PROC_BOARD_STATUS_FE_PROC_PRCV} ) );
    $msg .= sprintf( "---- Back End Processor -------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_BE_PROC_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature Event Status Code:          %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CURRENT_CELSIUS},
                    R1_getI2CMonitorTemperatureConditionString($envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_CONDITION}),
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MIN_CELSIUS},
                    $envStats{PROC_BOARD_STATUS_BE_PROC_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "    DIMM Socket Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_LMV}),
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MIN_MV},
                    $envStats{PROC_BOARD_STATUS_BE_PROC_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Reset Condition: %s\n", R1_getI2CMonitorProcessorResetConditionString($envStats{PROC_BOARD_STATUS_BE_PROC_PRCV}) );
    $msg .= sprintf( "---- Chassis EEPROM -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_CHASSIS_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{PROC_CHASSIS_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{PROC_CHASSIS_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Board EEPROM -------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{PROC_BOARD_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{PROC_BOARD_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{PROC_BOARD_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{PROC_BOARD_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Front End Power Supply -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Supply Condition:                       %s\n", R1_getI2CMonitorPowerSupplyConditionString($envStats{FE_POWER_SUPPLY_STATUS_PSCV}) );
    $msg .= sprintf( "    Cooling Fan Condition:                  %s\n", R1_getI2CMonitorCoolingFanConditionString($envStats{FE_POWER_SUPPLY_STATUS_CFCV}) );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{FE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{FE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Back End Power Supply ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Supply Condition:                       %s\n", R1_getI2CMonitorPowerSupplyConditionString($envStats{BE_POWER_SUPPLY_STATUS_PSCV}) );
    $msg .= sprintf( "    Cooling Fan Condition:                  %s\n", R1_getI2CMonitorCoolingFanConditionString($envStats{BE_POWER_SUPPLY_STATUS_CFCV}) );
    $msg .= sprintf( "---- Assembly EEPROM ----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{BE_POWER_SUPPLY_ASSEMBLY_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "---- Interface Board EEPROM ---------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{BE_POWER_SUPPLY_INTERFACE_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Front End Buffer Board -----------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "---- Temperature --------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
                    R1_getI2CMonitorTemperatureConditionString($envStats{FE_BATT_SDIMM_STATUS_TEMP_CONDITION}),
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
                    $envStats{FE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "---- Battery ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Terminal Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorBatteryConditionString($envStats{FE_BATT_SDIMM_STATUS_BATT_CONDITION}) );
    $msg .= sprintf( "---- Fuel Gauge ---------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Flow Rate:                              0x%08x\n", $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR} );
    $msg .= sprintf( "    Regulator Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Current Flow Condition:                 %s\n", R1_getI2CMonitorCurrentFlowConditionString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION}) );
    $msg .= sprintf( "    Fuel Gauge Condition:                   %s\n", R1_getI2CMonitorFuelGaugeConditionString($envStats{FE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION}) );
    $msg .= sprintf( "---- Main Regulator -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Input Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV} );
    $msg .= sprintf( "    Output Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV} );
    $msg .= sprintf( "    Supply Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV}),
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
                    $envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorMainRegulatorConditionString($envStats{FE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION}) );
    $msg .= sprintf( "---- Charger -------------------------------------\n" );
    $msg .= sprintf( "      Event Status Code:                    %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "        Warning:                            %d\n", $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "        Error:                              %d\n", $envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "        Flags:                              %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "      Condition:                            %s\n", R1_getI2CMonitorChargerConditionString($envStats{FE_BATT_SDIMM_STATUS_CHARGER_CONDITION}) );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{FE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{FE_BATT_SDIMM_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{FE_BATT_SDIMM_EEPROM_XCI_DATA}) );
    $msg .= sprintf( "*************************************************\n" );
    $msg .= sprintf( "-- Back End Buffer Board ------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "---- Temperature --------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_TEMP_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Temperature: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_CURRENT_CELSIUS},
                    R1_getI2CMonitorTemperatureConditionString($envStats{BE_BATT_SDIMM_STATUS_TEMP_CONDITION}),
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_MIN_CELSIUS},
                    $envStats{BE_BATT_SDIMM_STATUS_TEMP_MAX_CELSIUS} );
    $msg .= sprintf( "---- Battery ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_BATT_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Terminal Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_BATT_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorBatteryConditionString($envStats{BE_BATT_SDIMM_STATUS_BATT_CONDITION}) );
    $msg .= sprintf( "---- Fuel Gauge ---------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Flow Rate:                              0x%08x\n", $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CFR} );
    $msg .= sprintf( "    Regulator Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_VOLTAGE_MAX_MV} );
    $msg .= sprintf( "    Current Flow Condition:                 %s\n", R1_getI2CMonitorCurrentFlowConditionString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_CF_CONDITION}) );
    $msg .= sprintf( "    Fuel Gauge Condition:                   %s\n", R1_getI2CMonitorFuelGaugeConditionString($envStats{BE_BATT_SDIMM_STATUS_FUEL_GAUGE_FG_CONDITION}) );
    $msg .= sprintf( "---- Main Regulator -----------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Input Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_IVR_MAX_MV} );
    $msg .= sprintf( "    Output Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_OVR_MAX_MV} );
    $msg .= sprintf( "    Supply Voltage: %d (%s)  Range from %d to %d\n",
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_CURRENT_MV},
                    R1_getI2CMonitorLimitMonitorString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_LMV}),
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MIN_MV},
                    $envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_VOLTAGE_ROVR_MAX_MV} );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorMainRegulatorConditionString($envStats{BE_BATT_SDIMM_STATUS_MAIN_REGULATOR_CONDITION}) );
    $msg .= sprintf( "---- Charger ------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorChargerConditionString($envStats{BE_BATT_SDIMM_STATUS_CHARGER_CONDITION}) );
    $msg .= sprintf( "---- EEPROM -------------------------------------\n" );
    $msg .= sprintf( "    Event Status Code:                      %s\n", R1_getI2CMonitorStatusCodeString($envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATUS}) );
    $msg .= sprintf( "      Warning:                              %d\n", $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_WARNING} );
    $msg .= sprintf( "      Error:                                %d\n", $envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_STATS_ERROR} );
    $msg .= sprintf( "      Flags:                                %s\n", R1_getI2CMonitorEventFlagsString($envStats{BE_BATT_SDIMM_EEPROM_EVENT_PROP_FLAG_VALUE}) );
    $msg .= sprintf( "    Condition:                              %s\n", R1_getI2CMonitorEEPROMConditionString($envStats{BE_BATT_SDIMM_EEPROM_CONDITION}) );
    $msg .= sprintf( "%s\n"                                            , R1_getI2CMonitorXCIDataString($envStats{BE_BATT_SDIMM_EEPROM_XCI_DATA}) );

    return $msg;
}



##############################################################################
# Name: R1_getI2CMonitorStatusCodeString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorStatusCodeString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_STATUS_CODE_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_NOT_PRESENT )
    {
        $str = "Not_Present";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_VALID )
    {
        $str = "Valid";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_BUSY )
    {
        $str = "Busy";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_NOT_READY )
    {
        $str = "Not_Ready";
    }
    elsif( $val == I2C_MONITOR_STATUS_CODE_ERROR )
    {
        $str = "Error";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorTemperatureConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorTemperatureConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_TEMPERATURE_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_NORMAL )
    {
        $str = "Normal";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_COOL )
    {
        $str = "Cool";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_HOT )
    {
        $str = "Hot";
    }
    elsif( $val == I2C_MONITOR_TEMPERATURE_CONDITION_HOT_CRITICAL )
    {
        $str = "Critical";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorLimitMonitorString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorLimitMonitorString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_LIMIT_MONITOR_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_LIMIT_MONITOR_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_LIMIT_MONITOR_TRIPPED )
    {
        $str = "Tripped";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorPowerSupplyConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorPowerSupplyConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE )
    {
        $str = "High Temperature";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_DC_FAILED )
    {
        $str = "DC Failed";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_AC_FAILED )
    {
        $str = "AC Failed";
    }
    elsif( $val == I2C_MONITOR_POWER_SUPPLY_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorCoolingFanConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorCoolingFanConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_COOLING_FAN_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_FAILED )
    {
        $str = "Failed";
    }
    elsif( $val == I2C_MONITOR_COOLING_FAN_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorProcessorResetConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorProcessorResetConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_RUNNING )
    {
        $str = "Running";
    }
    elsif( $val == I2C_MONITOR_PROCESSOR_RESET_CONDITION_RESET )
    {
        $str = "Reset";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorBatteryConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorBatteryConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_BATTERY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_LOW_CAPACITY )
    {
        $str = "Low Capacity";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_UNDER_VOLTAGE )
    {
        $str = "Under Voltage";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_OVER_VOLTAGE )
    {
        $str = "Over Voltage";
    }
    elsif( $val == I2C_MONITOR_BATTERY_CONDITION_NOT_PRESENT )
    {
        $str = "Not Present";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorCurrentFlowConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorCurrentFlowConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_CURRENT_FLOW_CONDITION_ABNORMAL )
    {
        $str = "Abnormal";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorFuelGaugeConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorFuelGaugeConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_FUEL_GAUGE_CONDITION_SHUTDOWN )
    {
        $str = "Shutdown";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorMainRegulatorConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorMainRegulatorConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_OPERATIONAL )
    {
        $str = "Operational";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR )
    {
        $str = "Shutdown-Error";
    }
    elsif( $val == I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD )
    {
        $str = "Shutdown-Good";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorChargerConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorChargerConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_CHARGER_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_IDLE )
    {
        $str = "Idle";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_TRICKLE )
    {
        $str = "Trickle";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_BULK )
    {
        $str = "Bulk";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_OVER )
    {
        $str = "Over";
    }
    elsif( $val == I2C_MONITOR_CHARGER_CONDITION_TOPOFF )
    {
        $str = "Topoff";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorNVRAMBatteryConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorNVRAMBatteryConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_NVRAM_BATTERY_CONDITION_FAILED )
    {
        $str = "Failed";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorEEPROMConditionString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorEEPROMConditionString
{
    my ($val) = @_;
    my $str;

    if( $val == I2C_MONITOR_EEPROM_CONDITION_UNKNOWN )
    {
        $str = "Unknown";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_GOOD )
    {
        $str = "Good";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_BAD_CRC )
    {
        $str = "Bad_CRC";
    }
    elsif( $val == I2C_MONITOR_EEPROM_CONDITION_NOT_READABLE )
    {
        $str = "Not_Readable";
    }
    else
    {
        $str = "Undefined!";
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorXCIDataString
#
# Desc: Get the string, depending on the input value
#
# In:   Binary XCI data
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorXCIDataString
{
    my ($xciArray) = @_;
    my $msg = "";
    my $string = "";
    my @stuff;

    @stuff = unpack( "C8", substr($xciArray, 0, 8) );
    $msg .= sprintf( "      manuJedecId:                          [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3], $stuff[4], $stuff[5], $stuff[6], $stuff[7] );

    @stuff = unpack( "C1", substr($xciArray, 8, 1) );
    $msg .= sprintf( "      manuLocation:                         [%02x]\n", $stuff[0] );

    $string = unpack( "a7", substr($xciArray, 9, 7) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      modulePartNumber:                     [%s]\n", $string );

    $string = unpack( "a4", substr($xciArray, 16, 4) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      moduleDashNumber:                     [%s]\n", $string );

    $string = unpack( "a2", substr($xciArray, 20, 2) );
    $string =~ s/[^ -~]/\?/g;
    $msg .= sprintf( "      moduleRevisionLetters:                [%s]\n", $string );

    @stuff = unpack( "C5", substr($xciArray, 22, 5) );
    $msg .= sprintf( "      reserved-1:                           [%02x %02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3], $stuff[4] );

    @stuff = unpack( "C2", substr($xciArray, 27, 2) );
    $msg .= sprintf( "      revisionCode:                         [%02x %02x]\n",
        $stuff[0], $stuff[1] );

    @stuff = unpack( "C1", substr($xciArray, 29, 1) );
    $msg .= sprintf( "      manuYear:                             [%02x]\n", $stuff[0] );

    @stuff = unpack( "C1", substr($xciArray, 30, 1) );
    $msg .= sprintf( "      manuWeek:                             [%02x]\n", $stuff[0] );

    @stuff = unpack( "C4", substr($xciArray, 31, 4) );
    $msg .= sprintf( "      asmSerialNumber:                      [%02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3] );

    @stuff = unpack( "C23", substr($xciArray, 35, 23) );
    $msg .= sprintf( "      reserved-2:                           [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
        $stuff[0],  $stuff[1],  $stuff[2],  $stuff[3],  $stuff[4],
        $stuff[5],  $stuff[6],  $stuff[7],  $stuff[8],  $stuff[9],
        $stuff[10], $stuff[11], $stuff[12], $stuff[13], $stuff[14],
        $stuff[15], $stuff[16], $stuff[17], $stuff[18], $stuff[19],
        $stuff[20], $stuff[21], $stuff[22] );

    @stuff = unpack( "C4", substr($xciArray, 58, 4) );
    $msg .= sprintf( "      crc:                                  [%02x %02x %02x %02x]\n",
        $stuff[0], $stuff[1], $stuff[2], $stuff[3] );

    @stuff = unpack( "C2", substr($xciArray, 62, 2) );
    $msg .= sprintf( "      vendorSpecific:                       [%02x %02x]",
        $stuff[0], $stuff[1] );

    return $msg;
}


##############################################################################
# Name: R1_getI2CMonitorEventFlagsString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorEventFlagsString
{
    my ($val) = @_;
    my $str;

    if( $val == 0 )
    {
        $str = "None";
    }
    else
    {
        $str = "";

        if( $val & (1 << 0) )
        {
            $str = "$str-Info ";
        }

        if( $val & (1 << 4) )
        {
            $str = "$str-Info+ ";
        }

        if( $val & (1 << 1) )
        {
            $str = "$str-Warning ";
        }

        if( $val & (1 << 5) )
        {
            $str = "$str-Warning+ ";
        }

        if( $val & (1 << 2) )
        {
            $str = "$str-Error ";
        }

        if( $val & (1 << 6) )
        {
            $str = "$str-Error+ ";
        }

        if( $val & (1 << 3) )
        {
            $str = "$str-Debug ";
        }

        if( $val & (1 << 7) )
        {
            $str = "$str-Debug+ ";
        }
    }

    return $str;
}


##############################################################################
# Name: R1_getI2CMonitorDeviceFlagsString
#
# Desc: Get the string, depending on the input value
#
# In:   Value to be checked
#
# Out:  String
##############################################################################
sub R1_getI2CMonitorDeviceFlagsString
{
    my ($val) = @_;
    my $str;

    if( $val == 0 )
    {
        $str = "None";
    }
    else
    {
        $str = "";

        if( $val & (1 << 0) )
        {
            $str = "$str-Init ";
        }

        if( $val & (1 << 1) )
        {
            $str = "$str-Busy ";
        }

        if( $val & (1 << 2) )
        {
            $str = "$str-FRC ";
        }

        if( $val & (1 << 3) )
        {
            $str = "$str-RVal ";
        }

        if( $val & (1 << 4) )
        {
            $str = "$str-Psnt ";
        }

        if( $val & (1 << 7) )
        {
            $str = "$str-Mal ";
        }

        if( $val & 0x60 )
        {
            $str = "$str-Rsvd";
        }
    }
    return $str;
}


#
# END of R1 Stats Env support
#
#








1;   # we need this for a PM

##############################################################################
# $Log$
# Revision 1.9  2006/12/20 09:33:52  BharadwajS
# TBolt00017278 Fixing up FID 288 (StatsVDisk)
#
# Revision 1.8  2006/12/07 20:25:52  BharadwajS
# TBolt00017136 Fixing the Fid for vdisks (FID 280)
#
# Revision 1.7  2006/10/04 18:08:00  MiddenM
# TBolt00016149 - Add fid 312 for 750 reading of the smp phy information.
# A date/time is saved when original file is created when booting, and the
# current file is created with a date at the start of it. After the diff is
# done, the current file replaces the original file, so that future diffs
# can be done.
#
# Revision 1.6  2006/07/17 20:38:33  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.4.20.5  2006/05/15 17:01:29  HoltyB
# TBolt00000000:Added environmental PI interface/async events/validation
#
# Revision 1.4.20.4  2006/04/26 07:35:42  BharadwajS
# Adding error codes for new commands added
#
# Revision 1.4.20.3  2006/04/17 06:07:40  BharadwajS
# Log Acknowledgement
#
# Revision 1.4.20.2  2006/04/12 09:51:17  GudipudiK
# packet interface changes for CCBE changes
#
# Revision 1.4.20.1  2006/02/24 14:17:24  MiddenM
#
# Merge from WOOKIEE_EGGS_GA_BR into MODEL750_BR
#
# Revision 1.5  2006/01/12 11:05:28  ChannappaS
# Adding ISCSI and GeoRaid data to SnapDump
#
# Revision 1.4  2005/06/07 18:55:18  RysavyR
# TBolt00012897: A bunch more changes to the formatting of FID 299 at Steve Nowakowski's request. Rev by Steve N.
#
# Revision 1.3  2005/05/25 21:48:09  RysavyR
# TBolt00012897: Additional cleanup of  FID 299. Rev by Steve Nowakowski.
#
# Revision 1.2  2005/05/25 20:42:36  RysavyR
# TBolt00012897: Modified the formatting of FID 299. Rev by Steve Nowakowski.
#
# Revision 1.1.1.1  2005/05/04 18:53:56  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.43  2005/04/14 21:49:16  MiddenM
# TBolt00000000 - Add fidread 299 processing for SBOD pages 80/81.
#
# Revision 1.42  2005/04/13 20:48:40  RysavyR
# TBolt00012685: More fixes to the page 80 and 81 data in FID 353.
#
# Revision 1.41  2005/04/13 18:09:17  RysavyR
# TBolt00012685: Fixed up the page 80 and 81 data to FID 353, added earlier.
#
# Revision 1.40  2005/04/13 14:43:44  RysavyR
# TBolt00012685: Added page 80 and 81 data to FID 353.
#
# Revision 1.39  2005/04/06 16:10:47  McmasterM
# TBolt00012622: Controller locked up during reboot
# Added more Linux files to the backtrace system, and added some utility
# scripts to the kernel image's /root directory.  No kernel source changes.
#
# Revision 1.38  2005/03/29 15:39:06  McmasterM
# TBolt00012463: Snapshot info of size more than 5 MB is not sent to tomcat server
# Better reporting of fidread decode errors for David Palmi.
#
# Revision 1.37  2005/03/24 22:50:50  McmasterM
# TBolt00012463: Snapshot info of size more than 5 MB is not sent to tomcat server
# Switched the controller over to use BZIP2 compression instead of GZIP.  This
# new method is much more efficient for compressing the snapshot data.  In my
# testing, a single controller snapshot went from nearly 4 MB to 2.2MB.  LogTrim
# was modified to generate BZIP2 files, and logrotate was changed to BZIP2 also.
# Reviewed by Bryan Holty
#
# Revision 1.36  2005/03/02 15:49:55  BalemarthyS
# TBolt00012254 - FE NVRAM Part 5 SnapDump FID related changes
#
# Revision 1.35  2005/02/23 15:55:44  HoltyB
# TBolt00000000:  Fixes for Linux fidreads.
#
# Revision 1.34  2005/02/23 14:57:02  RysavyR
# TBolt00012347: Numerous fixes to SnapDump and underlying CCBE library to clean up errors seen on the snap server.
#
# Revision 1.33  2005/02/21 16:23:11  RysavyR
# TBolt00012311: Add bay info (FID 311) decoder. Rev by Tim Swatosh.
#
# Revision 1.32  2005/01/26 17:09:12  HoltyB
# TBolt00012008:  Changes to support Application logs on Wookiee.
# Reviewed by Mike McMaster.
#
# Revision 1.31  2004/12/21 22:12:13  RysavyR
# TBolt00011555: add passing of base addresses of logical fid data thru to the decoders.  Decoders now can optionally display the actual addresses of where the data was copied from.  Rev by Holty
#
# Revision 1.30  2004/12/16 22:23:37  HoltyB
# TBolt00011907:  Added interface for file transfer between client/controller.
# In snapdump and CCBE.
#
# Revision 1.29  2004/12/14 21:31:21  RysavyR
# TBolt00011557: Fixed the timestamp decoding on the CCB MRP backtrace for Wookiee captured flight recorder data. Rev by Bryan H.
#
# Revision 1.28  2004/12/08 21:52:18  HoltyB
# TBolt00011907:  Added Changes to support Linux file transfers.
#
# Revision 1.27  2004/08/16 19:58:21  RysavyR
# TBolt00011053: Fix the FID 280 (vdisk cache) decoder. Also added "NAME"
# to the vdisk display output.
#
# Revision 1.26  2004/07/13 19:18:22  RysavyR
# TBolt00000000: Add FID Header to Snapshot FIDs returned by the CCB.
# Also updated the CmdCodeHashes.pm with the latest MRP's etc.
#
# Revision 1.25  2004/07/08 15:12:45  RysavyR
# TBolt00000000: Bug fix in FmtCCBHeapStatsFID where corrupted flash segment
# was kicking us into the 'old' heap stats decoder, causing problems. Also bug fix
# in FmtCCBETSnap where an invalid timestamp was also causing problems.
# SnapDump.exe v1.10 promoted to the snap server.
#
# Revision 1.24  2004/07/02 15:08:18  SwatoshT
# TBolt00010380: Added support for SATA bay FC counters.
# Reviewed by Randy R.
#
# Revision 1.23  2004/02/24 23:07:35  RysavyR
# TBolt00010172: Add heap stats / top users to CCB NVRAM backtrace
# (to track down memory leaks).  Rev by Tim S.
#
# Revision 1.22  2004/02/24 22:32:37  RysavyR
# TBolt00010172: Add heap stats / top users to CCB NVRAM backtrace
# (to track down memory leaks).  Rev by Tim S.
#
# Revision 1.21  2004/02/11 17:17:07  RysavyR
# TBolt00010062: Added FICB to the DDR table. Also added
# CCB_MIRROR_PARTNER_LIST (fid 304). Rev by Brett.
#
# Revision 1.20  2004/02/02 21:03:53  RysavyR
# TBolt00009987: Added support for FID 303 -- CCB Statistics.
#
# Revision 1.19  2004/01/23 20:55:35  RysavyR
# TBolt00000000: CCBEDecodeFids now returns GOOD or ERROR.
# Needed to support SnapDump.pl.
#
# Revision 1.18  2003/12/30 19:35:56  SchibillaM
# TBolt00009808: Add the decode of the entire PDISKS hash (type==CMPL) following
# the existing summary data.
#
# Revision 1.17  2003/12/23 16:57:21  RysavyR
# Bugfix in FmtTargetResListFID().  Caused formatting of FID301 to fail.
#
# Revision 1.16  2003/12/12 16:09:28  SchibillaM
# TBolt00009629: Add decoders for cache maps and cache device paths.  Fix bug
# which resulted in an extra (invalid) GeoPool entry.
#
# Revision 1.15  2003/12/11 19:10:00  RysavyR
# Removed extraneous print statements.
#
# Revision 1.14  2003/12/10 21:45:47  SchibillaM
# TBolt00009629: Ad a C function type tag to each NVRAM copy.
#
# Revision 1.13  2003/12/10 18:02:07  SchibillaM
# TBolt00009629: Capture the timestamp from each NVRAM image in FID 298
# CCB NVRAM Flash Copies.  Order the images in reverse chronological order.
#
# Revision 1.12  2003/12/09 17:30:25  SchibillaM
# TBolt00009629: Add decoder support for FID 298 - CCB NVRAM Flash Copies.
#
# Revision 1.11  2003/12/08 20:23:26  SchibillaM
# TBolt00009629: Initial decoder for CCB NVRAM Flash Copies (FID 298).
#
# Revision 1.10  2003/12/05 16:33:56  SchibillaM
# TBolt00009629: Add FID 302 - List of FIDs to be dumped for a snapshot.  Reviewed
# by Randy.
#
# Revision 1.9  2003/11/21 20:38:24  SchibillaM
# TBolt00009629: FmtCCBNVR1FIDNew() replaces FmtCCBNVR1FID().  Lots of
# formatting changes to the CCB NVRAM FID.
#
# Revision 1.8  2003/11/12 17:02:57  SchibillaM
# TBolt00009629: Change FID299 so CCBE won't blow up if it isn't supported.
#
# Revision 1.7  2003/11/11 20:19:17  SchibillaM
# TBolt00000000: Add support for FID 300 - VCG Info.
#
# Revision 1.6  2003/11/07 21:18:07  SchibillaM
# TBolt00000000: Changes to decoders for FID 296 (FW Versions) and 256 (CCB
# Trace Buffer) to make them look more like the BFDump versions.  Move TracDec.pm
# from Test\TestLib to CBE\XIOTech.  Reviewed by Craig.
#
# Revision 1.5  2003/10/14 14:33:25  McmasterM
# TBolt00009397: Add logic to CCB to gather FCAL counters in background
# Added logic to CCB to collect and process the FCAL counters.  The data is
# stored in several arrays in the CCB DRAM, and are retrievable through the CCBE
# using the command 'fidread 299'.  The snapshot tools and DDR decoder have
# also been modified so that they are able to process the new arrays.
# Portions reviewed by Brett Tollefson
#
# Revision 1.4  2003/09/17 16:06:31  McmasterM
# Changed statsenv display to show hex data for manufacturing.
#
# Revision 1.3  2003/07/03 18:53:15  RysavyR
# TBolt00000000: Support for FID 296. Craig wrote it, I rev'd it.
#
# Revision 1.2  2003/06/25 13:13:28  MenningC
# TBOLT00000000: support for fid 291; reviewed by Mark S and Eric T
#
# Revision 1.1  2003/06/16 19:09:13  MenningC
# TBOLT00000000: import ddrdecode formatters into ccbcl; Reviewed by Randy and Erc
#
# Revision 1.5  2003/06/09 16:15:53  MenningC
# TBOLT00000000: Added R1 statsenv support.; reviewed by JW.
#
# Revision 1.4  2003/06/03 19:47:44  MenningC
# TBOLT00000000: Changed many of the 'display' functions in the CCBCL to fill a string rather than print to the screen. The test scripts can now use these functions. Reviewed by Jeff W.
#
# Revision 1.3  2003/05/21 21:35:10  MenningC
# Tbolt00000000:more FIDs added to decoders; reveiwed by JW
#
# Revision 1.2  2003/05/14 19:19:08  MenningC
# Tbolt00000000:additions to ddrdecode. reviewed by Jeff W
#
# Revision 1.1  2003/05/12 16:52:02  MenningC
# Tbolt00000000:support for decofing of new FIDs. reviewed by Jeff W
#
#
#
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
