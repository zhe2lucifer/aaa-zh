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
Current ALi author: Fawn.Fu, Oscar.Shi
Last update:        2017.02.25
-->

@file   aui_panel.h

@brief  Panel Module

        <b> Panel Module </b> is used to
        - Control LED panel
        - Control seven-segment display (SSD)
        - Set panel key map

        of <b> ALi Set-Top Box </b> (STB).

        The @a essential usage stepwise of Panel Module is described below:
        - Initialize the Panel Device
        - Open the Panel Device
        - Operate the Panel Device to perform the following main operations on
          ALi STB:
          - Display data on the SSD
          - Control the LEDs on the Panel Device
          - Set the AUI key map format
          - Set the AUI key repeating interval.
        - Close the Panel Device
        - De-Initialize the Panel Device

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_PANEL_H

#define _AUI_PANEL_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number </b> of the Panel Module and is used
by the function #aui_panel_version_get.\n
It is a hexadecimal number that consist of three (3) reading parts (sorted
from left to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_PANEL    (0X00010000)

/**
Macro to indicate the <b> maximum length </b> acceptable (in characters) for
the <b> name of a Panel Module </b>

@attention This macro is @a reserved to ALi R&D Dept.then users can ignore it
*/
#define AUI_FULL_FILE_NAME_LEN_MAX  (256)

/**
Macro to indicate the <b> maximum number of Panel Devices </b> that can be
opened together by the function #aui_panel_open

@warning At present not more than one (1) Panel Device can be opened together.
*/
#define MAX_PANEL_ID 1

/*******************************Global Type List*******************************/

/**
This enum is reserved to ALi R&D Dept. then user can ignore it
*/
typedef enum aui_en_panel_dev_status {

    AUI_PANEL_IDLE,

    AUI_PANEL_OPEN

} aui_panel_dev_status;

/**
Enum to specify all available <b> mode to show data </b> on the seven-segment
display (SSD) to be set, and is used by the functions #aui_panel_display and
#aui_panel_clear
*/
typedef enum aui_panel_data_type {

    /**
    This value is to indicate the <b> Character Mode </b>
    */
    AUI_PANEL_DATA_ANSI = 0x1,

    /**
    This value is to indicate the <b> Hexadecimal Number Mode </b>

    @attention This mode is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_PANEL_DATA_HEX = 0x2,

    /**
    This value is to indicate the <b> Command Mode </b> then user can enter
    specific commands to control the LEDs of the panel of ALi STB, e.g. the
    power LED
    */
    AUI_PANEL_CMD_LED_LOCK = 0x4

} aui_panel_data_type;

/**
Enum to list all the commands which are used for getting the configuration of
the Panel Device, and is used by the function #aui_panel_get
*/
typedef enum aui_en_panel_item_get {

    /**
    This enum value specifies the command to get the configuration of the
    Panel Device
    */
    AUI_PANEL_GET_CFGPARAM=0,

    /**
    This enum value specifies the maximum number of commands can be supported
    */
    AUI_PANEL_GET_CMD_LAST

} aui_panel_item_get;

/**
Enum to list all the command which are used for setting the Panel Device, and
is used by the function #aui_panel_set
*/
typedef enum aui_en_panel_item_set {

    /**
    This enum value specifies the command to set the Panel Device
    */
    AUI_PANEL_SET_CFGPARAM=0,

    /**
    This enum value specifies the maximum number of commands can be supported
    */
    AUI_PANEL_SET_CMD_LAST

} aui_panel_item_set;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Panel Module </b> reserved to ALi R&D Dept. then user
        can ignore it
        </div> @endhtmlonly

@attention  This struct is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef struct aui_st_panel_module_attr {

} aui_panel_module_attr, *aui_p_panel_module_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Panel Module </b> to specify all configuration details
        of the Panel Device
        </div> @endhtmlonly

        Struct to specify all configuration details of the Panel Device, and is
        used by the function #aui_panel_open
*/
typedef struct aui_st_cfg_panel {

    /**
    Member to record the number of LEDs of the Panel Device
    */
    unsigned long ul_led_cnt;

} aui_cfg_panel, *aui_p_cfg_panel;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Panel Module </b> to specify the format of AUI key paring
       </div> @endhtmlonly

       Struct to specify the <b> AUI key pairing format</b>, and is used by the
       function #aui_panel_set_key_map
*/
typedef struct aui_pannel_key_map {

    /**
    Member to record the @b physical (hardware) key value of the AUI key paring
    */
    unsigned long code;

    /**
    Member to record the @b logic (software) key value of the AUI key paring
    */
    unsigned short key;

} aui_pannel_key_map;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Panel Module </b> to specify the AUI key map format
        </div> @endhtmlonly

        Struct to specify the <b> AUI key map format </b>, and is used by the
        function #aui_panel_set_key_map
*/
typedef struct aui_pannel_key_map_cfg {

    /**
    Member as a pointer to the AUI key paring as defined in the struct
    #aui_panel_key_map
    */
    aui_pannel_key_map *map_entry;

    /**
    Member to indicate the number of AUI key parings in #map_entry
    */
    unsigned long unit_num;

} aui_pannel_key_map_cfg;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the <b> version number </b> of the Panel
                Module as defined in the macro #AUI_MODULE_VERSION_NUM_PANEL

@warning        This function can be used at any time

@param[in]      pul_version         = Pointer to the variable used to record the
                                      version number of the Panel Module to be
                                      gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the version number of the Panel
                                      Module performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is either
                                      invalid or NULL then getting of the version
                                      number of the Panel Module failed

*/
AUI_RTN_CODE aui_panel_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to @b initialize the Panel Device before its
                opening by the function #aui_panel_open

@param[in]      p_call_back_init    = Callback function used for the
                                      initialization of the Panel Device, as per
                                      comment for the function pointer #p_fun_cb
@param[in]      pv_param            = Input parameter of the callback function
                                      @b p_call_back_init
                                      - @b Caution: The callback function
                                           @b p_call_back_init @a must take an
                                           input parameter, anyway it may take
                                           @b NULL value as well

@return         @b AUI_RTN_SUCCESS  = Panel device initialized successfully
*/
AUI_RTN_CODE aui_panel_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to @b de-initialize the Panel Device.after its
                closing by the function #aui_panel_close

@param[in]      p_call_back_de_init = Callback function used to de-initialize
                                      of the Panel Device, as per comment for
                                      the function pointer #p_fun_cb
@param[in]      pv_param            = Input parameter of the callback function
                                      @b p_call_back_de_init
                                      - @b Caution: The callback function
                                           @b p_call_back_de_init @a must take
                                           a input parameter, anyway it may take
                                           @b NULL value as well

@return         @b AUI_RTN_SUCCESS  = Panel Device de-initialized successfully
*/
AUI_RTN_CODE aui_panel_de_init (

    p_fun_cb p_call_back_de_init,

    void *pv_param

    );

/**
@brief          Function used to @b open the Panel Device after its initialization
                by the function #aui_panel_init

@param[in]      ul_panelID          = ID of the Panel Device to be opened
@param[in]      p_panel_param       = Pointer to the struct #aui_cfg_panel, which
                                      specify configuration details of the Panel
                                      Device

@param[out]     pp_hdl_panel        = #aui_hdl pointer to the handle of the Panel
                                      Device just opened

@return         @b AUI_RTN_SUCCESS  = Panel Module opened successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter (i.e.
                                      [in], [out]) are invalid
@return         @b AUI_RTN_FAIL     = Opening of the Panel Device failed
@return         @b AUI_RTN_ENOMEN   = No memory for "malloc"
*/
AUI_RTN_CODE aui_panel_open (

    unsigned long ul_panelID,

    aui_cfg_panel *p_panel_param,

    aui_hdl *pp_hdl_panel

    );

/**
@brief          Function used to @b close the Panel Device before its
                de-initialization by the function #aui_panel_init then the
                related handle will be released (i.e. the related resources such
                as memory, device)

@param[in]      pv_hdl_panel        = #aui_hdl handle of the Panel Device already
                                      opened and to be closed

@return         @b AUI_RTN_SUCCESS  = Panel Module closed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter (i.e.
                                      [in], [out]) are invalid
*/
AUI_RTN_CODE aui_panel_close (

    aui_hdl pv_hdl_panel

    );

/**
@brief          Function used to <b> show data </b> on the Seven-Segment Display
                (SSD) of the Panel Device

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to show data on
                                      the Seven-Segment Display
@param[in]      data_type           = Mode of showing data on the Seven-Segment
                                      Display to be set, as defined in the enum
                                      #aui_panel_data_type
@param[in]      puc_buf             = Pointer to a buffer intended to hold the
                                      data to be shown on the Seven-Segment
                                      Display
@param[in]      ul_data_len         = Number of data in the buffer to be shown

@return         @b AUI_RTN_SUCCESS  = Showing of the data on Seven-Segment Display
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one of the parameters (i.e. [in],
                                      [out]) is invalid
*/
AUI_RTN_CODE aui_panel_display (

    aui_hdl handle,

    aui_panel_data_type data_type,

    unsigned char *puc_buf,

    unsigned long ul_data_len

    );

/**
@brief          Function used to <b> clear data </b> showing on the Seven-Segment
                Display (SSD) of the Panel Device

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to clear data
                                      showing on the Seven-Segment Display
@param[in]      data_type           = Mode of showing data on the Seven-Segment
                                      Display already set, as defined in the
                                      enum #aui_panel_data_type
                                      - @b Caution: This parameter and the
                                           namesake in the function
                                           #aui_panel_display @a should be the same

@return         @b AUI_RTN_SUCCESS  = Clearing of the data showing on
                                      Seven-Segment Display performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter (i.e.
                                      [in], [out]) are invalid
*/
AUI_RTN_CODE aui_panel_clear (

    aui_hdl handle,

    aui_panel_data_type data_type

    );

/**
@brief          Function used to <b> get the attributes configuration </b> of the
                Panel Device

@warning        This function is @a reserved to ALi R&D Dept. then user can
                ignore it

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to get the
                                      attributes configuration of the Panel Device
@param[in]      ul_item             = Command for getting the attributes
                                      configuration of the Panel Device, as
                                      defined in the enum #aui_panel_item_get

@param[out]     pv_param            = Pointer to an address memory where storing
                                      the attribute configuration of the Panel
                                      Device to be gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the attributes configuration
                                      of the Panel Device performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter (i.e.
                                      [in], [out]) are invalid
*/
AUI_RTN_CODE aui_panel_get (

    aui_hdl handle,

    aui_panel_item_get ul_item,

    void *pv_param

    );

/**
@brief          Function used to <b> set the attributes configuration </b> of
                the Panel device

@warning        This function is @a reserved to ALi R&D Dept. the user can
                ignore it

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to set the
                                      attributes configuration of the Panel Device
@param[in]      ul_item             = Command for setting the attributes
                                      configuration of the Panel Device, as
                                      defined in the enum #aui_panel_item_get
@param[in]      pv_param            = Pointer to an address memory where storing
                                      the attributes configuration of the Panel
                                      Device to be set

@return         @b AUI_RTN_SUCCESS  = Setting of the attributes configuration
                                      of the Panel Device performed successfully
@return         @b AUI_RTN_EINVAL   = A least of the parameters (i.e. [in])
                                      is invalid
*/
AUI_RTN_CODE aui_panel_set (

    aui_hdl handle,

    aui_panel_item_set ul_item,

    void *pv_param

    );

/**
@brief          Function used to <b> set the state of a discrete LED </b> of the
                Panel Device

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to set the state
                                      of a discrete LED of the Panel Device
@param[in]      ul_led_number       = ID Number of the specific LED of the Panel
                                      Device to be set
@param[in]      uc_led_active       = Variable as a @a flag to set the state of
                                      a discrete LED of the Panel Device, in
                                      particular:
                                      - @b 1 = LED ON
                                      - @b 0 = LED OFF

@return         @b AUI_RTN_SUCCESS  = Setting of the state of a discrete LED of
                                      the Panel Device performed successfully
@return         @b AUI_RTN_EINVAL   = A least of the parameters (i.e. [in]) is
                                      invalid
*/
AUI_RTN_CODE aui_panel_set_led_state (

    aui_hdl handle,

    unsigned long ul_led_number,

    unsigned char uc_led_active

    );

/**
@brief          Function used to <b> set the AUI key map </b> of the Panel Device

@warning        This function can be used @a only in projects based on
                <b> Linux OS </b>

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to set the AUI
                                      key map of the Panel Device
@param[in]      p_cfg               = Pointer to the struct
                                      #aui_pannel_key_map_cfg. which specify the
                                      AUI key map format of the Panel Device
@return         @b AUI_RTN_SUCCESS  = Setting of the AUI key map of the Panel
                                      Device performed successfully
@return         @b AUI_RTN_EINVAL   = A least of the parameters (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Setting of the AUI key map of the Panel
                                      Device failed for some reason
*/
AUI_RTN_CODE aui_panel_set_key_map (

    aui_hdl handle,

    aui_pannel_key_map_cfg *p_cfg

    );

/**
@brief          Function used to <b> set the AUI key repeat interval </b> of the
                Panel Device

@warning        This function can be used @a only in projects based on
                <b> Linux OS </b>

@param[in]      handle              = #aui_hdl handle of the Panel Device already
                                      opened and to be managed to set the AUI
                                      key repeat interval of the Panel Device


@param[in]      ul_delay             = Key hold down time (in <em> millisecond (ms)
                                      </em> unit) beyond which the key can be
                                      considered long term pressed

@param[in]      ul_interval          = Periodic interval time (in <em> millisecond
                                      (ms) </em> unit) for reporting whether the
                                      key is still pressed at <b> t &ge; delay
                                      </b>.\n
                                      That reporting last until the key is
                                      released

@return         @b AUI_RTN_SUCCESS  = Setting of the AUI key repeat interval of
                                      the Panel Device performed successfully
@return         @b AUI_RTN_EINVAL   = A least of the parameters (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Setting of the AUI key repeat interval of
                                      the Panel Device failed for some reason
*/
AUI_RTN_CODE aui_panel_set_key_rep_interval (

    aui_hdl handle,

    unsigned long ul_delay,

    unsigned long ul_interval

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_en_input_datatype aui_panel_data_type

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

