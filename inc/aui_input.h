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

@file   aui_input.h

@brief  Input Module

        <b> Input Module </b> is used to deal with <b> key events </b> from the
        <b> IR (InfraRed) Remote Control Unit </b> and <b> Front Panel </b> of
        <b> ALi Set-Top Box (STB) </b>

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Input Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_INPUT_H

#define _AUI_INPUT_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version member </b> of the Input Module, and it is a
hexadecimal number that consist of three (3) reading parts (sorted from left to
right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_KEY  (0x00010000)

/**
Macro to indicate the maximum length (in @a byte unit) of a key device name.
*/
#define AUI_FULL_FILE_NAME_LEN_MAX (256)

/*******************************Global Type List*******************************/

/**
Enum to specify the <b> operation source </b> of the key event
<em> "press/release" </em>, i.e. whether that key event is from IR Remote Control
Unit or Front Panel of ALi STB.
*/
typedef enum aui_en_key_type {

    /**
    This value indicates that the key event "press/release" is from the front
    panel of the ALi STB
    */
    AUI_KEY_TYPE_FRONTPANEL = 0x01,

    /**
    This value indicates that the key event "press/release" is from the IR remote
    control unit of ALi STB
    */
    AUI_KEY_TYPE_REMOTECTRL = 0x02

} aui_key_type;

/**
Enum to specify the action status of the key, i.e. whether the key is
<em> "pressed/released" </em>.
*/
typedef enum aui_en_key_status {

    /**
    This value indicates that the key is @a "pressed".
    */
    AUI_KEY_STATUS_PRESS = 0x01,

    /**
    This value indicates that the key is @a "released".
    */
    AUI_KEY_STATUS_RELEASE = 0x02,

    /**
    This value indicates that the key is being <em> "pressed repeatedly" </em>
    */
    AUI_KEY_STATUS_REPEAT = 0x04

} aui_key_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> to specify key attribute information
       </div> @endhtmlonly

       Struct to specify a list of information related to key attribute, and is
       used as input parameter of the callback function #aui_key_callback and
       the function #aui_key_key_get
*/
typedef struct aui_st_key_info {

    /**
    Member as defined in the enum #aui_key_type to specify the operation source
    of the key event <em> "press/release" </em>
    */
    aui_key_type     e_type;

    /**
    Member as defined in the enum #aui_key_status to specify the action status
    of the key, i.e. whether the key is @a "pressed/released"
    */
    aui_key_status   e_status;

    /**
    Member to specify the key code

    @note: If the application never set the IR key map by calling the function
           #aui_key_ir_map, the value of this member is the @b physical code
           otherwise is the @b logical key defined in the struct #aui_key_map_cfg
    */
    int             n_key_code;

    /**
    Member to specify the repeat count when the key is pressed repeatedly
    */
    int             n_count;

    /**
    Member to specify the input data from the IR Remote Control Unit and Front
    Panel of ALi STB

    @attention This variable is @a reserved to ALi R&D Dept. then user can
               @a ignore it
    */
    void*           p_ext_input_data;

} aui_key_info, *aui_p_key_info;

/**
Function pointer to specify the type of callback function registered with the
function #aui_key_callback_register for the key event <em> "press/release"
</em>.\n
This callback function receives @b *p_key_info as the key action event
information, which is given when the key from the IR Remote Control Unit or
Front Panel Panel of ALi STB is <em> "pressed/released" </em>.\n
The input parameters @b *pv_user_data, instead, is not used at present then is
always set to @b NULL.
*/
typedef void (*aui_key_callback) (

    aui_key_info *p_key_info,

    void *pv_user_data

    );

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> reserved to ALi R&D Dept. then user
       can ignore it
       </div> @endhtmlonly

@attention    This struct is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef struct aui_st_key_module_attr {

} aui_key_module_attr, *aui_p_key_module_attr;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> to specify the key device configuration
       </div> @endhtmlonly

       Struct to specify the configuration of the key device when it will be
       opened, and is used by the function #aui_key_open
*/
typedef struct aui_st_cfg_key {

    /**
    Member to specify the callback function when the key is pressed/released
    */
    aui_key_callback    fn_callback;

    /**
    Member to specify the input parameter of #fn_callback but it is not used at
    present then is always set to @b NULL, as per comment for the function
    pointer #aui_key_info
    */
    void *              pv_user_data;

} aui_cfg_key, *aui_p_cfg_key;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> to specify the setting of the IR
       Remote Control Working Mode
       </div> @endhtmlonly

       Struct to specify the setting of the IR Remote Control Working Mode,
       and is used by the function #aui_pan_ir_set_endian
*/
typedef struct aui_pan_ir_endian {

    /**
    Member as a @a flag to specify if the <b> key value data </b> can be
    @a reversed when processing the IR Remote Control Protocol, in particular:
    - @b 1 = Enable
    - @b 0 = Disable

    @warning  Set this flag to 1 (i.e. enable) to make valid the value of the
              variables #bit_msb_first and #byte_msb_first of this struct
    */
    unsigned int enable;

    /**
    Member to specify the setting of the IR Remote Control Protocol, as defined
    in the struct #aui_ir_protocol
    */
    aui_ir_protocol protocol;

    /**
    Member as a @a flag to specify if the <b> key value data </b> need to be <em>
    bit reversed </em> when processing the IR Remote Control Protocol,
    in particular:
    - @b 1 = Enable
    - @b 0 = Disable

    @warning  Set this flag to 1 (i.e. enable) to make valid the value of this
              variable, which firstly has been validated by the flag #enable of
              this struct
    */
    unsigned int bit_msb_first;

    /**
    Member to specify if the <b> key value </b> data need to be <em> byte
    reversed </em> when processing the IR Remote Control Protocol, in particular:
    - @b 1 = Enable
    - @b 0 = Disable

    @warning  Set this flag to 1 (i.e. enable) to make valid the value of this
              variable, which firstly has been validated by the flag #enable of
              this struct
    */
    unsigned int byte_msb_first;

} aui_pan_ir_endian;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> to specify a key pair
       </div> @endhtmlonly

       Struct to specify a key pair
*/
typedef struct aui_key_map {

    /**
    Member to specify the physics key value
    */
    unsigned long code;

    /**
    Member to specify the logic key value
    */
    unsigned short key;

} aui_key_map;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Input Module </b> to specify a key map configuration
       </div> @endhtmlonly

       Struct to specify a key map configuration
*/
typedef struct aui_key_map_cfg {

    /**
    Member to specify the key map pointer
    */
    aui_key_map *map_entry;

    /**
    Member to specify the number of elements in the member @b map_entry of this
    struct
    */
    unsigned long unit_num;

} aui_key_map_cfg;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of the Input Module as
                defined in the macro #AUI_MODULE_VERSION_NUM_KEY

@warning        This function can be used at any time

@param[out]     *pul_version               = Pointer to the variable whose value
                                             is the version number of the Input
                                             Module

@return         @b AUI_RTN_SUCCESS         = Version number of the Input Module
                                             got successfully
@return         @b Other_Values            = Getting of the version number of
                                             Input Module failed for some
                                             reasons

@note           The initial version number is set to @b NULL(0), user can modify
                that value according to own requirement
*/
AUI_RTN_CODE aui_key_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to perform the initialization of the Input Module
                before its opening by the function #aui_key_open

@param[in]      p_call_back_init           = Callback function used for the
                                             initialization of the Input Module,
                                             as per comment for the function
                                             pointer #p_fun_cb
@param[in]      pv_param                   = The input parameter of the callback
                                             function @b p_callback_init

@return         @b AUI_RTN_SUCCESS         = Input Module initialized successfully
@return         @b AUI_RTN_EINVAL          = Either one or both of the input
                                             parameters (i.e. [in]) are either
                                             NULL or invalid
@return         @b PANEL_ERR_NO_RESOURCE   = Creating of the key mutex failed
                                             during the initialization of the
                                             Input Module
@return         @b Other_Values            = Initializing of the Input Module
                                             failed for some reasons
*/
AUI_RTN_CODE aui_key_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the Input Module
                after its closing by the function #aui_key_close

@param[in]      p_call_back_de_init        = Callback function used to de-initialize
                                             the Input Module, as per comment
                                             for the function pointer #p_fun_cb
@param[in]      pv_param                   = The input parameter of the callback
                                             function @b p_callback_de_init

@return         @b AUI_RTN_SUCCESS         = Input Module de-initialized
                                             successfully
@return         @b AUI_RTN_EINVAL          = Either one or both of the input
                                             parameters (i.e. [in]) are either
                                             NULL or invalid
@return         @b Other_Values            = De-Initializing of the Input Module
                                             failed for some reasons
*/
AUI_RTN_CODE aui_key_de_init (

    p_fun_cb p_call_back_de_init,

    void *pv_param

    );

/**
@brief          Function used to open the Input Module and configure the desired
                attributes then to get the related handle

@warning        This function can @a only be used in the <b> Pre-Run stage </b>
                of the Input Module, in particular:
                - Either after performing the initialization of the Input Module
                  by the function #aui_key_init for the first opening of the
                  Input Module
                - Or after closing the Input Module by the function #aui_key_close,
                  considering the initialization of the Input Module has been
                  performed previously by the function #aui_key_init

@param[in]      ul_devID                   = Input Device ID
                                             - @b Caution: It is set to 0 as
                                                  default
@param[in]      *p_key_param               = Pointer to the key device
                                             configuration, as defined in the
                                             struct #aui_cfg_key
                                             - @b Caution: This input parameter
                                                  is reserved to ALi R&D Dept.
                                                  then user can set it to @b NULL
                                                  value

@param[out]     *pp_hdl_key                = #aui_hdl pointer to the handle of
                                             the Input Module just opened

@return         @b AUI_RTN_SUCCESS         = Input Module opened successfully
@return         @b AUI_RTN_EINVAL          = Either one or both of the parameter
                                             (i.e. [in], [out]) are
                                             - Either NULL
                                             - Or invalid
                                             - Or the panel device is not found

@return         @b Other_Values            = Opening the Input Module failed for
                                             some reasons
*/
AUI_RTN_CODE aui_key_open (

    unsigned long ul_devID,

    aui_cfg_key *p_key_param,

    aui_hdl *pp_hdl_key

    );

/**
@brief          Function used to close the Input Module already opened by the
                function #aui_key_open then the related handle will be released
                (i.e. the related resources such as memory, device)

@warning        This function can only be used in pair with the function
                #aui_key_open used for opening Input Module. After closing the
                Input Module user can:
                - Either perform the de-initialization of the Input Module by
                  the function #aui_key_de_init
                - Or open again the Input Module by the function #aui_key_open,
                  considering the initialization of the Input Module has been
                  performed previously by the function #aui_key_init

@param[in]      pv_hdl_key                 = #aui_hdl pointer to the handle of
                                             the Input Module already opened

@return         @b AUI_RTN_SUCCESS         = Input Module closed successfully.
@return         @b AUI_RTN_EINVAL          = The input parameter (i.e. [in]) is
                                             - Either NULL
                                             - Or invalid
                                             - Or the key device status is not
                                               @b AUI_KEY_DEV_OPEN.
@return         @b Other_Values            = Closing the Input Module failed
                                             for some reasons
*/
AUI_RTN_CODE aui_key_close (

    aui_hdl pv_hdl_key

    );

/**
@brief          Function used for registering a callback function to the Input
                Module for receiving the key information when the key is pressed/
                released, as defined in the struct #aui_key_info

@param[in]      handle                     = #aui_hdl pointer to the handle of
                                             the Input Module already opened and
                                             to be managed to register the key
                                             callback function
@param[in]      fn_callback                = Pointer to the key callback function
                                             to be registered, as per comment
                                             for the function pointer
                                             #common_callback

@return         @b AUI_RTN_SUCCESS         = Registering of the key callback
                                             function to the Input Module
                                             performed successfully
@return         @b AUI_RTN_EINVAL          = Either one or both of the input
                                             parameters (i.e. [in]) are
                                             - Either NULL
                                             - Or invalid
                                             - Or the key device status is not
                                               @b AUI_KEY_DEV_OPEN.
@return         @b Other_Values            = Registering of the callback function
                                             to the Input Module failed for some
                                             reasons

@note           If the @b fn_callback is NULL, it means unregistered the callback
                function to the handler
*/

AUI_RTN_CODE aui_key_callback_register (

    aui_hdl handle,

    aui_key_callback fn_callback

    );

/**
@brief          Function used for getting the key attribute information when
                the key is pressed/released, as defined in the struct
                #aui_key_info

@param[in]      handle                     = #aui_hdl pointer to the handle of
                                             the Input Module already opened
                                             and to be managed to get the key
                                             attribute information

@param[out]     *p_key_info                = Pointer to the key attribute
                                             information, as defined in the
                                             struct #aui_key_info

@return         @b AUI_RTN_SUCCESS         = Getting of key attribute information
                                             performed successfully
@return         @b AUI_RTN_EINVAL          = Either one or both of the parameter
                                             (i.e. [in], [out]) are
                                             - Either NULL
                                             - Or invalid
                                             - Or the key device status is not
                                               @b AUI_KEY_DEV_OPEN
                                             - Or the key value is invalid

@return         @b Other_Values            = Getting the key attribute
                                             information failed for some reasons
*/
AUI_RTN_CODE aui_key_key_get (

    aui_hdl handle,

    aui_key_info *p_key_info

    );

/**
@brief          Function used to set the IR Remote Control Working Mode

@param[in]      ir_endian                  = Desired IR Remote Control Working
                                             Mode to be set

@return         @b AUI_RTN_SUCCESS         = Setting of the desired IR Remote
                                             Control Working Mode performed
                                             successfully
@return         @b Other_Values            = Setting of the desired IR Remote
                                             Control Working Mode failed for
                                             some reasons
*/
AUI_RTN_CODE aui_pan_ir_set_endian (

    aui_pan_ir_endian ir_endian

    );

/**
@brief          Function used to set the IR remote key map

@param[in]      handle                     = #aui_hdl pointer to the handle of
                                             the Input Module already opened
@param[in]      p_cfg                      = Pointer to the key map configuration,
                                             as per struct #aui_key_map_cfg

@return         @b AUI_RTN_SUCCESS         = Setting of the IR remote key map
                                             performed successfully
@return         @b Other_Values            = Setting of the IR remote key map
                                             failed for some reasons

@note           This function is @a only availible for project based on <b>
                Linux OS </b>
*/
AUI_RTN_CODE aui_key_set_ir_map (

    aui_hdl handle,

    aui_key_map_cfg *p_cfg

    );

/**
@brief          Function used to set the IR remote key repeat interval

@param[in]      handle                     = #aui_hdl pointer to the handle of
                                             the Input Module already opened
@param[in]      ul_delay                   = The delay (in @a millisecond (ms)
                                             unit) before repeating a keystroke
                                             - @b Caution: Default value = 600 ms
@param[in]      ul_interval                = The period (in @a millisecond (ms)
                                             unit) to repeat a keystroke.
                                             - @b Caution: Default value = 350 ms

@return         @b AUI_RTN_SUCCESS         = Setting of the IR remote key repeat
                                             interval performed successfully
@return         @b Other_Values            = Setting of the IR remote key repeat
                                             interval failed for some reasons

@note           This function is @a only availible for project based on <b>
                Linux OS </b>
*/
AUI_RTN_CODE aui_key_set_ir_rep_interval (

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

#define aui_pan_ir_protocol aui_ir_protocol

#define AUI_KEY_CALLBACK aui_key_callback

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


