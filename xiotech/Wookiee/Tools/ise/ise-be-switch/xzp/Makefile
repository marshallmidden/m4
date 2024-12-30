#-----------------------------------------------------------------------------
#			  CONFIDENTIAL AND PROPRIETARY
#	   Copyright (C) 2007-2009 Xiotech Corporation, All Rights Reserved
#-----------------------------------------------------------------------------
# Only change the following three lines:
PKG_VIEW   := Xiotech-BES_Upgrade_H
VERSION    ?= 0_1_0_0
REV	   ?= 0.1.0.0
# Do not upgrade if before release (no letters, just numbers sparated by periods.
NOTB	   ?=''
# Do not upgrade if after release (no letters, just numbers sparated by periods.
NOTA	   ?=''
# This is the version that we are going to.
MATCH	   ?='4.1(3a)'
#-----------------------------------------------------------------------------
# Leave the following logic alone
#
PACKAGE    := $(PKG_VIEW)-$(VERSION)
PWD	   := ${shell pwd}
CCBCL_AT   := ${PWD}/../../../..
PACKAGE_AT := ${PWD}/UpdatePackageData
ISEEWOK_AT := ${PWD}/../
TMPUPD	   := ${PWD}/upd_tmp
IMAGE_STORE:= /home/image-store/Wookiee-blobs
#-----------------------------------------------------------------------------
.PHONY: all
all:
	rm -rf ${PACKAGE} ${PACKAGE}.zip  ${PACKAGE}.xzp ${TMPUPD}
	mkdir ${PACKAGE}
# The unpacking script that is run.
	cp ${PACKAGE_AT}/Install ${PACKAGE}
# The latest version of the ccblCL.pl script, and initialization file (modified)
	tar jcf ${PACKAGE}/CCBE.tbz2 --exclude=.svn -C ${PACKAGE_AT} ccbCL.ini -C ${CCBCL_AT} CCBE/ccbCL.pl CCBE/XIOTech
# The SystemStatusChecker (perl/ccbcl script) to be run between all upgrades (to prevent cascading problems)
	cp -f ${PACKAGE_AT}/../../../../SystemStatusChecker/SystemStatusChecker ${PACKAGE}
# perl script that runs the whole procedure.
	sed -e "s/MATCH/${MATCH}/" ${ISEEWOK_AT}/sw_do_many >${PACKAGE}/sw_do_many
# expect script that checks the switch version.
	sed -e "s/NOTA/${NOTA}/" -e "s/NOTB/${NOTB}/" -e "s/MATCH/${MATCH}/" ${ISEEWOK_AT}/check_sw >${PACKAGE}/check_sw
# expect script that scp's firmware to the switches
	sed -e "s/NOTA/${NOTA}/" -e "s/NOTB/${NOTB}/" -e "s/MATCH/${MATCH}/" ${ISEEWOK_AT}/do_sw >${PACKAGE}/do_sw
# The expect program and library.
	cp ${IMAGE_STORE}/E.tbz2 ${PACKAGE}
# The BE Switch controller firmware files to load.
	cp ${PACKAGE_AT}/kickstart.bin ${PACKAGE}
	cp ${PACKAGE_AT}/system.bin ${PACKAGE}
# .... The new package directory exists ...
# Compress the new directory.
	zip -r ${PACKAGE}.zip ${PACKAGE}
# Remove the original directory created above.
	rm -rf ${PACKAGE}
# Create the release and put it in this directory.
	mkdir ${TMPUPD}
	java -jar create_release.jar ${PACKAGE}.zip ${TMPUPD}
	mv ${TMPUPD}/${PACKAGE}.zip ${PWD}
	rm -rf ${TMPUPD}
	echo "**** Building ${PACKAGE}.xzp"
	sed -e 's/XIOSUP/$(PKG_VIEW)/' ${PACKAGE_AT}/XIOSUP.xml >$(PKG_VIEW).xml
	./UpdateSigner.pl -v $(REV) -p -$(VERSION)/-$(VERSION)-0.rpm -o . $(PKG_VIEW).xml ${PACKAGE}.zip
	rm -f $(PKG_VIEW).xml
	rm -f ${PACKAGE}.zip
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf ${PACKAGE} ${PACKAGE}.zip  ${PACKAGE}.xzp ${TMPUPD}
#-----------------------------------------------------------------------------
