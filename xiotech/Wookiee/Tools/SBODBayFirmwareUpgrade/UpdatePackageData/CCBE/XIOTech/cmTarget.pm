# $Header$
##############################################################################
#   Xiotech a Seagate Technology
#   Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Chris Nigbur
#
# Purpose:
#   Wrapper for all the different XIOTech virtual disk commands that
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
# Name:     targets
#
# Desc:     Retrieves target information for all targets.
#
# In:       NONE
#
# Returns:
##############################################################################
sub targets
{
    my ($self) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["targets"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_TARGETS_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        undef,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_targetsPacket);
}

##############################################################################
# Name:     targetCount
#
# Desc:     Retrieves the number of targets.
#
# Input:    None
#
# Returns:  Number of targets or UNDEF if an error occurred.
##############################################################################
sub targetCount
{
    my ($self) = @_;
    return $self->getObjectCount(PI_TARGET_COUNT_CMD);
}

##############################################################################
# Name:     targetInfo
#
# Desc:     Get information about a target
#
# In:       ID of a target
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub targetInfo
{
    my ($self, $tid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ["targetInfo"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_TARGET_INFO_CMD;
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
		                                \&_targetInfoPacket);
}

##############################################################################
# Name:     targetStatus
#
# Desc:     displays status information about a target
#
# In:       ID of a target
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub targetStatus
{
    my ($self, $tid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ["targetStatus"]];

    
    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $rc = 1;
    my $i;
    my $j;
    my $k;
    my %targets;
    my %servers;

    my %info;

    $info{STATUS} = PI_GOOD;

    %targets = $self->targets();
    if (%targets && $targets{STATUS} == PI_GOOD)
    {
        # Got the targets...
    }
    else
    {
        $rc = 0;
        $info{STATUS} = $targets{STATUS};
        $info{ERROR_CODE} = $targets{ERROR_CODE};
    }

    if ($rc)
    {
        %servers = $self->servers();

        if (%servers && $servers{STATUS} == PI_GOOD)
        {
            # Got the servers...
        }
        else
        {
            $rc = 0;
            $info{STATUS} = $servers{STATUS};
            $info{ERROR_CODE} = $servers{ERROR_CODE};
        }
    }
    
    if ($rc)                            
    {
        print " TID  Port   Owner       Port WWN      SID     Server WWN     LUN  VID  INITIATOR NAME\n";
        print " ---  ----  -------  ----------------  ---  ----------------  ---  ---  --------------\n";
        for ($i = 0; $i < $targets{COUNT}; ++$i)
        {
            printf " %3hu  %4lu  %7lu  %8.8x%8.8x", $targets{TARGETS}[$i]{TGD_TID}, 
                                                    $targets{TARGETS}[$i]{TGD_CHAN}, 
                                                    $targets{TARGETS}[$i]{TGD_OWNER},
                                                    $targets{TARGETS}[$i]{TGD_PNAME_LO},
                                                    $targets{TARGETS}[$i]{TGD_PNAME_HI};

            my $linecount = 0;
                   
            for ($j = 0; $j < $servers{COUNT}; ++$j)
            {
                if ($targets{TARGETS}[$i]{TGD_TID} == $servers{SERVERS}[$j]{TARGETID})
                {
                    if ($linecount++ > 0)
                    {
                        print "\n                                     ";
                    }
                    printf "  %3hu  %8.8x%8.8x", $servers{SERVERS}[$j]{SID}, 
                                                    $servers{SERVERS}[$j]{WWN_LO}, 
                                                    $servers{SERVERS}[$j]{WWN_HI};  
                    
                    if ($servers{SERVERS}[$j]{NLUNS} > 0)
                    {
                        for ($k = 0; $k < $servers{SERVERS}[$j]{NLUNS}; ++$k)
                        {
                            if ($k > 0)
                            {
                                print "\n                                                            ";
                            }
                            printf "  %3hu  %3hu", $servers{SERVERS}[$j]{LUNMAP}[$k]{LUN},
                                                    $servers{SERVERS}[$j]{LUNMAP}[$k]{VID};
			    if ($k == 0)
			    {
				printf "  %s", $servers{SERVERS}[$j]{INAME};
			    }
                        }
                    }   
                }
            }
            print "\n\n";
        }
    }
    
    return %info;
}


##############################################################################
# Name:     targetList
#
# Desc:     Retrieves an array containing the identifiers of the target.
#
# Input:    None
#
# Returns:
##############################################################################
sub targetList
{
    my ($self) = @_;
    return $self->getObjectList(PI_TARGET_LIST_CMD);
}

##############################################################################
# Name:     targetMove
#
# Desc:     Move a target to another controller and/or channel
#
# Input:    TARGET_ID           - Target Identifier
#           DEST_CONTROLLER_SN  - Destination controller serial number
#           CHANNEL             - Optional channel on destination controller
#
# Returns:
##############################################################################
sub targetMove
{
    my ($self, $target_id, $dest_controller_sn, $channel) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFF],
                ["targetMove"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_TARGET_MOVE_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();
    my $data = pack("SCCL",
                    $target_id,
                    $channel,
                    0,
                    $dest_controller_sn);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_genericResponsePacket);
}

##############################################################################
# Name:     targetResList
#
# Desc:     Get the target resource list.
#
# Input:    TID         - Target identifier
#           LIST_TYPE   - Type of resource list (1 = SERVER, 2 = VIRTUAL DISK)
#           SID         - Starting ID
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub targetResList
{
    my ($self,
        $tid,
        $listType,
        $sid) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 0, 0xFF],
                ["targetResList"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my $cmd = PI_TARGET_RESOURCE_LIST_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SSCCCC",
                    $sid,
                    $tid,
                    $listType,
                    0,0,0);

    my $packet = assembleXiotechPacket($cmd,
                                        $seq,
                                        $ts,
                                        $data,
                                        $self->{PORT}, VERSION_1);

	return $self->_handleSyncResponse($seq,
		                                $packet,
		                                \&_targetResListPacket);
}

##############################################################################
# Name:     targetSetProperties
#
# Desc:     Set the properties of a target.
#
# Input:    ID          - Target identifier
#           CHANNEL     - Channel number
#           OPTION      - Option for hard or soft ID
#           LOOP_ID     - Loop ID
#           OWNER       - Serial number of the controller who owns this target
#           CLUSTER     - Cluster Target ID
#
# Returns:  An empty hash on error, else returns a hash with the
#           following information
#
#       STATUS                  Status of the command
##############################################################################
sub targetSetProperties
{
    my ($self,
        $tid,
        $channel,
        $option,
        $loop_id,
        $owner,
        $cluster,
        $locked) = @_;

    logMsg("begin\n");

    # verify parameters
    my $args = [['i'],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFF],
                ['d', 2, 3],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFFFFFF],
                ['d', 0, 0xFFFF],
                ['d', 0, 0xFFFF],
                ["targetSetProperties"]];

    my %vp = verifyParameters(\@_, $args);
    if (%vp)
    {
        return %vp;
    }

    my %rsp = $self->targetInfo($tid);

    if (!%rsp || $rsp{STATUS} != PI_GOOD)
    {
        my $msg = "Unable to retrieve target information ($tid).\n";
        $rsp{MESSAGE} = $msg;
        return %rsp;
    }

    my $cmd = PI_TARGET_SET_PROPERTIES_CMD;
    my $seq = $self->{SEQ}->nextId();
    my $ts = $self->{SEQ}->nextTimeStamp();

    my $data = pack("SCCCCCCNNNNLLSSLLL",
                    $tid,
                    $channel,
                    $option,
                    $loop_id,
                    0,
                    $locked,
                    0,
                    $rsp{TGD_PNAME_LO}, $rsp{TGD_PNAME_HI},
                    $rsp{TGD_NNAME_LO}, $rsp{TGD_NNAME_HI},
                    $rsp{TGD_POWNER},
                    $owner,
                    $cluster,
                    0,
                    0,0,0);

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
# Name: displayTargets
#
# Desc: Print the targets
#
# In:   Targets Information Hash
##############################################################################
sub displayTargets
{
    my ($self, %info) = @_;
    my $msg = "";

    my $ttype;


    logMsg("begin\n");

    $msg .= sprintf  "Targets ($info{COUNT} targets):\n";
    $msg .= sprintf  "\n";

    $msg .= sprintf " TID  TYPE   PORT  OPTION  FCID  LOCK      PORT WWN           NODE WWN         POWNER       OWNER     CLUSTER  PPORT  APORT\n";
    $msg .= sprintf " ---  -----  ----  ------  ----  ----  -----------------  -----------------  ----------  -----------  -------  -----  -----\n";


    for (my $i = 0; $i < $info{COUNT}; $i++)
    {
        if(($info{TARGETS}[$i]{TGD_OPT} & 0x80) > 0)
        {
            $ttype = "iSCSI";
        }
        else
        {
            $ttype = "FC";
        }

        $msg .= sprintf " %3hu %6s %4hu    0x%2.2x  %4hu  0x%2.2x   %8.8x%8.8x   %8.8x%8.8x  %10lu  %11lu  %7hu  %5hu  %5hu\n",
                $info{TARGETS}[$i]{TGD_TID},
                $ttype,
                $info{TARGETS}[$i]{TGD_CHAN},
                $info{TARGETS}[$i]{TGD_OPT},
                $info{TARGETS}[$i]{TGD_ID},
                $info{TARGETS}[$i]{LOCK},
                $info{TARGETS}[$i]{TGD_PNAME_LO}, $info{TARGETS}[$i]{TGD_PNAME_HI},
                $info{TARGETS}[$i]{TGD_NNAME_LO}, $info{TARGETS}[$i]{TGD_NNAME_HI},
                $info{TARGETS}[$i]{TGD_POWNER},
                $info{TARGETS}[$i]{TGD_OWNER},
                $info{TARGETS}[$i]{TGD_CLUSTER},
                $info{TARGETS}[$i]{TGD_PPORT},
                $info{TARGETS}[$i]{TGD_APORT};
    }

    $msg .= sprintf "\n";

    return $msg;
    
}

##############################################################################
# Name: displayTargetInfo
#
# Desc: Print the target information
#
# In:   Target Information Hash
##############################################################################
sub displayTargetInfo
{
    my ($self, %info) = @_;
    my $msg = "";

    my $ttype;

    if(($info{TGD_OPT} & 0x80) > 0)
    {
        $ttype = "iSCSI";
    }
    else
    {
        $ttype = "FC";
    }
    
    logMsg("begin\n");

    $msg .= sprintf "Target Information:\n";
    $msg .= sprintf "  STATUS:               0x%x\n", $info{STATUS_MRP};
    $msg .= sprintf "  LEN:                  %lu\n", $info{LEN};
    $msg .= sprintf "  TGD_TID:              %hu\n", $info{TGD_TID};
    $msg .= sprintf "  TGD_TYPE:             %s\n", $ttype;
    $msg .= sprintf "  TGD_CHAN:             %hu\n", $info{TGD_CHAN};
    $msg .= sprintf "  TGD_OPT:              0x%x\n", $info{TGD_OPT};
    $msg .= sprintf "  TGD_ID:               %lu\n", $info{TGD_ID};
    $msg .= sprintf "  LOCK:                 %hu\n", $info{LOCK};
    $msg .= sprintf "  TGD_PNAME:            %8.8x%8.8x\n", $info{TGD_PNAME_LO}, $info{TGD_PNAME_HI};
    $msg .= sprintf "  TGD_NNAME:            %8.8x%8.8x\n", $info{TGD_NNAME_LO}, $info{TGD_NNAME_HI};
    $msg .= sprintf "  TGD_POWNER:           %lu\n", $info{TGD_POWNER};
    $msg .= sprintf "  TGD_OWNER:            %lu\n", $info{TGD_OWNER};
    $msg .= sprintf "  TGD_CLUSTER:          %hu\n", $info{TGD_CLUSTER};
    $msg .= sprintf "  TGD_PPORT:            %hu\n", $info{TGD_PPORT};
    $msg .= sprintf "  TGD_APORT:            %hu\n", $info{TGD_APORT};
    
    $msg .= sprintf "\n";

    return $msg;

}

##############################################################################
#
#   All commands that start with a _ should not be used and are considered
#   private.
#
##############################################################################


##############################################################################
# Name:     _targetsPacket
#
# Desc:     Parses the targets packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
##############################################################################
sub _targetsPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;

    if (commandCode($recvPacket) == PI_TARGETS_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        (
        $info{COUNT}
        ) = unpack("S", $parts{DATA});

        my @targets;

        for (my $i = 0; $i < $info{COUNT}; $i++)
        {
            my $start = 4 + (56 * $i);

            my $rsvd;
            my %tgd_pname;
            my %tgd_nname;

            # Unpack the data
            (
            $rsvd,
            $targets[$i]{STATUS_MRP},
            $targets[$i]{LEN},
            $targets[$i]{TGD_TID},
            $targets[$i]{TGD_CHAN},
            $targets[$i]{TGD_OPT},
            $targets[$i]{TGD_ID},
            $rsvd,
            $targets[$i]{LOCK},
            $rsvd,

            $tgd_pname{LO_LONG}, $tgd_pname{HI_LONG},
            $tgd_nname{LO_LONG}, $tgd_nname{HI_LONG},

            $targets[$i]{TGD_POWNER},
            $targets[$i]{TGD_OWNER},
            $targets[$i]{TGD_CLUSTER},
            $rsvd,
            $targets[$i]{TGD_PPORT},
            $targets[$i]{TGD_APORT},
            $rsvd
            ) = unpack("a3CLSCCCa1Ca1 NNNN LLSa2CCa10", substr($parts{DATA}, $start));

            # Now fixup all the 64 bit  numbers
            $targets[$i]{TGD_PNAME} = longsToBigInt(%tgd_pname);
            $targets[$i]{TGD_PNAME_LO} = $tgd_pname{LO_LONG};
            $targets[$i]{TGD_PNAME_HI} = $tgd_pname{HI_LONG};
            $targets[$i]{TGD_NNAME} = longsToBigInt(%tgd_nname);
            $targets[$i]{TGD_NNAME_LO} = $tgd_nname{LO_LONG};
            $targets[$i]{TGD_NNAME_HI} = $tgd_nname{HI_LONG};
        }

        $info{TARGETS} = [@targets];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a targets packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _targetInfoPacket
#
# Desc:     Parses the target info packet and places the information
#           in a hash
#
# In:       scalar  $packet to be parsed (If no packet then will attempt to read
#           one from the SAN box
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _targetInfoPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;

    if (commandCode($recvPacket) == PI_TARGET_INFO_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;
        my %tgd_pname;
        my %tgd_nname;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{TGD_TID},
        $info{TGD_CHAN},
        $info{TGD_OPT},
        $info{TGD_ID},
        $rsvd,
        $info{LOCK},
        $rsvd,

        $tgd_pname{LO_LONG}, $tgd_pname{HI_LONG},
        $tgd_nname{LO_LONG}, $tgd_nname{HI_LONG},

        $info{TGD_POWNER},
        $info{TGD_OWNER},
        $info{TGD_CLUSTER},
        $rsvd,
        $info{TGD_PPORT},
        $info{TGD_APORT},
        $rsvd
        ) = unpack("a3CLSCCCa1Ca1 NNNN LLSa2CCa10", $parts{DATA});

        # Now fixup all the 64 bit  numbers
        $info{TGD_PNAME} = longsToBigInt(%tgd_pname);
        $info{TGD_PNAME_LO} = $tgd_pname{LO_LONG};
        $info{TGD_PNAME_HI} = $tgd_pname{HI_LONG};
        $info{TGD_NNAME} = longsToBigInt(%tgd_nname);
        $info{TGD_NNAME_LO} = $tgd_nname{LO_LONG};
        $info{TGD_NNAME_HI} = $tgd_nname{HI_LONG};
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a target info packet\n");
    }

    return %info;
}

##############################################################################
# Name:     _targetResList
#
# Desc:     Parses the target resource list packet and places the information
#           in a hash.  This is sort of kludgy since the data can be returned
#           in one of 3 different formats based on SIZE.  
#
# In:       scalar  $packet to be parsed 
#
# Returns:
#       Empty hash if we have errors else a hash with the following:
#
##############################################################################
sub _targetResListPacket
{
    my ($self,
        $seq,
        $recvPacket) = @_;

    logMsg("begin\n");

    my %info;
    my $i;
    my $j;

    if (commandCode($recvPacket) == PI_TARGET_RESOURCE_LIST_CMD)
    {
        my %parts = disassembleXiotechPacket($recvPacket);

        my $rsvd;

        $info{STATUS} = $parts{STATUS};
        $info{ERROR_CODE} = $parts{ERROR_CODE};

        # Unpack the data
        (
        $rsvd,
        $info{STATUS_MRP},
        $info{LEN},
        $info{COUNT},   # count of entries
        $info{SIZE}     # size in bytes of each entry
        ) = unpack("a3CLSS", $parts{DATA});

        my @idlist;

        print "count (ndevs): " . $info{COUNT} . "\n";    # debug
        print "size (bytes per entry): " . $info{SIZE} . "\n\n";    # debug

        my $index = 0;  # Index into output array

        # $i counts through the "records" in the response data.
        for ($i = 0; $i < $info{COUNT}; $i++)
        {
            # A record may contain more than one value based on SIZE
            # SIZE=2 and SIZE=8 records are handled below
            if ($info{SIZE} < 16)
            {
                # $j counts through "fields" in each "record"
                for ($j = 0; $j < ($info{SIZE} / 2); $j++)
                {
                    # To get to the next field in a record, skip past $i
                    # records + $j fields, where each field is a UINT16 value.
                    my $start = 12 + ($info{SIZE} * $i) + (2 * $j);

                    $idlist[$index] = unpack("S", substr($parts{DATA}, $start, 2));

                    $index++;
                }
            }
            else
            {
                # The SIZE=16 record has a different format.  Put each 
                # record into a hash.
                my %wwn;

                my $start = 12 + ($info{SIZE} * $i);

                (
                $idlist[$i]{SID},
                $idlist[$i]{TID},
                $wwn{LO_LONG}, $wwn{HI_LONG},
                $rsvd
                ) = unpack("SSNNL", substr($parts{DATA}, $start));

                # Now fixup all the 64 bit numbers
                $idlist[$i]{WWN} = longsToBigInt(%wwn);
                $idlist[$i]{WWN_HI} = $wwn{HI_LONG};
                $idlist[$i]{WWN_LO} = $wwn{LO_LONG};
            }
        }

        $info{LIST} = [@idlist];
    }
    else
    {
        $self->_handleError($recvPacket);
        logMsg("Unexpected packet: We expected a target resource list packet\n");
    }

    return %info;
}

1;

##############################################################################
# Change log:
# $Log$
# Revision 1.5  2006/10/06 23:23:00  sferris
# Tbolt00000000
# add the initiator name to targetStatus
#
# Revision 1.4  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.2.2.2  2006/04/26 09:06:20  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.2.2.1  2006/02/24 14:17:23  MiddenM
#
# Merge from WOOKIEE_EGGS_GA_BR into MODEL750_BR
#
# Revision 1.3  2006/01/23 14:03:56  BoddukuriV
# TBolt00000000 - Removed CRLFs
#
# Revision 1.2  2005/12/23 08:13:48  BalemarthyS
# Merged ISCSI & GEORAID related changes
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.21  2004/06/08 22:04:39  KohlmeyerA
# Tbolt00000000:  Updated some display functions to store data in a string and return
# that string instead of just printing the data.  Reviewed by Craig.
#
# Revision 1.20  2003/02/07 15:05:51  SchibillaM
# TBolt00007143: Add support for new Target Resource List options.  These are
# used by Server Validation in CCB code.
#
# Revision 1.19  2003/01/08 21:47:17  TeskeJ
# tbolt00006680 - targetstatus display offset when multiple LUNs per server
# rev by Tim
#
# Revision 1.18  2003/01/07 19:03:12  TeskeJ
# tbolt00000000 - changed 'channel' to 'port' to avoid continued confusion.
# No functional change. Rev by Steve
#
# Revision 1.17  2002/12/31 16:08:57  NigburC
# TBolt00006589 - Updated the target info and targets functions to handle
# the PROC changes for the fields.  This also added two new fields to the
# display, TGD_PPORT and TGD_APORT.
# Reviewed by Steve Howe (virtually).
#
# Revision 1.16  2002/11/15 16:00:46  NigburC
# TBolt00000000 - Updated the TARGETSTATUS command to use the group
# requests for TARGETS and SERVERS to make the data retrieval faster.
# Reviewed by Ed Mole.
#
# Revision 1.15  2002/05/30 18:35:03  HoltyB
# Fixed owner in targetStatus to display the correct owner id
#
# Revision 1.14  2002/05/28 14:44:42  HoltyB
# Added ability on targetSetProperties to lock and unlock the target
#
# Revision 1.13  2002/04/26 18:19:48  NigburC
# TBolt00004077 - Added the SERVERS and TARGETS commands to the
# CCB, CCBE and CCBCL.
#
# Revision 1.12  2002/04/01 16:46:54  HoltyB
# Added lock status of target in targetInfo
#
# Revision 1.11  2002/03/29 20:23:47  SchibillaM
# TBolt00003528: Fix bug in Get Target Resource List that caused CCB to loop
# forever because it never allocated enough memory for the request.  Also
# support new size field in return packet.  Required proc code M340.
# Reviewed by Chris.
#
# Revision 1.10  2002/03/11 13:27:04  HoltyB
# Added port WWN to targetStatus
#
# Revision 1.9  2002/03/07 20:26:25  HoltyB
# Made a fix to targetStatus so it would display the luns
#
# Revision 1.8  2002/03/04 17:29:44  NigburC
# TBolt00003191 - Added the TARGETMOVE command to CCBE and CCB.
#
# Revision 1.7  2002/02/14 22:03:22  HoltyB
# added targetStatus - diplays target information
#
# Revision 1.6  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.5  2002/02/05 23:20:09  NigburC
# TBOLT00002954,
# TBOLT00002877,
# TBOLT00002730
#
# This set of changes include the above work items and defects.  Also are
# changes associated with MRP packet structure changes.
# We have moved many functions from CPS_INIT.C to PI_VCG.C in order
# to start returning error codes from the functions that work on the VCG.
# New functions have been added including but not limited to:
#   RESCANDEVICE
#   TARGETRESLIST
#   SERVERLOOKUP (not yet implemented in CCBE and CCBCL)
#   VDISKOWNER
#
# Revision 1.4  2001/12/20 22:33:53  NigburC
# Modified the CCBCL to handle the checking for active connection in the
# big switch statement instead of in each of the command handlers.  This way
# we could use the code from just one area and print the same message in the
# case a command needs an active connection and one is not available.
#
# Added getObjectCount and getObjectList functions to the CCBE and now
# use them for all count and list operations.  These functions now match the
# changes to the packets where LIST no longer takes any input and returns
# a count value along with the list of values.
#
# Revision 1.3  2001/12/12 17:38:31  NigburC
# Completed unmanaged server, create server code with owners.
# Completed the target code, updated to match the new structure sizes.
#
# Revision 1.2  2001/12/05 15:49:30  NigburC
# Added TARGETSETPROP and renamed VDISKRAIDINFO to RAIDINFO.
#
# Revision 1.1  2001/11/28 16:12:21  NigburC
# Added many additional command handlers...
# TARGET commands
# SERVER commands
# Many others...
# Replaced LOGIN and LOGOUT with CONNECT and DISCONNECT
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
