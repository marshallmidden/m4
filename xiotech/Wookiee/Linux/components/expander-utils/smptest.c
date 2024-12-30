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
#include <getopt.h>
#include "sg_iosup.h"
#include "sg_smpsup.h"

static void do_smp_op(const char *path, void *smp_req, int req_len,
                      void *smp_resp, int resp_len)
{
    int fd;

    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("smptest unable to open SMP portal (%s), errno=%d\n", path, errno);
        exit(1);
    }
    do_smp_io(fd, smp_req, req_len, smp_resp, resp_len);
    close(fd);
}

int main(int argc, char *argv[])
{
    char smpdevpath[256];
    int c;
    int resp_len, req_len;
    int action = 0xff, maxspeed = 0xff;
    unsigned char *smp_req, *smp_resp;
    int phy = 0xff;
    int option_index = 0;
    static struct option long_options[] = {
        {"on", 0, 0, 0},
        {"off", 0, 0, 0},
        {"reset", 0, 0, 0},
        {"disc",  0, 0, 0},
        {"info",  0, 0, 0},
        {"phy", 1, 0, 'p'},
        {"speed", 1, 0, 's'},
        {0, 0, 0, 0}
    };

    get_smp_portal_path(smpdevpath);
    while (1)
    {
        c = getopt_long_only(argc, argv, "p:s:",
                             long_options, &option_index);
        if (c == -1)
            break;
        //   printf(" %c arg %s  index %d\n", c, optarg, option_index);
        switch (c) 
        {
            case 0:
                switch (option_index)
                {
                    case 0:
                    case 2:
                        action = 2;
                        break;
                    case 1: 
                        action = 3;
                        break;
                    case 3:
                        action = 4;
                        break;
                    case 4:
                        action = 5;
                        phy = 1; //get man info no phy needed but need to get past check
                        break;
                    case 5:
                        printf("option %s with optarg %s\n",
                               long_options[option_index].name, optarg);
                        phy = atoi(optarg);
                        break;
                }
                break;
            case 'p': 
                phy = atoi(optarg);
                if (phy > 25 || phy < 0) {
                    printf(" !!! invalid phy %d \n", phy);
                    return -1;
                }
                break;
            case 's': 
                maxspeed = atoi(optarg);
                if (maxspeed == 1)
                    maxspeed = 0x80;
                else if (maxspeed == 3)
                    maxspeed = 0x90;      
                else
                    maxspeed = 0xff;
                break;
        }
    }
    printf("action %d phy %d \n", action, phy);
    if (phy == 0xff || action == 0xff)
    {
        printf(" !!!please supply a phy and an action \n");
        printf(" !!!  --phy=<phy> --reset|--disc|--on|--off|--info \n");
        return -1;
    }
    smp_req  = malloc(1024);
    smp_resp = malloc(1024);
    resp_len = 56;
    req_len = 16;
    memset(smp_req, 0, req_len);
    memset(smp_resp, 0x0, resp_len);

    smp_req[0] = 0x40;
    smp_req[1] = 0x10;
    smp_req[3] = 0x00;
    smp_req[9] = phy;
    do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
    if (action == 4)
    {
        dispbuf(smp_resp, 56);
    }
    else if (action == 5)
    {
        resp_len = 64;
        req_len = 8;
        memset(smp_req, 0, req_len);
        memset(smp_resp, 0x0, resp_len);

        smp_req[0] = 0x40;
        smp_req[1] = 0x1;
        do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
        dispbuf(smp_resp, 64);
    }
    else
    {
        if (maxspeed == 0xff)
            maxspeed = smp_resp[44] & 0xf0;
 
        resp_len = 8;
        req_len = 43;
        memset(smp_req, 0, req_len);
        memset(smp_resp, 0x0, resp_len);
        smp_req[0] = 0x40;
        smp_req[1] = 0x91;
        smp_req[3] = 0x00;
        smp_req[9] = phy;
        smp_req[10] = action;
        smp_req[32] = 0x80;
        smp_req[33] = maxspeed;
        do_smp_op(smpdevpath, smp_req, req_len, smp_resp, resp_len);
        printf("REQ:");
        dispbuf(smp_req, req_len);
        printf("RESP:");
        dispbuf(smp_resp, resp_len);
 
        resp_len = 56;
        req_len = 16;
    }
    free(smp_req);
    free(smp_resp);
    return 0;
}

