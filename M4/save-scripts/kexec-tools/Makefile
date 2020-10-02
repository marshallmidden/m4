WORKSPACE := `pwd`
SPECFILE  := kexec-tools.spec
#------------------------------------------------------------------------------
# makedumpfile
# NOTE: Last check-in to git source was January 16th, 2020 for a kernel change
# that occurred July 18, 2019, and that Parsec Labs committed Sept 16, 2019.
# It makes kernel crash dumps occur again.
#-----------------------------------------------------------------------------
# This source fetched on 2020-03-05 and renamed after the "git clone" from: 
#	https://git.code.sf.net/p/makedumpfile/code
# and then tar-red up and put into:
#	SOURCES/makedumpfile-1.6.7.txz
#-----------------------------------------------------------------------------
# How to setup before building.
#------------------------------------------------------------------------------
# 1) Presuming a LightSpeed build system that can compile (such as a LightSpeed
#    install of Release 4.5 on 2020-03-09), and it can yum install packages.
#    See confluence:
#	http://confluence.parsec.lab:8090/display/PAR/Configure+Parsec+Internal+RHEL+Repo?focusedCommentId=15892551#comment-15892551
#?? # 2) Three packages need to be installed -- see make target 'yum_install'.
#------------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'Targets:'
	@echo '  all bb rpmbuild - Unpack source, compile, install, and make rpm file.'
	@echo '  clean           - Remove files created (except rpm).'
	@echo '  reallyclean     - Remove all create files, including rpm.'
#?? 	@echo '  yum_install     - install three packages from repository needed to compile and make.'
	@echo '  bp              - RPM %prep - unpack source step.'
	@echo '  bc              - RPM %build - %prep and compile.'
	@echo '  bi              - RPM %install - %prep, %build, and "make install".'
	@echo '  install         - Copy RPM into isoimage/RPMS directory.'
#?? #-----------------------------------------------------------------------------
#?? # The three rpm packages needed to be installed follow...
#?? # Note: 'repoquery --location "package-name"' used to get following URLs.
#?? .PHONY: yum_install
#?? yum_install:
#?? #       http://172.22.1.13/repo/75/Packages/rpm-build-4.11.3-32.el7.x86_64.rpm
#?? 	yum install -y rpm-build
#?? #       http://172.22.1.13/repo/75/Packages/elfutils-devel-0.170-4.el7.x86_64.rpm
#?? 	yum install -y elfutils-devel
#?? #	http://172.22.1.13/repo/75/Packages/bzip2-devel-1.0.6-13.el7.x86_64.rpm
#?? 	yum install -y bzip2-devel	# Requires (may install) bzip2-libs.
#-----------------------------------------------------------------------------
# Only for testing would one do the following two steps.
#-- make -C SOURCES install
# Get installed /usr/sbin/makedumpfile into initramfs!
#-- dracut --force
#-----------------------------------------------------------------------------
# -bp = Do the %prep stage, unpack the source
.PHONY: bp
bp:
	rpmbuild -bp --define "_topdir ${WORKSPACE}" SPECS/${SPECFILE}
#-----------------------------------------------------------------------------
# -bc = Do the %build state, first "%prep", then 'make' command.
.PHONY: bc
bc:
	rpmbuild -bc --define "_topdir ${WORKSPACE}" SPECS/${SPECFILE}
#-----------------------------------------------------------------------------
# -bi = Do the %install state, first "%prep", "%build", then 'make install'.
.PHONY: bi
bi:
	rpmbuild -bi \
		--define "_topdir ${WORKSPACE}" \
		SPECS/${SPECFILE}
#-----------------------------------------------------------------------------
# -bb = Build package, after the "%prep", "%build", and "%install" stages.
# Note: no debuginfo package is created.
.PHONY: rpmbuild bb all
rpmbuild bb all:
	rpmbuild -bb \
		--define "debug_package %{nil}" \
		--define "_topdir ${WORKSPACE}" \
		SPECS/${SPECFILE}
#-----------------------------------------------------------------------------
.PHONY: install
install:
	cp RPMS/x86_64/kexec-tools-2.0.15-13.el7.x86_64.rpm ../../isoimage/RPMS/
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rpmbuild --clean --define "_topdir ${WORKSPACE}" SPECS/${SPECFILE} || true
	rm -rf BUILD BUILDROOT SRPMS
#------------------------------------------------------------------------------
.PHONY: reallyclean
reallyclean:
	${MAKE} clean
	rm -rf RPMS
#------------------------------------------------------------------------------
# End of file Makefile
#-----------------------------------------------------------------------------
