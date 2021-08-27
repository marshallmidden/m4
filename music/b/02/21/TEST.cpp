#!/bin/bash -ex
A='gcc -E -x c -Wno-invalid-pp-token'
B='-undef'
C='-Wundef'
D='-Werror'
E='-nostdinc'
#  F='-dMI'
F=''
G='-P'
H='-C'
I='-CC'

Z='v2-1 -o v2-1.E'
$A ${Z}1
$A $B ${Z}2
$A $B $C ${Z}3
$A $B $C $D ${Z}4
$A $B $C $D $E ${Z}5
$A $B $C $D $E $F ${Z}6
$A $B $C $D $E $F $G ${Z}7
$A $B $C $D $E $F $G $H ${Z}8
$A $B $C $D $E $F $G $H $I ${Z}9
