#!/bin/csh -x
rm -f TEMPORARY.Z TEMPORARY.Y $2
touch TEMPORARY.Z TEMPORARY.Y
sed \
    -e '1,/^.*gdb.* tracelog/d' \
    \
    -e 's///g' \
    -e 's/^.* Entering routine=\(0x[0-9A-Fa-f]*\), exit_to=.*$/\1/' \
    -e 's/^.* Exiting routine=\(0x[0-9A-Fa-f]*\), exit_to=.*$/\1/' \
    \
    -e '/^tracelog/d' \
    -e '/gdb.*quit/,$d' \
    -e '/(gdb) q/,$d' \
    -e '/Script done on .*$/d' \
    $1 >> TEMPORARY.Z

set AAA=`sort -u TEMPORARY.Z`
echo MODEL=$MODEL
foreach i ($AAA)
  ${DIRECTORY}/nmfind.ccb $i >> TEMPORARY.Y
end

sed -e 's///g' \
    -e 's/^(gdb) tracelog\[/tracelog[/' \
    -e 's/^\(.*\) \( Exiting\|Entering\) routine=XK_TaskSwitch, exit_to=\(.*\)$/==============================================================================\n\1 \2 routine=XK_TaskSwitch, exit_to= \3/' \
    -f TEMPORARY.Y $1 > $2
rm -f TEMPORARY.Z TEMPORARY.Y
