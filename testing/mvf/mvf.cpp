#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int foo(void);

#if 0
On the x86, the following options are allowed:
3dnow
no-3dnow
    Enable/disable the generation of the 3DNow! instructions.
3dnowa
no-3dnowa
    Enable/disable the generation of the enhanced 3DNow! instructions.
abm
no-abm
    Enable/disable the generation of the advanced bit instructions.
adx
no-adx
    Enable/disable the generation of the ADX instructions.
aes
no-aes
    Enable/disable the generation of the AES instructions.
avx
no-avx
    Enable/disable the generation of the AVX instructions.
avx2
no-avx2
    Enable/disable the generation of the AVX2 instructions.
avx5124fmaps
no-avx5124fmaps
    Enable/disable the generation of the AVX5124FMAPS instructions.
avx5124vnniw
no-avx5124vnniw
    Enable/disable the generation of the AVX5124VNNIW instructions.
avx512bitalg
no-avx512bitalg
    Enable/disable the generation of the AVX512BITALG instructions.
avx512bw
no-avx512bw
    Enable/disable the generation of the AVX512BW instructions.
avx512cd
no-avx512cd
    Enable/disable the generation of the AVX512CD instructions.
avx512dq
no-avx512dq
    Enable/disable the generation of the AVX512DQ instructions.
avx512er
no-avx512er
    Enable/disable the generation of the AVX512ER instructions.
avx512f
no-avx512f
    Enable/disable the generation of the AVX512F instructions.
avx512ifma
no-avx512ifma
    Enable/disable the generation of the AVX512IFMA instructions.
avx512pf
no-avx512pf
    Enable/disable the generation of the AVX512PF instructions.
avx512vbmi
no-avx512vbmi
    Enable/disable the generation of the AVX512VBMI instructions.
avx512vbmi2
no-avx512vbmi2
    Enable/disable the generation of the AVX512VBMI2 instructions.
avx512vl
no-avx512vl
    Enable/disable the generation of the AVX512VL instructions.
avx512vnni
no-avx512vnni
    Enable/disable the generation of the AVX512VNNI instructions.
avx512vpopcntdq
no-avx512vpopcntdq
    Enable/disable the generation of the AVX512VPOPCNTDQ instructions.
bmi
no-bmi
    Enable/disable the generation of the BMI instructions.
bmi2
no-bmi2
    Enable/disable the generation of the BMI2 instructions.
cldemote
no-cldemote
    Enable/disable the generation of the CLDEMOTE instructions.
clflushopt
no-clflushopt
    Enable/disable the generation of the CLFLUSHOPT instructions.
clwb
no-clwb
    Enable/disable the generation of the CLWB instructions.
clzero
no-clzero
    Enable/disable the generation of the CLZERO instructions.
crc32
no-crc32
    Enable/disable the generation of the CRC32 instructions.
cx16
no-cx16
    Enable/disable the generation of the CMPXCHG16B instructions.
default
    See Function Multiversioning, where it is used to specify the default function version.
f16c
no-f16c
    Enable/disable the generation of the F16C instructions.
fma
no-fma
    Enable/disable the generation of the FMA instructions.
fma4
no-fma4
    Enable/disable the generation of the FMA4 instructions.
fsgsbase
no-fsgsbase
    Enable/disable the generation of the FSGSBASE instructions.
fxsr
no-fxsr
    Enable/disable the generation of the FXSR instructions.
gfni
no-gfni
    Enable/disable the generation of the GFNI instructions.
hle
no-hle
    Enable/disable the generation of the HLE instruction prefixes.
lwp
no-lwp
    Enable/disable the generation of the LWP instructions.
lzcnt
no-lzcnt
    Enable/disable the generation of the LZCNT instructions.
mmx
no-mmx
    Enable/disable the generation of the MMX instructions.
movbe
no-movbe
    Enable/disable the generation of the MOVBE instructions.
movdir64b
no-movdir64b
    Enable/disable the generation of the MOVDIR64B instructions.
movdiri
no-movdiri
    Enable/disable the generation of the MOVDIRI instructions.
mwait
no-mwait
    Enable/disable the generation of the MWAIT and MONITOR instructions.
mwaitx
no-mwaitx
    Enable/disable the generation of the MWAITX instructions.
pclmul
no-pclmul
    Enable/disable the generation of the PCLMUL instructions.
pconfig
no-pconfig
    Enable/disable the generation of the PCONFIG instructions.
pku
no-pku
    Enable/disable the generation of the PKU instructions.
popcnt
no-popcnt
    Enable/disable the generation of the POPCNT instruction.
prefetchwt1
no-prefetchwt1
    Enable/disable the generation of the PREFETCHWT1 instructions.
prfchw
no-prfchw
    Enable/disable the generation of the PREFETCHW instruction.
ptwrite
no-ptwrite
    Enable/disable the generation of the PTWRITE instructions.
rdpid
no-rdpid
    Enable/disable the generation of the RDPID instructions.
rdrnd
no-rdrnd
    Enable/disable the generation of the RDRND instructions.
rdseed
no-rdseed
    Enable/disable the generation of the RDSEED instructions.
rtm
no-rtm
    Enable/disable the generation of the RTM instructions.
sahf
no-sahf
    Enable/disable the generation of the SAHF instructions.
sgx
no-sgx
    Enable/disable the generation of the SGX instructions.
sha
no-sha
    Enable/disable the generation of the SHA instructions.
shstk
no-shstk
    Enable/disable the shadow stack built-in functions from CET.
sse
no-sse
    Enable/disable the generation of the SSE instructions.
sse2
no-sse2
    Enable/disable the generation of the SSE2 instructions.
sse3
no-sse3
    Enable/disable the generation of the SSE3 instructions.
sse4
no-sse4
    Enable/disable the generation of the SSE4 instructions (both SSE4.1 and SSE4.2).
sse4.1
no-sse4.1
    Enable/disable the generation of the SSE4.1 instructions.
sse4.2
no-sse4.2
    Enable/disable the generation of the SSE4.2 instructions.
sse4a
no-sse4a
    Enable/disable the generation of the SSE4A instructions.
ssse3
no-ssse3
    Enable/disable the generation of the SSSE3 instructions.
tbm
no-tbm
    Enable/disable the generation of the TBM instructions.
vaes
no-vaes
    Enable/disable the generation of the VAES instructions.
vpclmulqdq
no-vpclmulqdq
    Enable/disable the generation of the VPCLMULQDQ instructions.
waitpkg
no-waitpkg
    Enable/disable the generation of the WAITPKG instructions.
wbnoinvd
no-wbnoinvd
    Enable/disable the generation of the WBNOINVD instructions.
xop
no-xop
    Enable/disable the generation of the XOP instructions.
xsave
no-xsave
    Enable/disable the generation of the XSAVE instructions.
xsavec
no-xsavec
    Enable/disable the generation of the XSAVEC instructions.
xsaveopt
no-xsaveopt
    Enable/disable the generation of the XSAVEOPT instructions.
xsaves
no-xsaves
    Enable/disable the generation of the XSAVES instructions.
amx-tile
no-amx-tile
    Enable/disable the generation of the AMX-TILE instructions.
amx-int8
no-amx-int8
    Enable/disable the generation of the AMX-INT8 instructions.
amx-bf16
no-amx-bf16
    Enable/disable the generation of the AMX-BF16 instructions.
uintr
no-uintr
    Enable/disable the generation of the UINTR instructions.
hreset
no-hreset
    Enable/disable the generation of the HRESET instruction.
kl
no-kl
    Enable/disable the generation of the KEYLOCKER instructions.
widekl
no-widekl
    Enable/disable the generation of the WIDEKL instructions.
avxvnni
no-avxvnni
    Enable/disable the generation of the AVXVNNI instructions.
cld
no-cld
    Enable/disable the generation of the CLD before string moves.
fancy-math-387
no-fancy-math-387
    Enable/disable the generation of the sin, cos, and sqrt instructions on the 387 floating-point unit.
ieee-fp
no-ieee-fp
    Enable/disable the generation of floating point that depends on IEEE arithmetic.
inline-all-stringops
no-inline-all-stringops
    Enable/disable inlining of string operations.
inline-stringops-dynamically
no-inline-stringops-dynamically
    Enable/disable the generation of the inline code to do small string operations and calling the library routines for large operations.
align-stringops
no-align-stringops
    Do/do not align destination of inlined string operations.
recip
no-recip
    Enable/disable the generation of RCPSS, RCPPS, RSQRTSS and RSQRTPS instructions followed an additional Newton-Raphson step instead of doing a floating-point division.
general-regs-only
    Generate code which uses only the general registers.
arch=ARCH
    Specify the architecture to generate code for in compiling the function.
tune=TUNE
    Specify the architecture to tune for in compiling the function.
fpmath=FPMATH
    Specify which floating-point unit to use. You must specify the target("fpmath=sse,387") option as target("fpmath=sse+387") because the comma would separate different options.
prefer-vector-width=OPT
    On x86 targets, the prefer-vector-width attribute informs the compiler to use OPT-bit vector width in instructions instead of the default on the selected platform.
    Valid OPT values are:
    none
        No extra limitations applied to GCC other than defined by the selected platform.
    128
        Prefer 128-bit vector width for instructions.
    256
        Prefer 256-bit vector width for instructions.
    512
        Prefer 512-bit vector width for instructions.
    On the x86, the inliner does not inline a function that has different target options than the caller, unless the callee has a subset of the target options of the caller. For example a function declared with target("sse3") can inline a function with target("sse2"), since -msse3 implies -msse2.

#endif	/* 0 */

/* Order:
    MMX
    SSE
    SSE2
    SSE3
    SSSE3
    SSE4.1
    SSE4.2
    POPCNT
    AVX
    AVX2 
 */

__attribute__((target("default")))
int foo(void)
{
    fprintf(stderr, "default\n");
    return(0);
}

__attribute__((target("mmx")))
int foo(void)
{
    fprintf(stderr, "mmx\n");
    return(1);
}

__attribute__((target("avx")))
int foo(void)
{
    fprintf(stderr, "avx\n");
    return(2);
}

__attribute__((target("avx2")))
int foo(void)
{
    fprintf(stderr, "avx2\n");
    return(3);
}
