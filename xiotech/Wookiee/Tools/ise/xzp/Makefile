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
PKG_VIEW   := Xiotech-ISE_UPDATER
VERSION         := 01060200
BUILDID         := 1450
FILE_NAME     := upgrade.ue
#-----------------------------------------------------------------------------
# Leave the following logic alone
#
PWD	   := ${shell pwd}
PACKAGE_AT := ${PWD}/UpdatePackageData
ISEEWOK_AT := ${PWD}/../ise-ewok-upgrade
TMPUPD	   := ${PWD}/upd_tmp
IMAGE_STORE:= /home/image-store/Wookiee-blobs
CCBCL_AT   := ${PWD}/../../../
BUILD_NUMBER_FILE := ${PWD}/xzpbuild

#-----------------------------------------------------------------------------
.PHONY: ise_updater
XZP_VERSION := ${shell sed -n -e '1s/^\(.*\)/\1/ p' ${BUILD_NUMBER_FILE}}
PACKAGE    := $(PKG_VIEW)_$(VERSION)-$(BUILDID)-${XZP_VERSION}

ise_updater:
	@echo '"${XZP_VERSION}"'
	rm -rf ${PACKAGE} ${PACKAGE}.zip  ${PACKAGE}.xzp ${TMPUPD}
	mkdir ${PACKAGE}
# The unpacking script that is run.
	cp -f ${PACKAGE_AT}/Install	 ${PACKAGE}
# The latest version of the ccblCL.pl script, and initialization file (modified)
	tar jcf ${PACKAGE}/CCBE.tbz2 --exclude=.svn -C ${PACKAGE_AT} ccbCL.ini -C ${CCBCL_AT} CCBE/ccbCL.pl CCBE/XIOTech
# The SystemStatusChecker (perl/ccbcl script) to be run between all ISE upgrades (to prevent cascading problems)
	cp -f ${PACKAGE_AT}/../../../SystemStatusChecker/SystemStatusChecker ${PACKAGE}
	cp -f ${PACKAGE_AT}/../../../SystemStatusChecker/check_warm_booted_sw ${PACKAGE}
# The file that says which version of ise firmware may be upgraded to the current one.
	cp -f ${PACKAGE_AT}/ise.cfg ${PACKAGE}
# perl script that runs the whole ISE upgrade procedure.
	cp -f ${ISEEWOK_AT}/do_many_ises ${PACKAGE}
# expect script that ssh-es to the ISE and does upgrade and monitoring of state.
	cp -f ${ISEEWOK_AT}/ise_upgrader ${PACKAGE}
# expect script that checks the switch version.
	cp -f ${ISEEWOK_AT}/check_switch ${PACKAGE}
# The expect program and library.
	cp -f ${IMAGE_STORE}/E.tbz2 ${PACKAGE}
# The ISE controller firmware to load -- change ise.cfg.
	cp -f ${PACKAGE_AT}/${FILE_NAME} ${PACKAGE}
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
	./UpdateSigner.pl -v ${VERSION}-${BUILDID}-${XZP_VERSION} -p -$(VERSION)/-$(VERSION)-0.rpm -o . $(PKG_VIEW).xml ${PACKAGE}.zip
	rm -f $(PKG_VIEW).xml
	rm -f ${PACKAGE}.zip

	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)

#-----------------------------------------------------------------------------
.PHONY: newfirmware new
# The following will create the other formats of version that is needed.
FILENAME := ${shell basename ${FW}}
OLDREV := ${shell sed -n -e 's/^\(REV\s*:=\s*\)\(.*\)\s*/\2/ p' Makefile}
NEWVERSION := ${NEW}
OLDVERSION := ${shell sed -n -e '1s/^"\(.*\)","\(.*\)"/\2/ p' ${PACKAGE_AT}/ise.cfg}
NEWBUILD   := ${BUILD}

newfirmware new:
	@ echo 'NEW        ='"'"${NEW}"'"
	@ echo 'NEWVERSION ='"'"${NEWVERSION}"'"
	@ echo 'OLDVERSION ='"'"${OLDVERSION}"'"
	@ echo 'BUILD      ='"'"${NEWBUILD}"'"
	@ echo 'FILE       ='"'"${FILENAME}"'"
# Get the old firmware version.
# Make sure the new firmware version is specified.
	@ test "${NEW}" != "" || \
	    { echo "Please set variable NEW to new version. Current=${VERSION}"; exit 1; }
	@ test "${BUILD}" != "" || \
	    { echo "Please set variable BUILD to new version. Current=${BUILDID}"; exit 1; }
	@ test "${FW}" != "" || \
	    { echo "Please set variable FW to file path."; exit 1; }
# Create new Makefile with new version.
	@ sed -e 's/^\(VERSION\s*:=\s*\)\(.*\)\s*/VERSION         := '${NEWVERSION}'/' \
		  -e 's/^\(BUILDID\s*:=\s*\)\(.*\)\s*/BUILDID         := '${NEWBUILD}'/' \
		  -e 's/^\(FILE_NAME\s*:=\s*\)\(.*\)\s*/FILE_NAME     := '${FILENAME}'/' \
	      Makefile >AAA.Makefile
# Add new firmware version to ise.cfg, and old one allowed to upgrade from to here.
	sed -e '1s/^"\(.*\)","\(.*\)"/"'${FILENAME}'","'${NEWVERSION}'","'${NEWBUILD}'"/' \
	    -e '2s/^/"'${OLDVERSION}'"\n/' \
	    UpdatePackageData/ise.cfg > AAA.ise.cfg
		 
# Get new controller firmware into correct place.
	cp ${FW} ${PACKAGE_AT}/${FILENAME}
# Move newly created files into position.
	mv AAA.ise.cfg ${PACKAGE_AT}/ise.cfg
	mv AAA.Makefile Makefile
# Now run the new Makefile.
	${MAKE}
#-----------------------------------------------------------------------------
.PHONY:	help
help:
	@ echo
	@ echo 'Use:'
	@ echo '     "make" to create new xzp file called ' ${PKG_VIEW}-${VERSION}.xzp
	@ echo '     "make NEW=01020400 BUILD=937 new" to:'
	@ echo '     a) copy from ~/ISE/controller.fw to UpdatePackageData/controller.fw'
	@ echo '     b) change UpdatePackageData/ise.cfg so current version may upgrade from,'
	@ echo '     c) change ise.cfg so that 01020400 is the new version. NEW/BUILD above.'
	@ echo '     d) change Makefile to have new version in it.'
	@ echo 'Else edit Makefile and UpdatePackageData/ise.cfg as required and do "make".'
	@ echo 'P.S. probably want to do svn commit afterwards.'
	@ echo
#-----------------------------------------------------------------------------
