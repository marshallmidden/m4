#!/bin/sh

d=0
i=""
l=""
s=""
n=""
group=""
mode=""
owner=""
for n in $* ; do
  if [ "x$s" == "x" ]; then
    case $n in
    "-g")
	s=$n;;
    "-m")
	s=$n;;
    "-o")
	s=$n;;
# ignore the following options.
    "-b"|"-c"|"-C"|"-p"|"-s"|"-S")
	s="-s"
	;;
    "-d")
	d=1
	s="-s"
	;;
    "-*")
	echo "gzipelf.sh (install) unrecognized argument $n, ignoring"
	s="-s";;
    esac
    if [ "x$s" == "x" ]; then
      if [ "x$l" != "x" ]; then
	if [ "x$i" != "x" ]; then
	  i="$i $l"
	else
	  i="$l"
	fi
      fi
      l=$n
    fi
    if [ "x$s" == "x-s" ]; then
      s=""
    fi
  else
    case $s in
    "-g")
	group=$n;;
    "-m")
	mode=$n;;
    "-o")
	owner=$n;;
    esac
    s=""
  fi
done
  
# echo TOPDIR=${TOPDIR}
if [ "x" = "x${TOPDIR}" ]; then
  TOPDIR=`pwd`/../ ;
  if [ ! -x ${TOPDIR}/brecis/holes/gzipelf/gzipelf ]; then
    TOPDIR=`pwd`/../../ ;
    if [ ! -x ${TOPDIR}/brecis/holes/gzipelf/gzipelf ]; then
      TOPDIR=`pwd`/../../../ ;
      if [ ! -x ${TOPDIR}/brecis/holes/gzipelf/gzipelf ]; then
	TOPDIR=`pwd`/../../../../ ;
	if [ ! -x ${TOPDIR}/brecis/holes/gzipelf/gzipelf ]; then
	  TOPDIR=`pwd`/../../../../../ ;
	  if [ ! -x ${TOPDIR}/brecis/holes/gzipelf/gzipelf ]; then
	    echo "tried but can't find gzipelf."
	    exit 1;
	  fi;
	fi;
      fi;
    fi;
  fi;
fi;
# echo TOPDIR=${TOPDIR}
if [ x$d == x0 ] ; then
  for n in $i ; do
  #----
    rm -f /tmp/GZIPELF.$$
    t=`dd if=$n bs=1 count=3 skip=1 2>/dev/null`
  # Kludge, if directory does not exist, then trouble happens.
    if [ -d "$l" ]; then
      w="$l/$n"
    else
      if [ -x "$l" ]; then
	rm -f $l
      fi
      w="$l"
    fi
    if [ -x "$w" ]; then
      echo "ALREADY EXISTS, REMOVING $w"
      rm -f $w
    fi
    if [ "x$t" == x'ELF' ]; then
      cp "$n" /tmp/GZIPELF.$$
      mips-strip -x --strip-unneeded -R .comment -R .pdr -R .note -R .mdebug.abi32 -R .toss /tmp/GZIPELF.$$
      ${TOPDIR}/brecis/holes/gzipelf/truncate /tmp/GZIPELF.$$
      ${TOPDIR}/brecis/holes/gzipelf/gzipelf /tmp/GZIPELF.$$ > "$w"
      rm -f /tmp/GZIPELF.$$
    else
      cp "$n" "$w"
    fi
  #----
    if [ "x${mode}" != "x" ]; then
      chmod ${mode} "$w"
    else
      chmod a+x "$w"
    fi
    if [ "x${group}" != "x" ]; then
      chgrp ${group} "$w"
    fi
    if [ "x${owner}" != "x" ]; then
      chown ${owner} "$w"
    fi
  done
else
  for n in $i $l ; do
    oIFS="${IFS}"
    IFS='%'
    set - `echo $n | sed -e 's@/@%@g' -e 's@^%@/@'`
    IFS="${oIFS}"
    d=''
    while [ $# -ne 0 ] ; do
      d="$d${1}"
      shift
      if [ ! -d "$d" ] ;
      then
	mkdir "$d"
	if [ "x${mode}" != "x" ]; then
	  chmod ${mode} "$d"
	fi
	if [ "x${group}" != "x" ]; then
	  chgrp ${group} "$d"
	fi
	if [ "x${owner}" != "x" ]; then
	  chown ${owner} "$d"
	fi
      fi
      d="$d/"
    done
  done
fi
