#include <stdio.h>


#define FRONTEND
/* #define BACKEND */
/* #define CCB_RUNTIME_CODE */


#include "../../Shared/Src/L_XIO3D.c"


void zeromemory()
{
    unsigned int i;

    /* Clear Sared Memory */
    for (i = 0; i< SIZE_FE_LTH; i+=4) 
    {
        *(INT32*)(FE_BASE_ADDR +  i) = 0;
    }
    for (i = 0; i< SIZE_BE_LTH; i+=4) 
    {
        *(INT32*)(BE_BASE_ADDR +  i) = 0;
    }
    for (i = 0; i< SIZE_CCB_LTH; i+=4) 
    {
        *(INT32*)(CCB_BASE_ADDR + i) = 0;
    }
}

int main()
{
    fprintf(stderr, "in main\n");

    SETUP_linux();

    fprintf(stderr, "before zeroing memory\n");
    zeromemory();
    fprintf(stderr, "after zeroing memory\n");
    exit(0);
}
