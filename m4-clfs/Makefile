# NOTDONEYET -- items to be done yet.
#=============================================================================
# Following puts compiler tools into new_root, if true.
export INCLUDE_COMPILERS_IN_PACKAGE=true
# export INCLUDE_COMPILERS_IN_PACKAGE=false
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Following puts perl into new_root, if true.
export INCLUDE_PERL_IN_PACKAGE=true
# export INCLUDE_PERL_IN_PACKAGE=false
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Following puts a statically compiled bash into /etc/bash, if true.
export INCLUDE_STATIC_BASH_IN_PACKAGE=true
# export INCLUDE_STATIC_BASH_IN_PACKAGE=false
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# No ONE_PART to have two partitions, first swap.
export ONE_PART=true
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Following removes the untar-red sources when no longer needed (for bld03 saving disk space).
# export RM_UNPACKED_SOURCES_WHEN_DONE=true
export RM_UNPACKED_SOURCES_WHEN_DONE=false
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Following specifies an alternate linux configuration. Comment out for default.
#? export LINUX_CONFIGURATION=SAVED.CONFIG.linux-4.9.1
export LINUX_CONFIGURATION=SAVED.CONFIG.linux-4.9.1.nommu
#-----------------------------------------------------------------------------
.PHONY: help
help:
	@echo 'clean                    - To remove the following (and make .run.* files).'
	@echo '  clean-cross-tools      -   To remove created cross-tools.'
	@echo '  clean-unpacked-sources -   To remove untarred sources.'
	@echo '  clean-new_root         -   To remove created new_root directory.'
	@echo ''
	@echo 'all                      - To compile everything and create new_root directory'
	@echo '  dev                    -   Also move it to /dev/sd? specified in 000.set.variables.'
	@echo '  new_root_image.file    -   Create image of new_root for dd-ing, etc.'
	@echo '  genext2fs              -   Same as above, another name for new_root_image.file.'
	@echo '  gen2dev-sdb1           -   Does genext2fs then dd image to /dev/sdb1.'
	@echo '  gen2dev-sdb2           -   Does genext2fs then dd image to /dev/sdb2.'
	@echo ' '
	@echo 'header-files             - Create the include files.'
	@echo 'cross-tools              - Create the cross-compiler tools.'
	@echo 'minimal                  - Create a minimally bootable system.'
	@echo 'usable                   - Create a usable system. (all+dev)'
#=============================================================================
ONE000=1.clfs.set.variables 000.set.variables
ZERO000=0.clfs.set.variables 000.set.variables
#=============================================================================
.PHONY: all
all:	.run.400
#-----------------------------------------------------------------------------
.PHONY: usable dev
usable dev Run.This.4th:	.run.400 ${ZERO000} ${ONE000}
	./Run.This.4th
#-----------------------------------------------------------------------------
.PHONY: new_root.image.file genext2fs
genext2fs new_root.image.file ../new_root.image.file: Run.This.genext2fs .run.400 ${ZERO000} ${ONE000}
	./Run.This.genext2fs
#-----------------------------------------------------------------------------
.PHONY: gen2dev-sdb1
gen2dev-sdb1: ../new_root.image.file
	root dd if=../new_root.image.file of=/dev/sdb1 bs=1M
#-----------------------------------------------------------------------------
.PHONY: gen2dev-sdb2
gen2dev-sdb2: ../new_root.image.file
	root dd if=../new_root.image.file of=/dev/sdb2 bs=1M
#=============================================================================
.PHONY: YESclean
YESclean:	clean-cross-tools clean-unpacked-sources clean-new_root
	rm -f .run.*

.PHONY: clean-cross-tools
clean-cross-tools:
	rm -rf ../clfs-cross-tools/*

.PHONY: clean-unpacked-sources
clean-unpacked-sources:
	rm -rf ../sources/.unpack.* ../sources/.patch.* ../sources/*

.PHONY: clean-new_root
clean-new_root:
	root rm -rf ../new_root/*
#=============================================================================
.run.025: 025-creatingtoolsdir ${ZERO000}
	./025-creatingtoolsdir						&& touch .run.025
.run.026: .run.025 026-creatingcrossdir ${ZERO000}
	./026-creatingcrossdir						&& touch .run.026
#-----------------------------------------------------------------------------
# Compile and install cross-tools commands/headers/libraries/etc.
.run.032: .run.026 032-file ${ZERO000}
	./032-file							&& touch .run.032
.PHONY: header-files
header-files .run.033: .run.032 033-linux-headers ${ZERO000}
	./033-linux-headers						&& touch .run.033
.run.034: .run.033 034-m4 ${ZERO000}
	./034-m4							&& touch .run.034
.run.035: .run.034 035-ncurses ${ZERO000}
	./035-ncurses							&& touch .run.035
.run.036: .run.035 036-pkg-config-lite ${ZERO000}
	./036-pkg-config-lite						&& touch .run.036
.run.037: .run.036 037-gmp ${ZERO000}
	./037-gmp							&& touch .run.037
.run.038: .run.037 038-mpfr ${ZERO000}
	./038-mpfr							&& touch .run.038
.run.039: .run.038 039-mpc ${ZERO000}
	./039-mpc							&& touch .run.039
.run.040: .run.039 040-isl ${ZERO000}
	./040-isl							&& touch .run.040
.run.041: .run.040 041-binutils ${ZERO000}
	./041-binutils							&& touch .run.041
.run.042: .run.041 042-gcc-static ${ZERO000}
	./042-gcc-static						&& touch .run.042
#-----------------------------------------------------------------------------
# Finish by making a shared glibc and a gcc that uses it.
.run.043: .run.042 043-glibc ${ZERO000}
	./043-glibc							&& touch .run.043
.PHONY: cross-tools
cross-tools .run.044: .run.043 044-gcc-final ${ZERO000}
	./044-gcc-final							&& touch .run.044
#-----------------------------------------------------------------------------
# Put compiler prerequirements into package.
.run.047: .run.044 047-gmp ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./047-gmp; fi		&& touch .run.047
.run.048: .run.047 048-mpfr ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./048-mpfr; fi	&& touch .run.048
.run.049: .run.048 049-mpc ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./049-mpc; fi		&& touch .run.049
.run.050: .run.049 050-isl ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./050-isl; fi		&& touch .run.050
#-----------------------------------------------------------------------------
# Compile and install tools for native build.
# Need zlib first
.run.051: .run.050 051-zlib ${ONE000}
	./051-zlib							&& touch .run.051
.run.052: .run.051 052-binutils ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./052-binutils; fi	&& touch .run.052
.run.053: .run.052 053-gcc ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./053-gcc; fi		&& touch .run.053
#-----------------------------------------------------------------------------
.run.054: .run.053 054-ncurses ${ONE000}
	./054-ncurses							&& touch .run.054
.run.055: .run.054 055-bash ${ONE000}
	./055-bash							&& touch .run.055
.run.056: .run.055 056-bzip2 ${ONE000}
	./056-bzip2							&& touch .run.056
.run.057: .run.056 057-check ${ONE000}
	./057-check							&& touch .run.057
.run.058: .run.057 058-coreutils ${ONE000}
	./058-coreutils							&& touch .run.058
.run.059: .run.058 059-diffutils ${ONE000}
	./059-diffutils							&& touch .run.059
.run.060: .run.059 060-file ${ONE000}
	./060-file							&& touch .run.060
.run.061: .run.060 061-findutils ${ONE000}
	rm -f ../sources/.unpack.findutils-4.4.2	# Cannot compile twice, must unpack and then compile.
	./061-findutils							&& touch .run.061
.run.062: .run.061 062-gawk ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./062-gawk; fi	&& touch .run.062
.run.063: .run.062 063-gettext ${ONE000}
	./063-gettext							&& touch .run.063
.run.064: .run.063 064-grep ${ONE000}
	./064-grep							&& touch .run.064
.run.065: .run.064 065-gzip ${ONE000}
	./065-gzip							&& touch .run.065
.run.066: .run.065 066-make ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./066-make; fi	&& touch .run.066
.run.067: .run.066 067-patch ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./067-patch; fi	&& touch .run.067
.run.068: .run.067 068-sed ${ONE000}
	./068-sed							&& touch .run.068
.run.069: .run.068 069-tar ${ONE000}
	./069-tar							&& touch .run.069
.run.070: .run.069 070-texinfo ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./070-texinfo; fi	&& touch .run.070
.run.071: .run.070 071-util-linux ${ONE000}
	./071-util-linux						&& touch .run.071
.run.072: .run.071 072-vim ${ONE000}
	./072-vim							&& touch .run.072
.run.073: .run.072 073-xz ${ONE000}
	./073-xz							&& touch .run.073
.run.074: .run.073 074-choose ${ONE000}
	./074-choose && ( echo "SHOULD HAVE FAILED!"; echo "FIGURE OUT WHAT HAPPENED!"; exit 1) || true
	touch .run.074
#-----------------------------------------------------------------------------
.run.076: .run.074 076-bc ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./076-bc; fi		&& touch .run.076
#-----------------------------------------------------------------------------
.run.078: .run.076 078-e2fsprogs ${ONE000}
	./078-e2fsprogs							&& touch .run.078
.run.079: .run.078 079-kmod ${ONE000}
	./079-kmod							&& touch .run.079
.run.080: .run.079 080-shadow ${ONE000}
	./080-shadow							&& touch .run.080
.run.081: .run.080 081-sysvinit ${ONE000}
	./081-sysvinit							&& touch .run.081
.run.082: .run.081 082-eudev ${ONE000}
	./082-eudev							&& touch .run.082
.run.083: .run.082 083-kernel ${ONE000}
	./083-kernel							&& touch .run.083
.run.084: .run.083 084-grub-build ${ONE000}
	./084-grub-build						&& touch .run.084
.run.085: .run.084 085-creatingdirs ${ONE000}
	./085-creatingdirs						&& touch .run.085
.run.086: .run.085 086-createfiles ${ONE000}
	./086-createfiles						&& touch .run.086
.run.088: .run.086 088-pwdgroup ${ONE000}
	./088-pwdgroup							&& touch .run.088
.run.089: .run.088 089-fstab ${ONE000}
	./089-fstab							&& touch .run.089
.run.090: .run.089 090-settingenvironment ${ONE000}
	./090-settingenvironment					&& touch .run.090
#-----------------------------------------------------------------------------
# Make sure ld.so.conf, etc. is ready for running.
.run.095: .run.090 Extra.095.ld.so
	./Extra.095.ld.so						&& touch .run.095
#-----------------------------------------------------------------------------
# Put hello world out there.
.run.097: .run.095 Extra.097.helloworld ${ONE000}
	./Extra.097.helloworld						&& touch .run.097
#-----------------------------------------------------------------------------
# Put everything for grub out -- except for the grub part in the MBR, etc.
.run.098: .run.097 Extra.098.semi-grub-install
	./Extra.098.semi-grub-install					&& touch .run.098
#=============================================================================
.PHONY: minimal
minimal: .run.098
	./Run.This.4th
#=============================================================================
# Following creates a stand-alone setup for later -- not a chroot environment for doing later setup.
.run.104: .run.098 104-tcl ${ONE000}
	./104-tcl							&& touch .run.104
#-----------------------------------------------------------------------------
#-- .run.105: .run.104 105-expect
#-- # NOTDONEYET
#-- 	#- * cannot be cross-compiled *
#-- 	#- ./105-expect
#-- 	touch .run.105
#-----------------------------------------------------------------------------
#-- .run.121: .run.105 121-flex ${ONE000}
.run.121: .run.104 121-flex ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./121-flex; fi	&& touch .run.121
.run.122: .run.121 122-bison ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./122-bison; fi	&& touch .run.122
.run.130: .run.122 130-procps-ng ${ONE000}
	./130-procps-ng							&& touch .run.130
.run.133: .run.130 133-iana-etc ${ONE000}
	./133-iana-etc							&& touch .run.133
.run.134: .run.133 134-libtool ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./134-libtool; fi	&& touch .run.134
.run.135: .run.134 135-iproute2 ${ONE000}
	./135-iproute2							&& touch .run.135
.run.137: .run.135 137-gdbm ${ONE000}
	./137-gdbm							&& touch .run.137
#-----------------------------------------------------------------------------
# Optionally a cross-compiled perl.
.run.138: .run.137 138-perl ${ONE000}
	if ${INCLUDE_PERL_IN_PACKAGE} ; then ./138-perl; fi		&& touch .run.138
#-----------------------------------------------------------------------------
.run.140: .run.138 140-autoconf ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./140-autoconf; fi	&& touch .run.140
.run.141: .run.140 141-automake ${ONE000}
	if ${INCLUDE_COMPILERS_IN_PACKAGE} ; then ./141-automake; fi	&& touch .run.141
#-----------------------------------------------------------------------------
.run.151: .run.141 151-less ${ONE000}
	./151-less							&& touch .run.151
.run.153: .run.151 153-iputils ${ONE000}
	./153-iputils							&& touch .run.153
.run.154: .run.153 154-kbd ${ONE000}
	./154-kbd							&& touch .run.154
#-----------------------------------------------------------------------------
.run.161: .run.154 161-psmisc ${ONE000}
	./161-psmisc							&& touch .run.161
.run.162: .run.161 162-libestr ${ONE000}
	./162-libestr							&& touch .run.162
.run.163: .run.162 163-libee ${ONE000}
	./163-libee							&& touch .run.163
.run.164: .run.163 164-rsyslog ${ONE000}
	./164-rsyslog							&& touch .run.164
.run.175: .run.164 175-bootscripts ${ONE000}
	./175-bootscripts						&& touch .run.175
.run.181: .run.175 181-profile ${ONE000}
	./181-profile							&& touch .run.181
.run.182: .run.181 182-locale ${ONE000}
	./182-locale							&& touch .run.182
.run.183: .run.182 183-inputrc ${ONE000}
	./183-inputrc							&& touch .run.183
#-----------------------------------------------------------------------------
.run.200: .run.183 200-inetutils ${ONE000}
	./200-inetutils							&& touch .run.200
.run.201: .run.200 201-dhcpcd ${ONE000}
	./201-dhcpcd							&& touch .run.201
.run.202: .run.201 202-openssl ${ONE000}
	./202-openssl							&& touch .run.202
.run.203: .run.202 203-openssh ${ONE000}
	./203-openssh							&& touch .run.203
.run.204: .run.203 204-wget ${ONE000}
	./204-wget							&& touch .run.204
.run.205: .run.204 205-curl ${ONE000}
	./205-curl							&& touch .run.205
.run.206: .run.205 206-etc-files ${ONE000}
	./206-etc-files							&& touch .run.206
.run.207: .run.206 207-net-tools ${ONE000}
	./207-net-tools							&& touch .run.207
#-----------------------------------------------------------------------------
# Strip everything (really executables) to reduce needed space.
.run.400: .run.207 Extra.400.strip ${ONE000}
	./Extra.400.strip						&& touch .run.400
#-----------------------------------------------------------------------------
.PHONY: newkernel
newkernel:
	./083-kernel && ./Run.This.4th && root poweroff
#
# vi: sw=4 ts=8 noexpandtab
#
# End of Makefile
