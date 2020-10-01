#-----------------------------------------------------------------------------
CFLAGS+= -O3
OWNER=root
# GROUP=root    # RedHat 7.5
# GROUP=wheel
GROUP=admin	# mac
#-----------------------------------------------------------------------------
all: root
#-----------------------------------------------------------------------------
root: root.c
#-----------------------------------------------------------------------------
install: root
	rm -f /usr/local/bin/root
	cp root /usr/local/bin/root
	chown ${OWNER}:${GROUP} /usr/local/bin/root
	chmod 6550 /usr/local/bin/root

	ls -l /usr/local/bin/root
#-----------------------------------------------------------------------------
# vi: sw=4 ts=8 expandtab
# End of Makefile
