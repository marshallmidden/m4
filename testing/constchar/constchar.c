#include <stdio.h>
#include <stdlib.h>

#define UNUSED          __attribute__ ((__unused__))

#define MAX_FAULT_POINTS      32  // 1 longword's worth of faults - matches frdv.Fault[] & FaultClear[]
typedef unsigned long long PTR_SIZE_t;

typedef struct  frdv
{
    PTR_SIZE_t BaseFaultHelp;
} FDRV_t;



struct frdv _frdv; // Don't use this directly, use the pointer below

struct frdv  *GBL_frdv_p = &_frdv;

#define GBL_FAULT_GET_HELP_STRING(COMP,POINT) \
  ((char const**)(GBL_frdv_p->BaseFaultHelp + ( ((COMP) * MAX_FAULT_POINTS + (POINT)) * sizeof(char*))))

#define GBL_FAULT_SET_HELP_STRING(COMP,POINT,STRING) \
  if (GBL_frdv_p->BaseFaultHelp ) \
    { \
    char const * *STR; \
    STR = GBL_FAULT_GET_HELP_STRING(COMP,POINT); \
    *STR = STRING; \
    }

#define CM_NUM	5

#define CM_TJF_PreWarp 6
#define CM_TJF_InitCold 7

int main(UNUSED int argc, UNUSED char **ARGV)
{
    char **fakeit[4*MAX_FAULT_POINTS];

    GBL_frdv_p->BaseFaultHelp = (PTR_SIZE_t)fakeit;

    if (GBL_frdv_p->BaseFaultHelp )
    {
	char const** STR;
//	STR = GBL_FAULT_GET_HELP_STRING(CM_NUM, CM_TJF_PreWarp);
	STR = ((char const**)(GBL_frdv_p->BaseFaultHelp + ( ((CM_NUM) * MAX_FAULT_POINTS + (CM_TJF_PreWarp)) * sizeof(char*))));
	*STR = "hi there";
    }

    GBL_FAULT_SET_HELP_STRING ( CM_NUM, CM_TJF_PreWarp, "CM Pre-WARP init " );
    GBL_FAULT_SET_HELP_STRING ( CM_NUM, CM_TJF_InitCold, "CM Init Cold" );
    exit (0);
}
