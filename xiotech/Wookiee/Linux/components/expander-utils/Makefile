CC = gcc
CFLAGS := -O2 -ggdb -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes
#CFLAGS += -Wunreachable-code
LDFLAGS = -Wl,-O2
PROGS := dtemp smpgetphydata smpsetspeed smptest sg_fwupdate
OBJS := $(addsuffix .o,${PROGS}) sg_iosup.o sg_smpsup.o

.PHONY: all clean

all:  ${PROGS}

smpsetspeed: smpsetspeed.o sg_iosup.o sg_smpsup.o

smpgetphydata: smpgetphydata.o sg_iosup.o sg_smpsup.o

dtemp: dtemp.o sg_iosup.o sg_smpsup.o

sg_fwupdate: sg_fwupdate.o sg_iosup.o

smptest: smptest.o sg_iosup.o sg_smpsup.o

${OBJS}: sg_iosup.h sg_smpsup.h

clean:
	rm -f ${OBJS}
	rm -f ${PROGS}
