/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current ALi author: Steven.Zhang
Last update:        2017.02.25
-->

@file   aui_gpio.h

@brief  General Purpose Input/Output (GPIO) Module

        System usually has simple circuit which uses CPU to control and send
        commands to single or many devices, and those require only one bit in
        order to define the on/off states.\n
        Therefore, the micro-controller chip will provides a general programmable
        interface called <b> General Purpose Input/Output (GPIO) Module </b>.

        GPIO is usually divided into several groups:
        - Control registers, which provides the operational attributes
        - Data registers, which provides the current PIN value

        Each control register corresponds to data register one-by-one.

@note   Different Chip-sets supports different GPIO numbers and attributes, user
        has to check the chip data sheet for more details.

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_GPIO_H_

#define _AUI_GPIO_H_

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C"

{

#endif

/*******************************Global Macro List*****************************/

/**
Macro used to set the initialization status for the Input/Output direction of
GPIO Module
*/
#define AUI_GPIO_NONE (-1)

/*******************************Global Type List******************************/

/**
Enum to specify the <b> GPIO control register value </b> which is used for
setting the related attributes of the data register Input/Output direction
*/

typedef enum aui_gpio_dir {

    /**
    Value to specify the <b> GPIO Input Mode </b> with which the CPU can read
    the current data PIN.
    */
    AUI_GPIO_I_DIR,

    /**
    Value to specify the <b> GPIO Output Mode </b> with which the CPU can write
    the current data PIN.
    */
    AUI_GPIO_O_DIR

} aui_gpio_dir;

/**
Enum to specify the <b> GPIO interrupt type </b>\n
In particular, since an application might want monitoring the logic value change
of the GPIO Pin, this enum is used to select the different logic transition type
as cause of a GPIO interrupt
*/
typedef enum aui_gpio_interrupt_type {

    /**
    Value to specify <b> no interrupt </b>.
    */
    AUI_GPIO_INTERRUPT_DISABLED = 0,

    /**
    Value to specify an interrupt on the <b> 0-->1 transition </b>,
    i.e on the rising edge of the logic value change of the GPIO Pin.
    */
    AUI_GPIO_INTERRUPT_RISING_EDGE,

    /**
    Value to specify an interrupt on a <b> 1-->0 transition </b>,
    i.e. on the falling edge of the logic value change of the GPIO Pin.
    */
    AUI_GPIO_INTERRUPT_FALLING_EDGE,

    /**
    Value to specify an interrupt on <b> both 0-->1 and 1-->0 transition </b>,
    i.e. on both of the rising and falling edge of the logic value change of the GPIO Pin.
    */
    AUI_GPIO_INTERRUPT_EDGE,

    /**
    Value to specify the maximum number of interrupt type specified in this enum
    */
    AUI_GPIO_INTERRUPT_MAX,

} aui_gpio_interrupt_type;

/**
Function pointer to specify the callback function to be invoked when a GPIO
interrupt event occurs as per the enum #aui_gpio_interrupt_type
*/
typedef void (*aui_gpio_callback) (

    int gpio_index,

    aui_gpio_interrupt_type interrupt_type,

    void *pv_user_data

    );

/**
Enum to specify the <b> GPIO data register value </b>
*/
typedef enum aui_gpio_value {

    /**
    Value to specify the @b low value of the GPIO data register
    (it is considered as @b 0)
    */
    AUI_GPIO_VALUE_LOW,

    /**
    Value to specify the @b high value of the GPIO data register
    (it is considered as @b 1)
    */
    AUI_GPIO_VALUE_HIGH

} aui_gpio_value;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> General Purpose Input/Output (GPIO) Module </b>
        to specify the GPIO interrupt attributes </b>\n
        </div> @endhtmlonly

        Struct to specify the <b> GPIO interrupt attributes </b>
*/
typedef struct aui_gpio_interrupt_attr {

    /**
    Member to specify the GPIO interrupt type, as defined in the enum
    #aui_gpio_interrupt_type
    */
    aui_gpio_interrupt_type interrupt_type;

    /**
    Member to specify the GPIO callback function to be invoked when a GPIO
    interrupt event occurs, as per comment for the function pointer
    #aui_gpio_callback
    */
    aui_gpio_callback p_callback;

    /**
    Member to specify the user data as input parameter of the GPIO callback
    function which is the member @b p_callback of this struct
    */
    void *pv_user_data;

    /**
    Member to specify a proper time interval (in @a millisecond (ms) unit) to
    ignore the <b> switch bounces </b> that can occur when pressing a button,
    then avoid (by software) that an application may @a think a callback is
    called more than once. \n\n

    In particular, this member can be set as below:
    - @b 0  = 200 ms (default value)
    - @b >0 = Switch bounces will be ignored for the specified time interval
    - @b <0 = The software debounce feature is disabled
    */
    int debounce_interval;

} aui_gpio_interrupt_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> General Purpose Input/Output (GPIO) Module </b> to
        specify the list of configuration attributes
        </div> @endhtmlonly

        Struct to specify the list of configuration attributes
*/
typedef struct aui_gpio_attr {

    /**
    Member to specify an @a Index for referring to different <b> PINs for GPIO
    Device </b>. The values of this Index are integer from the value <b> zero
    (0) </b>
    */
    int uc_dev_idx;

    /**
    Member to specify the <b> GPIO control register value </b>  as defined in
    the enum #aui_gpio_dir
    */

    aui_gpio_dir io;

    /**
    Member to specify the <b> GPIO data register value </b> as defined in
    the enum #aui_gpio_value. In particular, this member specifies the initial
    state of the GPIO pin when it is going to be opened as an output pin.
    */
    aui_gpio_value value_out;

} aui_gpio_attr;

/*****************************Global Function List*****************************/

/**
@brief          Function used to open, register the GPIO Module and configure
                the desired attributes, then get the related handle.

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the GPIO Module

@param[in]      *p_attr             = Pointer to a struct #aui_gpio_attr, which
                                      collects the list of configuration
                                      attributes for the GPIO Module

@param[out]     *p_handle           = #aui_hdl pointer to the handle of the
                                      GPIO Module just opened

@return         @b AUI_RTN_SUCCESS  = GPIO Module opened successfully then user
                                      can start to configure the GPIO device
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter
                                      (i.e. [in], [out]) are invalid
@return         @b Other_Values     = Opening of the GPIO Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_gpio_open (

    aui_gpio_attr *p_attr,

    aui_hdl *p_handle

    );

/**
@brief          Function used to close the GPIO Module already opened by the
                function #aui_gpio_open then the related handle will be released
                (i.e. the related resources such as memory, device)

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the GPIO Module in pair with its the opening by the function
                #aui_gpio_open

@param[in]      handle              =  the handle of the GPIO Module already opened and to be
                                      closed

@return         @b AUI_RTN_SUCCESS  = GPIO Module closed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in]) is invalid
@return         @b Other_Values     = Closing of the GPIO Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_gpio_close (

    aui_hdl handle

    );

/**
@brief          Function used to set the GPIO data register values

@param[in]      handle              = #aui_hdl handle of the GPIO Module already
                                      opened and to be managed for setting the
                                      data register value
@param[in]      value               = The data register value to be set

@return         @b AUI_RTN_SUCCESS  = Setting of the data register value performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = Either one or both the input parameter
                                      (i.e. [in]) is invalid
@return         @b Other_Values     = Settings of the data register value failed
                                      for some reason
*/
AUI_RTN_CODE aui_gpio_set_value (

    aui_hdl handle,

    aui_gpio_value value

    );

/**
@brief          Function used to get the GPIO data register values

@param[in]      handle              = #aui_hdl handle of the GPIO Module already
                                      opened and to be managed for getting the
                                      data register value

@param[in]      p_value             = The data register value to be gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the data register value
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both the parameters (i.e.
                                      [in], [out]) are invalid
@return         @b Other_Values     = Getting of the data register value failed
                                      for some reason
*/
AUI_RTN_CODE aui_gpio_get_value (

    aui_hdl handle,

    aui_gpio_value *p_value

    );

/**
@brief          Function used to perform the initialization of the GPIO Module
                before its opening by the function #aui_gpio_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the GPIO Module

@param[in]      p_call_back_init   = Callback function used to initialize the
                                     GPIO Module, as per comment for the function
                                     pointer @b p_fun_cb
@param[in]      pv_param           = The input parameter of the callback function
                                     @b p_call_back_init, which is the @a first
                                     parameter [in] of this function

@return         @b AUI_RTN_SUCCESS = GPIO Module initialized successfully
@return         @b AUI_RTN_EINVAL  = Either one or both input parameter (i.e.
                                     [in]) is invalid
@return         @b Other_Values    = Initializing of the GPIO Module failed for
                                     some reasons
*/

AUI_RTN_CODE aui_gpio_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the GPIO Module
                after its closing by the function #aui_gpio_close

@return         @b AUI_RTN_SUCCESS  = GPIO Module de-initialized successfully
@return         @b AUI_RTN_EINVAL   = Either one or both input parameter (i.e.
                                      [in]) is invalid
@return         @b Other_Values     = De-initializing of the GPIO Module failed
                                      for some reasons
*/

AUI_RTN_CODE aui_gpio_deinit (

    void

    );

/**
@brief          Function used to register the callback function to be invoked when
                a GPIO interrupt event occurs

@param[in]      handle              = #aui_hdl handle of the GPIO Module already
                                      opened
@param[in]      p_interrupt_attr    = GPIO interrupt attributes, as defined in the
                                      struct #aui_gpio_interrupt_attr

@return         @b AUI_RTN_SUCCESS  = Registering of the the callback function
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both input parameter (i.e.
                                      [in]) is invalid
@return         @b Other_Values     = Registering of the callback function failed
                                      for some reasons
*/
AUI_RTN_CODE aui_gpio_interrupt_reg (

    aui_hdl handle,

    aui_gpio_interrupt_attr *p_interrupt_attr

    );

/**
@brief          Function used to unregister the callback function to be invoked when
                a GPIO interrupt event occurs

@param[in]      handle              = #aui_hdl handle of the GPIO Module already
                                      opened and for which a callback function
                                      has already been registred by the function
                                      #aui_gpio_interrupt_reg

@return         @b AUI_RTN_SUCCESS  = Unregistering of the callback function
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Unregistering of the callback function failed
                                      for some reasons
*/
AUI_RTN_CODE aui_gpio_interrupt_unreg (

    aui_hdl handle

    );

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


