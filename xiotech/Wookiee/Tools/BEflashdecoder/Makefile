#=============================================================================
export IN ?= shared_memory_NVSRAM_BE
export OUT ?= shared_memory_NVSRAM_BE.new
#=============================================================================
CC = gcc
C_INCLUDES += -I.
C_INCLUDES += -I/home/marshall_midden/b/Proc/inc
C_TAGS += /home/marshall_midden/b/Proc/inc/*.h
C_INCLUDES += -I/home/marshall_midden/b/Shared/Inc
C_TAGS += /home/marshall_midden/b/Shared/Inc/*.h
C_INCLUDES += -I/home/marshall_midden/b/Proc/obj_3000Debug
C_TAGS += /home/marshall_midden/b/Proc/obj_3000Debug/*.h

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C_GCC_FLAGS += -g -fpack-struct
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# C_GCC_FLAGS += -D_FORTIFY_SOURCE=2
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#-- C_GCC_FLAGS += -O3
C_GCC_FLAGS += -O0
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
C_GCC_FLAGS += -fno-strict-aliasing
#--2024-12-12 C_GCC_FLAGS += -fno-rename-registers
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Lots of warning messages.
C_GCC_FLAGS += -W
C_GCC_FLAGS += -Wall
C_GCC_FLAGS += -Werror
C_GCC_FLAGS += -Wno-system-headers
# C_GCC_FLAGS += -Wtraditional
C_GCC_FLAGS += -Waggregate-return
# C_GCC_FLAGS += -Warray-bounds							# added in gcc 4.3.1
# C_GCC_FLAGS += -Wattributes							# added in gcc 4.3.1
C_GCC_FLAGS += -Wbad-function-cast
#-- C_GCC_FLAGS += -Wcast-align
C_GCC_FLAGS += -Wcast-qual
C_GCC_FLAGS += -Wchar-subscripts
C_GCC_FLAGS += -Wcomments
C_GCC_FLAGS += -Wdeprecated-declarations
C_GCC_FLAGS += -Wendif-labels
# C_GCC_FLAGS += -Wextra							# added in gcc 4.3.1
C_GCC_FLAGS += -Wfloat-equal
C_GCC_FLAGS += -Wformat
C_GCC_FLAGS += -Wformat-nonliteral
C_GCC_FLAGS += -Wformat-security
C_GCC_FLAGS += -Wimplicit-int
C_GCC_FLAGS += -Wimport
C_GCC_FLAGS += -Winline
C_GCC_FLAGS += -Wlong-long
# C_GCC_FLAGS += -Wlogical-op							# added in gcc 4.3.1
C_GCC_FLAGS += -Wmissing-braces
C_GCC_FLAGS += -Wmissing-declarations
C_GCC_FLAGS += -Wmissing-format-attribute
C_GCC_FLAGS += -Wmissing-noreturn
C_GCC_FLAGS += -Wmissing-prototypes
C_GCC_FLAGS += -Wmultichar
C_GCC_FLAGS += -Wnested-externs
# C_GCC_FLAGS += -Woverflow							# added in gcc 4.3.1
C_GCC_FLAGS += -Wparentheses
C_GCC_FLAGS += -Wpointer-arith
C_GCC_FLAGS += -Wredundant-decls
C_GCC_FLAGS += -Wreturn-type
C_GCC_FLAGS += -Wshadow
C_GCC_FLAGS += -Wsign-compare
C_GCC_FLAGS += -Wstrict-aliasing
# C_GCC_FLAGS += -Wstrict-overflow						# added in gcc 4.3.1
C_GCC_FLAGS += -Wstrict-prototypes
C_GCC_FLAGS += -Wswitch
C_GCC_FLAGS += -Wswitch-enum
C_GCC_FLAGS += -Wtrigraphs
C_GCC_FLAGS += -Wundef
#-- C_GCC_FLAGS += -Wuninitialized		# Needs -O > 0
C_GCC_FLAGS += -Wunknown-pragmas
#-- C_GCC_FLAGS += -Wunreachable-code	## 2008-10-08 -- gives false positives with gcc 3.3.3 and 4.3.1.
C_GCC_FLAGS += -Wunused
C_GCC_FLAGS += -Wunused-function
C_GCC_FLAGS += -Wunused-label
C_GCC_FLAGS += -Wunused-parameter
C_GCC_FLAGS += -Wunused-value
C_GCC_FLAGS += -Wunused-variable
C_GCC_FLAGS += -Wwrite-strings
# C_GCC_FLAGS += -Wvolatile-register-var					# added in gcc 4.3.1

LD = gcc
LD += -static
LD += -Xlinker -E
LD += -Xlinker -d
LD += -Xlinker --no-strip-discarded
LD += -Xlinker --discard-none
LD += -Xlinker --no-undefined
LD += -Xlinker --warn-common
LD += -Xlinker --warn-constructors
LD += -Xlinker --warn-multiple-gp
LD += -Xlinker --warn-once
LD += -Xlinker --error-unresolved-symbols
LD += -Xlinker -z -Xlinker defs
#-----
C_CODEFLAGS += -Wno-cast-qual
# ============================================================================
CFLAGS += ${C_GCC_FLAGS} ${C_INCLUDES} ${C_CODEFLAGS}
CFLAGS += -DMODEL=3000 -DMODEL_3000=1
# ============================================================================
# Following is for Link Layer located in different directory.
# VPATH = ${WHERE}/../../Shared/Src
# ============================================================================
.PHONY: help
help:
	@echo ''
	@echo 'make Targets:'
	@echo '  clean   - remove generated files.'
	@echo '  all     - comple programs.'
	@echo '  tags    - create vi tags, emacs TAGS files.'
	@echo '  txt     - Create file "txt" from "$${IN}"  [$${IN}='${IN}']'
	@echo '  replace - Create file "$${OUT}" from "txt" [$${OUT}='${OUT}']'
	@echo ''
	@echo 'Example:'
	@echo '  make IN=customer_BE txt  - Create file "txt" from customer_BE'
	@echo '  vi txt'
	@echo '  make OUT=new_BE replace  - Create file named new_BE from "txt"'
	@echo ''
	@echo 'DEFAULTS: make txt ; vi txt ; make replace'
	@echo ''
	@echo 'DEBUGGING Targets:'
	@echo '  gdb.BEascii - Debug a core file from the binary to ascii program.'
	@echo '  gdb.BEbinary - Debug a core file from the ascii to binary program.'
	@echo ''
# ============================================================================
.PHONY: all
all: BEascii BEbinary
#-----------------------------------------------------------------------------
BEascii: BEascii.o
	${LD} -o $@ $<
BEascii.o: BEbinary.c
	${CC} -o BEascii.o ${CFLAGS} -DBE_ASCII -c BEbinary.c
#-----------------------------------------------------------------------------
BEbinary: BEbinary.o FromProcsrc.o
	${LD} -o $@ BEbinary.o FromProcsrc.o
BEbinary.o: BEbinary.c
	${CC} -o $@ ${CFLAGS} -c $<
#-----------------------------------------------------------------------------
FromProcsrc.o: FromProcsrc.c
	${CC} -o $@ ${CFLAGS} -c $<
# ============================================================================
.PHONY: clean
clean:
	rm -f BEascii BEbinary *.o core* tags TAGS typescript
	rm -f txt b.hex s.hex BE
	rm -f ${OUT}
# ----------------------------------------------------------------------------
.PHONY: tag tags ctags
tag tags ctags:
	ctags *.[hc] ${C_TAGS}
#	etags *.[hc] 
# ----------------------------------------------------------------------------
s.hex:
	dd if=${IN} bs=512 skip=64 | /usr/bin/hexdump -f ${HOME}/bin/HEXDUMP > s.hex
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
b.hex:	BEbinary
	./BEbinary < txt | /usr/bin/hexdump -f ${HOME}/bin/HEXDUMP > b.hex
#-----------------------------------------------------------------------------
.PHONY: txt
txt:	BEascii
	@# Create txt file from ${IN} that can be edited.
	dd if=${IN} bs=512 skip=64 | ./BEascii > txt
#-----------------------------------------------------------------------------
BE:	BEascii
	@# Create txt file from ${IN} that can be edited.
	dd if=${IN} bs=512 skip=64 of=BE
#-----------------------------------------------------------------------------
.PHONY: bin
bin:	BEbinary
	./BEbinary < txt >bin
#-----------------------------------------------------------------------------
.PHONY: replace
${OUT} replace: BEbinary
	@# Make everything else in file be the same.
	cp ${IN} ${OUT}
	@# Replace the NVRAM part 2 section.
	./BEbinary < txt | dd conv=notrunc of=${OUT} bs=512 seek=64
# ============================================================================
.PHONY: gdb.BEascii
gdb.BEascii:
	gdb BEascii core
# ============================================================================
.PHONY: gdb.BEbinary
gdb.BEbinary:
	gdb BEbinary core
# ============================================================================
# vi:sw=8 ts=8 noexpandtab
# End of Makefile
