CFLAGS += -m32
all: m

m:	m.o
#	${CC} ${CFLAGS} -Bstatic -o m m.o -lm
	${CC} ${CFLAGS} -o m m.o -lm
#	${CC} -Bstatic -o m m.o

clean:
	rm -f *.o m
	
