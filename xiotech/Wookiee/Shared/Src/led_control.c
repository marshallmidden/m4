/* $URL: file:///media/m4/svn-repo/eng/trunk/storage/Wookiee/Shared/Src/led_control.c $ */
/**
******************************************************************************
**
**  @file       led_control.c
**
**  @brief      Top Level Interface for Led Control.
**
**  Top Level Interface for Led Control.
**
**  Copyright (c) 2006-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

/***********************************************
** This include must be first in all hw modules.
***********************************************/
#include <stdint.h>
#include "hw_common.h"

/*
** Rest of includes.
*/
#include "led_control.h"

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define LED_NUM_DRIVERS \
    (uint32_t)(sizeof(led_drivers) / sizeof(led_control_submodule_driver*))

#define LED_NUM_FUNCS \
    (uint32_t)(sizeof(led_funcs) / sizeof(void*))
#define LED_NUM_TEST_STATES \
    (uint32_t)(sizeof(led_test_states) / sizeof(uint32_t))

/*
******************************************************************************
** Private variables
******************************************************************************
*/
/*
** Two spots to add new module.  First extern the _led_control_submodule_driver
** and then add it to the led_drivers.
*/
#ifdef ENABLE_LED_PCA9551
    extern struct _led_control_submodule_driver pca9551_driver;
#endif

static led_control_submodule_driver* led_drivers[] =
{
#ifdef ENABLE_LED_PCA9551
    &pca9551_driver,
#endif
};

/*
** Add new functions and states to be covered by the led_test.
*/
static void* led_funcs[] =
{
    lc_led_attention,
    lc_led_status,
    lc_led_session,
    lc_led_beacon,
    lc_led_beacon_other,
};

/*
** Add new states to be covered by the led_test.
*/
uint32_t led_test_states[] =
{
    LED_STATE_ON_OK,
    LED_STATE_ON_WARN,
    LED_STATE_ON_ERR,
    (LED_STATE_ON_OK|LED_STATE_BLINK_SLOW),
    (LED_STATE_ON_WARN|LED_STATE_BLINK_SLOW),
    (LED_STATE_ON_ERR|LED_STATE_BLINK_SLOW),
    (LED_STATE_ON_OK|LED_STATE_BLINK_FAST),
    (LED_STATE_ON_WARN|LED_STATE_BLINK_FAST),
    (LED_STATE_ON_ERR|LED_STATE_BLINK_FAST),

    /* Leave this for last to turn LED's Back off */
    LED_STATE_OFF,
};

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_FUNCTIONS
**  @defgroup _HW_MODULE_LED_PRIVATE_FUNCTIONS LED Control Private Functions
**
**  @brief      These are the private functions available for this interface.
**
**  @{
**/
static int32_t led_change_led_state( uint32_t state,
                                     led_control_submodule_driver* ldriver,
                                     lcsm_state_function* pfunc );
/* @} */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Initialize LED control module and sub-components
**
**              This function will initialize the LED module, and any
**              subcomponents necessary to drive the LED.
**
**  @return     0  on Success.
**  @return     -1 on Failure.
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
int32_t lc_init( void )
{
    int32_t     fail_count  = 0;
    uint32_t    lcount      = 0;
    int32_t     rc = 0;

    hw_printf("led_init - detected %u drivers\n", LED_NUM_DRIVERS);

    /*
    ** Loop through and intialize devices.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        hw_assert(led_drivers[lcount-1]->led_init_func);

        if ( led_drivers[lcount-1]->led_init_func() < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        rc = -1;
        hw_printf("led_init - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
    return(rc);
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of all the LED's.
**
**              Set the state of all thn LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_all( uint32_t state )
{
    uint32_t                fcount      = 0;

    /*
    ** Shut down all devices through all functions.
    */
    for ( fcount = 0; fcount < LED_NUM_FUNCS; ++fcount )
    {
        ((int32_t(*)(uint32_t))led_funcs[fcount])(state);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of the Attention LED's.
**
**              Set the state of the Attention LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_attention( uint32_t state )
{
    int32_t                 fail_count  = 0;
    uint32_t                lcount      = 0;

    /*
    ** Loop through and change to state.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        if ( led_change_led_state(state, led_drivers[lcount-1], &led_drivers[lcount-1]->attention) < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        hw_printf("led_attention - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of the Status LED's.
**
**              Set the state of the Status LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_status( uint32_t state )
{
    int32_t                 fail_count  = 0;
    uint32_t                lcount      = 0;

    /*
    ** Loop through and intialize devices.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        if ( led_change_led_state(state, led_drivers[lcount-1], &led_drivers[lcount-1]->status) < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        hw_printf("led_status - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of the Session LED's.
**
**              Set the state of the Session LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_session( uint32_t state )
{
    int32_t                 fail_count  = 0;
    uint32_t                lcount      = 0;

    /*
    ** Loop through and change to state.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        if ( led_change_led_state(state, led_drivers[lcount-1], &led_drivers[lcount-1]->session) < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        hw_printf("led_session - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of the Beacon LED's.
**
**              Set the state of the Beacon LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_beacon( uint32_t state )
{
    int32_t                 fail_count  = 0;
    uint32_t                lcount      = 0;

    /*
    ** Loop through and change to state.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        if ( led_change_led_state(state, led_drivers[lcount-1], &led_drivers[lcount-1]->beacon) < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        hw_printf("led_beacon - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Set the state of the Other Beacon LED's.
**
**              Set the state of the Other Beacon LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**
**  @return     none
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
void lc_led_beacon_other( uint32_t state )
{
    int32_t                 fail_count  = 0;
    uint32_t                lcount      = 0;

    /*
    ** Loop through and change to state.
    */
    for ( lcount = LED_NUM_DRIVERS; lcount > 0; --lcount )
    {
        if ( led_change_led_state(state, led_drivers[lcount-1], &led_drivers[lcount-1]->beacon_other) < 0 )
        {
            ++fail_count;
        }
    }

    if ( fail_count )
    {
        hw_printf("led_beacon_other - %d drivers of %u FAILED!\n", fail_count, LED_NUM_DRIVERS);
    }
}

/**
******************************************************************************
**  @ingroup _HW_MODULE_LED_
**
**  @brief      Test LED's.
**
**              Test LED's.
**
**  @return     none
**
**  @attention  none
**
**  @warning    This function can take some time!.
**
******************************************************************************
**/
void lc_test( void )
{
    uint32_t                scount;

    /*
    ** Do all states to all functions and all modules.
    */
    for ( scount = 0; scount < LED_NUM_TEST_STATES; ++scount )
    {
        lc_led_all(led_test_states[scount]);
        hw_sleep (5);
    }
}

/**
******************************************************************************
**
**  @brief      Set the state of LED's.
**
**              Set the state of LED's.
**
**  @param      state   - state to send to led drivers. \ref _HW_MODULE_LED_CONSTANTS_STATE
**  @param      ldriver - pointer to driver structure.
**  @param      pfunc   - pointer to function structure.
**
**  @return     0  on Success.
**  @return     -1 on Failure.
**
**  @attention  none
**
**  @warning    none.
**
******************************************************************************
**/
int32_t led_change_led_state( uint32_t state,
                              led_control_submodule_driver* ldriver,
                              lcsm_state_function* pfunc )
{
    int32_t rc = 0;

    if ( ldriver->hw_status == LED_DRIVER_INITIALIZED )
    {
        if ( pfunc->state_func )
        {
            if ( (state & pfunc->state_mask) != state )
            {
                hw_printf("led_change_led_state - driver %s state 0x%08X, "
                   "function %p function mask 0x%08X (state not supported)\n",
                   ldriver->hw_name, state, pfunc->state_func, pfunc->state_mask);
            }
            else if ( pfunc->state_func(state & pfunc->state_mask) < 0 )
            {
                rc = -1;
            }
        }
    }
    else
    {
        rc = -1;
        hw_printf("led_change_led_state - state 0x%08X, function %p "
                "function mask 0x%08X (driver %s not initialized)\n",
                state, pfunc->state_func, pfunc->state_mask, ldriver->hw_name);
    }

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
