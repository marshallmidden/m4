#!/usr/bin/perl -w
##############################################################################
# Xiotech Corporation
# Copyright (c) 2006  Xiotech
# ======================================================================
#
# \file     UpdateSigner.pl
# \author   Kevin Utecht
#
# This script takes the following command line options (1 and 2 are mandatory)
#   1) meta-data file.  For example, Xiotech-Platform.xml (see Samples directory)
#   2) Update Package Data file. For example, XIOTECH-PLATFORM-3000-0510-f000.zip
#   3) Output directory.  Where to place the created Update Package file.
#   3) Version assocated with Update Package Data.  For example, 0510-f000
#   4) Primary file.  The main file inside Update Package Data.  For example,
#        "XIOTECH-PLATFORM-750-0510-f030/Firmware 0510-f030/750-0510-f030.rpm2"
#
# This script creates an Update Package.  
#
##############################################################################

use Getopt::Std;

(getopts('o:t:v:p:') and @ARGV == 2)
  or die ("Usage: UpdateSigner.pl [-o output-directory] [-t target] [-v version] [-p primary-file] meta-data-file update-package-data\n");

$metaData = $ARGV[0];
$metaDataBackup = "$metaData.bak";
$updatePackageData = $ARGV[1];
if ($opt_o)
{
  $outputDir = $opt_o;
}
else
{
  $outputDir = ".";
}

# the system call subroutine
sub run
{
    my $rc;
#   print "> $_[0]\n";
    $rc = (system "$_[0]") >> 8;
#   print ">> rc = $rc\n";
    if ($rc) { die "\nAbort: \"$_[0]\" failed, rc = $rc...\n" };
}

sub searchReplaceMetaData
{
    my $rc;
    $rc = (system "sed -e $_[0] $metaDataBackup > temp") >> 8;
    if ($rc) { die "\nAbort: \"$_[0]\" failed, rc = $rc...\n" };
    run "rm -f $metaDataBackup";
    run "mv temp $metaDataBackup";
}

my $basename;
$_ = $updatePackageData;
s/.*\/(.*)\..*$/$1/g;
s/\..*//g;
$basename = $_;

run "cp $metaData $metaDataBackup";

# create hash over updatePackateData to add to meta-data.xml
run "openssl sha1 $updatePackageData | awk '{print \$2}' > checksum";
open(FILE, "checksum");
$_ = <FILE>;
chop;
searchReplaceMetaData "'s/DNE_PAYLOAD_SHA1_DIGEST/$_/g'";

searchReplaceMetaData "'s/DNE_PACKAGE_FILENAME/$basename.xzp/g'";

if ($opt_t)
{
  searchReplaceMetaData "'s/DNE_TARGET/$opt_t/g'";
}

if ($opt_v)
{
  searchReplaceMetaData "'s/DNE_VERSION/$opt_v/g'";

  $_ = $opt_v;
  s/\./_/g;
#  searchReplaceMetaData "'s/DNE_USCORE_VERSION/$_/g'";
}

if ($opt_p)
{
  $_ = $opt_p;
  s/(\/)/\\$1/g;  # convert /'s to \/
  searchReplaceMetaData "'s/DNE_PRIMARY_FILE/$_/g'";
}

run "xmlsec1 --sign --privkey UpdateSignerPrivateKey.pem $metaDataBackup > $basename.xml";
run "rm -f checksum";
run "rm -f $metaDataBackup";
my $range = 5000;
my $randomNumber = int(rand($range));
my $tempFile = "Temp$randomNumber.zip";
run "zip -0 -j $tempFile $basename.xml $updatePackageData";
run "mv $tempFile $outputDir/$basename.xzp";
run "rm -f $basename.xml";
