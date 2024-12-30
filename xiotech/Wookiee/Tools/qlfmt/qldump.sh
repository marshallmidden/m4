#!/bin/bash
# $Id: qldump.sh 43325 2008-02-21 20:39:27Z steve_wirtz $
#
# qldump - Produce QLogic dump from core file
#
# Copyright 2007 Xiotech Corporation. All rights reserved.
#
# Mark Rustad, 5/15/2007

usage()
{
	echo "$0 <executable> <corefile> <portno>"
	echo ''
	echo "executable	is the executable corresponding to the dumpfile"
	echo "corefile  is the proc core file containing QLogic dumps"
	echo "portno	is the port number to take a dump of"
}

fail()
{
	echo "$1"
	[[ -z "$2" ]] || usage
	exit 1
}

exec=$1
core=$2
portno=$3

[[ -n "${exec}" ]] || fail "executable not specified" 1
[[ -x "${exec}" ]] || fail "executable ${exec} not executable" 1
[[ -n "${core}" ]] || fail "corefile not specified" 1
[[ -r "${core}" ]] || fail "corefile ${core} not readable" 1
[[ -n "${portno}" ]] || fail "portno not specified" 1

case "${exec}" in
*Back.t) proc=be;;
*Front.t) proc=fe;;
*) fail "Unrecognized executable ${exec}";;
esac

echo "Dumping Qlogic port ${portno} for ${proc}"

gdb -nx -x  gdbqldmpscr ${exec} ${core} <<EOF 2>/dev/null >/dev/null
QLDump ${portno} ${proc}
quit
EOF

status=$?
echo ""
if [[ ${status} -ne 0 ]]; then
	fail "gdb failed with ${status}"
fi

procport=${proc}${portno}
outfile=qldmp${procport}.out
echo "Debug dump for ${proc} adapter ${portno}" > ${outfile}
echo "Time " `date` >> ${outfile}
echo "" >> ${outfile}
if [[ -f qldmpatio${procport}.bin ]]; then
        ./qlfmt qldmp{,req,res,asyqa,atio}${procport}.bin >> ${outfile}
else
        ./qlfmt qldmp{,req,res,asyqa}${procport}.bin >> ${outfile}
fi


# echo "Dump is in ${outfile}"
