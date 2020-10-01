CFLAGS = -O2

CFLAGS += -static

all:	gzipelf truncate list

gzipelf: gzipelf.o
	$(CC) $(CFLAGS) gzipelf.o -o $@ -Wl,-q

truncate: truncate.o
	$(CC) $(CFLAGS) truncate.o -o $@

clean:
	rm -f *.[oa] gzipelf truncate demo fdtest *.gz
	rm -f list *.core

test:	gzipelf ../../demo/demo
	./gzipelf ../../demo/demo > demo.gz
	chmod a+x demo.gz
	./gzipelf ../../init/init > fdtest.gz
	chmod a+x fdtest.gz
	./gzipelf ../../../uC-src/bin/busybox > busybox.gz
	chmod a+x busybox.gz

	cp demo.gz fdtest.gz busybox.gz ../../../romdisk/local/
