#!/mksnt/perl -w
#====================================================================
#
# FILE NAME:    FrDec.pl
#
# AUTHOR:       Tim Swatosh
#
# DATE:         10/19/2003
#
# DESCRIPTION:  Decodes/annotates a Flight Recorder trace dump file.
#
#====================================================================

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
$count = 0;

READFILE:
while(read F, $buffer, 16) {

    ++$count;
    if ($count == 1)
    {
        ($begin, $in, $out, $end) = unpack "LLLL", $buffer;

        $wrapCount = (($in - $begin)/16) + $count;
        printf OUT "Wrap count = %d\n", $wrapCount;
        next;
    }


    if ($count == $wrapCount)
    {
        printf OUT " ***** WRAP POINT **** \n";
    }

    ($id, $sdata, $data1, $data2, $data3) = unpack "SSLLL", $buffer;

    #
    # If we are at the end of the data or suspect a bad entry, exit.
    # We are only looking at one value here, we could validate others...
    #
    if($id == 0 || $id == 0xFFFF) {
#        last;
        next;
    }


    #
    # Figure out which class trace point this is; decode if possible.
    #
    $idDesc = sprintf "0x%08X", $id;
    $dataDesc = sprintf "0x%08X 0x%08X 0x%08X", $data1, $data2, $data3;
   
    # ISP entry
    if (($id & 0x00F0) == 0x0010)
    {
        if ($id == 0x0010)
        {
            $idDesc  = "ISP IO:";
        }
        elsif ($id == 0x0011)
        {
            $idDesc  = "ISP CDB:";
        }
        elsif ($id == 0x0012)
        {
            $idDesc  = "ISP Buffer:";
        }
        elsif ($id == 0x0013)
        {
            $idDesc  = "ISP PDBC:";
        }
        elsif ($id == 0x0014)
        {
            $idDesc  = "ISP Thread:";
        }
        elsif ($id == 0x0015)
        {
            $idDesc  = "ISP Unth:";
        }
    }
    # Physical  entry
    elsif (($id & 0xFFFF) == 0xA026) 
    {
        $idDesc = sprintf "FPM(%02X) ", $sdata>>8;
        if ($sdata == 0x0000)
        {
            $idDesc  = $idDesc."Loop up:";
            $dataDesc  = sprintf "Port= 0x%08X  unused= 0x%08X DevCnt= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0100)
        {
            $idDesc  = $idDesc."Spawn Task:";
            $dataDesc  = sprintf "Port= 0x%08X  MonPCB= 0x%08X DisPCB= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0200)
        {
            $idDesc  = $idDesc."Task Rdy:";
            $dataDesc  = sprintf "Port= 0x%08X  MonPCB= 0x%08X DisPCB= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0300)
        {
            $idDesc  = $idDesc."Loop Start:";
            $dataDesc  = sprintf "Port= 0x%08X  DevNum= 0x%08X DevCnt= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0400)
        {
            $idDesc  = $idDesc."Loop Exit:";
            $dataDesc  = sprintf "Port= 0x%08X  unused= 0x%08X chnState= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0500)
        {
            $idDesc  = $idDesc."Loop GetPortDB:";
            $dataDesc  = sprintf "portDB->sst= 0x%08X  portDB->mst= 0x%08X portDB->prliw3= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0600)
        {
            $idDesc  = $idDesc."Loop LoginLoopPort:";
            $dataDesc  = sprintf "rc= 0x%08X  portDB->mst= 0x%08X portDB->prliw3= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0700)
        {
            $idDesc  = $idDesc."Loop Mid:";
            $dataDesc  = sprintf "rc= 0x%08X  portDB->mst= 0x%08X portDB->prliw3= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0800)
        {
            $idDesc  = $idDesc."Loop chkInitiator:";
            $dataDesc  = sprintf "rc= 0x%08X  portDB->ndn[0]= 0x%08X portDB->ndn[1]= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0A00)
        {
            $idDesc  = $idDesc."Lip Issued:";
            $dataDesc  = sprintf "Port= 0x%08X  lipIssued= 0x%08X unused= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0C00)
        {
            $idDesc  = $idDesc."Loop Down Lip:";
            $dataDesc  = sprintf "Port= 0x%08X  lipIssued= 0x%08X unused= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0x0F00)
        {
            $idDesc  = $idDesc."Exit:";
            $dataDesc  = sprintf "Port= 0x%08X  MonPCB= 0x%08X DisPCB= 0x%08X ", 
                             $data1,$data2,$data3;
        }
    }
    elsif (($id & 0xFFFF) == 0xB026) 
    {
        $idDesc  = $idDesc."FPM Loop down:";
        $dataDesc  = sprintf "Port= 0x%08X  unused= 0x%08X unused= 0x%08X ", 
                         $data1,$data2,$data3;
    }
    # FAB_ProcessRSCN
    elsif (($id & 0xF0FF) == 0xE026) 
    {
        $idDesc = sprintf "FAB_RSCN(%02X) ", (($sdata>>16)& 0xF0);
        if ($id == 0xE026)
        {
            $idDesc  = $idDesc."GNN_FT:";
            $dataDesc  = sprintf "Port= 0x%08X  rc= 0x%08X nsCnt= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE126)
        {
            $idDesc  = $idDesc."Exit loop down :";
            $dataDesc  = sprintf "Port= 0x%08X  unused= 0x%08X chState= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE226)
        {
            $idDesc  = $idDesc."Loop-Get port name:";
            $dataDesc  = sprintf "Port= 0x%08X  rc= 0x%08X portID= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE326)
        {
            $idDesc  = $idDesc."Loop-login pre primary:";
            $dataDesc  = sprintf "Port= 0x%08X  unused= 0x%08X portID= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE426)
        {
            $idDesc  = $idDesc."Loop-login post primary:";
            $dataDesc  = sprintf "Port= 0x%08X  rc= 0x%08X portID= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE526)
        {
            $idDesc  = $idDesc."Loop-login NOT primary:";
            $dataDesc  = sprintf "Port= 0x%08X  rc= 0x%08X portID= 0x%08X ", 
                             $data1,$data2,$data3;
        }
        elsif ($sdata == 0xE626)
        {
            $idDesc  = $idDesc."Exit:";
            $dataDesc  = sprintf "Port= 0x%08X  chState= 0x%08X nsCnt= 0x%08X ", 
                             $data1,$data2,$data3;
        }
    }
    # Physical  entry
    elsif (($id & 0x00F0) == 0x0020) 
    {
        if ($id == 0x0020)
        {
            $idDesc  = "PHY Exec:";
        }
        elsif ($id == 0x0021)
        {
            $idDesc  = "PHY Comp:";
        }
        elsif ($id == 0x0022)
        {
            $idDesc  = "PHY Cancel:";
        }
        elsif ($id == 0x0023)
        {
            $idDesc  = "PHY Join:";
        }
        elsif ($id == 0x0024)
        {
            $idDesc  = "PHY GetILT:";
        }
        elsif ($id == 0x1026)
        {
            $idDesc  = "PHY Crt Dev:";
        }
        elsif ($id == 0x2026)
        {
            $idDesc = sprintf "PHY FndDev(%02X) ", $sdata>>8;
            if ($sdata == 0x0000)
            {
                $idDesc  = $idDesc."Start:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X WWN= 0x%08X%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data3,$data2;
            }
            elsif ($sdata == 0x0100)
            {
                $idDesc  = $idDesc."Found:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pLid= 0x%08X dev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
            elsif ($sdata == 0x0200)
            {
                $idDesc  = $idDesc."NOT Found:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pdd= 0x%08X pdd->pDev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
            elsif ($sdata == 0x0300)
            {
                $idDesc  = $idDesc."NOT Found:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pLid= 0x%08X dev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
            elsif ($sdata == 0x0400)
            {
                $idDesc  = $idDesc."NOT Found:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pLid= 0x%08X dev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
            elsif ($sdata == 0x0500)
            {
                $idDesc  = $idDesc."NOT Found:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pLid= 0x%08X dev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
            elsif ($sdata == 0x0900)
            {
                $idDesc  = $idDesc."Exit:";
                $dataDesc  = sprintf "Port= 0x%02X Lun = 0x%02X Lid= 0x%02X pLid= 0x%08X dev= 0x%08X ", 
                                 ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,$data2,$data3;
            }
        }
        elsif ($id == 0x3026)
        {
            $idDesc  = "PHY Init Drv:";
        }
        elsif ($id == 0x4026)
        {
            $idDesc  = "PHY Mov Dev:";
        }
        elsif ($id == 0x5026)
        {
            $idDesc  = "PHY Rm Dev:";
        }
        elsif ($id == 0xC026)
        {
        
            $idDesc = sprintf "PHY DISC(%02X) ", $sdata;

            if ($sdata == 0x0000)
            {
                $idDesc  = $idDesc."Start:";
                $dataDesc  = sprintf "NotifyReq= 0x%08X fpmPCB = 0x%08X fdiscPCB= 0x%08X ", 
                                 $data1,$data2,$data3;
            }
            elsif ($sdata == 0x0001)
            {
                $idDesc  = $idDesc."Spawn Task:";
                $dataDesc  = sprintf "unused= 0x%08X fpmPCB = 0x%08X fdiscPCB= 0x%08X ", 
                                 $data1,$data2,$data3;
            }
            elsif ($sdata == 0x0002)
            {
                $idDesc  = $idDesc."Task Active:";
                $dataDesc  = sprintf "unused= 0x%08X fpmPCB = 0x%08X fdiscPCB= 0x%08X ", 
                                 $data1,$data2,$data3;
            }
            elsif ($sdata == 0x0003)
            {
                $idDesc  = $idDesc."Exit:";
                $dataDesc  = sprintf "unused= 0x%08X fpmPCB = 0x%08X fdiscPCB= 0x%08X ", 
                                 $data1,$data2,$data3;
            }
        }

        elsif (($id & 0x000000FF)== 0x27)
        {
            $idDesc  = "PHY Retry:";
            $dataDesc  = sprintf "Retry= 0x%02X Port= 0x%02X QL stat= 0x%02X CMD= 0x%08X%08X DEV= %08X", 
                    ($id>>8)&0x00FF,($id>>16)&0x00FF, ($id>>16)&0x00FF,$data1,$data2,$data3;
        }
        elsif ($id == 0x0028)
        {
            $idDesc  = "PHY Chk Cond:";
        }
        elsif ($id == 0x0029)
        {
            $idDesc  = "PHY Chk4Retry:(0029)";
            $dataDesc  = sprintf "Port= 0x%04X LoopID= 0x%04X Reason= 0x%04X Reason= 0x%04X  %08X", 
                    ($data1&0xFFFF),($data1>>16), ($data2&0xFFFF),($data2>>16),$data3;
        }
        elsif ($id == 0x1029)
        {
            $idDesc  = "PHY Chk4Retry:(1029)";
            $dataDesc  = sprintf "Port= 0x%04X LoopID= 0x%04X Reason= 0x%04X Reason= 0x%04X  %08X", 
                    ($data1&0xFFFF),($data1>>16), ($data2&0xFFFF),($data2>>16),$data3;
        }
        elsif ($id == 0x002A)
        {
            $idDesc  = "PHY Chk4Retry:(002A)";
            $dataDesc  = sprintf "QL ST= 0x%02X Req ST= 0x%02X SCSI ST= 0x%02X DEV WC= 0x%04X Retry= 0x%02X Port= 0x%02X DEV= 0x%08X", 
                    ($data1)&0x00FF,($data1>>8)&0x00FF, ($data1>>16)&0x00FF,($data2&0xFFFF),($data2>>16)&0x00FF,($data2>>24)&0x00FF,$data3;
        }
    }
    # Hotswap entry
    elsif (($id & 0x00FF) == 0x00d0)
    {
        if ($id == 0x00d0)
        {
            $idDesc  = "HOTSWAP: TASK";
        }
        elsif ($id == 0x01d0)
        {
            $idDesc  = "HOTSWAP: Top ";
        }
        elsif ($id == 0x02d0)
        {
            $idDesc  = "HOTSWAP: Missing Drive ";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X OLD DEVST= 0x%08X", $data1,$data2,$data3;
        }
        elsif ($id == 0x03d0)
        {
            $idDesc  = "HOTSWAP: Inquire Complete ";
        }

    }
    # Inquire entry
    elsif (($id & 0x00FF) == 0x00d1)
    {
        if ($id == 0x00d1)
        {
            $idDesc  = "INQUIRE: Start";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  0x%08X", $data1,$data2,$data3;
        }
        elsif ($id == 0x01d1)
        {
            $idDesc  = "INQUIRE: Pre-Init Drive ";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  PDX= 0x%08X", $data1,$data2,$data3;
        }
        elsif ($id == 0x02d1)
        {
            $idDesc  = "INQUIRE: Post-Init Drive (op) ";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  DEVST= 0x%08X", $data1,$data2,$data3;
        }
        elsif ($id == 0x03d1)
        {
            $idDesc  = "INQUIRE: Post-Init Drive (nonop) ";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  DEVST= 0x%08X", $data1,$data2,$data3;
        }

    }
    # Init Drive entry
    elsif (($id & 0x00FF) == 0x00d2)
    {
        if ($id == 0x00d2)
        {
            $idDesc  = "INITDRV: Start";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  TEST TYPE= 0x%08X", $data1,$data2,$data3;
        }
        elsif ($id == 0x01d2)
        {
            $idDesc  = "INITDRV: End";
            $dataDesc  = sprintf "PID= 0x%08X PDD= 0x%08X  STATUS= 0x%08X", $data1,$data2,$data3;
        }
    }
    # Misc Rebuild and device status entry
    elsif (($id & 0x00FF) == 0x00d3)
    {
        if ($id == 0x00d3)
        {
            $idDesc  = "HOTSPARE_PSD:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  STAT= 0x%02X ASTAT= 0x%02X", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3>>8, $data3&0x00FF;
        }
        elsif ($id == 0x01d3)
        {
            $idDesc  = "P2UPDATE:";
        }
        elsif ($id == 0x02d3)
        {
            $idDesc  = "CanRebuild:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RESULT= 0x%08X", $data1>>16, $data1&0x0000FFFF,
                                                                                                    $data2,$data3;
        }
        elsif ($id == 0x03d3)
        {
            $idDesc  = "CanSpare:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RESULT= 0x%08X", $data1>>16, $data1&0x0000FFFF,
                                                                                                    $data2,$data3;
        }
        elsif ($id == 0x04d3)
        {
            $idDesc  = "FindHotSpare:";
            $dataDesc  = sprintf "CAPH= 0x%08X CAPL= 0x%08X HSPSD= 0x%08X", $data1, $data2,$data3;
        }
        elsif ($id == 0x05d3)
        {
            $idDesc  = "SetRaidStat:";
            $dataDesc  = sprintf "RID= 0x%04X  0x%04X RDD= 0x%08X  RD= 0x%02X RDA= 0x%02X 0x%04X", $data1>>16, $data1&0x0000FFFF,
                                                                     $data2,$data3>>24, ($data3>>16)&0x000000FF,$data3&0x0000FFFF;
        }
        elsif ($id == 0x06d3)
        {
            $idDesc  = "SetPSDStat:";
            $dataDesc  = sprintf "RID= 0x%04X  PID= 0x%04X PSD= 0x%08X  0x%04X PS= 0x%02X PSA= 0x%02X ", $data1>>16, $data1&0x0000FFFF,
                                                                     $data2,$data3>>16, ($data3>>8)&0x000000FF,$data3&0x000000FF;
        }
        elsif ($id == 0x07d3)
        {
            $idDesc  = "GetRaidErrStat:";
            $dataDesc  = sprintf "RID= 0x%04X  0x%04X  0x%08X  RD= 0x%02X RDA= 0x%02X 0x%04X", $data1>>16, $data1&0x0000FFFF,
                                                                     $data2,$data3>>24, ($data3>>16)&0x000000FF,$data3&0x0000FFFF;
        }
        elsif ($id == 0x08d3)
        {
            $idDesc  = "StartInitRaid:";
            $dataDesc  = sprintf "RID= 0x%04X  PID= 0x%04X RDD= 0x%08X  PSD= 0x%02X ", $data1>>16, $data1&0x0000FFFF,
                                                                     $data2,$data3;
        }
        elsif ($id == 0x09d3)
        {
            $idDesc  = "InopChk:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PDD= 0x%08X  RESULT= 0x%08X", $data1>>16, $data1&0x0000FFFF,
                                                                                                    $data2,$data3;
        }
        elsif ($id == 0x0ad3)
        {
            $idDesc  = "DefChk:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X  0x%08X  RESULT= 0x%08X", $data1>>16, $data1&0x0000FFFF,
                                                                                                    $data2,$data3;
        }
        elsif ($id == 0x0bd3)
        {
            $idDesc  = "FailDev:";
            $dataDesc  = sprintf "FailPID= 0x%08X HSPID= 0x%08X Options= 0x%08X", $data1, $data2,$data3;
        }
        elsif ($id == 0x0cd3)
        {
            $idDesc  = "ProcessSOS:";
            $dataDesc  = sprintf "NVRSOS= 0x%08X  0x%08X RESULT= 0x%08X", $data1, $data2,$data3;
        }
        elsif ($id == 0x0dd3)
        {
            $idDesc  = "P2UpdateNVRAM:";
        }
        elsif ($id == 0x0ed3)
        {
            $idDesc  = "RestoreNVRAM:";
            $dataDesc  = sprintf "NVRII= 0x%08X  PDX=0x%08X RESULT= 0x%08X", $data1, $data2,$data3;
        }
    }
    # Rebuild entry
    elsif (($id & 0x00FF) == 0x00d4)
    {
        if ($id == 0x00d4)
        {
            $idDesc  = "RebuildPSD Start:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RDD= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x01d4)
        {
            $idDesc  = "RebuildPSD:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RBR= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x02d4)
        {
            $idDesc  = "ReDirectPSD:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  HSPID= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x03d4)
        {
            $idDesc  = "PSDRebuilder Start:";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RBR= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x04d4)
        {
            $idDesc  = "PSDRebuilder Done: (Raid)";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  RBR= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x05d4)
        {
            $idDesc  = "PSDRebuilder Done: (PSD)";
            $dataDesc  = sprintf "RID= 0x%04X PID= 0x%04X PSD= 0x%08X  EC= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x06d4)
        {
            $idDesc  = "PSDRebuilder Done: (Pid)";
            $dataDesc  = sprintf " 0x%04X PID= 0x%04X PSD= 0x%08X  RBR= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
        elsif ($id == 0x07d4)
        {
            $idDesc  = "CancelRebuild:";
            $dataDesc  = sprintf "RID= 0x%04X  0x%04X  0x%08X  RBR= 0x%08X ", $data1>>16, $data1&0x0000FFFF,
                                                                                                        $data2,$data3;
        }
    }
    # Raid Error entry
    elsif (($id & 0x00FF) == 0x00d5)
    {
        if ($id == 0x00d5)
        {
            $idDesc  = "Raid Err End:";
            $dataDesc  = sprintf "RID= 0x%04X  PID= 0x%04X  PSD= 0x%08X  RD= 0x%02X RDA= 0x%02X PS= 0x%02X PS= 0x%02X", 
                       $data1>>16, $data1&0x0000FFFF,$data2,$data3>>24, ($data3>>16)&0x000000FF,
                       ($data3>>8)&0x000000FF,$data3&0x000000FF;
        }
        elsif ($id == 0x01d5)
        {
            $idDesc  = "Raid Err Exec:";
            $dataDesc  = sprintf "0x%08X 0x%08X Start/End= 0x%08X ", $data1,$data2,$data3;
        }
        elsif ($id == 0x02d5)
        {
            $idDesc  = "Raid Err Exec: (ilt)";
            $dataDesc  = sprintf "RID= 0x%04X 0x%04X RDD= 0x%08X ILT= 0x%08X ", $data1>>16,$data1&0x0000FFFF,$data2,$data3;
        }
        elsif ($id == 0x03d5)
        {
            $idDesc  = "Raid Err Start:";
            $dataDesc  = sprintf "ILT= 0x%08X PRP= 0x%08X  RSNS= 0x%08X ", $data1,$data2,$data3;
        }
    }
    # F_port Monitor
    elsif (($id & 0x00FF) == 0x0026)
    {
        if ($id == 0x00d5)
        {
            $idDesc  = "Raid Err End:";
            $dataDesc  = sprintf "RID= 0x%04X  PID= 0x%04X  PSD= 0x%08X  RD= 0x%02X RDA= 0x%02X PS= 0x%02X PS= 0x%02X", 
                       $data1>>16, $data1&0x0000FFFF,$data2,$data3>>24, ($data3>>16)&0x000000FF,
                       ($data3>>8)&0x000000FF,$data3&0x000000FF;
        }
    }

    #
    # Write the formatted data out
    #
    printf OUT "%-20s %-70s (0x%08x)\n",  $idDesc, $dataDesc,$id;
}

#
# Close files and exit
#
close F;
close OUT;
