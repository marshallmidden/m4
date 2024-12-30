/* $Id: iscsi_tsl.h 157459 2011-08-03 14:38:37Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_tsl.h
 **
 **  @brief      Transport sub layer functions header file
 **
 **  This provides API's for transport sub layer, epoll
 **
 **  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#ifndef __ISCSI_TP_H
#define __ISCSI_TP_H

#define TSL_EPOLL_MAX_INTERFACE          4
#define TSL_EPOLL_MAX_SERVER_SUPPORTED   32
#define TSL_EPOLL_MAX_WAIT_EVENTS        68
//#define TSL_EPOLL_MAX_WAIT_EVENTS      TSL_EPOLL_MAX_INTERFACE + (TSL_EPOLL_MAX_SERVER_SUPPORTED * 2)

#define TSL_EPOLL_MAX_EVENTS             10
#define TSL_EPOLL_MAX_TIMEOUT            10000

#define TSL_ISCSI_EVENT_ACCEPT           1
#define TSL_ISCSI_EVENT_ACCEPT_PROCESS   2
#define TSL_ISCSI_EVENT_ACCEPT_REJECT    3
#define TSL_ISCSI_EVENT_ACCEPT_RESET     4
#define TSL_ISCSI_EVENT_READ             5
#define TSL_ISCSI_EVENT_WRITE            6
#define TSL_ISCSI_EVENT_READ_COMP        7
#define TSL_ISCSI_EVENT_WRITE_COMP       8
#define TSL_ISCSI_EVENT_CLOSE_CONN         9     /* closing a TCP connection */

#define TSL_ACCEPT_CONNECTION        1
#define TSL_CONNECTED                2
#define TSL_DISCONNECTED                3
#define    TSL_ISCSI_EVENT_CLOSE_CONNECTION 4

#define TSL_ERROR_ON_CONN       -1
#define TSL_TRY_AGAIN_CONN      -2
#define TSL_PARTIAL_SEND        -3

#define TSL_RESOURCE_UNAVIAL     1
#define ISCSI_PARTIAL_SEND       2
#define ISCSI_CLOSE_CONN_STATE   3

#define MY_IFUP                             1
#define MY_IFDOWN                           2
#define MY_ARPON                            3
#define MY_ARPOFF                           4
#define MAC_ADDR_LEN                        6
#define IP_ADDR_LEN                         4
#define ARP_FRAME_TYPE                      0x0806
#define ETHER_HW_TYPE                       1
#define IP_PROTO_TYPE                       0x0800
#define OP_ARP_REQUEST                      1
#define OP_ARP_REPLY                        2

struct arp_packet {
    UINT8  targ_hw_addr[MAC_ADDR_LEN];
    UINT8  src_hw_addr[MAC_ADDR_LEN];
    UINT16 frame_type;
    UINT16 hw_type;
    UINT16 prot_type;
    UINT8  hw_addr_size;
    UINT8  prot_addr_size;
    UINT16 op;
    UINT8  sndr_hw_addr[MAC_ADDR_LEN];
    UINT8  sndr_ip_addr[IP_ADDR_LEN];
    UINT8  rcpt_hw_addr[MAC_ADDR_LEN];
    UINT8  rcpt_ip_addr[IP_ADDR_LEN];
    UINT8  padding[18];
};

extern void TSL_SetNonBlocking (int);

extern int ISCSI_TSL_Event_Notify (ISCSI_TPD *pTPD, int Event, char* buffer, int length);
extern ISCSI_TPD* tsl_AcptConnection (ISCSI_TPD*);
extern int tsl_InitConnection (UINT32 addr, UINT16 tid, UINT8 port);
extern int tsl_RejectConnection (int socket_fd);
extern int tsl_send_data (ISCSI_TPD* pTPD, char* buff1, int len1, char* buff2, int len2);
extern int tsl_recv_data (ISCSI_TPD* pTPD, char* buffer, int length);
extern int tsl_CloseConnection(ISCSI_TPD* pTPD);
extern void tsl_add_output_event (ISCSI_TPD* pTPD);
extern void tsl_remove_output_event (ISCSI_TPD* pTPD);
extern int tsl_send(ILT* pILT);
extern void tsl_init(void);
extern UINT32 tsl_IsPortUp(UINT8 port);
extern int tsl_if_set(UINT8 port, UINT8 req);

extern void tsl_addAddr(UINT8 port, UINT32 addr, UINT16 tid, UINT32 ipPrefix);
extern void tsl_delAddr(UINT8 port, UINT32 addr, UINT16 tid, UINT32 ipPrefix, UINT32 ipGw);
extern INT32  tsl_ev_add(INT32 fd, UINT32 events, void *func, void *pRef);
extern INT32  tsl_ev_del(INT32 fd, UINT32 events, void *func, void *pRef);
extern INT32  tsl_ev_mod(INT32 fd, UINT32 events, void *func, void *pRef);

extern int tsl_arpUpdate (UINT8 port, UINT32 addr);
extern INT32 tsl_recv_dataOut (ISCSI_TPD *pTPD, SGL_DESC* pSglDesc, INT32 length, UINT8, UINT32 buffOffset);
extern UINT8 iscsi_TargetCleanup(UINT8 port, UINT16 tid);

#endif  /* __ISCSI_TP_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
