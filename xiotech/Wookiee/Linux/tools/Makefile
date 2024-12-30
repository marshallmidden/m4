CFLAGS+= -O3

all: root

root: root.c

install: root toenv runenv
	rm -f /usr/local/bin/root
	cp root /usr/local/bin/root
	chown root:root /usr/local/bin/root
	chmod 6550 /usr/local/bin/root

	cp toenv /usr/local/bin/toenv
	cp runenv /usr/local/bin/runenv

	ls -l /usr/local/bin/root /usr/local/bin/*env*

clean:
	rm -f root
	rm -rf FixPartition FixPartition.tbz2

# End of Makefile
