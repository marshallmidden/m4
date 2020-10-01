CFLAGS += -g
CFLAGS += -save-temps

a.out: a.o b.o c.o m.o
	${CC} ${CFLAGS} -o $@ a.o b.o c.o m.o

clean:
	rm -f a.out *.o *.i *.s
