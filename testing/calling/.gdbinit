break c
run
s 6
set variable $AA=(unsigned int)$esp
print $AA
set variable $AA=$AA&~0xff
print $AA
x/88 $AA
info frame 0
info frame 1
info frame 2
info frame 3
bt full


