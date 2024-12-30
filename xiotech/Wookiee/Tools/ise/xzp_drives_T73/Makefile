#-----------------------------------------------------------------------------
#			  CONFIDENTIAL AND PROPRIETARY
#	   Copyright (C) 2007-2009 Xiotech Corporation, All Rights Reserved
#-----------------------------------------------------------------------------
# ISE upgrade Ewok Script builder ....minimal Makefile version for validation
#
#-----------------------------------------------------------------------------
# In UpdatePackageData, change do_many_ises (perl script) and ise_upgrader
# expect script called by do_many_ises for any new logic desired. In this file,
# only change the following three lines:
PKG_VIEW   := Xiotech-ISE_DISK_UPDATER-800822
VERSION    := 0_1_0_2
REV	   := 0.1.0.2
#-----------------------------------------------------------------------------
# Leave the following logic alone
#
PACKAGE    := $(PKG_VIEW)-$(VERSION)
PWD	   := ${shell pwd}
CCBCL_AT   := ${PWD}/../../../
PACKAGE_AT := ${PWD}/UpdatePackageData
ISEEWOK_AT := ${PWD}/../ise-drives
TMPUPD	   := ${PWD}/upd_tmp
IMAGE_STORE:= /home/image-store/Wookiee-blobs

#-----------------------------------------------------------------------------
ise_updater:
	rm -rf ${PACKAGE} ${PACKAGE}.zip  ${PACKAGE}.xzp ${TMPUPD}
	mkdir ${PACKAGE}
# The unpacking script that is run.
	cp -f ${PACKAGE_AT}/Install	 ${PACKAGE}
# The latest version of the ccblCL.pl script, and initialization file (modified)
	tar jcf ${PACKAGE}/CCBE.tbz2 -C ${PACKAGE_AT} ccbCL.ini -C ${CCBCL_AT} CCBE/ccbCL.pl CCBE/XIOTech
# The file that says which rev of ise disk firmware may be upgraded to the current one.
	cp -f ${PACKAGE_AT}/ise_disk.cfg ${PACKAGE}
# perl script that runs the whole ISE upgrade procedure.
	cp -f ${ISEEWOK_AT}/disk_do_many_ises ${PACKAGE}
# expect script that ssh-es to the ISE and does upgrade and monitoring of state.
	cp -f ${ISEEWOK_AT}/ise_disk_upgrader ${PACKAGE}
# The expect program and library.
	cp -f ${IMAGE_STORE}/E.tbz2 ${PACKAGE}
# The ISE controller firmware to load -- change ise.cfg.
	cp -f ${PACKAGE_AT}/*.ima ${PACKAGE}
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

