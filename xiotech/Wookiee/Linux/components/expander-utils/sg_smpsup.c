
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> 
#include <ctype.h>
#include <dirent.h>
#include "sg_iosup.h"
#include "sg_smpsup.h"

#define DEBUGIO 0

int do_smp_io(int fd, void *smp_req, int smp_req_size, void *smp_resp,
              int smp_resp_size)
{
    int res;
 
    res = write(fd, smp_req, smp_req_size);
    if (res == -1) {
        printf("writing to: %s(%d)\n", strerror(errno), errno);
    } else if (!res) {
        printf("nothing could be written\n");
    }
    if (res <= 0)
        return res;
    res = read(fd, smp_resp, smp_resp_size);
    if (res == -1) {
        printf("reading from %s(%d)\n", strerror(errno), errno);
    } else if (!res) { 
        printf("nothing could be read\n");
    }
    return res;
}

static int get_os_channel(int port)
{
    char *path;
    int rc = 0xffff;
    struct dirent *entry;
    DIR *dir;
 
    path = malloc(128);
    strcpy (path, "/sys/bus/pci/drivers/aic94xx/0000:03:04.0");
    dir = opendir(path);
    if (dir == NULL)
        return rc;

    entry = readdir (dir);
    while (entry != NULL)
    {
        if (strstr(entry->d_name, "host") != NULL)
            break;
        entry = readdir (dir);
    }
    if (entry != NULL) {
        rc = strtol(&(entry->d_name[4]), NULL, 10);
    }
    closedir(dir);
    free(path);
    return rc;
}

int get_smp_portal_path(char *path)
{
    struct dirent *entry;
    DIR *dir;
    int fd;
    char dirname[128];
    int hostnum;

    hostnum = get_os_channel(0);
    sprintf(dirname,
            "/sys/class/scsi_host/host%d/device/sas/ha/ports/0/domain/",
            hostnum);
 
    dir = opendir(dirname);
    if (dir == NULL) {
        printf("ERROR %s doesn't exist\n", dirname);
        return -1;
    }

    entry = readdir (dir);
    while (entry != NULL)
    {
        if (strcmp (entry->d_name, ".") && strcmp (entry->d_name, ".."))
            break;
        entry = readdir (dir);
    }
    if (entry == NULL)
        return -1;
 
    sprintf(path, "%s%s/smp_portal", dirname, entry->d_name);
    closedir(dir);
    fd = open(path, O_RDWR);
    if (fd < 0)
        return -1;
    close(fd);
    return 0;
}
