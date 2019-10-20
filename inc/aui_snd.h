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
Current Author:     Amu.tu, Nick.Li
Last update:        2017.03.28
-->

@file       aui_snd.h

@brief      Sound (SND) Module

            <b> Sound (SND) Module </b> is used to output audio stream to
            different interface such as
            - I2SO
            - SPDIF
            - HDMI

            Normally, there are two (2) scenarios for using SND Module:
            - <b> Scenario 1 </b> \n
               The Audio Raw Data is decoded by Audio Decoder (DECA) Module,
               then the audio signal in Pulse Code Modulation (PCM) is sent to
               the sound output interfaces such as
                - I2SO
                - SPDIF
                - HDMI
            - <b> Scenario 2 </b> \n
               The Audio Raw Data is passed through DECA Module, and the Raw
               Data is sent to either SPDIF or HDMI sound output interface

            SND Module provides the following functions:
            - Configuration of the sound output data type
            - Configuration of the sound output interface type
            - Set/Get the audio channel
            - Adjustment of the audio volume
            - Mute/Unmute audio
            - Pause/Resume audio

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Sound (SND) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_SND_H

#define _AUI_SND_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number of SND Module </b>, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_SND  (0X00010000)

/**
Macro to specify the maximum SPDIF delay time (in @a millisecond (ms))

@note  The minumum value can be @b 0 (zero)
*/
#define AUI_SPDIF_DELAY_TIME_MAX 500

/*******************************Global Type List*******************************/

/**
Enum to specify all the <b> sound output interfaces type </b> supported by SND
Module
*/
typedef enum aui_snd_out_type {
    /**
    Value to specify the @b CVBS as sound output interface type
    */
    AUI_SND_OUT_I2SO = 0,

    /**
    Value to specify the @b SPDIF as sound output interface type
    */
    AUI_SND_OUT_SPDIF,

    /**
    Value to specify the @b HDMI as sound output interface type
    */
    AUI_SND_OUT_HDMI,

    /**
    Value to specify the total number of sound output interface type supported
    by SND Module, i.e. the total number of items of this enum
    */
    AUI_SND_OUT_LAST

} aui_snd_out_type, *aui_p_snd_out_type;

/**
Enum to specify all the <b> sound output data type </b> supported by SND Module
*/
typedef enum aui_snd_data_type {

    /**
    Value to specify an @b invalid sound output data type
    */
    AUI_SND_OUT_MODE_INVALID=-1,

    /**
    Value to specify the <b> Pulse Code Modulation (PCM) </b> as sound output
    data type

    @note   In this mode, if audio description tracking is enabled (i.e. audio
            PID type is #AUI_DMX_STREAM_AUDIO_DESC as defined in the enum
            #aui_dmx_stream_type), the audio description tracking will be mixed
            with the main audio tracking.
    */
    AUI_SND_OUT_MODE_DECODED = 0,

    /**
    Value to specify the <b> Raw Audio Data </b> as sound output data type,
    where:
    - If the audio stream is @b DD stream, it will be by-passed to SPDIF
      and HDMI output
    - If the audio stream is @b DD+ stream, it will be by-passed to HDMI
      interface, while the output in SPDIF interface will be downgraded
      to DD stream.

    @note  In this mode, if audio description tracking is enabled (i.e. audio
           PID type is #AUI_DMX_STREAM_AUDIO_DESC as defined in the enum
           #aui_dmx_stream_type), only the main audio tracking is enabled on
           the output interface as the audio description tracking is disabled
           on the output interface.
    */
    AUI_SND_OUT_MODE_ENCODED,

    /**
    Value to specify the <b> Raw Audio Data </b> as sound output data type,
    where:
    - If the audio stream is @b DD stream, it will be by-passed to SPDIF
      and HDMI output (the same as the enum value #AUI_SND_OUT_MODE_ENCODED)
    - If the audio stream is @b DD+ stream, the output of HDMI and SPDIF
      interface will be downgraded to DD stream (unlike the enum value
      #AUI_SND_OUT_MODE_ENCODED)

    @note  In this mode, if audio description track is enabled (i.e. audio
           PID type is #AUI_DMX_STREAM_AUDIO_DESC as defined in the enum
           #aui_dmx_stream_type), only the main audio tracking is enabled on
           the output interface as the audio description tracking is disabled
           on the output interface.
    */
    AUI_SND_OUT_MODE_FORCE_DD,

    /**
    Value to specify the total number of sound output data type supported by SND
    Module, i.e. the total number of items of this enum
    */
    AUI_SND_OUT_MODE_LAST

} aui_snd_data_type, *aui_p_snd_data_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> to specify the status of the
        sound output interface
        </div> @endhtmlonly

        Struct to specify the status of the sound output interface
*/
typedef struct aui_snd_out_type_status {
    /**
    Member to specify the sound output interface type, as defined in the enum
    #aui_snd_out_type
    */
    aui_snd_out_type snd_out_type;

    /**
    Member as a @a flag to specify the status of the sound output interface,
    in particular
    - @b 1 = Interface enabled
    - @b 0 = Interface disabled
    */
    unsigned char uc_enabel;

} aui_snd_out_type_status, *aui_p_snd_out_type_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> to specify the sound output mode
        </div> @endhtmlonly

        Struct to specify the sound output mode
*/
typedef struct aui_snd_out_mode {
    /**
    Member to specify the sound output interface type, as defined in the enum
    #aui_snd_out_type
    */
    aui_snd_out_type snd_out_type;

    /**
    Member to specify the sound output data type, as defined in the enum
    #aui_snd_data_type
    */
    aui_snd_data_type snd_data_type;

} aui_snd_out_mode, *aui_p_snd_out_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> to specify the attributes of
        the sound output interface
        </div> @endhtmlonly

        Struct to specify the attributes of the sound output interface
*/
typedef struct aui_snd_out_interface_attr {
    /**
    Member to specify the sound output interface type, as defined in the enum
    #aui_snd_out_type
    */
    aui_snd_out_type snd_out_type;

    /**
    Member as a @a flag to specify the status of the sound output interface,
    in particular
    - @b 1 = Interface enabled
    - @b 0 = Interface disabled
    */
    unsigned char uc_enabel;

    /**
    Member to specify the sound output data type, as defined in the enum
    #aui_snd_data_type
    */
    aui_snd_data_type snd_data_type;

} aui_snd_out_interface_attr, *aui_p_snd_out_interface_attr;

/**
Enum to specify all the <b> audio channels mode </b> supported by SND Module
*/
typedef enum aui_snd_channel_mode {
    /**
    Value to specify an @b invalid audio channel mode
    */
    AUI_SND_CHANNEL_MODE_SND_DUP_NONE,

    /**
    Value to specify the @b left audio channel mode
    */
    AUI_SND_CHANNEL_MODE_SND_DUP_L,

    /**
    Value to specify the @b right audio channel mode
    */
    AUI_SND_CHANNEL_MODE_SND_DUP_R,

    /**
    Value to specify the @b MONO audio channel mode
    */
    AUI_SND_CHANNEL_MODE_SND_DUP_MONO,

    /**
    Value to specify the total number of audio channel mode supported by SND
    Module, i.e. the total number of items of this enum
    */
    AUI_SND_CHANNEL_MODE_LAST

} aui_snd_channel_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> to specify the attributes of
        a sound device
        </div> @endhtmlonly

        Struct to specify the attributes of a sound device
*/
typedef struct aui_attr_snd {
    /**
    Member as an @a Index (which values are integer from zero (0)) used to refer
    different supported SND devices
    */
    unsigned char uc_dev_idx;

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    Member as a @a flag to specify the status of a SND device,in particular
    - @b 1 = Mute
    - @b 0 = Unmute
    */
    unsigned char uc_snd_mute_status;

    /**
    Member to specify the a <b> audio channel mode </b>, as defined in the enum
    #aui_snd_channel_mode
    */
    aui_snd_channel_mode en_snd_channel_mode;

    /**
    Member to specify the <b> video/audio synchronization way </b>
    */
    unsigned long ul_sync_level;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned long ulDACStatus;

    /**
    Member to specify the attributes of @b I2SO sound output interface type, as
    defined in the struct #aui_snd_out_interface_attr
    */
    aui_snd_out_interface_attr snd_outI2so_attr;

    /**
    Member to specify the attributes of @b SPDIF sound output interface type, as
    defined in the struct #aui_snd_out_interface_attr
    */
    aui_snd_out_interface_attr snd_out_spdif_attr;

    /**
    Member to specify the attributes of @b HDMI sound output interface type, as
    defined in the struct #aui_snd_out_interface_attr
    */
    aui_snd_out_interface_attr snd_out_hdmi_attr;

    /**
    Member to specify the attributes of an @b external sound output interface
    type, as defined in the struct #aui_snd_out_interface_attr
    */
    aui_snd_out_interface_attr *p_snd_out_ext_attr;

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

} aui_attr_snd, *aui_p_attr_snd;

/**
Enum used to perform miscellaneous settings on a sound device

@note   This enum is used by the function #aui_snd_set to perform a specific
        setting where
        - The parameter @b ul_item takes the items related to the specific
          setting to perform
        - The parameter @b pv_param takes the pointer as per the description
          of the specific setting to perform
*/
typedef enum aui_snd_item_set {

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API


    /**
    Value used to set the <b> volume level </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the desired
                  volume level which is in the range <b> [0, 100] </b>

    @note   @b 2. This value is @a only used for projects based on <b> TDS OS </b>
                  \n
                  In projects based on <b> Linux OS </b>, instead, user can use
                  the function #aui_snd_vol_set to set the desired volume level

    @warning      This value refers to a deprecated setting, please use the
                  function #aui_snd_vol_set instead

    */
    AUI_SND_VOL_SET=0,

    /**
    Value used to set @b mute/unmute on a sound device

    @note   @b 1. The parameter @b pv_param takes the pointer to a @a flag which
                  specifies mute/unmute state, in particular
                  - @b 1 = Mute
                  - @b 0 = Unmute

    @note:  @b 2. This value is @a only used for projects based on <b> TDS OS </b>
                  \n
                  In projects based on <b> Linux OS </b>, instead, user can use
                  the function #aui_snd_mute_set to set mute/unmute

    @warning      This value refers to a deprecated setting, please use the
                  function #aui_snd_mute_set instead

    */
    AUI_SND_MUTE_SET,

    /**
    Value used to set the <b> audio channel mode </b> by controlling how the left
    and right channel output PCM Data

    @note   The param @b pv_param takes the pointer to an audio channel mode as
            defined in the enum #aui_snd_channel_mode
    */
    AUI_SND_CHANNEL_MODE_SET,

    /**
    Value used to <b> enable or disable </b> the sound output interface

    @note   @b 1. The parameter @b pv_param takes the pointer to a sound output
                  interface type as defined in the enum #aui_en_snd_out_type

    @note   @b 2  The function #aui_snd_out_interface_type_set can also be used
                  to perform the same setting
    */
    AUI_SND_OUT_INTERFACE_SET,

    /**
    Value used to set the <b> sound output mode </b>, raw data bypassing.

    @note   @b  1. The parameter @b pv_param points to the struct
                   #aui_snd_out_mode

    @note   @b  2. This value is @a only used for projects based on <b> TDS OS </b>.
                   \n
                   In projects based on <b> Linux OS </b>, instead, user can use
                   the function #aui_snd_out_data_type_set to set the desired
                   sound output mode
    */
    AUI_SND_OUT_INTERFACE_MODE_SET,

    /**
    @warning    This value refers to a setting no longer supported then is
                @a deprecated
    */
    AUI_SND_SAMPLE_RATE_SET,

    /**
    Value used to set the @b HDMI as <b> sound output interface type </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the value
                  #AUI_SND_OUT_HDMI of the enum #aui_snd_out_type

    @note   @b 2. This value is used @a only in projects based on <b> TDS OS </b>

    @warning      This value refers to a setting no longer supported then is
                  @a deprecated, please use the function #aui_snd_out_data_type_set
                  instead
    */
    AUI_SND_HDMI_BYPASS_SET,

    /**
    Value used to set the @b SPDIF as <b> sound output interface type </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the value
                  #AUI_SND_OUT_SPDIF of the enum #aui_snd_out_type

    @note   @b 2. This value is used @a only in projects based on <b> TDS OS </b>

    @warning      This value refers to a setting no longer supported then is
                  @a deprecated, please use the function #aui_snd_out_data_type_set
                  instead

    */
    AUI_SND_SPIDF_BYPASS_SET,

    /**
    @warning    This value refers to a setting no longer supported then is
                @a deprecated
    */
    AUI_SND_SYNC_LEVEL_SET,

    /**
    Value used to set the <b> synchronization delay time </b>

    @note   The parameter @b pv_param takes the pointer to the desired
            synchronization delay time which is in the range
            <b> [0, #AUI_SPDIF_DELAY_TIME_MAX] </b>. If the input delay
            time is out of this range, it will be clipped.
    */
    AUI_SND_SYNC_DELAY_TIME,

    /**
    @warning    This setting is no longer supported then is @a deprecated
    */
    AUI_SND_IO_CONTROL,

    /**
    @warning    This setting is no longer supported then is @a deprecated
    */
    AUI_SND_FADE_ENABLE,

    /**
    @warning    This setting is no longer supported then is @a deprecated
    */
    AUI_SND_FADE_SPEED,

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

    /**
    Value used to set @b mute/unmute on DAC

    @note   @b 1. The parameter @b pv_param takes the pointer to a @a flag which
                  specifies mute/unmute state, in particular
                  - @b 1 = Mute
                  - @b 0 = Unmute

    @note   @b 2. This value is @a only used for projects based on <b> TDS OS </b>
    */
    AUI_SND_DAC_MUTE_SET,


    /**
    Value used to register the <b> callback function </b> to the sound driver

    @note   @b  1. The parameter @b pv_param points to the struct
                   #aui_st_snd_io_reg_callback_para

    @note   @b  2. This value is @a only used in projects based on <b> TDS OS </b>
    */
    AUI_SND_REG_CB,

    /**
    Value used to enable/disable the <b> SPDIF delay time </b>

    @note   @b 1. The parameter @b pv_param take the pointer to a @a flag which
                  specifies enabling/disabling of the SPDIF delay time, in
                  particular
                  - @b 1 = Enabled
                  - @b 0 = Disabled

    @note   @b 2  This value is used @a only in projects based on <b> TDS OS </b>
    */
    AUI_SND_SPDIF_DELAY_ENABLE,

    /**
    Value used to specify the <b> SPDIF delay time </b> value

    @note   @b 1. The parameter @b pv_param takes the pointer to the SPDIF delay
                  time value

    @note   @b 2. This value is used @a only in projects based on <b> TDS OS </b>
    */
    AUI_SND_SPDIF_DELAY_TIME,

    /**
    Value to specify the total number of items available in this enum
    */
    AUI_SND_SET_CMD_LAST

} aui_snd_item_set;

/**
Enum used to get information about miscellaneous settings on a sound device

@note   This enum is used by function #aui_snd_get to get information about a
        specific setting where
        - The parameter @b ul_item takes the item relates to the specific setting
        - The parameter @b pv_param takes the pointer as per the description of
          the specific setting
*/
typedef enum aui_snd_item_get {

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API


    /**
    Value used to get information about the <b> volume level </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the volume
                  level value
    @note   @b 2. Also the function #aui_snd_vol_get can be used to get the
                  same information
    */
    AUI_SND_VOL_GET=0,

    /**
    Value used to get information about the <b> mute/unmute state </b> on a
    sound device

    @note   @b 1. The parameter @b pv_param takes the pointer to the mute/
                  unmute state

    @note   @b 2. Also the function #aui_snd_mute_get can be used to get the
                  same information
    */
    AUI_SND_MUTE_GET,

    /**
    Value used to get information about the <b> audio channel mode </b>

    @note   The parameter @b pv_param takes the pointer to the audio channel mode
            as defined in the #aui_snd_channel_mode
    */
    AUI_SND_CHANNEL_MODE_GET,

    /**
    Value used to get information about the <b> sound output interface </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the sound output
                  interface as defined in the enum #aui_en_snd_out_type

    @note   @b 2. Also the function #aui_snd_out_interface_type_get can be used
                  to get the same information
    */
    AUI_SND_OUT_INTERFACE_GET,

    /**
    Value used to get information about the <b> sound output mode </b>

    @note   @b 1. The parameter @b pv_param takes points to the struct
                  #aui_snd_out_mode

    @note   @b 2. Also the function #aui_snd_out_data_type_get can be used to
                  get the same information

    @note   @b 3. This value is @a only used for projects based on <b> TDS OS </b>
    */
    AUI_SND_OUT_INTERFACE_MODE_GET,

    /**
    @warning    Getting this information is no longer supported then is
                @a deprecated
    */
    AUI_SND_SAMPLE_RATE_GET,

    /**
    @warning    Getting this information is no longer supported then is
                @a deprecated
    */
    AUI_SND_HDMI_BYPASS_GET,

    /**
    @warning    Getting this information is no longer supported then is
                @a deprecated
    */
    AUI_SND_SPIDF_BYPASS_GET,

    /**
    @warning    Getting this information is no longer supported then is
                @a deprecated
    */
    AUI_SND_DAC_MUTE_GET,

    /**
    @warning    Getting this information is no longer supported then is
                @a deprecated
    */
    AUI_SND_SYNC_LEVEL_GET,

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

    /**
    Value used to get the SPDIF delay time (in @a millisecond (ms) unit)

    @note  This value is used only in projects based on <b> Linux OS </b>
    **/
    AUI_SND_SPDIF_DELAY_TIME_GET,

    /**
    Value to specify the total number of item available in this enum
    */
    AUI_SND_GET_CMD_LAST
} aui_snd_item_get;

/**
Function pointer to specify the prototype of a type of callback function listed
in the enum #aui_snd_cbtype and to be registered to a sound driver
*/
typedef void (*aui_snd_cbfunc) (

    void *pv_param

    );

/**
Enum to specify all supported callback type which can be registered to a sound
driver
*/
typedef enum aui_snd_cbtype {

    /**
    Value to specify a callback type used to notify an application that there
    is still some data below the threshold.
    */
    AUI_SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD = 0,

    /**
    Value to specify a callback type used to notify an application that there
    is no data to output currently
    */
    AUI_SND_CB_MONITOR_OUTPUT_DATA_END,

    /**
    Value to specify a callback type used to notify an application that an
    error occurred
    */
    AUI_SND_CB_MONITOR_ERRORS_OCCURED,

    /**
    Value to specify that the playing of the mix audio is ended
    */
    AUI_SND_CB_MONITOR_MIX_DATA_END,

    /**
    Value to specify that the mix audio is forcibly paused because no main
    audio output in playing mode, then the mix audio will be resumed when the
    main audio output resumes
    */
    AUI_SND_CB_MONITOR_MIX_PAUSED,

    /**
    Value to specify the total number of item available in this enum
    */
    AUI_SND_CB_MONITOR_LAST

} aui_snd_cbtype;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> used to register a callback
        function to a sound driver
        </div> @endhtmlonly

        Struct used to register a callback function to a sound driver
*/
typedef struct aui_snd_io_reg_callback_para {

    /**
    Member to specify the callback type to be registered to a sound driver as
    defined in the enum #aui_snd_cbtype,
    */
    aui_snd_cbtype e_cbtype;

    /**
    Member as the pointer to the callback function to be registered as per
    comment to the function pointer #aui_snd_cbfunc
    */
    aui_snd_cbfunc p_cb;

    /**
    Member as the pointer to the parameter of the callback function to be
    registered as defined in the member #p_cb of this struct
    */
    void *pv_param;

    /**
    Member to specify the threshold value of the specific sound output data type
    */
    unsigned long threshold;

} aui_snd_io_reg_callback_para;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> used to specify
        the attributes of I2S output (PCM data) capture buffer
        </div> @endhtmlonly

        Struct used to specify the attributes of I2S output (PCM data)
        capture buffer
*/
typedef struct aui_snd_i2s_output_capture_buffer {

    /**
    Member as pointer to the PCM data capture buffer

    @note About the PCM data format:
          - Stereo
          - 24 bits per sample

    @note Furthermore:
          - The PCM data are aligned to 32-bits left-justified
          - Left and right channels will interleave one sample
            per 32-bit word as below:

                 |Address byte:    3  2  1  0        7  6  5  4    |
                 |                -------------     -------------  |
                 |Little-Endian:  |L3|L2|L1|00|     |R3|R2|R1|00|  |
                 |                -------------     -------------  |
     */
    void* pv_buffer_data;

    /**
    Member to specify the length (in bytes unit) of the PCM data capture buffer
    */
    unsigned long ul_buffer_length;

    /**
    Member to specify the sample rate of the PCM data capture buffer

    @note That sample rate is related to the input stream, and may
          changes when zapping to a new channel
    */
    unsigned long ul_sample_rate;

} aui_snd_i2s_output_capture_buffer;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Sound (SND) Module </b> used to specify
        the attributes of the captured I2S output (PCM data)
        </div> @endhtmlonly

        Struct used to specify the attributes of the captured I2S output
        (PCM data)
*/
typedef struct aui_snd_i2s_output_capture_attr {

    /**
    @warning  This memmber is reserved to ALi R&D Dept. then user can ignore it
    */
    unsigned long ul_reserved;

} aui_snd_i2s_output_capture_attr;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of the SND module

@param[in]      pul_version             = Pointer to the version number

@return         @b AUI_RTN_SUCCESS      = Getting of the version number performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameters (i.e. [in]) is
                                          invalid
*/
AUI_RTN_CODE aui_snd_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to perform the initialization of the SND device
                before its opening by the function #aui_snd_open

@param[in]      p_call_back_init        = Callback function used for the
                                          initialization of the SND device as
                                          per comment of the function pointer
                                          #p_fun_cb
@param[in]      *pv_param               = The parameter of the callback function
                                          @b p_call_back_init used for the
                                          initialization of SND device

@return         @b AUI_RTN_SUCCESS      = Initializing of the SND device performed
                                          successfully
@return         @b Other_Values         = Initializing of the SND device failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the De-Initialization of the sound device,
                after its closing by the function #aui_snd_close

@param[in]      p_call_back_init        = Callback function used to de-initialize
                                          the sound device as per comment of the
                                          function pointer #p_fun_cb
@param[in]      *pv_param               = The parameter of the callback function
                                          @b p_call_back_init used to de-initialize
                                          the SND device

@return         @b AUI_RTN_SUCCESS      = De-initializing of the SND device
                                          performed successfully
@return         @b Other_Values         = De-initializing of the SND device
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to open the SND device according to the desired
                attributes, then get the related handle

@warning        This function can @a only be used:
                - Either after performing the initialization of the SND device
                  by the function #aui_snd_init for the first opening of the
                  SND Module
                - Or after closing the SND device by the function #aui_snd_close,
                  considering the initialization of the SND device has been
                  performed previously by the function #aui_snd_init

@param[in]      *p_attr_snd             = Pointer to a struct #aui_attr_snd which
                                          collects the desired attributes for
                                          the SND device
@param[out]     p_handle                = #aui_hdl pointer to the handle of the
                                          SND device just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the SND device performed
                                          successfully
@return         @b Other_Values         = Opening of the SND device failed for
                                          some reasons

@note           The parameter @b p_attr_snd can not be @b NULL, and the @a only
                member to be set in the struct #aui_attr_snd is the device index
                @b uc_dev_idx which @b only can be set as @b 0.\n
                The other members in that struct #aui_attr_snd don't need to be
                set here. Please refer to the sample code of opening SND Module
                for more clarifications.
*/
AUI_RTN_CODE aui_snd_open (

    const aui_attr_snd *p_attr_snd,

    aui_hdl* const p_handle

    );

/**
@brief          Function used to close the SND device already opened by the
                function #aui_snd_open then the related handle will be released
                (i.e. the related resources such as memory, device)

@warning        After closing the SND device, user can:
                - Either perform the De-Initialization of the SND device by the
                  function #aui_snd_de_init
                - Or open again the SND device by the function #aui_snd_open,
                  considering the initialization of the SND device has been
                  performed previously by the function #aui_snd_init

@param[in]      handle                  = #aui_hdl handle of the sound device
                                          already opened and to be closed

@return         @b AUI_RTN_SUCCESS      = Closing of the SND device performed
                                          successfully
@return         @b Other_Values         = Closing of the SND device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_close (

    aui_hdl handle

    );

/**
@brief          Function used to start the SND device already opened by the
                function #aui_snd_open

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be started
@param[in]      *p_attr_snd             = Pointer to a struct #aui_attr_snd,
                                          which collects the desired attributes
                                          for the SND device
                                          - @b Caution: This parameter is
                                                        @a reserved to ALi
                                                        R&D Dept. for future
                                                        development then user
                                                        can ignore it

@return         @b AUI_RTN_SUCCESS      = Starting of the SND device performed
                                          successfully
@return         @b Other_Values         = Starting of the SND device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_start (

    aui_hdl handle,

    const aui_attr_snd *p_attr_snd

    );

/**
@brief          Function used to stop the SND device already started by the
                function #aui_snd_start

@warning        After stopping the SND device, it can be re-started again with
                the function #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be stopped
@param[in]      *p_attr_snd             = Pointer to a struct #aui_attr_snd,
                                          which collects the desired attributes
                                          for the SND device
                                          - @b Caution: This parameter is
                                                        @a reserved to ALi R&D
                                                        Dept. for future development
                                                        then user can ignore it

@return         @b AUI_RTN_SUCCESS      = Stopping of the SND device performed
                                          successfully
@return         @b Other_Values         = Stopping of the SND device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_stop (

    aui_hdl handle,

    const aui_attr_snd *p_attr_snd

    );

/**
@brief          Function used to pause the SND device already started by the
                function #aui_snd_start.

@note           By this function, SND device will output SND data immediately.

@warning        After pausing the SND device, it can be resumed with the function
                #aui_snd_resume

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already started then to be paused
@param[in]      *p_attr_snd             = Pointer to a struct #aui_attr_snd,
                                          which collects the desired attributes
                                          for the SND device
                                          - @b Caution: This parameter is
                                                        @a reserved to ALi R&D
                                                        Dept. for future development
                                                        then user can ignore it

@return         @b AUI_RTN_SUCCESS      = Pausing of the SND device performed
                                          successfully
@return         @b Other_Values         = Pausing of the SND device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_pause (

    aui_hdl handle,

    const aui_attr_snd *p_attr_snd

    );

/**
@brief          Function used to resume the SND device already paused by the
                function #aui_snd_pause

@warning        After resuming the SND device, the audio will be played again,
                and two (2) actions can be performed as below:
                - Pausing the SND device again by the function #aui_snd_pause
                - Stopping the SND device by the function #aui_snd_stop

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already paused then to be resumed
@param[in]      *p_attr_snd             = Pointer to a struct #aui_attr_snd,
                                          which collects the desired attributes
                                          for the SND device
                                          - @b Caution: This parameter is reserved
                                                        to ALi R&D Dept. for future
                                                        development then user can
                                                        ignore it

@return         @b AUI_RTN_SUCCESS      = Resuming of the SND device performed
                                          successfully
@return         @b Other_Values         = Resuming of the SND device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_resume (

    aui_hdl handle,

    const aui_attr_snd *p_attr_snd

    );

/**
@brief          Function used to perform a specific setting to a SND device,
                after its opening by the functions #aui_snd_open

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed to
                                          perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting of the SND device to be
                                          performed, as defined in the enum
                                          #aui_snd_item_set
@param[in]      *pv_param               = The pointer as per the description of
                                          the specific setting of the SND device
                                          to be performed, as defined in the
                                          enum #aui_snd_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting of the SND device
                                          performed successfully
@return         @b Other_Values         = Specific setting of the SND device
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_set (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get information about a specific setting of a
                SND device, after its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting information about a specific
                                          setting
@param[in]      ul_item                 = The item related to the specific setting
                                          of the SND device, as defined in the
                                          enum #aui_snd_item_get
@param[in]      *pv_param               = The pointer as per the description of
                                          the specific setting of the SND device,
                                          as defined in the enum #aui_snd_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific setting
                                          information performed successfully
@return         @b Other_Values         = Getting of the specific setting
                                          information failed for some reasons
*/
AUI_RTN_CODE aui_snd_get (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to adjust the volume level for a SND device

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          adjusting the volume level
@param[in]      uc_vol                  = The desired volume level to be set
                                          (within the range [0,100])

@return         @b AUI_RTN_SUCCESS      = Adjusting of the volume level for SND
                                          device performed successfully
@return         @b Other_Values         = Adjusting of the volume level for SND
                                          device failed for some reasons
*/
AUI_RTN_CODE aui_snd_vol_set (

    aui_hdl handle,

    unsigned char uc_vol

    );

/**
@brief          Function used to get the volume level of the SND device

@param[in]      p_hdl_snd               = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting the volume level
@param[in]      puc_vol                 = Pointer to the buffer intended to
                                          store the volume level

@return         @b AUI_RTN_SUCCESS      = Getting of the volume level performed
                                          successfully
@return         @b Other_Values         = Getting of the volume level failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_vol_get (

    aui_hdl p_hdl_snd,

    unsigned char* const puc_vol

    );

/**
@brief          Function used to set mute/unmute state to the SND device, after
                its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting mute/unmute state
@param[in]      uc_enable               = Flag to specify mute/unmute state,
                                          in particular
                                          - @b 1 = Mute
                                          - @b 0 = Unmute

@return         @b AUI_RTN_SUCCESS      = Setting of the mute/unmute state
                                          performed successfully
@return         @b Other_Values         = Setting of the mute/unmute state
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_mute_set (

    aui_hdl handle,

    unsigned char uc_enable

    );

/**
@brief          Function used to get information about mute/unmute state from
                the SND device, after its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device already
                                          opened and to be managed for getting the
                                          mute/unmute state
@param[in]      puc_enable              = Flag to specify mute/unmute state, in
                                          particular
                                          - @b 1 = Mute
                                          - @b 0 = Unmute

@return         @b AUI_RTN_SUCCESS      = Getting of the mute/unmute state performed
                                          successfully
@return         @b Other_Values         = Getting of the mute/unmute state failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_mute_get (

    aui_hdl handle,

    unsigned char* const puc_enable

    );

/**
@brief          Function used to set the status (i.e. enable/disable) on a sound
                output interface of the SND device, after its starting by the
                functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting a sound output interface type
@param[in]      snd_out_type            = The sound output interface type with
                                          its status (i.e. enable/disable) to
                                          be set, as defined in the struct
                                          #aui_snd_out_type_status

@return         @b AUI_RTN_SUCCESS      = Setting of the status on a sound
                                          output interface type performed
                                          successfully
@return         @b Other_Values         = Setting of the status on a sound
                                          output interface type failed for
                                          some reasons
*/
AUI_RTN_CODE aui_snd_out_interface_type_set (

    aui_hdl handle,

    aui_snd_out_type_status snd_out_type

    );

/**
@brief          Function used to get information about the status (i.e. enable/
                disable) on a sound output interface of the SND device, after
                its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting information about the status
                                          of a sound output interface type
@param[in,out]  p_snd_out_type          = This parameter can be
                                          - <b> [in] </b> as @b snd_out_type
                                            to specify which sound output
                                            interface need to be read
                                          - <b> [out] </b> as @b puc_enable to
                                            stores the status of the interface
                                            \n\n

                                          @b Caution: Please refer to the struct
                                                      #aui_snd_out_type_status
                                                      for more clarifications
                                                      about the parameter
                                                      @b p_snd_out_type

@return         @b AUI_RTN_SUCCESS      = Getting of the status on a sound output
                                          interface type performed successfully
@return         @b Other_Values         = Getting of the status on a sound output
                                          interface type failed for some reasons
*/
AUI_RTN_CODE aui_snd_out_interface_type_get (

    aui_hdl handle,

    aui_snd_out_type_status* const p_snd_out_type

    );

/**
@brief          Function used to set the sound output data type (Raw data by
                passing) on a specific sound output interface type of the SND
                device, after its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting a sound output data type
@param[in]      snd_out_type            = The sound output interface type with
                                          its sound output data type to be set,
                                          as defined in the struct
                                          #aui_snd_out_mode

@return         @b AUI_RTN_SUCCESS      = Setting of the sound output data type
                                          performed successfully
@return         @b Other_Values         = Setting of the sound output data type
                                          failed some reasons
*/
AUI_RTN_CODE aui_snd_out_data_type_set (

    aui_hdl handle,

    aui_snd_out_mode snd_out_type

    );

/**
@brief          Function used to get information about the sound output data
                type on a specific sound output interface of the SND device,
                after its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting information about the sound
                      output data type

@param[in,out]  p_snd_out_type          = This parameter can be
                                          - <b> [in] </b> as @b snd_out_type to
                                            specify which sound output interface
                                            need to be read
                                          - <b> [out] </b> as @b snd_data_type
                                            to indicate the sound data type for
                                            the specified sound output interface
                                            by the parameter @b snd_out_type
                                            \n\n

                                          @b Caution: Please refer to the struct
                                                      #aui_snd_out_mode for more
                                                      clarifications about the
                                                      parameter @b p_snd_out_type

@return         @b AUI_RTN_SUCCESS      = Getting of the sound output data type
                                          performed successfully
@return         @b Other_Values         = Getting of the sound output data type
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_out_data_type_get (

    aui_hdl handle,

    aui_snd_out_mode* const p_snd_out_type

    );

/**
@brief          Function used to set the video/audio synchronization way of a
                SND device,  after its starting by the functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting the video/audio synchronization
                                          way
@param[in]      ul_sync_level           = The level of the video/audio
                                          synchronization way

@return         @b AUI_RTN_SUCCESS      = Setting of the video/audio
                                          synchronization way performed
                                          successfully
@return         @b Other_Values         = Setting of the video/audio
                                          synchronization way failed for some
                                          reasons

@note           This function is @a only used in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_snd_sync_level_set (

    aui_hdl handle,

    unsigned long ul_sync_level

    );

/**
brief           Function used to get information about the video/audio
                synchronization way of a SND device, after its starting by the
                functions #aui_snd_start

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting information about the video/audio
                                          synchronization
@param[in]      pul_sync_level          = Pointer to the buffer intended to store
                                          the level of video/audio synchronization
                                          way

@return         @b AUI_RTN_SUCCESS      = Getting of the video/audio synchronization
                                          way performed successfully
@return         @b Other_Values         = Getting of the video/audio synchronization
                                          way failed for some reasons

@note           This function is @a only used in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_snd_sync_level_get (

    aui_hdl handle,

    unsigned long *pul_sync_level

    );

/**
@brief          Function used to set the Sound channel

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting the sound channel
@param[in]      channel_mode            = Channel mode to be set

@return         @b AUI_RTN_SUCCESS      = Setting of the sound channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) in invalid
@return         @b Other_Value          = Setting of the sound channel failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_channel_set (

    aui_hdl handle,

    aui_snd_channel_mode channel_mode

    );

/**
@brief          Function used to set the audio description volume by changing
                the mix offset.

@note           @b 1. The volume of Audio Description (AD) channel does not change
                      linearly. Instead, the audio description track is mixed with
                      the main audio track with the offset coefficient.\n
                      The output PCM of the audio description will be given by the
                      formula below:\n\n

                      <b> Output_PCM = Main_track_PCM + (AD_track_PCM x (4 + offset) / 4) </b>

@note           @b 2. The volume of the main audio track is not affected by this
                      function

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting the sound channel

@param[in]      offset                  = The mixing offset to be set which value
                                          is within the range [-3,3]

@return         @b AUI_RTN_SUCCESS      = Setting of the sound channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) in invalid
@return         @b Other_Value          = Setting of the sound channel failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_desc_volume_offset_set (

    aui_hdl handle,

    int offset

    );

/**
@brief          Function used to set the audio mixing level balance
                between the primary (main) and secondary audio
                stream.

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          setting the sound channel
@param[in]      balance                 = The mixing offset to be set which
                                          value is within the range <b> [-16,16] </b>,
                                          in particular:
                                          - @b 0 (<b> default value </b>):\n
                                            No adjustment of the audio level
                                            between the main and secondary audio
                                            streams is performed
                                          - <b> [-16,0) </b>:\n
                                            Increase the volume proportion of the
                                            main audio stream and decrease the
                                            volume proportion of the secondary
                                            stream.\n
                                            When the mixing balance level is equal
                                            to the minimum value (i.e @b -16),
                                            only the main audio stream is played.
                                          - <b> (0,16] </b>:\n
                                            Increase the volume portion of the
                                            secondary audio stream and decrease
                                            the volume proportion of the main
                                            stream.\n
                                            When the mixing balance level is equal
                                            to the maximum value (i.e. @b 16),
                                            only the secondary audio stream is played.

@return         @b AUI_RTN_SUCCESS      = Setting of the sound channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) in invalid
@return         @b Other_Value          = Setting of the sound channel failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_mix_balance_set (

    aui_hdl handle,

    int balance

    );

/**
@brief          Function used to get the sound channel

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting the sound channel
@param[out]     p_channel_mode          = Pointer to the buffer intended to
                                          store the channel mode

@return         @b AUI_RTN_SUCCESS      = Getting of the sound channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in], [out])
                                          is invalid
@return         @b Other_Value          = Setting of the sound channel failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_channel_get (

    aui_hdl handle,

    aui_snd_channel_mode *p_channel_mode

    );

/**
@brief          Function used to start capturing I2S output (PCM data)

@warning        The PCM data capture feature may not be available due to
                security constrains so this function may not be applicable.

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          starting the capture of PCM data
@param[in]      p_attr                  = Pointer to a struct #aui_snd_i2s_output_capture_attr
                                          which collects the attributes of the
                                          captured PCM data

@return         @b AUI_RTN_SUCCESS      = Starting the capture of PCM data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Value          = Starting the capture of PCM data failed
                                          for some reasons

*/
AUI_RTN_CODE aui_snd_i2s_output_capture_start (

    aui_hdl handle,

    aui_snd_i2s_output_capture_attr *p_attr

    );

/**
@brief          Function used to stop capturing I2S output (PCM data)

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          stopping the capture of PCM data

@return         @b AUI_RTN_SUCCESS      = Stopping the capture of PCM data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Value          = Stopping the capture of PCM data failed
                                          for some reasons
*/
AUI_RTN_CODE aui_snd_i2s_output_capture_stop (

    aui_hdl handle

    );

/**
@brief          Function used to get the I2S output (PCM data) capture buffer(s).

@note           The I2S otput output (PCM data) is captured into multiple
                buffers so the application needs to go through all these
                buffers for getting all PCM data.

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          getting the PCM data capture buffer(s)
@param[out]     pp_buffer               = Pointer to the PCM data capture buffer(s).
                                          - @b Caution: This buffer is allocated
                                                        in AUI internally so the
                                                        application <em> must not </em>
                                                        release it by the system API
                                                        @b free() but, instead, by the
                                                        function
                                                        #aui_snd_i2s_output_capture_buffer_release.
                                                        That is a @a must.

@param[out]     pul_buffer_num          = Total number of PCM data capture buffer(s)
                                          to be gone through for getting all PCM data.

@return         @b AUI_RTN_SUCCESS      = Getting of the PCM data capture buffer(s)
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in], [out])
                                          is invalid
@return         @b Other_Value          = Getting of the PCM data capture buffer(s)
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_i2s_output_capture_buffer_get (

    aui_hdl handle,

    aui_snd_i2s_output_capture_buffer **pp_buffer,

    unsigned long *pul_buffer_num

    );

/**
@brief          Function used to release the I2S output (PCM data) buffer(s).

@param[in]      handle                  = #aui_hdl handle of the SND device
                                          already opened and to be managed for
                                          releasing the PCM data capture buffer(s)
@param[in]      ul_buffer_num           = Total number of PCM data capture buffer(s)
                                          to be released.
                                          - @b Caution: It @a must be equal to the
                                                        total number of PCM data
                                                        capture buffer(s) to be
                                                        gone through for getting
                                                        all PCM data, as specified
                                                        in the function
                                                        #aui_snd_i2s_output_capture_buffer_get.

@return         @b AUI_RTN_SUCCESS      = Releasing of the PCM data capture buffer(s)
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],) is
                                          invalid
@return         @b Other_Value          = Releasing of the PCM data capture buffer(s)
                                          failed for some reasons
*/
AUI_RTN_CODE aui_snd_i2s_output_capture_buffer_release (

    aui_hdl handle,

    unsigned long ul_buffer_num

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_snd_cbtype_e  aui_snd_cbtype

#define aui_en_snd_out_type  aui_snd_out_type

#define aui_en_snd_data_type aui_snd_data_type

#define aui_st_snd_out_type_status aui_snd_out_type_status

#define aui_st_snd_out_mode aui_snd_out_mode

#define aui_st_snd_out_interface_attr aui_snd_out_interface_attr

#define aui_snd_channel_mode_em aui_snd_channel_mode

#define aui_st_attr_snd aui_attr_snd

#define aui_en_snd_item_set aui_snd_item_set

#define aui_en_snd_item_get aui_snd_item_get

#define aui_st_snd_io_reg_callback_para aui_snd_io_reg_callback_para

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


