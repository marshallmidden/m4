# $Id: decodeSupport.pm 145456 2010-08-11 20:33:28Z m4 $
##############################################################################
#   CCBE Integration test library - supporting formatters
#
#   A set of library functions for formatting the buffers that are extracted
#   from the controller for debug.  This contins formatters for commonly
#   used controller structures and very generic itmes.
#
#   It is expected that the user will write a perl script that calls 
#   these.
#
#   For XIOtech internal use only.       
#
##############################################################################
=head1 NAME

XIOTech::decodeSupport

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
           FltRecPLookUp
           FmtCCBbty
           FmtCCBCtlrSU
           FmtCCBETBoot
           FmtCCBETErrCnts
           FmtCCBETRegs
           FmtCCBETSnap
           FmtCCBQmMc
           FmtDataString
           FmtGandRRegs
           FmtIRAM
           FmtLoopSenseData
           FmtPhysDevice
           FmtString
           FmtTargetStruct
           FmtWwn
           GetIPAddr
           InitConstants
           MrpPLookUp
           UpdateConsts
           ValidateLength
           
           
           
           
              
        The less significant ones
           DevTypeLookup
           FmtIRAMBEPart
           FmtIRAMFEPart

=cut


package XIOTech::decodeSupport;

use warnings;
use lib "../CCBE";


use XIOTech::cmdMgr;
use XIOTech::cmUtils;
use XIOTech::constants;
use XIOTech::errorCodes;
use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::logMgr;
use XIOTech::TracDec;

use XIOTech::CmdCodeHashes;

use Time::Local;


#use TestLibs::Logging;
#use TestLibs::Constants;
#use TestLibs::utility;
use constant GOOD => 0;
use constant ERROR => 1;

#
# - perl compiler/interpreter flags 'n' things
#

use strict;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    # set the version for version checking
    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    #$VERSION = do { my @r = (q$Revision: 145456 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker

    @ISA         = qw(Exporter);
    @EXPORT      = qw(

                       &FltRecPLookUp

                       &FmtCCBbty
                       &FmtCCBCtlrSU
                       &FmtCCBETBoot
                       &FmtCCBETDataRun
                       &FmtCCBETErrCnts
                       &FmtCCBETRegs
                       &FmtCCBETSnap
                       &FmtCCBQmMc
                       &FmtCCBStats

                       &FmtDataString
                       &FmtGandRRegs
                       &FmtBitmap

                       &FmtIRAM
                       &FmtLoopSenseData
                       &FmtPhysDevice
                       &FmtString
                       &FmtTargetStruct
                       &FmtWwn
                       &GetIPAddr
                       &InitConstants
                       &MrpPLookUp
                       &UpdateConsts
                       &Uptime
                       &ValidateLength
           
                       &DevTypeLookup
                       &FmtIRAMBEPart
                       &FmtIRAMFEPart

                      );
    #%EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],

    # your exported package globals go here,
    # as well as any optionally exported functions
    #@EXPORT_OK   = qw($Var1 %Hashit &func3);

#    TestLibs::Logging::logVersion(__PACKAGE__, q$Revision: 145456 $);
#    TestLibs::Logging::logVersion(__PACKAGE__, q$Name$);
}
    our @EXPORT_OK;

##############################################################################
#               Public Functions
##############################################################################

##############################################################################
# Name:     FmtPhysDevice()
#
# Desc:     format the physical device structure
#
# Call: 
#   FmtPhysDevice ( $destPtr, $hashPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results ( may be 0 )
#           $hashPtr - pointer to return a hash with the data ( may be 0 )
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#                     (may be zero )
#
#           The following may be optional
#
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, currently not 
#                        used, but may be used if the data differs between 
#                        processors
#           $address - Memory address where the data came from.
#
#  Return: GOOD or ERROR
#    
# The data structure being decoded...
#
#typedef struct phyDevOut_t
#{
#    UINT8   rsvd[3];        /* RESERVED                             */
#    UINT8   status;         /* Status                               */
#    UINT32  len;            /* Length of this return packet in bytes*/
#    UINT8   pd_class;       /* Device class                         */
#    UINT8   pd_devtype;     /* Device type                          */
#    UINT8   pd_miscstat;    /* Misc status                          */
#    UINT8   pd_loopmap;     /* Index into loop map                  */
#    UINT8   pd_channel;     /* FC channel #                         */
#    UINT8   rsvd2;          /* RESERVED                             */
#    UINT16  pd_lun;         /* LUN for this device                  */
#/* QUAD 1 */
#    UINT32  pd_id;          /* FC ordinal                           */
#    UINT32  pd_dev;         /* DEVice pointer                       */
#    UINT16  pd_pid;         /* PID for this device                  */
#    UINT16  rsvd3;          /* RESERVED                             */
#    UINT8   pd_poststat;    /* post status                          */
#    UINT8   pd_devstat;     /* device status                        */
#    UINT8   pd_fled;        /* fail led status                      */
#    UINT8   pctrem;         /* % remaining of rebuild operation     */
#/* QUAD 2 */
#    UINT64  pd_devcap;      /* device capacity                      */
#    UINT32  pd_qd;          /* queue depth                          */
#    UINT32  pd_rps;         /* avg req/sec                          */
#/* QUAD 3 */
#    UINT32  pd_avgsc;       /* avg sector count per req             */
#    UINT32  pd_sserial;     /* system serial #                      */
#    UINT64  pd_rreq;        /* read request count                   */
#/* QUAD 4 */
#    UINT64  pd_wreq;        /* write request count                  */
#    UINT8   ps_vendid[8];   /* vendor ID                            */
#/* QUAD 5 */
#    UINT8   pd_rev[4];      /* revision                             */
#    UINT32  pd_err;         /* error count                          */
#    UINT8   ps_prodid[16];  /* product id                           */
#    UINT8   ps_serial[12];  /* serial #                             */
#    UINT32  pd_daml;        /* DAML pointer for this disk           */
#    UINT64  pd_tas;         /* total available space                */
#/* QUAD 8 */
#    UINT64  pd_las;         /* largest available space              */
#    UINT64  pd_wwn;         /* WWN for this device                  */
#/* QUAD 9 */
#    UINT32  R10_miscomp;    /* R10 Miscompare count                 */
#    UINT8   pd_dname[4];    /* GUI positioning info                 */
#    UINT32  lfcnt;          /* Loop statistics                      */
#    UINT32  lscnt;          /* Loop statistics                      */
#/* QUAD 10 */
#    UINT32  lgcnt;          /* Loop statistics                      */
#    UINT32  pscnt;          /* Loop statistics                      */
#    UINT32  itcnt;          /* Loop statistics                      */
#    UINT32  iccnt;          /* Loop statistics                      */
#/* QUAD 11 */
#    UINT32  miscomp;        /* Miscompare count as parity stripe    */
#    UINT32  devmiscomp;     /* Miscompare count as data device      */
#    UINT64  rbtotal;        /* Rebuild - total # sectors            */
#/* QUAD 12 */
#    UINT64  pd_rbremain;    /* Rebuild - remain # sectors           */
#    UINT16  pd_ses;         /* Current enclosure PID                */
#    UINT8   pd_slot;        /* Current slot number n enclosure      */
#    UINT8   rsvd8[13];      /* RESERVED                             */
#/* TOTAL SIZE: 216 Bytes */
#} PI_PHY_DEV_OUT, *PPI_PHY_DEV_OUT;
#
#
##############################################################################
sub FmtPhysDevice
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
    my $item9;
    my $msg;
    my $str;
    my %info;

    ##########################################
    # physical device structure 
    ##########################################
    # print "formatting a physical device \n";
    
    $msg = "\n";

    # --- Begin structure -------------------------------------------------
    #    UINT8   rsvd[3];        /* RESERVED                             */
    #    UINT8   status;         /* Status                               */
    #    UINT32  len;            /* Length of this return packet in bytes*/

    #    UINT8   pd_class;       /* Device class                         */
    #    UINT8   pd_devtype;     /* Device type                          */
    #    UINT8   pd_miscstat;    /* Misc status                          */
    #    UINT8   pd_loopmap;     /* Index into loop map                  */
    #    UINT8   pd_channel;     /* FC channel #                         */
    #    UINT8   rsvd2;          /* RESERVED                             */
    #    UINT16  pd_lun;         /* LUN for this device                  */
    
    $fmt = sprintf("x%d CCC C L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" reserved:     0x%02x 0x%02x 0x%02x    \n", $item1, $item2, $item3);
    $msg .= sprintf(" status:       0x%02x     \n",$item4);
    $msg .= sprintf(" length:       0x%08x     \n",$item5);

    $info{RSVD1A}  = $item1;
    $info{RSVD1B}  = $item2;
    $info{RSVD1C}  = $item3;
    $info{STATUS} = $item4;
    $info{LEN}    = $item5;


    $offset += 8;                         # 8 bytes processed

    $fmt = sprintf("x%d CCCC CC S ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $str = DevTypeLookup($item2);


    $msg .= sprintf(" pd_class:     0x%02x    \n", $item1);
    $msg .= sprintf(" pd_devtype:   0x%02x  %s  \n", $item2, $str);
    $msg .= sprintf(" pd_miscstat:  0x%02x    \n", $item3);
    $msg .= sprintf(" pd_loopmap:   0x%02x    \n", $item4);

    $msg .= sprintf(" pd_channel:   0x%02x    \n", $item5);
    $msg .= sprintf(" rsvd2:        0x%02x    \n", $item6);
    $msg .= sprintf(" pd_lun:       0x%08x    \n", $item7);

    $info{PD_CLASS}    = $item1;
    $info{PD_DEVTYPE}  = $item2;
    $info{PD_MISCSTAT} = $item3;
    $info{PD_LOOPMAP}  = $item4;
    $info{PD_CHANNEL}  = $item5;
    $info{RSVD2}       = $item6;
    $info{PD_LUN}      = $item7;


    $offset += 8;                         # 8 bytes processed
    
    #/* QUAD 1 */
    #    UINT32  pd_id;          /* FC ordinal                           */
    #    UINT32  pd_dev;         /* DEVice pointer                       */
    #    UINT16  pd_pid;         /* PID for this device                  */
    #    UINT16  rsvd3;          /* RESERVED                             */
    #    UINT8   pd_poststat;    /* post status                          */
    #    UINT8   pd_devstat;     /* device status                        */
    #    UINT8   pd_fled;        /* fail led status                      */
    #    UINT8   pctrem;         /* % remaining of rebuild operation     */
    

    $fmt = sprintf("x%d LL SS CCCC", $offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_id:        0x%08x    \n", $item1);
    $msg .= sprintf(" pd_dev:       0x%08x    \n", $item2);
    $msg .= sprintf(" pd_pid:       0x%04x    \n", $item3);
    $msg .= sprintf(" rsvd3:        0x%04x    \n", $item4);
    $msg .= sprintf(" pd_poststat:  0x%02x    \n", $item5);
    $msg .= sprintf(" pd_devstat:   0x%02x    \n", $item6);
    $msg .= sprintf(" pd_fled:      0x%02x    \n", $item7);
    $msg .= sprintf(" pctrem:       0x%02x    \n", $item8);

    $info{PD_ID}       = $item1;
    $info{PD_DEV}      = $item2;
    $info{PD_PID}      = $item3;
    $info{PD_RSVD3}    = $item4;
    $info{PD_POSTSTAT} = $item5;
    $info{PD_DEVSTAT}  = $item6;
    $info{PD_FLED}     = $item7;
    $info{PCTREM}      = $item8;

    $offset += 16;                         #  bytes processed

    #/* QUAD 2 */
    #    UINT64  pd_devcap;      /* device capacity                      */
    #    UINT32  pd_qd;          /* queue depth                          */
    #    UINT32  pd_rps;         /* avg req/sec                          */

    $fmt = sprintf("x%d LL LL", $offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_devcap:    0x%08x%08x \n",  $item2, $item1);
    $msg .= sprintf(" pd_qd:        0x%08x    \n", $item3);
    $msg .= sprintf(" pd_rps:       0x%08x    \n", $item4);

    $info{PD_DEVCAP_HI}  = $item2;
    $info{PD_DEVCAP_LO}  = $item1;
    $info{PD_QD}         = $item3;
    $info{PD_RPS}        = $item4;

    $offset += 16;                         #  bytes processed

    #/* QUAD 3 */
    #    UINT32  pd_avgsc;       /* avg sector count per req             */
    #    UINT32  pd_sserial;     /* system serial #                      */
    #    UINT64  pd_rreq;        /* read request count                   */

    
    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_avgsc:     0x%08x    \n", $item1);
    $msg .= sprintf(" pd_sserial:   0x%08x    \n", $item2);
    $msg .= sprintf(" pd_rreq:      0x%08x%08x \n",  $item4, $item3);

    $info{PD_AVGSC}   = $item1;
    $info{PD_SSERIAL} = $item2;
    $info{PD_RREQ_HI} = $item4;
    $info{PD_RREQ_LO} = $item3;

    $offset += 16;                         #  bytes processed

    #/* QUAD 4 */
    #    UINT64  pd_wreq;        /* write request count                  */
    #    UINT8   ps_vendid[8];   /* vendor ID                            */

    $fmt = sprintf("x%d LL",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_wreq:      0x%08x%08x \n",  $item2, $item1);

    $info{PD_WREQ_HI} = $item2;
    $info{PD_WREQ_LO} = $item1;

    $offset += 8;                         #  bytes processed

    $str = FmtString( $bufferPtr, $offset, 8);

    $msg .= sprintf(" ps_vendid:    %8s\n",  $str);

    $info{PS_VENDID} = $item2;

    $offset += 8;                         #  bytes processed


    #/* QUAD 5 */
    #    UINT8   pd_rev[4];      /* revision                             */
    #    UINT32  pd_err;         /* error count                          */
    #    UINT8   ps_prodid[16];  /* product id                           */
    #    UINT8   ps_serial[12];  /* serial #                             */
    #    UINT32  pd_daml;        /* DAML pointer for this disk           */
    #    UINT64  pd_tas;         /* total available space                */

    # revision and error count
    $fmt = sprintf("x%d LL",$offset);      # generate the format string
    ($item1, $item2) =   unpack $fmt , $$bufferPtr;

    $str = FmtString( $bufferPtr, $offset, 4);
    
    $msg .= sprintf(" pd_rev:       %4s  ( 0x%08x )\n",  $str, $item1);
    $msg .= sprintf(" pd_err:       0x%08x \n", $item2);

    $info{PD_REV} = $str;
    $info{PD_ERR} = $item2;

    $offset += 8;                         #  bytes processed

    # product ID
    $str = FmtString( $bufferPtr, $offset, 16);
    $msg .= sprintf(" ps_prodid:    %16s  \n",  $str);
    $info{PS_PRODID} = $str;
    $offset += 16;                         #  bytes processed

    # serial number
    $str = FmtString( $bufferPtr, $offset, 12);
    $msg .= sprintf(" ps_serial:    %12s  \n",  $str);
    $info{PS_SERIAL} = $str;
    $offset += 12;                         #  bytes processed

    
    # DAML and space
    $fmt = sprintf("x%d L LL",$offset);      # generate the format string
    ($item1, $item2, $item3) =   unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" pd_daml:      0x%08x \n", $item1);
    $msg .= sprintf(" pd_tas:       0x%08x%08x \n",  $item3, $item2);

    $info{PD_DAML} = $item1;
    $info{PD_TAS_LO} = $item3;
    $info{PD_TAS_HI} = $item2;

    $offset += 12;                         #  bytes processed


    #/* QUAD 8 */
    #    UINT64  pd_las;         /* largest available space              */
    #    UINT64  pd_wwn;         /* WWN for this device                  */

    $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;
    
    $str = FmtWwn( $bufferPtr, ($offset + 8));

    $msg .= sprintf(" pd_las:       0x%08x%08x \n",  $item2, $item1);
    $msg .= sprintf(" pd_wwn:       %16s \n", $str);

    $info{PD_LAS_LO} = $item2;
    $info{PD_LAS_HI} = $item1;
    $info{PD_WWN_LO} = $item3;
    $info{PD_WWN_HI} = $item4;


    $offset += 16;                         #  bytes processed



    #/* QUAD 9 */
    #    UINT32  R10_miscomp;    /* R10 Miscompare count                 */
    #    UINT8   pd_dname[4];    /* GUI positioning info                 */
    #    UINT32  lfcnt;          /* Loop statistics                      */
    #    UINT32  lscnt;          /* Loop statistics                      */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    $str = FmtString( $bufferPtr, ($offset + 4), 4);
    
    $msg .= sprintf(" R10_miscomp:  0x%08x \n", $item1);
    $msg .= sprintf(" pd_dname:     %4s  ( 0x%08x )\n",  $str, $item2);
    $msg .= sprintf(" lfcnt:        0x%08x \n", $item3);
    $msg .= sprintf(" lscnt:        0x%08x \n", $item4);

    $info{PD_R10MISCOMP} = $item1;
    $info{PD_DNAME} = $str;
    $info{LFCNT} = $item3;
    $info{LSCNT} = $item4;

    $offset += 16;                         #  bytes processed



    #/* QUAD 10 */
    #    UINT32  lgcnt;          /* Loop statistics                      */
    #    UINT32  pscnt;          /* Loop statistics                      */
    #    UINT32  itcnt;          /* Loop statistics                      */
    #    UINT32  iccnt;          /* Loop statistics                      */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =   unpack $fmt , $$bufferPtr;

    
    $msg .= sprintf(" lgcnt:        0x%08x \n", $item1);
    $msg .= sprintf(" pscnt:        0x%08x \n", $item2);
    $msg .= sprintf(" itcnt:        0x%08x \n", $item3);
    $msg .= sprintf(" iccnt:        0x%08x \n", $item4);

    $info{LGCNT} = $item1;
    $info{PSCNT} = $item2;
    $info{ITCNT} = $item3;
    $info{ICCNT} = $item4;

    $offset += 16;                         #  bytes processed



    #/* QUAD 11 */
    #    UINT32  miscomp;        /* Miscompare count as parity stripe    */
    #    UINT32  devmiscomp;     /* Miscompare count as data device      */
    #    UINT64  rbtotal;        /* Rebuild - total # sectors            */

    $fmt = sprintf("x%d LLLL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_avgsc:     0x%08x    \n", $item1);
    $msg .= sprintf(" pd_sserial:   0x%04x    \n", $item2);
    $msg .= sprintf(" pd_rreq:      0x%08x%08x \n",  $item4, $item3);

    $info{PD_AVGSC}   = $item1;
    $info{PD_SSERIAL} = $item2;
    $info{PD_RREQ_HI} = $item4;
    $info{PD_RREQ_LO} = $item3;

    $offset += 16;                         #  bytes processed


    #/* QUAD 12 */
    #    UINT64  pd_rbremain;    /* Rebuild - remain # sectors           */
    #    UINT16  pd_ses;         /* Current enclosure PID                */
    #    UINT8   pd_slot;        /* Current slot number n enclosure      */
    #    UINT8   rsvd8[13];      /* RESERVED                             */
    #/* TOTAL SIZE: 216 Bytes */



    $fmt = sprintf("x%d LL SC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" pd_rbremain:  0x%08x%08x \n",  $item2, $item1);
    $msg .= sprintf(" pd_ses:       0x%04x    \n", $item3);
    $msg .= sprintf(" pd_slot:      0x%02x    \n", $item4);

    $info{PD_RBREMAIN_HI} = $item2;
    $info{PD_RBREMAIN_LO} = $item1;
    $info{PD_SES}         = $item3;
    $info{PD_SLOT}        = $item4;

#    $offset += 16;                         #  bytes processed
    $offset += 11;                         #  bytes processed

    # read geo-location 

    $fmt = sprintf("x%d Ca4C ",$offset);      # generate the format string
    ($item1,$item2,$item3 ) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf(" gl_id:        %u    \n", $item3);
    $info{GL_ID}        = $item3;

    $offset += 6;                         #  bytes processed

    # the rest are reserved . . . 

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    if ( $hPtr )
    {
        %$hPtr = %info;            # hash will overwrite
    }

    # print "fmt phys device ... done \n";

    return GOOD;
}

##############################################################################
# Name:     FmtTargetStruct()
#
# Desc:     format the physical device structure
#
# Call: 
#   FmtTargetStruct ( $destPtr, $hashPtr, $bufferPtr, $offset, $reqLength, $processor, $address);
#
# Input:    $destPtr - pointer to a scalar for the ascii results ( may be 0 )
#           $hashPtr - pointer to return a hash with the data ( may be 0 )
#           $bufferPtr - pointer to a buffer scalar with the data
#           $offset - offset into the buffer to the start of the data
#                     (may be zero )
#
#           The following may be optional
#
#           $reqLength - The number of bytes to process. There must be
#                        this many bytes in the buffer and it should be
#                        a multiple of i6 bytes.
#           $processor - text string with the processor, currently not 
#                        used, but may be used if the data differs between 
#                        processors
#           $address - Memory address where the data came from.
#
#  Return: GOOD or ERROR
#    
# The data structure being decoded...
#
#typedef struct getTargetInfo_t
#{
#    UINT8   rsvd[3];        /* Reserved                                 */
#    UINT8   status;         /* Status                                   */
#    UINT32  len;            /* Length of this return packet in bytes    */
#
#    UINT16  tgd_tid;        /* Target ID                                */
#    UINT8   tgd_chan;       /* Channel number                           */
#    UINT8   tgd_opt;        /* Options hard or soft ID, active or inact */
#    UINT8   tgd_fcid;       /* Fibre Channel ID                         */
#    UINT8   rsvd0;          /* Reserved                                 */
#    UINT8   tgd_lock;       /* Target lock bits                         */
#    UINT8   rsvd1;          /* Reserved                                 */
#
#    UINT64  tgd_pname;      /* Port name                                */
#
#    UINT64  tgd_nname;      /* Node name                                */
#
#    UINT32  tgd_powner;     /* Previous owning controller serial number */
#    UINT32  tgd_owner;      /* Owning controller serial number          */
#
#    UINT16  tgd_cluster;    /* Clustered target ID                      */
#    UINT8   rsvd2[2];       /* Reserved                                 */
#    UINT8   tgd_pport;      /* Primary port                             */
#    UINT8   tgd_aport;      /* Alternate port                           */
#    UINT8   rsvd3[6];       /* Reserved                                 */
#    UINT32  tgd_modMask;    /* Define which fields to modify 0 = all    */
#}PI_GET_TARGET_INFO;
##############################################################################
sub FmtTargetStruct
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
    my $item9;
    my $msg;
    my $str;
    my %info;

    ##########################################
    # Target structure  from mrp2.h
    ##########################################
    #    UINT8   rsvd[3];        /* Reserved                                 */
    #    UINT8   status;         /* Status                                   */
    #    UINT32  len;            /* Length of this return packet in bytes    */
    #
    $fmt = sprintf("x%d CCC C L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" reserved:     0x%02x 0x%02x 0x%02x    \n", $item1, $item2, $item3);
    $msg .= sprintf(" status:       0x%02x     \n",$item4);
    $msg .= sprintf(" length:       0x%08x     \n",$item5);

    $info{RSVD1A}  = $item1;
    $info{RSVD1B}  = $item2;
    $info{RSVD1C}  = $item3;
    $info{STATUS} = $item4;
    $info{LEN}    = $item5;


    $offset += 8;                         # 8 bytes processed


    #    UINT16  tgd_tid;        /* Target ID                                */
    #    UINT8   tgd_chan;       /* Channel number                           */
    #    UINT8   tgd_opt;        /* Options hard or soft ID, active or inact */
    #    UINT8   tgd_fcid;       /* Fibre Channel ID                         */
    #    UINT8   rsvd0;          /* Reserved                                 */
    #    UINT8   tgd_lock;       /* Target lock bits                         */
    #    UINT8   rsvd1;          /* Reserved                                 */

    $fmt = sprintf("x%d SCC CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" tgd_tid:      0x%04x \n", $item1);
    $msg .= sprintf(" tgd_chan:     0x%02x \n", $item2);
    $msg .= sprintf(" tgd_opt:      0x%02x \n", $item3);
    $msg .= sprintf(" tgd_fcid:     0x%02x \n", $item4);
    $msg .= sprintf(" rsvd0:        0x%02x \n", $item5);
    $msg .= sprintf(" tgd_lock:     0x%02x \n", $item6);
    $msg .= sprintf(" rsvd1:        0x%02x \n", $item7);

    $info{TGD_TID}  = $item1;
    $info{TGD_CHAN} = $item2;
    $info{TGD_OPT}  = $item3;
    $info{TGD_FCID} = $item4;
    $info{RSVD0}    = $item5;
    $info{TGD_LOCK} = $item6;
    $info{RSVD1}    = $item7;


    $offset += 8;                         # 8 bytes processed


    #
    #    UINT64  tgd_pname;      /* Port name                                */
    #
    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf(" port WWN:     %16s\n", $item1);

    $info{TGD_PNAME_LO}  = $item1;
    $info{TGD_PNAME_HI}  = $item2;

    $offset += 8;                         # add bytes processed

    #
    #    UINT64  tgd_nname;      /* Node name                                */
    #
    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;

    $item1 = FmtWwn($bufferPtr, $offset);
    $msg .= sprintf(" node WWN:     %16s\n", $item1);

    $info{TGD_NNAME_LO}  = $item1;
    $info{TGD_NNAME_HI}  = $item2;

    $offset += 8;                         # add bytes processed


    #
    #    UINT32  tgd_powner;     /* Previous owning controller serial number */
    #    UINT32  tgd_owner;      /* Owning controller serial number          */
    #    UINT16  tgd_cluster;    /* Clustered target ID                      */
    #    UINT8   rsvd2[2];       /* Reserved                                 */
    #

    $fmt = sprintf("x%d LL SS ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" tgd_powner:   0x%08x \n", $item1);
    $msg .= sprintf(" tgd_owner:    0x%08x \n", $item2);
    $msg .= sprintf(" tgd_cluster:  0x%04x \n", $item3);
    $msg .= sprintf(" rsvd2:        0x%04x \n", $item4);

    $info{TGD_POWNER}   = $item1;
    $info{TGD_OWNER}    = $item2;
    $info{TGD_CLUSTER}  = $item3;
    $info{RSVD2}        = $item4;
   
    $offset += 12;                         # add bytes processed



    #    UINT8   tgd_pport;      /* Primary port                             */
    #    UINT8   tgd_aport;      /* Alternate port                           */
    #    UINT8   rsvd3[6];       /* Reserved                                 */
    #    UINT32  tgd_modMask;    /* Define which fields to modify 0 = all    */

    $fmt = sprintf("x%d CC SSS L",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(" tgd_pport:    0x%02x \n", $item1);
    $msg .= sprintf(" tgd_aport:    0x%02x \n", $item2);
    $msg .= sprintf(" rsvd3:        0x%04x 0x%04x 0x%04x \n", $item3, $item4, $item5);
    $msg .= sprintf(" tgd_modMask:  0x%08x \n", $item6);

    $info{TGD_PPORT}   = $item1;
    $info{TGD_APORT}   = $item2;
    $info{RSVD3A}      = $item3;
    $info{RSVD3B}      = $item4;
    $info{RSVD3C}      = $item5;
    $info{TGD_MODMASK} = $item6;
   
    $offset += 12;                         # add bytes processed

    $msg .= "\n";

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
sub FmtIRAM
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $version ) = @_;
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
    my $msg = "\n";
    my $str;
    my %info;

    if ( !defined($version) )
    {
        $version = 0;
    }

    $msg .= "!!! This is broken and has been for some time !!!\n";
    $msg .= "!!! Feel free to fix it if you need to !!!\n";

    # NOTE; address is probably 64 in all cases
    if ( $address == 0 ) { $address = 64; }

    if ( 0 == $version )
    {
    ##########################################
    #  structure  from iram.inc
    ##########################################
    ##
    ## --- Byte aligned storage
    ##
    #V_fcalchg:
    #        .byte   0                       # FC-AL change T/F
    ##
    #K_cputype:
    #        .byte   0                       # Type of CPU code is running on
    #        .set    CPU_RN,0                # 0 = RN
    #        .set    CPU_ZION,1              # 1 = ZION
    #K_ledreg:
    #        .byte   0                       # LED register shadow
    #_ispmap:
    #        .byte   0                       # ISP device bitmap
    #_isp2300:
    #        .byte   0                       # ISP 2300 indicator
    #_rtctr:
    #        .byte   0                       # Qlogic ordinal counter
    #_isprena:
    #        .byte   0                       # Enable ISP response queue processing
    #                                        #  bits - used by interrupt routine
    #_isprqwt:
    #        .byte   0                       # ISP request queue wait flags

    $fmt = sprintf("x%d CCCC CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: V_fcalchg:    0x%02x     \n", $address, $item1);
    $msg .= sprintf("            K_cputype:    0x%02x     \n",$item2);
    $msg .= sprintf("             K_ledreg:    0x%02x     \n",$item3);
    $msg .= sprintf("              _ispmap:    0x%02x     \n",$item4);
    $msg .= sprintf("             _isp2300:    0x%02x     \n",$item5);
    $msg .= sprintf("               _rtctr:    0x%02x     \n",$item6);
    $msg .= sprintf("             _isprena:    0x%02x     \n",$item7);
    $msg .= sprintf("             _isprqwt:    0x%02x     \n",$item8);

    $info{V_FCALCHG}  = $item1;
    $info{K_CPUTYPE}  = $item2;
    $info{K_LEDREG}   = $item3;
    $info{_ISPMAP}    = $item4;
    $info{_ISP2300}   = $item5;
    $info{_RTCTR}     = $item6;
    $info{_ISPRENA}   = $item7;
    $info{_ISPRQWT}   = $item8;

    $offset += 8;                         #   bytes processed
    $address += 8;                  
    }

    if ( 1 == $version )
    {
    ##########################################
    #  structure  from iram.inc
    ##########################################
    #V_fcalchg:
    #        .byte   0                       # FC-AL change T/F
    ##
    #K_cputype:
    #        .byte   0                       # Type of CPU code is running on
    #        .set    CPU_RN,0                # 0 = RN
    #        .set    CPU_ZION,1              # 1 = ZION
    #        .set    CPU_XEON,2              # 2 = i386 XEON processor
    #K_ledreg:
    #        .byte   0                       # LED register shadow
    #_ispmap:
    #        .word   0                       # MAXISP - ISP device bitmap
    #_isp2300:
    #        .word   0                       # MAXISP - ISP 2300 indicator
    #_rtctr:
    #        .byte   0                       # Qlogic ordinal counter
    #_isprena:
    #        .word   0                       # Enable ISP response queue processing
    #                                        # MAXISP - bits - used by interrupt routine
    #_isprqwt:
    #        .word   0                       # MAXISP - ISP request queue wait flags

    $fmt = sprintf("x%d CCCL LCLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: V_fcalchg:    0x%02x     \n", $address, $item1);
    $msg .= sprintf("            K_cputype:    0x%02x     \n",$item2);
    $msg .= sprintf("             K_ledreg:    0x%02x     \n",$item3);
    $msg .= sprintf("              _ispmap:    0x%08x     \n",$item4);
    $msg .= sprintf("             _isp2300:    0x%08x     \n",$item5);
    $msg .= sprintf("               _rtctr:    0x%02x     \n",$item6);
    $msg .= sprintf("             _isprena:    0x%08x     \n",$item7);
    $msg .= sprintf("             _isprqwt:    0x%08x     \n",$item8);

    $info{V_FCALCHG}  = $item1;
    $info{K_CPUTYPE}  = $item2;
    $info{K_LEDREG}   = $item3;
    $info{_ISPMAP}    = $item4;
    $info{_ISP2300}   = $item5;
    $info{_RTCTR}     = $item6;
    $info{_ISPRENA}   = $item7;
    $info{_ISPRQWT}   = $item8;

    $offset += 20;                         #   bytes processed
    $address += 20;                  
    }

    if ( 0 == $version )
    {
    #_ispaywt:
    #        .byte   0                       # ISP async queue stall flags
    #_offlflg:
    #        .byte   0                       # ISP offline flags
    #_ispfflags:
    #        .byte   0                       # Fiber connect flags
    #_fc4flgs:
    #        .byte   0                       # FC-4 registration flags
    ##
    ## --- Word aligned storage
    ##
    ## --- PCI information anchors
    ##
    #K_pcidevs:
    #        .word   0                       # PCI device struct anchor

    $fmt = sprintf("x%d CCCC  L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =
                            unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x:  _ispaywt:    0x%02x     \n", $address, $item1);
    $msg .= sprintf("             _offlflg:    0x%02x     \n",$item2);
    $msg .= sprintf("           _ispfflags:    0x%02x     \n",$item3);
    $msg .= sprintf("             _fc4flgs:    0x%02x     \n",$item4);
    $msg .= sprintf("            K_pcidevs:    0x%08x     \n",$item5);

    $info{_ISPAYWT}    = $item1;
    $info{_OFFLFLG}    = $item2;
    $info{_ISPFFLAGS}  = $item3;
    $info{_FC4FLGS}    = $item4;
    $info{K_PCIDEVS}   = $item5;

    $offset += 8;                         #   bytes processed
    $address += 8;
    }

    if ( 1 == $version )
    {
    #     _ispaywt:
    #        .word   0                       # MAXISP - ISP async queue stall flags
    #/*        .byte   0                       # Spare */
    #_ispfflags:
    #        .word   0                       # MAXISP - Fiber connect flags
    #_fc4flgs:
    #       .word   0                       # MAXISP - FC-4 registration flags
    ##
    ## --- Word aligned storage
    ##
    ## --- PCI information anchors
    ##
    #K_pcidevs:
    #        .word   0                       # PCI device struct anchor

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x:  _ispaywt:    0x%08x     \n", $address, $item1);
    $msg .= sprintf("           _ispfflags:    0x%08x     \n",$item2);
    $msg .= sprintf("             _fc4flgs:    0x%08x     \n",$item3);
    $msg .= sprintf("            K_pcidevs:    0x%08x     \n",$item4);

    $info{_ISPAYWT}    = $item1;
    $info{_ISPFFLAGS}  = $item2;
    $info{_FC4FLGS}    = $item3;
    $info{K_PCIDEVS}   = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;

     }

    #_K_xpcb:
    #K_xpcb:
    #        .word   0                       # Executing PCB
    #K_pcborg:
    #        .word   0                       # Origin of PCB thread
    #K_time:
    #        .word   0                       # Executive time
    #K_tmrexec_pcb:
    #        .word   0                       # Timer exec PCB

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_xpcb:       0x%08x     \n", $address, $item1);
    $msg .= sprintf("          K_pcborg:       0x%08x     \n",$item2);
    $msg .= sprintf("            K_time:       0x%08x     \n",$item3);
    $msg .= sprintf("     K_tmrexec_pcb:       0x%08x     \n",$item4);

    $info{K_XPCB}        = $item1;
    $info{K_PCBORG}      = $item2;
    $info{K_TIME}        = $item3;
    $info{K_TMREXEC_PCB} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #K_dfmexec_pcb:
    #        .word   0                       # Deferred free mem exec

    ##
    ## --- Kernel definitions ----------------------------------------------
    ##
    #K_t1cnt:
    #        .word   0                       # Timer 1 interrupt counter
    #_K_poffset:
    #        .word   0                       # PCI address gen offset - cacheable
    ##
    ## --- Misc definitions ------------------------------------------------
    ##

    $fmt = sprintf("x%d LL L ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_dfmexec_pcb:   0x%08x     \n", $address, $item1);
    $msg .= sprintf("                  K_t1cnt:   0x%08x     \n", $item2);
    $msg .= sprintf("               _K_poffset:   0x%08x     \n", $item3);

    $info{K_DFMEXEC_PCB} = $item1;
    $info{K_T1CNT}       = $item2;
    $info{_K_POFFSET}    = $item3;

    $offset += 12;                         #   bytes processed
    $address += 12;                  

    #m_hbeat_qu:
    #        .space  16,0                    # Heartbeat QCB

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: m_hbeat_qu:   0x%08x 0x%08x 0x%08x 0x%08x     \n",
                                    $address, $item1, $item2, $item3, $item4);

    $info{m_hbeat_qu0} = $item1;
    $info{m_hbeat_qu1} = $item2;
    $info{m_hbeat_qu2} = $item3;
    $info{m_hbeat_qu3} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #M_iltorgc:
    #        .word   0                       # Origin of free ILT list in CDRAM
    #M_iltorgn:
    #        .word   0                       # Origin of free ILT list in NCDRAM
    #M_vrporgc:
    #        .word   0                       # VRP queue base in Cacheable DRAM
    #M_vrporgn:
    #        .word   0                       # VRP queue base in non-Cacheable DRAM
    ##

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: M_iltorgc:    0x%08x     \n", $address, $item1);
    $msg .= sprintf("            M_iltorgn:    0x%08x     \n", $item2);
    $msg .= sprintf("            M_vrporgc:    0x%08x     \n", $item3);
    $msg .= sprintf("            M_vrporgn:    0x%08x     \n", $item4);

    $info{M_ILTORGC} = $item1;
    $info{M_ILTORGN} = $item2;
    $info{M_VRPORGC} = $item3;
    $info{M_VRPORGN} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    ## --- Link Layer definitions ------------------------------------------
    ##
    ##     This is used by the intertupt handler to find the right process
    ##     associated with an inbound message request for a sync state change.
    ##
    #L_orptask0:
    #        .word   0,0                     # PCB for l$exec_orp tasks 0 & 1
    ##
    ##     The next 2 sets of two words must be left in the following order.
    ##     These are shortcuts to find the designated PCBs for the irp
    ##     and completion tasks. The interrupt handlers use these for a quick
    ##     way to enable those tasks.
    ##
    #L_irptask0:
    #        .word   0,0                     # PCB for l$exec_irp tasks 0 & 1

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: L_orptask0:   0x%08x     \n", $address, $item1);
    $msg .= sprintf("            L_orptask1:   0x%08x     \n", $item2);
    $msg .= sprintf("            L_irptask0:   0x%08x     \n", $item3);
    $msg .= sprintf("            L_irptask1:   0x%08x     \n", $item4);

    $info{L_ORPTASK0} = $item1;
    $info{L_ORPTASK1} = $item2;
    $info{L_IRPTASK0} = $item3;
    $info{L_IRPTASK1} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #L_comptask0:
    #        .word   0,0                     # PCB for l$exec_comp tasks 0 & 1
    ##
    ##     Statistics pointer
    ##
    #L_stattbl:
    #        .word   0                       # pointer to the lls structure
    ##

    $fmt = sprintf("x%d LL L ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: L_comptask0:  0x%08x     \n", $address, $item1);
    $msg .= sprintf("            L_comptask1:  0x%08x     \n", $item2);
    $msg .= sprintf("            L_stattbl  :  0x%08x     \n", $item3);

    $info{L_comptask0} = $item1;
    $info{L_comptask1} = $item2;
    $info{L_stattbl}   = $item3;

    $offset += 12;                         #   bytes processed
    $address += 12;                  

    if ( 0 == $version )
    {
        ##     QCS/QB for submitting ILTs to the initiating processors link layer.
        ##     1 QB is needed for each interfacing processor.
        ##     Only 2 of the possible 8 QBs are used and defined to save memory.
        ##
        #LINK_QCS:
        #        .space  qcsiz,0                 # Link Layer QCS  (0x44 bytes long)
        #
        #        .set    qc_flags,0              # Operation flags
        #        .set    qc_stat,qc_flags+1      # Queue status bits
        #        .set    qc_nent,qc_stat+1       # Number of queued entries in all QBs

        $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
        ($item1, $item2, $item3) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("  \n0x%08x: LINK_QCS:     \n", $address);
        $msg .= sprintf("                qc_flags:    0x%02x     \n", $item1);
        $msg .= sprintf("                qc_stat:     0x%02x     \n", $item2);
        $msg .= sprintf("                qc_nent:     0x%04x     \n", $item3);

        $info{QC_FLAGS} = $item1;
        $info{QC_STAT}  = $item2;
        $info{QC_NENT}  = $item3;

        $offset += 4;                         #   bytes processed
        $address += 4;                  

        #        .set    qc_pcb0,qc_nent+2       # PCB of q0 handler task
        #        .set    qc_pcb1,qc_pcb0+4       # PCB of q1 handler task
        #        .set    qc_pcb2,qc_pcb1+4       # PCB of q2 handler task
        #        .set    qc_pcb3,qc_pcb2+4       # PCB of q3 handler task

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                             unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qc_pcb0:     0x%08x     \n", $item1);
        $msg .= sprintf("                qc_pcb1:     0x%08x     \n", $item2);
        $msg .= sprintf("                qc_pcb2:     0x%08x     \n", $item3);
        $msg .= sprintf("                qc_pcb3:     0x%08x     \n", $item4);

        $info{QC_PCB0} = $item1;
        $info{QC_PCB1} = $item2;
        $info{QC_PCB2} = $item3;
        $info{QC_PCB3} = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #        .set    qc_pcb4,qc_pcb3+4       # PCB of q4 handler task
        #        .set    qc_pcb5,qc_pcb4+4       # PCB of q5 handler task
        #        .set    qc_pcb6,qc_pcb5+4       # PCB of q6 handler task
        #        .set    qc_pcb7,qc_pcb6+4       # PCB of q7 handler task

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qc_pcb4:     0x%08x     \n", $item1);
        $msg .= sprintf("                qc_pcb5:     0x%08x     \n", $item2);
        $msg .= sprintf("                qc_pcb6:     0x%08x     \n", $item3);
        $msg .= sprintf("                qc_pcb7:     0x%08x     \n", $item4);

        $info{QC_PCB4} = $item1;
        $info{QC_PCB5} = $item2;
        $info{QC_PCB6} = $item3;
        $info{QC_PCB7} = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #        .set    qc_qb0,qc_pcb7+4        # first QB
        #        .set    qc_qb1,qc_qb0+4         # second QB
        #        .set    qc_qb2,qc_qb1+4         # third QB
        #        .set    qc_qb3,qc_qb2+4         # fourth QB

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qc_qb0:      0x%08x     \n", $item1);
        $msg .= sprintf("                qc_qb1:      0x%08x     \n", $item2);
        $msg .= sprintf("                qc_qb2:      0x%08x     \n", $item3);
        $msg .= sprintf("                qc_qb3:      0x%08x     \n", $item4);

        $info{QC_QB0} = $item1;
        $info{QC_QB1} = $item2;
        $info{QC_QB2} = $item3;
        $info{QC_QB3} = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #        .set    qc_qb4,qc_qb3+4         # fifth QB
        #        .set    qc_qb5,qc_qb4+4         # sixth QB
        #        .set    qc_qb6,qc_qb5+4         # seventh QB
        #        .set    qc_qb7,qc_qb6+4         # eighth QB
        ## --- End structure ---------------------------------------------------
        #
        #        .set    qcsiz,qc_qb7+4          # sizeof QCB

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qc_qb4:      0x%08x     \n", $item1);
        $msg .= sprintf("                qc_qb5:      0x%08x     \n", $item2);
        $msg .= sprintf("                qc_qb6:      0x%08x     \n", $item3);
        $msg .= sprintf("                qc_qb7:      0x%08x     \n", $item4);

        $info{QC_QB4} = $item1;
        $info{QC_QB5} = $item2;
        $info{QC_QB6} = $item3;
        $info{QC_QB7} = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #.llqb0:
        #        .space  qbsiz,0                 # Link queue 0 QB    
        ##
        # --- Begin structure -------------------------------------------------
        #
        #        .set    qb_flags,0              # Status/Operation flags
        #        .set    qb_qpos,qb_flags+1      # QB ordinal
        #        .set    qb_size,qb_qpos+1       # Number of queued ILTs

        $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
        ($item1, $item2, $item3) =  
                           unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("  \n0x%08x: llqb0:     \n", $address);
        $msg .= sprintf("                qb_flags:    0x%02x     \n", $item1);
        $msg .= sprintf("                qb_qpos:     0x%02x     \n", $item2);
        $msg .= sprintf("                qb_size:     0x%04x     \n", $item3);

        $info{QB_FLAGS0} = $item1;
        $info{QB_QPOS0}  = $item2;
        $info{QB_SIZE0}  = $item3;

        $offset += 4;                         #   bytes processed
        $address += 4;                  

        #        .set    qb_lowat,qb_size+2      # Low water mark
        #        .set    qb_max,qb_lowat+2       # Maximum queue entries
        #        .set    qb_pstat,qb_max+2       # Proc stat code for blocked PCBs
        #        .set    qb_qfirst,qb_pstat+4    # First ILT in chain
        #        .set    qb_qlast,qb_qfirst+4    # Last ILT in chain
        #
        # --- End structure ---------------------------------------------------
        #
        #        .set    qbsiz,qb_qlast+4        # sizeof QCB

        $fmt = sprintf("x%d SSL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qb_lowat:    0x%04x     \n", $item1);
        $msg .= sprintf("                qb_max:      0x%04x     \n", $item2);
        $msg .= sprintf("                qb_pstat:    0x%08x     \n", $item3);
        $msg .= sprintf("                qb_qfirst:   0x%08x     \n", $item4);
        $msg .= sprintf("                qb_qlast:    0x%08x     \n", $item5);

        $info{QB_LOWAT0}  = $item1;
        $info{QB_MAX0}    = $item2;
        $info{QB_PSTAT0}  = $item3;
        $info{QB_QFIRST0} = $item4;
        $info{QB_QLAST0}  = $item5;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #.llqb1:
        #        .space  qbsiz,0                 # Link queue 1 QB
        ##
        # --- Begin structure -------------------------------------------------
        #
        #        .set    qb_flags,0              # Status/Operation flags
        #        .set    qb_qpos,qb_flags+1      # QB ordinal
        #        .set    qb_size,qb_qpos+1       # Number of queued ILTs

        $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
        ($item1, $item2, $item3) =  
                           unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("  \n0x%08x: llqb1:     \n", $address);
        $msg .= sprintf("                qb_flags:    0x%02x     \n", $item1);
        $msg .= sprintf("                qb_qpos:     0x%02x     \n", $item2);
        $msg .= sprintf("                qb_size:     0x%04x     \n", $item3);

        $info{QB_FLAGS1} = $item1;
        $info{QB_QPOS1}  = $item2;
        $info{QB_SIZE1}  = $item3;

        $offset += 4;                         #   bytes processed
        $address += 4;                  

        #        .set    qb_lowat,qb_size+2      # Low water mark
        #        .set    qb_max,qb_lowat+2       # Maximum queue entries
        #        .set    qb_pstat,qb_max+2       # Proc stat code for blocked PCBs
        #        .set    qb_qfirst,qb_pstat+4    # First ILT in chain
        #        .set    qb_qlast,qb_qfirst+4    # Last ILT in chain
        #
        # --- End structure ---------------------------------------------------
        #
        #        .set    qbsiz,qb_qlast+4        # sizeof QCB

        $fmt = sprintf("x%d SSL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5) =  
                           unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("                qb_lowat:    0x%04x     \n", $item1);
        $msg .= sprintf("                qb_max:      0x%04x     \n", $item2);
        $msg .= sprintf("                qb_pstat:    0x%08x     \n", $item3);
        $msg .= sprintf("                qb_qfirst:   0x%08x     \n", $item4);
        $msg .= sprintf("                qb_qlast:    0x%08x     \n", $item5);

        $info{QB_LOWAT1}  = $item1;
        $info{QB_MAX1}    = $item2;
        $info{QB_PSTAT1}  = $item3;
        $info{QB_QFIRST1} = $item4;
        $info{QB_QLAST1}  = $item5;

        $offset += 16;                         #   bytes processed
        $address += 16;                  
    }

    ##
    #_qrporg:
    #        .word   0                       # Free QRP thread origin
    ##
    ## --- QLogic ISP information
    ##
    if ( 0 == $version )
    {
        $fmt = sprintf("x%d LL ",$offset);      # generate the format string
        ($item1, $item2) =  
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf("  \n0x%08x: _qrporg:      0x%08x     \n", $address, $item1);
        $msg .= sprintf("            (reserved):   0x%08x     \n", $item2);

        $info{_QRPORG}   = $item1;
        $info{RESERVED2} = $item2;

        $offset += 8;                         #   bytes processed
        $address += 8;                  
    }
    else
    {
        $fmt = sprintf("x%d L ",$offset);      # generate the format string
        ($item1) =  
        unpack $fmt , $$bufferPtr;

        $msg .= sprintf("  \n0x%08x: _qrporg:      0x%08x     \n", $address, $item1);

        $info{_QRPORG}   = $item1;

        $offset += 4;                         #   bytes processed
        $address += 4;                  
    }

    #_ispdefq:
    #        .space  MAXISP*8                # Head/Tail of deferred queue
    #                                        #  for MAXISP chips

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _ispdefq:     \n", $address);
    $msg .= sprintf("                Head          Tail  \n");
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ISPDEFQ0H} = $item1;
    $info{_ISPDEFQ0T} = $item2;
    $info{_ISPDEFQ1H} = $item3;
    $info{_ISPDEFQ1T} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ISPDEFQ2H} = $item1;
    $info{_ISPDEFQ2T} = $item2;
    $info{_ISPDEFQ3H} = $item3;
    $info{_ISPDEFQ3T} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #_rtpcb:
    #        .space  MAXISP*8                # PCBs for ISP handler tasks
    #                                        # +0 = I/O handler
    #                                        # +4 = async event handler

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _rtpcb:     \n", $address);
    $msg .= sprintf("                I/O handler   Async event handler \n");
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_RTPCB0I} = $item1;
    $info{_RTPCB0A} = $item2;
    $info{_RTPCB1I} = $item3;
    $info{_RTPCB1A} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_RTPCB2I} = $item1;
    $info{_RTPCB2A} = $item2;
    $info{_RTPCB3I} = $item3;
    $info{_RTPCB3A} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #_ilthead:
    #        .space  MAXISP*8,0              # Forward/back thread for ILTs
    #                                        #  per ISP instance - head

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _ilthead:     \n", $address);
    $msg .= sprintf("                Head FWD      Head back \n");
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ILTHEAD0F} = $item1;
    $info{_ILTHEAD0B} = $item2;
    $info{_ILTHEAD1F} = $item3;
    $info{_ILTHEAD1B} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ILTHEAD2F} = $item1;
    $info{_ILTHEAD2B} = $item2;
    $info{_ILTHEAD3F} = $item3;
    $info{_ILTHEAD3B} = $item4;

    $offset +=16;                         #   bytes processed
    $address +=16;                  

    #_ilttail:
    #        .space  MAXISP*8,0              # Forward/back thread for ILTs
    #                                        #  per ISP instance - tail

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _ilttail:     \n", $address);
    $msg .= sprintf("                Tail FWD      Tail back \n");
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ILTTAIL0F} = $item1;
    $info{_ILTTAIL0B} = $item2;
    $info{_ILTTAIL1F} = $item3;
    $info{_ILTTAIL1B} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item1, $item2);
    $msg .= sprintf("                0x%08x    0x%08x     \n", $item3, $item4);

    $info{_ILTTAIL2F} = $item1;
    $info{_ILTTAIL2B} = $item2;
    $info{_ILTTAIL3F} = $item3;
    $info{_ILTTAIL3B} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #_ispstr:
    #        .space  MAXISP*4,0              # ISP data structs

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _ispstr:     \n", $address);
    
    $msg .= sprintf("                    0x%08x     \n", $item1);
    $msg .= sprintf("                    0x%08x     \n", $item2);
    $msg .= sprintf("                    0x%08x     \n", $item3);
    $msg .= sprintf("                    0x%08x     \n", $item4);

    $info{_ISPSTR0} = $item1;
    $info{_ISPSTR1} = $item2;
    $info{_ISPSTR2} = $item3;
    $info{_ISPSTR3} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #_isprqptr:
    #        .space  MAXISP*4                # IN pointers for request handling

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _isprqptr:     \n", $address);
    
    $msg .= sprintf("                    0x%08x     \n", $item1);
    $msg .= sprintf("                    0x%08x     \n", $item2);
    $msg .= sprintf("                    0x%08x     \n", $item3);
    $msg .= sprintf("                    0x%08x     \n", $item4);

    $info{_ISPRQPTR0} = $item1;
    $info{_ISPRQPTR1} = $item2;
    $info{_ISPRQPTR2} = $item3;
    $info{_ISPRQPTR3} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #_asyqa:
    #        .space  MAXISP*4,0              # Asynchronous event QCB anchors
    ##

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _asyqa:     \n", $address);
    
    $msg .= sprintf("                    0x%08x     \n", $item1);
    $msg .= sprintf("                    0x%08x     \n", $item2);
    $msg .= sprintf("                    0x%08x     \n", $item3);
    $msg .= sprintf("                    0x%08x     \n", $item4);

    $info{_ASYQA0} = $item1;
    $info{_ASYQA1} = $item2;
    $info{_ASYQA2} = $item3;
    $info{_ASYQA3} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    ## --- Word aligned structures (Do not rearrange these, they are loaded/stored
    ##     as a pair in <isp2x00.as>! )
    ##
    #_qlcmb1:
    #        .short  0                       # QLogic mailbox reg 1 temp storage
    #_qlcmb2:
    #        .short  0                       # QLogic mailbox reg 2 temp storage
    ##
    ## --- QRP statistic counters
    ##
    #ISP_qrpmax:
    #        .short  0                       # Maximum QRPs
    #ISP_qrpcnt:
    #        .short  0                       # Available QRPs
    ##
    ## --- Kernel ----------------------------------------------------------
    #k_dmcount:
    #        .word   0                       # Deferred memory count
    #_K_rrstate:
    #        .word   0                       # State of the round robin scheduling

        $fmt = sprintf("x%d SS SS LL",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, $item6) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf("  \n0x%08x:    _qlcmb1:      0x%04x     \n", $address, $item1);
        $msg .= sprintf("               _qlcmb2:      0x%04x     \n", $item2);
        $msg .= sprintf("            ISP_qrpmax:      0x%04x     \n", $item3);
        $msg .= sprintf("            ISP_qrpcnt:      0x%04x     \n", $item4);
        $msg .= sprintf("            k_dmcount:       0x%08x     \n", $item5);
        $msg .= sprintf("            _K_rrstate:      0x%08x     \n", $item6);

        $info{_QLCMB1}    = $item1;
        $info{_QLCMB2}    = $item2;
        $info{ISP_QRPMAX} = $item3;
        $info{ISP_QRPCNT} = $item4;
        $info{K_DMCOUNT}  = $item5;
        $info{_K_RRSTATE} = $item6;

        $offset += 16;                         #   bytes processed
        $address += 16;

    if ( 0 == $version )
    {
        #_K_rrtimer:
        #        .word   0                       # Timer value where rr will be started
        #k_idle_time:
        #        .word   0                       # Cumulative Idle Time for Proc Util
        #k_last_idle_time:
        #        .word   0                       # The last Idle Time for Proc Util Calc
        #k_last_time_calc:
        #        .word   0                      # Last time Procesor Utilization
        #                                        #  calculations took place
    
        $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                         unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("  \n0x%08x: _K_rrtimer:         0x%08x     \n", $address, $item1);
        $msg .= sprintf("            k_idle_time:        0x%08x     \n", $item2);
        $msg .= sprintf("            k_last_idle_time:   0x%08x     \n", $item3);
        $msg .= sprintf("            k_last_time_calc:   0x%08x     \n", $item4);

        $info{_K_RRTIMER}       = $item1;
        $info{K_IDLE_TIME}      = $item2;
        $info{K_LAST_IDLE_TIME} = $item3;
        $info{K_LAST_TIME_CALC} = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #k_prev_time_calc:
        #        .word   0                      # Previous time Proc Util took place
        #
        #K_dcdram:
        #        .space  8,0                     # Deferred CDRAM

        $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf("  \n0x%08x: k_prev_time_calc:   0x%08x     \n", $address, $item1);
        $msg .= sprintf("            K_dcdram:     0x%08x  0x%08x    \n", $item3, $item4);

        $info{K_PREV_TIME_CALC} = $item1;
        $info{K_DCDRAM0}        = $item3;
        $info{K_DCDRAM1}        = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  
    }
    else  # version == 1
    {
        #_K_rrtimer:
        #        .word   0                       # Timer value where rr will be started
        #K_dcdram:
        #        .space  8,0                     # Deferred CDRAM

        $fmt = sprintf("x%d LL LL",$offset);        # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        
        $msg .= sprintf("  \n0x%08x: _K_rrtimer:      0x%08x     \n", $address, $item1);
        $msg .= sprintf("            (reserved):      0x%08x     \n", $item2);
        $msg .= sprintf("             K_dcdram:    0x%08x  0x%08x    \n", $item3, $item4);

        $info{_K_RRTIMER}       = $item1;
        $info{K_DCDRAM0}        = $item3;
        $info{K_DCDRAM1}        = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  
    }

    #K_disram:
    #        .space  8,0                     # Deferred ISRAM
    #K_dncdram:
    #        .space  8,0                     # Deferred NCDRAM

    $fmt = sprintf("x%d LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_disram:     0x%08x  0x%08x    \n", $address, $item1, $item2);
    $msg .= sprintf("            K_dncdram:    0x%08x  0x%08x    \n", $item3, $item4);

    $info{K_disram0}  = $item1;
    $info{K_disram1}  = $item3;
    $info{K_dncdram0} = $item4;
    $info{K_dncdram1} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #K_cdram:
    #        .space  fmsiz,0                 # Cacheable memory FMM
    #
    # --- Begin structure -------------------------------------------------
    #                                                                  *****
    #        .set    fm_org,0                # Origin of free memory thd <w>
    #        .set    fm_s0len,fm_org+4       # Segment 0 length          <w>
    #        .set    fm_fms,fm_s0len+4       # Address of FMS (within II)<w>
    #        .set    fm_sorg,fm_fms+4        # Secondary free memory thd <w>
    #                                                                  *****
    #        .set    fm_waitstat,fm_sorg+4   # Memory wait status        <b>
    #        .set    fm_options,fm_waitstat+1 # Memory Options           <b>
    #
    #       reserved 2
    #
    # --- End structure ---------------------------------------------------
    #
    #        .set    fmsiz,fm_options+3      # Size of structure
    #

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_cdram:     \n", $address);
    $msg .= sprintf("                fm_org:      0x%08x     \n", $item1);
    $msg .= sprintf("                fm_s0len:    0x%08x     \n", $item2);
    $msg .= sprintf("                fm_fms:      0x%08x     \n", $item3);
    $msg .= sprintf("                fm_sorg:     0x%08x     \n", $item4);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;
 #   $info{QB_SIZE1}  = $item3;
 #   $info{QB_SIZE1}  = $item3;


    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                fm_waitstat: 0x%02x     \n", $item1);
    $msg .= sprintf("                fm_options:  0x%02x     \n", $item2);
    $msg .= sprintf("                (reserved):  0x%04x     \n", $item3);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;


    $offset += 4;                         #   bytes processed
    $address += 4;                  

    #K_ncdram:
    #        .space  fmsiz,0                 # Non cacheable memory FMM

    # --- Begin structure -------------------------------------------------
    #                                                                  *****
    #        .set    fm_org,0                # Origin of free memory thd <w>
    #        .set    fm_s0len,fm_org+4       # Segment 0 length          <w>
    #        .set    fm_fms,fm_s0len+4       # Address of FMS (within II)<w>
    #        .set    fm_sorg,fm_fms+4        # Secondary free memory thd <w>
    #                                                                  *****
    #        .set    fm_waitstat,fm_sorg+4   # Memory wait status        <b>
    #        .set    fm_options,fm_waitstat+1 # Memory Options           <b>
    #
    #       reserved 2
    #
    # --- End structure ---------------------------------------------------
    #
    #        .set    fmsiz,fm_options+3      # Size of structure
    #

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_ncdram:     \n", $address);
    $msg .= sprintf("                fm_org:      0x%08x     \n", $item1);
    $msg .= sprintf("                fm_s0len:    0x%08x     \n", $item2);
    $msg .= sprintf("                fm_fms:      0x%08x     \n", $item3);
    $msg .= sprintf("                fm_sorg:     0x%08x     \n", $item4);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;
 #   $info{QB_SIZE1}  = $item3;
 #   $info{QB_SIZE1}  = $item3;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                fm_waitstat: 0x%02x     \n", $item1);
    $msg .= sprintf("                fm_options:  0x%02x     \n", $item2);
    $msg .= sprintf("                (reserved):  0x%04x     \n", $item3);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;

    $offset += 4;                         #   bytes processed
    $address += 4;                  

    #K_isram:
    #        .space  fmsiz,0                 # Internal SRAM FMM
    #
    # --- Begin structure -------------------------------------------------
    #                                                                  *****
    #        .set    fm_org,0                # Origin of free memory thd <w>
    #        .set    fm_s0len,fm_org+4       # Segment 0 length          <w>
    #        .set    fm_fms,fm_s0len+4       # Address of FMS (within II)<w>
    #        .set    fm_sorg,fm_fms+4        # Secondary free memory thd <w>
    #                                                                  *****
    #        .set    fm_waitstat,fm_sorg+4   # Memory wait status        <b>
    #        .set    fm_options,fm_waitstat+1 # Memory Options           <b>
    #
    #       reserved 2
    #
    # --- End structure ---------------------------------------------------
    #
    #        .set    fmsiz,fm_options+3      # Size of structure
    #

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_isram:     \n", $address);
    $msg .= sprintf("                fm_org:      0x%08x     \n", $item1);
    $msg .= sprintf("                fm_s0len:    0x%08x     \n", $item2);
    $msg .= sprintf("                fm_fms:      0x%08x     \n", $item3);
    $msg .= sprintf("                fm_sorg:     0x%08x     \n", $item4);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;
 #   $info{QB_SIZE1}  = $item3;
 #   $info{QB_SIZE1}  = $item3;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d CC S ",$offset);      # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                fm_waitstat: 0x%02x     \n", $item1);
    $msg .= sprintf("                fm_options:  0x%02x     \n", $item2);
    $msg .= sprintf("                (reserved):  0x%04x     \n", $item3);

 #   $info{QB_FLAGS1} = $item1;
 #   $info{QB_QPOS1}  = $item2;

    $offset += 4;                         #   bytes processed
    $address += 4;                  

    $offset += 4;                         #   bytes processed
    $address += 4;                  

    #K_dma0cb:
    #        .space  dmacbsiz,0              # DMA 0 control block
    # --- Structure for DMA control block -----------------------------------------
    #
    #        .set    dma_begin,0             # Begin of DMA control queue
    #        .set    dma_in,dma_begin+4      # IN pointer of DMA control queue
    #        .set    dma_out,dma_in+4        # OUT pointer of DMA control queue
    #        .set    dma_end,dma_out+4       # Limit of DMA control queue
    #        .set    dma_free,dma_end+4      # first available DMA descriptor
    #
    # --- end of structure
    #
    #        .set    dmacbsiz,dma_free+4     # sizeof dmacb
    #

    $fmt = sprintf("x%d LL LL L",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_dma0cb:     \n", $address);
    $msg .= sprintf("                dma_begin:   0x%08x     \n", $item1);
    $msg .= sprintf("                dma_in:      0x%08x     \n", $item2);
    $msg .= sprintf("                dma_out:     0x%08x     \n", $item3);
    $msg .= sprintf("                dma_end:     0x%08x     \n", $item4);
    $msg .= sprintf("                dma_free:    0x%08x     \n", $item5);

 #  $info{QB_FLAGS1} = $item1;
 #  $info{QB_QPOS1}  = $item2;
 #  $info{QB_SIZE1}  = $item3;
 #  $info{QB_SIZE1}  = $item3;

    $offset += 20;                         #   bytes processed
    $address += 20;                  

 #  $info{QB_FLAGS1} = $item1;
 #  $info{QB_QPOS1}  = $item2;

    $offset += 12;                         #   bytes processed
    $address += 12;                        #  there is some padding in this one

    #K_dma1cb:
    #        .space  dmacbsiz,0              # DMA 1 control block
    ##

    $fmt = sprintf("x%d LL LL L",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: K_dma1cb:     \n", $address);
    $msg .= sprintf("                dma_begin:   0x%08x     \n", $item1);
    $msg .= sprintf("                dma_in:      0x%08x     \n", $item2);
    $msg .= sprintf("                dma_out:     0x%08x     \n", $item3);
    $msg .= sprintf("                dma_end:     0x%08x     \n", $item4);
    $msg .= sprintf("                dma_free:    0x%08x     \n", $item5);

 #  $info{QB_FLAGS1} = $item1;
 #  $info{QB_QPOS1}  = $item2;
 #  $info{QB_SIZE1}  = $item3;
 #  $info{QB_SIZE1}  = $item3;

    $offset += 20;                         #   bytes processed
    $address += 20;                  

 #  $info{QB_FLAGS1} = $item1;
 #  $info{QB_QPOS1}  = $item2;

    # Now we have to determine if the rest is from FE or BE processor

#        .set    DMACBOFF,K_dma1cb-K_dma0cb # offset from CB 0 to CB 1
##
## -- pull in the processor specific internal ram definitions
##
#.ifdef FRONTEND
#        .include    "iramfe.inc"
#.else  # BACKEND
#        .include    "irambe.inc"

    my $remaining = length($$bufferPtr) - $offset;

    $msg .= "\nBytes left available to process = $remaining \n\n";

    if ( 
           ($remaining < 0x158) ||           # FE size ( is larger of two )
           (index(uc($processor),"BE") > -1)  # $processor contains "BE" 
       )
    {
        # it is too small to be the FE or contains BE in name
        if ($remaining >= 0xCC )
        {
            # enought bytes for BE
            FmtIRAMBEPart ( \$msg, \%info, $bufferPtr, $offset, 0, $processor, $address, $version);
        }
        else
        {
            # not enough here to decode !
            $msg .= "\n############################################################## \n";
            $msg .= " Insufficient data for processor specific part, skipping . . . \n";
            $msg .= "############################################################## \n";
        }
    }
    elsif (
            index(uc($processor),"FE") > -1  # $processor contains "FE" 
          )
    {
        # not too small and has FE in name
        FmtIRAMFEPart ( \$msg, \%info, $bufferPtr, $offset, 0, $processor, $address, $version);
    }
    else
    {
        # big enough for either, but no processor tag, do both
        $msg .= "\n########################################################## \n";
        $msg .= " Unable to determine processor type, evaluating both . . . \n";
        $msg .= "########################################################## \n\n";

        FmtIRAMFEPart ( \$msg, \%info, $bufferPtr, $offset, 0, $processor, $address, $version);
        FmtIRAMBEPart ( \$msg, \%info, $bufferPtr, $offset, 0, $processor, $address, $version);
    } 

#.endif
##
#.iramend:
#        .byte   0                       # End of SRAM

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

###############################################################################
sub FmtIRAMBEPart
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $version) = @_;
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
    my $msg = "\nBack End processor specific portion:\n\n";
    my $str;
    my %info;

    if ( !defined($version) )
    {
        $version = 0;
    }

    # NOTE; address is probably x284 in all cases
    if ( $address == 0 ) { $address = 0x00000284; }

    ##########################################
    #  structure  from iramBE.inc
    ##########################################
    ##
    ## --- misc definitions -------------------------------------------
    ##
    #M_prporg:
    #        .word   0                       # Origin of free PRP list
    #M_rrporg:
    #        .word   0                       # Origin of free RRP list
    #M_scborg:
    #        .word   0                       # Origin of free SCB list
    #M_rpnorg:
    #        .word   0                       # Origin of free RPN list
    #M_rrborg:
    #        .word   0                       # Origin of free RRB list

    $fmt = sprintf("x%d LL LL L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: M_prporg:     0x%08x     \n", $address, $item1);
    $msg .= sprintf("            M_rrporg:     0x%08x     \n", $item2);
    $msg .= sprintf("            M_scborg:     0x%08x     \n", $item3);
    $msg .= sprintf("            M_rpnorg:     0x%08x     \n", $item4);
    $msg .= sprintf("            M_rrborg:     0x%08x     \n\n", $item5);

    $info{M_PRPORG}  = $item1;
    $info{M_RRPORG}  = $item2;
    $info{M_SCBORG}  = $item3;
    $info{M_RPNORG}  = $item4;
    $info{M_RRBORG}  = $item5;

    $offset += 20;                         #   bytes processed
    $address += 20;                  

    ##
    ## --- physical definitions ---------------------------------------
    ##
    #P_chn_ind:
    #_P_chn_ind:
    #        .space  MAXCHN*4,0              # CHN lookup table (pointers)

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: P_chn_ind:    0x%08x  0x%08x  0x%08x  0x%08x    \n\n",
                                $address, $item1, $item2, $item3, $item4);

    $info{P_CHN_IND0}  = $item1;
    $info{P_CHN_IND1}  = $item2;
    $info{P_CHN_IND2}  = $item3;
    $info{P_CHN_IND3}  = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    if ( 1 == $version )
    {
        #P_orc:
        #        .word   0                       # Outstanding req count

        $fmt = sprintf("x%d L ",$offset);      # generate the format string
        ($item1) =  
               unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf(    "0x%08x: P_orc:        0x%08x     \n\n", $address, $item1);

        $info{P_ORC}     = $item1;

        $offset += 4;                         #   bytes processed
        $address += 4;                  
    }
    else
    {
        #P_primask:
        #        .word   0                       # Priority mask constant
        #P_imsk:
        #        .word   0                       # IMSK address
        #P_intmask:
        #        .word   0                       # Interrupt mask
        #P_orc:
        #        .word   0                       # Outstanding req count

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4 ) =  
                            unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf(    "0x%08x: P_primask:    0x%08x     \n", $address, $item1);
        $msg .= sprintf("            P_imsk:       0x%08x     \n", $item2);
        $msg .= sprintf("            P_intmask:    0x%08x     \n", $item3);
        $msg .= sprintf("            P_orc:        0x%08x     \n", $item4);

        $info{P_PRIMASK} = $item1;
        $info{P_IMSK}    = $item2;
        $info{P_INTMASK} = $item3;
        $info{P_ORC}     = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  
    }

    #P_que:
    #        .word   0                       # P$que addr
    #P_ageadj:
    #        .word   0                       # Age adjustment
    ##
    #R_orc:
    #        .word   0                       # RAID Outstanding request count
    #V_orc:
    #        .word   0                       # Virtual outstanding req count

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: P_que:        0x%08x     \n", $address, $item1);
    $msg .= sprintf("            P_ageadj:     0x%08x     \n", $item2);
    $msg .= sprintf("            R_orc:        0x%08x     \n", $item3);
    $msg .= sprintf("            V_orc:        0x%08x     \n", $item4);

    $info{P_QUE}    = $item1;
    $info{P_AGEADJ} = $item2;
    $info{R_ORC}    = $item3;
    $info{V_ORC}    = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    if ( 1 == $version )
    {
        $offset += 4;                         #   bytes processed
        $address += 4;
    }
    else
    {
        $offset += 8;                         #   bytes processed
        $address += 8;                  
    }

    #P_exec_qu:
    #        .space  16,0                    # Executive QCB
    #    UINT32 * begin;                 /* Org of circular queue ptr            */
    #    UINT32 * in;                    /* Insert pointer                       */
    #    UINT32 * out;                   /* Remove pointer                       */
    #    UINT32 * end;                   /* End of circular que + 1              */

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: P_exec_qu:     \n", $address);
    $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
    $msg .= sprintf("                * in:        0x%08x     \n", $item2);
    $msg .= sprintf("                * out:       0x%08x     \n", $item3);
    $msg .= sprintf("                * end:       0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #P_comp_qu:
    #        .space  16,0                    # Completion QCB

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: P_comp_qu:     \n", $address);
    $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
    $msg .= sprintf("                * in:        0x%08x     \n", $item2);
    $msg .= sprintf("                * out:       0x%08x     \n", $item3);
    $msg .= sprintf("                * end:       0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #P_regsave:
    #        .space  16,0                    # Register save area
    #                                        #  for interrupt routines

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "\n0x%08x: P_regsave:    0x%08x  0x%08x  0x%08x  0x%08x    \n",
                                $address, $item1, $item2, $item3, $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    ##
    ## --- RAID definitions
    ##
    #R_exec_qu:
    #        .space  16,0                    # Executive QCB

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: R_exec_qu:     \n", $address);
    $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
    $msg .= sprintf("                * in:        0x%08x     \n", $item2);
    $msg .= sprintf("                * out:       0x%08x     \n", $item3);
    $msg .= sprintf("                * end:       0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #R_r5exec_qu:
    #        .space  16,0                    # RAID 5 executive QCB

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: R_r5exec_qu:     \n", $address);
    $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
    $msg .= sprintf("                * in:        0x%08x     \n", $item2);
    $msg .= sprintf("                * out:       0x%08x     \n", $item3);
    $msg .= sprintf("                * end:       0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #R_scsum:
    #        .word   0                       # Shadowed NVA checksum
    #R_nvac:                                 # NVAC structure (alias)
    #R_nc_nvarec:
    #        .word   0                       # Start of NVA records
    #R_nc_csum:
    #        .word   0                       # Address of NVA checksum
    #R_nc_mapbase:
    #        .word   0                       # Base of NVA map
    #R_nc_mapptr:
    #        .word   0                       # Base of NVA map

    $fmt = sprintf("x%d LL LL L ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "\n0x%08x: R_scsum:      0x%08x     \n", $address, $item1);
    $msg .= sprintf("            R_nc_nvarec:  0x%08x     \n", $item2);
    $msg .= sprintf("            R_nc_csum:    0x%08x     \n", $item3);
    $msg .= sprintf("            R_nc_mapbase: 0x%08x     \n", $item4);
    $msg .= sprintf("            R_nc_mapptr:  0x%08x     \n", $item5);

    $info{R_scsum}  = $item1;
    $info{R_nc_nvarec}  = $item2;
    $info{R_nc_csum}  = $item3;
    $info{R_nc_mapbase}  = $item4;
    $info{R_nc_mapptr}  = $item5;

    $offset += 20;                         #   bytes processed
    $address += 20;                  

    ##
    ## --- virtual definitions ---------------------------------------- *****
    ##
    $offset += 12;                         #   bytes processed
    $address += 12;                  

    #V_exec_qu:
    #        .space  16,0                    # Executive QCB
    ##
    #
    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: V_exec_qu:     \n", $address);
    $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
    $msg .= sprintf("                * in:        0x%08x     \n", $item2);
    $msg .= sprintf("                * out:       0x%08x     \n", $item3);
    $msg .= sprintf("                * end:       0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    if ( 1 == $version )
    {
        #_V_exec_mqu:
        #V_exec_mqu:
        #        .space  16,0                    # Executive medium priority QCB

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        
        $msg .= sprintf("  \n0x%08x: V_exec_mqu:     \n", $address);
        $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
        $msg .= sprintf("                * in:        0x%08x     \n", $item2);
        $msg .= sprintf("                * out:       0x%08x     \n", $item3);
        $msg .= sprintf("                * end:       0x%08x     \n", $item4);

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #
        #_V_exec_hqu:
        #V_exec_hqu:
        #        .space  16,0                    # Executive High priority QCB

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =  
                            unpack $fmt , $$bufferPtr;
        
        $msg .= sprintf("  \n0x%08x: V_exec_hqu:     \n", $address);
        $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
        $msg .= sprintf("                * in:        0x%08x     \n", $item2);
        $msg .= sprintf("                * out:       0x%08x     \n", $item3);
        $msg .= sprintf("                * end:       0x%08x     \n", $item4);

        $offset += 16;                         #   bytes processed
        $address += 16;                  

        #V_exec_xqu:
        #      .space 16,0                  # Expedited Executive QCB
        ##
        #
        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf("  \n0x%08x: V_exec_xqu:     \n", $address);
        $msg .= sprintf("                * begin:     0x%08x     \n", $item1);
        $msg .= sprintf("                * in:        0x%08x     \n", $item2);
        $msg .= sprintf("                * out:       0x%08x     \n", $item3);
        $msg .= sprintf("                * end:       0x%08x     \n", $item4);

        $offset += 16;                         #   bytes processed
        $address += 16;
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

###############################################################################
sub FmtIRAMFEPart
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $version) = @_;
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
    my $msg = "\nFront End processor specific portion:\n\n";
    my $str;
    my %info;

    if ( !defined($version) )
    {
        $version = 0;
    }

    # NOTE; address is probably x284 in all cases
    if ( $address == 0 ) { $address = 0x00000284; }

    ##########################################
    #  structure  from iramFE.inc
    ##########################################
# --- Byte aligned storage
#
#C_exec_pcb:
#        .word   0                       # Executive PCB
#C_ioexec_pcb:
#        .word   0                       # I/O Executive PCB
##
## --- Front End definitions ------------------------------------- *****
##
##
## --- Non-cached Write Buffer Proxy FMM and FMS
##
#c_ncwbp_addr:
#        .word   0                       # Address - Write Buffer Proxy space

    $fmt = sprintf("x%d LL L ",$offset);      # generate the format string
    ($item1, $item2, $item3 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: C_exec_pcb:   0x%08x     \n", $address, $item1);
    $msg .= sprintf("            C_ioexec_pcb: 0x%08x     \n",$item2);
    $msg .= sprintf("            c_ncwbp_addr: 0x%08x     \n",$item3);

    $info{C_EXEC_PCB}   = $item1;
    $info{C_IOEXEC_PCB} = $item2;
    $info{C_NCWBP_ADDR} = $item3;

    $offset += 12;                         #   bytes processed
    $address += 12;                  
#
#c_ncwbp_fms:
#        .space  fssiz,0                 # Proxy FMS
# --- Begin structure -------------------------------------------------
#                                                                  *****
#        .set    fs_cur,0                # Available memory          <w>
#        .set    fs_max,fs_cur+4         # Maximum memory            <w>
#        .set    fs_min,fs_max+4         # Minimum memory            <w>
#        .set    fs_wait,fs_min+4        # Process waits             <w>
#                                                                  *****
# --- End structure ---------------------------------------------------

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: c_ncwbp_fms:     \n", $address);
    $msg .= sprintf("                fs_cur:      0x%08x     \n", $item1);
    $msg .= sprintf("                fs_max:      0x%08x     \n", $item2);
    $msg .= sprintf("                fs_min:      0x%08x     \n", $item3);
    $msg .= sprintf("                fs_wait:     0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#c_ncwbp_fmm:
#        .space  fmsiz,0                 # Proxy FMM
# --- Begin structure -------------------------------------------------
#                                                                  *****
#        .set    fm_org,0                # Origin of free memory thd <w>
#        .set    fm_s0len,fm_org+4       # Segment 0 length          <w>
#        .set    fm_fms,fm_s0len+4       # Address of FMS (within II)<w>
#        .set    fm_sorg,fm_fms+4        # Secondary free memory thd <w>
#                                                                  *****
#        .set    fm_waitstat,fm_sorg+4   # Memory wait status        <b>
#        .set    fm_options,fm_waitstat+1 # Memory Options           <b>
#
#       reserved 2
#
# --- End structure ---------------------------------------------------

    $fmt = sprintf("x%d LL LL CCCC",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: c_ncwbp_fmm:     \n", $address);
    $msg .= sprintf("                fm_org:      0x%08x     \n", $item1);
    $msg .= sprintf("                fm_s0len:    0x%08x     \n", $item2);
    $msg .= sprintf("                fm_fms:      0x%08x     \n", $item3);
    $msg .= sprintf("                fm_sorg:     0x%08x     \n", $item4);
    $msg .= sprintf("                fm_waitstat: 0x%02x     \n", $item5);
    $msg .= sprintf("                fm_options:  0x%02x     \n", $item6);
    $msg .= sprintf("                (reserved):  0x%02x     \n", $item7);
    $msg .= sprintf("                (reserved):  0x%02x     \n\n", $item8);

    $offset += 24;                         #   bytes processed
    $address += 24;                        # includes alignment pad
#
#C_exec_qht:
#        .space  8,0                     # Queue head/tail

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: C_exec_qht:   0x%08x  0x%08x   \n", 
                          $address, $item1, $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  

#C_ioexec_qht:
#        .space  8,0                     # I/O queue head/tail
##

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "\n0x%08x: C_ioexec_qht: 0x%08x  0x%08x   \n", 
                           $address, $item1, $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  

## --- Note: the forward pointer (word 0) of <c_hlruq> must be initialized to
##           point to <c_tlruq>; word 1 must be zero.
##
##           Backward pointer (word 1) of <c_tlruq> must be initialized to
##           point to <c_hlruq>; word 0 must be zero.
##
##           This initialization must be done before any items are linked
##           to the LRU queue!
##
#c_hlruq:
#        .space  8,0                     # Cache tag LRU queue - head

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "\n0x%08x: c_hlruq:      0x%08x  0x%08x   \n", 
                           $address, $item1, $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  

#c_tlruq:
#        .space  8,0                     # Cache tag LRU queue - tail


    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "\n0x%08x: c_tlruq:      0x%08x  0x%08x   \n", 
                           $address, $item1, $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  

#_C_ca:
#        .space  casiz,0                 # Cache Statistics
# --- Begin basic structure -------------------------------------------
#
# !!! NOTE: IF THIS STRUCTURE CHANGES, CCB mrp.h MUST ALSO CHAGE !!!!!!!!!
#                                                                  *****
#        .set    ca_status,0             # Cache status              <b>
#
#       reserved 1
#
#        .set    ca_battery,ca_status+2  # Battery status            <b>
#        .set    ca_stopcnt,ca_battery+1 # Stop I/O Count            <b>
#        .set    ca_size,ca_stopcnt+1    # Cache size                <w>
#        .set    ca_maxcwr,ca_size+4     # Maximum cached write      <w>
#        .set    ca_maxsgl,ca_maxcwr+4   # Mazimum num of SGLs/op    <w>
#                                                                  *****
#        .set    ca_numTags,ca_maxsgl+4  # Total number of tags      <w>
#        .set    ca_tagsDirty,ca_numTags+4 # Current num of tags dirty <w>
#        .set    ca_tagsResident,ca_tagsDirty+4 # Current # of tags resident <w>
#        .set    ca_tagsFree,ca_tagsResident+4 # Current num of tags free <w>
#                                                                  *****
#        .set    ca_tagsFlushIP,ca_tagsFree+4 # Current # tags flush in progr <w>
#        .set    ca_numBlks,ca_tagsFlushIP+4 # Total number of blocks   <w>
#        .set    ca_blocksDirty,ca_numBlks+4 # Current num of dirty blocks <w>
#        .set    ca_blocksResident,ca_blocksDirty+4 # Current # of blocks res <w>
#                                                                  *****
#        .set    ca_blocksFree,ca_blocksResident+4 # Current # of blocks free <w>
#        .set    ca_blocksFlushIP,ca_blocksFree+4 # Num blocks flush in prog <w>
#
#       reserved 8
#
#                                                                  *****
#
# --- End basic structure ---------------------------------------------

    $fmt = sprintf("x%d CCCC L LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: _C_ca:     \n", $address);
    $msg .= sprintf("                ca_status:          0x%02x     \n", $item1);
    $msg .= sprintf("                (reserved):         0x%02x     \n", $item2);
    $msg .= sprintf("                ca_battery:         0x%02x     \n", $item3);
    $msg .= sprintf("                ca_stopcnt:         0x%02x     \n", $item4);
    $msg .= sprintf("                ca_size:            0x%08x     \n", $item5);
    $msg .= sprintf("                ca_maxcwr:          0x%08x     \n", $item6);
    $msg .= sprintf("                ca_maxsgl:          0x%08x     \n", $item7);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                ca_numTags:         0x%08x     \n", $item1);
    $msg .= sprintf("                ca_tagsDirty:       0x%08x     \n", $item2);
    $msg .= sprintf("                ca_tagsResident:    0x%08x     \n", $item3);
    $msg .= sprintf("                ca_tagsFree:        0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d LL LL LL",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                ca_tagsFlushIP:     0x%08x     \n", $item1);
    $msg .= sprintf("                ca_numBlks:         0x%08x     \n", $item2);
    $msg .= sprintf("                ca_blocksDirty:     0x%08x     \n", $item3);
    $msg .= sprintf("                ca_blocksResident:  0x%08x     \n", $item4);
    $msg .= sprintf("                ca_blocksFree:      0x%08x     \n", $item5);
    $msg .= sprintf("                ca_blocksFlushIP:   0x%08x     \n", $item6);

    $offset += 24;                         #   bytes processed
    $address += 24;
                      
    $fmt = sprintf("x%d LL",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("                (reserved):         0x%08x     \n", $item1);
    $msg .= sprintf("                (reserved):         0x%08x     \n\n", $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  

#C_wbbaseaddr:
#        .word   0                       # Write buffer base address for BE i960
#C_wbsize:
#        .word   0                       # Write buffer size in bytes

    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2, $item3 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: C_wbbaseaddr: 0x%08x     \n", $address, $item1);
    $msg .= sprintf("            C_wbsize:     0x%08x     \n",$item2);

    $info{C_WBBASEADDR}   = $item1;
    $info{C_WBSIZE} = $item2;

    $offset += 8;                         #   bytes processed
    $address += 8;                  

#c_wflushq:
#        .space  cqsize,0                # Flush exec queue
# --- Begin structure ---------------------------------------------------------
#
#        .set    chead,0                 # Head of ILT/Placeholder queue <w>
#        .set    ctail,chead+4           # Tail of ILT/Placeholder queue <w>
#        .set    cpcb,ctail+4            # Handler task PCB              <w>
#
#   Reserved 4
#
#
# --- End structure -----------------------------------------------------------

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: c_wflushq:     \n", $address);
    $msg .= sprintf("                chead:       0x%08x     \n", $item1);
    $msg .= sprintf("                ctail:       0x%08x     \n", $item2);
    $msg .= sprintf("                cpcb:        0x%08x     \n", $item3);
    $msg .= sprintf("                (reserved):  0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#c_mirrorq:
#        .space  cqsize,0                # Mirror exec queue control block

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: gMirrorQueue:     \n", $address);
    $msg .= sprintf("                chead:       0x%08x     \n", $item1);
    $msg .= sprintf("                ctail:       0x%08x     \n", $item2);
    $msg .= sprintf("                cpcb:        0x%08x     \n", $item3);
    $msg .= sprintf("                (reserved):  0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#_C_CIMT_dir:
#        .space  CIMTMAX*4,0             # CIMT directory area
##
## --- Word aligned storage
##

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: cimtDir:     \n", $address);
    $msg .= sprintf("                             0x%08x     \n", $item1);
    $msg .= sprintf("                             0x%08x     \n", $item2);
    $msg .= sprintf("                             0x%08x     \n", $item3);
    $msg .= sprintf("                             0x%08x     \n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    if ( 1 == $version )
    {
        #C_ctv:
        #     .word 0                          # Controller Throttle Value
        ##
        #
        $fmt = sprintf("x%d L ",$offset);          # generate the format string
        ($item1) =
                unpack $fmt , $$bufferPtr;

        $msg .= sprintf("  \n0x%08x: C_ctv:     \n", $address);
        $msg .= sprintf("                          0x%08x     \n\n", $item1);

        $offset += 4;                         #   bytes processed
        $address += 4;
    }

#C_orc:
#        .word   0                       # Cache outstanding host request count
#C_flush_orc:
#        .word   0                       # Cache oustanding flush request count
#C_exec_qcd:
#        .word   0                       # Cache current queue depth
#C_owsrpc:
#        .word   0                       # Cache outstanding write SRP count

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: C_orc:        0x%08x     \n", $address, $item1);
    $msg .= sprintf("            C_flush_orc:  0x%08x     \n",$item2);
    $msg .= sprintf("            C_exec_qcd:   0x%08x     \n",$item3);
    $msg .= sprintf("            C_owsrpc:     0x%08x     \n\n",$item4);

    $info{C_ORC}       = $item1;
    $info{C_FLUSH_ORC} = $item2;
    $info{C_EXEC_QCD}  = $item3;
    $info{C_OWSRPC}    = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#C_orsrpc:
#        .word   0                       # Cache outstanding read SRP count
#c_plfreec:
#        .word   0                       # Free placeholder list anchor - CDRAM
#c_plfreen:
#        .word   0                       # Free placeholder list anchor - NCDRAM
#c_pltotal:
#        .word   0                       # Total number of Placeholders

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: C_orsrpc:     0x%08x     \n", $address, $item1);
    $msg .= sprintf("            c_plfreec:    0x%08x     \n",$item2);
    $msg .= sprintf("            c_plfreen:    0x%08x     \n",$item3);
    $msg .= sprintf("            c_pltotal:    0x%08x     \n\n",$item4);

    $info{C_ORSRPC}    = $item1;
    $info{C_PLFREEC}   = $item2;
    $info{C_PLFREEN}   = $item3;
    $info{C_PLTOTAL}   = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#c_plavail:
#        .word   0                       # Number of Placeholders available
#c_rbfreec:
#        .word   0                       # Free RB node list anchor - CDRAM
#c_rbfreen:
#        .word   0                       # Free RB node list anchor - NCDRAM
#c_rbtotal:
#        .word   0                       # Total number of RB Nodes

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: c_plavail:    0x%08x     \n", $address, $item1);
    $msg .= sprintf("            c_rbfreec:    0x%08x     \n",$item2);
    $msg .= sprintf("            c_rbfreen:    0x%08x     \n",$item3);
    $msg .= sprintf("            c_rbtotal:    0x%08x     \n\n",$item4);

    $info{C_PLAVAIL} = $item1;
    $info{C_RBFREEC} = $item2;
    $info{C_RBFREEN} = $item3;
    $info{C_RBTOTAL} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#c_rbavail:
#        .word   0                       # Number of RB Nodes available
#c_rbifreec:
#        .word   0                       # Free RBI node list anchor - CDRAM
#c_rbifreen:
#        .word   0                       # Free RBI node list anchor - NCDRAM
#c_rbitotal:
#        .word   0                       # Total number of RBI Nodes

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: c_rbavail:    0x%08x     \n", $address, $item1);
    $msg .= sprintf("            c_rbifreec:   0x%08x     \n",$item2);
    $msg .= sprintf("            c_rbifreen:   0x%08x     \n",$item3);
    $msg .= sprintf("            c_rbitotal:   0x%08x     \n\n",$item4);

    $info{C_RBAVAIL}  = $item1;
    $info{C_RBIFREEC} = $item2;
    $info{C_RBIFREEN} = $item3;
    $info{C_RBITOTAL} = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#c_rbiavail:
#        .word   0                       # Number of RBI Nodes available
#_c_tgfree:
#        .word   0                       # Free cache tag list
#c_wcresourc:
#        .word   0                       # T/F, Stalled due to WC Resources
#C_exec_cqd:
#        .space  4,0                     # Current queue depth

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: c_rbiavail:   0x%08x     \n", $address, $item1);
    $msg .= sprintf("            _c_tgfree:    0x%08x     \n",$item2);
    $msg .= sprintf("            c_wcresourc:  0x%08x     \n",$item3);
    $msg .= sprintf("            C_exec_cqd:   0x%08x     \n\n",$item4);

    $info{C_RBIAVAIL}  = $item1;
    $info{_C_TGFREE}   = $item2;
    $info{C_WCRESOURC} = $item3;
    $info{C_EXEC_CQD}  = $item4;

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    if ( 1 == $version )
    {
        #C_ioexec_cqd:
        #        .space  4,0                     # I/O current queue depth
        #c_bgflush:
        #        .space  4,0                     # PCB pointer for BG Flush Task
        #c_allowVLinkOps:
        #        .word   0                       # Flag to determine if VLink Ops are
        #                                            allowed to go even though C$Stop is
        #                                            active
        #                                          FALSE = VLink Ops treated Normal
        #                                          TRUE  = VLink Ops allowd through
        ##
        ## --- Registered IMT list
        ##       IMTs are placed on this list when not active.
        ##
        #C_imt_head:
        #        .word   0                       # Registered IMT list head pointer
        #C_imt_tail:
        #        .word   0                       # Registered IMT list tail pointer

        $fmt = sprintf("x%d LL LL L ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5 ) =
                            unpack $fmt , $$bufferPtr;

        $msg .= sprintf(    "0x%08x: C_ioexec_cqd:      0x%08x     \n", $address, $item1);
        $msg .= sprintf("            c_bgflush:         0x%08x     \n",$item2);
        $msg .= sprintf("            c_allowVLinkOps:   0x%08x     \n",$item3);
        $msg .= sprintf("            C_imt_head:        0x%08x     \n",$item4);
        $msg .= sprintf("            C_imt_tail:        0x%08x     \n\n",$item5);

        $info{C_IOEXEC_CQD}      = $item1;
        $info{C_BGFLUSH}         = $item2;
        $info{C_ALLOWVLINKOPS}   = $item3;
        $info{C_IMT_HEAD}        = $item4;
        $info{C_IMT_TAIL}        = $item5;

        $offset += 20;                         #   bytes processed
        $address += 20;
    }
    else
    {
        #C_ioexec_cqd:
        #        .space  4,0                     # I/O current queue depth
        #c_bgflush:
        #        .space  4,0                     # PCB pointer for BG Flush Task
        ##
        ## --- Registered IMT list
        ##       IMTs are placed on this list when not active.
        ##
        #C_imt_head:
        #        .word   0                       # Registered IMT list head pointer
        #C_imt_tail:
        #        .word   0                       # Registered IMT list tail pointer

        $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
        ($item1, $item2, $item3, $item4 ) =  
                        unpack $fmt , $$bufferPtr;
    
        $msg .= sprintf(    "0x%08x: C_ioexec_cqd: 0x%08x     \n", $address, $item1);
        $msg .= sprintf("            c_bgflush:    0x%08x     \n",$item2);
        $msg .= sprintf("            C_imt_head:   0x%08x     \n",$item3);
        $msg .= sprintf("            C_imt_tail:   0x%08x     \n",$item4);

        $info{C_IOEXEC_CQD} = $item1;
        $info{C_BGFLUSH}    = $item2;
        $info{C_IMT_HEAD}   = $item3;
        $info{C_IMT_TAIL}   = $item4;

        $offset += 16;                         #   bytes processed
        $address += 16;                  
    }

#_mag_imt_head:
#        .word   0                       # allocated IMT list head pointer
#_mag_imt_tail:
#        .word   0                       # allocated IMT list tail pointer

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: mag_imt_head:0x%08x     \n", $address, $item1);
    $msg .= sprintf("            mag_imt_tail:0x%08x     \n",$item2);

    $info{_MAG_IMT_HEAD} = $item1;
    $info{_MAG_IMT_TAIL} = $item2;

    $offset += 8;                         #   bytes processed
    $address += 8;                  

##
## --- MagDriver Statistical counters
##
#tag_counts:                             # tag queue type counts
#tag_00:
#        .word   0                       # tag type 00 counter
#tag_01:
#        .word   0                       # tag type 01 counter
#tag_02:
#        .word   0                       # tag type 02 counter
#tag_03:
#        .word   0                       # tag type 03 counter
#tag_04:
#        .word   0                       # tag type 04 counter
#tag_05:
#        .word   0                       # tag type 05 counter


#tag_06:
#        .word   0                       # tag type 06 counter
#tag_07:
#        .word   0                       # tag type 07 counter

    $fmt = sprintf("x%d LL LL LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("  \n0x%08x: tag_counts:     \n", $address);
    $msg .= sprintf("                tag_00       0x%08x     \n", $item1);
    $msg .= sprintf("                tag_01       0x%08x     \n", $item2);
    $msg .= sprintf("                tag_02       0x%08x     \n", $item3);
    $msg .= sprintf("                tag_03       0x%08x     \n", $item4);
    $msg .= sprintf("                tag_04       0x%08x     \n", $item5);
    $msg .= sprintf("                tag_05       0x%08x     \n", $item6);
    $msg .= sprintf("                tag_06       0x%08x     \n", $item7);
    $msg .= sprintf("                tag_07       0x%08x     \n\n", $item8);

    $offset += 32;                         #   bytes processed
    $address += 32;                  

    if ( 1 == $version )
    {
        #D_vlorc:
        #      .word    0                    # DLM VLink Outstanding Request Counter
        ##
        #
        $fmt = sprintf("x%d L ",$offset);      # generate the format string
        ($item1) =
                unpack $fmt , $$bufferPtr;

        $msg .= sprintf(    "0x%08x: D_vlorc:         0x%08x     \n\n", $address, $item1);

        $info{D_VLORC} = $item1;

        $offset += 4;                         #   bytes processed
        $address += 4;
    }

##
## --- ISP
##
#_req_cnt:
#        .word   0                       # //// TEMP //// request counter
##**********************************************************************
#
    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf(    "0x%08x: req_cnt:     0x%08x     \n", $address, $item1);

    $info{_REQ_CNT} = $item1;

    $offset += 4;                         #   bytes processed
    $address += 4;                  

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

###############################################################################
sub FmtWwn
{
    # function to byte flip a WWN
    my ($bufferPtr, $offset) = @_;
    
    my $fmt;
    my $msg;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $item5;
    my $item6;
    my $item7;
    my $item8;

    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7,
             $item8) =  unpack $fmt , $$bufferPtr;
    
    $msg = sprintf( "%02x%02x", $item1, $item2);
    $msg .= sprintf( "%02x%02x%02x%02x%02x%02x", $item3, $item4,
                   $item5, $item6, $item7, $item8);
    
    return $msg;
}

##############################################################################
# Name:     FormatDataString()
#
# Desc:     Format binary data in various formats to a string
#
# Input:    data
#           address (that is came from)
#           format (byte/short/word/binary)
#           reqLength - requested data length (0 = all available data)   
#      
##############################################################################
sub FmtDataString
{
    my ( $buffer, $address, $format, $reqLength, $offset) = @_;

    # String to store output
    my $str = "";
    my $fmtx;
    my $fmt;
    my $localBuffer = $$buffer;
    my $j;
    my $avail;

    if ( $format eq "text" ) {return $localBuffer;}

    if ( $format eq "trace" ) {return CCBDecoder($localBuffer); }

    # set up the byte count and templates based upon output format requested
    my $addrTpl = "%08X:  ";
    my $asciiTpl = "  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n";
    my $byteTpl = "CCCC CCCC CCCC CCCC";
    my $unpackTpl = "L L L L";
    my $byteTpl2 = "C";
    my $unpackTpl2 = "L";
    my $printfTpl = "%08X %08X  %08X %08X";
    my $printfTpl2 = "%08X ";
    my $wordSize = 4;
    
    if ($format =~ /^byte$/i) 
    {
        $unpackTpl = $byteTpl;
        $unpackTpl2 = "C";
        $wordSize = 1;
        $printfTpl = "%02X %02X %02X %02X %02X %02X %02X %02X  " .
             "%02X %02X %02X %02X %02X %02X %02X %02X"; 
        $printfTpl2 = "%02X ";
    }
    elsif ($format =~ /^short$/i) 
    {
        $unpackTpl = "SS SS SS SS";
        $unpackTpl2 = "S";
        $wordSize = 2;
        $printfTpl = "%04X %04X %04X %04X  %04X %04X %04X %04X";
        $printfTpl2 = "%04X ";
    }
    elsif ($format =~ /^text$/i) 
    {
        $unpackTpl = "SS SS SS SS";
        $unpackTpl2 = "S";
        $wordSize = 2;
        $printfTpl = "%04X %04X %04X %04X  %04X %04X %04X %04X";
        $printfTpl2 = "%04X ";
    }

    # get the overall length of the data buffer
    my $length = length $localBuffer;

    # if the requested length is defined return only the amount requested.
    if (defined $reqLength)
    {
        if ( $length < $offset + $reqLength)
        {
            # buffer is shorted than requested
            $avail = $length - $offset;
            $str .= "\nNote: $reqLength bytes were requested. Only $avail bytes available.\n";
            $str .= "      Request is being truncated to $avail bytes.\n\n";
            $reqLength = $avail;
        }
        $length = $reqLength;
    }

    if ( $length < 16 )
    {
        $localBuffer .= "xxxxxxxxxxxxxxxx";
    }
        
    my $i;
    my @rowData;
    my @asciiData;
    my $padLen = 0;
    my $bytesToTake;
    my $wordsToTake;

    for ($i=0; $i<$length; $i+=16) 
    {
        @rowData = ();
        @asciiData = ();

        # note: the number of bytes must be a multiple of 16
        # or we will get an error. Probably should check this.
        
        $fmt = sprintf("x%d ",$offset);      # generate offset component
                                             # of the format string

        $bytesToTake = int( min( 16, ($length - $i) ) );
        $wordsToTake = int( ($bytesToTake + $wordSize - 1)/$wordSize);  
        
        #$str .= sprintf("%2d: ",$wordsToTake);
              
        $fmtx = $fmt . sprintf("%s%d", $unpackTpl2, $wordsToTake);

                                                  # add unpack format part
        push @rowData, unpack $fmtx, $localBuffer;    # gets the hex part of
                                                  # the display

        $fmtx = $fmt . $byteTpl;

        my $tmp;
        foreach $tmp (unpack $fmtx, $localBuffer)     # get and format the
        {                                         # ASCII part of the 
            if ($tmp < 0x20 or $tmp >= 0x7f)      # display
            {
                $tmp = 0x2e; # '.'
            }
            push @asciiData, $tmp;
        }

        # format the line of data to be printed

        $str .= sprintf $addrTpl , $address;         # Address field

        for ( $j = 0; $j < $wordsToTake; $j++)       # Hex data
        {
            my $tmpval;
            if (defined($rowData[$j])) { $str .= sprintf($printfTpl2, $rowData[$j]);}
            # Toss any extra bytes.
        }

        $str .= " ";

        my $blanks =  32 + 16/$wordSize  - (2 * ( $bytesToTake  ) + $wordsToTake) ;

        for ( $j = 0; $j < $blanks; $j++)       # dead space
        {
            $str .= " ";
        }

        for ( $j = 0; $j < $bytesToTake; $j++)       # Ascii data
        {
            $str .= sprintf("%c", $asciiData[$j]);
        }

        $str .= "\n";

        $offset += 16;

        $address += 16;
    }

    return $str;
}

##############################################################################
sub FmtLoopSenseData
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
    my $length;
    my $numPdisks;
    my $i;

    my $ret;
    my %rsp;
    my $idx = 0;
    my @pdisks;

    my $startingOffset = $offset;

    my $currentByte = 0;

    $length =  length($$bufferPtr);

    #
    # start by getting the number of pdisks and their IDs from the buffer
    #

    #
    # get the pdisk count
    #
    
    $fmt = sprintf("x%d S", $offset);      # generate the format string
    ($item1) =     unpack $fmt , $$bufferPtr;

    $numPdisks = $item1;

    $offset += 2;
    
    #
    # pull out the list of pdisks
    #
    
    # $msg .= sprintf("     These Pdisks: ", $item1);

    for ( $i = 0; $i < $numPdisks; $i++ )
    {

        $fmt = sprintf("x%d S", $offset);      # generate the format string
        ($item1) =     unpack $fmt , $$bufferPtr;
        
        $pdisks[$i] = $item1;

        $offset += 2;

    }

    $msg .= " \n";

    #
    # now offset points to the first pdiskinfo record. Start a loop 
    # to walk thru each pdisk and collect the data
    #

    $rsp{COUNT} = 0;

    for ( $i = 0; $i < $numPdisks; $i++ )
    {
        $item5 = substr($$bufferPtr, $offset, 216); 

        $length =  length($item5);

        my $header = pack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16", 
                    128, ( $length  ), 0, 1,
                    PI_PDISK_INFO_CMD, 45657, 0, 1053011891, 
                    '   ', 0, 0, '        ',
                    0, 0, 0, '        ',
                    '                                ',
                    '-¦|a+ÑMxT+Q4?ç?F',
                    '+¦¦+i^SÑ?éVÜ=d¥  ');
        my %rp;

        $rp{'header'} = $header;

        $rp{'data'} = $item5;
    
        my %pdiskInfo = XIOTech::cmdMgr::_physicalDiskInfoPacket(0, 0, \%rp);

        #
        # now %pdiskinfo look like it came form the controller, not a buffer
        #

        $offset += 216;          # Move down the buffer to the sense part

        # process the pdiskinfo data

        if( %pdiskInfo )
        {
            if( $pdiskInfo{STATUS} == PI_GOOD )
            {
                #
                # Got the physical disk information - save it into the hash
                #
                $rsp{PDISKS}[$rsp{COUNT}]{PD_PID}       = $pdiskInfo{PD_PID};
                $rsp{PDISKS}[$rsp{COUNT}]{PD_DNAME}     = $pdiskInfo{PD_DNAME};
                $rsp{PDISKS}[$rsp{COUNT}]{SES}          = $pdiskInfo{SES};
                $rsp{PDISKS}[$rsp{COUNT}]{SLOT}         = $pdiskInfo{SLOT};
                $rsp{PDISKS}[$rsp{COUNT}]{PD_ID}        = $pdiskInfo{PD_ID};
                $rsp{PDISKS}[$rsp{COUNT}]{PD_CHANNEL}   = $pdiskInfo{PD_CHANNEL};
                $rsp{PDISKS}[$rsp{COUNT}]{PD_LOOPMAP}   = $pdiskInfo{PD_LOOPMAP};
                $rsp{PDISKS}[$rsp{COUNT}]{PD_DEVTYPE}   = $pdiskInfo{PD_DEVTYPE};                      
                
                #
                # Zero the counters before reading the log sense data
                #
                $rsp{PDISKS}[$rsp{COUNT}]{CIP}          = "N/A";

                $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}     = "N/A";

                $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}      = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}     = "N/A";
                $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}     = "N/A";

                $rsp{PDISKS}[$rsp{COUNT}]{POM}          = "N/A";

                #
                # fake the response from the SCSI command. We process it as
                # a buffer so there is less work.
                #

                $fmt = sprintf("x%d CCC L", $offset);      # generate the format string
                ($item1, $item2, $item3, $item4 ) =   unpack $fmt , $$bufferPtr;
        
                $offset += 7;

                if ( $item4 > 0 )
                {
                    $item5 = substr($$bufferPtr, $offset, $item4); 

                    $offset += $item4;   # advance past this data

                    my $currentByte = 2;          # skip page number
                    
                    # get the page length. This is the nuber of bytes to process

                    my $pageLength = unpack( "n", substr($item5, $currentByte) );

                    $currentByte += 2;            # past page length

                      
                    while( $currentByte < $pageLength )
                    {
                        my $parameterCode = unpack( "n", substr($item5, $currentByte) );
                          #printf("Parameter code: 0x%04x\n", $parameterCode) if( $debug );

                        if( $parameterCode == 0x0000 )
                        {
                            #printf("Temperature data\n") if( $debug );
                            $currentByte += 6;
                        }
                        elsif( $parameterCode == 0x0001 )
                        {
                            #printf("Reference temperature data\n") if( $debug );
                            $currentByte += 6;
                        }
                        elsif( $parameterCode == 0x0002 )
                        {
                            #printf("Undocumented temperature data\n") if( $debug );
                            $currentByte += 6;
                        }
                        elsif( $parameterCode == 0x80FF )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{CIP} = unpack( "b", substr($item5, $currentByte + 5) );
                            #printf("CIP:      0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{CIP}) if( $debug );
                            $currentByte += 6;
                        }
                        elsif( $parameterCode == 0x8100 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LFCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8101 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LSCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_A}) if( $debug );
                        }
                        elsif( ($parameterCode == 0x8102) ||
                               ($parameterCode == 0x8103) )
                        {
                            #printf("Undocumented parameter data\n") if( $debug );
                            $currentByte += 8;
                        }
                        elsif( $parameterCode == 0x8104 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("ITCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8105 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("ICCNT_A:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8106 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF7I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8107 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF7R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8108 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF8I_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8109 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF8R_A: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_A}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8110 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LFCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LFCNT_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8111 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LSCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LSCNT_B}) if( $debug );
                        }
                        elsif( ($parameterCode == 0x8112) ||
                               ($parameterCode == 0x8113) )
                        {
                            #printf("Undocumented parameter data\n") if( $debug );
                            $currentByte += 8;
                        }
                        elsif( $parameterCode == 0x8114 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("ITCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ITCNT_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8115 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}  = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("ICCNT_B:  0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{ICCNT_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8116 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF7I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7I_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8117 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF7R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF7R_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8118 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF8I_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8I_B}) if( $debug );
                        }
                        elsif( $parameterCode == 0x8119 )
                        {
                            $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B} = unpack( "N", substr($item5, $currentByte + 4) );
                            $currentByte += 8;
                            #printf("LIPF8R_B: 0x%0x\n", $rsp{PDISKS}[$rsp{COUNT}]{LIPF8R_B}) if( $debug );
                        }
                        else
                        {
                            #printf("Default\n") if( $debug );
                            $currentByte = $pageLength;
                        }

                    }   # end of while currentByte

                #
                # Now we have finished this pdiak
                #

                }   # end if if $item4   ( there was some sense data )

            }  # end of if pdisk info is good ( which should always be the case)

            ##
            # Bump the pdisk counter, but do this LAST!
            ##
            $rsp{COUNT}  = $rsp{COUNT} + 1;

        }    # end of if there exists pdisk info  ( which should always be the case)

    }   # end of for loop thru the drives 

    #
    # now we have a big hash ( %rsp )  It needs to be printed.    
    # 
     
    $msg .= XIOTech::cmdMgr::displayPhysicalDisks(0, "LOOP", %rsp);
    
    $$destPtr .= $msg;
}

##############################################################################
# Name:     MrpPLookUp()
#
# Desc:     Look up titles for proc flight recorder
#
# Input:    a byte (or 2)
#
# Output:   ASCII string
#     
sub MrpPLookUp
{
    my ($key, $key2) = @_;

    my $ret;
    $ret = " <unknown>";

# The following is based on mrp.inc


#       mr_func definitions

    if ( $key == 0x100 ) {$ret = " Create or expand a virtual device";}
    if ( $key == 0x101 ) {$ret = " Get SES device list";}
    if ( $key == 0x102 ) {$ret = " Label a physical device";}
    if ( $key == 0x103 ) {$ret = " Fail a device";}
    if ( $key == 0x104 ) {$ret = " Raw SCSI IO ";}
    if ( $key == 0x105 ) {$ret = " Initialize a RAID device";}
    if ( $key == 0x106 ) {$ret = " FCAL analysis ";}
    if ( $key == 0x107 ) {$ret = " Delete a virtual device ";}
    if ( $key == 0x108 ) {$ret = " Set caching mode ";}
    if ( $key == 0x109 ) {$ret = " Set server properties ";}
    if ( $key == 0x10A ) {$ret = " Reset NVRAM,  clear devices";}
    if ( $key == 0x10B ) {$ret = " Restore from NVRAM ";}
    if ( $key == 0x10C ) {$ret = " Awake ";}
    if ( $key == 0x10D ) {$ret = " WWN/LUN lookup ";}
    if ( $key == 0x10E ) {$ret = " Generic ";}
    if ( $key == 0x10F ) {$ret = " Start or stop a device ";}
    if ( $key == 0x110 ) {$ret = " Scrubbing/Scanning control";}
    if ( $key == 0x111 ) {$ret = " Set default label behavior ";}
    if ( $key == 0x112 ) {$ret = " Get BE Device Paths";}
    if ( $key == 0x113 ) {$ret = " Restore a physical device ";}
    if ( $key == 0x114 ) {$ret = " Defragment device ";}
    if ( $key == 0x115 ) {$ret = " Set attribute ";}
    if ( $key == 0x116 ) {$ret = " Get back end loop information";}
    if ( $key == 0x117 ) {$ret = " Get server list";}
    if ( $key == 0x118 ) {$ret = " Get virtual device list";}
    if ( $key == 0x119 ) {$ret = " Get RAID list";}
    if ( $key == 0x11A ) {$ret = " Get physical device list";}
    if ( $key == 0x11B ) {$ret = " Get misc device list";}
    if ( $key == 0x11C ) {$ret = " Get virtual device information";}
    if ( $key == 0x11D ) {$ret = " Get RAID information";}
    if ( $key == 0x11E ) {$ret = " Get physical device information";}
    if ( $key == 0x11F ) {$ret = " Map a LUN to a VDisk";}
    if ( $key == 0x120 ) {$ret = " Unmap a LUN from a VDisk";}
    if ( $key == 0x121 ) {$ret = " Get SES information ";}
    if ( $key == 0x122 ) {$ret = " Create a server";}
    if ( $key == 0x123 ) {$ret = " Delete a server ";}
    if ( $key == 0x124 ) {$ret = " Get misc device info";}
    if ( $key == 0x125 ) {$ret = " Virtual disk control";}
    if ( $key == 0x126 ) {$ret = " Assign system information ";}
    if ( $key == 0x127 ) {$ret = " Get back end II information";}
    if ( $key == 0x128 ) {$ret = " Get back end link information ";}
    if ( $key == 0x129 ) {$ret = " Get back end boot code ";}
    if ( $key == 0x12A ) {$ret = " Get back end diagnostic code header ";}
    if ( $key == 0x12B ) {$ret = " Get back end proc code header ";}
    if ( $key == 0x12C ) {$ret = " Burn back end flash code";}
    if ( $key == 0x12D ) {$ret = " Read/Write memory on back end ";}
    if ( $key == 0x12E ) {$ret = " Configure target ";}
    if ( $key == 0x12F ) {$ret = " Get Mirror partner list ";}
    if ( $key == 0x130 ) {$ret = " Set background priority";}
    if ( $key == 0x131 ) {$ret = " Get target list";}
    if ( $key == 0x132 ) {$ret = " Reset BE Qlogic chip ";}
    if ( $key == 0x133 ) {$ret = " VDisk or controller name changed";}
    if ( $key == 0x134 ) {$ret = " Get remote controller count ";}
    if ( $key == 0x135 ) {$ret = " Get remote controller information ";}
    if ( $key == 0x136 ) {$ret = " Get remote virtual disk information ";}
    if ( $key == 0x137 ) {$ret = " Set foreign targets ";}
    if ( $key == 0x138 ) {$ret = " Create vlink ";}
    if ( $key == 0x139 ) {$ret = " Get virtual link information ";}
    if ( $key == 0x13A ) {$ret = " Create a new controller  ";}
    if ( $key == 0x13B ) {$ret = " Rescan physical devices ";}
    if ( $key == 0x13C ) {$ret = " Resync RAID or stripes";}
    if ( $key == 0x13D ) {$ret = " Get local NVRAM image ";}
    if ( $key == 0x13E ) {$ret = " Put local NVRAM image";}
    if ( $key == 0x13F ) {$ret = " Delete SES or disk drive device";}
    if ( $key == 0x140 ) {$ret = " Mode page ";}
    if ( $key == 0x141 ) {$ret = " Get device count";}
    if ( $key == 0x142 ) {$ret = " Get Vdisk Owner ";}
    if ( $key == 0x143 ) {$ret = " Hotspare information";}
    if ( $key == 0x144 ) {$ret = " File system file to file copy";}
    if ( $key == 0x145 ) {$ret = " Get device List";}
    if ( $key == 0x146 ) {$ret = " Get BE Port List ";}
    if ( $key == 0x147 ) {$ret = " Break vlink lock ";}
    if ( $key == 0x148 ) {$ret = " Get SOS structure for a drive";}
    if ( $key == 0x149 ) {$ret = " Update a SOS structure";}
    if ( $key == 0x14A ) {$ret = " Force a BE error trap";}
    if ( $key == 0x14B ) {$ret = " Update a SCMT structure";}
    if ( $key == 0x14C ) {$ret = " Perform a BE Loop Primitive ";}
    if ( $key == 0x14D ) {$ret = " Target movement control";}
    if ( $key == 0x14E ) {$ret = " Fail / Unfail controller";}
    if ( $key == 0x14F ) {$ret = " Set name of device (vdisk, ctrl,  etc.) ";}
    if ( $key == 0x150 ) {$ret = " interprocessor DataGram ";}
    if ( $key == 0x151 ) {$ret = " BE no-op";}
    if ( $key == 0x152 ) {$ret = " Put a file system report";}
    if ( $key == 0x153 ) {$ret = " Get DLink information";}
    if ( $key == 0x154 ) {$ret = " Get DLock information";}
    if ( $key == 0x155 ) {$ret = " Degrade / restore port";}

    if ( $key == 0x1FD ) {$ret = " Internal file system no op";}
    if ( $key == 0x1FE ) {$ret = " Internal file system operation ";}
    if ( $key == 0x1FF ) {$ret = " Back end heartbeat from CCB";}

    if ( $key == 0x200 ) {$ret = " Report server configuration";}
    if ( $key == 0x201 ) {$ret = " Server configuration complete";}
    if ( $key == 0x202 ) {$ret = " Report caching configuration";}
    if ( $key == 0x203 ) {$ret = " Caching configuration complete ";}
    if ( $key == 0x204 ) {$ret = " Spare";}
    if ( $key == 0x205 ) {$ret = " Stop caching ";}
    if ( $key == 0x206 ) {$ret = " Continue caching";}
    if ( $key == 0x207 ) {$ret = " Set system serial number,  etc. ";}
    if ( $key == 0x208 ) {$ret = " Virtual disk config has changed ";}
    if ( $key == 0x209 ) {$ret = " Server config has changed ";}
    if ( $key == 0x20A ) {$ret = " Report target config";}
    if ( $key == 0x20B ) {$ret = " Reset configuration, NVRAM, etc ";}
    if ( $key == 0x20C ) {$ret = " Set controller serial number, FE ";}

    if ( $key == 0x300 ) {$ret = " Create log entry (Front end to CCB)";}
    if ( $key == 0x301 ) {$ret = " Create log entry (Back end to CCB) ";}

    if ( $key == 0x400 ) {$ret = " Get VDisk info to FEP ";}
    if ( $key == 0x401 ) {$ret = " Set Sequence Number";}
    if ( $key == 0x402  ) {$ret = " Set Mirror Partner";}
    if ( $key == 0x402 ) {$ret = " Front end to back end max ";}

    if ( $key == 0x500 ) {$ret = " Get front end loop information";}
    if ( $key == 0x501 ) {$ret = " Get server information";}
    if ( $key == 0x502 ) {$ret = " Get cache information ";}
    if ( $key == 0x503 ) {$ret = " Get front end link information ";}
    if ( $key == 0x504 ) {$ret = " Get front end II information";}
    if ( $key == 0x505 ) {$ret = " Get cache device information";}
    if ( $key == 0x506 ) {$ret = " Get server statistics ";}
    if ( $key == 0x507 ) {$ret = " Set battery health ";}
    if ( $key == 0x508 ) {$ret = " Resume cache initialization";}
    if ( $key == 0x509 ) {$ret = " Get front end boot code header";}
    if ( $key == 0x50A ) {$ret = " Get front end diagnostic code header";}
    if ( $key == 0x50B ) {$ret = " Get front end proc code header";}
    if ( $key == 0x50C ) {$ret = " Burn back end flash code";}
    if ( $key == 0x50D ) {$ret = " Read/Write memory on front end ";}
    if ( $key == 0x50E ) {$ret = " Reset FE Qlogic chip ";}
    if ( $key == 0x50F ) {$ret = " Server lookup MRP";}
    if ( $key == 0x510 ) {$ret = " Generic";}
    if ( $key == 0x511 ) {$ret = " Assign a Mirror Partner ";}
    if ( $key == 0x512 ) {$ret = " FE Fibre Heartbeat List for DLM";}
    if ( $key == 0x513 ) {$ret = " Continue Cache Init w/o Mirror Partner";}
    if ( $key == 0x514 ) {$ret = " Flush FE Cache w/o Mirror Partner";}
    if ( $key == 0x515 ) {$ret = " Invalidate the FE Write Cache";}
    if ( $key == 0x516 ) {$ret = " Flush the BE Write Cache";}
    if ( $key == 0x517 ) {$ret = " Invalidate the BE Write Cache";}
    if ( $key == 0x518 ) {$ret = " Mode page";}
    if ( $key == 0x519 ) {$ret = " Get Device List";}
    if ( $key == 0x51A ) {$ret = " Get FE Port List";}
    if ( $key == 0x51B ) {$ret = " Get target resource list";}
    if ( $key == 0x51C ) {$ret = " Stop I/O ";}
    if ( $key == 0x51D ) {$ret = " Start I/O ";}
    if ( $key == 0x51E ) {$ret = " Set FE port event notification ";}
    if ( $key == 0x51F ) {$ret = " Force a FE error trap";}
    if ( $key == 0x520 ) {$ret = " Perform a FE Loop Primitive";}
    if ( $key == 0x521 ) {$ret = " Get target information";}
    if ( $key == 0x522 ) {$ret = " Fail / Unfail port";}
    if ( $key == 0x523 ) {$ret = " No-op";}
    if ( $key == 0x523 ) {$ret = " Max CCB to front end define commands";}

    if ( $key == 0x5FF ) {$ret = " Front end heartbeat from CCB";}

    return $ret;
}

##############################################################################
# Name:     FltRecPLookup()
#
# Desc:     Look up titles for proc flight recorder
#
# Input:    a byte (or 2)
#
# Output:   ASCII string
#     
##############################################################################

sub FltRecPLookUp
{
    my ($key, $key2) = @_;

    my $ret;
    $ret = "";

    # the following is based on fr.inc

    if ( $key == 0xe0 ) {$ret = "DLM - rrp received ";}
    if ( $key == 0xe1 ) {$ret = "DLM - message execution ";}
    if ( $key == 0xe2 ) {$ret = "DLM - datagram send ";}
    if ( $key == 0xe3 ) {$ret = "DLM - Queued to BE SRP Executive ";}
    if ( $key == 0xe4 ) {$ret = "DLM - BE SRP Executive ";}
    if ( $key == 0xe5 ) {$ret = "DLM - Queue to DRP Executive ";}
    if ( $key == 0xe6 ) {$ret = "DLM - DRP Executive ";}
    if ( $key == 0xd0 ) {$ret = "Hotswap  ";}
    if ( $key == 0xd1 ) {$ret = "Inquire  ";}
    if ( $key == 0xd2 ) {$ret = "Init Drive ";}
    if ( $key == 0xd3 ) {$ret = "Misc Functions  ";}
    if ( $key == 0xd4 ) {$ret = "Rebuild functions  ";}
    if ( $key == 0xd5 ) {$ret = "Raid Error  ";}
    if ( $key == 0xc0) {$ret = "Kernel - memory allocation";}
    if ( $key == 0xc1) {$ret = "Kernel - memory release";}
    if ( $key == 0xc2) {$ret = "Kernel - defered memory release";}
    if ( $key == 0xc3) {$ret = "Kernel - context switch timing";}
    if ( $key == 0xb0) {$ret = "Misc  ";}
    if ( $key == 0xb1) {$ret = "Misc  ";}
    if ( $key == 0xb2) {$ret = "Misc - allocate an ILT  ";}
    if ( $key == 0xb3) {$ret = "Misc - release an ILT  ";}
    if ( $key == 0xa0 ) {$ret = "AplDriver  ";}
    if ( $key == 0x90 ) {$ret = "IDriver  ";}
    if ( $key == 0x80 ) {$ret = "CDriver  ";}
    if ( $key == 0x70 ) {$ret = "MagDriver  ";}
    if ( $key == 0x60 ) {$ret = "Cache - Queued to Cache Overlapped  ";}
    if ( $key == 0x61 ) {$ret = "Cache - Initial Overlapped Check  ";}
    if ( $key == 0x62 ) {$ret = "Cache - Queued to I/O Executive  ";}
    if ( $key == 0x63 ) {$ret = "Cache - I/O Executive  ";}
    if ( $key == 0x64 ) {$ret = "Cache - Non-cached Read Complete 1  ";}
    if ( $key == 0x65 ) {$ret = "Cache - Non-cached Read Complete 2  ";}
    if ( $key == 0x66 ) {$ret = "Cache - Non-cached Write Complete 1  ";}
    if ( $key == 0x67 ) {$ret = "Cache - Non-cached Write Complete 2  ";}
    if ( $key == 0x68 ) {$ret = "Cache - Next Level Completion  ";}
    if ( $key == 0x69 ) {$ret = "Cache - Call lower function  ";}
    if ( $key == 0x6A ) {$ret = "Cache - Call Upper function  ";}
    if ( $key == 0x6B ) {$ret = "Cache - Write Cache Data complete  ";}
    if ( $key == 0x6C ) {$ret = "Cache - I/O Complete (or Write Comp)  ";}
    if ( $key == 0x6D ) {$ret = "Cache - Que to the DRP Executive  ";}
    if ( $key == 0x6E ) {$ret = "Cache - DRP Executive  ";}
    if ( $key == 0x6F ) {$ret = "Cache - DRP Completion  ";}
    if ( $key == 0x50 ) {$ret = "Link  ";}
    if ( $key == 0x51 ) {$ret = "Link  - outbound VRP exec  ";}
    if ( $key == 0x52 ) {$ret = "Link  - inbound VRP exec  ";}
    if ( $key == 0x53 ) {$ret = "Link  - completion routine  ";}
    if ( $key == 0x54 ) {$ret = "Link  - completion exec  ";}
    if ( $key == 0x55 ) {$ret = "Link  - processor sync  ";}
    if ( $key == 0x40 ) {$ret = "Virtual ";}
    if ( $key == 0x41 ) {$ret = "Virtual ";}
    if ( $key == 0x42 ) {$ret = "Virtual ";}
    if ( $key == 0x30 ) {$ret = "Raid - main exec  ";}
    if ( $key == 0x31 ) {$ret = "Raid  - completion  ";}
    if ( $key == 0x32 ) {$ret = "Raid  - parity scan bad  ";}
    if ( $key == 0x33 ) {$ret = "Raid  - raid 5 insert RRB into RPN  ";}
    if ( $key == 0x34 ) {$ret = "Raid  - raid 5 complete RRB  ";}
    if ( $key == 0x20 ) {$ret = "Physical  ";}
    if ( $key == 0x21 ) {$ret = "Physical  ";}
    if ( $key == 0x22 ) {$ret = "Physical  ";}
    if ( $key == 0x23 ) {$ret = "Physical  ";}
    if ( $key == 0x24 ) {$ret = "Physical  ";}
    if ( $key == 0x26 ) {$ret = "Physical  ";}
    if ( $key == 0x27 ) {$ret = "Physical  ";}
    if ( $key == 0x28 ) {$ret = "Physical  ";}
    if ( $key == 0x10 ) {$ret = "ISP  ";}
    if ( $key == 0x11 ) {$ret = "ISP  ";}
    if ( $key == 0x12 ) {$ret = "ISP  ";}
    if ( $key == 0x13 ) {$ret = "ISP  ";}
    if ( $key == 0x14 ) {$ret = "ISP  ";}
    if ( $key == 0x15 ) {$ret = "ISP  ";}

    return $ret;
}

##############################################################################
sub FmtString
{
    my ( $bufferPtr, $offset, $length ) = @_;

    my $fmt;
    my @chars;
    my $i;

    $fmt = sprintf("x%d C%d", $offset, $length);
        
    @chars = (unpack( $fmt, $$bufferPtr));

    # $i = scalar(@chars);
    # print" num chars = $i \n";

    for (  $i = 0; $i < scalar(@chars) ; $i++ )
    { 
        if ( $chars[$i] < 32 || $chars[$i] > 127 )    # non printable
        {
            if ( $chars[$i] != 10 && $chars[$i] != 13 )   # cr and LF
            {
                $chars[$i] = 46; # '.'
            }
        }
    }

    $fmt = sprintf("C%d", $length);
    
    $i = pack( $fmt, @chars);

    $i =~ s/\x0D/\x0A/g;    # clean up cr/lf's
    $i =~ s/\x0A{1,}/\n/g;  # (takes 2 steps)

    return $i;
}

##############################################################################
sub GetIPAddr
{
    my( $bufferPtr, $offset ) = @_;

    my $msg;
    my $item1;
    my $item2;
    my $item3;
    my $item4;
    my $fmt;

    $fmt = sprintf("x%d CCCC",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg = sprintf("%d:%d:%d:%d", $item1, $item2, $item3, $item4);

    return $msg;
}    

##############################################################################
# Name:     InitConstants()
#
# Desc:     Initialize a hash of psuedo constants that may be needed to analyze
#           the data. These are based upon M970 code and if they need to change
#           the new values should be in the data stream being decoded. This fcn 
#           should be called before real decoding begins. The hash may be
#           updated as the stream is processed. This could have been globals, 
#           then we'd have to handle side effects. 
#
# Input:    data
#           address (that is came from)
#           format (byte/short/word/binary)
#           reqLength - requested data length (0 = all available data)   
#      
##############################################################################
sub InitConstants
{
    my ($constPtr) = @_;

    if (!$constPtr)
    {
        return ERROR;
    }

    # currently, the Consts packet does not contain ALL of these

    $constPtr->{MAXDRPERRAID} = 0x040;
    $constPtr->{MAXDRIVES}    = 0x400;
    $constPtr->{MAXSES}       = 0x080;
    $constPtr->{MAXMISC}      = 0x040;
    $constPtr->{MAXTARGETS}   = 64;
    $constPtr->{MAXVLINKS}    = 0x040;
    $constPtr->{MAXVIRTUALS}  = 0x200;
    $constPtr->{MAXSERVERS}   = 0x400;
    $constPtr->{MAXRAIDS}     = 0x400;
    $constPtr->{MAXLDDS}      = 0x100;
    $constPtr->{MAXLUN}       = 64;
    $constPtr->{MAXTAG}       = 31;
    $constPtr->{MAXCHN}       = 4;
    $constPtr->{MAXISP}       = 4;
    $constPtr->{MAXDEV}       = 256;

    $constPtr->{MAXCIMT}      = 0;
    $constPtr->{MAXICIMT}     = 0;
    $constPtr->{MAXVRP}       = 0;
    $constPtr->{MAXIF}        = 4;
    $constPtr->{MAXCTRL}      = 2;

    return GOOD;
}
 
##############################################################################
#
#          Name: UpdateConsts
# Call: 
#   UpdateConsts ( $destPtr, $bufferPtr, $offset, $reqLength, $processor, $address, $constPtr);
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
#           $constPtr - pointer to hash with psuedo constants
#
#  Return: GOOD or ERROR
#    
# The data structure being decoded...
#
# --- Begin Part 5 Constants data entry structure ---------------------
#
#        .set    nv5c_tgt,0              # Max # of targets          <w>
#        .set    nv5c_isp,nv5c_tgt+4     # Max # of ISPs             <w>
#        .set    nv5c_virt,nv5c_isp+4    # Max # of vdisks           <w>
#        .set    nv5c_cimt,nv5c_virt+4   # Max # of CIMTs            <w>
#                                                                  *****
#        .set    nv5c_lun,nv5c_cimt+4    # Max # of LUNs             <w>
#        .set    nv5c_serv,nv5c_lun+4    # Max # of servers          <w>
#        .set    nv5c_icimt,nv5c_serv+4  # Max # of ICIMTs           <w>
#        .set    nv5c_chn,nv5c_icimt+4   # Max # of channels         <w>
#                                                                  *****
#        .set    nv5c_raid,nv5c_chn+4    # Max # of raids            <w>
#        .set    nv5c_dev,nv5c_raid+4    # Max # of devices          <w>
#        .set    nv5c_vrp,nv5c_dev+4     # Max # of VRPs             <w>
#        .set    nv5c_drive,nv5c_vrp+4   # Max # of drives           <w>
#                                                                  *****
#        .set    nv5c_if,nv5c_drive+4    # Max # of interfaces       <w>
#        .set    nv5c_ctrl,nv5c_if+4     # Max # of controllers      <w>
#        .set    nv5c_ses,nv5c_ctrl+4    # Max # of SESs             <w>
#        .set    nv5c_lid,nv5c_ses+4     # Max # of LIDs             <w>
#                                                                  *****
#        .set    nv5c_size,nv5c_lid+4    # Size of constants entry
#
# --- End structure ---------------------------------------------------
#
#
##############################################################################

sub UpdateConsts
{
    my ( $destPtr, $bufferPtr, $offset, $length, $processor, $address, $constPtr )= @_;
    
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


    $msg = "\n\nUpdating script constants...\n\n";

    #        .set    nv5c_tgt,0              # Max # of targets          <w>
    #        .set    nv5c_isp,nv5c_tgt+4     # Max # of ISPs             <w>
    #        .set    nv5c_virt,nv5c_isp+4    # Max # of vdisks           <w>
    #        .set    nv5c_cimt,nv5c_virt+4   # Max # of CIMTs            <w>

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $constPtr->{MAXTARGETS}   = $item1;
    $constPtr->{MAXISP}       = $item2;
    $constPtr->{MAXVIRTUALS}  = $item3;
    $constPtr->{MAXCIMT}      = $item4;

    $msg .= sprintf( "      Max # of targets: 0x%08x ", $item1);
    $msg .= sprintf( "         Max # of ISPs: 0x%08x \n", $item2);
    $msg .= sprintf( "       Max # of vdisks: 0x%08x ", $item3);
    $msg .= sprintf( "        Max # of CIMTs: 0x%08x \n", $item4);

    $offset += 16;



    #                                                                  *****
    #        .set    nv5c_lun,nv5c_cimt+4    # Max # of LUNs             <w>
    #        .set    nv5c_serv,nv5c_lun+4    # Max # of servers          <w>
    #        .set    nv5c_icimt,nv5c_serv+4  # Max # of ICIMTs           <w>
    #        .set    nv5c_chn,nv5c_icimt+4   # Max # of channels         <w>
    #                                                                  *****

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $constPtr->{MAXLUN}       = $item1;
    $constPtr->{MAXSERVERS}   = $item2;
    $constPtr->{MAXICIMT}     = $item3;
    $constPtr->{MAXCHN}       = $item4;

    $msg .= sprintf( "         Max # of LUNs: 0x%08x ", $item1);
    $msg .= sprintf( "      Max # of servers: 0x%08x \n", $item2);
    $msg .= sprintf( "       Max # of ICIMTs: 0x%08x ", $item3);
    $msg .= sprintf( "     Max # of channels: 0x%08x \n", $item4);

    $offset += 16;



    #        .set    nv5c_raid,nv5c_chn+4    # Max # of raids            <w>
    #        .set    nv5c_dev,nv5c_raid+4    # Max # of devices          <w>
    #        .set    nv5c_vrp,nv5c_dev+4     # Max # of VRPs             <w>
    #        .set    nv5c_drive,nv5c_vrp+4   # Max # of drives           <w>
    #                                                                  *****
    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $constPtr->{MAXRAIDS}     = $item1;
    $constPtr->{MAXDEV}       = $item2;
    $constPtr->{MAXVRP}       = $item3;
    $constPtr->{MAXDRIVES}    = $item4;

    $msg .= sprintf( "        Max # of raids: 0x%08x ", $item1);
    $msg .= sprintf( "      Max # of devices: 0x%08x \n", $item2);
    $msg .= sprintf( "         Max # of VRPs: 0x%08x ", $item3);
    $msg .= sprintf( "       Max # of drives: 0x%08x \n", $item4);

    $offset += 16;



    #        .set    nv5c_if,nv5c_drive+4    # Max # of interfaces       <w>
    #        .set    nv5c_ctrl,nv5c_if+4     # Max # of controllers      <w>
    #        .set    nv5c_ses,nv5c_ctrl+4    # Max # of SESs             <w>
    #        .set    nv5c_lid,nv5c_ses+4     # Max # of LIDs             <w>
    #                                                                  *****

    $fmt = sprintf("x%d LLLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $constPtr->{MAXIF}        = $item1;
    $constPtr->{MAXCTRL}      = $item2;
    $constPtr->{MAXSES}       = $item3;
    $constPtr->{MAXLDDS}      = $item4;

    $msg .= sprintf( "   Max # of interfaces: 0x%08x ", $item1);
    $msg .= sprintf( "  Max # of controllers: 0x%08x \n", $item2);
    $msg .= sprintf( "         Max # of SESs: 0x%08x ", $item3);
    $msg .= sprintf( "         Max # of LIDs: 0x%08x \n", $item4);

    $offset += 16;



    $$destPtr .= $msg;

    return GOOD;

}

##############################################################################
# this functions checks to see if there are enough bytes in the buffer
# to be processed by the caller. If there are not, then what is there will be 
# handled as hex so that the user at least gets something. The $needed value
# must be what the parser wants to process, not what the records header has 
#
sub ValidateLength
{

    my ( $destPtr, $bufferPtr, $offset, $length, $title, $address )= @_;

    my $available;
    my $ret;
    my $msg;
    my $needed;

    $needed = SizeLookup( $title );   # gets # bytes to be processed

    $available = length( $$bufferPtr ) - $offset;   # bytes available to do

#print ("In ValidateLength: title = $title, needed = $needed, wanted = $length, available = $available offset = $offset \n"); 
#print ("In ValidateLength: if ( \$title eq \"$title\"    ) { return $length; } \n"); 

#if ($needed == $length )
#{
#    print ("In ValidateLength: MATCH \n");
#}
#else
#{ 
#    print ("In ValidateLength: ************\n"); 
#    print ("In ValidateLength: * NO MATCH *\n"); 
#    print ("In ValidateLength: ************ \n"); 
#}

    if ( ($available >= $needed) || ($needed == 0) )
    {
        # there is enough to parse, let the parser run. OR we
        # don't know how much is needed, so let the parser run 
        # anyway
        return GOOD;
    }

#print ("In ValidateLength: need to do this one ($title) as hex \n"); 

    $$destPtr .= "\n------> Incomplete record, formatting as hex only....\n\n";
    
    $$destPtr .= FmtDataString( $bufferPtr, $address, "word", $available, $offset);

    # we've done this as hex, return ERROR to indicate tere was a problem
    # with the file. The caller(s) should be able to gracefully exit.

    return ERROR;
}

###############################################################################
# map device type to a string of some sort
sub DevTypeLookup
{
    my ($t) = @_;
    
    my $r = " other ";

    # values from pdd.inc
    if  ( $t == 0 ) { $r = "Disk Drive"; }
    if  ( $t == 0x0d ) { $r = "SES device (enclosure)"; }

    return $r;

}

##############################################################################
#
# This is a lookup table for the sizes of the arrayes we will process.
# Any time a Fmt routine is changed, this table shoud be updated. THe values 
# represent the number of bytes each parser requires. 
#
##############################################################################
sub SizeLookup
{
    my ( $title ) = @_;

    if ( $title eq "CIMTs"    ) { return 48; }             # FmtCIMTs
    if ( $title eq "Consts"   ) { return 64; }             # UpdateConsts
    if ( $title eq "Internal" ) { return 192; }            # FmtKii

    if ( $title eq "Run FWH"  ) { return 128; }            # FmtCIMTs
    if ( $title eq "Boot FWH" ) { return 128; }            # FmtCIMTs
    if ( $title eq "Diag FWH" ) { return 128; }            # FmtCIMTs

    if ( $title eq "Flt Rec"  ) { return 4800; }           # FmtCIMTs
    if ( $title eq "MRP Trce" ) { return 4800; }           # FmtCIMTs

    if ( $title eq "Defn EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "Phys EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "Raid EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "Virt EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "Rd5 EQ"   ) { return 16; }             # FmtCIMTs
    if ( $title eq "Rint EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "XORx EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "FSys EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "XORc EQ"  ) { return 16; }             # FmtCIMTs
    if ( $title eq "RdErr EQ" ) { return 16; }             # FmtCIMTs

    # PCBs are a 16 byte header plus a bunch of lines printed
    # as hex. SO, lets require a minimum of 1 line only. (They are
    # also not all the same length anyway.)
    
    if ( $title eq "Phys PCB" ) { return 32; }           # 
    if ( $title eq "Raid PCB" ) { return 32; }           # 
    if ( $title eq "Rd5 PCB"  ) { return 32; }           # 
    if ( $title eq "Virt PCB" ) { return 32; }           # 
    if ( $title eq "Rint PCB" ) { return 32; }           # 
    if ( $title eq "XORc PCB" ) { return 32; }           # 
    if ( $title eq "XORx PCB" ) { return 32; }           # 
    if ( $title eq "FSys PCB" ) { return 32; }           # 
    if ( $title eq "RdErrPCB" ) { return 32; }           # 
    if ( $title eq "Defn PCB" ) { return 32; }           # 
    if ( $title eq "Curr PCB" ) { return 32; }           # 
    if ( $title eq "CachePCB" ) { return 32; }
    if ( $title eq "CchIOPCB" ) { return 32; }

    if ( $title eq "Trgt def" ) { return 48; }             # FmtCIMTs
    if ( $title eq "iTGDs" )    { return 112;} 
    if ( $title eq "ISCSISES" ) { return 1664;} 
    if ( $title eq "ISCSICON" ) { return 12464;} 
    if ( $title eq "MLMTs"    ) { return 32; }             # FmtCIMTs
    if ( $title eq "Link QCS" ) { return 80; }             # FmtCIMTs
    if ( $title eq "Defrag T" ) { return 1024; }           # FmtCIMTs
    if ( $title eq "VDD"      ) { return 112; }            # FmtCIMTs
    if ( $title eq "LLDMT Dr" ) { return 16; }             # FmtCIMTs
    if ( $title eq "LLDMTs"   ) { return 16; }             # FmtCIMTs
    if ( $title eq "DTMTs"    ) { return 112; }            # FmtCIMTs

    if ( $title eq "Target"    ) { return 32; }
    if ( $title eq "CIMT Dir"    ) { return 16; }
    if ( $title eq "Trc log"    ) { return 4800; }
    if ( $title eq "IMT"    ) { return 880; }
    if ( $title eq "ILMT/WET"    ) { return 224; }
    if ( $title eq "VDMT"    ) { return 32; }
    if ( $title eq "SDD"    ) { return 80; }
    if ( $title eq "LVM"    ) { return 16; }
    if ( $title eq "VCD"    ) { return 112; }
    if ( $title eq "SvrdbDir"    ) { return 16; }
    if ( $title eq "Svr DB"    ) { return 2048; }
    if ( $title eq "ICIMT Dr"    ) { return 16; }
    if ( $title eq "ICIMTs"    ) { return 1104; }
    if ( $title eq "Itrc log"    ) { return 4800; }
    if ( $title eq "TMTs"    ) { return 368; }
    if ( $title eq "LTMTs"    ) { return 192; }
    if ( $title eq "LSMTs"    ) { return 48; }
    if ( $title eq "TMTs"    ) { return 368; }
    if ( $title eq "LSMTs"    ) { return 48; }
    if ( $title eq "Itrc log"    ) { return 4800; }

  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs  FmtVCD
  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs
  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs
  # if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs
  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs
  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs
  #  if ( $title eq "CIMTs" ) { return 48; }           # FmtCIMTs

    # no match, return 0 to indicate we just process the data, length not
    # validated.
    return 0;
}

##############################################################################
# Name:     FmtCCBETRegs
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    /* 8-bit registers */
#    UINT8 statusGpodReg;    /* GPOD Output Data Register */
#    UINT8 reserved0[15];    /* Align to 16 byte boundary */
#
#    /* 16-bit registers */
#    UINT16 nmiPatusrReg;    /* Primary ATU Status Register      */                
#    UINT16 nmiSatusrReg;    /* Secondary ATU Status Register    */                    
#
#    UINT16 nmiPcrReg;       /* Primary Command Register        */
#    UINT16 nmiBcrReg;       /* Bridge Control Register         */
#    UINT16 nmiPsrReg;       /* Primary Status Register         */
#    UINT16 nmiSsrReg;       /* Secondary Status Register        */
#    UINT16 nmiSderReg;      /* Secondary Decode Enable Register */
#    UINT16 nmiPatucmdReg;   /* Primary ATU Command Register     */
#    UINT16 nmiSatucmdReg;   /* Secondary ATU Command Register   */
#    UINT8  reserved1[14];   /* Align to 16 byte boundary       */
#
#    /* 32-bit registers */
#    UINT32 nmiNisrReg;      /* nmi NMI Interrupt Status Register            */
#
#    UINT32 nmiPatuimrReg;   /* nmi Primary ATU Interrupt Mask Register      */
#    UINT32 nmiSatuimrReg;   /* nmi Secondary ATU Interrupt Mask Register    */
#    UINT32 nmiAtucrReg;     /* nmi ATU Configuration Register               */
#
#    UINT32 nmiMcisrReg;     /* nmi bit 0, Memory Controller Interrupt Status Register   */
#    UINT32 nmiPatuisrReg;   /* nmi bit 1, Primary ATU Interrupt Status Register         */
#    UINT32 nmiSatuisrReg;   /* nmi bit 2, Secondary ATU Interrupt Status Register       */
#    UINT32 nmiPbisrReg;     /* nmi bit 3, Primary Bridge Interrupt Status Register      */
#    UINT32 nmiSbisrReg;     /* nmi bit 4, Secondary Bridge Interrupt Status Register    */
#    UINT32 nmiCsr0Reg;      /* nmi bit 5, Channel Status Register           */
#    UINT32 nmiCsr1Reg;      /* nmi bit 6, Channel Status Register           */
#    UINT32 nmiCsr2Reg;      /* nmi bit 7, Channel Status Register           */
#    UINT32 nmiIisrReg;      /* nmi bit 8 / 9 is external, Inbound Interrupt Status Register */
#    UINT32 nmiAsrReg;       /* nmi bit 10, Accelerator Status Register      */
#    UINT32 nmiBiuisrReg;    /* nmi bit 11, BIU Interrupt Status Register    */
#                                                                          
#    UINT32 nmiEccrReg;      /* ECC Control Register     */
#    UINT32 nmiEctstReg;     /* ECC Test Register        */
#
#    UINT32 nmiElog0Reg;     /* ECC Log Registers        */
#    UINT32 nmiElog1Reg;     /* ECC Log Registers        */
#    UINT32 nmiEcar0Reg;     /* ECC Address Registers    */
#    UINT32 nmiEcar1Reg;     /* ECC Address Registers    */
#    
#    UINT8  reserved2[12];   /* Align to 16 byte boundary */
#} ERRORTRAP_DATA_I960_TRACE_REGISTERS,
#
#      
##############################################################################
sub FmtCCBETRegs
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
    my $item9;
    my $msg = "\nERRORTRAP_DATA_I960_TRACE_REGISTERS:\n\n";
    my $str;
    my %info;

    #    /* 8-bit registers */
    #    UINT8 statusGpodReg;    /* GPOD Output Data Register */
    #    UINT8 reserved0[15];    /* Align to 16 byte boundary */
    #

    $fmt = sprintf("x%d CCS LLL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("statusGpodReg  0x%02X\n", $item1);
#    $msg .= sprintf("                 reserved: 0x%02x 0x%04x 0x%0x    \n",$item2, $item3, $item4);
#    $msg .= sprintf("                 reserved: 0x%08x 0x%08x    \n",$item5, $item6);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    /* 16-bit registers */
    #    UINT16 nmiPatusrReg;    /* Primary ATU Status Register      */                
    #    UINT16 nmiSatusrReg;    /* Secondary ATU Status Register    */                    
    #
    #    UINT16 nmiPcrReg;       /* Primary Command Register        */
    #    UINT16 nmiBcrReg;       /* Bridge Control Register         */

    #    UINT16 nmiPsrReg;       /* Primary Status Register         */
    #    UINT16 nmiSsrReg;       /* Secondary Status Register        */

    #    UINT16 nmiSderReg;      /* Secondary Decode Enable Register */
    #    UINT16 nmiPatucmdReg;   /* Primary ATU Command Register     */

    #    UINT16 nmiSatucmdReg;   /* Secondary ATU Command Register   */
    #    UINT8  reserved1[14];   /* Align to 16 byte boundary       */
    #
    #
    $fmt = sprintf("x%d SS SS SS SS  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("nmiPatusrReg   0x%04X\n", $item1);
    $msg .= sprintf("nmiSatusrReg   0x%04X\n", $item2);
    $msg .= sprintf("nmiPcrReg      0x%04X\n", $item3);
    $msg .= sprintf("nmiBcrReg      0x%04X\n", $item4);

    $msg .= sprintf("nmiPsrReg      0x%04X\n", $item5);
    $msg .= sprintf("nmiSsrReg      0x%04X\n", $item6);
    $msg .= sprintf("nmiSderReg     0x%04X\n", $item7);
    $msg .= sprintf("nmiPatucmdReg  0x%04X\n", $item8);
    $offset += 16;                         #   bytes processed
    $address += 16;                  

    $fmt = sprintf("x%d SS L LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiSatucmdReg  0x%04X\n", $item1);
#    $msg .= sprintf("                 reserved: 0x%04x 0x%08x  \n",$item2, $item3);
#    $msg .= sprintf("                 reserved: 0x%08x  ",$item4);
#    $msg .= sprintf("                 reserved: 0x%08x     \n",$item5);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    /* 32-bit registers */
    #    UINT32 nmiNisrReg;      /* nmi NMI Interrupt Status Register            */
    #
    #    UINT32 nmiPatuimrReg;   /* nmi Primary ATU Interrupt Mask Register      */
    #    UINT32 nmiSatuimrReg;   /* nmi Secondary ATU Interrupt Mask Register    */
    #    UINT32 nmiAtucrReg;     /* nmi ATU Configuration Register               */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiNisrReg     0x%08X\n", $item1);
    $msg .= sprintf("nmiPatuimrReg  0x%08X\n", $item2);
    $msg .= sprintf("nmiSatuimrReg  0x%08X\n", $item3);
    $msg .= sprintf("nmiAtucrReg    0x%08X\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiMcisrReg;     /* nmi bit 0, Memory Controller Interrupt Status Register   */
    #    UINT32 nmiPatuisrReg;   /* nmi bit 1, Primary ATU Interrupt Status Register         */
    #    UINT32 nmiSatuisrReg;   /* nmi bit 2, Secondary ATU Interrupt Status Register       */
    #    UINT32 nmiPbisrReg;     /* nmi bit 3, Primary Bridge Interrupt Status Register      */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiMcisrReg    0x%08X\n", $item1);
    $msg .= sprintf("nmiPatuisrReg  0x%08X\n", $item2);
    $msg .= sprintf("nmiSatuisrReg  0x%08X\n", $item3);
    $msg .= sprintf("nmiPbisrReg    0x%08X\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiSbisrReg;     /* nmi bit 4, Secondary Bridge Interrupt Status Register    */
    #    UINT32 nmiCsr0Reg;      /* nmi bit 5, Channel Status Register           */
    #    UINT32 nmiCsr1Reg;      /* nmi bit 6, Channel Status Register           */
    #    UINT32 nmiCsr2Reg;      /* nmi bit 7, Channel Status Register           */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiSbisrReg    0x%08X\n", $item1);
    $msg .= sprintf("nmiCsr0Reg     0x%08X\n", $item2);
    $msg .= sprintf("nmiCsr1Reg     0x%08X\n", $item3);
    $msg .= sprintf("nmiCsr2Reg     0x%08X\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiIisrReg;      /* nmi bit 8 / 9 is external, Inbound Interrupt Status Register */
    #    UINT32 nmiAsrReg;       /* nmi bit 10, Accelerator Status Register      */
    #    UINT32 nmiBiuisrReg;    /* nmi bit 11, BIU Interrupt Status Register    */
    #                                                                          
    #    UINT32 nmiEccrReg;      /* ECC Control Register     */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiIisrReg     0x%08X\n", $item1);
    $msg .= sprintf("nmiAsrReg      0x%08X\n", $item2);
    $msg .= sprintf("nmiBiuisrReg   0x%08X\n", $item3);
    $msg .= sprintf("nmiEccrReg     0x%08X\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiEctstReg;     /* ECC Test Register        */
    #
    #    UINT32 nmiElog0Reg;     /* ECC Log Registers        */
    #    UINT32 nmiElog1Reg;     /* ECC Log Registers        */
    #    UINT32 nmiEcar0Reg;     /* ECC Address Registers    */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiEctstReg    0x%08X\n", $item1);
    $msg .= sprintf("nmiElog0Reg    0x%08X\n", $item2);
    $msg .= sprintf("nmiElog1Reg    0x%08X\n", $item3);
    $msg .= sprintf("nmiEcar0Reg    0x%08X\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiEcar1Reg;     /* ECC Address Registers    */
    #    
    #    UINT8  reserved2[12];   /* Align to 16 byte boundary */
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiEcar1Reg    0x%08X\n", $item1);
#    $msg .= sprintf("reserved: 0x%08x  \n",$item2);
#    $msg .= sprintf("reserved: 0x%08x  ",$item3);
#    $msg .= sprintf("reserved: 0x%08x     \n",$item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     FmtCCBETErrCnts
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    UINT32 nmiFwFaultCnt;   /* totala NMI count          */
#
#    UINT32 nmiBrkCnt;       /*                           */
#    UINT32 nmiUnexpCnt;     /*                           */
#
#    UINT32 nmiMceCnt;       /* bit0, MCU                 */
#    UINT32 nmiPaeCnt;       /* bit1                      */
#    UINT32 nmiSaeCnt;       /* bit2                      */
#    UINT32 nmiPbieCnt;      /* bit3                      */
#
#    UINT32 nmiSbeCnt;       /* bit4                      */
#    UINT32 nmiDmac0eCnt;    /* bit5                      */
#    UINT32 nmiDmac1eCnt;    /* bit6                      */
#    UINT32 nmiDmac2eCnt;    /* bit7                      */
#
#    UINT32 nmiMuiCnt;       /* bit8                      */
#    UINT32 nmiEniCnt;       /* bit9                      */
#    UINT32 nmiAaueCnt;      /* bit10                     */
#    UINT32 nmiBiueCnt;      /* bit11, bus interface err. */
#
#    UINT32 nmiEccSglCnt;    /* counts single ecc         */
#    UINT32 nmiEccMulCnt;    /* counts multi ecc          */
#    UINT32 nmiEccNotCnt;    /* counts non logged ecc     */
#    
#    UINT8 reserved[8];      /* Align to 16 byte boundary */
#} ERRORTRAP_DATA_ERROR_COUNTERS,
#      
##############################################################################
sub FmtCCBETErrCnts
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
    my $item9;
    my $msg = "\nERRORTRAP_DATA_ERROR_COUNTERS:\n\n";
    my $str;
    my %info;

    #    UINT32 nmiFwFaultCnt;   /* totala NMI count          */
    #
    #    UINT32 nmiBrkCnt;       /*                           */
    #    UINT32 nmiUnexpCnt;     /*                           */
    #
    #    UINT32 nmiMceCnt;       /* bit0, MCU                 */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiFwFaultCnt  0x%08x\n", $item1);
    $msg .= sprintf("nmiBrkCnt      0x%08x\n", $item2);
    $msg .= sprintf("nmiUnexpCnt    0x%08x\n", $item3);
    $msg .= sprintf("nmiMceCnt      0x%08x\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiPaeCnt;       /* bit1                      */
    #    UINT32 nmiSaeCnt;       /* bit2                      */
    #    UINT32 nmiPbieCnt;      /* bit3                      */
    #
    #    UINT32 nmiSbeCnt;       /* bit4                      */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiPaeCnt      0x%08x\n", $item1);
    $msg .= sprintf("nmiSaeCnt      0x%08x\n", $item2);
    $msg .= sprintf("nmiPbieCnt     0x%08x\n", $item3);
    $msg .= sprintf("nmiSbeCnt      0x%08x\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiDmac0eCnt;    /* bit5                      */
    #    UINT32 nmiDmac1eCnt;    /* bit6                      */
    #    UINT32 nmiDmac2eCnt;    /* bit7                      */
    #
    #    UINT32 nmiMuiCnt;       /* bit8                      */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiDmac0eCnt   0x%08x\n", $item1);
    $msg .= sprintf("nmiDmac1eCnt   0x%08x\n", $item2);
    $msg .= sprintf("nmiDmac2eCnt   0x%08x\n", $item3);
    $msg .= sprintf("nmiMuiCnt      0x%08x\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiEniCnt;       /* bit9                      */
    #    UINT32 nmiAaueCnt;      /* bit10                     */
    #    UINT32 nmiBiueCnt;      /* bit11, bus interface err. */
    #
    #    UINT32 nmiEccSglCnt;    /* counts single ecc         */
    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiEniCnt      0x%08x\n", $item1);
    $msg .= sprintf("nmiAaueCnt     0x%08x\n", $item2);
    $msg .= sprintf("nmiBiueCnt     0x%08x\n", $item3);
    $msg .= sprintf("nmiEccSglCnt   0x%08x\n", $item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    #    UINT32 nmiEccMulCnt;    /* counts multi ecc          */
    #    UINT32 nmiEccNotCnt;    /* counts non logged ecc     */
    #    
    #    UINT8 reserved[8];      /* Align to 16 byte boundary */

    #
    $fmt = sprintf("x%d LL LL  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("nmiEccMulCnt   0x%08x\n", $item1);
    $msg .= sprintf("nmiEccNotCnt   0x%08x\n", $item2);
#    $msg .= sprintf("                 reserved: 0x%08x  ",$item3);
#    $msg .= sprintf("                 reserved: 0x%08x  \n",$item4);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     FmtCCBStats
#
# Desc:     Formats CCB statistics (heap, link layer, ethernet etc)
#           into a string
#
#           /* CCB statistics (heap, link layer, ethernet etc) */
#           CCB_STATS_STRUCTURE ccbStatistics;
#           typedef struct _CCB_STATS_STRUCTURE
#           {
#               UINT32 hrcount; /* Kernel ticks                 */
#
#               /* Heap Statistics */
#               UINT32 memavl;  /* Total DRAM available         */
#               UINT32 minavl;  /* Minimum DRAM available       */
#               UINT32 maxavl;  /* Maximum DRAM available       */
#               UINT32 memwait; /* Total memory waits           */
#               UINT32 memcnt;  /* Outstanding malloc count     */
#
#               /* ILT Statistics */
#               UINT32 iltcnt;  /* Total ILTs available         */
#               UINT32 iltmax;  /* Maximum ILTs available       */
#
#               /* PCB Statistics */
#               UINT32 pcbcnt;  /* Number of active tasks       */
#
#               /* FW Version - why(?) */
#               UINT32 fwvers;  /* Firmware version number      */
#               UINT32 fwcomp;  /* Firmware revision count      */
#
#               /* Ethernet Statistics */
#               UINT32 psent;   /* Ethernet packets sent        */
#               UINT32 precv;   /* Ethernet packets received    */
#               ETHERNET_STATISTIC_COUNTERS ethernetCounters;
#
#               /* Link Layer Statistics */
#               UINT16 vrpOCount; /* outstanding outbound VRP count */
#               UINT16 vrpICount; /* outstanding inbound VRP count  */
#               UINT32 vrpOTotal; /* total outbound VRP count       */
#               UINT32 vrpITotal; /* total inbound VRP count        */
#    
#           } CCB_STATS_STRUCTURE, *CCB_STATS_STRUCTURE_PTR;
#
#      
##############################################################################
sub FmtCCBStats
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
    my $item9;
    my $msg = "\nERRORTRAP_CCB_STATISTICS:\n\n";
    my $str;
    my %info;

    #   UINT32 hrcount; /* Kernel ticks                 */
    #   
    #   /* Heap Statistics */
    #   UINT32 memavl;  /* Total DRAM available         */
    #   UINT32 minavl;  /* Minimum DRAM available       */
    #   UINT32 maxavl;  /* Maximum DRAM available       */
    #   UINT32 memwait; /* Total memory waits           */
    #   UINT32 memcnt;  /* Outstanding malloc count     */
    #
    $fmt = sprintf("x%d L LLLLL  ", $offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("Uptime                 ");
    $msg .= Uptime($item1>>3);
    $msg .= sprintf("\nHeap \n");
    $msg .= sprintf("  Heap Size                    %lu\n", $item4);
    $msg .= sprintf("  Cur Available                %lu\n", $item2);
    $msg .= sprintf("  Low Water Mark               %lu\n", $item3);
    $msg .= sprintf("  Wait Count                   %lu\n", $item5);
    $msg .= sprintf("  Outstanding Mallocs          %lu\n", $item6);

    $offset += 24;                         #   bytes processed
    $address += 24;                  

    #   /* ILT Statistics */
    #   UINT32 iltcnt;  /* Total ILTs available         */
    #   UINT32 iltmax;  /* Maximum ILTs available       */
    #
    #   /* PCB Statistics */
    #   UINT32 pcbcnt;  /* Number of active tasks       */
    #
    $fmt = sprintf("x%d LL L  ", $offset);   # generate the format string
    ($item1, $item2, $item3) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("ILTs\n");
    $msg .= sprintf("  Total ILTs Available         %lu\n", $item1);
    $msg .= sprintf("  Maximum ILTs Used            %lu\n", $item2);
    
    $msg .= sprintf("Tasks\n");
    $msg .= sprintf("  Active Task Count            %lu\n", $item3);
    
    $offset += 12;                         #   bytes processed
    $address += 12;                  
    
    
    #   /* FW Version - why(?) */
    #   UINT32 fwvers;  /* Firmware version number      */
    #   UINT32 fwcomp;  /* Firmware revision count      */
    #
    $fmt = sprintf("x%d CCCC CCCC  ", $offset);   # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
                        
    my $msgFW = sprintf("Firmware\n");
    $msgFW .= sprintf("  Version                      %c%c%c%c\n", 
                    $item1, $item2, $item3, $item4);
    $msgFW .= sprintf("  Revision                     %c%c%c%c\n", 
                    $item5, $item6, $item7, $item8);

    $offset += 8;                         #   bytes processed
    $address += 8;  
                    

    #   /* Ethernet Statistics */
    #   UINT32 psent;   /* Ethernet packets sent        */
    #   UINT32 precv;   /* Ethernet packets received    */
    #
    $fmt = sprintf("x%d LL ", $offset);      # generate the format string
    ($item1, $item2) = unpack $fmt , $$bufferPtr;

    my $msgEth = sprintf("\nEthernet Statistics\n");
    $msgEth .= sprintf("    Packets Sent (BF only)     %lu\n", $item1);
    $msgEth .= sprintf("    Packets Received (BF only) %lu\n", $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;                  


    #   /* Statistical Counters structure */
    #   typedef struct ETHERNET_STATISTIC_COUNTERS_STRUCT
    #   {
    #       UINT32 transmitGoodFrames;                      /* Bytes 0-3   */
    #       UINT32 transmitMaximumCollisions;               /* Bytes 4-7   */
    #       UINT32 transmitLateCollisions;                  /* Bytes 8-11  */
    #       UINT32 transmitUnderrunErrors;                  /* Bytes 12-15 */
    #       UINT32 transmitLostCarrierSense;                /* Bytes 16-19 */
    #       UINT32 transmitDeferred;                        /* Bytes 20-23 */
    #       UINT32 transmitSingleCollisions;                /* Bytes 24-27 */
    #       UINT32 transmitMultipleCollisions;              /* Bytes 28-31 */
    #       UINT32 transmitTotalCollisions;                 /* Bytes 32-35 */
    #   
    #       UINT32 receiveGoodFrames;                       /* Bytes 36-39 */
    #       UINT32 receiveCRCErrors;                        /* Bytes 40-43 */
    #       UINT32 receiveAlignmentErrors;                  /* Bytes 44-47 */
    #       UINT32 receiveResourceErrors;                   /* Bytes 48-51 */
    #       UINT32 receiveOverrunErrors;                    /* Bytes 52-55 */
    #       UINT32 receiveCollisionDetectErrors;            /* Bytes 56-59 */
    #       UINT32 receiveShortFrameErrors;                 /* Bytes 60-63 */
    #   
    #       /* Extended Counters - Configure byte 6, bit 5 set to zero */
    #       UINT32 flowControlTransmitPause;                /* Bytes 64-67 */
    #       UINT32 flowControlReceivePause;                 /* Bytes 68-71 */
    #       UINT32 flowControlReceiveUnsupported;           /* Bytes 72-75 */
    #   
    #       /* Command unit dump complete field */
    #       UINT32 dumpStatus;                              /* Bytes 76-79 */
    #   } ETHERNET_STATISTIC_COUNTERS, *ETHERNET_STATISTIC_COUNTERS_PTR;
    #   

    # Figure out if we are HN or BF
    $fmt = sprintf("x%d Z8 Z8", $offset);      # generate the format string
    ($item1, $item2) = unpack $fmt, $$bufferPtr;

    if ($item1 =~ /ETHST01/)  # New HyperNode format
    {
        $msgEth .= sprintf("  EyeCatcher                   %s\n", $item1);
        $msgEth .= sprintf("  Interface                    %s\n", $item2);
        
        $offset += 16;                         #   bytes processed
        $address += 16;                  

        $fmt = sprintf("x%d LLLL LLLL", $offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, 
         $item6, $item7, $item8 ) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  == Receive ==\n");
        $msgEth .= sprintf("    Bytes:                     %lu\n", $item1);
        $msgEth .= sprintf("    Packets:                   %lu\n", $item2);
        $msgEth .= sprintf("    Errors:                    %lu\n", $item3);
        $msgEth .= sprintf("    Dropped:                   %lu\n", $item4);
        $msgEth .= sprintf("    Overrun:                   %lu\n", $item5);
        $msgEth .= sprintf("    Frame:                     %lu\n", $item6);
        $msgEth .= sprintf("    Compressed:                %lu\n", $item7);
        $msgEth .= sprintf("    Multicast:                 %lu\n", $item8);

        $offset += 32;                         #   bytes processed
        $address += 32;                  

        $fmt = sprintf("x%d LLLL LLLL", $offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, 
         $item6, $item7, $item8 ) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  == Transmit ==\n");
        $msgEth .= sprintf("    Bytes:                     %lu\n", $item1);
        $msgEth .= sprintf("    Packets:                   %lu\n", $item2);
        $msgEth .= sprintf("    Errors:                    %lu\n", $item3);
        $msgEth .= sprintf("    Dropped:                   %lu\n", $item4);
        $msgEth .= sprintf("    Overrun:                   %lu\n", $item5);
        $msgEth .= sprintf("    Collisions:                %lu\n", $item6);
        $msgEth .= sprintf("    Carrier:                   %lu\n", $item7);
        $msgEth .= sprintf("    Compressed:                %lu\n", $item8);

        $offset += 32;                         #   bytes processed
        $address += 32;                  
    }
    else # Old BF format
    {
        $fmt = sprintf("x%d LLL LLL LLL ", $offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, 
         $item6, $item7, $item8, $item9, ) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  Transmit\n");
        $msgEth .= sprintf("    Good Frames                %lu\n", $item1);
        $msgEth .= sprintf("    Maximum Collisions         %lu\n", $item2);
        $msgEth .= sprintf("    Late Collisions            %lu\n", $item3);
        $msgEth .= sprintf("    Underrun Errors            %lu\n", $item4);
        $msgEth .= sprintf("    Lost Carrier Sense         %lu\n", $item5);
        $msgEth .= sprintf("    Deferred                   %lu\n", $item6);
        $msgEth .= sprintf("    Single Collisions          %lu\n", $item7);
        $msgEth .= sprintf("    Multiple Collisions        %lu\n", $item8);
        $msgEth .= sprintf("    Total Collisions           %lu\n", $item9);

        $offset += 36;                         #   bytes processed
        $address += 36;                  


        $fmt = sprintf("x%d LLL LLL L ", $offset);      # generate the format string
        ($item1, $item2, $item3, $item4, $item5, 
         $item6, $item7) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  Receive\n");
        $msgEth .= sprintf("    Good Frames                %lu\n", $item1);
        $msgEth .= sprintf("    CRC Errors                 %lu\n", $item2);
        $msgEth .= sprintf("    Alignment Errors           %lu\n", $item3);
        $msgEth .= sprintf("    Resource Errors            %lu\n", $item4);
        $msgEth .= sprintf("    Overrun Errors             %lu\n", $item5);
        $msgEth .= sprintf("    Collision Detect Errors    %lu\n", $item6);
        $msgEth .= sprintf("    Short Frame Errors         %lu\n", $item7);

        $offset += 28;                         #   bytes processed
        $address += 28;                  


        $fmt = sprintf("x%d LLL ", $offset);      # generate the format string
        ($item1, $item2, $item3) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  Flow Control\n");
        $msgEth .= sprintf("    Transmit Pause             %lu\n", $item1);
        $msgEth .= sprintf("    Receive Pause              %lu\n", $item2);
        $msgEth .= sprintf("    Receive Unsupported        %lu\n", $item3);

        $offset += 12;                         #   bytes processed
        $address += 12;                  


        $fmt = sprintf("x%d L ", $offset);      # generate the format string
        ($item1) = unpack $fmt , $$bufferPtr;

        $msgEth .= sprintf("  Dump Status                  0x%08X\n", $item1);

        $offset += 4;                         #   bytes processed
        $address += 4;   
    }


    #   /* Link Layer Statistics */
    #   UINT16 vrpOCount;           /* outstanding outbound VRP count */
    #   UINT16 vrpICount;           /* outstanding inbound VRP count  */
    #   UINT32 vrpOTotal;           /* total outbound VRP count       */
    #   UINT32 vrpITotal;           /* total inbound VRP count        */
    #
    $fmt = sprintf("x%d SS LL ", $offset);      # generate the format string
    ($item1, $item2, $item3, $item4) = unpack $fmt , $$bufferPtr;

    my $msgLL = sprintf("Link Layer\n");
    $msgLL .= sprintf("  Outbound VRP Count           %lu\n", $item1);
    $msgLL .= sprintf("  Inbound VRP Count            %lu\n", $item2);
    $msgLL .= sprintf("  Total VRPs Sent              %lu\n", $item3);
    $msgLL .= sprintf("  Total VRPs Received          %lu\n", $item4);

    $offset += 12;                         #   bytes processed
    $address += 12;                  


    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;      # append to callers item
        $$destPtr .= $msgLL;         
        $$destPtr .= $msgFW;         
        $$destPtr .= $msgEth;        
    }

    return GOOD;
}

##############################################################################
# Name:     Uptime
#
# Desc:     Convert kernel ticks to system up time
#
##############################################################################
sub Uptime
{
    my ($upSec) = @_; 

    my ($days, $hours, $min);

    $days = int($upSec / 86400);
    $upSec -= $days * 86400;

    $hours = int($upSec / 3600);
    $upSec -= $hours * 3600;

    $min = int($upSec / 60);
    $upSec -= $min * 60;

    return sprintf "%u day(s) %u hrs %u min %u sec", $days, $hours, $min, $upSec;
}

##############################################################################
# Name:     FmtGandRRegs
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    UINT32 rRegisters[I960_NUM_R_REGS];
#    UINT32 gRegisters[I960_NUM_G_REGS];
#} ERRORTRAP_DATA_CPU_REGISTERS,
##      
##############################################################################
sub FmtGandRRegs
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $fmt;
    my $msg = "\n";
    
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
    
    
    ########################
    $msg .=  "\n--- ERRORTRAP_DATA_CPU_REGISTERS: ------------------------------\n";

    # g0-7
    $fmt = sprintf( "x%d LLLL LLLL",       $offset +  0x00 );
    ($g0, $g1, $g2, $g3, $g4, $g5, $g6, $g7) = unpack $fmt, $$bufferPtr;

    # g8-fp
    $fmt = sprintf( "x%d LLLL LLLL",       $offset +  0x20 );
    ($g8, $g9, $g10, $g11, $g12, $g13, $g14, $fp) = unpack $fmt, $$bufferPtr;

    # pfp - r7
    $fmt = sprintf( "x%d LLLL LLLL",       $offset +  0x40 );
    ($pfp, $sp, $rip, $r3, $r4, $r5, $r6, $r7) = unpack $fmt, $$bufferPtr;

    # r8-r15
    $fmt = sprintf( "x%d LLLL LLLL",       $offset +  0x60 );
    ($r8, $r9, $r10, $r11, $r12, $r13, $r14, $r15) = unpack $fmt, $$bufferPtr;

    $msg .= sprintf("\n  Registers:\n".
                    "  pfp: %08X      g0: %08X\n".
                    "   sp: %08X      g1: %08X\n".
                    "  rip: %08X      g2: %08X\n".
                    "   r3: %08X      g3: %08X\n".
                    "   r4: %08X      g4: %08X\n".
                    "   r5: %08X      g5: %08X\n".
                    "   r6: %08X      g6: %08X\n".
                    "   r7: %08X      g7: %08X\n".
                    "   r8: %08X      g8: %08X\n".
                    "   r9: %08X      g9: %08X\n".
                    "  r10: %08X     g10: %08X\n".
                    "  r11: %08X     g11: %08X\n".
                    "  r12: %08X     g12: %08X\n".
                    "  r13: %08X     g13: %08X\n".
                    "  r14: %08X     g14: %08X\n".
                    "  r15: %08X      fp: %08X\n",
                    $pfp, $g0, $sp,  $g1, $rip, $g2, $r3,  $g3,
                    $r4,  $g4, $r5,  $g5, $r6,  $g6, $r7,  $g7,
                    $r8,  $g8, $r9,  $g9, $r10, $g10, $r11, $g11,
                    $r12, $g12, $r13, $g13, $r14, $g14, $r15, $fp);

    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     FmtBitmap
#
# Desc:     Formats a bitmap into a list 
#
# Inputs:   reqLength   Number of bytes in the map
#
#
#      
##############################################################################
sub FmtBitmap
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    
    my $fmt;
    my $item1;
    my $msg;
    my @bitmap;
    

    # Unpack the number of bytes specified by reqLength    
    $fmt = sprintf("x%d a%d", $offset, $reqLength);     # generate the format string
    ($item1) = unpack $fmt , $$bufferPtr;

    @bitmap = XIOTech::cmdMgr::ParseBitmap($item1);
    $msg = sprintf("@bitmap");
    

    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     BCDToHex
#
# Desc:     Convert BCD number to Hex.
#
##
##############################################################################
sub BCDToHex
{
    my ($n) = @_;
    my $rc = 0;

    for(my $i = 0; $i < 4; $i++)
    {
        $rc += ($n & 0xF) * (10 ** $i);
        $n >>= 4;
    }

    return $rc;
}

##############################################################################
# Name:     FmtCCBETSnap
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    /* Timestamp of error */
#    TIMESTAMP timestamp;
#
#    /* Important parts of the firmware header of code that's in error */
#    FW_DATA firmwareRevisionData;
#
#    /* G and R registers at time of error */
#    ERRORTRAP_DATA_CPU_REGISTERS cpuRegisters;
#
#    /* MACH registers for Zion CCB at time of error */
#    MACH machRegisters;
#    UINT8 machRegistersPad[8];  /* Pad to align back out to 16 bytes */
#    
#    /* Important registers inside the i960 we'd like to trace */
#    ERRORTRAP_DATA_I960_TRACE_REGISTERS traceRegisters;
#
#    /* Call stack at time of error */
#    INT8 callStack[CALLSTACK_SIZE];
#
#    /* CRC to validate snapshot data */
#    UINT8 reserved1[12]; /* Pad to align back out to 16 bytes */
#    UINT32 snapshotDataCRC;
#} ERRORTRAP_DATA_ERROR_SNAPSHOT,
#      
#
# Return:   Timestamp in Epoch time.
#
##############################################################################
sub FmtCCBETSnap
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
    my $item9;
    my $msg = "\nERRORTRAP_DATA_ERROR_SNAPSHOT:\n\n";
    my $str;
    my $timeStamp;
    my $ret;


    #    /* Timestamp of error */
    #    TIMESTAMP timestamp;
    #    typedef struct
    #    {
    #        UINT16  year;               /* Year 0 -9999                         */
    #        UINT8   month;              /* Month 1 -12                          */
    #        UINT8   date;               /* Day of the month 1 - 31              */

    #        UINT8   day;                /* Day of the week 1 - 7 (1 = Sunday)   */
    #        UINT8   hours;              /* Hour 0 - 23     (0 = midnight)       */
    #        UINT8   minutes;            /* Minutes 0 - 59                       */
    #        UINT8   seconds;            /* Seconds 0 - 59                       */

    #        UINT32  systemSeconds;      /* Seconds the system has been running  */
    #    } TIMESTAMP, *TIMESTAMP_PTR;
    #
    #
    my @days = ( "Holiday",
                 "Sun",
                 "Mon",
                 "Tue",
                 "Wed",
                 "Thu",
                 "Fri",
                 "Sat");
    $fmt = sprintf( "x%d SCC CCCC L",     $offset  );

    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
        unpack $fmt , $$bufferPtr;

    # Timestamp is in BCD.  Some fix-ups are done below.  Most of these are
    # done so timegm() doesn't blow up.

    $item7 = BCDToHex($item7);
    $item6 = BCDToHex($item6);
    $item5 = BCDToHex($item5);
    $item3 = BCDToHex($item3);
    $item2 = BCDToHex($item2);
    $item1 = BCDToHex($item1);
    
    # Invalid day is forced to "Holiday"
    if ( $item4 > 7 || $item4 < 0 ) 
    { 
        $item4 = 0; 
    }

    # Invalid year is forced to 1970
    if (($item1 < 1970) or ($item1 > 2100))
    { 
        $item1 = 1970; 
    }

    # Invalid month is forced to January
    if (($item2 <= 0) or ($item2 > 12)) 
    { 
        $item2 = 1; 
    }

    # Invalid day of month is forced to 1
    if (($item3 <= 0) or ($item3 > 31)) 
    { 
        $item3 = 1; 
    }

    # Invalid hour is forced to 0
    if (($item5 < 0) or ($item5 > 23)) 
    { 
        $item5 = 0; 
    }

    # Invalid minute is forced to 0
    if (($item6 < 0) or ($item6 > 59)) 
    { 
        $item6 = 0; 
    }

    # Invalid second is forced to 0
    if (($item7 < 0) or ($item7 > 59))
    { 
        $item7 = 0; 
    }

    # Convert the time stamp to Epoch time.  This will allow for easy
    # comparison between time stamps of multiple NVRAM snapshots.
    $timeStamp = timegm($item7, $item6, $item5, $item3, $item2-1, $item1); 

    $msg .= sprintf("Time of snapshot   %s  %02u-%02u-%04u  %02u:%02u:%02u\n",
        $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);

    $msg .= sprintf("System Uptime      ");
    $msg .= Uptime($item8);
    $msg .= "\n";

    $offset += 12;                         #   bytes processed
    $address += 12;                  


    #    /* Important parts of the firmware header of code that's in error */
    #    FW_DATA firmwareRevisionData;
    #
    #    typedef struct {
    #       UINT32 revision;                        /* Firmware Revision                */
    #       UINT32 revCount;                        /* Firmware Revision Counter        */
    #       UINT32 buildID;                         /* Who / where firmware was built   */
    #       FW_HEADER_TIMESTAMP timeStamp;          /* Time Firmware was built          */
    #    } FW_DATA, *FW_DATA_PTR;
    #    typedef struct FW_HEADER_TIMESTAMP_STRUCT
    #    {
    #        UINT16 year;
    #        UINT8  month;
    #        UINT8  date;
    #        UINT8  day;
    #        UINT8  hours;
    #        UINT8  minutes;
    #        UINT8  seconds;
    #    } FW_HEADER_TIMESTAMP, *FW_HEADER_TIMESTAMP_PTR;
    #
    #
    $fmt = sprintf("x%d A4A4A4",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

#    $msg .= sprintf( "revision       $item1\n");
#    $msg .= sprintf( "revCount       $item2\n");
#    $msg .= sprintf( "buildID        $item3\n");
    
    
    $msg .= sprintf("CCB FW Version     $item1 $item2 $item3 "); 
    
    $offset += 12;                         # add bytes processed
    $address += 12;                  


    $fmt = sprintf( "x%d SCC CCCC",     $offset  );

    ($item1, $item2, $item3, $item4, $item5, $item6, $item7) =  
        unpack $fmt , $$bufferPtr;

    # Invalid days are forced to "Holiday"
    if ( $item4 > 7 || $item4 < 0 ) 
    { 
        $item4 = 0; 
    }

#    $msg .= sprintf( "Timestamp      %s  %02X-%02X-%04X  %02X:%02X:%02X\n",
#        $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);


    # The FW Version timestamp is displayed as part of the CCB FW Version
    # line above.
    $msg .= sprintf("%s  %02X-%02X-%04X  %02X:%02X:%02X\n",
        $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);


    $offset += 8;                         #   bytes processed
    $address += 8;                  



    #    /* G and R registers at time of error */
    #    ERRORTRAP_DATA_CPU_REGISTERS cpuRegisters;
    #
    $ret = FmtGandRRegs( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 128;                         #   bytes processed
    $address += 128;                  


    #    /* MACH registers for Zion CCB at time of error */
    #    MACH machRegisters; 
    #    typedef volatile struct MACH_STRUCT
    #    {
    #        UINT8 systemStatus0;                /* Read / (Write to clear bit 0)    */
    #        UINT8 diagSwitchesStatus;           /* Read only    */
    #        UINT8 flashSwitchesStatus;          /* Read only    */
    #        UINT8 boardMachRevStatus;           /* Read only    */
    #        UINT8 frontPanelControl;            /* Read / Write */
    #        UINT8 miscControl;                  /* Read / Write */
    #        UINT8 heartbeatToggleControl;       /* Read / Write */
    #        UINT8 watchDogReTriggerControl;     /* Read / Write */
    #    } MACH, *MACH_PTR;
    #
    #    UINT8 machRegistersPad[8];  /* Pad to align back out to 16 bytes */
    #    
    $msg .= "\nMACH registers:\n\n";

    $fmt = sprintf("x%d CCCC CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8 ) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("systemStatus0             0x%02X\n", $item1);
    $msg .= sprintf("diagSwitchesStatus        0x%02X\n", $item2);
    $msg .= sprintf("flashSwitchesStatus       0x%02X\n", $item3);
    $msg .= sprintf("boardMachRevStatus        0x%02X\n", $item4);
    $msg .= sprintf("frontPanelControl         0x%02X\n", $item5);
    $msg .= sprintf("miscControl               0x%02X\n", $item6);
    $msg .= sprintf("heartbeatToggleControl    0x%02X\n", $item7);
    $msg .= sprintf("watchDogReTriggerControl  0x%02X\n", $item8);

    $offset += 8;                         #   bytes processed
    $address += 8;
                      
    $fmt = sprintf("x%d LL", $offset);      # generate the format string
    ($item1, $item2 ) = unpack $fmt , $$bufferPtr;
    # $msg .= sprintf("reserved: 0x%08x 0x%08x \n",  $item1, $item2);

    $offset += 8;                         #   bytes processed
    $address += 8;


    #    /* Important registers inside the i960 we'd like to trace */
    #    ERRORTRAP_DATA_I960_TRACE_REGISTERS traceRegisters;
    #
    $ret = FmtCCBETRegs( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 144;                         #   bytes processed
    $address += 144;                  


    #    /* Call stack at time of error */
    #    INT8 callStack[CALLSTACK_SIZE];
    #
    $msg .= "\nCallstack:\n";
    $msg .= FmtString( $bufferPtr, $offset, 1024 );
    $offset += 1024;                         #   bytes processed
    $address += 1024;                  


    #    /* CRC to validate snapshot data */
    #    UINT8 reserved1[12]; /* Pad to align back out to 16 bytes */
    #    UINT32 snapshotDataCRC;
    $msg .= "\n\n";

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) = unpack $fmt, $$bufferPtr;
    
#    $msg .= sprintf("  \n0x%08x:        reserved:  0x%08x     \n", $address, $item1);
#    $msg .= sprintf("                   reserved:  0x%08x     \n",$item2);
#    $msg .= sprintf("                   reserved:  0x%08x     \n",$item3);
    $msg .= sprintf("snapshotDataCRC  0x%08X\n",$item4);


    $offset += 16;                         #   bytes processed
    $address += 16;                  


    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return $timeStamp;

}

##############################################################################
# Name:     FmtCCBETBoot
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    /* Counters for tracking the quantity and type of errors */
#    ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
#
#    /* Snapshot of error */
#    ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
#
#    INT8 reservedSpace[SIZE_6K -
#        sizeof(ERRORTRAP_DATA_ERROR_COUNTERS) -
#        sizeof(ERRORTRAP_DATA_ERROR_SNAPSHOT) -
#        sizeof(RTC_STRUCTURE)];
#} ERRORTRAP_DATA_BOOT,
#      
##############################################################################
sub FmtCCBETBoot
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
    my $item9;
    my $msg = "\n\nERRORTRAP_DATA_BOOT:\n";
    my $str;
    my %info;
    my $ret;


#    /* Counters for tracking the quantity and type of errors */
#    ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
#
    $ret = FmtCCBETErrCnts( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 80;                         #   bytes processed
    $address += 80;                  


#    /* Snapshot of error */
#    ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
#

    $ret = FmtCCBETSnap ( \$msg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 1360;                         #   bytes processed
    $address += 1360;                  


#    INT8 reservedSpace[SIZE_6K -
#        sizeof(ERRORTRAP_DATA_ERROR_COUNTERS) -
#        sizeof(ERRORTRAP_DATA_ERROR_SNAPSHOT) -
#        sizeof(RTC_STRUCTURE)];

    $item1 = 6144 - 1440;

#    $msg .= "\nreserved space: \n\n";
    
#    $msg .= FmtDataString( $bufferPtr, $address, "word", $item1, $offset);

    $offset += $item1;                         #   bytes processed
    $address += $item1;                  


    # copy the data to the callers pointers
    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
# Name:     FmtCCBETDataRun
#
# Desc:     Formats a structure for the CCB into a string
#
#typedef struct
#{
#    /* Counters for tracking the quantity and type of errors */
#    ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
#
#    /* Snapshot of error */
#    ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
#
#    /* Runtime code trace data */
#    INT8 traceEvData[TRACEEVENT_SIZE];
#    
#    /* Runtime code trace data */
#    INT8 serialPortData[SERIALDATA_SIZE];
#
#    /* Misc reserved space (42K total struct size) */
#    INT8 reservedSpace[(SIZE_32K + SIZE_8K + SIZE_2K) -
#        sizeof(ERRORTRAP_DATA_ERROR_COUNTERS) -
#        sizeof(ERRORTRAP_DATA_ERROR_SNAPSHOT) -
#        TRACEEVENT_SIZE -
#        SERIALDATA_SIZE];
#} ERRORTRAP_DATA_RUN,
#      
##############################################################################
sub FmtCCBETDataRun
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
    my $item9;
    my $msg = "\n\nERRORTRAP_DATA_RUN: \n\n";
    my $str;
    my %info;
    my $ret;
    my $timeStamp;

    #   typedef struct
    #   {
    #       /* Counters for tracking the quantity and type of errors */
    #       ERRORTRAP_DATA_ERROR_COUNTERS errorCounters;
    #
    my $errCntMsg;
    $ret = FmtCCBETErrCnts( \$errCntMsg, 0, $bufferPtr, $offset, 0, $processor, $address);
    
    $offset += 80;                         #   bytes processed
    $address += 80;                  


    #       /* Snapshot of error */
    #       ERRORTRAP_DATA_ERROR_SNAPSHOT errorSnapshot;
    #
    my $errTrapMsg;
    $timeStamp = FmtCCBETSnap( \$errTrapMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 1360;                         #   bytes processed
    $address += 1360;                  
    
    $errTrapMsg .= "\n---------------------------------------------------------------------\n";


    #       /* Runtime code trace data */
    #       INT8 traceEvData[TRACEEVENT_SIZE];       20 K
    #    
    my $traceMsg = "Trace Event Data:\n\n";;
    # $msg .= "Trace Event Data:\n\n";
    # $msg .= FmtDataString( $bufferPtr, $address, "word", 20480, $offset);
    #XIOTech::decodeRings::FmtMrpTrcP( \$msg, $bufferPtr, $offset, 20480, $processor, $address );

    FmtCCBTracDec( \$traceMsg, 0, $bufferPtr, $offset, 20480, $processor, $address);
    

    $offset += 20480;                         #   bytes processed
    $address += 20480;                  
    
    $traceMsg .= "\n---------------------------------------------------------------------\n";


    #       /* Runtime code trace data */            20 k
    #       INT8 serialPortData[SERIALDATA_SIZE];
    #
    my $serialMsg .= "Serial Port Data:\n\n";

    $serialMsg .= FmtString( $bufferPtr, $offset, 20480 );

    $offset += 20480;                         #   bytes processed
    $address += 20480;                  


    #       /* CCB statistics (heap, link layer, ethernet etc) */
    #       CCB_STATS_STRUCTURE ccbStatistics;
    #
    $ret = FmtCCBStats( \$serialMsg, 0, $bufferPtr, $offset, 0, $processor, $address);

    $offset += 144;                         #   bytes processed
    $address += 144;                  

    #       /* Runtime Heap Statistics */
    #       HEAP_STATS_IN_NVRAM heapStatsNV;
    $ret = XIOTech::fmtFIDs::FmtCCBHeapStatsFID2( \$serialMsg, 0, $bufferPtr, $offset, 
            -1, # pass -1 for reqLength to indicate that we are coming from a NVRAM decode.  
            $processor, $address, 0);

    $serialMsg .= "\n---------------------------------------------------------------------\n";

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
        $$destPtr .= $errTrapMsg;
        $$destPtr .= $serialMsg;
        $$destPtr .= $traceMsg;
        $$destPtr .= $errCntMsg;
    }

    return $timeStamp;
}

##############################################################################
# Name:     FmtCCBCtlrSU
#
# Desc:     Formats a structure for the CCB into a string
#
#    typedef struct
#    {
#        UINT32      schema;                     /* schema version for setup       */
#        UINT8       rsvd1[4];                   /* RESERVED                       */
#        IP_ADDRESS  ipAddress;
#        IP_ADDRESS  gatewayAddress;
#        IP_ADDRESS  subnetMask;
#
#        UINT32      systemSN;                   /* System Serial Number           */
#        UINT32      controllerSN;               /* Controller Serial Number       */
#
#        UINT8       configFlags;                /* Configuration Flags - Bits     */
#                                                /* Bit 0 - License Applied        */
#                                                /* Bit 1 - Init Logs if 0         */
#                                                /* Bit 2 - Replacement Controller */
#                                                /* Bit 3 - RESERVED               */
#                                                /* Bit 4 - RESERVED               */
#                                                /* Bit 5 - RESERVED               */
#                                                /* Bit 6 - RESERVED               */
#                                                /* Bit 7 - RESERVED               */
#
#        UINT8       rsvd2[199];                 /* RESERVED                       */
#
#        UINT8       useDHCP;                    /* Flag 1= Use DHCP               */
#        UINT8       ethernetConfigured;         /* Flag 1= Ethernet configured    */
#
#        UINT8       rsvd3[18];                  /* RESERVED                       */
#    
#        IP_ADDRESS  debugConsoleIPAddr;         /* The address EtherWrite sends   */
#                                                /*   to, in network byte order    */
#    
#        UINT8       debugConsoleChannel;        /* This is the port offset for    */
#                                                /* EtherWrite to send data to     */
#                                                /* DEBUG_PORT_NUMBER +            */
#                                                /* debugConsoleChannel            */
#
#        UINT8       reserved[219];              /* RESERVED */
#        UINT32      crc;
#    } CONTROLLER_SETUP, *PCONTROLLER_SETUP;
#      
##############################################################################
sub FmtCCBCtlrSU
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
    my $item9;
    my $msg = "\nCONTROLLER_SETUP:\n\n";
    my $str;
    my %info;


    #        UINT32      schema;                     /* schema version for setup       */
    #        UINT8       rsvd1[4];                   /* RESERVED                       */
    
    $fmt = sprintf("x%d LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("schema                 0x%08x\n", $item1);
#    $msg .= sprintf("                    rsvd1:  0x%08x  \n",$item2);
    
    $offset += 8;                         #   bytes processed
    $address += 8;                  

    #        IP_ADDRESS  ipAddress;
    #        IP_ADDRESS  gatewayAddress;

    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("ipAddress              %d.%d.%d.%d\n",
                    $item1, $item2, $item3, $item4);
    $msg .= sprintf("gatewayAddress         %d.%d.%d.%d\n",
                    $item5, $item6, $item7, $item8);


    $offset += 8;                         #   bytes processed
    $address += 8;                  

    #        IP_ADDRESS  subnetMask;
    #
    $fmt = sprintf("x%d CCCC ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("subnetMask             %d.%d.%d.%d\n",
                    $item1, $item2, $item3, $item4);


    $offset += 4;                         #   bytes processed
    $address += 4;                  


    #        UINT32      systemSN;                   /* System Serial Number           */
    #        UINT32      controllerSN;               /* Controller Serial Number       */
    #
    #        UINT8       configFlags;                /* Configuration Flags - Bits     */
    #                                                /* Bit 0 - License Applied        */
    #                                                /* Bit 1 - Init Logs if 0         */
    #                                                /* Bit 2 - Replacement Controller */
    #                                                /* Bit 3 - RESERVED               */
    #                                                /* Bit 4 - RESERVED               */
    #                                                /* Bit 5 - RESERVED               */
    #                                                /* Bit 6 - RESERVED               */
    #                                                /* Bit 7 - RESERVED               */
    #

    $fmt = sprintf("x%d LL C ", $offset);   # generate the format string
    ($item1, $item2, $item3) =  unpack $fmt, $$bufferPtr;
    
    $msg .= sprintf("systemSN               %d\n", $item1);
    $msg .= sprintf("controllerSN           %d\n", $item2);
    $msg .= sprintf("configFlags            0x%02x\n", $item3);

    $offset += 9;                           #   bytes processed
    $address += 9;                  

    #   ISP_CONFIG  be;

    $fmt = sprintf("x%d C CCCC ", $offset);   # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  unpack $fmt, $$bufferPtr;

    $msg .= sprintf("BE port count:         %d\n", $item1);
    $msg .= sprintf("BE port pair settings: 0x%02x 0x%02x 0x%02x 0x%02x\n",
                $item2, $item3, $item4, $item5);

    $offset += 5;                           #   bytes processed
    $address += 5;                  

    #   ISP_CONFIG  fe;

    $fmt = sprintf("x%d C CCCC ", $offset);   # generate the format string
    ($item1, $item2, $item3, $item4, $item5) =  unpack $fmt, $$bufferPtr;

    $msg .= sprintf("FE port count:         %d\n", $item1);
    $msg .= sprintf("FE port pair settings: 0x%02x 0x%02x 0x%02x 0x%02x\n",
                $item2, $item3, $item4, $item5);

    $offset += 5;                           #   bytes processed
    $address += 5;                  


    #        UINT8       rsvd2[189];                 /* RESERVED           */
    #
#    $msg .= "\n 189 additional bytes rsvd2 \n";
#    $msg .= FmtDataString( $bufferPtr, $address, "word", 189, $offset);

    $offset += 189;                         #   bytes processed
    $address += 189;                  



#        UINT8       useDHCP;                    /* Flag 1= Use DHCP               */
#        UINT8       ethernetConfigured;         /* Flag 1= Ethernet configured    */
#
    $fmt = sprintf("x%d CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= "\n";
    $msg .= sprintf("useDHCP                0x%02x\n", $item1);
    $msg .= sprintf("ethernetConfigured     0x%02x\n",$item2);
#    $msg .= sprintf("                    rsvd3:  0x%02x  0x%02x \n", $item3, $item4);


    $offset += 4;                         #   bytes processed
    $address += 4;                  



#        UINT8       rsvd3[18];                  /* RESERVED                       */
#    
#    $msg .= "\n 16 additional bytes rsvd3 \n";
#    $msg .= FmtDataString( $bufferPtr, $address, "word", 16, $offset);
    $offset += 16;                         #   bytes processed
    $address += 16; 


#        IP_ADDRESS  debugConsoleIPAddr;         /* The address EtherWrite sends   */
#                                                /*   to, in network byte order    */
#    
    $fmt = sprintf("x%d CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= "\n";
    $msg .= sprintf("debugConsoleIPAddr     %d.%d.%d.%d\n",
                    $item1, $item2, $item3, $item4);


    $offset += 4;                         #   bytes processed
    $address += 4; 
                     

#        UINT8       debugConsoleChannel;        /* This is the port offset for    */
#                                                /* EtherWrite to send data to     */
    $fmt = sprintf("x%d C  ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("debugConsoleChannel    0x%02x\n", $item1);


    $offset += 1;                         #   bytes processed
    $address += 1;                  

#                                                /* DEBUG_PORT_NUMBER +            */
#                                                /* debugConsoleChannel            */
#
#        UINT8       reserved[219];              /* RESERVED */

#    $msg.= "\n 219 reserved bytes skipped \n\n";

    $offset += 219;                         #   bytes processed
    $address += 219;                  



#        UINT32      crc;
    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) = unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("crc                    0x%08x\n", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;

}

##############################################################################
# Name:     FmtCCBbty
#
# Desc:     Formats a structure for the CCB into a string
#
#      
#typedef struct I2C_NVRAM_STORAGE_STRUCT
#{
#    I2C_BATTERY_DATA frontEndBatteryData;
#    I2C_BATTERY_DATA backEndBatteryData;
#    UINT8 reserved[128 -                    /* Future growth - fill to 128 bytes */
#        (2 * sizeof(I2C_BATTERY_DATA)) -    /* Size of I2C_BATTERY_DATA structs */
#        sizeof(UINT32)];                    /* Space for crc */
#    UINT32 crc;
#} I2C_NVRAM_STORAGE, *I2C_NVRAM_STORAGE_PTR;
###############################################################################
sub FmtCCBbty
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
    my $item9;
    my $msg = "\nI2C_NVRAM_STORAGE:\n\n";
    my $str;
    my %info;

    my @days = ( "Holiday",
                 "Sun",
                 "Mon",
                 "Tue",
                 "Wed",
                 "Thu",
                 "Fri",
                 "Sat");

#   typedef struct I2C_NVRAM_STORAGE_STRUCT
#   {
#        I2C_BATTERY_DATA frontEndBatteryData;

#    TIMESTAMP  previousChargeTimestamp;
#    MILLIVOLTS batteryPeakMillivolts;         U16
#    UINT8 reserved[32 -                     /* Future growth - fill to 32 bytes */
#        sizeof(TIMESTAMP) -                 /* Size of previousChargeTimestamp  */
#        sizeof(MILLIVOLTS)];                /* Size of batteryPeakMillivolts    */
#
    $fmt = sprintf( "x%d SCC CCCC L",     $offset  );

    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
        unpack $fmt , $$bufferPtr;


    # Invalid days are forced to "Holiday"
    if ( $item4 > 7 || $item4 < 0 ) 
    { 
        $item4 = 0; 
    }

    $msg .= "frontEndBatteryData:\n\n";
    $msg .= sprintf("Timestamp              %s  %02X-%02X-%04X  %02X:%02X:%02X\n",
        $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);

    $msg .= sprintf("systemSeconds          0x%08x\n",$item8);

    $offset += 12;                         #   bytes processed
    $address += 12;                  

    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("batteryPeakMillivolts  0x%02x\n\n", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

#    $msg .= "\n 16 reserved bytes skipped \n\n";
    
    $offset += 16;                         #   bytes processed
    $address += 16;                  


#        I2C_BATTERY_DATA backEndBatteryData;

    $fmt = sprintf("x%d SCC CCCC L", $offset);

    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
        unpack $fmt , $$bufferPtr;


    # Invalid days are forced to "Holiday"
    if ( $item4 > 7 || $item4 < 0 ) 
    { 
        $item4 = 0; 
    }

    $msg .= "backEndBatteryData:\n\n";
    $msg .= sprintf("Timestamp              %s  %02X-%02X-%04X  %02X:%02X:%02X\n",
        $days[$item4], $item2, $item3, $item1, $item5, $item6, $item7);

    $msg .= sprintf("systemSeconds          0x%08x\n",$item8);

    $offset += 12;                         #   bytes processed
    $address += 12;                  

    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("batteryPeakMillivolts  0x%02x\n", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

#    $msg .= "\n 16 reserved bytes skipped \n\n";
    
    $offset += 16;                         #   bytes processed
    $address += 16;                  

#    $msg .= "\n 60 reserved bytes skipped \n\n";
    
    $offset += 60;                         #   bytes processed
    $address += 60;                  


#        UINT8 reserved[128 -                    /* Future growth - fill to 128 bytes */
#            (2 * sizeof(I2C_BATTERY_DATA)) -    /* Size of I2C_BATTERY_DATA structs */
#            sizeof(UINT32)];                    /* Space for crc */
#        UINT32 crc;
#   } I2C_NVRAM_STORAGE, *I2C_NVRAM_STORAGE_PTR;

    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("crc                    0x%08x\n", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;

}

##############################################################################
# Name:     FmtCCBTracDec
#
# Desc:     Formats a structure for the CCB into a string
#
#      
##############################################################################
sub FmtCCBTracDec
{
    my ( $destPtr, $hPtr, $bufferPtr, $offset, $reqLength, $processor, $address) = @_;
    my $version = 0;
    
    # copy the data to the callers pointers
    if ( $destPtr )
    {
        my ($hdrMagicNum, $hdrFid, $hdrVer) = unpack
            "x$offset " . DDR_FID_HEADER, $$bufferPtr;

        # Look for FID header
        if ($hdrMagicNum == DDR_FID_HEADER_MAGIC_NUM and $hdrFid == 256)
        {
            $offset += 32;
            $reqLength -= 32;

            $version = $hdrVer;
        }
        
        $$destPtr .= CCBDecoder(substr($$bufferPtr, $offset, $reqLength), 0, $version);
    }

    return GOOD;
}

##############################################################################
# Name:     FmtCCBQmMc
#
# Desc:     Formats a structure for the CCB into a string
#
#    typedef  struct _QM_MASTER_CONFIG
#    {
#        unsigned long   schema;                 /* schema version for quorum        */
#        unsigned long   magicNumber;            /* magic number for quorum          */
#        unsigned long   virtualControllerSN;    /* VCG System ID (licensed)         */
#        unsigned long   electionSerial;         /* election serial number           */
#        /* QUAD */
#        unsigned long   currentMasterID;        /* Id of current Master cntrl       */
#        unsigned long   numControllersInVCG;    /* # of controllers in group        */
#        unsigned char   communicationsKey[16];  /* signiture key                    */
#        IP_ADDRESS      ipAddress;              /* IP address for Master            */
#        IP_ADDRESS      gatewayAddress;         /* Gateway address - system wide    */
#        /* QUADS (2) */
#        IP_ADDRESS      subnetMask;             /* Subnet Mask - system wide        */
#
#        /* Active Controller map and padding for additional controllers */
#        unsigned char   activeControllerMap[MAX_CONTROLLERS];
#        unsigned char   activeControllerMap_Pad[128 - MAX_CONTROLLERS];
#
#        UINT8           rsvd180[12];            /* RESERVED                         */
#        /* QUADS (9) */
#        UINT8           rsvd192[4];             /* RESERVED                         */
#
#        unsigned long   ownedDriveCount;          /* Number of drives owned by the VCG */
#
#        /* Pad */
#        UINT8           pad[3412];              /* pad out crc to end of sector -16 */
#        unsigned int    CRC;                    /* CRC for master config record     */
#    } QM_MASTER_CONFIG, *PQM_MASTER_CONFIG;
#
#
#      
##############################################################################
sub FmtCCBQmMc
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
    my $item9;
    my $msg = "\n";
    my $str;
    my %info;


    #    typedef  struct _QM_MASTER_CONFIG
    #    {
    #        unsigned long   schema;                 /* schema version for quorum        */
    #        unsigned long   magicNumber;            /* magic number for quorum          */
    #        unsigned long   virtualControllerSN;    /* VCG System ID (licensed)         */
    #        unsigned long   electionSerial;         /* election serial number           */

    $fmt = sprintf("x%d LL LL ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4) =  
                        unpack $fmt , $$bufferPtr;

    $msg .= sprintf("schema                 0x%08X\n", $item1);
    $msg .= sprintf("magicNumber            0x%08X\n", $item2);
    $msg .= sprintf("virtualControllerSN    0x%08X\n", $item3);
    $msg .= sprintf("electionSerial         0x%08X\n", $item4);


    $offset += 16;                         #   bytes processed
    $address += 16;                  


    #        /* QUAD */
    #        unsigned long   currentMasterID;        /* Id of current Master cntrl       */
    #        unsigned long   numControllersInVCG;    /* # of controllers in group        */

    $fmt = sprintf("x%d LL  ",$offset);      # generate the format string
    ($item1, $item2) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("currentMasterID        0x%08X\n", $item1);
    $msg .= sprintf("numControllersInVCG    0x%08X\n\n", $item2);


    $offset += 8;                         #   bytes processed
    $address += 8;                  


    #        unsigned char   communicationsKey[16];  /* signiture key                    */

    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("communicationsKey      0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    $item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8);


    $offset += 8;                         #   bytes processed
    $address += 8;
                      
    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("communicationsKey      0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    $item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8);


    $offset += 8;                         #   bytes processed
    $address += 8;                  



    #        IP_ADDRESS      ipAddress;              /* IP address for Master            */
    #        IP_ADDRESS      gatewayAddress;         /* Gateway address - system wide    */

    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("ipAddress              %d.%d.%d.%d\n",
                    $item1, $item2, $item3, $item4);
    $msg .= sprintf("gatewayAddress         %d.%d.%d.%d\n",
                    $item5, $item6, $item7, $item8);


    $offset += 8;                         #   bytes processed
    $address += 8;                  

    #        /* QUADS (2) */
    #        IP_ADDRESS      subnetMask;             /* Subnet Mask - system wide        */
    #
    $fmt = sprintf("x%d CCCC CCCC  ",$offset);      # generate the format string
    ($item1, $item2, $item3, $item4, $item5, $item6, $item7, $item8) =  
                        unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("subnetMask             %d.%d.%d.%d\n\n",
                    $item1, $item2, $item3, $item4);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

#        /* Active Controller map and padding for additional controllers */
#        unsigned char   activeControllerMap[MAX_CONTROLLERS];
#        unsigned char   activeControllerMap_Pad[128 - MAX_CONTROLLERS];
#

    $msg .= "Active Controller Map:\n\n";
    $msg .= FmtDataString( $bufferPtr, $address, "byte", 128, $offset);

    $offset += 128;                         #   bytes processed
    $address += 128;                  

#        UINT8           rsvd180[12];            /* RESERVED                         */
#        /* QUADS (9) */
#        UINT8           rsvd192[4];             /* RESERVED                         */
#

#    $msg .= "\nreserved bytes:\n\n";
#    $msg .= FmtDataString( $bufferPtr, $address, "byte", 16, $offset);

    $offset += 16;                         #   bytes processed
    $address += 16;                  

#        unsigned long   ownedDriveCount;          /* Number of drives owned by the VCG */
    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =     unpack $fmt , $$bufferPtr;
    
    $msg .= "\n";
    $msg .= sprintf("ownedDriveCount        %d\n", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  

#
#        /* Pad */
#        UINT8           pad[3412];              /* pad out crc to end of sector -16 */

#    $msg .= "\n  3412 reserved bytes skipped \n\n";                  
    
    $offset += 3412;                         #   bytes processed
    $address += 3412;

#        unsigned int    CRC;                    /* CRC for master config record     */

    $fmt = sprintf("x%d L  ",$offset);      # generate the format string
    ($item1) =     unpack $fmt , $$bufferPtr;
    
    $msg .= sprintf("CRC                    0x%08X\n ", $item1);


    $offset += 4;                         #   bytes processed
    $address += 4;                  


#    } QM_MASTER_CONFIG, *PQM_MASTER_CONFIG;
#

    # copy the data to the callers pointers

    if ( $destPtr )
    {
        $$destPtr .= $msg;         # append to callers item
    }

    return GOOD;
}

##############################################################################
#
#          Name: min
#
#        Inputs: oneValue, anotherValue
#
#       Outputs: the less positive of the two values
#
#  Globals Used: none
#
#   Description: basic min function because I know I will need it.  
#
#
##############################################################################
# basic min(x,y) function
sub min
{
    my ($a, $b ) = @_;
    if ( $a > $b ) {return $b};     # b is smaller, send it back
    return $a;                      # a=b or is smaller, send it back
}

##############################################################################

1;   # we need this for a PM

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
