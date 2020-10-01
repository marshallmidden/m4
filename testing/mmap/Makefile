
all: tail

tail: tail.o

tail.o: tail.c

clean:
	rm -f tail tail.o

test:	FILE tail
	./tail 4 FILE
