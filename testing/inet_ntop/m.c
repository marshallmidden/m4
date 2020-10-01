#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    char szIpAddr[INET6_ADDRSTRLEN] = "0.0.0.0";

    struct in_addr ia = {0};

    if(NULL == inet_ntop(AF_INET, &ia, szIpAddr, INET6_ADDRSTRLEN))
    {
      fprintf(stderr, "NULL returned\n");
    }
    fprintf(stderr, "szIpAddr=(%s)\n", szIpAddr);

    return(0);
}
