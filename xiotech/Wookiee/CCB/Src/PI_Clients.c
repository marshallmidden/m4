/* $Id: PI_Clients.c 143020 2010-06-22 18:35:56Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_Clients.c
**
**  @brief      Platform Interface client information.
**
**  This file contains the code for the client information functionality
**  of the Platform Interface.  The client information module handles
**  the client connection list and provides accessor functions for the
**  rest of the system.
**
**  Copyright (c) 2005-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_Clients.h"
#include "debug_files.h"
#include "ipc_common.h"
#include "logdef.h"
#include "logging.h"
#include "PortServer.h"
#include "PacketInterface.h"
#include "slink.h"
#include "XIO_Const.h"
#include <sys/time.h>

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/**
**  @ingroup    PI_CLIENTS
**
** List of client connections.
**/
static S_LIST *pclients = NULL;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
MUTEX       gClientsMutex;
extern void AsyncEventPingTask(TASK_PARMS *pparms);

/*
******************************************************************************
** Public variables - not externed in any header file
******************************************************************************
*/
extern time_t skip_login_logout_msg_until;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static int32_t pi_clients_compare(void *item1, void *item2);
static int32_t pi_clients_compare_key(void *item, void *key);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Initializes the packet server configuration information
**              to make it available for use.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_initialize(void)
{
    pclients = CreateList();

    /*
     * Create the task taht will send the asynchronous PING events.
     */
    TaskCreate(AsyncEventPingTask, NULL);
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Get the pointer to the list of clients for iteration.
**
**  @return     none
**
******************************************************************************
**/
S_LIST *pi_clients_get_list(void)
{
    if (pclients != NULL)
    {
        (void)LockMutex(&gClientsMutex, MUTEX_WAIT);
    }

    return (pclients);
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Release the pointer to the list of clients for iteration.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_release_list(void)
{
    UnlockMutex(&gClientsMutex);
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Compare function which compares which compares the two
**              pi_client_info items.  The comparison uses the file
**              descriptor stored in the structure as the key value.
**
**  @param      item1 - First of two items to compare
**  @param      item2 - Second of two items to compare
**
**  @return     return == 0 if item1 == item2
**  @return     return <  0 if item1 <  item2
**  @return     return >  0 if item1 >  item2
**
******************************************************************************
**/
static INT32 pi_clients_compare(void *item1, void *item2)
{
    INT32       rc = 0;
    INT32       sockfd_key = ((pi_client_info *)item1)->sockfd;
    INT32       sockfd_item = ((pi_client_info *)item2)->sockfd;

    if (sockfd_key < sockfd_item)
    {
        rc = -1;
    }
    else if (sockfd_key > sockfd_item)
    {
        rc = 1;
    }

    return rc;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Compare function which compares a given key to a
**              pi_client_info item.  the comparison uses the key
**              and the file descriptor stored in the item structure.
**
**  @param      item - Item to compare with key
**  @param      key - Key to use in comparisons
**
**  @return     return == 0 if key == item->sockfd
**  @return     return <  0 if key <  item->sockfd
**  @return     return >  0 if key >  item->sockfd
**
******************************************************************************
**/
static INT32 pi_clients_compare_key(void *item, void *key)
{
    INT32       rc = 0;
    INT32       sockfd_item = ((pi_client_info *)item)->sockfd;
    INT32       sockfd_key = (INT32)key;

    if (sockfd_key < sockfd_item)
    {
        rc = -1;
    }
    else if (sockfd_key > sockfd_item)
    {
        rc = 1;
    }

    return rc;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Adds a new client connection to the list of connections.
**
**  @param      ipaddr - Client IP address
**  @param      port - Client port number
**  @param      sockfd - Socket file descriptor
**
**  @return     pi_client_info* - Client information pointer
**
******************************************************************************
**/
pi_client_info *pi_clients_add(UINT32 ipaddr, INT32 port, INT32 sockfd, INT32 tmo_receive)
{
    S_LIST     *plist;
    pi_client_info *pinfo;

    dprintf(DPRINTF_DEFAULT, "pi_clients_add-ENTER (sockfd: %d)\n", sockfd);

    plist = pi_clients_get_list();

    pinfo = MallocWC(sizeof(*pinfo));
    pinfo->ipaddr = ipaddr;
    pinfo->port = port;
    pinfo->sockfd = sockfd;
    pinfo->berror = FALSE;
    pinfo->client_type = PI_UNSPECIFIED_CLIENT;
    pinfo->tmo_receive = tmo_receive;

    AddElement(plist, pinfo);

    pi_clients_release_list();

    dprintf(DPRINTF_DEFAULT, "pi_clients_add-EXIT (pinfo: %p, sockfd: %d)\n", pinfo,
            sockfd);

    return pinfo;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Removes a client connection from the list of connections.
**
**  @param      pinfo_remove - Pointer to the client to remove from the list.
**
**  @return     pi_client_info* - Client information pointer or NULL if
**                                 the connection cannot be found.
**
**  @attention  This removes the client information from the list and expects
**              the caller to free the information.
**
******************************************************************************
**/
pi_client_info *pi_clients_remove(pi_client_info *pinfo_remove)
{
    S_LIST     *plist;
    pi_client_info *pinfo = NULL;

    dprintf(DPRINTF_DEFAULT, "pi_clients_remove-ENTER (pinfo_remove: %p, sockfd: %d)\n",
            pinfo_remove, pinfo_remove->sockfd);

    plist = pi_clients_get_list();

    pinfo = RemoveElement(plist, (void *)pinfo_remove, (void *)&pi_clients_compare);

    if (pinfo == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "pi_clients_remove-Failed to remove client from list\n");
    }

    pi_clients_release_list();

    dprintf(DPRINTF_DEFAULT, "pi_clients_remove-EXIT (pinfo: %p, sockfd: %d)\n",
            pinfo, (pinfo != NULL ? pinfo->sockfd : SOCKET_ERROR));

    return pinfo;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Retrieves a client connection from the list of connections.
**
**  @param      sockfd - Socket file descriptor
**
**  @return     pi_client_info* - Client information pointer or NULL if
**                                 the client connection cannot be found.
**
**  @attention  The list still owns the information so the caller must not
**              free the information.
**
******************************************************************************
**/
static pi_client_info *pi_clients_get(INT32 sockfd)
{
    S_LIST     *plist;
    pi_client_info *pinfo = NULL;

//    dprintf(DPRINTF_DEFAULT, "pi_clients_get-ENTER (sockfd: %d)\n", sockfd);

    plist = pi_clients_get_list();

    pinfo = FindElement(plist, (void *)sockfd, (void *)&pi_clients_compare_key);

    if (pinfo == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "pi_clients_get-Failed to find client in list\n");
    }

    pi_clients_release_list();

//    dprintf(DPRINTF_DEFAULT, "pi_clients_get-EXIT (pinfo: %p, sockfd: %d)\n", pinfo, sockfd);

    return pinfo;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Closes the socket connection for a client.
**
**  @param      pinfo - Pointer to the client which needs its socket closed.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_close(pi_client_info *pinfo)
{
    LOG_MRP     opLog;          /* Size = 264 bytes */
    time_t      cur_time;

    ccb_assert(pinfo != NULL, pinfo);

    /*
     * Print a debug message to the serial console.
     */
    dprintf(DPRINTF_DEFAULT, "pi_clients_close-Closing socket connection (ipaddr: %08x, port: %d, sockfd: %d)\n",
            pinfo->ipaddr, pinfo->port, pinfo->sockfd);

    /*
     * Make a log entry
     */
    cur_time = time(NULL);
    if (cur_time > skip_login_logout_msg_until)
    {
        opLog.mleEvent = LOG_CONFIGURATION_SESSION_END;
    }
    else
    {
        opLog.mleEvent = LOG_CONFIGURATION_SESSION_END_DEBUG;
    }
    opLog.mleLength = 8;
    ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->port = pinfo->port;
    ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->flags = 0;
    ((LOG_SESSION_START_STOP_PKT *)(opLog.mleData))->IPAddress = (UINT32)(pinfo->ipaddr);
    AsyncEventHandler(NULL, &opLog);

    /*
     * close the socket connection
     */
    Close(pinfo->sockfd);

    pinfo->sockfd = SOCKET_ERROR;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Set the client into an error state.
**
**  @param      pinfo - Pointer to the client which needs its error state set.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_error(pi_client_info *pinfo)
{
    ccb_assert(pinfo != NULL, pinfo);

    /*
     * Print a debug message to the serial console.
     */
    dprintf(DPRINTF_DEFAULT, "pi_clients_error-Setting the error state of a client (ipaddr: %08x, port: %d, sockfd: %d)\n",
            pinfo->ipaddr, pinfo->port, pinfo->sockfd);

    pinfo->berror = TRUE;
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Retrieves the registered events for a given client socket
**              connection.
**
**  @param      sockfd - Socket file descriptor
**
**  @return     Current event registration information if sock file
**              descriptor is still valid.  Zero if there are no events
**              or if the file descriptor is not valid.
**
******************************************************************************
**/
UINT64 pi_clients_register_events_get(INT32 sockfd)
{
    pi_client_info *pinfo;

    pinfo = pi_clients_get(sockfd);

    return (pinfo != NULL ? pinfo->regEvents : 0);
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Sets the events a client is registered to receive.
**
**  @param      sockfd - Socket file descriptor
**  @param      events - Bitmap of the events the client wants to receive.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_register_events_set(INT32 sockfd, UINT64 events)
{
    pi_client_info *pinfo;

    pinfo = pi_clients_get(sockfd);

    if (pinfo != NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "Setting client event registration (sockfd: %d, port: %u, events: %8.8x%8.8x).",
                   sockfd, pinfo->port, ((UINT32)(events >> 32)), (UINT32)events);

        pinfo->regEvents = events;
    }
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Retrieves the receive timeout for a given client socket
**              connection.
**
**  @param      sockfd - Socket file descriptor
**
**  @return     Current receive timeout if sock file descriptor is still
**              valid.  PI_PORT_TIMEOUT_DEFAULT if the file descriptor is
**              not valid.
**
******************************************************************************
**/
UINT32 pi_clients_tmo_receive_get(INT32 sockfd)
{
    pi_client_info *pinfo;

    pinfo = pi_clients_get(sockfd);

    return (pinfo != NULL ? pinfo->tmo_receive : PI_PORT_TIMEOUT_DEFAULT);
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      Sets the receive timeout for a  given client socket connection.
**
**  @param      sockfd - Socket file descriptor
**  @param      tmo_receive - New receive timeout for the client.
**
**  @return     none
**
******************************************************************************
**/
void pi_clients_tmo_receive_set(INT32 sockfd, UINT32 tmo_receive)
{
    pi_client_info *pinfo;

    pinfo = pi_clients_get(sockfd);

    if (pinfo != NULL)
    {
        LogMessage(LOG_TYPE_DEBUG, "Setting client receive timeout (sockfd: %d, port: %u, timeout: %u).",
                   sockfd, pinfo->port, tmo_receive);

        pinfo->tmo_receive = tmo_receive;
    }
}


/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      count the number of clients of client_type.
**
**  @param      client_type - Client type
**
**  @return     number of clients
**
******************************************************************************
**/
UINT8 pi_clients_count_client_type(UINT8 client_type)
{
    UINT8       count = 0;
    S_LIST     *plist;
    pi_client_info *pinfo;

    dprintf(DPRINTF_DEFAULT, "pi_clients_count_client_type-ENTER (client_type: %d)\n",
            client_type);

    plist = pi_clients_get_list();

    SetIterator(plist);

    while ((pinfo = Iterate(plist)) != NULL)
    {
        if (pinfo->client_type == client_type)
        {
            ++count;
        }
    }
    pi_clients_release_list();

    dprintf(DPRINTF_DEFAULT, "pi_clients_count_client_type-EXIT (count: %d)\n", count);

    return count;
}

/**
******************************************************************************
**
**  @ingroup    PI_CLIENTS
**
**  @brief      register the client_type for the socket
**
**  @param      client_type - Client type
**
**  @return     number of clients
**
******************************************************************************
**/
void pi_clients_register_client_type(UINT8 client_type, UINT32 sockfd)
{

    pi_client_info *pinfo;

    dprintf(DPRINTF_DEFAULT, "pi_clients_register_client_type-ENTER (client_type: %d)\n",
            client_type);

    pinfo = pi_clients_get(sockfd);
    if (pinfo != NULL)
    {
        pinfo->client_type = client_type;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "pi_clients_register_client_type: pi_clients_get returned NULL\n");
    }
}



/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
