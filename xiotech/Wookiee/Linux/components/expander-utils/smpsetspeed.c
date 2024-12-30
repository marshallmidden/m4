#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h> 
#include <dirent.h>
#include "sg_iosup.h"
#include "sg_smpsup.h"


/*
  changes the max phy speed on the 750's expander for all attached hard drives.
  this is specific to the 750 HW
*/


static void do_smp_op(const char *path, void *smp_req, int req_len,
                      void *smp_resp, int resp_len)
{
    int fd;

    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("smpsetspeed unable to open SMP portal, errno=%d\n", errno);
        exit(1);
    }
    do_smp_io(fd, smp_req, req_len, smp_resp, resp_len);
    close(fd);
}


int main(int argc, char *argv[])
{
    int minspeed = 0x80;
    char smpdevpath[256];
    int newspeed = 0x80;
    //  int fd;
    int resp_len, req_len;
    unsigned char *smp_req, *smp_resp;
    int c;
    int phy;
    int dev_attached;

    while ((c = getopt(argc, argv, "13")) != -1) {
        switch (c) {
            case '1':
                newspeed = 0x80;
                break;
            case '3':
                newspeed = 0x90;
                break;
            default:
            case '?':
                printf("USAGE:  smpsetspeed <speed>  \n");
                printf("    smpsetspeed -1  for 1.5 Gbps\n");
                printf("    smpsetspeed -3  for 3.0 Gbps\n");
                return 0;
        }
    }
 
    if (get_smp_portal_path(smpdevpath) == -1) {
        fprintf(stderr,"ERROR: smpsetspeed unable to get smp_portal path \n");
        return -1;
    }
    if (newspeed == 0x90)
        printf("Setting all drive PHYs to 3.0 Gbps\n");
    else        
        printf("Setting all drive PHYs to 1.5 Gbps\n");
    smp_req  = malloc(1024);
    smp_resp = malloc(1024);
    
    for (phy = 4; phy < 20; phy++)
    {
        // discover
 
        resp_len = 56;
        req_len = 16;
        memset(smp_req, 0, req_len);
        memset(smp_resp, 0x0, resp_len);
        smp_req[0] = 0x40;
        smp_req[1] = 0x10;
        smp_req[3] = 0x00;
        smp_req[9] = phy;
        do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
#if 0
        fd = open(smpdevpath, O_RDWR);
        if (fd < 0) {
            printf("open error\n");
            return -1;
        }
        do_smp_io(fd, smp_req, req_len, smp_resp, resp_len);
        close(fd);
#endif /* 0 */
        if ((smp_resp[41] & 0xf0) == newspeed)
            continue;
        if ((smp_resp[12] & 0x30) == 0)
            dev_attached = 0;
        else 
            dev_attached = 1;
 
        //set speed
        resp_len = 8;
        req_len = 43;
        // printf(" setspeed");
        memset(smp_req, 0, 1024);
        memset(smp_resp, 0x0, 1024);
        smp_req[0] = 0x40;
        smp_req[1] = 0x91;
        smp_req[3] = 0x00;
        smp_req[9] = phy;
        smp_req[10] = 0; //no op
        smp_req[32] = minspeed;
        smp_req[33] = newspeed;
        do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
#if 0
        fd = open(smpdevpath, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "ERROR: smpsetspeed unable to open smp_portal\n");
            return -1;
        }
        do_smp_io(fd, smp_req, req_len, smp_resp, resp_len);
        close(fd);
#endif /* 0 */
    
        if (!dev_attached)
            continue;
        //on

        memset(smp_req, 0, req_len);
        memset(smp_resp, 0, resp_len);
        smp_req[0] = 0x40;
        smp_req[1] = 0x91;
        smp_req[3] = 0x00;
        smp_req[9] = phy;
        smp_req[10] = 2; //hardreset
        smp_req[32] = minspeed;
        smp_req[33] = newspeed;
        do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
#if 0
        fd = open(smpdevpath, O_RDWR);
        if (fd < 0) {
            fprintf(stderr,"ERROR: smpsetspeed unable to open smp_portal\n");
            return -1;
        }
        do_smp_io(fd, smp_req, req_len, smp_resp, resp_len);
        close(fd);
#endif /* 0 */
        resp_len = 60;
        req_len = 16;
    }
    free(smp_req);
    free(smp_resp);
    return 0;
}

