CC = gcc
#-----------------------------------------------------------------------------
CFLAGS = -O2 -fno-strict-aliasing -msoft-float -g3 -ggdb3
#-----------------------------------------------------------------------------
DEFINES = 
#-----------------------------------------------------------------------------
INCLUDES =
#-----------------------------------------------------------------------------
FW_CC_WARNING_FLAGS = -W -Wall -Werror

FW_CC_WARNING_FLAGS += -Wno-system-headers
#? FW_CC_WARNING_FLAGS += -Wtraditional                 ## ouch, this gcc version is really bad at extra warnings with this turned on
#? FW_CC_WARNING_FLAGS += -Waggregate-return            ## This code returns aggregates...                       ARGH!
#? FW_CC_WARNING_FLAGS += -Wbad-function-cast           ## I cannot follow where the ER_LOG, etc. is wrong.      ARGH!
FW_CC_WARNING_FLAGS += -Wcast-align
#? FW_CC_WARNING_FLAGS += -Wcast-qual                   ## some low level acorn macros/routines lose qualifiers. ARGH!
FW_CC_WARNING_FLAGS += -Wchar-subscripts
FW_CC_WARNING_FLAGS += -Wcomments
#? FW_CC_WARNING_FLAGS += -Wconversion                  ## cannot have - volatile UINT8 != UINT8.
FW_CC_WARNING_FLAGS += -Wdeprecated-declarations
FW_CC_WARNING_FLAGS += -Wendif-labels
FW_CC_WARNING_FLAGS += -Wfloat-equal
FW_CC_WARNING_FLAGS += -Wformat
#? FW_CC_WARNING_FLAGS += -Wformat-nonliteral           ## Need special compiling for psm_types.h                ARGH
#? FW_CC_WARNING_FLAGS += -Wformat-security             ## Need special compiling for psm_types.h                ARGH
FW_CC_WARNING_FLAGS += -Wimplicit-int
FW_CC_WARNING_FLAGS += -Wimport
#? FW_CC_WARNING_FLAGS += -Winline                      ## WOW, never saw this fail before in any code.          ARGH!
FW_CC_WARNING_FLAGS += -Wlong-long
FW_CC_WARNING_FLAGS += -Wmissing-braces
#? FW_CC_WARNING_FLAGS += -Wmissing-declarations        ## They have these.
#? FW_CC_WARNING_FLAGS += -Wmissing-format-attribute    ## Need special compiling for psm_types.h                ARGH
FW_CC_WARNING_FLAGS += -Wmissing-noreturn
#? FW_CC_WARNING_FLAGS += -Wmissing-prototypes          ## They don't have function prototypes.                  ARGH!
FW_CC_WARNING_FLAGS += -Wmultichar
FW_CC_WARNING_FLAGS += -Wnested-externs
#-- FW_CC_WARNING_FLAGS += -Wpacked                     ## cannot have this.
#-- FW_CC_WARNING_FLAGS += -Wpadded                     ## cannot have this warning turned on -- too much padding filled in automatically.
FW_CC_WARNING_FLAGS += -Wparentheses
FW_CC_WARNING_FLAGS += -Wpointer-arith
#? FW_CC_WARNING_FLAGS += -Wredundant-decls             ## Lots of redundant declarations.                       ARGH!
FW_CC_WARNING_FLAGS += -Wreturn-type
#? FW_CC_WARNING_FLAGS += -Wshadow                      ## index, etc. I am not allowed to just fix everything.  ARGH!
FW_CC_WARNING_FLAGS += -Wsign-compare
FW_CC_WARNING_FLAGS += -Wstrict-aliasing
#? FW_CC_WARNING_FLAGS += -Wstrict-prototypes           ## They don't have function prototypes.                  ARGH!
FW_CC_WARNING_FLAGS += -Wswitch
#? FW_CC_WARNING_FLAGS += -Wswitch-default              ## missing default on case. i.e. intentional.
FW_CC_WARNING_FLAGS += -Wswitch-enum
FW_CC_WARNING_FLAGS += -Wtrigraphs
#? FW_CC_WARNING_FLAGS += -Wundef                       ## #if BUILD_ENV -- and not defined is an error          ARGH!
FW_CC_WARNING_FLAGS += -Wuninitialized
FW_CC_WARNING_FLAGS += -Wunknown-pragmas
#? FW_CC_WARNING_FLAGS += -Wunreachable-code ## 2008-10-08 -- gives false positives with gcc 3.3.3 and 4.3.1
FW_CC_WARNING_FLAGS += -Wunused
FW_CC_WARNING_FLAGS += -Wunused-function
FW_CC_WARNING_FLAGS += -Wunused-label
FW_CC_WARNING_FLAGS += -Wunused-parameter
FW_CC_WARNING_FLAGS += -Wunused-value
FW_CC_WARNING_FLAGS += -Wunused-variable
FW_CC_WARNING_FLAGS += -Wwrite-strings

# for psm below
FW_CC_WARNING_FLAGS += -Wno-unused-parameter -Wno-unknown-pragmas

#-----------------------------------------------------------------------------
.PHONY: all
all:  constchar
#-----------------------------------------------------------------------------
.PHONY: constchar.o
constchar.o:
	${CC} ${CFLAGS} ${DEFINES} ${INCLUDES} ${FW_CC_WARNING_FLAGS} -c -o constchar.o constchar.c

