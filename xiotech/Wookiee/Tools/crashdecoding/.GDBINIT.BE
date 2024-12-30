#
set solib-absolute-prefix /env/sles9-32-Wookiee/
core-file  Back.t.core
#
source .gdbinit

directory SRCS/Proc/src
directory SRCS/Proc/inc
directory SRCS/Shared/Inc
directory SRCS/Shared/Src
directory SRCS/Proc/obj_MODELDEBUGPERF
#
#	Print current process name.
echo \n
forkname
#
# 	Print "c" backtrace.
echo \n
bt 18
#
#	Print registers at time of be errtrapping.
echo \n
errtrapb
#
#	print backtrace of process in errtrapb.
echo \n
ebt
#
echo Try: vdiskstatus vdiskstd raidstatus pdiskstatus pdiskstd pdiskfwv pdiskses\n
echo \     beorcs allbequeues DumpDevQueues DevList bemmfree last mrp flight\n
echo \     pcbstate btpcbs memlistscount memlists\n
echo \     p *K_ii\n
#
#-----------------------------------------------------------------------------
# Other things that you might want to do.
#
# g	-- if errtrapb, worthless
# r	-- if errtrapb, worthless
#
# Following only if CT_HISTORY is on.
#	tracelog 90
#	tracelogreg 10
