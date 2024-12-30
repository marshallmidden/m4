
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctype.h>
#include "sg_iosup.h"

#define DEBUGIO 0

static void store_be_n(unsigned long val, unsigned char *addr, int len)
{
    if (len <= 0)
        return;
    while (len) {
        addr[--len] = (unsigned char)(val & 0xFF);
        val >>= 8;
    }
}

static void store_le_n(unsigned long val, unsigned char *addr, int len)
{
    if (len <= 0)
        return;
    while (len--) {
        *addr++ = (unsigned char)(val & 0xFF);
        val >>= 8;
    }
}

void dispbuf(const unsigned char *buffer, int len)
{
    int i, j;
    char dispstr[32];

    for (i = 0, j = 0; i < len; i++, j++) {
        unsigned char   ch = buffer[i];

        if (!(i % 16)) {
            if (i != 0) {
                dispstr[16] = 0;
                printf("    %s", dispstr);
                j = 0;
            }
            printf("\n%02hhX", ch);
            if (isalnum(ch) || ispunct(ch))
                dispstr[j] = ch;
            else
                dispstr[j] = '.';
            continue;
        }
        if (!(i % 4))
            printf(" ");
        printf("%02hhX", ch);
        if (isalnum(ch) || ispunct(ch))
            dispstr[j] = ch;
        else
            dispstr[j] = '.';
    }
    dispstr[j] = 0;
    printf("    %s\n", dispstr);
}

int finish_io(int sg_fd, unsigned char **buffer, unsigned int *tag)
{
    sg_io_hdr_t io_hdr;

    if (read(sg_fd, &io_hdr, sizeof(sg_io_hdr_t)) < 0)
    {
        if (errno != EAGAIN )
        {
            printf("SG_IO read error %d %s\n", errno, strerror(errno));
            close(sg_fd);
            exit(0);
            //return -2;
        }
        return -1;
    }
    else
    {
#if DEBUGIO
        int i;
        
        for (i = 0; i < io_hdr.cmd_len; i++)
            printf("%02hX ", io_hdr.cmdp[i]);
        printf(" done \n");
#endif
        
        free(io_hdr.sbp);
        free(io_hdr.cmdp);
        *tag = (int)io_hdr.usr_ptr;
        *buffer = io_hdr.dxferp ;
        if (io_hdr.info &0x01)
        {
            printf(" sg header-> info %08X host %04hX driver %04hX \n",            io_hdr.info,io_hdr.host_status,io_hdr.driver_status);
            return 2;
        }
        return 1;
    }
}

int do_scsi_io_wait(int sg_fd, const unsigned char *cdb, int cdblen,
                    unsigned char *datap, int dir, int datalen)
{
    unsigned char sense_buffer[32];
    sg_io_hdr_t io_hdr;

    memset(&io_hdr, 0, sizeof io_hdr);

    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = cdblen;
    io_hdr.mx_sb_len = 32;
    io_hdr.dxfer_direction = dir;
    io_hdr.dxfer_len = datalen;

    io_hdr.flags |= SG_FLAG_DIRECT_IO;
    io_hdr.dxferp = datap;
    io_hdr.iovec_count = 0;
    io_hdr.cmdp = (unsigned char *)cdb;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = 15000;         /* 15000 millisecs == 15 seconds */
    /* io_hdr.flags = 0; */         /* take defaults: indirect IO, etc */
    /* io_hdr.pack_id = 0; */
    /* io_hdr.usr_ptr = NULL; */
    memset(&sense_buffer, 0, sizeof sense_buffer);

    if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) 
    {
        printf("SG_IO ioctl error %d %s\n", errno, strerror(errno));
        return -1;
    }
    if ((io_hdr.info & SG_INFO_OK_MASK) == SG_INFO_OK) 
    {
        return datalen - io_hdr.resid;
    }
    else
    {
        printf(" sg header-> info %08X host %04hX driver %04hX\n",
               io_hdr.info, io_hdr.host_status, io_hdr.driver_status);
        return -2;
    }
}

int inquiry(int handle, int vpd, int page, unsigned char *data)
{
    int ret;
    unsigned char cdb[6] = {INQUIRY, vpd, page, 0, 0xff, 0};

    ret = do_scsi_io_wait(handle, cdb, 6, data, SG_DXFER_FROM_DEV, 0xff);
    return ret;
}

int log_sense(int handle, int page, unsigned char *data)
{
    int ret;
    unsigned char cdb[10] = {LOG_SENSE, 0, page, 0, 0, 0, 0, 0, 0xff, 0};

    ret = do_scsi_io_wait(handle, cdb, 10, data, SG_DXFER_FROM_DEV, 0xff);
    return ret;
}

int readcap(int handle, unsigned char *data)
{
    int ret;
    static const unsigned char cdb[10] =
        {READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    ret = do_scsi_io_wait(handle, cdb, 10, data, SG_DXFER_FROM_DEV, 0xff);
    return ret;
}

int read_buffer(int handle, unsigned char mode, unsigned char bufid,
                unsigned int offset, unsigned int length,
                unsigned char *datap, unsigned int tag)
{
    unsigned char cdb[10];

    cdb[0] = READ_BUFFER;
    cdb[1] = mode;
    cdb[2] = bufid;
    store_be_n(offset, &cdb[3], 3);
    store_be_n(length, &cdb[6], 3);
    cdb[9] = 0;
    return do_scsi_io_wait(handle, cdb, 10, datap, SG_DXFER_FROM_DEV, length);
}

int write_buffer(int handle, unsigned char mode, unsigned char bufid,
                 unsigned int offset, unsigned int length,
                 unsigned char *datap, unsigned int tag)
{
    unsigned char cdb[10];

    cdb[0] = WRITE_BUFFER;
    cdb[1] = mode;
    cdb[2] = bufid;
    store_be_n(offset, &cdb[3], 3);
    store_be_n(length, &cdb[6], 3);
    cdb[9] = 0;
    return do_scsi_io_wait(handle, cdb, 10, datap, SG_DXFER_TO_DEV, length);
}

unsigned int get_cdb_lba(unsigned char *cdb)
{
    unsigned int lba = 0;
 
    lba = cdb[2] << 24;
    lba |= (cdb[3] << 16);
    lba |= (cdb[4] << 8);
    lba |= cdb[5];
    return lba;
}

static unsigned int ata_passthru(int handle, unsigned short features,
                                 unsigned char sectorcount, unsigned long long lba,
                                 unsigned char device, unsigned char atacommand,
                                 unsigned char *data)
{
    int ret;
    int dir;
    int len;
    static unsigned char cdb[12] = {0xa1,0xa,0,0,0,0,0,0,0,0,0,0};
    //      cdb[3] = (features & 0xff00)>>8;
    //      cdb[4] = features & 0xff;
    //      cdb[5] = (sectorcount & 0xff00)>>8;
    //      cdb[6] = sectorcount & 0xff;
    //      cdb[7] =  (lba &0xFFll);
    //      cdb[9] =  (lba &0xFF00ll)>>8;
    //      cdb[11] =  (lba &0xFF0000ll)>>16;
    //      /*cdb[10] = (lba &0xFF000000ll)>>24;
    //      cdb[11] = (lba &0xFF00000000ll)>>32;
    //      cdb[12] = (lba &0xFF0000000000ll)>>40;*/
    //      cdb[13] = device;
    //      cdb[14] = atacommand;
    cdb[3] = features & 0xff;
    cdb[4] = sectorcount & 0xff;
    store_le_n(lba, &cdb[5], 3);
    cdb[8] = device;
    cdb[9] = atacommand;

    if (data == NULL)
    {
        dir = SG_DXFER_NONE;
        len = 0;
    } else {
        if (features == 0xd5)
            dir = SG_DXFER_FROM_DEV;
        else
            dir = SG_DXFER_TO_DEV;
        len = sectorcount * 0x200;
    }
    ret = do_scsi_io_wait(handle, cdb, 12, data, dir, len);
    return ret;
}

unsigned int ata_smart_enable(int handle)
{
    return ata_passthru(handle, 0xd8, 0, 0xc24f00, 0, 0xb0, NULL);
}

unsigned int ata_smart_get_temp(int handle)
{
    unsigned short *buffer;
    int ret;

    buffer = malloc(256);
    memset(buffer, 0, 512);
    buffer[0] = 0x0500;
    buffer[1] = 0x0100;
    buffer[2] = 0x0200;

    //      ata_passthru(handle, 0xd6, 1, 0xc24fe0, 0, 0xb0, (unsigned char *)buffer);
    //      memset(buffer, 0, 512);

    ret = ata_passthru(handle, 0xd5, 1, 0xc24fe0, 0, 0xb0,
                       (unsigned char *)buffer);
    if (ret < 0)
        return -1;
    //      dispbuf((unsigned char *)buffer, 256);
    ret = buffer[100];
    free(buffer);
    return ret;
}
