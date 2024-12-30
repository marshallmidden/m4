# $Id: cmiSCSI.pm 144138 2010-07-14 18:45:02Z m4 $
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2006  Xiotech
# ======================================================================
#
# Purpose:
#   Wrapper for all the different XIOTech iSCSI commands that
#   can be sent to the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use XIOTech::xiotechPackets;
use XIOTech::PI_CommandCodes;
use XIOTech::seqNumber;

use XIOTech::bigNums;
use XIOTech::error;

use XIOTech::logMgr;

use strict;

##############################################################################
# Name:     iSCSISetTgtParam
#
# Desc:     Configure an iSCSI negotiable parameter for a target.
#
# Input:    paramName   - parameter name
#           paramVal    - parameter value
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSISetTgtParam
{
    my ($self,
        $tid,
        $paramId,
        $paramVal) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_SET_TGTPARAM;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $setmap=0;

    my $ip=0;
    my $subnetMask=0;
    my $gateway=0;
    my $maxConnections=0;
    my $initialR2T=0;
    my $immediateData=0;
    my $dataSequenceInOrder=0;
    my $dataPDUInOrder=0;
    my $ifMarker=0;
    my $ofMarker=0;
    my $errorRecoveryLevel=0;
    my $targetPortalGroupTag=0;
    my $maxBurstLength=0;
    my $firstBurstLength=0;
    my $defaultTime2Wait=0;
    my $defaultTime2Retain=0;
    my $maxOutstandingR2T=0;
    my $maxRecvDataSegmentLength=0;
    my $ifMarkInt=0;
    my $ofMarkInt=0;
    my $headerDigest=0;
    my $dataDigest=0;
    my $authMethod=0;
    my $mtuSize=0;
    my $tgtAlias = "";

    my $rsv1=0;
    my $rsv2=0;
    my $rsv3=0;
    my $rsv4=0;

    $setmap = 1 << $paramId;

    if ($paramId == 0)
    {
        $ip = $paramVal;
    }
    if ($paramId == 1)
    {
        $subnetMask = $paramVal;
    }
    if ($paramId == 2)
    {
        $gateway = $paramVal;
    }
    if ($paramId == 3)
    {
        $maxConnections = $paramVal;
    }
    if ($paramId == 4)
    {
        $initialR2T = $paramVal;
    }
    if ($paramId == 5)
    {
        $immediateData = $paramVal;
    }
    if ($paramId == 6)
    {
        $dataSequenceInOrder = $paramVal;
    }
    if ($paramId == 7)
    {
        $dataPDUInOrder = $paramVal;
    }
    if ($paramId == 8)
    {
        $ifMarker = $paramVal;
    }
    if ($paramId == 9)
    {
        $ofMarker = $paramVal;
    }
    if ($paramId == 10)
    {
        $errorRecoveryLevel = $paramVal;
    }
    if ($paramId == 11)
    {
        $targetPortalGroupTag = $paramVal;
    }
    if ($paramId == 12)
    {
        $maxBurstLength = $paramVal;
    }
    if ($paramId == 13)
    {
        $firstBurstLength = $paramVal;
    }
    if ($paramId == 14)
    {
        $defaultTime2Wait = $paramVal;
    }
    if ($paramId == 15)
    {
        $defaultTime2Retain = $paramVal;
    }
    if ($paramId == 16)
    {
        $maxOutstandingR2T = $paramVal;
    }
    if ($paramId == 17)
    {
        $maxRecvDataSegmentLength = $paramVal;
    }
    if ($paramId == 18)
    {
        $ifMarkInt = $paramVal;
    }
    if ($paramId == 19)
    {
        $ofMarkInt = $paramVal;
    }
    if ($paramId == 20)
    {
        $headerDigest = $paramVal;
    }
    if ($paramId == 21)
    {
        $dataDigest = $paramVal;
    }
    if ($paramId == 22)
    {
        $authMethod = $paramVal;
    }
    if ($paramId == 23)
    {
        $mtuSize = $paramVal;
    }
    if ($paramId == 24)
    {
        if ($paramVal eq "NULL")
        {
            $tgtAlias = "";
        }    
        else
        {
        $tgtAlias = $paramVal;
    }
    }

    my $data = pack("LSLLLSCCCCCCCSLLSSSLSSCCCLa32LLLS",
                    $setmap,
                    $tid,
                    $ip,
                    $subnetMask,
                    $gateway,
                    $maxConnections,
                    $initialR2T,
                    $immediateData,
                    $dataSequenceInOrder,
                    $dataPDUInOrder,
                    $ifMarker,
                    $ofMarker,
                    $errorRecoveryLevel,
                    $targetPortalGroupTag,
                    $maxBurstLength,
                    $firstBurstLength,
                    $defaultTime2Wait,
                    $defaultTime2Retain,
                    $maxOutstandingR2T,
                    $maxRecvDataSegmentLength,
                    $ifMarkInt,
                    $ofMarkInt,
                    $headerDigest,
                    $dataDigest,
                    $authMethod,
                    $mtuSize,
                    $tgtAlias,
                    $rsv1,
                    $rsv2,
                    $rsv3,
                    $rsv4);
#    my $data = pack("SCLC",
#                    $tid,
#                    $paramId,
#                    $paramVal,
#                    0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     iSCSITgtInfo
#
# Desc:     Get Info of iSCSI Target.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSITgtInfo
{
    my ($self,
        $tid) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_TGT_INFO;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SS",
                    $tid,
                    0);


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_iSCSITgtInfoPacket);
}

##############################################################################
# Name:     iSCSISetChap
#
# Desc:     Configure an iSCSI negotiable parameter for a target.
#
# Input:    paramName   - parameter name
#           paramVal    - parameter value
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSISetChap
{
    my ($self,
        $opt,
        $tid,
        $type,
        $sname,
        $key1,
        $key2) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_SET_CHAP;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $count = 1;
    my $rsvd = 0;
    my $data = pack("SSSCa256a32a32C",
                    $count,
                    $opt,
                    $tid,
                    $type,
                    $sname,
                    $key1,
                    $key2,
                    $rsvd);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}

##############################################################################
# Name:     iSCSIChapInfo
#
# Desc:     Get CHAP User info of iSCSI Target.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSIChapInfo
{
    my ($self,
        $tid) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_CHAP_INFO;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SS",
                    $tid,
                    0);


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_iSCSIChapInfoPacket);
}

##############################################################################
# Name:     iSCSISessionInfo
#
# Desc:     Get CHAP User info of iSCSI Target.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSISessionInfo
{
    my ($self,
        $tid) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_SESSION_INFO;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SS",
                    $tid,0);


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_iSCSIStatsPacket);
}

##############################################################################
# Name:     iSCSIServerSessionInfo
#
# Desc:     Get session info of iSCSI Server.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSCSIServerSessionInfo
{
    my ($self,
        $sname) = @_;

    logMsg("begin\n");

    my $cmd = PI_ISCSI_SESSION_INFO_SERVER;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("a256",
                    $sname);


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_iSCSIStatsPacket);
}

##############################################################################
# Name:     dlmPathSelectionAlgorithm
#
# Desc:     Selects the DLM Path Algorithm in case of ICL.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub dlmPathSelectionAlgorithm
{
    my ($self, $option) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0x0F],
                ["dlmPathSelectionAlgorithm"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_DLM_PATH_SELECTION_ALGO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("CCCC",
                    $option,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
#                                        \&_statusResponsePacket);
                                         \&_displaydlmPathSelectionAlgorithm);


}

##############################################################################
# Name:     dlmPathStats
#
# Desc:     Get DLM Path stats between controllers.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub dlmPathStats
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_DLM_PATH_STATS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = undef;


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_DLMPathStatsPacket);
}


##############################################################################
# Name:     IDDInfo
#
# Desc:     Get IDD info.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub IDDInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_IDD_INFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = undef;


    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_IDDInfoPacket);
}

##############################################################################
# Name:     iSNSSetParam
#
# Desc:     Configure iSNS Server.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSNSSetParam
{
    my ($self,
        $flags_dummy,
        $flags,
        $ip1,
        $port1,
        $proto1,
        $ip2,
        $port2,
        $proto2) = @_;

    logMsg("begin\n");

    my $nset = 2;
    my $cmd = PI_SETISNSINFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data;

    $data = pack ("LLLSSLSS",
                            $flags,
                            $nset,
                            $ip1,
                            $port1,
                            $proto1,
                            $ip2,
                            $port2,
                            $proto2);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT});

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_statusResponsePacket);
}


##############################################################################
# Name:     iSNSInfo
#
# Desc:     Get Info of iSNS Server information.
#
# Input:    none
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub iSNSInfo
{
    my ($self) = @_;

    logMsg("begin\n");

    my $cmd = PI_GETISNSINFO_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = undef;

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

    return $self->_handleSyncResponse($seq,
                                        $packet,
                                        \&_iSNSInfoPacket);
}




##############################################################################
# Name:     _iSNSInfoPacket 
#
# Desc:     Parses the iSNS info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _iSNSInfoPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $flags;
    my $i;

    if (commandCode($recvPacket) == PI_GETISNSINFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @isnsinfo;
        my $start = 8;
                                
        (
            $flags,
            $count,
        ) = unpack("LL", substr($parts{DATA}, $start));
        $start += 8;
            
        for($i = 0; $i<$count;$i++)
        {
            
            # Unpack the data
            (
                $isnsinfo[$i]{IP},
                $isnsinfo[$i]{PORT},
                $isnsinfo[$i]{PROTO},
            ) = unpack("NSS", substr($parts{DATA}, $start));
            $start += 8;
        }
        $info{COUNT} = $count;
        $info{FLAGS} = $flags;
        $info{ISNSINFO} = [@isnsinfo];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a iSNS info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _iSCSITgtInfoPacket 
#
# Desc:     Parses the iSCSI target info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _iSCSITgtInfoPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $rsvd1;
    my $rsvd2;
    my $i;

    if (commandCode($recvPacket) == PI_ISCSI_TGT_INFO)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @iscsitgtinfo;
        my $start = 8;

            (
            $count,
            $rsvd1) 
            = unpack("SS", substr($parts{DATA}, $start));

            $start += 4;
            
        for($i = 0; $i<$count;$i++)
        {
            
            # Unpack the data
            (
            $iscsitgtinfo[$i]{TID},
            $iscsitgtinfo[$i]{TGTIP},
            $iscsitgtinfo[$i]{SUBNETMASK},
            $iscsitgtinfo[$i]{GATEWAY},
            $iscsitgtinfo[$i]{MAXCONNECTIONS},
            $iscsitgtinfo[$i]{R2T},
            $iscsitgtinfo[$i]{IMMEDDATA},
            $iscsitgtinfo[$i]{DATASEQINORDER},
            $iscsitgtinfo[$i]{DATAPDUINORDER},
            $iscsitgtinfo[$i]{IFMARKER},
            $iscsitgtinfo[$i]{OFMARKER},
            $iscsitgtinfo[$i]{ERRORRECOVERY},
            $iscsitgtinfo[$i]{TPGT},
            $iscsitgtinfo[$i]{MAXBURSTLEN},
            $iscsitgtinfo[$i]{FIRSTBURSTLEN},
            $iscsitgtinfo[$i]{DEFAULTTIME2WAIT},
            $iscsitgtinfo[$i]{DEFAULTTIME2RETAIN},
            $iscsitgtinfo[$i]{MAXOUTSTR2T},
            $iscsitgtinfo[$i]{MAXRECVDATASEGMENTLEN},
            $iscsitgtinfo[$i]{IFMARKINT},
            $iscsitgtinfo[$i]{OFMARKINT},
            $iscsitgtinfo[$i]{HEADERDIGEST},
            $iscsitgtinfo[$i]{DATADIGEST},
            $iscsitgtinfo[$i]{AUTHMETHOD},
            $iscsitgtinfo[$i]{MTUSIZE},
            $iscsitgtinfo[$i]{TGTALIAS},
            $iscsitgtinfo[$i]{RSV1},
            $iscsitgtinfo[$i]{RSV2},
            $iscsitgtinfo[$i]{RSV3},
            $iscsitgtinfo[$i]{RSV4},
            $iscsitgtinfo[$i]{CONFIGMAP},
            ) = unpack("SNNNSCCCCCCCSLLSSSLSSCCCLa32LLLSL", substr($parts{DATA}, $start));
#            ) = unpack("SLLLSCCCCCCCSLLSSSLSSCCCLa32LLLSL", substr($parts{DATA}, $start));
            $start += 104;
        }
        $info{COUNT} = $count;
        $info{ISCSITGTINFO} = [@iscsitgtinfo];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a iSCSI target info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _iSCSIChapInfoPacket 
#
# Desc:     Parses the iSCSI target CHAP info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _iSCSIChapInfoPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $rsvd;

    if (commandCode($recvPacket) == PI_ISCSI_CHAP_INFO)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @iscsichapinfo;
        my $start = 8;

            # Unpack the data
            (
            $count,
            $rsvd
            ) = unpack("SS", substr($parts{DATA}, $start));
        
        $info{COUNT} = $count;

         my $i = 0;
         $start += 4;
         for ($i = 0; $i < $count; $i++)
         {
            (
            $iscsichapinfo[$i]{TID},
            $rsvd,
            $iscsichapinfo[$i]{SNAME}
            ) = unpack("SSa256", substr($parts{DATA}, $start));
            $start += 260;
             
         }

        $info{ISCSICHAPINFO} = [@iscsichapinfo];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a CHAP info packet\n");
    }

    return %info;
}


##############################################################################
# Name:     _iSCSIStatsPacket
#
# Desc:     Parses the iSCSI stats packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _iSCSIStatsPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $numconns = 0;
    my $conns = 0;
    my @numconnindx;
    my $rsvd;

    if (commandCode($recvPacket) == PI_ISCSI_SESSION_INFO 
       || commandCode($recvPacket) == PI_ISCSI_SESSION_INFO_SERVER)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @iscsistats;
        my $start = 8;
            # Unpack the data
            (
            $count,
            $rsvd
            ) = unpack("SS", substr($parts{DATA}, $start));
        
        $info{COUNT} = $count;

         my $i = 0;
         $start += 4;
         for ($i = 0; $i < $count; $i++)
         {
            (
            $iscsistats[$i]{TID},
            $rsvd,
#            $iscsistats[$i]{SID_HI},
#            $iscsistats[$i]{SID_LO},
            $iscsistats[$i]{ISID_0},
            $iscsistats[$i]{ISID_1},
            $iscsistats[$i]{ISID_2},
            $iscsistats[$i]{ISID_3},
            $iscsistats[$i]{ISID_4},
            $iscsistats[$i]{ISID_5},
            $iscsistats[$i]{TSIH},
            $iscsistats[$i]{SSTATE},
            $iscsistats[$i]{MAXCONN},
            $iscsistats[$i]{TGTNAME},
            $iscsistats[$i]{INITIATORNAME},
            $iscsistats[$i]{TGTALIAS},
            $iscsistats[$i]{INITIATORALIAS},
            $iscsistats[$i]{TGTADDRESS},
            $iscsistats[$i]{TPGT},
            $iscsistats[$i]{R2T},
            $iscsistats[$i]{IMMEDIATEDATA},
            $iscsistats[$i]{MAXBURSTLEN},
            $iscsistats[$i]{FIRSTBURSTLEN},
            $iscsistats[$i]{DEFAULTTIME2WAIT},
            $iscsistats[$i]{DEFAULTTIME2RETAIN},
            $iscsistats[$i]{MAXOUTSTANDINGR2T},
            $iscsistats[$i]{DATAPDUINORDER},
            $iscsistats[$i]{DATASEQINORDER},
            $iscsistats[$i]{ERRORRECOVERYLEVEL},
            $iscsistats[$i]{SESSIONTYPE},
            $iscsistats[$i]{PARAMMAP},
            $iscsistats[$i]{PARAMSENTMAP},
#            $iscsistats[$i]{NUMCONN}
            $conns,
            ) = unpack("SSCCCCCCSLSa256a256a256a256a256SCCLLSSSCCCCLLLL", substr($parts{DATA}, $start));

            $iscsistats[$i]{NUMCONN} = $conns;
            $numconnindx[$i] = $conns;
            $numconns += $conns;
            $start += 1332;
#            $start += 1328;

            my $j = 0;
#            for ($j = 0; $j < $iscsistats[$i]{NUMCONN}; $j++)

            for ($j = 0; $j < $iscsistats[$i]{NUMCONN}; $j++)
            {
               (
                $iscsistats[$i]{CONNINFO}[$j]{CID},
                $iscsistats[$i]{CONNINFO}[$j]{CSTATE},
                $iscsistats[$i]{CONNINFO}[$j]{NUMPDURCVD},
                $iscsistats[$i]{CONNINFO}[$j]{NUMPDUSNT},
                $iscsistats[$i]{CONNINFO}[$j]{NUMREADS_LO},
                $iscsistats[$i]{CONNINFO}[$j]{NUMREADS_HI},
                $iscsistats[$i]{CONNINFO}[$j]{NUMWRITES_LO},
                $iscsistats[$i]{CONNINFO}[$j]{NUMWRITES_HI},
                $iscsistats[$i]{CONNINFO}[$j]{HEADERDIGEST},
                $iscsistats[$i]{CONNINFO}[$j]{DATADIGEST},
                $iscsistats[$i]{CONNINFO}[$j]{AUTHMETHOD},
                $iscsistats[$i]{CONNINFO}[$j]{CHAPALGO},
                $iscsistats[$i]{CONNINFO}[$j]{IFMARKER},
                $iscsistats[$i]{CONNINFO}[$j]{OFMARKER},
                $iscsistats[$i]{CONNINFO}[$j]{MAXRECVDATASEGMENTLEN},
                $iscsistats[$i]{CONNINFO}[$j]{MAXSENDDATASEGMENTLEN},
                $iscsistats[$i]{CONNINFO}[$j]{CPARAMMAP},
                $iscsistats[$i]{CONNINFO}[$j]{CPARAMSENTMAP},
                $iscsistats[$i]{CONNINFO}[$j]{CRSVD}
               ) = unpack("LLLLLLLLa256a256a256CCCLLLLC", substr($parts{DATA}, $start));
               $start += 820;
            }
            for (; $j < 2; $j++)
            {
               $start += 820;
            }
       }
        $info{ISCSISESSIONINFO} = [@iscsistats];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a Sessions packet\n");
    }

    return %info;
}


##############################################################################
# Name: _displaydlmPathSelectionAlgorithm
#
# Desc: Print the DLM Path Selection response
#
# In:   Physical Disk Information Hash
##############################################################################
sub _displaydlmPathSelectionAlgorithm
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    my %info;
    my $msg = "";
    my $algoType;
    my $start = 8;

    logMsg("begin\n");

    my %parts = disassembleXiotechPacket($recvPacket);

    $info{STATUS} = $parts{STATUS};
    $info{ERROR_CODE} = $parts{ERROR_CODE};

    # Unpack the data
    (
        $algoType
    ) = unpack("C", substr($parts{DATA}, $start));

    $info{ALGOTYPE} = $algoType;

    if(!defined($algoType))
    {
        printf "Cannot read data\n";
        return %info;
    }

#    printf "Selected %u\n",$algoType;

    if ($algoType == 0)
    {
        printf "Selected Round Robin Algorithm\n";
    }
    elsif ($algoType == 1)
    {
        printf "Selected ICL 50%% Weighted RR Algorithm\n";
    }
    elsif ($algoType == 2)
    {
        printf "Selected ICL 100%% Weighted Algorithm\n";
    }

    return %info;
}

##############################################################################
# Name:     DLMPathStatsPacket
#
# Desc:     Parses the DLM Path Stats Info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _DLMPathStatsPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $rsvd;

    if (commandCode($recvPacket) == PI_DLM_PATH_STATS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @dlmstatsinfo;
        my $start = 8;

            # Unpack the data
            (
            $dlmstatsinfo[0]{STATS1},
            $dlmstatsinfo[0]{STATS2},
            $dlmstatsinfo[0]{STATS3},
            $dlmstatsinfo[0]{STATS4},
            $dlmstatsinfo[0]{STATS5},
            $dlmstatsinfo[1]{STATS6},
            $dlmstatsinfo[1]{STATS7},
            $dlmstatsinfo[1]{STATS8},
            $dlmstatsinfo[1]{STATS9},
            $dlmstatsinfo[1]{STATS10},
            $dlmstatsinfo[2]{STATS11},
            $dlmstatsinfo[2]{STATS12},
            $dlmstatsinfo[2]{STATS13},
            $dlmstatsinfo[2]{STATS14},
            $dlmstatsinfo[2]{STATS15},
            $dlmstatsinfo[3]{STATS16},
            $dlmstatsinfo[3]{STATS17},
            $dlmstatsinfo[3]{STATS18},
            $dlmstatsinfo[3]{STATS19},
            $dlmstatsinfo[3]{STATS20},
            $dlmstatsinfo[4]{STATS21},
            $dlmstatsinfo[4]{STATS22},
            $dlmstatsinfo[4]{STATS23},
            $dlmstatsinfo[4]{STATS24},
            $dlmstatsinfo[4]{STATS25},
            ) = unpack("LLLLLLLLLLLLLLLLLLLLLLLLL", substr($parts{DATA}, $start));

#        $info{DLMSTATSINFO} = [@dlmstatsinfo];

        print "\n";
        print "\nDLM Traffic Statistics >>>";
        print "\n\n";
        print "       This Controller          TargetController         Count\n";
        print "       ---------------          ----------------         --------\n";

#        print "$dlmstatsinfo[0]{STATS1}\n";
#        print "$dlmstatsinfo[0]{STATS2}\n";
#        print "$dlmstatsinfo[0]{STATS3}\n";
#        print "$dlmstatsinfo[0]{STATS4}\n";
#        print "$dlmstatsinfo[0]{STATS5}\n";
#        print "$dlmstatsinfo[1]{STATS6}\n";
#        print "$dlmstatsinfo[1]{STATS7}\n";
#        print "$dlmstatsinfo[1]{STATS8}\n";
#        print "$dlmstatsinfo[1]{STATS9}\n";
#        print "$dlmstatsinfo[1]{STATS10}\n";
#        print "$dlmstatsinfo[2]{STATS11}\n";
#        print "$dlmstatsinfo[2]{STATS12}\n";
#        print "$dlmstatsinfo[2]{STATS13}\n";
#        print "$dlmstatsinfo[2]{STATS14}\n";
#        print "$dlmstatsinfo[2]{STATS15}\n";
#        print "$dlmstatsinfo[3]{STATS16}\n";
#        print "$dlmstatsinfo[3]{STATS17}\n";
#        print "$dlmstatsinfo[3]{STATS18}\n";
#        print "$dlmstatsinfo[3]{STATS19}\n";
#        print "$dlmstatsinfo[3]{STATS20}\n";
#        print "$dlmstatsinfo[4]{STATS21}\n";
#        print "$dlmstatsinfo[4]{STATS22}\n";
#        print "$dlmstatsinfo[4]{STATS23}\n";
#        print "$dlmstatsinfo[4]{STATS24}\n";
#        print "$dlmstatsinfo[4]{STATS25}\n";


        print "        port -0                  port -0     ";
        print "            $dlmstatsinfo[0]{STATS1}\n";
        print "        port -0                  port -1     ";
        print "            $dlmstatsinfo[0]{STATS2}\n";
        print "        port -1                  port -0     ";
        print "            $dlmstatsinfo[1]{STATS6}\n";
        print "        port -1                  port -1     ";
        print "            $dlmstatsinfo[1]{STATS7}\n";
        print "        port -2                  port -2     ";
        print "            $dlmstatsinfo[2]{STATS13}\n";
        print "        port -2                  port -3     ";
        print "            $dlmstatsinfo[2]{STATS14}\n";
        print "        port -3                  port -2     ";
        print "            $dlmstatsinfo[3]{STATS18}\n";
        print "        port -3                  port -3     ";
        print "            $dlmstatsinfo[3]{STATS19}\n";
        print "        icl port                 icl port";
        print "                $dlmstatsinfo[4]{STATS25}\n";
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a DLM Stats packet\n");
    }
}


##############################################################################
# Name:     _IDDInfoPacket 
#
# Desc:     Parses the IDD Info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _IDDInfoPacket 
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $count;
    my $rsvd;

    if (commandCode($recvPacket) == PI_IDD_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        my @iddinfo;
        my $start = 8;

            # Unpack the data
            (
            $count,
            ) = unpack("L", substr($parts{DATA}, $start));
        
        $info{COUNT} = $count;

         my $i = 0;
         $start += 4;
         for ($i = 0; $i < $count; $i++)
         {
            (
            $iddinfo[$i]{FLAGS},
            $rsvd,
            $iddinfo[$i]{LID},
            $iddinfo[$i]{PTG},
            $iddinfo[$i]{PORT},
            $iddinfo[$i]{SG_FD},
            $iddinfo[$i]{SG_NAME},
            $iddinfo[$i]{INAME_LO},
            $iddinfo[$i]{INAME_HI},
            $iddinfo[$i]{TNAME_LO},
            $iddinfo[$i]{TNAME_HI},
            $iddinfo[$i]{PNAME_LO},
            $iddinfo[$i]{PNAME_HI},
            $iddinfo[$i]{TIP},
            $iddinfo[$i]{IIP},
            ) = unpack("SSCCS La16 NN NN NN NN", substr($parts{DATA}, $start));
            $start += 72;
         }

        $info{IDDINFO} = [@iddinfo];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a IDD info packet\n");
    }

    return %info;
}


##############################################################################
# Name: displayiSCSITgtInfo
#
# Desc: Print iSCSI target info
#
# In:   Target Information Hash
##############################################################################
sub displayiSCSITgtInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");
    my $i;
    my $msg = "";
    $msg .= sprintf  "iSCSI Target Info (%u targets):\n", $info{COUNT};
    for ($i = 0; $i < $info{COUNT}; $i++)
    {

    $info{ISCSITGTINFO}[$i]{TGTIP} = long2ip(0, $info{ISCSITGTINFO}[$i]{TGTIP});
    $info{ISCSITGTINFO}[$i]{SUBNETMASK} = long2ip(0, $info{ISCSITGTINFO}[$i]{SUBNETMASK});
    $info{ISCSITGTINFO}[$i]{GATEWAY} = long2ip(0, $info{ISCSITGTINFO}[$i]{GATEWAY});

#    $msg .= sprintf  "iSCSI Target Info (tid: %u):\n",$info{ISCSITGTINFO}[$i]{TID};
    $msg .= sprintf "\tTID                      : %u\n", $info{ISCSITGTINFO}[$i]{TID};
    $msg .= sprintf "\tIP Address               : %s\n", $info{ISCSITGTINFO}[$i]{TGTIP};
    $msg .= sprintf "\tSubnet Mask              : %s\n", $info{ISCSITGTINFO}[$i]{SUBNETMASK};
    $msg .= sprintf "\tDefault Gateway          : %s\n", $info{ISCSITGTINFO}[$i]{GATEWAY};
    $msg .= sprintf "\tMAX Connections          : %u\n", $info{ISCSITGTINFO}[$i]{MAXCONNECTIONS};
    $msg .= sprintf "\tInitial R2T              : %u\n", $info{ISCSITGTINFO}[$i]{R2T};
    $msg .= sprintf "\tImmediate Data           : %u\n", $info{ISCSITGTINFO}[$i]{IMMEDDATA};
    $msg .= sprintf "\tData SEQ Inorder         : %u\n", $info{ISCSITGTINFO}[$i]{DATASEQINORDER};
    $msg .= sprintf "\tData PDU Inorder         : %u\n", $info{ISCSITGTINFO}[$i]{DATAPDUINORDER};
    $msg .= sprintf "\tIF Marker                : %u\n", $info{ISCSITGTINFO}[$i]{IFMARKER};
    $msg .= sprintf "\tOF Marker                : %u\n", $info{ISCSITGTINFO}[$i]{OFMARKER};
    $msg .= sprintf "\tError Recovery Level     : %u\n", $info{ISCSITGTINFO}[$i]{ERRORRECOVERY};
    $msg .= sprintf "\tPortal Group TAG         : %u\n", $info{ISCSITGTINFO}[$i]{TPGT};
    $msg .= sprintf "\tMAX Burst Size           : %u\n", $info{ISCSITGTINFO}[$i]{MAXBURSTLEN};
    $msg .= sprintf "\tFIRST Burst Size         : %u\n", $info{ISCSITGTINFO}[$i]{FIRSTBURSTLEN};
#    $msg .= sprintf "\tDEFAULT TIME TO WAIT     : %u\n", $info{ISCSITGTINFO}[$i]{DEFAULTTIME2WAIT};
#    $msg .= sprintf "\tDEFAULT TIME TO RETAIN   : %u\n", $info{ISCSITGTINFO}[$i]{DEFAULTTIME2RETAIN};
    $msg .= sprintf "\tMAX Outstanding R2Ts     : %u\n", $info{ISCSITGTINFO}[$i]{MAXOUTSTR2T};
    $msg .= sprintf "\tMAX Rx Data Segment      : %u\n", $info{ISCSITGTINFO}[$i]{MAXRECVDATASEGMENTLEN};
#    $msg .= sprintf "\tIF Mark INT              : %u\n", $info{ISCSITGTINFO}[$i]{IFMARKINT};
#    $msg .= sprintf "\tOF Mark INT              : %u\n", $info{ISCSITGTINFO}[$i]{OFMARKINT};
    $msg .= sprintf "\tHeader Digest            : %u\n", $info{ISCSITGTINFO}[$i]{HEADERDIGEST};
    $msg .= sprintf "\tData Digest              : %u\n", $info{ISCSITGTINFO}[$i]{DATADIGEST};
    $msg .= sprintf "\tAuthentication Method    : %u\n", $info{ISCSITGTINFO}[$i]{AUTHMETHOD};
    $msg .= sprintf "\tMTU Size                 : %u\n", $info{ISCSITGTINFO}[$i]{MTUSIZE};
    $msg .= sprintf "\tTarget Alias             : %s\n", $info{ISCSITGTINFO}[$i]{TGTALIAS};
    $msg .= sprintf " Config Enable Map         : 0x%x\n", $info{ISCSITGTINFO}[$i]{CONFIGMAP};
    $msg .= sprintf "\n\n";
    }

    return $msg;
}

##############################################################################
# Name: displayiSNSInfo
#
# Desc: Print iSNS info
#
# In:   Isns Information Hash
##############################################################################
sub displayiSNSInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");
    my $i;
    my $msg = "";
    $msg .= sprintf  "\niSNS status";
    if (0 == $info{FLAGS})
    {
        $msg .= sprintf " : DISABLED\n\n";
    }
    elsif (1 == $info{FLAGS})
    {
        $msg .= sprintf " : ENABLED\n\n";
    }
    elsif (2 == $info{FLAGS})
    {
        $msg .= sprintf " : AUTO | DISABLED\n\n";
    }
    elsif (3 == $info{FLAGS})
    {
        $msg .= sprintf " : AUTO | ENABLED\n\n";
    }
    $msg .= sprintf("(#) : iSNS Server IP   Port Proto\n");
    $msg .= sprintf("---------------------------------\n");
    for ($i = 0; $i < $info{COUNT}; $i++)
    {
        $info{ISNSINFO}[$i]{IP} = long2ip(0, $info{ISNSINFO}[$i]{IP});

        $msg .= sprintf "(%u) : ", $i;
        $msg .= sprintf "%15s", $info{ISNSINFO}[$i]{IP};
        $msg .= sprintf ":%5s", $info{ISNSINFO}[$i]{PORT};
        if (0 == $info{ISNSINFO}[$i]{PROTO})
        {
            $msg .= sprintf "  TCP\n";
        }
        else
        {
            $msg .= sprintf "  UDP\n";
        }
    }
    $msg .= sprintf "\n";
    return $msg;
}

##############################################################################
# Name: displayiSCSIChapInfo
#
# Desc: Print iSCSI target info 
#
# In:   CHAP Information Hash
##############################################################################
sub displayiSCSIChapInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "CHAP Info (%u users):\n", $info{COUNT};
    $msg .= sprintf  "\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf "TID: %3u    SNAME: %s\n",$info{ISCSICHAPINFO}[$i]{TID}, 
                                            $info{ISCSICHAPINFO}[$i]{SNAME};
    }

    return $msg;
}

##############################################################################
# Name: displayiSCSIStats
#
# Desc: Print iSCSI session info 
#
# In:   Session Information Hash
##############################################################################
sub displayiSCSIStats
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

#    $msg .= sprintf  "Total Sessions:  %u\n", $info{COUNT};
#    $msg .= sprintf  "\n";

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $msg .= sprintf " Session #%u (of %u):\n", $i+1, $info{COUNT};
        $msg .= sprintf "   Target ID             : %u\n", $info{ISCSISESSIONINFO}[$i]{TID};
        $msg .= sprintf "   Initiator Name        : %48.48s\n", $info{ISCSISESSIONINFO}[$i]{INITIATORNAME};
        $msg .= sprintf "   Initiator Alias       : %32.32s\n", $info{ISCSISESSIONINFO}[$i]{INITIATORALIAS};
        $msg .= sprintf "   Target Name           : %32.32s\n", $info{ISCSISESSIONINFO}[$i]{TGTNAME};
        $msg .= sprintf "   Target Alias          : %32.32s\n", $info{ISCSISESSIONINFO}[$i]{TGTALIAS};
#        $msg .= sprintf "   Target Session ID     : %u\n", $info{ISCSISESSIONINFO}[$i]{TSIH};
#        $msg .= sprintf " TARGET ADDRESS        : %256s\n", $info{ISCSISESSIONINFO}[$i]{TGTADDRESS};
        $msg .= sprintf "   Session ID            : %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n", $info{ISCSISESSIONINFO}[$i]{ISID_0}, 
                                                      $info{ISCSISESSIONINFO}[$i]{ISID_1},
                                                      $info{ISCSISESSIONINFO}[$i]{ISID_2},
                                                      $info{ISCSISESSIONINFO}[$i]{ISID_3},
                                                      $info{ISCSISESSIONINFO}[$i]{ISID_4},
                                                      $info{ISCSISESSIONINFO}[$i]{ISID_5};
        $msg .= sprintf "   Target Session ID     : %u\n", $info{ISCSISESSIONINFO}[$i]{TSIH};
        $msg .= sprintf "   Session Type          : %u\n", $info{ISCSISESSIONINFO}[$i]{SESSIONTYPE};
        $msg .= sprintf "   Session State         : %u\n", $info{ISCSISESSIONINFO}[$i]{SSTATE};
        $msg .= sprintf "   MAX Connections       : %u\n", $info{ISCSISESSIONINFO}[$i]{MAXCONN};
        $msg .= sprintf "   Portal Group TAG      : %u\n", $info{ISCSISESSIONINFO}[$i]{TPGT};
        $msg .= sprintf "   Immediate DATA        : %u\n", $info{ISCSISESSIONINFO}[$i]{IMMEDIATEDATA};
        $msg .= sprintf "   Initial R2T           : %u\n", $info{ISCSISESSIONINFO}[$i]{R2T};
        $msg .= sprintf "   MAX Outstanding R2Ts  : %u\n", $info{ISCSISESSIONINFO}[$i]{MAXOUTSTANDINGR2T};
        $msg .= sprintf "   MAX Burst Length      : %u\n", $info{ISCSISESSIONINFO}[$i]{MAXBURSTLEN};
        $msg .= sprintf "   First Burst Length    : %u\n", $info{ISCSISESSIONINFO}[$i]{FIRSTBURSTLEN};
#        $msg .= sprintf "   Default Time 2 Wait   : %u\n", $info{ISCSISESSIONINFO}[$i]{DEFAULTTIME2WAIT};
#        $msg .= sprintf "   Default Time 2 Retain : %u\n", $info{ISCSISESSIONINFO}[$i]{DEFAULTTIME2RETAIN};
        $msg .= sprintf "   Data PDU Inorder      : %u\n", $info{ISCSISESSIONINFO}[$i]{DATAPDUINORDER};
        $msg .= sprintf "   Data SEQ Inorder      : %u\n", $info{ISCSISESSIONINFO}[$i]{DATASEQINORDER};
        $msg .= sprintf "   Error Recovery Level  : %u\n", $info{ISCSISESSIONINFO}[$i]{ERRORRECOVERYLEVEL};
#        $msg .= sprintf " PARAM NEGOTIATE MAP   : %u\n", $info{ISCSISESSIONINFO}[$i]{PARAMMAP};
#        $msg .= sprintf " PARAM SENT MAP        : %u\n", $info{ISCSISESSIONINFO}[$i]{PARAMSENTMAP};
#        $msg .= sprintf " NUM CONNECTIONS       : %u\n", $info{ISCSISESSIONINFO}[$i]{NUMCONN};
#        $msg .= sprintf " CONNECTION INFO       :\n";
        
    $msg .= sprintf  "\n";
#    $msg .= sprintf  "\n";

        for (my $j=0; $j < $info{ISCSISESSIONINFO}[$i]{NUMCONN}; $j++)
        {
            $msg .= sprintf "\tConnection #%u (of %u):\n", $j+1, $info{ISCSISESSIONINFO}[$i]{NUMCONN};
            $msg .= sprintf "\t  CID                   : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{CID};
            $msg .= sprintf "\t  State                 : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{CSTATE};
            $msg .= sprintf "\t  Header Digest         : %10.10s\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{HEADERDIGEST};
            $msg .= sprintf "\t  Data Digest           : %10.10s\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{DATADIGEST};
            $msg .= sprintf "\t  Authentication        : %10.10s\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{AUTHMETHOD};
            $msg .= sprintf "\t  CHAP Type             : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{CHAPALGO};
            $msg .= sprintf "\t  MAX Tx Segment        : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{MAXSENDDATASEGMENTLEN};
            $msg .= sprintf "\t  MAX Rx Segment        : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{MAXRECVDATASEGMENTLEN};
            $msg .= sprintf "\t      Rx PDU Count      : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMPDURCVD};
            $msg .= sprintf "\t      Tx PDU Count      : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMPDUSNT};
            $msg .= sprintf "\t      Reads             : %x%x\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMREADS_LO}, 
                                                      $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMREADS_HI};
            $msg .= sprintf "\t      Writes            : %x%x\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMWRITES_LO},
                                                      $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{NUMWRITES_HI};
#            $msg .= sprintf "\t  IF MARKER             : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{IFMARKER};
#            $msg .= sprintf "\t  OF MARKER             : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{OFMARKER};
#            $msg .= sprintf "\t  PARAM NEGOTIATE MAP   : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{CPARAMMAP};
#            $msg .= sprintf "\t  PARAM SENT MAP        : %u\n", $info{ISCSISESSIONINFO}[$i]{CONNINFO}[$j]{CPARAMSENTMAP};
    $msg .= sprintf  "\n";
#    $msg .= sprintf  "\n";
        }
    }

    return $msg;
}

##############################################################################
# Name: displayIDDInfo
#
# Desc: Print IDD info 
#
# In:   IDD Information Hash
##############################################################################
sub displayIDDInfo
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "IDD Info (%u IDDs):\n", $info{COUNT};

    $msg .= sprintf  "\n";

    $msg .= sprintf " FLAGS  LID  PTG  PORT  SG_FD  SG_NAME    INAME              TNAME              PNAME             TGT_IP        SRV_IP \n";
    my $s1;
    my $s2;

    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        $s1 = long2ip(0, $info{IDDINFO}[$i]{TIP});
        $s2 = long2ip(0, $info{IDDINFO}[$i]{IIP});
        $msg .= sprintf "%3u   %3u  %3u   %3d     %3d   %s   %8.8x%8.8x   %8.8x%8.8x   %8.8x%8.8x  %s  %s\n", $info{IDDINFO}[$i]{FLAGS},
            $info{IDDINFO}[$i]{LID},
            $info{IDDINFO}[$i]{PTG},
            $info{IDDINFO}[$i]{PORT},
            $info{IDDINFO}[$i]{SG_FD},
            $info{IDDINFO}[$i]{SG_NAME},
            $info{IDDINFO}[$i]{INAME_LO},
            $info{IDDINFO}[$i]{INAME_HI},
            $info{IDDINFO}[$i]{TNAME_LO},
            $info{IDDINFO}[$i]{TNAME_HI},
            $info{IDDINFO}[$i]{PNAME_LO},
            $info{IDDINFO}[$i]{PNAME_HI},
            $s1,
            $s2;
    }

    return $msg;
}


##############################################################################
# Name: displayDLMPathStats
#
# Desc: Print DLM path stats info 
#
# In:   DLM path stats Information Hash
##############################################################################
sub displayDLMPathStats
{
    my ($self, %info) = @_;

    logMsg("begin\n");

    my $msg = "";

    $msg .= sprintf  "DLM Path Stats Info:\n";

    $msg .= sprintf  "\n";
    $msg .= sprintf  "\n";

#    $msg .= sprintf " \t ThisController     targetController     count \n";
#   $msg .= sprintf " \t -----------------------------------------------\n";
    $msg .= sprintf" Count =========        %lu\n",$info{DLMSTATSINFO}[0]{STATS1};






#    $msg .= sprintf " \    iscsi-0 port        iscsi-0 port      %lu\n",$info{DLMSTATSINFO}[0]{STATS1};
#    $msg .= sprintf " \    iscsi-0 port        iscsi-1 port      %lu\n",$info{DLMSTATSINFO}[0]{STATS2};
#    $msg .= sprintf " \    iscsi-1 port        iscsi-0 port      %lu\n",$info{DLMSTATSINFO}[0]{STATS3};
#    $msg .= sprintf " \    iscsi-1 port        iscsi-1 port      %lu\n",$info{DLMSTATSINFO}[0]{STATS4};
#    $msg .= sprintf " \    icl-port            icl-port          %lu\n",$info{DLMSTATSINFO}[0]{STATS5}; 


#            $info{DLMSTATSINFO}[0]{STATS1},
#    $msg .= sprintf  "\n";
#    $msg .= sprintf " \ iscsi-0 port        iscsi-1 port";
#            $info{DLMSTATSINFO}[0]{STATS2},
#    $msg .= sprintf  "\n";
#    $msg .= sprintf " \ iscsi-1 port        iscsi-0 port";
#            $info{DLMSTATSINFO}[0]{STATS3},
#    $msg .= sprintf  "\n";
#    $msg .= sprintf " \ iscsi-1 port        iscsi-1 port";
#            $info{DLMSTATSINFO}[0]{STATS4},
#    $msg .= sprintf  "\n";
#    $msg .= sprintf " \ icl-port            icl-port";
#            $info{DLMSTATSINFO}[0]{STATS5},
    $msg .= sprintf  "\n";

    return $msg;
}

##############################################################################

1;

##############################################################################
#######
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
