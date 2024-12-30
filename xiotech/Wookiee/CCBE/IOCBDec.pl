#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    IOCBDec.pl
#
# AUTHOR:       Tim Swatosh
#
# DATE:         10/19/2003
#
# DESCRIPTION:  Decodes/annotates a IOCB trace dump file.
#
#====================================================================

use constant IOCB_SIZE =>    64;
use constant IOCB_HDR_SIZE =>    4;
use constant IOCB_DATA_SIZE =>    60;
use constant IOCB_HDR =>
# 
           "C           # Entry Type
            C           # Entry Count
            C           # System Defined 1
            C           # Entry Status
";
use constant IOCB_CMD_TYPE2 =>
           "L           # Handle (ILT*)
            CC          # Reserved  | Target
            S           # Lun
            CC          # Command Reference Number | Reserved 
            S           # Timeout
            S           # Data Segment Count
            C20         # SCSI CDB
            LL          # Total Byte Count (64 bit)
            LL          # Seg Addr | Seg Length
            LL          # Seg Addr | Seg Length
            LL          # Seg Addr | Seg Length
";

use constant IOCB_CMD_TYPE3 =>
           "L           # Handle (ILT*)
            CC          # Reserved  | Target
            S           # Lun
            CC          # Command Reference Number | Reserved 
            S           # Timeout
            S           # Data Segment Count
            C20         # SCSI CDB
            LL          # Total Byte Count (64 bit)
           LLL          # Seg Addr (64 Bit) | Seg Length
           LLL          # Seg Addr (64 Bit) | Seg Length
";

use constant IOCB_CTIO_TYPE0 =>
           "L           # Reserved
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
            SSSS        # Seg Addr | Seg Length | Seg Addr | Seg Length
";

use constant IOCB_CTIO_TYPE1 =>
          "LLL          # Seg Addr (64 Bit) | Seg Length
           LLL          # Seg Addr (64 Bit) | Seg Length
           LLL          # Seg Addr (64 Bit) | Seg Length
           LLL          # Seg Addr (64 Bit) | Seg Length
           LLL          # Seg Addr (64 Bit) | Seg Length
";
use constant IOCB_STATUS_TYPE0 =>
           "L           # Handle (ILT*)
            S           # SCSI Status
            S           # Completion Status
            S           # State Flags
            S           # Status Flags
            S           # Response Information Length
            S           # Sense Data Length
            L           # Residual Transfer Length
            C8          # FCP Response Information
            C16         # SCSI Sense Data
            C20         # SCSI CDB
";
use constant IOCB_STATUS_CTIO_0 =>
           "C60         # Extended Sense Data
";
use constant IOCB_MARKER =>
           "L           # System Defined
            CC          # Reserved  | Target
            CC          # Modifier  | VP Index
            S           # Flags
            S           # Lun
            C48         # Reserved
";
use constant IOCB_ENABLE_LUN =>
           "L           # System Defined
            CC          # Reserved  | Target
            C6          # Reserved
            CC          # status    | Reserved
            CC          # Command Count | Imm Notify Count
            S           # Reserved
            S           # Timeout
            C40         # Reserved
";
use constant IOCB_MODIFY_LUN =>
           "L           # System Defined
            CC          # Reserved  | Target
            CC          # Operators | Reserved
            C4          # Reserved 
            CC          # status    | Reserved
            CC          # Command Count | Imm Notify Count
            S           # Reserved
            S           # Timeout
            C40         # Reserved
";
use constant IOCB_IMMED_NOTIFY =>
           "L           # System Defined
            CC          # Reserved  | Initiator ID
            S           # Lun
            CC          # Target | Reserved
            S           # Status Modifier
            S           # Status 
            S           # Task Flags 
            S           # Sequence Identifier 
            S           # SRR RX_ID 
            L           # SRR Relative Offset 
            S           # SRR IU 
            S           # SRR OX_ID 
            C30         # Reserved
            S           # OX_ID 
";
use constant IOCB_NOTIFY_ACK =>
           "L           # System Defined
            CC          # Reserved  | Initiator ID
            CC          # Target | Reserved
            S           # Flags
            S           # Response Code
            S           # Status 
            S           # Task Flags 
            S           # Sequence Identifier 
            S           # SRR RX_ID 
            L           # SRR Relative Offset 
            S           # SRR IU 
            S           # SRR Flags 
            S           # SRR Reject Code
            CC          # SRR reject vendor unique | SRR Explanation  
            C26         # Reserved
            S           # OX_ID 
";

##############################################################################
# Name:     fmtHeaderString
#
# Desc:     Format IOCB header.
#
# Input:    iocb and text name
##############################################################################
sub fmtHeaderString
{
    ($iocb, $name) = @_;

#   printf "IOCB(0x%02X): Count: %d Status: 0x%02X\n", 
#                     $$iocb{TYPE}, 
#                       $$iocb{COUNT}, 
#                       $$iocb{STATUS};
    $hdrStr = sprintf "IOCB(0x%02X)-%s: Count: %d Status: 0x%02X", 
                       $$iocb{TYPE}, 
                       $name,
                       $$iocb{COUNT}, 
                       $$iocb{STATUS};
    
    return($hdrStr);
}
##############################################################################
# Name:     fmtIOCBHexString
#
# Desc:     Format IOCB Hex data string.
#
# Input:    buffer
##############################################################################
sub fmtIOCBHexString
{
    my ($buffer) = @_;
    my $i;
    my @array;

    for ($i=0; $i < IOCB_DATA_SIZE / 4; $i++)
    {
#        $array[$i] = unpack("L",$($buffer+($i*4)));
    }

    for ($i=0; $i < IOCB_DATA_SIZE / 4; $i++)
    {
        my $tempStr =sprintf " 0x%08X",$array[$i];
        $dataStr = $dataStr.$tempStr;
        if ($i%4)
        {
            $dataStr = $dataStr."\n";
        }
    }


    return($dataStr);
}
################################################################################
####### Main ###################################################################
################################################################################
    my %iocb;

    ($script = $0) =~ s/^.*\\//;
    unless (@ARGV == 1) { die "\nUsage: $script file-to-decode\n\n" }
    ($tracefile)=@ARGV;

    $outfile = "$tracefile-out";


    #
    # Initialize hash tables
    #
    # my %AllHashes = BuildCmdCodeHashTables();

    #
    # Open the output file
    #
    open OUT, ">$outfile" or die "\nAbort: Can't open $outfile...\n";
    print "Output being written to $outfile...\n";
    print OUT "Line      Absolute Time    Relative Time    Type         Sub-Type (text/numeric)\n\n";

    #
    # Open the input trace file
    #
    open F, "$tracefile" or die "\nAbort: Can't open $tracefile...\n";
    binmode F;


    #
    # seek back to the wrap point in the file
    #
    # seek F, $firstO, 0 or die;  

    #
    # read the FR header Begin, In, Out, End pointers
    #
  
    if(read F, $buffer, 16)
    {
        # Strip off pointers
        ($beginPtr, 
         $inPtr, 
         $outPtr, 
         $endPtr) = unpack( "LLLL", $buffer);

        printf OUT "Begin Pointer: 0x%08X\n",  $beginPtr;
        printf OUT "In Pointer: 0x%08X\n",  $inPtr;
        printf OUT "Out Pointer: 0x%08X\n",  $outPtr;
        printf OUT "End Pointer: 0x%08X\n",  $endPtr;
    }



    # Each IOCB is 64 bytes long
    while(read F, $buffer, IOCB_HDR_SIZE) {

        # Get the common header for all IOCBs
        ($iocb{TYPE}, 
         $iocb{COUNT}, 
         $iocb{SYSDEF}, 
         $iocb{STATUS}) = unpack( IOCB_HDR, $buffer);

        #
        # If we are at the end of the data or suspect a bad entry, exit.
        # We are only looking at one value here, we could validate others...
        #
        $type = $iocb{TYPE};
        if($type == 0xFF) 
        {
            if(read F, $buffer, 12)
            {
                ($temp1,
                $temp2,
                $temp3) = unpack("LLL", $buffer); 

                if(($temp1 == 0x0FFFFFFFF) &&
                   ($temp2 == 0x0FFFFFFFF) &&
                   ($temp3 == 0x0FFFFFFFF))
                {
                    if(read F, $buffer, 16)
                    {
                        # Strip off pointers
                        ($beginPtr, 
                         $inPtr, 
                         $outPtr, 
                         $endPtr) = unpack( "LLLL", $buffer);

                        printf OUT "Begin Pointer: 0x%08X\n",  $beginPtr;
                        printf OUT "In Pointer: 0x%08X\n",  $inPtr;
                        printf OUT "Out Pointer: 0x%08X\n",  $outPtr;
                        printf OUT "End Pointer: 0x%08X\n",  $endPtr;

                        next;
                    }
                    else
                    {
                        last;
                    }
                
                }
                else
                {
                    last;
                }
            }
            else
            {
                last;
            }
        }

        read F, $buffer, IOCB_DATA_SIZE;
        #
        # Figure out which class trace point this is; decode if possible.
        #
        $hdrDesc = fmtHeaderString(\%iocb, "UNKNOWN");
        $dataDesc = "";

#        $dataDesc = fmtIOCBHexString(\$buffer);
       
    
        if ($type == 0x02)      # CTIO Type 0
        {
        }
        elsif ($type == 0x03)  # Status Type 0
        {
            $hdrDesc = fmtHeaderString(\%iocb, "Status Type 0");
        }
        elsif ($type == 0x04)  # Marker
        {
        }
        elsif ($type == 0x0A)  # CTIO Type 1
        {
        }
        elsif ($type == 0x0B)  # Enable LUN
        {
        }
        elsif ($type == 0x0C)  # Modify LUN
        {
        }
        elsif ($type == 0x0D)  # Immediate Nodify
        {
            $hdrDesc = fmtHeaderString(\%iocb, "Immediate Notify");
        }
        elsif ($type == 0x0E)  # Nodify ACK
        {
            $hdrDesc = fmtHeaderString(\%iocb, "Notify ACK");
        }
        elsif ($type == 0x10)  # Status Continuation
        {
        }
        elsif ($type == 0x11)  # Command Type 2
        {
            $hdrDesc = fmtHeaderString(\%iocb, "Command Type 2");
#           "L           # Handle (ILT*)
#            CC          # Reserved  | Target
#            S           # Lun
#            CC          # Command Reference Number | Reserved 
#            S           # Timeout
#            S           # Data Segment Count
#            C20         # SCSI CDB
#            LL          # Total Byte Count (64 bit)
#            LL          # Seg Addr | Seg Length
#            LL          # Seg Addr | Seg Length
#            LL          # Seg Addr | Seg Length
            # Get the data
            ($iocb{HANDLE}, 
             $iocb{TARGET}, $iocb{RSVD1}, 
             $iocb{LUN},  
             $iocb{RSVD2}, $iocb{CRN}, 
             $iocb{TIMEOUT}, 
             $iocb{DATA_SEG_COUNT}, 
             $iocb{SCSI_CDB}, 
             $iocb{TBC_LO},$iocb{TBC_HI}, 
             $iocb{SEG_LEN_1},$iocb{SEG_ADDR_1}, 
             $iocb{SEG_LEN_2},$iocb{SEG_ADDR_2}, 
             $iocb{SEG_LEN_3},$iocb{SEG_ADDR_3}) = unpack( IOCB_CMD_TYPE2, $buffer);

            $dataDesc = sprintf "Handle = 0x%08X Target ID = 0x%02X Lun = 0x%04X \n",
                        $iocb{HANDLE},$iocb{TARGET}, $iocb{LUN};
            $dataDesc .= sprintf "CNR = 0x%02X Timeout = %d DS Count = 0x%04X \n",
                        $iocb{CNR},$iocb{TIMEOUT}, $iocb{DATA_SEG_COUNT};
            $dataDesc .= sprintf "CDB = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X
            Timeout = %d DS Count = 0x%04X \n",
                        $iocb{CNR},$iocb{TIMEOUT}, $iocb{DATA_SEG_COUNT};
        }
        elsif ($type == 0x16)  # Accept target IO Type 2
        {
            $hdrDesc = fmtHeaderString(\%iocb, "Accept Target IO Type 2");
        }
        elsif ($type == 0x17)  # CTIO Type 2
        {
            $hdrDesc = fmtHeaderString(\%iocb, "CTIO Type 2");
        }
        elsif ($type == 0x19)  # Command Type 3
        {
        }
        else
        {
        }
        #
        # Write the formatted data out
        #
        printf OUT "%s %s\n",  $hdrDesc, $dataDesc;
    }

#
# Close files and exit
#
close F;
close OUT;
