#! /usr/bin/perl 
# $Id: decodeRings.pm 161200 2013-06-04 19:47:36Z marshall_midden $
##############################################################################
#  
#   CCBE Integration test library - Debug Dump decoder rings
#
#   1/28/2003  Xiotech   Craig Menning
#
#   A set of library functions for extracting debug information from a 
#   buffer.
#
#   It is expected that the user will write a perl script that calls 
#   these.
#
#   Copyright 2002-2009 Xiotech Corporation. All rights reserved.
#
#   For Xiotech internal use only.       
#
##############################################################################
=head1 NAME

XIOTech::decodeRings

$Id: decodeRings.pm 161200 2013-06-04 19:47:36Z marshall_midden $

=head1 SUPPORTED PLATFORMS

=begin html

 <UL> 
     <LI>Linux</LI> 
     <LI>Windows</LI> 
 </UL>

=end html

=head1 SYNOPSIS

A buffer is passed to this module and the function in this module decode
it and format it to ASCII strings. These strings are then logged and the 
user can look them over.

Entry pointes will be provided so that the user may call individual 
decoder functions that take the buffer to an ASCII string only. This will 
facilitate sharing of code among groups.

=head1 DESCRIPTION

Test Functions Available (exported)

        The more significant ones

              
                      
              
        The less significant ones


=cut


#                         
# - what I am
#

package XIOTech::decodeRings;

#
# - other modules used
#

use warnings;
use lib "../CCBE";

use Socket;

use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::constants;
use XIOTech::errorCodes;
use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::logMgr;

################################################use TestLibs::Constants;
################################################use TestLibs::utility;

use constant GOOD => 0;
use constant ERROR => 1;

use XIOTech::decodeSupport;

#
# - perl compiler/interpreter flags 'n' things
#

#use Getopt::Std;
#use FileHandle;
#use Text::Abbrev;
#use Cwd;
#use IO::Handle;

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
    #$VERSION = do { my @r = (q$Revision: 161200 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker

                        #




    @ISA         = qw(Exporter);
    @EXPORT      = qw(


                        &DiagProc
                        &FmtCIMTs
                        &FmtDefragT
                        &FmtDtmts
                        &FmtExecQ
                        &FmtFICB
                        &FmtFwh
                        &FmtFwhShort
                        &FmtFwhCCBEFormat
                        &FmtFltRecP
                        &FmtGenDir
                        &FmtICIMTs
                        &FmtIlmtWet
                        &FmtImt
                        &FmtIspRspQ
                        &FmtItrace
                        &FmtKii
                        &FmtLinkQCS
                        &FmtLldmtS
                        &FmtLsmt
                        &FmtLTMTs
                        &FmtMlmt
                        &FmtMrpTrcP
                        &FmtNvramDump
                        &FmtNvramDumpBEParts
                        &FmtPcb
                        &FmtProcData
                        &FmtPt1Sig
                        &FmtPt1ECCInit
                        &FmtPt1Post
                        &FmtSdd
                        &FmtSvrDB
                        &FmtTarget
                        &FmtTgtDef
                        &FmtITgd
                        &FmtIStats
                        &FmtIConn
                        &FmtIDD
                        &FmtTMTs
                        &FmtTrace
                        &FmtVCD
                        &FmtVdd
                        &FmtVdmt
                        &Part1Decode
                        &SelectDecoder

                      );
    #%EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],

    # your exported package globals go here,
    # as well as any optionally exported functions
    #@EXPORT_OK   = qw($Var1 %Hashit &func3);

 #   TestLibs::Logging::logVersion(__PACKAGE__, q$Name$);
}
    our @EXPORT_OK;


##############################################################################
#
#               Public Functions
#
##############################################################################

##############################################################################
# Name:     FmtKii()
#
# Desc:     Format the II structure (internal information)
#
# Call: 
#   FmtKii ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#typedef struct ii
#{
#   UINT8       rsvd0[16];          /* Reserved                             */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT8       rsvd16[10];         /* Reserved                             */
#    UINT16      status;             /* Firmware status                      */
#    UINT8       chgcnt;             /* Change counter                       */
#    UINT8       scrub;              /* Scrub enable                         */
#    UINT8       gpri;               /* Global priority                      */
#    UINT8       utzn;               /* % processor utilization              */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      time;               /* 1/4 sec timer                        */
#    UINT32      ircur;              /* Available internal RAM               */
#    UINT32      irmax;              /* Maximum internal RAM                 */
#    UINT32      irmin;              /* Minimum internal RAM                 */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      irwait;             /* Waits internal RAM                   */
#    UINT32      sdcur;              /* Available cacheable SDRAM            */
#    UINT32      sdmax;              /* Maximum cacheable SDRAM              */
#    UINT32      sdmin;              /* Minimum cacheable SDRAM              */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      sdwait;             /* Waits cacheable SDRAM                */
#    UINT32      nccur;              /* Available non-cache SDRAM            */
#    UINT32      ncmax;              /* Maximum non-cache SDRAM              */
#    UINT32      ncmin;              /* Minimum non-cache SDRAM              */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      ncwait;             /* Waits non-cache SDRAM                */
#    UINT32      rscur;              /* Available remote SRAM                */
#    UINT32      rsmax;              /* Maximum remote SRAM                  */
#    UINT32      rsmin;              /* Minimum remote SRAM                  */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      rswait;             /* Waits remote SRAM                    */
#    UINT32      pcbcur;             /* Number of active PCBs                */
#    UINT32      pcbmax;             /* Maximum number of PCBs               */
#    UINT32      iltcur;             /* Current number of ILTs               */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      iltmax;             /* Maximum number of ILTs               */
#    UINT32      prpcur;             /* Current number of PRPs               */
#    UINT32      prpmax;             /* Maximum number of PRPs               */
#    UINT32      rrpcur;             /* Current number of RRPs               */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      rrpmax;             /* Maximum number of RRPs               */
#    UINT32      scbcur;             /* Current number of SCBs               */
#    UINT32      scbmax;             /* Maximum number of SCBs               */
#    UINT32      rpncur;             /* Current number of RPNs               */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      rpnmax;             /* Maximum number of RPNs               */
#    UINT32      rrbcur;             /* Current number of RRBs               */
#    UINT32      rrbmax;             /* Maximum number of RRBs               */
#    UINT8       rsvd140[4];         /* Reserved                             */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      nvacur;             /* Current number of NVAs               */
#    UINT32      nvamax;             /* Maximum number of NVAs               */
#    UINT32      nvamin;             /* Minimum number of NVAs               */
#    UINT32      nvawait;            /* Number of wait NVAs                  */
#                                    /* QUAD BOUNDARY                    *****/
#};
##############################################################################
sub FmtKii
{
    my ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $version ) = @_;
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $msg;

    ##########################################
    # processor  internal information @ K_ii
    ##########################################
    
        
    $msg = "\n";

    if ( !defined ($version) )
    {
        $version = 0;
    }

    if ( 1 == $version )
    {
        #   UNIT16      vers;               /* Firmwave version      */
        #   UINT16      rev;                /* Firmware revision     */
        #   UINT8       user[4];            /* Person who built code */
        #   UINT8       bdate[8];           /* Build date mm/dd/yy   */

        $fmt = sprintf("x%d SS L LL",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5 ) =
                         unpack $fmt , $$bufferPtr;

        $msg .= sprintf(" ii_vers:  0x%04x          \n",$item1);
        $msg .= sprintf(" ii_rev:   0x%04x          \n",$item2);
        $msg .= sprintf(" ii_user:  0x%08x          \n",$item3);
        $msg .= sprintf(" ii_bdate: 0x%08x0x%08x    \n",$item5, $item4);

        $offset += 16;

        #   UINT8       btime[8];           /* Build time hh:mm:ss       */
        #   UINT16      bldCnt;             /* Incrementing build count  */
        ##
        #

        $fmt = sprintf("x%d LL S",$offset);      # generate the format string
        ($item1, $item2, $item3 ) =
                         unpack $fmt , $$bufferPtr;

        $msg .= sprintf(" ii_btime:  0x%08x0x%08x          \n",$item2, $item1);
        $msg .= sprintf(" ii_bldCnt: 0x%04x                \n",$item3);

        $offset += 10;
    }
    else
    {
        # --- Begin structure -------------------------------------------------
        #   UINT8       rsvd0[16];          /* Reserved          
        #                                   /* QUAD BOUNDARY   
        #   UINT8       rsvd16[10];         /* Reserved       
    
        $offset += 26;
    } 
    
    #    UINT16      status;             /* Firmware status                      */
    #    UINT8       chgcnt;             /* Change counter                       */
    #    UINT8       scrub;              /* Scrub enable                         */
    #    UINT8       gpri;               /* Global priority                      */
    #    UINT8       utzn;               /* % processor utilization              */

    $fmt = sprintf("x%d SCCCC",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" ii_status: 0x%04x    \n",$item1);
    $msg .= sprintf(" ii_chgcnt: 0x%02x        ",$item2);
    $msg .= sprintf(" ii_scrub: 0x%02x       ",$item3);
    $msg .= sprintf("   ii_gpri: 0x%02x        ",$item4);
    $msg .= sprintf("      utzn: 0x%02x \n",$item5);

    $offset += 6;                         # 6 bytes processed

    #    UINT32      time;               /* 1/4 sec timer                        */
    #    UINT32      ircur;              /* Available internal RAM               */
    #    UINT32      irmax;              /* Maximum internal RAM                 */
    #    UINT32      irmin;              /* Minimum internal RAM                 */
    #                                    /* QUAD BOUNDARY                    *****/
    
    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("   ii_time: 0x%08x ",$item1);
    $msg .= sprintf("  ii_ircur: 0x%08x ",$item2);
    $msg .= sprintf("  ii_irmax: 0x%08x ",$item3);
    $msg .= sprintf("   ii_irmin: 0x%08x \n",$item4);

    $offset += 16;                         # add bytes processed

    #    UINT32      irwait;             /* Waits internal RAM                   */
    #    UINT32      sdcur;              /* Available cacheable SDRAM            */
    #    UINT32      sdmax;              /* Maximum cacheable SDRAM              */
    #    UINT32      sdmin;              /* Minimum cacheable SDRAM              */
    
    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_irwait: 0x%08x ",$item1);
    $msg .= sprintf("  ii_sdcur: 0x%08x ",$item2);
    $msg .= sprintf("  ii_sdmax: 0x%08x ",$item3);
    $msg .= sprintf("   ii_sdmin: 0x%08x \n",$item4);

    $offset += 16;                         # add bytes processed

    #    UINT32      sdwait;             /* Waits cacheable SDRAM                */
    #    UINT32      nccur;              /* Available non-cache SDRAM            */
    #    UINT32      ncmax;              /* Maximum non-cache SDRAM              */
    #    UINT32      ncmin;              /* Minimum non-cache SDRAM              */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_sdwait: 0x%08x ",$item1);
    $msg .= sprintf("  ii_nccur: 0x%08x ",$item2);
    $msg .= sprintf("  ii_ncmax: 0x%08x ",$item3);
    $msg .= sprintf("   ii_ncmin: 0x%08x \n",$item4);

    $offset += 16;                         # add bytes processed

    #    UINT32      ncwait;             /* Waits non-cache SDRAM                */
    #    UINT32      rscur;              /* Available remote SRAM                */
    #    UINT32      rsmax;              /* Maximum remote SRAM                  */
    #    UINT32      rsmin;              /* Minimum remote SRAM                  */


    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_ncwait: 0x%08x ",$item1);
    $msg .= sprintf("  ii_rscur: 0x%08x ",$item2);
    $msg .= sprintf("  ii_rsmax: 0x%08x ",$item3);
    $msg .= sprintf("   ii_rsmin: 0x%08x \n",$item4);


    $offset += 16;                         # add bytes processed

    #    UINT32      rswait;             /* Waits remote SRAM                    */
    #    UINT32      pcbcur;             /* Number of active PCBs                */
    #    UINT32      pcbmax;             /* Maximum number of PCBs               */
    #    UINT32      iltcur;             /* Current number of ILTs               */


    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_rswait: 0x%08x ",$item1);
    $msg .= sprintf(" ii_pcbcur: 0x%08x ",$item2);
    $msg .= sprintf(" ii_pcbmax: 0x%08x ",$item3);
    $msg .= sprintf("  ii_iltcur: 0x%08x \n",$item4);

    $offset += 16;                         # add bytes processed

    #    UINT32      iltmax;             /* Maximum number of ILTs               */
    #    UINT32      prpcur;             /* Current number of PRPs               */
    #    UINT32      prpmax;             /* Maximum number of PRPs               */
    #    UINT32      rrpcur;             /* Current number of RRPs               */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_iltmax: 0x%08x ",$item1);
    $msg .= sprintf(" ii_prpcur: 0x%08x ",$item2);
    $msg .= sprintf(" ii_prpmax: 0x%08x ",$item3);
    $msg .= sprintf("  ii_rrpcur: 0x%08x \n",$item4);

    $offset += 16;                         # add bytes processed

    #    UINT32      rrpmax;             /* Maximum number of RRPs               */
    #    UINT32      scbcur;             /* Current number of SCBs               */
    #    UINT32      scbmax;             /* Maximum number of SCBs               */
    #    UINT32      rpncur;             /* Current number of RPNs               */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf(" ii_rrpmax: 0x%08x ",$item1);
    $msg .= sprintf(" ii_scbcur: 0x%08x ",$item2);
    $msg .= sprintf(" ii_scbmax: 0x%08x ",$item3);
    $msg .= sprintf("  ii_rpncur: 0x%08x \n",$item4);


    $offset += 16;                         # add bytes processed


    #    UINT32      rpnmax;             /* Maximum number of RPNs               */
    #    UINT32      rrbcur;             /* Current number of RRBs               */
    #    UINT32      rrbmax;             /* Maximum number of RRBs               */
    #    UINT8       rsvd140[4];         /* Reserved                             */

    $fmt = sprintf("x%d LLL",$offset);      # generate the format string
    ($item1, $item2, $item3 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_rpnmax: 0x%08x ",$item1);
    $msg .= sprintf(" ii_rrbcur: 0x%08x ",$item2);
    $msg .= sprintf(" ii_rrbmax: 0x%08x \n",$item3);


    $offset += 16;                         # add bytes processed

    #    UINT32      nvacur;             /* Current number of NVAs               */
    #    UINT32      nvamax;             /* Maximum number of NVAs               */
    #    UINT32      nvamin;             /* Minimum number of NVAs               */
    #    UINT32      nvawait;            /* Number of wait NVAs                  */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" ii_nvacur: 0x%08x ",$item1);
    $msg .= sprintf(" ii_nvamax: 0x%08x ",$item2);
    $msg .= sprintf(" ii_nvamin: 0x%08x ",$item3);
    $msg .= sprintf(" ii_nvawait: 0x%08x \n",$item4);

    if ( 1 == $version )
    {
        $offset += 16;
    }
    #                                                                  *****
    # --- End structure -----( K_ii )---------------------------------------

    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
#
#          Name: FmtFwh
# Call: 
#   FmtFwh ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        .set    branch                          # Branch instruction              <w>
#        .set    fh_rsvd0,0                      # Space for branch instruction   7<w>
#                                                #                                *****
#        .set    fh_magicNumber,fh_rsvd0+32      # Indication of a valid header    <w>
#        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
#        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
#        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
#                                                #                                *****
#        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
#        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
#        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
#        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>
#                                                #                                *****
#        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
#        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
#        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
#                                                #                                *****
#        .set    fh_loadID,fh_burnSequence+4     # Firmware load information      6<w>
#        .set    fh_rsvd3,fh_loadID+fl_size      # Pad out to 32 words            5<w>
#        .set    fh_hdrCksum,fh_rsvd3+(5*4)      # Checksum of this header only    <w>
#                                                                                *****
#        .set    fh_size,fh_hdrCksum+4           # size of firmware header
#
#
#
##############################################################################

sub FmtFwh
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my @ar1;
    my @ar2;
    my @ar3;
    my $msg;

    ##########################################
    # firmware header 
    ##########################################

    # --- Begin structure -------------------------------------------------
    #        .set    branch                          # Branch instruction              <w>
    $offset += 4;

    #        .set    fh_rsvd0,0                      # Space for branch instruction   7<w>
    $offset += 28;
    
    #        .set    fh_magicNumber,fh_rsvd0+28      # Indication of a valid header    <w>
    #        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
    #        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
    #        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "magicNumber: 0x%08x ", $item1);
    $msg .= sprintf( "      rsvd1: 0x%08x ", $item2);
    $msg .= sprintf( "  productID: 0x%08x   ", $item3);
    $msg .= sprintf( "     target: 0x%08x \n", $item4);

    $offset += 16;                         # add bytes processed

    
    #        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
    #        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
    #        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
    #        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>

    $fmt = sprintf("x%d A4A4A4A4",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;


    $msg .= sprintf( "   revision: $item1       ");
    $msg .= sprintf( "   revCount: $item2       ");
    $msg .= sprintf( "    buildID: $item3         ");
    $msg .= sprintf( "sys release: $item4 \n");
    
    $offset += 16;                         # add bytes processed



    #        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
    #        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
    #        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
            

    my $year;
    my $month;
    my $date;
    my $day;
    my $hours;
    my $minutes;
    my $seconds;
    my $rsvd2;
    my $burnSequence;

    $fmt = sprintf("x%d SCCCCCCLL",$offset);      # generate the format string
    ($year, $month, $date, $day, $hours, $minutes, $seconds, $rsvd2, $burnSequence) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf (" Build Date: %2x/%2x/%4x ", $month, $date, $year);
    $msg .= sprintf ( "%02x:%02x:%02x", $hours, $minutes, $seconds);

    $msg .= sprintf( "                      rsvd2: 0x%08x  ", $rsvd2);
    
    $msg .= sprintf( "burnSequence: 0x%08x\n", $burnSequence);


    $offset += 16;                         # add bytes processed


    #        .set    fh_loadID,fh_burnSequence+4     # Firmware load information      6<w>
    #        .set    fh_rsvd3,fh_loadID+fl_size      # Pad out to 32 words            5<w>
    #        .set    fh_hdrCksum,fh_rsvd3+(5*4)      # Checksum of this header only    <w>



    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;

    #msg .= sprintf( "magicNumber: 0x%08x ", $item1);

    $msg .= sprintf( "   emcAddrA: 0x%08x ", $item1);
    $msg .= sprintf( "   emcAddrB: 0x%08x ", $item2);
    $msg .= sprintf( "   targAddr: 0x%08x   ", $item3);
    $msg .= sprintf( "     length: 0x%08x \n", $item4);
    $msg .= sprintf( "   checksum: 0x%08x  ", $item5);
    $msg .= sprintf( "                   compatibilityID: 0x%08x\n", $item6);

    $offset += 24;                         # add bytes processed


    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5,$item6) =  
                        unpack $fmt , $$bufferPtr;
    #msg .= sprintf( "magicNumber: 0x%08x ", $item1);

    $msg .= sprintf( "     rsvd30: 0x%08x ", $item1);
    $msg .= sprintf( "     rsvd31: 0x%08x ", $item2);
    $msg .= sprintf( "     rsvd32: 0x%08x   ", $item3);
    $msg .= sprintf( "     rsvd33: 0x%08x \n", $item4);
    $msg .= sprintf( "     rsvd34: 0x%08x ", $item5);
    $msg .= sprintf( "   hdrCksum: 0x%08x\n\n", $item6);



    $offset += 24;                         # add bytes processed

    #       .set    fh_size,fh_hdrCksum+4           # size of firmware header


    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
##############################################################################
#
#          Name: FmtFwh
# Call: 
#   FmtFwh ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        .set    branch                          # Branch instruction              <w>
#        .set    fh_rsvd0,0                      # Space for branch instruction   7<w>
#                                                #                                *****
#        .set    fh_magicNumber,fh_rsvd0+32      # Indication of a valid header    <w>
#        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
#        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
#        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
#                                                #                                *****
#        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
#        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
#        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
#        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>
#                                                #                                *****
#        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
#        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
#        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
#                                                #                                *****
#        .set    fh_loadID,fh_burnSequence+4     # Firmware load information      6<w>
#        .set    fh_rsvd3,fh_loadID+fl_size      # Pad out to 32 words            5<w>
#        .set    fh_hdrCksum,fh_rsvd3+(5*4)      # Checksum of this header only    <w>
#                                                                                *****
#        .set    fh_size,fh_hdrCksum+4           # size of firmware header
#
#
#
##############################################################################
sub FmtFwhShort
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my @ar1;
    my @ar2;
    my @ar3;
    my $msg;

    ##########################################
    # firmware header 
    ##########################################

    # --- Begin structure -------------------------------------------------

    
    #        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
    #        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
    #        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
    #        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>

    $fmt = sprintf("x%d A4A4A4",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;


    $msg .= sprintf( "FW Version: $item1 $item2 $item3\n");
    
    $offset += 12;                         # add bytes processed



    #        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
    #        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
    #        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
            

    my $year;
    my $month;
    my $date;
    my $day;
    my $hours;
    my $minutes;
    my $seconds;
    my $rsvd2;
    my $burnSequence;

    $fmt = sprintf("x%d SCCCCCC",$offset);      # generate the format string
    ($year, $month, $date, $day, $hours, $minutes, $seconds) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf ("Build Date: %x/%2x/%4x ", $month, $date, $year);
    $msg .= sprintf ( "%02x:%02x:%02x\n\n", $hours, $minutes, $seconds);



    $offset += 8;                         # add bytes processed



    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
#
#          Name: FmtFwhCCBEFormat
# Call: 
#   FmtFwhCCBEFormat ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        .set    branch                          # Branch instruction              <w>
#        .set    fh_rsvd0,0                      # Space for branch instruction   7<w>
#                                                #                                *****
#        .set    fh_magicNumber,fh_rsvd0+32      # Indication of a valid header    <w>
#        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
#        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
#        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
#                                                #                                *****
#        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
#        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
#        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
#        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>
#                                                #                                *****
#        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
#        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
#        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
#                                                #                                *****
#        .set    fh_loadID,fh_burnSequence+4     # Firmware load information      6<w>
#        .set    fh_rsvd3,fh_loadID+fl_size      # Pad out to 32 words            5<w>
#        .set    fh_hdrCksum,fh_rsvd3+(5*4)      # Checksum of this header only    <w>
#                                                                                *****
#        .set    fh_size,fh_hdrCksum+4           # size of firmware header
#
#
#
##############################################################################
sub FmtFwhCCBEFormat
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my @ar1;
    my @ar2;
    my @ar3;
    my $msg;

    ##########################################
    # firmware header 
    ##########################################

    # --- Begin structure -------------------------------------------------
    #        .set    branch                          # Branch instruction              <w>
    #        .set    fh_rsvd0,0                      # Space for branch instruction   7<w>
    #        .set    fh_magicNumber,fh_rsvd0+28      # Indication of a valid header    <w>
    #        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
    #        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
    #        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
    
    # Skip the stuff above
    $offset += 48;
    
    #        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
    #        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
    #        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
    #        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>

    $fmt = sprintf("x%d A4A4A4A4",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "$item1    ");
    $msg .= sprintf( "$item2     ");
    $msg .= sprintf( "$item3     ");
    $msg .= sprintf( "$item4     ");
    
    $offset += 16;                         # add bytes processed

    #        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
    #        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
    #        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
            
    my $year;
    my $month;
    my $date;
    my $day;
    my $hours;
    my $minutes;
    my $seconds;
    my $rsvd2;
    my $burnSequence;

    $fmt = sprintf("x%d SCCCCCCLL",$offset);      # generate the format string
    ($year, $month, $date, $day, $hours, $minutes, $seconds, $rsvd2, $burnSequence) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf (" %2x/%2x/%4x  ", $month, $date, $year);
    $msg .= sprintf ( "%02x:%02x:%02x\n", $hours, $minutes, $seconds);

    $$destPtr .= $msg;

    return GOOD;
}

##############################################################################
##############################################################################
#
#          Name: FmtTgtDef
# Call: 
#   FmtTgtDef ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# typedef struct tgd
# {
#     UINT16      tid;                /* Target ID                            */
#     UINT8       port;               /* Port Number                          */
#     UINT8       opt;                /* Target options                       */
#     UINT8       fcid;               /* FC ID if hard                        */
#     UINT8       rsvd5;              /* Reserved                             */
#     UINT8       lock;               /* Locked target indicator              */
#     UINT8       rsvd7;              /* Reserved                             */
#     UINT64      pname;              /* FC port world wide name              */
#                                     /* QUAD BOUNDARY                    *****/
#     UINT64      nname;              /* FC node world wide name              */
#     UINT32      powner;             /* Serial number of previous owner      */
#     UINT32      owner;              /* Serial number of current owner       */
#                                     /* QUAD BOUNDARY                    *****/
#     UINT16      cluster;            /* Cluster number                       */
#     UINT16      rsvd2;              /* Reserved                             */
#     UINT8       pport;              /* Prefered port                        */
#     UINT8       aport;              /* Alternate port                       */
#     UINT8       rsvd3[10];          /* Reserved                             */
#                                    /* QUAD BOUNDARY                    *****/
# };
#
#
#
##############################################################################

sub FmtTgtDef
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;

    ##########################################
    # target definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------
    #     UINT16      tid;                /* Target ID                            */
    #     UINT8       port;               /* Port Number                          */
    #     UINT8       opt;                /* Target options                       */

    #     UINT8       fcid;               /* FC ID if hard                        */
    #     UINT8       rsvd5;              /* Reserved                             */
    #     UINT8       lock;               /* Locked target indicator              */
    #     UINT8       rsvd7;              /* Reserved                             */

    #     UINT64      pname;              /* FC port world wide name              */

    $fmt = sprintf("x%d SCC CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6, $item7,
             $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "     Target ID: 0x%04x       ", $item1);
    $msg .= sprintf( "Port Number: 0x%02x              ", $item2);
    $msg .= sprintf( "Target options: 0x%02x \n", $item3);
    
    $msg .= sprintf( " FC ID if hard: 0x%02x            ", $item4);
    $msg .= sprintf( "Reserved: 0x%02x     ", $item5);
    $msg .= sprintf( "Locked target indicator: 0x%02x           ", $item6);
    $msg .= sprintf( "Reserved: 0x%02x \n", $item7);

    $offset += 8;                         # add bytes processed

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "                 FC port world wide name: %16s\n", $item1);

    $offset += 8;                         # add bytes processed

    
    #     UINT64      nname;              /* FC node world wide name              */
    #     UINT32      powner;             /* Serial number of previous owner      */
    #     UINT32      owner;              /* Serial number of current owner       */


    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "                 FC node world wide name: %16s\n", $item1);


    $offset += 8;                         # add bytes processed

    $fmt = sprintf("x%d  L L",$offset);      # generate the format string
    ($item1, $item2) =  unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "        powner: $item1          ");
    $msg .= sprintf( "   owner: $item2    \n");
    
    $offset += 8;                         # add bytes processed


    #     UINT16      cluster;            /* Cluster number                       */
    #     UINT16      rsvd2;              /* Reserved                             */
    #     UINT8       pport;              /* Prefered port                        */
    #     UINT8       aport;              /* Alternate port                       */
    #     UINT8       rsvd3[10];          /* Reserved                             */

            
    $fmt = sprintf("x%d SS CC S LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4,
            $item5, $item6, $item7) =  unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Cluster number: 0x%04x          ", $item1);
    $msg .= sprintf( "Reserved: 0x%04x           ", $item2);
    $msg .= sprintf( " Preferred port: %4d ",$item3);
    $msg .= sprintf( "    Alternate port: $item4 \n ");
    $msg .= sprintf( "     Reserved: 0x%04x       ", $item5);
    $msg .= sprintf( "   Reserved: 0x%08x           ", $item6);
    $msg .= sprintf( "   Reserved: 0x%08x    \n", $item7);
    
    $offset += 16;                         # add bytes processed



    $$destPtr .= $msg;

    return GOOD;

}

# ISCSI_CODE
##############################################################################
##############################################################################
#
#          Name: FmtITgd
# Call: 
#   FmtTgtDef ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# typedef struct i_tgd
# {
# };
#
#
#
##############################################################################

sub FmtITgd
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;
    my $ipstr;

    ##########################################
    # i_tgd definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------

    $fmt = sprintf("x%d SLLLS CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6, $item7,
             $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "Target ID               : 0x%04x\n", $item1);
    
    $ipstr = XIOTech::cmdMgr::net2ip(0, $item2);
    $msg .= sprintf( " IP Address             : %s\n", $ipstr);
    
    $ipstr = XIOTech::cmdMgr::net2ip(0, $item3);
    $msg .= sprintf( " Subnet Mask            : %s \n", $ipstr);
    
    $ipstr = XIOTech::cmdMgr::net2ip(0, $item4);
    $msg .= sprintf( " Default Gateway        : %s \n", $ipstr);

    $msg .= sprintf( " maxConnections         : %u \n", $item5);
    $msg .= sprintf( " initialR2T             : %u \n", $item6);
    $msg .= sprintf( " immediateData          : %u \n", $item7);
    $msg .= sprintf( " dataSequenceInOrder    : %u \n", $item8);
    $msg .= sprintf( " dataPDUInOrder         : %u \n", $item9);

    $offset += 20;                         # add bytes processed



    $fmt = sprintf("x%d CCC S LL SSS ",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6, $item7,
             $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
                        
    $msg .= sprintf( " ifMarker               : %u \n", $item1);
    $msg .= sprintf( " ofMarker               : %u \n", $item2);
    $msg .= sprintf( " errorRecoveryLevel     : %u \n", $item3);
    $msg .= sprintf( " targetPortalGroupTag   : %u \n", $item4);
    $msg .= sprintf( " maxBurstLength         : %u \n", $item5);
    $msg .= sprintf( " firstBurstLength       : %u \n", $item6);
    $msg .= sprintf( " defaultTime2Wait       : %u \n", $item7);
    $msg .= sprintf( " defaultTime2Retain     : %u \n", $item8);
    $msg .= sprintf( " maxOutstandingR2T      : %u \n", $item9);

    $offset += 19;                         # add bytes processed

    $fmt = sprintf("x%d LSS CCC L ",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6, $item7) = 
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " maxRecvDataSegmentLength: %u \n", $item1);
    $msg .= sprintf( " ifMarkInt               : %u \n", $item2);
    $msg .= sprintf( " ofMarkInt               : %u \n", $item3);
    $msg .= sprintf( " headerDigest            : %u \n", $item4);
    $msg .= sprintf( " dataDigest              : %u \n", $item5);
    $msg .= sprintf( " authMethod              : %u \n", $item6);
    $msg .= sprintf( " mtuSize                 : %u \n", $item7);
    
    $offset += 15;                         # add bytes processed

    $fmt = sprintf("x%d a32 LLLS",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5) = 
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( " tgtAlias                : %s \n", $item1);
    $msg .= sprintf( " maxSendDataSegmentLength: %u \n", $item2);
    $msg .= sprintf( " numUsers                : %u \n", $item3);

    $offset += 46;                         # add bytes processed

    
    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
##############################################################################
#
#          Name: FmtIStats
# Call: 
#   FmtTgtDef ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# typedef struct session
# {
# };
#
#
#
##############################################################################

sub FmtIStats
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;
    my $ipstr;

    ##########################################
    # session definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------

    $fmt = sprintf("x%d L",$offset);      # generate the format string
    ($item1) = unpack $fmt , $$bufferPtr;
    $offset += 4;                         # add bytes processed

    $fmt = sprintf("x%d CCCC LLL",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Session State  : %u\n", $item1);
    $msg .= sprintf( "Active Conns   : %u\n", $item2);
    $msg .= sprintf( "Version        : %u\n", $item3);
    $msg .= sprintf( "Cmd SN         : %u\n", $item5);
    $msg .= sprintf( "Exp Cmd SN     : %u\n", $item6);
    $msg .= sprintf( "Max Cmd SN     : %u\n", $item7);
    
    $offset += 16;                         # add bytes processed

    $item1 = FmtWwn($bufferPtr, ($offset));
    $msg .= sprintf( "sid            : %s\n", $item1);

    $offset += 8;                         # add bytes processed

    $fmt = sprintf("x%d LL",$offset);      # generate the format string
    ($item1, $item2 ) =  
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf( "itt            : %u\n", $item1);
    $msg .= sprintf( "ttt            : %u\n", $item2);

    $offset += 8;                         # add bytes processed

    $fmt = sprintf("x%d SSL ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
                        
    $msg .= sprintf( "Target ID      : 0x%04x\n", $item1);
    $offset += 8;                         # add bytes processed

                        
# Process Session Params

    $fmt = sprintf("x%d Sa254a254a254a254a254S ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6,$item7) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Max Connections: %u\n", $item1);
    $msg .= sprintf( "Target Name    : %s\n", substr($item2,0,32));
    $msg .= sprintf( "Initiator Name : %s\n", substr($item3,0,32));
    $msg .= sprintf( "Target Alias   : %s\n", substr($item4,0,32));
    $msg .= sprintf( "Initiator Alias: %s\n", substr($item5,0,32));
    $msg .= sprintf( "Target Address : %s\n", substr($item6,0,32));
    $msg .= sprintf( "Portal Grp Tag : %u\n", $item7);

    $offset += 1274;                        # add bytes processed

    $fmt = sprintf("x%d LL SSS C",$offset);      # generate the format string
    ($item1, $item2, $item3, 
             $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
                        
    $msg .= sprintf( "Max Burst Len  : %u\n", $item1);
    $msg .= sprintf( "First Burst Len: %u\n", $item2);
    $msg .= sprintf( "Dflt Time2Wait : %u\n", $item3);
    $msg .= sprintf( "Dflt Time2Retn : %u\n", $item4);
    $msg .= sprintf( "Max Outstdg R2T: %u\n", $item5);
    $msg .= sprintf( "BitField 1     : %u\n", $item6);
    $offset += 15;                        # add bytes processed


    $fmt = sprintf("x%d LL a248",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Param Map      : %u\n", $item1);
    $msg .= sprintf( "Param Sent Map : %u\n", $item2);
    $offset += 256;                        # add bytes processed


    $fmt = sprintf("x%d L ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf( "First Cmd SN   : %u\n", $item1);

    $offset += 4;                      # add bytes processed

# Process PDU
    $offset += 52;                       # go over pdu header

    $fmt = sprintf("x%d LLLL L",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;


    $msg .= sprintf( "AHS Len        : %u\n", $item1);
    $msg .= sprintf( "AHS Addr       : %u\n", $item2);
    $msg .= sprintf( "Position       : %u\n", $item3);
    $msg .= sprintf( "CDB Length     : %u\n", $item5);

    $offset += 20;                        # add bytes processed

    $offset += 15;

    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
##############################################################################
#
#          Name: FmtIConn
# Call: 
#   FmtTgtDef ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# typedef struct session
# {
# };
#
#
#
##############################################################################

sub FmtIConn
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item2a;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $item9;
    my $msg;
    my $ipstr;

    ##########################################
    # connection definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------

    $fmt = sprintf("x%d L",$offset);      # generate the format string
    ($item1) = unpack $fmt , $$bufferPtr;

    $offset += 4;
    
    $fmt = sprintf("x%d LLCa52LCLLLS",$offset);      # generate the format string
    ($item1, $item2, $item2a, $item3, $item4, $item5,
              $item6, $item7, $item8, $item9) = 
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Conn cleanup        : %u\n", $item1);
    $msg .= sprintf( "Is CHAP             : %u\n", $item2);
    $msg .= sprintf( "CSG State           : %u\n", $item2a);
    $msg .= sprintf( "AHS Pointer         : 0x%04x\n", $item4);
    $msg .= sprintf( "Conn State          : %u\n", $item5);
    $msg .= sprintf( "Recv Len            : %u\n", $item6);
    $msg .= sprintf( "Stat SN             : %u\n", $item7);
    $msg .= sprintf( "Exp Stat SN         : %u\n", $item8);
    $msg .= sprintf( "Conn ID             : %u\n", $item9);
    
    $offset += 80;                         # add bytes processed

    $fmt = sprintf("x%d CCL",$offset);     # generate the format string
    ($item1, $item2, $item3) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Recv State   : %u\n", $item1);

    $offset += 6;                          # add bytes processed

    $fmt = sprintf("x%d LLLCL",$offset);   # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf( "Send ILT Head       : 0x%04x\n", $item1);
    $msg .= sprintf( "Send ILT Tail       : 0x%04x\n", $item2);
    $msg .= sprintf( "Send Count          : %u\n", $item3);
    $msg .= sprintf( "Send State          : %u\n", $item4);
    $msg .= sprintf( "Session Ptr         : 0x%04x\n", $item5);

    $offset += 17;                          # add bytes processed
 
# Process Connection Params
 
    $msg .= sprintf( "Connection Parameters:\n");

    $fmt = sprintf("x%d a4097a4097a4097",$offset);     # generate the format string
    ($item1, $item2, $item3) =
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf( "Header Digest       : %s\n", substr($item1,0,32));
    $msg .= sprintf( "Data Digest         : %s\n", substr($item2,0,32));
    $msg .= sprintf( "Auth Method         : %s\n", substr($item3,0,32));

    $offset += 12291;                       # add bytes processed


    $fmt = sprintf("x%d CC LL LL",$offset); # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =
                        unpack $fmt , $$bufferPtr;
    $msg .= sprintf( "CHAP Algo           : %u\n", $item1);
    $msg .= sprintf( "Bit Field           : %u\n", $item2);
    $msg .= sprintf( "OfMarkInt           : %u-%u\n", $item3,$item4);
    $msg .= sprintf( "IfMarkInt           : %u-%u\n", $item5,$item6);

    $offset += 18;                          # add bytes processed


    $fmt = sprintf("x%d LLLL",$offset);     # generate the format string
    ($item1, $item2, $item3, $item4) =
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "MaxRcvDataSegmentLen: %u\n", $item1);
    $msg .= sprintf( "MaxSndDataSegmentLen: %u\n", $item2);
    $msg .= sprintf( "ParamMap            : %u\n", $item3);
    $msg .= sprintf( "ParamSentMap        : %u\n", $item4);
                        
    $offset += 16;                          # add bytes processed

# move over to the end
    $offset += 9276;
#    $offset += 37;
    
    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
#
#          Name: FmtIDD
# Call: 
#   FmtTgtDef ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#
#
##############################################################################

sub FmtIDD
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item2a;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $item9;
    my $msg;
    my $ipstr;

    ##########################################
    # connection definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------

    $fmt = sprintf("x%d SCCCCSLa16 ",$offset);      # generate the format string
    ($item1, $item2, $item2a, $item3, $item4, $item5,
              $item6, $item7, $item8) = 
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Flags          : %u\n", $item1);
    $msg .= sprintf( "Werr           : %u\n", $item2);
    $msg .= sprintf( "Rerr           : %u\n", $item2a);
    $msg .= sprintf( "LID            : %u\n", $item3);
    $msg .= sprintf( "PTG            : %u\n", $item4);
    $msg .= sprintf( "Port           : %u\n", $item5);
    $msg .= sprintf( "SG_FD          : %u\n", $item6);
    $msg .= sprintf( "SG_Name        : %s\n", $item7);
    
    $offset += 28;                         # add bytes processed

    $item1 = FmtWwn($bufferPtr, ($offset));
    $item2 = FmtWwn($bufferPtr, ($offset + 8));
    $item3 = FmtWwn($bufferPtr, ($offset + 16));

    $msg .= sprintf( "        INAME (WWN): %16s \n", $item1);
    $msg .= sprintf( "        TNAME (WWN): %16s \n", $item2);
    $msg .= sprintf( "        PNAME (WWN): %16s \n", $item3);

    $offset += 24;                         # add bytes processed

    $fmt = sprintf("x%d NN",$offset);      # generate the format string
    ($item1, $item2) = unpack $fmt , $$bufferPtr;

    $item3 = XIOTech::cmdMgr::long2ip(0, $item1);
    $item4 = XIOTech::cmdMgr::long2ip(0, $item2);
    $msg .= sprintf( "        TARGET IP    : %s \n", $item3);
    $msg .= sprintf( "        INITIATOR IP : %s \n", $item4);

    $offset += 20;                         # move to end of structure

    $$destPtr .= $msg;

    return GOOD;
}
# ISCSI_CODE

##############################################################################

##############################################################################
#
#          Name: FmtPcb
# Call: 
#   FmtPcb ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    struct pcb_t
#    {
#        struct pcb_t * thd;             /* Thread word                          */
#        UINT8  global;                  /* Global reg restore (t/f)             */
#        UINT8  pri;                     /* Process priority                     */
#        UINT8  stat;                    /* Process status                       */
#        UINT8  rsvd1;
#        UINT32 time;                    /* Process timeout                      */
#        UINT32 pfp;                     /* Current saved PFP                    */
#        UINT32 gxx[15];                 /* 'G' registers                        */
#        UINT32 rsvd2;
#        struct frame_t sf0;             /* 1st stack frame                      */
#        struct frame_t sf1;             /* 2nd stack frame                      */
#        struct frame_t sf2;             /* 3rd stack frame                      */
#        UINT32 stack[832];              /* Remainder of the stack space         */
#    } ;
#
#
#
##############################################################################

sub FmtPcb
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version )= @_;
    
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
    my $msg;
    my $available;

    if ( !defined($version) )
    {
        $version = 0;
    }

    $msg = "\n";

    if ( 1 == $version )
    {
        #         volatile struct PCB *pc_thd;       /* Thread word                  */
        #         volatile UINT8  pc_global;         /* Global reg restore (t/f)     */
        #         UINT8           pc_pri;            /* Process priority             */
        #         volatile UINT8  pc_stat;           /* Process status               */
        #         UINT8           pc_rsreg;          /* Restore G Regs.              */
        #         volatile INT32  pc_time;           /* Process timeout              */
        #         SF              *pc_pfp;           /* Current saved PFP            */

        $fmt = sprintf("x%d L CCCC LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =
                            unpack $fmt , $$bufferPtr;


        $msg .= sprintf( "Thread word        : 0x%08x    ", $item1);
        $msg .= sprintf( "Global reg restore (t/f): 0x%02x\n", $item2);
        $msg .= sprintf( "Process priority   : 0x%02x                    ", $item3);
        $msg .= sprintf( "Process status: 0x%02x  \n", $item4);

        $msg .= sprintf( "Restore G Registers: 0x%02x                  ", $item5);
        $msg .= sprintf( "Process timeout : 0x%08x  \n", $item6);
        $msg .= sprintf( "Current saved PFP  : 0x%08x  \n", $item7);
        $msg .= sprintf( "\nProcess stack:  \n");


        $offset += 16;                         # add bytes processed
        $length -= 16;
        $address +=16;

        #         UINT32          pc_gRegs[15];      /* 'G' registers                */

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "Global Reg g0 : 0x%08x", $item1);
        $msg .= sprintf( "          Global Reg g1 : 0x%08x\n", $item2);
        $msg .= sprintf( "Global Reg g2 : 0x%08x", $item3);
        $msg .= sprintf( "          Global Reg g3 : 0x%08x\n", $item4);

        $offset += 16;                         # add bytes processed
        $length -= 16;
        $address +=16;

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "Global Reg g4 : 0x%08x", $item1);
        $msg .= sprintf( "          Global Reg g5 : 0x%08x\n", $item2);
        $msg .= sprintf( "Global Reg g6 : 0x%08x", $item3);
        $msg .= sprintf( "          Global Reg g7 : 0x%08x\n", $item4);

        $offset += 16;                         # add bytes processed
        $length -= 16;
        $address +=16;

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "Global Reg g8 : 0x%08x", $item1);
        $msg .= sprintf( "          Global Reg g9 : 0x%08x\n", $item2);
        $msg .= sprintf( "Global Reg g10: 0x%08x", $item3);
        $msg .= sprintf( "          Global Reg g11: 0x%08x\n", $item4);

        $offset += 16;                         # add bytes processed
        $length -= 16;
        $address +=16;

        $fmt = sprintf("x%d LL L ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "Global Reg g12: 0x%08x", $item1);
        $msg .= sprintf( "          Global Reg g13: 0x%08x\n", $item2);
        $msg .= sprintf( "Global Reg g14: 0x%08x\n\n", $item3);

        $offset += 12;                         # add bytes processed
        $length -= 12;
        $address +=12;

        #         UINT32          rsvd2;             /* save for g15.                */
        $fmt = sprintf("x%d L ",$offset);      # generate the format string
        ($item1) =
                 unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "Reserved: 0x%08x\n\n", $item1);

        $offset += 4;                         # add bytes processed
        $length -= 4;
        $address +=4;

        #         char            pc_fork_name[XIONAME_MAX];/* name of process.     */

        $item1 = FmtString ($bufferPtr, $offset, 32);
        $msg .= sprintf ("Name of the Process: %32s \n\n\n", $item1);

        $offset += 32;                         # add bytes processed
        $length -= 32;
        $address +=32;

        $$destPtr .= $msg;
    }
    else
    {
        #        struct pcb_t * thd;             /* Thread word                          */
        #        UINT8  global;                  /* Global reg restore (t/f)             */
        #        UINT8  pri;                     /* Process priority                     */
        #        UINT8  stat;                    /* Process status                       */
        #        UINT8  rsvd1;
        #        UINT32 time;                    /* Process timeout                      */
        #        UINT32 pfp;                     /* Current saved PFP                    */

        $fmt = sprintf("x%d L CCCC LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "      Thread word: 0x%08x    ", $item1);
        $msg .= sprintf( "Global reg restore (t/f): 0x%02x\n", $item2);
        $msg .= sprintf( " Process priority: 0x%02x                    ", $item3);
        $msg .= sprintf( "Process status: 0x%02x  \n", $item4);

        $msg .= sprintf( "         reserved: 0x%02x                   ", $item5);
        $msg .= sprintf( "Process timeout: 0x%08x  \n", $item6);
        $msg .= sprintf( "Current saved PFP: 0x%08x  \n", $item7);
        $msg .= sprintf( "\nProcess stack:  \n");

        $offset += 16;                         # add bytes processed
        $length -= 16;
        $address +=16;

        $$destPtr .= $msg;

        #        struct frame_t sf0;             /* 1st stack frame                      */
        #        struct frame_t sf1;             /* 2nd stack frame                      */
        #        struct frame_t sf2;             /* 3rd stack frame                      */
        #        UINT32 stack[832];              /* Remainder of the stack space         */
    }

    # just dump the remaining part as hex


    # the following lines 'fix' the length in the event we only have a partial
    # record. We need this as the length may not be correct for a shorter record.

    $available = length( $$bufferPtr ) - $offset;   # bytes available to do

    if ( $length > $available ) 
    { 
#        print " avail = $available, len = $length, offset = $offset \n\n";
#        sleep(10);
    
        $length = $available; 
    }

    # now generate the hex data.

    $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);

    return GOOD;

}


##############################################################################


##############################################################################



##############################################################################

##############################################################################
#
#          Name: FmtMlmt
# Call: 
#   FmtMlmt ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        # --- Begin MLMT data structure ----------------------------------------------
#        #
#                .set    mlmt_link,0             # link list field               <w>
#                .set    mlmt_sn,mlmt_link+4     # MAGNITUDE node serial number  <w>
#                .set    mlmt_dtmthd,mlmt_sn+4   # DTMT list head pointer        <w>
#                .set    mlmt_dtmttl,mlmt_dtmthd+4 # DTMT list tail pointer      <w>
#        #                                                               ******0x10****
#                .set    mlmt_dgdtmt,mlmt_dtmttl+4 # datagram message last DTMT  <w>
#                                                #  used address
#        #
#                .set    mlmt_flags,mlmt_dgdtmt+4 # Flags                        <b>
#                    .set    MLMT_POLL_PATH,0    #   Bit 0 = Poll Path
#                    .set    MLMT_LOST_ALL_SENT,1 #  Bit 1 = Lost All Paths Msg Sent
#                                                #   Bits 2-7 = Reserved
#        #
#        #   Reserved 3 bytes                                                   3<b>
#        #
#                .set    mlmt_lastpolldtmt,mlmt_flags+4 # Last polled DTMT       <w>
#        #
#        #   Reserved 4 bytes                                                    <w>
#        #
#        # --- End MLMT data structure ------------------------------------------------
#
#        .set    mlmt_size,mlmt_lastpolldtmt+8 # size of MLMT
#
#
#
#
##############################################################################

sub FmtMlmt
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;

    ##########################################
    # target definitions
    ##########################################

    $msg = "\n";

    # --- Begin structure -------------------------------------------------
    #                .set    mlmt_link,0             # link list field               <w>
    #                .set    mlmt_sn,mlmt_link+4     # MAGNITUDE node serial number  <w>
    #                .set    mlmt_dtmthd,mlmt_sn+4   # DTMT list head pointer        <w>
    #                .set    mlmt_dtmttl,mlmt_dtmthd+4 # DTMT list tail pointer      <w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "       link list field: 0x%08x       ", $item1);
    $msg .= sprintf( "        MAG node sn: 0x%08x             \n", $item2);
    $msg .= sprintf( "DTMT list head pointer: 0x%08x    ", $item3);
    $msg .= sprintf( "DTMT list tail pointer: 0x%08x  \n", $item4);

    $offset += 16;                         # add bytes processed


    #                .set    mlmt_dgdtmt,mlmt_dtmttl+4 # datagram message last DTMT  <w>
    #                                                #  used address
    #        #
    #                .set    mlmt_flags,mlmt_dgdtmt+4 # Flags                        <b>
    #                    .set    MLMT_POLL_PATH,0    #   Bit 0 = Poll Path
    #                    .set    MLMT_LOST_ALL_SENT,1 #  Bit 1 = Lost All Paths Msg Sent
    #                                                #   Bits 2-7 = Reserved
    #        #
    #        #   Reserved 3 bytes                                                   3<b>
    #        #
    #                .set    mlmt_lastpolldtmt,mlmt_flags+4 # Last polled DTMT       <w>
    #        #
    #        #   Reserved 4 bytes                                                    <w>


    $fmt = sprintf("x%d L C CS L L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "             last DTMT: 0x%08x    ", $item1);
    $msg .= sprintf( "Flags: 0x%02x   ", $item2);
    $msg .= sprintf( "reserved: 0x%02x    ", $item3);
    $msg .= sprintf( "reserved: 0x%04x \n", $item4);
    $msg .= sprintf( "      Last polled DTMT: 0x%08x            ", $item5);
    $msg .= sprintf( "      reserved: 0x%08x \n", $item6);

    $offset += 16;                         # add bytes processed


 


    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
##############################################################################

##############################################################################
#
#          Name: FmtFltRecP
# Call: 
#   FmtFltRecP ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#         - byte 0 of fr_parm0 is the type defined below.
#         - fr_parm1 should be the ILT when appropriate
#         - fr_parm2 should be the VRP/RRP/SRP/PRP when appropriate
#         - fr_parm3 is user defined
#
#       Viewing the data:
#       - data variable fr_queue points to the circular queue pointers
#       - the 3 words of queue pointers are BEGIN, NEXT, BEGIN, and END
#       - the data immediately follows the queue pointers
#           - words 0-3 are parms 0-3
#           If DEBUG_FLIGHTREC_TIME is enabled 4 additional words are added:
#           - word 4 is the timer interrupt tick counter from the Kii struct
#           - word 5 is the incremental time in useconds
#           - word 6-7 are spares
#
#
#
#
#
#
##############################################################################

sub FmtFltRecP
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;

    my $i;
    my $used = 0;

    ##########################################
    # target definitions
    ##########################################                                                

    $msg = "\n";

    $msg .="Address     type  modifiers       ILT(?)      V/R/S/PRP   user defined  Name\n";
    $msg .="----------  ----  --------------  ----------  ----------  ------------  ---------------------------\n";



    while ( $length > 15 )
    {

        #         - byte 0 of fr_parm0 is the type defined below.
        #         - fr_parm1 should be the ILT when appropriate
        #         - fr_parm2 should be the VRP/RRP/SRP/PRP when appropriate
        #         - fr_parm3 is user defined

        $fmt = sprintf("x%d C CCC LLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("0x%08x  ", ($address + $used));
        
        $msg .= sprintf( "0x%02x  ", $item1);
        $msg .= sprintf( "0x%02x 0x%02x 0x%02x  ", $item2, $item3, $item4);
        $msg .= sprintf( "0x%08x  ", $item5);
        $msg .= sprintf( "0x%08x  ", $item6);
        $msg .= sprintf( "0x%08x    ", $item7);
        $msg .= FltRecPLookUp($item1)."\n";


        $offset += 16;                         # add bytes processed
        $length -= 16;
        $used += 16;
    }



    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
##############################################################################

##############################################################################
#
#          Name: FmtMrpTrcP
# Call: 
#   FmtMrpTrcP ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        #       Load g0 with the event id, g1 with the associated data you want to
#        #       save, then call this function. The parms will be saved in a circular
#        #       queue.
#        #
#        defTraceQue:
#                .word   defTraceQue+16
#                .word   defTraceQue+16
#                .word   defTraceQueEnd-16
#                .word   1                       # Enable it
#                .space  16*1024,0               # Allocate 1024 entries
#        defTraceQueEnd:
#                ld      K_ii+ii_time,r8         # read gross timer
#                ld      TCR0,r9                 # read hdwr timer 0 reload counter
#                intctl  r13,r13                 # enable interrupts
#                stl     g0,(r5)                 # store event data in the queue
#                stl     r8,8(r5)
#        #
#
#
#
#
#
##############################################################################

sub FmtMrpTrcP
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;
    my $id;
    my $data;
    my $tCourse;
    my $tFine;
    my $dataDesc;
    my $count;
    my $msg;

    my $i;
    my $used = 0;
    my $t0;
    my $t0d;
    my $t0Old;
    my $t1;
    my $t1d;
    my $t1Old;

    $t1Old = 0;
    $t0Old = 0;

    $msg = "\n";

    # Initialize count of number of trace lines
    $ count = 1;

    while ( $length > 15 )
    {
        #   - byte 0 of fr_parm0 is the type defined below.
        #   - fr_parm1 should be the ILT when appropriate
        #   - fr_parm2 should be the VRP/RRP/SRP/PRP when appropriate
        #   - fr_parm3 is user defined
        
        # MRS: Since I'm not sure how this data compares to the CCB MRP
        # data, I'll start by trying to make the order of the columns
        # the same.  It looks like they are - 
        #
        # Absolute_Time  Delta_Time  ID_Description  Data_Description  Data
        #

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($id, $data, $tCourse, $tFine) = unpack $fmt, $$bufferPtr;
    
        # Old msg lines.  Leave as comments for now.
        # $msg .= sprintf("0x%08x: ", ($address + $used));
        # $msg .= sprintf( " Event ID: 0x%08x ", $id);
        # $msg .= sprintf( " Data: 0x%08x  ", $data);


        # Absolute and delta time calculations.  This is different than 
        # the CCB and I don't understand either one so for now I'll leave
        # it as is.
        $t0 = int($tCourse / 8);                                    # whole seconds
        $t1 = ($tCourse % 8) * 125000000 + ((125000000 - $tFine));  # fractional seconds

        $t0d = $t0 - $t0Old;                    # compute deltas
        $t1d = $t1 - $t1Old;

        $t1d = ($t0d * 1000000000) + $t1d;      # combine to single number
        
        # Old msg lines.  Leave as comments for now.
        # $msg .= sprintf("  time:%6d sec %10.3f usec, delta;%17.3f us  - ", 
        #                $t0, $t1/1000, $t1d/1000);
        
        $t0Old = $t0;   # save old value
        $t1Old = $t1;   # save old value

        # Old msg lines.  Leave as comments for now.
        # $msg .= MrpPLookUp($data);
        # $msg .= "\n";

        # Get a string description of the data
        $dataDesc = MrpPLookUp($data);


        # Build the msg string for this entry
        $msg .= sprintf("%-5u %6ds %10.3fus  %17.3fus    0x%08X  %40s  / 0x%08X\n",
                         $count++, $t0, $t1/1000, $t1d/1000, $id, $dataDesc, $data); 
        
        $offset += 16;  # add bytes processed
        $length -= 16;
        $used += 16;
    }



    $$destPtr .= $msg;

    return GOOD;

}





##############################################################################

##############################################################################


sub FmtItrace
{

    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;


    my $msg;
    my $toGo;
    my $name;
    my $recType;
    my $chipInst;
    my $alpa;
    my $lun;
    my $data1;
    my $data2;
    my $timeStamp;
    my $zeroCount;

    my %strings;

    $strings{    1  } = "Initiator trace -  online event";
    $strings{    2  } = "Initiator trace -  offline event";
    $strings{ 0x40  } = "Initiator trace -  Test Unit Ready ";
    $strings{ 0x41  } = "Initiator trace -  Start Stop Unit";
    $strings{ 0x42  } = "Initiator trace -  Inquire LUN";
    $strings{ 0x43  } = "Initiator trace -  Mode Sense ";
    $strings{ 0x45  } = "Initiator trace -  *** fabric GAN";
    $strings{ 0x46  } = "Initiator trace -  *** fabric discovery        ";
    $strings{ 0x4e  } = "Initiator trace -  discovery task reissue";
    $strings{ 0x4f  } = "Initiator trace -  i command complete";
    $strings{ 0x50  } = "Initiator trace -  open session";
    $strings{ 0x51  } = "Initiator trace -  close session";
    $strings{ 0x52  } = "Initiator trace -  request session parameters";
    $strings{ 0x53  } = "Initiator trace -  send SCSI command";
    $strings{ 0x54  } = "Initiator trace -  task mamagement function";
    $strings{ 0x60  } = "Initiator trace -  enable task";
    $strings{ 0x70  } = "Initiator trace -  function complete";
    $strings{ 0x71  } = "Initiator trace -  SCSI command complete";
    $strings{ 0x80  } = "Initiator trace -  target identified";
    $strings{ 0x81  } = "Initiator trace -  discovery complete";
    $strings{ 0x82  } = "Initiator trace -  target gone";
    $strings{ 0xe0  } = "Initiator trace -  bad provider or requesor id";
    $strings{ 0xe1  } = "Initiator trace -  task timeout";
    $strings{ 0xf0  } = "Initiator trace -  private loop discovery start";
    $strings{ 0xf1  } = "Initiator trace -  gan trace";
    $strings{ 0xfc  } = "Initiator trace -  1st gan trace";
    $strings{ 0xf2  } = "Initiator trace -  fabric discover loop";
    $strings{ 0xf3  } = "Initiator trace -  fabric discover error";
    $strings{ 0xf8  } = "Initiator trace -  fabric login error";
    $strings{ 0xff  } = "Initiator trace -  debug trace";


    #   address   lun                            timestamp
    #             |  AL-PA                       |
    #             |  |  chip instance            |
    #             |  |  |  record type           |
    #                                   ILT?     |
    #             |  |  |  |  data      |        |
    #   A0883B80  00 00 03 03 002A0000  00000000 86B25133  ......*.....3Q..


    #   A0883B90  3A A0 03 01 00000010  00000081 88DB1B4E  ...:........N...
    #   A0883BA0  3A A0 03 02 00000012  00000024 00000000  ...:....$.......

    $toGo = $length;
    $zeroCount = 0;

    while( $toGo >= 16 )
    {
        
 

        $fmt = sprintf("x%d CCCC LLL",$offset);      # generate the format string
        ($recType, $chipInst, $alpa, $lun, $data1, $data2, $timeStamp) =  
                            unpack $fmt , $$bufferPtr;
        #msg .= sprintf( "magicNumber: 0x%08x ", $item1);



        # see if we got anything
        if (  ( $recType !=0 )   ||
              ( $chipInst !=0 )  ||
              ( $alpa !=0 )      ||
              ( $lun !=0 )       ||
              ( $data1 !=0 )     ||
              ( $data2 !=0 )     ||
              ( $timeStamp !=0 ) )
        {
            # at least one item was non-0

            # get a string for the record type
            if ( $strings{$recType} )
            {
                $name = $strings{$recType};
            }
            else
            {
                $name = "undefined";
            }

            $msg = sprintf("addr 0x%08x: ", $address);
            $msg .= sprintf("time:0x%08x ",$timeStamp); 
            $msg .= sprintf("chip:%3d ",$chipInst); 
            $msg .= sprintf("AL-PA:0x%02x ",$alpa); 
            $msg .= sprintf("LUN:%3d ",$lun); 
            $msg .= sprintf("data1:0x%08x ",$data1); 
            $msg .= sprintf("data2:0x%08x ",$data2); 
            $msg .= $name . "\n"; 

            $$destPtr .= $msg;

        }
        else
        {
            # skip this one, count it
            $zeroCount++;
        }

        $address += 16;
        $toGo -= 16;
        $offset += 16;

    }

    $msg = "\n$zeroCount lines with only zeros not printed.";
    
    $$destPtr .= $msg;

    return GOOD;
}


##############################################################################
##############################################################################
# Name:     FmtTrace()
#
# Desc:     Format Trace debug data  into a string  
#
# Input:    data , starting address, length of the data
#
#      
##############################################################################
sub FmtTrace
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
    my $fmt;

    my $msg;
    my $toGo;
    my $name;
    my $recType;
    my $chipInst;
    my $exchID;
    my $lun;
    my $data1;
    my $data2;
    my $timeStamp;
    my $zeroCount;
    my $string;

    my %strings;

    $strings{ 1     } = "Trace -   XL\$receive_io input trace record ";
    $strings{ 2     } = "Trace -   c\$cdb4 input trace record ";
    $strings{ 3     } = "Trace -   c\$imno4 input trace record  ";
    $strings{ 4     } = "Trace -   c\$offl4 input trace record ";
    $strings{ 5     } = "Trace -   call MAG\$submit_vrp trace record ";
    $strings{ 6     } = "Trace -   mag1_srpreq input trace record  ";
    $strings{ 7     } = "Trace -   mag1_srpcomp input trace record ";
    $strings{ 8     } = "Trace -   mag1_MAGcomp input trace record         ";
    $strings{ 9     } = "Trace -   call ISP\$receive_io trace record ";
    $strings{ 0x0a  } = "Trace -   nag1_iocr input trace record ";
    $strings{ 0x0e  } = "Trace -   data trace record ";
    $strings{ 0x0f  } = "Trace -   SENSE data trace record ";


    #   address   lun                            timestamp
    #             |  AL-PA                       |
    #             |  |  chip instance            |
    #             |  |  |  record type           |
    #                                   ILT?     |
    #             |  |  |  |  data      |        |
    #   A0883B80  00 00 03 03 002A0000  00000000 86B25133  ......*.....3Q..


    #   A0883B90  3A A0 03 01 00000010  00000081 88DB1B4E  ...:........N...
    #   A0883BA0  3A A0 03 02 00000012  00000024 00000000  ...:....$.......
    #

    $toGo = $length;
    $zeroCount = 0;

    while( $toGo >= 16 )
    {
        

        $fmt = sprintf("x%d CCS LLL",$offset);      # generate the format string
        ($recType, $chipInst, $exchID, $data1, $data2, $timeStamp) =  
                            unpack $fmt , $$bufferPtr;
        # extract a row of data
        

        # see if we got anything
        if (  ( $recType !=0 )   ||
              ( $chipInst !=0 )  ||
              ( $exchID !=0 )    ||
              ( $data1 !=0 )     ||
              ( $data2 !=0 )     ||
              ( $timeStamp !=0 ) )
        {
            # at least one item was non-0

            # get a string for the record type
            if ( $strings{$recType} )
            {
                $name = $strings{$recType};
            }
            else
            {
                $name = "undefined";
            }

            $msg = sprintf("addr 0x%08x: ", $address);
            $msg .= sprintf("time:0x%08x ",$timeStamp); 
            $msg .= sprintf("chip:%3d ",$chipInst); 
            $msg .= sprintf("exchID:0x%04x ",$exchID); 
            $msg .= sprintf("data1:0x%08x ",$data1); 
            $msg .= sprintf("data2:0x%08x ",$data2); 
            $msg .= $name . "\n"; 

            $$destPtr .= $msg;

        }
        else
        {
            # skip this one, count it
            $zeroCount++;
        }

        $address += 16;
        $toGo -= 16;
        $offset += 16;

    }



    $msg = "\n$zeroCount lines with only zeros not printed.";
    
    $$destPtr .= $msg;

    return GOOD;
}


##############################################################################

##############################################################################
#
#          Name: FmtDefEq
# Call: 
#   FmtDefEq ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin structure -------------------------------------------------
#                                                                  *****
#        .set    qu_head,0               # Queue head                <w>
#        .set    qu_tail,qu_head+4       # Queue tail                <w>
#        .set    qu_qcnt,qu_tail+4       # Queue count               <w>
#        .set    qu_pcb,qu_qcnt+4        # Associated PCB            <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
#
#
##############################################################################

sub FmtExecQ
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    qu_head,0               # Queue head                <w>
    #        .set    qu_tail,qu_head+4       # Queue tail                <w>
    #        .set    qu_qcnt,qu_tail+4       # Queue count               <w>
    #        .set    qu_pcb,qu_qcnt+4        # Associated PCB            <w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "Queue head: 0x%08x ", $item1);
    $msg .= sprintf( "Queue tail: 0x%08x ", $item2);
    $msg .= sprintf( "Queue count: 0x%08x ", $item3);
    $msg .= sprintf( "Associated PCB: 0x%08x ", $item4);
    $msg .= sprintf( "\n");


    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################

##############################################################################
#
#          Name: FmtLinkQCS
# Call: 
#   FmtLinkQCS ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin structure -------------------------------------------------
#
#        .set    qc_flags,0              # Operation flags
#        .set    qc_stat,qc_flags+1      # Queue status bits
#        .set    qc_nent,qc_stat+1       # Number of queued entries in all QBs
#        .set    qc_pcb0,qc_nent+2       # PCB of q0 handler task
#        .set    qc_pcb1,qc_pcb0+4       # PCB of q1 handler task
#        .set    qc_pcb2,qc_pcb1+4       # PCB of q2 handler task
#        .set    qc_pcb3,qc_pcb2+4       # PCB of q3 handler task
#        .set    qc_pcb4,qc_pcb3+4       # PCB of q4 handler task
#        .set    qc_pcb5,qc_pcb4+4       # PCB of q5 handler task
#        .set    qc_pcb6,qc_pcb5+4       # PCB of q6 handler task
#        .set    qc_pcb7,qc_pcb6+4       # PCB of q7 handler task
#        .set    qc_qb0,qc_pcb7+4        # first QB
#        .set    qc_qb1,qc_qb0+4         # second QB
#        .set    qc_qb2,qc_qb1+4         # third QB
#        .set    qc_qb3,qc_qb2+4         # fourth QB
#        .set    qc_qb4,qc_qb3+4         # fifth QB
#        .set    qc_qb5,qc_qb4+4         # sixth QB
#        .set    qc_qb6,qc_qb5+4         # seventh QB
#        .set    qc_qb7,qc_qb6+4         # eighth QB
#
# --- End structure ---------------------------------------------------
#
#
##############################################################################

sub FmtLinkQCS
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    qc_flags,0              # Operation flags
    #        .set    qc_stat,qc_flags+1      # Queue status bits
    #        .set    qc_nent,qc_stat+1       # Number of queued entries in all QBs

    $fmt = sprintf("x%d CCS ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "Operation flags: 0x%02x    ", $item1);
    $msg .= sprintf( "Queue status bits: 0x%02x    ", $item2);
    $msg .= sprintf( "Number of queued entries: 0x%04x ", $item3);
    $msg .= sprintf( "\n");

    $offset += 4;

    #        .set    qc_pcb0,qc_nent+2       # PCB of q0 handler task
    #        .set    qc_pcb1,qc_pcb0+4       # PCB of q1 handler task
    #        .set    qc_pcb2,qc_pcb1+4       # PCB of q2 handler task
    #        .set    qc_pcb3,qc_pcb2+4       # PCB of q3 handler task

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "PCB of q0 handler task: 0x%08x       ", $item1);
    $msg .= sprintf( "PCB of q1 handler task: 0x%08x \n", $item2);
    $msg .= sprintf( "PCB of q2 handler task: 0x%08x       ", $item3);
    $msg .= sprintf( "PCB of q3 handler task: 0x%08x \n", $item4);

    $offset += 16;

    #        .set    qc_pcb4,qc_pcb3+4       # PCB of q4 handler task
    #        .set    qc_pcb5,qc_pcb4+4       # PCB of q5 handler task
    #        .set    qc_pcb6,qc_pcb5+4       # PCB of q6 handler task
    #        .set    qc_pcb7,qc_pcb6+4       # PCB of q7 handler task

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "PCB of q4 handler task: 0x%08x       ", $item1);
    $msg .= sprintf( "PCB of q5 handler task: 0x%08x \n", $item2);
    $msg .= sprintf( "PCB of q6 handler task: 0x%08x       ", $item3);
    $msg .= sprintf( "PCB of q7 handler task: 0x%08x \n", $item4);

    $offset += 16;

    #        .set    qc_qb0,qc_pcb7+4        # first QB
    #        .set    qc_qb1,qc_qb0+4         # second QB
    #        .set    qc_qb2,qc_qb1+4         # third QB
    #        .set    qc_qb3,qc_qb2+4         # fourth QB

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "first QB: 0x%08x ", $item1);
    $msg .= sprintf( " second QB: 0x%08x ", $item2);
    $msg .= sprintf( "  third QB: 0x%08x ", $item3);
    $msg .= sprintf( " fourth QB: 0x%08x \n", $item4);

    $offset += 16;


    #        .set    qc_qb4,qc_qb3+4         # fifth QB
    #        .set    qc_qb5,qc_qb4+4         # sixth QB
    #        .set    qc_qb6,qc_qb5+4         # seventh QB
    #        .set    qc_qb7,qc_qb6+4         # eighth QB


    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "fifth QB: 0x%08x ", $item1);
    $msg .= sprintf( "  sixth QB: 0x%08x ", $item2);
    $msg .= sprintf( "seventh QB: 0x%08x ", $item3);
    $msg .= sprintf( " eighth QB: 0x%08x \n", $item4);



    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################

##############################################################################
#
#          Name: FmtDefragT
# Call: 
#   FmtDefragT ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#  void N_tracedefrag(pNVRSOS, UINT16, pSOS, UINT16, UINT32);
#
#
#
##############################################################################

sub FmtDefragT
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;
    my $toGo;


    $msg = "\n";

    $toGo = $length;

    while( $toGo >= 16 )
    {
        #  void N_tracedefrag(pNVRSOS, UINT16, pSOS, UINT16, UINT32);

        $fmt = sprintf("x%d LSLSL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "pNVRSOS: 0x%08x   ", $item1);
        $msg .= sprintf( "UINT16: 0x%04x   ", $item2);
        $msg .= sprintf( "pSOS: 0x%08x   ", $item3);
        $msg .= sprintf( "UINT16: 0x%04x   ", $item2);
        $msg .= sprintf( "UINT32: 0x%08x   ", $item4);
        $msg .= sprintf( "\n");


        $address += 16;
        $toGo -= 16;
        $offset += 16;

    }

    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################


##############################################################################
#
#          Name: FmtIlmtWet
# Call: 
#   FmtIlmtWet ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin structure --------------------------------------------------------
#
#        .set    ilm_link,0              # Link list field for ILMTs     <w>
#                                        #  assoc. with same VDMT
#        .set    ilm_imt,ilm_link+4      # Assoc. IMT address            <w>
#        .set    ilm_vdmt,ilm_imt+4      # Assoc. VDMT address           <w>
#        .set    ilm_ehand,ilm_vdmt+4    # Event handler table           <w>
#                                                               ***********
#        .set    ilm_cmdtbl,ilm_ehand+4  # Command index table address   <w>
#        .set    ilm_cmdhand,ilm_cmdtbl+4 # Command handler table address<w>
#        .set    ilm_whead,ilm_cmdhand+4 # Working tasks head pointer    <w>
#        .set    ilm_wtail,ilm_whead+4   # Working tasks tail pointer    <w>
#                                                               ***********
#        .set    ilm_ahead,ilm_wtail+4   # Aborted tasks head pointer    <w>
#        .set    ilm_atail,ilm_ahead+4   # Aborted tasks tail pointer    <w>
#        .set    ilm_bhead,ilm_atail+4   # Blocked ILT queue head ptr.   <w>
#        .set    ilm_btail,ilm_bhead+4   # Blocked ILT queue tail ptr.   <w>
#                                                               ***********
#        .set    ilm_snshead,ilm_btail+4 # Pending sense queue head ptr. <w>
#        .set    ilm_snstail,ilm_snshead+4 # Pending sense queue tail    <w>
#        .set    ilm_dfenv,ilm_snstail+4 # Pointer to default            <w>
#                                        #  environment table
#        .set    ilm_wkenv,ilm_dfenv+4   # Pointer to working            <w>
#                                        #  environment table
#                                                               ***********
#        .set    ilm_flag1,ilm_wkenv+4   # Flag byte #1                  <b>
#                                        # Note: Any flags set in this byte
#                                        #       will slow down initial
#                                        #       command processing.
#                                        # Bit 7 = 
#                                        #     6 = 
#                                        #     5 = 
#                                        #     4 = 
#                                        #     3 = 
#                                        #     2 = 1 = pending SENSE
#                                        #     1 = 1 = ACA active
#                                        #     0 = 1 = flushing commands
#        .set    ilm_flag2,ilm_flag1+1   # Flag byte #2                  <b>
#                                        # Bit 7 = 
#                                        #     6 = 
#                                        #     5 = 
#                                        #     4 = 1=MODE parameters changed
#                                        #     3 = 1=Tasks cleared by another
#                                        #            initiator
#                                        #     2 = 1=Bus device reset function
#                                        #            occurred
#                                        #     1 = 1=SCSI bus reset received
#                                        #     0 = 1=Power-on reset occurred
#        .set    ilm_attr,ilm_flag2+1    # Attributes                    <s>
#        .set    ilm_cimt,ilm_attr+2     # assoc. CIMT address           <w>
#        .set    ilm_origcmdhand,ilm_cimt+4 # original command handler   <w>
#                                        #  table address
#        .set    ilm_enblcnt,ilm_origcmdhand+4 # # tasks enabled         <s>
#
# --- Reserved 2
#
#        .set    ilm_ltmt,ilm_enblcnt+4 # assoc. LTMT address            <w>
#
# --- End structure ----------------------------------------------------------
#
#
##############################################################################

sub FmtIlmtWet
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    ilm_link,0              # Link list field for ILMTs     <w>
    #                                        #  assoc. with same VDMT
    #        .set    ilm_imt,ilm_link+4      # Assoc. IMT address            <w>
    #        .set    ilm_vdmt,ilm_imt+4      # Assoc. VDMT address           <w>
    #        .set    ilm_ehand,ilm_vdmt+4    # Event handler table           <w>

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "              ILMT Link list: 0x%08x       ", $item1);
    $msg .= sprintf( "             Assoc. IMT addr: 0x%08x    \n", $item2);
    $msg .= sprintf( "            Assoc. VDMT addr: 0x%08x       ", $item3);
    $msg .= sprintf( "         Event handler table: 0x%08x", $item4);
    $msg .= sprintf( "\n");

    $offset += 16;
    $length -= 16;
    $address += 16;

    #        .set    ilm_cmdtbl,ilm_ehand+4  # Command index table address   <w>
    #        .set    ilm_cmdhand,ilm_cmdtbl+4 # Command handler table address<w>
    #        .set    ilm_whead,ilm_cmdhand+4 # Working tasks head pointer    <w>
    #        .set    ilm_wtail,ilm_whead+4   # Working tasks tail pointer    <w>
    #

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "        Cmd index table addr: 0x%08x       ", $item1);
    $msg .= sprintf( "      Cmd handler table addr: 0x%08x \n", $item2);
    $msg .= sprintf( "      Working tasks head ptr: 0x%08x       ", $item3);
    $msg .= sprintf( "      Working tasks tail ptr: 0x%08x \n", $item4);

    $offset += 16;
    $length -= 16;
    $address += 16;

    #        .set    ilm_ahead,ilm_wtail+4   # Aborted tasks head pointer    <w>
    #        .set    ilm_atail,ilm_ahead+4   # Aborted tasks tail pointer    <w>
    #        .set    ilm_bhead,ilm_atail+4   # Blocked ILT queue head ptr.   <w>
    #        .set    ilm_btail,ilm_bhead+4   # Blocked ILT queue tail ptr.   <w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  Aborted tasks head pointer: 0x%08x       ", $item1);
    $msg .= sprintf( "  Aborted tasks tail pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "  Blocked ILT queue head ptr: 0x%08x       ", $item3);
    $msg .= sprintf( "  Blocked ILT queue tail ptr: 0x%08x \n", $item4);

    $offset += 16;
    $length -= 16;
    $address += 16;

    #        .set    ilm_snshead,ilm_btail+4 # Pending sense queue head ptr. <w>
    #        .set    ilm_snstail,ilm_snshead+4 # Pending sense queue tail    <w>
    #        .set    ilm_dfenv,ilm_snstail+4 # Pointer to default            <w>
    #                                        #  environment table
    #        .set    ilm_wkenv,ilm_dfenv+4   # Pointer to working            <w>
    #                                        #  environment table

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "Pending sense queue head ptr: 0x%08x       ", $item1);
    $msg .= sprintf( "Pending sense queue tail ptr: 0x%08x \n", $item2);
    $msg .= sprintf( "Pointer to default env table: 0x%08x       ", $item3);
    $msg .= sprintf( "Pointer to working env table: 0x%08x \n", $item4);


    $offset += 16;
    $length -= 16;
    $address += 16;


    #        .set    ilm_flag1,ilm_wkenv+4   # Flag byte #1                  <b>
    #                                        # Note: Any flags set in this byte
    #                                        #       will slow down initial
    #                                        #       command processing.
    #                                        # Bit 7 = 
    #                                        #     6 = 
    #                                        #     5 = 
    #                                        #     4 = 
    #                                        #     3 = 
    #                                        #     2 = 1 = pending SENSE
    #                                        #     1 = 1 = ACA active
    #                                        #     0 = 1 = flushing commands
    #        .set    ilm_flag2,ilm_flag1+1   # Flag byte #2                  <b>
    #                                        # Bit 7 = 
    #                                        #     6 = 
    #                                        #     5 = 
    #                                        #     4 = 1=MODE parameters changed
    #                                        #     3 = 1=Tasks cleared by another
    #                                        #            initiator
    #                                        #     2 = 1=Bus device reset function
    #                                        #            occurred
    #                                        #     1 = 1=SCSI bus reset received
    #                                        #     0 = 1=Power-on reset occurred
    #        .set    ilm_attr,ilm_flag2+1    # Attributes                    <s>
    #        .set    ilm_cimt,ilm_attr+2     # assoc. CIMT address           <w>
    #        .set    ilm_origcmdhand,ilm_cimt+4 # original command handler   <w>
    #                                        #  table address
    #        .set    ilm_enblcnt,ilm_origcmdhand+4 # # tasks enabled         <s>
    #
    # --- Reserved 2


    $fmt = sprintf("x%d CCS L L SS ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    #                "----------------------------:      x       "
    $msg .= sprintf( "                Flag byte #1: %08b         ", $item1);
    $msg .= sprintf( "                Flag byte #2: %08b \n", $item2);
    $msg .= sprintf( "                  Attributes: 0x%04x           ", $item3);
    $msg .= sprintf( "         assoc. CIMT address: 0x%08x \n", $item4);
    $msg .= sprintf( " orig cmd handler table addr: 0x%08x       ", $item5);
    $msg .= sprintf( "              # tasks enable: 0x%04x \n", $item6);
    $msg .= sprintf( "                    reserved: 0x%04x \n", $item7);

    $offset += 16;
    $length -= 16;
    $address += 16;


    #        .set    ilm_ltmt,ilm_enblcnt+4 # assoc. LTMT address            <w>

    $fmt = sprintf("x%d L",$offset);      # generate the format string
    ($item1) =     unpack $fmt , $$bufferPtr;
    
    #                "----------------------------:      x       "
    $msg .= sprintf( "                LTMT address: 0x%08x\n ", $item1);

    $offset += 4;
    $length -= 4;
    $address += 4;

    $msg .= "\nEnvironment Table:\n";
    $$destPtr .= $msg;

    # just dump the remaining part as hex

    $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);


    return GOOD;

}

##############################################################################

##############################################################################


##############################################################################
#
#          Name: FmtImt
# Call: 
#   FmtImt ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $cPtr, $version);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, cyurrently not 
#                        used, but may be used if the data differs between 
#                        processors
#           $address - Memory address where the data came from.
#           $cPtr - pointer to a hask of key constants.
#
#  Return: GOOD or ERROR
#
#  NOTE: This functions requires the 'constants' hash to operate properly.
#    
# The data structure being decoded...
#
#        struct im_t
#        {
#            struct im_t * link;             /* IMT link list field              <w> */
#            UINT8  fcaddr;                  /* Assoc. FC address (Loop ID)      <b> */
#            UINT8  vpid;                    /* Assoc. Virtual port ID           <b> */
#            UINT16 rsvd1;
#            struct ci_t * cimt;             /* Assoc. CIMT address              <w> */
#            void (*ehand)();                /* default event handler table      <w> */

#            UINT32 rsvd2[2];
#            UINT64 mac;                     /* FC MAC address (WWN)            8<b> */

#            UINT8  rsvd3;
#            UINT8  flags;                   /* Flags byte                       <b> */
#            UINT8  rsvd4;
#            UINT8  pri;                     /* priority                         <b> */
#            UINT16 cfgsiz;                  /* configuration record size        <s> */
#            UINT16 sid;                     /* Server ID - index into SDX       <s> */
#            UINT16 tid;                     /* Target ID                        <s> */
#            UINT16 rsvd5;
#            UINT32 pendvrp;                 /* pending image assoc. VRP         <w> */

#            UINT32 pendtask;                /* pending image task ILT list      <w> */
#            UINT32 inacttmr;                /* inactive IMT timer               <w> */
#            struct im_t * link2;            /* allocated link list field        <w> */
#            struct ltmt_t * ltmt;           /* assoc. LTMT address              <w> */

#            struct ilmt_t * ilmtdir[MAXLUN];/* ILMT directory             LUNMAX<w> */

#            struct
#            {
#                UINT16 vid;                 /* virtual drive #                  <s> */
#                UINT16 lun;                 /* assigned LUN                     <s> */
#                UINT16 attr;                /* virtual device attributes        <s> */
#            } cfg[MAXLUN];
#                                            /* --- Statistics                       */

#            struct imst_t inprog;           /* periodic in-progress                 */

#            struct imst_t agg;              /* aggregate stats                 x<w> */

#            struct imst_t per;              /* periodic stats                  x<w> */

#           struct fls_t  lstatus;          /* FCAL link status                x<w> */

#            UINT16 qdepth;                  /* queue depth                      <s> */
#            UINT16 rsvd6;
#        } ;
#
#
#
##############################################################################

sub FmtImt
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $constPtr, $version )= @_;
    
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
    my $msg;
    my $i;
    my $j;
    my $numLuns;
    my $padBytes;


    $msg = "\n";

#$msg = FmtDataString( $bufferPtr, $address, "word", $length, $offset);


    # need to peel of the pad bytes

    $padBytes = 4;

    if($version == 1)
    {    

    #            struct im_t * link;             /* IMT link list field              <w> */
    #            UINT16  fcaddr;                  /* Assoc. FC address (Loop ID)      <s> */
    #            UINT16  vpid;                    /* Assoc. Virtual port ID           <s> */
    #            struct ci_t * cimt;             /* Assoc. CIMT address              <w> */
    #            void (*ehand)();                /* default event handler table      <w> */

    $fmt = sprintf("x%d L SS LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "         IMT link list field: 0x%08x       ", $item1);
    $msg .= sprintf( " Assoc. FC address (Loop ID): 0x%04x  \n", $item2);
    $msg .= sprintf( "      Assoc. Virtual port ID: 0x%04x             ", $item3);
    $msg .= sprintf( "         Assoc. CIMT address: 0x%08x       ", $item5);
    $msg .= sprintf( " default event handler table: 0x%08x", $item6);
    $msg .= sprintf( "\n");
    }
    if ($version == 0)
    {

    #            struct im_t * link;             /* IMT link list field              <w> */
    #            UINT8  fcaddr;                  /* Assoc. FC address (Loop ID)      <b> */
    #            UINT8  vpid;                    /* Assoc. Virtual port ID           <b> */
    #            UINT16 rsvd1;
    #            struct ci_t * cimt;             /* Assoc. CIMT address              <w> */
    #            void (*ehand)();                /* default event handler table      <w> */

    $fmt = sprintf("x%d L CCS LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "         IMT link list field: 0x%08x       ", $item1);
    $msg .= sprintf( " Assoc. FC address (Loop ID): 0x%02x  \n", $item2);
    $msg .= sprintf( "      Assoc. Virtual port ID: 0x%02x             ", $item3);
    $msg .= sprintf( "                    reserved: 0x%08x  \n", $item4);
    $msg .= sprintf( "         Assoc. CIMT address: 0x%08x       ", $item5);
    $msg .= sprintf( " default event handler table: 0x%08x", $item6);
    $msg .= sprintf( "\n");
    }

    $offset += 16;
    $length -= 16;
    $address += 16;

    #            UINT32 rsvd2[2];
    #            UINT64 mac;                     /* FC MAC address (WWN)            8<b> */

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =    unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                    reserved: 0x%08x       ", $item1);
    $msg .= sprintf( "                    reserved: 0x%08x \n", $item2);

    $item3 = FmtWwn($bufferPtr, ($offset + 8));
    $msg .= sprintf( "        FC MAC address (WWN): %16s \n", $item3);

    $offset += 16;
    $length -= 16;
    $address += 16;

    #            UINT8  rsvd3;
    #            UINT8  flags;                   /* Flags byte                       <b> */
    #            UINT8  rsvd4;
    #            UINT8  pri;                     /* priority                         <b> */
    #            UINT16 cfgsiz;                  /* configuration record size        <s> */
    #            UINT16 sid;                     /* Server ID - index into SDX       <s> */
    #            UINT16 tid;                     /* Target ID                        <s> */
    #            UINT16 rsvd5;
    #            UINT32 pendvrp;                 /* pending image assoc. VRP         <w> */

    $fmt = sprintf("x%d CCCC SS SS L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                    reserved: 0x%02x             ", $item1);
    $msg .= sprintf( "                       flags: %08b \n", $item2);
    $msg .= sprintf( "                    reserved: 0x%02x             ", $item3);
    $msg .= sprintf( "                    priority: 0x%02x \n", $item4);
    $msg .= sprintf( "   configuration record size: 0x%04x           ", $item5);
    $msg .= sprintf( "  Server ID - index into SDX: 0x%04x \n", $item6);
    $msg .= sprintf( "                   Target ID: 0x%04x           ", $item7);
    $msg .= sprintf( "                    reserved: 0x%04x \n", $item8);
    $msg .= sprintf( "    pending image assoc. VRP: 0x%08x \n", $item9);

    $offset += 16;
    $length -= 16;
    $address += 16;

    #            UINT32 pendtask;                /* pending image task ILT list      <w> */
    #            UINT32 inacttmr;                /* inactive IMT timer               <w> */
    #            struct im_t * link2;            /* allocated link list field        <w> */
    #            struct ltmt_t * ltmt;           /* assoc. LTMT address              <w> */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    #                "----------------------------:      x       "
    $msg .= sprintf( " pending image task ILT list: 0x%08x       ", $item1);
    $msg .= sprintf( "          inactive IMT timer: 0x%08x \n", $item2);
    $msg .= sprintf( "   allocated link list field: 0x%08x       ", $item3);
    $msg .= sprintf( "         assoc. LTMT address: 0x%08x \n", $item4);


    $offset += 16;
    $length -= 16;
    $address += 16;

    my $weGonnaDoHex = 0;      # flag for later

    if ( $constPtr->{MAXLUN} > 0 )
    {
        # we have information on the number of luns, we don't have to guess
        
        $numLuns = $constPtr->{MAXLUN};   # got # from hash
    }
    else    
    {
        
        # we don't know the number of luns, so we get to make a guess 
        # based upon the data we are processing...

        # this breaks if the structure changes.
        
        # now the next chunk is a little difficult. The size of the structure 
        # based on the number of max. luns. {MAXLUN} We don't know that exactly
        # (or want to hard code it) so we will imply it from the number of bytes 
        # remaining to be processed. There are several things following the 
        # MAXLUNs related parts. Specifically...
        #
        # imst_t inprog        48 bytes
        # imst_t agg           48 bytes
        # imst_t attr          48 bytes
        # flt_t  lstatus       24 bytes
        # qdepth                2 bytes
        # reserved6             2 bytes

        #                    ----------
        #             TOTAL:  172 bytes
        #
        #
        # the size of the MAXLUn related structures are
        #
        #  * ilmtdir     4 bytes
        #  cfg           6 bytes
        #               --------
        #        TOTAL: 10 bytes per lun
        #
        # So, number of luns is
        #
        #   #luns = ($length - <total following bytes> - <pad bytes> ) / <bytes per lun>
        #
        #         = ($length - 172 - pad ) / 10
        #
        # Then, make sure the result makes sense... dump the raminder in hex
        # if not.
        #



        $numLuns = ($length - 172 - $padBytes) / 10;

        if ( ($length - 172) < 10 )
        {
            # there must be at least one lun
            print(" can't figure out the number for MAXLUNs\n");
            print " length = $length, numLuns ~= $numLuns \n";
            $weGonnaDoHex = 1;
        }
    
        if ( (int($numLuns) * 10) + 172 + $padBytes  != $length )
        {
            # the length didn't work out right
            print(" can't figure out the number for MAXLUNs\n");
            print " length = $length, numLuns ~= $numLuns \n";
            $weGonnaDoHex = 1;
        }
    
    }   # end of figuring out the lun count


    if ( $weGonnaDoHex != 0 )
    {
        # do the rest in hex, first save off what we've done so far
        
        $$destPtr .= $msg;

        # now finish up in hex

        $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);
    }
    else
    {
        # we can figure out the format

        #print " length = $length, numLuns = $numLuns \n";


        # first the ilmt dir

        $msg .= "\nILMT directory: \n";

        $i = int($numLuns);
        $j = 0;

        while ( $i > 0 )
        {
            $fmt = sprintf("x%d L ",$offset);      # generate the format string
            ($item1) =     unpack $fmt , $$bufferPtr;

            $msg .= sprintf( "Entry %3d: 0x%08x    ", $j, $item1);
            $j++;

            if ( $j % 4 == 0 )
            {
                # line feed every 4th one
                $msg .= "\n";
            }

            $offset += 4;
            $length -= 4;
            $address += 4;
            $i--;
        }
        
        $msg .= "\n";

        # next the cfg ones

        $msg .= "\nconfig structures: \n";

        $i = int($numLuns);
        $j = 0;

        while ( $i > 0 )
        {
            $fmt = sprintf("x%d SSS ",$offset);      # generate the format string
            ($item1, $item2, $item3) =     unpack $fmt , $$bufferPtr;

            $msg .= sprintf( "Entry %3d: vid:0x%04x lun:0x%04x attr:0x%04x    ",
                           $j, $item1, $item2, $item3);
            $j++;

            if ( $j % 2 == 0 )
            {
                # line feed every 2ndh one
                $msg .= "\n";
            }

            $offset += 6;
            $length -= 6;
            $address += 6;
            $i--;
        }
        
        $msg .= "\n";

        $$destPtr .= $msg;

    }
    
    # start a new msg string for the rest.

    $msg = "\n";



    #            struct imst_t inprog;           /* periodic in-progress                 */
    #    struct imst_t
    #    {
    #        UINT64 cmds;                        /* total # commands            2<w> */
    #        UINT64 bytes;                       /* total # bytes               2<w> */
    #        UINT64 writes;                      /* total # write commands      2<w> */
    #        UINT64 wbytes;                      /* total # write bytes         2<w> */
    #        UINT64 reads;                       /* total # read commands       2<w> */
    #        UINT64 rbytes;                      /* total # read bytes          2<w> */
    #    } ;

    $msg .= "Periodic In-progress:\n";
    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "     total # commands: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "        total # bytes: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "total # write command: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;


    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  total # write bytes: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "total # read commands: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "   total # read bytes: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;

    $msg .= "\n";

    #            struct imst_t agg;              /* aggregate stats                 x<w> */

    $msg .= "Aggregate Stats:\n";
    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "     total # commands: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "        total # bytes: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "total # write command: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;


    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  total # write bytes: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "total # read commands: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "   total # read bytes: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;

    $msg .= "\n";

    #            struct imst_t per;              /* periodic stats                  x<w> */

    $msg .= "Periodic Stats:\n";
    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "     total # commands: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "        total # bytes: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "total # write command: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;


    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  total # write bytes: 0x%08x%08x    ", $item1, $item2);
    $msg .= sprintf( "total # read commands: 0x%08x%08x    ", $item3, $item4);
    $msg .= sprintf( "   total # read bytes: 0x%08x%08x \n",  $item5, $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;

    $msg .= "\n";

    #           struct fls_t  lstatus;          /* FCAL link status                x<w> */
    #    struct fls_t
    #    {
    #        UINT32  lifcnt;                 /* Link Failure Count                   */
    #        UINT32  lsscnt;                 /* Loss of Sync Count                   */
    #        UINT32  lsgcnt;                 /* Loss of Signal Count                 */
    #        UINT32  pspec;                  /* Primitive Seq error count            */
    #        UINT32  ivtqc;                  /* Inv. Xmission Word Count             */
    #        UINT32  ivcrc;                  /* Invalid CRC count                    */
    #    } ;

    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "       Link Failure Count: 0x%08x    ", $item1);
    $msg .= sprintf( "        Loss of Sync Coun: 0x%08x    ", $item2);
    $msg .= sprintf( "     Loss of Signal Count: 0x%08x \n",  $item3);
    $msg .= sprintf( "Primitive Seq error count: 0x%08x    ", $item4);
    $msg .= sprintf( " Inv. Xmission Word Count: 0x%08x    ", $item5);
    $msg .= sprintf( "        Invalid CRC count: 0x%08x \n",  $item6);

    $offset += 24;
    $length -= 24;
    $address += 24;

    $msg .= "\n";



    #            UINT16 qdepth;                  /* queue depth                      <s> */
    #            UINT16 rsvd6;
    $fmt = sprintf("x%d SS",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "          queue depth: 0x%04x    ", $item1);
    $msg .= sprintf( "             reserved: 0x%04x \n",  $item2);

    $offset += 4;
    $length -= 4;
    $address += 4;

    if($version == 1)
    {
        # UINT8               i_name[256];
        $fmt = sprintf("x%d a256",$offset);      # generate the format string
        ($item1) =   unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "   initiator name: %s    ", $item1);
        $offset += 256;
        $length -= 256;
        $address += 256;
    }

    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
#
#          Name: FmtCIMTs
# Call: 
#   FmtCIMTs ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    struct ci_t
#    {
#        struct im_t * imthead;          /* Assoc. IMT list head pointer     <w> */
#        struct im_t * imttail;          /* Assoc. IMT list tail pointer     <w> */
#        void (*ehand)();                /* FC incoming event handler table  <w> */
#        UINT8 num;                      /* interface #                      <b> */
#        UINT8 state;                    /* target interface state code      <b> */
#        UINT8 istate;                   /* initiator state code             <b> */
#        UINT8 rsvd1;
#                                        /* --- Trace data ---                   */
#        UINT16 tflg;                    /* trace flags                      <s> */
#        UINT16 dftflg;                  /* default trace flags              <s> */
#        UINT32 curtr;                   /* current trace record pointer     <w> */
#        UINT32 begtr;                   /* beginning trace record pointer   <w> */
#        UINT32 endtr;                   /* ending trace record pointer      <w> */
#                                        /* LTMT                                 */
#        struct ltmt_t * ltmthd;         /* LTMT list head pointer           <w> */
#        struct ltmt_t * ltmttl;         /* LTMT list tail pointer           <w> */
#                                        /* Statistics                           */
#        UINT16 numhosts;                /* number of active hosts           <s> */
#        UINT16 rsvd2;
#        UINT32 rsvd3;
#    } ;
#
#
##############################################################################

sub FmtCIMTs
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        struct im_t * imthead;          /* Assoc. IMT list head pointer     <w> */
    #        struct im_t * imttail;          /* Assoc. IMT list tail pointer     <w> */
    #        void (*ehand)();                /* FC incoming event handler table  <w> */
    #        UINT8 num;                      /* interface #                      <b> */
    #        UINT8 state;                    /* target interface state code      <b> */
    #        UINT8 istate;                   /* initiator state code             <b> */
    #        UINT8 rsvd1;

    $fmt = sprintf("x%d LLL CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "   Assoc. IMT list head pointer: 0x%08x       ", $item1);
    $msg .= sprintf( "Assoc. IMT list tail pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "FC incoming event handler table: 0x%08x       ", $item3);
    $msg .= sprintf( "                 interface #: 0x%02x \n", $item4);
    $msg .= sprintf( "    target interface state code: 0x%02x             ", $item5);
    $msg .= sprintf( "        initiator state code: 0x%02x \n", $item6);
    $msg .= sprintf( "                       reserved: 0x%02x \n", $item7);

    $offset += 16;

    #        UINT16 tflg;                    /* trace flags                      <s> */
    #        UINT16 dftflg;                  /* default trace flags              <s> */
    #        UINT32 curtr;                   /* current trace record pointer     <w> */
    #        UINT32 begtr;                   /* beginning trace record pointer   <w> */
    #        UINT32 endtr;                   /* ending trace record pointer      <w> */

    $fmt = sprintf("x%d SS LLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                    trace flags: 0x%04x           ", $item1);
    $msg .= sprintf( "         default trace flags: 0x%04x \n", $item2);
    $msg .= sprintf( "   current trace record pointer: 0x%08x       ", $item3);
    $msg .= sprintf( "  beginning trace record ptr: 0x%08x \n", $item4);
    $msg .= sprintf( "   Pending trace record pointer: 0x%08x \n", $item5);

    $offset += 16;

    #        struct ltmt_t * ltmthd;         /* LTMT list head pointer           <w> */
    #        struct ltmt_t * ltmttl;         /* LTMT list tail pointer           <w> */
    #                                        /* Statistics                           */
    #        UINT16 numhosts;                /* number of active hosts           <s> */
    #        UINT16 rsvd2;
    #        UINT32 rsvd3;

    $fmt = sprintf("x%d LL SS L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "         LTMT list head pointer: 0x%08x       ", $item1);
    $msg .= sprintf( "      LTMT list tail pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "         number of active hosts: 0x%04x           ", $item3);
    $msg .= sprintf( "                    reserved: 0x%04x 0x%08x \n", $item4, $item5);

    $offset += 16;




    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
 

##############################################################################
#
#          Name: FmtTarget
# Call: 
#   FmtTarget ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    typedef struct tar_t
#    {
#        struct tar_t *fthd;             /* Forward thread                       */
#        UINT16      tid;                /* Target ID                            */
#        UINT16      entry;              /* Target entry number                  */
#        UINT64      ptn;                /* Target port world wide name          */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT64      ndn;                /* Target node world wide name          */
#        UINT8       opt;                /* Target options                       */
#        UINT8       haid;               /* Target hard assigned ID              */
#        UINT8       rsvd26;             /* Reserved                             */
#        UINT8       vpid;               /* Virtual port ID (LID)                */
#        UINT32      portid;             /* Port ID                              */
#                                        /* QUAD BOUNDARY                    *****/
#    };
#
#
##############################################################################

sub FmtTarget
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version )= @_;
    
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
    my $msg;

    if ( !defined ($version) )
    {
        $version = 0;
    }

    $msg = "\n";

    if ( 1 == $version )
    {
        #        struct tar_t *fthd;             /* Forward thread                       */
        #        UINT16      tid;                /* Target ID                            */
        #        UINT16      entry;              /* Target entry number                  */
        #        UINT8       opt;                /* Target options                       */
        #        UINT8       rsvd1;
        #        UINT8       hardID;             /* Target hard assigned ID              */
        #        UINT8       rsvd2;

        $fmt = sprintf("x%dL SS CCCC ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "              Forward thread: 0x%08x       ", $item1);
        $msg .= sprintf( "                   Target ID: 0x%04x \n", $item2);
        $msg .= sprintf( "         Target entry number: 0x%04x \n", $item3);
        $msg .= sprintf( "              Target options: 0x%02x \n", $item4);
        $msg .= sprintf( "     Target hard assigned ID: 0x%02x \n", $item6);

        $offset += 12;

        #        UINT64      ptn;                /* Target port world wide name          */

        $item1 = FmtWwn($bufferPtr, ($offset));
        $msg .= sprintf( " Target port world wide name: %16s \n", $item1);

        $offset += 8;

        #        UINT64      ndn;                /* Target node world wide name          */

        $item1 = FmtWwn($bufferPtr, $offset);
        $msg .= sprintf( " Target node world wide name: %16s \n", $item1);

        $offset += 8;

        #        UINT32      flags;              /* FC4 registration flag                */
        #        UINT32      vpid;               /* Virtual port ID (LID)                */
        #        UINT32      portid;             /* Port ID                              */

        $fmt = sprintf("x%d LL L ",$offset);      # generate the format string
        ($item1, $item2, $item3) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "       FC4 registration flag: 0x%08x \n", $item1);
        $msg .= sprintf( "       Virtual Port ID (LID): 0x%08x \n", $item2);
        $msg .= sprintf( "                     Port ID: 0x%08x \n", $item3);

        $offset += 12;
    }
    else
    {
        #        struct tar_t *fthd;             /* Forward thread                       */
        #        UINT16      tid;                /* Target ID                            */
        #        UINT16      entry;              /* Target entry number                  */
        #        UINT64      ptn;                /* Target port world wide name          */

        $fmt = sprintf("x%dL SS ",$offset);      # generate the format string
        ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "              Forward thread: 0x%08x       ", $item1);
        $msg .= sprintf( "                   Target ID: 0x%04x \n", $item2);
        $msg .= sprintf( "         Target entry number: 0x%04x \n", $item3);


        $item4 = FmtWwn($bufferPtr, ($offset + 8));
        $msg .= sprintf( " Target port world wide name: %16s \n", $item4);

        $offset += 16;

        #        UINT64      ndn;                /* Target node world wide name          */
        #        UINT8       opt;                /* Target options                       */
        #        UINT8       haid;               /* Target hard assigned ID              */
        #        UINT8       rsvd26;             /* Reserved                             */
        #        UINT8       vpid;               /* Virtual port ID (LID)                */
        #        UINT32      portid;             /* Port ID                              */


        $item3 = FmtWwn($bufferPtr, $offset);
        $msg .= sprintf( " Target node world wide name: %16s \n", $item3);

        $offset += 8;

        $fmt = sprintf("x%d CCCC L ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "              Target options: 0x%02x             ", $item1);
        $msg .= sprintf( "     Target hard assigned ID: 0x%02x \n", $item2);
        $msg .= sprintf( "                    Reserved: 0x%02x             ", $item3);
        $msg .= sprintf( "       Virtual port ID (LID): 0x%02x \n", $item4);
        $msg .= sprintf( "                     Port ID: 0x%08x \n", $item5);

        $offset += 8;
    }   

    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
##############################################################################

##############################################################################
#
#          Name: FmtCimtDir
# Call: 
#   FmtCimtDir ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#      4 unit32 addresses
#
##############################################################################

sub FmtGenDir
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\nDirectory:\n";


    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "           Entry 0: 0x%08x \n", $item1);
    $msg .= sprintf( "           Entry 1: 0x%08x \n", $item2);
    $msg .= sprintf( "           Entry 2: 0x%08x \n", $item3);
    $msg .= sprintf( "           Entry 3: 0x%08x \n", $item4);



    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################

##############################################################################
#
#          Name: FmtDtmts
# Call: 
#   FmtDtmts ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    # --- Basic DTMT data structure ---------------------------------------------
#    #
#            .set    dtmt_link,0             # Link list field                 <w>
#            .set    dtmt_type,dtmt_link+4   # Target type code                <b>
#            .set    dtmt_state,dtmt_type+1  # State of target                 <b>
#            .set    dtmt_sulindx,dtmt_state+1 # Storage unit list index       <s>
#            .set    dtmt_lldmt,dtmt_sulindx+2 # Assoc. link-level driver      <w>
#                                            #  ILT/VRP
#            .set    dtmt_lldid,dtmt_lldmt+4 # Link-level driver session ID    <w>
#    #                                                               ******0x10****
#            .set    dtmt_ehand,dtmt_lldid+4 # Event handler table             <w>
#            .set    dtmt_alpa,dtmt_ehand+4  # AL-PA address                   <w>
#            .set    dtmt_nwwn,dtmt_alpa+4   # Node WWN                       8<b>
#    #                                                               ******0x20****
#            .set    dtmt_pwwn,dtmt_nwwn+8   # Port WWN                       8<b>
#            .set    dtmt_tpmthd,dtmt_pwwn+8 # Target path management table    <w>
#                                            #  list head pointer
#            .set    dtmt_tpmttl,dtmt_tpmthd+4 # Target path management table  <w>
#                                            #  list tail pointer
#    #                                                               ******0x30***
#            .set    dtmt_bnr,dtmt_tpmttl+4  # Banner field                    <w>
#            .set    dtmt_alias_dtmt,dtmt_bnr+4 # Alias node DTMT address      <w>
#            .set    dtmt_pri_dtmt,dtmt_alias_dtmt+4 # Primary DTMT address    <w>
#    #
#    # --- Reserved 4                                                         4<b>
#    #
#    #                                                               ******0x40***
#    #
#            .set    dtmt_tdata,dtmt_pri_dtmt+8 # Target data area           48<b>
#    #
#            .set    dtmt_size,dtmt_tdata+48 # size of DTMT
#
# --- End of basic DTMT data structure --------------------------------------
#
##############################################################################

sub FmtDtmts
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #            .set    dtmt_link,0             # Link list field                 <w>
    #            .set    dtmt_type,dtmt_link+4   # Target type code                <b>
    #            .set    dtmt_state,dtmt_type+1  # State of target                 <b>
    #            .set    dtmt_sulindx,dtmt_state+1 # Storage unit list index       <s>
    #            .set    dtmt_lldmt,dtmt_sulindx+2 # Assoc. link-level driver      <w>
    #                                            #  ILT/VRP
    #            .set    dtmt_lldid,dtmt_lldmt+4 # Link-level driver session ID    <w>

    $fmt = sprintf("x%d L CCS LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                  Link list field: 0x%08x", $item1);
    $msg .= sprintf( "            Target type code: 0x%02x \n", $item2);
    $msg .= sprintf( "                  State of target: 0x%02x      ", $item3);
    $msg .= sprintf( "     Storage unit list index: 0x%04x \n", $item4);
    $msg .= sprintf( " Assoc. link-level driver ILT/VRP: 0x%08x", $item4);
    $msg .= sprintf( "  Link-level drvr session ID: 0x%08x \n", $item4);
    
    $offset += 16;

    #            .set    dtmt_ehand,dtmt_lldid+4 # Event handler table             <w>
    #            .set    dtmt_alpa,dtmt_ehand+4  # AL-PA address                   <w>
    #            .set    dtmt_nwwn,dtmt_alpa+4   # Node WWN                       8<b>

    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) = unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "              Event handler table: 0x%08x", $item1);
    $msg .= sprintf( "               AL-PA address: 0x%08x \n", $item2);

    $offset += 8;

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "                         Node WWN: %16s \n", $item1);
    
    $offset += 8;

    #            .set    dtmt_pwwn,dtmt_nwwn+8   # Port WWN                       8<b>

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "                         Port WWN: %16s \n", $item1);
    
    $offset += 8;

    #            .set    dtmt_tpmthd,dtmt_pwwn+8 # Target path management table    <w>
    #                                            #  list head pointer
    #            .set    dtmt_tpmttl,dtmt_tpmthd+4 # Target path management table  <w>
    #                                            #  list tail pointer

    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) = unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "Target path mgmt tbl lst head ptr: 0x%08x", $item1);
    $msg .= sprintf( "           list tail pointer: 0x%08x \n", $item2);

    $offset += 8;

    #            .set    dtmt_bnr,dtmt_tpmttl+4  # Banner field                    <w>
    #            .set    dtmt_alias_dtmt,dtmt_bnr+4 # Alias node DTMT address      <w>
    #            .set    dtmt_pri_dtmt,dtmt_alias_dtmt+4 # Primary DTMT address    <w>
    #    #
    #    # --- Reserved 4                                                         4<b>

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                     Banner field: 0x%08x", $item1);
    $msg .= sprintf( "     Alias node DTMT address: 0x%08x \n", $item2);
    $msg .= sprintf( "              Primary DTMT addres: 0x%08x", $item3);
    $msg .= sprintf( "                    reserved: 0x%08x \n", $item4);
    
    $offset += 16;

    #            .set    dtmt_tdata,dtmt_pri_dtmt+8 # Target data area           48<b>
    #    #
    $msg .= "\nTarget Data Area:\n";

    $$destPtr .= $msg;

    $$destPtr .= FmtDataString( $bufferPtr, ($address + 0x40), "byte", ($length - 0x40), $offset);

    return GOOD;

}

##############################################################################
##############################################################################
#
#          Name: FmtVdd
# Call: 
#   FmtVdd ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    typedef struct vd
#    {
#        UINT16      vid;                /* Virtual device ID                    */
#        UINT8       mirror;             /* Mirror status                        */
#        UINT8       status;             /* VDisk status                         */
#        UINT16      scorvid;            /* Secondary copy orig. VID             */
#        UINT8       scpcomp;            /* Secondary copy percent complete      */
#        UINT8       raidcnt;            /* Number of RAIDs in this VDisk        */
#        UINT64      devcap;             /* Device capacity                      */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT32      error;              /* Error count                          */
#        UINT32      qd;                 /* Queue depth                          */
#        UINT32      rps;                /* Avg req/sec (last second)            */
#        UINT32      avgsc;              /* Avg sector count (last second)       */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT64      rreq;               /* Read request count                   */
#        UINT64      wreq;               /* Write request count                  */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT16      attr;               /* VDisk attribute                      */
#        UINT8       draidcnt;           /* Deferred RAID count                  */
#        UINT8       rsvd51[5];          /* Reserved                             */
#        UINT32      sprc;               /* Sample period request count          */
#        UINT32      spsc;               /* Sample period sector  count          */
#                                        /* QUAD BOUNDARY                    *****/
#        void        *schead;            /* Original vd scmt list head pointer   */
#        void        *sctail;            /* Original vd scmt list tail pointer   */
#        void        *cpscmt;            /* Copy vd scmt pointer                 */
#        pVLAR       vlinks;             /* VLinks assoc. records                */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT8       name[16];           /* VDisk name                           */
#                                        /* QUAD BOUNDARY                    *****/
#        pRDD        drdd;               /* Deferred RAID pointers               */
#        pRDD        rdd;                /* RAID pointers                        */
#    };
#
##############################################################################

sub FmtVdd
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version )= @_;
    
    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item3a;
    my $item3b;
    my $item3c;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;
    my $item9;
    my $msg;

    if ( !defined ($version) )
    {
        $version = 0;
    }


    $msg = "\n";

    #        UINT16      vid;                /* Virtual device ID                    */
    #        UINT8       mirror;             /* Mirror status                        */
    #        UINT8       status;             /* VDisk status                         */
    #        UINT16      scorvid;            /* Secondary copy orig. VID             */
    #        UINT8       scpcomp;            /* Secondary copy percent complete      */
    #        UINT8       raidcnt;            /* Number of RAIDs in this VDisk        */
    #        UINT64      devcap;             /* Device capacity                      */

    $fmt = sprintf("x%d SCC SCC LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "               Virtual device ID: 0x%04x    ", $item1);
    $msg .= sprintf( "                   Mirror status: 0x%02x      \n", $item2);
    $msg .= sprintf( "                    VDisk status: 0x%02x      ", $item3);
    $msg .= sprintf( "        Secondary copy orig. VID: 0x%04x \n", $item4);
    $msg .= sprintf( " Secondary copy percent complete: 0x%02x      ", $item5);
    $msg .= sprintf( "   Number of RAIDs in this VDisk: 0x%02x \n", $item6);
    $msg .= sprintf( "                 Device capacity: 0x%08x%08x \n", $item8, $item7);

    $offset += 16;

    #        UINT32      error;              /* Error count                          */
    #        UINT32      qd;                 /* Queue depth                          */
    #        UINT32      rps;                /* Avg req/sec (last second)            */
    #        UINT32      avgsc;              /* Avg sector count (last second)       */

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                     Error count: 0x%08x", $item1);
    $msg .= sprintf( "                     Queue depth: 0x%08x \n", $item2);
    $msg .= sprintf( "       Avg req/sec (last second): 0x%08x", $item3);
    $msg .= sprintf( "  Avg sector count (last second): 0x%08x   \n", $item4);

    $offset += 16;

    #        UINT64      rreq;               /* Read request count                   */
    #        UINT64      wreq;               /* Write request count                  */
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                     Error count: 0x%08x%08x", $item1, $item2);
    $msg .= sprintf( "             Queue depth: 0x%08x%08x \n", $item3, $item4);

    $offset += 16;

    if ( 1 == $version )
    {
        #        UINT16      attr;               /* VDisk attribute                      */
        #        UINT8       draidcnt;           /* Deferred RAID count                  */
        #        UINT8       owner;              /* ID of the owning DCN in the DSC      */
        #        UINT8       priority;          /* Reserved                             */
        #        GR_GeoRaidVdiskInfo grInfo;
        #        struct: 
        #                UINT8 vdOpState:3;
        #                UINT8 permFlags:5;
        #                UINT8 tempFlags;
        #                UINT8 aswapProcessIdx;
        #
        #        UINT32      sprc;               /* Sample period request count          */
        #        UINT32      spsc;               /* Sample period sector  count          */

        $fmt = sprintf("x%d SCC CCCC LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item3a, $item3b, $item3c, $item4, $item5, $item6) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "                             VDisk attribute: 0x%04x    ", $item1);
        $msg .= sprintf( "                         Deferred RAID count: 0x%02x \n", $item2);
        $msg .= sprintf( "             ID of the owning DCN in the DSC: 0x%02x \n", $item3);
        $msg .= sprintf( "                                    Priority: 0x%08x \n", $item3a);
        $msg .= sprintf( "                                  Perm Flags: 0x%08x \n", $item3b);
        $msg .= sprintf( "                                  Temp Flags: 0x%08x \n", $item3c);
        $msg .= sprintf( "                             AswapProcessIdx: 0x%08x \n", $item4);
        $msg .= sprintf( "                     Sample period req count: 0x%08x \n", $item5);
        $msg .= sprintf( "                  Sample period sector count: 0x%08x \n", $item6);

        $offset += 16;
    }
    else
    {
        #        UINT16      attr;               /* VDisk attribute                      */
        #        UINT8       draidcnt;           /* Deferred RAID count                  */
        #        UINT8       rsvd51[5];          /* Reserved                             */
        #        UINT32      sprc;               /* Sample period request count          */
        #        UINT32      spsc;               /* Sample period sector  count          */

        $fmt = sprintf("x%d SCC L LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "                 VDisk attribute: 0x%04x    ", $item1);
        $msg .= sprintf( "             Deferred RAID count: 0x%02x \n", $item2);
        $msg .= sprintf( "                        Reserved: 0x%02x 0x%08x", $item3, $item4);
        $msg .= sprintf( "    Sample period req count: 0x%08x   \n", $item5);
        $msg .= sprintf( "      Sample period sector count: 0x%08x      \n", $item6);

        $offset += 16;
   }

    #        void        *schead;            /* Original vd scmt list head pointer   */
    #        void        *sctail;            /* Original vd scmt list tail pointer   */
    #        void        *cpscmt;            /* Copy vd scmt pointer                 */
    #        pVLAR       vlinks;             /* VLinks assoc. records                */

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  Original vd scmt list head ptr: 0x%08x", $item1);
    $msg .= sprintf( "  Original vd scmt list tail ptr: 0x%08x \n", $item2);

    if ( 1 == $version )
    {
        $msg .= sprintf( "                 DCD element: 0x%08x", $item3);
    }
    else
    {
        $msg .= sprintf( "        Copy vd scmt pointer: 0x%08x", $item3);
    }

    $msg .= sprintf( "           VLinks assoc. records: 0x%08x   \n", $item4);

    $offset += 16;

    #        UINT8       name[16];           /* VDisk name                           */

    $item1 = FmtString( $bufferPtr, $offset, 16 );

    $msg .= sprintf( "               VDisk name: %16s \n", $item1);
    
    $offset += 16;

    if ( 1 == $version )
    {
        #        pRDD        drdd;               /* Deferred RAID pointers          */
        #        pRDD        rdd;                /* RAID pointers                   */
        #        void*       pOutHead;           /* Output list head pointer        */
        #        void*       pOutTail;           /* Output list tail pointer        */


        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "          Deferred RAID pointers: 0x%08x", $item1);
        $msg .= sprintf( "                   RAID pointers: 0x%08x \n", $item2);
        $msg .= sprintf( "        Output list Head pointer: 0x%08x", $item3);
        $msg .= sprintf( "        Output list tail pointer: 0x%08x \n", $item4);

        $offset += 16;
    }
    else
    {
        #        pRDD        drdd;               /* Deferred RAID pointers               */
        #        pRDD        rdd;                /* RAID pointers                        */

        $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
        ($item1, $item2) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "          Deferred RAID pointers: 0x%08x", $item1);
        $msg .= sprintf( "                   RAID pointers: 0x%08x \n", $item2);
    }

    $$destPtr .= $msg;

    return GOOD;

}
##############################################################################



##############################################################################


##############################################################################
#
#          Name: FmtLldmtS
# Call: 
#   FmtLldmtS ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#        .set    lldmt_vrp,il_w0         # assoc. VRP address        <w>
#                                        # Note: This field is used when
#                                        #       sending SRPs related to
#                                        #       this ILT/VRP
#        .set    lldmt_dtmthd,il_w1      # assoc. DTMT list head     <w>
#                                        #  pointer
#        .set    lldmt_dtmttl,il_w2      # assoc. DTMT list tail     <w>
#                                        #  pointer
#        .set    lldmt_channel,il_w3     # Channel associated with   <b>
#
##############################################################################

sub FmtLldmtS
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";


    $fmt = sprintf("x%d LLLC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "     assoc. VRP address: 0x%08x \n", $item1);
    $msg .= sprintf( "  assoc. DTMT list head: 0x%08x \n", $item2);
    $msg .= sprintf( "  assoc. DTMT list tail: 0x%08x \n", $item3);
    $msg .= sprintf( "Channel associated with: 0x%02x \n", $item4);



    $$destPtr .= $msg;

    return GOOD;

}


 
##############################################################################
#
#          Name: FmtVdmt
# Call: 
#   FmtVdmt ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    struct vdm_t
#    {
#       UINT32 link;                /* VDMT link list field                 */
#       UINT32 ihead;               /* ILMT list head pointer               */
#       UINT32 itail;               /* ILMT list tail pointer               */
#       UINT32 rilmt;               /* VD reserved ILMT address             */
#                                    /*    0 denotes VD not reserved locally */
#       UINT64 devcap;              /* device capacity                      */
#       UINT16 vid;                 /* virtual device ID                    */
#    } ;
#
##############################################################################

sub FmtVdmt
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #       UINT32 link;                /* VDMT link list field                 */
    #       UINT32 ihead;               /* ILMT list head pointer               */
    #       UINT32 itail;               /* ILMT list tail pointer               */
    #       UINT32 rilmt;               /* VD reserved ILMT address             */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "      VDMT link list field: 0x%08x ", $item1);
    $msg .= sprintf( "    ILMT list head pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "    ILMT list tail pointer: 0x%08x ", $item3);
    $msg .= sprintf( "  VD reserved ILMT address: 0x%08x \n", $item4);

    $offset += 16;

    #       UINT64 devcap;              /* device capacity                      */
    #       UINT16 vid;                 /* virtual device ID                    */

    $fmt = sprintf("x%d LLL ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "           device capacity: 0x%08x%08x \n", $item2, $item1);
    $msg .= sprintf( "         virtual device ID: 0x%04x \n", $item3);


    $$destPtr .= $msg;

    return GOOD;

}
 
##############################################################################
#
#          Name: FmtSdd
# Call: 
#   FmtSdd ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $version);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#    typedef struct sd
#    {
#        UINT16      sid;                /* Server ID                            */
#        UINT16      nluns;              /* Number of LUNs                       */
#        UINT16      tid;                /* Target ID for this server            */
#        UINT8       status;             /* Server status                        */
#        UINT8       pri;                /* HAB priority                         */
#        UINT32      attrib;             /* Server attributes                    */
#        UINT32      session;            /* Session identifier                   */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT64      reqcnt;             /* Server request count                 */
#        UINT16      lsid;               /* Linked Server ID                     */
#        UINT8       rsvd26[2];          /* Reserved                             */
#        UINT32      owner;              /* Serial number of owning controller   */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT64      wwn;                /* World wide name of server            */
#        UINT8       rsvd40[8];          /* Reserved                             */
#                                        /* QUAD BOUNDARY                    *****/
#        UINT8       name[16];           /* Server name                          */
#                                        /* QUAD BOUNDARY                    *****/
#        pLVM        lvm;                /* LUN mappings                         */
#        pLVM        ilvm;               /* Invisisble LUN mappings (LUN FF)     */
#                                        /* Do not change the order of the two   */
#                                        /* lvm pointers.  Order is important.   */
#    };
#
#
##############################################################################

sub FmtSdd
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        UINT16      sid;                /* Server ID                            */
    #        UINT16      nluns;              /* Number of LUNs                       */
    #        UINT16      tid;                /* Target ID for this server            */
    #        UINT8       status;             /* Server status                        */
    #        UINT8       pri;                /* HAB priority                         */
    #        UINT32      attrib;             /* Server attributes                    */
    #        UINT32      session;            /* Session identifier                   */

    $fmt = sprintf("x%d SS SCC LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                 Server ID: 0x%04x     ", $item1);
    $msg .= sprintf( "            Number of LUNs: 0x%04x \n", $item2);
    $msg .= sprintf( " Target ID for this server: 0x%04x     ", $item3);
    $msg .= sprintf( "             Server status: 0x%02x \n", $item4);
    $msg .= sprintf( "              HAB priority: 0x%02x       ", $item5);
    $msg .= sprintf( "         Server attributes: 0x%08x \n", $item6);
    $msg .= sprintf( "        Session identifier: 0x%08x \n", $item7);

    $offset += 16;

    #        UINT64      reqcnt;             /* Server request count                 */
    #        UINT16      lsid;               /* Linked Server ID                     */
    #        UINT8       rsvd26[2];          /* Reserved                             */
    #        UINT32      owner;              /* Serial number of owning controller   */

    $fmt = sprintf("x%d LL SS L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "      Server request count: 0x%08x%08x ", $item1, $item2);
    $msg .= sprintf( "  Linked Server ID: 0x%04x \n", $item3);
    $msg .= sprintf( "                  Reserved: 0x%04x     \n", $item4);
    $msg .= sprintf( "  Serial #  of owning ctlr: 0x%04x \n", $item5);

    $offset += 16;





    #        UINT64      wwn;                /* World wide name of server            */

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( " World wide name of server: %16s \n", $item1);

    $offset += 8;

    #        UINT8       rsvd40[8];          /* Reserved                             */
    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                  Reserved: 0x%08x 0x%08x \n", $item1, $item2);
    
    $offset += 8;



    #        UINT8       name[16];           /* Server name                          */


    $item1 = FmtString( $bufferPtr, $offset, 16 );

    $msg .= sprintf( "               Server name: %16s \n", $item1);
    
    $offset += 16;

    if($version == 1)
    {
        $fmt = sprintf("x%d a256 ",$offset);      # generate the format string
        ($item1) =   unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "     iSCSI Server Name: %s\n", $item1);
        $offset += 256;
    }

    #        pLVM        lvm;                /* LUN mappings                         */
    #        pLVM        ilvm;               /* Invisisble LUN mappings (LUN FF)     */



    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "              LUN mappings: 0x%08x", $item1);
    $msg .= sprintf( "  invisible LUN mappings (LUN FF): 0x%08x \n", $item2);

    $offset += 8;



    $$destPtr .= $msg;

    return GOOD;

}


##############################################################################
##############################################################################

 
##############################################################################
#
#          Name: FmtLsmt
# Call: 
#   FmtLsmt ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Data structure --------------------------------------------------------
#
#        .set    lsmt_link,0             # Link list field                 <w>
#        .set    lsmt_lun,lsmt_link+4    # device LUN (VDisk) #            <s>
#
# --- Reserved 2                                                         2<b>
#
#        .set    lsmt_ltmt,lsmt_lun+4    # assoc. LTMT address             <w>
#        .set    lsmt_dlmid,lsmt_ltmt+4  # assoc. DLM session ID           <w>
#                                                               *************
#        .set    lsmt_smsghd,lsmt_dlmid+4 # Send message list head ptr.    <w>
#        .set    lsmt_smsgtl,lsmt_smsghd+4 # Send message list tail ptr.   <w>
#        .set    lsmt_vrphd,lsmt_smsgtl+4 # VRP list head pointer          <w>
#        .set    lsmt_vrptl,lsmt_vrphd+4 # VRP list tail pointer           <w>
#                                                               *************
#        .set    lsmt_ilt,lsmt_vrptl+4   # assoc. ILT address              <w>
#        .set    lsmt_ehand,lsmt_ilt+4   # event handler routine table     <w>
#        .set    lsmt_psid,lsmt_ehand+4  # Initiator driver (provider) ID  <w>
#
#        .set    lsmt_size,lsmt_psid+4   # size of LSMT
#
# --- End of data structure -------------------------------------------------
#
##############################################################################

sub FmtLsmt
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    lsmt_link,0             # Link list field                 <w>
    #        .set    lsmt_lun,lsmt_link+4    # device LUN (VDisk) #            <s>
    #
    # --- Reserved 2                                                         2<b>
    #
    #        .set    lsmt_ltmt,lsmt_lun+4    # assoc. LTMT address             <w>
    #        .set    lsmt_dlmid,lsmt_ltmt+4  # assoc. DLM session ID           <w>

    $fmt = sprintf("x%d LSS LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "           Link list field: 0x%08x ", $item1);
    $msg .= sprintf( "      device LUN (VDisk) #: 0x%04x \n", $item2);
    $msg .= sprintf( "                Reserved 2: 0x%08x ", $item3);
    $msg .= sprintf( "       assoc. LTMT address: 0x%08x \n", $item4);
    $msg .= sprintf( "     assoc. DLM session ID: 0x%08x \n", $item5);

    $offset += 16;

    #        .set    lsmt_smsghd,lsmt_dlmid+4 # Send message list head ptr.    <w>
    #        .set    lsmt_smsgtl,lsmt_smsghd+4 # Send message list tail ptr.   <w>
    #        .set    lsmt_vrphd,lsmt_smsgtl+4 # VRP list head pointer          <w>
    #        .set    lsmt_vrptl,lsmt_vrphd+4 # VRP list tail pointer           <w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "Send message list head ptr: 0x%08x ", $item1);
    $msg .= sprintf( "Send message list tail ptr: 0x%08x \n", $item2);
    $msg .= sprintf( "     VRP list head pointer: 0x%08x ", $item3);
    $msg .= sprintf( "     VRP list tail pointer: 0x%08x \n", $item4);

    $offset += 16;

    #        .set    lsmt_ilt,lsmt_vrptl+4   # assoc. ILT address              <w>
    #        .set    lsmt_ehand,lsmt_ilt+4   # event handler routine table     <w>
    #        .set    lsmt_psid,lsmt_ehand+4  # Initiator driver (provider) ID  <w>

    $fmt = sprintf("x%d LLL ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "        assoc. ILT address: 0x%08x ", $item1);
    $msg .= sprintf( " event handler routine tbl: 0x%08x \n", $item2);
    $msg .= sprintf( "       Initiator driver ID: 0x%08x \n", $item3);


    $$destPtr .= $msg;

    return GOOD;

}
 
##############################################################################

 
##############################################################################
#
#          Name: FmtTMTs
# Call: 
#   FmtTMTs ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin structure ------------------------------------------------
##
##
##       This area is for use by the initator only!
##
#        .set    tm_link,0               # Forward thread                <w>
#        .set    tm_icimt,tm_link+4      # pointer to ICIMT              <w>
#        .set    tm_tmr0,tm_icimt+4      # timer 0                       <s>
#        .set    tm_tmr1,tm_tmr0+2       # timer 1                       <s>
#        .set    tm_dsrc,tm_tmr1+2       # discovery source              <b>
#        .set    tm_state,tm_dsrc+1      # TMT state byte                <b>
#        .set    tm_flag,tm_state+1      # flag byte
#                                        # 1 spare bytes                1<b>
#        .set    tm_DLUM,tm_flag+2       # discovery LUN usage map      2<w>
#        .set    tm_FLUM,tm_DLUM+8       # fabric LUN usage map         2<w>
#        .set    tm_NEWalpa,tm_FLUM+8    # new alpa                      <w>
#
##
##       This area is for use by the Link Manager only!
##
#        .set    tm_ltmt,tm_NEWalpa+4    # assoc. LTMT address           <w>
##
##       This area is commom but should only be updated by the
##       initator.
##
#        .set    tm_lid,tm_ltmt+4        # LID of this target            <b>
#        .set    tm_ptype,tm_lid+1       # port type                     <b>
#        .set    tm_chipID,tm_ptype+1    # chip instance                 <b>
#        .set    tm_pdt,tm_chipID+1      # peripheral device type        <b>
#        .set    tm_alpa,tm_pdt+1        # AL-PA  (port ID)              <w>
#        .set    tm_N_name,tm_alpa+4     # Node name                    8<b>
#        .set    tm_P_name,tm_N_name+8   # Port name (world wide ID)    8<b>
#        .set    tm_venid,tm_P_name+8    # vendor ID                    8<b>
#        .set    tm_proid,tm_venid+8     # product id                  16<b>
#        .set    tm_version,tm_proid+16  # product revision number      4<b>
#        .set    tm_sn,tm_version+4      # device serial number        16<b>
#
#
#        .set    tm_tlmtdir,tm_sn+16     # tlmt dirctory           MAXLUN<w>
#
##
## --- End structure ---------------------------------------------------
##
##############################################################################

sub FmtTMTs
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    tm_link,0               # Forward thread                <w>
    #        .set    tm_icimt,tm_link+4      # pointer to ICIMT              <w>
    #        .set    tm_tmr0,tm_icimt+4      # timer 0                       <s>
    #        .set    tm_tmr1,tm_tmr0+2       # timer 1                       <s>
    #        .set    tm_dsrc,tm_tmr1+2       # discovery source              <b>
    #        .set    tm_state,tm_dsrc+1      # TMT state byte                <b>
    #        .set    tm_flag,tm_state+1      # flag byte
    #                                        # 1 spare bytes                1<b>

    $fmt = sprintf("x%d LL SS CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "            Forward thread: 0x%08x ", $item1);
    $msg .= sprintf( "          pointer to ICIMT: 0x%08x \n", $item2);
    $msg .= sprintf( "                   timer 0: 0x%04x     ", $item3);
    $msg .= sprintf( "                   timer 1: 0x%04x \n", $item4);
    $msg .= sprintf( "          discovery source: 0x%02x       ", $item5);
    $msg .= sprintf( "            TMT state byte: 0x%02x \n", $item6);
    $msg .= sprintf( "                 flag byte: 0x%02x       ", $item7);
    $msg .= sprintf( "             1 spare bytes: 0x%02x \n", $item8);

    $offset += 16;
    $length -= 16;
    $address += 16;

    #        .set    tm_DLUM,tm_flag+2       # discovery LUN usage map      2<w>
    #        .set    tm_FLUM,tm_DLUM+8       # fabric LUN usage map         2<w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "   discovery LUN usage map: 0x%08x 0x%08x \n", $item2, $item1);
    $msg .= sprintf( "      fabric LUN usage map: 0x%08x 0x%08x \n", $item3, $item4);

    $offset += 16;
    $length -= 16;
    $address += 16;


    #        .set    tm_NEWalpa,tm_FLUM+8    #                      <w>
    #
    ##
    ##       This area is for use by the Link Manager only!
    ##
    #        .set    tm_ltmt,tm_NEWalpa+4    # assoc. LTMT address           <w>
    ##
    ##       This area is commom but should only be updated by the
    ##       initator.
    ##
    #        .set    tm_lid,tm_ltmt+4        # LID of this target            <b>
    #        .set    tm_ptype,tm_lid+1       # port type                     <b>
    #        .set    tm_chipID,tm_ptype+1    # chip instance                 <b>
    #        .set    tm_pdt,tm_chipID+1      # peripheral device type        <b>
    #        .set    tm_alpa,tm_pdt+1        # AL-PA  (port ID)              <w>


    $fmt = sprintf("x%d LL CCCC L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                  new alpa: 0x%08x ", $item1);
    $msg .= sprintf( "       assoc. LTMT address: 0x%08x \n", $item2);
    $msg .= sprintf( "        LID of this target: 0x%02x       ", $item3);
    $msg .= sprintf( "                 port type: 0x%02x \n", $item4);
    $msg .= sprintf( "            chip instancee: 0x%02x       ", $item5);
    $msg .= sprintf( "    peripheral device type: 0x%02x \n", $item6);
    $msg .= sprintf( "          AL-PA  (port ID): 0x%08x \n", $item7);

    $offset += 16;
    $length -= 16;
    $address += 16;








    #        .set    tm_N_name,tm_alpa+4     # Node name                    8<b>
    #        .set    tm_P_name,tm_N_name+8   # Port name (world wide ID)    8<b>

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "                  Node name: %16s\n", $item1);

    $offset += 8;

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf( "  Port name (world wide ID): %16s\n", $item1);


    $offset += 8;

    $length -= 16;
    $address += 16;



    #        .set    tm_venid,tm_P_name+8    # vendor ID                    8<b>

    $item1 = FmtString( $bufferPtr, $offset, 8 );
    $msg .= sprintf( "                  vendor ID: %8s \n", $item1);
    $offset += 8;
    $length -= 8;
    $address += 8;


    #        .set    tm_proid,tm_venid+8     # product id                  16<b>

    $item1 = FmtString( $bufferPtr, $offset, 16 );
    $msg .= sprintf( "                 product id: %16s \n", $item1);
    $offset += 16;
    $length -= 16;
    $address += 16;

    #        .set    tm_version,tm_proid+16  # product revision number      4<b>

    $item1 = FmtString( $bufferPtr, $offset, 4 );
    $msg .= sprintf( "    product revision number: %4s \n", $item1);
    $offset += 4;
    $length -= 4;
    $address += 4;

    #        .set    tm_sn,tm_version+4      # device serial number        16<b>

    $item1 = FmtString( $bufferPtr, $offset, 16 );
    $msg .= sprintf( "       device serial number: %16s \n", $item1);
    $offset += 16;
    $length -= 16;
    $address += 16;

    #
    #
    #        .set    tm_tlmtdir,tm_sn+16     # tlmt dirctory           MAXLUN<w>


    $msg .= sprintf( "\ntlmt dirctory:  \n");

    $$destPtr .= $msg;

    $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);

 
    return GOOD;

}
 

##############################################################################


##############################################################################
#
#          Name: FmtICIMTs
# Call: 
#   FmtICIMTs ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin structure -------------------------------------------------
#
#        .set    ici_link,0              # Forward thread                <w>
#
#        .set    ici_chpid,ici_link+4    # CIMT chip instance            <b>
#        .set    ici_mylid,ici_chpid+1   # LID of my interface           <b>
#        .set    ici_state,ici_mylid+1   # CIMT state code               <b>
#        .set    ici_tmrctl,ici_state+1  # timer control flag            <b>
#        .set    ici_mypid,ici_tmrctl+1  # PID of my interface           <w>
#        .set    ici_lpmapptr,ici_mypid+4# loop map pointer              <w>
##                                                                ***********
#        .set    ici_actqhd,ici_lpmapptr+4 # active queue head           <w>
#                                        #
#        .set    ici_actqtl,ici_actqhd+4 # active queue tail             <w>
#        .set    ici_tmtQ,ici_actqtl+4   # TMT queue                     <w>
#        .set    ici_disQ,ici_tmtQ+4     # discovery queue               <w>
##                                                                ***********
#        .set    ici_trbnr,ici_disQ+4    # trace banner                  <w>
#                                        #
#        .set    ici_curtr,ici_trbnr+4   # current trace pointer         <w>
#        .set    ici_begtr,ici_curtr+4   # begining trace pointer        <w>
#        .set    ici_endtr,ici_begtr+4   # ending trace pointer          <w>
##                                                                ***********
#        .set    ici_tflg,ici_endtr+4    # trace flags                   <s>
#        .set    ici_dftflg,ici_tflg+2   # default trace flags           <s>
#                                        #
#        .set    ici_fstart,ici_dftflg+2 # fabric starting alpa          <w>
#        .set    ici_fcur,ici_fstart+4   # fabric current alpa           <w>
#        .set    ici_ftenable,ici_fcur+4 # foreign target enable flag    <b>
#        .set    ici_FCTOI,ici_ftenable+1 # fabric ctl t/o inhibit       <b>
#                                        # 2 spare bytes                2<b>
##                                                                ***********
#        .set    ici_lidtbl,ici_FCTOI+3  # lid table                    4<w>
#
#
#
#        .set    ici_tmdir,ici_lidtbl+16 # target management dir   MAXLID<w>
##
## --- End structure ---------------------------------------------------
##    
##
###############################################################################

sub FmtICIMTs
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    #        .set    ici_link,0              # Forward thread                <w>
    #
    #        .set    ici_chpid,ici_link+4    # CIMT chip instance            <b>
    #        .set    ici_mylid,ici_chpid+1   # LID of my interface           <b>
    #        .set    ici_state,ici_mylid+1   # CIMT state code               <b>
    #        .set    ici_tmrctl,ici_state+1  # timer control flag            <b>
    #        .set    ici_mypid,ici_tmrctl+1  # PID of my interface           <w>
    #        .set    ici_lpmapptr,ici_mypid+4# loop map pointer              <w>

    $fmt = sprintf("x%d L CCCC LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "            Forward thread: 0x%08x ", $item1);
    $msg .= sprintf( "        CIMT chip instance: 0x%02x \n", $item2);
    $msg .= sprintf( "       LID of my interface: 0x%02x       ", $item3);
    $msg .= sprintf( "           CIMT state code: 0x%02x \n", $item4);
    $msg .= sprintf( "        timer control flag: 0x%02x       ", $item5);
    $msg .= sprintf( "       PID of my interface: 0x%08x \n", $item6);
    $msg .= sprintf( "          loop map pointer: 0x%08x \n", $item7);

    $offset += 16;
    $address += 16;
    $length -= 16;

    #        .set    ici_actqhd,ici_lpmapptr+4 # active queue head           <w>
    #                                        #
    #        .set    ici_actqtl,ici_actqhd+4 # active queue tail             <w>
    #        .set    ici_tmtQ,ici_actqtl+4   # TMT queue                     <w>
    #        .set    ici_disQ,ici_tmtQ+4     # discovery queue               <w>



    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "         active queue head: 0x%08x ", $item1);
    $msg .= sprintf( "         active queue tail: 0x%08x \n", $item2);
    $msg .= sprintf( "                 TMT queue: 0x%08x ", $item3);
    $msg .= sprintf( "           discovery queue: 0x%08x \n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;



    #        .set    ici_trbnr,ici_disQ+4    # trace banner                  <w>
    #                                        #
    #        .set    ici_curtr,ici_trbnr+4   # current trace pointer         <w>
    #        .set    ici_begtr,ici_curtr+4   # begining trace pointer        <w>
    #        .set    ici_endtr,ici_begtr+4   # ending trace pointer          <w>



    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "              trace banner: 0x%08x ", $item1);
    $msg .= sprintf( "     current trace pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "    begining trace pointer: 0x%08x ", $item3);
    $msg .= sprintf( "      ending trace pointer: 0x%08x \n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;





    #        .set    ici_tflg,ici_endtr+4    # trace flags                   <s>
    #        .set    ici_dftflg,ici_tflg+2   # default trace flags           <s>
    #                                        #
    #        .set    ici_fstart,ici_dftflg+2 # fabric starting alpa          <w>
    #        .set    ici_fcur,ici_fstart+4   # fabric current alpa           <w>
    #        .set    ici_ftenable,ici_fcur+4 # foreign target enable flag    <b>
    #        .set    ici_FCTOI,ici_ftenable+1 # fabric ctl t/o inhibit       <b>
    #                                        # 2 spare bytes                2<b>


    $fmt = sprintf("x%d SS L L CCS",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "               trace flags: 0x%04x     ", $item1);
    $msg .= sprintf( "       default trace flags: 0x%04x \n", $item2);
    $msg .= sprintf( "      fabric starting alpa: 0x%08x ", $item3);
    $msg .= sprintf( "       fabric current alpa: 0x%08x \n", $item4);
    $msg .= sprintf( "foreign target enable flag: 0x%02x       ", $item5);
    $msg .= sprintf( "    fabric ctl t/o inhibit: 0x%02x \n", $item6);
    $msg .= sprintf( "             2 spare bytes: 0x%04x \n", $item7);

    $offset += 16;
    $address += 16;
    $length -= 16;






    #        .set    ici_lidtbl,ici_FCTOI+3  # lid table                    4<w>
    #
    #
    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                 lid table: 0x%08x 0x%08x 0x%08x 0x%08x \n",
                          $item1, $item2, $item3, $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;


    #
    #        .set    ici_tmdir,ici_lidtbl+16 # target management dir   MAXLID<w>


    $msg .= sprintf( "\ntarget management dir   MAXLID<w>:  \n");

    $$destPtr .= $msg;

    $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);

 
 
    return GOOD;

}

##############################################################################

 
##############################################################################
#
#          Name: FmtSvrDB
# Call: 
#   FmtSvrDB ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Begin Port ID/Server Name table structure -------------------------------
#
#        .set    ieeeaddr,0              # IEEE address location      <8b>
#
# --- End structure
#
#
##############################################################################

sub FmtSvrDB
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    while ( $length > 0 )
    {

        #        .set    ieeeaddr,0              # IEEE address location      <8b>

        $item1 = FmtWwn($bufferPtr, $offset);

        $offset += 8;

        $item2 = FmtWwn($bufferPtr, $offset);

        $offset += 8;
    
        $msg .= sprintf( "%08X: IEEE address location: %016s %016s \n",
                              $address, $item1, $item2);

        $length -= 16;
        $address += 16;

    }



    $$destPtr .= $msg;

    return GOOD;

}
 

##############################################################################

##############################################################################
#
#          Name: FmtLTMTs
# Call: 
#   FmtLTMTs ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# --- Data structure --------------------------------------------------------
#
#        .set    ltmt_link,0             # Link list field                 <w>
#        .set    ltmt_type,ltmt_link+4   # Target type code                <b>
#        .set    ltmt_lst,ltmt_type+1    # Link state code                 <b>
#        .set    ltmt_dlmst,ltmt_lst+1   # Data-link manager state code    <b>
##
## --- Reserved 1                                                          <b>
##
#        .set    ltmt_cimt,ltmt_dlmst+2  # Assoc. CIMT                     <w>
#        .set    ltmt_tmt,ltmt_cimt+4    # Assoc. TMT                      <w>
##                                                               *************
#        .set    ltmt_dlmid,ltmt_tmt+4   # Data-link manager session ID    <w>
#        .set    ltmt_ehand,ltmt_dlmid+4 # Event handler table             <w>
#        .set    ltmt_seshead,ltmt_ehand+4 # Session management table      <w>
#                                        #  link list head pointer
#        .set    ltmt_sestail,ltmt_seshead+4 # Session management table    <w>
#                                        #  link list tail pointer
##                                                               *************
#        .set    ltmt_msghead,ltmt_sestail+4 # Message list head pointer   <w>
#        .set    ltmt_msgtail,ltmt_msghead+4 # Message list tail pointer   <w>
#        .set    ltmt_imt,ltmt_msgtail+4 # Assoc. IMT                      <w>
#        .set    ltmt_ilt,ltmt_imt+4     # Assoc. ILT                      <w>
##                                                               *************
## --- peer magnitude information
##
#        .set    ltmt_sn,ltmt_ilt+4      # MAGNITUDE serial number         <w>
#        .set    ltmt_path,ltmt_sn+4     # Path/Channel/Interface #        <b>
#        .set    ltmt_cl,ltmt_path+1     # assigned cluster #              <b>
#        .set    ltmt_vdcnt,ltmt_cl+1    # # VDisks                        <b>
#        .set    ltmt_flag1,ltmt_vdcnt+1 # special INQUIRY flag byte #1    <b>
#                                        # Bit 7 = 
#                                        #     6 = 
#                                        #     5 = 
#                                        #     4 = 
#                                        #     3 = 
#                                        #     2 = 
#                                        #     1 = 
#                                        #     0 = 1=node operating in
#                                        #         target-only mode
#        .set    ltmt_pname,ltmt_flag1+1 # peer MAG assigned node name    8<b>
##                                                               *************
#        .set    ltmt_ip,ltmt_pname+8    # assigned IP address of peer    4<b>
#                                        #  MAGNITUDE node
#        .set    ltmt_alias,ltmt_ip+4    # alias node serial number        <w>
##
##       Reserved                                                         4<b>
##
## --- datagram control area
##
#        .set    ltmt_xchgID,ltmt_alias+8 # exchange ID                    <b>
#                                        # 3 spare bytes                  3<b>
##                                                               *************
#        .set    ltmt_dgqdepth,ltmt_xchgID+4 # Number of DG on Queue       <w>
#        .set    ltmt_dgqdmax,ltmt_dgqdepth+4 # Maximum Queue Depth seen   <w>
#        .set    ltmt_dghead,ltmt_dgqdmax+4 # DG hold queue head           <w>
#        .set    ltmt_dgtail,ltmt_dghead+4 # DG hold queue tail            <w>
##                                                               *************
#        .set    ltmt_tmsg,ltmt_dgtail+4           # target msg tbl       8<w>
#        .set    ltmt_imsg,ltmt_tmsg+(ltmt_dgmax*4)# initiator msg tbl   16<w>
#
#        .set    ltmt_size,ltmt_imsg+(ltmt_dgmax*(4*2)) # size of LTMT
#
# --- End of data structure -------------------------------------------------
##
#
##############################################################################

sub FmtLTMTs
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";


    #        .set    ltmt_link,0             # Link list field                 <w>
    #        .set    ltmt_type,ltmt_link+4   # Target type code                <b>
    #        .set    ltmt_lst,ltmt_type+1    # Link state code                 <b>
    #        .set    ltmt_dlmst,ltmt_lst+1   # Data-link manager state code    <b>
    ##
    ## --- Reserved 1                                                          <b>
    ##
    #        .set    ltmt_cimt,ltmt_dlmst+2  # Assoc. CIMT                     <w>
    #        .set    ltmt_tmt,ltmt_cimt+4    # Assoc. TMT                      <w>

    $fmt = sprintf("x%d L CCCC LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "            Link list field: 0x%08x ", $item1);
    $msg .= sprintf( "           Target type code: 0x%02x \n", $item2);
    $msg .= sprintf( "            Link state code: 0x%02x       ", $item3);
    $msg .= sprintf( "  Data-link mngr state code: 0x%02x \n", $item4);
    $msg .= sprintf( "                   Reserved: 0x%02x       ", $item5);
    $msg .= sprintf( "                Assoc. CIMT: 0x%08x \n", $item6);
    $msg .= sprintf( "                 Assoc. TMT: 0x%08x \n", $item7);

    $offset += 16;
    $address += 16;
    $length -= 16;



    #        .set    ltmt_dlmid,ltmt_tmt+4   # Data-link manager session ID    <w>
    #        .set    ltmt_ehand,ltmt_dlmid+4 # Event handler table             <w>
    #        .set    ltmt_seshead,ltmt_ehand+4 # Session management table      <w>
    #                                        #  link list head pointer
    #        .set    ltmt_sestail,ltmt_seshead+4 # Session management table    <w>
    #                                        #  link list tail pointer

    $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  Data-link mngr session ID: 0x%08x ", $item1);
    $msg .= sprintf( "        Event handler table: 0x%08x \n", $item2);
    $msg .= sprintf( "  Session mngmnt tbl hd ptr: 0x%08x ", $item3);
    $msg .= sprintf( "  Session mngmnt tbl tl ptr: 0x%08x \n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;


    #        .set    ltmt_msghead,ltmt_sestail+4 # Message list head pointer   <w>
    #        .set    ltmt_msgtail,ltmt_msghead+4 # Message list tail pointer   <w>
    #        .set    ltmt_imt,ltmt_msgtail+4 # Assoc. IMT                      <w>
    #        .set    ltmt_ilt,ltmt_imt+4     # Assoc. ILT                      <w>

    $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "  Message list head pointer: 0x%08x ", $item1);
    $msg .= sprintf( "  Message list tail pointer: 0x%08x \n", $item2);
    $msg .= sprintf( "                 Assoc. IMT: 0x%08x ", $item3);
    $msg .= sprintf( "                 Assoc. ILT: 0x%08x \n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;


    #        .set    ltmt_sn,ltmt_ilt+4      # MAGNITUDE serial number         <w>
    #        .set    ltmt_path,ltmt_sn+4     # Path/Channel/Interface #        <b>
    #        .set    ltmt_cl,ltmt_path+1     # assigned cluster #              <b>
    #        .set    ltmt_vdcnt,ltmt_cl+1    # # VDisks                        <b>
    #        .set    ltmt_flag1,ltmt_vdcnt+1 # special INQUIRY flag byte #1    <b>
    #                                        # Bit 7 = 
    #                                        #     6 = 
    #                                        #     5 = 
    #                                        #     4 = 
    #                                        #     3 = 
    #                                        #     2 = 
    #                                        #     1 = 
    #                                        #     0 = 1=node operating in
    #                                        #         target-only mode
    #        .set    ltmt_pname,ltmt_flag1+1 # peer MAG assigned node name    8<b>


    $fmt = sprintf("x%d L CCCC",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "    MAGNITUDE serial number: 0x%08x ", $item1);
    $msg .= sprintf( "   Path/Channel/Interface #: 0x%08x \n", $item2);
    $msg .= sprintf( "         assigned cluster #: 0x%08x ", $item3);
    $msg .= sprintf( "                   AVDisks : 0x%08x \n", $item4);
    $msg .= sprintf( "  spcl INQUIRY flag byte #1: 0x%08x \n", $item5);

    $offset += 8;

    $item1 = FmtWwn($bufferPtr, $offset);

    $item2 = FmtString( $bufferPtr, $offset, 8 );

    $offset += 8;

    $msg .= sprintf( "peer MAG assigned node name: %016s ( %8s )\n", $item1, $item2);

    $address += 16;
    $length -= 16;


    #        .set    ltmt_ip,ltmt_pname+8    # assigned IP address of peer    4<b>
    #                                        #  MAGNITUDE node
    #        .set    ltmt_alias,ltmt_ip+4    # alias node serial number        <w>
    ##
    ##       Reserved                                                         4<b>
    ##
    ## --- datagram control area
    ##
    #        .set    ltmt_xchgID,ltmt_alias+8 # exchange ID                    <b>
    #                                        # 3 spare bytes                  3<b>

    $item1 = GetIPAddr( $bufferPtr, $offset );    
    
    $offset += 4;

    $fmt = sprintf("x%d L L CCS",$offset);      # generate the format string
    ($item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( " assigned IP address of MAG: %s \n", $item1);
    $msg .= sprintf( "   alias node serial number: 0x%08x ", $item2);
    $msg .= sprintf( "                   reserved: 0x%08x \n", $item3);
    $msg .= sprintf( "                exchange ID: 0x%02x       ", $item4);
    $msg .= sprintf( "                spare bytes: 0x%02x 0x%04x \n", $item5, $item6);

    $offset += 12;
    $address += 16;
    $length -= 16;


    #        .set    ltmt_dgqdepth,ltmt_xchgID+4 # Number of DG on Queue       <w>
    #        .set    ltmt_dgqdmax,ltmt_dgqdepth+4 # Maximum Queue Depth seen   <w>
    #        .set    ltmt_dghead,ltmt_dgqdmax+4 # DG hold queue head           <w>
    #        .set    ltmt_dgtail,ltmt_dghead+4 # DG hold queue tail            <w>

    $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "      Number of DG on Queue: 0x%08x ", $item1);
    $msg .= sprintf( "   Maximum Queue Depth seen: 0x%08x \n", $item2);
    $msg .= sprintf( "         DG hold queue head: 0x%08x ", $item3);
    $msg .= sprintf( "         DG hold queue tail: 0x%08x \n\n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;


    #        .set    ltmt_tmsg,ltmt_dgtail+4           # target msg tbl       8<w>

    $fmt = sprintf("x%d LL LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "          target msg tbl #1: 0x%08x ", $item1);
    $msg .= sprintf( "          target msg tbl #2: 0x%08x \n", $item2);
    $msg .= sprintf( "          target msg tbl #3: 0x%08x ", $item3);
    $msg .= sprintf( "          target msg tbl #4: 0x%08x \n", $item4);
    $msg .= sprintf( "          target msg tbl #5: 0x%08x ", $item5);
    $msg .= sprintf( "          target msg tbl #6: 0x%08x \n", $item6);
    $msg .= sprintf( "          target msg tbl #7: 0x%08x ", $item7);
    $msg .= sprintf( "          target msg tbl #8: 0x%08x \n\n", $item8);

    $offset += 32;
    $address += 32;
    $length -= 32;



    #        .set    ltmt_imsg,ltmt_tmsg+(ltmt_dgmax*4)# initiator msg tbl   16<w>

    $fmt = sprintf("x%d LL LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "       initiator msg tbl #1: 0x%08x ", $item1);
    $msg .= sprintf( "       initiator msg tbl #2: 0x%08x \n", $item2);
    $msg .= sprintf( "       initiator msg tbl #3: 0x%08x ", $item3);
    $msg .= sprintf( "       initiator msg tbl #4: 0x%08x \n", $item4);
    $msg .= sprintf( "       initiator msg tbl #5: 0x%08x ", $item5);
    $msg .= sprintf( "       initiator msg tbl #6: 0x%08x \n", $item6);
    $msg .= sprintf( "       initiator msg tbl #7: 0x%08x ", $item7);
    $msg .= sprintf( "       initiator msg tbl #8: 0x%08x \n", $item8);

    $offset += 32;
    $address += 32;
    $length -= 32;

    $fmt = sprintf("x%d LL LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "       initiator msg tbl #9: 0x%08x ", $item1);
    $msg .= sprintf( "      initiator msg tbl #10: 0x%08x \n", $item2);
    $msg .= sprintf( "      initiator msg tbl #11: 0x%08x ", $item3);
    $msg .= sprintf( "      initiator msg tbl #12: 0x%08x \n", $item4);
    $msg .= sprintf( "      initiator msg tbl #13: 0x%08x ", $item5);
    $msg .= sprintf( "      initiator msg tbl #14: 0x%08x \n", $item6);
    $msg .= sprintf( "      initiator msg tbl #15: 0x%08x ", $item7);
    $msg .= sprintf( "      initiator msg tbl #16: 0x%08x \n", $item8);

    $offset += 32;
    $address += 32;
    $length -= 32;



    #        .set    ltmt_size,ltmt_imsg+(ltmt_dgmax*(4*2)) # size of LTMT



 #   $msg .= sprintf( "\ntarget management dir   MAXLID<w>:  \n");

    $$destPtr .= $msg;

 #   $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);

 

    return GOOD;

}
##############################################################################

##############################################################################
       
##############################################################################
#
#          Name: FmtVCD
# Call: 
#   FmtVCD ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#      struct vc_t
#      {
#          UINT16  vid;                    /* Virtual ID                           */
#          UINT8   error_flush_cnt;        /* Error State Flush Cnt                */
#          UINT8   stat;                   /* Status                               */
#          UINT32  rsvd;
#          UINT32  cache;                  /* Data in Cache Tree Ptr               */
#          UINT32  dirty;                  /* Dirty data Tree Pointer              */
#          UINT32  io;                     /* Interval I/O Tree pointer            */
#          UINT64  write_count;            /* Number of outstanding host cached write commands */
#          UINT64  flushLBA;               /* Flush LBA                            */
#          UINT64  rdhits;                 /* Cache read hits                      */
#          UINT64  rdpart;                 /* Cache partial read hits              */
#          UINT64  rdmiss;                 /* Cache read misses                    */
#          UINT64  wrhits;                 /* Cache write hits                     */
#          UINT64  wrpart;                 /* Cache partial write hits             */
#          UINT64  wrmiss;                 /* Cache write misses                   */
#          UINT64  wrtbyres;               /* Bypassed writes - resources          */
#          UINT64  wrtbylen;               /* Bypass writes - length               */
#      } ;
#
##############################################################################

sub FmtVCD
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version )= @_;
    
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
    my $msg;

    if ( !defined ($version) )
    {
        $version = 0;
    }

    $msg = "\n";

    #          UINT16  vid;                    /* Virtual ID                           */
    #          UINT8   error_flush_cnt;        /* Error State Flush Cnt                */
    #          UINT8   stat;                   /* Status                               */
    #          UINT32  rsvd;
    #          UINT32  cache;                  /* Data in Cache Tree Ptr               */
    #          UINT32  dirty;                  /* Dirty data Tree Pointer              */
    #          UINT32  io;                     /* Interval I/O Tree pointer            */


    $fmt = sprintf("x%d SCC LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "                Virtual ID: 0x%04x     ", $item1);
    $msg .= sprintf( "     Error State Flush Cnt: 0x%02x \n", $item2);
    $msg .= sprintf( "                    Status: 0x%02x       ", $item3);
    $msg .= sprintf( "                  reserved: 0x%08x \n", $item4);
    $msg .= sprintf( "    Data in Cache Tree Ptr: 0x%08x ", $item5);
    $msg .= sprintf( "   Dirty data Tree Pointer: 0x%08x \n", $item6);
    $msg .= sprintf( " Interval I/O Tree pointer: 0x%08x \n", $item7);

    $offset += 20;

    if ( 1 == $version )
    {
        #          UINT32  write_count;            /* Number of outstanding host cached write commands */
        #          UINT64  flushLBA;               /* Flush LBA                            */
        #          UINT64  rdhits;                 /* Cache read hits                      */
        #          UINT64  rdpart;                 /* Cache partial read hits              */


        $fmt = sprintf("x%d LLL LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "  # of outstanding host cached write cmds: 0x%08x     \n", $item1);
        $msg .= sprintf( "                                Flush LBA: 0x%08x%08x \n", $item3, $item2);
        $msg .= sprintf( "                          Cache read hits: 0x%08x%08x \n", $item5, $item4);
        $msg .= sprintf( "                  Cache partial read hits: 0x%08x%08x \n", $item7, $item6);

        $offset += 28;
    }
    else
    {
        #          UINT64  write_count;            /* Number of outstanding host cached write commands */
        #          UINT64  flushLBA;               /* Flush LBA                            */
        #          UINT64  rdhits;                 /* Cache read hits                      */
        #          UINT64  rdpart;                 /* Cache partial read hits              */


        $fmt = sprintf("x%d LLLL LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf( "  # of outstanding host cached write cmds: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "                                Flush LBA: 0x%08x%08x \n", $item4, $item3);
        $msg .= sprintf( "                          Cache read hits: 0x%08x%08x \n", $item6, $item5);
        $msg .= sprintf( "                  Cache partial read hits: 0x%08x%08x \n", $item8, $item7);

        $offset += 32;
    }

    if ( 1 == $version )
    {
        #          UINT64  rdmiss;                 /* Cache read misses                    */
        #          UINT64  wrhits;                 /* Cache write hits                     */
        #          UINT64  wrpart;                 /* Cache partial write hits             */
        #          UINT64  wrmiss;                 /* Cache write misses                   */

        $fmt = sprintf("x%d LLLL LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "                        Cache read misses: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "                         Cache write hits: 0x%08x%08x \n", $item4, $item3);
        $msg .= sprintf( "                 Cache partial write hits: 0x%08x%08x \n", $item6, $item5);
        $msg .= sprintf( "                       Cache write misses: 0x%08x%08x \n", $item8, $item7);

        $offset += 32;

        #          UINT64  wrtbyres;               /* Bypassed writes - resources          */
        #          UINT64  wrtbylen;               /* Bypass writes - length               */

        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;


        $msg .= sprintf( "              Bypassed writes - resources: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "                   Bypass writes - length: 0x%08x%08x \n", $item4, $item3);

        $offset += 16;

        #          UINT64  capacity;               /* VDisk Capacity                       */
        #          UINT32  rsvd;                   /* Reserved                             */
        #          UINT32  vtv;                    /* VDisk Throttle Value                 */

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "                    VDisk Capacity: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "              VDisk Throttle Value: 0x%08x      \n", $item4);

        $offset += 16;


        #          UINT32  thead;                 /* Throttle Head of Queue                */
        #          UINT32  ttail;                 /* Throttle Tail of Queue                */
        #          UINT32  fwdwait;               /* Wait queue Forward pointer            */
        #          UINT32  bwdwait;               /* Wait queue Backward pointer           */

        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf( "                         Throttle Head of Queue: 0x%08x \n", $item1);
        $msg .= sprintf( "                         Throttle Tail of Queue: 0x%08x \n", $item2);
        $msg .= sprintf( "                     Wait queue Forward pointer: 0x%08x \n", $item3);
        $msg .= sprintf( "                    Wait queue Backward pointer: 0x%08x \n", $item4);

        $offset += 16;
    }
    else
    {
        #          UINT64  rdmiss;                 /* Cache read misses                    */
        #          UINT64  wrhits;                 /* Cache write hits                     */
        #          UINT64  wrpart;                 /* Cache partial write hits             */
        #          UINT64  wrmiss;                 /* Cache write misses                   */

        $fmt = sprintf("x%d LLLL LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "                        Cache read misses: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "                         Cache write hits: 0x%08x%08x \n", $item4, $item3);
        $msg .= sprintf( "                 Cache partial write hits: 0x%08x%08x \n", $item6, $item5);
        $msg .= sprintf( "                       Cache write misses: 0x%08x%08x \n", $item8, $item7);


        #          UINT64  wrtbyres;               /* Bypassed writes - resources          */
        #          UINT64  wrtbylen;               /* Bypass writes - length               */

        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;


        $msg .= sprintf( "              Bypassed writes - resources: 0x%08x%08x \n", $item2, $item1);
        $msg .= sprintf( "                   Bypass writes - length: 0x%08x%08x \n", $item4, $item3);
    }

    $$destPtr .= $msg;

    return GOOD;

}
##############################################################################
       
##############################################################################
#
#          Name: FmtIspRspQ
# Call: 
#   FmtIspRspQ ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#   struct qcb_t
#   {
#        UINT32 * begin;                 /* Org of circular queue ptr            */
#        UINT32 * in;                    /* Insert pointer                       */
#        UINT32 * out;                   /* Remove pointer                       */
#        UINT32 * end;                   /* End of circular que + 1              */
#   } ;
#
#
##############################################################################

sub FmtIspRspQ
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

#        UINT32 * begin;                 /* Org of circular queue ptr            */
#        UINT32 * in;                    /* Insert pointer                       */
#        UINT32 * out;                   /* Remove pointer                       */
#        UINT32 * end;                   /* End of circular que + 1              */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf( "   Org of circular queue ptr: 0x%08x ", $item1);
    $msg .= sprintf( "             Insert pointer : 0x%08x \n", $item2);
    $msg .= sprintf( "              Remove pointer: 0x%08x ", $item3);
    $msg .= sprintf( "     End of circular que + 1: 0x%08x \n", $item4);

    $offset += 16;
    $address += 16;
    $length -= 16;


    $msg .= "Address:                                                  \n";
    $msg .= "----------  ----------  ----------  ----------  ----------  ----------  ----------  ----------  ---------- \n";
    
    while ( $length > 0 )
    {
        # some 16 word structure

        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "0x%08x  0x%08x  0x%08x  0x%08x  0x%08x  ", $address, $item1, $item2, $item3, $item4);
        $offset += 16;


        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "0x%08x  0x%08x  0x%08x  0x%08x \n", $item1, $item2, $item3, $item4);
        $offset += 16;

        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "            0x%08x  0x%08x  0x%08x  0x%08x  ", $item1, $item2, $item3, $item4);
        $offset += 16;
        $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        $msg .= sprintf( "0x%08x  0x%08x  0x%08x  0x%08x \n", $item1, $item2, $item3, $item4);
        $offset += 16;

        $address += 64;
        $length -= 64;

    }




    $$destPtr .= $msg;

    return GOOD;

}
##############################################################################
##############################################################################
#
#          Name: FmtNvramDump
# Call: 
#   FmtNvramDump ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, used to handle
#                        differences between FE and BE processors and
#                        the slightly different packets from the boot
#                        dump.
#           $address - Memory address where the data came from.
#
#  Return: GOOD or ERROR
#    
# The data structure being decoded...
#
#    see nvr.h   and ProcBoot\tpb.h
#
#
##############################################################################
sub FmtNvramDump
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;

    my $fmt;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    
    my $pfp;           
    my $sp;            
    my $rip;           
    my $r3;            
     
    my $r4;            
    my $r5;            
    my $r6;            
    my $r7;            
     
    my $r8;            
    my $r9;            
    my $r10;           
    my $r11;           
     
    my $r12;           
    my $r13;           
    my $r14;           
    my $r15;           
     
    my $g0;            
    my $g1;            
    my $g2;            
    my $g3;            
     
    my $g4;            
    my $g5;            
    my $g6;            
    my $g7;            
     
    my $g8;            
    my $g9;            
    my $g10;           
    my $g11;           
     
    my $g12;           
    my $g13;           
    my $g14;           
    my $fp;            
    my $msg = "\n";
    my $j;
    my $maxRegSets;

    my @days = ( "Holiday",
                 "Sun",
                 "Mon",
                 "Tue",
                 "Wed",
                 "Thu",
                 "Fri",
                 "Sat");

    my $baseAddress = $offset;

 #   if ( $processor eq "boot" )
 #   {
 #       $baseAddress = 0x6600;
 #   }
 #   
 #   if ( $processor eq "bootsnap" )
 #   {
 #       $baseAddress = $offset;
 #   }

    ############################################################################
    $msg .=  "\n--- Backtrace Header ------------------------------------------\n";
    #.equ NVSRAM_NMI_CRC,               (NVSRAM_NMI_BASE + 0x00000000)
    #.equ NVSRAM_NMI_TIMESTAMP,         (NVSRAM_NMI_BASE + 0x00000004)

    ########################
    #.equ NVSRAM_NMI_CRC,               (NVSRAM_NMI_BASE + 0x00000000)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000000 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf( "Backtrace area CRC: 0x%08X \n", $item1 );

    ########################
    if ( $processor ne "boot" )
    {
        #.equ NVSRAM_NMI_TIMESTAMP,         (NVSRAM_NMI_BASE + 0x00000004)
        $fmt = sprintf( "x%d SCC CCCC",     $baseAddress + 0x00000004 );

        ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
            unpack $fmt , $$bufferPtr;


        if ( $item4 > 7 || $item4 < 0 ) { $item4 = 0; }


        $msg .= sprintf("Timestamp  %s  %02X-%02X-%04X  %02X:%02X:%02X\n",
            $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);
    }
    ############################################################################
    $msg .=  "\n--- i960 Registers --------------------------------------------\n";
    #.equ NVSRAM_NMI_ISR,               (NVSRAM_NMI_BASE + 0x00000010)
    #.equ NVSRAM_NMI_LEVELS,            (NVSRAM_NMI_BASE + 0x00000014)
    #.equ NVSRAM_NMI_EVENT,             (NVSRAM_NMI_BASE + 0x00000018)
    #
    #.equ NVSRAM_NMI_RIP,               (NVSRAM_NMI_BASE + 0x00000020)
    #.equ NVSRAM_NMI_SP,                (NVSRAM_NMI_BASE + 0x00000024)
    #.equ NVSRAM_NMI_FP,                (NVSRAM_NMI_BASE + 0x00000028)
    #.equ NVSRAM_NMI_PFP,               (NVSRAM_NMI_BASE + 0x0000002c)
    #
    #.equ NVSRAM_NMI_PATUIMR,           (NVSRAM_NMI_BASE + 0x00000030)
    #.equ NVSRAM_NMI_SATUIMR,           (NVSRAM_NMI_BASE + 0x00000034)
    #.equ NVSRAM_NMI_PATUCMD,           (NVSRAM_NMI_BASE + 0x00000038)
    #.equ NVSRAM_NMI_SATUCMD,           (NVSRAM_NMI_BASE + 0x0000003a)
    #.equ NVSRAM_NMI_ATUCR,             (NVSRAM_NMI_BASE + 0x0000003c)
    #
    #.equ NVSRAM_NMI_MCISR,             (NVSRAM_NMI_BASE + 0x00000040)
    #.equ NVSRAM_NMI_PATUISR,           (NVSRAM_NMI_BASE + 0x00000044)
    #.equ NVSRAM_NMI_SATUISR,           (NVSRAM_NMI_BASE + 0x00000048)
    #.equ NVSRAM_NMI_PBISR,             (NVSRAM_NMI_BASE + 0x0000004c)
    #.equ NVSRAM_NMI_SBISR,             (NVSRAM_NMI_BASE + 0x00000050)
    #.equ NVSRAM_NMI_CSR0,              (NVSRAM_NMI_BASE + 0x00000054)
    #.equ NVSRAM_NMI_CSR1,              (NVSRAM_NMI_BASE + 0x00000058)
    #.equ NVSRAM_NMI_CSR2,              (NVSRAM_NMI_BASE + 0x0000005c)
    #.equ NVSRAM_NMI_IISR,              (NVSRAM_NMI_BASE + 0x00000060)
    #.equ NVSRAM_NMI_ASR,               (NVSRAM_NMI_BASE + 0x00000064)
    #.equ NVSRAM_NMI_BIUISR,            (NVSRAM_NMI_BASE + 0x00000068)
    #
    #.equ NVSRAM_NMI_ECCR,              (NVSRAM_NMI_BASE + 0x00000070)
    #.equ NVSRAM_NMI_ECTST,             (NVSRAM_NMI_BASE + 0x00000074)
    #.equ NVSRAM_NMI_RET_FLAG,          (NVSRAM_NMI_BASE + 0x00000078)
    #.equ NVSRAM_NMI_HLT_FLAG,          (NVSRAM_NMI_BASE + 0x0000007c)
    #
    #.equ NVSRAM_NMI_PATUSR,            (NVSRAM_NMI_BASE + 0x00000080)
    #.equ NVSRAM_NMI_SATUSR,            (NVSRAM_NMI_BASE + 0x00000084)
    #.equ NVSRAM_NMI_PCR,               (NVSRAM_NMI_BASE + 0x00000086)
    #.equ NVSRAM_NMI_BCR,               (NVSRAM_NMI_BASE + 0x00000088)
    #.equ NVSRAM_NMI_PSR,               (NVSRAM_NMI_BASE + 0x0000008a)
    #.equ NVSRAM_NMI_SSR,               (NVSRAM_NMI_BASE + 0x0000008c)
    #.equ NVSRAM_NMI_SDER,              (NVSRAM_NMI_BASE + 0x0000008e)

    ########################
#.equ NVSRAM_NMI_ISR,               (NVSRAM_NMI_BASE + 0x00000010)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000010 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NISR           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_LEVELS,            (NVSRAM_NMI_BASE + 0x00000014)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000014 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("valid_levels   0x%08X\n", $item1);

#.equ NVSRAM_NMI_EVENT,             (NVSRAM_NMI_BASE + 0x00000018)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000018 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("last_event     0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_RIP,               (NVSRAM_NMI_BASE + 0x00000020)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000020 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("last_RIP       0x%08X\n", $item1);

#.equ NVSRAM_NMI_SP,                (NVSRAM_NMI_BASE + 0x00000024)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000024 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("last_SP        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_FP,                (NVSRAM_NMI_BASE + 0x00000028)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000028 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("last_FP        0x%08X\n", $item1);

#.equ NVSRAM_NMI_PFP,               (NVSRAM_NMI_BASE + 0x0000002c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000002c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("last_PFP       0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PATUIMR,           (NVSRAM_NMI_BASE + 0x00000030)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000030 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PATUIMR        0x%08X\n", $item1);

#.equ NVSRAM_NMI_SATUIMR,           (NVSRAM_NMI_BASE + 0x00000034)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000034 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SATUIMR        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PATUCMD,           (NVSRAM_NMI_BASE + 0x00000038)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x00000038 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PATUCMD        0x%04X\n", $item1);

#.equ NVSRAM_NMI_SATUCMD,           (NVSRAM_NMI_BASE + 0x0000003a)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x0000003a );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SATUCMD        0x%04X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ATUCR,             (NVSRAM_NMI_BASE + 0x0000003c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000003c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("ATUCR          0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_MCISR,             (NVSRAM_NMI_BASE + 0x00000040)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000040 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("MCISR          0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PATUISR,           (NVSRAM_NMI_BASE + 0x00000044)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000044 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PATUISR        0x%08X\n", $item1);

#.equ NVSRAM_NMI_SATUISR,           (NVSRAM_NMI_BASE + 0x00000048)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000048 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SATUISR        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PBISR,             (NVSRAM_NMI_BASE + 0x0000004c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000004c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PBISR          0x%08X\n", $item1);

#.equ NVSRAM_NMI_SBISR,             (NVSRAM_NMI_BASE + 0x00000050)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000050 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SBISR          0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_CSR0,              (NVSRAM_NMI_BASE + 0x00000054)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000054 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("CSR0           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_CSR1,              (NVSRAM_NMI_BASE + 0x00000058)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000058 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("CSR1           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_CSR2,              (NVSRAM_NMI_BASE + 0x0000005c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000005c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("CSR2           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_IISR,              (NVSRAM_NMI_BASE + 0x00000060)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000060 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("IISR           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ASR,               (NVSRAM_NMI_BASE + 0x00000064)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000064 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("ASR            0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_BIUISR,            (NVSRAM_NMI_BASE + 0x00000068)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000068 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("BIUISR         0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECCR,              (NVSRAM_NMI_BASE + 0x00000070)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000070 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("ECCR           0x%08X\n", $item1);

#.equ NVSRAM_NMI_ECTST,             (NVSRAM_NMI_BASE + 0x00000074)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000074 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("ECTST          0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_RET_FLAG,          (NVSRAM_NMI_BASE + 0x00000078)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000078 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("RET_FLAG       0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_HLT_FLAG,          (NVSRAM_NMI_BASE + 0x0000007c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000007c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("HLT_FLAG       0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PATUSR,            (NVSRAM_NMI_BASE + 0x00000080)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x00000080 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PATUSR         0x%04X\n", $item1);

#.equ NVSRAM_NMI_SATUSR,            (NVSRAM_NMI_BASE + 0x00000084)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x00000084 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SATUSR         0x%04X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PCR,               (NVSRAM_NMI_BASE + 0x00000086)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x00000086 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PCR            0x%04X\n", $item1);

    ########################
#.equ NVSRAM_NMI_BCR,               (NVSRAM_NMI_BASE + 0x00000088)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x00000088 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("BCR            0x%04X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PSR,               (NVSRAM_NMI_BASE + 0x0000008a)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x0000008a );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("PSR            0x%04X\n", $item1);

#.equ NVSRAM_NMI_SSR,               (NVSRAM_NMI_BASE + 0x0000008c)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x0000008c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SSR            0x%04X\n", $item1);

    ########################
#.equ NVSRAM_NMI_SDER,              (NVSRAM_NMI_BASE + 0x0000008e)
    $fmt = sprintf( "x%d S",            $baseAddress + 0x0000008e );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("SDER           0x%04X\n", $item1);

    ############################################################################
    $msg .=  "\n--- CPLD Registers --------------------------------------------\n";
    #.equ NVSRAM_NMI_REG_0,             (NVSRAM_NMI_BASE + 0x00000090)
    #.equ NVSRAM_NMI_REG_1,             (NVSRAM_NMI_BASE + 0x00000091)
    #.equ NVSRAM_NMI_REG_2,             (NVSRAM_NMI_BASE + 0x00000092)
    #.equ NVSRAM_NMI_REG_3,             (NVSRAM_NMI_BASE + 0x00000093)
    #.equ NVSRAM_NMI_REG_4,             (NVSRAM_NMI_BASE + 0x00000094)
    #.equ NVSRAM_NMI_REG_5,             (NVSRAM_NMI_BASE + 0x00000095)
    #.equ NVSRAM_NMI_REG_6,             (NVSRAM_NMI_BASE + 0x00000096)
    #.equ NVSRAM_NMI_REG_7,             (NVSRAM_NMI_BASE + 0x00000097)
    
    ########################
#.equ NVSRAM_NMI_REG_0,             (NVSRAM_NMI_BASE + 0x00000090)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000090 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg0           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_1,             (NVSRAM_NMI_BASE + 0x00000091)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000091 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg1           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_2,             (NVSRAM_NMI_BASE + 0x00000092)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000092 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg2           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_3,             (NVSRAM_NMI_BASE + 0x00000093)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000093 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg3           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_4,             (NVSRAM_NMI_BASE + 0x00000094)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000094 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg4           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_5,             (NVSRAM_NMI_BASE + 0x00000095)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000095 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg5           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_6,             (NVSRAM_NMI_BASE + 0x00000096)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000096 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg6           0x%02X\n", $item1);

    ########################
#.equ NVSRAM_NMI_REG_7,             (NVSRAM_NMI_BASE + 0x00000097)
    $fmt = sprintf( "x%d C",            $baseAddress + 0x00000097 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("Reg7           0x%02X\n", $item1);


    ############################################################################
    $msg .=  "\n--- Global Registers ------------------------------------------\n";
    #.equ NVSRAM_NMI_GLOBAL,            (NVSRAM_NMI_BASE + 0x000000c0)

    ########################
#.equ NVSRAM_NMI_GLOBAL,            (NVSRAM_NMI_BASE + 0x000000c0)
    # g0-7
    $fmt = sprintf( "x%d LLLL LLLL",       $baseAddress + 0x000000c0 + 0x00 );
    ($g0, $g1, $g2, $g3, $g4, $g5, $g6, $g7) = unpack $fmt, $$bufferPtr;
 

    # g8-fp
    $fmt = sprintf( "x%d LLLL LLLL",       $baseAddress + 0x000000c0 + 0x20 );
    ($g8, $g9, $g10, $g11, $g12, $g13, $g14, $fp) = unpack $fmt, $$bufferPtr;

    $msg .= sprintf("\n  Registers:\n".
                    "  g0: %08X       g8: %08X\n".
                    "  g1: %08X       g9: %08X\n".
                    "  g2: %08X      g10: %08X\n".
                    "  g3: %08X      g11: %08X\n".
                    "  g4: %08X      g12: %08X\n".
                    "  g5: %08X      g13: %08X\n".
                    "  g6: %08X      g14: %08X\n".
                    "  g7: %08X       fp: %08X\n",
                    $g0,  $g8,  $g1,  $g9, $g2, $g10, $g3, $g11,
                    $g4, $g12,  $g5, $g13, $g6, $g14, $g7, $fp);


    ############################################################################
    $msg .=  "\n--- Local Registers -------------------------------------------\n";
    #.equ NVSRAM_NMI_LOCAL,             (NVSRAM_NMI_BASE + 0x00000100) /* 16 Levels */

    if ( $processor eq "boot" )
    {
        $maxRegSets = 0;
    }
    else
    {
        $maxRegSets = 16;
    }
    
    ########################
    for( $j = 0; $j < $maxRegSets; $j++ )
    {
        $msg .= "  --- Register level $j ---\n";

        # pfp-r7
        $fmt = sprintf("x%d LLLL LLLL",       
                       $baseAddress + 0x00000100 + 0x00 + ($j * 0x40));
        ($pfp, $sp, $rip, $r3, $r4, $r5, $r6, $r7) = unpack $fmt, $$bufferPtr;
     
        # r8-r15
        $fmt = sprintf("x%d LLLL LLLL",
                       $baseAddress + 0x00000100 + 0x20 + ($j * 0x40));
        ($r8, $r9, $r10, $r11, $r12, $r13, $r14, $r15) = unpack $fmt, $$bufferPtr;

        $msg .= sprintf("\n  Registers:\n".
                        "  pfp: %08X       r8: %08X\n".
                        "   sp: %08X       r9: %08X\n".
                        "  rip: %08X      r10: %08X\n".
                        "   r3: %08X      r11: %08X\n".
                        "   r4: %08X      r12: %08X\n".
                        "   r5: %08X      r13: %08X\n".
                        "   r6: %08X      r14: %08X\n".
                        "   r7: %08X      r15: %08X\n",
                        $pfp, $r8,   $sp, $r9,   $rip, $r10,   $r3,  $r11,
                        $r4,  $r12,  $r5, $r13,  $r6,  $r14,   $r7,  $r15);

        $msg .= "\n";
    }

    ############################################################################

    if ( $processor ne "boot" )
    {
    
        $msg .=  "\n--- Output for PCBDec.pl tool ---------------------------------\n";

        #.equ NVSRAM_NMI_LEVELS,        (NVSRAM_NMI_BASE + 0x00000014)
        $fmt = sprintf( "x%d L",            $baseAddress + 0x00000014 );
        ($item1) = unpack $fmt, $$bufferPtr;

        $msg .= "Call Stack:\n";
        for( $j = 0; (($j < $item1) && ($j < 16)); $j++ )
        {
            $fmt = sprintf("x%d LLL",          $baseAddress + 0x00000100 + 0x00 + ($j * 0x40) );
            ($item2, $item3, $item4) = unpack $fmt, $$bufferPtr;
            $msg .= sprintf("%2d) PFP:0x%08x  SP:0x%08x  RIP:0x%08x \n", 
                            $j, $item2, $item3, $item4 );
        }

        ############################################################################
        $msg .=  "\n--- Firmware Headers ------------------------------------------\n";
        #.equ NVSRAM_NMI_BOOT_HDR,          (NVSRAM_NMI_BASE + 0x00000500)
        #.equ NVSRAM_NMI_DIAG_HDR,          (NVSRAM_NMI_BASE + 0x00000580)
        #.equ NVSRAM_NMI_FW_HDR,            (NVSRAM_NMI_BASE + 0x00000600)

        ########################
        #.equ NVSRAM_NMI_BOOT_HDR,          (NVSRAM_NMI_BASE + 0x00000500)
        $msg .= "--- Boot Header Info ---\n";

        FmtFwh( \$msg,
            $bufferPtr,
            $baseAddress + 0x00000500,   # Offset
            $length -  $baseAddress - 0x00000500,   # Length
            $processor,
            $address + $baseAddress + 0x00000500 ); # Address

        ########################
        #.equ NVSRAM_NMI_DIAG_HDR,          (NVSRAM_NMI_BASE + 0x00000580)
        $msg .= "--- Diag Header Info ---\n";

        FmtFwh( \$msg,
            $bufferPtr,
            $baseAddress + 0x00000580,   # Offset
            $length -  $baseAddress - 0x00000580,   # Length
            $processor,
            $address + $baseAddress + 0x00000580);  # Address

        ########################
        #.equ NVSRAM_NMI_FW_HDR,            (NVSRAM_NMI_BASE + 0x00000600)
        $msg .= "--- Runtime Header Info ---\n";

        FmtFwh( \$msg,
            $bufferPtr,
            $baseAddress + 0x00000600,   # Offset
            $length  - $baseAddress - 0x00000600,   # Length
            $processor,
            $address + $baseAddress + 0x00000600 ); # Address
    
        ########################
    
    }
    else
    {
        # only 0x200 boot code bytes, adjust baseAddress so the next data can be found
        $baseAddress -= 0x600;

    }

    $msg .=  "\n--- Protected Area --------------------------------------------\n";
    ###
    # protected area for nvsram data, only cleared by diags.
    ###

    # define NVSRAM_NMI_PROTECT_START (NVSRAM_NMI_BASE + 0x00000700)

    # define NVSRAM_NMI_ECC_INIT_TOT (NVSRAM_NMI_BASE + 0x00000740) /*all ecc init errors*/

    # define NVSRAM_NMI_ECC_INIT_MCE_CNT (NVSRAM_NMI_BASE + 0x00000750) /* total mce NMIs during init */
    # define NVSRAM_NMI_ECC_INIT_SGL (NVSRAM_NMI_BASE + 0x00000754) /* total single bit */
    # define NVSRAM_NMI_ECC_INIT_MUL (NVSRAM_NMI_BASE + 0x00000758) /* total multi bit */
    # define NVSRAM_NMI_ECC_INIT_NOT (NVSRAM_NMI_BASE + 0x0000075c) /* total non logged */

    # define NVSRAM_NMI_ECC_INIT_ELOG0 (NVSRAM_NMI_BASE + 0x00000760) /* last value */
    # define NVSRAM_NMI_ECC_INIT_ELOG1 (NVSRAM_NMI_BASE + 0x00000764)
    # define NVSRAM_NMI_ECC_INIT_ECAR0 (NVSRAM_NMI_BASE + 0x00000768)
    # define NVSRAM_NMI_ECC_INIT_ECAR1 (NVSRAM_NMI_BASE + 0x0000076c)

    ########################
    # define NVSRAM_NMI_ECC_INIT_TOT (NVSRAM_NMI_BASE + 0x00000740) /*all ecc init errors*/
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000740 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_TOT       0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_MCE_CNT (NVSRAM_NMI_BASE + 0x00000750) /* total mce NMIs during init */
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000750 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMIECCINIT_MCE_CNT     0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_SGL (NVSRAM_NMI_BASE + 0x00000754) /* total single bit */
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000754 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_SGL       0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_MUL (NVSRAM_NMI_BASE + 0x00000758) /* total multi bit */
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000758 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_MUL       0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_NOT (NVSRAM_NMI_BASE + 0x0000075c) /* total non logged */
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000075c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_NOT       0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_ELOG0 (NVSRAM_NMI_BASE + 0x00000760) /* last value */
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000760 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_ELOG0     0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_ELOG1 (NVSRAM_NMI_BASE + 0x00000764)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000764 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_ELOG1     0x%08X\n", $item1);
                                            
    ########################
    # define NVSRAM_NMI_ECC_INIT_ECAR0 (NVSRAM_NMI_BASE + 0x00000768)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000768 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_ECAR0     0x%08X\n", $item1);

    ########################
    # define NVSRAM_NMI_ECC_INIT_ECAR1 (NVSRAM_NMI_BASE + 0x0000076c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000076c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_INIT_ECAR1     0x%08X\n", $item1);

    ########################
    # undefined words
    $fmt = sprintf( "x%d LLLL",            $baseAddress + 0x00000770 );
    ($item1, $item2, $item3, $item4) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("undefined data         0x%08X 0x%08X 0x%08X 0x%08X \n\n",
                     $item1, $item2, $item3, $item4);



    #.equ NVSRAM_NMI_ELOG0,             (NVSRAM_NMI_BASE + 0x00000780)
    #.equ NVSRAM_NMI_ELOG1,             (NVSRAM_NMI_BASE + 0x00000784)
    #.equ NVSRAM_NMI_ECAR0,             (NVSRAM_NMI_BASE + 0x00000788)
    #.equ NVSRAM_NMI_ECAR1,             (NVSRAM_NMI_BASE + 0x0000078C)
    #
    #.equ NVSRAM_NMI_FW_FAULT_CNT,      (NVSRAM_NMI_BASE + 0x000007a0)
    #.equ NVSRAM_NMI_DG_FAULT_CNT,      (NVSRAM_NMI_BASE + 0x000007a4)
    #
    #.equ NVSRAM_NMI_BRK_CNT,           (NVSRAM_NMI_BASE + 0x000007b0)
    #.equ NVSRAM_NMI_UNEXP_CNT,         (NVSRAM_NMI_BASE + 0x000007b4)
    #
    #.equ NVSRAM_NMI_MCE_CNT,           (NVSRAM_NMI_BASE + 0x000007c0)
    #.equ NVSRAM_NMI_PAE_CNT,           (NVSRAM_NMI_BASE + 0x000007c4)
    #.equ NVSRAM_NMI_SAE_CNT,           (NVSRAM_NMI_BASE + 0x000007c8)
    #.equ NVSRAM_NMI_PBIE_CNT,          (NVSRAM_NMI_BASE + 0x000007cc)
    #
    #.equ NVSRAM_NMI_SBE_CNT,           (NVSRAM_NMI_BASE + 0x000007d0)
    #.equ NVSRAM_NMI_DMAC0E_CNT,        (NVSRAM_NMI_BASE + 0x000007d4)
    #.equ NVSRAM_NMI_DMAC1E_CNT,        (NVSRAM_NMI_BASE + 0x000007d8)
    #.equ NVSRAM_NMI_DMAC2E_CNT,        (NVSRAM_NMI_BASE + 0x000007dc)
    #
    #.equ NVSRAM_NMI_MUI_CNT,           (NVSRAM_NMI_BASE + 0x000007e0)
    #.equ NVSRAM_NMI_ENI_CNT,           (NVSRAM_NMI_BASE + 0x000007e4)
    #.equ NVSRAM_NMI_AAUE_CNT,          (NVSRAM_NMI_BASE + 0x000007e8)
    #.equ NVSRAM_NMI_BIUE_CNT,          (NVSRAM_NMI_BASE + 0x000007ec)
    #
    #.equ NVSRAM_NMI_ECC_SGL_CNT,       (NVSRAM_NMI_BASE + 0x000007f0)
    #.equ NVSRAM_NMI_ECC_MUL_CNT,       (NVSRAM_NMI_BASE + 0x000007f4)
    #.equ NVSRAM_NMI_ECC_NOT_CNT,       (NVSRAM_NMI_BASE + 0x000007f8)
    #.equ NVSRAM_NMI_CNT,               (NVSRAM_NMI_BASE + 0x000007fc)



    ########################
#.equ NVSRAM_NMI_ELOG0,             (NVSRAM_NMI_BASE + 0x00000780)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000780 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ELOG0              0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ELOG1,             (NVSRAM_NMI_BASE + 0x00000784)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000784 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ELOG1              0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECAR0,             (NVSRAM_NMI_BASE + 0x00000788)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x00000788 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECAR0              0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECAR1,             (NVSRAM_NMI_BASE + 0x0000078c)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x0000078c );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECAR1              0x%08X\n", $item1);

    ########################
    # undefined words
    $fmt = sprintf( "x%d LLLL",            $baseAddress + 0x00000790 );
    ($item1, $item2, $item3, $item4) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("undefined data         0x%08X 0x%08X 0x%08X 0x%08X \n\n",
                     $item1, $item2, $item3, $item4);

    ########################
    if ( $processor ne "boot" )
    {
        #.equ NVSRAM_NMI_FW_FAULT_CNT,      (NVSRAM_NMI_BASE + 0x000007a0)
        $fmt = sprintf( "x%d L",            $baseAddress + 0x000007a0 );
        ($item1) = unpack $fmt, $$bufferPtr;
        $msg .= sprintf("NMI_FW_FAULT_CNT       0x%08X\n", $item1);
    }
    ########################
#.equ NVSRAM_NMI_DG_FAULT_CNT,      (NVSRAM_NMI_BASE + 0x000007a4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007a4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_DG_FAULT_CNT       0x%08X\n", $item1);

    ########################
    if ( $processor ne "boot" )
    {
        #.equ NVSRAM_NMI_BRK_CNT,           (NVSRAM_NMI_BASE + 0x000007b0)
        $fmt = sprintf( "x%d L",            $baseAddress + 0x000007b0 );
        ($item1) = unpack $fmt, $$bufferPtr;
        $msg .= sprintf("NMI_BRK_CNT            0x%08X\n", $item1);
    }
    ########################
#.equ NVSRAM_NMI_UNEXP_CNT,         (NVSRAM_NMI_BASE + 0x000007b4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007b4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_UNEXP_CNT          0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_MCE_CNT,           (NVSRAM_NMI_BASE + 0x000007c0)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007c0 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_MCE_CNT            0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PAE_CNT,           (NVSRAM_NMI_BASE + 0x000007c4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007c4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_PAE_CNT            0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_SAE_CNT,           (NVSRAM_NMI_BASE + 0x000007c8)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007c8 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_SAE_CNT            0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_PBIE_CNT,          (NVSRAM_NMI_BASE + 0x000007cc)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007cc );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_PBIE_CNT           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_SBE_CNT,           (NVSRAM_NMI_BASE + 0x000007d0)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007d0 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_SBE_CNT            0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_DMAC0E_CNT,        (NVSRAM_NMI_BASE + 0x000007d4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007d4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_DMAC0E_CNT         0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_DMAC1E_CNT,        (NVSRAM_NMI_BASE + 0x000007d8)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007d8 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_DMAC1E_CNT         0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_DMAC2E_CNT,        (NVSRAM_NMI_BASE + 0x000007dc)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007dc );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_DMAC2E_CNT         0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_MUI_CNT,           (NVSRAM_NMI_BASE + 0x000007e0)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007e0 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_MUI_CNT            0x%08X\n", $item1);

    ########################
    # NMI_SYS_NMI_CNT ??
    #.equ NVSRAM_NMI_ENI_CNT,           (NVSRAM_NMI_BASE + 0x000007e4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007e4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_SYS_NMI_CNT        0x%08X\n", $item1);
    ########################
#.equ NVSRAM_NMI_AAUE_CNT,          (NVSRAM_NMI_BASE + 0x000007e8)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007e8 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_AAUE_CNT           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_BIUE_CNT,          (NVSRAM_NMI_BASE + 0x000007ec)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007ec );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_BIUE_CNT           0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECC_SGL_CNT,       (NVSRAM_NMI_BASE + 0x000007f0)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007f0 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_SGL_CNT        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECC_MUL_CNT,       (NVSRAM_NMI_BASE + 0x000007f4)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007f4 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_MUL_CNT        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_ECC_NOT_CNT,       (NVSRAM_NMI_BASE + 0x000007f8)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007f8 );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_ECC_NOT_CNT        0x%08X\n", $item1);

    ########################
#.equ NVSRAM_NMI_CNT,               (NVSRAM_NMI_BASE + 0x000007fc)
    $fmt = sprintf( "x%d L",            $baseAddress + 0x000007fc );
    ($item1) = unpack $fmt, $$bufferPtr;
    $msg .= sprintf("NMI_CNT                0x%08X\n", $item1);

    ############################################################################
    if ($processor =~ /\bBE\b/ )
    {
        $msg .=  "\n--- BE Specific Parts -----------------------------------------\n";

        FmtNvramDumpBEParts( \$msg,
            $bufferPtr,
            $baseAddress + 0x00000800,   # Offset
            $length  - $baseAddress - 0x00000800,   # Length
            $processor,
            $address + $baseAddress + 0x00000800 ); # Address
    }

    # $msg .= "\n\n--------------------- END OF NVRAM DUMP ----------------------------------------\n";
    $$destPtr .= $msg;

    return GOOD;
}

##############################################################################
#
#          Name: FmtNvramDumpBEParts
# Call: 
#   FmtNvramDumpBEParts ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#  see nvr.h
#
##############################################################################

sub FmtNvramDumpBEParts
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $item10;
    my $item11;
    my $item12;
    my $msg;
    my $i;
    my $j;
    my $initialOffset;
    my $byteCount = 0;

    $msg = "\n";

    # Checksum PART II          <w>
    # reserved
    # Version                   <s>
    # Revision                  <s>
    # Length of structure       <w>

    $initialOffset = $offset;

    $fmt = sprintf("x%d LL SSL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("Checksum PART II               0x%08X\n", $item1);
#    $msg .= sprintf("reserved: 0x%08x \n",         $item2);
    $msg .= sprintf("Version                        0x%08X\n", $item3);
    $msg .= sprintf("Revision                       0x%08X\n", $item4);
    $msg .= sprintf("Length of structure            0x%08X\n", $item5);

    $offset += 16;
    $address += 16;
    $length -= 16;

    #########################

    # Magic number              <s>
    # revision of nvram format  <b>
    # Default label for ndisk <b>
    # Sequence number           <w>
    # Global priority           <b>
    # Next Vlink ID to use           <b>
    # Foreign Target Enable Max           <b>
    #       reserved 1
    # Virtual Controller Group ID      <w>

    $fmt = sprintf("x%d SCC L CCCC L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("Magic number                   0x%04X\n", $item1);
    $msg .= sprintf("Revision of NVRAM format       0x%02X\n", $item2);
    $msg .= sprintf("Default label for ndisk        0x%02X\n", $item3);
    $msg .= sprintf("Sequence number                0x%08X\n", $item4);
    $msg .= sprintf("Global priority                0x%02X\n", $item5);
    $msg .= sprintf("Next Vlink ID to use           0x%02X\n", $item6);
    $msg .= sprintf("Foreign Target Enable Max      0x%02X\n", $item7);
#    $msg .= sprintf("reserved: 0x%02x \n", $item8);
    $msg .= sprintf("Virtual Controller Group ID    0x%08X\n", $item9);

    $offset += 16;
    $address += 16;
    $length -= 16;

    #########################

    # Starting Offset of Local Images      <w>
    #       reserved 8
    # Scrub enable (T/F)       <b>
    # Global cache enable (T/F) <b>
    # Next VID to use           <s>

    $fmt = sprintf("x%d CCCC LL CCS ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8, $item9) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("WHQL status                    0x%02X\n", $item1);
#    $msg .= sprintf("reserved                       0x%08x \n", $item2);
#    $msg .= sprintf("reserved                       0x%08x ", $item3);
#    $msg .= sprintf("reserved                       0x%08x ", $item4);
#    $msg .= sprintf("reserved                       0x%08x ", $item5);
#    $msg .= sprintf("reserved                       0x%08x ", $item6);
    $msg .= sprintf("Scrub enable (T/F)             0x%02X\n", $item7);
    $msg .= sprintf("Global cache enable (T/F)      0x%02X\n", $item8);
    $msg .= sprintf("Next VID to use                0x%04X\n", $item9);

    $offset += 16;
    $address += 16;
    $length -= 16;


    #########################

    # Starting Offset of Local Images      <w>
    #       reserved 8
    # Scrub enable (T/F)       <b>
    # Global cache enable (T/F) <b>
    # Next VID to use           <s>

#    $fmt = sprintf("x%d L LL CCS ",$offset);      # generate the format string
#    ($item1, $item2, $item3, $item4, $item5, $item6) =  
#                        unpack $fmt , $$bufferPtr;
#    
#    $msg .= sprintf( "Start Offset of Local Images: 0x%08x ", $item1);
#    $msg .= sprintf( "                    reserved: 0x%08x \n", $item2);
#    $msg .= sprintf( "                    reserved: 0x%08x ", $item3);
#    $msg .= sprintf( "          Scrub enable (T/F): 0x%02x \n", $item4);
#    $msg .= sprintf( "   Global cache enable (T/F): 0x%02x       ", $item5);
#    $msg .= sprintf( "             Next VID to use: 0x%04x \n", $item6);

    $offset += 16;
    $address += 16;
    $length -= 16;



    #########################

    #       reserved 16

  #  $offset += 16;
  #  $address += 16;
  #  $length -= 16;


    #########################

    my $drive_num = 0;
    my $vdisk_num = 0;
    my $server_num = 0;
    my $enclosure_num = 0;
    my $total_cap = 0;
    my $rec_length;
    my $rec_type;
    my $rec_status;
    
    
    #########################

    # Record length             <s>
    # Record type               <b>
    # Status                    <b>

    $fmt = sprintf("x%d SCC ",$offset);      # generate the format string
    ($rec_length, $rec_type, $rec_status) =  
                        unpack $fmt , $$bufferPtr;


    $offset += 4;
    $address += 4;
    $length -= 4;
            my $offsettmp;
            my $addresstmp;
            my $lengthtmp;

    #########################

    while ($rec_type != 3 && $length > 0)
    {
        # added for JW
        $i = $offset - $initialOffset;
        $msg .= sprintf("\nOffset  0x%08x: ",$i);

        if ($rec_type == 0x0B || $rec_type == 0x0C || $rec_type == 0x0D)
        {
            if ($rec_type == 0x0B)
            {
                $msg .= "--- Begin physical drive record structure ---------------------------\n";
                $drive_num++;
            }
            elsif ($rec_type == 0x0C)
            {
                $msg .= "--- Begin SES enclosure record structure ----------------------------\n";
                $enclosure_num++;
            }
            elsif ($rec_type == 0x0D)
            {
                $msg .= "--- Begin misc device record structure ------------------------------\n";
            }

            ######################

            # Physical device ID        <s>
            # Device class              <b>
            # Channel installed in      <b>
            # FC ID                     <w>
            # System serial             <w>

            $fmt = sprintf("x%d SCC LL ",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("Physical device IDs    0x%04X\n", $item1);
            $msg .= sprintf("Device class           0x%02X\n", $item2);
            $msg .= sprintf("Channel installed in   0x%02X\n", $item3);
            $msg .= sprintf("FC ID                  0x%08X\n", $item4);
            $msg .= sprintf("System serial          0x%08X\n", $item5);
                                                    
            $offset += 12;
            $address += 12;
            $length -= 12;

            ######################

            # Product ID                <q>     

            $item1 = FmtString( $bufferPtr, $offset, 16 );
            $msg .= sprintf("Product ID             $item1\n");

            $offset += 16;
            $address += 16;
            $length -= 16;

            ######################

            if ($rec_type == 0x0B)
            {
                
                my @ID = split / */, $item1;
                
                if ($item1 =~ /^ST/)
                {
                    my $count = 3;
                    my $drive_size = "";
                    while($ID[$count] =~ /\d/)
                    {
                        $drive_size = "$drive_size$ID[$count]";
                        $count++;
                    }
                    $drive_size /= 1000;
                    $total_cap += $drive_size;
                    $msg .= sprintf("Drive Capacity         $drive_size GB\n");
                }                                       
            }

            ######################

            # Vendor ID                 <l>

            $item1 = FmtString( $bufferPtr, $offset, 8 );
            $msg .= sprintf("Vendor ID              $item1\n");

            $offset += 8;
            $address += 8;
            $length -= 8;


            # Serial number             <t>

            $item1 = FmtString( $bufferPtr, $offset, 12 );
            $msg .= sprintf("Serial number          $item1\n");

            $offset += 12;
            $address += 12;
            $length -= 12;

            ######################

            # World wide name           <l>

            $item1 = FmtWwn($bufferPtr, $offset);
            $msg .= sprintf("World wide name        %16s\n", $item1);

            $offset += 8;                         # add bytes processed
            $address += 8;
            $length -= 8;

            ######################


            # LUN                       <l>
            # Misc Status                <c>
            # DName                       <l>
            # Reserved 11

            $fmt = sprintf("x%d L C L ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("LUN                    0x%08X\n", $item1);
            $msg .= sprintf("MiscStat               0x%02X\n", $item2);
            $msg .= sprintf("DName                  0x%08X\n", $item3);

            $offset += 20;
            $address += 20;
            $length -= 20;

            ######################

        }
        elsif ($rec_type == 0x09)
        {

            $msg .= "--- Begin RAID record structure -------------------------------------\n";

            #    typedef struct NVRR
            #    {
            #        UINT16      rid;                /* RAID device ID                       */
            #        UINT8       type;               /* RAID device type                     */
            #        UINT8       depth;              /* RAID device depth                    */
            #        UINT16      vid;                /* VDisk ID to which RAID belongs       */
            #        UINT16      devcount;           /* Number of physical devices in RAID   */
            #        UINT32      sps;                /* Sectors per stripe                   */
            #        UINT64      devcap;             /* Device capacity                      */
            #        UINT32      spu;                /* Sectors per unit                     */
            #        UINT32      slen;               /* Segment length                       */
            #        UINT32      rlen;               /* Rebuild len for rebuild in progress  */
            #        UINT8       astatus;            /* Additional status                    */
            #        UINT8       rsvd[11];           /* Reserved                             */
            #    };
            #########################
            # RAID ID                   <s>
            # Type                      <b>
            # Depth                     <b>
            # Virtual ID of owner       <s>
            # Device count              <s>
            # Sectors/stripe            <w>
             
            my $deviceCount;
            $byteCount = 0;

            $fmt = sprintf("x%d SCC SS L",$offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("RAID ID                0x%04X\n", $item1);
            $msg .= sprintf("Type                   0x%02X\n", $item2);
            $msg .= sprintf("Depth                  0x%02X\n", $item3);
            $msg .= sprintf("Virtual ID of owner    0x%04X\n", $item4);
            $msg .= sprintf("Device count           0x%04X\n", $item5);
            $msg .= sprintf("Sectors/stripe         0x%08X\n", $item6);

            $deviceCount = $item5;

            $byteCount += 12;
            #########################
            # Device capacity           <l>
            # Sectors/unit              <w>
            # Segment length / 256      <w>
            
            $fmt = sprintf("x%d LL LL",$offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("Device capacity        0x%08X%08X\n", $item2, $item1);
            $msg .= sprintf("Sectors/unit           0x%04X    \n", $item3);
            $msg .= sprintf("Segment length / 256   0x%04X    \n", $item4);

            $byteCount += 16;
            #########################
            # rlen          <w>
            # Astatus       <c>
            #read NVRAM, $blanks, 11; #reserved

            $fmt = sprintf("x%d LC",$offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("rlen                   0x%08X\n", $item1);
            $msg .= sprintf("Additional status      0x%02X\n", $item2);

            $byteCount += 16;
            #########################
            #    typedef struct nrrx
            #    {
            #        UINT16      pid;                /* Physical device ID                   */
            #        UINT8       status;             /* PSD status                           */
            #        UINT8       astatus;            /* Additional status                    */
            #        UINT32      sda;                /* Starting disk address of PSD         */
            #    };

            $msg .= "PSD      PID     Status        AStatus         Start LBA of PSD \n";

            for ($i = 0; $i < $deviceCount; $i++)
            {       
                # Physical device ID            <s>
                # Status of device              <b>
                # Additional Status of device   <b>
                # Starting address of PSD       <w>

                $fmt = sprintf("x%d SCC L", $offset + $byteCount);      # generate the format string
                ($item1, $item2, $item3, $item4) =  
                                    unpack $fmt , $$bufferPtr;
    
                $msg .= sprintf("%3d", $i);
                $msg .= sprintf("   0x%04X", $item1);
                $msg .= sprintf("       0x%02X", $item2);
                $msg .= sprintf("           0x%02X", $item3);
                $msg .= sprintf("               0x%08X \n", $item4);

                $byteCount += 8;
            }
            $offset += $rec_length - 4;       # position pointers based upon record size
            $address += $rec_length - 4;      # adjust 4 bytes for header
            $length -= $rec_length - 4;
            #########################
        }
        elsif ($rec_type == 0x29)
        {

            $msg .= "--- Begin RAID GT2TB record structure -------------------------------\n";

            #    typedef struct NVRR_GT2TB
            #    {
            #        UINT16      rid;                /* RAID device ID                       */
            #        UINT8       type;               /* RAID device type                     */
            #        UINT8       depth;              /* RAID device depth                    */
            #        UINT16      vid;                /* VDisk ID to which RAID belongs       */
            #        UINT16      devcount;           /* Number of physical devices in RAID   */
            #        UINT32      sps;                /* Sectors per stripe                   */
            #        UINT64      devcap;             /* Device capacity                      */
            #        UINT32      spu;                /* Sectors per unit                     */
            #        UINT64      slen;               /* Segment length                       */
            #        UINT8       astatus;            /* Additional status                    */
            #        UINT8       rsvd[3];            /* Reserved                             */
            #        UINT32      notMirrorCSN;       /* Not Mirroring Controller Serial Number*/
            #        UINT32      owner;              /* Owning controller at time of save    */
            #    };
            #########################
            # RAID ID                   <s>
            # Type                      <b>
            # Depth                     <b>
            # Virtual ID of owner       <s>
            # Device count              <s>
            # Sectors/stripe            <w>

            my $deviceCount;
            $byteCount = 0;

            $fmt = sprintf("x%d SCC SS L",$offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("RAID ID                0x%04X\n", $item1);
            $msg .= sprintf("Type                   0x%02X\n", $item2);
            $msg .= sprintf("Depth                  0x%02X\n", $item3);
            $msg .= sprintf("Virtual ID of owner    0x%04X\n", $item4);
            $msg .= sprintf("Device count           0x%04X\n", $item5);
            $msg .= sprintf("Sectors/stripe         0x%08X\n", $item6);

            $deviceCount = $item5;

            $byteCount += 12;
            #########################
            # Device capacity           <l>
            # Sectors/unit              <w>
            # Segment length / 256      <l>
            
            $fmt = sprintf("x%d LL L LL",$offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("Device capacity        0x%08X%08X\n", $item2, $item1);
            $msg .= sprintf("Sectors/unit           0x%04X    \n", $item3);
            $msg .= sprintf("Segment length / 256   0x%08X%08X\n", $item5, $item4);

            $byteCount += 20;
            #########################
            #Astatus      <c>
            #read NVRAM, $blanks, 11; #reserved

            $fmt = sprintf("x%d C",$offset + $byteCount);      # generate the format string
            ($item1) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("Additional status      0x%02X\n", $item2);

            $byteCount += 1 + 3+4+4;    # aStatus, rsvd[3], notMirrorCSN, owner
            #########################
            #    typedef struct NVRRX_GT2TB
            #    {
            #        UINT16      pid;                /* Physical device ID                   */
            #        UINT8       status;             /* PSD status                           */
            #        UINT8       astatus;            /* Additional status                    */
            #        UINT64      sda;                /* Starting disk address of PSD         */
            #    };

            $msg .= "PSD      PID     Status        AStatus         Start LBA of PSD \n";

            for ($i = 0; $i < $deviceCount; $i++)
            {       
                # Physical device ID            <s>
                # Status of device              <b>
                # Additional Status of device   <b>
                # Starting address of PSD       <l>

                $fmt = sprintf("x%d SCC LL", $offset + $byteCount);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5) =  
                                    unpack $fmt , $$bufferPtr;
    
                $msg .= sprintf("%3d", $i);
                $msg .= sprintf("   0x%04X", $item1);
                $msg .= sprintf("       0x%02X", $item2);
                $msg .= sprintf("           0x%02X", $item3);
                $msg .= sprintf("               0x%08X%08X \n", $item5, $item4);

                $byteCount += 12;
            }
            $offset += $rec_length - 4;       # position pointers based upon record size
            $address += $rec_length - 4;      # adjust 4 bytes for header
            $length -= $rec_length - 4;
            #########################
        }
        elsif ($rec_type == 0x0A)
        {
            $msg .= "--- Begin Virtual Device record structure ---------------------------\n";
     
            $vdisk_num++;

            #    typedef struct nrv
            #    {
            #        UINT16      vid;                /* VDisk device ID                      */
            #        UINT8       draidcnt;           /* VDisk defered RAID count             */
            #        UINT8       raidcnt;            /* VDisk RAID count                     */
            #        UINT64      devcap;             /* Device capacity                      */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT16      attr;               /* VDisk attribute                      */
            #        UINT8       vlarcnt;            /* VDisk VLAR count                     */
            #        UINT8       rsvd35[13];         /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT8       name[16];           /* Vdisk name                           */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };
            #

            my $raidCount;
            my $draidCount;
            my $vlarCount;
            my $atrbt;

            $byteCount = 0;

            $fmt = sprintf("x%d SCC LL", $offset + $byteCount);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("VDisk device ID        %d\n", $item1);
            $msg .= sprintf("VDisk deferred RAIDs   0x%02X\n", $item2);
            $msg .= sprintf("VDisk RAID count       0x%02X\n", $item3);
            $msg .= sprintf("Device capacity        0x%08X%08X\n", $item5, $item4);

            $raidCount = $item3;
            $draidCount = $item2;

            $byteCount += 12;

            $fmt = sprintf("x%d SC",$offset + $byteCount);      # generate the format string
            ($item1, $item2) =     unpack $fmt , $$bufferPtr;
    
            $msg .= sprintf("VDisk attribute        0x%04X\n", $item1);
            $msg .= sprintf("VDisk VLAR count       0x%02X\n", $item2);

            $atrbt = $item1;
            $vlarCount = $item2;
            
            $byteCount += 16;

            $item1 = FmtString( $bufferPtr, $offset + $byteCount, 16 );
            $msg .= sprintf("Vdisk name             $item1\n");

            $byteCount += 16;


            #    {
            #        UINT16      rid;                /* RAID device ID                       */
            #    };

            for ($i = 0; $i < $raidCount; $i++)
            {
                # RAID device ID            <s>

                $fmt = sprintf("x%d S",$offset + $byteCount);      # generate the format string
                ($item1) =     unpack $fmt , $$bufferPtr;
                $msg .= sprintf("Raid ID                0x%04X\n", $item1);

                $byteCount += 2;

            }

            for ($i = 0; $i < $draidCount; $i++)
            {
                # RAID device ID            <s>

                $fmt = sprintf("x%d S",$offset + $byteCount);      # generate the format string
                ($item1) =     unpack $fmt , $$bufferPtr;
                $msg .= sprintf("Deferred Raid ID       0x%04X\n", $item1);

                $byteCount += 2;
                $address += 2;
                $length -= 2;
            }

            #    typedef struct nrvx3
            #    {
            #        UINT32      srcsn;              /* Source controller serial number      */
            #        UINT8       srccl;              /* Source controller cluster number     */
            #        UINT8       srcvd;              /* Source controller vdisk number       */
            #        UINT8       attr;               /* Attributes                           */
            #        UINT8       poll;               /* VLink poll timer count               */
            #        UINT16      repvd;              /* Reported VDisk number                */
            #        UINT8       name[52];           /* Name                                 */
            #        UINT32      agnt;               /* Agent serial number                  */
            #    };


            for ($i = 0; $i < $vlarCount; $i++)
            {
                # Source Controller Serial Number            <w>
                # Source Controller Cluster Number            <b>
                # Source Controller vdisk Number            <b>
                # Attributes            <b>
                # VLink Poll Timer Count            <b>
                # Reported Vdisk Number            <s>

                $fmt = sprintf("x%d L CCCC S",$offset + $byteCount);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5, $item6) =  
                                    unpack $fmt , $$bufferPtr;
    
                $msg .= sprintf("Src Ctlr Serial #      0x%08X\n", $item1);
                $msg .= sprintf("Src Ctlr Cluster #     0x%02X\n", $item2);
                $msg .= sprintf("Src Ctlr vdisk #       0x%02X\n", $item3);
                $msg .= sprintf("Attributes             0x%02X\n", $item4);
                $msg .= sprintf("VLink Poll Timer Count 0x%02X\n", $item5);
                $msg .= sprintf("Reported Vdisk Number  0x%04X\n", $item6);

                $atrbt = $item4;

                $byteCount += 10;


                # Name            <s>

                $item1 = FmtString( $bufferPtr, $offset + $byteCount, 52 );
                $msg .= sprintf("Name                   $item1\n");

                $byteCount += 52;


                # Agent Serial Number            <w>

                $fmt = sprintf("x%d L",$offset + $byteCount);      # generate the format string
                ($item1) =     unpack $fmt , $$bufferPtr;
    
                $msg .= sprintf("Agent Serial Number    0x%08X\n", $item1);
                $byteCount += 4;
            }

            if ($atrbt == 0x40)
            {
                #read NVRAM, $blanks, 64;
                $byteCount += 64;
            }

            $offset += $rec_length - 4;       # position pointers based upon record size
            $address += $rec_length - 4;      # adjust 4 bytes for header
            $length -= $rec_length - 4;
            
        }
        elsif ($rec_type == 0x08)
        {
            $msg .= "--- Begin Server Device record structure ----------------------------\n";

            $server_num++;

            #    typedef struct nrs
            #    {
            #        UINT16      sid;                /* Server ID                            */
            #        UINT16      nluns;              /* Number of LUNs mapped in server      */
            #        UINT16      tid;                /* Target server is mapped to           */
            #        UINT8       stat;               /* Status                               */
            #        UINT8       pri;                /* Server priority                      */
            #        UINT32      owner;              /* Owning controller                    */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT64      wwn;                /* World wide name                      */
            #        UINT32      attrib;             /* Server attributes                    */
            #        UINT16      lsid;               /* Linked Server ID                     */
            #        UINT8       rsvd30[2];          /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT8       name[16];           /* Name                                 */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };

            my $lunCount;

            $offsettmp = $offset;
            $addresstmp = $address;
            $lengthtmp = $length;

            $fmt = sprintf("x%d SSS CC L",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Server ID                  0x%04X\n", $item1);
            $msg .= sprintf("# of LUNs mapped in server 0x%04X\n", $item2);
            $msg .= sprintf("Target server is mapped to 0x%04X\n", $item3);
            $msg .= sprintf("Status                     0x%02X\n", $item4);
            $msg .= sprintf("Server priority            0x%02X\n", $item5);
            $msg .= sprintf("Owning controller          0x%08X\n", $item6);

            $lunCount = $item2;

            $offset += 12;
            $address += 12;
            $length -= 12;


            $item1 = FmtWwn($bufferPtr, $offset);
            $msg .= sprintf("World wide name            %16s\n", $item1);

            $offset += 8;
            $address += 8;
            $length -= 8;


            $fmt = sprintf("x%d LSS",$offset);      # generate the format string
            ($item1, $item2, $item3) =     unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Server attributes          0x%08X ", $item1);
            $msg .= sprintf("Linked Server ID           0x%04X \n", $item2);
#            $msg .= sprintf("reserved: 0x%04x \n", $item3);

            $offset += 8;
            $address += 8;
            $length -= 8;


            $item1 = FmtString( $bufferPtr, $offset, 16 );
            $msg .= sprintf("Name                       $item1\n");

            $offset += 16;
            $address += 16;
            $length -= 16;


            ##########################

            #    typedef struct nrsx
            #    {
            #        UINT16      vid;                /* Virtual device ID                    */
            #        UINT16      lun;                /* LUN the VID is mapped onto           */
            #    };
              
              
            for ($i = 0; $i < $lunCount; $i++)
            {
                # Virtual device ID         <s>
                # LUN                       <s>

                $fmt = sprintf("x%d SS",$offset);      # generate the format string
                ($item1, $item2) =     unpack $fmt , $$bufferPtr;

                $msg .= sprintf("Virtual device ID      0x%04X\n", $item1);
                $msg .= sprintf("LUN                    0x%04X\n", $item2);

                $offset += 4;
                $address += 4;
                $length -= 4;

            }

            if ($lunCount != 0)
            {
                if ((($lunCount * 4) % 16) != 0)
                {
                    $item1 = (16 - (($lunCount * 4) % 16));  #reserved
                    $offset += $item1;
                    $address += $item1;
                    $length -= $item1;

                }
            }
# Go over Server extension structures
            $offset = $offsettmp + $rec_length -4;
            $address = $addresstmp + $rec_length-4;
            $length = $lengthtmp - $rec_length+4;
        }
        elsif ($rec_type == 0x0E)
        {
            $msg .= "--- Begin Target Device record structure ----------------------------\n";

            #    typedef struct nrt
            #    {
            #        UINT16      tid;                /* Target ID                            */
            #        UINT8       port;               /* Port mapped onto                     */
            #        UINT8       opt;                /* Options                              */
            #        UINT8       fcid;               /* Fibre channel ID                     */
            #        UINT8       rsvd9;              /* Reserved                             */
            #        UINT8       lock;               /* Locked target indicator              */
            #        UINT8       rsvd11;             /* Reserved                             */
            #        UINT32      owner;              /* Owning controller                    */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT64      pname;              /* Port world wide name                 */
            #        UINT64      nname;              /* Node world wide name                 */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT32      powner;             /* Previous owner                       */
            #        UINT16      cluster;            /* Cluster                              */
            #        UINT16      rsvd2;              /* Reserved                             */
            #        UINT8       pport;              /* Prefered port                        */
            #        UINT8       aport;              /* Alternate port                       */
            #        UINT8       rsvd38[6];          /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };

            $offsettmp = $offset;
            $addresstmp = $address;
            $lengthtmp = $length;
            
            $fmt = sprintf("x%d SCC CCCC L",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Target ID                  0x%04X\n", $item1);
            $msg .= sprintf("Port mapped onto           0x%02X\n", $item2);
            $msg .= sprintf("Options                    0x%02X\n", $item3);
            $msg .= sprintf("Fibre channel ID           0x%02X\n", $item4);
#            $msg .= sprintf("Reserved: 0x%02x       ", $item5);
            $msg .= sprintf("Locked target indicator    0x%02X\n", $item6);
#            $msg .= sprintf("Reserved: 0x%02x       ", $item5);
            $msg .= sprintf("Owning controller          0x%08X\n", $item6);

            $offset += 12;
            $address += 12;
            $length -= 12;


            $item1 = FmtWwn($bufferPtr, $offset);
            $msg .= sprintf("Port world wide name       %16s\n", $item1);
            $offset += 8;                         # add bytes processed
            $address += 8;
            $length -= 8;


            $item1 = FmtWwn($bufferPtr, $offset);
            $msg .= sprintf("Node world wide name       %16s\n", $item1);
            $offset += 8;                         # add bytes processed
            $address += 8;
            $length -= 8;


            $fmt = sprintf("x%d L SS CC SL",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Previous owner             0x%08X\n", $item1);
            $msg .= sprintf("Cluster                    0x%04X\n", $item2);
#            $msg .= sprintf("Reserved: 0x%04x     ", $item3);
            $msg .= sprintf("Preferred port             0x%02X\n", $item4);
            $msg .= sprintf("Alternate port             0x%02X\n", $item5);
#            $msg .= sprintf("Reserved: 0x%04x 0x%08x\n", $item6, $item7);

            $offset += 16;
            $address += 16;
            $length -= 16;

# printf ("\n Going over Target extension structures \n");
            my $taddr;
            my $ipstr;

            $taddr = $addresstmp + $rec_length-4;
            if ($taddr == $address)
            {
                # Reached end of record
            }
            else
            {
                # Extension structure exists, read it
                $fmt = sprintf("x%d SLLL",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;
                                
                $msg .= sprintf("Target ID                  %u\n", $item1);
                $ipstr = XIOTech::cmdMgr::net2ip(0, $item2); 
                $msg .= sprintf("IP Address                 %s\n", $ipstr);
                $ipstr = XIOTech::cmdMgr::net2ip(0, $item3); 
                $msg .= sprintf("Subnet Mask                %s\n", $ipstr);
                $ipstr = XIOTech::cmdMgr::net2ip(0, $item4); 
                $msg .= sprintf("Default Gateway            %s\n", $ipstr);
                $offset += 14;
                $address += 14;
                $length -= 14;


                $fmt = sprintf("x%d SCCCC",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
                $msg .= sprintf("Max Connections             %u\n", $item1);
                $msg .= sprintf("initialR2T                  %u\n", $item2);
                $msg .= sprintf("immediateData               %u\n", $item3);
                $msg .= sprintf("dataSequenceInOrder         %u\n", $item4);
                $msg .= sprintf("dataPDUInOrder              %u\n", $item5);

                $offset += 6;
                $address += 6;
                $length -= 6;

                $fmt = sprintf("x%d CCCSL",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
                $msg .= sprintf("ifMarker                    %u\n", $item1);
                $msg .= sprintf("ofMarker                    %u\n", $item2);
                $msg .= sprintf("errorRecoveryLevel          %u\n", $item3);
                $msg .= sprintf("targetPortalGroupTag        %u\n", $item4);
                $msg .= sprintf("maxBurstLength              %u\n", $item5);

                $offset += 9;
                $address += 9;
                $length -= 9;

                $fmt = sprintf("x%d LSSSL",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
                $msg .= sprintf("firstBurstLength            %u\n", $item1);
                $msg .= sprintf("defaultTime2Wait            %u\n", $item2);
                $msg .= sprintf("defaultTime2Retain          %u\n", $item3);
                $msg .= sprintf("maxOutstandingR2T           %u\n", $item4);
                $msg .= sprintf("maxRecvDataSegmentLength    %u\n", $item5);
                                
                $offset += 14;
                $address += 14;
                $length -= 14;

                $fmt = sprintf("x%d SSCCC",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;
                $msg .= sprintf("ifMarkInt                   %u\n", $item1);
                $msg .= sprintf("ofMarkInt                   %u\n", $item2);
                $msg .= sprintf("headerDigest                %u\n", $item3);
                $msg .= sprintf("dataDigest                  %u\n", $item4);
                $msg .= sprintf("authMethod                  %u\n", $item5);

                $offset += 7;
                $address += 7;
                $length -= 7;
                
                $fmt = sprintf("x%d La32LL",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;

                $msg .= sprintf("mtuSize                     %u\n", $item1);
                $msg .= sprintf("tgtAlias                    %s\n", $item2);
                $msg .= sprintf("numUsers                    %u\n", $item4);
                $offset += 44;
                $address += 44;
                $length -= 44;

                if ($item4 > 0)
                {
                    $offset += 6;
                    $address += 6;
                    $length -= 6;
                    # Read CHAP User Info
                    $msg .= sprintf("CHAP User names:\n", $item4);
                    for ($item5 = 0; $item5 < $item4; $item5++)
                    {
                        $fmt = sprintf("x%d a256a32a32",$offset);      # generate the format string
                        ($item1, $item2, $item3) =  
                                        unpack $fmt , $$bufferPtr;
                        $msg .= sprintf("User name           %s\n", $item1);
                        $offset += 44;
                        $address += 44;
                        $length -= 44;
                        
                    }
                }
            }
            
            $offset = $offsettmp + $rec_length-4;
            $address = $addresstmp + $rec_length-4;
            $length = $lengthtmp - $rec_length+4;
        }
        elsif ($rec_type == 0x0F)
        {
            $msg .= "--- Begin Xiotech LDD record structure ----------------------------\n";
    
            #    typedef struct nrx
            #    {
            #        UINT16      lid;                /* LDD ID                               */
            #        UINT8       pmask;              /* Path mask                            */
            #        UINT8       ppri;               /* Path priority                        */
            #        UINT64      devcap;             /* Device capacity                      */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT8       serial[12];         /* Serial number                        */
            #        UINT16      basevd;             /* Base virtual disk number             */
            #        UINT8       basecl;             /* Base cluster number                  */
            #        UINT8       rsvd23;             /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT64      basenode;           /* Base node world wide name            */
            #        UINT32      basesn;             /* Base serial number                   */
            #        UINT16      lun;                /* LUN                                  */
            #        UINT8       rsvd30[2];          /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT8       basename[16];       /* Base device name                     */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT32      owner;              /* Owner of the LDD                     */
            #        UINT8       rsvd36[12];         /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };


            $fmt = sprintf("x%d SCC LL",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("LDD ID                     0x%04X\n", $item1);
            $msg .= sprintf("Path mask                  0x%02X\n", $item2);
            $msg .= sprintf("Path priority              0x%02X\n", $item3);
            $msg .= sprintf("Device capacity            0x%08X%08X\n", $item5, $item4);

            $offset += 12;
            $address += 12;
            $length -= 12;

            #######################

            # print this one as ASCII and HEX

            $item1 = FmtString( $bufferPtr, $offset, 12 );
            # $msg .= sprintf( "              Serial number: $item1\n");
            
            $msg .=         "Serial number              ";
            $msg .= $item1;

            $fmt = sprintf("x%d LLL ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("    ( 0x%08X 0x%08X 0x%08X ) \n", $item1, $item2, $item3);

            $offset += 12;
            $address += 12;
            $length -= 12;
            
            #######################

            $fmt = sprintf("x%d SCC ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Base virtual disk number   0x%04X\n", $item1);
            $msg .= sprintf("Base cluster number        0x%02X\n", $item2);
#            $msg .= sprintf("Reserved: 0x%02x \n", $item3);

            $offset += 4;
            $address += 4;
            $length -= 4;


            #######################

            $item1 = FmtWwn($bufferPtr, $offset);
            $msg .= sprintf("Base node world wide name  %16s\n", $item1);
            $offset += 8;                         # add bytes processed
            $address += 8;
            $length -= 8;




            $fmt = sprintf("x%d LSS ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Base serial number         0x%08X\n", $item1);
            $msg .= sprintf("LUN                        0x%04X\n", $item2);
#            $msg .= sprintf("Reserved: 0x%04x \n", $item3);

            $offset += 8;
            $address += 8;
            $length -= 8;

            #######################

            $item1 = FmtString( $bufferPtr, $offset, 16 );
            $msg .= sprintf("Base device name           $item1\n");

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################

            $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Owner of the LDD           0x%08X\n", $item1);
#            $msg .= sprintf("Reserved: 0x%08x 0x%08x 0x%08x \n", $item2, $item3, $item4);

            $offset += 16;
            $address += 16;
            $length -= 16;
        }
        elsif ($rec_type == 0x10)
        {
            $msg .="--- Begin Foreign LDD record structure ----------------------------\n";

            #    typedef struct nrf
            #    {
            #        UINT16      lid;                /* LDD ID                               */
            #        UINT8       pmask;              /* Path mask                            */
            #        UINT8       ppri;               /* Path priority                        */
            #        UINT64      devcap;             /* Device capacity                      */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT8       serial[12];         /* Serial number                        */
            #        UINT8       vendid[8];          /* Vendor ID string                     */
            #        UINT8       prodid[16];         /* Product ID string                    */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT32      rev;                /* Revision                             */
            #        UINT16      lun;                /* LUN                                  */
            #        UINT8       rsvd54[6];          /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #        UINT32      owner;              /* Owner of the LDD                     */
            #        UINT8       rsvd60[12];         /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };

            $fmt = sprintf("x%d S CC LL ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("LDD ID                     0x%04X\n", $item1);
            $msg .= sprintf("Path mask                  0x%02X\n", $item2);
            $msg .= sprintf("Path priority              0x%02X\n", $item3);
            $msg .= sprintf("Device capacity            0x%08X%08X\n", $item5, $item4);

            $offset += 12;
            $address += 12;
            $length -= 12;

            #######################


            $item1 = FmtString( $bufferPtr, $offset, 12 );
            $msg .= sprintf("Serial number              $item1\n");

            $offset += 12;
            $address += 12;
            $length -= 12;

            $item1 = FmtString( $bufferPtr, $offset, 8 );
            $msg .= sprintf("Vendor ID string           $item1\n");

            $offset += 8;
            $address += 8;
            $length -= 8;

            $item1 = FmtString( $bufferPtr, $offset, 16 );
            $msg .= sprintf("Product ID string          $item1\n");

            $offset += 16;
            $address += 16;
            $length -= 16;

            $fmt = sprintf("x%d L S SL ",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Revision                   0x%04X\n", $item1);
            $msg .= sprintf("LUN                        0x%02X\n", $item2);
#            $msg .= sprintf("Reserved: 0x%08x 0x%08x \n", $item3, $item4);

            $offset += 12;
            $address += 12;
            $length -= 12;

            #######################

             $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Owner of the LDD           0x%04X\n", $item1);
#            $msg .= sprintf("reserved: 0x%08x 0x%08x 0x%08x \n", $item2, $item3, $item4);

            $offset += 16;
            $address += 16;
            $length -= 16;
        }
        elsif ($rec_type == 0x11)
        {
            $msg .= "--- Begin Copy Operation structure ------------------------------------\n";

            #typedef struct nrc
            #{
            #    UINT16          origvid;        /* Origin VID                           */
            #    UINT16          copyvid;        /* Copy VID                             */
            #    UINT8           ctype;          /* Type (mirror, swap, break)           */
            #    UINT8           cstate;         /* Copy state                           */
            #    UINT8           percent;        /* Percent complete                     */
            #    UINT8           request;        /* Action (pause, resume, abort, etc)   */
            #    UINT8           rsvd[4];        /* Reserved                             */
            #                                    /* QUAD BOUNDARY                    *****/
            #};


            $fmt = sprintf("x%d SS CCCC ",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5, $item6) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Origin VID                 0x%04X\n", $item1);
            $msg .= sprintf("Copy VID                   0x%04X\n", $item2);
            $msg .= sprintf("Type (mirror, swap, break) 0x%02X\n", $item3);
            $msg .= sprintf("Copy state                 0x%02X\n", $item4);
            $msg .= sprintf("Percent complete           0x%02X\n", $item5);
            $msg .= sprintf("Action                     0x%02X\n", $item6);

            $offset += 12;
            $address += 12;
            $length -= 12;
        }
        elsif ($rec_type == 0x12)
        {
            $msg .= "--- Begin Mirror Partner record structure ----------------------------\n";

            #    typedef struct nrm
            #    {
            #        UINT32      myserial;           /* Serial number of controller          */
            #        UINT32      mypartner;          /* Serial number of partner controller  */
            #        UINT8       rsvd12[4];          /* Reserved                             */
            #                                        /* QUAD BOUNDARY                    *****/
            #    };

             $fmt = sprintf("x%d L LL ",$offset);      # generate the format string
            ($item1, $item2, $item3) =  
                                unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Serial # of controller     0x%08X\n", $item1);
            $msg .= sprintf("Serial # of partner ctrl   0x%08X\n", $item2);
#            $msg .= sprintf("Reserved: 0x%08x\n", $item3);

            $offset += 12;
            $address += 12;
            $length -= 12;
        }
        elsif ($rec_type == 0x13)
        {
#            printf "Workset START   offset=0x%X  address=0x%X  length=0x%X\n",
#                    $offset, $address, $length;
            
            $msg .= "--- Begin Workset record structure ----------------------------\n";

            #    typedef struct NVRW
            #    {
            #        DEF_WORKSET     workset[DEF_MAX_WORKSETS];
            #    } NVRW;
            #    
            #    #define MAX_WORKSETS                16                                    
            #    
            #    typedef struct DEF_WORKSET                                    
            #    {
            #      UINT8     name[DEF_WS_NAME_SIZE];             /**< Workset name           */                                    
            #      UINT8     vBlkBitmap[DEF_WS_VB_MAP_SIZE];     /**< VBlock bitmap          */
            #      UINT8     serverBitmap[DEF_WS_S_MAP_SIZE];    /**< Server bitmap          */                                    
            #      UINT8     defaultVPort;                       /**< Default VPort          */
            #    } DEF_WORKSET;                                    
            #    
            #    #define DEF_WS_NAME_SIZE            16
            #    #define DEF_WS_VB_MAP_SIZE          2
            #    #define DEF_WS_S_MAP_SIZE           32
            # 
            
            # Variable to hold the size of a Workset struct as defined above.
            # Use this to determine the number of worksets.
            my $sizeOfWorkset = 51;
            
            # Length of the entire Workset record, as read from the header.
            # The actual data lenth may be less but this will allow us to
            # calculate the correct starting point for the next record type.
            my $worksetRecordLength = $rec_length - 4;
             
            # Ofset used within the workset data. 
            my $worksetOffset = $offset; 
            my $worksetCount = ($worksetRecordLength / $sizeOfWorkset);
            $worksetCount = 16 if ($worksetCount > 16);
               
            $msg .= "\n";
            for ($i = 0; $i < $worksetCount; $i++)
            {
                $item1 = FmtString( $bufferPtr, $worksetOffset, 16 );
                $msg .= sprintf("[$i] Name: $item1\n");
                $worksetOffset += 16;

                $fmt = sprintf("x%d a2",$worksetOffset);    # generate the format string
                ($item1) = unpack $fmt , $$bufferPtr;
                
                my @vBlkBitmap = XIOTech::cmdMgr::ParseBitmap($item1);
                $msg .= sprintf("VBlock bitmap: @vBlkBitmap\n");
                $worksetOffset += 2;
                
                $fmt = sprintf("x%d a32",$worksetOffset);   # generate the format string
                ($item1) = unpack $fmt , $$bufferPtr;
                
                my @serverBitmap = XIOTech::cmdMgr::ParseBitmap($item1);
                $msg .= sprintf("Server bitmap: @serverBitmap\n");
                $worksetOffset += 32;
                
                $fmt = sprintf("x%d C ",$worksetOffset);    # generate the format string
                ($item1) = unpack $fmt , $$bufferPtr;
                $msg .= sprintf("Default VPort: %d\n\n", $item1);
                $worksetOffset += 1;
            }
            
            # Set up for the next record type using the record lenth from
            # the header.  This may differ from the actual amount of data 
            # processed.
            $offset += $worksetRecordLength;
            $address += $worksetRecordLength;
            $length -= $worksetRecordLength;
            
#            printf "Workset END     offset=0x%X  address=0x%X  length=0x%X\n",
#                    $offset, $address, $length;
        }
        elsif ($rec_type == 0x14)
        {
        # Old GEOPOOL code.  In order to make this work with any version
        # there is a need not to decode.  Previous version showed as UNUSED
        # and killed snapdump from processing FID 2 properly
            $msg .= "GeoPool - No Longer used:/n";

            if ( $rec_length > 4 )
            {
                $msg .= FmtDataString( $bufferPtr, $address, "word", $rec_length - 4, $offset);
            }

            $offset += $rec_length - 4;
            $address += $rec_length - 4;
            $length -= $rec_length - 4;
            $$destPtr .= $msg;
        }
        elsif ($rec_type == 0x16)
        {
             $msg .= "--- Begin Default Copy Configuration Record structure------------------\n";

            # typedef struct NVDMCR
            # {
            #    UINT32      rid;                /* next rigistration ID of              */
            #    UINT8       cr_pri;             /* COR priority                         */
            #    UINT8       pr_pri;             /* proc priority                        */
            #    UINT8       rsvd06[10];         /* reserved                             */
            # } NVDMCR;

            $fmt = sprintf("x%d L C C  ",$offset);  # generate the format string
            ($item1, $item2, $item3) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("rid                        0x%08X\n", $item1);
            $msg .= sprintf("COR priority               0x%02X\n", $item2);
            $msg .= sprintf("Proc priority              0x%02X\n", $item3);
            
            #Set up for the next record type using the record lenth from
            #the header.  This may differ from the actual amount of data 
            #processed.
            
            $offset += 16;
            $address += 16;
            $length -= 16;
        }
        elsif ($rec_type == 0x17)
        {

            $msg .= "--- Begin Copy Configuration Record structure--------------------------\n";
            #typedef struct NVCOPY
            #{
            #    UINT16      svid;               /* source VID                           */
            #    UINT8       stype;              /* source SCD type                      */
            #    UINT8       shidx;              /* src phs1/2 upd hdlrs idx             */
            #    UINT16      dvid;               /* dest VID                             */
            #    UINT8       dtype;              /* desnssnt SCD type                    */
            #    UINT8       dhidx;              /* dest phs1/2 upd hdlrs idx            */
            #    UINT32      tsegs;              /* total segments                       */
            #    UINT32      rid;                /* copy registration ID                 */
            #                        /*                   -------<0x10>---   */
            #    UINT32      rcsn;               /* CM MAG serial number                 */
            #    UINT8       rcscl;              /* CM source cl num                     */
            #    UINT8       rcsvd;              /* CM source Vdisk num                  */
            #    UINT8       rcdcl;              /* CM destination cl num                */
            #    UINT8       rcdvd;              /* CM destination Vdisk num             */
            #    UINT32      rssn;               /* Copy source serial num               */
            #    UINT32      rdsn;               /* Copy dest serial num                 */
            #                        /*                   -------<0x20>---   */
            #    UINT8       rscl;               /* Copy source cl num                   */
            #    UINT8       rsvd;               /* Copy source Vdisk num                */
            #    UINT8       rdcl;               /* Copy dest cl num                     */
            #    UINT8       rdvd;               /* Copy dest Vdisk num                  */
            #    UINT8       gid;                /* user defined group ID                */
            #    UINT8       cr_crstate;         /* cor copy registation state           */
            #    UINT8       cm_cstate;          /* CM  cstate                           */
            #    UINT8       cm_type;            /* CM  type                             */
            #    UINT8       cm_pri;             /* CM  priority                         */
            #    UINT8       cm_mtype;           /* CM copy type/mirror type             */
            #    UINT8       rsvd2a[2];          /* reserve                              */
            #    UINT32      powner;             /* primary owning controller s/n        */
            #                        /*                   -------<0x30>---   */
            #    UINT32      sowner;             /* secondary owning controller s/n      */
            #    UINT8       rsvd34[12];         /* reserve                              */
            #                        /*                   -------<0x40>---   */
            #    UINT8       label[16];          /* copy label                           */
            #                        /*                   -------<0x50>---   */
            #                        /* This is the RCC storage area         */
            #    UINT32      nssn;               /*   new source sn                      */
            #    UINT32      ndsn;               /*   new destination sn                 */
            #    UINT32      cssn;               /*   current (old) source sn            */
            #    UINT32      cdsn;               /*   current (old) dest sn              */
            #} NVCOPY;
            
            $fmt = sprintf("x%d S CC S CC LL",$offset);  # generate the format string

            ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  unpack $fmt , $$bufferPtr;

            #    UINT16      svid;               /* source VID                           */
            #    UINT8       stype;              /* source SCD type                      */
            #    UINT8       shidx;              /* src phs1/2 upd hdlrs idx             */
            #    UINT16      dvid;               /* dest VID                             */
            #    UINT8       dtype;              /* desnssnt SCD type                    */
            #    UINT8       dhidx;              /* dest phs1/2 upd hdlrs idx            */
            #    UINT32      tsegs;              /* total segments                       */
            #    UINT32      rid;                /* copy registration ID                 */
            #                        /*                   -------<0x10>---   */

            $msg .= sprintf("Source VID                      0x%04X\n", $item1);
            $msg .= sprintf("Source SCD Type                 0x%02X\n", $item2);
            $msg .= sprintf("Src phs1/2 upd hdrl idx         0x%02X\n", $item3);
            $msg .= sprintf("Dest VID                        0x%04X\n", $item4);
            $msg .= sprintf("Desnssnt SCD Type               0x%02X\n", $item5);
            $msg .= sprintf("Dest phs1/2 upd hdrl idx        0x%02X\n", $item6);
            $msg .= sprintf("Total segments                  0x%08X\n", $item7);
            $msg .= sprintf("Copy registration ID            0x%08X\n", $item8);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################

            #    UINT32      rcsn;               /* CM MAG serial number                 */
            #    UINT8       rcscl;              /* CM source cl num                     */
            #    UINT8       rcsvd;              /* CM source Vdisk num                  */
            #    UINT8       rcdcl;              /* CM destination cl num                */
            #    UINT8       rcdvd;              /* CM destination Vdisk num             */
            #    UINT32      rssn;               /* Copy source serial num               */
            #    UINT32      rdsn;               /* Copy dest serial num                 */
            #                        /*                   -------<0x20>---   */

            $fmt = sprintf("x%d L CCCC LL",$offset);      # generate the format string

            ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("CM MAG serial number            0x%08X\n", $item1);
            $msg .= sprintf("CM source cl num                0x%02X\n", $item2);
            $msg .= sprintf("CM source Vdisk num             0x%02X\n", $item3);
            $msg .= sprintf("CM destination cl num           0x%02X\n", $item4);
            $msg .= sprintf("CM destination Vdisk num        0x%02X\n", $item5);
            $msg .= sprintf("Copy source serial num          0x%08X\n", $item6);
            $msg .= sprintf("Copy dest serial num            0x%08X\n", $item7);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################

            #    UINT8       rscl;               /* Copy source cl num                   */
            #    UINT8       rsvd;               /* Copy source Vdisk num                */
            #    UINT8       rdcl;               /* Copy dest cl num                     */
            #    UINT8       rdvd;               /* Copy dest Vdisk num                  */
            #    UINT8       gid;                /* user defined group ID                */
            #    UINT8       cr_crstate;         /* cor copy registation state           */
            #    UINT8       cm_cstate;          /* CM  cstate                           */
            #    UINT8       cm_type;            /* CM  type                             */
            #    UINT8       cm_pri;             /* CM  priority                         */
            #    UINT8       cm_mtype;           /* CM copy type/mirror type             */
            #    UINT8       rsvd2a[2];          /* reserve                              */
            #    UINT32      powner;             /* primary owning controller s/n        */
            #                        /*                   -------<0x30>---   */

            $fmt = sprintf("x%d CCCCCCCCCC S L",$offset);      # generate the format string

            ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8, $item9, $item10, $item11, $item12 ) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Copy source cl num              0x%02X\n", $item1);
            $msg .= sprintf("Copy source Vdisk num           0x%02X\n", $item2);
            $msg .= sprintf("Copy dest cl num                0x%02X\n", $item3);
            $msg .= sprintf("Copy dest Vdisk num             0x%02X\n", $item4);
            $msg .= sprintf("user defined group ID           0x%02X\n", $item5);
            $msg .= sprintf("cor copy regist state           0x%02X\n", $item6);
            $msg .= sprintf("CM cstate                       0x%02X\n", $item7);
            $msg .= sprintf("CM type                         0x%02X\n", $item8);
            $msg .= sprintf("CM priority                     0x%02X\n", $item9);
            $msg .= sprintf("CM copy/mirror type             0x%02X\n", $item10);
            $msg .= sprintf("primary owning controller s/n   0x%08X\n", $item12);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################


            #    UINT32      sowner;             /* secondary owning controller s/n      */
            #    UINT8       rsvd34[12];         /* reserve                              */
            #                        /*                   -------<0x40>---   */

            $fmt = sprintf("x%d L",$offset);      # generate the format string

            ($item1) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("secondary owning controller s/n 0x%08X\n", $item1);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################

            #    UINT8       label[16];          /* copy label                           */
            #                        /*                   -------<0x50>---   */

            $item1 = FmtString( $bufferPtr, $offset, 16 );

            $msg .= sprintf("copy label                      %16s \n", $item1);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################

            #                        /* This is the RCC storage area         */
            #    UINT32      nssn;               /*   new source sn                      */
            #    UINT32      ndsn;               /*   new destination sn                 */
            #    UINT32      cssn;               /*   current (old) source sn            */
            #    UINT32      cdsn;               /*   current (old) dest sn              */
            $fmt = sprintf("x%d LLLL",$offset);      # generate the format string

            ($item1, $item2, $item3, $item4) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("new source sn                   0x%08X\n", $item1);
            $msg .= sprintf("new destination sn              0x%08X\n", $item2);
            $msg .= sprintf("current (old) source sn         0x%08X\n", $item3);
            $msg .= sprintf("current (old) dest sn           0x%08X\n", $item4);

            $offset += 16;
            $address += 16;
            $length -= 16;

            #######################
        }
        elsif ($rec_type == 0x18)
        {
            $msg .= "--- Begin ISNS Record structure --------------------------\n";
            $fmt = sprintf("x%d LL",$offset);      # generate the format string
            ($item1, $item2 ) =  unpack $fmt , $$bufferPtr;

            $msg .= sprintf("Global Flags                    0x%08X\n\n", $item1);

            $offset += 8;
            $address += 8;
            $length -= 8;

            my $locind;
            for($locind = 0; $locind < 5; $locind++)
            {
                $fmt = sprintf("x%d LSS",$offset);      # generate the format string
                ($item1, $item2, $item3 ) =  unpack $fmt , $$bufferPtr;

                $msg .= sprintf("IP                              0x%08X\n", $item1);
                $msg .= sprintf("Port                            0x%08X\n", $item2);
                $msg .= sprintf("Server Flags                    0x%08X\n\n", $item3);

                $offset += 8;
                $address += 8;
                $length -= 8;
            }
            $offset += 12;
            $address += 12;
            $length -= 12;
        }
        elsif ($rec_type == 0x19)
        {
            $msg .= "--- Begin Persistent Reservations Record structure ---------------------\n";

            #typedef struct NVRPR
            #{
            #    UINT16 vid;      /* VID                                  */
            #    UINT16 sid;      /* sid of initiator holding reservation */
            #    UINT8  scope;    /* scope of reservation                 */
            #    UINT8  type;     /* type of reservation                  */
            #    UINT8  regCount; /* number of registrations              */
            #    UINT8  rsvd;
            #} NVRPR;
            #
            #typedef struct NVRPRX
            #{
            #    UINT16 sid;       /* sid of initiator  */
            #    UINT8  tid;       /* tid */
            #    UINT8  lun;       /* LUN */
            #    UINT8  key[8];    /* 8 Byte Registration key */
            #} NVRPRX;

            $fmt = sprintf("x%d SSCCC",$offset);      # generate the format string
            ($item1, $item2, $item3, $item4, $item5 ) =  unpack $fmt , $$bufferPtr;
            $msg .= sprintf("VID                        0x%04X\n", $item1);
            $msg .= sprintf("SID of reservation holder  0x%04X\n", $item2);
            $msg .= sprintf("Scope of reservation       0x%02X\n", $item3);
            $msg .= sprintf("Type of reservation        0x%02X\n", $item4);
            $msg .= sprintf("No. of reservation keys    0x%02X\n", $item5);

            $offset += 8;
            $address += 8;
            $length -= 8;

            my $locind;
            my $keycount = $item5;
            my $prVid = $item1;

            for($locind = 0; $locind < $keycount; $locind++)
            {
                $msg .= sprintf("\n--- Begin PR Key Record structure for VID 0x%04X----------------\n", $prVid);
                $fmt = sprintf("x%d SCC",$offset);      # generate the format string
                ($item1, $item2, $item3 ) =  unpack $fmt , $$bufferPtr;
                $msg .= sprintf("SID                          0x%04X\n", $item1);
                $msg .= sprintf("TID                          0x%02X\n", $item2);
                $msg .= sprintf("LUN                          0x%02X\n", $item3);
                
                $offset += 4;
                $address += 4;
                $length -= 4;

                # Get the reservation key
                $fmt = sprintf("x%d CCCCCCCC",$offset);      # generate the format string
                ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =
                            unpack $fmt, $$bufferPtr;
                $msg .= sprintf("Reservation Key              %02X %02X %02X %02X %02X %02X %02X %02X\n", 
                                    $item1, $item2, $item3, $item4,
                                    $item5, $item6, $item7, $item8);

                $offset += 8;
                $address += 8;
                $length -= 8;
            }

            my $oldOffset = $offset;
            $offset = (($offset + 15) & ~0xF);
            $address = (($address + 15) & ~0xF);
            $length -= ($offset - $oldOffset);
        }
        else
        {
            $msg .= "Unknown record type found:  ";
            $msg .= sprintf("L:0x%04X T:0x%02X S:0x%02X  Address: 0x%X offset: 0x%X \n", 
                            $rec_length, $rec_type, $rec_status, $address, $offset );

            print "Unknown record type found:  ";
            printf("L:0x%04X T:0x%02X S:0x%02X  Address: 0x%X offset: 0x%X \n", 
                   $rec_length, $rec_type, $rec_status, $address , $offset);

            if ( $rec_length > 4 )
            {
                $msg .= FmtDataString( $bufferPtr, $address, "word", $rec_length - 4, $offset);
            }

            $offset += $rec_length - 4;
            $address += $rec_length - 4;
            $length -= $rec_length - 4;
            $$destPtr .= $msg;
            return ERROR;
        }

        # Get the next record header
        $fmt = sprintf("x%d SCC ",$offset);      # generate the format string
        ($rec_length, $rec_type, $rec_status) =  
                            unpack $fmt , $$bufferPtr;

        $offset += 4;
        $address += 4;
        $length -= 4;
    }

    $msg .= "\n\n--------------------- NVRAM Summary ----------------------------------------\n";
    $msg .= "Number of Physical Drives                  $drive_num\n";
    $msg .= "Number of Virtual Disks                    $vdisk_num\n";
    $msg .= "Number of Servers                          $server_num\n";
    $msg .= "Number of Enclosures                       $enclosure_num\n";
    $msg .= "Total capacity of all Seagate drives       $total_cap GB\n";


    $$destPtr .= $msg;

    return GOOD;

}
    


##############################################################################
##############################################################################
##############################################################################

 
##############################################################################
##############################################################################


##############################################################################
#
#          Name: FmtPt1Sig
# Call: 
#   FmtPt1Sig ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#  see tpb.h in ProcBoot/src  near NVSRAM_SIGNATURE_BASE
#
#
#
##############################################################################

sub FmtPt1Sig
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";

    
    # define NVSRAM_SIGNATURE_BASE (BASE_NVSRAM + 0x00006200)
    # define NVSRAM_SIG_CRC             (NVSRAM_SIGNATURE_BASE + 0x00000000) /* 512 byte CRC */
    #


    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "          CRC: 0x%08x  \n",
                             $item1);

    $address += 16;
    $offset += 16;

    #                        /* 00-15 = 16 bit id value-operator entry */
    #                        /* 16-23 = 8 bit family code OR'd (0x10 - bigfoot */
    #                        /* 24-31 = always 0 */
    # define NVSRAM_SIG_CONTROLLER_ID   (NVSRAM_SIGNATURE_BASE + 0x00000010) /* 32 bits stored */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "    Signature: 0x%08x \n",
                             $item1);

    $address += 16;
    $offset += 16;


    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################

##############################################################################
#
#          Name: FmtPt1ECCInit
# Call: 
#   FmtPt1ECCInit ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#  see tpb.h in ProcBoot/src  near NVSRAM_ECC_BATT_BASE
#
#
#
##############################################################################

sub FmtPt1ECCInit
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;


    $msg = "\n";



    # define ADR_BATT_SIG1            (BATTERY_RESERVED_AREA_LO + 0x000)
    # define ADR_BATT_SIG2            (BATTERY_RESERVED_AREA_LO + 0x004)



    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "           Batt Sig 1: 0x%08x ", $item1);
    $msg .= sprintf( "           Batt Sig 2: 0x%08x \n", $item2);

    $address += 16;      # includes 2 undefined words
    $offset += 16;



    #define ADR_BATT_WC_START        (BATTERY_RESERVED_AREA_LO + 0x010)
    #define ADR_BATT_WC_END          (BATTERY_RESERVED_AREA_LO + 0x014)


    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "        Batt WC start: 0x%08x ", $item1);
    $msg .= sprintf( "          Batt WC end: 0x%08x \n", $item2);

    $address += 16;
    $offset += 16;

    #define ADR_BATT_CURR_SNGL_CNT   (BATTERY_RESERVED_AREA_LO + 0x020)
    #define ADR_BATT_CURR_MULT_CNT   (BATTERY_RESERVED_AREA_LO + 0x024)
    #define ADR_BATT_TOT_SNGL_CNT    (BATTERY_RESERVED_AREA_LO + 0x028)
    #define ADR_BATT_TOT_MULT_CNT    (BATTERY_RESERVED_AREA_LO + 0x02c)

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "     Batt curr single: 0x%08x ", $item1);
    $msg .= sprintf( "       Batt curr mult: 0x%08x \n", $item2);
    $msg .= sprintf( "    Batt total single: 0x%08x ", $item3);
    $msg .= sprintf( "      Batt total mult: 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;


    # define ADR_BATT_SNGL_ELOG_OFF   (BATTERY_RESERVED_AREA_LO + 0x030) # num is 0 to 15
    # define ADR_BATT_MULT_ELOG_OFF   (BATTERY_RESERVED_AREA_LO + 0x034) # num is 0 to 15

    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single elog off: 0x%08x ", $item1);
    $msg .= sprintf( "   Batt mult elog off: 0x%08x \n", $item2);

    $address += 16;
    $offset += 16;

    # define ADR_BATT_SNGL_ELOG_REG   (BATTERY_RESERVED_AREA_LO + 0x040)
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "\n Batt single elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;   

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;



    # define ADR_BATT_SNGL_ECAR_REG   (BATTERY_RESERVED_AREA_LO + 0x080)


    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "\n Batt single ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Batt single ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;





    # define ADR_BATT_MULT_ELOG_REG   (BATTERY_RESERVED_AREA_LO + 0x0c0)

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "\n   Batt mult elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult elog reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;


    # define ADR_BATT_MULT_ECAR_REG   (BATTERY_RESERVED_AREA_LO + 0x100)

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "\n   Batt mult ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   Batt mult ecar reg: 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                                $item1, $item2, $item3, $item4);

    $address += 16;
    $offset += 16;

    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
  

##############################################################################
#
#          Name: FmtPt1Post
# Call: 
#   FmtPt1Post ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
#  see tpb.h in ProcBoot/src  near ADR_CRC_BOOT_DATA
#
#
#
##############################################################################

sub FmtPt1Post
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )= @_;
    
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
    my $msg;
    my $i;

    $msg = "\n";

    
    # define ADR_CRC_BOOT_DATA   (LD_BASE_ADDR + 0x00000000) /* word - crc */
    # define ADR_MAGIC_VALUE     (LD_BASE_ADDR + 0x00000004) /* magic # */
    # define ADR_CONTROLLER_ID   (LD_BASE_ADDR + 0x00000008) /* Controller ID # */
    # define ADR_POST_INFO       (LD_BASE_ADDR + 0x0000000c)


    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "                   CRC: 0x%08x ", $item1);
    $msg .= sprintf( "           Magic value: 0x%08x \n", $item2);
    $msg .= sprintf( "         Controller ID: 0x%08x ", $item3);
    $msg .= sprintf( "      POST information: 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    # define ADR_CONTROL_CCB     (LD_BASE_ADDR + 0x00000010) /* byte status-ccb*/
    # define ADR_COMMAND_CCB     (LD_BASE_ADDR + 0x00000011) /* byte cmd for-ccb */
    # define ADR_COMMAND_INV_CNT (LD_BASE_ADDR + 0x00000012) /* byte invalid cmds*/
    # define ADR_LAST_COMMAND    (LD_BASE_ADDR + 0x00000013) /* byte last cmd*/

    $fmt = sprintf("x%d CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "            ccb status: 0x%02x       ", $item1);
    $msg .= sprintf( "           ccb command: 0x%02x \n", $item2);
    $msg .= sprintf( " invalid command count: 0x%02x       ", $item3);
    $msg .= sprintf( "          last command: 0x%02x \n", $item4);

    $address += 8;
    $offset += 8;

    # define ADR_BOOT_CRC_STATUS (LD_BASE_ADDR + 0x00000018) /* byte-0=pass */
    # define ADR_DIAG_CRC_STATUS (LD_BASE_ADDR + 0x00000019) /*     -1=hdr crc failed*/
    # define ADR_FW_CRC_STATUS   (LD_BASE_ADDR + 0x0000001a) /*     -2=code crc failed*/
    # define ADR_Q22S_CRC_STATUS (LD_BASE_ADDR + 0x0000001b)
    # define ADR_Q22M_CRC_STATUS (LD_BASE_ADDR + 0x0000001c)
    # define ADR_Q23S_CRC_STATUS (LD_BASE_ADDR + 0x0000001d)
    # define ADR_Q23M_CRC_STATUS (LD_BASE_ADDR + 0x0000001e)

    $fmt = sprintf("x%d CCCC CCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "       boot crc status: 0x%02x       ", $item1);
    $msg .= sprintf( "       diag crc status: 0x%02x \n", $item2);
    $msg .= sprintf( "         fw crc status: 0x%02x       ", $item3);
    $msg .= sprintf( "       Q22s crc status: 0x%02x \n", $item4);
    $msg .= sprintf( "       Q22m crc status: 0x%02x       ", $item5);
    $msg .= sprintf( "       Q23s crc status: 0x%02x \n", $item6);
    $msg .= sprintf( "       Q23m crc status: 0x%02x \n", $item7);

    $address += 8;
    $offset += 8;


    # define ADR_SDRAM_SHORT_END (LD_BASE_ADDR + 0x00000020) /*short sdram value*/
    # define ADR_SDRAM_LONG_END  (LD_BASE_ADDR + 0x00000024) /*max end value */
    # define ADR_BATTERY_RES_HI  (LD_BASE_ADDR + 0x00000028) /*hi reserved batt info*/

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "       SDRAM short end: 0x%08x ", $item1);
    $msg .= sprintf( "        SDRAM long end: 0x%08x \n", $item2);
    $msg .= sprintf( "        Battery Res hi: 0x%08x ", $item3);
    $msg .= sprintf( "                      : 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    # define ADR_MACH_REGS_0_TO_7 (LD_BASE_ADDR + 0x00000030) /* 8 bytes */
    $fmt = sprintf("x%d CCCC CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "            MACH reg 0: 0x%02x       ", $item1);
    $msg .= sprintf( "            MACH reg 1: 0x%02x \n", $item2);
    $msg .= sprintf( "            MACH reg 2: 0x%02x       ", $item3);
    $msg .= sprintf( "            MACH reg 3: 0x%02x \n", $item4);
    $msg .= sprintf( "            MACH reg 4: 0x%02x       ", $item5);
    $msg .= sprintf( "            MACH reg 5: 0x%02x \n", $item6);
    $msg .= sprintf( "            MACH reg 6: 0x%02x       ", $item7);
    $msg .= sprintf( "            MACH reg 7: 0x%02x \n", $item8);

    $address += 8;
    $offset += 8;

    # define ADR_SCRAMBLE        (LD_BASE_ADDR + 0x00000034) /* w-diags bus scrambler*/

    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "              scramble: 0x%08x ", $item1);
    $msg .= sprintf( "                      : 0x%08x \n", $item2);

    $address += 8;
    $offset += 8;

    #define ADR_POST_FATAL      (LD_BASE_ADDR + 0x00000040) /*w-!0 is fatal */
    #define ADR_POST_NONFATAL   (LD_BASE_ADDR + 0x00000044) /*w-!0 is non-fatal */
    #define ADR_POST_NONFATAL1  (LD_BASE_ADDR + 0x00000048) /*w-!0 is non-fatal */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "            POST fatal: 0x%08x ", $item1);
    $msg .= sprintf( "        POST non-fatal: 0x%08x \n", $item2);
    $msg .= sprintf( "      POST non-fatal 1: 0x%08x ", $item3);
    $msg .= sprintf( "                 rsvd : 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    #define ADR_I2C_DATA_128    (LD_BASE_ADDR + 0x00000050)      128 bytes
    $msg .= sprintf( "\nI2C data (128 bytes):  \n");
    $msg .= FmtDataString( $bufferPtr, $offset, "word", 128, $offset);
    $msg .= "\n";

    $address += 128;
    $offset += 128;

    # define ADR_I2C_SIZE        (LD_BASE_ADDR + 0x000000d0) /* Calc sdram size*/
    $fmt = sprintf("x%d L ",$offset);      # generate the format string
    ($item1) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "       Calc SDRAM size: 0x%08x \n", $item1);

    $address += 4;
    $offset += 4;

    # define ADR_I2C_READ_STATUS (LD_BASE_ADDR + 0x000000d4)/* 0 = OK */ 
    #                                                   /* !0 = 5 possible errors*/
    # define ADR_I2C_CHKSUM      (LD_BASE_ADDR + 0x000000d5) /*Calc sdram chksm*/
    # define ADR_I2C_ECC         (LD_BASE_ADDR + 0x000000d6) /*ECC/ yes=1, no=0*/
    # define ADR_SDRAM_ALL_CLEAR (LD_BASE_ADDR + 0x000000d7) /*sdram cleared:yes=1,no=0*/
    # define ADR_PROC_ID         (LD_BASE_ADDR + 0x000000d8) /*s-2 byte proc id */
    # define ADR_STEP_NUM        (LD_BASE_ADDR + 0x000000da) /*b-cpu step # */
    # define ADR_ECCR_VALUE      (LD_BASE_ADDR + 0x000000db) /*b-0x07=zion */
    # define ADR_POST_LEDS       (LD_BASE_ADDR + 0x000000dc) /*b-latest led value */
    # define ADR_CHIP_COUNT      (LD_BASE_ADDR + 0x000000dd) /* b-number of SDIMM chips*/

    $fmt = sprintf("x%d CCCC SCCC C",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8, $item9 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "       I2C read status: 0x%02x       ", $item1);
    $msg .= sprintf( "          I2C checksum: 0x%02x \n", $item2);
    $msg .= sprintf( "               I2C ECC: 0x%02x       ", $item3);
    $msg .= sprintf( "       SDRAM all clear: 0x%02x \n", $item4);
    $msg .= sprintf( "               Proc ID: 0x%04x       ", $item5);
    $msg .= sprintf( "           Step number: 0x%02x \n", $item6);
    $msg .= sprintf( "            ECCR value: 0x%02x       ", $item7);
    $msg .= sprintf( "             POST LEDs: 0x%02x \n", $item8);
    $msg .= sprintf( "            chip count: 0x%02x \n", $item9);

    $address += 12;
    $offset += 12;


    # define ADR_ACCESS_REM      (LD_BASE_ADDR + 0x000000e0)/*access to other i960*/
    # define ADR_ACCESS_LOC      (LD_BASE_ADDR + 0x000000e4)/*access from other i960*/
    # define ADR_FLASH_SIZE      (LD_BASE_ADDR + 0x000000e8) /*flash size*/
    # define ADR_NVSRAM_SIZE     (LD_BASE_ADDR + 0x000000ec) /*nvsram size*/
   
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "         access remote: 0x%08x ", $item1);
    $msg .= sprintf( "          access local: 0x%08x \n", $item2);
    $msg .= sprintf( "            FLASH size: 0x%08x ", $item3);
    $msg .= sprintf( "           NVRAM size : 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    # define ADR_FLASH_BASE      (LD_BASE_ADDR + 0x000000f0) /*flash part base addr*/
    # define ADR_FLASH_FW_BASE   (LD_BASE_ADDR + 0x000000f4) /* w-fw base addr */
    # define ADR_FLASH_DG_BASE   (LD_BASE_ADDR + 0x000000f8) /* w-diag base addr */
    # define ADR_FLASH_BT_BASE   (LD_BASE_ADDR + 0x000000fc) /* w-boot base addr */
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "            FLASH base: 0x%08x ", $item1);
    $msg .= sprintf( "         FLASH FW base: 0x%08x \n", $item2);
    $msg .= sprintf( "         FLASH DG base: 0x%08x ", $item3);
    $msg .= sprintf( "         FLASH BT base: 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    # define ADR_BT_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000100) /* expected boot hdr crc*/
    # define ADR_BT_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000104) /* calculated*/
    # define ADR_BT_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000108) /* expected boot code crc*/
    # define ADR_BT_COD_CRC_CALC (LD_BASE_ADDR + 0x0000010c) /* calculated*/
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " BOOT hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "BOOT code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_DG_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000110) /* expected diag hdr crc*/
    # define ADR_DG_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000114) /* calculated*/
    # define ADR_DG_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000118) /* expected diag code crc*/
    # define ADR_DG_COD_CRC_CALC (LD_BASE_ADDR + 0x0000011c) /* calculated*/

    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " DIAG hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "DIAG code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_FW_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000120) /* expected fw hdr crc*/
    # define ADR_FW_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000124) /* calculated*/
    # define ADR_FW_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000128) /* expected fw code crc*/
    # define ADR_FW_COD_CRC_CALC (LD_BASE_ADDR + 0x0000012c) /* calculated*/
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   FW hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "  FW code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_Q22S_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000130)
    # define ADR_Q22S_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000134)
    # define ADR_Q22S_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000138)
    # define ADR_Q22S_COD_CRC_CALC (LD_BASE_ADDR + 0x0000013c)
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Q22S hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "Q22S code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

      
    # define ADR_Q22M_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000140)
    # define ADR_Q22M_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000144)
    # define ADR_Q22M_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000148)
    # define ADR_Q22M_COD_CRC_CALC (LD_BASE_ADDR + 0x0000014c)
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Q22M hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "Q22M code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;


    # define ADR_Q23S_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000150)
    # define ADR_Q23S_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000154)
    # define ADR_Q23S_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000158)
    # define ADR_Q23S_COD_CRC_CALC (LD_BASE_ADDR + 0x0000015c)
    
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Q23S hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "Q23S code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;


    # define ADR_Q23M_HDR_CRC_EXP  (LD_BASE_ADDR + 0x00000160)
    # define ADR_Q23M_HDR_CRC_CALC (LD_BASE_ADDR + 0x00000164)
    # define ADR_Q23M_COD_CRC_EXP  (LD_BASE_ADDR + 0x00000168)
    # define ADR_Q23M_COD_CRC_CALC (LD_BASE_ADDR + 0x0000016c)

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " Q23M hdr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "Q23M code CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_CRC_DF_NMI_EXP    (LD_BASE_ADDR + 0x00000170) /* expected diag/fw NMI crc*/
    # define ADR_CRC_DF_NMI_CALC   (LD_BASE_ADDR + 0x00000174) /* calculated */
    # define ADR_CRC_SPD_UP_EXP    (LD_BASE_ADDR + 0x00000178) /* upper 64 SPD exp*/
    # define ADR_CRC_SPD_UP_CALC   (LD_BASE_ADDR + 0x0000017c) /* upper 64 SPD calc*/

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( " D/FW NMI CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( " upper 64 SPD expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_2ND_BUS_SLOT_0    (LD_BASE_ADDR + 0x00000180) /* 2ndary slot 0 info */
    # define ADR_2ND_BUS_SLOT_1    (LD_BASE_ADDR + 0x00000184) /* 2ndary slot 1 info    */
    # define ADR_2ND_BUS_SLOT_2    (LD_BASE_ADDR + 0x00000188) /* 2ndary slot 2 info    */
    # define ADR_2ND_BUS_SLOT_3    (LD_BASE_ADDR + 0x0000018c) /* 2ndary slot 3 info    */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "   2nd Bus slot 0 info: 0x%08x ", $item1);
    $msg .= sprintf( "   2nd Bus slot 1 info: 0x%08x \n", $item2);
    $msg .= sprintf( "   2nd Bus slot 2 info: 0x%08x ", $item3);
    $msg .= sprintf( "   2nd Bus slot 3 info: 0x%08x \n", $item4);

    $address += 16;
    $offset += 16;

    # define ADR_CRC_SIG_EXP       (LD_BASE_ADDR + 0x00000190) /* expected SIG crc*/
    # define ADR_CRC_SIG_CALC      (LD_BASE_ADDR + 0x00000194) /* calculated */
    # define ADR_CRC_BT_NMI_EXP    (LD_BASE_ADDR + 0x00000198) /* expected boot NMI crc*/
    # define ADR_CRC_BT_NMI_CALC   (LD_BASE_ADDR + 0x0000019c) /* calculated */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "      SIG CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
#    $msg .= sprintf( "            calculated: 0x%08x \n", $item2);
    $msg .= sprintf( " boot NMI CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_CRC_POST_CURR_EXP  (LD_BASE_ADDR + 0x000001a0) /* expected POST current crc*/
    # define ADR_CRC_POST_CURR_CALC (LD_BASE_ADDR + 0x000001a4) /* calculated */
    # define ADR_CRC_POST_PREV_EXP  (LD_BASE_ADDR + 0x000001a8) /* expected POST previous crc*/
    # define ADR_CRC_POST_PREV_CALC (LD_BASE_ADDR + 0x000001ac) /* calculated */

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "POST curr CRC expected: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "POST prev CRC expected: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    # define ADR_FAIL_TEST_INFO     (LD_BASE_ADDR + 0x000001b0) /* base address CCB log */
    $msg .= sprintf( "\nFail test info (48b):  \n");
    $msg .= FmtDataString( $bufferPtr, $offset, "word", 48, $offset);
    $msg .= "\n";

    $address += 48;
    $offset += 48;

    # define ADR_2ND_BUS_BIT_MAP   (LD_BASE_ADDR + 0x000001e0) /* address lines with adapters */
    $fmt = sprintf("x%d L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "address lines w/adptrs: 0x%08x \n", $item1);

    $address += 32;
    $offset += 32;

    # - 0x1e4 to 0x1ff - available


    # now add the fw headers. We are at offset 200 here

    # /* boot module header info */
    # define ADR_BT_HEADER       (LD_BASE_ADDR + 0x00000200)
    $i = 0x00;
    $msg .= "--- boot module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* diag module header info */
    # define ADR_DG_HEADER       (LD_BASE_ADDR + 0x00000280)
    $i = 0x80;
    $msg .= "--- diag module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* firmware module header info */
    # define ADR_FW_HEADER       (LD_BASE_ADDR + 0x00000300)
    $i = 0x100;
    $msg .= "--- firmware module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* QLogic 2200 single module header info */
    # define ADR_QL22S_HEADER    (LD_BASE_ADDR + 0x00000380)
    $i = 0x180;
    $msg .= "--- QLogic 2200 single module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* QLogic 2200 multi module header info */
    # define ADR_QL22M_HEADER    (LD_BASE_ADDR + 0x00000400)
    $i = 0x200;
    $msg .= "--- QLogic 2200 multi module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* QLogic 2300 single module header info */
    # define ADR_QL23S_HEADER    (LD_BASE_ADDR + 0x00000480)
    $i = 0x280;
    $msg .= "--- QLogic 2300 single module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address

    # /* QLogic 2300 multi module header info */
    # define ADR_QL23M_HEADER    (LD_BASE_ADDR + 0x00000500)
    $i = 0x300;
    $msg .= "--- QLogic 2300 multi module header info ---\n";
    FmtFwh( \$msg,
        $bufferPtr,
        $offset + $i,   # Offset
        $length - $i,   # Length
        $processor,
        $address + $i);  # Address
               

    $offset += 0x380;    # adjust for the above fw headers
    
               
    # new items added 8/03               
        #define ADR_CRC_CIRC0_NMI_EXP (LD_BASE_ADDR + 0x00000580) /* expected circular 0 crc*/
        #define ADR_CRC_CIRC0_NMI_CALC (LD_BASE_ADDR + 0x00000584) /* calculated */
        #define ADR_CRC_CIRC1_NMI_EXP (LD_BASE_ADDR + 0x00000588) /* expected circular 1 crc*/
        #define ADR_CRC_CIRC1_NMI_CALC (LD_BASE_ADDR + 0x0000058c) /* calculated */
        #define ADR_CRC_CIRC2_NMI_EXP (LD_BASE_ADDR + 0x00000590) /* expected circular 2 crc*/
        #define ADR_CRC_CIRC2_NMI_CALC (LD_BASE_ADDR + 0x00000594) /* calculated */
        #define ADR_CRC_CIRC3_NMI_EXP (LD_BASE_ADDR + 0x00000598) /* expected circular 3 crc*/
        #define ADR_CRC_CIRC3_NMI_CALC (LD_BASE_ADDR + 0x0000059c) /* calculated */
        #define ADR_CRC_CIRC4_NMI_EXP (LD_BASE_ADDR + 0x000005a0) /* expected circular 4 crc*/
        #define ADR_CRC_CIRC4_NMI_CALC (LD_BASE_ADDR + 0x000005a4) /* calculated */
        #define ADR_CRC_CIRC5_NMI_EXP (LD_BASE_ADDR + 0x000005a8) /* expected circular 5 crc*/
        #define ADR_CRC_CIRC5_NMI_CALC (LD_BASE_ADDR + 0x000005ac) /* calculated */
        #define ADR_CRC_CIRC6_NMI_EXP (LD_BASE_ADDR + 0x000005b0) /* expected circular 6 crc*/
        #define ADR_CRC_CIRC6_NMI_CALC (LD_BASE_ADDR + 0x000005b4) /* calculated */
        #define ADR_CRC_CIRC7_NMI_EXP (LD_BASE_ADDR + 0x000005b8) /* expected circular 7 crc*/
        #define ADR_CRC_CIRC7_NMI_CALC (LD_BASE_ADDR + 0x000005bc) /* calculated */
          
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "expected circular 0 crc: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "expected circular 1 crc: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "expected circular 2 crc: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "expected circular 3 crc: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "expected circular 4 crc: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "expected circular 5 crc: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf( "expected circular 6 crc: 0x%08x ", $item1);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item2, ($item2 - $item1));
    $msg .= sprintf( "expected circular 7 crc: 0x%08x ", $item3);
    $msg .= sprintf( "            calculated: 0x%08x (dif %8x)\n", $item4, ($item4 - $item3));

    $address += 16;
    $offset += 16;



    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################

##############################################################################
#
#     This fcn handles the files that are data dumps from the FE or BE 
#     processors.  Data is from NVRAM part 5.
#
#
#
##############################################################################
sub DiagProc
{

    
    my ( $buffer, $flags, $destPtr, $version ) = @_;
    
    my $processor; 
    my $title;   
    my $length;    
    my $crc;       
    my @tS;
    my $fmtTime = ""; 
    my $reseerved1;
    my $fmt;           
    my $offset;
    my $outStr = "";   
    my $headerLength = 16;     # a constant     

    my $fieldTitle; 
    my $memAddress; 
    my $available;

    my $fieldLength;
    my $msg = "";
    my $ret = GOOD;

    my @days = ( "Holiday", "Sunday", "Monday", "Tuesday",
                 "Wednesday", "Thursday", "Friday", "Saturday");

    my %psuedoConstants;     # a hash of psuedo constants

    if ( !defined($version) )
    {
        $version = 0;
    }

    InitConstants ( \%psuedoConstants );  # init the hash
    

    # buffer is the input stream
    # flags control the outcome in some yet-to-be-determined manner

    #####################
    # process the header
    #####################

    $processor = 0;                # CCB, BE or FE, extracted from title
    $title = "-";                  # title in the header
    $length = 0;                   # length following the header to end of file
    $crc = 0;                      # 16 bit CRC of bytes following header
    #$timeStamp = 0;                # 12 bytes of timestamp
    $reseerved1 = 0;               # 4 pad bytes
    $fmt = "";                     # format used for unpack
    $offset = 0;                   # where we are working in the buffer

    # now get the header data from the buffer
    #        00000000  67616944 20454220  000069C0 AD6C1979  Diag BE .i..y.l.
    #        00000010  28012003 42361603  00000000 00000000  . .(..6B........
    #         timestamp = 3 32 1 40 3 22 54 66

    $fmt = "A8LL";

    ($title, $length, $crc ) = unpack $fmt, $buffer;
    
    @tS = unpack ("x16C8", $buffer);

    $msg = sprintf(" %s  %02x%02x/%02x/%02x %02x:%02x:%02x \n\n",
                   $days[$tS[4]], $tS[1], $tS[0], $tS[2], 
                   $tS[3], $tS[5], $tS[6], $tS[7]    );


    $outStr .= "Title: $title  CRC: $crc  TimeStamp: $msg \n"; 
#    print (" Title: $title, CRC: $crc, TimeStamp: $msg \n"); 

    $outStr .= "processing $length bytes\n";

    $processor = substr $title, 4;

    # advance to the first field the start decoding

    $offset = 32;                   # 32 bytes in file header

    #################################
    # Process the individual records
    #################################

    $available = length( $buffer ) ;   # bytes available to do


    while (  ($length - $offset >= 16)  &&  ($available - $offset >= 16) ) 
                                    # there must be at least a full 
                                    # header left to do (32 bytes for the file
                                    # header plus 16 for the record header)
    {
        # get the header for this block

        
        $fieldTitle = "-";          # title of block
        $memAddress = 0;            # address in memory where this data
                                    # came from
        $fieldLength = 0;           # amount of data in this block

        # now read the header

        $fmt = sprintf ("x%dA8LL", $offset);

        ($fieldTitle, $memAddress, $fieldLength) = unpack $fmt, $buffer;
        


        # validate the input to see if it may possibly be real

# (add code here)
        
        # there must be at least fieldLength bytes remaining

        if ( $fieldLength + $headerLength > ($available - $offset) )
        {
            # print what we have decoded
            $msg .= "\n\n##############################################";
            $msg .= "##############################################\n\n";
            $msg .= sprintf("Section: $fieldTitle   Memory address:0x%08x    Length: $fieldLength \n", $memAddress);
            $msg .= sprintf("Section: $fieldLength + $headerLength > ($available - $offset) \n" );
        
            # this is bad, we have an overrun of the buffer
            $msg .= "\n\n   WARNING: Decoded field runs past the end of the buffer.\n";
            $msg .= "            There may be some missing data.\n";
            $msg .= "\n\n##############################################";
            $msg .= "##############################################\n\n";
            $outStr .= $msg;
  #          return ERROR;
        }

        if ( $fieldLength < 1 )
        {
            # print what we have decoded
            $msg .= "\n\n##############################################";
            $msg .= "##############################################\n\n";
            $msg .= sprintf("Section: $fieldTitle   Memory address:0x%08x,    Length: $fieldLength \n", $memAddress);
        
            # this is bad, we have a length that is not real
            $msg .= "\n\nDecoded field length is not correct.\n\n";
            $msg .= "\n\n##############################################";
            $msg .= "##############################################\n\n";
            $outStr .= $msg;
            return ERROR;
        }


        # we have the header, now decode the following block of data
        # (provided there is some)
        if ( $fieldLength > 0 )
        {
            # pass the buffer to the basic decoder selector
            $msg = "";
            $ret = SelectDecoder( 
                            \$msg, 
                            \$buffer, 
                            $fieldTitle, 
                            $fieldLength, 
                            $processor, 
                            ($offset + $headerLength), 
                            $memAddress,
                            \%psuedoConstants,
                            $version
                            );

            # the message string should be formatted
            $outStr .= $msg;

        }
    

        # advance into the buffer

        $offset = $offset + $headerLength + $fieldLength;

        # do next one
    }

    $$destPtr .= $outStr;

    return $ret;
}

##############################################################################
#
#     This fcn handles the files that are data dumps from the FE or BE 
#     processors. Data is 32 kbytes from NVRAM part 1.
#
#     This file is a collection of structure, hopefully at fixed addresses.
#     The decoder will process the identified structures only and skip any 
#     unknown things. Unknown data will not be processed in any way.
#
#     Decoding is based upon the contents of ProcBoot\src\tpb.h
#
##############################################################################
sub Part1Decode
{

    
    my ( $bufferPtr, $flags, $destPtr ) = @_;

    my $ret = GOOD;

    my $fmt;
    my $i;
    my $startAddr;
    my $startRec;

    my $msg;
    my $msgd = "";
    my $msgl;

    $msgl = "---------------------------------------------------";
    $msgl .= $msgl."\n";

    # all decoders called have the following parameter list
    #  ( $destPtr, $bufferPtr, $offset, $length, $processor, $address )
    # we will use ( \$msg, $bufferPtr, OFFSET, LEN, 0, OFFSET) Where OFFSET
    # and LEN are determined form the proc header file for the block 
    # being decoded. Note: not all decoders use LEN.


    $msgd .=  "\n############################################################\n";
    $msgd .=    "            NVRAM Part 1: Signatures and IDs";
    $msgd .=  "\n############################################################\n";
    $msg = "";
    $ret = FmtPt1Sig ( \$msg, $bufferPtr, 0x6200, 512, 0, 0x6200);
    $msgd .=  $msg;

    if ( $ret == GOOD) 
    {

        $msgd .=  "\n############################################################\n";
        $msgd .=    "             NVRAM Part 1: ECC init data @ 0x6400";
        $msgd .=  "\n############################################################\n";
        $msg = "";
        $ret = FmtPt1ECCInit ( \$msg, $bufferPtr, 0x6400, 512, 0, 0x6400);
        $msgd .=  $msg;

    }

    if ( $ret == GOOD) 
    {

        $msgd .=  "\n############################################################\n";
        $msgd .=    "         NVRAM Part 1: Boot Module NMI data @ 0x6600";
        $msgd .=  "\n############################################################\n";
        $msg = "";
        $ret = FmtNvramDump ( \$msg, $bufferPtr, 0x6600, 0x800, "boot", 0);
        $msgd .=  $msg;
    }

    if ( $ret == GOOD) 
    {

        $msgd .=  "\n############################################################\n";
        $msgd .=    "         NVRAM Part 1: latest POST data @ 0x6800";
        $msgd .=  "\n############################################################\n";
        $msg = "";
        $ret = FmtPt1Post ( \$msg, $bufferPtr, 0x6800, 0x800, 0, 0x6800);
        $msgd .=  $msg;
    }

    if ( $ret == GOOD) 
    {

        $msgd .=  "\n############################################################\n";
        $msgd .=    "           NVRAM Part 1: Previous POST data @ 0x7000";
        $msgd .=  "\n############################################################\n";
        $msg = "";
        $ret = FmtPt1Post ( \$msg, $bufferPtr, 0x7000, 0x800, 0, 0x7000);
        $msgd .=  $msg;
    }

    # Aug 2003, 8 NMI captures were added in the 8-24k area. These are
    # a circular buffer. Need to find the first part, then do a loop 
    # to process the 8 in the correct order. Since it is circular, we just
    # need to find the oldest one and start from there

    # first, is there anything there

    $ret = AnythingThere($bufferPtr, 8192, 16384);

    if ($ret == 1)
    {
        # if so/now, identify the first of the 8 NMI captures

        $msgd .=  "\n############################################################\n";
        $msgd .=  "         (up to) Eight error trap / NMI records found \n";
        $msgd .=  "############################################################\n";
        # offset of the current/next pointer
        $fmt = " x24580 l" ;        # 0x6004
                    
        ($startRec) = unpack ($fmt, $$bufferPtr );
        
        $startRec = $startRec % 8;       # just in case we get a bad number        
            
        # Loop thru the 8
        for ( $i = 0; $i < 8; $i++ )
        {
            $startAddr = 8192 + ($startRec * 2048);


            $msgd .=  "\n############################################################\n";

            $msgd .=  sprintf("  NVRAM Part 1: NMI buffer data # %d  @ 0x%04x (record %d) \n",
                           $i, $startAddr, $startRec);

            $msgd .=  "############################################################\n";

            # see if there is anything there

            $ret = AnythingThere($bufferPtr, $startAddr + 4, 0x7FC);
            
            # $ret = 0;
            
            if ( $ret == 1 )
            {
                # something was there, get the data


                $msg = "";
                $ret = FmtNvramDump ( \$msg, $bufferPtr, $startAddr, 0x800, "pt 1", 0);
            
            }
            else
            {
                $msg = "\n    This block was all 0s - decoding skipped.\n\n";
            }

            # add to the string buffer
            $msgd .=  $msg;

            # point to next record

            $startRec = ( 1 + $startRec) % 8;

            
        }
    }
    else
    {   
        $msgd .=  "\n#########################################################\n";
        $msgd .=  "         no additional error trap / NMI records found \n";
        $msgd .=  "#########################################################\n";
        
    }



    $msgd .=  "\n############################################################\n";

    $msgd .=    "             NVRAM Part 1: Most recent NMI data \n";

    $msgd .=  "############################################################\n";

    $msgd .= $msgl . "NVRAM Part 1: NMI data @ 0x7800";
    $msg = "";
    $ret = FmtNvramDump ( \$msg, $bufferPtr, 0x7800, 0x800, "pt 1", 0);
    $msgd .=  $msg;




    # done

    $msgd .= "\n\n--------------------- END OF NVRAM DUMP ----------------------------------------\n";




    $$destPtr .= $msgd;


    return $ret;



}

###############################################################################
# Name:     AnythingThere($bufferPtr, $offset, $length)
#
# Desc:     Scans a portion of the buffer to see if any non-zero data
#           is present. Returns 0 if all data is 0, returns one if there
#           is any non-zero data.
#
# Input:    pointer to a buffer
#           offset to start checking from
#           length (number of bytes) to check
#      
##############################################################################

sub AnythingThere
{
    my  ($bufferPtr, $offset, $length) = @_;

    my $i;
    my $dataByte;
    my $fmt;

    for ( $i = 0; $i < $length; $i++ )
    {
        $fmt = sprintf( "x%d C",  $offset + $i );
        ($dataByte ) = unpack($fmt, $$bufferPtr );
        if ($dataByte != 0 )
        {
            return 1;
        }
    }         

    return (0);
}

###############################################################################
##############################################################################
# Name:     FmtProcData()
#
# Desc:     Format binary data in various formats to STDOUT or a file
#
# Input:    data
#           address (that it came from)
#           format (byte/short/word/binary)
#           filename (if output to go to a file; undef otherwise)
#           reqLength - requested data length (0 = all available data)   
#      
##############################################################################
sub FmtProcData
{
    my ( $ctlr, $proc, $buffer, $address, $format, $destPtr) = @_;

    my $msg = "";
    my $item;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $i;
    my %rsp;
    my $length;
    my $zeroCount;
    my $t0;
    my $t0d;
    my $t0Old;
    my $t1;
    my $t1d;
    my $t1Old;
    my $ret;
    my $fmt;

    $length = length ($buffer);

    # print "Entering FmtProcData $format, $proc\n";

    if ( $format eq "procK_ii" )          # was K_ii
    {
        ##########################################
        # processor  internal information @ K_ii
        ##########################################
        
            
        $msg = "\n";
    
        # --- Begin structure -------------------------------------------------
        #       reserved 26                                                 <s>
        
        #$msg .= "+++++++++++++++++++++++++++++++++++++++++++++++++ new decoder +++++++++++++++++++++++++++++++++++++++++++\n";

        $ret = FmtKii( \$msg, \$buffer, 0, $length, $proc, $address );
        

    
    }
    elsif ($format eq "mrpTrace")               # was  "MRP tr"
    {
        #######################
        #  MRP TRACE DECODE
        #######################

        #print "mrp trace decode\n";


        $zeroCount = 0;

        $msg = "\n";

        #       base      <w>
        $item = unpack "L" , $buffer;
        $buffer = substr $buffer, 4;
        $msg .= sprintf("BASE: 0x%08X  ",$item);

        #        next    <w>
        $item = unpack "L" , $buffer;
        $buffer = substr $buffer, 4;
        $msg .= sprintf("NEXT: 0x%08X  ",$item);

        #        End    <w>    
        $item = unpack "L" , $buffer;
        $buffer = substr $buffer, 4;
        $msg .= sprintf("END: 0x%08X  ",$item);

        #        flags          <w>
        $item = unpack "L" , $buffer;
        $buffer = substr $buffer, 4;
        $msg .= sprintf("FLAGS: 0x%08X\n",$item);

        $t1Old = 0;
        $t0Old = 0;

        # new
        #$msg .= "+++++++++++++++++++++++++++++++++++++++++++++++++ new decoder +++++++++++++++++++++++++++++++++++++++++++\n";
       
        $length = 16 * 1024;
        $ret = FmtMrpTrcP( \$msg, \$buffer, 0, $length, $proc, $address );
    }
#    elsif ($format eq "procDir")
#    {
#        #############################
#        # processor data directory 
#        #############################
#        $msg = "Raw HEX version of the directory:\n";
#        #FmtData($buffer, $address, "word", undef);
#
#        $msg .= FmtDataString( \$buffer, $address, "word", undef, 0);
#
#
#
#        $msg .= "\n\n";
#
#        # fetch and process all directory entries
#        $msg .= ProcessProcDir($ctlr, $proc, $buffer, $address, $format);
#    
#    }
    elsif ($format eq "fltRec")             # was "flt rec"
    {
        ##################
        # Flight Recorder
        ##################

        
        
        #       base      <w>

        #        next    <w>
        #        End    <w>    
        #        flags          <w>

        ($item1, $item2, $item3, $item4) =  
                            unpack " LLLL " , $buffer;
        
        $msg .= sprintf( "\n BASE: 0x%08x ", $item1);
        $msg .= sprintf( " NEXT: 0x%08x ", $item2);
        # $msg .= sprintf( " END: 0x%08x ", $item3);
        $msg .= sprintf( " END: 0x%08x  \n", $item4);

        $address += 16;                        # add bytes processed
        $length -= 16;                         # add bytes processed



        $ret = FmtFltRecP( \$msg, \$buffer,16, $length, $proc, $address );

        #$msg .= FmtFltRecData( $buffer, $address, length($buffer) );
        $msg .= "\n";

    }
    elsif (   ($format eq "iTrace0" ) ||
              ($format eq "iTrace1" ) ||
              ($format eq "iTrace2" ) ||
              ($format eq "iTrace3" ) 
           )               
    {

        $msg = "\n";
        $ret = FmtItrace( \$msg, \$buffer, 0, $length, $proc, $address );



    }
    elsif (   ($format eq "trace0" ) ||
              ($format eq "trace1" ) ||
              ($format eq "trace2" ) ||
              ($format eq "trace3" ) 
           )               
    {
        
        $msg = "\n";
        $ret = FmtTrace( \$msg, \$buffer, 0, $length, $proc, $address );
        

    }
    elsif (   ($format eq "physEQ" ) ||
              ($format eq "raidEQ" ) ||
              ($format eq "raid5EQ" ) ||
              ($format eq "virtEQ" ) ||
              ($format eq "rderr EQ" ) ||
              ($format eq "defineEQ" ) ||
              ($format eq "rinitEQ" ) ||
              ($format eq "xorcomEQ" ) ||
              ($format eq "fsysEQ" ) ||
              ($format eq "physCQ" ) ||     # new 6/3/03
              ($format eq "xorXEQ" ) 
           )               
    {
        # show what we got
        $msg = "\n";
        $ret = FmtExecQ( \$msg, \$buffer, 0, $length, $proc, $address );
    }

    elsif ($format eq "DiagNVRM" )                
    {
        # this one is the nvram part 5 stuff from the field

        # need to call the correct fcn directly
        #$ret = DecodeDump( $buffer, 0,  " ");

        $msg = "";  
        DiagProc ( $buffer, 0, \$msg);

    }


    elsif  (  ($format eq "rderrPCB" ) ||               
              ($format eq "physPCB" ) ||
              ($format eq "raidPCB" ) ||
              ($format eq "raid5PCB" ) ||
              ($format eq "virtPCB" ) ||
              ($format eq "definPCB" ) ||
              ($format eq "rinitPCB" ) ||
              ($format eq "xorCPCB" ) ||
              ($format eq "xorXPCB" ) ||
              ($format eq "physCPCB" ) ||
              ($format eq "fsysPCB" ) 
           )
    {
        # this one is new
        $ret = FmtPcb( \$msg, \$buffer, 0, $length, $proc, $address );
    }


    elsif (
            ($format eq "ispReqQ0" ) ||
            ($format eq "ispReqQ1" ) ||
            ($format eq "ispReqQ2" ) ||
            ($format eq "ispReqQ3" ) ||
            ($format eq "ispRspQ0" ) ||
            ($format eq "ispRspQ1" ) ||
            ($format eq "ispRspQ2" ) ||
            ($format eq "ispRspQ3" ) 
          )             
    {
        # this one is the nvram part 5 stuff from the field
        $ret = FmtIspRspQ( \$msg, \$buffer, 0, $length, $proc, $address );

    }
    elsif ( 
            $format eq "etIRAM"      ||
            $format eq "feIRAM"      ||
            $format eq "beIRAM"
           )  
    {
        
        $ret = FmtIRAM( \$msg, 0, \$buffer, 0, $length, $proc, $address );
        
    }

    elsif ( 
            $format eq "bktrNVRM"   
          )  
    {


        # here buffer need to be 32768 long and only the specific record 
        # present

        $msg = " NVRAM part 1 confirmed.\n";
        
        $ret = Part1Decode( \$buffer, 0, \$msg);


    }


    else
    {
        ###############
        # other stuff
        ###############
        
        $msg = "Other type ($format) handled as HEX: ";

#print ("\n Couldn't match data type....>$format< \n");

        # $msg =  FmtDataString( $buffer, $address, $format, undef);
        #$msg .=  FmtDataStringCG( $buffer, $address, "word", undef);

        $msg .= FmtDataString( \$buffer, $address, "word", undef, 0);


    }

# print $msg;

    return( $msg); 

}


##############################################################################
#
#          Name: FmtFICB
# Call: 
#   FmtFICB ( $destPtr, $bufferPtr, $offset, $reqLength, 
#                          $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results
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
# typedef struct
# {
#    UINT32      vcgID;              /* Virtual controller group ID          */
#    UINT32      cSerial;            /* Controller serial number             */
#    UINT32      seq;                /* Sequence number                      */
#    UINT32      ccbIPAddr;          /* CCB IP address                       */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT32      mirrorPartner;      /* Mirroring partner for resync         */
#    UINT8       rsvd20[12];         /* Reserved                             */
#                                    /* QUAD BOUNDARY                    *****/
#    UINT8       vcgName[16];        /* VCG name                             */
#                                    /* QUAD BOUNDARY                    *****/
# } FICB;
#
#
##############################################################################
sub FmtFICB
{
    my ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    
    my $fmt;
    my $msg = "";

    ##########################################
    # Mirror Partner List 
    ##########################################

    # --- Begin structure -------------------------------------------------
    
    $fmt = sprintf( "x%d L L L L L x12 A16", $offset ); # generate the format string
    my ($vcgID, $cSerial, $seq, $ccbIPAddr, $mirrorPartner, $vcgName) =  
            unpack $fmt, $$bufferPtr;

    $msg .= sprintf "Firmware Initialization Control Block (FICB)\n\n";

    $msg .= sprintf( "vcgID:         %u / 0x%X\n", $vcgID, $vcgID );
    $msg .= sprintf( "cSerial:       %u / 0x%X\n", $cSerial, $cSerial );
    $msg .= sprintf( "seq:           %u\n", $seq );
    $msg .= sprintf( "ccbIPAddr:     %s\n", inet_ntoa(pack "L", $ccbIPAddr));
    $msg .= sprintf( "mirrorPartner: %u / 0x%X\n", $mirrorPartner, $mirrorPartner );
    $msg .= sprintf( "vcgName:       \"%s\"\n", $vcgName );

    # finished
    $$destPtr .= $msg;

    return GOOD;
}



##############################################################################
##############################################################################
sub SelectDecoder
{
    my ( $destPtr, $bufferPtr, $type, $length, $processor, $offset, $address, $constPtr, $version  ) =@_;

    my $ret = GOOD;

    # this is just a "switch" statement pointing us to the corrct 
    # decoder ring for the data 'type'. Since we have the processor, 
    # we can also handle the same type differently depending upon the 
    # processor

    # this will not be terribly exciting code. Caffeine is suggested.

    # need to remove spaces from 'type'
#    $$destPtr .= "\nvoid $type()\n{\n} \n";

    if ( !defined($version) )
    {
        $version = 0;
    }
      
    $$destPtr .= sprintf("\nddrDecode: Processor: %4s  Record ID: %8s  ", $processor, $type);  
    $$destPtr .= sprintf("memory address: 0x%08x  length: 0x%08x\n", $address, $length);  


    $ret = ValidateLength( $destPtr, $bufferPtr, $offset, $length, $type, $address );

    if ($ret == ERROR)
    {
        # the buffer length validation failed, we are done.
        $$destPtr .= "\n------------------------------------------------------------";
        $$destPtr .= "------------------------------------------------------------\n";
        return $ret;
    }

    if ( $type eq "Internal")
    {
        # internal Information aka K_ii
        $ret = FmtKii( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }


    elsif ( ($type eq "Consts")   )
    {   
        # proc define layer executive PCB
        $ret = UpdateConsts( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $constPtr  );
    }

    elsif ( ($type eq "VCD")   )
    {   
        # proc define layer executive PCB
        $ret = FmtVCD( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }

    elsif ( ($type eq "LTMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtLTMTs( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

                 
    elsif ( ($type eq "Svr DB")   )
    {   
        # proc define layer executive PCB
        $ret = FmtSvrDB( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "ICIMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtICIMTs( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "TMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtTMTs( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "LSMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtLsmt( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

     elsif ( ($type eq "SDD")   )
    {   
        # proc define layer executive PCB
        $ret = FmtSdd( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }

    elsif ( ($type eq "VDMT")   )
    {   
        # proc define layer executive PCB
        $ret = FmtVdmt( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "LLDMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtLldmtS( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "VDD")   )
    {   
        # proc define layer executive PCB
        $ret = FmtVdd( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }

    elsif ( ($type eq "DTMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtDtmts( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "CIMT Dir")  ||
            ($type eq "LLDMT Dr")  ||                
            ($type eq "ICIMT Dr")  ||                
            ($type eq "SvrdbDir")  
          )
    {   
        # proc define layer executive PCB
        $ret = FmtGenDir( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Target")   )
    {   
        # proc define layer executive PCB
        $ret = FmtTarget( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }

    elsif ( ($type eq "CIMTs")   )
    {   
        # proc define layer executive PCB
        $ret = FmtCIMTs( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "IMT")   )
    {   
        # proc define layer executive PCB
        $ret = FmtImt( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $constPtr ,$version);
    }

    elsif ( ($type eq "ILMT/WET")   )
    {   
        # proc define layer executive PCB
        $ret = FmtIlmtWet( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Defrag T")   )
    {   
        # proc define layer executive PCB
        $ret = FmtDefragT( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Link QCS")   )
    {   
        # proc define layer executive PCB
        $ret = FmtLinkQCS( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Defn EQ")   ||
            ($type eq "RdErr EQ")  ||
            ($type eq "Raid EQ")   ||
            ($type eq "Phys EQ")   ||
            ($type eq "Rd5 EQ")    ||
            ($type eq "Virt EQ")   ||                             
            ($type eq "Rint EQ")   ||
            ($type eq "XORc EQ")   ||
            ($type eq "XORx EQ")   ||
            ($type eq "Phys CQ")   ||            # new 6/3/03
            ($type eq "FSys EQ")   
          )
    {   
        # generic  executive queue handler
        $ret = FmtExecQ( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Curr PCB")  ||
            ($type eq "Defn PCB")  ||
            ($type eq "Phys PCB")  ||
            ($type eq "Raid PCB")  ||
            ($type eq "Rd5 PCB")   ||
            ($type eq "Virt PCB")  ||
            ($type eq "Rint PCB")  ||
            ($type eq "XORc PCB")  ||
            ($type eq "XORx PCB")  ||
            ($type eq "FSys PCB")  ||
            ($type eq "CachePCB")  ||
            ($type eq "RdErrPCB")  ||
            ($type eq "PhysEPCB")  ||
            ($type eq "PhysCPCB")  ||
            ($type eq "CchIOPCB")  
          )
    {   
        # generic PCB handler
        $ret = FmtPcb( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $version );
    }

    # new 6/3/03
    elsif ( 
            ($type eq "ISPReqQ0")  ||
            ($type eq "ISPReqQ1")  ||
            ($type eq "ISPReqQ2")  ||
            ($type eq "ISPReqQ3")  ||
            ($type eq "ISPRspQ0")  ||
            ($type eq "ISPRspQ1")  ||
            ($type eq "ISPRspQ2")  ||
            ($type eq "ISPRspQ3")    
          )
    {   
        $ret = FmtIspRspQ( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( 
            $type eq "BE IRAM"      ||
            $type eq "FE IRAM"
           )  
    {
        
        $ret = FmtIRAM( $destPtr, 0, $bufferPtr, $offset, $length, $processor, $address, $version );
        
    }




    elsif ( ($type eq "Trc log")   )
    {   
        # proc incoming trace
        $ret = FmtTrace( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Itrc log")   )
    {   
        # proc initiator trace
        $ret = FmtItrace( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "MRP Trce")   )
    {   
        # proc MRP trace
        $ret = FmtMrpTrcP( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Flt Rec")   )
    {   
        # proc flight recorder
        $ret = FmtFltRecP( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "MLMTs")   )
    {   
        # Magnitude Link Management tabless  - multiple
        $ret = FmtMlmt( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    elsif ( ($type eq "Trgt def")   )
    {   
        # target definitions  - multiple
        $ret = FmtTgtDef( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }
# ISCSI_CODE    
    elsif ( ($type eq "iTGDs")   )
    {   
        # target definitions  - multiple
        $ret = FmtITgd( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }
    elsif ( ($type eq "ISCSISES")   )
    {   
        # iSCSI Sessions  - multiple
        $ret = FmtIStats( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }
    elsif ( ($type eq "ISCSICON")   )
    {   
        # iSCSI Connections  - multiple
        $ret = FmtIConn( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }
    elsif ( ($type eq "IDD")   )
    {   
        # IDD definitions - multiple
        $ret = FmtIDD( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }
# ISCSI_CODE
    elsif (  ($type eq "Run FWH")  || 
             ($type eq "Boot FWH") || 
             ($type eq "Diag FWH")  
          )
    {   
        # fw headers - 3 different ones
        $ret = FmtFwh( $destPtr, $bufferPtr, $offset, $length, $processor, $address );
    }

    else
    {
        # whatever is not done goes out in HEX

print "\nnon-matching record...>$type<\n";

        $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $length, $offset);
        $ret = GOOD;
    }

    $$destPtr .= "\n------------------------------------------------------------";
    $$destPtr .= "------------------------------------------------------------\n";

    return $ret;

}


##############################################################################
##############################################################################


1;   # we need this for a PM
#########
#
# Modelines:
# vi: sw=4 ts=4 expandtab
