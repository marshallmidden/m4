.\" $Id: HOWTO-build-kernels.ms 11794 2006-07-17 20:39:06Z RustadM $
.\"
.RP no
.LH HOWTO Build Linux Kernels for Xiotech Storage Platforms
.TL
HOWTO Build Linux Kernels for Xiotech Storage Platforms
.FS
$Id: HOWTO-build-kernels.ms 11794 2006-07-17 20:39:06Z RustadM $
.FE
.AU
Mark D. Rustad
.AI
Storage Platform Group
Xiotech Corporation
Eden Prairie, Minnesota, USA.
.AB
.PP
This document describes how to build Linux kernels for Xiotech storage
platforms.
.AE
.DA
.nr PI 0n
\fBNOTE!\fP
.PP
Before doing anything, be aware that files within
.I Wookiee/Linux/xio3d
- and some other subdirectories of Wookiee/Linux - must be edited with
8-space hard tabs. You will note that many files in xio3d include a line
like:
.ID
\fC# vi:ts=8 sw=8 noexpandtab\fP
.DE
which sets 8-space hard tabs automatically in vi. This just helps prevent
me from making mistakes. This is not a minor consideration because we need
to be able to exchange patches with other kernel developers. This convention
must be followed in order for this process to work. We need to be able
to diff and patch things reliably. This has meaning beyond simply
avoiding being ridiculed by kernel developers when we submit patches -
although that is also a consideration. There is zero tolerance among Linux
kernel developers for this. This means that anyone who works on the Linux
kernel has to be extremely careful when working in various parts of the
tree, as our normal conventions are at odds with kernel conventions.
.NH 1
Overview
.PP
The build process implemented in
.I Wookiee/Linux/xio3d/Makefile
is capable of building different kernels for different products based
on the value of the \fIMODEL\fP variable as used in the top-level
.I Makefile .
This capability allows different models to have kernels built with
different configurations from different base kernel source versions,
or with different sets of patches, although normally I try to use the
same set of patches with a particular version of kernel source.

The top-level \fIMakefile\fP references files in
.I Wookiee/Linux/xio3d
to get the version of the kernel tarball
located in
.I Wookiee/Linux/kernels .
The same files provide the
version information for the kernel build process. These files have
names in the form
.I KERNEL_VERSION- ${MODEL}.
At present, the names are
.I KERNEL_VERSION-3000
and
.I KERNEL_VERSION-750 .
These files define
.I Makefile
values for variables
.I KERNVER ,
.I KERNREV ,
and
.I KMODEL .
.PP
\fIKERNVER\fP specifies the version of kernel source to use for the build.
This results in the \fIMakefile\fP accessing
.I /usr/src/linux- ${KERNVER}
for the kernel source. Refer to the Kernel Source section for the
procedure to set up the kernel source on a particular build machine. The
.I KERNVER
value is a component of the kernel name as well as the kernel
tarball name. This variable selects the subdirectory that holds
patches, build directories, kernel configurations and everything else
specific to building this particular kernel revision.
.PP
\fIKERNREV\fP specifies the identity of a particular build. This is normally incremented to create unique kernel identities. I usually suffix the number
by a letter to denote a development build. I remove the letter for any build
that actually gets committed to CVS in
.I Wookiee/Linux/kernels .
The value of this variable is normally something like \fI-7\fP. This
value becomes a part of both the kernel name and the kernel tarball name.
If the value is something like
.I -2a ,
it is a development build leading up to a
.I -2
build which may be checked into CVS for others to use in their Wookiee
builds.
.PP
\fIKMODEL\fP specifies the kernel model. For example the 3000 uses the
value \fI3d3000\fP and the 750 uses the value \fI750\fP. This value
is used as part of both the kernel name and the kernel tarball name.
.PP
The settings of these three variables control what is built and what
it will be called.
.NH 1
Kernel Source
.PP
The first thing to do is get the required kernel source set up. The
.I Makefile
in
.I Wookiee/Linux/xio3d
provides a target
.I setup_src
for setting up kernel source for use. This only needs to be
done once per kernel source version for a particular build
machine. Once the source has been set up, multiple developers can
build their own kernels on that machine without interference because
the base kernel source is never modified once it is set up.
.PP
To use
.I setup_src ,
it is necessary to provide the kernel source tarballs in a known location.
I have provided these tarballs in my personal CVS repository,
.I RustadM ,
to make them easy to get to, and yet not have to be in the main CVS
repository used by so many developers. By checking out
.I RustadM
into the same directory where
.I projects
is, the
.I setup_src
process will find the needed kernel source. For example, if
.I projects/Wookiee
is checked out into a directory called 750, then
.I RustadM
should also be checked out in the 750 directory so that both
.I projects
and
.I RustadM
are both in the same directory.
.PP
Once that is done, the source can be set up. Do this by executing the
following command in the
.I Wookiee/Linux/xio3d
directory:
.ID
\fCmake MODEL=750 setup_src\fP
.DE
That will set up the needed kernel source for the 750 product. Remember that
this does not need to be done if the source for the kernel revision
used by that model has been previously set up by any user on that build
system. Performing an unneccessary
.I setup_src
on a kernel revision has the potential to cause trouble if another developer
happens to be performing a build involving that kernel revision on the
same machine at the time that a
.I setup_src
is done, because of the temporary disruption of the kernel source area.
.PP
Note that for some kernel source versions, some patches are applied directly
to the kernel source. This is needed because of a problem in the kernel build
system of some kernel source releases that prevents the normal means of
patching the kernel from working.
Patches applied directly to the kernel source are called prepatches and
should be avoided. The only thing things that should be patched in this way
are things that cannot be patched using the normal patch mechanism.
Look at the script
.I prepatch
and the file
.I prepatches
in the
.I 2.6.15
subdirectory for an example of prepatches.
.NH 1
Patches
.NH 2
Applying Patches
.PP
To apply patches to a kernel, the
.I Makefile
target
.I patch
is provided. The
.I patch
target first removes any existing build directories associated with the
kernel build and then creates and sets up new build directories.
It then invokes the
.I patch
script to apply the needed patches. Note that the
.I patch
script is in the kernel source version specific directory, so it may
use different methods depending on the kernel source version. Note
that the set of patches to apply are listed in a
.I mkdefs- ${KMODEL}
file, thus the set of patches can be
.I MODEL -specific.
As mentioned before, normally the set of patches will be the same regardless
of the
.I MODEL ,
but they need not be. The list of patches to apply wind up in the
.I Makefile
variable
.I PATCHES
which is passed along with the
.I KMODEL
value to the
.I patch
script.
.PP
The process of applying the patches results in the
.I MODEL -specific
objects directory structure holding the updated source files. These files
override the corresponding files in the base kernel source. This is the
key to avoiding modifications to the kernel source tree itself.
.PP
The
.I patch
target is used much like the other
.I Makefile
targets, so
.ID
\fCmake MODEL=750 patch\fP
.DE
will apply patches to the kernel for the specified model.
.NH 2
Generating Patches
.PP
The script
.I mkpatch.sh
is provided in
.I Wookiee/Linux/xio3d
to aid in generating patch files for patching the kernel. The first argument
to this script is the
.I MODEL
for which the patch is intended. The second argument identifies the path
to the file to generate a patch from. This path is only the path within
the kernel source. For example, to generate a patch for
.I drivers/scsi/scsi.c ,
one would use the path
.I drivers/scsi/scsi.c .
The command would look like:
.ID
\fC ./mkpatch.sh 750 drivers/scsi/scsi.c
.DE
This is because
.I mkpatch.sh
uses the values in
.I KERNEL_VERSION- ${MODEL}
to locate the updated source file in the specific objects directory and
to locate the original file in the unmodified kernel source to generate
the patch file, which is deposited into the kernel version directory.
.PP
Note that the patch file still must be included in the value of the
.I PATCHES
.I Makefile
variable to be effective. You may wish to change the name of the patch file,
or combine the content of the file with other patch files in order
to group related changes together.
.NH 2
Developing Without Patches
.PP
Patches are great for reliably making sets of changes to source code such
as the Linux kernel, but they can be inconvenient for active development.
Imagine having to generate or modify a patch for each change you make
in a file that you are actively developing. It means that patches
have to be reapplied,
which means all objects are discarded and everything has to be rebuilt.
Fortunately there is an alternative
that is more convenient during development and debugging stages.
.NH 3
Override
.PP
A directory can be provided in the
.I KERNVER
directory called
.I override .
If such a directory exists, it holds files that will be copied into
corresponding directories in the objects directory for building. Effectively,
these files override or supply entirely new files to the kernel build.
Because the
.I Makefile
is smart enough to only copy them when they change, the kernel can be
built over and over without repatching. Proper dependencies exist, so that
only what needs to be rebuilt will be rebuilt.
.PP
Although the
.I override
mechanism is intended to be used for completely new files, it can also be
used when one is modifying existing kernel source files. Simply copy
the starting kernel source file into a corresponding place in the
.I override
tree and make changes. Once the changes are stable, the
.I mkpatch.sh
script can be used to make a patch corresponding to the changes in the
file. Once the patch has been set up, be sure to remove the file from
the
.I override
directory structure.
.NH 3
xio3d Special Case
.PP
The
.I xio3d
driver is a special case. The files
.I xio3d.c
and
.I xio3d.h
are copied when changed into the objects directory automatically. This
is done to make
.I xio3d
easy to work on, and to keep a single source for it regardless of
kernel version. These files are a part of every Xiotech Linux
kernel and are easy to handle specially. This means that the xio3d
driver source has to be tolerant of various kernel versions, so there is
conditional code within the driver to achieve that.
.NH 1
Configuring a Kernel
.PP
The
.I Makefile
provides some targets for controlling kernel config files.
.NH 2
menuconfig
.PP
The kernel can be configured using the
.I menuconfig
target in the
.I Makefile .
This allows configurations to be saved in files named
.I config- ${KMODEL}
in the
.I KERNVER
directory. This uses a simple curses-based interface for viewing and
changing kernel configurations. This tool is provided by the
Linux kernel, with support added for saving the config files identified
by
.I KMODEL .
An example of the command could be:
.ID
\fCmake MODEL=750 menu_config\fP
.DE
.NH 2
fix_config
.PP
The
.I fix_config
target is used to update an existing kernel configuration file to be
consistent with new or modified kernel sources. This is necessary
whenever a different kernel version is adopted or any of the
.I Kbuild
files in the kernel are patched. This step is more important that many
people think it is. It really is important that kernel config files be
consistent with the kernel source they will be used to build. When they
are not, sometimes bizarre, non-obvious problems show up. This can be
performed with the following command:
.ID
\fCmake MODEL=750 fix_config\fP
.DE
For the curious, this effectively feeds the existing config file to the
kernel configuration tool and takes the default for any new kernel
config options that are then present.
.PP
After doing a
.I fix_config ,
it is usually a good idea to browse the kernel configuration with
.I menuconfig
just to make sure that everything is as expected even though it
is usually fine.
.NH 1
Building the Kernel
.PP
The kernel tarball is built using the
.I Makefile
target
.I tar .
For example, a kernel can be built with the command:
.ID
\fCmake MODEL=750 tar &> bld750.out &\fP
.DE
The command above demonstrates redirecting both stdout and stderr to the
file
.I bld750.out
and puts the command in the background. This allows the developer to
monitor the progress of the build with the
.I less
command (the F command in
.I less
can be used to follow the output as it is generated). This is helpful
because if there are error messages, they will be saved in the file
for subsequent review and correction.
.NH 1
Installing the Kernel
.PP
Upon successful completion of a kernel build, the kernel tarball is left
in the
.I Wookiee/Linux/xio3d
directory with the name
\fIkern-\fP${KERNVER}-${KMODEL}${KERNREV}\fI.tar.gz\fP. In addition,
a file with the name
\fIsyms-\fP${KERNVER}-${KMODEL}${KERNREV}\fI.tar.gz\fP is created
which holds symbolic information for the corresponding kernel. The
.I kern-*.tar.gz
file can be uncompressed on a \fBcontroller\fP
by using the folllowing command:
.ID
\fCtar xzf kern-\fP${KERNVER}-${KMODEL}${KERNREV}\fC.tar.gz -C /\fP
.DE
but be sure \fBnot\fP to do that on your build machine! Using the above
command is useful to try out a kernel, but \fBnot\fP when running the Xiotech
application!
.PP
Normally, a new kernel should be incorporated into a firmware build.
That is done by moving the kernel tarball into the
.I Wookiee/Linux/kernels
directory and building the firmware. Installing that firmware on
a target controller will result in running the new kernel and
the new firmware.
.PP
The
.I Makefile
provides the
.I install
target for installing the kernel in the
.I kernels
directory. An example of this would be:
.ID
\fCmake MODEL=750 install\fP
.DE
.NH 1
More Stuff
.PP
There is more going on in the
.I Makefile
than just building the kernel. The kernel tarball contains a variety
of files other than the results of the kernel build process.
.NH 2
files-${KMODEL}
.PP
The directories named \fIfiles-\fP${KMODEL} contain files that are added
to the kernel tarball. The files are held in a directory structure that
is carried into the tarball.
.NH 2
Other Components
.PP
The
.I Makefile
builds other included components. This includes some components
currently held in
.I Wookiee/Linux
such as memtest86+, scsirastools and so forth. The
.I Makefile
has rules for building these components, but whether they are built
is controlled by
.I Makefile
variables set in the \fImkdefs-\fP${KMODEL} for the particular model.
For example, the 3000 requires minor modifications to scsirastools to
monitor the internal RAID, but the 750 has no internal SCSI drives
that require scsirastools or the needed change for the 3000, so the 750
build does not include anything from scsirastools.
.NH 1
Other Targets
.PP
The
.I Makefile
has other targets as well. Here are more examples of them:
.ID
\fCmake help      # Displays some Makefile help
make doc       # Makes the pdf of this file using groff and ps2pdf
make distclean # Removes all built things and the kernel source!\fP
.DE

.\" # vi:ts=8 sw=8 noexpandtab
