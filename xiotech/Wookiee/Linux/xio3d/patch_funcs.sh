# $Id: patch_funcs.sh 42506 2008-02-13 21:48:29Z mdr $
# Functions to assist in applying kernel patches when a separate build
# directory is used. The original source directory is always left untouched.
#
# Copyright 2004-2005 Xiotech Corporation. All rights reserved.
#
# Mark D. Rustad

# pcpy copies a file to be patched from the source to the parallel build
# directory, creating an empty file in the build directory if there is no
# original source file.
#
# arg1 is the directory path within the tree
# arg2 is the file name

function pcpy
{
	dir="$1"
	fn="$2"
	test ! -d obj-${MODEL}/${dir} && mkdir -p obj-${MODEL}/${dir}
	if [ ! -f obj-${MODEL}/${dir}/${fn} ]; then
		if [ -f ${KERNSRC}/${dir}/${fn} ]; then
			cp ${KERNSRC}/${dir}/${fn} obj-${MODEL}/${dir}
		else
			touch obj-${MODEL}/${dir}/${fn}
		fi
	fi
}


# objpatch patches a file by placing the patched file into the build (obj)
# tree, thereby overriding the original in the kernel source tree.
#
# arguments are a list of diff files to apply.

function objpatch
{
	for p in $*; do
		grep "^+++" $p | while read a b z; do
			f="${b##*/}"
			pth="${b#*/}"
			pth="${pth%${f}}"
			if [ "x${pth}" = "x" ]; then
				pth=.
			fi
			pcpy "$pth" "$f"
		done
		echo Applying patch "$p"
		/usr/bin/patch -d obj-${MODEL} -p1 <"$p"
		echo Applied patch "$p"
	done
}

