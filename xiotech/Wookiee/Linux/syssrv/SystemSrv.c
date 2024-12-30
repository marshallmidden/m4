/* $Id $ */
/**
******************************************************************************
**
**  @file       SystemSrv.c
**
**  @brief      CCB "system()" server.
**
**  This file compiled to a stand-alone server that runs to serve system()
**  calls to the CCB.  It must be started as root.
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**     
******************************************************************************
**/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "XIO_Types.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/* Must match that which is defined in xk_kernel.h.  Duplicated here to allow
** it to build without including xk_kernel.h.
*/
#define  SYSTEM_SRV_SOCKET_PATH "/tmp/SystemSrv.socket"

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Socket read routine (loops on read)
**
**  @param      fd     - socket descriptor
**  @param      ptr    - pointer to buffer to receive data
**  @param      nbytes - number of bytes to read
**
**  @return     number of bytes read or -1 on error.
**
******************************************************************************
**/
INT32 readn(INT32 fd, char *ptr, INT32 nbytes)
{
    INT32 nleft, nread;

    nleft = nbytes;
    while (nleft > 0) 
    {
        nread = read(fd, ptr, nleft);
        if (nread < 0)
        {
            return(nread);              /* error, return < 0 */
        }
        else if (nread == 0)
        {
            break;                      /* EOF */
        }

        nleft -= nread;
        ptr   += nread;
    }

    return(nbytes - nleft);             /* return >= 0 */
}

/**
******************************************************************************
**
**  @brief      The main server routine
**
**  @return     Doesn't return
**
******************************************************************************
**/
int main(int argc, char **argv)
{
    INT32   sockfd = -1;
    INT32   newsockfd = -1;
    UINT32  clilen;
    INT32   servlen;
    struct sockaddr_un cli_addr;
    struct sockaddr_un serv_addr;
    INT32   length;
    INT32   rc;
    UINT8   sysRC;
    char   *pCmd = NULL;

    while(1)
    {
        do 
        {
            /*
            ** Unlink the old socket, if it exists.
            */
            unlink(SYSTEM_SRV_SOCKET_PATH);

            /*
            ** Open a socket (a UNIX domain stream socket).
            */
            if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            {
                printf("server: can't open stream socket, errno %d\n", errno);
                break; /* to bottom of do..while(0) */
            }

            /*
            ** Bind our local address so that the client can send to us.
            */
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sun_family = AF_UNIX;
            strcpy(serv_addr.sun_path, SYSTEM_SRV_SOCKET_PATH);
            servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

            if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
            {
                printf("server: can't bind local address, errno %d\n", errno);
                break; /* to bottom of do..while(0) */
            }

            /*
            ** Allow admin and user (wookiee) access.  Again, I'm not
            ** including xk_kernel.h, but this uid and gid used below must
            ** match those defined for XK_ADMIN_UID, XK_USER_GID.
            */
            if ( chown(SYSTEM_SRV_SOCKET_PATH, 0, 1000) < 0)
            {
                printf("server: can't chown socket, errno %d\n", errno);
                break; /* to bottom of do..while(0) */
            }

            if ( chmod(SYSTEM_SRV_SOCKET_PATH, 0770) < 0)
            {
                printf("server: can't chmod socket, errno %d\n", errno);
                break; /* to bottom of do..while(0) */
            }

            /*
            ** Set up as listener.
            */
            if ( listen(sockfd, 5) < 0)
            {
                printf("server: can't listen on socket, errno %d\n", errno);
                break; /* to bottom of do..while(0) */
            }

            /*
            ** Loop on client connections
            */
            while(1)
            {
                /*
                ** Wait for a connection from a client process.
                */
                clilen = sizeof(cli_addr);
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0)
                {
                    printf("server: accept error, errno %d\n", errno);
                    break; /* to bottom of while(1) */
                }

                /*
                ** First word (4 bytes) is length to follow
                */
                rc = readn(newsockfd, (char *)&length, 4);
                if (rc < 0)
                {
                    printf("server: read error 1, errno %d\n", errno);
                    break; /* to bottom of while(1) */
                }

                /*
                ** Malloc a buffer
                */
                if (length >= 0x10000)
                {
                    printf("server: length field too long: %d\n", length);
                    break; /* to bottom of while(1) */
                }
                else
                {
                    pCmd = (char *)malloc(length); 
                }

                /*
                ** Read the command string
                */
                rc = readn(newsockfd, pCmd, length);
                if (rc < 0)
                {
                    printf("server: read error 2, errno %d\n", errno);
                    break; /* to bottom of while(1) */
                }

                /*
                ** Do it!
                */
                /* puts(pCmd); */
                sysRC = (UINT8)(system(pCmd) >> 8);

                /*
                ** Send the return code back
                */
                rc = write(newsockfd, &sysRC, 1);
                if (rc < 0)
                {
                    printf("server: write error, errno %d\n", errno);
                    break; /* to bottom of while(1) */
                }

                /*
                ** Done
                */
                close(newsockfd);
                free(pCmd);
                pCmd = NULL;
                
            } /* fall through to bottom of do..while(0) */

        } while(0);

        /*
        ** If we get here, something bad happened.  Clean up, wait a bit and
        ** then try and set everything up again.
        */
        if (pCmd)
        {
            free(pCmd);
            pCmd = NULL;
        }
        if (sockfd >= 0)
        {
            close(sockfd);
            sockfd = -1;
        }
        if (newsockfd >= 0)
        {
            close(newsockfd);
            newsockfd = -1;
        }

        sleep(1);
    }

    exit(0);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
