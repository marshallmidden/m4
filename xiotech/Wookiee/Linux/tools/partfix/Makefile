#-----------------------------------------------------------------------------
CFLAGS += -Iinclude

CFLAGS += -DHAVE_USLEEP
CFLAGS += -DHAVE_MEMPCPY
CFLAGS += -DHAVE_STRNCHR
CFLAGS += -DHAVE_STRNLEN

CFLAGS += -DPACKAGE_STRING=\"util-linux\ 2.23\"
#-----------------------------------------------------------------------------
C_GCC_FLAGS += -O4
C_GCC_FLAGS += -fno-strict-aliasing
#--2024-12-12 C_GCC_FLAGS += -fno-rename-registers
C_GCC_FLAGS += -W
C_GCC_FLAGS += -Wall
C_GCC_FLAGS += -Werror
# C_GCC_FLAGS += -Waggregate-return
C_GCC_FLAGS += -Warray-bounds
C_GCC_FLAGS += -Wattributes
C_GCC_FLAGS += -Wbad-function-cast
#-- C_GCC_FLAGS += -Wcast-align
C_GCC_FLAGS += -Wcast-qual
C_GCC_FLAGS += -Wchar-subscripts
C_GCC_FLAGS += -Wcomments
C_GCC_FLAGS += -Wextra
# C_GCC_FLAGS += -Wfloat-equal
# C_GCC_FLAGS += -Wformat
# C_GCC_FLAGS += -Wformat-nonliteral
# C_GCC_FLAGS += -Wformat-security
# C_GCC_FLAGS += -Wimplicit-int
# C_GCC_FLAGS += -Wimport
# C_GCC_FLAGS += -Winline
# C_GCC_FLAGS += -Wlong-long
# C_GCC_FLAGS += -Wlogical-op                             # added in gcc 4.3.1
# C_GCC_FLAGS += -Wmissing-braces
# C_GCC_FLAGS += -Wmissing-declarations
# C_GCC_FLAGS += -Wmissing-format-attribute
# C_GCC_FLAGS += -Wmissing-noreturn
# C_GCC_FLAGS += -Wmissing-prototypes
# C_GCC_FLAGS += -Wmultichar
# C_GCC_FLAGS += -Wnested-externs
# C_GCC_FLAGS += -Woverflow                               # added in gcc 4.3.1
# C_GCC_FLAGS += -Wparentheses
# C_GCC_FLAGS += -Wpointer-arith
# C_GCC_FLAGS += -Wredundant-decls
# C_GCC_FLAGS += -Wreturn-type
# C_GCC_FLAGS += -Wshadow
# C_GCC_FLAGS += -Wsign-compare
# C_GCC_FLAGS += -Wstrict-aliasing
# C_GCC_FLAGS += -Wstrict-overflow                        # added in gcc 4.3.1
# C_GCC_FLAGS += -Wstrict-prototypes
# C_GCC_FLAGS += -Wswitch
# C_GCC_FLAGS += -Wswitch-enum
# C_GCC_FLAGS += -Wtrigraphs
# C_GCC_FLAGS += -Wundef
# C_GCC_FLAGS += -Wuninitialized
# C_GCC_FLAGS += -Wunknown-pragmas
# C_GCC_FLAGS += -Wunreachable-code
# C_GCC_FLAGS += -Wunsafe-loop-optimizations                # added in gcc 4.3.1
# C_GCC_FLAGS += -Wunused
# C_GCC_FLAGS += -Wunused-function
# C_GCC_FLAGS += -Wunused-label
# C_GCC_FLAGS += -Wunused-parameter
# C_GCC_FLAGS += -Wunused-value
# C_GCC_FLAGS += -Wunused-variable
# C_GCC_FLAGS += -Wwrite-strings
# C_GCC_FLAGS += -Wvolatile-register-var                  # added in gcc 4.3.1

CFLAGS += ${C_GCC_FLAGS}
#-----------------------------------------------------------------------------
.PHONY: all
all: partfix
#-----------------------------------------------------------------------------
partfix: partfix.o
#-----------------------------------------------------------------------------
partfix.o: Makefile
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f partfix *.o core tags
#-----------------------------------------------------------------------------
.PHONY: test
test: partfix
	cp SDA.save SDA
	./partfix SDA
#-----------------------------------------------------------------------------
.PHONY: ctags tags
tags ctags:
	ctags *.[ch] include/*.h
#-----------------------------------------------------------------------------
.PHONY: dist
dist:
	rm -rf ../FixPartition
	mkdir ../FixPartition ../FixPartition/XIOTech
	cp -p GET.SDA FIX.SDA PUT.SDA DO3 Makefile SDA SDA.save partfix.c ../FixPartition/
	cp -p ../../../CCBE/ccbCL.pl ../FixPartition
	cp -p ../../../CCBE/XIOTech/* ../FixPartition/XIOTech/
	tar jcf ../FixPartition.tbz2 -C .. FixPartition
#-----------------------------------------------------------------------------
