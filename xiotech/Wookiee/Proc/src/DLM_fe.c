/* $Id: DLM_fe.c 155354 2011-04-26 16:18:42Z m4 $ */
/**
******************************************************************************
**
**  @file       DLM_fe.c
**
**  @brief      Data Link Manager - Front End functions
**
**  To provide support for the Data-link Manager logic which
**  supports XIOtech Controller-to-XIOtech Controller functions
**  and services for Fibre communications.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "DLM_fe.h"

#include "CA_CI.h"
#include "DLM_Comm.h"
#include "dtmt.h"
#include "mlmt.h"
#include "ficb.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include <time.h>
#include <string.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define MIN_POLL_CNT        30  /* Minimum number of Successful polls to
                                    declare the path usable                */
#define CHECK_POLL_WAIT     5   /* Time, in seconds, to wait for before
                                    checking the counts again               */

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/* Structure for delayed messages. */
struct DL_EVENT {
    struct DL_EVENT *next;                  /* Pointer to next entry in list. */
    UINT32           event;                 /* The event warning log message number. */
    UINT32           SN;                    /* This controller serial number. */
    UINT8            remote_path;           /* The remote path. */
    UINT8            remote_cluster;        /* The remote cluster. */
    UINT8            local_path;            /* The local path. */
    UINT8            icl_flag;              /* Is this an ICL flag. */
    time_t           wait_until;            /* Time in seconds to stop at. */
    UINT32           count;                 /* Count of number of times hit. */
    UINT8            report_yet;            /* Flag that warning was issued. */
};

/* Pointer to first entry in linked delayed message list. */
static struct DL_EVENT *first_delayed_message = NULL;
/* How long to wait for delaying a message. */
static time_t        WAIT_DELAY_TIME = 30;

/* NOTE: The following copied from logdef.h in CCB/Inc. */
struct LOG_LOST_PATH_PKT
{
    UINT32  controllerSN;
    UINT8   cluster;
    UINT8   opath;
    UINT8   tpath;
    UINT8   iclPathFlag;
    UINT8   dgStatus;
    UINT8   dgErrorCode1;
    UINT8   dgErrorCode2;
    UINT8   rsvd2;
};


struct DL_MESSAGE_SEND {
    struct LOG_HEADER_PKT    header;
    struct LOG_LOST_PATH_PKT lostPath;
};

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static UINT32   amRunning = FALSE;   /* Show if the task is running or not  */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern CA C_ca;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
void DL_add_delayed_message(UINT32, UINT32, UINT8, UINT8, UINT8, UINT8);
void DL_check_delayed(void);
UINT32 DL_remove_delayed_message(UINT32, UINT8, UINT8, UINT8, UINT8);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Mirror Partner Fibre Path Communications Available
**
**              This function determines if the Front End Fibre Paths have
**              been operational long enough to reliably allow Mirroring of
**              controller information to the Mirror Partner
**
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a separate running task and is not directly callable
**
******************************************************************************
**/
void DLM_MirrorPartnerFECommAvailable(void);

void DLM_MirrorPartnerFECommAvailable(void)
{
    UINT32     done = FALSE;              /* Flag to know when to end task      */
    UINT32     mirrorPartner = 0;         /* Serial Number of Mirror Partner    */
    MLMT*      pMP_MLMT;                  /* Mirror Partners MLMT               */
    DTMT*      pDTMT;                     /* DTMT (path) associated with the MP */
    UINT32     pollCounts = 0;            /* Cumulative Successful Poll Counters*/
    LOG_MIRROR_CAPABLE_PKT  LogCommGood;  /* Log Message to send                */

    /*
    ** Determine if this task is already running and return if it is.
    ** Otherwise start the polling to see when to let the CCB know the
    ** communications link is up and available to start mirroring controller
    ** information to the Mirror Partner
    */
    if (amRunning == FALSE)
    {
        amRunning = TRUE;               /* Set Flag showing task is running */


        /*
        ** Loop forever until the mirror partner has been successfully talked
        ** to or not in a Mirroring problem state (problem already resolved).
        */
        while ( (done == FALSE) &&
                ( ((C_ca.status & (1<<CA_MIRRORBROKEN)) != 0) ||
                  ((C_ca.status & (1<<CA_ERROR)) != 0) ) )
        {
            /*
            ** Determine if a new Mirror Partner has been assigned and need
            ** to find when best to talk to it (already know problems from
            ** test above), so clear out the poll counts and start over.
            */
            if (mirrorPartner != K_ficb->mirrorPartner)
            {
                /*
                ** Clear out all the polling counts to ensure there is
                ** the correct number of valid polls before saying the
                ** connection is good
                */
                mirrorPartner = K_ficb->mirrorPartner; /* Get latest MP     */

                pMP_MLMT = DLM_FindController(mirrorPartner);
                                        /* Will not find ourself in the tables*/

                if (pMP_MLMT != NULL)
                {
                    pDTMT = pMP_MLMT->pDTMTHead; /* Get the first path in list*/

                    while (pDTMT != NULL)   /* There is another path        */
                    {
                        pDTMT->td.xio.pollCount = 0; /* Zero out poll counter*/
                        pDTMT = pDTMT->td.xio.pNext; /* Get next DTMT in MLMT */
                    }
                }
            }

            /*
            ** Wait for the paths to accumulate successful polls
            */
            TaskSleepMS(CHECK_POLL_WAIT * 1000);

            /*
            ** Only test if the mirror partner has not changed.  If it has
            ** changed, start over from the top.
            */
            if (mirrorPartner == K_ficb->mirrorPartner)
            {
                /*
                ** Find the Mirror Partner and check all the paths to
                ** determine if the minimum time has elapsed with at least
                ** one path operation (to do this determination, add up
                ** all the path successful poll counters and compare to
                ** the minimum).
                */
                pollCounts = 0;             /* Reset the Cumulative counter */

                pMP_MLMT = DLM_FindController(mirrorPartner);
                                        /* Will not find ourself in the tables*/

                if (pMP_MLMT != NULL)
                {
                    pDTMT = pMP_MLMT->pDTMTHead; /* Get the first path in list*/

                    while ((done == FALSE) &&    /* Not done AND            */
                           (pDTMT != NULL))      /* There is another path   */
                    {
                        pollCounts += pDTMT->td.xio.pollCount;
                        pDTMT = pDTMT->td.xio.pNext; /* Get next DTMT in MLMT */

                        if (pollCounts >= MIN_POLL_CNT)
                        {
                            /*
                            ** Polled enough to believe the communications is
                            ** now reliable enough to mirror controller
                            ** information. Report the good news to the CCB.
                            */
                            LogCommGood.header.event = LOG_MIRROR_CAPABLE;
                            LogCommGood.header.length = sizeof(LOG_MIRROR_CAPABLE_DAT);
                            LogCommGood.data.mirrorPartnerSN = mirrorPartner;
                            /*
                             * Note: message is short, and L$send_packet copies into the MRP.
                             */
                            MSC_LogMessageStack(&LogCommGood, sizeof(LOG_MIRROR_CAPABLE_PKT));

                            done = TRUE;        /* All Done                 */
                        }
                    }
                }
                else if (mirrorPartner == K_ficb->cSerial)
                {
                    /*
                    ** The mirror partner has been reset to ourselves, just
                    ** exit the task
                    */
                    done = TRUE;
                }
            }
        }

        /*
        ** All Done - reported the successful communications.  Flag as done and
        ** exit (kills task).
        */
        amRunning = FALSE;              /* All done, clear the running flag */
    }
}   /* End of DLM_MirrorPartnerFECommAvailable */


/**
******************************************************************************
**
**  @brief      Delayed (debounced) PATH-LOST warning messages to log.
**
**  @param      event          - Event number for log message.
**  @param      SN             - Serial number of this controller.
**  @param      remote_path    - HAB that went down.
**  @param      remote_cluster - Other Xiotech cluster number.
**  @param      local_path     - This Xiotech Path communications are on.
**  @param      icl_flag       - Flag if this is the icl link (750 only).
**
**  @return     none
**
******************************************************************************
**/
void DL_add_delayed_message(UINT32 event, UINT32 SN, UINT8 remote_path,
                            UINT8 remote_cluster, UINT8 local_path,
                            UINT8 icl_flag)
{
    struct DL_EVENT *p = first_delayed_message;

    while (p != NULL)
    {
        if (p->event == event &&
            p->SN == SN &&
            p->remote_path == remote_path &&
            p->remote_cluster == remote_cluster &&
            p->local_path == local_path &&
            p->icl_flag == icl_flag)
        {
            p->count++;
            break;
        }
        p = p->next;
    }

    if (p == NULL)
    {
        /* Create new entry. */
        p = p_MallocC(sizeof(struct DL_EVENT), __FILE__, __LINE__);
        /* Link in old first entry, and set first to this entry. */
        p->next = first_delayed_message;
        first_delayed_message = p;
        p->count = 0;
    }

    /* Fill in data. Duplicate setting doesn't matter. */
    p->event = event;
    p->SN = SN;
    p->remote_path = remote_path;
    p->remote_cluster = remote_cluster;
    p->local_path = local_path;
    p->icl_flag = icl_flag;
    p->wait_until = time(NULL) + WAIT_DELAY_TIME;   /* Add 30 seconds to delay. */
    p->report_yet = FALSE;
}   /* End of DL_add_delayed_message */


/**
******************************************************************************
**
**  @brief      Process delayed messages -- if need to.
**
**  @param      event        - Event number for log message.
**  @param      controllerSN - Serial number of this controller.
**  @param      opath        - HAB that went down.
**  @param      cluster      - Other Xiotech cluster number.
**  @param      tpath        - This Xiotech Path communications are on.
**  @param      iclPathFlag  - Flag if this is the icl link (750 only).
**
**  @return     none
**
******************************************************************************
**/
void DL_check_delayed(void)
{
    struct DL_EVENT *previous = NULL;
    struct DL_EVENT *p = first_delayed_message;
    time_t  current_time = time(NULL);

    while (p != NULL)
    {
        struct DL_EVENT *next = p->next;

        if (p->wait_until < current_time)
        {
            if (p->report_yet == FALSE)
            {
                struct DL_MESSAGE_SEND m;

                p->report_yet = TRUE;
                memset(&m, 0, sizeof(m));
                m.header.event = p->event;
                m.lostPath.controllerSN = p->SN;
                m.lostPath.cluster = p->remote_cluster;
                m.lostPath.opath = p->remote_path;
                m.lostPath.tpath = p->local_path;
                m.lostPath.iclPathFlag = p->icl_flag;
                m.lostPath.dgStatus = 0;
                m.lostPath.dgErrorCode1 = 0;
                m.lostPath.dgErrorCode2 = 0;
/*                m.lostPath.rsvd2 = */
                MSC_LogMessageStack(&m, sizeof(m));
            }
            /* If older than a day, delete. */
            else if ((p->wait_until + 60*60*24) < current_time)
            {
                if (previous == NULL)
                {
                    first_delayed_message = next;
                }
                else
                {
                    previous->next = next;
                }
                p_Free(p, sizeof(struct DL_EVENT), __FILE__, __LINE__);
                p = previous;
            }
        }
        previous = p;
        p = next;
    }
}   /* End of DL_check_delayed */


/**
******************************************************************************
**
**  @brief      Remove delayed message Path-Lost warning messages from list.
**
**  @param      SN             - Serial number of this controller.
**  @param      remote_path    - HAB that went down.
**  @param      remote_cluster - Other Xiotech cluster number.
**  @param      local_path     - This Xiotech Path communications are on.
**  @param      icl_flag       - Flag if this is the icl link (750 only).
**
**  @return     0   - if the message was found, but not reported as Warning yet.
**  @return     1   - if the message was found, and has already been reported.
**  @return     2   - if the message was not found -- i.e. first time seen.
**
******************************************************************************
**/
UINT32 DL_remove_delayed_message(UINT32 SN, UINT8 remote_path,
                            UINT8 remote_cluster, UINT8 local_path,
                            UINT8 icl_flag)
{
    struct DL_EVENT *previous = NULL;
    struct DL_EVENT *p = first_delayed_message;

    while (p != NULL)
    {
        if (p->SN == SN &&
            p->remote_path == remote_path &&
            p->remote_cluster == remote_cluster &&
            p->local_path == local_path &&
            p->icl_flag == icl_flag)
        {
            int i = p->report_yet;          /* Save return checking value. */

            if (previous == NULL)
            {
                first_delayed_message = p->next;
            }
            else
            {
                previous->next = p->next;
            }
            p_Free(p, sizeof(struct DL_EVENT), __FILE__, __LINE__);
            if (i == FALSE)
            {
                return(0);
            }
            return(1);
        }
        previous = p;
        p = p->next;
    }
    return(2);
}   /* End of DL_remove_delayed_message */


/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
