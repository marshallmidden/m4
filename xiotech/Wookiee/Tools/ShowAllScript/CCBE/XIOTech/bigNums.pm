# $Id: bigNums.pm 144092 2010-07-13 21:40:51Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
#
# Purpose:
#   Handles 64bit numbers by using the BigInt package
##############################################################################
package XIOTech::bigNums;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(bigIntTolongs longsToBigInt bigIntToHex);

use Math::BigInt;
use strict;

##############################################################################
# Name: BigIntTolongs
#
# Desc: Converts a Math::BigInt to a set of scalars (~4 bytes each)
#
# NOTE: Only works for integers >=0 &&< (2^64 - 1) 18446744073709551615
#
# Need to do range check
##############################################################################
sub bigIntTolongs
{
    my ($param) = @_;

    my $in;

    # This way if we use a string representation
    # we will contine to work

    # Get a big integer object and then extract the HI word.
    $in = Math::BigInt->new($param);
    my $hiBI = $in->brsft("+32");

    # Get a big integer object and then extract the LO word.
    $in = Math::BigInt->new($param);
    my $loBI = $in->band(0xFFFFFFFF);

    # Remove the + from the beginning
    $hiBI = $hiBI + 0;
    $loBI = $loBI + 0;

    return (HI_LONG => $hiBI,
            LO_LONG => $loBI);
}

##############################################################################
# Name: longsToBigInt
#
# Desc: Converts 2 scalars to a single long
#
# NOTE: Only works for scalars  >= 0 && <= 2^32 in size
#
# In: hash with the follow indexs: HI_LONG, LO_LONG
#
# Returns a BigInt on success, else returns undef.
##############################################################################
sub longsToBigInt
{
    my (%longs) = @_;

    if (($longs{HI_LONG} <= 0xFFFFFFFF) &&
        ($longs{LO_LONG} <= 0xFFFFFFFF))
    {
        my $hBI = Math::BigInt->new($longs{HI_LONG});
        my $lBI = Math::BigInt->new($longs{LO_LONG});
        my $result = $lBI | ($hBI << 32 );
        return $result;
    }
    else
    {
        return undef;
    }
}

##############################################################################
# Name: hexChar
#
# Desc: Converts a number in the range of 0-15 to 0-F for hex
#
# In:   Scalar number to be converted
#
# Returns:  Text value of number using base 16, undef if not in the range of
#           0-15 decimal
##############################################################################
sub hexChar
{
    my ($parm) = @_;

    my $rc = $parm + 0;

    if ($parm > 9)
    {
        if ($parm == 10)
        {
            $rc = "a";
        }
        elsif ($parm == 11)
        {
            $rc = "b";
        }
        elsif ($parm == 12)
        {
            $rc = "c";
        }
        elsif ($parm == 13)
        {
            $rc = "d";
        }
        elsif ($parm == 14)
        {
            $rc = "e";
        }
        elsif ($parm == 15)
        {
            $rc = "f";
        }
        else
        {
            $rc = undef;
        }
    }
   
    return $rc;
}

##############################################################################
# Name: bigIntToHex
#
# Desc: Converts a Math::BigInt to a hex string representation
#
# In:   BigInt  Number to be converted
#
# Returns:  scalar that is the textual representation of the big integer
#           On error we return undef
#
# Note: Currently limited to negative numbers in the range of -1 to -2^4095
#       but could be easily extended so thats -1 to
#-5221944407065762533458763553583121912899821245236918901921167416419769539857
# 7872842441340596749877917044505335721963141899378671909289680363161804392568
# 2638972978488271854999170180795067191859157214035005927973113188159419698856
# 3728361673421722933087484039543529018520356420243700593045572339888917990145
# 0334346948844089389297345281509513047029978972671641173465151334822152951250
# 7986199933857107770846917779942645743159118957217248367043905936319748237550
# 0945206745042085308375468341669252755164860441347753849918081847059665076068
# 9841291859404591682837561065924642318406277511299915020617239243129783724609
# 7308511903252956622805412865917690043804311051417135098849101156584508839003
# 3375977425399608182096851426875623920074535795677299913952566998057758971355
# 5341556704529213644213989577742489147716176725853261163453069745299384650106
# 1481697843891439474220308003706472837459911525285821188577408160690315522951
# 4580684633541714282203652239499859508907328817366119251336265299498979980453
# 9973460088731240885922493372782962508916453523655971658277540378411092328587
# 3186648442456409760158728501220463308455437074192539205964902261490928669488
# 8240515630429515006512067335948633366082457555658014603908690167180451219023
# 54170201577095168
#
#   Postive numbers are unlimted in their range at this time
#
# :-)
##############################################################################
sub bigIntToHex
{
    my ($param) = @_;
    my $decNum = 0;
    my @convNum = ();
    my $count = 0;
    my $rc = "0x";
    my @negativeExps = (31, 63, 127, 255, 511, 1023, 2047, 4095);
    
    #Need to validate that this is a correctly formated BigInt
    if ($param !~ /^[\+\-]{1}\d+$/  && (ref($param) ne 'Math::BigInt'))
    {
        #Not so return error
        return undef;
    }

    my $in = Math::BigInt->new($param);
    
    
    #Negative numbers are converted to two's complement notation and
    #then converted to hex.
    if ($in->bcmp(0) < 0)
    {
        #convert to a positive number
        $in = $in * Math::BigInt->new("-1");
        
        #Create a big int == 2 for 2 some power
        my $range = Math::BigInt->new("2");
        
        my $foundRange = 0;
        
        #Find out what range this negative is in
        for (my $r = 0; $r < scalar(@negativeExps); ++$r)
        {
            if ($in->bcmp($range->bpow($negativeExps[$r])) < 0)
            {
                #We have found the correct range for this number
                #so calulate the 2^N for the unsigned range
                #
                # e.g.  Largest negative signed 4 byte ints are 2^31
                #       so we take 2^32 - number to get to two complement
                $range = $range->bpow($negativeExps[$r] + 1);
                $in = $range - $in;
                $foundRange = 1;
                last;
            }   
        }

        #In the case that we were unable to find a suitable range return error
        if (!$foundRange)
        {
            return undef;
        }
    }
    
    #caluclate the number to base 16
    do
    {
        ++$count;
        $convNum[$count] = $in->bmod(16);
        $in = $in / 16; 
    } 
    while($in->bcmp(0) != 0);

    #Build the hex representation
    for (my $i = $count; $i > 0; --$i)
    {
        my $hexChar = hexChar($convNum[$i]);
        
        if (defined($hexChar))
        {
            $rc = $rc . $hexChar;
        }
        else
        {
            #Get out and return error
            $rc = undef;
            return $rc;
        }
    }

    return $rc;
}

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
