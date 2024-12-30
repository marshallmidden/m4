/* $Id: XIO_Types.h 137382 2010-04-19 16:02:18Z mdr $*/
/**
******************************************************************************
**
**  @file       XIO_Types.h
**
**  @brief      Basic type definitions
**
**  Basic types defined here, no structures or other
**  extraneous crap should be in here.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _XIO_TYPES_H_
#define _XIO_TYPES_H_

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/**
**  @name UNSIGNED TYPE DEFINITIONS
**  @{
**/
#ifdef _MSC_VER
    typedef unsigned __int64            UINT64;
#elif defined(_WIN32) && defined(__GNUC__)
    typedef unsigned long long          UINT64;
#elif defined(_WIN32)
    typedef unsigned _int64             UINT64;
#else
    typedef unsigned long long          UINT64;
#endif

#ifdef _WIN
    #ifndef _BASETSD_H_
        typedef unsigned _int32         UINT32;
    #endif
#else
    typedef unsigned int                UINT32;
#endif

typedef unsigned short                  UINT16;
typedef unsigned char                   UINT8;
/* @} */

/**
**  @name SIGNED INTEGER TYPE DEFINITIONS
**  @{
**/
#ifdef _MSC_VER
    typedef signed __int64              INT64;
#elif defined(_WIN32) && defined(__GNUC__)
    typedef long long                   INT64;
#elif defined(_WIN32)
    typedef _int64                      INT64;
#else
    typedef signed long long            INT64;
#endif

#ifdef _WIN
    #ifndef _BASETSD_H_
        typedef _int32                  INT32;
    #endif
#else
    typedef signed int                  INT32;
#endif

typedef signed short                    INT16;
typedef signed char                     INT8;
/* @} */

/**
**  @name OTHER TYPE DEFINITIONS
**  @{
**/
typedef UINT32                          IP_ADDRESS;
typedef signed char                     CHAR;
typedef UINT32                          BUFFER;

#ifdef _WIN32
    #ifndef _WINDOWS
        typedef enum { false = 0, true  = 1 } bool;
    #endif
#else
    #ifndef __cplusplus     // Jeff Johnson 11/1/05 for EWOK to compile with C++
        #define HAVE_BOOL 1
        typedef enum { false = 0, true  = 1 } bool;
    #endif
#endif

#ifdef __GNUC__
    #define ZeroArray(X,Y)              X Y[0]
#else
    #define ZeroArray(X,Y)              UINT8 Y[]
#endif

typedef struct tword_t
{
    UINT32  lo;                         /* Low word of 96 bit value         */
    UINT32  mid;                        /* Middle word of 96 bit value      */
    UINT32  hi;                         /* High word of 96 bit value        */
} u_tword;                              /* 96 bit value                     */

typedef struct qword_t
{
    UINT32  lo;                         /* Low word of 128 bit value        */
    UINT32  midlo;                      /* Low middle word of 128 bit value */
    UINT32  midhi;                      /* High middle word of 128 bit value*/
    UINT32  hi;                         /* High word of 128 bit value       */
} u_qword;                              /* 128 bit value                    */

/**
** Time stamp definition - time in BCD format
**/
typedef struct TIMESTAMP
{
    UINT16  year;               /**< Year 0 -9999                           */
    UINT8   month;              /**< Month 1 -12                            */
    UINT8   date;               /**< Day of the month 1 - 31                */
    UINT8   day;                /**< Day of the week 1 - 7 (1 = Sunday)     */
    UINT8   hours;              /**< Hour 0 - 23     (0 = midnight)         */
    UINT8   minutes;            /**< Minutes 0 - 59                         */
    UINT8   seconds;            /**< Seconds 0 - 59                         */
    UINT32  systemSeconds;      /**< Seconds the system has been running    */
} TIMESTAMP, *TIMESTAMP_PTR;

/* @} */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _XIO_TYPES_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
