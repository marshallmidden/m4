/* $URL: svn://cvs.xiotech.com/trunk/common/xthread/xthread.c $ */
/**
******************************************************************************
**
**  @file       xthread.c
**
**  @version    $Revision: 117 $
**
**  @brief      Odin threading library.
**
**  @date       $Date: 2006-01-03 10:02:13 -0600 (Tue, 03 Jan 2006) $
**
**  @author     Bryan Holty
**
**  modified   $Author: bbd $
**
**  $Id: xthread.c 117 2006-01-03 16:02:13Z bbd $
**  
**  Odin threading library.
**
**  Copyright (c) 2005 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "xiostd.h"
#include "dbg_xprintf.h"
#include "xthread.h"
#include "xthread_memory.h"


/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/


/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define xthread_add_lock(l,t) \
    do { \
        xthread_data* tdata = (xthread_data*)pthread_getspecific(xkeys); \
        if ( !tdata ) break; \
        tdata->mlist[tdata->mcount].type = (t); \
        tdata->mlist[tdata->mcount].genptr = (void*)(l); \
        ++tdata->mcount; \
    } while (0)

#define xthread_del_lock()  \
    do { \
        xthread_data* tdata = (xthread_data*)pthread_getspecific(xkeys); \
        if ( !tdata ) break; \
        --tdata->mcount; \
        tdata->mlist[tdata->mcount].type = XTHREAD_TYPE_NONE; \
        tdata->mlist[tdata->mcount].genptr = NULL; \
    } while (0)

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/


/*
******************************************************************************
** Private variables
******************************************************************************
*/
static pthread_key_t   xkeys;
static pthread_mutex_t xqmutex;

static xthread_data*   xqhead       = NULL;


/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void*   xthread_task( void* data ) __attribute__((noreturn));
static void*   xthread_cleanup( void* data );
static int32_t xthread_data_enqueue( xthread_data* tdata );
static int32_t xthread_data_dequeue( xthread_data* tdata );
static int32_t xthread_check_lock( void* lock, uint32_t type );
static int32_t xthread_check_unlock( void* lock, uint32_t type );
/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize xthread library.
**
**  Initialize xthread library.
**  
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         none.
**
**  @warning    none.
**
******************************************************************************
**/
int32_t xthread_init( void )
{
    int32_t             rc          = -1;

    /*
    ** Initialize our thread keys if not already done.
    */
    if ( (rc = pthread_key_create(&xkeys, NULL)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "pthread_key_create (%s)\n", strerror(rc));
    }
    
    /*
    ** Initialize our queue mutex.
    */
    else
    {
        rc = xthread_mutexinit(&xqmutex);
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      xthread_create will create a pthread.
**
**  Wrapper for pthread_create
**
**  This function is a user friendly library to the pthread library, 
**  simplifying the user interface and experience.  There are a couple of
**  macros that also make this easier:
**
**    Create a thread, you DO NOT wish to wait for with xthread_join:
**
**  xthread_cr(mainFunc, funcArg)
**
**
**    Create a thread with a cleanup function, 
**    you DO NOT wish to wait for with xthread_join
**
**  xthread_cr_clnp(mainFunc, funcArg, cleanFunc)
**
**
**    Create a thread, you DO wish to wait for with xthread_join
**
**  xthread_crj(mainFunc, funcArg)
**
**
**    Create a thread with a cleanup function, 
**    you DO wish to wait for with xthread_join
**
**  xthread_crj_clnp(mainFunc, funcArg, cleanFunc)
**
**
**  When this function is called a new thread will be created calling
**  "mainFunc(funcArg)".  The thread will begin immediately, so make sure any
**  semaphores/mutexes the thread may use are initialized prior to this call
**  if the thread does not take care of the initialization itself.
**
**  If cleanFunc is defined, this will be pushed onto the pthread exit stack
**  to be called in the case that the thread terminates earlier.  Normally by
**  an outside enitity.  This could happen in the case where the User requests
**  a system shutdown, and our shutdown routine requests termination of all
**  threads.  Another case may be a system crash, with the error handler 
**  requesting termination of all threads.  This cleanFunc offers the ability
**  to attempt some cleanup when we know we are going down.  If cleanFunc is
**  NULL, it will be ignored.  The cleanup function will be passed the same
**  data as mainFunc.  "cleanFunc(funcArg)".
**
**  The threadFlags are some flags that will dictate the attributes of the 
**  pthread that is created (see xthread.h).
**      XTHREAD_FLAG_NONE -         No flags are defined.
**      XTHREAD_FLAG_DETACH -       Clean up the thread data immediately on 
**                                  termination of the thread.  You cannot
**                                  do an xthread_join to wiat for this 
**                                  thread to exit with this flag set.
**      XTHREAD_FLAG_INH_PARENT -   Inherit scheduling policy from parent.
**
**      XTHREAD_FLAG_DEFAULT -      This is a combination of:
**                                  XTHREAD_FLAG_DETACH|XTHREAD_FLAG_INH_PARENT
**                                  This should be used in the default case,
**                                  unless you need additional functionality.
**
**  @param      mainFunc    This is the function to call with the new thread
**  @param      funcArg     This is a pointer to any argument structure to 
**                          pass to the thread. 
**  @param      cleanFunc   This is a cleanup function, only called in the case
**                          where the thread is cancelled by an outside entity.
**                          i.e. System shutdown will terminate running threads
**                          This WILL NOT be called upon normal thread 
**                          termination (i.e. thread finishes and exits).
**                          This will be ignored if NULL.
**  @param      threadFlags Flags for the creation of the thread (see above):
**                              XTHREAD_FLAG_NONE
**                              XTHREAD_FLAG_DETACH
**                              XTHREAD_FLAG_INH_PARENT
**                              XTHREAD_FLAG_DEFAULT
**
**  @return     pointer to the xthread_data on SUCCESS.  NULL on FAILURE
**
**  @sa         xthread_join.
**
**  @warning    None.
**
******************************************************************************
**/
xthread_data* xthread_create( void* (*mainFunc)(void *), 
                                     void* funcArg,
                                     void* (*cleanFunc)(void *),
                                     uint32_t threadFlags )
{
    int32_t             rc          = -1;
    xthread_data*       tdata       = NULL;
    pthread_attr_t      tattr;

    do
    {
        /*
        ** This is the only parameter we really care about.
        */
        if ( mainFunc == NULL )
        {
            xprintf(XPRINTF_XTHREAD, "mainFunc NULL\n");
            break;
        }
        
        /*
        ** Allocate our thread data.
        */
        tdata = (xthread_data*)xthread_mem_realloc(NULL, sizeof(xthread_data), XM_FLAG_CLEAR, __FILE__, __LINE__);

        if ( tdata == NULL )
        {
            xprintf(XPRINTF_XTHREAD, "xthread_mem_realloc (%s)\n", strerror(errno));
            break;
        }
        else
        {
            bzero((void *)tdata, sizeof(xthread_data));
            /*
            ** Copy our inputs.
            */
            tdata->mainFunc     = mainFunc;
            tdata->funcArg      = funcArg;
            tdata->cleanFunc    = cleanFunc;
            tdata->threadFlags  = threadFlags;

            /*
            ** Enqueue the data.
            */
            if ( xthread_data_enqueue(tdata) != 0 )
            {
                break;
            }
        }
        
        /*
        ** Initialize the pthread attributes.
        */
        if ( (rc = pthread_attr_init(&tattr)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_attr_init (%s)\n", strerror(rc));
            break;
        }

        /*
        ** If the detach state is requested, set the proper attribute.
        */
        if ( (threadFlags & XTHREAD_FLAG_DETACH) &&
             ( (rc = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED)) != 0) )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_attr_setdetachstate (%s)\n", strerror(rc));
            break;
        }
        
        /*
        ** If the inherit state is requested, set the proper attribute.
        */
        if ( (threadFlags & XTHREAD_FLAG_INH_PARENT) &&
             ( (rc = pthread_attr_setinheritsched(&tattr, PTHREAD_INHERIT_SCHED)) != 0) )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_attr_setinheritsched (%s)\n", strerror(rc));
            break;
        }
        
#if 0
	/*
	** Safe and portable programs do not depend upon the default stack
	** limit, but instead, explicitly allocate enough stack for each thread
	** by using the pthread_attr_setstacksize routine.
	*/
#if PTHREAD_STACK_MIN != 16384
#error PTHREAD_STACK_MIN != 16384, what is the signific reason?
#endif

	if ((rc = pthread_attr_setstacksize(&tattr, PTHREAD_STACK_MIN)) != 0)
	{
	    xprintf(XPRINTF_XTHREAD,
	    	"pthread_attr_setstacksize(%d) failed (%s)\n", PTHREAD_STACK_MIN, strerror(rc));
	    break;
	}
#endif /* 0 */

        /*
        ** Create the thread.
        */
        if ( (rc = pthread_create(&tdata->threadId, &tattr, xthread_task, tdata)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_create (%s)\n", strerror(rc));
            break;
        }
        else
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xthread_create tid: %p\n", (void *)(tdata->threadId));
        }
    } while (0);

    /*
    ** If we were unsuccessful, and 
    ** allocated the data, free it.
    */
    if ( rc != 0 )
    {
        if ( tdata )
        {
            xthread_data_dequeue(tdata);
            xthread_mem_realloc(tdata, 0, 0, __FILE__, __LINE__);
            tdata = NULL;
        }

        xprintf(XPRINTF_XTHREAD, "xthread_create failed (%s)\n", strerror(rc));
    }
    
    return tdata;
}

/**
******************************************************************************
**
**  @brief      This function will wait for a thread to exit.
**
**  Wrapper for pthread_join
**
**  This function will wait for the thread in tdata to exit.  This will block
**  until the thread tdata exits.
**
**  This function cannot be called from the same thread with which is
**  to be waited for.
**
**  This call can only be made to a thread that was created without the
**  XTHREAD_FLAG_DETACH flag.  The call will fail if this was not the case.
**  
**  @param      tdata           Pointer to the xthread_data of the thread
**                              to be waited on.
**  @param      retval          Pointer to pointer that will point to 
**                              return data.  Ignored if NULL
**
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         xthread_create.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_join( xthread_data* tdata, void** retval )
{
    int32_t             rc          = -1;
    uint8_t             freeflag    = 1;

    /*
    ** Make sure we have a valid pointer, 
    ** and that the DETACH flag is not set.
    */
    if ( tdata )
    {
        /*
        ** If the detach flag is set, record the error.
        */
        if ( tdata->threadFlags & XTHREAD_FLAG_DETACH )
        {
            xprintf(XPRINTF_XTHREAD, "xthread_join detach flag set\n");
            freeflag = 0;
            rc = -1;
        }
            
        /*
        ** Make the call to do the join.  Errors from pthread_join man page:
        **  ESRCH   No thread could be found corresponding to that specified by th.
        **  EINVAL  The th thread has been detached.
        **  EINVAL  Another thread is already waiting on termination of th.
        **  EDEADLK The th argument refers to the calling thread.
        */
        else if ( (rc = pthread_join(tdata->threadId, retval)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, "pthread_join (%s)\n", strerror(rc));

            /*
            ** If the call fails, see why (see above)
            ** we do not want to free the memory if:
            **  EINVAL  possibly detached, or someone is already waiting on it.
            **  EDEADLK from the calling thread.
            */
            if ( (rc == EINVAL) || (rc == EDEADLK) )
            {
                freeflag = 0;
            }
            
            rc = -1;
        }
        
        /*
        ** We made it!!
        */
        else
        {
            rc = 0;
        }

        /*
        ** If the freeflag is set, free the xthread_data*.
        */
        if ( freeflag )
        {
            xthread_data_dequeue(tdata);
            xthread_mem_realloc(tdata, 0, 0, __FILE__, __LINE__);
            tdata = NULL;
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_join NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      This function will terminate an existing thread.
**
**  Wrapper for pthread_cancel
**
**  This function will terminate an existing thread, if the thread exists.
**  
**  @param      tdata           Pointer to the xthread_data of the thread
**                              to terminate.
**                              If tdata is NULL, ALL threads will be 
**                              cancelled, except for the calling thread.
**
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         none.
**
**  @attention  If NULL is passed in, the return value has no meaning.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_cancel( xthread_data* tdata )
{
    int32_t             rc          = -1;
    
    /*
    ** Make sure we have a valid pointer, 
    */
    if ( tdata )
    {
        /*
        ** Make the call to do the cancel.
        */
        if ( (rc = pthread_cancel(tdata->threadId)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, "pthread_cancel (%s)\n", strerror(rc));
            rc = -1;
        }
        
        /*
        ** We made it!!
        */
        else
        {
            rc = 0;
        }
    }
    else
    {
        /*
        ** Get the current thread... If any.
        */
        xthread_data* cur_tdata = (xthread_data*)pthread_getspecific(xkeys);
        
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_cancel CANCELLING ALL !!!\n");

        /*
        ** Try to lock the queue mutex. Don,t let it stop us.
        */
        rc = xthread_mutexlock(&xqmutex, 0);
        
        tdata = xqhead;
        while ( tdata != NULL )
        {
            /*
            ** Make the call to do the cancel.
            */
            if ( (cur_tdata != tdata) &&
                 (pthread_cancel(tdata->threadId) != 0) )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "pthread_cancel failed (%p)\n", 
                        (void*)tdata->threadId);
            }

            /*
            ** Advance the pointer.
            */
            tdata = tdata->fptr;

            /*
            ** Check for wrap.
            */
            if ( tdata == xqhead )
            {
                break;
            }
        }

        /*
        ** If we locked the mutex, unlock it.
        */
        if ( rc == 0 )
        {
            xthread_mutexunlock(&xqmutex);
        }
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Initialize a pthread_mutex.
**
**  Wrapper for pthread_mutex_init.
**
**  This function will initalize mutex.  This function does some additional
**  attribute settings to the mutex.  i.e.  automatically selects the fastest
**  mutex type available.
**
**  A pthread_mutex_t may not be used (locked or unlocked), until it has been
**  initialized.
**
**  @param      mutex           Pointer to mutex to initialize.
**
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         xthread_mutexdestroy, xthread_mutexlock, xthread_mutexunlock.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_mutexinit( pthread_mutex_t* mutex )
{
    int32_t             rc          = -1;
    pthread_mutexattr_t tmutexattr;

    /*
    ** Check for a valid mutex pointer.
    */
    if ( mutex )
    {
        /*
        ** Initialize our mutex attributes.
        */
        if ( (rc = pthread_mutexattr_init(&tmutexattr)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_mutexattr_init (%s)\n", strerror(rc));
            rc = -1;
        }
        
        /*
        ** Set the mutex attributes type.
        */
        else if ( (rc = pthread_mutexattr_settype(&tmutexattr, PTHREAD_MUTEX_ADAPTIVE_NP)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_mutexattr_settype (%s)\n", strerror(rc));
            rc = -1;
        }
        
        /*
        ** Initialize the mutex.
        */
        else if ( (rc = pthread_mutex_init(mutex, &tmutexattr)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_mutexattr_settype (%s)\n", strerror(rc));
            rc = -1;
        }

        /*
        ** We made it!.
        */
        else
        {
            rc = 0;
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_mutexinit NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Destroy a pthread_mutex.
**
**  Wrapper for pthread_mutex_destroy.
**
**  This function will destroy mutex.
**
**  A pthread_mutex_t may not be used (locked or unlocked), after it has been
**  destroyed.
**
**  @param      mutex           Pointer to mutex to initialize.
**
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         xthread_mutexinit, xthread_mutexlock, xthread_mutexunlock.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_mutexdestroy( pthread_mutex_t* mutex )
{
    int32_t             rc          = -1;

    /*
    ** Check for a valid mutex pointer.
    */
    if ( mutex )
    {
        /*
        ** Destroy the mutex.
        */
        if ( (rc = pthread_mutex_destroy(mutex)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_mutexattr_settype (%s)\n", strerror(rc));
            rc = -1;
        }

        /*
        ** We made it!.
        */
        else
        {
            rc = 0;
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_mutexinit NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Lock a mutex.
**
**  Wrapper for pthread_mutex_lock.
**
**  This function will lock a mutex, or attempt to lock a mutex, based upon 
**  wait.
**
**  A pthread_mutex_t may not be used (locked or unlocked), until it has been
**  initialized.
**
**  @param      mutex           Pointer to mutex to initialize.
**  @param      wait            0 = don't wait(try), 1 = wait.
**
**  @return     0 on Lock, -1 on error, 1 with wait if already locked
**
**  @sa         xthread_mutexinit, xthread_mutexdestroy, xthread_mutexunlock.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_mutexlock( pthread_mutex_t* mutex, uint8_t wait )
{
    int32_t             rc          = -1;

    /*
    ** Check for a valid mutex pointer.
    */
    if ( mutex )
    {
        /*
        ** Check the lock.
        */
        if ( (rc = xthread_check_lock((void*)mutex, XTHREAD_TYPE_MUTEX)) != 0 )
        {
            /*
            ** Do we already have it locked, we need to unlock!!  
            ** We want a write lock, and may have the read lock.
            */
            if ( rc == 1 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_mutexlock ALREADY LOCKED BY US. FINE!!!\n");
                rc = 0;
            }

            xprintf(XPRINTF_XTHREAD, "xthread_mutexlock lock check failed\n");
        }

        /*
        ** If we want to wait for the mutex, lock it.
        */
        else if ( wait )
        {
            /*
            ** Lock.
            */
            if ( (rc = pthread_mutex_lock(mutex)) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "pthread_mutex_lock (%s)\n", strerror(rc));
                rc = -1;
            }

            /*
            ** add mutex to table.
            */
            else
            {
                xthread_add_lock(mutex, XTHREAD_TYPE_MUTEX);
            }
        }
        
        /*
        ** If we want to not wait if it is locked, use
        ** trylock. errors from man page.
        **  EBUSY  the mutex could not be acquired because it was currently locked.
        **  EINVAL the mutex has not been properly initialized.
        */
        else if ( (rc = pthread_mutex_trylock(mutex)) != 0 )
        {
            /*
            ** If the lock failed and it was not because someone else
            ** has it locked, set the failure.
            */
            if ( rc != EBUSY )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "pthread_mutex_trylock (%s)\n", strerror(rc));
                rc = -1;
            }
            
            /*
            ** Set that it is already locked.
            */
            else
            {
                rc = 1;
            }
        }
        
        /*
        ** add mutex to table.
        */
        else
        {
            xthread_add_lock(mutex, XTHREAD_TYPE_MUTEX);
        }
        
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_mutexlock NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Unlock a mutex.
**
**  Wrapper for pthread_mutex_unlock.
**
**  Unlock a mutex
**
**  A pthread_mutex_t may not be used (locked or unlocked), until it has been
**  initialized.
**
**  @param      mutex           Pointer to mutex unlock.
**
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         xthread_mutexlock, xthread_mutexdestroy, xthread_mutexinit.
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_mutexunlock( pthread_mutex_t* mutex )
{
    int32_t             rc          = -1;

    /*
    ** Check mutex ptr.
    */
    if ( mutex )
    {
        /*
        ** Pre-check lock.
        */
        if ( (rc = xthread_check_unlock((void*)mutex, XTHREAD_TYPE_MUTEX)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, "xthread_rwlockunlock lcok check failed\n");
            rc = -1;
        }
    
        /*
        ** Unlock.
        */
        else if ( (rc = pthread_mutex_unlock(mutex)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_mutex_trylock (%s)\n", strerror(rc));
            rc = -1;
        }
        
        /*
        ** Success, remove from table!
        */
        else
        {
            xthread_del_lock();
        }
    } 
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_mutexunlock NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
 *
 *  @brief      Initialize a pthread_rwlock.
 *
 *  Wrapper for pthread_rwlock_init.
 *
 *  This function will initalize rwlock.  This function does some additional
 *  attribute settings to rwlock.  
 *
 *  A pthread_rwlock_t may not be used, until it has been initialized.
 *
 *  @param      rwlock        Pointer to rwlock to initialize.
 *
 *  @return     0 on SUCCESS, -1 on error
 *
 *  @sa         xthread_rwlockrd, xthread_rwlockwr, xthread_rwlockunlock
 *
 *  @warning    None.
 *
******************************************************************************
**/
int32_t xthread_rwlockinit( pthread_rwlock_t *rwlock )
{
    int32_t             rc          = -1;
    pthread_rwlockattr_t trwattr;

    /*
    ** Check for a valid mutex pointer.
    */
    if ( rwlock )
    {
        /*
        ** Initialize the rwlock attributes.
        */
        if ( (rc = pthread_rwlockattr_init(&trwattr)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_rwlockattr_init (%s)\n", strerror(rc));
            rc = -1;
        }
            
        /*
        ** Set the attribute type to private.
        */
        else if ( (rc = pthread_rwlockattr_setpshared(&trwattr, 
                        PTHREAD_PROCESS_PRIVATE)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_rwlockattr_setpshared (%s)\n", strerror(rc));
            rc = -1;
        }

        /*
        ** Set the type.
        */
        else if ( (rc = pthread_rwlockattr_setkind_np(&trwattr, 
                        PTHREAD_RWLOCK_PREFER_WRITER_NP)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_rwlockattr_setkind_np (%s)\n", strerror(rc));
            rc = -1;
        }

        /*
        ** Initialize the rwlock.
        */
        else if ( (rc = pthread_rwlock_init(rwlock, &trwattr)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_rwlock_init (%s)\n", strerror(rc));
            rc = -1;
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_rwlockinit NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
 *
 *  @brief      Lock a rwlock for writing.
 *
 *  Wrapper for pthread_rwlock_wrlock.
 *
 *  This function will lock a rwlock for writing, or attempt to lock a rwlock
 *  lock for writing, based upon wait.
 *
 *  A pthread_rwlock_t may not be used, until it has been initialized.
 *
 *  @param      rwlock          Pointer to rwlock to lock for writing.
 *  @param      wait            0 = don't wait(try), 1 = wait.
 *
 *  @return     0 on Lock, -1 on error, 1 with wait if already locked
 *
 *  @sa         xthread_rwlockrd, xthread_rwlockinit, xthread_rwlockunlock
 *
 *  @warning    None.
 *
******************************************************************************
**/
int32_t xthread_rwlockwr( pthread_rwlock_t *rwlock, uint8_t wait )
{
    int32_t             rc          = -1;
    uint8_t             prevlocked  = 0;

    /*
    ** Check for a valid rwlock pointer.
    */
    if ( rwlock )
    {
        /*
        ** Check the lock.
        */
        while ( prevlocked <= 1 )
        {
            if ( (rc = xthread_check_lock((void*)rwlock, XTHREAD_TYPE_RWLOCK)) != 0 )
            {
                /*
                ** Do we already have it locked, we need to unlock!!  
                ** We want a write lock, and may have the read lock.
                */
                if ( rc == 1 )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "xthread_rwlockwr ALREADY LOCKED BY US. Unlocking!!!\n");

                    if ( xthread_rwlockunlock(rwlock) != 0 )
                    {
                        xprintf(XPRINTF_XTHREAD, 
                                "xthread_rwlockwr ALREADY LOCKED BY US. Unlocking Failed!!!\n");
                        rc = -1;
                        break;
                    }

                    ++prevlocked;
                    continue;
                }

                xprintf(XPRINTF_XTHREAD, "xthread_rwlockwr lock check failed\n");
                rc = -1;
                break;
            }

            break;
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_rwlockwr NULL DATA !!!\n");
    }
        
    /*
    ** If things are well.
    */
    if ( rc == 0 )
    {
        /*
        ** If we want to wait for the rwlock, lock it.
        */
        if ( wait )
        {
            /*
            **  EINVAL  The value specified by rwlock does not refer to an 
            **          initialized read-write lock object.
            **  EDEADLK The current thread already owns the read-write lock 
            **          for writing or reading
            */
            if ( (rc = pthread_rwlock_wrlock(rwlock)) != 0 )
            {
                /*
                ** If we already have the lock, return success
                */
                if ( rc == EDEADLK )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "pthread_rwlock_wrlock ALREADY LOCKED (%s)\n",
                            strerror(rc));
                    rc = 0;
                }
                else
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "pthread_rwlock_wrlock (%s)\n", strerror(rc));
                }
                rc = -1;
            }
        }

        /*
        ** If we want to not wait if it is locked, use
        ** trylock. errors from man page.
        **  EBUSY   The read-write lock could not be acquired for writing
        **          because it was already locked for reading or writing.
        **  EINVAL  The value specified by rwlock does not refer to an 
        **          initialized read-write lock object.
        */
        else if ( (rc = pthread_rwlock_trywrlock(rwlock)) != 0 )
        {
            /*
            ** If the lock failed and it was not because someone else
            ** has it locked, set the failure.
            */
            if ( rc != EBUSY )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "pthread_rwlock_trywrlock (%s)\n", strerror(rc));
                rc = -1;
            }

            /*
            ** Set that it is already locked.
            */
            else
            {
                rc = 1;
            }
        }

        /*
        ** Add to table.
        */
        if ( rc >= 0 )
        {
            xthread_add_lock(rwlock, XTHREAD_TYPE_RWLOCK);
        }
    }
    
    return rc;
}

/**
******************************************************************************
 *
 *  @brief      Lock a rwlock for reading.
 *
 *  Wrapper for pthread_rwlock_rdlock.
 *
 *  This function will lock a rwlock for reading, or attempt to lock a rwlock
 *  lock for reading, based upon wait.
 *
 *  A pthread_rwlock_t may not be used, until it has been initialized.
 *
 *  @param      rwlock          Pointer to rwlock to lock for reading.
 *  @param      wait            0 = don't wait(try), 1 = wait.
 *
 *  @return     0 on Lock, -1 on error, 1 with wait if already locked
 *
 *  @sa         xthread_rwlockwr, xthread_rwlockinit, xthread_rwlockunlock
 *
 *  @warning    None.
 *
******************************************************************************
**/
int32_t xthread_rwlockrd( pthread_rwlock_t *rwlock, uint8_t wait )
{
    int32_t             rc          = -1;
    uint8_t             prevlocked  = 0;

    /*
    ** Check for a valid rwlock pointer.
    */
    if ( rwlock )
    {
        /*
        ** Check the lock.
        */
        while ( prevlocked <= 1 )
        {
            if ( (rc = xthread_check_lock((void*)rwlock, XTHREAD_TYPE_RWLOCK)) != 0 )
            {
                /*
                ** Do we already have it locked, we need to unlock!!  
                ** We want a write lock, and may have the read lock.
                */
                if ( rc == 1 )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "xthread_rwlockrd ALREADY LOCKED BY US. Unlocking!!!\n");

                    if ( xthread_rwlockunlock(rwlock) != 0 )
                    {
                        xprintf(XPRINTF_XTHREAD, 
                                "xthread_rwlockrd ALREADY LOCKED BY US. Unlocking Failed!!!\n");
                        rc = -1;
                        break;
                    }

                    ++prevlocked;
                    continue;
                }

                xprintf(XPRINTF_XTHREAD, "xthread_rwlockrd lock check failed\n");
                rc = -1;
                break;
            }

            prevlocked = 0;
            break;
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_rwlockrd NULL DATA !!!\n");
    }
        
    /*
    ** If things are well.
    */
    if ( rc == 0 )
    {
        /*
        ** If we want to wait for the rwlock, lock it.
        */
        if ( wait )
        {
            /*
            **  EINVAL  The value specified by rwlock does not refer to an 
            **          initialized read-write lock object.
            **  EDEADLK The current thread already owns the read-write lock 
            **          for writing or reading
            */
            if ( (rc = pthread_rwlock_rdlock(rwlock)) != 0 )
            {
                /*
                ** If we already have the lock, return success
                */
                if ( rc == EDEADLK )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "pthread_rwlock_trywrlock ALREADY LOCKED (%s)\n",
                            strerror(rc));
                    rc = 0;
                }
                else
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "pthread_rwlock_wrlock (%s)\n", strerror(rc));
                }
                rc = -1;
            }
        }
        
        /*
        ** If we want to not wait if it is locked, use
        ** trylock. errors from man page.
        **  EBUSY   The read-write lock could not be acquired for writing
        **          because it was already locked for reading or writing.
        **  EINVAL  The value specified by rwlock does not refer to an 
        **          initialized read-write lock object.
        */
        else if ( (rc = pthread_rwlock_tryrdlock(rwlock)) != 0 )
        {
            /*
            ** If the lock failed and it was not because someone else
            ** has it locked, set the failure.
            */
            if ( rc != EBUSY )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "pthread_rwlock_trywrlock (%s)\n", strerror(rc));
                rc = -1;
            }
            
            /*
            ** Set that it is already locked.
            */
            else
            {
                rc = 1;
            }
        }
        
        if ( rc >= 0 )
        {
            xthread_add_lock((void*)rwlock, XTHREAD_TYPE_RWLOCK);
        }
    }

    return rc;
}

/**
******************************************************************************
 *
 *  @brief      Unlock a rwlock.
 *
 *  Wrapper for pthread_rwlock_unlock.
 *
 *  Unlock a rwlock 
 *
 *  A pthread_rwlock_t may not be used (locked or unlocked), until it has been
 *  initialized.
 *
 *  @param      rwlock         Pointer to rwlock to initialize.
 *
 *  @return     0 on SUCCESS, -1 on error
 *
 *  @sa         xthread_rwlockwr, xthread_rwlockinit, xthread_rwlockrd
 *
 *  @warning    None.
 *
******************************************************************************
**/
int32_t xthread_rwlockunlock( pthread_rwlock_t *rwlock )
{
    int32_t             rc          = -1;

    if ( rwlock )
    {
        if ( (rc = xthread_check_unlock((void*)rwlock, XTHREAD_TYPE_RWLOCK)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, "xthread_rwlockunlock lcok check failed\n");
            rc = -1;
        }
        
        else if ( (rc = pthread_rwlock_unlock(rwlock)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_rwlock_unlock (%s)\n", strerror(rc));
            rc = -1;
        }
        else
        {
            xthread_del_lock();
        }
    } 
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_rwlockunlock NULL DATA !!!\n");
    }

    return rc;
}

/*****************************************************************************
* PRIVATE FUNCTIONS
*****************************************************************************/

/**
******************************************************************************
**
**  @brief      Our friendly thread.
**
**  This function is called as a thread for the xthread library.
**
**  Each xthread that is created starts here.  This thread will set up some
**  of the thread parameters, push the cleanup handlers, and call the function
**
**  @param      data           Pointer to the xthread_data
**
**  @return     void* to join.
**
**  @sa         xthread_create.
**
**  @warning    None.
**
******************************************************************************
**/
static void *cleanup_task( xthread_data *tdata )
{
    void *retdata;

    pthread_cleanup_push((void(*)(void *))tdata->cleanFunc, tdata->funcArg);
    retdata = tdata->mainFunc(tdata->funcArg);
    pthread_cleanup_pop(0);
    return(retdata);
}

/**
******************************************************************************
**
**  @brief      Our friendly thread.
**
**  This function is called as a thread for the xthread library.
**
**  Each xthread that is created starts here.  This thread will set up some
**  of the thread parameters, push the cleanup handlers, and call the function
**
**  @param      data           Pointer to the xthread_data
**
**  @return     void* to join.
**
**  @sa         xthread_create.
**
**  @warning    None.
**
******************************************************************************
**/
static void* xthread_task( void* data ) 
{
    xthread_data*       tdata       = (xthread_data*)data;
    int32_t             rc          = 0;
    void*               retdata     = NULL;
    sigset_t            mask;

    /*
    ** We need to ignore certain signals sent to the main process.
    */
    sigemptyset(&mask);

    /* Add to ignore here */
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);

    /*
    ** Make sure we have valid pointers
    */
    if ( tdata && tdata->mainFunc )
    {
        /*
        ** Set the thread specific data to point to our xthread_data
        */
        if ( (rc = pthread_setspecific(xkeys, (void*)tdata)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_setspecific (%s)\n", strerror(rc));
        }
        
        /*
        ** Enable Cancelling of threads.
        */
        if ( (rc = pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_setcancelstate (%s)\n", strerror(rc));
        }
        
        /*
        ** Set the cancel type to asynchronous (immediate)
        */
        if ( (rc = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "pthread_setcanceltype (%s)\n", strerror(rc));
        }

        /*
        ** Push our internal cleanup handler
        */
        pthread_cleanup_push((void(*)(void *))xthread_cleanup, (void*)tdata);
        
        /*
        ** If a cleanup handler was requested, push it and call the function
        */
        if ( tdata->cleanFunc )
        {
                retdata = cleanup_task(tdata);
        }
        
        /*
        ** Otherwise, just call the function.
        */
        else 
        {
           retdata = tdata->mainFunc(tdata->funcArg);
        }
        
        /*
        ** Pop and execute our internal cleanup handler.
        */
        pthread_cleanup_pop(1);
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_task NULL DATA !!!\n");
    }

    /*
    ** Exit with our retrun data.
    */
    pthread_exit(retdata);
}

/**
******************************************************************************
**
**  @brief      Our friendly thread cleanup handler.
**
**  This function is the cleanup handler for our internal xthreads.
**
**  This cleanup handler will free the data for our threads if they are 
**  detached.  Otherwise xthread_join will clean it up.
**
**  @param      data           Pointer to the xthread_data
**
**  @return     none.
**
**  @sa         xthread_task.
**
**  @warning    None.
**
******************************************************************************
**/
static void* xthread_cleanup( void* data )
{
    xthread_data* tdata = (xthread_data*)data;
    
    /*
    ** Check for a valid pointer.
    */
    if (tdata)
    {
        /*
        ** Check if there are any mutex's left unlocked by this task.
        */
        if ( tdata->mcount != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "!!! xthread_cleanup exiting with (%d) mutex's locked !!!\n", 
                    tdata->mcount);

            /*
            ** Free mutex's being abandoned.
            */
            while ( tdata->mcount != 0 )
            {
                if ( tdata->mlist[(tdata->mcount-1)].type == XTHREAD_TYPE_MUTEX )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "!!! xthread_cleanup unlocking mutex (%p) !!!\n", 
                            tdata->mlist[(tdata->mcount-1)].genptr);

                    xthread_mutexunlock((pthread_mutex_t*)tdata->mlist[(tdata->mcount-1)].genptr);
                }
                
                else if ( tdata->mlist[(tdata->mcount-1)].type == XTHREAD_TYPE_RWLOCK )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "!!! xthread_cleanup unlocking rwlock (%p) !!!\n", 
                            tdata->mlist[(tdata->mcount-1)].genptr);
                    
                    xthread_rwlockunlock((pthread_rwlock_t*)tdata->mlist[(tdata->mcount-1)].genptr);
                }
                
                else
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "!!! UNKNOWN LOCK TYPE...  SKIPPING (%p) !!!\n", 
                            tdata->mlist[(tdata->mcount-1)].genptr);
                    
                    --tdata->mcount;
                }
            }
        }

        /*
        ** If this task was detached, don't free the memory.
        ** xthread_join will take care of that.
        */
        if ( tdata->threadFlags & PTHREAD_CREATE_DETACHED )
        {
            xthread_data_dequeue(tdata);
            xthread_mem_realloc(tdata, 0, 0, __FILE__, __LINE__);
            tdata = NULL;
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_cleanup NULL DATA !!!\n");
    }

    return NULL;
}

/**
******************************************************************************
**
**  @brief      Enqueue xthread data.
**
**  Enqueue xthread data.
**
**  @param      tdata          Pointer to the xthread_data
**
**  @return     none.
**
**  @sa         xthread_data_dequeue.
**
**  @warning    none.
**
******************************************************************************
**/
static int32_t xthread_data_enqueue( xthread_data* tdata )
{
    int32_t rc = -1;
    
    /*
    ** Make sure the pointer is valid.
    */
    if ( tdata )
    {
        /*
        ** Lock the queue mutex.
        */
        if ( (rc = xthread_mutexlock(&xqmutex, 1)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xthread_data_enqueue mutex lock failed\n");
        }
        
        /*
        ** Mutex Locked.
        */
        else
        {
            /*
            ** If this is the first item in the queue,
            ** set the queue ptr to point to tdata
            ** and the threads bptr pointer to point to
            ** itself.
            */
            if ( xqhead == NULL )
            {
                xqhead = tdata;
                tdata->bptr = tdata;
            }

            /*
            ** Fix up the pointers.
            */
            tdata->fptr         = xqhead;
            tdata->bptr         = xqhead->bptr;
            tdata->fptr->bptr   = tdata;
            tdata->bptr->fptr   = tdata;

            /*
            ** Unlock the mutex.
            */
            if ( (rc = xthread_mutexunlock(&xqmutex)) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_data_enqueue mutex unlock failed\n");
            }
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_data_enqueue NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Dequeue xthread data.
**
**  Dequeue xthread data.
**
**  @param      tdata          Pointer to the xthread_data
**
**  @return     none.
**
**  @sa         xthread_data_enqueue.
**
**  @warning    None.
**
******************************************************************************
**/
static int32_t xthread_data_dequeue( xthread_data* tdata )
{
    int32_t rc = -1;
    
    /*
    ** Make sure the pointer is valid.
    */
    if ( xqhead && tdata )
    {
        /*
        ** Lock the queue mutex.
        */
        if ( (rc = xthread_mutexlock(&xqmutex, 1)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xthread_data_dequeue mutex lock failed\n");
        }
        
        /*
        ** Mutex Locked.
        */
        else
        {
            /*
            ** If last item, simply set queue to NULL.
            */
            if ( xqhead->fptr == xqhead )
            {
                xqhead = NULL;
            }
            
            /*
            ** Fix up pointers.
            */
            else
            {
                tdata->fptr->bptr = tdata->bptr;
                tdata->bptr->fptr = tdata->fptr;

                /*
                ** If the queue pointer was pointing to this
                ** task, point it at the next item.
                */
                if ( xqhead == tdata )
                {
                    xqhead = tdata->fptr;
                }
            }

            /*
            ** Unlock the mutex.
            */
            if ( (rc = xthread_mutexunlock(&xqmutex)) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_data_dequeue mutex unlock failed\n");
            }
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xthread_data_dequeue NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Pre-check a lock
**
**  Check a lock to see if we have enough room 
**  and check to see if we already have the lock.
**
**  @param      lock            Pointer to lock to check.
**  @param      type            see xthread.h (XTHREAD_MUTEX_CHECKING).
**
**  @return     0 on OK to Lock, -1 on not OK, 1 if already locked
**
**  @sa         xthread_check_unlock
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_check_lock( void* lock, uint32_t type )
{
    int32_t             rc          = 0;
    int32_t             mcount      = 0;
    xthread_data*       tdata       = (xthread_data*)pthread_getspecific(xkeys);

    /*
    ** See if we were able to get the thread data.
    */
    if (tdata != NULL)
    {
        /*
        ** Make sure we are not locking over the maximum allowed.
        */
        if ( tdata->mcount >= XTHREAD_MAX_LOCK )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "Thread %p:  xthread_check_lock too many mutex locks(%d)\n", 
                    (void *)(tdata->threadId), tdata->mcount);
            rc = -1;
        }

        /*
        ** Make sure we do not already have this lock locked
        */
        else
        {
            for ( mcount = (tdata->mcount - 1); mcount >= 0; --mcount )
            {
                if ( (tdata->mlist[mcount].type == type) &&
                        (tdata->mlist[mcount].genptr == lock) )
                {
                    xprintf(XPRINTF_XTHREAD, 
                            "Thread %p:  xthread_check_lock already locked(%p)\n", 
                            (void *)(tdata->threadId), lock);
                    rc = 1;
                }
            }
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, "xthread_check_lock no thread data\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Pre-check an unlock
**
**  Make sure we have the lock locked.
**
**  @param      lock            Pointer to lock to check.
**  @param      type            see xthread.h (XTHREAD_MUTEX_CHECKING).
**
**  @return     0 on Locked, -1 on not Locked
**
**  @sa         xthread_check_lock
**
**  @warning    None.
**
******************************************************************************
**/
int32_t xthread_check_unlock( void* lock, uint32_t type )
{
    int32_t             rc          = 0;
    int32_t             mcount      = 0;
    xthread_data*       tdata       = (xthread_data*)pthread_getspecific(xkeys);

    if ( tdata != NULL )
    {
        mcount = (tdata->mcount - 1);

        /*
        ** See if any lock's are locked by this thread.
        */
        if ( mcount < 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "Thread %p:  xthread_check_unlock No locks(%p)\n", 
                    (void *)(tdata->threadId), lock);
            rc = -1;
        }
            
        /*
        ** Check for the ordering.
        ** The best use of lock's is to unlock 
        ** in the reverse order of locking.
        */
        else if ( tdata->mlist[mcount].genptr != lock )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "Thread %p:  xthread_check_unlock out of order(%p)\n", 
                    (void *)(tdata->threadId), lock);

            /*
            ** Look for the lock in the table.
            */
            for ( mcount = (tdata->mcount - 1); mcount >= 0; --mcount )
            {
                /*
                ** If we find it, shift things around
                ** and place the lock at the end.
                */
                if ( (tdata->mlist[mcount].type == type) &&
                     (tdata->mlist[mcount].genptr == lock) )
                {
                    while ( (uint32_t)mcount < (tdata->mcount - 1) )
                    {
                        tdata->mlist[mcount].type = tdata->mlist[mcount+1].type;
                        tdata->mlist[mcount].genptr = tdata->mlist[mcount+1].genptr;
                        ++mcount;
                    }
                    tdata->mlist[(tdata->mcount - 1)].type = type;
                    tdata->mlist[(tdata->mcount - 1)].genptr = lock;
                    break;
                }
            }

            /*
            ** Did it exist at all in our list??
            */
            if ( mcount < 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "Thread %p:  xthread_check_unlock lock not locked(%p)\n", 
                        (void *)(tdata->threadId), lock);
                rc = -1;
            }
        }
    }
    else
    {
        xprintf(XPRINTF_XTHREAD, "xthread_check_unlock no thread data\n");
    }

    return rc;
}

/***
** Modelines
** vi:sw=4 ts=4 expandtab
***/
