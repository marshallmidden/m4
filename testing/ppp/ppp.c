#include <stdio.h>
#include <stdlib.h>

void FreePacket(int **packet)
{
    int *p = *packet;

    if (p)
    {
        *packet = 0;
    }
    else
    {
        fprintf(stderr, "p == NULL\n");
    }
}

int main()
{

    int *rc = NULL;

    fprintf(stderr, "&rc=%p\n", &rc);

    rc = malloc(sizeof(*rc));
    fprintf(stderr, "rc=%p\n", rc);

    FreePacket(&rc);
    fprintf(stderr, "&rc=%p\n", &rc);
    fprintf(stderr, "rc=%p\n", rc);

    exit(0);
}
