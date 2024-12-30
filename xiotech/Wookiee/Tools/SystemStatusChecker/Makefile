###############################################################################################
#                                 CONFIDENTIAL AND PROPRIETARY
#                  Copyright (C) 2010 Xiotech Corporation, All Rights Reserved
###############################################################################################
# 
# \file     Makefile
# \author   Tom Marlin
# 
# System Health Check  makefile.
#
#
###############################################################################################
#----------------------------------------------------------------------------------------------
PKG_VIEW   := System_Health_Checker
VERSION    ?= 1_0_0_5
REV        ?= 1_0_0_5
PWD        := ${shell pwd}
PACKAGE_AT := ${PWD}/UpdatePackageData
CCBCL_AT   := ${PWD}/../../
PACKAGE    := $(PKG_VIEW)-$(VERSION)
TMPUPD     := ${PWD}/upd_tmp
#----------------------------------------------------------------------------------------------

.PHONY: all
all:

# Clean up temporary files and directories
	echo "Removing temporary files..."
	rm System_Health*.zip -rf
	rm UpdatePackageData/CCBE.tbz2 -rf
	rm UpdatePackageData/SystemStatusChecker -rf 

# Clean up old files
	rm -rf ${PACKAGE} ${PACKAGE}.zip ${PACKAGE}.xzp ${TMPUPD}
	mkdir ${PACKAGE}
	
# Copy required files
	cp -f SystemStatusChecker ${PACKAGE}
	cp -f check_warm_booted_sw ${PACKAGE}
	cp -f ${PACKAGE_AT}/Install ${PACKAGE}

# Grab the latest version of the ccblCL.pl script, and initialization file (modified)
	echo	"Including CCBCL..."
	tar jcf ${PACKAGE}/CCBE.tbz2 --exclude=.svn -C ${PACKAGE_AT} ccbCL.ini -C ${CCBCL_AT} CCBE/ccbCL.pl CCBE/XIOTech

# Compress the new directory
	zip -r ${PACKAGE}.zip ${PACKAGE}

# Remove the original directory created above
	rm -rf ${PACKAGE}
# Create the release and put it in this directory
	mkdir ${TMPUPD}
	java -jar create_release.jar ${PACKAGE}.zip ${TMPUPD}
	mv ${TMPUPD}/${PACKAGE}.zip ${PWD}
	rm -rf ${TMPUPD}
	echo "**** Building ${PACKAGE}.xzp"
	sed -e 's/XIOSUP/$(PKG_VIEW)/' ${PACKAGE_AT}/XIOSUP.xml >$(PKG_VIEW).xml
	./UpdateSigner.pl -v $(REV) -p -$(VERSION)/-$(VERSION)-0.rpm -o . $(PKG_VIEW).xml ${PACKAGE}.zip
	rm -f $(PKG_VIEW).xml
	rm -f ${PACKAGE}.zip


#
# 'pkg_clean' target removes all generated files
#

proj_clean: pkg_clean

.PHONY: pkg_clean

pkg_clean:
	rm System_Health* -rf

