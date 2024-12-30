/* $Id: hw_common.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       hw_common.h
**
**  @brief      Common header for hw components.
**
**  Common header for hw components.
**
**  @defgroup _HW_MODULES_ Hardware Modules
**
**  Copyright (c) 2006-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __HW_COMMON__
#define __HW_COMMON__

/**
**  @ingroup _HW_MODULES_
**  @defgroup _HW_MODULES_COMMON Common Hardware routines
**
**  @brief      Common Hardware routines.
**/

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**
**  @ingroup _HW_MODULES_COMMON
**  @defgroup _HW_MODULES_MACROS Internal Macros
**
**  @brief      Internal Macros
**
**  @{
**/
#ifdef CCB_RUNTIME_CODE
    #include "XIO_Std.h"
    #include "debug_files.h"

    #define hw_malloc               MallocWC
    #define hw_free                 Free
    #define hw_printf(fmt...)       dprintf(DPRINTF_DEFAULT,fmt)
    #define hw_assert(cnd)          ;
    #define fmt64u                  "%llu"
    #define fmt64i                  "%lld"
    #define hw_openfd               open
    #define hw_closefd              Close
    #define hw_sleep(tm)            TaskSleepMS(tm*1000)
    #define hw_crtask(func,parm)    do { \
                                      TASK_PARMS p; \
                                      p.p1 = (size_t)parm; \
                                      TaskCreate((void(*)(TASK_PARMS*))func,&p); \
                                    } while (0)
    #define hw_lock_init(plock)     do { \
                                        if (plock) {hw_free(plock);} \
                                        if ((plock = hw_malloc(sizeof(MUTEX)))) \
                                            {XK_InitMutex((MUTEX*)plock, __FILE__, __LINE__);} \
                                    } while (0)
    #define hw_lock_lock(plock)     (void)XK_LockMutex((MUTEX*)plock, MUTEX_WAIT, __FILE__, __LINE__)
    #define hw_lock_unlock(plock)   XK_UnlockMutex((MUTEX*)plock)
#else
#error "This got compiled?"
    #include <assert.h>
    #include <errno.h>
//    #include <signal.h>
    #include <stdarg.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>

    #define hw_malloc               malloc
    #define hw_free                 free
    #define hw_printf(fmt...)       fprintf(stderr,fmt)
    #define hw_assert               assert
    #define fmt64u                  "%llu"
    #define fmt64i                  "%lld"
    #define hw_openfd               open
    #define hw_closefd              close
    #define hw_sleep                sleep
    #define hw_crtask(func,parm)    do { \
                                      pthread_t tid; \
                                      pthread_create(&tid,NULL,func,parm); \
                                    } while (0)
    #define hw_lock_init(plock)     do { \
                                        if (plock) hw_free(plock); \
                                        if ((plock = hw_malloc(sizeof(pthread_mutex_t)))) \
                                            pthread_mutex_init((pthread_mutex_t*)plock); \
                                    } while (0)
    #define hw_lock_lock(plock)     pthread_mutex_lock((pthread_mutex_t*)plock, 1)
    #define hw_lock_unlock(plock)   pthread_mutex_unlock((pthread_mutex_t*)plock)
#endif
/* @} */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULES_COMMON
**  @defgroup _HW_MODULES_COMMON_FUNCTIONS Common Hardware Functions
**
**  @brief      This is a complete list of function within this library.
**              Public functions are available for use in the Interface.
**              Private functions are internally used within the library.
**/

/**
**  @ingroup _HW_MODULES_COMMON_FUNCTIONS
**  @defgroup _HW_MODULES_COMMON_PUBLIC_FUNCTIONS Common Hardware Public Functions
**
**  @brief      These are the public functions available for this interface.
**
**  @{
**/

extern int32_t hw_write_int64_as_str(int64_t value, const char *filename);

extern int32_t hw_read_int64_from_str(int64_t *value, const char *filename);

extern int32_t hw_read_buffer(uint8_t *readbuf, uint32_t bytes, const char *filename);
extern int32_t hw_write_buffer(const uint8_t *writebuf, uint32_t bytes, const char *filename);
/* @} */

#endif /* __HW_COMMON__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
