/* $URL: $ */
/**
******************************************************************************
**
**  @file       xthread_memory.c
**
**  @version    $Revision$
**
**  @brief      Xthread memory routines.
**
**  @date       $Date$
**
**  @author     Bryan Holty
**
**  modified   $Author$
**
**  $Id$
**  
**  Xthread memory routines.
**
**  Copyright (c) 2005 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "xiostd.h"
#include "xthread_memory.h"
#include "xthread.h"
#include "dbg_xprintf.h"


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
static pthread_cond_t       xm_cvar;
static pthread_mutex_t      xm_mutx;

#ifdef XTHREAD_MQUEUE
static pthread_mutex_t      xm_qmutx;
static xm_data*             xm_qhead     = NULL;
static uint32_t             xm_qcount    = 0;
#endif /* XTHREAD_MQUEUE */

static volatile uint32_t    xm_waiters  = 0;

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
#ifdef XTHREAD_MQUEUE
static int32_t xm_data_enqueue( xm_data* mdata );
static int32_t xm_data_dequeue( xm_data* mdata );
#endif /* XTHREAD_MQUEUE */

#ifdef XTHREAD_MCHECK
static void xm_memory_error( enum mcheck_status status );
#endif
/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize xthread mem library.
**
**  Initialize xthread memory library.
**  
**  @return     0 on SUCCESS, -1 on error
**
**  @sa         none.
**
**  @warning    none.
**
******************************************************************************
**/
int32_t xthread_mem_init( void )
{
    int32_t             rc          = 0;
    pthread_condattr_t  condinit;

/*
** Turn on mem tracing if debug.
*/
#ifdef XTHREAD_MCHECK
    /*
    ** Turn on additional memory checking.
    */
    if ( mcheck(xm_memory_error) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, "mcheck FAILED\n");
        abort();
    }
#endif
    
#ifdef XTHREAD_MTRACE
    /*
    ** Turn on MTRACE.
    */
    mtrace();
#endif
    
    /*
    ** Initialize our cond_var attribute.
    */
    if ( (rc = pthread_condattr_init(&condinit)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "pthread_condattr_init (%s)\n", strerror(rc));
        rc = -1;
    }

    /*
    ** Set the process shared attribute.
    */
    else if ( (rc = pthread_condattr_setpshared(&condinit, 
                    PTHREAD_PROCESS_PRIVATE)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "pthread_condattr_setpshared (%s)\n", strerror(rc));
        rc = -1;
    }

    /*
    ** Initialize condition variable.
    */
    else if ( (rc = pthread_cond_init(&xm_cvar, &condinit)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "pthread_cond_init (%s)\n", strerror(rc));
        rc = -1;
    }
    
    /*
    ** Initialize our cond_var mutex.
    */
    else if ( (rc = xthread_mutexinit(&xm_mutx)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "xthread_mutexinit (%s)\n", strerror(rc));
        rc = -1;
    }

#ifdef XTHREAD_MQUEUE
    /*
    ** Initialize our queue mutex.
    */
    else if ( (rc = xthread_mutexinit(&xm_qmutx)) != 0 )
    {
        xprintf(XPRINTF_XTHREAD, 
                "xthread_mutexinit (%s)\n", strerror(rc));
        rc = -1;
    }
#endif /* XTHREAD_MQUEUE */

    return rc;
}

/**
******************************************************************************
**
**  @brief      Re-allocate memory for a thread.
**
**              malloc  - (memptr == NULL) && (size > 0).
**              realloc - (memptr != NULL) && (size > 0).
**              free    - (memptr != NULL) && (size == 0).
**
**  @param      memptr  - pointer to memory to free or realloc.
**  @param      size    - size to allocate or reallocate.
**  @param      flags   - flags for operation. (see xthread_memory.h)
**  @param      fname   - File Name called from (__FILE__).
**  @param      line    - line number within file (__LINE__).
**
**  @return     pointer to meomory freed, malloc'd or realloc'd on success
**              NULL if malloc fails and wait flag not set
**              pointer to old allocation if realloc fails and wait flag not set
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         See also (i.e. "sample_func2").
**
******************************************************************************
**/
void* xthread_mem_realloc( void* memptr, size_t size, uint32_t flags, 
                           const char* fname, uint32_t line )
{
    xm_data*    header      = NULL;
    xm_data*    header2     = NULL;
    int32_t     rc          = 0;

    /*
    ** Check validity.
    */
    if ( memptr )
    {
        /*
        ** get the header.
        */
        header2 = (void*)((size_t)memptr - sizeof(xm_data));

        /*
        ** Check header validity.
        */
        if ( header2->magic == XM_MAGIC )
        {
#ifdef XTHREAD_MQUEUE
            /*
            ** Dequeue.
            */
            if ( (!(header2->flags & XM_FLAG_NOQUEUE)) &&
                    (xm_data_dequeue(header2) != 0) )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_mem_realloc dequeue failed (%p)\n", memptr);
                abort();
            }
#endif /* XTHREAD_MQUEUE */
        }

        /*
        ** Uh-oh!!!
        */
        else
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xthread_mem_realloc bad magic (%p)\n", memptr);
            abort();
        }

#ifdef XTHREAD_MCHECK
        xm_memory_error(mprobe(header2));
#endif

        /*
        ** If size is zero, free!!
        */
        if ( size == 0 )
        {
            /*
            ** Clear magic.
            */
            header2->magic = 0xABACADAE;
            
            /*
            ** Free the memory.
            */
            free(header2);

            /*
            ** Look for waiters.
            */
            if ( xm_waiters )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_mem_realloc waking up (%u) waiters, \n", 
                        xm_waiters);
                pthread_cond_broadcast(&xm_cvar);
            }
        }
    }

    /*
    ** if size is 0 and memptr is NULL, tried to free NULL pointer.
    */
    else if ( !size )
    {
        xprintf(XPRINTF_XTHREAD, 
                "xthread_mem_free NULL pointer (%p)\n", memptr);
        abort();
    }
    
    /*
    ** Check that we have a valid size.
    */
    if ( size )
    {
        while ( (header = (xm_data*)realloc(header2, (size + sizeof(xm_data)))) == NULL )
        {
            /*
            ** Do we want to wait??
            */
            if ( flags & XM_FLAG_WAIT )
            {
                /*
                ** Lock the mutex.
                */
                if ( (rc = xthread_mutexlock(&xm_mutx, 1)) == 0 )
                {
                    /*
                    ** increment waiters.
                    */
                    ++xm_waiters;

                    xprintf(XPRINTF_XTHREAD, 
                            "xthread_mem_realloc waiting on memory - "
                            "%u waiters, waiting\n", xm_waiters);

                    /*
                    ** Wait on the condition variable.
                    */
                    if ( (rc = pthread_cond_wait(&xm_cvar, &xm_mutx)) != 0 )
                    {
                        xprintf(XPRINTF_XTHREAD, 
                                "pthread_cond_wait (%s)\n", strerror(rc));
                    }

                    /*
                    ** decrement waiters.
                    */
                    --xm_waiters;

                    /*
                    ** Unlock the mutex.
                    */
                    xthread_mutexunlock(&xm_mutx);
                }

                /*
                ** Something failed!!!,  We need to wait and try again!!
                */
                if ( rc != 0 )
                {
                    sleep(1);
                }

                /*
                ** Carry on.
                */
                continue;
            }

            /*
            ** We don't want to wait!!
            */
            break;
        }

        /*
        ** Is header valid.
        */
        if ( header )
        {
            /*
            ** Set the memptr.
            */
            memptr = (void*)((size_t)header + sizeof(xm_data));

            /*
            ** Fill in the header.
            */
            header->memptr                  = memptr;
            header->size                    = size;
            header->line                    = line;
            header->magic                   = XM_MAGIC;
            header->flags                   = flags;
            header->fptr                    = NULL;
            header->bptr                    = NULL;

            strncpy(header->fname, fname, XM_MAX_FNAME);
            header->fname[XM_MAX_FNAME-1]   = '\0';

#ifdef XTHREAD_MQUEUE
            /*
            ** Enqueue the header.
            */
            if ( (!(flags & XM_FLAG_NOQUEUE)) &&
                    (xm_data_enqueue(header) != 0) )
            {
                xprintf(XPRINTF_XTHREAD, "xm_data_enqueue failed (%p)\n", memptr);
                abort();
            }
#endif /* XTHREAD_MQUEUE */

            /*
            ** Check for Clear flag.
            */
            if ( flags & XM_FLAG_CLEAR )
            {
                bzero(memptr, size);
            }
        }
    }
       
    return memptr;
}

/**
******************************************************************************
**
**  @brief      retrieve memory records
**
**              usage:
**
**                  xm_data* mdata = NULL;
**                  size_t mcount = 0;
**
**                  mcount = xthread_mem_inspect(&mdata);
**
**                  if (mcount)
**                  {
**                      process records...
**                      mdata[0].magic ...
**                      
**                      free_routine(mdata);
**                  }
**
**  @param      memptr  - pointer to a pointer where to put records
**
**  @return     number of records.
**
**  @attention  Memory returned in memptr must be freed by caller.
**
**  @warning    none.
**
******************************************************************************
**/
uint32_t xthread_mem_inspect( xm_data** memptr )
{
    uint32_t    rc      = 0;
    uint32_t    windex  = 0;
    xm_data*    walker  = NULL;
    xm_data*    mdata   = NULL;

#ifdef XTHREAD_MQUEUE
        /*
        ** Lock the queue mutex.
        */
        if ( xthread_mutexlock(&xm_qmutx, 1) == 0 )
        {
            /*
            ** Allocate if we have good pointers.
            */
            if ( xm_qhead && memptr )
            {
                mdata = (xm_data*)xthread_mem_realloc( 
                                    NULL, (xm_qcount * sizeof(xm_data)),
                                    XM_FLAG_NOQUEUE, __FILE__, __LINE__ );
            }

            /*
            ** Did we allocate memory.  If so, fill it in.
            */
            if ( mdata )
            {
                rc = xm_qcount;
                walker = xm_qhead;
                *memptr = mdata;

                /*
                ** Copy the data.
                */
                for ( windex = 0; windex < xm_qcount; ++windex )
                {
                    memcpy(&mdata[windex], walker, sizeof(xm_data));
                    mdata[windex].fptr = NULL;
                    mdata[windex].bptr = NULL;
                    walker = walker->fptr;
                }
            }

            /*
            ** Unlock the mutex.
            */
            if ( xthread_mutexunlock(&xm_qmutx) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xthread_mem_inspect mutex unlock failed\n");
            }
        }

        /*
        ** Lock Failed.
        */
        else
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xthread_mem_inspect mutex lock failed\n");
        } 
#else
    memptr = memptr;
#endif /* XTHREAD_MQUEUE */

    return rc;
}


/*****************************************************************************
* PRIVATE FUNCTIONS
*****************************************************************************/

#ifdef XTHREAD_MCHECK
/**
******************************************************************************
**
**  @brief      Mem Error handler.
**
**  @param      status          Status of error
**
**  @return     none.
**
**  @sa         none.
**
**  @warning    none.
**
******************************************************************************
**/
static void xm_memory_error( enum mcheck_status status )
{
    /*
    ** Check the status.
    */
    switch ( status )
    {
        /*
        ** OK
        */
        case MCHECK_OK:
        case MCHECK_DISABLED:
            break;
            
        case MCHECK_HEAD:
            fprintf(stderr,
                    "!!! MEM_ERR: The data immediately before the block was modified !!!\n");
            abort();
            break;

        case MCHECK_TAIL:
            fprintf(stderr,    
                    "!!! MEM_ERR: The data immediately after the block was modified !!!\n");
            abort();
            break;

        case MCHECK_FREE:
            fprintf(stderr,    
                    "!!! MEM_ERR: Double Free !!!\n");
            abort();
            break;

        default:
            fprintf(stderr,    
                    "!!! MEM_ERR: Unknown Error !!!\n");
            abort();
            break;
    }
}
#endif /* XTHREAD_MCHECK */

#ifdef XTHREAD_MQUEUE
/**
******************************************************************************
**
**  @brief      Enqueue xthread mem data.
**
**  Enqueue xthread mem data.
**
**  @param      mdata          Pointer to the xm_data
**
**  @return     none.
**
**  @sa         xm_data_dequeue.
**
**  @warning    none.
**
******************************************************************************
**/
static int32_t xm_data_enqueue( xm_data* mdata )
{
    int32_t rc = -1;
    
    /*
    ** Make sure the pointer is valid.
    */
    if ( mdata )
    {
        /*
        ** Lock the queue mutex.
        */
        if ( (rc = xthread_mutexlock(&xm_qmutx, 1)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xm_data_enqueue mutex lock failed\n");
        }
        
        /*
        ** Mutex Locked.
        */
        else
        {
            /*
            ** If this is the first item in the queue,
            ** set the queue ptr to point to mdata
            ** and the threads bptr pointer to point to
            ** itself.
            */
            if ( xm_qhead == NULL )
            {
                xm_qhead = mdata;
                mdata->bptr = mdata;
            }

            /*
            ** Fix up the pointers.
            */
            mdata->fptr         = xm_qhead;
            mdata->bptr         = xm_qhead->bptr;
            mdata->fptr->bptr   = mdata;
            mdata->bptr->fptr   = mdata;
            
            /*
            ** Increment counter
            */
            ++xm_qcount;

            /*
            ** Unlock the mutex.
            */
            if ( (rc = xthread_mutexunlock(&xm_qmutx)) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xm_data_enqueue mutex unlock failed\n");
            }
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xm_data_enqueue NULL DATA !!!\n");
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Dequeue xthread mem data.
**
**  Dequeue xthread mem data.
**
**  @param      mdata          Pointer to the xm_data
**
**  @return     none.
**
**  @sa         xm_data_enqueue.
**
**  @warning    None.
**
******************************************************************************
**/
static int32_t xm_data_dequeue( xm_data* mdata )
{
    int32_t rc = -1;
    
    /*
    ** Make sure the pointer is valid.
    */
    if ( xm_qhead && mdata )
    {
        /*
        ** Lock the queue mutex.
        */
        if ( (rc = xthread_mutexlock(&xm_qmutx, 1)) != 0 )
        {
            xprintf(XPRINTF_XTHREAD, 
                    "xm_data_dequeue mutex lock failed\n");
        }
        
        /*
        ** Mutex Locked.
        */
        else
        {
            /*
            ** If last item, simply set queue to NULL.
            */
            if ( xm_qhead->fptr == xm_qhead )
            {
                xm_qhead = NULL;
            }
            
            /*
            ** Fix up pointers.
            */
            else
            {
                mdata->fptr->bptr = mdata->bptr;
                mdata->bptr->fptr = mdata->fptr;

                /*
                ** If the queue pointer was pointing to this
                ** task, point it at the next item.
                */
                if ( xm_qhead == mdata )
                {
                    xm_qhead = mdata->fptr;
                }
            }
            
            /*
            ** Decrement counter
            */
            --xm_qcount;

            /*
            ** Unlock the mutex.
            */
            if ( (rc = xthread_mutexunlock(&xm_qmutx)) != 0 )
            {
                xprintf(XPRINTF_XTHREAD, 
                        "xm_data_dequeue mutex unlock failed\n");
            }
        }
    }
    
    else
    {
        xprintf(XPRINTF_XTHREAD, 
                "!!! xm_data_dequeue NULL DATA !!!\n");
    }

    return rc;
}
#endif /* XTHREAD_MQUEUE */


/***
** Modelines
** vi:sw=4 ts=4 expandtab
***/
