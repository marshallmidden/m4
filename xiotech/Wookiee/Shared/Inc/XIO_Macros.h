/* $Id: XIO_Macros.h 159966 2012-10-01 23:20:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       XIO_Macros.h
**
**  @brief      Macro definitions
**
**  Contains general purpose macro definitions.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XIO_MACROS_H_
#define _XIO_MACROS_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/* These two macros are for unused parameters on functions (gcc -W option),
   and for functions that do not return. */
#define UNUSED  __attribute__((unused)) /*@unused@*/
#define NORETURN __attribute__((noreturn))

/**
**  @name Bit Handling Macros
**  @{
**/
#define BIT_TEST(data,bit)  ((data) & (1 << (bit)) ? (TRUE) : (FALSE))
#define BIT_SET(data,bit)   ((data) |= (1 << (bit)))
#define BIT_CLEAR(data,bit) ((data) &= ~(1 << (bit)))
/* @} */

/**
**  @name LSW/MSW Handling Macros
**  @{
**/
#define XIO_LSW(x)  ((UINT16)((x) & 0xffff))
#define XIO_MSW(x)  ((UINT16)((UINT32)((x) >> 16)))
/* @} */

/**
**  @name Minimum and Maximum Comparison Macros
**  @{
**/
#ifndef MIN
    #define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
/* @} */

/**
**  @name "dimension_of" macros to retrieve the number of elements in an array
**  @{
**/
#define dimension_of(a) (sizeof(a) / sizeof((a)[0]))
/* @} */

/**
**  @name Is Power Of Two Macro
**  @{
**/
#define IS_POWER_OF_TWO(n)  (((n) == 0) || ((n) & ((n) - 1)) ? FALSE : TRUE)
/* @} */

/**
**  @name CASSERT
**  @{
**/
#define CASSERT_CONCAT_(a, b)   a##b
#define CASSERT_CONCAT(a, b)    CASSERT_CONCAT_(a, b)
#define CASSERT(x) extern int CASSERT_CONCAT(__fail, __LINE__)[1 - (2 * !(x))]
/* @} */

#ifndef DOXYGEN
// Tack LOCATE_IN_SHMEM after a variable definition (and before initialization)
// to put into a shared memory section. Such variables are allocatable "a",
// writeable "w", and do not contain data "@nobits" -- i.e. the section only
// occupies space (and labels are at the specified addresses, etc.). The '#' is
// a comment to the assembler saying to ignore the rest of the line. This is
// because gcc tacks on (,"aw",@progbits) for us. We do not have the @progbits
// to take effect, and the comment ignores the rest of the line. Gcc additionally
// does not parse the parameter passed into the section attribute.
#define LOCATE_IN_SHMEM __attribute__ ((section (".shmem,\"aw\",@nobits #")))
#else /* DOXYGEN */
#define LOCATE_IN_SHMEM
#endif /* DOXYGEN */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XIO_MACROS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
