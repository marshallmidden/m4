#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mvf.cpp"

int main(int argc, char **argv)
{
    int a = foo();
    fprintf(stderr, "a=%d\n", a);
    exit(0);
}

