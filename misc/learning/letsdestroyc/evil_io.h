#ifndef EVIL_NO_IO
  // The IO Module.
  // Included by default. To pretend C is high-level.


  // User wants IO, give the all the IO.
  #include <stdio.h>

  // Yes, Generics. (aka type-switch). It's C11 only,
  // but who cares.
  // stdint identifiers (inttypes.h) should be catered for by the below.
  // Original display_format macro by Robert Gamble, (c) 2012
  // used with permission.
  // Expanded upon to incorporate const, volatile and const volatile types,
  // as they don't get selected for. (static does for obvious reasons).

  // Whilst volatile types can change between accesses, technically using a
  // _Generic _shouldn't_ access it, but compile to the right choice.

  #define display_format(x) _Generic((x), \
    char: "%c", \
    signed char: "%hhd", \
    unsigned char: "%hhu", \
    signed short: "%hd", \
    unsigned short: "%hu", \
    signed int: "%d", \
    unsigned int: "%u", \
    long int: "%ld", \
    unsigned long int: "%lu", \
    long long int: "%lld", \
    unsigned long long int: "%llu", \
    float: "%f", \
    double: "%f", \
    long double: "%Lf", \
    char *: "%s", \
    void *: "%p", \
    volatile char: "%c", \
    volatile signed char: "%hhd", \
    volatile unsigned char: "%hhu", \
    volatile signed short: "%hd", \
    volatile unsigned short: "%hu", \
    volatile signed int: "%d", \
    volatile unsigned int: "%u", \
    volatile long int: "%ld", \
    volatile unsigned long int: "%lu", \
    volatile long long int: "%lld", \
    volatile unsigned long long int: "%llu", \
    volatile float: "%f", \
    volatile double: "%f", \
    volatile long double: "%Lf", \
    volatile char *: "%s", \
    volatile void *: "%p", \
    const char: "%c", \
    const signed char: "%hhd", \
    const unsigned char: "%hhu", \
    const signed short: "%hd", \
    const unsigned short: "%hu", \
    const signed int: "%d", \
    const unsigned int: "%u", \
    const long int: "%ld", \
    const unsigned long int: "%lu", \
    const long long int: "%lld", \
    const unsigned long long int: "%llu", \
    const float: "%f", \
    const double: "%f", \
    const long double: "%Lf", \
    const char *: "%s", \
    const void *: "%p", \
    const volatile char: "%c", \
    const volatile signed char: "%hhd", \
    const volatile unsigned char: "%hhu", \
    const volatile signed short: "%hd", \
    const volatile unsigned short: "%hu", \
    const volatile signed int: "%d", \
    const volatile unsigned int: "%u", \
    const volatile long int: "%ld", \
    const volatile unsigned long int: "%lu", \
    const volatile long long int: "%lld", \
    const volatile unsigned long long int: "%llu", \
    const volatile float: "%f", \
    const volatile double: "%f", \
    const volatile long double: "%Lf", \
    const volatile char *: "%s", \
    const volatile void *: "%p", \
    default: "%d")

  // The main printing function.
  #define display(x) printf(display_format(x), x)
  #define displayf(f, x) fprintf(f, display_format(x), x)

  // Windows has a different line ending.
  #if defined(_WIN32) || defined(__WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64) || defined(WIN64) || defined(__WIN64__) || defined(__WINNT) || defined(__WINNT__) || defined(WINNT)
    #define displayln(x) printf(display_format(x), x); printf("%s", "\r\n")
    #define displayfln(f, x) fprintf(f, display_format(x), x); printf("%s", "\r\n")
  #else
    #define displayln(x) printf(display_format(x), x); printf("%c", '\n')
    #define displayfln(f, x) fprintf(f, display_format(x), x); printf("%c", '\n')
  #endif

  // Basically a _Generic.
  #define repr_type(x) _Generic((0,x), \
    char: "char", \
    signed char: "signed char", \
    unsigned char: "unsigned char", \
    signed short: "signed short", \
    unsigned short: "unsigned short", \
    signed int: "signed int", \
    unsigned int: "unsigned int", \
    long int: "long int", \
    unsigned long int: "unsigned long int", \
    long long int: "long long int", \
    unsigned long long int: "unsigned long long int", \
    float: "float", \
    double: "double", \
    long double: "long double", \
    char *: "char pointer", \
    void *: "void pointer", \
    volatile char: "volatile char", \
    volatile signed char: "volatile signed char", \
    volatile unsigned char: "volatile unsigned char", \
    volatile signed short: "volatile signed short", \
    volatile unsigned short: "volatile unsigned short", \
    volatile signed int: "volatile signed int", \
    volatile unsigned int: "volatile unsigned int", \
    volatile long int: "volatile long int", \
    volatile unsigned long int: "volatile unsigned long int", \
    volatile long long int: "volatile long long int", \
    volatile unsigned long long int: "volatile unsigned long long int", \
    volatile float: "volatile float", \
    volatile double: "volatile double", \
    volatile long double: "volatile long double", \
    volatile char *: "volatile char pointer", \
    volatile void *: "volatile void pointer", \
    const char: "const char", \
    const signed char: "const signed char", \
    const unsigned char: "const unsigned char", \
    const signed short: "const signed short", \
    const unsigned short: "const unsigned short", \
    const signed int: "const signed int", \
    const unsigned int: "const unsigned int", \
    const long int: "const long int", \
    const unsigned long int: "const unsigned long int", \
    const long long int: "const long long int", \
    const unsigned long long int: "const unsigned long long int", \
    const float: "const float", \
    const double: "const double", \
    const long double: "const long double", \
    const char *: "const char pointer", \
    const void *: "const void pointer", \
    const volatile char: "const volatile char", \
    const volatile signed char: "const volatile signed char", \
    const volatile unsigned char: "const volatile unsigned char", \
    const volatile signed short: "const volatile signed short", \
    const volatile unsigned short: "const volatile unsigned short", \
    const volatile signed int: "const volatile signed int", \
    const volatile unsigned int: "const volatile unsigned int", \
    const volatile long int: "const volatile long int", \
    const volatile unsigned long int: "const volatile unsigned long int", \
    const volatile long long int: "const volatile long long int", \
    const volatile unsigned long long int: "const volatile unsigned long long int", \
    const volatile float: "const volatile float", \
    const volatile double: "const volatile double", \
    const volatile long double: "const volatile long double", \
    const volatile char *: "const volatile char pointer", \
    const volatile void *: "const volatile void pointer", \
    default: "Unknown")


  // endl, just a symbol that can be used to produce the normal
  // line ending.
  // endlf can take a file to print to.
  // e.g. ```display(x); display(y); endl;```
  //  ```endlf(FILE* x);```
  // Windows has a different line ending.
  #if defined(_WIN32) || defined(__WIN32) || defined(WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64) || defined(WIN64) || defined(__WIN64__) || defined(__WINNT) || defined(__WINNT__) || defined(WINNT)
    #define endl printf("%s", "\r\n")
    #define endlf(f) fprintf(f, "%s", "\r\n")
  #else
    #define endl printf("%c", '\n')
    #define endlf(f) fprintf(f, "%c", '\n')
  #endif

#endif