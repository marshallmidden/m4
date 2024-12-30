/* $URL: file:///media/m4/svn-repo/eng/trunk/storage/Wookiee/Shared/Src/led_pca9551.c $ */
/**
******************************************************************************
**
**  @file       led_pca9551.c
**
**  @brief      Interface to pca9551 LED Linux driver.
**
**  Interface to pca9551 LED Linux driver..
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


/**
**  @ingroup _HW_MODULE_LED_SUB_MODULES
**  @defgroup _HW_MODULE_LED_SUB_PCA9551 PCA9551 LED Driver Interface
**
**  @brief  Interface to pca9551 LED Linux Driver.
**/

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
const char pca9551_name_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/name";
const char pca9551_input0_str[]         = "/sys/bus/i2c/drivers/pca9551/0-0060/input0";
const char pca9551_blink_cycle0_str[]   = "/sys/bus/i2c/drivers/pca9551/0-0060/blink_cycle0";
const char pca9551_blink_cycle1_str[]   = "/sys/bus/i2c/drivers/pca9551/0-0060/blink_cycle1";
const char pca9551_blink_period0_str[]  = "/sys/bus/i2c/drivers/pca9551/0-0060/blink_period0";
const char pca9551_blink_period1_str[]  = "/sys/bus/i2c/drivers/pca9551/0-0060/blink_period1";
const char pca9551_led0_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led0";
const char pca9551_led1_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led1";
const char pca9551_led2_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led2";
const char pca9551_led3_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led3";
const char pca9551_led4_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led4";
const char pca9551_led5_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led5";
const char pca9551_led6_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led6";
const char pca9551_led7_str[]           = "/sys/bus/i2c/drivers/pca9551/0-0060/led7";

/**
**  @ingroup _HW_MODULE_LED_SUB_PCA9551
**  @defgroup _HW_MODULE_LED_SUB_PCA9551_CONSTANTS PCA9551 Private Constants.
**
**  @brief      These are the private constants internal for interface.
**
**  @{
**/
#define PCA9551_SLOW_PERIOD             0x80    /**< setting for slow period */
#define PCA9551_SLOW_CYCLE              0xA4    /**< setting for slow cycle  */
#define PCA9551_FAST_PERIOD             0x10    /**< setting for fast period */
#define PCA9551_FAST_CYCLE              0x80    /**< setting for fast cycle  */

#define PCA9551_LED_ON                  0x00    /**< 2 bit LED ON               */
#define PCA9551_LED_OFF                 0x01    /**< 2 bit LED OFF              */
#define PCA9551_LED_SLOW_BLINK          0x02    /**< 2 bit LED ON, Slow Blink   */
#define PCA9551_LED_FAST_BLINK          0x03    /**< 2 bit LED ON, Fast Blink   */
/* @} */

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_SUB_PCA9551
**  @defgroup _HW_MODULE_LED_SUB_PCA9551_STRUCTURES PCA9551 Private Structures.
**
**  @brief      These are the private structures internal for interface.
**
**  @{
**/
/** LED Register see 2 bit LED settings \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
typedef struct _pca9551_led_register
{
    uint8_t led0                                : 2;    /**< led0 2 bit led state */
    uint8_t led1                                : 2;    /**< led1 2 bit led state */
    uint8_t led2                                : 2;    /**< led2 2 bit led state */
    uint8_t led3                                : 2;    /**< led3 2 bit led state */
} pca9551_led_register;

/** Input Register                */
typedef struct _pca9551_input_register
{
    uint8_t in0                                 : 1;    /**< bit 0 input state */
    uint8_t in1                                 : 1;    /**< bit 1 input state */
    uint8_t in2                                 : 1;    /**< bit 2 input state */
    uint8_t in3                                 : 1;    /**< bit 3 input state */
    uint8_t in4                                 : 1;    /**< bit 4 input state */
    uint8_t in5                                 : 1;    /**< bit 5 input state */
    uint8_t in6                                 : 1;    /**< bit 6 input state */
    uint8_t in7                                 : 1;    /**< bit 7 input state */
} pca9551_input_register;

/** Hardware Structure              */
typedef struct _pca9551_data
{
    pca9551_input_register  input;              /**< Input register see struct _pca9551_input_register */
    uint8_t                 blink_period0;      /**< Blink period 0, see \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
    uint8_t                 blink_cycle0;       /**< Blink cycle 0, see \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
    uint8_t                 blink_period1;      /**< Blink period 1, see \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
    uint8_t                 blink_cycle1;       /**< Blink cycle 1, see \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
    pca9551_led_register    led0_led3;          /**< led's 0 - 3 see struct _pca9551_led_register and
                                                     \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
    pca9551_led_register    led4_led7;          /**< led's 4 - 7 see struct _pca9551_led_register and
                                                     \ref _HW_MODULE_LED_SUB_PCA9551_CONSTANTS */
} pca9551_data;
/* @} */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static pca9551_data led_data = { {0,0,0,0,0,0,0,0},
                                 0,0,0,0,
                                 {0,0,0,0},
                                 {0,0,0,0}};

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

/**
**  @ingroup _HW_MODULE_LED_SUB_PCA9551
**  @defgroup _HW_MODULE_LED_SUB_PCA9551_FUNCTIONS PCA9551 Private Functions
**
**  @brief      These are the private functions internal for interface.
**
**  @{
**/
static int32_t led_pca9551_init( void );
static int32_t led_pca9551_attention( uint32_t state );
static int32_t led_pca9551_status( uint32_t state );
static int32_t led_pca9551_session( uint32_t state );
static int32_t led_pca9551_beacon( uint32_t state );
static int32_t led_pca9551_beacon_other( uint32_t state );

static uint8_t led_pca9551_state_to_led_value( uint32_t state );
/* @} */

/*
******************************************************************************
** *** led_control interface. ***
******************************************************************************
*/
/*
** This structure is the interface to the led_control module.
** It must be externed there under some sort of compile time directive.
**
** NOTE: This structure must come after the function prototypes to work.
*/
#define PCA9551_DFLT_STATUS (LED_STATE_ON_OK|LED_STATE_BLINK_SLOW|LED_STATE_BLINK_FAST)

struct _led_control_submodule_driver pca9551_driver =
{
    .hw_name                = "pca9551",
    .led_init_func          = led_pca9551_init,
    .attention =
    {
        .state_mask = PCA9551_DFLT_STATUS,
        .state_func = led_pca9551_attention,
    },
    .status =
    {
        .state_mask = (PCA9551_DFLT_STATUS|LED_STATE_ON_WARN|LED_STATE_ON_ERR),
        .state_func = led_pca9551_status,
    },
    .session =
    {
        .state_mask = PCA9551_DFLT_STATUS,
        .state_func = led_pca9551_session,
    },
    .beacon =
    {
        .state_mask = PCA9551_DFLT_STATUS,
        .state_func = led_pca9551_beacon,
    },
    .beacon_other =
    {
        .state_mask = PCA9551_DFLT_STATUS,
        .state_func = led_pca9551_beacon_other,
    },
};

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initialize pca9551 device
**
**              This function will initialize the pca9551.
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  Special info or none
**
**  @warning    Warning or none.
**
**  @sa         See also (i.e. "sample_func2").
**
******************************************************************************
**/
int32_t led_pca9551_init( void )
{
    int32_t rc;
    uint8_t tmpreg;
    char    tmpstr[32];

    hw_printf("led_pca9551 initializing\n");

    /*
    ** Read in the registers.
    */
    /*
    ** Validate the name.
    */
    if ( (rc = hw_read_buffer((uint8_t*)tmpstr, 32, pca9551_name_str)) != 0 )
    {
        hw_printf("led_pca9551_init failed reading %s\n", pca9551_name_str);
        pca9551_driver.hw_status = LED_DRIVER_NOT_PRESENT;
    }
    else if ( strncmp("pca9551", tmpstr, 7) != 0 )
    {
        tmpstr[31] = '\0';
        hw_printf("led_pca9551_init name mismatch (pca9551) vs (%s)\n", tmpstr);
        pca9551_driver.hw_status = LED_DRIVER_NOT_PRESENT;
        rc = -1;
    }

    /*
    ** Get the input.
    */
    else if ( (rc = hw_read_uint8_from_str((uint8_t*)&led_data.input, pca9551_input0_str)) != 0 )
    {
        hw_printf("led_pca9551_init failed reading %s\n", pca9551_input0_str);
    }

    /*
    ** Get LED Status.
    */
    else
    {
        /*
        ** LED 0
        */
        if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led0_str)) == 0 )
        {
            led_data.led0_led3.led0 = tmpreg;
        }
        else
        {
            hw_printf("led_pca9551_init failed reading %s\n", pca9551_led0_str);
        }

        /*
        ** LED 1
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led1_str)) == 0 )
            {
                led_data.led0_led3.led1 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led1_str);
            }
        }

        /*
        ** LED 2
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led2_str)) == 0 )
            {
                led_data.led0_led3.led2 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led2_str);
            }
        }

        /*
        ** LED 3
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led3_str)) == 0 )
            {
                led_data.led0_led3.led3 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led3_str);
            }
        }

        /*
        ** LED 4
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led4_str)) == 0 )
            {
                led_data.led4_led7.led0 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led4_str);
            }
        }

        /*
        ** LED 5
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led5_str)) == 0 )
            {
                led_data.led4_led7.led1 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led5_str);
            }
        }

        /*
        ** LED 6
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led6_str)) == 0 )
            {
                led_data.led4_led7.led2 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led6_str);
            }
        }

        /*
        ** LED 7
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_read_uint8_from_str(&tmpreg, pca9551_led7_str)) == 0 )
            {
                led_data.led4_led7.led3 = tmpreg;
            }
            else
            {
                hw_printf("led_pca9551_init failed reading %s\n", pca9551_led7_str);
            }
        }
    }

    /*
    ** Setup Blink rates.
    */
    if ( rc == 0 )
    {
        /*
        ** Slow period.
        */
        if ( (rc = hw_write_uint8_as_str(PCA9551_SLOW_PERIOD, pca9551_blink_period0_str)) == 0 )
        {
            led_data.blink_period0 = PCA9551_SLOW_PERIOD;
        }
        else
        {
            hw_printf("led_pca9551_init failed writing %s\n", pca9551_blink_period0_str);
        }

        /*
        ** Slow cycle.
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_write_uint8_as_str(PCA9551_SLOW_CYCLE, pca9551_blink_cycle0_str)) == 0 )
            {
                led_data.blink_cycle0 = PCA9551_SLOW_CYCLE;
            }
            else
            {
                hw_printf("led_pca9551_init failed writing %s\n", pca9551_blink_cycle0_str);
            }
        }

        /*
        ** Fast period.
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_write_uint8_as_str(PCA9551_FAST_PERIOD, pca9551_blink_period1_str)) == 0 )
            {
                led_data.blink_period1 = PCA9551_FAST_PERIOD;
            }
            else
            {
                hw_printf("led_pca9551_init failed writing %s\n", pca9551_blink_period1_str);
            }
        }

        /*
        ** Fast cycle.
        */
        if ( rc == 0 )
        {
            if ( (rc = hw_write_uint8_as_str(PCA9551_FAST_CYCLE, pca9551_blink_cycle1_str)) == 0 )
            {
                led_data.blink_cycle1 = PCA9551_FAST_CYCLE;
            }
            else
            {
                hw_printf("led_pca9551_init failed writing %s\n", pca9551_blink_cycle1_str);
            }
        }
    }

    /*
    ** Did we fail??
    */
    if ( rc == 0 )
    {
        pca9551_driver.hw_status = LED_DRIVER_INITIALIZED;
    }
    else if ( pca9551_driver.hw_status == LED_DRIVER_UNINITIALIZED)
    {
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the attention led.
**
**              Set the state of the attention led.
**
**  @param      state   - state to convert to led signal value
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static int32_t led_pca9551_attention( uint32_t state )
{
    int32_t rc = 0;
    uint8_t ledval = led_pca9551_state_to_led_value(state);

    if ( (rc = hw_write_uint8_as_str(ledval, pca9551_led0_str)) == 0 )
    {
        led_data.led0_led3.led0 = ledval;
    }
    else
    {
        hw_printf("led_pca9551_attention failed writing %s\n", pca9551_led0_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the status led.
**
**              Set the state of the status led.
**
**  @param      state   - state to convert to led signal value
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static int32_t led_pca9551_status( uint32_t state )
{
    int32_t rc = 0;
    uint8_t ledval = led_pca9551_state_to_led_value(state);
    uint8_t ledg;
    uint8_t ledr;

    ledg = ledr = ledval;

    if ( state & LED_STATE_ON_ERR )
    {
        ledg = PCA9551_LED_OFF;
    }

    if ( (rc = hw_write_uint8_as_str(ledg, pca9551_led4_str)) == 0 )
    {
        led_data.led4_led7.led0 = ledg;
    }
    else
    {
        hw_printf("led_pca9551_status failed writing %s\n", pca9551_led4_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    if ( state & LED_STATE_ON_OK )
    {
        ledr = PCA9551_LED_OFF;
    }

    if ( (rc = hw_write_uint8_as_str(ledr, pca9551_led5_str)) == 0 )
    {
        led_data.led4_led7.led1 = ledr;
    }
    else
    {
        hw_printf("led_pca9551_status failed writing %s\n", pca9551_led5_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the session led.
**
**              Set the state of the session led.
**
**  @param      state   - state to convert to led signal value
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static int32_t led_pca9551_session( uint32_t state )
{
    int32_t rc = 0;
    uint8_t ledval = led_pca9551_state_to_led_value(state);

    if ( (rc = hw_write_uint8_as_str(ledval, pca9551_led2_str)) == 0 )
    {
        led_data.led0_led3.led2 = ledval;
    }
    else
    {
        hw_printf("led_pca9551_session failed writing %s\n", pca9551_led2_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the status led.
**
**              Set the state of the status led.
**
**  @param      state   - state to convert to led signal value
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static int32_t led_pca9551_beacon( uint32_t state )
{
    int32_t rc = 0;
    uint8_t ledval = led_pca9551_state_to_led_value(state);

    if ( (rc = hw_write_uint8_as_str(ledval, pca9551_led6_str)) == 0 )
    {
        led_data.led4_led7.led2 = ledval;
    }
    else
    {
        hw_printf("led_pca9551_beacon failed writing %s\n", pca9551_led6_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the status led.
**
**              Set the state of the status led.
**
**  @param      state   - state to convert to led signal value
**
**  @return     0 on success
**  @return     -1 on error.
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static int32_t led_pca9551_beacon_other( uint32_t state )
{
    int32_t rc = 0;
    uint8_t ledval = led_pca9551_state_to_led_value(state);

    if ( (rc = hw_write_uint8_as_str(ledval, pca9551_led1_str)) == 0 )
    {
        led_data.led0_led3.led1 = ledval;
    }
    else
    {
        hw_printf("led_pca9551_beacon failed writing %s\n", pca9551_led1_str);
        pca9551_driver.hw_status = LED_DRIVER_MISBEHAVING;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the state of the attention led.
**
**              This function will initialize the pca9551.
**
**  @param      state   - state to convert to led signal value
**
**  @return     pca9551 led value
**
**  @attention  none
**
**  @warning    none.
**
**  @sa         none.
**
******************************************************************************
**/
static uint8_t led_pca9551_state_to_led_value( uint32_t state )
{
    uint8_t ledval = 0;

    if ( LED_ON_MASK(state) )
    {
        if ( LED_BLINK_MASK(state) )
        {
            if ( state & LED_STATE_BLINK_SLOW )
            {
                ledval = PCA9551_LED_SLOW_BLINK;
            }
            else
            {
                ledval = PCA9551_LED_FAST_BLINK;
            }
        }
        else
        {
            ledval = PCA9551_LED_ON;
        }
    }
    else
    {
        ledval = PCA9551_LED_OFF;
    }

    return ledval;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
