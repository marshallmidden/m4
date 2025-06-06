#!/bin/csh
rm -f TEMPORARY.Z TEMPORARY.Y $2
touch TEMPORARY.Z TEMPORARY.Y
sed \
    -e '1,/^.*gdb.* tracelog/d' \
    \
    -e 's///g' \
    -e 's/^.* Entering routine=\(0x[0-9A-Fa-f]*\), exit_to=.*$/\1/' \
    -e 's/^.* Exiting routine=\(0x[0-9A-Fa-f]*\), exit_to=.*$/\1/' \
    -e 's/^.* callx [gr][0-9][0-9]* .\(0x[0-9A-Fa-f]*\).*$/\1/' \
    -e 's/^.* bx .[0-9][0-9]* .\(0x[0-9A-Fa-f]*\).*$/\1/' \
    \
    -e '/^r0  /d' \
    -e '/^r8  /d' \
    -e '/^g0  /d' \
    -e '/^g8  /d' \
    \
    -e '/^tracelog/d' \
    -e '/gdb.*quit/,$d' \
    -e '/(gdb) q/,$d' \
    -e '/Script done on .*$/d' \
    $1 >> TEMPORARY.Z

set AAA=`sort -u TEMPORARY.Z`
echo MODEL=$MODEL
foreach i ($AAA)
  ${DIRECTORY}/nmfind.fe $i >> TEMPORARY.Y
end

sed -e 's///g' \
    -e 's/^(gdb) tracelog\[/tracelog[/' \
    -e 's/^\(.*\) \(leaving\|ending\) task fork_name \(.*\)$/==============================================================================\n\1 \2 task fork_name \3/' \
    -f TEMPORARY.Y $1 > $2
rm -f TEMPORARY.Z TEMPORARY.Y
