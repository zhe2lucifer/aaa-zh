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
Current ALi Author: Niker.Li, Fawn.Fu
Last update:        2017.03.02
-->

@file       aui_hdmi.h

@brief      High-Definition Multimedia Interface (HDMI) Module

            <b> High-Definition Multimedia Interface (HDMI) Module </b> is used
            to output <b> compressed </b> or <b> uncompressed  audio </b> and
            <b> video </b> data to HDMI-Compliant sink device.\n
            Main functions include:
            - Control audio/video output data to HDMI-compliant sink device
              such as:
              - Enable/Disable HDMI Audio
              - Set AV mute
              - Set colour depth
              - etc.
            - Get the capacity of HDMI-compliant sink device such as:
              - Audio resolution
              - Video resolution
              - Whether support HDCP (High-Bandwidth Digital Content Protection)
                or not
            - HDCP control function such as:
              - Enable/Disable/Set HDCP key
              - Get HDCP Authentication status
            - Send CEC (Consumer Electronics Control) message to control
              HDMI-compliant sink device.

@note For further details, please refer to the ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "HDMI Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_HDMI_H

#define _AUI_HDMI_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number of HDMI Module </b>, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_HDMI (0X00010000)

/**
@warning  This macro is no longer supported then is @a deprecated
*/
#define AUI_MAX_HDMI_CALLBACKFUNC 16

/**
@warning  This macro is no longer supported then is @a deprecated
*/
#define AUI_HDMI_NAME_LEN 32

/**
Macro to indicate that all <b> Extended Display Identification Data (EDID) </b> h
as to be @a read
*/
#define AUI_HDMI_RAW_EDID_ALL 0XFFFF

/*******************************Global Type List*******************************/

/**
@brief  @htmlonly <div class="details">
        Struct of the  <b> High-Definition Multimedia Interface (HDMI) Module
        </b> <em> deprecated </em> then user can ignore it
        </div> @endhtmlonly

@warning    This struct is no longer supported then is @a deprecated
*/
typedef struct aui_hdmi_callback {

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    void * fnCallback;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    void * pvUserData;

} aui_hdmi_callback, *aui_p_hdmi_callback;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module </b>
        to specify the attributes available to be configured for HDMI Module
        </div> @endhtmlonly

        Struct to specifies the attributes to be configured for HDMI Module
*/
typedef struct aui_st_attr_hdmi {

    /**
    Member as @a Index which integer values (from the value <b> zero (0) </b>)
    refer to different HDMI Devices
    */
    unsigned char uc_dev_idx;

} aui_attr_hdmi, *aui_p_attr_hdmi;

/**
Enum to specify the different <b> HDMI Color Space Format </b>
*/
typedef enum aui_hdmi_color_space {

    /**
    Value to specify the <b> YCbCr_422 </b> Color Space Format,
    i.e. <b> YYYYCbCrCbCr </b>
    */
    AUI_HDMI_YCBCR_422,

    /**
    Value to specify the <b> YCbCr_444 </b> Color Space Format,
    i.e. <b> YYYYCbCrCbCrCbCrCbCr </b>
    */
    AUI_HDMI_YCBCR_444,

    /**
    Value to specify the <b> RGB MODE1 </b> Format,
    i.e. the range of the RGB Color Format is <b> [0;255] </b>
    */
    AUI_HDMI_RGB_MODE1,

    /**
    Value to specify the <b> RGB MODE2 </b> Format,
    i.e. the range of the RGB Color Format is @a limited at <b> [16;235] </b>

    @warning  This value is no longer supported then user can ignore it.\n
              Currently, HDMI driver @a only support the full range [0;255].
    */
    AUI_HDMI_RGB_MODE2,

    /**
    Value to specify the <b> YCbCr_420 </b> Color Space Format,
    i.e. <b> YYYYCbCb </b>

    @note This value is extended for <b> 861-F </b>, and is @a only avalaible
          on HDMI2.0
    */
    AUI_HDMI_YCBCR_420

} aui_hdmi_color_space;

/**
Enum to specify the different <b> HDMI Video Resolution </b> supported by
HDMI Module

@note The @b "I" and @b "P" letter present in the descriptions stand for
      as follows:
      - @b I =  Interlaced Scanning
      - @b P =  Progressive Scanning
*/
typedef enum aui_hdmi_res {

    /**
    Value to specify an <b> Invalid </b> HDMI Video Resolution.
    */
    AUI_HDMI_RES_INVALID = 0,

    /**
    Value to specify the <b> 480I </b> HDMI Video Resolution with <b> 60 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_480I,

    /**
    Value to specify the <b> 480P </b> HDMI Video Resolution with <b> 60 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_480P,

    /**
    Value to specify the <b> 576I </b> HDMI Video Resolution with <b> 50 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_576I ,

    /**
    Value to specify the <b> 576P </b> HDMI Video Resolution with <b> 50 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_576P,

    /**
    Value to specify the <b> 720P </b> HDMI Video Resolution with <b> 50 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_720P_50,

    /**
    Value to specify the <b> 720P </b> HDMI Video Resolution with <b> 60 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_720P_60,

    /**
    Value to specify the <b> 1080I/25 </b> HDMI Video Resolution, i.e. with
    <b> 25 fps </b> (frames per second.

    @note It is also known as <b> 1080I 50Hz </b>, i.e. with 50 fields per second
    */
    AUI_HDMI_RES_1080I_25,

    /**
    Value to specify the <b> 1080I/30 </b> HDMI Video Resolution, i.e. with
    <b> 30 fps </b> (frames per second).

    @note It's also known as <b> 1080I 60Hz </b>, i.e. with 60 fields per second
    */
    AUI_HDMI_RES_1080I_30,

    /**
    Value to specify the <b> 1080P </b> HDMI Video Resolution with <b> 24 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_1080P_24,

    /**
    Value to specify the <b> 1080P </b> HDMI Video Resolution with <b> 25 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_1080P_25,

    /**
    Value to specify the <b> 1080P </b> HDMI Video Resolution with <b> 30 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_1080P_30,

    /**
    Value to specify the <b> 1080P </b> HDMI Video Resolution  with <b> 50 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_1080P_50,

    /**
    Value to specify the <b> 1080P </b> HDMI Video Resolution with <b> 60 fps </b>
    (frames per second)
    */
    AUI_HDMI_RES_1080P_60,

    /**
    Value to specify the <b> 2160P - 4096x2160 </b> HDMI Video Resolution with
    <b> 24 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode
    */
    AUI_HDMI_RES_4096X2160_24,

    /**
    Value to specify the <b> 2160P - 3840x2160 </b> HDMI Video Resolution with
    <b> 24 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode
    */
    AUI_HDMI_RES_3840X2160_24,

    /**
    Value to specify the <b> 2160P - 3840x2160 </b> HDMI Video Resolution with
    <b> 25 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode
    */
    AUI_HDMI_RES_3840X2160_25,

    /**
    Value to specify the <b> 2160P 3840x2160 </b> HDMI Video Resolution with
    <b> 30 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode
    */
    AUI_HDMI_RES_3840X2160_30,

    /**
    Value to specify the <b> 2160P - 3840x2160 </b> HDMI Video Resolution with
    <b> 50 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_3840X2160_50,

    /**
    Value to specify the <b> 2160P - 3840x2160 </b> HDMI Video Resolution with
    <b> 60 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_3840X2160_60,

    /**
    Value to specify the <b> 2160P - 4096x2160 </b> HDMI Video Resolution with
    <b> 25 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_4096X2160_25,

    /**
    Value to specify the <b> 2160P - 4096x2160 </b> HDMI Video Resolution with
    <b> 30 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_4096X2160_30,

    /**
    Value to specify the <b> 2160P - 4096x2160 </b> HDMI Video Resolution with
    <b> 50 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_4096X2160_50,

    /**
    Value to specify the <b> 2160P - 4096x2160 </b> HDMI Video Resolution with
    <b> 60 fps </b> (frames per second)

    @Warning  @a Only the progressive scan mode

    @note Extend for 861-F
    */
    AUI_HDMI_RES_4096X2160_60,

    /**
    Value to specify the <b> total number </b> of HDMI Video Resolution supported
    */
    AUI_HDMI_RES_NUM

} aui_hdmi_res;

// @coding

/**
Enum to specify the different <b> HDMI Audio Coding </b> supported by the H
DMI Module
*/
typedef enum aui_hdmi_audio_fmt {

    /**
    Value to specify an <b> Invalid </b> HDMI Audio Coding
    */
    AUI_EDID_AUDIO_INVALID = 0,

    /**
    Value to specify the <b> LPCM </b> Audio Coding
    */
    AUI_EDID_AUDIO_LPCM,

    /**
    Value to specify the <b> AC3 </b> Audio Coding
    */
    AUI_EDID_AUDIO_AC3,

    /**
    Value to specify the <b> MPEG1 </b> Audio Coding
    */
    AUI_EDID_AUDIO_MPEG1,

    /**
    Value to specify the <b> MPEG3 </b> Audio Coding
    */
    AUI_EDID_AUDIO_MP3,

    /**
    Value to specify the <b> MPEG2 </b> Audio Coding
    */
    AUI_EDID_AUDIO_MPEG2,

    /**
    Value to specify the <b> AAC </b> Audio Coding
    */
    AUI_EDID_AUDIO_AAC,

    /**
    Value to specify the <b> DTS </b> Audio Coding
    */
    AUI_EDID_AUDIO_DTS,

    /**
    Value to specify the <b> ATRAC </b> Audio Coding
    */
    AUI_EDID_AUDIO_ATRAC,

    /**
    Value to specify the <b> One-bit </b> Audio Coding
    */
    AUI_EDID_AUDIO_ONEBITAUDIO,

    /**
    Value to specify the <b> DD-Plus </b> Audio Coding
    */
    AUI_EDID_AUDIO_DD_PLUS,

    /**
    Value to specify the <b> DTS-HD </b> Audio Coding
    */
    AUI_EDID_AUDIO_DTS_HD,

    /**
    Value to specify the <b> MAT-MLP </b> Audio Coding
    */
    AUI_EDID_AUDIO_MAT_MLP,

    /**
    Value to specify the <b> DST </b> Audio Coding
    */
    AUI_EDID_AUDIO_DST,

    /**
    Value to specify the <b> Microsoft WMA Pro </b> Audio Coding
    */
    AUI_EDID_AUDIO_BYE1PRO,

    /**
    Value to specify the <b> total number </b> of Audio Coding available
    */
    AUI_EDID_AUDIO_NUM

} aui_hdmi_audio_fmt;

// @endcoding

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module
        </b> to specify the EDID (Extended Display Identification Data) Short
        Video Descriptor
        </div> @endhtmlonly

        Struct to specify the <b> EDID (Extended Display Identification Data)
        Short Video Descriptor </b>
*/
typedef struct aui_short_video_desc {

    /**
    Member as a @a flag to specify the whether <b> HDMI Video Resolution </b>
    if @b native or @b non-native, in particular
    - @b 1 = Native
    - @b 0 = Non-native
    */
    unsigned char native_indicator;

    /**
    Member to specify the <b> HDMI Video Resolution ID </b>
    */
    unsigned char video_id_code;

    /**
    Member as the pointer to the next EDID Short Video Descriptor
    */
    struct aui_short_video_desc * next;

} aui_short_video_desc;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module
        </b> to specify the EDID (Extended Display Identification Data) Short
        Audio Descriptor </div> @endhtmlonly

        Struct to specify the <b> EDID (Extended Display Identification Data)
        Short Audio Descriptor </b>.
*/
typedef struct aui_short_audio_desc {

    /**
    Member to specify the <b> HDMI Audio Coding </b> as defined in the enum
    #aui_hdmi_audio_fmt
    */
    unsigned short audio_format_code;

    /**
    Member to specify the <b> Maximum Number of HDMI Audio Channel </b>
    supported
    */
    unsigned char max_num_audio_channels;

    /**
    Member to specify the <b> HDMI Audio Sample Rate </b>
    (in @a HZ unit)
    */
    unsigned char audio_sampling_rate;

    /**
    Member to specify the <b> Maximum HDMI Audio Bit Rate </b>
    (in @a bps (bit per second) unit) supported
    */
    unsigned short max_audio_bit_rate;

    /**
    Member as the pointer to the next EDID Short Audio Descriptor
    */
    struct aui_short_audio_desc *next;

} aui_short_audio_desc;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module
        </b> to specify the Capabilities of HDMI Sink Device
        </div> @endhtmlonly

        Struct to specify the <b> Capabilities of HDMI Sink Device </b>, which
        originate from EDID Information
*/
typedef struct aui_hdmi_sink_capability {

    /**
    Member to specify the <b> desired HDMI Video Resolution </b> as defined
    in the enum #aui_hdmi_res.
    */
    aui_hdmi_res hdmi_prefer_video_resolution;

    /**
    Member to specify an array of <b> all supported HDMI Video Resolutions
    </b> as defined in the enum #aui_hdmi_res.
    */
    aui_hdmi_res hdmi_supported_video_mode[AUI_HDMI_RES_NUM];

    /**
    Member to specify the <b> desired HDMI Audio Coding </b>.as defined in the
    enum #aui_hdmi_audio_fmt
    */
    aui_hdmi_audio_fmt hdmi_prefer_audio_mode;

    /**
    Member to specify an array of <b> all supported HDMI AUdio Coding </b> as
    defined in the enum #aui_hdmi_audio_fmt.
    */
    aui_hdmi_audio_fmt hdmi_supported_audio_mode[AUI_EDID_AUDIO_NUM];

    /**
    Member as <b> HDCP flag </b> to specify whether the TV supports HDCP or not,
    in particular
    - @b 1 = Supported
    - @b 0 = Not supported.
    */
    unsigned char   hdmi_hdcp_support;

} aui_hdmi_sink_capability;

/**
Enum to specify the <b> HDMI Hot-Plug Event </b>

@warning  This enum is no longer supported then is @a deprecated
*/
typedef enum aui_hdmi_event {

    /**
    @warning  This value is no longer supported then is @a deprecated
    */
    AUI_HDMI_PLUGIN,

    /**
    @warning  This value is no longer supported then is @a deprecated
    */
    AUI_HDMI_PLUGOUT

} aui_hdmi_event;

/**
Enum to specify the <b> HDMI Connection Status </b>
*/
typedef enum aui_hdmi_link_status {

    /**
    Value to specify that HDMI is <b> not connected </b> to the HDMI sink device
    */
    AUI_HDMI_STATUS_UNLINK = 0x01,

    /**
    Value to specify that HDMI is <b> connected </b> to the HDMI sink device
    */
    AUI_HDMI_STATUS_LINK = 0x20,

    /**
    Value to specify that HDMI is <b> connected </b> to the HDMI sink device
    and the <b> HDCP Authentication </b> has been performed @b successfully
    */
    AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED = 0x02,

    /**
    Value to specify that HDMI is <b> connected </b> to the HDMI sink device
    and the <b> HDCP Authentication failed </b> for some reasons
    */
    AUI_HDMI_STATUS_LINK_HDCP_FAILED = 0x04,

    /**
    Value to specify that HDMI is <b> connected </b> to the HDMI sink device
    and the <b> HDCP Authentication </b> has been @b ignored
    */
    AUI_HDMI_STATUS_LINK_HDCP_IGNORED = 0x08,

    /**
    Value to specify the <b> total number </b> of <b> HDMI Connections </b>
    */
    AUI_HDMI_STATUS_MAX = 0x10

} aui_hdmi_link_status;

/**
Enum to specify the <b> HDMI Property </b>
*/
typedef enum aui_hdmi_property_item {

    /**
    Value to specify the <b> HDMI Vendor Name </b>.
    */
    AUI_HDMI_VENDOR_NAME,

    /**
    Value to specify the <b> HDMI Product Description </b>.
    */
    AUI_HDMI_PRODUCT_DESCRIPTION,

    /**
    @warning  This value is no longer supported then is @a deprecated
    */
    AUI_HDMI_HDCP_KEY,

    /**
    @warning  This value is no longer supported then is @a deprecated
    */
    AUI_HDMI_LINK_STATUS

} aui_hdmi_property_item;


/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module
        </b> to specify the HDCP Key Information
        </div> @endhtmlonly

        Struct to specify the <b> HDCP Key Information </b>

@warning  This struct is no longer supported then is @a deprecated

@note   @a Only when HDCP authentication is passed, the HDMI sink device (TV)
        can show Video and Output Audio from the source HDMI device (STB)
*/
typedef struct aui_hdmi_hdcp_key_info {

    /**
    Member to specify the <b> HDCP KSV (Key Select Vector) </b>
    */
    void    *puc_hdcp_ksv;

    /**
    Member to specify the <b> HDCP Keys </b>.
    */
    void    *puc_encrypted_hdcp_keys;

    /**
    Member to specify that the <b> length of HDCP KSV </b> is <b> 8 bits </b>
    */
    int     n_ksv_length;

    /**
    Member to specify that the <b> length of HDCP KSV </b> is <b> 312 bits </b>
    */
    int     n_hdcp_keys_length;

} aui_hdmi_hdcp_key_info;

/**
Enum to specify the different callback types for all possible HDMI events\n
*/
typedef enum aui_hdmi_cb_type {

    /**
    This callback type is for the the event <b> HDMI EDID Ready </b>
    */
    AUI_HDMI_CB_EDID_READY = 0,

    /**
    This callback type is for the event <b> HDMI Plug-Out </b>
    */
    AUI_HDMI_CB_HOT_PLUG_OUT,

    /**
    This callback type is for the event <b> HDMI Debug Message </b>
    */
    AUI_HDMI_CB_DBG_MSG,

    /**
    This callback type is for the event <b> HDMI Plug-In </b>
    */
    AUI_HDMI_CB_HOT_PLUG_IN,

    /**
    This callback type is for the event <b> HDMI Receive CEC Message </b>
    */
    AUI_HDMI_CB_CEC_MESSAGE,

    /**
    This callback type is for the event <b> HDCP Failed </b>
    */
    AUI_HDMI_CB_HDCP_FAIL

} aui_hdmi_cb_type;

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDMI EDID Ready </b> as
defined in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_edid_ready_cb) (

    void

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDMI Plug-Out </b> as
defined in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_plug_out_cb) (

    void

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDMI Debug Message </b> as
defined in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_dbg_msg_cb) (

    unsigned char uc_param1,

    unsigned char *puc_param2

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDMI Plug-In </b> as defined
in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_plug_in_cb) (

    void

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDMI Receive CEC Message </b>
as defined in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_cec_msg_cb) (

    unsigned char *puc_param1,

    unsigned char uc_param2

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_hdmi_callback_reg for the event <b> HDCP Failed </b> as defined
in the enum #aui_hdmi_cb_type
*/
typedef void (*aui_hdmi_hdcp_fail_cb) (

    unsigned char *puc_param1,

    unsigned char uc_param2

    );

/**
Enum used to control <b> HDMI Audio Bitstream </b> output mode
*/
typedef enum aui_hdmi_audio_bs_output_set {

    /**
    Value to specify that the <b> HDMI Bitstream Audio Output </b> is <b>
    controlled by driver </b>
    */
    AUI_HDMI_AUD_BS_AUTO = 0,

    /**
    Value to specify that the <b> HDMI Bitstream Audio Output </b> is <b>
    controlled manually </b>
    */
    AUI_HDMI_AUD_BS_MANUAL

} aui_hdmi_audio_bs_output_set, aui_hdmi_audio_bs_output_set_e;

/**
Enum to specify the <b> Type of the HDMI Device </b>
*/
typedef enum aui_hdmi_type {

    /**
    Value to specify the <b> DVI Device </b>.
    */
    AUI_HDMI_TYPE_DVI = 0,

    /**
    Value to specify the <b> HDMI Device </b>
    */
    AUI_HDMI_TYPE_HDMI

} aui_hdmi_type, aui_hdmi_type_e;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module
        </b> to specify the HDMI EDID Video Format information
        </div> @endhtmlonly

        Struct to specify the <b> HDMI EDID Video Format </b> informations
*/
typedef struct aui_hdmi_edid_video_format {

    /**
    Member to specify the <b> Horizontal Active Pixels </b>
    */
    unsigned int video_format;

    /**
    Member to specify te <b> Refresh Rate </b> (in @a second unit)
    */
    unsigned int field_rate;

    /**
    Member to specify the <b> Image Aspect Ratio </b>
    */
    unsigned int aspect_ratio;

} aui_hdmi_edid_video_format, aui_hdmi_edid_video_format_e;

// @coding

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module </b>
        to specify the CEA Short Descriptor from EDID
        </div> @endhtmlonly

        Struct to specify the <b> CEA Short Descriptor from EDID </b>
*/
typedef struct aui_short_cea_desc {

    /**
    Member to specify the 8-bit CEA data.\n\n

    @b Example: For <b> HDMI Video Description Data </b>
    - The bit 7 is the most significant bit to indicates whether the HDMI Video
      resolution is native or not
    - The bit 6 -> bit 0 indicate the HDMI Video Resolution
    */
    unsigned char cea_data;

    /**
    Member as the pointer to the next CEA Short Descriptor from EDID
    */
    struct aui_short_cea_desc *next;

} aui_short_cea_desc;

// @endcoding

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module </b>
        to specify the HDMI 3D Descriptor from EDID
        </div> @endhtmlonly

        Struct to specify the <b> HDMI 3D Descriptor from EDID </b>
*/
typedef struct aui_hdmi_3d_descriptor {

    /**
    Member to specify that he <b> Multiple 3D </b> is supported by HDMI sink device
    */
    unsigned char hdmi_3d_multi_present;

    /**
    Member to specify the width of the interval of the used Video ID Codes <b>
    [HDMI_VIC_1; HDMI_VIC_M] </b> where <b> M &ge; 1 </b>
    */
    unsigned char hdmi_vic_len;

    /**
    Member to specify zero or more bytes for information about 3D formats supported
    */
    unsigned char hdmi_3d_len;

    /**
    Member to specify the <b> CEA Short Descriptor from EDID </b> as defined in
    the struct #aui_short_cea_desc
    */
    aui_short_cea_desc    *short_hdim_vic_desc;

    /**
    Member to specify the <b> CEA Short Descriptor from EDID </b> as defined in
    the struct #aui_short_cea_desc
    */
    aui_short_cea_desc    *short_3d_desc;

} aui_hdmi_3d_descriptor, aui_hdmi_3d_descriptor_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> High-Definition Multimedia Interface (HDMI) Module </b>
        to specify the HDMI EDID Information
        </div> @endhtmlonly

        Struct to specify the <b> HDMI EDID Information </b>

@warning  This struct is no longer supported then is @a deprecated
*/
typedef struct aui_hdmi_edid_info {

  /**
  Member to specify the <b> Vendor ID </b>
  */
  unsigned short  vendor_id;

  /**
  Member to specify the <b> Product ID </b> assigned by the vendor
  */
  unsigned short  product_id;

  /**
  Member to specify the <b> Serial number </b> assigned by the vendor.

  @note  This value may be undefined
  */
  unsigned int  serial_num;

  /**
  Member to specify the <b> Monitor name </b>
  */
  unsigned char  monitor_name[14];

  /**
  Member to specify the <b> manufacturing week </b>
  */
  unsigned short   manuf_week;

  /**
  Member to specify the <b> manufacturing year </b>
  */
  unsigned char manuf_year;

  /**
  Member to specify the <b> EDID Version number </b>
  */
  unsigned short  edid_version;

  /**
  Member to specify the <b> EDID Revision number </b>
  */
  unsigned short  edid_revision;

  /**
  Member to specify the <b> ID of the First Video Format </b>
  */
  unsigned int first_video_id;

  /**
  Member to specify the <b> Peer Device Video Format </b>
  */
  aui_hdmi_edid_video_format preferred_video_format;

} aui_hdmi_edid_info, aui_hdmi_edid_info_e;

/**
Enum to specify miscellaneous information available to be gotten from the HDMI
Device.\n

@note This enum is used by the function #aui_hdmi_get where
      - The parameter @b pv_param_out takes the pointer as per the description of the
        specific information to get.
      - The parameter @b pv_param_in takes the value NULL as default, unless otherwise
        noted as per the description of the specific information to get
*/
typedef enum aui_en_hdmi_item_get {

    /**
    Value to specify the <b> HDMI Connection Status </b>.

    @note For this value, @b pv_param_out should point to the enum
          #aui_hdmi_link_status
    */
    AUI_HDMI_CONNECT_INFO_GET=0,

    /**
    Value to specify the <b> Current HDMI Type </b>.

    @note  For this value, @b pv_param_out should point to the enum
           #aui_hdmi_type.
    */
    AUI_HDMI_TYPE_GET,

    /**
    Value to specify the <b> Current HDCP Verification Result </b>

    @note  For this value, @b pv_param_out should point to the enum
           #aui_hdmi_link_status
    */
    AUI_HDMI_HDCP_STATUS_GET,

    /**
    Value to specify the <b> Desired HDMI Video Format </b>.

    @note  For this value, @b pv_param_out should point to the enum
           #aui_hdmi_edid_video_format
    */
    AUI_HDMI_PREFER_VIDEO_FMT_GET,

    /**
    Value to specify the <b> HDMI Logical Address </b>

    @note  For this value, @b pv_param_out should point to the the enum
           #aui_hdmi_cec_logic_addr
    */
    AUI_HDMI_LOGICAL_ADDR_GET,

    /**
    Value to specify the <b> HDMI Physical Address </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_PHYSICAL_ADDR_GET,

    /**
    Value to specify the <b> HDMI Video CEA Data Block Number </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_VIC_NUM_GET,

    /**
    Value to specify the <b> HDMI Audio CEA Data Block Number </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_AUD_NUM_GET,

    /**
    Value to specify the <b> HDMI CEA Short Video Descriptor </b>

    @note  For this value
           - @b pv_param_in should point to an integer variable representing
             the Short Video Descriptor ID
           - @b pv_param_out should point to the struct #aui_short_video_desc
    */
    AUI_HDMI_VIDEO_FMT_ID_GET,

    /**
    Value to specify the <b> HDMI CEA Short Audio Descriptor </b>

    @note  For this value
           - @b pv_param_in should point to an integer variable representing
             the Short Audio Descriptor ID
           - @b pv_param_out should point to the struct #aui_short_audio_des
    */
    AUI_HDMI_AUDIO_FMT_ID_GET,

    /**
    @warning  This value is no longer supported then is @a deprecated, please
              use the value #AUI_HDMI_EDID_FIRSTVID_GET instead.
    */
    AUI_HDMI_FIRST_VIDEO_ID_GET,

    /**
    Value to specify the <b> HDMI EDID CEA 3D Descriptor </b>

    @note  @b pv_param_out should point to the struct #aui_hdmi_3d_descriptor
    */
    AUI_HDMI_3D_DESC_GET,

    /**
    Value to specify the <b> EDID Manufacturer Name </b>

    @note @b pv_param_out should point to an unsigned short array with length
          greater-than or equal to four (4) (e.g. <b> manuf_name[len],
          len &ge; 4 </b>)
    */
    AUI_HDMI_EDID_MANNAME_GET,

    /**
    Value to specify the <b> EDID Manufacturer Product Code </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_EDID_PROID_GET,

    /**
    Value to specify the <b> EDID Serial Number </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_EDID_SENUM_GET,

    /**
    Value to specify the <b> EDID Monitor Name </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           array which length is greater-than or equal to thirteen (13) (e.g.
           <b> monitor_name[len], len &ge; 13 </b>)
    */
    AUI_HDMI_EDID_MONNAME_GET,

    /**
    Value to specify the <b> EDID Manufacturing Week </b>

    @note  For this value, @b pv_param_out should point to an unsigned char
           variable
    */
    AUI_HDMI_EDID_MANWEEK_GET,

    /**
    Value to specify the <b> EDID Manufacturing Year </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_EDID_MANYEAR_GET,

    /**
    Value to specify the <b> EDID Version </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_EDID_VERSION_GET,

    /**
    Value to specify the <b> EDID Revision </b>

    @note  For this value, @b pv_param_out should point to an unsigned short
           integer variable
    */
    AUI_HDMI_EDID_REVISION_GET,

    /**
    Value to specify the <b> EDID CEA First Short Video Descriptor </b>

    @note  For this value, @b pv_param_out should point to the enum
           #aui_short_video_desc
    */
    AUI_HDMI_EDID_FIRSTVID_GET,

    /**
    Value to specify the <b> ALL Raw EDID Data Length </b>

    @note  For this value, @b pv_param_out should point to an unsigned integer
           variable
    */
    AUI_HDMI_EDID_LEN_GET,

    /**
    Value to specify the <b> Preferred Video Resolution </b>

    @note  For this value, @b pv_param_out should point to the enum #aui_hdmi_res
    */
    AUI_HDMI_PREFER_VIDEO_RES_GET,

    /**
    Value to specify the <b> All Supported Video Resolution </b>

    @note  For this value, @b pv_param_out should point to an array of the enum
           #aui_hdmi_res, and the length of that array should be greater-than or
           equal to #AUI_HDMI_RES_NUM defined in the same enum
           (e.g. @b hdmi_supported_video_mode[#AUI_HDMI_RES_NUM])
    */
    AUI_HDMI_ALL_VIDEO_RES_GET,

    /**
    Value to specify the <b> Audio Status </b>

    @note  For this value, @b pv_param_out should point to an unsigned integer
           variable as a @a flag which value are
           - @b 1 = Audio ON
           - @b 0 = Audio Off
    */
    AUI_HDMI_AUDIO_STATUS_GET,

    /**
    Value to specify the <b> HDCP Status </b>

    @note  For this value, @b pv_param_out should point to an unsigned integer
           variable as a @a flag which values are
           - @b 1 = HDCP Authentication performed successfully
           - @b 0 = HDCP Authentication failed
    */
    AUI_HDMI_HDCP_ENABLE_GET,

    /**
    Value to specify the <b> HDMI Status </b>

    @note  For this value, @b pv_param_out should point to an unsigned integer
           variable as a @a flag which value are
           - @b 1 = HDMI On
           - @b 0 = HDMI Off.
    */
    AUI_HDMI_STATUS_GET,

    /**
    Value to specify the <b> EDID Vendor Name </b>

    @note  For this value, @b pv_param_out should point to an unsigned char
           array with length signed char which length is greater-than or equal
           to eight (8) (e.g. <b> vendor_name[len], len &ge; 8 </b>)
      */
    AUI_HDMI_EDID_VENNAME_GET,

    /**
    Value to specify the <b> EDID product description </b>

    @note  For this value, @b pv_param_out should point to an unsigned char
           array which length is greater-than or equal to sixteen (16)
           (e.g. <b> product_desc[len], len &ge; 16 </b>)
    */
    AUI_HDMI_EDID_PRONAME_GET,

    /**
    Value to specify the <b> HDMI Video Color Space Format </b>

    @note  For this value, @b pv_param_out should point to the enum
           #aui_hdmi_color_space
    */
    AUI_HDMI_IOCT_GET_COLOR_SPACE,

    /**
    Value to specify the current <b> HDMI Video Deep Color Format </b>
    of the connected HDMI TV set

    @note  For this value, @b pv_param_out should point to the enum
           #aui_hdmi_deepcolor
    */
    AUI_HDMI_IOCT_GET_DEEP_COLOR,

    /**
    Value to specify <b> all </b> of the <b> HDMI Video EDID Deep Color Formats </b>
    supported by the connected HDMI TV set

    @note  For this value, @b pv_param_out should point to the bitwise OR value of
           the enum #aui_hdmi_deepcolor
    */
    AUI_HDMI_IOCT_GET_EDID_DEEP_COLOR,

    /**
    Value to specify the <b> AV Blank Status </b>.

    @note  For this value, @b pv_param_out should point to an unsigned
           integer variable as a @a flag which value are
           - @b 1 = AV Blank Enable
           - @b 0 = AV Blank Disable
    */
    AUI_HDMI_IOCT_GET_AV_BLANK,

    /**
    Value to specify the total number of items available in this enum
    #aui_hdmi_item_get
    */
    AUI_HDMI_GET_CMD_LAST

} aui_hdmi_item_get;

/**
Enum to specify miscellaneous setting for the HDMI Device

@note  This enum is used by the function #aui_hdmi_set where
       - The parameter @b pv_param_in should point to a variable or set as NULL
       - The parameter @b pv_param_out takes the value NULL as default, unless
         otherwise noted as per the description of the specific information to set
*/
typedef enum aui_en_hdmi_item_set {

    /**
    Value to specify the setting <b> HDMI AV Mute </b>
    */
    AUI_HDMI_AV_MUTE_SET=0,

    /**
    Value to specify the setting <b> HDMI AV Un-Mute </b>.
    */
    AUI_HDMI_AV_UNMUTE_SET,

    /**
    @warning  This value is no longer supported then is @a deprecated
    */
    AUI_HDMI_AUD_BS_OUTPUT_CTRL_SET,

    /**
    Value to specify the setting <b> HDMI Video Color Space </b>

    @note  For this value, @b pv_param_in should point to the enum
           #aui_hdmi_color_space
    */
    AUI_HDMI_IOCT_SET_COLOR_SPACE,

    /**
    Value to specify the setting <b> HDMI Video Deep Color </b>

    @note  For this value, @b pv_param_in should point to the enum
           #aui_hdmi_deepcolor
    */
    AUI_HDMI_IOCT_SET_DEEP_COLOR,

    /**
    Value to specify the setting <b> HDMI AV Blank </b>

    @note  For this value, @b pv_param_in should point to an integer variable
           as a @a flag which values are
           - @b 1 = Blank
           - @b 0 = Un-Blank
    */
    AUI_HDMI_IOCT_SET_AV_BLANK,

    /**
    Value to specify the total number of items available in this enum
    */
    AUI_HDMI_SET_CMD_LAST

} aui_hdmi_item_set;

/**
Enum to specify the <b> HDMI CEC Logical Address </b> for different devices
*/
typedef enum aui_cec_logic_addr {

    /**
    Value to specify the @b TV
    */
    AUI_CEC_LA_TV           = 0x0,

    /**
    Value to specify the <b> Recording Device 1 </b>
    */
    AUI_CEC_LA_RECORD_1     = 0x1,

    /**
    Value to specify the <b> Recording Device 2 </b>
    */
    AUI_CEC_LA_RECORD_2     = 0x2,

    /**
    Value to specify the <b> Tuner 1</b>
    */
    AUI_CEC_LA_TUNER_1      = 0x3,

    /**
    Value to specify the <b> Playback Device 1 </b>
    */
    AUI_CEC_LA_PLAYBACK_1   = 0x4,

    /**
    Value to specify the <b> Audio System </b>
    */
    AUI_CEC_LA_AUDIO_SYSTEM = 0x5,

    /**
    Value to specify the <b> Tuner 2 </b>
    */
    AUI_CEC_LA_TUNER_2      = 0x6,

    /**
    Value to specify the <b> Tuner 3 </b>
    */
    AUI_CEC_LA_TUNER_3      = 0x7,

    /**
    Value to specify the <b> Playback Device 2 </b>
    */
    AUI_CEC_LA_PLAYBACK_2   = 0x8,

    /**
    Value to specify the <b> Playback Device 3 </b>
    */
    AUI_CEC_LA_RECORD_3     = 0x9,

    /**
    Value to specify the <b> Tuner 4 </b>
    */
    AUI_CEC_LA_TUNER_4      = 0xA,

    /**
    Value to specify the <b> Playback Device 3 </b>
    */
    AUI_CEC_LA_PLAYBACK_3   = 0xB,

    /**
    This value is for using a <b> reserved device </b>
    */
    AUI_CEC_LA_RESERVED_1   = 0xC,

    /**
    This value is for using a <b> reserved device </b>
    */
    AUI_CEC_LA_RESERVED_2   = 0xD,

    /**
    This value is for using a <b> special device </b>
    */
    AUI_CEC_LA_FREE_USE     = 0xE,

    /**
    This value is for using
    - Either an <b> Unregistered device </b> (as Initiator Address)
    - Or <b> Broadcast devices </b> (as Destination address)
    */
    AUI_CEC_LA_BROADCAST    = 0xF

} aui_cec_logic_addr, aui_hdmi_cec_logic_addr;

/**
Enum to specify the <b> HDMI Deep Color Definition </b>
*/
typedef enum aui_hdmi_deepcolor {

    /**
    Value to specify <b> 24 bits deep color </b>
    */
    AUI_HDMI_DEEPCOLOR_24 = 0x01,

    /**
    Value to specify <b> 30 bits deep color </b>
    */
    AUI_HDMI_DEEPCOLOR_30 = 0x02,

    /**
    Value to specify <b> 36 bits deep color </b>
    */
    AUI_HDMI_DEEPCOLOR_36 = 0x04,

    /**
    Value to specify <b> 48 bits deep color </b>.
    */
    AUI_HDMI_DEEPCOLOR_48 = 0x08

} aui_hdmi_deepcolor;

/**
@warning  This function pointer is no longer supported then is @a deprecated
*/
typedef void (*aui_hdmi_callback_func) (

    aui_hdmi_event hdmi_evt,

    aui_hdl p_hdl_hdmi,

    void *p_hdmi_event_data,

    void *p_hdmi_user_data

    );

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C"

{

#endif

/**
@brief        Function used to get the version number of the HDMI Module as
              defined in the macro #AUI_MODULE_VERSION_NUM_HDMI

@warning      This function can be used at any time

@param[out]   pul_version        = Pointer to the version number of HDMI Module

@return       @b AUI_RTN_SUCCESS = Getting of the version number of HDMI Module
                                   performed successfully
@return       @b AUI_RTN_EINVAL  = The output parameter (i.e. [out]) is invalid
@return       @b Other_Values    = Getting of the version number of HDMI Module
                                   failed for some reason
*/
AUI_RTN_CODE aui_hdmi_version_get (

    unsigned long *pul_version

    );

/**
@brief        Function used to perform the initialization of the HDMI Module
              before its opening by the function #aui_hdmi_open

@warning      This function can be used @a only in the <b> Pre-Run Stage </b>
              of the HDMI Module

@param[in]    p_call_back_init   = Callback function used for the initialization
                                   of the HDMI Module, as per comment for the
                                   function pointer #p_fun_cb

@return       @b AUI_RTN_SUCCESS = HDMI Module initialized successfully
@return       @b AUI_RTN_EINVAL  = Either one or both input parameter (i.e.
                                   [in]) is invalid
@return       @b Other_Values    = Initializing of the HDMI Module failed for
                                   some reasons

@note         This function should be used only once during system setup. About
              the callback function @b p_call_back_init as input parameter:
              - In projects based on <b> Linux OS </b>, it is suggested to set as
                @b NULL
              - In projects based on <b> TDS OS </b>, it will perform some HDMI
                device attachments and configurations, please refer to the sample
                code of the initialization for more clarifications
*/
AUI_RTN_CODE aui_hdmi_init (

    p_fun_cb p_call_back_init

    );

/**
@brief        Function used to perform the de-initialization of the HDMI Module
              after its closing by the function #aui_hdmi_close

@param[in]    p_call_back_init   = Callback function used to de-initialize the
                                   HDMI Module as per comment for the function
                                   pointer #p_fun_cb

@return       @b AUI_RTN_SUCCESS = HDMI Module de-initialized successfully
@return       @b AUI_RTN_EINVAL  = The input parameter (i.e. [in]) is invalid
@return       @b Other_Values    = De-initializing of the HDMI Module failed
                                   for some reasons
*/
AUI_RTN_CODE aui_hdmi_de_init (

    p_fun_cb p_call_back_init

    );

/**
@brief        Function used to configure the desired attributes of HDMI Module
              and open it, then get the related handle

@warning      This function can @a only be used in the <b> Pre-Run Stage </b>
              of the HDMI Module, in particular:
              - Either after performing the initialization of the HDMI Module
                by the function #aui_hdmi_init for the first opening of the
                HDMI Module
              - Or after closing the HDMI Module by the function #aui_hdmi_close,
                considering the initialization of the HDMI Module has been
                performed previously by the function #aui_hdmi_init

@param[in]    p_attr_hdmi        = Pointer to a struct #aui_attr_hdmi, which
                                   collects the desired attributes for the
                                   HDMI Module
@param[out]   pp_hdl_hdmi        = #aui_hdl pointer to the handle of the HDMI
                                   Module just opened

@return       @b AUI_RTN_SUCCESS = HDMI Module opened successfully then user
                                   can start to configure the HDMI Device
@return       @b AUI_RTN_EINVAL  = Either one or both of the parameter (i.e.
                                   [in], [out]) are invalid
@return       @b Other_value     = Opening of the HDMI Module failed for
                                   some reasons
*/
AUI_RTN_CODE aui_hdmi_open (

    aui_attr_hdmi *p_attr_hdmi,

    aui_hdl *pp_hdl_hdmi

    );

/**
@brief        Function used to close the HDMI Module already opened by the
              function #aui_hdmi_open, then the related handle (i.e. the related
              resources such as memory, device) will be released

@warning      This function can @b only be used in the <b> Post-Run Stage </b>
              of the HDMI Module in pair with its the opening by the function
              #aui_hdmi_open. After closing the HDMI Module, user can
              - Either perform the De-initialization of the HDMI Module by the
                function #aui_hdmi_de_init
              - Or open again the HDMI Module by the function #aui_hdmi_open,
                considering the initialization of the hdmi Module has been
                performed previously by the function #aui_hdmi_init

@param[in]    p_hdl_hdmi         = #aui_hdl handle of HDMI module already opened
                                   and to be stopped/closed

@return       @b AUI_RTN_SUCCESS = HDMI module closed successfully
@return       @b AUI_RTN_EINVAL  = The input parameter (i.e. [in]) is invalid
@return       @b Other_value     = Closing of the HDMI Module failed for some
                                   reasons
*/
AUI_RTN_CODE aui_hdmi_close (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to select the method to be used for getting the
              clear HDCP key, after starting HDMI by the functions #aui_hdmi_open
              and before enabling HDCP by the function #aui_hdmi_hdcp_on

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened/started and to be managed for the
                                    selection of the clear HDCP key getting
                                    method
@param[in]    mem_sel             = Flag used for selecting the method to be
                                    used for getting the clear HDCP key, in
                                    particular:
                                    - @b 1 = CE (hardware) decryption
                                    - @b 0 = Software decryption

@return       @b AUI_RTN_SUCCESS  = Selection of the clear HDCP key getting
                                    method performed successfully
@return       @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                    is invalid
@return       @b Other_value      = Selection of the HDCP key getting method
                                    failed for some reason
*/
AUI_RTN_CODE aui_hdmi_mem_sel (

    aui_hdl p_hdl_hdmi,

    unsigned int mem_sel

    );

/**
@brief        Function used to perform a HDMI setting as defined in the
              #aui_en_hdmi_item_set, after opening HDMI Module by the functions
              #aui_hdmi_open

@param[in]    p_hdl_hdmi          = #aui_hdl handle of HDMI Module already
                                    opened and to be managed for performing
                                    a setting
@param[in]    ul_item             = The item related to the specific setting
                                    of the HDMI Module to be performed, as
                                    defined in the enum #aui_hdmi_item_set
@param[in]    pv_param_in         = The pointer as per the description of the
                                    specific setting of the HDMI Module to be
                                    performed and defined in the enum
                                    #aui_hdmi_item_set

@param[out]   pv_param_out        = The pointer as per the description of the
                                    specific setting of the HDMI Module to be
                                    performed and defined in the enum
                                    #aui_hdmi_item_set

@return       @b AUI_RTN_SUCCESS  = Specific setting of the HDMI Module
                                    performed successfully
@return       @b AUI_RTN_EINVAL   = Either one or both of the parameters (i.e.
                                    [in], [out]) are invalid
@return       @b Other_value      = Specific setting of the HDMI Module failed
                                    for some reasons
*/
AUI_RTN_CODE aui_hdmi_set (

    aui_hdl p_hdl_hdmi,

    unsigned long ul_item,

    void *pv_param_out,

    void *pv_param_in

    );

/**
@brief        Function used to get a specific HDMI information as defined in the
              enum #aui_hdmi_item_get, after opening HDMI Module by the
              function #aui_hdmi_open

@param[in]    p_hdl_hdmi          = #aui_hdl handle of HDMI Module already
                                    opened and to be managed for getting HDMI
                                    information
@param[in]    ul_item             = The item related to the specific information
                                    of the HDMI Module to be gotten, as defined
                                    in the enum #aui_hdmi_item_get

@param[in]    pv_param_in         = The pointer as per the description of the
                                    specific information of the HDMI Module
                                    to be gotten, as defined in the enum
                                    #aui_hdmi_item_get

@param[out]   pv_param_out        = The pointer as per the description of the
                                    specific information of the HDMI Module to
                                    be gotten, as defined in the enum
                                    #aui_hdmi_item_get

@return       @b AUI_RTN_SUCCESS  = Getting of the specific information of the
                                    HDMI Module performed successfully
@return       @b AUI_RTN_EINVAL   = Either one or both of the parameters (i.e.
                                    [in], [out]) are invalid
@return       @b Other_value      = Getting of the specific information of the
                                    HDMI Module failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_get (

    aui_hdl p_hdl_hdmi,

    unsigned long ul_item,

    void *pv_param_out,

    void *pv_param_in

    );

/**
@brief        Function used to set HDCP key after opening HDMI Module by the
              functions #aui_hdmi_open.\n
              When plugging HDMI device and enabling HDCP by the function
              #aui_hdmi_hdcp_on, HDMI Device will use the HDCP key set by this
              function to complete the HDCP Authentication.

@param[in]    p_hdl_hdmi          = #aui_hdl handle of HDMI Module already
                                    opened and to be managed for setting the
                                    HDCP key
@param[in]    p_hdcpkey           = Pointer to the HDCP key
@param[in]    ul_length           = Length of the HDCP key

@return       @b AUI_RTN_SUCCESS  = Setting of the HDCP key performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = At least one input parameters (i.e.
                                    [in]) is invalid
@return       @b Other_value      = Setting of the HDCP key failed for some
                                    reasons

@note         This function is @a only used in <b> SRAM KEY MODE </b>
*/
AUI_RTN_CODE aui_hdcp_params_set (

    aui_hdl p_hdl_hdmi,

    void* const p_hdcpkey,

    unsigned long ul_length

    );

/**
@brief        Function used to read the EDID data from HDMI sink device after
              EDID data are ready, for which the previously
              #AUI_HDMI_CB_EDID_READY callback type will be called

              EDID data mainly display device information such as:
              - Image Resolution
              - Audio Format
              - Sound Channel
              - 3D
              - etc.

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for reading EDID
                                    data
@param[in]    block_idx           = EDID data block index

@param[out]   pul_ediddata        = Pointer to EDID data to be read
@param[out]   pn_datalen          = Length of EDID data to be read

@return       @b AUI_RTN_SUCCESS  = Reading of the EDID data performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = Either one or both of the parameters (i.e.
                                    [in], [out]) are invalid.
@return       @b Other_value      = Reading of the EDID data failed for some
                                    reasons
*/
AUI_RTN_CODE aui_hdmi_ediddata_read (

    aui_hdl p_hdl_hdmi,

    unsigned long *pul_ediddata,

    unsigned int *pn_datalen,

    unsigned int block_idx

    );

/**
@brief        Function used to enable the HDCP Authentication after setting the
              HDCP key by the function #aui_hdcp_params_set

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for enabling the
                                    HDCP Authentication

@return       @b AUI_RTN_SUCCESS  = Enabling of the HDCP Authentication performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return       @b Other_value      = Enabling of the HDCP Authentication failed
                                    for some reasons.

@note         This function will trigger the handshake authentication process
              between AUI HDMI and HDMI TV (it will take few seconds) then return
              immediately.\n
              The updating of HDCP status migh fail if polling it immediately
              after calling this function.
*/
AUI_RTN_CODE aui_hdmi_hdcp_on (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to disable HDCP Authentication already enabled by the
              functions #aui_hdmi_hdcp_on

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for disabling the
                                    HDCP Authentication

@return       @b AUI_RTN_SUCCESS  = Disabling of the HDCP Authentication
                                    performed successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is
                                    invalid
@return       @b Other_value      = Disabling of the HDCP Authentication failed
                                    for some reasons
*/
AUI_RTN_CODE aui_hdmi_hdcp_off (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to select the HDMI Audio Output Format

@warning      This function is no longer supported then is @a deprecated

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for selecting the
                                    HDMI Audio Output Format
@param[in]    pcm_out             = Flag to specify the desired HDMI Audio Output
                                    format, in particular
                                    - @b 1 = PCM
                                    - @b 0 = BS (SPDIF)

@return       @b AUI_RTN_SUCCESS  = Setting of the HDMI Audio Output Format
                                    performed successfully
@return       @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                    (i.e. [in]) is invalid
@return       @b Other_value      = Setting of the HDMI Audio Output Format
                                    failed for some reasons

@note         Make sure that the format chosen in the Sound and HDMI Module are
              the same
*/
AUI_RTN_CODE aui_hdmi_audio_select (

    aui_hdl p_hdl_hdmi,

    int pcm_out

    );

/**
@brief        Function used to enable HDMI Audio Output

@warning      Usually HDMI Audio Output is enabled as default after opening the
              HDMI Module then this function can be used to re-enable it after
              its disabling

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for enabling HDMI
                                    Audio Output

@return       @b AUI_RTN_SUCCESS  = Enabling of the HDMI Audio Output performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = the input parameter (i.e. [in]) is invalid
@return       @b Other_value      = Enabling of the HDMI Audio Output failed
                                    for some reasons
*/
AUI_RTN_CODE aui_hdmi_audio_on (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to disable the HDMI Audio Output

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for disabling HDMI
                                    Audio Output

@return       @b AUI_RTN_SUCCESS  = Disabling of the HDMI Audio Output performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return       @b Other_value      = Disabling of the HDMI Audio Output failed
                                    for some reasons
*/
AUI_RTN_CODE aui_hdmi_audio_off (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to get
              - The current HDCP Authentication Status, i.e. enabled/disabled
              - Whether the current downstream is from a HDMI repeater or not

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened

@param[out]   puc_repeater        = Flag to specify whether the current
                                    downstream is from a HDMI repeater or not,
                                    in particular
                                    - @b 1 = Downstream is from a HDMI repeater
                                    - @b 0 = Downstream is not from a HDMI repeater
@param[out]   pul_success         = Flag to specify the HDCP Authentication Status,
                                    in particular
                                    - @b 1 = Both HDCP enabled
                                             and HDCP authentication passed
                                    - @b 0 = Either HDCP disabled
                                             or HDCP authentication failed

@return       @b AUI_RTN_SUCCESS  = Getting of the current HDCP Authentication
                                    Status and downstream repeater performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = Either one or both of the parameters (i.e.
                                    [in], [out]) are invalid
@return       @b Other_value      = Getting of the current HDCP Authentication
                                    Status and downstream repeater failed for
                                    some reasons
*/
AUI_RTN_CODE aui_hdcp_status_get (

    aui_hdl p_hdl_hdmi,

    unsigned char *puc_repeater,

    unsigned long *pul_success

    );

/**
@brief        Function is used to start the HDMI Module after its opening by the
              function #aui_hdmi_open

@warning      This function is no longer supported then is @a deprecated

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be started.

@return       @b AUI_RTN_SUCCESS  = Starting of the HDMI Module performed
                                    successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is
                                    invalid
@return       @b Other_value      = Starting of the HDMI Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_hdmi_start (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to enable the HDMI Audio/Video Output to the TV Device

@warning      Usually HDMI Audio/Video Output to TV Device is enabled as default
              after opening the HDMI Module, then this function can be used to
              re-enable it after its disabling

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for enabling the
                                    HDMI Audio/Video Output to TV Device

@return       @b AUI_RTN_SUCCESS  = Enabling of the HDMI Audio/Video Output
                                    to TV Device performed successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) are invalid
@return       @b Other_value      = Enabling of the HDMI Audio/Video Output to
                                    TV Device failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_on (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to disable the HDMI Audio/Video Output to TV Device

@param[in]    p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                    opened and to be managed for disabling the
                                    HDMI Audio/Video Output to TV Device

@return       @b AUI_RTN_SUCCESS  = Disabling of the HDMI Audio/Video Output
                                    to TV Device performed successfully
@return       @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return       @b Other_value      = Disabling of the HDMI Audio/Video Output
                                    to TV Device failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_off (

    aui_hdl p_hdl_hdmi

    );

/**
@brief        Function used to get the HDMI Sink Device capability related to
              the preferred HDMI  Video Resolution

@warning      This function is no longer supported then is @a deprecated.
              Please refer to the function #aui_hdmi_sinkcapability_get instead
*/
AUI_RTN_CODE aui_hdmi_sink_prefervideo_get (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_sink_capability *p_caps

    );

/**
@brief        Function used to get the HDMI Sink Device capability related to
              all supported HDMI Video Resolutions

@warning      This function is no longer supported then is @a deprecated.
              Please refer to the function #aui_hdmi_sinkcapability_get instead
*/
AUI_RTN_CODE aui_hdmi_sink_video_support (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_sink_capability *p_caps

    );

/**
@brief        Function used to get the HDMI Sink Device capability related to
              all supported HDMI Audio Coding

@warning      This function is no longer supported then is @a deprecated.
              Please refer to the function #aui_hdmi_sinkcapability_get instead
*/
AUI_RTN_CODE aui_hdmi_sink_audio_support (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_sink_capability *p_caps

    );

/**
@brief        Function used to get the HDMI Sink Device capability related to the
              preferred HDMI Audio Coding

@warning      This function is no longer supported then is @a deprecated.
              Please refer to the function #aui_hdmi_sinkcapability_get instead
*/
AUI_RTN_CODE aui_hdmi_sink_preferaudio_get (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_sink_capability *p_caps

    );

/**
@brief        Function used to get the HDMI Sink Device capabilities such as:
              - Preferred HDMI Audio Coding
              - Preferred HDMI Video Resolution
              - ALl supported HDMI Audio Coding
              - All supported HDMI Video Resolutions
              - Whether the TV device support HDCP or not
              as defined in the enum #aui_hdmi_sink_capability

@param[in]    p_hdl_hdmi            = #aui_hdl handle of the HDMI Module already
                                      opened and to be managed for getting a
                                      HDMI Sink Device capability
@param[in]    p_caps                = Pointer to the specific HDMI sink Device
                                      capability as defined in the struct
                                      #aui_hdmi_sink_capability

@return       @b AUI_RTN_SUCCESS    = Getting of the specific HDMI Sink Device
                                      capability performed successfully
@return       @b AUI_RTN_EINVAL     = Either one or both of the input parameters
                                      (i.e. [in]) is invalid
@return       @b Other_value        = Getting of the specific HDMI Sink Device
                                      capability failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_sinkcapability_get (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_sink_capability *p_caps

    );

/**
@brief         Function used to register a callback function to the HDMI Module
               belonging to a callback type for a HDMI event, and to be actioned
               when that HDMI event occurs.\n

@warning       That registering can be performed after the initialization of
               HDMI Module by the function #aui_hdmi_init

@param[in]     p_hdl_hdmi           = #aui_hdl handle of the HDMI Module already
                                      opened and to be managed for registering
                                      a callback function for a specific HDMI
                                      event
@param[in]     hdmi_cb_type         = Callback type for a a HDMI event, as defined
                                      in the enum #aui_hdmi_cb_type
@param[in]     p_hdmi_callback_func = Pointer to the specific callback function
                                      to be registered for a HDMI event
@param[in]     p_user_data          = Pointer to the user data as input parameter
                                      of the callback function to be registered
                                      for a HDMI event
                                      - @b Caution: This pointer can be set as
                                           NULL if the callback function does
                                           not expect user data

@return        @b AUI_RTN_SUCCESS   = Registering of the callback function to the
                                      HDMI Module performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                      is invalid
@return        @b Other_value       = Registering of the callback function to
                                      the HDMI Module failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_callback_reg (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_cb_type hdmi_cb_type,

    void *p_hdmi_callback_func,

    void *p_user_data

    );

/**
@brief         Function used to de-register the callback function previously
               registered to the HDMI Module for a specific HDMI event by the
               function #aui_hdmi_callback_reg

@param[in]     p_hdl_hdmi           = #aui_hdl handle of the HDMI Module
                                      already opened and to be managed for
                                      de-registering the callback function
                                      previously registered for a specific
                                      HDMI event
@param[in]     hdmi_cb_type         = Callback type for a a HDMI event, as
                                      defined in the enum #aui_hdmi_cb_type
@param[in]     p_hdmi_callback_func = Pointer to the specific callback
                                      function to be unregistered for a HDMI
                                      event
                                      - @b Caution: This pointer can be set
                                           as NULL
@param[in]     pv_user_data         = Pointer to the user data as input
                                      parameter of the callback function to be
                                      unregistered for a HDMI event
                                      - @b Caution: This pointer can be set as
                                           NULL if the callback function does
                                           not expect user data

@return        @b AUI_RTN_SUCCESS   = De-registering of the callback function
                                      from the HDMI Module performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                      is invalid.
@return        @b Other_value       = De-registering of the callback function
                                      from HDMI Module failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_callback_del (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_cb_type hdmi_cb_type,

    void *p_hdmi_callback_func,

    void *pv_user_data

    );

/**
@brief           Function used to set a specific HDMI property

@param[in]       p_hdl_hdmi         = #aui_hdl handle of the HDMI Module already
                                      opened and to be set for a specific HDMI
                                      property
@param[in]       en_propety_type    = HDMI Property type to be set, as defined
                                      in the enum #aui_hdmi_property_item
@param[in]       pv_data            = Pointer to the HDMI property data to be set
@param[in]       pn_len             = Pointer to the HDMI property data length

@return          @b AUI_RTN_SUCCESS = Setting of the specific HDMI property
                                      performed successfully
@return          @b AUI_RTN_EINVAL  = At least one input parameters (i.e. [in])
                                      is invalid
@return          @b Other_value     = Setting of the specific HDMI property
                                      failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_property_set (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_property_item en_propety_type,

    void *pv_data,

    int *pn_len

    );

/**
@brief          Function used to get a specific HDMI property

@param[in]      p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                      opened and to be managed for getting a
                                      specific HDMI property
@param[in]      en_propety_type     = HDMI Property type to be gotten, as defined
                                      in the enum #aui_hdmi_property_item

@param[out]     p_data              = Pointer to the HDMI property data just
                                      gotten
@param[out]     p_len               = Pointer to the HDMI property data length

@return         @b AUI_RTN_SUCCESS  = Getting of the specific HDMI property
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameters (i.e.
                                      [in], [out]) are invalid
@return         @b Other_value      = Getting of the specific HDMI property
                                      failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_property_get (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_property_item en_propety_type,

    void *p_data,

    int *p_len

    );

/**
@warning        This function is no longer supported then is @a deprecated
*/
AUI_RTN_CODE aui_hdmi_set_hdcp (

    unsigned char *puc_ciphered_hdcp,

    int len

    );

/**
@brief          Function used to set a HDMI Color Space Format.

@param[in]      p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                      opened and to be set for the HDMI Color
                                      Space Format
@param[in]      color_space         = The HDMI Color Space Format to be set, as
                                      defined in the enum #aui_hdmi_color_space

@return         @b AUI_RTN_SUCCESS  = Setting of the HDMI Color Space Format
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                      is invalid
@return         @b Other_value      = Setting of the HDMI color Space format
                                      failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_set_color (

    aui_hdl p_hdl_hdmi,

    aui_hdmi_color_space color_space

    );

/**
@brief          Function used to set AV Blank status to AUI HDMI. The AUI HDMI
                device will transimit blank video and silente audio to HDMI
                receivers

@param[in]      p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                      opened and to be set for the HDMI AV Blank
@param[in]      uc_blank            = Status of the HDMI AV Blank to be set, in
                                      particular
                                      - @b 1 = Blank, i.e. video and audio output
                                               disabled, only a black screen is
                                               showed
                                      - @b 0 = Un-Blank, i.e. video and audio
                                               output enabled

@return         @b AUI_RTN_SUCCESS  = Setting of the status of the HDMI AV Blank
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                      is invalid
@return         @b Other_value      = Setting of the status of the HDMI AV Blank
                                      failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_set_avblank (

    aui_hdl p_hdl_hdmi,

    unsigned char uc_blank

    );

/**
@brief          Function used to send AVMute/AVUnMute signal to attached receivers
                to minimize video flashes, audio pops, etc. during format changes,
                color space change, etc.

@param[in]      p_hdl_hdmi          = #aui_hdl handle of the hdmi Module already
                                      opened and to bet for the HDMI AV Mute
@param[in]      uc_on_off           = Status of the HDMI Mute to be set, in
                                      particular
                                      - @b 1 = Mute, i.e. HDMI receiver will mute
                                               its output
                                      - @b 0 = Un-Mute, i.e. HDMI receiver will
                                               un-mute its output
@return         @b AUI_RTN_SUCCESS  = Setting of the status of the HDMI AV Mute
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                      is invalid
@return         @b Other_value      = Setting of the status of the HDMI AV Mute
                                      failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_set_avmute (

    aui_hdl p_hdl_hdmi,

    unsigned char uc_on_off

    );

/**
@brief          Function used to set the status of the HDMI CEC message feature

@param[in]      p_hdl_hdmi           = #aui_hdl handle of the HDMI Module already
                                       opened and to be set for the HDMI CEC
                                       message feature
@param[in]      uc_on_off            = Status of the HDMI CEC message feature
                                       to be set, in particular
                                       - @b 1 = CEC message on
                                       - @b 0 = CEC message off
@return         @b AUI_RTN_SUCCESS   = Setting of the status of the HDMI CEC
                                       message feature performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                       is invalid
@return         @b Other_value       = Setting of the status of the HDMI CEC
                                       message feature failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_cec_set_onoff (

    aui_hdl p_hdl_hdmi,

    unsigned char uc_on_off

    );

/**
@brief          Function used to get the status of the HDMI CEC message feature
                previously set by the function #aui_hdmi_cec_set_onoff

@param[in]      p_hdl_hdmi           = #aui_hdl handle of the HDMI Module already
                                       opened and to be managed for getting the
                                       status of the HDMI CEC message feature
@param[in]      puc_on_off           = Pointer to the status of the HDMI CEC
                                       message feature just gotten, in particular
                                       - @b 1 = CEC message on
                                       - @b 0 = CEC message off
@return         @b AUI_RTN_SUCCESS   = Getting of the status of the HDMI CEC
                                       message feature performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                       is invalid
@return         @b Other_value       = Getting of the status of the HDMI CEC
                                       message feature failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_cec_get_onoff (

    aui_hdl p_hdl_hdmi,

    unsigned char *puc_on_off

    );

/**
@brief          Function used to set the HDMI STB logical address

@param[in]      p_hdl_hdmi           = #aui_hdl handle of the HDMI Module already
                                       opened and to be set for the HDMI STB
                                       logical address
@param[in]      uc_logical_addr      = The HDMI STB logical address to be set

@return         @b AUI_RTN_SUCCESS   = Setting of the HDMI STB logical address
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                       is invalid
@return         @b Other_value       = Setting of the HDMI STB logical address
                                       failed for some reasons
*/
AUI_RTN_CODE aui_hdmi_cec_set_logical_address (

    aui_hdl p_hdl_hdmi,

    unsigned char uc_logical_addr

    );

/**
@brief           Function used to get the STB logical address previously set by
                 the function #aui_hdmi_cec_set_logical_address

@param[in]       p_hdl_hdmi          = #aui_hdl handle of the HDMI Module already
                                       opened and to be managed for getting the
                                       HDMI STB logical address
@param[in]       puc_logical_addr    = Pointer to the HDMI STB logical address
                                       just gotten

@return          @b AUI_RTN_SUCCESS  = Getting of the HDMI STB logical address
                                       performed successfully
@return          @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                       is invalid
@return          @b Other_value      = Getting of the HDMI STB logical address
                                       failed for some reasons

@note            You can use this API to get STB's logical address
*/
AUI_RTN_CODE aui_hdmi_cec_get_logical_address (

    aui_hdl p_hdl_hdmi,

    unsigned char *puc_logical_addr

    );

/**
@brief            Function used to transmit the HDMI CEC message

@param[in]        p_hdl_hdmi         = #aui_hdl handle of the HDMI Module already
                                       opened and to be managed for transmitting
                                       the HDMI CEC message
@param[in]        puc_msg            = Pointer to the HDMI CEC message to be
                                       transmitted
@param[in]        uc_msg_length      = HDMI CEC message length

@return           @b AUI_RTN_SUCCESS = Transmitting of the HDMI CEC message
                                       performed successfully
@return           @b AUI_RTN_EINVAL  = At least one input parameters (i.e. [in])
                                       is invalid
@return           @b Other_value     = Transmitting of the HDMI CEC message failed
                                       for some reasons
*/
AUI_RTN_CODE aui_hdmi_cec_transmit (

    aui_hdl p_hdl_hdmi,

    unsigned char *puc_msg,

    unsigned char uc_msg_length

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_en_hdmi_property aui_hdmi_property_item

#define aui_hdmi_callback_s aui_hdmi_callback

#define aui_short_audio_des aui_short_audio_desc

#define aui_short_audio_des_t aui_short_audio_desc

#define aui_enhdmisink_capability aui_hdmi_sink_capability

#define aui_enhdmisink_capability_t aui_hdmi_sink_capability

#define aui_enhdmisink_capability_e aui_hdmi_sink_capability

#define aui_enhdmi_event aui_hdmi_event

#define aui_enhdmi_event_e aui_hdmi_event

#define aui_hdmi_cb_type_e aui_hdmi_cb_type

#define aui_en_hdmi_link_status aui_hdmi_link_status

#define AUIHDMI_PLUGIN AUI_HDMI_PLUGIN

#define AUIHDMI_PLUGOUT AUI_HDMI_PLUGOUT

#define AUIHDMI_STATUS_UNLINK AUI_HDMI_STATUS_UNLINK

#define AUIHDMI_STATUS_LINK AUI_HDMI_STATUS_LINK

#define AUIHDMI_STATUS_LINK_HDCP_SUCCESSED AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED

#define AUIHDMI_STATUS_LINK_HDCP_FAILED AUI_HDMI_STATUS_LINK_HDCP_FAILED

#define AUIHDMI_STATUS_LINK_HDCP_IGNORED AUI_HDMI_STATUS_LINK_HDCP_IGNORED

#define AUIHDMI_STATUS_MAX AUI_HDMI_STATUS_MAX

#define aui_en_hdmi_link_status_e aui_hdmi_link_status

#define aui_picfmt aui_hdmi_color_space

#define AUIHDMI_VENDOR_NAME AUI_HDMI_VENDOR_NAME

#define AUIHDMI_PRODUCT_DESCRIPTION AUI_HDMI_PRODUCT_DESCRIPTION

#define AUIHDMI_HDCP_KEY AUI_HDMI_HDCP_KEY

#define AUIHDMI_LINK_STATUS AUI_HDMI_LINK_STATUS

#define aui_hdcpkeyinfo_s aui_hdmi_hdcp_key_info

#define aui_hdmi_property_e aui_hdmi_property

#define AUI_YCBCR_422 AUI_HDMI_YCBCR_422

#define AUI_YCBCR_444 AUI_HDMI_YCBCR_444

#define AUI_RGB_MODE1 AUI_HDMI_RGB_MODE1

#define AUI_RGB_MODE2 AUI_HDMI_RGB_MODE2

/**
Struct to specify the HDMI EDID short CEA descriptor
*/
typedef struct short_cea_desc {

  unsigned char cea_data;

  struct short_cea_desc *next;

} short_cea_desc_t;

/**
Struct to specify the HDMI EDID short video descriptor
*/
typedef struct short_video_desc {

  unsigned char native_indicator;

  unsigned char video_id_code;

  struct short_video_desc * next;

} short_video_desc_t;

/**
Struct to specify the HDMI property, as defined in the enum
#aui_hdmi_property_item
*/
typedef struct aui_hdmi_property {

    /**
    Member to specify the <b> HDMI Vendor Name </b>
    */
    unsigned char   vendor_name[32];

    /**
    Member to specify the <b> HDMI Product Description </b>
    */
    unsigned char   product_description[32];

    /**
    @warning  This member is no longer supported then is @a deprecated
    */
    aui_hdmi_hdcp_key_info hdcp_key_info;

    /**
    @warning  This member is no longer supported then is @a deprecated
    */
    unsigned char link_status;

} aui_hdmi_property_e;

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


