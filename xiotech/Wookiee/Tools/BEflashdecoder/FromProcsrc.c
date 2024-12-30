#include "nvram.h"


/* ------------------------------------------------------------------------ */
/* Function prototypes. */
void NV_P2ChkSum(NVRII *);
/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @brief      Calculate and record the checksum for PART II of the NVRAM.
**
**              Each word within the PART II area (excluding the checksum word)
**              is summed up and stored. Interrupts are disabled during this
**              operation.
**
**  @param      nvramImage  - Address of the data for the NVRAM.
**
**  @return     none
**
******************************************************************************
**/
void NV_P2ChkSum(NVRII *nvramImage)
{
    UINT32      checksum;       /* Value for checksum                   */
    UINT32     *value;          /* Ptr to the image                     */
    UINT32      words;          /* Words to checksum                    */

    /* Calculate the checksum starting at the field following the
       checksum. This skips the checksum value itself. */
    for (checksum = NR_MAGIC,
         value = (UINT32 *)&nvramImage->rsvd4[0],
         words = (nvramImage->length - 4 + 3) / 4;
         words > 0; words--, checksum += *value, value++) ;

    nvramImage->cSum2 = checksum;
}   /* End 0f NV_P2ChkSum */

