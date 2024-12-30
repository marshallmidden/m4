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
#include <byteswap.h>
#include "sg_iosup.h"
#include "sg_smpsup.h"

/*
  retreives and displays information about the expander 
    
*/
static int vitesse_phy_to_slot(int phy)
{
    switch (phy)
    {
        case 4: return 0;
        case 5: return 4;
        case 6: return 8;
        case 7: return 12;
        case 8: return 1;
        case 9: return 5;
        case 10: return 9;
        case 11: return 13;
        case 12: return 2;
        case 13: return 6;
        case 14: return 10;
        case 15: return 14;
        case 16: return 3;
        case 17: return 7;
        case 18: return 11;
        case 19: return 15;
    }
    /*RETURN NO SLOT FOR ALL OTHERS*/
    return 0xFF;
}

int main(int argc, char *argv[])
{
    char smpdevpath [256];

    int fd ;
    int i,ret;
    int resp_len,req_len;
    int resp_len2,req_len2;
    unsigned int * dwptr;
    unsigned char *smp_req, *smp_resp;
    unsigned char *smp_req2, *smp_resp2;


    get_smp_portal_path(smpdevpath);
    
    fd = open(smpdevpath, O_RDWR);
    if (fd == -1) {
        printf("opening %s: %s(%d)\n",  smpdevpath,
               strerror(errno), errno);
        return fd;
    }

    resp_len =32;
    req_len =16;
    smp_req  = malloc(256);
    smp_resp = malloc(256);
    
    resp_len2 =56;
    req_len2 =16;
    smp_req2  = malloc(256);
    smp_resp2 = malloc(256);
    
    printf("PHY SLOT   INV_DWORD    RUN_DISP    LOSS_SYNC    PHY_RST_PROB  DEVICE     LINK\n");
    for (i=0; i <24; i++)
    { 
        fd = open(smpdevpath, O_RDWR);
        memset(smp_req,0,req_len);
        memset(smp_resp,0x0,resp_len);

        smp_req[0] =0x40;
        smp_req[1] =0x11;
        smp_req[3] =0x00;
        smp_req[9] =i;
        
        smp_req2[0] =0x40;
        smp_req2[1] =0x10;
        smp_req2[3] =0x00;
        smp_req2[9] =i;
        
        ret =do_smp_io(fd,smp_req,req_len,smp_resp,resp_len);
        close (fd);
        fd = open(smpdevpath, O_RDWR);
        ret =do_smp_io(fd,smp_req2,req_len2,smp_resp2,resp_len2);
        close (fd);
        //  dispbuf(smp_resp,resp_len);
        dwptr = (unsigned int *)smp_resp;
        
        printf("%02d   %03d   %08X     %08X    %08X     %08X",i,vitesse_phy_to_slot(i)+1
               ,bswap_32(dwptr[3]),bswap_32(dwptr[4]),bswap_32(dwptr[5]),bswap_32(dwptr[6]));
        if (smp_resp2[14] & 0x0f)
            printf ("      HOST  ");
        else if (smp_resp2[15] & 0x08)
            printf ("      SAS   ");
        else if (smp_resp2[15] & 0x05)
            printf ("      SATA  ");
        else if (smp_resp2[15] & 0x02)
            printf ("      SMP  ");
        else if (smp_resp2[15] & 0x0f)
            printf ("      OTHER ");
        else
            printf ("      NONE  ");
        if (smp_resp2[13]==1)
            printf(" Disabled");
        else if (smp_resp2[13]==2)
            printf(" Reset Prob");
        else if (smp_resp2[13]==3)
            printf(" Spinup hold");
        else if (smp_resp2[13]==8)
            printf(" 1.5 Gbps");
        else if (smp_resp2[13]==9)
            printf(" 3.0 Gbps");
        else 
            printf(" ");
        //      sata affiliation info
        //         if (smp_resp2[15] & 0x06)
        //         {
        //             resp_len =60;
        //             req_len =16;
        //             smp_req[0] =0x40;
        //             smp_req[1] =0x12;
        //             smp_req[3] =0x00;
        //             smp_req[9] =i;
        //             fd = open(smpdevpath, O_RDWR);
        //             ret =do_smp_io(fd,smp_req,req_len,smp_resp,resp_len);
        //             close (fd);
        //             dwptr = (unsigned int *)smp_resp;
        //             
        //             printf(" %02hX %08X%08X" ,smp_resp[11] ,bswap_32(dwptr[12]),dwptr[13]);
        //         }
        printf ("\n");
    }
   
    free(smp_req);
    free(smp_resp);
    return 0;
}

