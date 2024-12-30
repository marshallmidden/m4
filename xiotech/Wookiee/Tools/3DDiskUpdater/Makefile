##############################################################################
#				  CONFIDENTIAL AND PROPRIETARY
#		   Copyright (C) 2007 Xiotech Corporation, All Rights Reserved
###############################################################################
#
# \file		Makefile
# \author	Todd Burkey
#
# CCB Script Runner....minimal makefile	version	for	validation
#
###############################################################################
#
# In UpdatePackageData,	change shell.me	for	any	new	logic desired and
# ONLY add new scripts/macros/command lists	using the .me extension.
# In this file,	only change	the	following three	lines:
#
PKG_VIEW   = Xiotech-3D_DISK_UPDATER
VERSION    = 0_5_0_9
REV        = 0.5.0.9
PKG_NAME   = $(PKG_VIEW)-$(VERSION)
PKG_DIR    = $(PKG_NAME)
#
# Leave	the	following logic	alone
#
all:
	rm -f $(PKG_NAME).zip
	mkdir $(PKG_DIR)
	cp -f ../SystemStatusChecker/SystemStatusChecker $(PKG_DIR)
	cp -f UpdatePackageData/Install		$(PKG_DIR)
	cp -f UpdatePackageData/TestLibs.tbz2 $(PKG_DIR)
	cp -f UpdatePackageData/*.pl		$(PKG_DIR)
	cp -f UpdatePackageData/*.ima		$(PKG_DIR)
	cp -f UpdatePackageData/*.cmd		$(PKG_DIR)
	zip	-r $(PKG_NAME).zip $(PKG_DIR)
	rm -rf $(PKG_DIR)
	rm -rf ~upd_tmp
	mkdir ~upd_tmp
	java -jar create_release.jar $(PKG_NAME).zip	~upd_tmp
	mv ~upd_tmp/$(PKG_NAME).zip .
	rm -rf ~upd_tmp
	echo "**** Building	$(PKG_NAME).xzp"
	sed	-e 's/XIOSUP/$(PKG_VIEW)/' UpdatePackageData/XIOSUP.xml	>$(PKG_VIEW).xml
	./UpdateSigner.pl -v $(REV)	-p -$(VERSION)/-$(VERSION)-0.rpm -o	. $(PKG_VIEW).xml $(PKG_NAME).zip
	rm -f $(PKG_VIEW).xml
	rm -f $(PKG_NAME).zip

