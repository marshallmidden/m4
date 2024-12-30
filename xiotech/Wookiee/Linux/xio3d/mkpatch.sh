#!/bin/bash -e
# $Id: mkpatch.sh 51161 2008-05-08 22:57:25Z mdr $
# Make a patch file
#   First parameter is model.
#   Second parameter is path to file.

MODEL="$1"
FILE="$2"
PFILE=${FILE##*/}.patch
vercmd=($(grep KERNVER KERNEL_VERSION-${MODEL} | tr "\?=" "  "))
verbasecmd=($(grep KERNBASEVER KERNEL_VERSION-${MODEL} | tr "\?=" "  "))
kmodcmd=($(grep KMODEL KERNEL_VERSION-${MODEL} | tr "\?=" "  "))
KERNVER=${vercmd[1]}
KERNBASEVER=${verbasecmd[1]:-${KERNVER}}
KMODEL=${kmodcmd[1]}
if [ "${KERNVER}" = "" -o "${KERNBASEVER}" = "" -o "${FILE}" = "" \
	-o "${PFILE}" = "" ]
then
	echo "Wrong, KERNVER=${KERNVER}, FILE=${FILE}, PFILE=${PFILE}"
	exit 1
fi
cd ${KERNBASEVER}
rm -f a
ln -sf /usr/src/linux-${KERNVER} a
diff -Nup {a,obj-${MODEL}}/${FILE} > ${PFILE} &&
	{ rm -f a; echo No differences!; exit 1; }
rm -f a
echo Patch file ${KERNBASEVER}/${PFILE} written

# vi:sw=8 ts=8 noexpandtab
