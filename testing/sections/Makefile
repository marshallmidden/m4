all:		sections
sections:	sections.o be.ld
	$gcc -static -o $@ -Xlinker -Map -Xlinker $@.map \
		-Xlinker --script -Xlinker be.ld sections.o
	nm -n -l $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort >$@.nm
	objdump -lxrtGdDShp $@ >$@.objdump
	objdump -lxrtGdDShp sections.o >sections.o.objdump

