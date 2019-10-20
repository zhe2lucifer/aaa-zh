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
Current ALi Author: Niker.Li
Last update:        2017.04.01
-->

@file aui_vbi.h

@brief  Vertical Blanking Interval (VBI) Module

    <b> Vertical Blanking Interval(VBI) Module </b> is used to transmit data to
    TV Encoder during blank periods, such as Teletext Data.\n
    User has to make sure the display module is ready before using this VBI module.\n
    The general flow of using VBI module is listed below:
    - Init
    - Open
    - Select output device
    - Start
    - Stop
    - Close
    - Deinit

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly

**/

#ifndef _AUI_VBI_H

#define _AUI_VBI_H

/******************************Included Header File List******************************/

#include "aui_common.h"

/**********************************Global Macro List**********************************/

/**
Macro to specify the length of a Teletext Data line
*/
#define VBI_TTX_LINE_LENGTH     (46)

/**********************************Global Type List***********************************/

/**
Enum to specify the <b> VBI data type </b> of a VBI device
*/
typedef enum aui_vbi_data_type {

    /**
    Enum to specify the <b> teletext data </b>
    */
    AUI_VBI_DATA_TYPE_TELETEXT,

    /**
    Enum to specify the <b> closed caption data </b>
    */
    AUI_VBI_DATA_TYPE_CLOSED_CAPTION

} aui_vbi_data_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Vertical Blanking Interval (VBI) Module </b> to specify
        opening parameters of a VBI Device
        </div> @endhtmlonly

        Struct to specify opening parameters for a VBI Device
*/
typedef struct aui_st_vbi_open_param {

    /**
    Member to specify the VBI Device Index which start from the zero (0) value
    */
    unsigned long vbi_index;

    /**
    Member to specify the <b> VBI data type </b> as defined in the enum
    #aui_vbi_data_type
    */
    aui_vbi_data_type data_type;

} aui_vbi_open_param;

/**
Enum to specify the Encoding Type then the Output Device
*/
typedef enum aui_vbi_output_device {

    /**
    Value to specify the HD TV Encoder to output VBI data
    */
    AUI_VBI_OUTPUT_HD,

    /**
    Value to specify the SD TV Encoder to output VBI data
    */
    AUI_VBI_OUTPUT_SD

} aui_vbi_output_device;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Vertical Blanking Interval (VBI) Module </b> to specify
        Teletext starting parameters
        </div> @endhtmlonly

        Struct to specify the Teletext starting parameters for a VBI Device
*/
typedef struct aui_st_ttx_start_param {

    /**
    Member to specify the Teletext PID
    */
    unsigned int ttx_pid;

    /**
    Member to specify the DMX Device Index which start from zero (0) value

    @note   This Index is used to denote which DMX Device the data come from,
            and the number of DMX Device depends on hardware platform.
    */
    unsigned int dmx_index;

} aui_ttx_start_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Vertical Blanking Interval (VBI) Module </b> to
        specify the Teletext Data Format
        </div> @endhtmlonly

        Struct to specify the Teletext Data Format
*/
typedef struct aui_vbi_ttx_line {

    /**
    Member to specify the Teletext Data

    @note The lenght of the Teletext Data is <b> 46 bytes </b> where:
          1. <b> 1st byte </b> is to specify the <b> Data Unit Type </b> as below:
             - @b 0x02 = <b> EBU (European Broadcasting Union) </b> Teletext
                         non-subtitle data
             - @b 0x03 = EBU Teletext subtitle data
             - @b 0xFF = Data Unit for stuffing
          2. <b> 2nd byte </b> is to specify the <b> Data Unit Length </b>\n
             @b Caution: For EBU Teletext Data Type (i.e. @b 0x02 and @b 0x03),
                         this field shall always be set to the value @b 0x2C.
          3. The remaining <b> 44 bytes </b> are to contain the specific Teletext
             Data as specified in <b> Section 4.3 </b> of <b> ETSI EN 300 472 </b>
    */
    char vbi_data[VBI_TTX_LINE_LENGTH];

} aui_vbi_ttx_line;


/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Vertical Blanking Interval (VBI) Module </b> to
        specify the closed caption start parameters
        </div> @endhtmlonly

        Struct to specify the closed caption start parameters
*/
typedef struct aui_vbi_cc_start_param {

    /**
    Member to specify the handle of the video decoder as the data source

    @warning  The data source @a must be the video decoder, and the related
              handle <a> must not be </b> NULL.
    */
    void* cc_decv_hdl;

	/**
    Some streams contain multiple formats of 608/708 data encoded into user data.                                |
    AUI VBI will only send the selected user data over VBI.

    This member will select the user data type for CC over VBI.
    */
    aui_decv_user_data_type user_data_type;

	

} aui_vbi_cc_start_param;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C"

{

#endif

/**
@brief         Function used to initialize a VBI Device

@warning       This function
               - @a Must be called before using the VBI Device
               - Can @a only be used in pair with the function #aui_vbi_de_init
               - Cannot be called twice without calling the function
                 #aui_vbi_de_init

@param[in]     p_callback_init       = Pointer to the callback function used for
                                       the initialization proces
@param[in]     pv_param              = Parameter passed to the callback function
                                       @b p_callback_init

@return        @b AUI_RTN_SUCCESS    = Initializating of the VBI Device performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Initializing of the VBI Device failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_init (

    p_fun_cb p_callback_init,

    void *pv_param

    );

/**
@brief         Function used to release the resource allocated for a VBI Device

@warning       This function can be called after no need to use the VBI Device
               anymore, i.e. after the function #aui_vbi_close. However, this
               function is used in pair with the function #aui_vbi_init

@param[in]     p_callback_init       = Pointer to the callback function used for
                                       the de-initialization process
@param[in]     pv_param              = Parameter passed to the callback function
                                       @b p_callback_init

@return        @b AUI_RTN_SUCCESS    = De-Initializing of the VBI Device
                                       performed
                                       succesfully
@return        @b AUI_RTN_FAIL       = De-Initializing of the VBI Device
                                       failed for some reasons
*/
AUI_RTN_CODE aui_vbi_de_init (

    p_fun_cb p_callback_init,

    void *pv_param

    );

/**
@brief         Function used to open a VBI Device

@warning       This function can be called after the function #aui_vbi_init

@param[in]     p_open_param          = Opening parameters for a VBI Device, as
                                       defined in the struct #aui_st_vbi_open_param
                                       - @b Caution:  In project based on <b>
                                            Linux OS </b>, this parameter should
                                            be set.\n
                                            In project based on <b> TDS
                                            OS </b>, instead,it should be set
                                            to NULL since the VBI Device Index
                                            is reserved to ALi R&D Dept.


@param[out]    p_vbi_handle          = #aui_hdl handle of the VBI Device

@return        @b AUI_RTN_SUCCESS    = Opening of the VBI Device performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Opening of the VBI Device failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_open (

    aui_vbi_open_param *p_open_param,

    aui_hdl *p_vbi_handle

    );

/**
@brief         Function used to close a VBI Device

@param[in]     vbi_handle            = #aui_hdl handle of the VBI Device

@return        @b AUI_RTN_SUCCESS    = Closing of the VBI Device performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Closing of the VBI Device failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_close (

    aui_hdl vbi_handle

    );

/**
@brief        Function used to write a line of Teletext Data to TV Encoder

@param[in]    vbi_handle             = #aui_hdl handle of the opened VBI Device

@param[in]    *p_ttx_data            = Pointer to a struct to specify the Teletext
                                       Data Format

@return       @b AUI_RTN_SUCCESS     = Injecting of the the Teletext Data to the
                                       VBI Module performed successfully

@return       @b AUI_RTN_FAIL        = Injecting of the the Teletext Data to the
                                       VBI Module failed for some reasons
*/
AUI_RTN_CODE aui_vbi_ttx_write_line (

    aui_hdl vbi_handle,

    aui_vbi_ttx_line *p_ttx_data

    );

/**
@brief         Function used to start transmitting Teletext data to TV Encoder

@warning       Before using this function, please make sure DMX works well,
               in particular make sure the function #aui_vbi_select_output_dev
               has been called firstly.\n

@param[in]     vbi_handle            = #aui_hdl handle of the opened VBI device

@param[in]     p_start_param         = Teletext starting parameters for a VBI
                                       Device, as defined in the struct
                                       #aui_ttx_start_param

@return        @b AUI_RTN_SUCCESS    = Starting of the transmission of Teletext
                                       Data to TV Encoder performed successfully
@return        @b AUI_RTN_FAIL       = Starting of the transmission of Teletext
                                       Data to TV Encoder failed for some reasons
*/
AUI_RTN_CODE aui_vbi_ttx_start (

    aui_hdl vbi_handle,

    aui_ttx_start_param *p_start_param

    );

/**
@brief         Function used to stop transmitting Teletext Data to TV Encoder

@warning       This function can be called after the function #aui_vbi_ttx_start

@param[in]     vbi_handle            = #aui_hdl handle of the VBI Device

@return        @b AUI_RTN_SUCCESS    = Stopping of the transmission of Teletext
                                       Data to TV Encoder performed successfully
@return        @b AUI_RTN_FAIL       = Stopping of the transmission of Teletext
                                       Data to TV Encoder failed for some reasons
*/
AUI_RTN_CODE aui_vbi_ttx_stop (

    aui_hdl vbi_handle

    );

/**
@brief         Function used to select the output device for transmitting
               Teletext Data to TV Encoder

@warning       This function @a must be called before the function #aui_vbi_ttx_start

@param[in]     vbi_handle            = #aui_hdl handle of the VBI Device

@param[in]     output_device         = Encoding Type then Ouput Device, as defined
                                       in the enum #aui_vbi_output_device

@return        @b AUI_RTN_SUCCESS    = Selecting of the ouput device performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Selecting of the output device failed for
                                       some reasons

@note          VBI uses CVBS interface to output data. With reference to the enum
               #aui_vbi_output_device, in <b> SD Mode </b> the input parameter
               @b output_device takes the value #VBI_OUTPUT_SD (currently only
               the SD Mode is supported)
*/
AUI_RTN_CODE aui_vbi_select_output_dev (

    aui_hdl vbi_handle,

    aui_vbi_output_device output_device

    );

/**
@brief         Function used to start closed caption

@warning       Make sure that VBI device is open before using this function

@param[in]     vbi_handle            = #aui_hdl handle of the VBI Device
@param[in]     p_start_param         = Pointer to closed caption start parameters,
                                       as defined in the struct #aui_vbi_cc_start_param

@return        @b AUI_RTN_SUCCESS    = Starting of closed caption performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Starting of closed caption failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_cc_start (

    aui_hdl vbi_handle,

    aui_vbi_cc_start_param *p_start_param

    );

/**
 @brief        Function used to stop closed caption

@warning       Make sure that VBI device is open before using this function

@param[in]     vbi_handle            = #aui_hdl handle of the VBI Device

@return        @b AUI_RTN_SUCCESS    = Stopping of closed caption performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Stopping of closed caption failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_cc_stop (

    aui_hdl vbi_handle

    );

/**
@brief         Function used to set video output aspect ratio.

@warning       This function can be called after the function #aui_vbi_ttx_start

@warning       This function is reserved to ALi R&D Dept. then user can ignore it

@param[in]     aspect_ratio          = Aspect ratio

@return        @b AUI_RTN_SUCCESS    = Setting of the aspect ratio performed
                                       successfully
@return        @b AUI_RTN_FAIL       = Setting of the aspect ratio failed for
                                       some reasons
*/
AUI_RTN_CODE aui_vbi_set_wss_aspect_ratio (

    unsigned int aspect_ratio

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define VBI_OUTPUT_SD AUI_VBI_OUTPUT_SD

#define VBI_OUTPUT_HD AUI_VBI_OUTPUT_HD

#define aui_vbi_en_output_device aui_vbi_output_device

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


