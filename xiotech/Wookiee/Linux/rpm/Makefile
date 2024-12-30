# $Id: Makefile 158811 2011-12-20 20:42:56Z m4 $
#
# Makes the platform RPM (.rpm) and uploadable (.rpm2) images
# A 'Release', 'Version' and 'MODEL should be passed in to make on
# the command line:
#
# make all RELEASE=0400 VER=E110 MODEL=3000
#
#-----------------------------------------------------------------------------

include ../../Bin/MMODEL.mk

# IMPORTANT: Files added to the 'FILES' variable must also be added into the 
#		'platformApps-${MMODEL}.spec' file (in this same directory) under the 
#		"%install" and "%files" sections.

OBJFILES := ccbrun pam iscsid syssrv Back.t Front.t shmdump
FILES := 2400.bin 2400mid.bin 2500.bin 2500mid.bin \
		chgnetcfg kernel.tgz kernelver bvm xioFidScript gzshm

# NOTE: VER & RELEASE are passed in on the command line
NAME         := platformApps-${MMODEL}
NAME_REL     := ${NAME}-${RELEASE}
NAME_REL_VER := ${NAME_REL}-${VER}
MODEL_REL_VER 	 := ${MMODEL}-${RELEASE}-${VER}

# Directories

YETIAPPS     := ../../built-${MODEL}${TYPE}
TOPDIR := ${shell test -r ~/.rpmmacros && awk '/^\%_topdir/{print $$2}' ~/.rpmmacros}
ifeq (${strip ${TOPDIR}},)
TOPDIR       := /usr/src/packages
endif

# Non-standard utilities

ROOT         := root

#-----------------------------------------------------------------------------
all: clean build

#-----------------------------------------------------------------------------
build:
	@echo 'TOPDIR = ${TOPDIR}'

# Create the build directory tree if its not there.  This is basically a hack
# to allow for the NFS RO /usr file system that the EP guys use in their
# development environment.

	test ! -d ${TOPDIR} || mkdir -p -m 755 ${TOPDIR}/{BUILD,RPMS,SOURCES,SPECS}

# Make the source compressed tar file

	cd ${YETIAPPS} && md5sum ${FILES} > MD5sums
	mkdir ${YETIAPPS}/${NAME_REL}
	cd ${YETIAPPS} && cp ${FILES} ${OBJFILES} MD5sums ${NAME_REL}
ifeq (x${PERF},xPERF)
	cd ${YETIAPPS}/${NAME_REL} && strip ${OBJFILES}
endif
	cd ${YETIAPPS}/${NAME_REL} && md5sum ${OBJFILES} >> MD5sums
	cd ${YETIAPPS} && md5sum ${OBJFILES} >> MD5sums
	cd ${YETIAPPS} && tar -czf ${NAME}.tar.gz ${NAME_REL}
	cp ${YETIAPPS}/${NAME}.tar.gz ${TOPDIR}/SOURCES

# Modify the spec file with requested version & release. Notice how our
# definition of Release and Version are exactly opposite of that of the rpm
# utility!

	sed -e "s/^Version:   .*/Version:   ${RELEASE}/" \
	    -e "s/^Release:   .*/Release:   ${VER}/" \
		${NAME}.spec | tee ${TOPDIR}/SPECS/${NAME}.spec >/dev/null
	rm -rf ${TOPDIR}/${USER}
	rpmbuild -bb --buildroot ${TOPDIR}/${USER} \
		--target i586-SuSE-Linux ${TOPDIR}/SPECS/${NAME}.spec

# Build the RPM

	cp ${TOPDIR}/RPMS/i586/${NAME_REL_VER}.i586.rpm ${YETIAPPS}/${NAME_REL_VER}.rpm

# Add a Xiotech header to the RPM, call the result an ".rpm2"

	cd ${YETIAPPS} && \
		perl ../Tools/AddHdr/AddHdr.pl ${NAME_REL_VER}.rpm \
			../Tools/AddHdr/${MODEL}RPM.cfg ${RELEASE};
	mv ${YETIAPPS}/${NAME_REL_VER}.ima ${YETIAPPS}/${MODEL_REL_VER}.rpm2

# Clean up

	-rm -rf ${YETIAPPS}/${NAME_REL}
	-rm ${YETIAPPS}/${NAME}.tar.gz


#-----------------------------------------------------------------------------
clean:
	-rm -rf ${YETIAPPS}/${NAME}*

#-----------------------------------------------------------------------------
apply install:
	${ROOT} rpm -i --force --nodeps ${YETIAPPS}/${NAME_REL_VER}.rpm

#-----------------------------------------------------------------------------
remove delete erase:
	${ROOT} rpm -e ${NAME_REL}

#-----------------------------------------------------------------------------
query:
	@rpm -q ${NAME_REL}

#-----------------------------------------------------------------------------
queryall:
	@rpm -q ${NAME}

#-----------------------------------------------------------------------------
run:
	cd /opt/xiotech/release/${NAME_REL_VER} && ${ROOT} ./pam

#-----------------------------------------------------------------------------
help:
	@echo ''
	@echo 'Usage: make target(s) RELEASE=RRRR VER=VVVV'
	@echo ''
	@echo '  all        - clean then build [DEFAULT, SUGGESTED]'
	@echo '  apply      - apply the rpm'
	@echo '  build      - build rpm image'
	@echo '  clean      - remove generated files'
	@echo '  help       - print this help/usage text'
	@echo '  query      - query this specific rpm'
	@echo '  queryall   - query all rpms of this type (platformApps)'
	@echo '  remove     - remove the rpm'
	@echo '  run        - run pam from the installed directory'
	@echo ''
	@echo 'If your development system has a Read-Only "/usr" filesystem,'
	@echo 'create a ".rpmmacros" file in your home directory with the'
	@echo 'following 2 lines in it:'
	@echo ''
	@echo '# Path to top of build area.'
	@echo '%_topdir /opt/xiotech/rpmroot/packages'
	@echo ''

#-----------------------------------------------------------------------------
# vi:sw=4 ts=4 noexpandtab
