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
PKG_VIEW   = Xiotech-SB4-08-update
VERSION		   = 2_0_0_30
REV		   = 2.0.0.30
#
# Leave	the	following logic	alone
#
all:
	rm -f $(PKG_VIEW)-$(VERSION).zip
	mkdir $(PKG_VIEW)-$(VERSION)
	cp -f ../SystemStatusChecker/SystemStatusChecker $(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/Install		$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/CCBE.tar.gz	$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/TestLibs.tar.gz	$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/*.me		$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/*.pl		$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/*.ima		$(PKG_VIEW)-$(VERSION)
	cp -f UpdatePackageData/*.cmd		$(PKG_VIEW)-$(VERSION)
	zip	-r $(PKG_VIEW)-$(VERSION).zip $(PKG_VIEW)-$(VERSION)
	rm -rf $(PKG_VIEW)-$(VERSION)
	rm -rf ~upd_tmp
	mkdir ~upd_tmp
	java -jar create_release.jar $(PKG_VIEW)-$(VERSION).zip	~upd_tmp
	mv ~upd_tmp/$(PKG_VIEW)-$(VERSION).zip .
	rm -rf ~upd_tmp
	echo "**** Building	$(PKG_VIEW)-$(VERSION).xzp"
	sed	-e 's/XIOSUP/$(PKG_VIEW)/' UpdatePackageData/XIOSUP.xml	>$(PKG_VIEW).xml
	./UpdateSigner.pl -v $(REV)	-p -$(VERSION)/-$(VERSION)-0.rpm -o	. $(PKG_VIEW).xml $(PKG_VIEW)-$(VERSION).zip
	rm -f $(PKG_VIEW).xml
	rm -f $(PKG_VIEW)-$(VERSION).zip

