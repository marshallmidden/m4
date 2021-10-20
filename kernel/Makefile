WORKSPACE:=`pwd`

#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  kb        - kb-clean then do the rpmbuild as done via Jenkins.'
	@echo '              unpacks source from tar and apply patches.'
	@echo '  kb-clean  - Get rid of rpmbuild directories/files, etc.'
	@echo '  prep      - kb-clean then do an rpmbuild -bp to unpack source with patches.'
	@echo '  kernel    - Used by Jenkins automation scripts to build the'
	@echo '              kernel for ISO installation and update .tgz files.'
	@echo ''
	@echo '  all       - After "make kb" has been done, and edits (BUILD/k*/l*),'
	@echo '              do a kernel make, all the make installs.'
	@echo '  clean     - Within BUILD/kernel-*/linux-* do a make clean.'
#-----------------------------------------------------------------------------
.PHONY: kernel
kernel:
	rm -f SUCCESS
	rm -rf RPMS
	(cd SPECS && \
	 rpmbuild -bb --define "_topdir "`pwd`/.. \
	  --without kabichk   \
	  --without xen       \
	  --without fips      \
	  --without up        \
	  --without perf      \
	  --without debug     \
	  lightspeed.spec     \
	)
	touch SUCCESS
#-----------------------------------------------------------------------------
.PHONY: kprep
kprep:
	rm -f SUCCESS
	rm -rf RPMS
	(cd SPECS && \
	 rpmbuild -bp --define "_topdir "`pwd`/.. \
	    --without kabichk   \
	    --without xen       \
	    --without fips      \
	    --without up        \
	    --without perf      \
	    --without debug     \
	    lightspeed.spec     \
	)
	touch SUCCESS
	echo "kernel src expanded and patched"
#-----------------------------------------------------------------------------
.PHONY: kb
kb:	kb-clean
	echo ""
	echo "build is starting up"

	time rpmbuild -bb --define "_topdir ${WORKSPACE}" \
	    --without kabichk   \
	    --without xen       \
	    --without fips      \
	    --without up        \
	    --without perf      \
	    --without debug     \
	    ${WORKSPACE}/SPECS/lightspeed.spec
	echo "build complete"
#-----------------------------------------------------------------------------
.PHONY: kb-clean
kb-clean:
	rm -rf BUILD BUILDROOT RPMS/ SRPMS/
#-----------------------------------------------------------------------------
.PHONY: prep
prep:	kb-clean
	echo ""

	time rpmbuild -bp --define "_topdir ${WORKSPACE}" \
	    --without kabichk   \
	    --without xen       \
	    --without fips      \
	    --without up        \
	    --without perf      \
	    --without debug     \
	    ${WORKSPACE}/SPECS/lightspeed.spec
	echo "kernel src expanded, patched, and cleaned"
#-----------------------------------------------------------------------------
.PHONY: all
all:
	(cd BUILD/kernel-*/linux-*/ && \
	    ${MAKE} olddefconfig && \
	    ${MAKE} -j 255 ARCH=x86_64 all && \
	    ${MAKE} -j 255 ARCH=x86_64 modules_install && \
	    ${MAKE} -j 255 ARCH=x86_64 headers_install && \
	    ${MAKE} -j 255 ARCH=x86_64 install)
	sync && sync
	@echo
	@echo 'You might want to edit file /boot/efi/EFI/redhat/grub.cfg to: set default="0"'
	@echo 'and comment out all lines in file /boot/efi/EFI/redhat/grub.cfg then reboot.'
	@echo
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	(cd BUILD/kernel-*/linux-*/ && \
	    ${MAKE} -j 255 clean)
#-----------------------------------------------------------------------------
.PHONY: echo
echo:
	echo "WORKSPACE=${WORKSPACE}"
	which gcc
	gcc --version
	which rpm
	rpm --version
	which rpmbuild
	rpmbuild --version
#-----------------------------------------------------------------------------
# End of file Makefile
#-----------------------------------------------------------------------------
