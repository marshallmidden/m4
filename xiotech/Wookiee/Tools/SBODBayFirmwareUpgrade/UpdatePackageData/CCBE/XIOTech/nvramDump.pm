# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Bryan Holty
#
# Purpose:
#   Wrapper for all the different XIOTech Error Codes that can be received
#   from the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use strict;

my $rec_length;
my $rec_status;
my $rec_type;
my $blanks;



sub writeMemToFile
{
    my ($self, $data, $name) = @_;
    my $fullname;
    my $fh;
    my $tmp = "";

    if (!defined($name))
    {
        $name = "dmp.txt";
    }
    
    if ($ENV{TMP})
    {
        $tmp = $ENV{TMP};

        if ((substr($tmp, (length $tmp) - 1) cmp "\\") != 0)
        {
            $tmp .= "\\";
        }
    }
    elsif ($ENV{TEMP})
    {
        $tmp = $ENV{TEMP};

        if ((substr($tmp, (length $tmp) - 1) cmp "\\") != 0)
        {
            $tmp .= "\\";
        }
    }
    
    $fullname = $tmp . $name;

        
    if (defined($data) && defined($fullname))
    {
        open NVRAM, ">$fullname";
        binmode NVRAM;
        
        syswrite NVRAM, $data;
        close NVRAM;
    }
    else
    {
        return undef;
    }

    return $fullname;
}



sub dislplayNvramDump
{
    my ($self, $in_file, $out_file, $proc) = @_;

    my $nvr_file;

    my @days = ( "Undef",
                 "Sun",
                 "Mon",
                 "Tue",
                 "Wed",
                 "Thu",
                 "Fri",
                 "Sat");
    
    my $fh = *STDOUT;
    if (defined $out_file) {
        $fh = *FH;
        open $fh, ">$out_file" or $fh = *STDOUT;
    }
    
    open NVRAM, $in_file or die "Can't open file: $in_file";
    binmode NVRAM;
    
    print $fh "\n\n--------------------- BEGINNING NVRAM DUMP ----------------------------------------\n\n";

    seek NVRAM, 0x7800, 0;

    #define NVSRAM_NMI_REG_0   (BASE_NVSRAM + 0x00007800) /*backtraceCrc - UINT32 */
    my $backtraceCrc;
    read NVRAM, $backtraceCrc, 4;
    $backtraceCrc = unpack "L", $backtraceCrc;
    printf($fh "Backtrace area CRC: 0x%08X\n", $backtraceCrc);

    my $year;
    read NVRAM, $year, 2;
    $year = unpack "S", $year;

    my $month;
    read NVRAM, $month, 1;
    $month = unpack "C", $month;

    my $day;
    read NVRAM, $day, 1;
    $day = unpack "C", $day;

    my $dayofWeek;
    read NVRAM, $dayofWeek, 1;
    $dayofWeek = unpack "C", $dayofWeek;

    my $hour;
    read NVRAM, $hour, 1;
    $hour = unpack "C", $hour;

    my $minute;
    read NVRAM, $minute, 1;
    $minute = unpack "C", $minute;

    my $second;
    read NVRAM, $second, 1;
    $second = unpack "C", $second;

    printf($fh "Timestamp: %s  %02X-%02X-%04X  %02X:%02X:%02X\n\n", 
           $days[$dayofWeek], $month, $day, $year, $hour, $minute, $second);

    read NVRAM, $blanks, 4;


    #define NVSRAM_NMI_ISR     (BASE_NVSRAM + 0x00007810) /* isr */
    my $NMI_ISR;
    read NVRAM, $NMI_ISR, 4;
    $NMI_ISR = unpack "L", $NMI_ISR;
    printf($fh "NISR: 0x%08x\n", $NMI_ISR);

    #define NVSRAM_NMI_LEVELS  (BASE_NVSRAM + 0x00007814) /*levels recorded */
    my $NMI_LEVELS;
    read NVRAM, $NMI_LEVELS, 4;
    $NMI_LEVELS = unpack "L", $NMI_LEVELS;
    printf($fh "valid_levels: 0x%08x\n", $NMI_LEVELS);
    
    #define NVSRAM_NMI_EVENT   (BASE_NVSRAM + 0x00007818) /* last event type */
    my $NMI_EVENT;
    read NVRAM, $NMI_EVENT, 4;
    $NMI_EVENT = unpack "L", $NMI_EVENT;
    printf($fh "last_event: 0x%08x\n", $NMI_EVENT);

    read NVRAM, $blanks, 4;
    
    #define NVSRAM_NMI_RIP     (BASE_NVSRAM + 0x00007820) /* last rip */
    my $NMI_RIP;
    read NVRAM, $NMI_RIP, 4;
    $NMI_RIP = unpack "L", $NMI_RIP;
    printf($fh "last_RIP: 0x%08x\n", $NMI_RIP);

    #define NVSRAM_NMI_SP      (BASE_NVSRAM + 0x00007824)
    my $NMI_SP;
    read NVRAM, $NMI_SP, 4;
    $NMI_SP = unpack "L", $NMI_SP;
    printf($fh "last_SP: 0x%08x\n", $NMI_SP);
    
    #define NVSRAM_NMI_FP      (BASE_NVSRAM + 0x00007828)
    my $NMI_FP;
    read NVRAM, $NMI_FP, 4;
    $NMI_FP = unpack "L", $NMI_FP;
    printf($fh "last_FP: 0x%08x\n", $NMI_FP);

    #define NVSRAM_NMI_PFP     (BASE_NVSRAM + 0x0000782c)
    my $NMI_PFP;
    read NVRAM, $NMI_PFP, 4;
    $NMI_PFP = unpack "L", $NMI_PFP;
    printf($fh "last_PFP: 0x%08x\n", $NMI_PFP);

    #define NVSRAM_NMI_PATUIMR (BASE_NVSRAM + 0x00007830)
    read NVRAM, $nvr_file, 4;
    $nvr_file = unpack "L", $nvr_file;
    printf($fh "PATUIMR: 0x%08x\n", $nvr_file);

    #define NVSRAM_NMI_SATUIMR (BASE_NVSRAM + 0x00007834)
    read NVRAM, $nvr_file, 4;
    $nvr_file = unpack "L", $nvr_file;
    printf($fh "SATUIMR: 0x%08x\n", $nvr_file);

    #define NVSRAM_NMI_PATUCMD (BASE_NVSRAM + 0x00007838) /* short */
    read NVRAM, $nvr_file, 4;
    $nvr_file = unpack "L", $nvr_file;
    printf($fh "PATUCMD: 0x%08x\n", $nvr_file);

    #define NVSRAM_NMI_SATUCMD (BASE_NVSRAM + 0x0000783a) /* short */
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "SATUCMD: 0x%04x\n", $nvr_file);

    #define NVSRAM_NMI_ATUCR   (BASE_NVSRAM + 0x0000783c) /* short */
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "ATUCR: 0x%04x\n", $nvr_file);


    #define NVSRAM_NMI_MCISR   (BASE_NVSRAM + 0x00007840) /* nmi bit 0 */
    my $NMI_MCISR;
    read NVRAM, $NMI_MCISR, 4;
    $NMI_MCISR = unpack "L", $NMI_MCISR;
    printf($fh "MCISR: 0x%08x\n", $NMI_MCISR);

    #define NVSRAM_NMI_PATUISR (BASE_NVSRAM + 0x00007844) /* 1 */
    my $NMI_PATUISR;
    read NVRAM, $NMI_PATUISR, 4;
    $NMI_PATUISR = unpack "L", $NMI_PATUISR;
    printf($fh "PATUISR: 0x%08x\n", $NMI_PATUISR);

    #define NVSRAM_NMI_SATUISR (BASE_NVSRAM + 0x00007848) /* 2 */
    my $NMI_SATUISR;
    read NVRAM, $NMI_SATUISR, 4;
    $NMI_SATUISR = unpack "L", $NMI_SATUISR;
    printf($fh "SATUISR: 0x%08x\n", $NMI_SATUISR);

    #define NVSRAM_NMI_PBISR   (BASE_NVSRAM + 0x0000784c) /* 3 */
    my $NMI_PBISR;
    read NVRAM, $NMI_PBISR, 4;
    $NMI_PBISR = unpack "L", $NMI_PBISR;
    printf($fh "PBISR: 0x%08x\n", $NMI_PBISR);

    #define NVSRAM_NMI_SBISR   (BASE_NVSRAM + 0x00007850) /* 4 */
    my $NMI_SBISR;
    read NVRAM, $NMI_SBISR, 4;
    $NMI_SBISR = unpack "L", $NMI_SBISR;
    printf($fh "SBISR: 0x%08x\n", $NMI_SBISR);

    #define NVSRAM_NMI_CSR0    (BASE_NVSRAM + 0x00007854) /* 5 */
    my $NMI_CSR0;
    read NVRAM, $NMI_CSR0, 4;
    $NMI_CSR0 = unpack "L", $NMI_CSR0;
    printf($fh "CSR0: 0x%08x\n", $NMI_CSR0);

    #define NVSRAM_NMI_CSR1    (BASE_NVSRAM + 0x00007858) /* 6 */
    my $NMI_CSR1;
    read NVRAM, $NMI_CSR1, 4;
    $NMI_CSR1 = unpack "L", $NMI_CSR1;
    printf($fh "CSR1: 0x%08x\n", $NMI_CSR1);

    #define NVSRAM_NMI_CSR2    (BASE_NVSRAM + 0x0000785c) /* 7 */
    my $NMI_CSR2;
    read NVRAM, $NMI_CSR2, 4;
    $NMI_CSR2 = unpack "L", $NMI_CSR2;
    printf($fh "CSR2: 0x%08x\n", $NMI_CSR2);

    #define NVSRAM_NMI_IISR    (BASE_NVSRAM + 0x00007860) /* 8 / 9 is external */
    my $NMI_IISR;
    read NVRAM, $NMI_IISR, 4;
    $NMI_IISR = unpack "L", $NMI_IISR;
    printf($fh "IISR: 0x%08x\n", $NMI_IISR);

    #define NVSRAM_NMI_ASR     (BASE_NVSRAM + 0x00007864) /* 10 */
    my $NMI_ASR;
    read NVRAM, $NMI_ASR, 4;
    $NMI_ASR = unpack "L", $NMI_ASR;
    printf($fh "ASR: 0x%08x\n", $NMI_ASR);

    #define NVSRAM_NMI_BIUISR  (BASE_NVSRAM + 0x00007868) /* nmi bit 11 */
    my $NMI_BIUISR;
    read NVRAM, $NMI_BIUISR, 4;
    $NMI_BIUISR = unpack "L", $NMI_BIUISR;
    printf($fh "BIUISR: 0x%08x\n", $NMI_BIUISR);

    read NVRAM, $blanks, 4;

    #define NVSRAM_NMI_ECCR    (BASE_NVSRAM + 0x00007870) /* ecc control reg */
    my $NMI_ECCR;
    read NVRAM, $NMI_ECCR, 4;
    $NMI_ECCR = unpack "L", $NMI_ECCR;
    printf($fh "ECCR  0x%08x\n", $NMI_ECCR);

    #define NVSRAM_NMI_ECTST   (BASE_NVSRAM + 0x00007874) /* ecc test reg */
    read NVRAM, $NMI_ECCR, 4;
    $NMI_ECCR = unpack "L", $NMI_ECCR;
    printf($fh "ECTST: 0x%08x\n", $NMI_ECCR);

    #define NVSRAM_NMI_RET_FLAG (BASE_NVSRAM + 0x00007878) /* 1=return from nmi*/
    my $RET_FLAG;
    read NVRAM, $RET_FLAG, 4;
    $RET_FLAG = unpack "L", $RET_FLAG;
    printf($fh "RET_FLAG: 0x%08x\n", $RET_FLAG);

    #define NVSRAM_NMI_HLT_FLAG (BASE_NVSRAM + 0x0000787c) /* 1=halt from nmi */
    my $HLT_FLAG;
    read NVRAM, $HLT_FLAG , 4;
    $HLT_FLAG  = unpack "L", $HLT_FLAG ;
    printf($fh "HLT_FLAG: 0x%08x\n", $HLT_FLAG );


    #define NVSRAM_NMI_PATUSR  (BASE_NVSRAM + 0x00007880)
    my $NMI_PATUSR;
    read NVRAM, $NMI_PATUSR, 4;
    $NMI_PATUSR = unpack "L", $NMI_PATUSR;
    printf($fh "PATUSR: 0x%08x\n", $NMI_PATUSR);

    #define NVSRAM_NMI_SATUSR  (BASE_NVSRAM + 0x00007884)
    my $NMI_SATUSR;
    read NVRAM, $NMI_SATUSR, 4;
    $NMI_SATUSR = unpack "L", $NMI_SATUSR;
    printf($fh "SATUSR: 0x%08x\n", $NMI_SATUSR);

    #define NVSRAM_NMI_PCR     (BASE_NVSRAM + 0x00007886)
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "PCR: 0x%04x\n", $nvr_file);

    #define NVSRAM_NMI_BCR     (BASE_NVSRAM + 0x00007888)
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "BCR: 0x%04x\n", $nvr_file);

    #define NVSRAM_NMI_PSR     (BASE_NVSRAM + 0x0000788a)
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "PSR: 0x%04x\n", $nvr_file);

    #define NVSRAM_NMI_SSR     (BASE_NVSRAM + 0x0000788c)
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "SSR: 0x%04x\n", $nvr_file);

    #define NVSRAM_NMI_SDER    (BASE_NVSRAM + 0x0000788e)
    read NVRAM, $nvr_file, 2;
    $nvr_file = unpack "S", $nvr_file;
    printf($fh "SDER: 0x%04x\n", $nvr_file);


    read NVRAM, $blanks, 46;

    #define NVSRAM_NMI_GLOBAL  (BASE_NVSRAM + 0x000078c0) /* nmi data-g0-g14*/

    print $fh "\n--- Gobal Registers -------------------------------------------------\n";
    my $Registers;
    my $i;
    my $j;
    read NVRAM, $Registers, 60;
    my @g_reg = unpack "LLLLLLLLLLLLLLL", $Registers;
    for($i =0; $i < @g_reg; $i++)
    {
        printf($fh "g%d: 0x%08x\n", $i, $g_reg[$i]); 
    }

    read NVRAM, $blanks, 4;

    #define NVSRAM_NMI_LOCAL   (BASE_NVSRAM + 0x00007900) /* store up to 16 levels*/
    #                                                      /*  of r0-r15 reg data */
    #                                                      /*  16 levels of 16 words*/
    #                                                      /*  0x10 X 0x40 = 0x400*/
    #                                                      /*  area is 0x7900-0x7cff*/
    for($j = 0; $j < 16; $j++)
    {
        print $fh "\n---  Register level $j -----------------------------------------\n\n";
        read NVRAM, $Registers, 64;
        my @r_reg = unpack "LLLLLLLLLLLLLLLL", $Registers;
        for($i =0; $i < @r_reg; $i++)
        {
            printf($fh "r%d: 0x%08x\n", $i, $r_reg[$i]); 
        }
    }


    #define NVSRAM_NMI_BOOT_HDR (BASE_NVSRAM + 0x00007d00) /* boot header info */

    print $fh "\nBoot Header Info\n\n";
    $self->ReadingFWHeader($fh);

    #define NVSRAM_NMI_DIAG_HDR (BASE_NVSRAM + 0x00007d80) /* diag header info */

    print $fh "\nDiag Header Info\n\n";
    $self->ReadingFWHeader($fh);


    #define NVSRAM_NMI_FW_HDR   (BASE_NVSRAM + 0x00007e00) /* fw header info */

    print $fh "\nFw Header Info\n\n";
    $self->ReadingFWHeader($fh);

    #print $fh "\n\n";
    #$self->ReadingFWHeader($fh);
    read NVRAM, $blanks, 128;

    #define NVSRAM_NMI_PROTECT_START (BASE_NVSRAM + 0x00007f00) /* no clear after here*/
    read NVRAM, $blanks, 112;

    #        /* proc board controller serial number - 32 bit binary - */
    #        /* 1-4 gig decimal digit entered for controller serial number */
    #define NVSRAM_CONTROLLER_SER_NUM (BASE_NVSRAM + 0x00007f70) 
    read NVRAM, $nvr_file, 4;
    $nvr_file = unpack "L", $nvr_file;
    printf($fh "NVSRAM_CONTROLLER_SER_NUM: 0x%08x\n", $nvr_file);

    #        /* proc board unit serial number - 32 bit binary - */
    #        /* 1-4 gig decimal digit entered for unit serial number */

    #define NVSRAM_UNIT_SER_NUM     (BASE_NVSRAM + 0x00007f74) 
    read NVRAM, $nvr_file, 4;
    $nvr_file = unpack "L", $nvr_file;
    printf($fh "NVSRAM_UNIT_SER_NUM: 0x%08x\n", $nvr_file);


    read NVRAM, $blanks, 8;
                        
    #define NVSRAM_NMI_ELOG0   (BASE_NVSRAM + 0x00007f80) /* w-log 0 */
    my $NMI_ELOG0;
    read NVRAM, $NMI_ELOG0, 4;
    $NMI_ELOG0 = unpack "L", $NMI_ELOG0;
    printf($fh "NMI_ELOG0: 0x%08x\n", $NMI_ELOG0);

    #define NVSRAM_NMI_ELOG1   (BASE_NVSRAM + 0x00007f84)
    my $NMI_ELOG1;
    read NVRAM, $NMI_ELOG1, 4;
    $NMI_ELOG1 = unpack "L", $NMI_ELOG1;
    printf($fh "NMI_ELOG1: 0x%08x\n", $NMI_ELOG1);

    #define NVSRAM_NMI_ECAR0   (BASE_NVSRAM + 0x00007f88) /* w-addr reg 0 */
    my $NMI_ECAR0;
    read NVRAM, $NMI_ECAR0, 4;
    $NMI_ECAR0 = unpack "L", $NMI_ECAR0;
    printf($fh "NMI_ECAR0: 0x%08x\n", $NMI_ECAR0);

    #define NVSRAM_NMI_ECAR1   (BASE_NVSRAM + 0x00007f8C)
    my $NMI_ECAR1;
    read NVRAM, $NMI_ECAR1, 4;
    $NMI_ECAR1 = unpack "L", $NMI_ECAR1;
    printf($fh "NMI_ECAR1: 0x%08x\n", $NMI_ECAR1);

    read NVRAM, $blanks, 16;

    #define NVSRAM_NMI_FW_FAULT_CNT (BASE_NVSRAM + 0x00007fa0)
    my $NMI_FW_FAULT_CNT;
    read NVRAM, $NMI_FW_FAULT_CNT, 4;
    $NMI_FW_FAULT_CNT = unpack "L", $NMI_FW_FAULT_CNT;
    printf($fh "NMI_FW_FAULT_CNT  0x%08x\n", $NMI_FW_FAULT_CNT);

    #define NVSRAM_NMI_DG_FAULT_CNT (BASE_NVSRAM + 0x00007fa4)
    my $NMI_DG_FAULT_CNT;
    read NVRAM, $NMI_DG_FAULT_CNT, 4;
    $NMI_DG_FAULT_CNT = unpack "L", $NMI_DG_FAULT_CNT;
    printf($fh "NMI_DG_FAULT_CNT: 0x%08x\n", $NMI_DG_FAULT_CNT);

    read NVRAM, $blanks, 8;

    #define NVSRAM_NMI_BRK_CNT    (BASE_NVSRAM + 0x00007fb0)
    my $NMI_BRK_CNT;
    read NVRAM, $NMI_BRK_CNT, 4;
    $NMI_BRK_CNT = unpack "L", $NMI_BRK_CNT;
    printf($fh "NMI_BRK_CNT: 0x%08x\n", $NMI_BRK_CNT);

    #define NVSRAM_NMI_UNEXP_CNT  (BASE_NVSRAM + 0x00007fb4)
    my $NMI_UNEXP_CNT;
    read NVRAM, $NMI_UNEXP_CNT, 4;
    $NMI_UNEXP_CNT = unpack "L", $NMI_UNEXP_CNT;
    printf($fh "NMI_UNEXP_CNT: 0x%08x\n", $NMI_UNEXP_CNT);

    read NVRAM, $blanks, 8;

    #define NVSRAM_NMI_MCE_CNT    (BASE_NVSRAM + 0x00007fc0)
    my $NMI_MCE_CNT;
    read NVRAM, $NMI_MCE_CNT, 4;
    $NMI_MCE_CNT = unpack "L", $NMI_MCE_CNT;
    printf($fh "NMI_MCE_CNT: 0x%08x\n", $NMI_MCE_CNT);

    #define NVSRAM_NMI_PAE_CNT    (BASE_NVSRAM + 0x00007fc4)
    my $NMI_PAE_CNT;
    read NVRAM, $NMI_PAE_CNT, 4;
    $NMI_PAE_CNT = unpack "L", $NMI_PAE_CNT;
    printf($fh "NMI_PAE_CNT: 0x%08x\n", $NMI_PAE_CNT);

    #define NVSRAM_NMI_SAE_CNT    (BASE_NVSRAM + 0x00007fc8)
    my $NMI_SAE_CNT;
    read NVRAM, $NMI_SAE_CNT, 4;
    $NMI_SAE_CNT = unpack "L", $NMI_SAE_CNT;
    printf($fh "NMI_SAE_CNT: 0x%08x\n", $NMI_SAE_CNT);

    #define NVSRAM_NMI_PBIE_CNT   (BASE_NVSRAM + 0x00007fcc)
    my $NMI_PBIE_CN;
    read NVRAM, $NMI_PBIE_CN, 4;
    $NMI_PBIE_CN = unpack "L", $NMI_PBIE_CN;
    printf($fh "NMI_PBIE_CN: 0x%08x\n", $NMI_PBIE_CN);

    #define NVSRAM_NMI_SBE_CNT    (BASE_NVSRAM + 0x00007fd0)
    my $NMI_SBE_CNT;
    read NVRAM, $NMI_SBE_CNT , 4;
    $NMI_SBE_CNT  = unpack "L", $NMI_SBE_CNT ;
    printf($fh "NMI_SBE_CNT:  0x%08x\n", $NMI_SBE_CNT );

    #define NVSRAM_NMI_DMAC0E_CNT (BASE_NVSRAM + 0x00007fd4)
    my $NMI_DMAC0E_CNT;
    read NVRAM, $NMI_DMAC0E_CNT , 4;
    $NMI_DMAC0E_CNT  = unpack "L", $NMI_DMAC0E_CNT ;
    printf($fh "NMI_DMAC0E_CNT   0x%08x\n", $NMI_DMAC0E_CNT );

    #define NVSRAM_NMI_DMAC1E_CNT (BASE_NVSRAM + 0x00007fd8)
    my $DMAC1E_CNT;
    read NVRAM, $DMAC1E_CNT, 4;
    $DMAC1E_CNT = unpack "L", $DMAC1E_CNT;
    printf($fh "DMAC1E_CNT: 0x%08x\n", $DMAC1E_CNT);

    #define NVSRAM_NMI_DMAC2E_CNT (BASE_NVSRAM + 0x00007fdc)
    my $DMAC2E_CNT;
    read NVRAM, $DMAC2E_CNT, 4;
    $DMAC2E_CNT = unpack "L", $DMAC2E_CNT;
    printf($fh "DMAC2E_CNT: 0x%08x\n", $DMAC2E_CNT);

    #define NVSRAM_NMI_MUI_CNT    (BASE_NVSRAM + 0x00007fe0)
    my $NMI_MUI_CNT;
    read NVRAM, $NMI_MUI_CNT, 4;
    $NMI_MUI_CNT = unpack "L", $NMI_MUI_CNT;
    printf($fh "NMI_MUI_CNT: 0x%08x\n", $NMI_MUI_CNT);

    #define NVSRAM_NMI_ENI_CNT    (BASE_NVSRAM + 0x00007fe4)
    my $NMI_ENI_CNT;
    read NVRAM, $NMI_ENI_CNT, 4;
    $NMI_ENI_CNT = unpack "L", $NMI_ENI_CNT;
    printf($fh "NMI_ENI_CNT: 0x%08x\n", $NMI_ENI_CNT);

    #define NVSRAM_NMI_AAUE_CNT   (BASE_NVSRAM + 0x00007fe8)
    my $NMI_AAUE_CNT;
    read NVRAM, $NMI_AAUE_CNT, 4;
    $NMI_AAUE_CNT = unpack "L", $NMI_AAUE_CNT;
    printf($fh "NMI_AAUE_CNT: 0x%08x\n", $NMI_AAUE_CNT);

    #define NVSRAM_NMI_BIUE_CNT   (BASE_NVSRAM + 0x00007fec) /* bus errors */
    my $NMI_BIUE_CNT;
    read NVRAM, $NMI_BIUE_CNT, 4;
    $NMI_BIUE_CNT = unpack "L", $NMI_BIUE_CNT;
    printf($fh "NMI_BIUE_CNT: 0x%08x\n", $NMI_BIUE_CNT);


    #define NVSRAM_NMI_ECC_SGL_CNT (BASE_NVSRAM + 0x00007ff0)
    my $NMI_ECC_SGL_CNT;
    read NVRAM, $NMI_ECC_SGL_CNT, 4;
    $NMI_ECC_SGL_CNT = unpack "L", $NMI_ECC_SGL_CNT;
    printf($fh "NMI_ECC_SGL_CNT: 0x%08x\n", $NMI_ECC_SGL_CNT);

    #define NVSRAM_NMI_ECC_MUL_CNT (BASE_NVSRAM + 0x00007ff4)
    my $NMI_ECC_MUL_CNT;
    read NVRAM, $NMI_ECC_MUL_CNT, 4;
    $NMI_ECC_MUL_CNT = unpack "L", $NMI_ECC_MUL_CNT;
    printf($fh "NMI_ECC_MUL_CNT: 0x%08x\n", $NMI_ECC_MUL_CNT);

    #define NVSRAM_NMI_ECC_NOT_CNT (BASE_NVSRAM + 0x00007ff8)
    my $NMI_ECC_NOT_CNT;
    read NVRAM, $NMI_ECC_NOT_CNT, 4;
    $NMI_ECC_NOT_CNT = unpack "L", $NMI_ECC_NOT_CNT;
    printf($fh "NMI_ECC_NOT_CNT: 0x%08x\n", $NMI_ECC_NOT_CNT);

    #define NVSRAM_NMI_CNT        (BASE_NVSRAM + 0x00007ffc) /* counts all nmi */
    my $NMI_CNT;
    read NVRAM, $NMI_CNT, 4;
    $NMI_CNT = unpack "L", $NMI_CNT;
    printf($fh "NMI_CNT: 0x%08x\n", $NMI_CNT);

    
    if (uc($proc) eq "BE")
    {
    
        #
        # --- Begin base structure (PART II) ----------------------------------
        #

        # Checksum PART II          <w>
        read NVRAM, $nvr_file, 4;
        $nvr_file = unpack "L", $nvr_file;
        printf($fh "\nChecksum PART II: 0x%08x\n", $nvr_file);

        # reserved
        read NVRAM, $blanks, 4;
    
        # Version                   <s>
        read NVRAM, $nvr_file, 2;
        $nvr_file = unpack "S", $nvr_file;
        printf($fh "Version: 0x%04x\n", $nvr_file);

        # Revision                  <s>
        read NVRAM, $nvr_file, 2;
        $nvr_file = unpack "S", $nvr_file;
        printf($fh "Revision: 0x%04x\n", $nvr_file);

        # Length of structure       <w>
        my $struct_length; 
        read NVRAM, $struct_length, 4;
        $struct_length = unpack "L", $struct_length;
        printf($fh "Length of structure: 0x%08x\n", $struct_length);


        # Magic number              <s>
        read NVRAM, $nvr_file, 2;
        $nvr_file = unpack "S", $nvr_file;
        printf($fh "Magic number: 0x%04x\n", $nvr_file);

        # revision of nvram format  <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "revision of nvram format: 0x%02x\n", $nvr_file);

        # Default label for ndisk <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Default label for ndisk: 0x%02x\n", $nvr_file);

        # Sequence number           <w>
        read NVRAM, $nvr_file, 4;
        $nvr_file = unpack "L", $nvr_file;
        printf($fh "Sequence number: 0x%08x\n", $nvr_file);

        # Global priority           <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Global priority: 0x%02x\n", $nvr_file);


        # Next Vlink ID to use           <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Next Vlink ID to use: 0x%02x\n", $nvr_file);

        # Foreign Target Enable Max           <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Foreign Target Enable Max: 0x%02x\n", $nvr_file);
        
        #
        #       reserved 1
        #
        read NVRAM, $blanks, 1;
    

        # Virtual Controller Group ID      <w>
        read NVRAM, $nvr_file, 4;
        $nvr_file = unpack "L", $nvr_file;
        printf($fh "Virtual Controller Group ID: 0x%08x\n", $nvr_file);

        # Starting Offset of Local Images      <w>
        read NVRAM, $nvr_file, 4;
        $nvr_file = unpack "L", $nvr_file;
        printf($fh "Starting Offset of Local Images: 0x%08x\n", $nvr_file);

        #
        #       reserved 8
        #
        read NVRAM, $blanks, 8;

        # Scrub enable (T/F)       <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Scrub enable (T/F): 0x%02x\n", $nvr_file);

        # Global cache enable (T/F) <b>
        read NVRAM, $nvr_file, 1;
        $nvr_file = unpack "C", $nvr_file;
        printf($fh "Global cache enable (T/F): 0x%02x\n", $nvr_file);

        # Next VID to use           <s>
        read NVRAM, $nvr_file, 2;
        $nvr_file = unpack "S", $nvr_file;
        printf($fh "Next VID to use: 0x%04x\n", $nvr_file);

        #
        #       reserved 16
        #
        read NVRAM, $blanks, 16;
    
        my $drive_num = 0;
        my $vdisk_num = 0;
        my $server_num = 0;
        my $enclosure_num = 0;
        my $total_cap = 0;

        $self->ReadBase();

        while ($rec_type != 3 && $struct_length > 0)
        {
        if ($rec_type == 0x0B || $rec_type == 0x0C || $rec_type == 0x0D)
            {
                if ($rec_type == 0x0B)
                {
                    print $fh "\n--- Begin physical drive record structure ---------------------------\n";
                    $drive_num++;
                }elsif ($rec_type == 0x0C)
                {
                    print $fh "\n--- Begin SES enclosure record structure ----------------------------\n";
                    $enclosure_num++;
                }elsif ($rec_type == 0x0D)
                {
                    print $fh "\n--- Begin misc device record structure ------------------------------\n";
                }
                
                #$self->PrintBase($fh);
           
                # Physical device ID        <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Physical device ID: 0x%04x\n", $nvr_file);

                # Device class              <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Device class: 0x%02x\n", $nvr_file);

                # Channel installed in      <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Port installed in: 0x%02x\n", $nvr_file);

                # FC ID                     <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "FC ID 0x%08x\n", $nvr_file);
    
                # System serial             <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "System serial: 0x%08x\n", $nvr_file);

                # Product ID                <q>     
                my @ID = ();
                my $id_name = "";
                my $x;
                for($x = 0; $x < 16; $x++)
                {
                    read NVRAM, $nvr_file, 1;
                    push @ID, unpack "A", $nvr_file;
                    $id_name = "$id_name$nvr_file";
                }

                printf($fh "Product ID: $id_name\n");
                if ($rec_type == 0x0B)
                {
                    if ($id_name =~ /^ST/)
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
                        print $fh "Drive Capcity: $drive_size GB\n";
                    }
                }
                # Vendor ID                 <l>
                read NVRAM, $nvr_file, 8;
                #$nvr_file = unpack "L2", $nvr_file;
                printf($fh "Vendor ID: $nvr_file\n");

                # Serial number             <t>
                read NVRAM, $nvr_file, 12;
                #$nvr_file = unpack "L", $nvr_file;
                printf($fh "Serial number: $nvr_file\n");


                # World wide name           <l>
                my @nvr_array = ();
                for($x = 0; $x < 8; $x++)
                {
                    read NVRAM, $nvr_file, 1;
                    push @nvr_array , unpack "C", $nvr_file;
                }
                printf($fh "World wide name: 0x");
                my $wwn_byte;
                foreach $wwn_byte(@nvr_array)
                {printf($fh "%02x",$wwn_byte);}
                print $fh "\n";

                # LUN                       <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "LUN: 0x%08x\n", $nvr_file);
        
                # Misc Status                <c>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "MiscStat: 0x%02x\n", $nvr_file);
        
                # DName                       <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "DName: 0x%08x\n", $nvr_file);

                # Reserved 11
                read NVRAM, $blanks, 11;


            }
            elsif ($rec_type == 0x09)
            {
                print $fh "\n--- Begin RAID record structure -------------------------------------\n";
                #$self->PrintBase($fh);

                # RAID ID                   <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "RAID ID: 0x%04x\n", $nvr_file);

                # Type                      <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Type: 0x%02x\n", $nvr_file);

                # Depth                     <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Depth: 0x%02x\n", $nvr_file);

                # Virtual ID of owner       <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Virtual ID of owner: 0x%04x\n", $nvr_file);

                # Device count              <s>
                my $device_cnt;
                read NVRAM, $device_cnt, 2;
                $device_cnt = unpack "S", $device_cnt;
                printf($fh "Device count: 0x%04x\n", $device_cnt);


                # Sectors/stripe            <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Sectors/stripe: 0x%08x\n", $nvr_file);

                # Device capacity           <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Device capacity: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Sectors/unit              <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Sectors/unit: 0x%08x\n", $nvr_file);

                # Segment length / 256      <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Segment length / 256: 0x%08x\n", $nvr_file);
            
                # rlen      <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "rlen: 0x%08x\n", $nvr_file);
            
                #Astatus      <c>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Additional status: 0x%02x\n", $nvr_file);
            
                printf($fh "\n");
                
                read NVRAM, $blanks, 11; #reserved
                my $i;
                for ($i =0; $i < $device_cnt; $i++)
                {       
                    #read NVRAM, $blanks, 8;
                    # Physical device ID        <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "Physical device ID: 0x%04x\n", $nvr_file);

                    # Status of device          <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Status of device: 0x%02x\n", $nvr_file);

                    # Additional Status of device          <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Additional Status of device: 0x%02x\n", $nvr_file);

                    #read NVRAM, $blanks, 5;     #additional status and reserved

                    # Starting address of PSD   <l>
                    read NVRAM, $nvr_file, 4;
                    @nvr_array = unpack "L", $nvr_file;
                    printf($fh "Starting address of PSD: 0x%08x\n\n",$nvr_array[0]);

                    #read NVRAM, $blanks, 8;
                
                }
                
                if ($device_cnt != 0)
                {
                    read NVRAM, $blanks, (($device_cnt * 8) % 16);  #reserved
                }
                #read NVRAM, $blanks, 8;
               # print "RAID\n";

            }elsif ($rec_type == 0x0A)
            {
    
                print $fh "\n--- Begin Virtual Device record structure ---------------------------\n";
                #$self->PrintBase($fh);
                $vdisk_num++;
                # Virtual device ID         <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Virtual Disk ID of owner: 0x%04x\n", $nvr_file);

                # Attribute                 <b>
                my $atrbt;
                read NVRAM, $atrbt, 1;
                $atrbt = unpack "C", $atrbt;
                printf($fh "Attribute: 0x%02x\n", $atrbt);

                # Number of RAIDs           <b>
                my $raid_cnt;
                read NVRAM, $raid_cnt, 1;
                $raid_cnt = unpack "C", $raid_cnt;
                printf($fh "Number of RAIDs: 0x%02x\n", $raid_cnt);
        
                # Device capacity           <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Device capacity: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Cache enable              <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Cache enable: 0x%02x\n", $nvr_file);

                # DRAID COUNT              <b>
                my $draid_count;
                read NVRAM, $nvr_file, 1;
                $draid_count = unpack "C", $nvr_file;
                printf($fh "Defered raid Count: 0x%02x\n", $draid_count);

                # VDisk VLAR count              <b>
                my $vlar_count;
                read NVRAM, $nvr_file, 1;
                $vlar_count = unpack "C", $nvr_file;
                printf($fh "VDisk VLAR count: 0x%02x\n", $vlar_count);
            
                read NVRAM, $blanks, 13;  #reserved
            
                my $i;
                for ($i = 0; $i < $raid_cnt; $i++)
                {
                    # RAID device ID            <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "Raid ID: 0x%04x\n", $nvr_file);
                }
            
                if ($raid_cnt != 0)
                {
                    if ((($raid_cnt * 2) % 16) != 0)
                    {
                        read NVRAM, $blanks, (16 - (($raid_cnt * 2) % 16));  #reserved
                    }
                }

                for ($i = 0; $i < $draid_count; $i++)
                {
                    # RAID device ID            <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "Defered Raid ID: 0x%04x\n", $nvr_file);
                }
            
                if ($draid_count != 0)
                {
                    if ((($draid_count * 2) % 16) != 0)
                    {
                        read NVRAM, $blanks, (16 - (($draid_count * 2) % 16));  #reserved
                    }
                }
            
                for ($i = 0; $i < $vlar_count; $i++)
                {
                    # Source Controller Serial Number            <w>
                    read NVRAM, $nvr_file, 4;
                    $nvr_file = unpack "L", $nvr_file;
                    printf($fh "Source Controller Serial Number: 0x%08x\n", $nvr_file);

                    # Source Controller Cluster Number            <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Source Controller Cluster Number: 0x%02x\n", $nvr_file);

                    # Source Controller vdisk Number            <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Source Controller vdisk Number: 0x%02x\n", $nvr_file);

                    # Attributes            <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Attributes: 0x%02x\n", $nvr_file);

                    # VLink Poll Timer Count            <b>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "VLink Poll Timer Count: 0x%02x\n", $nvr_file);

                    # Reported Vdisk Number            <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "Reported Vdisk Number: 0x%04x\n", $nvr_file);

                    # Name            <s>
                    read NVRAM, $nvr_file, 52;
                    #$nvr_file = unpack "S", $nvr_file;
                    printf($fh "Name: $nvr_file\n");

                    # Agent Serial Number            <w>
                    read NVRAM, $nvr_file, 4;
                    $nvr_file = unpack "L", $nvr_file;
                    printf($fh "Agent Serial Number: 0x%08x\n", $nvr_file);
                }
            
                if ($atrbt == 0x40)
                {
                    read NVRAM, $blanks, 64;
                }
            
               #    print "VDISK\n";

            }elsif ($rec_type == 0x08)
            {
                print $fh "\n--- Begin Server Device record structure ----------------------------\n";
                #$self->PrintBase($fh);
                $server_num++;

                # Server device ID          <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Server device ID: 0x%04x\n", $nvr_file);

                # Number of LUNs            <s>
                my $lun_cnt;
                read NVRAM, $lun_cnt, 2;
                $lun_cnt = unpack "S", $lun_cnt;
                printf($fh "Number of LUNs: 0x%04x\n", $lun_cnt);

                # Target Server is mapped to        <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "TID: 0x%04x\n", $nvr_file);
            
                # Status                    <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Status: 0x%02x\n", $nvr_file);
    
                # Priority                  <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Priority: 0x%02x\n", $nvr_file);

                # Owner     <L>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Owner: 0x%08x\n", $nvr_file);

                # Server WWN                <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Server WWN: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Attributes           <L>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Attributes: 0x%08x\n", $nvr_file);

                read NVRAM, $blanks, 4;
            
                my $i;
                for ($i = 0; $i < $lun_cnt; $i++)
                {
                    # Virtual device ID         <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "Virtual device ID: 0x%04x\n", $nvr_file);

                    # LUN                       <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "LUN: 0x%04x\n", $nvr_file);

                    #read NVRAM, $blanks, 12;  #reserved
                }
                if ($lun_cnt != 0)
                {
                    if ((($lun_cnt * 4) % 16) != 0)
                    {
                        read NVRAM, $blanks, (16 - (($lun_cnt * 4) % 16));  #reserved
                    }
                }
            
                
                # print "SERVER\n";
        
            }elsif ($rec_type == 0x0E)
            {
                print $fh "\n--- Begin Target Device record structure ----------------------------\n";
                #$self->PrintBase($fh);

                # Target ID                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Target ID: 0x%04x\n", $nvr_file);

                # Channel                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Port: 0x%02x\n", $nvr_file);

                # Options                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Options: 0x%02x\n", $nvr_file);

                # FC ID                     <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "FC ID: 0x%02x\n", $nvr_file);

                read NVRAM, $blanks, 3;
            
                # Owner                 <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Owner: 0x%08x\n", $nvr_file);

                # Port name                 <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Port name: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Node name                 <l>
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Node name: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Previous Owner                 <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Previous Owner: 0x%08x\n", $nvr_file);

                # Cluster                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Cluster: 0x%04x\n", $nvr_file);
            
               #    print "TARGET\n";
                
                read NVRAM, $blanks, 10;
            }elsif ($rec_type == 0x0F)
            {
                print $fh "\n--- Begin Xiotech LDD record structure ----------------------------\n";
                #$self->PrintBase($fh);
    
                # LDD ID                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "LDD ID: 0x%04x\n", $nvr_file);

                # Path Mask                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Path Mask: 0x%02x\n", $nvr_file);

                # Path Priority                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Path Priority: 0x%02x\n", $nvr_file);

                # Device capacity           <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Device capacity: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Serial number             <t>
                read NVRAM, $nvr_file, 12;
                #$nvr_file = unpack "L", $nvr_file;
                printf($fh "Serial number: $nvr_file\n");

                # Base Virtual Disk Number                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "Base Virtual Disk Number: 0x%04x\n", $nvr_file);

                # Base Cluster Number                     <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Base Cluster Number: 0x%02x\n", $nvr_file);

                #reserved
                read NVRAM, $blanks, 1;
            
                # Base Node World Wide Name                 <l>
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Base Node World Wide Name: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Base Serial Number                 <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Base Serial Number: 0x%08x\n", $nvr_file);

                # LUN                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "LUN: 0x%04x\n", $nvr_file);
            
                # reserved
                read NVRAM, $blanks, 2;

                #   print "XIOTECH LDD\n";
            
            
            }elsif ($rec_type == 0x10)
            {
                print $fh "\n--- Begin Foreign LDD record structure ----------------------------\n";
                #$self->PrintBase($fh);

                # LDD ID                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "LDD ID: 0x%04x\n", $nvr_file);

                # Path Mask                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Path Mask: 0x%02x\n", $nvr_file);

                # Path Priority                   <b>
                read NVRAM, $nvr_file, 1;
                $nvr_file = unpack "C", $nvr_file;
                printf($fh "Path Priority: 0x%02x\n", $nvr_file);

                # Device capacity           <l>
                my @nvr_array;
                read NVRAM, $nvr_file, 8;
                @nvr_array = unpack "LL", $nvr_file;
                printf($fh "Device capacity: 0x%08x%08x\n", $nvr_array[1], $nvr_array[0]);

                # Serial number             <t>
                read NVRAM, $nvr_file, 12;
                #$nvr_file = unpack "L", $nvr_file;
                printf($fh "Serial number: $nvr_file\n");

                # Vendor ID                 <l>
                read NVRAM, $nvr_file, 8;
                #@nvr_array = unpack "LL", $nvr_file;
                printf($fh "Vendor ID: $nvr_file\n");

                # Product ID                <q>     
                my @ID = ();
                my $id_name = "";
                my $x;
                for($x = 0; $x < 16; $x++)
                {
                    read NVRAM, $nvr_file, 1;
                    push @ID, unpack "A", $nvr_file;
                    $id_name = "$id_name$nvr_file";
                }

                printf($fh "Product ID: $id_name\n");
            
                # Revision                 <l>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Revision: 0x%08x\n", $nvr_file);

                # LUN                 <s>
                read NVRAM, $nvr_file, 2;
                $nvr_file = unpack "S", $nvr_file;
                printf($fh "LUN: 0x%04x\n", $nvr_file);
            
                # reserved
                read NVRAM, $blanks, 6;

               #    print "FOREIGN LDD\n";
            
            
            }elsif ($rec_type == 0x11)
            {
                print $fh "\n--- Begin Local Image record structure ----------------------------\n";
                #$self->PrintBase($fh);

                # Length                 <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Length: 0x%08x\n", $nvr_file);

                # Controller ID                 <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Controller ID: 0x%08x\n", $nvr_file);

                # Mirror Partner                 <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Mirror Partner: 0x%08x\n", $nvr_file);

                # reserved
                read NVRAM, $blanks, 4;

                # Number of Raid Extensions                   <b>
                my $numRaids;
                my $totPsd = 0;
            
                read NVRAM, $nvr_file, 4;
                $numRaids = unpack "L", $nvr_file;
                printf($fh "Number of Raid Extensions: 0x%08x\n", $numRaids);
            
                my $i;
                for ($i = 0; $i < $numRaids; $i++)
                {
                    # Raid ID         <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh " Raid ID: 0x%04x\n", $nvr_file);

                    # PSD's                       <s>
                    my $psd;
                    read NVRAM, $nvr_file, 2;
                    $psd = unpack "S", $nvr_file;
                    printf($fh " PSD's: 0x%04x\n", $psd);
        
                    $totPsd += $psd;

                    # Additional Status                       <s>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "Additional Status: 0x%02x\n", $nvr_file);

                    read NVRAM, $blanks, 3;
                    
                    my $j;

                    for ($j = 0; $j < $psd; ++$j)
                    {
                        read NVRAM, $nvr_file, 2;
                        $nvr_file = unpack "S", $nvr_file;
                        printf($fh "  Physical ID: 0x%04x\n", $nvr_file);

                        read NVRAM, $nvr_file, 1;
                        $nvr_file = unpack "C", $nvr_file;
                        printf($fh "  Status: 0x%02x\n", $nvr_file);    
        
                        # Additional Status                       <s>
                        read NVRAM, $nvr_file, 1;
                        $nvr_file = unpack "C", $nvr_file;
                        printf($fh "Additional Status: 0x%02x\n", $nvr_file);
                   
                    }
                }

                # Number of Drive Status Extentions                   <b>
                my $drvSts;
                read NVRAM, $nvr_file, 4;
                $drvSts = unpack "L", $nvr_file;
                printf($fh "Number of Drive Status Extensions: 0x%08x\n", $drvSts);
            

                for ($i = 0; $i < $drvSts; $i++)
                {
                    # PID         <s>
                    read NVRAM, $nvr_file, 2;
                    $nvr_file = unpack "S", $nvr_file;
                    printf($fh "  PID: 0x%04x\n", $nvr_file);

                    # Status                       <s>
                    read NVRAM, $nvr_file, 1;
                    $nvr_file = unpack "C", $nvr_file;
                    printf($fh "  Status: 0x%02x\n", $nvr_file);

                    # reserved
                    read NVRAM, $blanks, 1;
                
                }
            
                if (($numRaids + $totPsd + $drvSts) != 0)
                {
                    my $totalBytes = (($numRaids + $totPsd + $drvSts + 2) * 4);
                    read NVRAM, $blanks, (16 - ($totalBytes % 16));  #reserved
                }
            

                # Number of Rebuild Extensions                  <w>
                #   read NVRAM, $nvr_file, 4;
                #   $nvr_file = unpack "L", $nvr_file;
                #   printf($fh "Number of Rebuild Extensions: 0x%08x\n", $nvr_file);
            
                # Raid ID Being Rebuilt                  <s>
                #   read NVRAM, $nvr_file, 2;
                #   $nvr_file = unpack "S", $nvr_file;
                #   printf($fh "Raid ID Being Rebuilt: 0x%04x\n", $nvr_file);
            
                # Physical ID Being Rebuilt                  <s>
                #   read NVRAM, $nvr_file, 2;
                #   $nvr_file = unpack "S", $nvr_file;
                #   printf($fh "Physical ID Being Rebuilt: 0x%04x\n", $nvr_file);
            
                # Rebuild Length                  <w>
                #   read NVRAM, $nvr_file, 4;
                #   $nvr_file = unpack "L", $nvr_file;
                #   printf($fh "Rebuild Length: 0x%08x\n", $nvr_file);
            
                #   print "LOCAL IMAGE\n";
            
            
            }elsif ($rec_type == 0x12)
            {
                print $fh "\n--- Begin Mirror Partner record structure ----------------------------\n";
                #$self->PrintBase($fh);

                # Length                 <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "My Serial Number: 0x%08x\n", $nvr_file);

                # Controller ID                 <w>
                read NVRAM, $nvr_file, 4;
                $nvr_file = unpack "L", $nvr_file;
                printf($fh "Partener Serial Number: 0x%08x\n", $nvr_file);
                
                read NVRAM, $blanks, 4;
            
            }else
            {
                print "Unknown record type found:  ";
                printf("0x%04x%02x%02x\n", $rec_length, $rec_type, $rec_status );
            }

            $self->ReadBase();
        }

        print $fh "\n\n--------------------- NVRAM Summary ----------------------------------------\n";
        print $fh "Number of Physical Drives: $drive_num\n";
        print $fh "Number of Virtual Disks: $vdisk_num\n";
        print $fh "Number of Servers: $server_num\n";
        print $fh "Number of Enclosures: $enclosure_num\n";
        print $fh "Total capcity of all Seagate drives: $total_cap GB\n";
    
    }
    
    print $fh "\n\n--------------------- END OF NVRAM DUMP ----------------------------------------\n";
    
    if($fh ne *STDOUT) {
        close $fh;
    }

    return PI_GOOD;
}


sub ReadingFWHeader
{
    my ($self, $fh) = @_;
    read NVRAM, $blanks, 32;

    # magicNumber               static      0x08
    my $magicNumber;
    read NVRAM, $magicNumber, 4;
    $magicNumber = unpack "L", $magicNumber;
    printf($fh "magicNumber: 0x%08x\n", $magicNumber);

    # rsvd1                     
    my $rsvd1;
    read NVRAM, $rsvd1, 4;
    $rsvd1 = unpack "L", $rsvd1;
    printf($fh "rsvd1: 0x%08x\n", $rsvd1);

    # productID   (1000=Tbolt)  
    my $productID;
    read NVRAM, $productID, 4;
    $productID = unpack "L", $productID;
    printf($fh "productID: 0x%08x\n", $productID);

    # target      (1=emc)       
    my $target;
    read NVRAM, $target, 4;
    $target = unpack "L", $target;
    printf($fh "target: 0x%08x\n", $target);
            
    # revision                  
    my @revision_a;
    my $revision;
    read NVRAM, $revision, 4;   
    @revision_a = unpack "A4", $revision;
    print $fh "revision: @revision_a\n";

    # revCount                  
    my @revCount_a;
    my $revCount;
    read NVRAM, $revCount, 4;
    @revCount_a = unpack "A4", $revCount;
    print $fh "revCount: @revCount_a\n";

    # buildID                   
    my @buildID_a;
    my $buildID;
    read NVRAM, $buildID, 4;
    @buildID_a = unpack "A4", $buildID;
    print $fh "buildID: @buildID_a\n";

    # vendorID                  
    my $vendorID;
    read NVRAM, $vendorID, 4;
    $vendorID = unpack "L", $vendorID;
    printf($fh "vendorID: 0x%08x\n", $vendorID);

            
    # timestamp.year            
    my $year;
    read NVRAM, $year, 2;
    $year = unpack "S", $year;

    # timestamp.month           
    my $month;
    read NVRAM, $month, 1;
    $month = unpack "C", $month;

    # timestamp.date            
    my $date;
    read NVRAM, $date, 1;
    $date = unpack "C", $date;

    # timestamp.day             
    my $day;
    read NVRAM, $day, 1;
    $day = unpack "C", $day;

    # timestamp.hours                 
    my $hours;
    read NVRAM, $hours, 1;
    $hours = unpack "C", $hours;

    # timestamp.minutes         
    my $minutes;
    read NVRAM, $minutes, 1;
    $minutes = unpack "C", $minutes;

    # timestamp.seconds         
    my $seconds;
    read NVRAM, $seconds, 1;
    $seconds = unpack "C", $seconds;

    printf ($fh "Build Date: %x/%x/%x ", $month, $day, $year);
    printf ($fh "%02x:%02x:%02x\n", $hours, $minutes, $seconds);

    # rsvd2                     
    my $rsvd2;
    read NVRAM, $rsvd2, 4;
    $rsvd2 = unpack "L", $rsvd2;
    printf($fh "rsvd2: 0x%08x\n", $rsvd2);
    
    # burnSequence              
    my $burnSequence;
    read NVRAM, $burnSequence, 4;
    $burnSequence = unpack "L", $burnSequence;
    printf($fh "burnSequence: 0x%08x\n", $burnSequence);

    # loadID.emcAddrA
    my $emcAddrA;
    read NVRAM, $emcAddrA, 4;
    $emcAddrA = unpack "L", $emcAddrA;
    printf($fh "emcAddrA: 0x%08x\n", $emcAddrA);

    # loadID.emcAddrB           
    my $emcAddrB;
    read NVRAM, $emcAddrB, 4;
    $emcAddrB = unpack "L", $emcAddrB;
    printf($fh "emcAddrB: 0x%08x\n", $emcAddrB);

    # loadID.targAddr           
    my $targAddr;
    read NVRAM, $targAddr, 4;
    $targAddr = unpack "L", $targAddr;
    printf($fh "targAddr: 0x%08x\n", $targAddr);

    # loadID.length             
    my $length;
    read NVRAM, $length, 4;
    $length = unpack "L", $length;
    printf($fh "length: 0x%08x\n", $length);

    # loadID.checksum           
    my $checksum;
    read NVRAM, $checksum, 4;
    $checksum = unpack "L", $checksum;
    printf($fh "checksum: 0x%08x\n", $checksum);

    # loadID.compatibilityID    
    my $compatibilityID;
    read NVRAM, $compatibilityID, 4;
    $compatibilityID = unpack "L", $compatibilityID;
    printf($fh "compatibilityID: 0x%08x\n", $compatibilityID);

    # rsvd3[0]                  
    my $rsvd30;
    read NVRAM, $rsvd30, 4;
    $rsvd30 = unpack "L", $rsvd30;
    printf($fh "rsvd30: 0x%08x\n", $rsvd30);

    # rsvd3[1]                  
    my $rsvd31;
    read NVRAM, $rsvd31, 4;
    $rsvd31 = unpack "L", $rsvd31;
    printf($fh "rsvd31: 0x%08x\n", $rsvd31);

    # rsvd3[2]                  
    my $rsvd32;
    read NVRAM, $rsvd32, 4;
    $rsvd32 = unpack "L", $rsvd32;
    printf($fh "rsvd32: 0x%08x\n", $rsvd32);

    # rsvd3[3]
    my $rsvd33;
    read NVRAM, $rsvd33, 4;
    $rsvd33 = unpack "L", $rsvd33;
    printf($fh "rsvd33: 0x%08x\n", $rsvd33);

    # rsvd3[4]                  
    my $rsvd34;
    read NVRAM, $rsvd34, 4;
    $rsvd34 = unpack "L", $rsvd34;
    printf($fh "rsvd34: 0x%08x\n", $rsvd34);

    # hdrCksum                  
    my $hdrCksum;
    read NVRAM, $hdrCksum, 4;
    $hdrCksum = unpack "L", $hdrCksum;
    printf($fh "hdrCksum: 0x%08x\n\n", $hdrCksum);

}

sub ReadBase
{
    my ($self) = @_;

    # Record length             <s>
    read NVRAM, $rec_length, 2;
    $rec_length = unpack "S", $rec_length;
    
    # Record type               <b>
    read NVRAM, $rec_type, 1;
    $rec_type = unpack "C", $rec_type;

    # Status                    <b>
    read NVRAM, $rec_status, 1;
    $rec_status = unpack "C", $rec_status;
}

sub PrintBase
{
    my ($self, $fh) = @_;

    printf($fh "Record length: 0x%04x\n", $rec_length);
    printf($fh "Record type: 0x%02x\n", $rec_type);
    printf($fh "Status: 0x%02x\n", $rec_status);
}

1;
##############################################################################
# Change log:
# $Log$
# Revision 1.1  2005/05/04 18:53:56  RysavyR
# Initial revision
#
# Revision 1.9  2003/04/22 13:39:32  TeskeJ
# tbolt00008122 - 'pdisks loop' updates and changed 'channel' to 'port'
# rev by Chris
#
# Revision 1.8  2003/02/05 20:43:12  TeskeJ
# tbolt00006870 - nvramread command displays wrong pdisk info.
# Structures mismatched between perl & nvr.h.
# rev by Bryan
#
# Revision 1.7  2003/01/31 20:35:04  SchibillaM
# TBolt00007023: Fix NVRAM dump to properly display CRC and timestamp.
#
# Revision 1.6  2002/11/19 21:14:55  NigburC
# TBolt00006343 - Modified the PDISK INFO response packet definition to
# match what the PROC is now returning.  The size of the structure did not
# change but the contents did.  The SES and SLOT information changed
# wherein two values became reserved and the names of the others changed.
# Reviewed by Jeff Williams.
#
# Revision 1.5  2002/02/20 20:34:36  HoltyB
# made changes to coincide with the existing structure of nvram
#
# Revision 1.4  2002/02/14 19:22:09  HoltyB
# relabeled some of the registers
#
# Revision 1.3  2002/01/22 15:02:04  HoltyB
# updated for support of FE memory displays
#
# Revision 1.2  2002/01/21 20:42:24  HoltyB
# added some smarts in the printing procedures to produce better performance
#
# Revision 1.1  2002/01/21 18:34:42  HoltyB
# initial integration
#
###############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
