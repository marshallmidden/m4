/* $Id: hw_monitor.c 158981 2012-02-22 23:32:25Z m4 $ */
/**
******************************************************************************
**
**  @file       hw_monitor.c
**
**  @brief      Hardware monitoring module.
**
**  Copyright (c) 2006-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

/***********************************************
** This include must be first in all hw modules.
***********************************************/
#include <stdint.h>
#include "hw_common.h"

#include <errno.h>

/* Rest of includes. */

#include "XIO_Macros.h"
#include "hw_mon.h"
#include "HWM.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
//#define HWMON_DEBUG
#define XIO_HW_7000 "SMX7DWE"
#define XIO_HW_3000 "SMX6DH8-XG2"

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/** Internal structure for hw_mon */
typedef struct hw_mon_data
{
    void       *lock;
    uint16_t    running;
    uint8_t     rsvd[6];
    hw_mon_envdevice_internal *root_dev;
    hw_mon_register_data init;
} hw_mon_data;

/*
******************************************************************************
** Public variables not defined in any header file.
******************************************************************************
*/
extern UINT8 gMonitorEnabledFlag;

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/**
** Root hardware device.
**/
static hw_mon_envdevice_internal *linked_root_dev;

static struct hw_mon_envdevice_internal null_root_dev =
{
    .parent_device = NULL,
    .dev =
    {
            .id = 0,
            .type = EDT_CONTROLLER,
            .status = ES_NA,
            .num_msr = 0,
            .num_dev = 0,
            .length = sizeof(struct hw_mon_envdevice_internal),
            .name = "controller",
            .flags = EDF_FRU,
            .dyn_off = HWMON_INT_DV_DYN,
    },
};

extern struct hw_mon_envdevice_internal *root_dev_x7dwe;
extern struct hw_mon_envdevice_internal *root_dev_x6dh8_xg2;

static struct hw_mon_data hwm_data =
{
    .lock = NULL,
    .root_dev = NULL,
    .running = 0,
};

static const char *xio_platform = "UNKNOWN";

static const HWM_PLATFORM hwm_platforms[] =
{
    {
        .name = XIO_HW_7000,.flags = 0,
        .cpu_fans = 0,.case_fans = 4,
        .root_dev = &root_dev_x7dwe,
        .analyze = &analyze_x7dwe,
    },
#if defined(MODEL_3000) || defined(MODEL_4700)
    {
        .name = XIO_HW_3000,
        .flags = PLATFORM_FLAG_DISABLE_BMC_NET | PLATFORM_FLAG_CONFIG_BMC_NET |
        PLATFORM_FLAG_LOCAL_RAID,
        .cpu_fans = 0,.case_fans = 4,
        .root_dev = &root_dev_x6dh8_xg2,
        .analyze = &analyze_x6dh8_xg2,
    },
#endif /* MODEL_3000 || MODEL_4700 */
    {
        .name = NULL,.flags = 0,
        .cpu_fans = -1,.case_fans = -1,
    }
};

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

const HWM_PLATFORM *hwm_platform;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_HWMON_FUNCTIONS
**  @defgroup _HW_MODULE_HWMON_PRIVATE_FUNCTIONS LED Control Private Functions
**
**  @brief      These are the private functions available for this interface.
**
**  @{
**/

static int32_t _hw_hwmon_query_data(const hw_mon_data *hwdev, hw_mon_envdevice_internal *pdev);
static int32_t _hw_hwmon_measure_data(const hw_mon_data *hwdev, hw_mon_envmeasure_internal *pmes);
static int32_t _hw_hwmon_load_hysteresis(int64_t *val, const char *sys,
                                         hw_mon_envmeasure_internal *pmes);
static int32_t _hw_hwmon_read_data(int64_t *val, const char *sys, hw_mon_envmeasure_internal *pmes);
static int32_t _hw_hwmon_write_data(const int64_t *val, const char *sys, hw_mon_envmeasure_internal *pmes);
static int32_t _hw_hwmon_get_device_str(char *instr, const hw_mon_envdevice_internal *pdev);
static char *_hw_hwmon_get_measure_str(char *instr, const hw_mon_envmeasure_internal *pmes);
static void *_hw_hwmon_task(void *data);

/* @} */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**  @ingroup _HW_MODULE_HWMON_
**
**  @brief      Initialize hardware/environmental module/monitor.
**
**              This function will initialize the hardware/environmental
**              module/monitor.  This function wiil initialize the name,
**              flags, and logging functions.  In addition this function will
**              set up the configuration of the hardwre monitored.
**
**              If hw_mon_register_data.conf_file is NULL, attempt to
**              initialize with hw_mon_envdevice_internal*  linked_root_dev;
**
**  @param      init_data   pointer to data with which to initialize hw_mon
**              \ref _HW_MODULE_HWMON_STRUCT_REGISTER_DATA
**
**  @return     0  on success
**  @return     -1 on error
**
******************************************************************************
**/
int32_t hw_hwmon_init(hw_mon_register_data *init_data)
{
    if (hwm_data.running)       /* See if we are already running. */
    {
        return 0;
    }

    if (!init_data)
    {
        return -1;
    }

    /* Copy the init data. */

    memcpy(&hwm_data.init, init_data, sizeof(*init_data));

    {
        char       *p = getenv("XIO_HW_TYPE");

        if (p && p[0])
        {
            xio_platform = p;
            fprintf(stderr, "%s: xio_platform=%s\n", __func__, xio_platform);
        }
    }

    if (xio_platform)
    {
        const HWM_PLATFORM *hwm;

        for (hwm = &hwm_platforms[0]; hwm->name; ++hwm)
        {
            if (strcmp(hwm->name, xio_platform) == 0)
            {
                fprintf(stderr, "%s found platform name=%s\n", __func__, hwm->name);
                break;
            }
        }
        hwm_platform = hwm;
    }

    if (!xio_platform || !hwm_platform || !hwm_platform->name)
    {
        fprintf(stderr, "%s: platform %s not found\n", __func__, xio_platform);
    }

    if (hwm_platform && hwm_platform->root_dev && *hwm_platform->root_dev)
    {
        linked_root_dev = *hwm_platform->root_dev;
    }
    else
    {
        fprintf(stderr, "%s: Using null_root_dev\n", __func__);
        linked_root_dev = &null_root_dev;
    }
    hwm_data.root_dev = linked_root_dev;

    /* Set the root name if we have data. */

    if (init_data->top_level_name && hwm_data.root_dev)
    {
        strncpy(hwm_data.root_dev->dev.name, init_data->top_level_name, ENV_MAX_NAME_STR);
        hwm_data.root_dev->dev.name[ENV_MAX_NAME_STR - 1] = '\0';
    }

    if (hwm_data.lock == NULL)
    {
        hw_lock_init(hwm_data.lock);    /* Init the mutex. */
    }

    /* Create the hwmon task. */

    if (xio_platform && linked_root_dev != &null_root_dev)
    {
        hw_crtask(_hw_hwmon_task, NULL);
        hwm_data.running = 1;
    }

    return 0;
}


/**
******************************************************************************
**
**  @brief      Allocate and return a pointer to the env data.
**
**              Strip out all internal data and return all valid information.
**
**  @param      pdevice pointer to place allocated device data.
**  @param      rdevice device root to gather information, NULL for root device.
**
**  @return     0  on error
**  @return     size allocated on success
**
**  @attention  Caller is responsible for freeing memory.
**
******************************************************************************
**/
uint32_t hw_hwmon_get_clean_data(env_device ** pdevice, void *rdevice)
{
    uint32_t    rc = 0;
    int32_t     mindex;
    size_t      mlength = 0;
    size_t      mnewlength = 0;
    int32_t     dindex;
    uint8_t     num_msr = 0;
    uint8_t     ign_fail = 1;
    hw_mon_envmeasure_internal *tpmes;
    hw_mon_envdevice_internal *tpidev;
    hw_mon_envdevice_internal *tpidev2;
    env_device *tpdev;
    hw_mon_envdevice_internal *root_dev;

    root_dev = (hw_mon_envdevice_internal *)rdevice;

    hw_assert(pdevice);         /* Asserts. */

    if (!root_dev)              /* Set root_dev, if not done. */
    {
        root_dev = hwm_data.root_dev;
    }

    if (!root_dev->dlock)       /* Init Lock if necessary. */
    {
        hw_lock_init(root_dev->dlock);
    }

    hw_lock_lock(root_dev->dlock);      /* Lock Mutex */

    if (!pdevice || !hwm_data.root_dev) /* Check valid pointers. */
    {
        hw_printf("%s: ERROR null pointer: pdevice %p, root_dev %p\n",
                  __func__, pdevice, hwm_data.root_dev);
        rc = 0;
        goto out;
    }

    if (root_dev->dev.flags & EMF_DEBUG_ONLY)   /* Check for debug only flag. */
    {
        rc = 0;
        goto out;
    }

    /* Pointer checked out, carry on. */

    if (!*pdevice)
    {
        *pdevice = (env_device *)hw_malloc(hwm_data.root_dev->dev.length);
    }

    if (!*pdevice)
    {
        rc = 0;
        goto out;
    }

    /* Analyze first. */

    if (root_dev->dev.status != ES_NOACCESS && root_dev->dev.num_msr)
    {
        /* Offset to data. */

        mlength = (size_t)root_dev + root_dev->dev.dyn_off;
        tpmes = (hw_mon_envmeasure_internal *)mlength;

        /* Loop through measures. */

        for (mindex = 0; mindex < root_dev->dev.num_msr; ++mindex)
        {
            uint8_t     add_msr = 1;

            /* Check on whether we need to add measure. */

            if (tpmes->access.flags & (EMF_STATUS_ONLY | EMF_IGN_FFAIL))
            {
                add_msr = 0;
            }

            if (!(tpmes->access.flags & EMF_IGN_FFAIL))
            {
                ign_fail = 0;
            }

            /* Calculate new device status. */

            if (tpmes->msr.status == ES_NA)
            {
                add_msr = 0;
            }

            /* Add measure if necessary. */

            if (add_msr)
            {
                ++num_msr;

                /* Copy env_measure. */

                memcpy((void *)((size_t) * pdevice + sizeof(env_device) + mnewlength),
                       &tpmes->msr, sizeof(env_measure));

                /* Set new dyn_off to immediately after env_measure. */

                ((env_measure *)((size_t) * pdevice + sizeof(env_device) + mnewlength))->dyn_off =
                    sizeof(env_measure);

                /* Set new length to env_measure + data. */

                ((env_measure *)((size_t) * pdevice + sizeof(env_device) + mnewlength))->length =
                    sizeof(env_measure) + sizeof(env_generic_measure_data);

                mnewlength += sizeof(env_measure);      /* Adjust length. */

                /* Copy data. */

                memcpy((void *)((size_t)*pdevice + sizeof(env_device) + mnewlength),
                       &tpmes->data, sizeof(env_generic_measure_data));

                /* Adjust length. */

                mnewlength += sizeof(env_generic_measure_data);
            }

            /* Reset measure length. */

            mlength += tpmes->msr.length;
            tpmes = (hw_mon_envmeasure_internal *)mlength;
        }
    }

    /* If we can process the device, carry on. */

    if (root_dev->dev.status != ES_NA || !ign_fail || mnewlength)
    {
        /* Copy the device. */

        memcpy(*pdevice, &root_dev->dev, sizeof(env_device));
        tpdev = *pdevice;
        tpdev->length = sizeof(env_device) + mnewlength;
        tpdev->num_dev = 0;
        tpdev->num_msr = num_msr;
        tpdev->dyn_off = (num_msr) ? sizeof(env_device) : 0;

        /* Loop through devices. */

        if (root_dev->dev.num_dev)
        {
            /* Offset to data. */

            tpidev = (hw_mon_envdevice_internal *)mlength;
            tpidev2 = (hw_mon_envdevice_internal *)((size_t)tpdev + tpdev->length);

            for (dindex = 0; dindex < root_dev->dev.num_dev; ++dindex)
            {
                /* Measure and analyze the data. */

                rc = hw_hwmon_get_clean_data((env_device **) & tpidev2, (void *)tpidev);
                if (rc != 0)
                {
                    tpdev->length += rc;
                    tpdev->dyn_off = sizeof(env_device);
                    ++tpdev->num_dev;
                }

                /* Set up the tmpdev to copy. */

                tpidev2 = (hw_mon_envdevice_internal *)((size_t)tpdev + tpdev->length);

                /* Reset measure length. */

                mlength += tpidev->dev.length;
                tpidev = (hw_mon_envdevice_internal *)mlength;
            }
        }

        rc = tpdev->length;
    }

  out:
    /* Unlock Mutex */

    hw_lock_unlock(root_dev->dlock);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Read the hardware structure, read the values, and evaluate.
**
**              Read pdev, use hwdev logging facilities to log any issues.
**
**  @param      hwdev       hardware structure containing flags and log functions.
**  @param      pdev        device structure to query and monitor
**
**  @return     0  on success
**  @return     -1 on error
**
**  @attention  function is recursive.
**
******************************************************************************
**/
int32_t _hw_hwmon_query_data(const hw_mon_data *hwdev, hw_mon_envdevice_internal *pdev)
{
    int32_t     rc = 0;
    int32_t     mindex = 0;
    size_t      mlength = 0;
    int32_t     dindex = 0;
    uint8_t     logthis;
    hw_mon_envmeasure_internal *tpmes;
    hw_mon_envdevice_internal *tpdev;
    uint16_t    devstat = ES_NA;
    hwenv_ldata log_data = { {'\0'},
                             0, 0,
                             {0, 0, {0}, 0, 0, 0, 0, {0}, 0, 0}};
    char        out_str[128] = { 0 };
    hwilf       logfunc;

    /* Asserts. */

    hw_assert(hwdev);
    hw_assert(pdev);

    if (!pdev->dlock)           /* Init Lock if necessary. */
    {
        hw_lock_init(pdev->dlock);
    }

    hw_lock_lock(pdev->dlock);  /* Lock Mutex */

    if (!hwdev || !pdev)        /* Check valid pointers. */
    {
        fprintf(stderr, "%s: ERROR null pointer: hwdev %p, pdev %p\n",
                __func__, hwdev, pdev);
        rc = -1;
        goto out;
    }

    /* Pointers checked out, carry on. */
    logfunc = hwdev->init.log_info;

    logthis = 1;                /* Default logging to true. */

    /* Game on... */

    /* Loop through measures. */

    if (pdev->dev.num_msr)
    {
        /* Offset to data. */

        mlength = (size_t)pdev + pdev->dev.dyn_off;
        tpmes = (hw_mon_envmeasure_internal *)mlength;

        logthis = 0;            /* Default logging to false. */

        for (mindex = 0; mindex < pdev->dev.num_msr; ++mindex)
        {
            /* Measure and analyze the data. */

            if (_hw_hwmon_measure_data(hwdev, tpmes) != 0)
            {
                hw_printf("%s: ERROR: hwdev %p, pmes %p\n", __func__, hwdev, tpmes);
            }

            /* Calculate new device status. */

            if (tpmes->msr.status > devstat)
            {
                devstat = tpmes->msr.status;
            }

            /* Check on whether we need to log. */

            if (!(tpmes->access.flags & EMF_IGN_FFAIL))
            {
                logthis = 1;
            }

            /* Reset measure length. */

            mlength += tpmes->msr.length;
            tpmes = (hw_mon_envmeasure_internal *)mlength;
        }
    }

#ifdef HWMON_DEBUG
    hw_printf("HWM... device %s\n", pdev->dev.name);
    hw_printf("HWM...     id       %u\n", pdev->dev.id);
    hw_printf("HWM...     type     %u\n", pdev->dev.type);
    hw_printf("HWM...     status   %u\n", pdev->dev.status);
    hw_printf("HWM...     num_msr  %u\n", pdev->dev.num_msr);
    hw_printf("HWM...     num_dev  %u\n", pdev->dev.num_dev);
    hw_printf("HWM...     length   %u\n", pdev->dev.length);
    hw_printf("HWM...     flags    0x%08X\n", pdev->dev.flags);
    hw_printf("HWM...     dyn_off  %u\n", pdev->dev.dyn_off);
#endif  /* HWMON_DEBUG */

    /* Loop through devices. */

    if (pdev->dev.num_dev)
    {
        /* Offset to data. */

        tpdev = (hw_mon_envdevice_internal *)mlength;

        for (dindex = 0; dindex < pdev->dev.num_dev; ++dindex)
        {
            /* Measure and analyze the data. */

            if (_hw_hwmon_query_data(hwdev, tpdev) != 0)
            {
                hw_printf("%s: ERROR recurse: hwdev %p, pdev %p\n",
                          __func__, hwdev, tpdev);
            }

            /* Reset measure length. */

            mlength += tpdev->dev.length;
            tpdev = (hw_mon_envdevice_internal *)mlength;
        }
    }

    /* Check device status change. */

    if (pdev->dev.status == devstat || devstat == ES_NA)
    {
        goto out;
    }

    switch (devstat)
    {
        case ES_GOOD:          /* LOG DEV GOOD. */
            _hw_hwmon_get_device_str(out_str, pdev);
            sprintf(log_data.logstr, "%s GOOD", out_str);
#ifdef HWMON_DEBUG
            hw_printf("HWM... LOG... %s GOOD\n", out_str);
            hw_printf("HWM...   old: %u new: %u\n", pdev->dev.status, devstat);
#endif  /* HWMON_DEBUG */
            break;

        case ES_WARNING:       /* LOG DEV WARN. */
            _hw_hwmon_get_device_str(out_str, pdev);
            sprintf(log_data.logstr, "%s WARNING", out_str);
#ifdef HWMON_DEBUG
            hw_printf("HWM... LOG... %s WARNING\n", out_str);
            hw_printf("HWM...   old: %u new: %u\n", pdev->dev.status, devstat);
#endif  /* HWMON_DEBUG */
            logfunc = hwdev->init.log_warn;
            break;

        case ES_ERROR:         /* LOG DEV ERROR. */
            _hw_hwmon_get_device_str(out_str, pdev);
            sprintf(log_data.logstr, "%s ERROR", out_str);
#ifdef HWMON_DEBUG
            hw_printf("HWM... LOG... %s ERROR\n", out_str);
            hw_printf("HWM...   old: %u new: %u\n", pdev->dev.status, devstat);
#endif  /* HWMON_DEBUG */
            logfunc = hwdev->init.log_error;
            break;

        case ES_NOACCESS:      /* LOG DEV ERROR. */
            _hw_hwmon_get_device_str(out_str, pdev);
            sprintf(log_data.logstr, "%s ACCESS ERROR", out_str);
#ifdef HWMON_DEBUG
            hw_printf("HWM... LOG... %s ACCESS ERROR\n", out_str);
            hw_printf("HWM...   old: %u new: %u\n", pdev->dev.status, devstat);
#endif  /* HWMON_DEBUG */
            logfunc = hwdev->init.log_error;
            break;

        default:               /* LOG DEV UNKNOWN. */
            _hw_hwmon_get_device_str(out_str, pdev);
            sprintf(log_data.logstr, "%s UNKNOWN STATE", out_str);
#ifdef HWMON_DEBUG
            hw_printf("HWM... LOG... %s UNKNOWN\n", out_str);
            hw_printf("HWM...   old: %u new: %u\n", pdev->dev.status, devstat);
#endif  /* HWMON_DEBUG */
            logfunc = hwdev->init.log_error;
            break;
    }

    /* Log it if necessary. */

    if (!logthis || (pdev->dev.flags & EMF_DEBUG_ONLY))
    {
        logfunc = hwdev->init.log_debug;
    }

    /* Do we need to send binary as well */

    /* TODO: Binary */
    if (hwdev->init.flags & HWMON_BIN_LOGS)
    {
        /* Append binary and set binary flag */
        hw_printf("HWM... LOG... Binary Mode unsupported\n");
    }

    logfunc(&log_data);

    pdev->dev.status = devstat; /* Set the new devstat. */

  out:
    /* Unlock Mutex */

    hw_lock_unlock(pdev->dlock);

    return rc;
}


/**
******************************************************************************
**
**  @brief  Compute nominal sensor status from current value
**
**  @param  mes     - Pointer to sensor measure
**  @param  data    - Sensor data
**  @param  dir     - Pointer to error direction, < 0 under, > 0 over
**
**  @return Nominal sensor status
**
******************************************************************************
**/
static int32_t compute_sensor_status(hw_mon_envmeasure_internal *mes,
                                     int64_t data, int32_t *dir)
{
    int32_t     status = ES_GOOD;

    *dir = 0;
    if (data <= mes->data.min_error)
    {
        *dir = -1;
        if (!(mes->access.flags & EMF_IGN_FFAIL))
        {
            status = ES_ERROR;
        }
    }
    else if (data < mes->data.min_warn)
    {
        *dir = -1;
        if (!(mes->access.flags & EMF_IGN_FFAIL))
        {
            status = ES_WARNING;
        }
    }
    else if (data >= mes->data.max_error)
    {
        *dir = 1;
        if (!(mes->access.flags & EMF_IGN_FFAIL))
        {
            status = ES_ERROR;
        }
    }
    else if (data > mes->data.max_warn)
    {
        *dir = 1;
        if (!(mes->access.flags & EMF_IGN_FFAIL))
        {
            status = ES_WARNING;
        }
    }
    else if (mes->msr.status != ES_GOOD)
    {
        mes->access.flags &= ~EMF_IGN_FFAIL;
    }

    return status;
}


/**
******************************************************************************
**
**  @brief      Analyze the measure and log according to hwdev
**
**  @param      hwdev   hardware structure containing flags and log functions.
**  @param      pmes    measure structure to query, monitor, and log.
**
**  @return     0  on success
**  @return     -1 on error
**
******************************************************************************
**/
int32_t _hw_hwmon_measure_data(const hw_mon_data *hwdev, hw_mon_envmeasure_internal *pmes)
{
    int32_t     rc = 0;
    int64_t     tmpstat = 0;
    int64_t     tmpdata = 0;
    char        out_str[256] = { '\0' };
    hwenv_ldata log_data = { {'\0'},
                              0, 0,
                              {0, 0, {0}, 0, 0, 0, 0, {0}, 0, 0}};
    hwilf       logfunc;
    uint8_t     logthis;
    uint8_t     logmsr;
    int32_t     dir;
    uint32_t    newstatus;
    uint32_t    oldstatus;

    /* Asserts. */

    hw_assert(hwdev);
    hw_assert(pmes);

    /* Check valid pointers. */

    if (!hwdev || !pmes)
    {
        hw_printf("%s: ERROR null pointer: hwdev %p, pmes %p\n", __func__, hwdev, pmes);
        return -1;
    }

    /* Pointers checked out, carry on. */

    logfunc = hwdev->init.log_debug;
    logthis = (pmes->access.flags & EMF_IGN_FFAIL) ||
        (pmes->access.parent_device->dev.flags & EMF_DEBUG_ONLY) ? 0 : 1;
    logmsr = 0;

    /* Load the hysteresis. */

    /* Get the status. */

    tmpstat = pmes->access.parent_device->dev.status;

    /* Max error. */

    if (_hw_hwmon_load_hysteresis(&pmes->data.max_error, pmes->access.max_error,
                                  pmes) != 0)
    {
        if (tmpstat != ES_NOACCESS)
        {
            hw_printf("HWM... LOG... %s ERROR no access max error (%d - %s)\n",
                      _hw_hwmon_get_measure_str(out_str, pmes), errno, strerror(errno));
        }
    }

    /* Min error. */

    if (_hw_hwmon_load_hysteresis(&pmes->data.min_error, pmes->access.min_error,
                                  pmes) != 0)
    {
        if (tmpstat != ES_NOACCESS)
        {
            hw_printf("HWM... LOG... %s ERROR no access min error (%d - %s)\n",
                      _hw_hwmon_get_measure_str(out_str, pmes), errno, strerror(errno));
        }
    }

    /* Max warn. */

    if (_hw_hwmon_load_hysteresis(&pmes->data.max_warn, pmes->access.max_warn, pmes) != 0)
    {
        if (tmpstat != ES_NOACCESS)
        {
            hw_printf("HWM... LOG... %s ERROR no access max warn (%d - %s)\n",
                      _hw_hwmon_get_measure_str(out_str, pmes), errno, strerror(errno));
        }
    }

    /* Min warn. */

    if (_hw_hwmon_load_hysteresis(&pmes->data.min_warn, pmes->access.min_warn, pmes) != 0)
    {
        if (tmpstat != ES_NOACCESS)
        {
            hw_printf("HWM... LOG... %s ERROR no access min warn (%d - %s)\n",
                      _hw_hwmon_get_measure_str(out_str, pmes), errno, strerror(errno));
        }
    }

    /* Load the input data. */

    /* Read the input. */

    if (!pmes->access.data)     /* If none, assume data supplied */
    {
        tmpdata = pmes->data.data;
    }
    else
    {
        if (_hw_hwmon_read_data(&tmpdata, pmes->access.data, pmes))
        {
            if (tmpstat != ES_NOACCESS)
            {
                /* Set state to error. */
                pmes->msr.status = ES_NOACCESS;
            }
            goto out;
        }
    }

    /* Analyze the data. */

    oldstatus = pmes->msr.status;
    newstatus = compute_sensor_status(pmes, tmpdata, &dir);

    if (pmes->msr.type == EMT_COUNT)
    {
        if (oldstatus != ES_NA && tmpdata == pmes->data.data)
        {
            if (pmes->access.count < 50)
            {
                ++pmes->access.count;
            }
        }
        else if (oldstatus != newstatus || pmes->access.count++ > 2)
        {
            pmes->data.data = tmpdata;
            pmes->msr.status = newstatus;
            oldstatus = ES_NA;
            pmes->access.count = 0;
        }
    }
    else
    {
        switch (oldstatus)
        {
            case ES_NA:
                if (newstatus != oldstatus)
                {
                    pmes->msr.status = newstatus;
                    pmes->data.data = tmpdata;
                    pmes->access.count = 0;
                }
                break;

            case ES_GOOD:
            case ES_WARNING:
                if (newstatus == oldstatus || pmes->access.count++ > 3)
                {
                    pmes->msr.status = newstatus;
                    pmes->data.data = tmpdata;
                    pmes->access.count = 0;
                }
                break;

            case ES_ERROR:
                if (newstatus == oldstatus || newstatus == ES_GOOD)
                {
                    pmes->msr.status = newstatus;
                    pmes->data.data = tmpdata;
                    pmes->access.count = 0;
                }
                break;

            case ES_NOACCESS:
                if (newstatus != oldstatus)
                {
                    pmes->msr.status = newstatus;
                    pmes->data.data = tmpdata;
                    pmes->access.count = 0;
                }
                break;

            default:
                fprintf(stderr, "%s: Unknown oldstatus, %d\n", __func__, oldstatus);
                pmes->msr.status = newstatus;
                pmes->data.data = tmpdata;
                pmes->access.count = 0;
                break;
        }
    }

    /* Check if status really changed and needs to be logged */

    if (oldstatus != newstatus && pmes->msr.status == newstatus)
    {
        const char *dirstr;
        const char *sevstr;
        static const char *sevstrs[] =
        {
            [ES_NA] = "no access",
            [ES_GOOD] = "GOOD",
            [ES_WARNING] = "WARNING",
            [ES_ERROR] = "ERROR",
            [ES_NOACCESS] = "no access",
        };

        if (dir < 0)
        {
            dirstr = "under ";
        }
        else if (dir > 0)
        {
            dirstr = "over ";
        }
        else
        {
            dirstr = "";
        }

        sevstr = sevstrs[newstatus];
        switch (newstatus)
        {
            case ES_NA:
                fprintf(stderr, "%s: newstatus should not be ES_NA!\n", __func__);
                break;

            case ES_GOOD:
                logmsr = 1;
                logfunc = hwdev->init.log_info;
                break;

            case ES_WARNING:
                logmsr = 1;
                logfunc = hwdev->init.log_warn;
                break;

            case ES_ERROR:
            case ES_NOACCESS:
                logmsr = 1;
                logfunc = hwdev->init.log_error;
                break;

            default:
                fprintf(stderr, "%s: newstatus has bad value %d\n", __func__, newstatus);
                sevstr = "UNKNOWN";
                break;
        }

        if (logmsr)
        {
            sprintf(log_data.logstr, "%s %s%s",
                    _hw_hwmon_get_measure_str(out_str, pmes), dirstr, sevstr);
        }
    }

    /* Log if necessary. */

    if (logmsr)
    {
        if (!logthis)
        {
            logfunc = hwdev->init.log_debug;
        }

        /* Do we need to send binary as well */

        /* TODO: Binary */
        if (hwdev->init.flags & HWMON_BIN_LOGS)
        {
            /* Append binary and set binary flag */
            hw_printf("HWM... LOG... Binary Mode unsupported\n");
        }

        logfunc(&log_data);
    }

  out:

#ifdef HWMON_DEBUG
    hw_printf("HWM...     measure %s\n", pmes->msr.name);
    hw_printf("HWM...        id       %u\n", pmes->msr.id);
    hw_printf("HWM...        type     %u\n", pmes->msr.type);
    hw_printf("HWM...        status   %u\n", pmes->msr.status);
    hw_printf("HWM...        length   %u\n", pmes->msr.length);
    hw_printf("HWM...        dyn_off  %u\n", pmes->msr.dyn_off);
    hw_printf("HWM...      access\n");
    hw_printf("HWM...        data          %s\n", pmes->access.data);
    hw_printf("HWM...        max_error     %s\n", pmes->access.max_error);
    hw_printf("HWM...        max_warn      %s\n", pmes->access.max_warn);
    hw_printf("HWM...        min_error     %s\n", pmes->access.min_error);
    hw_printf("HWM...        min_warn      %s\n", pmes->access.min_warn);
    hw_printf("HWM...        calc_wr_fmt  %s\n", pmes->access.calc_wr_fmt);
    hw_printf("HWM...        calc_rd_fmt   %s\n", pmes->access.calc_rd_fmt);
    hw_printf("HWM...        parent_device %p\n", pmes->access.parent_device);
    hw_printf("HWM...      data\n");
    hw_printf("HWM...        data         " fmt64i "\n", pmes->data.data);
    hw_printf("HWM...        max_error    " fmt64i "\n", pmes->data.max_error);
    hw_printf("HWM...        max_warn     " fmt64i "\n", pmes->data.max_warn);
    hw_printf("HWM...        min_error    " fmt64i "\n", pmes->data.min_error);
    hw_printf("HWM...        min_warn     " fmt64i "\n", pmes->data.min_warn);
#endif  /* HWMON_DEBUG */

    return rc;
}


/**
******************************************************************************
**
**  @brief      Load a hysteresis
**
**  @param      val       value of current hysteresis, and return value.
**  @param      sys       path to sys filesystem file to read/write hysteresis
**  @param      pmes      Pointer to measurement
**
**  @return     0  on success
**  @return     -1 on error
**
******************************************************************************
**/
int32_t _hw_hwmon_load_hysteresis(int64_t *val, const char *sys,
                                  hw_mon_envmeasure_internal *pmes)
{
    int32_t     rc = 0;
    int64_t     tmpdata = 0;

    /* If we have data to read, read it. */

    if (!sys || sys[0] == '\0')
    {
        return 0;
    }

    /* Read the data. */

    if (_hw_hwmon_read_data(&tmpdata, sys, pmes) != 0)
    {
        return -1;
    }

    /* We have a different value than what was read, change it. */

    if (*val != tmpdata && *val != LLONG_MAX && *val != LLONG_MIN)
    {
        _hw_hwmon_write_data(val, sys, pmes);
    }
    /* else if we have defaults and read a value, use it. */
    else if (*val == LLONG_MAX || *val == LLONG_MIN)
    {
        *val = tmpdata;
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Read a value from a device and apply cfmt if applicable.
**
**  @param      val       location to store value read.
**  @param      sys       path to sys filesystem file to read hysteresis
**  @param      pmes      Pointer to measure structure
**
**  @return     0  on success
**  @return     -1 on error
**
******************************************************************************
**/
int32_t _hw_hwmon_read_data(int64_t *val, const char *sys,
                            hw_mon_envmeasure_internal *pmes)
{
    int64_t     tmprd = 0;

    /* Read the data. */

    if (hw_read_int64_from_str(&tmprd, sys) != 0)
    {
        return -1;
    }

    /* Calculate the data if need be. */

    if (pmes->access.calc_rd)
    {
        tmprd = pmes->access.calc_rd(tmprd);
    }

    *val = tmprd;               /* Save the data. */

    return 0;
}


/**
******************************************************************************
**
**  @brief      Apply cfmt if applicable and write a value to a device.
**
**  @param      val       location to write value from
**  @param      sys       path to sys filesystem file to write hysteresis
**  @param      pmes      Pointer to measure structure
**
**  @return     0  on success
**  @return     -1 on error
**
******************************************************************************
**/
int32_t _hw_hwmon_write_data(const int64_t *val, const char *sys,
                             hw_mon_envmeasure_internal *pmes)
{
    int64_t     tmpwrt;

    tmpwrt = *val;              /* Set the variable */

    if (pmes->access.calc_wr)
    {
        tmpwrt = pmes->access.calc_wr(tmpwrt);
    }

    /* Write the data. */

    if (hw_write_int64_as_str(tmpwrt, sys) != 0)
    {
        return -1;
    }

    return 0;
}


/**
******************************************************************************
**
**  @brief      Get the full device string from the root device.
**
**  @param      instr      string to store name in.
**  @param      pdev       device to retrieve full name of.
**
**  @return     string length written
**
******************************************************************************
**/
int32_t _hw_hwmon_get_device_str(char *instr, const hw_mon_envdevice_internal *pdev)
{
    int32_t     num = 0;

    if (pdev == NULL)
    {
        return 0;
    }

    num = _hw_hwmon_get_device_str(instr, pdev->parent_device);

    return sprintf((char *)(instr + num), "%s ", pdev->dev.name) + num;
}


/**
******************************************************************************
**
**  @brief      Get the full measure string from the root device.
**
**  @param      instr      string to store name in.
**  @param      pmes       measure to retrieve full name of.
**
**  @return     pointer to string written
**
******************************************************************************
**/
char       *_hw_hwmon_get_measure_str(char *instr, const hw_mon_envmeasure_internal *pmes)
{
    sprintf((char *)(instr + _hw_hwmon_get_device_str(instr, pmes->access.parent_device)),
            "%s", pmes->msr.name);
    return instr;
}


/*
******************************************************************************
**
**  @brief  Check for valid sensor status
**
**  @param  Pointer to hw_mon_envmeasure_internal structure for sensor
**
**  @return TRUE if sensor is valid
**
******************************************************************************
**/
int sensor_valid(hw_mon_envmeasure_internal *s)
{
    return s->msr.status != ES_NA && s->msr.status != ES_NOACCESS;
}


/*
******************************************************************************
**
**  @brief  Update voltage reading
**
**  @param  r - Pointer to VOLTAGE_INPUT_READING structure to be updated
**
**  @param  m - Pointer to env_generic_measure_data providing the measurement
**
**  @return none
**
******************************************************************************
**/
void update_voltage_reading(VOLTAGE_INPUT_READING *r, env_generic_measure_data *m)
{
    LIMIT_MONITOR_VALUE lim = LIMIT_MONITOR_GOOD;

    if (m->data > r->maximumMillivolts)
    {
        r->maximumMillivolts = m->data;
    }

    if (m->data < r->minimumMillivolts)
    {
        r->minimumMillivolts = m->data;
    }

    r->currentMillivolts = m->data;

    if (m->data > m->max_error || m->data > m->max_warn ||
        m->data < m->min_error || m->data < m->min_warn)
    {
        lim = LIMIT_MONITOR_TRIPPED;
    }

    r->limitMonitorValue = lim;
}


/*
******************************************************************************
**
**  @brief  Update cpu temperature
**
**  @param  ts - Pointer to TEMPERATURE_STATUS structure to be updated
**
**  @param  m - Pointer to env_generic_measure_data providing the measurement
**
**  @return none
**
******************************************************************************
**/
void update_cpu_temperature(TEMPERATURE_STATUS *ts, env_generic_measure_data *m)
{
    TEMPERATURE_CONDITION_VALUE cond = TEMPERATURE_CONDITION_NORMAL;
    int         current_DC = (m->data + 500) / 1000;

    if (current_DC > ts->maximumDegreesCelsius)
    {
        ts->maximumDegreesCelsius = current_DC;
    }

    if (current_DC < ts->minimumDegreesCelsius)
    {
        ts->minimumDegreesCelsius = current_DC;
    }

    ts->currentDegreesCelsius = current_DC;

    if (m->data > m->max_warn)
    {
        cond = TEMPERATURE_CONDITION_HOT;
    }

    if (m->data > m->max_error)
    {
        cond = TEMPERATURE_CONDITION_HOT_CRITICAL;
    }

    if (m->data < m->min_warn)
    {
        cond = TEMPERATURE_CONDITION_COLD;
    }

    if (m->data < m->min_error)
    {
        cond = TEMPERATURE_CONDITION_COLD_CRITICAL;
    }

    ts->conditionValue = cond;
}


/**
******************************************************************************
**
**  @brief      Task that runs and executes the hardware monitor.
**
**  @param      data - unused
**
**  @return     none
**
******************************************************************************
**/
void       *_hw_hwmon_task(UNUSED void *data)
{
    /* Loop forever monitoring data. */

    for (;;)
    {
        /* Sleep HW_HWMON_FREQ seconds and then gather the data. */

        hw_sleep(HW_HWMON_FREQ);

        if (!hwm_data.running)
        {
            break;
        }
        if (!gMonitorEnabledFlag)
        {
            continue;
        }

        hw_lock_lock(hwm_data.lock);    /* Lock Mutex */

        /* Query the data. */

        _hw_hwmon_query_data(&hwm_data, hwm_data.root_dev);

        hw_lock_unlock(hwm_data.lock);  /* Unlock Mutex */

        hwm_platform->analyze();
    }

    hw_printf("%s: EXITING!!!\n", __func__);

    return NULL;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
