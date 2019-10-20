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
Last update:        2017.02.25
-->

@file   aui_rtc.h

@brief  Real Time Clock (RTC) Module

        <b> Real Time Clock (RTC) Module </b> is used to keep track of the
        current time.Below the main working flow of this module:
        - Initialize / De-initialize the RTC Module
        - Open/Close the RTC Module
        - Set/Get the RTC time value
        - Set/Get the RTC alarm time value/attributes

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Real Time Clock (RTC) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_RTC_H

#define _AUI_RTC_H

/*************************Included Header File List***************************/

#include "aui_common.h"

//#include <bus/rtc/rtc.h>

/*******************************Global Macro List*****************************/

/**
Macro to indicate the version number of RTC Module, which is a hexadecimal
number that consist of three (3) reading parts (sorted from left to right)
as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_RTC  (0X00010000)

/**
Macro to indicate the maximum number of RTC Devices supported by RTC Module
*/
#define AUI_RTC_DEV_CNT_MAX         (8)

/*******************************Global Type List*******************************/

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Real Time clock (RTC) Module </b> to specify the RTC
        time format
        </div> @endhtmlonly

        Struct to specify the RTC time format i.e. in <b> YYYY/MM/DD, hh:mm:ss </b>
        where:
        - @b YYYY = Year
        - @b MM   = Month
        - @b DD   = Day
        - @b hh   = Hour
        - @b mm   = Minutes
        - @b ss   = Second
*/
typedef struct aui_clock {

    /**
    Member to specify the @b Year field of the RTC time, which range is
    <b> [0, 65535] </b>
    */
    unsigned short year;

    /**
    Member to specify the @b Month field of the RTC time, which range is
    <b> [1, 12] </b>
    */
    unsigned char month;

    /**
    Member to specify the <b> Day of the month </b> field of the RTC time, which
    range is <b> [1, 31] </b>
    */
    unsigned char date;

    /**
    Member to specify the <b> Day of the week </b> field of the RTC time,
    which range is <b> [0, 6] </b>

    @warning  @b 1. Please note this member <b> DOES NOT </b> specify the
                    field "day of the month" for which, instead, need to
                    use the member @b date of this struct

    @warning  @b 2. This field is @a deprecated so it can be ignored
    */
    unsigned char day;

    /**
    Member to specify the @b Hours field of the RTC time, which range is
    <b> [0, 23] </b>
    */
    unsigned char hour;

    /**
    Member to specify the @b Minutes field of the RTC time, which range is
    <b> [0, 59] </b>
    */
    unsigned char min;

    /**
    Member to specify the @b Seconds field of the RTC time, which range is
    <b> [0, 59] </b>
    */
    unsigned char sec;

} aui_clock;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Real Time clock (RTC) Module </b> to specify
        RTC alarm attributes
        </div> @endhtmlonly

        Struct to specify the RTC alarm attributes

@note   The alarm index range is limited in this struct, in particualar there
        are two (2) types of RTC alarm as below:
        - @b Minutes alarm      = The alarm time is accurate to minute
        - @b Milliseconds alarm = The alarm time is accurate to millisecond
*/
typedef struct aui_rtc_alarm_attr {

    /**
    Member to specify the @b minimum alarm index of the @b minutes alarm time
    */
    unsigned char min_alarm_num_start;

    /**
    Member to specify the @b maximum alarm index of the @b minutes alarm time
    */
    unsigned char min_alarm_num_end;

    /**
    Member to specify the @b minimum alarm index of the @b milliseconds alarm time
    */
    unsigned char ms_alarm_num_start;

    /**
    Member to specify the @b maximum alarm index of the @b milliseconds alarm time
    */
    unsigned char ms_alarm_num_end;

    /**
    Member as a @a flag to specify whether the alarm callback function is enabled
    to work or not, in particular:
    - @b 1 = Alarm callback function enabled to work
    - @b 0 = Alarm callback function not enabled to work
    */
    unsigned char cb_en;

} aui_rtc_alarm_attr, *p_aui_rtc_alarm_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Real Time clock (RTC) Module </b> to specify minutes
        alarm attributes
        </div> @endhtmlonly

        Struct to specify the attribute of minutes alarm
*/
typedef struct aui_min_alarm {

    /**
    Member to specify the @b month of the @b minutes alarm time
    */
    unsigned char month;

    /**
    Member to specify the <b> day of the month </b> of the @b minute alarm time
    */
    unsigned char date;

    /**
    Member to specify the @b hours of the @b minutes alarm time
    */
    unsigned char hour;

    /**
    Member to specify the @b minutes of the @b minutes alarm time
    */
    unsigned char min;

} aui_min_alarm;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Real Time clock (RTC) Module </b> to specify
        milliseconds alarm attributes
        </div> @endhtmlonly

        Struct to specify the attribute of milliseconds alarm
*/
typedef struct aui_ms_alarm {

    /**
    Member to specify @b Hours of the @b milliseconds alarm time
    */
    unsigned char hour;

    /**
    Member to specify the @b Minutes of the @b milliseconds alarm time
    */
    unsigned char min;

    /**
    Member to specify the @b Seconds of the @b milliseconds alarm time
    */
    unsigned char sec;

    /**
    Membert to specify the @b Milliseconds of the @b Milliseconds alarm time
    */
    unsigned short ms;

} aui_ms_alarm;

/**
Function pointer to specifies the type of RTC alarm callback function

@note     The callback function can be called when the alarm time is over

@warning  The alarm callback function can work whether the member #cb_en of the
          struct #aui_rtc_alarm_attr is set as @b 1
*/
typedef void (*aui_rtc_alarm_callback) (

    void

    );

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of the RTC Module

@param[out]     *pul_version        = Pointer to the variable which contains
                                      the version number of the RTC Module

@return         @b AUI_RTN_SUCCESS  = Getting of the version number of RTC
                                      Module performed successfully
@return         @b Other_Values     = Getting of the version number of RTC
                                      Module failed for some reasons

@note           The initial version number value is set by the macro
                #AUI_MODULE_VERSION_NUM_RTC, users can modify it according the
                own requirements
*/
AUI_RTN_CODE aui_rtc_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to perform the initialization of the RTC Module
                before its opening by the function #aui_rtc_open

@return         @b AUI_RTN_SUCCESS  = RTC Module initialized successfully
@return         @b Other_Values     = Initializing of the RTC Module failed for
                                      some other reasons
*/
AUI_RTN_CODE aui_rtc_init (

    void

    );

/**
@brief          Function used to perform the de-initialization of the RTC Module
                after its initialization by the function #aui_rtc_init

@return         @b AUI_RTN_SUCCESS  = RTC Module de-initialized successfully
@return         @b Other_Values     = De-initializing of the RTC Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_rtc_de_init (

    void

    );

/**
@brief          Function used to open the RTC Module and configure the desired
                attributes then get the related handle

@warning        This function can be used
                - Either after performing the initialization of the RTC Module
                  by the function #aui_rtc_init for the first opening of the
                  RTC Module
                - Or after closing the RTC Module by the function #aui_rtc_close,
                  considering the initialization of the RTC Module has been
                  performed previously by the function #aui_rtc_init

@param[out]     *p_rtc_handler      = #aui_hdl pointer to the handle of the RTC
                                      Module just opened

@return         @b AUI_RTN_SUCCESS  = RTC Module opened successfully
@return         @b Other_Values     = Opening of the RTC Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_rtc_open (

    aui_hdl *p_rtc_handler

    );

/**
@brief          Function used to close the RTC Module already opened by the
                function #aui_rtc_open then the related handle (i.e. the
                related resources such as memory, device) will be released

@warning        This function can @a only be used in pair with the function
                #aui_rtc_open. In particular, after closing the RTC Module
                user can
                - Either perform the de-initialization of the RTC Module
                  by the function #aui_rtc_de_init
                - Or open again the RTC Module by the function #aui_rtc_open,
                  considering the initialization of the RTC Module has been
                  performed previously by the function #aui_rtc_init

@param[in]       rtc_handler        = #aui_hdl pointer to the handle of the
                                      RTC Module already opened and to be
                                      closed

@return          @b AUI_RTN_SUCCESS = RTC Module closed successfully
@return          @b Other_Values    = Closing of the RTC Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_rtc_close (

    aui_hdl rtc_handler

    );

/**
brief           Function used to set the clock time value of the RTC low
                level driver

@param[in]      rtc_handler         = #aui_hdl pointer to the handle of the
                                      RTC Module already opened
@param[in]      *p_clock            = #aui_clock pointer to the clock time
                                      value to be set to the RTC module

@return         @b AUI_RTN_SUCCESS  = Setting of the RTC time performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid (i.e. the pointer is @b NULL)
@return         @b RTC_ERR          = An error to set the clock time value to
                                      the RTC low level driver occurred
@return         @b Other_Values     = Setting of the RTC time failed for some
                                      other reasons
*/
AUI_RTN_CODE aui_rtc_set_time (

    aui_hdl rtc_handler,

    aui_clock *p_clock

    );

/**
@brief          Function used to get the clock time value from the RTC low
                level driver

@param[in]      rtc_handler         = #aui_hdl pointer to the handle of the RTC
                                       Module already opened
@param[out]     *p_clock            = #aui_clock pointer to the clock time value
                                      to be gotten from the RTC low level driver

@return         @b AUI_RTN_SUCCESS  = Getting of the RTC time performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out]]
                                      is invalid (i.e. the pointer is @b NULL)
@return         @b RTC_ERR          = An error to get the clock time value from
                                      the RTC low level driver occurred
@return         @b Other_Values     = Getting of the RTC time failed for some
                                      other reasons
*/
AUI_RTN_CODE aui_rtc_get_time (

    aui_hdl rtc_handler,

    aui_clock *p_clock

    );

/**
@brief          Function used to get the attribute information of the RTC module

@param[in]      rtc_handler          = #aui_hdl pointer to the handle of the RTC
                                       Module already opened
@param[out]     *p_alarm             = Pointer to a struct #aui_rtc_alarm_attr
                                       which stores the attribute information
                                       of the RTC module just gotten

@return         @b AUI_RTN_SUCCESS   = Getting of the attribute information of
                                       the RTC module performed successfully
@return         @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in], [out])
                                       is invalid (i.e. the pointer is @b NULL)
@return         @b Other_Values      = Getting of the attribute information of
                                       the RTC module failed for some reasons
*/
AUI_RTN_CODE aui_rtc_get_alarm_info (

    aui_hdl rtc_handler,

    aui_rtc_alarm_attr *p_alarm

    );

/**
brief           Function used to set the minutes alarm time

@note           As example, if the minutes alarm time is set as below:
                - Month      = 1
                - Date       = 23
                - Hour       = 3
                - <b> Minute = 12 </b>

@note           then the system will alarm when the RTC time will be equal to
                <b> January 23th, 03:12:00 </b>

@param[in]      rtc_handler          = #aui_hdl pointer to the handle of the
                                       RTC Module already opened
@param[in]      alarm_num            = Minutes alarm index which range is
                                       [#min_alarm_num_start, #min_alarm_num_end]
                                       as defined in the struct #aui_rtc_alarm_attr
@param[in]      p_min_alarm          = Pointer to a struct #aui_min_alarm which
                                       specify the time is accurate to the minutes
@param[in]      cb                   = Alarm callback function as per comment of
                                       the function pointer #aui_rtc_alarm_callback

@return         @b AUI_RTN_SUCCESS   = Setting of the minute alarm time
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b RTC_ERR           = An error to set the minute alarm time to
                                       the RTC low level driver occurred
@return         @b Other_Values      = Setting of the minute alarm time failed
                                       for some other reasons
*/
AUI_RTN_CODE aui_rtc_min_alarm_set (

    aui_hdl rtc_handler,

    int alarm_num,

    aui_min_alarm *p_min_alarm,

    aui_rtc_alarm_callback cb

    );

/**
@brief          Function used to set the milliseconds alarm time

@note           As example, if the milliseconds minute alarm time is set as
                below:
                - Month            = 1
                - Date             = 23
                - Hour             = 3
                - Minute           = 12
                - <b> Millisecond  = 2000 </b>

@note           then the system will alarm when the RTC time will be equal to
                <b> January 23th, 03:12:00:02 </b>\n


@param[in]      rtc_handler          = #aui_hdl pointer to the handle of the
                                       RTC Module already opened.
@param[in]      alarm_num            = Milliseconds alarm index which range is
                                       [#ms_alarm_num_start, #ms_alarm_num_end]
                                       as defined in the struct #aui_rtc_alarm_attr
@param[in]      p_ms_alarm           = Pointer to a struct #aui_ms_alarm which
                                       specify the time is accurate to the
                                       milliseconds
@param[in]      cb                   = Alarm callback function as per comment of
                                       the function pointer #aui_rtc_alarm_callback

@return         @b AUI_RTN_SUCCESS   = Setting of the millisecond alarm time
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b RTC_ERR           = An error to set the millisecond alarm time
                                       to the RTC low level driver occurred
@return         @b Other_Values      = Setting of the millisecond alarm time
                                       failed for some other reasons

*/
AUI_RTN_CODE aui_rtc_ms_alarm_set (

    aui_hdl rtc_handler,

    int alarm_num,

    aui_ms_alarm *p_ms_alarm,

    aui_rtc_alarm_callback cb

    );

/**
@brief          Function used to enable/disable the RTC alarm time

@param[in]      rtc_handler          = #aui_hdl pointer to the handle of the
                                       RTC Module already opened
@param[in]      uc_alarm_num         = Alarm index to specify which alarm
                                       time is enabled/disabled
@param[in]      uc_enable            = Flag to specify whether the RTC alarm
                                       time is enabled or disabled, in particular:
                                       - @b 1 = Enabled
                                       - @b 0 = Disables

@return         @b AUI_RTN_SUCCESS   = Switching of the RTC alarm time
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = A least one input parameter [in]
                                       is invalid
@return         @b RTC_ERR           = An error to enable/disable the RTC alarm
                                       time occurred
@return         @b Other_Values      = Switchin of the RTC alarm time failed
                                       for some other reasons
*/
AUI_RTN_CODE aui_rtc_alarm_switch (

    aui_hdl rtc_handler,

    unsigned char uc_alarm_num,

    unsigned char uc_enable

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_clock_st aui_clock

#define aui_rtc_alarm_attr_st aui_rtc_alarm_attr

#define aui_min_alarm_st aui_min_alarm

#define aui_ms_alarm_st aui_ms_alarm

#define cb_alarm_callback aui_rtc_alarm_callback

AUI_RTN_CODE aui_rtc_set_alarm_minutes (

    aui_hdl rtc_handler,

    int alarm_num,

    aui_min_alarm *p_min_alarm,

    aui_rtc_alarm_callback *cb

    );

AUI_RTN_CODE aui_rtc_set_alarm_ms(aui_hdl rtc_handler,

    int alarm_num,

    aui_ms_alarm *p_ms_alarm,

    aui_rtc_alarm_callback *cb

    );

#endif

/// @endcond

#ifdef __cplusplus

}

#endif

#endif

/* END Of FILE */


