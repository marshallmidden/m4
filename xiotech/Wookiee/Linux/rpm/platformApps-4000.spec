# spec file for package 'platformApps-4000'
#
# Copyright (c) 2006-2009 Xiotech Corporation. All rights reserved.

Name:      platformApps-4000
Version:   XXXX
Release:   YYYY

Summary:   Xiotech 3D Platform code release package
#Requires:  We may have to use this eventually 
License:   Proprietary
Group:     Applications/System
Source0:   platformApps-4000.tar.gz
BuildRoot: /tmp/%name-%version-%release-buildroot
AutoReqProv: no

%define	xiodir	/opt/xiotech
%define fwver	%name-%version-%release
%define bindir 	%xiodir/release/%fwver

%description
'platformApps-4000' is a Xiotech 3D code release package targeted for Yeti.

# prep the build - create build dir, untar etc.
%prep
%setup

# nothing to build, already done.
%build

# install the files.
%install
install -d            %buildroot/%bindir
install ccbrun        %buildroot/%bindir
install pam           %buildroot/%bindir
install syssrv        %buildroot/%bindir
install Back.t        %buildroot/%bindir
install Front.t       %buildroot/%bindir
install 2400.bin      %buildroot/%bindir
install 2400mid.bin   %buildroot/%bindir
install 2500.bin      %buildroot/%bindir
install 2500mid.bin   %buildroot/%bindir
install chgnetcfg     %buildroot/%bindir
install kernel.tgz    %buildroot/%bindir
install kernelver     %buildroot/%bindir
install bvm           %buildroot/%bindir
install xioFidScript  %buildroot/%bindir
install iscsid        %buildroot/%bindir
install gzshm         %buildroot/%bindir
install shmdump       %buildroot/%bindir
install MD5sums       %buildroot/%bindir

# ^ Basically, the sections above this line are RPM BUILD commands,
#----------------------------------------------------------------------
# v and, below this line are RPM INSTALL commands.

# The files listed here are packaged into the RPM and are placed here
# by the RPM.
%files
%attr(755, root, root) %dir %bindir
%attr(755, root, root) %bindir/ccbrun
%attr(755, root, root) %bindir/pam
%attr(755, root, root) %bindir/syssrv
%attr(755, root, root) %bindir/Back.t
%attr(755, root, root) %bindir/Front.t  
%attr(644, root, root) %bindir/2400.bin
%attr(644, root, root) %bindir/2400mid.bin
%attr(644, root, root) %bindir/2500.bin
%attr(644, root, root) %bindir/2500mid.bin
%attr(644, root, root) %bindir/chgnetcfg
%attr(644, root, root) %bindir/kernel.tgz
%attr(644, root, root) %bindir/kernelver
%attr(755, root, root) %bindir/bvm
%attr(755, root, root) %bindir/xioFidScript
%attr(755, root, root) %bindir/iscsid
%attr(755, root, root) %bindir/gzshm
%attr(755, root, root) %bindir/shmdump
%attr(644, root, root) %bindir/MD5sums

%post
mem=`dmidecode | grep -A 8 'DMI type 17,' | grep Size: | grep -v No |
	awk '{ mem += $2 } END { print mem }'`
if [ ${mem} -lt 2048 ]; then
	rm -rf %bindir
	exit 99
fi
unset mem
install -m 755 -o xiotech -g xiotech %bindir/bvm %xiodir
install -m 755 -o 0 -g 0 -d /mnt/floppy
rm -f %xiodir/latest_platform
ln -sf release/%fwver %xiodir/latest_platform
rm -f %xiodir/platform_boots
echo 0 > %xiodir/platform_boots
sync
sync

echo "*** Installed %fwver Successfully ***"
true    # make sure we return "success" from this section

%postun
rm -rf %bindir

%changelog
