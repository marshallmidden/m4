# RedHat 7.4 Kernel Source for LightSpeed

The initial commit of this repository contains patched kernel source
from RedHat's source RPM (SRPM) version 3.10.0-693.11.1.el7 (see
below for the package directory layout)

## Theory of Operation

The RedHat kernel is a highly patched and curated version of an
upstream Linux kernel.org release.  For example, the RedHat 7.4
kernel is based on the very old 3.10 kernel.org kernel but with
thousands of patches.  You can see the patch history in the file
./SPECS/kernel.spec

Each change RedHat makes to the kernel is explicitly referenced in
the ./SPECS/kernel.spec file.  All the original source code and
patches are included in the ./SOURCES/ directory. At Parsec Labs,
kernel modifications must follow this same methodology. The kernel
is built using the RedHat rpmbuild command similar to the way that
any other RPM package is built and packaged.

The purpose of this README is to document the process to follow
when building the Parsec version of the RedHat kernel.

## Making the Parsec Lightspeed Kernel

Step 1 : Clone this repository, make a feature or bugfix branch:

``` shell
% git clone ssh://git@10.0.10.234:7999/pe/rhel-kernel.git
% cd rhel-kernel
% git checkout -b feature/PE-ZZZ-my-kernel-feature
```

Step 2 : Expand and Patch the Kernel Source

Expanding the kernel source tree is done by using the -bp option
to the rpmbuild command.  You can use the following recipe to perform
this action within your development tree:

``` shell
% export TOP=$(pwd)
% rpmbuild -bp --define "_topdir $TOP" \
  --without kabichk   \
  --with baseonly     \
  --without xen       \
  --without fips      \
  --without up        \
  --without perf      \
  --without debuginfo \
  --without debug     \
  SPECS/lightspeed.spec
```

Note the use of the rpmbuild command and that we are using a copy
of the original spec file named lightspeed.spec (not the redhat
kernel.spec file)

Upon performing the above command, you will have the kernel source
code in state last checked in with all the previous Parsec patches
applied.  The source code will be in the following directory
structure:

``` shell
BUILD
└── kernel-3.10.0-693.11.1.el7
    └── linux-3.10.0-693.11.1.el7.lightspeed.x86_64
        ├── arch
        ├── block
        ├── configs
        ├── crypto
        ├── Documentation
        ├── drivers
        ├── firmware
        ├── fs
        ├── include
        ├── init
        ├── ipc
        ├── kernel
        ├── lib
        ├── mm
        ├── net
        ├── samples
        ├── scripts
        ├── security
        ├── sound
        ├── tools
        ├── usr
        └── virt
```

The exact path and directory names may change over time as the
version numbers change.

Step 3 : Hack, develop, craft your changes

In step 2, the last patched state of the kernel source now exists
in the following sub-directory as previously mentioned:

```
├── BUILD
│   └── kernel-3.10.0-693.11.1.el7
│       └── linux-3.10.0-693.11.1.el7.lightspeed.x86_64
```

You can develop within this hierarchy but note that those contents
are *not* under git revision control.  We will be creating a patch
file and a spec file change to commit next.

Step 4. generate a patch

Using the diff, command, generate a single patch file representing
your changes to the source code in step 3.

```
% patch -p .... blah blah blah
```

The patch should be in patch file format, for example:

file: patch-x.2018-01-26_14_42.patch
``` diff
diff --git a/arch/x86/kernel/smpboot.c b/arch/x86/kernel/smpboot.c
index a7f8d9f..63fda5f 100644
--- a/arch/x86/kernel/smpboot.c
+++ b/arch/x86/kernel/smpboot.c
@@ -338,6 +338,7 @@ static void __init smp_init_package_map(struct cpuinfo_x86 *c, unsigned int cpu)
 	 */
 	max_physical_pkg_id = DIV_ROUND_UP(MAX_LOCAL_APIC, ncpus);
```

Step 5 : add your patch file to SOURCES and update the SPEC file

```
cp patch-x.2018-01-26_14_42.patch SOURCES/
```

Make sure you use the next sequential patch id number available in
the 40000 range (40002 as of this writing).  The next available
patch id can be found at the following location:

``` shell
$ grep -A2 Lightspeed SPECS/lightspeed.spec 
# Lightspeed kernel patches.
Patch40000: patch-3.2018-01-18_16-41.patch
Patch40001: patch-3.2018-01-25_16-30.patch
```

So, in the above example, your patch would use Patch40002 as its name.


Step 6 : add and commit your patch file(s) and new spec file

```
% git add SPECS/lightspeed.spec SOURCES/mypatch42000.patch
% git commit -m 'good commit message'
```
## CI and Building of the Kernel

The rhel-kernel repository is monitored by the jenkins server located
below:

  http://10.0.11.140:8080/job/rhel-kernel-current/

If you want to build an RPM version of your code prior to merge,
you can again use the rpmbuild command but with the -bb argument
as follows:

``` shell
% export TOP=$(pwd)
% rpmbuild -bb --define "_topdir $TOP" \
  --without kabichk   \
  --with baseonly     \
  --without xen       \
  --without fips      \
  --without up        \
  --without perf      \
  --without debuginfo \
  --without debug     \
  SPECS/lightspeed.spec
```

If your build and packaging are successful, the RPMS of the kernel
will appear in ./RPMS/x86_64 directory.  This directory is not
tracked by git.

If you want to install your kernel, you can use the rpm command on
your test system as follows:

```shell
# rpm -ivh kernel-3........lightspeed.x86_64.rpm
```

If you are overwriting the same version of the kernel, you can
include the --force option to rpm.

# Appendix

## RedHat Package Details

The origin of this original drop of code came from the official
RedHat 7.4 kernel SRPM file.

```
Name        : kernel
Version     : 3.10.0
Release     : 693.11.1.el7
Architecture: ppc
Install Date: (not installed)
Group       : System Environment/Kernel
Size        : 92820225
License     : GPLv2
Signature   : RSA/SHA256, Mon 30 Oct 2017 06:41:00 AM EDT, Key ID 199e2f91fd431d51
Source RPM  : (none)
Build Date  : Fri 27 Oct 2017 05:40:50 AM EDT
Build Host  : ppc-015.build.eng.bos.redhat.com
Relocations : (not relocatable)
Packager    : Red Hat, Inc. <http://bugzilla.redhat.com/bugzilla>
Vendor      : Red Hat, Inc.
URL         : http://www.kernel.org/
Summary     : The Linux kernel
Description :
The kernel package contains the Linux kernel (vmlinuz), the core of any
Linux operating system.  The kernel handles the basic functions
of the operating system: memory allocation, process allocation, device
input and output, etc.
```

# ----------------------------------------------------------------------------

## Install new kernel v5.10.1

Kernel v5.10.1 (probably anything newer than v5.5 needs a newer gcc
(version 10.2.0 is good) and new rpmbuild, which needs more packages.
Default is for gcc 4.8.5 with redhat 7.5, and 8.x.y for centos 8.1.

The following documents how to get new gmp, mpfr, mpc, binutils before
putting in the new gcc -- which uses those compiled with old gcc for the
first steps of installing the new gcc -- which itself is a three step process.

```
    cd new-gcc-10.2.0+friends
    ./00-new-a-gcc+tools
    cd ..
```

Now to use the new tools:

```
    prepath () {
	case ":$PATH:" in
	  *":$1:"*) :;;         # in the middle
	  "$1:"*) :;;           # at the end
	  *":$1") :;;           # at the beginning
	  "$1") :;;             # if only one
	  *) PATH=$1:$PATH;;
	esac
    }
    prepath ~/new-a/bin
    which gcc                   # should be ~/root/new-a/bin/gcc
    gcc --version               # gcc (GCC) 10.2.0
    make kb                     # creates rpm files
    make all                    # installs into /boot
    vi +20 /boot/efi/EFI/redhat/grub.cfg     # change 1 to 0
    reboot
```

