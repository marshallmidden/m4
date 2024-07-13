# Start of Makefile
#-----------------------------------------------------------------------------
OS := ${shell uname}
#-----------------------------------------------------------------------------
#-- CC = gcc
#-----------------------------------------------------------------------------
CFLAGS   +=-g
#-- CFLAGS   +=-O0
CFLAGS   +=-O3
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
CFLAGS += -W
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra
CFLAGS += -Wno-system-headers
#++? CFLAGS += -Wpedantic
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#-- CFLAGS += -Wabi
#-- CFLAGS += -Wabi-tag
CFLAGS += -Waddress
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# CFLAGS += -Wtraditional
CFLAGS += -Waggregate-return

#?? CFLAGS += -Waggressive-loop-optimizations
#-- CFLAGS += -Waligned-new
#?? CFLAGS += -Walloc-zero
#?? CFLAGS += -Walloca
#?? CFLAGS += -Warray-bounds=1
#++  CFLAGS += -Warray-bounds=2
#-- CFLAGS += -Wassign-intercept
#?? CFLAGS += -Wattribute-alias
CFLAGS += -Wattributes
CFLAGS += -Wbad-function-cast
#?? CFLAGS += -Wbool-compare
#?? CFLAGS += -Wbool-operation
#?? CFLAGS += -Wbuiltin-declaration-mismatch
CFLAGS += -Wbuiltin-macro-redefined
CFLAGS += -Wc++-compat
#-- CFLAGS += -Wc90-c99-compat
#?? CFLAGS += -Wc99-c11-compat
CFLAGS += -Wcast-align
#?? CFLAGS += -Wcast-align=strict
#?? CFLAGS += -Wcast-function-type
CFLAGS += -Wcast-qual
#-- CFLAGS += -Wcatch-value
CFLAGS += -Wchar-subscripts
#?? CFLAGS += -Wchkp
#-- CFLAGS += -Wclass-memaccess
#?? CFLAGS += -Wclobbered
CFLAGS += -Wcomment
CFLAGS += -Wcomments
#-- CFLAGS += -Wconditionally-supported
CFLAGS += -Wconversion
#-- CFLAGS += -Wconversion-null
#?? CFLAGS += -Wcoverage-mismatch
CFLAGS += -Wcpp
#-- CFLAGS += -Wctor-dtor-privacy
CFLAGS += -Wdangling-else
CFLAGS += -Wdate-time
CFLAGS += -Wdeclaration-after-statement
#-- CFLAGS += -Wdelete-incomplete
#-- CFLAGS += -Wdelete-non-virtual-dtor
CFLAGS += -Wdeprecated
CFLAGS += -Wdeprecated-declarations
#?? CFLAGS += -Wdesignated-init
CFLAGS += -Wdisabled-optimization
#?? CFLAGS += -Wdiscarded-array-qualifiers
#?? CFLAGS += -Wdiscarded-qualifiers
CFLAGS += -Wdiv-by-zero
CFLAGS += -Wdouble-promotion
CFLAGS += -Wduplicate-decl-specifier
#?? CFLAGS += -Wduplicated-branches
#?? CFLAGS += -Wduplicated-cond
#-- CFLAGS += -Weffc++
CFLAGS += -Wempty-body
CFLAGS += -Wendif-labels
CFLAGS += -Wenum-compare
CFLAGS += -Werror=implicit-function-declaration
CFLAGS += -Wexpansion-to-defined
#-- CFLAGS += -Wextra-semi
CFLAGS += -Wfloat-conversion
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat
#?? CFLAGS += -Wformat-contains-nul
CFLAGS += -Wformat-extra-args
CFLAGS += -Wformat-nonliteral
#?? CFLAGS += -Wformat-overflow=1
#++  CFLAGS += -Wformat-overflow=2
CFLAGS += -Wformat-security
#?? CFLAGS += -Wformat-signedness
#?? CFLAGS += -Wformat-truncation=1
#++ CFLAGS += -Wformat-truncation=2
CFLAGS += -Wformat-y2k
CFLAGS += -Wformat-zero-length
#?? CFLAGS += -Wformat=1
#++ CFLAGS += -Wformat=2
#?? CFLAGS += -Wframe-address
#?? CFLAGS += -Wfree-nonheap-object
#?? CFLAGS += -Whsa
#?? CFLAGS += -Wif-not-aligned
CFLAGS += -Wignored-attributes
CFLAGS += -Wignored-qualifiers
CFLAGS += -Wimplicit
#?? CFLAGS += -Wimplicit-fallthrough=3
CFLAGS += -Wimplicit-function-declaration
CFLAGS += -Wimplicit-int
CFLAGS += -Wincompatible-pointer-types
#-- CFLAGS += -Winherited-variadic-ctor
CFLAGS += -Winit-self
CFLAGS += -Winline
#?? CFLAGS += -Wlogical-op
CFLAGS += -Wint-conversion
#?? CFLAGS += -Wint-in-bool-context
CFLAGS += -Wint-to-pointer-cast
#?? CFLAGS += -Winvalid-memory-model
#-- CFLAGS += -Winvalid-offsetof
CFLAGS += -Winvalid-pch
#?? CFLAGS += -Wjump-misses-init
#-- CFLAGS += -Wliteral-suffix
CFLAGS += -Wlogical-not-parentheses
#?? CFLAGS += -Wlong-long
#?? CFLAGS += -Wlto-type-mismatch
CFLAGS += -Wmain
#?? CFLAGS += -Wmaybe-uninitialized
#?? CFLAGS += -Wmemset-elt-size
CFLAGS += -Wmemset-transposed-args
#?? CFLAGS += -Wmisleading-indentation
#?? CFLAGS += -Wmissing-attributes
CFLAGS += -Wmissing-braces
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wmissing-noreturn
#?? CFLAGS += -Wmissing-parameter-type
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmultichar
#-- CFLAGS += -Wmultiple-inheritance
#?? CFLAGS += -Wmultistatement-macros
#-- CFLAGS += -Wnamespaces
CFLAGS += -Wnarrowing
CFLAGS += -Wnested-externs
#-- CFLAGS += -Wnoexcept
#-- CFLAGS += -Wnoexcept-type
#-- CFLAGS += -Wnon-template-friend
#-- CFLAGS += -Wnon-virtual-dtor
CFLAGS += -Wnonnull
#?? CFLAGS += -Wnonnull-compare
#?? CFLAGS += -Wnormalized
CFLAGS += -Wnull-dereference
CFLAGS += -Wodr
#-- CFLAGS += -Wold-style-cast
#?? CFLAGS += -Wold-style-declaration
CFLAGS += -Wold-style-definition
#?? CFLAGS += -Wopenmp-simd
CFLAGS += -Woverflow
CFLAGS += -Woverlength-strings
#-- CFLAGS += -Woverloaded-virtual
CFLAGS += -Woverride-init
#?? CFLAGS += -Woverride-init-side-effects
CFLAGS += -Wpacked
#?? CFLAGS += -Wpacked-bitfield-compat
#?? CFLAGS += -Wpacked-not-aligned
#-- CFLAGS += -Wpadded
CFLAGS += -Wparentheses
#-- CFLAGS += -Wpmf-conversions
CFLAGS += -Wpointer-arith
#?? CFLAGS += -Wpointer-compare
CFLAGS += -Wpointer-sign
CFLAGS += -Wpointer-to-int-cast
CFLAGS += -Wpragmas
#-- CFLAGS += -Wproperty-assign-default
#-- CFLAGS += -Wprotocol
CFLAGS += -Wredundant-decls
#-- CFLAGS += -Wregister
#-- CFLAGS += -Wreorder
#?? CFLAGS += -Wrestrict
#?? CFLAGS += -Wreturn-local-addr
CFLAGS += -Wreturn-type
#?? CFLAGS += -Wscalar-storage-order
#-- CFLAGS += -Wselector
CFLAGS += -Wsequence-point
#-- CFLAGS += -Wshadow-ivar
#?? CFLAGS += -Wshadow=compatible-local
#?? CFLAGS += -Wshadow=global
#?? CFLAGS += -Wshadow=local
CFLAGS += -Wsign-compare
CFLAGS += -Wsign-conversion
#-- CFLAGS += -Wsign-promo
#-- CFLAGS += -Wsized-deallocation
CFLAGS += -Wsizeof-array-argument
CFLAGS += -Wsizeof-pointer-div
CFLAGS += -Wsizeof-pointer-memaccess
CFLAGS += -Wstack-protector
#?? CFLAGS += -Wstrict-aliasing=3
#-- CFLAGS += -Wstrict-null-sentinel
#-- CFLAGS += -Wstrict-overflow
CFLAGS += -Wstrict-overflow=2
CFLAGS += -Wstrict-prototypes
#-- CFLAGS += -Wstrict-selector-match
#?? CFLAGS += -Wstringop-overflow=4
#?? CFLAGS += -Wstringop-truncation
#-- CFLAGS += -Wsubobject-linkage
#?? CFLAGS += -Wsuggest-attribute=cold
#?? CFLAGS += -Wsuggest-attribute=const
#?? CFLAGS += -Wsuggest-attribute=format
#?? CFLAGS += -Wsuggest-attribute=malloc
#?? CFLAGS += -Wsuggest-attribute=noreturn
#?? CFLAGS += -Wsuggest-attribute=pure
#?? CFLAGS += -Wsuggest-final-methods
#?? CFLAGS += -Wsuggest-final-types
#-- CFLAGS += -Wsuggest-override
CFLAGS += -Wswitch-bool
CFLAGS += -Wswitch-default
CFLAGS += -Wswitch-enum
#?? CFLAGS += -Wswitch-unreachable
#?? CFLAGS += -Wsync-nand
CFLAGS += -Wtautological-compare
#-- CFLAGS += -Wtemplates
#-- CFLAGS += -Wterminate
#?? CFLAGS += -Wtrampolines
CFLAGS += -Wtrigraphs
CFLAGS += -Wtype-limits
#-- CFLAGS += -Wundeclared-selector
CFLAGS += -Wundef
CFLAGS += -Wuninitialized
CFLAGS += -Wunknown-pragmas
CFLAGS += -Wunreachable-code
#?? CFLAGS += -Wunsuffixed-float-constants
#?? CFLAGS += -Wunused-but-set-parameter
#?? CFLAGS += -Wunused-but-set-variable
#?? CFLAGS += -Wunused-const-variable=1
CFLAGS += -Wunused-function
CFLAGS += -Wunused-label
CFLAGS += -Wunused-local-typedefs
CFLAGS += -Wunused-macros
CFLAGS += -Wunused-parameter
CFLAGS += -Wunused-result
CFLAGS += -Wunused-value
CFLAGS += -Wunused-variable
#-- CFLAGS += -Wuseless-cast
CFLAGS += -Wvarargs
CFLAGS += -Wvariadic-macros
#?? CFLAGS += -Wvector-operation-performance
#-- CFLAGS += -Wvirtual-inheritance
#-- CFLAGS += -Wvirtual-move-assign
CFLAGS += -Wvla
CFLAGS += -Wvolatile-register-var
CFLAGS += -Wwrite-strings
#-- CFLAGS += -Wzero-as-null-pointer-constant

#-----------------------------------------------------------------------------
LD = ${CC}
#-- LDLIBS += -lpthread
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
LDFLAGS += -m64
ifneq (${OS},Darwin)
# Optimize output file
LDFLAGS += -Xlinker -O
# Strip symbols in discarded sections
LDFLAGS += -Xlinker --strip-discarded
# Do not allow unresolved references in object files
LDFLAGS += -Xlinker --no-undefined
# Warn about duplicate common symbols
LDFLAGS += -Xlinker --warn-common
# Warn if global constructors/destructors are seen
LDFLAGS += -Xlinker --warn-constructors
# Warn if the multiple GP values are used
LDFLAGS += -Xlinker --warn-multiple-gp
# Warn only once per undefined symbol
LDFLAGS += -Xlinker --warn-once
# Report unresolved symbols as errors
LDFLAGS += -Xlinker --error-unresolved-symbols
# Treat warnings as errors
LDFLAGS += -Xlinker --fatal-warnings
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Remove unused sections (on some targets)
LDFLAGS += -Xlinker --gc-sections
LDFLAGS += -Xlinker --unresolved-symbols=report-all
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
# Check section addresses for overlaps (default)
LDFLAGS += -Xlinker --check-sections
# Discard temporary local symbols (default)
LDFLAGS += -Xlinker --discard-locals
endif
#-----------------------------------------------------------------------------
ifdef DEBUG
  CFLAGS += -g
  LDFLAGS += -g
endif
#-----------------------------------------------------------------------------
TARGETS = doublelinkedlist
#=============================================================================
.PHONY: help
help:
	@ echo 'Targets:'
	@ echo '  all     - compile all targets.'
	@ echo "  TARGETS: ${TARGETS}"
	@ echo '  clean   - remove executables, objects, and libraries.'
#=============================================================================
.PHONY: all
all: $(TARGETS)
#-----------------------------------------------------------------------------
TARGETS = doublelinkedlist
#-----------------------------------------------------------------------------
%.o: %.c
	@ echo "  CC $@"
	@ ${CC} -c ${CFLAGS} $<
%.c: Makefile
%: %.o
	@ echo "  LD $@"
	@ ${LD} -o $@ ${CFLAGS} ${LDFLAGS} $< ${LDLIBS}
#=============================================================================
#=============================================================================
${TARGETS}:
#-----------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f $(TARGETS) *~ *.o
#-----------------------------------------------------------------------------
.PHONY: tags ctags
tags ctags:
	ctags *.h *.c
#-----------------------------------------------------------------------------
# End of Makefile
