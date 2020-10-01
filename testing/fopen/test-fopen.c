#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>

#define M4DEBUG

static void doit(void)
{
    struct sockaddr_un un;
    int sock = 21, fd;

#ifdef M4DEBUG
FILE *M4= fopen("/tmp/M4-debug-libvirt", "a");
fprintf(M4, "entered unix_listen_saddr\n");
fflush(M4);
#endif  // M4DEBUG

#ifdef M4DEBUG
fprintf(M4, "memset 0 &un for length sockaddr_un %zu\n", sizeof(un));
fflush(M4);
memset(&un, 0, sizeof(struct sockaddr));
fprintf(M4, "memset 0 &un for length sockaddr %zu\n", sizeof(struct sockaddr));
fflush(M4);
#endif  // M4DEBUG

    un.sun_family = AF_UNIX;
    snprintf(un.sun_path, sizeof(un.sun_path), "%s", "The path name");

#ifdef M4DEBUG
fprintf(M4, "calling bind(%d, , %zu)\n", sock, sizeof(un));
fflush(M4);
fprintf(M4, "struct sockaddr_un  sun_family=%d, sun_path='%s'\n", un.sun_family, un.sun_path);
fflush(M4);
fclose(M4);
M4=NULL;
#endif  // M4DEBUG

}


int main(void)
{
    doit();
    exit(0);
}
