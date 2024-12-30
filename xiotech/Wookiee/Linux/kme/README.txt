SUMMARY
=======

This directory contains the Kernel Memory Editor (kme) program and
associated utility programs.

SALES PITCH
===========

KME allows you to view (and edit) the memory of a running program
with a spreadsheet-like interface.  You can view a single area of
memory, or a number of areas at the same time.  Each area is defined
by a starting address expression, and a variable display string.

With its associated tools, KME can read a program binary file
(created with gcc -g) to extract symbol addresses and structure
definitions.  With this information, you can enter symbolic
addresses and display their contents as C structures.

KME was originally designed to view kernel memory, but has been
enhanced to view user-mode programs, core files, other ELF
object files, peripheral card memory, and memory accessed through
a CPU emulator.

It is possible to change the contents of memory in a running
program (to adjust tuning constants for example, zero performance
counters).

A recent modification to KME allows KME to catch a signal in a
user program, stop the user program, refresh and pause the
display, and then restart the user program.

The ELF2KME program accompanies KME in this distribution.  This
program reads an ELF program binary to create a kme_defs file
containing some or all of the structure definitions used in the
program.


INSTALLATION
============

	% su
	% sh install.sh

DOCUMENTATION
=============

	% man kme		# The kme manual
	% perldoc elf2kme	# The elf2kme manual

EXAMPLE USE
===========

To view the Wookiee Proc module Back.t:

	% su					 # You must be root
	% cd /opt/xiotech/apps			 # Binary directory
	% elf2kme -t -f 2 Back.t >kme_defs	 # Create kme_defs (once only)
	% ps -ef | grep Back.t			 # Get the Back.t PID
	% kme -P <pid-from-ps-command> -n Back.t # Run KME

To view the Wookie Proc module after a crash:

	% su					 # You must be root
	% cd /opt/xiotech/apps			 # Binary directory
	% elf2kme -t -f 2 Back.t >kme_defs	 # Create kme_defs (once only)
	% kme -n Back.t -C /var/log/dump/Back.t.core # Run KME

WARNING
=======

KME is a complex program where every keystroke does something.
It was modelled after the "vi" editor and like the "vi" editor
it requires you RTFM (read the full manual) before you become
productive with the program.

SOURCE CODE
===========

The KME programs are licensed GPL, and all source code and
source code history remain available at:

	http://sourceforge.net/projects/kme

As of this writing, the latest released version of the program
is 2.1.0, obsolete by a couple days from the version included
here.  The latest (bleeding edge) version remains available
to anyone with a login on sourceforge.net.  Sourceforge logins
are free, and offer ulimited permanent email forwarding.
