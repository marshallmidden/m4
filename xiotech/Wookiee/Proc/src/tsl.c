/* $Id: tsl.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       tsl.c
**
**  @brief      TSL 'C' functions
**
**  This provides API for iSCSI Module, Socket Comminications,
**  Interface Monitoring functionality
**
**  Copyright (c) 2005-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <byteswap.h>

#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>

#include "def.h"
#include "ficb.h"
#include "MR_Defs.h"
#include "OS_II.h"
#include "pcb.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

#ifdef HISTORY_KEEP
extern void CT_history_pcb(const char *str, UINT32 pcb);
#endif

#ifdef FRONTEND
#include "icl.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"
#include "iscsi_tsl.h"
#include "iscsi_digest.h"
#include "fsl.h"
#include "iscsi_timer.h"
#include "li_pci.h"
#include "tmt.h"
#include "system.h"
#include "isp.h"
typedef UINT64 u64;
typedef UINT32 u32;
typedef UINT16 u16;
typedef UINT8  u8;
#include <linux/ethtool.h>
#endif  /* FRONTEND */

#ifdef BACKEND
/* NOTE: Can not include iscsi_tsl.h without including a bunch others. */
/* But, that causes more problems with header files. Do it this way. */
extern void tsl_init(void);
extern INT32  tsl_ev_add(INT32 fd, UINT32 events, void *func, void *pRef);
extern INT32  tsl_ev_del(INT32 fd, UINT32 events, void *func, void *pRef);
extern INT32  tsl_ev_mod(INT32 fd, UINT32 events, void *func, void *pRef);
extern void TSL_SetNonBlocking (int);
#endif  /* BACKEND */

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define TSL_EV_MAX               260
// Relook into Task Priority TBD
#define TSL_EV_PRI               141
#define TSL_SEND_PRI             140
#define TSL_ARP_TASK             150

#define TSL_TCP_SEND_BUF_MAX     16777216
#define TSL_TCP_RECV_BUF_MAX     16777216
#define TSL_MAX_CONNECTIONS      256
#define TSL_DEFAULT_ISCSI_PORT   3260

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

struct tsl_ev_dat
{
    void        *cb;
    void        *pRef;
};

typedef struct rta
{
    unsigned short len;
    unsigned short type;
    INT32 data;
}rta;

typedef struct index_map
{
    INT32             index;
    char            name[16];
}index_map;

/*
******************************************************************************
** Private variables
******************************************************************************
*/

struct epoll_event tsl_ev[TSL_EV_MAX];

PCB *pTslEvTask = NULL;
INT32 tsl_efd = 0;
INT32 tsl_xfd = 0;

UINT32 ev_check_counter;
volatile INT32 gNfds = 0;
volatile UINT8 g_tsl_events_Ready = 0;

#ifdef FRONTEND
ISCSI_TPD gTPD[MAX_TARGETS];
INT32 tsl_netlink_socket = -1;
UINT8 iscsiAddRoute = 0;
index_map iface_map[8];
INT32 update_map = 0;

extern INT32 convertTargetNameInTo64bit(UINT8 *string,INT32 size, UINT64 *pTargetName);
static const char *p2n[MAX_FE_PORTS * 2];
extern UINT32 ispPow;
extern PCB   *isplepcb[MAX_PORTS];        /* Port loop event PCB + ICL             */
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern unsigned long CT_fork_tmp;
extern INT32 CT_xiofd;

#ifdef FRONTEND

extern TGD     *T_tgdindx[MAX_TARGETS];
extern void CT_LC_tsl_init(void);
extern void ISP_LoopDown(UINT32);
extern void iSNS_Init(void);

extern void CT_LC_tsl_events(void);
extern void CT_LC_tsl_netlink_event_task (void);
extern void CT_LC_isp_portOnlineHandler(int);
extern void CT_LC_tsl_arpTask(UINT8);
extern void CT_LC_ISCSI_LoopUp(UINT8);
extern void CT_LC_ISCSI_LoopDown(UINT8);
extern PCB     *isponpcb;                   /* Port online handler PCB       */
extern UINT32  onlineTimer[MAX_PORTS];


#endif

void tsl_ev_check(UINT8 wait, UINT8 force);
void tsl_events(void);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

#ifdef FRONTEND
void tsl_cleanupEvents(void* pRef);
void tsl_cb_accept  (UINT32 events, void *pRef);
void tsl_cb_event   (UINT32 events, void *pRef);
void tsl_cb_write   (UINT32 events, void *pRef);

void tsl_cb_netlink (UINT32 events, void *pRef);

INT32 tsl_netlink_event_init (void);
INT32 tsl_netlink_recvmsg (void);

INT32 tsl_netlink_init (void);
INT32 tsl_netlink_modIP (const char* device, INT32 newIP, INT32 msgType, INT32 mask);
INT32 tsl_netlink_send(const char *device, INT32 socketfd, INT32 newIP, INT32 msgType, INT32 mask);
INT32 tsl_netlink_getIndex(const char *name);
INT32 tsl_netlink_recv (INT32 socketfd);
INT32 tsl_netlink_update_index(struct nlmsghdr *nlmsg);
INT32 tsl_send_arp (UINT8 port, UINT32 addr);

INT32 tsl_iprule(INT32 ipAddr, UINT8 table, UINT8 cmdType);
INT32 tsl_modifyRoute(INT32 ipAddr, UINT8 prefix, UINT8 portIndex, UINT8 cmdType, UINT8 table);
INT32 tsl_netlink_addRoute(INT32 ipAddr, UINT8 prefix, INT32 gateWay, INT32 msgType, UINT8 port);

void ISCSI_LoopUp (UINT32 a, UINT32 b, UINT8 port);
void ISCSI_LoopDown (UINT32 a, UINT32 b, UINT8 port);
INT32 tsl_arpDelete(UINT8 port, UINT32 addr);
INT32 tsl_set_mtu(UINT8 port, INT32 mtu_size);
INT32 tsl_get_mtu(UINT8 port);
void tsl_arpTask(UINT32 a, UINT32 b, UINT8 port);

UINT32 arpPCB[MAX_PORTS  +   MAX_ICL_PORTS];
UINT32 LoopUp [MAX_PORTS +   MAX_ICL_PORTS];
UINT32 LoopDown [MAX_PORTS + MAX_ICL_PORTS];
void tsl_ifsetup(UINT8 port);
UINT32 tsl_getIP (char* device);
UINT32 tsl_updateTxQueLen(UINT8 port);

INT32 tsl_getPort(UINT8* portName);
void tsl_getPortName(UINT8* buf, UINT8* portName);

void    I_recv_online(UINT32 port, UINT32 lid, UINT32);
INT32 tsl_daemonInit (void);
void tsl_cb_acpt_daemon (UINT32 events, void *pRef);
void tsl_cb_daemon_events (UINT32 events, void *pRef);
#define ISCSI_PROC_DAEMON_COMM         3261
#endif

/*
******************************************************************************
** Code Start
******************************************************************************
*/

#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Updates Local ARP entry, Sends Gratious ARP
**              Initializes the socket for a Target
**
**  @param      UINT32  - UNUSED
**              UINT32  - UNUSED
**              UINT32  - port
**
**  @return     none
**
******************************************************************************
**/

void tsl_ifsetup(UINT8 port)
{
    TAR *pTar;
    UINT8 portIndex;

    /* Very unlikely, but if a task switch occurs, the tar[] list might be invalid. */
 restart:
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if (tsl_IsPortUp(port))
        {
            /* Update Local ARP Entry */
            tsl_arpUpdate(port, pTar->ipAddr);

            if (BIT_TEST(iscsiAddRoute, pTar->tid))
            {
                if (port == ICL_PORT)
                {
                     portIndex = tsl_netlink_getIndex(iclIfrName);
                }
                else
                {
                     portIndex = tsl_netlink_getIndex(p2n[port]);
                }

                if (T_tgdindx[pTar->tid] &&
                    T_tgdindx[pTar->tid]->prefOwner == K_ficb->cSerial &&
                    T_tgdindx[pTar->tid]->prefPort == port)
                {
                    tsl_netlink_addRoute(0, 0, pTar->ipGw, RTM_NEWROUTE, port);
                }
                tsl_modifyRoute(pTar->ipAddr, pTar->ipPrefix, portIndex, RTM_NEWROUTE, port+2);
                tsl_modifyRoute(pTar->ipAddr, pTar->ipPrefix, portIndex, RTM_DELROUTE, RT_TABLE_MAIN);
                tsl_iprule(pTar->ipAddr, port+2, RTM_NEWRULE);
                BIT_CLEAR(iscsiAddRoute, pTar->tid);
            }

            /* Update the TCP TxQueueLen */
            tsl_updateTxQueLen(port);

            /* NOTE: may task switch -- thus allow for s_Free() invalidating tar list. */
            UINT32 save_tar_link_abort = tar_link_abort[port];

            /* If task is being created, wait. */
            while (arpPCB[port] == (UINT32)-1)
            {
                TaskSleepMS(50);
            }

            /* Create Task for sending ARP */
            if (arpPCB[port] == 0x0)
            {
                void *tmp_addr;
                arpPCB[port] = -1;      // Flag task being created.
                CT_fork_tmp = (unsigned long)"tsl_arpTask";
                tmp_addr = TaskCreate3(C_label_referenced_in_i960asm(tsl_arpTask), TSL_ARP_TASK, port);
                arpPCB[port] = (UINT32)tmp_addr;
            }

            /* If tar list got smaller, then the linked list may not be valid, restart. */
            if (save_tar_link_abort != tar_link_abort[port])
            {
                goto restart;
            }
        }
    }
}/* tsl_ifsetup */

/**
******************************************************************************
**
**  @brief      Sends an ARP
**
**  @param      UINT32  - UNUSED
**              UINT32  - UNUSED
**              UINT8   - port
**
**  @return     none
**
**  @attention  This is a Task
**
******************************************************************************
**/

void tsl_arpTask(UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    int  arp_retry_count = 120;
    int  i;
    TAR *pTar;

    for (i = 0; i < arp_retry_count; i++)
    {
        if (arpPCB[port] == 0x0)
        {
            break;
        }

        for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
        {
            if (pTar->ipAddr != 0x0)
            {
                tsl_send_arp(port, pTar->ipAddr);
            }
        }

        /* Sleep, between sending ARPs */
        TaskSleepMS(500);
    }
    arpPCB[port] = 0;
}
#endif /* FRONTEND */

/**
******************************************************************************
**
**  @brief      Creates an TSL Task,
**              which handles Send/Receive of ISCSI PDU's
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void tsl_init(void)
{
    struct epoll_event ev;
    INT32 ret = 0;

    fprintf(stderr,"ISCSI_DEBUG: @@@@@@ TSL Task is Initialized @@@@@@@@@@@@\n");

#ifdef  FRONTEND
    {
        int i;
        struct pci_devs *dev;

        for (i = 0; i < MAX_FE_PORTS; ++i)
        {
            dev = LI_GetPCIdev(i);
            if (!dev || dev->busdevfn != ENET_BUSDEVFN)
            {
                p2n[i] = p2n[i + MAX_FE_PORTS] = "nodev";
            }
            else
            {
                p2n[i] = p2n[i + MAX_FE_PORTS] = dev->enetname;
            }
            fprintf(stderr, "tsl_init: Port %d is %s\n", i, p2n[i]);
        }
    }
#endif  /* FRONTEND */

    /*
    ** Open Epoll File Descriptor
    */
    if ((tsl_efd = epoll_create(TSL_EV_MAX)) == -1)
    {
         fprintf(stderr, "Error: epoll_create failed\n");
         return;
    }

    ev.events = EPOLLIN;
    ev.data.u32 = 0xFFFFFFFF;

    /*
    ** Open Epoll xio3d only File Descriptor
    */
    if((tsl_xfd = epoll_create(TSL_EV_MAX)) == -1)
    {
         fprintf(stderr,"ISCSI_DEBUG: Error: epoll_create xio3d failed\n");
            return;
    }

    /*
    ** control interface for xio3d fd
    */
    if((ret = epoll_ctl(tsl_xfd, EPOLL_CTL_ADD, CT_xiofd, &ev)) == -1)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: epoll_ctl(tsl_efd, EPOLL_CTL_ADD, CT_xiofd, &ev) (%x)\n",ret);
    }

    /*
    ** control interface for epoll fd
    */
    if((ret = epoll_ctl(tsl_efd, EPOLL_CTL_ADD, CT_xiofd, &ev)) == -1)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: epoll_ctl(tsl_efd, EPOLL_CTL_ADD, CT_xiofd, &ev) (%x)\n",ret);
    }

#ifdef FRONTEND
    /*
    ** Initializes the netlink socket,
    ** Updates Mapping Index for an Interface
    */
    tsl_netlink_init ();

    /*
    ** Initializes the netlink socket,
    ** Monitors the Interface for LoopUP and LoopDown events
    */
    tsl_netlink_event_init ();

    BIT_SET(K_ii.status, II_ISCSI);

{
    FILE* fstream = NULL;
    char buff[50] = {0};
    UINT32 ipAddr;

    ipAddr = tsl_getIP((char*)"iscsi0");
    if(ipAddr)
        tsl_netlink_modIP("iscsi0", ipAddr, RTM_DELADDR, 24);

    sprintf(buff,"/etc/sysconfig/network/ifcfg-iscsi0");
    fstream = fopen(buff,"w");
    if(fstream)
    {
        fclose(fstream);
        unlink(buff);
    }

    ipAddr = tsl_getIP((char*)"iscsi1");
    if(ipAddr)
        tsl_netlink_modIP("iscsi1", ipAddr, RTM_DELADDR, 24);
    sprintf(buff,"/etc/sysconfig/network/ifcfg-iscsi1");
    fstream = fopen(buff,"w");
    if(fstream)
    {
        fclose(fstream);
        unlink(buff);
    }

    ipAddr = tsl_getIP((char*)"icl0");
    if(ipAddr)
        tsl_netlink_modIP("icl0", ipAddr, RTM_DELADDR, 24);
    sprintf(buff,"/etc/sysconfig/network/ifcfg-icl0");
    fstream = fopen(buff,"w");
    if(fstream)
    {
        fclose(fstream);
        unlink(buff);
    }

}
#endif
    /*
    ** Create Task for TSL
    */
    CT_fork_tmp = (unsigned long)"tsl_events";
    pTslEvTask = TaskCreate2(C_label_referenced_in_i960asm(tsl_events), TSL_EV_PRI);
#ifdef FRONTEND

    /*
    ** Initialize socket for Managing Events from iSCSi Daemon
    */
    tsl_daemonInit ();

    /*
    ** Create the timer task
    */
    CreateTimerTask();

    /*
    ** iSNS Initialization
    */
    iSNS_Init();

#endif
}


/**
******************************************************************************
**
**  @brief      checks if any events on epoll,
**              if any event wakes the TSL Task
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void tsl_ev_check(UINT8 wait, UINT8 force)
{

    INT32 i    = 0;
    INT32 nfds = 0;
    INT32   found_tsl_event;
    struct epoll_event tmp_tsl_ev[TSL_EV_MAX];

    if ( pTslEvTask != NULL && (force || TaskGetState (pTslEvTask) != PCB_READY))
    {
        ev_check_counter++;
        if (wait)
        {
            while (1)
            {
                /*
                ** Are we ready for tsl_events.
                */
                if ( g_tsl_events_Ready )
                {
                    if ( (nfds = epoll_wait(tsl_efd,tmp_tsl_ev,TSL_EV_MAX,-1)) >= 0 )
                    {
                        break;
                    }
                }

                /*
                ** Only wait on xio3d events.
                */
                else if ( (nfds = epoll_wait(tsl_xfd,tmp_tsl_ev,TSL_EV_MAX,-1)) >= 0 )
                {
                    return;
                }

                /*
                ** If we were interrupted, continue.
                */
                if ( errno == EINTR )
                    continue;

                break;
            }
        }

        /*
        ** If the task is not ready, proceed.
        */
        else
        {
            nfds = epoll_wait(tsl_efd,tmp_tsl_ev,TSL_EV_MAX,0);
        }

        gNfds = 0;
        if ( nfds > 0 )
        {
            found_tsl_event = 0;
            for(i = 0; i < nfds; i++)
            {
                if ( tmp_tsl_ev[i].data.u32 != 0xFFFFFFFF )
                {
                    found_tsl_event = 1;
                    break;
                }
            }

            if (found_tsl_event)
            {

                memcpy(tsl_ev, tmp_tsl_ev, (sizeof(struct epoll_event)*TSL_EV_MAX));
                gNfds = nfds;

                if (TaskGetState(pTslEvTask) == PCB_NOT_READY)
                {
#ifdef HISTORY_KEEP
CT_history_pcb("tsl_ev_check setting ready pcb", (UINT32)(pTslEvTask));
#endif
                    TaskSetState(pTslEvTask, PCB_READY);
                }
            }
        }
    }
}


/**
 ******************************************************************************
 **
 **  @brief      Checks for the epoll events,
 **              Updates the Interface Mapping Index through netlink,
 **              Initializes the netlink socket for Interface Monitoring
 **
 **  @param      none
 **
 **  @return     none
 **
 **  @attention  This is a task
 **
 ******************************************************************************
 **/
NORETURN
        void tsl_events(void)
{
    struct tsl_ev_dat *ptDat;
    void (*cb)(UINT32,void *);
    INT32 i    = 0;
    INT32 nfds = 0;
    UINT32  hold_ev_check_counter;
    while (!((K_ii.status & (1 << II_SERVER)) &&
              (K_ii.status & (1 << II_VDMT)) &&
              (K_ii.status & (1 << II_SN))))
    {
        TaskSleepMS (125);
    }

    g_tsl_events_Ready = 1;

    while(1)
    {
        /*
         * Call tsl_ev_check to ensure the most current list. Note that if
         * gNfds is zero, we have wakened to find no active events, so it isn't
         * necessary to check again.
         */

        if (gNfds)
        {
            tsl_ev_check (0, 1);
        }

        /*
        ** Set Task to wait for events only if there are none to process.
        */

        if (gNfds == 0)
        {
            TaskSetMyState(PCB_NOT_READY);
        }
        TaskSwitch();

        /*
         * Call tsl_ev_check to ensure the most current list. It is possible to
         * "lose" events (if a connection with an active event is closed, for
         * example), so if there are no events now, just loop again.
         */

        tsl_ev_check (0, 1);
        if (gNfds == 0)
        {
            continue;
        }
        hold_ev_check_counter = ev_check_counter;
        nfds = gNfds;

        /*
        ** Handle the epoll events. If a callback does a taskswitch with this
        ** task not PCB_READY, then the tsl_ev_check can change the tsl_ev array.
        ** So, if the tsl_ev_check routine runs during a callback, end this
        ** loop and return to the main loop to get the new tsl_ev array.
        */
        for(i = 0; i < nfds && hold_ev_check_counter == ev_check_counter; i++)
        {
            if ( tsl_ev[i].data.u32 == 0xFFFFFFFF )
            {
                continue;
            }

            ptDat = (struct tsl_ev_dat *)&(tsl_ev[i].data.u64);

            if((cb = ptDat->cb) != NULL)
            {
                /*
                ** Event Callback
                */
                (*cb)(tsl_ev[i].events, ptDat->pRef);
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Add Socket Descriptor to an epoll,
**              registers the callback for this socket
**
**  @param      INT32  - Socket Descriptor
**              UINT32 - epoll events (EPOLLIN/EPOLLOUT)
**              void*  - Callback Function pointer
**              void*  - Reference Pointer (ISCSI_TPD)
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_ev_add(INT32 fd, UINT32 events, void *func, void *pRef)
{
    struct tsl_ev_dat *ptDat;
    struct epoll_event ev;
    INT32 ret = 0;

    ev.events      = events;

    /*
    ** Update callback function pointer and Reference Pointer in epoll
    */
    ptDat = (struct tsl_ev_dat *)&(ev.data.u64);
    ptDat->cb   = func;
    ptDat->pRef = pRef;

    /*
    ** Add to epoll
    */

    if((ret = epoll_ctl(tsl_efd, EPOLL_CTL_ADD, fd, &ev)) == -1)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: tsl_ev_add (%x) %d  %s\n",ret,errno,strerror(errno));
    }
    return (ret);
}

/**
******************************************************************************
**
**  @brief      Deletes Socket Descriptor from an epoll,
**
**  @param      INT32  - Socket Descriptor
**              UINT32 - epoll events (EPOLLIN/EPOLLOUT)
**              void*  - Callback Function pointer
**              void*  - Reference Pointer (ISCSI_TPD)
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_ev_del(INT32 fd, UINT32 events, void *func UNUSED, void *pRef)
{
    struct tsl_ev_dat *ptDat;
    struct epoll_event ev;
    INT32 ret = 0;

    ev.events = events;

    ptDat = (struct tsl_ev_dat *)&(ev.data.u64);
    ptDat->cb   = NULL;
    ptDat->pRef = NULL;

    /*
    ** Delete from epoll
    */
    if((ret = epoll_ctl(tsl_efd, EPOLL_CTL_DEL, fd, &ev)) == -1)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: tsl_ev_del (%x) socket fd = %d, TPD = %x\n",ret, fd, (UINT32)pRef);
    }
    return (ret);
}

/**
******************************************************************************
**
**  @brief      Modify epoll events for a socket
**
**  @param      INT32  - Socket Descriptor
**              UINT32 - epoll events (EPOLLIN/EPOLLOUT)
**              void*  - Callback Function pointer
**              void*  - Reference Pointer (ISCSI_TPD)
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_ev_mod(INT32 fd, UINT32 events, void *func, void *pRef)
{
    struct tsl_ev_dat *ptDat;
    struct epoll_event ev;
    INT32 ret = 0;

    ev.events      = events;

    /*
    ** Update callback function pointer and Reference Pointer in epoll
    */
    ptDat = (struct tsl_ev_dat *)&(ev.data.u64);
    ptDat->cb   = func;
    ptDat->pRef = pRef;

    /*
    ** Modify epoll events
    */
    if((ret = epoll_ctl(tsl_efd, EPOLL_CTL_MOD, fd, &ev)) == -1)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: tsl_ev_mod (%x)\n",ret);
    }
    return (ret);
}

#ifdef FRONTEND

/**
******************************************************************************
**
**  @brief      Callback Function, this is a listen socket
**              Any Event on this socket accepts new connection
**
**  @param      UINT32  - events
**              void*   - Reference Pointer
**
**  @return     none
**
******************************************************************************
**/
void tsl_cb_accept(UINT32 events UNUSED, void *pRef)
{
    ISCSI_TPD *pTPD = (ISCSI_TPD*)pRef;

    if(pTPD == NULL)
    {
        return;
    }

    if (iclPortExists)
    {
        if((ispPortAssignment[pTPD->tid] >= MAX_PORTS)
            && (ispPortAssignment[pTPD->tid] != ICL_PORT))
        {
           return;
        }
    }
    else
    {
        if(ispPortAssignment[pTPD->tid] >= MAX_PORTS)
        {
            return;
        }
    }

    if(pTPD->pConn == NULL)
    {
        /*
        ** Validate to accept New Connection
        */
        iscsiHandleTpEvt((ISCSI_TPD*)pRef, TSL_ISCSI_EVENT_ACCEPT);
    }
    else
    {
        UINT8 port;
        TAR *pTAR = NULL;

        fprintf(stderr,"ISCSI_DEBUG: ERROR!!! ACCEPT EVENT with ON EXISTING CONNECTION\n");
        if((pTAR = fsl_get_tar(pTPD->tid)) != NULL)
        {
            /*
            ** Invalid State!!!! Something very BAD happened!!! Reset & init
            ** the port to which this target is attached
            */
            if(iclPortExists)
            {
                if((port = ispPortAssignment[pTPD->tid]) < MAX_PORTS || (port == ICL_PORT))
                {
                     fsl_ResetPort(port, ISP_RESET_AND_INIT);
                }
            }
            else
            {
                if((port = ispPortAssignment[pTPD->tid]) < MAX_PORTS)
                {
                    fsl_ResetPort(port, ISP_RESET_AND_INIT);
                }
            }
        }
    }
}

/**
******************************************************************************
**
**  @brief      Callback Function,
**              Event on this socket are an ISCSI PDU Received
**              or an Close Connection
**
**  @param      UINT32  - events
**              void*   - Reference Pointer
**
**  @return     none
**
******************************************************************************
**/
void tsl_cb_event(UINT32 events, void *pRef)
{
    ISCSI_TPD *pTPD = (ISCSI_TPD*)pRef;
    UINT8       result = XIO_SUCCESS;

    if(pTPD == NULL)
    {
        return;
    }

    if (iclPortExists)
    {
        if((ispPortAssignment[pTPD->tid] >= MAX_PORTS)
            && (ispPortAssignment[pTPD->tid] != ICL_PORT))
        {
           return;
        }
    }
    else
    {
        if(ispPortAssignment[pTPD->tid] >= MAX_PORTS)
        {
            return;
        }
    }
    /*
     * First, process the input events, since that may create an output.
     */

    pTPD->send_indicator = 0;
    if (events & EPOLLIN)
    {
        if(pTPD->pConn != NULL)
        {
            result = iscsiHandleTpEvt(pTPD, TSL_ISCSI_EVENT_READ);
        }
        else
        {
            UINT8 port;
            TAR *pTAR = NULL;

            fprintf(stderr,"ISCSI_DEBUG: ERROR!!! READ EVENT with NO CONNECTION\n");
            if((pTAR = fsl_get_tar(pTPD->tid)) != NULL)
            {
                /*
                ** Invalid State!!!! Something very BAD happened!!! Reset & init
                ** the port to which this target is attached
                */
                if (iclPortExists)
                {
                    if((port = ispPortAssignment[pTPD->tid]) < MAX_PORTS || (port == ICL_PORT))
                    {
                       fsl_ResetPort(port, ISP_RESET_AND_INIT);
                    }
                }
                else
                {
                    if((port = ispPortAssignment[pTPD->tid]) < MAX_PORTS)
                    {
                        fsl_ResetPort(port, ISP_RESET_AND_INIT);
                    }
                }
            }
            return;
        }
    }

    /*
     * Now, do the send if we have an output event, or if we generated something
     * to send when processing the input event. Note that input event processing
     * may result in the connection closing, so get the connection pointer from the
     * session structure (it will be NULL if it closed).
     */

    if (result != XIO_CONN_CLOSE && (pTPD->send_indicator || (events & EPOLLOUT)))
    {
        /*
         * If we received an output event, clear any partial send indication
         */

        if (events & EPOLLOUT)
        {
            if (pTPD->pConn == NULL)
            {
                fprintf (stderr, "**************** pTPD->pConn is NULL **************\n");
                return;
            }
            pTPD->pConn->send_state = 0;
        }
        result = iscsi_send (pTPD);     /* NOTE: can close connection here. */
    }
    if (result != XIO_CONN_CLOSE)
    {
        pTPD->send_indicator = 2;
    }
}


/**
******************************************************************************
**
**  @brief      Callback function for netlink
**              Events are LoopUp or LoopDown
**
**  @param      UINT32  - events
**              void*   - Reference Pointer
**
**  @return     none
**
******************************************************************************
**/
void tsl_cb_netlink (UINT32 events UNUSED, void *pRef UNUSED)
{
    tsl_netlink_recvmsg ();
}


/**
******************************************************************************
**
**  @brief      Add an output event to the events we are interested in.
**
**  @param      UINT32  - events
**              void*   - Reference Pointer
**
**  @return     none
**
******************************************************************************
**/
void tsl_add_output_event (ISCSI_TPD *pTPD)
{
    /*
    ** Modify epoll event to Write
    */
    if (pTPD == NULL)
    {
        fprintf (stderr, "**************** pTPD is NULL **************\n");
        return;
    }
    tsl_ev_mod(pTPD->socket_fd, EPOLLIN | EPOLLOUT, tsl_cb_event, (void *)pTPD);
    pTPD->write_event_enabled++;
}

/**
******************************************************************************
**
**  @brief      Remove the output event from the events we are interested in.
**
**  @param      UINT32  - events
**              void*   - Reference Pointer
**
**  @return     none
**
******************************************************************************
**/
void tsl_remove_output_event (ISCSI_TPD *pTPD)
{
    /*
    ** Modify epoll event to Write
    */
    if (pTPD == NULL)
    {
        fprintf (stderr, "**************** pTPD is NULL **************\n");
        return;
    }
    tsl_ev_mod(pTPD->socket_fd, EPOLLIN, tsl_cb_event, (void *)pTPD);
    pTPD->write_event_enabled = 0;
}

#endif
/**
******************************************************************************
**
**  @brief      Set the Socket to Non Blocking
**
**  @param      INT32  - Socket Descriptor
**
**  @return     none
**
******************************************************************************
**/
void TSL_SetNonBlocking(INT32 sock)
{
    INT32 retVal = 0;

    /*
    ** Get FLAGS
    */
    retVal = fcntl(sock, F_GETFL);
    if (retVal != -1)
    {
        /*
        ** Set FLAGS
        */
        retVal = fcntl(sock, F_SETFL, retVal | O_NONBLOCK);
        if (retVal)
            fprintf(stderr, "Error: unable to set socket to Non Blocking\n");
    }
    else
    {
        fprintf(stderr, "Error:  unable to get fd flags\n");
    }
}
#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      Initialize all the iSCSI structures pertaining to the port.
**
**  @param      ISCSI_TPD  -  Pointer to TPD
**
**  @return     ISCSI_TPD  -  Pointer to TPD
**
******************************************************************************
**/
ISCSI_TPD* tsl_AcptConnection (ISCSI_TPD *pTPD)
{
    struct sockaddr_in client_addr;
    struct ISCSI_TPD* pNewTPD;
    socklen_t addr_len = sizeof(client_addr);
    INT32    new_socket_fd;
    INT32     opt_val=1;
    INT32     retVal = 0;
    socklen_t len;

    /*
    ** Accept New Connection
    */
    new_socket_fd = accept(pTPD->socket_fd,
        (struct sockaddr*)&client_addr, &addr_len);
    if (new_socket_fd < 0)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: accept failed\n");
        return NULL;
    }
    len = sizeof(opt_val);

    /*
    ** Set TCP_NODELAY
    */
    opt_val = 1;
    retVal = setsockopt(new_socket_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set tcp no delay\n");

#if 0
    /* we shouldn't need this.  if you think we do, please send a test case to sferris. */
    /*
    ** Set TCP_QUICKACK
    */
    opt_val = 1;
    retVal = setsockopt(new_socket_fd, IPPROTO_TCP, TCP_QUICKACK, (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set tcp quickack\n");
#endif


#if 0
    /* We don't set socket buffer sizes, since doing so disables the kernel's autotuning.
     * Instead, adjust the min, default, and max in /etc/sysctl.conf.
     */

    /*
    ** Update TCP Send Buffer Size
    */
    opt_val = TSL_TCP_SEND_BUF_MAX;
    len = sizeof(opt_val);
    retVal = setsockopt(new_socket_fd, SOL_SOCKET, SO_SNDBUF , (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set send buffer\n");

    /*
    ** Update TCP Recv Buffer Size
    */
    opt_val = TSL_TCP_RECV_BUF_MAX;
    len = sizeof(opt_val);
    retVal = setsockopt(new_socket_fd, SOL_SOCKET, SO_RCVBUF , (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set recv buffer\n");
#endif

    pNewTPD = (struct ISCSI_TPD*)s_MallocC(sizeof(struct ISCSI_TPD), __FILE__, __LINE__);

    /*
    ** Update ISCSI_TPD structure
    */
    pNewTPD->socket_fd  = new_socket_fd;
    pNewTPD->tid        = pTPD->tid;

    /*
    ** Set Socket to Non Blocking
    */
    TSL_SetNonBlocking (new_socket_fd);

    /*
    ** Add socket to epoll
    */
    tsl_ev_add(new_socket_fd, EPOLLIN, tsl_cb_event, (void *)pNewTPD);
    return pNewTPD;
}

/**
******************************************************************************
**
**  @brief      Interface between ISCSI and TSL
**
**  @param      ISCSI_TPD  -  Pointer to TPD
**              INT32      -  Event Type
**              char*      -  Buffer Pointer
**              INT32      -  Buffer Length
**
**  @return     none
**
******************************************************************************
**/
INT32 ISCSI_TSL_Event_Notify (ISCSI_TPD *pTPD, INT32 Event, char* buffer, INT32 length)
{
    INT32 retVal = 0;

    switch (Event)
    {
        case TSL_ISCSI_EVENT_ACCEPT_PROCESS:
        {
            ISCSI_TPD *tmp_addr;
            tmp_addr = tsl_AcptConnection (pTPD);
            retVal = (INT32)tmp_addr;
            break;
        }
        case TSL_ISCSI_EVENT_ACCEPT_REJECT:
        {
            retVal = tsl_RejectConnection (pTPD->socket_fd);
            break;
        }
        case TSL_ISCSI_EVENT_ACCEPT_RESET:
        {
            break;
        }
        case TSL_ISCSI_EVENT_READ:
        {
            retVal = tsl_recv_data (pTPD, buffer, length);
            break;
        }
        case TSL_ISCSI_EVENT_CLOSE_CONN:
        {
            retVal = tsl_CloseConnection (pTPD);
            break;
        }
        default:
            break;
    }
    return retVal;
}

/**
******************************************************************************
**
**  @brief      Initialize all the socket functionality
**
**  @param      UINT32     - IP Address
**              UINT16     - Target ID
**              UINT8      - Port Number
**
**  @return     INT32      - Socket Descriptor or Failure
**
******************************************************************************
**/
INT32 tsl_InitConnection (UINT32 ip_addr, UINT16 tid, UINT8 port)
{
    struct sockaddr_in serv_addr;
    INT32     socket_fd;
    INT32     retVal;
    INT32     opt_val=1;
    socklen_t len;

    fprintf(stderr,"ISCSI_DEBUG: tsl_InitConnection: ip addr = %x, tid = %d, port = %d\n",ip_addr, tid, port);
    memset(&gTPD[tid], 0, sizeof(ISCSI_TPD));

    /*
    ** Create Socket
    */
#if ICL_DEBUG
    if (port == ICL_PORT)
    {
        fprintf(stderr,"<%s:%s>  ICL port .. creating the socket\n",__FILE__,__func__);
    }
#endif
    socket_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error Creating socket\n");
        return -1;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = ip_addr;
    serv_addr.sin_port        = htons(TSL_DEFAULT_ISCSI_PORT);

    /*
    ** Set Socket to SO_REUSEADDR
    */
    len = sizeof(opt_val);
    retVal = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set socket to reuse\n");

    /*
    ** Set TCP_NODELAY
    */
    opt_val = 1;
    retVal = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set tcp no delay\n");

#if 0
    /* we shouldn't need this.  if you think we do, please send a test case to sferris. */
    /*
    ** Set TCP_QUICKACK
    */
    opt_val = 1;
    retVal = setsockopt(socket_fd, IPPROTO_TCP, TCP_QUICKACK, (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set tcp quickack\n");
#endif

#if 0
    /* We don't set socket buffer sizes, since doing so disables the kernel's autotuning.
     * Instead, adjust the min, default, and max in /etc/sysctl.conf.
     */

    /*
    ** Update TCP Send Buffer Size
    */
    opt_val = TSL_TCP_SEND_BUF_MAX;
    len = sizeof(opt_val);
    retVal = setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF , (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set send buffer\n");

    /*
    ** Update TCP Recv Buffer Size
    */
    opt_val = TSL_TCP_RECV_BUF_MAX;
    len = sizeof(opt_val);
    retVal = setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF , (void *)&opt_val, len);
    if (retVal < 0)
        fprintf(stderr,"ISCSI_DEBUG: Could not set recv buffer\n");
#endif

    /*
    ** Bind
    */
    retVal = bind (socket_fd, (struct sockaddr*)&serv_addr, sizeof (serv_addr));
    if (retVal < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error Bind on socket\n");
        close(socket_fd);
        return -1;
    }

    /*
    ** listen
    */
    retVal = listen (socket_fd, TSL_MAX_CONNECTIONS);
    if(retVal < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error listen, value = %d\n",retVal);
        close(socket_fd);
        return -1;
    }

    /*
    ** Set socket to Non Blocking
    */
    TSL_SetNonBlocking (socket_fd);

    /*
    ** Update TPD
    */
    gTPD[tid].tid       = tid;
    gTPD[tid].socket_fd = socket_fd;

    /*
    ** Add socket to epoll
    */
#if ICL_DEBUG
    if (port == ICL_PORT)
    {
        fprintf(stderr,"<%s:%s>  ICL port .. adding socket to EPOLL\n",__FILE__,__func__);
    }
#endif
    tsl_ev_add(socket_fd, EPOLLIN, tsl_cb_accept,(void *)&gTPD[tid]);
    return socket_fd;
}

/**
******************************************************************************
**
**  @brief      Rejects the Connection
**
**  @param      INT32  -  Socket Descriptor
**
**  @return     INT32  -  SUCCESS
**
******************************************************************************
**/
INT32 tsl_RejectConnection (INT32 socket_fd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    INT32 new_socket_fd;

    addr_len = sizeof(client_addr);
    new_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_len);
    close (new_socket_fd);

    return 1;
}


/**
******************************************************************************
**
**  @brief      Close the Connection
**
**  @param      ISCSI_TPD  -  Pointer to TPD
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_CloseConnection(ISCSI_TPD* pTPD)
{

    if (pTPD == NULL)
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: tsl_CloseConnection Invalid Input Parameter\n");
        return 0;
    }

    /*
    ** Remove socket from  epoll
    */
    tsl_ev_del(pTPD->socket_fd,EPOLLIN,tsl_cb_event,(void *)pTPD);
    /*
    ** cleanup event from epoll
    */
    tsl_cleanupEvents((void*)pTPD);
    fprintf(stderr,"tsl_CloseConnection: TPD = %p  fd %d\n", pTPD, pTPD->socket_fd);
    close(pTPD->socket_fd);
    pTPD->socket_fd = -1;
    pTPD->pConn->pTPD = NULL;
    pTPD->pConn = NULL;
    s_Free(pTPD, sizeof(ISCSI_TPD), __FILE__, __LINE__);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Receives ISCSI Data Out PDUs
**
**  @param      ISCSI_TPD  - Pointer to TPD
**              SGL_DESC*  - Pointer to SGL Descriptor
**              INT32      - Length
**              UINT8      - SGL Count
**
**  @return     INT32      - Number of bytes received / Error
**
******************************************************************************
**/
INT32 tsl_recv_dataOut (ISCSI_TPD *pTPD, SGL_DESC* pSglDesc, INT32 length, UINT8 sglCount, UINT32 buffOffset)
{
    UINT8 i=0, j=0;
    INT32 retVal = 0;
    UINT32 offset;
    UINT8 iov_count = 0;
    INT32 digestlen=0;
    int LenInThisSGL=0;
    int tmpLength=length;

    struct iovec iov[16];
    struct msghdr msg;
    struct sockaddr_nl nladdr;

    /*
    ** Validate Input Params
    */
    if ((pTPD == NULL) || (pTPD->socket_fd < 0) || (pTPD->pConn == NULL) || (pTPD->pConn->send_state == ISCSI_CLOSE_CONN_STATE))
    {
        fprintf(stderr, "ISCSI_DEBUG: tsl_recv_dataOut: Invalid Input Params pTPD %p or socket fd is -1\n", pTPD);
        return -1;
    }

    offset = pTPD->pConn->offset + buffOffset;

    for(i=0; i<sglCount; i++)
    {
        if (pSglDesc->len < offset)
        {
            offset = offset - pSglDesc->len;
            pSglDesc++;
            j++;
        }
        else
            break;
    }

    for(i=0; i<(sglCount-j) && (length != 0); i++)
    {
        iov[i].iov_base = (void*)((char*)pSglDesc->addr + offset);
        {
            int readLen = length;

            LenInThisSGL = pSglDesc->len - offset;

            if(readLen > LenInThisSGL)
            {
                readLen = LenInThisSGL;
            }
            else
            {
                readLen = length;
            }
            length = length - readLen;

            iov[i].iov_len  = readLen;
        }
        iov_count++;
        pSglDesc++;
        offset = 0;
    }

    memset(&msg, 0 ,sizeof(msg));
    msg.msg_name        = (void*)&nladdr;
    msg.msg_namelen     = sizeof(nladdr);
    msg.msg_iov         = &iov[0];
    msg.msg_iovlen      = iov_count;

    length = tmpLength;
    if(length > 0)
    {
        retVal = recvmsg(pTPD->socket_fd, &msg, 0);
    }
    if (retVal > 0 || length == 0)
    {
        /*
        ** update number of write bytes
        */
        pTPD->pConn->totalWrites += retVal;

        /*
        ** check if offset == recvLen it means we have read the complete data out
        ** if digest is enabled we should read digest
        ** we will use headeDigest field to keep data digest.
        */
        pTPD->pConn->offset += retVal;
        if(pTPD->pConn->offset == pTPD->pConn->recvLen)
        {
            if(stringCompare(pTPD->pConn->params.dataDigest.strval,(UINT8*) "CRC32C") == 0  )
            {
                digestlen = tsl_recv_data (pTPD,(char *)((ISCSI_GENERIC_HDR*)(pTPD->pConn->pHdr))->headerDigest +
                        (pTPD->pConn->recvState - IR_RECV_DATA),
                        4-(pTPD->pConn->recvState - IR_RECV_DATA));
                if(digestlen < 0)
                {
                    fprintf(stderr,"ISCSI_DEBUG: While reading digest error encounterd\n");
                    return digestlen;
                }else
                {
                    pTPD->pConn->recvState += digestlen;

                }

            }
            else
            {
                pTPD->pConn->recvState = IR_COMP;
            }
        }
        else
        {
            pTPD->pConn->recvState = IR_RECV_DATA;
        }


        return retVal;
    }
    else if (retVal == 0){
        fprintf(stderr,"ISCSI_DEBUG: tsl_recv_dataOut: fd %d recv zero bytes, error %d %s (length=%d)\n",
                pTPD->socket_fd, errno, strerror(errno), length);
        pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
        retVal = -1;
    }
    else if (retVal < 0)
    {
        /*
        ** receive error handling
        */
        switch(errno)
        {
            case EAGAIN:
            case EINTR:
                retVal = 0;
                break;
            case ECONNRESET:
            case ECONNABORTED:
            case ENETRESET:
            case ENETUNREACH:
            case ENETDOWN:
            case EHOSTUNREACH:
            case EPIPE:
            default:
                fprintf(stderr,"ISCSI_DEBUG: tsl_recv_dataOut fd %d error %d %s\n", pTPD->socket_fd, errno, strerror(errno));
                pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
                retVal = -1;
                break;
        }
    }
    return retVal;
}

/**
******************************************************************************
**
**  @brief      Receives ISCSI PDUs
**
**  @param      ISCSI_TPD  - Pointer to TPD
**              char*      - Data pointer
**              INT32      - Data Length
**
**  @return     INT32      - Number of bytes received / Error
**
******************************************************************************
**/
INT32 tsl_recv_data(ISCSI_TPD* pTPD, char* buffer, INT32 length)
{
    INT32 total = -1;
    INT32 retVal = 0;

    /*
    ** Validate Input Params
    */
    if ((pTPD == NULL) || (buffer == NULL) || (length <= 0) || (pTPD->socket_fd < 0) || (pTPD->pConn == NULL) || (pTPD->pConn->send_state == ISCSI_CLOSE_CONN_STATE)) {
        fprintf(stderr, "ISCSI_DEBUG: tsl_recv_data: Invalid Input Params pTPD %p buffer %p length %d\n",
                pTPD, buffer, length);
        return -1;
    }

    /*
    ** receive data
    */
    total = recv (pTPD->socket_fd, buffer, length, 0);

    if (total > 0)
    {
        retVal = total;
    }
    else if (total == 0){
        fprintf(stderr,"ISCSI_DEBUG: tsl_recv_data: recv zero bytes on fd=%d, pTPD=%p, tid = %d, length = %d\n",
                pTPD->socket_fd, pTPD, pTPD->tid, length);
        pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
        retVal = -1;
    }
    else if (total < 0)
    {
        /*
        ** receive error handling
        */
        switch(errno)
        {
            case ECONNRESET:
            case ECONNABORTED:
            case ENETRESET:
            case ENETUNREACH:
            case ENETDOWN:
            case EHOSTUNREACH:
            case EPIPE:
                fprintf(stderr,"ISCSI_DEBUG: tsl_recv fd=%d pTPD=%p length=%d buffer=%p, error %d %s\n",
                        pTPD->socket_fd, pTPD, length, buffer, errno, strerror(errno));
                pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
                retVal = -1;
                break;
            case EAGAIN:
            case EINTR:
                retVal = 0;
                break;
            default:
                fprintf(stderr,"ISCSI_DEBUG: tsl_recv default fd=%d pTPD=%p error %d %s\n",
                        pTPD->socket_fd, pTPD, errno, strerror(errno));
                retVal = -1;
                break;
        }
    }
    return retVal;
}


/**
******************************************************************************
**
**  @brief      Sends ISCSI_PDUs which are put into Queue
**
**  @param      ILT    - ILT pointer
**
**  @return     INT32  - Number of bytes sent / Error
**
******************************************************************************
**/
INT32 tsl_send(ILT* pILT)
{
    struct msghdr msg;
    struct iovec iov[3];
    ISCSI_TPD *pTPD;

    char tmpBuff[4];
    ILT* dataILT;
    INT32 flags=0;
    INT32 rVal=0;
    UINT8 iov_count = 0;

    dataILT = (ILT*)pILT + 1;
    memset(&msg, 0, sizeof (struct msghdr));
    msg.msg_iov = iov;
    msg.msg_flags  = MSG_NOSIGNAL;
    pTPD  = (ISCSI_TPD*)pILT->pi_pdu.w1;

    if((pTPD == NULL) || (pTPD->socket_fd == -1) || (pTPD->pConn == NULL) || (pTPD->pConn->send_state == ISCSI_CLOSE_CONN_STATE))
    {
        fprintf(stderr,"tsl_send: INVALID INPUT PARAMS:\n");
        pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
        return TSL_ERROR_ON_CONN;
    }

    // Header + Header Digest
    if (dataILT->pi_pdu.w1 != 0)
    {
        iov[iov_count].iov_base = (char*)(dataILT->pi_pdu.w0);
        iov[iov_count].iov_len = (UINT32)(dataILT->pi_pdu.w1);
        iov_count++;
    }

    // Data
    if (dataILT->pi_pdu.w3 != 0)
    {
        iov[iov_count].iov_base = (char*)(dataILT->pi_pdu.w2);
        iov[iov_count].iov_len  = (UINT32)(dataILT->pi_pdu.w3);
        iov_count++;
    }

    // Data Digest
    if(dataILT->pi_pdu.w5 != 0)
    {
        UINT32 digest = dataILT->pi_pdu.w4;
        memcpy(&tmpBuff, (char*)&(digest), 4);
        iov[iov_count].iov_base = (char*)&tmpBuff + (DIGEST_LENGTH - dataILT->pi_pdu.w5);
        iov[iov_count].iov_len  = (UINT32)dataILT->pi_pdu.w5;
        iov_count++;
    }

    msg.msg_iovlen = iov_count;
    flags |= MSG_NOSIGNAL;
    /*
    ** send data
    */
    rVal = sendmsg(pTPD->socket_fd, &msg, flags);
    if(rVal == (INT32)pILT->pi_pdu.len)
    {
        return rVal;
    }
    else if(rVal == -1)
    {
        /*
        ** send error handling
        */
        switch(errno)
        {
            case EAGAIN:
            case EINTR:
                rVal = TSL_TRY_AGAIN_CONN;
                break;
            case ECONNRESET:
            case ECONNABORTED:
            case ENETRESET:
            case ENETUNREACH:
            case ENETDOWN:
            case EHOSTUNREACH:
            case EPIPE:
            default:
                fprintf(stderr, "ISCSI_DEBUG: tsl_send fd %d error %d %s", pTPD->socket_fd, errno, strerror(errno));
                rVal = TSL_ERROR_ON_CONN;
                pTPD->pConn->send_state = ISCSI_CLOSE_CONN_STATE;
                break;
        }
    }
    else if(rVal < (INT32)pILT->pi_pdu.len)
    {
        // partial send
        pILT->pi_pdu.len = pILT->pi_pdu.len - rVal;

        if (rVal < (INT32)dataILT->pi_pdu.w1)
        {
             // update Header
             dataILT->pi_pdu.w1 = dataILT->pi_pdu.w1 - rVal;
             dataILT->pi_pdu.w0 = (UINT32)((char*)(dataILT->pi_pdu.w0) + rVal);
        }
        else
        {
             rVal = rVal - dataILT->pi_pdu.w1;
             dataILT->pi_pdu.w1 = 0;
             dataILT->pi_pdu.w0 = 0;

             if (rVal < (INT32)dataILT->pi_pdu.w3)
             {
                 // update data
                 dataILT->pi_pdu.w3 = dataILT->pi_pdu.w3 - rVal;
                 dataILT->pi_pdu.w2 = (UINT32)((char*)(dataILT->pi_pdu.w2) + rVal);
             }
             else
             {
                 rVal = rVal - dataILT->pi_pdu.w3;
                 dataILT->pi_pdu.w3 = 0;
                 dataILT->pi_pdu.w2 = 0;

                 // update digest
                 dataILT->pi_pdu.w5 = dataILT->pi_pdu.w5 - rVal;
             }
        }
        rVal = TSL_PARTIAL_SEND;
    }
    return rVal;
}/* tsl_send */

/**
******************************************************************************
**
**  @brief      Brings Down / UP an Interface
**              Sets ON / OFF for ARP Flags
**
**  @param      UINT8  - Port
**              UINT8  - Request Type
**
**  @return     SUCCESS / Error Value
**
******************************************************************************
**/

INT32 tsl_if_set(UINT8 port, UINT8 req)
{
    struct ifreq ifr;
    UINT32 mask = 0 , flags = 0;
    INT32 fd, err;

    switch(req)
    {
        case MY_IFUP:
            mask |= IFF_UP;
            flags |= IFF_UP;
            break;
        case MY_IFDOWN:
            mask |= IFF_UP;
            flags &= ~IFF_UP;
            break;
        case MY_ARPON:
            mask |= IFF_NOARP;
            flags &= ~IFF_NOARP;
            break;
        case MY_ARPOFF:
            mask |= IFF_NOARP;
            flags |= IFF_NOARP;
            break;
        default:
            break;
    }

    /*
    ** Adding ICL  Interface also along with other interfaces
    */
    if (ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if((err = ioctl(fd, SIOCGIFFLAGS, &ifr)) == 0)
    {
        if ((ifr.ifr_flags^flags) & mask)
        {
            ifr.ifr_flags &= ~mask;
            ifr.ifr_flags |= mask&flags;

            if((err = ioctl(fd, SIOCSIFFLAGS, &ifr)) != 0)
            {
                fprintf(stderr,"ISCSI_DEBUG: Error: ioctl SIOCSIFFLAGS failed\n");
            }
        }
    }
    else
    {
        fprintf(stderr,"ISCSI_DEBUG: Error: ioctl SIOCGIFFLAGS failed\n");
    }
    close(fd);
    return err;
}

/**
******************************************************************************
**
**  @brief      Receives the Netlink Message
**
**
**  @param      INT32 - Socket Descriptor
**
**  @return     0 - Success
**             -1 - Error
**
******************************************************************************
**/
INT32 tsl_netlink_recv (INT32 socketfd)
{
    INT32    retVal;
    char   buf[2048] = {0};
    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct nlmsghdr *nlmsg;

    struct msghdr msg;

    memset(&msg, 0 ,sizeof(msg));
    msg.msg_name        = (void*)&nladdr;
    msg.msg_namelen     = sizeof(nladdr);
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;

    iov.iov_base = (void*)&buf;
    iov.iov_len  = sizeof(buf);
    /*
    ** Receive message from Netlink
    */
    retVal = recvmsg(socketfd, &msg, 0);
    if (retVal <= 0)
    {
        return -1;
    }

    nlmsg = (struct nlmsghdr*)buf;

    /*
    ** Parse Till the end of Message
    */
    while (NLMSG_OK(nlmsg, (UINT32)retVal))
    {
        if (nlmsg->nlmsg_type == NLMSG_DONE)
        {
            return 0;
        }
        if (nlmsg->nlmsg_type == NLMSG_ERROR)
        {
            return -1;
        }

        /*
        ** Update Mapping for Interface name to Index
        */
        tsl_netlink_update_index(nlmsg);
        nlmsg = NLMSG_NEXT(nlmsg, retVal);
    }
    return 0;
}

/**
******************************************************************************
**
**  @brief      Builds a mapping between Interface name and Index
**
**
**  @param      struct nlmsghdr
**
**  @return     0 - Success
**
******************************************************************************
**/

INT32 tsl_netlink_update_index(struct nlmsghdr *nlmsg)
{
    struct ifinfomsg *ifi = NLMSG_DATA(nlmsg);
    struct rtattr *tb[IFLA_MAX+1];
    struct rtattr *rtat = IFLA_RTA(ifi);
    INT32 len = IFLA_PAYLOAD(nlmsg);

    memset(tb, 0, sizeof(tb));

    while (RTA_OK(rtat, len))
    {
        if (rtat->rta_type <= IFLA_MAX)
            tb[rtat->rta_type] = rtat;
        rtat = RTA_NEXT(rtat,len);
    }

    if (tb[IFLA_IFNAME] == NULL)
        return 0;

#if FE_ICL
    /*
    ** Don't copy into global variables when called first time during
    ** ICL identification. This gets invoked again during tsl_init()
    */

    if(iclIdentificationDone)
    {
#endif
        iface_map[update_map].index = ifi->ifi_index;
        strcpy(iface_map[update_map].name, RTA_DATA(tb[IFLA_IFNAME]));
        update_map++;
#if FE_ICL
    }
    else
    {
        /*
        ** Check if any ICL interface exists
        */
        if (strncmp(RTA_DATA(tb[IFLA_IFNAME]),"icl", 3) == 0)
        {
            iclPortExists = TRUE;
#if ICL_DEBUG
            fprintf(stderr,"<ICL>ICL port is identified....\n");
#endif
        }
    }
#endif

    return 0;
}

/**
******************************************************************************
**
**  @brief      Sends Netlink Message and Receives Netlink message,
**
**
**  @param      none
**
**  @return     0 - Success
**             -1 - Failure
**
******************************************************************************
**/

INT32 tsl_netlink_init (void)
{
    struct nlmsghdr     nlmsg;
    struct sockaddr_nl  local;
    struct sockaddr_nl  nladdr;

    INT32 socketfd = -1;
    INT32 retVal;
    char buf[17];

    /*
    ** Socket to Netlink
    */
    socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(socketfd, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        fprintf(stderr, "Error: Bind failed\n");
        return -1;
    }

    /*
    ** Update Message Header
    */
    memset(&nlmsg, 0, sizeof(nlmsg));
    nlmsg.nlmsg_len       = sizeof(nlmsg)+1;
    nlmsg.nlmsg_type      = RTM_GETLINK;
    nlmsg.nlmsg_flags     = NLM_F_ROOT | NLM_F_REQUEST;
    nlmsg.nlmsg_seq       = 1;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    memcpy(buf, &nlmsg, sizeof(nlmsg));
    buf[16] = AF_UNSPEC;

    /*
    ** Send Netlink Message Requesting Data
    */
    retVal = sendto(socketfd, &buf, 17, 0,
            (struct sockaddr*)&nladdr, sizeof(nladdr));
    if (retVal < 0)
    {
        fprintf(stderr, "Error: cannot send data\n");
        return -1;
    }

    /*
    ** Receive Netlink Data
    */
    retVal = tsl_netlink_recv(socketfd);
    if (retVal == -1)
    {
        close(socketfd);
        return -1;
    }
    close(socketfd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Add / Removes an IP for an Interface
**
**
**  @param     char* - Interface name
**             INT32   - IP Addr
**             INT32   - Add / Remove
**             INT32   - Subnet mask
**
**  @return     0 - Success
**             -1 - Failure
**
******************************************************************************
**/
INT32 tsl_netlink_modIP(const char* device, INT32 newIP, INT32 msgType, INT32 mask)
{
    struct sockaddr_nl  local;

    INT32 socketfd = -1;

    /*
    ** Validate Input Params
    */
    if(newIP == 0)
        return -1;

    /*
    ** Socket to Netlink
    */
    socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(socketfd, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        fprintf(stderr, "Error: bind netlink socket failed\n");
        return -1;
    }

    /*
    ** Send Message to Netlink to Add IP to an Interface
    */
    if (tsl_netlink_send(device, socketfd, newIP, msgType, mask) == -1)
    {
        fprintf(stderr, "Error: Adding IP to Interface failed\n");
        return -1;
    }
    close(socketfd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Get Index for a given Interface name
**
**
**  @param      char* - Interface name
**
**  @return     INT32 - Index
**
******************************************************************************
**/

INT32 tsl_netlink_getIndex(const char *name)
{
    INT32 i=0;
    for (i=0; i<8; i++)
    {
        if (strcmp(iface_map[i].name, name) == 0)
        {
            return iface_map[i].index;
        }
    }
    return 0;
}

/**
******************************************************************************
**
**  @brief      Builds and Sends Netlink Message to Add / Remove IP address
**              for a given Interface Name
**
**  @param      char* - Interface name
**              INT32   - Socket Descriptor
**              INT32   - IP Address
**              INT32   - Add / Remove
**              INT32   - Subnet Mask
**
**  @return     0 - Success
**             -1 - Failure
**
******************************************************************************
**/
INT32 tsl_netlink_send(const char *device, INT32 socketfd, INT32 newIP, INT32 msgType, INT32 mask)
{
    struct sockaddr_nl  nladdr;
    struct nlmsghdr     nlmsg;
    struct ifaddrmsg    ifaddr;
    struct rta          rtaData;
    struct iovec        iov;
    struct msghdr       msg;
    char buf[40];

    memset(&msg, 0 ,sizeof(msg));
    memset(&ifaddr, 0, sizeof(ifaddr));
    memset(&nlmsg, 0, sizeof(nlmsg));

    msg.msg_name           = (void*)&nladdr;
    msg.msg_namelen        = sizeof(nladdr);
    msg.msg_iov            = &iov;
    msg.msg_iovlen         = 1;

    nlmsg.nlmsg_len        = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    nlmsg.nlmsg_flags      = NLM_F_REQUEST;
    nlmsg.nlmsg_type       = msgType;
    nlmsg.nlmsg_seq        = 1;

    ifaddr.ifa_family      = AF_INET;
    ifaddr.ifa_prefixlen   = mask;
    ifaddr.ifa_index       = tsl_netlink_getIndex(device);

    if (ifaddr.ifa_index == 0)
        return -1;

    rtaData.len  = sizeof(rta);
    rtaData.type = IFA_LOCAL;
    rtaData.data = newIP;

    nlmsg.nlmsg_len += sizeof(rta);
    memcpy(buf, &nlmsg, sizeof(nlmsg));
    memcpy(buf+ sizeof(nlmsg), &ifaddr, sizeof(ifaddr));
    memcpy(buf+ sizeof(nlmsg)+ sizeof(ifaddr), &rtaData, sizeof(rtaData));

    iov.iov_base = (void*)&buf;
    iov.iov_len  = nlmsg.nlmsg_len;

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendmsg(socketfd, &msg, 0) <= 0)
    {
        fprintf(stderr, "Unable to send\n");
        return -1;
    }
    return 0;
}

/**
******************************************************************************
**
**  @brief      Checks whether an Port is UP or DOWN
**
**  @param      UINT8  - port
**
**  @return     0 - Success (Port UP)
**             -1 - Failure (Port Down)
**
******************************************************************************
**/

UINT32 tsl_IsPortUp(UINT8 port)
{
    INT32 fd;
    struct ifreq ifr;
    struct ethtool_value etv;

    if ( (!ICL_PRT(port)) && (p2n[port] == NULL) )
    {
        fprintf(stderr,"ISCSI_DEBUG: Invalid Params\n");
        return FALSE;
    }

    memset(&etv, 0, sizeof(etv));
    memset(&ifr, 0, sizeof (struct ifreq));

    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ioctl(fd, SIOCGIFFLAGS, &ifr) == 0)
    {
        if((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING))
        {
            memset(&ifr, 0, sizeof (struct ifreq));
            if(ICL_IsIclPort(port))
            {
                strcpy(ifr.ifr_name, iclIfrName);
            }
            else
            {
                strcpy(ifr.ifr_name, p2n[port]);
            }
            ifr.ifr_data = (caddr_t)&etv;
            etv.cmd = ETHTOOL_GLINK;
            if(ioctl(fd, SIOCETHTOOL, &ifr) != 0)
                fprintf(stderr,"RAGX: SIOCETHTOOL ioctl on port %d FAILED: %d:%s!!!\n", port, errno, strerror(errno));
            etv.data = 1;
        }
    }
    close(fd);
    return (etv.data);
}

/**
******************************************************************************
**
**  @brief      Builds and Sends Netlink Message to Add / Remove IP address
**              for a given Interface Name
**
**  @param      UINT8    - port number
**              UINT32   - IP Address
**
**  @return     0 - Success
**             -1 - Failure
**
******************************************************************************
**/

INT32 tsl_send_arp(UINT8 port, UINT32 addr)
{
    struct in_addr my_in_addr;
    struct arp_packet pkt;
    struct sockaddr sa;
    struct ifreq ifr;
    INT32 fd,i,sock;

    addr = bswap_32(addr);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        fprintf(stderr, "tsl_send_arp: socket failed\n");
        return -1;
    }

    /*
    ** Get HardWare Address
    */
    memset(&ifr, 0, sizeof (struct ifreq));

    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
    {
        fprintf(stderr, "Error: get Hardware address failed\n");
        close(fd);
        return -1;
    }
    close(fd);

    memset(&pkt, 0, sizeof(struct arp_packet));

    pkt.frame_type      = htons(ARP_FRAME_TYPE);
    pkt.hw_type         = htons(ETHER_HW_TYPE);
    pkt.prot_type       = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size    = MAC_ADDR_LEN;
    pkt.prot_addr_size  = IP_ADDR_LEN;
    pkt.op              = htons(OP_ARP_REQUEST);

    for (i = 0; i < MAC_ADDR_LEN; i++)
        pkt.targ_hw_addr[i] = 0xff;

    memcpy(pkt.src_hw_addr, ifr.ifr_hwaddr.sa_data, 6);
    memcpy(pkt.sndr_hw_addr, ifr.ifr_hwaddr.sa_data, 6);

    my_in_addr.s_addr = htonl(addr);

    memcpy(pkt.sndr_ip_addr,&my_in_addr,IP_ADDR_LEN);
    memcpy(pkt.rcpt_ip_addr,&my_in_addr,IP_ADDR_LEN);

    sa.sa_family = AF_INET;

    if(ICL_IsIclPort(port))
    {
        strcpy(sa.sa_data, iclIfrName);
    }
    else
    {
        strcpy(sa.sa_data, p2n[port]);
    }

    sock = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP));
    TSL_SetNonBlocking (sock);
    if (sendto(sock, (const void *)&pkt, sizeof(pkt), 0, &sa,sizeof(sa)) < 0)
    {
        fprintf(stderr,"ISCSI_DEBUG: Unable to send\n");
    }
    close(sock);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Adds an IP Address to an Interface
**              Brings Down an Interface, adds IP through netlink
**              Brings Up an interface, Broadcast the Gratipus ARP
**              opens socket communications for port
**
**  @param      UINT8  - Port
**              UINT32 - IP Address
**              UINT16 - Target ID
**
**  @return     none
**
******************************************************************************
**/
void tsl_addAddr(UINT8 port, UINT32 addr, UINT16 tid, UINT32 ipPrefix)
{
    {
        char tmp_buff[254];
        struct in_addr in;
        in.s_addr = addr;

        if(ICL_IsIclPort(port))
        {
#if ICL_DEBUG
            fprintf(stderr,"<%s:%s>ICL port\n",__FILE__,__func__);
#endif
            sprintf(tmp_buff, "Add IP %s, port %d Interface %s\n",
                inet_ntoa(in), port, iclIfrName);
        }
        else
        {
            sprintf(tmp_buff, "Add IP %s, port %d Interface %s\n",
            inet_ntoa(in), port, p2n[port]);
            fprintf(stderr, "Add IP %s, port %d Interface %s\n", inet_ntoa(in), port, p2n[port]);
        }
        iscsi_dprintf(tmp_buff);
    }

    /*
    ** Interface Down
    */
    tsl_if_set(port, MY_IFDOWN);

    /*
    ** Add IP to Interface
    */
    if(ICL_IsIclPort(port))
    {
        tsl_netlink_modIP(iclIfrName, addr, RTM_NEWADDR, ipPrefix);
    }
    else
    {
        tsl_netlink_modIP(p2n[port], addr, RTM_NEWADDR, ipPrefix);
    }

    /*
    ** Interface UP
    */
    tsl_if_set(port, MY_IFUP);

    /*
    ** Add Route
    */
    BIT_SET(iscsiAddRoute, tid);

    /*
    ** Set ARP Flag
    */
    tsl_if_set(port, MY_ARPON);

    /*
    ** opens listen socket
    */
    tsl_InitConnection(addr, tid, port);

}/* tsl_addAddr */

/**
******************************************************************************
**
**  @brief      Delete the IP Address from an Interface
**
**  @param      UINT8  - Port
**              UINT32 - IP Address
**              UINT16 - Target ID
**
**  @return     none
**
******************************************************************************
**/
void tsl_delAddr(UINT8 port, UINT32 addr, UINT16 tid, UINT32 ipPrefix, UINT32 ipGw)
{
    {
        char tmp_buff[254];
        struct in_addr in;
        in.s_addr = addr;

        if(ICL_IsIclPort(port))
        {
            sprintf(tmp_buff, "Del IP %s, port %d Interface %s\n",
                inet_ntoa(in), port, iclIfrName);
        }
        else
        {
            sprintf(tmp_buff, "Del IP %s, port %d Interface %s\n",
                inet_ntoa(in), port, p2n[port]);
        }
        fprintf(stderr, "Del IP %s, port %d Interface %s\n", inet_ntoa(in), port, p2n[port]);
        iscsi_dprintf(tmp_buff);
    }

    arpPCB[port] = 0x0;

    /*
    ** Close Session and Connection on the Interface
    */
    iscsi_TargetCleanup(port, tid);

    /*
    ** Delete route rule
    */
    tsl_iprule(addr, port+2, RTM_DELRULE);

    /*
    ** Delete route rule
    */
    tsl_netlink_addRoute(addr, ipPrefix, ipGw, RTM_DELROUTE, port);

    /*
    ** Delete Local ARP Entry
    */
    tsl_arpDelete(port, addr);

    /*
    ** Interface Down
    */
    tsl_if_set(port, MY_IFDOWN);

    /*
    ** Delete IP from Interface
    */
    if(ICL_IsIclPort(port))
    {
        tsl_netlink_modIP(iclIfrName, addr, RTM_DELADDR, ipPrefix);
    }
    else
    {
        tsl_netlink_modIP(p2n[port], addr, RTM_DELADDR, ipPrefix);
    }

    /*
    ** Interface UP
    */
    if (ispFailedPort[port] == FALSE)
    {
        tsl_if_set(port, MY_IFUP);
    }
}/* tsl_delAddr */

/**
******************************************************************************
**
**  @brief      Sets the rule
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_iprule(INT32 ipAddr, UINT8 table, UINT8 cmd)
{
    struct sockaddr_nl  local;
    struct sockaddr_nl  nladdr;
    struct iovec        iov;
    struct msghdr       msg;

    INT32 socketfd = -1;

    struct {
        struct nlmsghdr     n;
        struct rtmsg        r;
        struct rta          data;
    } req;

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_type    = cmd;
    req.n.nlmsg_flags   = NLM_F_REQUEST | NLM_F_CREATE;

    req.r.rtm_family    = AF_INET;
    req.r.rtm_protocol  = RTPROT_UNSPEC;
    req.r.rtm_scope     = RT_SCOPE_UNIVERSE;
    req.r.rtm_table     = table;
    req.r.rtm_type      = RTN_UNICAST;
    req.r.rtm_src_len   = 32;

    req.data.len  = 8;
    req.data.type = RTA_SRC;
    req.data.data = ipAddr;

    req.n.nlmsg_len = sizeof(req);


    socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (socketfd < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(socketfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: bind netlink socket failed\n");
        return -1;
    }

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name           = (void*)&nladdr;
    msg.msg_namelen        = sizeof(nladdr);
    msg.msg_iov            = &iov;
    msg.msg_iovlen         = 1;

    iov.iov_base = (void*)&req;
    iov.iov_len  = sizeof(req);

    if(sendmsg(socketfd, &msg, 0) <= 0){
        fprintf(stderr,"ISCSI_DEBUG: Unable to send, %s\n",strerror(errno));
        return -1;
    }
    close(socketfd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Deletes the routing Entry from main table
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_modifyRoute(INT32 ipAddr, UINT8 prefix, UINT8 portIndex, UINT8 cmdType, UINT8 table)
{
    struct sockaddr_nl  local;

    struct
    {
        struct nlmsghdr     nlmsg;
        struct rtmsg        rtmsg;
        struct rta          rtaDst;
        struct rta          rtaSrc;
        struct rta          rtaOif;
    }iscsi_route;

    struct iovec        iov;
    struct msghdr       msg;
    char buf[256] = {0};
    INT32 socketfd = -1;

    memset(&buf, 0, 256);
    socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (socketfd < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(socketfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: bind netlink socket failed\n");
        return -1;
    }
    memset(&iscsi_route, 0, sizeof(iscsi_route));
    iscsi_route.nlmsg.nlmsg_type    = cmdType;
    iscsi_route.nlmsg.nlmsg_flags   = NLM_F_REQUEST | NLM_F_CREATE;
    iscsi_route.nlmsg.nlmsg_seq     = 1;

    iscsi_route.rtmsg.rtm_family    = AF_INET;
    iscsi_route.rtmsg.rtm_dst_len   = prefix;
    iscsi_route.rtmsg.rtm_src_len   = prefix;
    iscsi_route.rtmsg.rtm_table     = table;
    iscsi_route.rtmsg.rtm_scope     = RT_SCOPE_LINK;
    iscsi_route.rtmsg.rtm_type      = RTN_UNICAST;

    if(cmdType == RTM_NEWROUTE)
    {
        iscsi_route.rtmsg.rtm_protocol  = RTPROT_KERNEL;
    }
    else
    {
        iscsi_route.rtmsg.rtm_protocol  = RTPROT_UNSPEC;
    }

    iscsi_route.rtaDst.type  = RTA_DST;
    iscsi_route.rtaDst.len   = sizeof(struct rta);
    {
        UINT32 gw = 0xFFFFFFFF;
        gw = bswap_32(gw << (32 - prefix));
        iscsi_route.rtaDst.data  = ipAddr & gw;
    }

    iscsi_route.rtaSrc.type  = RTA_PREFSRC;
    iscsi_route.rtaSrc.len   = sizeof(struct rta);
    iscsi_route.rtaSrc.data  = ipAddr;

    iscsi_route.rtaOif.type = RTA_OIF;
    iscsi_route.rtaOif.len  = sizeof(struct rta);
    iscsi_route.rtaOif.data = portIndex;

    iscsi_route.nlmsg.nlmsg_len = sizeof(iscsi_route);

    memset(&msg, 0, sizeof(msg));
    msg.msg_name     = (void*)&local;
    msg.msg_namelen  = sizeof(local);
    msg.msg_iov      = &iov;
    msg.msg_iovlen   = 1;

    iov.iov_base = (void*)&iscsi_route;
    iov.iov_len  = iscsi_route.nlmsg.nlmsg_len;

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if(sendmsg(socketfd, &msg, 0) <= 0){
        fprintf(stderr,"ISCSI_DEBUG: Unable to send, error = %s\n",strerror(errno));
        return -1;
    }
    close(socketfd);
    return 0;
}


/**
******************************************************************************
**
**  @brief      Adds the route
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_netlink_addRoute(INT32 ipAddr, UINT8 prefix, INT32 gateWay, INT32 msgType, UINT8 port)
{
    struct sockaddr_nl  local;

    struct
    {
        struct nlmsghdr     nlmsg;
        struct rtmsg        rtmsg;
        struct rta          rtaDst;
        struct rta          rtaGw;
        struct rta          rtaOif;
    }iscsi_route;

    struct iovec        iov;
    struct msghdr       msg;
    char buf[256] = {0};
    INT32 socketfd = -1;

    memset(&buf, 0, 256);
    socketfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (socketfd < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(socketfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: bind netlink socket failed\n");
        return -1;
    }

    memset(&iscsi_route, 0, sizeof(iscsi_route));
    iscsi_route.nlmsg.nlmsg_type    = msgType;
    iscsi_route.nlmsg.nlmsg_flags   = NLM_F_REQUEST|NLM_F_CREATE;
    iscsi_route.nlmsg.nlmsg_seq     = 1;

    iscsi_route.rtmsg.rtm_family    = AF_INET;
    iscsi_route.rtmsg.rtm_dst_len   = prefix;
    iscsi_route.rtmsg.rtm_table     = port+2;

    iscsi_route.rtmsg.rtm_protocol  = RTPROT_KERNEL;
    iscsi_route.rtmsg.rtm_scope     = RT_SCOPE_UNIVERSE;
    iscsi_route.rtmsg.rtm_type      = RTN_UNICAST;

    iscsi_route.rtaDst.type  = RTA_DST;
    iscsi_route.rtaDst.len   = sizeof(struct rta);

    if(ipAddr != 0)
    {
        UINT32 gw = 0xFFFFFFFF;
        gw = bswap_32(gw << (32 - prefix));
        iscsi_route.rtaDst.data  = ipAddr & gw;
    }
    else
    {
        iscsi_route.rtaDst.data  = 0;
    }

    iscsi_route.rtaGw.type   = RTA_GATEWAY;
    iscsi_route.rtaGw.len    = sizeof(struct rta);
    iscsi_route.rtaGw.data   = gateWay;

    iscsi_route.rtaOif.type = RTA_OIF;
    iscsi_route.rtaOif.len  = sizeof(struct rta);

    if(ICL_IsIclPort(port))
    {
        iscsi_route.rtaOif.data = tsl_netlink_getIndex(iclIfrName);
    }
    else
    {
        iscsi_route.rtaOif.data = tsl_netlink_getIndex(p2n[port]);
    }

    iscsi_route.nlmsg.nlmsg_len = sizeof(iscsi_route);

    memset(&msg, 0, sizeof(msg));
    msg.msg_name     = (void*)&local;
    msg.msg_namelen  = sizeof(local);
    msg.msg_iov      = &iov;
    msg.msg_iovlen   = 1;

    iov.iov_base = (void*)&iscsi_route;
    iov.iov_len  = iscsi_route.nlmsg.nlmsg_len;

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if(sendmsg(socketfd, &msg, 0) <= 0){
        fprintf(stderr,"ISCSI_DEBUG: Unable to send, error = %s\n",strerror(errno));
        return -1;
    }

    close(socketfd);
    return 0;
}


/**
******************************************************************************
**
**  @brief      Initialize all the iSCSI structures pertaining to the port.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_netlink_event_init (void)
{
    struct sockaddr_nl  local;

    /*
    ** Create Netlink socket
    */
    tsl_netlink_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (tsl_netlink_socket < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: Open netlink socket failed\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = RTMGRP_LINK;

    /*
    ** Bind
    */
    if (bind(tsl_netlink_socket, (struct sockaddr*)&local, sizeof(local)) < 0) {
        fprintf(stderr,"ISCSI_DEBUG: Error: bind netlink socket failed\n");
        return -1;
    }

    /*
    ** set socket to Non Blocking
    */
    TSL_SetNonBlocking (tsl_netlink_socket);

    /*
    ** add socket to epoll
    */
    tsl_ev_add(tsl_netlink_socket, EPOLLIN, tsl_cb_netlink, NULL);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Initialize all the iSCSI structures pertaining to the port.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
INT32 tsl_netlink_recvmsg (void)
{
    struct msghdr msg;
    struct iovec iov;
    struct sockaddr_nl  nladdr;
    struct ifinfomsg *ifi = NULL;
    char buf[4086] = {0};
    INT32 retVal = 0;
    UINT8 maxPorts = MAX_PORTS;

    struct nlmsghdr *nlmsg;
    UINT8 portName[8]={0};
    INT8 port = 0xFF;

    if(tsl_netlink_socket == -1)
    {
        return -1;
    }

    memset(&msg, 0 ,sizeof(msg));
    msg.msg_name        = (void*)&nladdr;
    msg.msg_namelen     = sizeof(nladdr);
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;

    iov.iov_base = (void*)&buf;
    iov.iov_len  = sizeof(buf);

    /*
    ** receive netlink message
    */
    retVal = recvmsg(tsl_netlink_socket, &msg, 0);
    if (retVal <= 0) {
        return -1;
    }
    if(iclPortExists)
    {
        maxPorts = MAX_PORTS + MAX_ICL_PORTS;
    }

    nlmsg = (struct nlmsghdr*)buf;

    /*
    ** Parse Till the end of Message
    */
    while (NLMSG_OK(nlmsg, (UINT32)retVal))
    {
        if ((nlmsg->nlmsg_type == NLMSG_DONE) || (nlmsg->nlmsg_type == NLMSG_ERROR))
        {
            return 0;
        }

        /*
        ** get the Interafce name from the recvd message
        */
        tsl_getPortName((UINT8 *)buf, portName);

        /*
        ** get the port from the interface name
        */
        port = tsl_getPort(portName);

        if(port > maxPorts)
        {
            nlmsg = NLMSG_NEXT(nlmsg, retVal);
            continue;
        }

        /*
        ** if iscsi target and Enabled
        */
        if(BIT_TEST(isprena, port) && (BIT_TEST(iscsimap, port)|| ICL_IsIclPort(port))
                    && !BIT_TEST(resilk, port))
        {
            ifi = NLMSG_DATA(nlmsg);

            if((ifi->ifi_flags & 0x41) == 0x41)
            {
                /*
                ** check if Offline -> Online, if already Online ignore it
                */
                if(!BIT_TEST(ispOnline, port))
                {
                    // fprintf(stderr,"ISCSI_DEBUG:  Loop %d is UP\n",port);
                    fprintf(stderr,"RAGX:PORT-%d:%s LoopUP ispOnline=0x%x!!!\n",port,p2n[port],ispOnline);

                    tsl_ifsetup(port);

                    /*
                     * If task is being created, wait.
                     */
                    while (LoopUp[port] == (UINT32)-1)
                    {
                        TaskSleepMS(50);
                    }

                    if(LoopUp[port] == 0)
                    {
                        void * tmp_addr;
                        CT_fork_tmp = (unsigned long)"ISCSI_LoopUp";
                        LoopUp[port] = -1;      // Flag that task is being created.
                        tmp_addr = TaskCreate3(C_label_referenced_in_i960asm(ISCSI_LoopUp), 32, port);
                        LoopUp[port] = (UINT32)tmp_addr;
                    }
                }
            }
            else
            {
                /*
                ** check if Online -> Offline
                ** check if already LoopDown event is sent
                */
                if((BIT_TEST(ispOnline, port)) && (!BIT_TEST(ispCp, port)))
                {
                    /*
                     * If task is being created, wait.
                     */
                    while (LoopDown[port] == (UINT32)-1)
                    {
                        TaskSleepMS(50);
                    }

                    if(LoopDown[port] == 0)
                    {
                        void * tmp_addr;
                        fprintf(stderr,"ISCSI_DEBUG:  Loop %d is DOWN\n",port);
                        LoopDown[port] = -1;        // Flag task being created.
                        CT_fork_tmp = (unsigned long)"ISCSI_LoopDown";
                        tmp_addr = TaskCreate3(C_label_referenced_in_i960asm(ISCSI_LoopDown), 32, port);
                        LoopDown[port] = (UINT32)tmp_addr;
                    }
                }
            }
        }
        nlmsg = NLMSG_NEXT(nlmsg, retVal);
    }
    return retVal;
}


INT32 tsl_getPort(UINT8 *portName)
{
    INT32 port;


    if( strcmp((char *)portName, iclIfrName) == 0)
    {
        return ICL_PORT;
    }

    /*
    ** get the port number from Interface Name
    */
    for(port = 0; port < MAX_PORTS; port++)
    {
        if(strcmp((char *)portName, p2n[port]) == 0)
        {
            return port;
        }
    }
    return 0xFF;
}

void tsl_getPortName(UINT8 *buf, UINT8 *portName)
{
    struct nlmsghdr *nlmsg = (struct nlmsghdr*)buf;
    struct ifinfomsg *ifi = NLMSG_DATA(nlmsg);
    struct rtattr *tb[IFLA_MAX+1];
    struct rtattr *rtat = IFLA_RTA(ifi);
    INT32 len = IFLA_PAYLOAD(nlmsg);

    memset(tb, 0, sizeof(tb));

    while (RTA_OK(rtat, len))
    {
        if (rtat->rta_type <= IFLA_MAX)
            tb[rtat->rta_type] = rtat;
        rtat = RTA_NEXT(rtat,len);
    }

    if (tb[IFLA_IFNAME] == NULL)
        return;

    /*
    ** get the Interface Name from the received netlink message
    */
    strcpy((char *)portName, RTA_DATA(tb[IFLA_IFNAME]));
}

/**
******************************************************************************
**
**  @brief      Removes the Session and Connection on an Interface
**
**  @param      UINT8  - Port
**
**  @return     SUCCESS
**
******************************************************************************
**/

UINT8 iscsi_TargetCleanup(UINT8 port, UINT16 tid)
{
    UINT8       result    = XIO_SUCCESS;
    TAR         *pTar       = NULL;

    /* Close all the sessions and connections within */
    for (pTar = tar[port]; pTar != NULL; pTar = pTar->fthd)
    {
        if((BIT_TEST(iscsimap, port) || ICL_IsIclPort(port)) && (pTar->tid == tid))
        {
            if(gTPD[pTar->tid].socket_fd != 0)
            {
                tsl_ev_del(gTPD[pTar->tid].socket_fd, EPOLLIN, tsl_cb_accept, &gTPD[pTar->tid]);

                /* Cleanup epoll events for pTPD[pTar->tid] */
                tsl_cleanupEvents((void *)&gTPD[pTar->tid]);
                close(gTPD[pTar->tid].socket_fd);
            }
            memset(&gTPD[pTar->tid], 0, sizeof(ISCSI_TPD));
       }/* -- End of if(BIT_TEST(iscsimap, port)) */
    }/* --- End of for(pTar = tar[port];... */
    return result;
}   /* End of iscsi_TargetCleanup() */


void iscsi_SessionCleanup (TAR* pTar)
{
    SESSION     *pSsn    = NULL;
    SESSION     *pTmpSsn = NULL;
    CONNECTION  *pConn   = NULL;

    for(pSsn = (SESSION*)pTar->portID; pSsn != NULL; pSsn = pTmpSsn)
    {
        pTmpSsn = pSsn->pNext;
        /*
        ** Go through all the sessions
        */
        if((pConn = pSsn->pCONN) != NULL)
        {
            iscsiCloseConn(pConn);
        }
    }/* End of  Session list iteration for() loop */
    pTar->portID = 0x0;
}

/**
******************************************************************************
**
**  @brief
**
**  @param      UINT8  - Interface
**
**  @return     none
**
******************************************************************************
**/

extern UINT16 I_get_lid(UINT8 port);

void ISCSI_LoopDown (UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    if(ICL_PRT(port))
    {
#if ICL_DEBUG
        fprintf(stderr,"<%s:%s>ICL-LoopDown.calling ICL_LoopDown\n",__FILE__,__func__);
#endif
        ICL_LoopDown(ICL_PORT);
        ICL_PortReset(ICL_PORT);
    }
    else
    {
        ISP_LoopDown(port);
    }
    LoopDown[port] = 0x0;
}

void ISCSI_LoopUp (UINT32 a UNUSED, UINT32 b UNUSED, UINT8 port)
{
    UINT32 bitPort = 1<<port;
    UINT8  event   = 11;

    /*
    ** If this is a transition from offline => online,
    ** then set the online bit and start the 10 second timer for this port
    */
    if ((ispOnline & bitPort) == 0)
    {
        ispOnline |= bitPort;
        BIT_CLEAR(ispfail, port);

        if(! ICL_PRT(port) )
        {
            onlineTimer[port] = 10;

            /*
             * Wait if isp_portOnlineHandler process is being created.
             */
            while (isponpcb == (PCB *)-1)
            {
                TaskSleepMS(50);
            }

            /*
            ** Fork online monitor process, if necessary.
            */
            if (isponpcb == NULL)
            {
                CT_fork_tmp = (unsigned long)"isp_portOnlineHandler";
                isponpcb = (PCB *) -1;      // Flag that task is being created.
                isponpcb = TaskCreate2(
                        C_label_referenced_in_i960asm(isp_portOnlineHandler),
                        ISPPONPRI_PRIORITY);
            }
            if (isplepcb[port] != NULL)
            {
                if (TaskGetState(isplepcb[port]) == PCB_NOT_READY)
                {
#ifdef HISTORY_KEEP
CT_history_pcb("ISCSI_LoopUp setting ready pcb", (UINT32)isplepcb[port]);
#endif  /* HISTORY_KEEP */
                    TaskSetState(isplepcb[port], PCB_READY);
                }
            }
        }
        else
        {
            ispofflfail &= ~bitPort;

            /*
            ** Set power up wait bit. In case of iSCSI ports, this is set in loopEventHandler
            ** whenever the port is online during poweron.
            ** Since ICL does not have any loopEventHandler, we are setting this bit here, when
            ** ICL port is online.
            */
            ispPow |= (1<<port);
            ICL_LogEvent(LOG_ICLPORT_LOOPUP);
        }

        /*
        ** Notify initiator of online event, when target is enable & ipAddr is set
        ** Patch ("if" condition (ispmap == iscsimap)) added to avoid invoking the
        ** initiator path for iSCSI in mixed mode. (ATS-158)
        */
       if(tar[port] && (BIT_TEST(tar[port]->opt, TARGET_ENABLE))
                            && (!BIT_TEST(ispCp, port))
                            && (tar[port]->ipAddr != 0)
                            && (ispmap == iscsimap))
       {
           UINT16 lid = I_get_lid(port);
#if ICL_DEBUG
           if(ICL_PRT(port))
           {
               fprintf(stderr,"<iSCSI_LoopUp> calling I_recv_online() for ICL port\n");
           }
#endif
           I_recv_online(port, lid, event);
       }
    }
    LoopUp[port] = 0x0;
}

/**
******************************************************************************
**
**  @brief      Updates the Local ARP Entry
**
**  @param      UINT8  - Interface
**              UINT32 - IP Address
**
**  @return     INT32  - SUCCESS / FAILURE
**
******************************************************************************
**/

INT32 tsl_arpUpdate (UINT8 port, UINT32 addr)
{
    struct arpreq arpreq;
    struct ifreq ifr;
    INT32 socket_fd;
    char tmp[6];

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0){
        fprintf(stderr,"ISCSI_DEBUG: socket open failed\n");
        close(socket_fd);
        return -1;
    }

    memset(&ifr,0,sizeof (struct ifreq));

    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }
    if (ioctl(socket_fd, SIOCGIFHWADDR, &ifr) < 0)
    {
        if(ICL_IsIclPort(port))
        {
            fprintf(stderr,"ISCSI_DEBUG: Error: Unable to get HW Addr for %s\n", iclIfrName);
        }
        else
        {
            fprintf(stderr,"ISCSI_DEBUG: Error: Unable to get HW Addr for %s\n", p2n[port]);
        }
        close(socket_fd);
        return -1;
    }

    memset(&arpreq, 0, sizeof(struct arpreq));
    arpreq.arp_flags                = ATF_PERM;
    arpreq.arp_pa.sa_family         = AF_INET;
    arpreq.arp_ha.sa_family         = ARPHRD_ETHER;
    arpreq.arp_netmask.sa_family    = AF_INET;

    if(ICL_IsIclPort(port))
    {
        strcpy(arpreq.arp_dev, iclIfrName);
    }
    else
    {
        strcpy(arpreq.arp_dev, p2n[port]);
    }

    memcpy(arpreq.arp_ha.sa_data, &(ifr.ifr_hwaddr.sa_data), 6);

    memset(&tmp, 0, sizeof(tmp));
    memcpy(&tmp[2], (char*)&addr, sizeof(addr));
    memcpy(arpreq.arp_pa.sa_data, &tmp, sizeof(tmp));

    /*
    ** Update Local ARP Entry
    */
    if (ioctl(socket_fd, SIOCSARP, &arpreq) < 0){
        close(socket_fd);
        return -1;
    }
    close (socket_fd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Deletes the Local ARP Entry
**
**  @param      UINT8  - Interface
**              UINT32 - IP Address
**
**  @return     INT32  - SUCCESS / FAILURE
**
******************************************************************************
**/

INT32 tsl_arpDelete(UINT8 port, UINT32 addr)
{
    struct arpreq arpreq;
    struct ifreq ifr;
    INT32 socket_fd;
    char tmp[6];

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0){
        fprintf(stderr,"ISCSI_DEBUG: socket open failed\n");
        close(socket_fd);
        return -1;
    }

    memset(&ifr,0,sizeof (struct ifreq));

    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }

    if (ioctl(socket_fd, SIOCGIFHWADDR, &ifr) < 0)
    {
        if(ICL_IsIclPort(port))
        {
            fprintf(stderr,"ISCSI_DEBUG: Error: Unable to get HW Addr for %s\n", iclIfrName);
        }
        else
        {
            fprintf(stderr,"ISCSI_DEBUG: Error: Unable to get HW Addr for %s\n", p2n[port]);
        }
        close(socket_fd);
        return -1;
    }

    memset(&arpreq, 0, sizeof(struct arpreq));

    arpreq.arp_pa.sa_family         = AF_INET;

    if(ICL_IsIclPort(port))
    {
        strcpy(arpreq.arp_dev, iclIfrName);
    }
    else
    {
        strcpy(arpreq.arp_dev, p2n[port]);
    }

    memcpy(arpreq.arp_ha.sa_data, &(ifr.ifr_hwaddr.sa_data), 6);

    memset(&tmp, 0, sizeof(tmp));
    memcpy(&tmp[2], (char*)&addr, sizeof(addr));
    memcpy(arpreq.arp_pa.sa_data, &tmp, sizeof(tmp));

    /*
    ** Delete Local ARP Entry
    */
    if (ioctl(socket_fd, SIOCDARP, &arpreq) < 0){
        close(socket_fd);
        return -1;
    }
    close (socket_fd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Sets the MTU Size for an Port
**
**  @param      UINT8  - Interface
**              UINT32 - MTU Size
**
**  @return     INT32  - SUCCESS / FAILURE
**
******************************************************************************
**/
INT32 tsl_set_mtu (UINT8 port, INT32 mtu_size)
{
    struct ifreq ifr;
    int sock;
    INT32 err = -1;

    if ( (!BIT_TEST(iscsimap, port) ) && (! ICL_IsIclPort(port)))
    {
//        fprintf(stderr, "tsl_set_mtu called for non-iSCSI port %u\n", port);
        return 0;
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "tsl_set_mtu %u %d failed to create socket\n", port, mtu_size);
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name,iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }
    ifr.ifr_addr.sa_family = AF_INET;
    ifr.ifr_mtu = mtu_size;

    err = ioctl(sock, SIOCSIFMTU, &ifr);
    if (err)
    {
        fprintf(stderr, "tsl_set_mtu unable to set size %d for iSCSI port %u name %s\n",
                mtu_size, port, ifr.ifr_name);
    }
//    else
//    {
//        fprintf(stderr, "tsl_set_mtu size %d for iSCSI port %u name %s\n",
//                mtu_size, port, ifr.ifr_name);
//    }
    close(sock);
    return err;
}

/**
******************************************************************************
**
**  @brief      Gets the MTU Size for an Port
**
**  @param      UINT8  - Interface
**              UINT32 - MTU Size
**
**  @return     INT32  - MTU Size / Error Value
**
******************************************************************************
**/
INT32 tsl_get_mtu(UINT8 port)
{
    struct ifreq ifr;
    INT32 fd, err = -1;

    memset (&ifr, 0, sizeof(struct ifreq));
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if(ICL_IsIclPort(port))
    {
        strcpy(ifr.ifr_name, iclIfrName);
    }
    else
    {
        strcpy(ifr.ifr_name, p2n[port]);
    }
    ifr.ifr_addr.sa_family = AF_INET;

    err = ioctl(fd, SIOCGIFMTU, &ifr);
    if (err)
    {
        fprintf(stderr, "unable to get mtu size\n");
        close(fd);
        return -1;
    }
    fprintf(stderr, "MTU = %d\n", ifr.ifr_mtu);
    close(fd);
    return ifr.ifr_mtu;
}

/**
******************************************************************************
**
**  @brief      Gets the IP Address for an Interface
**
**  @param      char* - Interface name
**
**  @return     INT32  - IP Address
**
******************************************************************************
**/

UINT32 tsl_getIP (char* device)
{
    struct ifreq ifr;
    INT32 fd = -1, err = -1;
    UINT32 ipAddr = 0;

    memset(&ifr, 0 , sizeof(struct ifreq));
    strcpy(ifr.ifr_name, device);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if((err = ioctl(fd, SIOCGIFADDR, &ifr)) < 0)
    {
         fprintf(stderr,"Error: tsl_getIP: ioctl SIOCGIFADDR failed\n");
         close(fd);
         return 0;
    }

    ipAddr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
    close(fd);
    return ipAddr;
}

/**
******************************************************************************
**
**  @brief      Updates the TCP trasnmit queue len
**
**  @param      char* - Interface name
**
**  @return     UINT32  - Success
**
******************************************************************************
**/
UINT32 tsl_updateTxQueLen(UINT8 port)
{
    struct ifreq ifr;
    INT32 fd = -1;

    memset(&ifr, 0 , sizeof(struct ifreq));
    strcpy(ifr.ifr_name, p2n[port]);
    ifr.ifr_qlen = 2000;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(ioctl(fd, SIOCSIFTXQLEN, &ifr) < 0)
    {
         fprintf(stderr,"Error: tsl_updateTxQueLen: ioctl failed\n");
    }

    close(fd);
    return 0;
}
#endif /*FRONTEND*/

/*****************************************************************************
********************** CODE TO SUPPORT iSCSI INITIATOR ***********************
*****************************************************************************/

#ifdef FRONTEND
#define ISCSI_DAEMON_SESSION_CLOSE     0
#define ISCSI_DAEMON_CONN_CLOSE        1
#define ISCSI_DAEMON_OTHER_EVENT       2
#define XIOPROC_NAMESPACE              "XIOPROC_NAMESPACE"
#define ISCSI_NAME_LENGTH 260

typedef struct iscsi_daemon_proc
{
    UINT32  msgType;
    char    i_name[ISCSI_NAME_LENGTH];
    char    t_name[ISCSI_NAME_LENGTH];
    INT32   pgt;
    INT32   error_no;
    UINT64  session_handle;
}DAEMON_PROC_MSG;

/**
******************************************************************************
**
**  @brief      Callback function, accepts new connection
**
**  @param      UINT32 - events
**              void*  - Reference pointer
**
**  @return     none
**
******************************************************************************
**/

void tsl_cb_acpt_daemon (UINT32 events UNUSED, void *pRef)
{
    INT32 socket_fd = 0x0;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    INT32    new_socket_fd;

    socket_fd = (INT32)pRef;

    /*
    ** Accept New Connection
    */
    new_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (new_socket_fd < 0)
    {
        fprintf(stderr,"ISCSI_DEBUG: tsl_cb_acpt_daemon: Error accept failed\n");
        return;
    }

    /*
    ** Set Socket to Non Blocking
    */
    TSL_SetNonBlocking (new_socket_fd);

    /*
    ** Add socket to epoll
    */
    tsl_ev_add(new_socket_fd, EPOLLIN , tsl_cb_daemon_events, (void *)new_socket_fd);
    fprintf(stderr,"ISCSI_DEBUG: tsl_cb_acpt_daemon: accepted socket (%d)\n",new_socket_fd);
}

/**
******************************************************************************
**
**  @brief      Receives and Process a message from iSCSI Daemon
**
**  @param      UINT32 - events
**              void*  - Reference pointer
**
**  @return     none
**
******************************************************************************
**/

DAEMON_PROC_MSG msg;

extern IDD *gIDX[MAX_PORTS+MAX_ICL_PORTS][MAX_DEV];

void tsl_cb_daemon_events (UINT32 events UNUSED, void *pRef)
{
    INT32 socket_fd = 0x0;

    /*
    ** recv message from iSCSI daemon
    */
    if(pRef == NULL)
    {
        return;
    }
    socket_fd = (INT32)pRef;

    if(recv(socket_fd, (char*)&msg, sizeof(DAEMON_PROC_MSG), 0) == sizeof(DAEMON_PROC_MSG))
    {
        switch(msg.msgType)
        {
          case ISCSI_DAEMON_CONN_CLOSE:
          case ISCSI_DAEMON_SESSION_CLOSE:
            {
                UINT64 i_name, t_name;
                UINT32 i, j;

                convertTargetNameInTo64bit((UINT8 *)msg.i_name,20, &i_name);
                convertTargetNameInTo64bit((UINT8 *)msg.t_name,20, &t_name);
                for(i = 0; i < MAX_PORTS; i++)
                {
                    for(j = 0; j < MAX_DEV; j++)
                    {
                        if((gIDX[i][j] != NULL)
                                && (gIDX[i][j]->i_name == i_name)
                                && (gIDX[i][j]->t_name == t_name)
                                && (gIDX[i][j]->ptg == msg.pgt))
                        {
                            fprintf(stderr,"ISCSI_DEBUG: tsl_cb_daemon_events() socket (%d) port (%d) lid (%d)\n",socket_fd,i,j);
                            BIT_CLEAR(gIDX[i][j]->flags, IDF_SESSION);
                            if (BIT_TEST(gIDX[i][j]->flags, IDF_LOGOUT))
                            {
                                if (gIDX[i][j]->pTLogoutPcb)
                                {
                                    if (TaskGetState((PCB *)(gIDX[i][j])->pTLogoutPcb) == PCB_NOT_READY ||
                                        TaskGetState((PCB *)(gIDX[i][j])->pTLogoutPcb) == PCB_TIMED_WAIT)
                                    {
#ifdef HISTORY_KEEP
CT_history_pcb("tsl_cb_daemon_events setting ready pcb", (UINT32)((gIDX[i][j])->pTLogoutPcb));
#endif
                                        TaskSetState((PCB *)(gIDX[i][j])->pTLogoutPcb, PCB_READY);
                                        fprintf(stderr,"ISCSI_DEBUG: tsl_cb_daemon_events() socket (%d) port (%d) lid (%d) waking pcb %p\n",
                                            socket_fd, i, j, gIDX[i][j]->pTLogoutPcb);
                                    }
                                }
                            }
                            else
                            {
                                fsl_logout(i,j);
                            }
                            close(socket_fd);
                            return;
                        }
                    }
                }
            }
            break;

          default:
            break;
        }
    }
    close(socket_fd);
}

/**
******************************************************************************
**
**  @brief      Creates a socket, listens on the socket,
**              socket create is added to epoll
**
**  @param      none
**
**  @return     INT32 - Success / Failure
**
******************************************************************************
**/

INT32 tsl_daemonInit (void)
{
    struct sockaddr_in serv_addr;
    INT32     socket_fd;
    INT32     retVal;

    /*
    ** Create Socket
    */
    socket_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        fprintf(stderr,"ISCSI_DEBUG: tsl_daemonInit: Error Creating socket\n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(ISCSI_PROC_DAEMON_COMM);

    /*
    ** Bind
    */
    retVal = bind (socket_fd, (struct sockaddr*)&serv_addr, sizeof (serv_addr));
    if (retVal < 0) {
        fprintf(stderr,"ISCSI_DEBUG: tsl_daemonInit: Error Bind on socket, %s\n",strerror(errno));
        close(socket_fd);
        return -1;
    }

    /*
    ** listen
    */
    retVal = listen (socket_fd, 50);
    if(retVal < 0) {
        fprintf(stderr,"ISCSI_DEBUG: tsl_daemonInit: Error listen on socket\n");
        close(socket_fd);
        return -1;
    }

    /*
    ** Set socket to Non Blocking
    */
    TSL_SetNonBlocking (socket_fd);

    /*
    ** Add socket to epoll
    */
    tsl_ev_add(socket_fd, EPOLLIN , tsl_cb_acpt_daemon, (void*)socket_fd);
    fprintf(stderr,"ISCSI_DEBUG: iscsi daemon: event added to epoll, sock =%d\n",socket_fd);
    return 0;
}

/**
******************************************************************************
**
**  @brief      Cleanup the Events for epoll
**
**  @param      void * - Reference pointer
**
**  @return     void
**
******************************************************************************
**/

void tsl_cleanupEvents(void *pRef)
{
    INT32 nfds = 0, i;
    struct tsl_ev_dat *ptDat;

    if ( (nfds = gNfds) > 0 )
    {
        for(i = 0; i < nfds; i++)
        {
            if (tsl_ev[i].data.u32 == 0xFFFFFFFF )
            {
                continue;
            }

            ptDat = (struct tsl_ev_dat *)&(tsl_ev[i].data.u64);

            if(ptDat->pRef == pRef)
            {
                ptDat->pRef = NULL;
            }
        }
    }
}


#endif

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
