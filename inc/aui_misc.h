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
Current ALi author: Oscar.Shi
Last update:        2017.03.24

-->

@file   aui_misc.h

@brief  Miscellaneous (MISC) Module

        <b> Miscellaneous (MISC) Module </b> is used to:
        - Perform the power management and access the related information of the
          targeted ALi board
        - Set the targeted ALi board into standby mode and/or reboot it
        - Get system information such as the
          - Chipset ID
          - Chipset Version Number
          - Product ID

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_MISC_H

#define _AUI_MISC_H

/*************************Included Header File List***************************/

#include "aui_common.h"

#include <time.h>

#ifdef __cplusplus

extern "C"

{

#endif

/******************************Global Macro List*******************************/

/**
Macro to specify the member of keys which can be used to wake up the platform in
project based on <b> Linux OS </b>.

In project based on <b> TDS OS </b>, instead, can be used only one (1) key.
*/
#define AUI_STANDBY_KEYLIST_LEN (2)

/*******************************Global Type List*******************************/

/**
Enum to specify what the seven segment displays (SSD) on the front panel of the
platform will show when it goes into standby mode.
*/
typedef enum aui_pannel_show_type {

    /**
    Value to specify that the SSDs will show a <b> blank string </b>
    */
    AUI_PANNEL_SHOW_NONE =0,

    /**
    Value to specify that the SSDs will show the string @b "OFF"
    */
    AUI_PANNEL_SHOW_OFF,

    /**
    Value to specify that the SSDs will show the <b> current time </b>
    */
    AUI_PANNEL_SHOW_TIME,

    /**
    Value to specify that the SSDs will <b> not change the content </b>
    */
    AUI_PANNEL_SHOW_ON

} aui_pannel_show_type;

// @coding

/**
Enum to specify the different ways available for the targeted platform to
awake from the standby mode
*/
typedef enum aui_standby_type {


    /**
    Value to specify a cold (hard) booting, i.e. the power of the system is
    physically turned off and back on again abruptly causing a initial boot
    of the platform.
    */
    AUI_STANDBY_COLD_BOOT,


    /**
    Value to specify the automatically awakening by the internal timer of the
    platform
    */
    AUI_STANDBY_TIMER = 1,

    /**
    Value to specify the awakening by pressing a specific key on the STB panel
    */
    AUI_STANDBY_PANEL,

    /**
    Value to specify the awakening by pressing a specific key on the IR remote
    controller
    */
    AUI_STANDBY_IR,

    /**
    Value to specify a watch dog rebooting, indicating that the plaftorm has
    just recovered from a fatal error
    */
    AUI_STANDBY_WATCHDOG,

    /**
    Value to specify a normal rebooting, indicating that the plaftorm has just
    boot up from a reboot triggered by the function #aui_sys_reboot
    */
    AUI_STANDBY_REBOOT

} aui_standby_type;

// @endcoding

/**
Enum to specify which system information are available to be received.
*/
typedef enum aui_sys_item_get {

    /**
    Value to specify the receiving of the chipset ID
    */
    AUI_SYS_GET_CHIP_ID,

    /**
    Value to specify the receiving of the chipset version number
    */
    AUI_SYS_GET_REV_ID,

    /**
    Value to specify the product ID
    */
    AUI_SYS_GET_PRODUCT_ID

} aui_sys_item_get;

/**
Enum to specify the different power states of the platform
*/
typedef enum aui_power_state {

    /**
    Value to specify that the platform is in the normal working state
    */
    AUI_POWER_ON,

    /**
    Value to specify that the platform is in the sleeping state.\n
    In this state, the memory module will still work in background.

    @note   @b 1. For more detailed information, please refer to the function
                  #aui_enter_pm_standby

    @note   @b 2. The awakening from this state is faster than from the state
                  #AUI_POWER_PMU_STANDBY
    */
    AUI_POWER_PM_STANDBY,

    /**
    Value to specify that the platform is in the deep sleeping state.\n
    In this state, only the PMU module is working in background.

    @note   The awakening from this state is slower than from the state
            #AUI_POWER_PM_STANDBY
    */
    AUI_POWER_PMU_STANDBY

} aui_power_state;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Miscellaneous (MISC) Module </b> to specify the IR key
        code and IR code protocol used for awakening the platform
        </div> @endhtmlonly

        Struct to specify the IR key code and IR code protocol used for awakening
        the platform.

@note   If using the <b> Linux Infrared Remote Control (LIRC) </b>, the IR key
        code can be found in the file\n\n

        <b><em> /etc/lirc/lircd.conf</em></b> \n\n

        and is split in a @b pre_data and @b code, for example
        1. @b KEY_YELLOW = 0x60DFF807
           - @b pre_data = 0x60DF
           - @b code = 0xF807
        2. @b KEY_BLUE = 0x60DFBA45
           - @b pre_data = 0x60DF
           - @b code = 0xBA45

@note   then the content file is exemplified below:

        begin remote

          name              ali_60_key_remote
          bits              16
          flags             SPACE_ENC|CONST_LENGTH
          eps               30
          aeps              100

          header            8913  4448
          one               555  1679
          zero              555   566
          ptrail            553
          repeat            8918  2222
          pre_data_bits     16
          pre_data          0x60DF
          gap               107508
          toggle_bit_mask   0x0

          begin codes
            KEY_YELLOW      0xF807
            KEY_BLUE        0xBA45
          end codes

        end remote
*/
typedef struct aui_ir_code {

    /**
    Member to specify the IR key code
    */
    unsigned long code;

    /**
    Member to specify the IR code protocol as defined in the enum #aui_ir_protocol
    */
    aui_ir_protocol protocol;

} aui_ir_code;

// @coding

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Miscellaneous (MISC) Module </b> to specify the
        parameters used by the platform when going into the standby mode
        </div> @endhtmlonly

        Struct to specify the parameters used by the platform when going in the
        standby mode
*/
typedef struct aui_standby_setting {

    /**
    Member to specify the current power state of the platform, as defined in the
    enum #aui_power_state
    */
    aui_power_state state;

    /**
    Member to specify the wake up time of the platform

    @note   If this pointer is set to NULL, the wake up timer will not be set.
    */
    struct tm *wakeup_time;

    /**
    Member to specify the key used for awaking the platform, as defined in the
    struct #aui_ir_code.
    */
    aui_ir_code standby_key[AUI_STANDBY_KEYLIST_LEN];

    /**
    Member to specify what the seven segment displays (SSD) on the front panel
    of the platform will show when it goes into standby mode, as defined in the
    enum #AUI_PANNEL_SHOW_TIME
    */
    aui_pannel_show_type display_mode;

} aui_standby_setting;

// @endcoding

/*****************************Global Function List*****************************/

/**
@brief          Function used to register the PMU bin data which are the MCU
                Module program data.\n
                The MCU Module will work even the platform is in standby mode.

@param[in]      puc_pmu_data        = Desired PMU bin data to be run in the MCU
                                      Module
@param[in]      ul_data_len         = The input PMU bin data length

@return         @b AUI_RTN_SUCCESS  = Running of the PMU bin data in MCU Module
                                      performed successfully
@return         @b AUI_RTN_FAIL     = Running of the PMU bin data in MCU Module
                                      failed for some reasons

@note           This function can work @a only in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_reg_pmu_bin (

    unsigned char *puc_pmu_data,

    unsigned long ul_data_len

    );

/**
@brief          Function used to perform the initialization of the Standby Module.

@return         @b AUI_RTN_SUCCESS  = Initializing of the Standby Module performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Initializing of the Standby Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_standby_init (

    void

    );

/**
@brief          Function used to set the seven segment displays (SSD) on the
                front panel of the platform for the standby mode.

@param[in]      standby_type        = What the SSDs will show during the standby
                                      mode, as defined in the enum
                                      #AUI_PANNEL_SHOW_TYPE

@return         @b AUI_RTN_SUCCESS  = Setting of the SSDs for the standby mode
                                      performed successfully
@return         @b AUI_RTN_FAIL     = Setting of the SSDs for the standby mode
                                      failed for some reasons
*/
AUI_RTN_CODE aui_standby_set_show_type (

    aui_pannel_show_type standby_type

    );

/**
@brief          Function used to set the wake-up keys of IR remote controller
                before the platform goes into standby mode.

@param[in]      firstkey            = The first wake-up key to be set
@param[in]      firstkey_type       = The protocol of the first wake-up key,
                                      please refer to the enum #aui_ir_protocol
@param[in]      secondkey           = The second wake-up key to be set
                                      - @b Caution: @a Only <b> Linux  OS </b>
                                           supports the second wake-up key.
@param[in]      secondkey_type      = The protocol of the second wake-up key,
                                      please refer to the enum #aui_ir_protocol

@return         @b AUI_RTN_SUCCESS  = Setting of the wake-up keys performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Setting of the wake-up keys failed for
                                      some reasons
*/
AUI_RTN_CODE aui_standby_set_ir (

    unsigned int firstkey,

    unsigned int firstkey_type,

    unsigned int secondkey,

    unsigned int secondkey_type
    );

// @coding

/**
@brief          Function used to set the current time into PMU Module \n
                If a timer is used to exit from the standby mode, the steps below
                should always be performed:
                - #aui_standby_init
                - #aui_standby_set_current_timer
                - #aui_standby_set_wakeup_timer
                - #aui_standby_enter

@param[in]      *p_time            = Pointer to the current time of the platform
                                     to be set into PMU Module for initializing
                                     the MCU time, which will keep on tick-tock
                                     during the standby mode.

@return         @b AUI_RTN_SUCCESS = Setting of the current time for the standby
                                     mode performed successfully
@return         @b AUI_RTN_FAIL    = Setting of the current time for the standby
                                     mode failed for some reasons
*/
AUI_RTN_CODE aui_standby_set_current_timer (

    struct tm *p_time

    );

/**
@brief          Function used to get the current time from the PMU Module after
                the platform waked-up.

@note           Before the platform goes into the standby mode, the RTC can be
                stored into the PMU Module. During the standby mode, the clock
                will be kept on working.\n
                After the platform waked-up from the standby mode, the current
                time can be gotten from PMU Module and used to set the RTC Module.\n
                If the platform is powered off, the current time in PMU will be lost.

@param[out]     *p_time             = Pointer to the current time of PMU Module
                                      just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the current time from PMU Module
                                      performed successfully
@return         @b AUI_RTN_FAIL     = Getting of the current time from PMU Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_standby_get_current_timer (

    struct tm *p_time

    );

/**
@brief          Function used to set the wake-up time before the platform goes
                into the standby mode.

@param[in]      *p_time             = Pointer to the wake-up time to be set

@return         @b AUI_RTN_SUCCESS  = Setting of the wake-up time performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Setting of the wake-up time failed for some
                                      reasons
*/
AUI_RTN_CODE aui_standby_set_wakeup_timer (

    struct tm *p_time

    );

// @endcoding

/**
@brief          Function used to know how the platform waked-up, i.e. either by
                IR remote/panel keys or a timer.

@param[in]      p_standby_type      = Pointer to the different ways available
                                      for the targeted platform to awake from
                                      the standby mode, as defined in the enum
                                      #aui_standby_type

@return         @b AUI_RTN_SUCCESS  = Getting of the wake-up mode information
                                      performed successfully
@return         @b AUI_RTN_FAIL     = Getting of the wake-up mode information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_get_power_status (

    aui_standby_type *p_standby_type

    );

/**
@brief          Function used to make the platform goes into the standby mode.

@return         @b AUI_RTN_SUCCESS  = Going into standby mode performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Going into standby mode failed for some
                                      reasons
*/
AUI_RTN_CODE aui_standby_enter (

    void

    );

/**
@brief          Function used to make the platform goes into the PM standby mode.

@note           This function is supported in projects based @a only on
                <b> Linux OS </b>, where the PM standby mode of the platform is
                also known as <b> Suspend-To-RAM (STR) </b>.\n

@return         @b AUI_RTN_SUCCESS  = Going into PM standby mode performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Going into PM standby mode failed some
                                      reasons

@note     @b 1. In PM standby mode, the main memory (RAM) of the platform
                will be kept on working and the information related to
                system configuration, open applications and active files
                are stored into it, while most of other system's components
                are turned off. Under those conditions, the platform will
                boot faster than in PMU mode when is invoked by user.

@note     @b 2. About hardware configuration, the power should be managed
                to keep the memory a while in PM standby mode.

@note     @b 3. About kernel configuration:
                1. For ALi PM standby function, enable the options
                   - Symbol: ALI_PM [=y]
                   - Symbol: ALI_STANDBY_TO_RAM [=y]
                2. For for the debug function, enable the option:
                   - Symbol: ALI_STR_DEBUG_ENABLE [=y]
*/
AUI_RTN_CODE aui_enter_pm_standby (

    void

    );

/**
@brief          Function used to make the platform goes into the PMU standby mode.\n
                In PMU standby mode, the platform will reboot when is invoked

@return         @b AUI_RTN_SUCCESS  = Going into PMU standby mode performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Going into PMU standby mode failed some
                                      reasons

@note           @b 1. This function is the same as the
                      #aui_standby_enter

@note           @b 2. It is recommended to use the function #aui_standby_set_state
                      (for both <b> TDS OS </b> and <b> Linux OS </b>)
*/
AUI_RTN_CODE aui_enter_pmu_standby (

    void

    );

/**
@brief          Function used to reboot the platform

@param[in]      ul_time_ms          = The desired time (in @a millisecond (ms))
                                      after which the platform will be rebooted

@return         @b AUI_RTN_SUCCESS  = Rebooting of the platform performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Rebooting of the platform failed for some
                                      reasons
*/
AUI_RTN_CODE aui_sys_reboot (

    unsigned long ul_time_ms

    );

/**
@brief          Function used to configure the standby mode parameters then make
                the platform goes into the standby mode.\n
                This function performs the steps below:
                - #aui_standby_set_current_timer
                - #aui_standby_set_wakeup_timer
                - #aui_standby_set_show_type
                - #aui_standby_set_ir

@param[in]      setting             = The desired standby mode parameter to be
                                      set

@return         @b AUI_RTN_SUCCESS  = Setting of the standby mode parameter
                                      performed successfully
@return         @b AUI_RTN_FAIL     = Setting of the standby mode parameters
                                      failed for some reasons
*/
AUI_RTN_CODE aui_standby_set_state (

    aui_standby_setting setting

    );

/**
@brief          Function used to get system information.

@param[in]      ul_item             = The desired system information to be gotten,
                                      as defined in the enum #aui_sys_item_get_e
@param[in]      pv_param            = Pointer to the memory location where the
                                      system information just gotten will be
                                      stored

@return         @b AUI_RTN_SUCCESS  = Getting of the system information performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Getting of the system information failed
                                      for some reasons
*/
AUI_RTN_CODE aui_sys_get (

    unsigned long ul_item,

    void *pv_param

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define AUI_PANNEL_SHOW_TYPE aui_pannel_show_type

#define aui_standby_type_e aui_standby_type

#define aui_sys_item_get_e aui_sys_item_get

#define aui_ir_code_st aui_ir_code

#define aui_standby_setting_st aui_standby_setting

#define AUI_SHOW_BANK AUI_PANNEL_SHOW_NONE

#define AUI_SHOW_OFF AUI_PANNEL_SHOW_OFF

#define AUI_SHOW_TIME AUI_PANNEL_SHOW_TIME

#define AUI_SHOW_ON AUI_PANNEL_SHOW_ON

#define AUI_COLD_BOOT AUI_STANDBY_COLD_BOOT

#define AUI_IR_NEC AUI_IR_TYPE_NEC

#define AUI_IR_LAB AUI_IR_TYPE_LAB

#define AUI_IR_50560 AUI_IR_TYPE_50560

#define AUI_IR_KF AUI_IR_TYPE_KF

#define AUI_IR_LOGIC AUI_IR_TYPE_LOGIC

#define AUI_IR_SRC AUI_IR_TYPE_SRC

#define AUI_IR_NSE AUI_IR_TYPE_NSE

#define AUI_IR_RC5 AUI_IR_TYPE_RC5

#define AUI_IR_RC5_X AUI_IR_TYPE_RC5_X

#define AUI_IR_RC6 AUI_IR_TYPE_RC6

#endif

/// @endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


