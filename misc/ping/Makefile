# Makefile profile, made by Marshall M. Midden 09/04/91 used with "mkmf".
# modified by hand to be very short.
#
OBJS	= ping.o
PROGRAM	= ping
SRCS	= ping.c
#=============================================================================
CC	= cc
CFLAGS	= -O
CPPFLAGS=
LINKER	= cc
RM	= rm -f
SUFFIXES= .o .c .s .f .p .l .y
#=============================================================================
# Set up default processing (clear them first!).
#
.SUFFIXES:
.SUFFIXES:	$(SUFFIXES)
#=============================================================================
# c compiler
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<
#=============================================================================
# The dependancies start here.
#
all:	$(PROGRAM)
program: $(PROGRAM)
$(PROGRAM): $(OBJS)
	$(LINKER) $(OBJS) -o $(PROGRAM)

clean:
	$(RM) $(OBJS) ${PROGRAM}

install:	${PROGRAM}
#	linstall ${PROGRAM}
	cp ${PROGRAM} /usr/local/bin/${PROGRAM}
	strip /usr/local/bin/${PROGRAM}
	chmod u+s /usr/local/bin/${PROGRAM}

# end of Makefile
