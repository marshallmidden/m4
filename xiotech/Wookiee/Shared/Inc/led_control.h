/* $Id: led_control.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       led_control.h
**
**  @brief      Top Level Interface for Led Control.
**
**  Top Level Interface for Led Control.
**
**  Copyright (c) 2006-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __LED_CONTROL__
#define __LED_CONTROL__

#include <stdint.h>

/**
**  @ingroup _HW_MODULES_
**  @defgroup _HW_MODULE_LED_ LED Control Module
**
**  @brief  Top Level Interface for Led Control.
**/

/**
**  @ingroup _HW_MODULE_LED_
**  @defgroup _HW_MODULE_LED_SUB_MODULES LED Control Sub-modules
**
**  @brief  Sub-level Linux LED hardware specific drivers.
**/

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_
**  @defgroup _HW_MODULE_LED_CONSTANTS LED Control Constants
**
**  @brief  Constants used in the LED Control Module.
**/

/**
**  @ingroup _HW_MODULE_LED_CONSTANTS
**  @defgroup _HW_MODULE_LED_CONSTANTS_STATE LED State Constants
**
**  @brief      Default States.  Sub-Modules must at a miniumum support
**              LED_STATE_OFF and LED_STATE_ON.
**
**              If LED_STATE_ON_OK, LED_STATE_ON_WARN, or LED_STATE_ON_ERR
**              is not set, the LED is off, regardless of a blink bit.
**
**              These are the current state bits that can be passed through.
**              There will have to be correlation between the user and
**              sub-module driver as to the action of the hardware.
**
**  @{
**/
#define LED_STATE_OFF           0x00000000  /**< LED State Off          */
#define LED_STATE_ON_OK         0x00000001  /**< Bit - LED State On OK        */
#define LED_STATE_ON_WARN       0x00000002  /**< Bit - LED State On Warning   */
#define LED_STATE_ON_ERR        0x00000004  /**< Bit - LED State On Error     */
#define LED_STATE_BLINK_SLOW    0x00000100  /**< Bit - LED State Blink Slowly */
#define LED_STATE_BLINK_FAST    0x00000200  /**< Bit - LED State Blink Fast   */
/* @} */

/**
**  @ingroup _HW_MODULE_LED_CONSTANTS
**  @defgroup _HW_MODULE_LED_CONSTANTS_STATUS LED Driver Status.
**
**  @brief      Constants for driver status.
**
**  @{
**/
#define LED_DRIVER_UNINITIALIZED    0  /**< LED Driver Uninitialized            */
#define LED_DRIVER_INITIALIZED      1  /**< LED Driver Initialized              */
#define LED_DRIVER_NOT_PRESENT      2  /**< LED Driver HW Device Not Present    */
#define LED_DRIVER_MISBEHAVING      3  /**< LED Driver Too many errors          */
/* @} */

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_
**  @defgroup _HW_MODULE_LED_MACROS LED Control Macros
**
**  @brief      Macros used with the LED Control Module.
**/

/**
**  @ingroup _HW_MODULE_LED_MACROS
**  @defgroup _HW_MODULE_LED_MACROS_MISC LED State Macros
**
**  @brief      Miscellaneous Macros
**
**  @{
**/
/** Mask for on state                          */
#define LED_STATE_IS_ON    (LED_STATE_ON_OK |    \
                             LED_STATE_ON_WARN | \
                             LED_STATE_ON_ERR)
/** Mask for blink state                          */
#define LED_BLINK_IS_ON    (LED_STATE_BLINK_SLOW | \
                            LED_STATE_BLINK_FAST)
/** Macro that returns whether an on bit is set in a state. */
#define LED_ON_MASK(a)  ((a) & LED_STATE_IS_ON)

/** Macro that returns whether a blink bit is set in a state. */
#define LED_BLINK_MASK(a) ((a) & LED_BLINK_IS_ON)
/* @} */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_
**  @defgroup _HW_MODULE_LED_STRUCT LED Control Structures
**
**  @brief      Structures used in the LED Control Module.
**/

/**
**  @ingroup _HW_MODULE_LED_STRUCT
**  @defgroup _HW_MODULE_LED_LOCK_STRUCT Sub-Module Structure
**
**  @brief      This structure will be used by led_control to initialize
**              and controle Sub-Module led device drivers.
**
**              If something in Logic or HW changes, simply add a new
**              function here, and to the driver affected.
**
**  @{
**/
/** Sub-module state functions  */
typedef struct _lcsm_state_function
{
    uint32_t    state_mask;                         /**< Mask of supported states   */
    uint32_t    reserved;                           /**< led init function          */
    int32_t     (*state_func)(uint32_t);            /**< State function             */
}lcsm_state_function;

/** Sub-module driver structure  */
typedef struct _led_control_submodule_driver
{
    const char         *hw_name;                /**< Name of led device     */
    uint32_t            hw_status;              /**< Status of driver       */
    uint32_t            reserved;               /**< Reserved               */
    int32_t             (*led_init_func)(void); /**< led init function      */
    lcsm_state_function attention;              /**< Attention function     */
    lcsm_state_function status;                 /**< Status function        */
    lcsm_state_function session;                /**< Session function       */
    lcsm_state_function beacon;                 /**< Beacon function        */
    lcsm_state_function beacon_other;           /**< Beacon other function  */
}led_control_submodule_driver;
/* @} */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_
**  @defgroup _HW_MODULE_LED_FUNCTIONS LED Control Functions
**
**  @brief      This is a complete list of function within this library.
**              Public functions are available for use in the Interface.
**              Private functions are internally used withing the library.
**/

/**
**  @ingroup _HW_MODULE_LED_FUNCTIONS
**  @defgroup _HW_MODULE_LED_PUBLIC_FUNCTIONS LED Control Public Functions
**
**  @brief      These are the public functions available for this interface.
**
**  @{
**/
extern int32_t lc_init(void);
extern void lc_test(void);
extern void lc_led_all(uint32_t state);

/*
** Any other additional functions need to be added to
** led_funcs[] in led_control.c
*/
extern void lc_led_attention(uint32_t state);
extern void lc_led_status(uint32_t state);
extern void lc_led_session(uint32_t state);
extern void lc_led_beacon(uint32_t state);
extern void lc_led_beacon_other(uint32_t state);
/* @} */

#endif /* ___HW_MODULE_LED_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
