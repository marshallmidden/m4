/* $Id: async_nv.c 161678 2013-09-18 19:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       async_nv.c
**
**  @brief      Functions to handle Non-volatile activity for Async replication
**              module.
**
**  Copyright (c) 2007-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "async_nv.h"
#include "defbe.h"

#include "CT_defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Const.h"
#include "NV_Memory.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "apool.h"
#include "system.h"
#include "QU_Library.h"
#include "pm.h"
#include "mem_pool.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
//#define dave_dbg    1

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/* Wait for up to (132*5) seconds [11 minutes] for mirror partner to appear. */
#define WAIT_FOR_MP_TIMES   132
//#define WAIT_FOR_MP_TIMES   24

#define ASYNC_NV_FILE   "/opt/xiotech/procdata/P7NV.dat"

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
APOOL_NV_IMAGE gApoolnvImage;
APOOL_NV_IMAGE gTempImage;
UINT8 p7id[4] = {"P7BE"}; /* Identification string to store at the starting of
                             the Apool NV record*/
extern UINT32 gMMCFound;
UINT32 gAsyncNVRecovered;
UINT8  gAsyncApoolOwner = FALSE;

UINT8  gApoolActive = 0;
PCB* gApoolOwnshipPCB = NULL;

extern void CT_LC_AR_ApoolOwnershipTask(UINT32, UINT32, UINT32, UINT32);
void AR_ApoolOwnershipTask(UINT32, UINT32, UINT32, UINT32);

void AR_NVUpdateTask (void);
extern UINT16   ap_reject_puts;
extern UINT32   gApoolOwner;

QU gAsyncNVque;
/*
******************************************************************************
** Public function prototypes - in other files
******************************************************************************
*/
extern INT32   MM_Write(UINT32 destAddr, UINT8 *srcBfr, UINT32 length);
extern INT32   MM_Read(UINT32 srcAddr, UINT8 *destBfr, UINT32 length);
void    AR_UpdateMemFromNV(void);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
extern UINT32 DLM_send_async_nva(UINT32);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief  Save the async NV image to a file
**
**          This routine is called just before shutting down the MM board
**          in order to save the async NV to a file so it can be used when
**          the controller is brought back up.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void AR_SaveNVtoFile(void)
{
    FILE    *f;
    size_t  sz;

    f = fopen(ASYNC_NV_FILE, "w");
    if (!f)
    {
        fprintf(stderr, "%s: Unable to save P7 NV, errno=%d\n",
                __func__, errno);
        return;
    }

    sz = fwrite(&gApoolnvImage, sizeof(gApoolnvImage), 1, f);
    if (sz != 1)
    {
        fprintf(stderr, "%s: Unable to write P7 NV, errno=%d\n",
                __func__, errno);
        return;
    }

    fclose(f);
    sync();
    sync();
}


/**
******************************************************************************
**
**  @brief     This function reads the Apool NV image from the MM card to a
**             temporary buffer and compares it with the global image.
**             This function is written for unit test purpose.
**  @param
**
**  @return
**
**  @attention
**
******************************************************************************
**/
void AR_VerifyApoolNVImage(void)
{
    INT32 i = 0xff;

    /* clear the temp image */
    memset ((void*)&gTempImage, 0, sizeof(APOOL_NV_IMAGE));

    MM_Read((UINT32)NV_P7BE_START,(UINT8*)&gTempImage, sizeof(APOOL_NV_IMAGE));

    i =  memcmp((void*)&gTempImage, (void*)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

    if(i==0)
    {
        fprintf(stderr,"%s: Write to MM card is successful\n",__func__);
    }

}


/**
******************************************************************************
**
**  @brief  Fetch async NV image from file
**
**  @param  none
**
**  @return TRUE if unable to restore async NV, FALSE if successful
**
******************************************************************************
**/
static int  FetchAsyncNVfromFile(void)
{
    FILE    *f;
    size_t  count;
    UINT32  rc;

    fprintf(stderr, "%s: Opening Async NV file\n", __func__);
    f = fopen(ASYNC_NV_FILE, "r");
    if (!f)
    {
        fprintf(stderr, "%s: fopen failed with errno=%d\n",
                __func__, errno);
        return TRUE;
    }

    count = fread(&gApoolnvImage, sizeof(gApoolnvImage), 1, f);
    if (count != 1)
    {
        fprintf(stderr, "%s: fread returned %d, errno=%d\n",
                __func__, count, errno);
        return TRUE;
    }

    fclose(f);                  /* Close the file */

    rc = memcmp(&gApoolnvImage.id, &p7id, sizeof(gApoolnvImage.id));
    if (rc != 0)
    {
        return TRUE;
    }

    rc = MM_Write(NV_P7BE_START, (void *)&gApoolnvImage, sizeof(gApoolnvImage));
    if (rc != 0)
    {
        fprintf(stderr,"%s: MM_Write FAILED\n", __func__);
    }

    return FALSE;
}


/**
******************************************************************************
**
**  @brief  Remove async NV file
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void RemoveAsyncNVFile(void)
{
    int rc;

    rc = unlink(ASYNC_NV_FILE);
    if (rc == -1)
    {
        fprintf(stderr, "%s: Unlink of async NV file failed, errno = %d\n",
                __func__, errno);
    }
    else
    {
        fprintf(stderr, "%s: Unlink of async NV file succeeded\n", __func__);
    }
}


/**
******************************************************************************
**
**  @brief      This function reads the Apool NV image from MicroMemory card
**              and validates the id / checksum and updates the global image
**              in the RAM.    This function can be called from apool init
**              function which should happen in online module before virtual
**              layer initialization.
**
**  @param      flag = 0 if doing long check, 1 for quick MM card check.
**
**  @return     GOOD if the Apool NV record in the MM card is valid
**              ERROR if it is invalid
**  @attention
**
******************************************************************************
**/
UINT32 AR_RecoverApoolNVImage(UINT32 flag)
{
    UINT32 err_ret;
    UINT32 value1 = 0;
    UINT32 value2 = 0;
    INT32  ret_val = 0;

    /*
    ** Check whether the Micromemory card is found and initialized
    */
    if(gMMCFound != TRUE)
    {
        err_ret = ERROR;  /*Think about sending a special error for this situation*/
        logAPOOLevent(AP_ASYNC_NVRAM_RESTORED_BAD, LOG_AS_ERROR, 2, 2, 0);
        gAsyncNVRecovered = FALSE;
        return(err_ret);
    }
    else
    {
        ret_val = MM_Read((UINT32)NV_P7BE_START, (UINT8*)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));
        if (ret_val != 0)
        {
            fprintf(stderr, "%s: MM_Read FAILED\n", __func__);
            err_ret = ERROR;
            logAPOOLevent(AP_ASYNC_NVRAM_RESTORED_BAD, LOG_AS_ERROR, 3, 3, 0);
            gAsyncNVRecovered = FALSE;
            return err_ret;
        }
    }
    if (flag == 1)              /* If quick check for MM board in this system. */
    {
        return(GOOD);
    }

    /*
    ** Check whether the data is valid on the card.
    */
    if (memcmp(&gApoolnvImage.id, &p7id, sizeof(gApoolnvImage.id)) != 0)
    {
        if (FetchAsyncNVfromFile())
        {
            err_ret = ERROR;
            logAPOOLevent(AP_ASYNC_NVRAM_UNCONFIGURED, LOG_AS_INFO, 0, 0, 0);
            gAsyncNVRecovered = FALSE;
            return err_ret;
        }
    }

    RemoveAsyncNVFile();

    if (gApoolnvImage.header.apool_id >= 16)
    {
        err_ret=10;
        value1 = (UINT32) gApoolnvImage.header.apool_id;
    }
    else if(gApoolnvImage.header.status >= 0x0f00)
    {
        err_ret=11;
        value1 = (UINT32) gApoolnvImage.header.status;
    }
    else if(gApoolnvImage.header.element_count > 5)
    {
        err_ret=12;
        value1 = (UINT32) gApoolnvImage.header.element_count;
    }
    else if(gApoolnvImage.header.cur_head_element >= gApoolnvImage.header.element_count)
    {
        err_ret=13;
        value1 = (UINT32) gApoolnvImage.header.cur_head_element;
    }
    else if(gApoolnvImage.header.cur_tail_element >= gApoolnvImage.header.element_count)
    {
        err_ret=14;
        value1 = (UINT32) gApoolnvImage.header.cur_tail_element;
    }
    else if(gApoolnvImage.header.apool_size == 0)
    {
        err_ret=15;
        value1 = (UINT32) gApoolnvImage.header.apool_size;
    }
    else if(gApoolnvImage.header.time_threshold >= 10000)
    {
        err_ret=16;
        value1 = (UINT32) gApoolnvImage.header.time_threshold;
    }
    else if(gApoolnvImage.header.mb_threshold >= 10000)
    {
        err_ret=17;
        value1 = (UINT32) gApoolnvImage.header.mb_threshold;
    }
// Should be MAX_ALINKS, but customers already have more than 47 ...
// Must allow those to continue to work.
    else if(gApoolnvImage.header.alink_count > 512)
    {
        err_ret=18;
        value1 = (UINT32) gApoolnvImage.header.alink_count;
    }
    else if(gApoolnvImage.header.element_entry_size != sizeof(APOOL_NV_ELEMENT))
    {
        err_ret=19;
        value1 = (UINT32) gApoolnvImage.header.element_entry_size;
    }
    else
    {
        fprintf(stderr,"%s: Data Recovered from MM is GOOD\n",__func__);
        gAsyncNVRecovered = TRUE;
        err_ret = GOOD;
        logAPOOLevent(AP_ASYNC_NVRAM_RESTORED_OK, LOG_AS_INFO, 0, 0, 0);
    }

    if(err_ret != GOOD)
    {
        logAPOOLevent(AP_ASYNC_NVRAM_RESTORED_BAD, LOG_AS_ERROR, err_ret, value1,value2);
        err_ret = ERROR;
        gAsyncNVRecovered = FALSE;
    }

    return(err_ret);
}

/**
******************************************************************************
**
**  @brief      Update the apool information with that from NVRAM.
**
**
**  @param      none
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/

void AR_UpdateMemFromNV(void)
{
    int     i;

    apool.id = gApoolnvImage.header.apool_id;
    apool.status = gApoolnvImage.header.status;
    apool.cur_head_element = gApoolnvImage.header.cur_head_element;
    apool.cur_tail_element = gApoolnvImage.header.cur_tail_element;
    apool.head_shadow = gApoolnvImage.element[gApoolnvImage.header.cur_head_element].head;
    apool.tail_shadow = gApoolnvImage.element[gApoolnvImage.header.cur_tail_element].tail;
    apool.cur_head_shadow_element = gApoolnvImage.header.cur_head_element;
    apool.cur_tail_shadow_element = gApoolnvImage.header.cur_tail_element;
    apool.length = gApoolnvImage.header.apool_size;
    apool.sequence_count = gApoolnvImage.header.sequence_count;
    apool.last_seq_count = gApoolnvImage.header.last_seq_count;
    apool.element_count = gApoolnvImage.header.element_count;
    apool.alink_count = gApoolnvImage.header.alink_count;
    apool.version = gApoolnvImage.header.version;
    for (i = 0; i < MAX_ELEMENTS; i++)
    {
        apool.element[i].apool_id = apool.id;
        apool.element[i].status = gApoolnvImage.element[i].status;
        apool.element[i].vid = gApoolnvImage.element[i].vid;
        apool.element[i].jump_to_element = gApoolnvImage.element[i].jump_to_element;
        apool.element[i].length = gApoolnvImage.element[i].length;
        apool.element[i].sda = gApoolnvImage.element[i].sda;
        apool.element[i].head = gApoolnvImage.element[i].head;
        apool.element[i].tail = gApoolnvImage.element[i].tail;
        apool.element[i].jump_offset = gApoolnvImage.element[i].jump_offset;
    }
}

/**
******************************************************************************
**
**  @brief      This function is called when the apool ownership is not of this
**              controller. This will set the apool status to reject puts and
**              sets the apool to inactive and logs an event to CCB.
**
**  @param      apool id
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_DisownApool(UINT16 apool_id)
{
    fprintf(stderr,"%s: Disowning enter\n",__func__);

    /* Block any potential IO */
    ap_reject_puts |= (1 << MR_REJECT_PUTS_DIS);

    /* If there is an ownership task working then wait till it's done */
    gAsyncApoolOwner = FALSE;
    while(gApoolOwnshipPCB)
    {
        if(gAsyncApoolOwner == TRUE)
        {
            // Ownership has been reaquired
            return;
        }
        TaskSleepMS(1);
    }

    /* Clear the appropriate flags */
    apool_disown(apool_id);

    gApoolActive = FALSE;
    logAPOOLevent(AP_ASYNC_BUFFER_OWNER_UNSET, LOG_AS_INFO, 0, apool_id,0);

    fprintf(stderr,"%s: Disowning exit\n",__func__);
}

/**
******************************************************************************
**
**  @brief      Starts the Apool ownership task if it is not running.
**
**  @param      apool id, vid
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_StartOwnershipTask(UINT16 apool_id, UINT16 vid)
{
    if(gApoolOwnshipPCB == NULL)
    {
        /*
        ** First indicate that we need to reject incoming requests to the apool
        ** even though the structure isn't initialized yet.
        */
        ap_reject_puts |= (1 << MR_REJECT_PUTS_OWN);
        CT_fork_tmp = (unsigned long)"AR_ApoolOwnershipTask";
        gApoolOwnshipPCB = (PCB *)-1;       // Flag task being created.
        gApoolOwnshipPCB = TaskCreate4(C_label_referenced_in_i960asm(AR_ApoolOwnershipTask),
                            APOOL_OWNERSHIP_PRIORITY,
                            apool_id,
                            vid
                            );
    }
}

/**
******************************************************************************
**
**  @brief      This task takes ownership of apool and initializes apool if
**              everything is ok.
**
**  @param      apool id , vid
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_ApoolOwnershipTask(UINT32 dummy1 UNUSED, UINT32 dummy2 UNUSED,UINT32 apool_id, UINT32 vid)
{
    UINT32  wait_count = 0;

    fprintf(stderr,"%s: AR_ApoolOwnershipTask Started\n",__func__);

    // Get the async nv data first to cover power cycle of sole survivor case.
    AR_RecoverApoolNVImage(0);

    /*
    ** Wait for Mirror Partner to become active.
    ** This is 132 loops, times 5 seconds = 660 seconds (11 minutes).
    */
    while (wait_count < WAIT_FOR_MP_TIMES)
    {
        TaskSleepMS(5000);   // Wait five secs at a time for up to WAIT_FOR_MP_TIMES (132)
        wait_count++;

        // If we are a one-way system, or mirror partner isn't ourself, then we can continue.
        if((gTDX.count/TARGETS_PER_CTRL) != 1 && K_ficb->cSerial == K_ficb->mirrorPartner)
        {
            fprintf(stderr,"%s: Waiting for MP to establish wait count = %d of %d\n",__func__, wait_count, WAIT_FOR_MP_TIMES);
        }
        else
        {
            break;
        }

        /*
        **  Make sure this controller is still the owner, if not then exit.
        */
        if(!gAsyncApoolOwner)
        {
            fprintf(stderr,"%s: No longer owner of apool\n",__func__);
            gApoolOwnshipPCB = NULL;
            return;
        }
    }

    if(K_ficb->cSerial != K_ficb->mirrorPartner)
    {
        /*
        ** Communication has been established so always send implicit update
        */
        fprintf(stderr,"%s: MP is established\n",__func__);
        AR_NVUpdate(AR_IMPLICIT);

        /*
        ** Give the MP a few seconds to respond
        */
        TaskSleepMS(5000);
    }

    /*
    **  Make sure this controller is still the owner, if not then exit.
    */
    if(!gAsyncApoolOwner)
    {
        fprintf(stderr,"%s: No longer owner of apool\n",__func__);
        gApoolOwnshipPCB = NULL;
        return;
    }

    /*
    ** Start the NVRAM update Task
    */
    if (gAsyncNVque.pcb == NULL)
    {
        CT_fork_tmp = (unsigned long)"AR_NVUpdateTask";
        gAsyncNVque.pcb = (PCB *)-1;        // Flag task being created.
        gAsyncNVque.pcb = TaskCreate2(C_label_referenced_in_i960asm(AR_NVUpdateTask),
            ASYNC_NVUPDATE_PRIORITY);
        fprintf(stderr,"%s:ASYNC NVRAM update Task started\n",__func__);
    }

    /*
    ** Initialize the apool and mover task
    */
    apool_init(apool_id, vid);
    logAPOOLevent(AP_ASYNC_BUFFER_OWNER_SET, LOG_AS_INFO, 0, apool_id, 0);
    gApoolActive = TRUE;
    gApoolOwnshipPCB = NULL;
    fprintf(stderr,"%s: AR_ApoolOwnershipTask Ended\n",__func__);
    PRINT_APOOL_STRUCTURE;
}
/**
******************************************************************************
**
**  @brief      Clear the all async nvram records on the MM card.
**
**  @param      none
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_ClearAsyncNVRAM(void)
{
    INT32 ret_val = 0;

    /*
    **  Clear the global RAM image, do not clear the NVRAM ID on this part
    */
    memset ((void*)&gApoolnvImage, 0, sizeof(APOOL_NV_IMAGE));

    /*
    ** Write to the MM CARD
    */
    ret_val = MM_Write((UINT32)NV_P7BE_START, (void*)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

    if (ret_val != 0)
    {
        fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__,__LINE__);
    }

    /*
    ** Update mirror partner
    */
    if(K_ficb->cSerial == K_ficb->mirrorPartner)
    {
        fprintf(stderr, "%s: MP is not established\n",__func__);
    }
    AR_SendAsyncNVToMirrorPartner(AR_UPDATE_ALL);
    gAsyncNVRecovered = FALSE;
    gAsyncApoolOwner = FALSE;
    gApoolActive = FALSE;
    logAPOOLevent(AP_ASYNC_NVRAM_INITIALIZED, LOG_AS_INFO, 0, 0, 0);
}

/**
******************************************************************************
**
**  @brief      Update all the fields of apool to the global RAM image of async
**              nvram.
**
**  @param      none
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_NVUpdateAllfields(void)
{
    int i;

    memcpy(&gApoolnvImage.id, p7id, sizeof(gApoolnvImage.id));
    gApoolnvImage.header.sequence_count = apool.sequence_count;
    gApoolnvImage.header.status         = apool.status;
    gApoolnvImage.header.apool_id       = apool.id;
    gApoolnvImage.header.cur_head_element = apool.cur_head_element;
    gApoolnvImage.header.cur_tail_element = apool.cur_tail_element;
    gApoolnvImage.header.apool_size =  apool.length;
    gApoolnvImage.header.last_seq_count = apool.last_seq_count;
    gApoolnvImage.header.alink_count   = apool.alink_count;
    gApoolnvImage.header.element_count  = apool.element_count;
    gApoolnvImage.header.element_entry_size = sizeof(APOOL_NV_ELEMENT);
    gApoolnvImage.header.version = apool.version;
    gApoolnvImage.header.time_threshold = apool.time_threshold;
    gApoolnvImage.header.mb_threshold   = apool.mb_threshold;

    for(i = 0;i<MAX_ELEMENTS;i++)
    {
        gApoolnvImage.element[i].apool_id = apool.element[i].apool_id;
        gApoolnvImage.element[i].length  = apool.element[i].length;
        gApoolnvImage.element[i].status  = apool.element[i].status;
        gApoolnvImage.element[i].vid     = apool.element[i].vid;
        gApoolnvImage.element[i].jump_to_element = apool.element[i].jump_to_element;
        gApoolnvImage.element[i].sda     = apool.element[i].sda;
        gApoolnvImage.element[i].head    = apool.element[i].head;
        gApoolnvImage.element[i].tail    = apool.element[i].tail;
        gApoolnvImage.element[i].jump_offset = apool.element[i].jump_offset;
    }
}

/**
******************************************************************************
**
**  @brief      Update Head related fields of apool to the global RAM image of async
**              nvram.
**
**  @param      none
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_NVUpdateHeadFields(void)
{
    int i;

    memcpy(&gApoolnvImage.id, p7id, sizeof(gApoolnvImage.id));
    gApoolnvImage.header.sequence_count = apool.sequence_count;
    gApoolnvImage.header.status         = apool.status;
    gApoolnvImage.header.apool_id       = apool.id;
    gApoolnvImage.header.cur_head_element = apool.cur_head_element;
 //   gApoolnvImage.header.cur_tail_element = apool.cur_tail_element;
    gApoolnvImage.header.apool_size =  apool.length;
 //   gApoolnvImage.header.last_seq_count = apool.last_seq_count;
    gApoolnvImage.header.alink_count   = apool.alink_count;
    gApoolnvImage.header.element_count  = apool.element_count;
    gApoolnvImage.header.element_entry_size = sizeof(APOOL_NV_ELEMENT);
    gApoolnvImage.header.version = apool.version;
    gApoolnvImage.header.time_threshold = apool.time_threshold;
    gApoolnvImage.header.mb_threshold   = apool.mb_threshold;

    for(i = 0;i<MAX_ELEMENTS;i++)
    {
        gApoolnvImage.element[i].apool_id = apool.element[i].apool_id;
        gApoolnvImage.element[i].length  = apool.element[i].length;
        gApoolnvImage.element[i].status  = apool.element[i].status;
        gApoolnvImage.element[i].vid     = apool.element[i].vid;
        gApoolnvImage.element[i].jump_to_element = apool.element[i].jump_to_element;
        gApoolnvImage.element[i].sda     = apool.element[i].sda;
        gApoolnvImage.element[i].head    = apool.element[i].head;
   //     gApoolnvImage.element[i].tail    = apool.element[i].tail;
        gApoolnvImage.element[i].jump_offset = apool.element[i].jump_offset;
    }
}

/**
******************************************************************************
**
**  @brief      Update Tail related fields of apool to the global RAM image of async
**              nvram.
**
**  @param      none
**
**  @return     none
**
**  @attention
**
******************************************************************************
**/
void AR_NVUpdateTailFields(void)
{
    int i;

    memcpy(&gApoolnvImage.id, p7id, sizeof(gApoolnvImage.id));
  //  gApoolnvImage.header.sequence_count = apool.sequence_count;
    gApoolnvImage.header.status         = apool.status;
    gApoolnvImage.header.apool_id       = apool.id;
  //  gApoolnvImage.header.cur_head_element = apool.cur_head_element;
    gApoolnvImage.header.cur_tail_element = apool.cur_tail_element;
    gApoolnvImage.header.apool_size =  apool.length;
    gApoolnvImage.header.last_seq_count = apool.last_seq_count;
    gApoolnvImage.header.alink_count   = apool.alink_count;
    gApoolnvImage.header.element_count  = apool.element_count;
    gApoolnvImage.header.element_entry_size = sizeof(APOOL_NV_ELEMENT);
    gApoolnvImage.header.version = apool.version;
    gApoolnvImage.header.time_threshold = apool.time_threshold;
    gApoolnvImage.header.mb_threshold   = apool.mb_threshold;

    for(i = 0;i<MAX_ELEMENTS;i++)
    {
        gApoolnvImage.element[i].apool_id = apool.element[i].apool_id;
        gApoolnvImage.element[i].length  = apool.element[i].length;
        gApoolnvImage.element[i].status  = apool.element[i].status;
        gApoolnvImage.element[i].vid     = apool.element[i].vid;
        gApoolnvImage.element[i].jump_to_element = apool.element[i].jump_to_element;
        gApoolnvImage.element[i].sda     = apool.element[i].sda;
  //      gApoolnvImage.element[i].head    = apool.element[i].head;
        gApoolnvImage.element[i].tail    = apool.element[i].tail;
  //      gApoolnvImage.element[i].jump_offset = apool.element[i].jump_offset;
    }
}

/**
******************************************************************************
**
**  @brief      This Task sends the async NV updates to the mirror partner
**              controller. This task is created to streamline the nvram
**              updates and not to have any task wait till the update packet
**              is complete in apool_put and apool_get paths.
**
**  @param      none
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
NORETURN
void AR_NVUpdateTask (void)
{
    ILT* pILT = NULL;

    while (FOREVER)
    {
        pILT = QU_DequeReqILT(&gAsyncNVque);

        if (pILT == NULL)
        {
            /*
            ** Queue is empty--set the process to not-ready state
            */
            QU_MakeExecProcessInactive(&gAsyncNVque);
            // fprintf(stderr,"%s: Update Task is inactive\n",__func__); // remove this print after debug
        }
        else
        {
            AR_SendAsyncNVToMirrorPartner((UINT16)(pILT->ilt_normal.w0));
            // fprintf(stderr,"%s: Dequeued a NV update request, req cnt: %d\n",__func__,gAsyncNVque.qcnt); //remove this print after debug
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
        }
        TaskSwitch();
    }
}

/**
******************************************************************************
**
**  @brief      This function queues a request to Async NVRAM task
**
**  @param      update type
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void AR_QueueAsyncNVUpdate (UINT16 updateType)
{
    ILT* pILT;

    /*
    ** Queue the request only if the task is already created, otherwise just
    ** call the update function.
    */
    if(gAsyncNVque.pcb == NULL)
    {
        AR_SendAsyncNVToMirrorPartner(updateType);
    }
    else
    {
        /* Before enqueueing, check whether there is any pending request on the queue,
           if it is there, just use the same ILT to the queue and ignore the previous request */
        pILT = QU_DequeReqILT(&gAsyncNVque);

        if(pILT == NULL)
        {
            pILT = get_ilt();
 #ifdef M4_DEBUG_ILT
 CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
 #endif /* M4_DEBUG_ILT */
        }
        pILT->ilt_normal.w0 = updateType;
        QU_EnqueReqILT_2(pILT, &gAsyncNVque);
        // fprintf(stderr,"%s: Enqueued a NV update request, req cnt: %d\n",__func__, gAsyncNVque.qcnt);
    }
}

/**
******************************************************************************
**
**  @brief      This function sends Async NV packet to mirror partner controller
**
**  @param      update type
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
UINT32 AR_SendAsyncNVToMirrorPartner(UINT16 update_type)
{
    UINT32 status = ERROR;

    /*
    ** If the mirror partner is not established, don't send the packet, return error
    */
    if(K_ficb->cSerial != K_ficb->mirrorPartner)
    {
        status = DLM_send_async_nva(update_type);
    }
#if APOOL_NV_DEBUG
    else
    {
        fprintf(stderr, "%s: MP is not established\n", __func__);
    }
#endif
    return(status);
}

/**
******************************************************************************
**
**  @brief      This function writes the apool data to RAM NV image, Micro-
**              memory card, and sends to the mirror partner controller based
**              on the update type.
**
**  @param      update type
**
**  @return     status
**
**  @attention  none
**
******************************************************************************
**/
UINT32 AR_NVUpdate (UINT16 update_type)
{
    UINT32 i=0;
    UINT32 status = ERROR;
    INT32  ret_val = 0;

    switch (update_type)
    {
        case AR_UPDATE_ALL:
            AR_NVUpdateAllfields();
            ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

            if (ret_val != 0)
            {
                fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
            }
            AR_QueueAsyncNVUpdate(update_type);
            break;

        case AR_APOOL_EXPAND:
            fprintf(stderr, "%s: Send Apool expand Update \n",__func__);
            gApoolnvImage.header.apool_size =  apool.length;
            gApoolnvImage.header.element_count  = apool.element_count;

            /*
            ** Update only fields related to the new element
            */
            i = apool.element_count - 1;
            gApoolnvImage.element[i].apool_id = apool.element[i].apool_id;
            gApoolnvImage.element[i].length  = apool.element[i].length;
            gApoolnvImage.element[i].status  = apool.element[i].status;
            gApoolnvImage.element[i].vid     = apool.element[i].vid;
            gApoolnvImage.element[i].jump_to_element = apool.element[i].jump_to_element;
            gApoolnvImage.element[i].sda     = apool.element[i].sda;
            gApoolnvImage.element[i].head    = apool.element[i].head;
            gApoolnvImage.element[i].tail    = apool.element[i].tail;
            gApoolnvImage.element[i].jump_offset = apool.element[i].jump_offset;

            ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));
            if (ret_val != 0)
            {
                fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
            }

            AR_SendAsyncNVToMirrorPartner(update_type); // Don't queue it, it will mix up with general updates
            break;

        case AR_IMPLICIT:
            fprintf(stderr, "%s: Send Implicit Update  apool head = 0X%llx apool tail= 0X%llx\n",
                       __func__, gApoolnvImage.element[0].head, gApoolnvImage.element[0].tail);
            AR_SendAsyncNVToMirrorPartner(update_type);
            break;

        case AR_IMPLICIT_RSP:
            fprintf(stderr, "%s: Sending response to Implicit update\n",__func__);
            //update_type = AR_UPDATE_ALL; /* Update type is all but don't need to get the data from apool struct*/
            AR_SendAsyncNVToMirrorPartner(update_type);
            break;

        case AR_ALINK_INIT:
        case AR_ALINK_DELETE:
            gApoolnvImage.header.alink_count = apool.alink_count;
            ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

            if (ret_val != 0)
            {
                fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
            }
            break;

        case AR_UPDATE_HEAD:
            AR_NVUpdateHeadFields();
            ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

            if (ret_val != 0)
            {
                fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
            }

            AR_QueueAsyncNVUpdate(update_type);
            break;

        case AR_UPDATE_TAIL:
            AR_NVUpdateTailFields();
            ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

            if (ret_val != 0)
            {
                fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
            }

            AR_QueueAsyncNVUpdate(update_type);
            break;

        default:
            break;
    }

    return(status);
}

/**
******************************************************************************
**
**  @brief      This function gets the nvram update from DLM packet received
**              for this module. Stores the data to the MM card, NV image and
**              apool structure based on the received update type
**
**  @param      Pointer to the received packet and update type
**
**  @return     status
**
**  @attention  none
**
******************************************************************************
**/
UINT32 AR_ReceiveNVUpdate(APOOL_NV_IMAGE* pPkt, UINT16 update_type)
{
    INT32 i=0;
    UINT32 status = TRUE;
    INT32 ret_val = 0;

#ifdef  dave_dbg
    // If I am the owner of the apool and receiving and update print the contents of
    // of the apool before and after the update.
    if(gApoolOwner == K_ficb->cSerial)
    {
        fprintf(stderr,"%s: Received async NV update type %d, apool contents before:\n",
                __func__, update_type);
        PRINT_APOOL_STRUCTURE;
    }
#endif
    switch(update_type)
    {
        case AR_UPDATE_ALL:
        case AR_IMPLICIT_RSP:
        case AR_UPDATE_HEAD:
        case AR_UPDATE_TAIL:
            if(update_type == AR_IMPLICIT_RSP)
            {
                fprintf(stderr,"%s: Received response to Implicit update\n",__func__);
            }
            status = apool_validate_seq_count(pPkt->header.sequence_count,0);
            if(status == TRUE)
            {
                memcpy(&gApoolnvImage, pPkt, (sizeof(APOOL_NV_IMAGE)));
                ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));

                if (ret_val != 0)
                {
                    fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
                    logAPOOLevent(AP_ASYNC_NVRAM_RESTORED_BAD, LOG_AS_ERROR, 4, 4, 0);
                }
                AR_UpdateMemFromNV();
            }
            break;

        case AR_IMPLICIT:
            fprintf(stderr,"%s: Received an Implicit update from MP\n", __func__);
            if(pPkt->header.sequence_count < gApoolnvImage.header.sequence_count)
            {
                AR_NVUpdate(AR_IMPLICIT_RSP);
            }
            break;

        case AR_APOOL_EXPAND:
            fprintf(stderr,"%s: Received the apool expand update\n",__func__);
            /*
            ** Update only fields related to the new element
            */
            i = pPkt->header.element_count - 1;
            if (i < MAX_ELEMENTS)
            {
                apool.length =  gApoolnvImage.header.apool_size =  pPkt->header.apool_size;
                apool.element_count = gApoolnvImage.header.element_count  =  pPkt->header.element_count;

                apool.element[i].apool_id =  gApoolnvImage.element[i].apool_id =  pPkt->element[i].apool_id;
                apool.element[i].length =    gApoolnvImage.element[i].length  = pPkt->element[i].length;
                apool.element[i].status =    gApoolnvImage.element[i].status  = pPkt->element[i].status;
                apool.element[i].vid =       gApoolnvImage.element[i].vid     = pPkt->element[i].vid;
                apool.element[i].jump_to_element = gApoolnvImage.element[i].jump_to_element = pPkt->element[i].jump_to_element;
                apool.element[i].sda = gApoolnvImage.element[i].sda     = pPkt->element[i].sda;
                apool.element[i].head = gApoolnvImage.element[i].head    = pPkt->element[i].head;
                apool.element[i].tail=  gApoolnvImage.element[i].tail    = pPkt->element[i].tail;
                apool.element[i].jump_offset = gApoolnvImage.element[i].jump_offset =pPkt->element[i].jump_offset;

                ret_val = MM_Write((UINT32)NV_P7BE_START, (void *)&gApoolnvImage, sizeof(APOOL_NV_IMAGE));
                if (ret_val != 0)
                {
                    fprintf(stderr,"%s:%d MM_Write FAILED\n",__func__, __LINE__);
                }
            }
            else
            {
                status = FALSE;
            }
            break;

        default:
            break;
    }
#ifdef  dave_dbg
    // If I am the owner of the apool and receiving and update print the contents of
    // of the apool after the update.
    if(gApoolOwner == K_ficb->cSerial)
    {
        fprintf(stderr,"%s: Received async NV update type %d, apool contents after:\n",__func__, update_type);
        PRINT_APOOL_STRUCTURE;
    }
#endif
    return(status);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
