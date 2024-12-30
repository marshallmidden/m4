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
PKG_VIEW   := Xiotech-ISE_DISK_UPDATER
VERSION   := 01060200-1435
REV   := 1.6.2.0-1435
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
# The SystemStatusChecker (perl/ccbcl script) to be run between all ISE upgrades (to prevent cascading problems)
	cp -f ${PACKAGE_AT}/../../../SystemStatusChecker/SystemStatusChecker ${PACKAGE}
	cp -f ${PACKAGE_AT}/../../../SystemStatusChecker/check_warm_booted_sw ${PACKAGE}
# perl script that runs the whole ISE upgrade procedure.
	cp -f ${ISEEWOK_AT}/disk_do_many_ises ${PACKAGE}/disk_do_many_ises
# expect script that ssh-es to the ISE and does upgrade and monitoring of state.
	cp -f ${ISEEWOK_AT}/ise_disk_upgrader ${PACKAGE}/ise_disk_upgrader
# The expect program and library.
	cp -f ${IMAGE_STORE}/E.tbz2 ${PACKAGE}
# The drive firmware to load -- change ise.cfg.
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
	sed -e 's/XIOSUP/$(PACKAGE)/' ${PACKAGE_AT}/XIOSUP.xml >$(PKG_VIEW).xml
	./UpdateSigner.pl -v $(REV) -p -$(VERSION)/-$(VERSION)-0.rpm -o . $(PKG_VIEW).xml ${PACKAGE}.zip
	rm -f $(PKG_VIEW).xml
	rm -f ${PACKAGE}.zip
#------------------------------------------------------------------------------
.PHONY: newfirmware new
# The following will create the other formats of version that is needed.
NEWVERSION := ${VERSION}
NEWREV := ${REV}

newfirmware new:
	@ echo 'VERSION        ='"'"${VERSION}"'"
	@ echo 'NEWVERSION ='"'"${NEWVERSION}"'"
	@ echo 'REV ='"'"${REV}"'"
	@ echo 'NEWREV   ='"'"${NEWREV}"'"
# Make sure the new firmware version is specified.
	@ test "${NEWVERSION}" != "" || \
	    { echo "Please set variable VERSION to new version. Current=${VERSION}"; exit 1; }
	@ test "${NEWREV}" != "" || \
	    { echo "Please set variable REV to new revision. Current=${REV}"; exit 1; }
#Create new Makefile with new version
	@ sed -e 's/^\(VERSION\s*:=\s*\)\(.*\)\s*/VERSION   := '$(NEWVERSION)'/' \
	        -e 's/^\(REV\s*:=\s*\)\(.*\)\s*/REV   := '$(NEWREV)'/' \
	     Makefile >AAA.Makefile
# Move newly created files into position
	mv AAA.Makefile Makefile
# Now run the new Makefile
	${MAKE}
#------------------------------------------------------------------------------
.PHONY:	help
help:
	@ echo
	@ echo 'Use:'
	@ echo '     "make" to create new xzp file. For example:' ${PKG_VIEW}-${VERSION}.xzp
	@ echo 'Example:'
	@ echo '     "make VERSION=01060100-xxxx REV=1.6.0.0-xxxx new"'
	@ echo '      Where xxxx is the nightly build version'
	@ echo
#-----------------------------------------------------------------------------
