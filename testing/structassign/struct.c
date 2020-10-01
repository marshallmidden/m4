#include <stdio.h>

struct testing {
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
} abc = {1,2,3,4};


static void routine(struct testing *v)
{
    struct testing *l;

    l = &abc;

    *v = *l;
    fprintf(stderr, "l=%p l->a=%d v=%p v->a=%d\n", l, l->a, v, v->a);
}

int main()
{
    struct testing local;
    routine(&local);
    fprintf(stderr, "&local=%p local.a=%d\n", &local, local.a);
    return(0);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
