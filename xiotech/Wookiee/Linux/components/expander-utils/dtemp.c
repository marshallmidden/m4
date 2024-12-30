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
#include <sys/wait.h> 
#include "sg_iosup.h"
#include "sg_smpsup.h"


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
    unsigned long long phytoport[20];
    unsigned long long portid;
    int fd;
    int i,phy,c,verbose=0;
    int resp_len,req_len;
    int tempdata[16];
    int res;
    unsigned char *smp_req, *smp_resp,* page83, *inq;


    int slot;
    while (1)
    {
   
        c = getopt (argc, argv, "vh?");
        if (c == -1)
            break;
        //   printf(" %c arg %s  index %d\n",c,optarg,option_index);
        switch (c) 
        {

            case 'v':
                verbose =1;
                break;
            case '?':
            case 'h':      
                printf("dtemp -- displays temp of mag750 hardrives and cpu \n");
                printf("      -- option -v  verbose output \n");
                return 0;
        }
    }
    get_smp_portal_path(smpdevpath);
    smp_req  = malloc(1024);
    smp_resp = malloc(1024);
    for (i=0 ;i < 20 ; i++)
    {
        resp_len =64;
        req_len =16;
        
        memset(smp_req,0,req_len);
        memset(smp_resp,0x0,resp_len);
        smp_req[0] =0x40;
        smp_req[1] =0x10;
        smp_req[3] =0x02;
        smp_req[9] =i;
        fd = open(smpdevpath, O_RDWR);
        do_smp_io(fd,smp_req,req_len,smp_resp,resp_len);
        // dispbuf(smp_resp,64);
        phytoport[i] = *((unsigned long long*)&smp_resp[24]);
        //  printf( " Port %016llX phy %d \n",phytoport[i],i);
        
        close (fd);
    }
    free(smp_req);
    free(smp_resp);
    page83 = malloc(1024);
    inq = malloc(1024);
    for (slot =0 ; slot <16; slot++)
        tempdata[slot] =0xff;
    for (i=1; i <20; i++)
    {
        sprintf(smpdevpath,"/dev/sg%d",i);
        fd = open(smpdevpath,O_RDWR |O_NONBLOCK);
        if (fd <0)
            continue;
        if (inquiry(fd,0,0,inq) <0)
        {
            slot = 0xff;
        }
        res = inquiry(fd,1,0x83,page83);
        if (res < 0)
        { 
            printf(" %s inquiry failed \n",smpdevpath);
            continue;
        }
        if (inq[0] ==0x0d) 
        {
            close(fd);
            continue;
        }
        if ((inq[8] =='A') && (inq[0x10] =='S') && (page83[0x4]== 1))
            portid = *((unsigned long long*)&page83[0x14]);
        else  if ((inq[8] =='A') && (inq[0x10] =='S'))
            portid = *((unsigned long long*)&page83[0x50]);
        else if ((inq[8] =='A') && (inq[0x10] =='H'))
            portid = *((unsigned long long*)&page83[0x14]);
        else if (inq[0x23] ==0x32)
        {
            portid = *((unsigned long long*)&page83[0x8]);
            portid = portid | 0x0100000000000000ll;
        }
        else
            portid = *((unsigned long long*)&page83[0x14]);
        
        for (slot =0; slot <16; slot++)
        {
            // printf("Testing %016llX  == %016llX \n",portid ,phytoport[slot+4]);
            if (portid == phytoport[slot+4])
                break;
        }
        phy = slot+4;
        slot = vitesse_phy_to_slot(slot+4);
        if (verbose)
            printf(" %s %016llX slot %02d phy %02d\n",smpdevpath,portid ,slot+1,phy);
        if (inq[8] !='A')
        {
            log_sense(fd,0x4d,page83);
            //dispbuf(page83,24);
            tempdata[slot] = page83[9];
        }
        else
        {
            tempdata[slot] = ata_smart_get_temp(fd);
            if (tempdata[slot]  ==-1)
            {
                printf(" %s get temp failed \n",smpdevpath);
                tempdata[slot] = 0xff;
            }
        }
        close(fd);
    }
    free(page83);
    free(inq);
    printf("Drives Temps");
    for (slot =0 ; slot <16; slot++)
    { 
        int f;
        if ((slot %4) ==0)
            printf("\n");
        if (tempdata[slot] != 0xff) {
            f= ((9*tempdata[slot])/5)+32;
            printf(" %02d: %02d C %02d F ",slot+1,tempdata[slot],f);
        } 
        else
            printf(" %02d: ----------",slot+1 );
    }
    printf("\n");

    char tempstr[128];
    FILE *sysfd = fopen("/sys/bus/i2c/devices/0-002f/temp1_input","r");
    if (sysfd)
    {
        fgets(tempstr,16,sysfd);
        tempstr[3] =tempstr[2];
        tempstr[2] ='.';
        tempstr[4] =0;
        printf("CPU 0 %s C\n",tempstr);
        fclose(sysfd);
    }
    sysfd = fopen("/sys/bus/i2c/devices/0-002f/temp3_input","r");
    if (sysfd)
    {
        fgets(tempstr,16,sysfd);
        tempstr[3] =tempstr[2];
        tempstr[2] ='.';
        tempstr[4] =0;
        printf("MB    %s C\n",tempstr);
        fclose(sysfd);
    }
    //     fd = system ("cat /sys/bus/i2c/devices/0-002f/temp1_input");
    //     wait(&i);
    // 
    //     fd = system ("cat /sys/bus/i2c/devices/0-002f/temp2_input");
    //     wait(&i);
    // 
    //     fd = system ("cat /sys/bus/i2c/devices/0-002f/temp3_input");
    //     wait(&i);
    //     printf("\n");
    return 0;
}

