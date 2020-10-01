CFLAGS+= -O3

#-----------------------------------------------------------------------------
all: root

#-----------------------------------------------------------------------------
root: root.c

#-----------------------------------------------------------------------------
install: install.root install.toenv install.runenv
	ls -l /usr/local/bin/root /usr/local/bin/*env* /usr/local/bin/clfs
#-----------------------------------------------------------------------------
.PHONEY: install.root
install.root: root
	rm -f /usr/local/bin/root
	cp root /usr/local/bin/root
	chown root:root /usr/local/bin/root
	chmod 6550 /usr/local/bin/root

	rm -f /usr/local/bin/clfs
	cp root /usr/local/bin/clfs
	chown clfs:clfs /usr/local/bin/clfs
	chmod 6550 /usr/local/bin/clfs

#-----------------------------------------------------------------------------
.PHONEY: install.toenv
install.toenv: toenv
	cp toenv /usr/local/bin/toenv

#-----------------------------------------------------------------------------
.PHONEY: install.runenv
install.runenv: runenv
	cp runenv /usr/local/bin/runenv

#-----------------------------------------------------------------------------
clfs: clfs.c

#-----------------------------------------------------------------------------
.PHONEY: install.clfs
install.clfs: clfs
	rm -f /usr/local/bin/clfs
	cp clfs /usr/local/bin/clfs
	chown clfs:clfs /usr/local/bin/clfs
	chmod 6550 /usr/local/bin/clfs

#-----------------------------------------------------------------------------
clean:
	rm -f root clfs
	rm -rf FixPartition FixPartition.tbz2

#-----------------------------------------------------------------------------
# End of Makefile
