CFLAGS += -m32
CFLAGS += -I /home/m4/tmp/HEAD/Proc/obj_7000Debug/
CFLAGS += -I /home/m4/tmp/HEAD/Sharec/Inc/
#-----------------------------------------------------------------------------
all: m

#-----------------------------------------------------------------------------
m:	m.o
#	${CC} ${CFLAGS} -Bstatic -o m m.o -lm
	${CC} ${CFLAGS} -o m m.o -lm
#	${CC} -Bstatic -o m m.o

#-----------------------------------------------------------------------------
clean:
	rm -f *.o m
	
#-----------------------------------------------------------------------------
