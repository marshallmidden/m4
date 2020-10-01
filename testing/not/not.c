#include <stdio.h>


int main()
{
    int a = strcmp("hi","hi");
    int b = !strcmp("hi","hi");
    int c = strcmp("hi","there");
    int d = !strcmp("hi","there");
    int e = strcmp("there","ist");
    int f = !strcmp("there","ist");
    fprintf(stderr, "strcmp: [0,1] (%d, %d), [-1,0] (%d, %d), [1,0] (%d, %d)\n", a,b,c,d,e,f);

/*     a = strcmp("hi","hi", 0); */
/*     b = !strcmp("hi","hi", 0); */
/*     fprintf(stderr, "strcmp 0: [0,1] (%d, %d)\n", a,b); */

    a = strncmp("hi","hi", 2);
    b = !strncmp("hi","hi", 2);
    c = strncmp("hi","there", 2);
    d = !strncmp("hi","there", 2);
    e = strncmp("there","ist", 2);
    f = !strncmp("there","ist", 2);
    fprintf(stderr, "strncmp: [0,1] (%d, %d), [-1,0] (%d, %d), [1,0] (%d, %d)\n", a,b,c,d,e,f);

    a = strncmp("hi","hi", 0);
    b = !strncmp("hi","hi", 0);
    fprintf(stderr, "strncmp 0: [0,1] (%d, %d)\n", a,b);

    a = bcmp("hi","hi", 2);
    b = !bcmp("hi","hi", 2);
    c = bcmp("hi","there", 2);
    d = !bcmp("hi","there", 2);
    e = bcmp("there","ist", 2);
    f = !bcmp("there","ist", 2);
    fprintf(stderr, "bcmp: [0,1] (%d, %d), [-1,0] (%d, %d), [1,0] (%d, %d)\n", a,b,c,d,e,f);

    a = bcmp("hi","hi", 0);
    b = !bcmp("hi","hi", 0);
    fprintf(stderr, "bcmp 0: [0,1] (%d, %d)\n", a,b);

    a = memcmp("hi","hi", 2);
    b = !memcmp("hi","hi", 2);
    c = memcmp("hi","there", 2);
    d = !memcmp("hi","there", 2);
    e = memcmp("there","ist", 2);
    f = !memcmp("there","ist", 2);
    fprintf(stderr, "memcmp: [0,1] (%d, %d), [-1,0] (%d, %d), [1,0] (%d, %d)\n", a,b,c,d,e,f);

    a = memcmp("hi","hi", 0);
    b = !memcmp("hi","hi", 0);
    fprintf(stderr, "memcmp 0: [0,1] (%d, %d)\n", a,b);

    exit(0);
}
