WORKSPACE:=`pwd`

#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  lshell-bb      -  Build lshell rpm (patch applied)'
	@echo '  lshell-clean   - Remove BUILD, BUILDROOT, RPMS, SRPMS directories.'
#-----------------------------------------------------------------------------
# rpmbuild options of interest:
#    -ba Build binary and source packages after:  %prep %build %install
#    -bb Build binary package after:  %prep %build %install
#    -bp Does %prep stage -- unpack sources and apply patches.
#    -bc Does %build stage -- like a "make"
#    -bi Does %install stage -- like a "make install"
#    -bl Does a 'list check' -- see if each file exists.
#    -bs Build the source package.
#-----------------------------------------------------------------------------
.PHONY: lshell-bb
lshell-bb:
	rpmbuild -bb --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-bp
lshell-bp:
	rpmbuild -bp --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-ba
lshell-ba:
	rpmbuild -ba --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-bc
lshell-bc:
	rpmbuild -bc --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-bi
lshell-bi:
	rpmbuild -bi --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-bl
lshell-bl:
	rpmbuild -bl --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-bs
lshell-bs:
	rpmbuild -bs --define "_topdir ${WORKSPACE}" ${WORKSPACE}/SPECS/lshell.spec
#-----------------------------------------------------------------------------
.PHONY: lshell-clean
lshell-clean:
	rm -rf BUILD BUILDROOT RPMS/ SRPMS/
#-----------------------------------------------------------------------------
# End of file Makefile
#-----------------------------------------------------------------------------

