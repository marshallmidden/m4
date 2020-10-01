.PHONY: all
#- LDFLAGS += -Bstatic
LDFLAGS += -static -Bstatic -lc_nonshared
all: hello
hello: hello.o
hello.c: Makefile

.PHONY: clean
clean:
	rm -f hello
