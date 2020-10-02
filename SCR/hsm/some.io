#!/bin/bash -e

# Set M to argument passed in.
M=$1

echo '' | tee -a some.io.log
/bin/date '+%Y-%m-%d_%H-%M-%S' | tee -a some.io.log

if [ "$1"x = "x" ]; then
    echo 'Must have an argument of directory (mount) to do some I/O on.' | tee -a some.io.log
    echo 'example: ./some.io /mnt' | tee -a some.io.log
    exit 1
fi

#-----------------------------------------------------------------------------
let i=1
# Do a first time du command to get a reference (it should not change!).
A=`du -x -s ${M}`
echo "$A" | tee -a some.io.log

# Loop forever -- until control-C is pressed.
for ((;;)) do
    B=`du -x -s ${M}`
    if [ "$A" != "$B" ]; then
	echo "" | tee -a some.io.log
	echo "Source directory $M changed size -- from $A to $B" | tee -a some.io.log
	exit 1
    fi
    echo -n '.' | tee -a some.io.log
    sleep 1
    let "j=$i % 80" || true
    if [ $j -eq 0 ]; then
	echo " $i" | tee -a some.io.log
    fi
    let "i=$i + 1"
done
#-----------------------------------------------------------------------------
