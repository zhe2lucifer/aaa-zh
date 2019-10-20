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
Current ALi Author: Wendy.He
Last update:        2017.02.23
-->

@file       aui_av.h

@brief      Audio/Video (AV) Module

            Audio/Video (Av) Module is used to playback more conveniently
            - Live TS Streams
            - Transport Streams (TS) (only in project based on Linux OS)
            - Elementary Streams (ES) (only in project based on Linux OS)

            and it is built on the top of the following modules
            - <b> Video Decoder </b> (DECV)
            - <b> Audio Decoder </b> (DECA)
            - @b De-Multiplexing (DMX)
            - @b Sound (SND)
            - @b Display Engine (DIS) (only for projects based on Linux OS)
            - @b AV-Injection (only for project based on Linux OS)

@note For further details, please refer to the ALi document
      <b><em> ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Audio/Video (AV) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_AV_H

#define _AUI_AV_H

/*************************Included Header File List***************************/

#include "aui_common.h"

#include "aui_decv.h"

#include "aui_deca.h"

#include "aui_snd.h"

#include "aui_dmx.h"

#include "aui_dsc.h"

#ifndef DEPRECATE_OLD_CODECID_IN_AUI_API

#define DEPRECATE_OLD_CODECID_IN_AUI_API

#endif

#include "aui_av_injecter.h"

/*******************************Global Type List*******************************/

/**
Enum to specify which Module/Device is using the buffer channel

@note  This enum is intended @a only for projects based on <b> Linux OS </b>.
*/
typedef enum aui_av_buffer_channel {

    /**
    Value to specify that the <b> clear or encrypted TS stream </b> will be
    written to the buffer of DMX device, then it will be decrypted for AV
    decoding.

    @note This channel type usually works with the enum value #AUI_DMX_ID_SW_DEMUX0
          for TS injection.
    */
    AUI_AV_BUFFER_CHANNEL_DMX = 0,

    /**
    Value to specify that the <b> audio ES data </b> will be written to the
    buffer of audio decoder directly
    */
    AUI_AV_BUFFER_CHANNEL_AUD_DECODER,

    /**
    Value to specify that the <b> video ES data </b> will be written to the
    buffer of video decoder directly
    */
    AUI_AV_BUFFER_CHANNEL_VID_DECODER,

    /**
    Value to specify that the <b> block encrypted TS stream </b> will be
    written to the buffer of DMX device, then it will be decrypted for AV
    decoding.

    @note   The data are the encypted TS packets (i.e. TS header and payload).\n
            The TS stream is over encrypted as complete blocks and has not
            visible MPEG-transport-structure any more.\n
            The raw (original) data are encrypted in #AUI_DSC_DATA_PURE mode
            as described in the enum #aui_en_dsc_data_type
    */
    AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA,

    /**
    Value to specify that the <b> encrypted audio ES data </b> will be written
    to the buffer of audio decoder

    The entire ES sample is encrypted
    */
    AUI_AV_BUFFER_CHANNEL_AUD_DECODER_FOR_ENCRYPTED_DATA,

    /**
    Value to specify that the <b> encrypted video ES data </b> will be written to
    the buffer of video decoder.

    @note  The ES sample can be
           - Either @b full-sample
           - Or @b sub-sample encrypted

    @note  In AVC NAL unit based encryption scheme
           - The NAL unit header is unencrypted
           - The NAL unit data are encrypted

    @note  When subsample encryption is used, the value of the member
           @b ul_subsample_count of the structure #aui_av_subsample_encryption_info
           should be greater than 0
    */
    AUI_AV_BUFFER_CHANNEL_VID_DECODER_FOR_ENCRYPTED_DATA,

    /**
    This value specify the total number of buffer channel usage available in
    this enum #aui_av_buffer_channel
    */
    AUI_AV_BUFFER_CHANNEL_MAX

} aui_av_buffer_channel, aui_av_buffer_channel_t;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video (AV) Module </b> to specify its all available
       settings
       </div> @endhtmlonly

       Struct to specify all available setting of AV Module

@note  This struct is intended @a only for projects based on <b> Linux OS </b>.
*/
typedef struct aui_av_inject_settings {

    /**
    Member to specify the <b> DMX ID </b> to be used by AV Module.

    @note   @b 1. With reference to the type of data to be injected into AV
                  Module as defined in the enum #aui_av_data_type, this member is
                  - Applicable when setting #AUI_AV_DATA_TYPE_NIM_TS or
                    #AUI_AV_DATA_TYPE_RAM_TS
                  - Ignored when setting #AUI_AV_DATA_TYPE_ES

    @note   @b 2. When data_type is #AUI_AV_DATA_TYPE_NIM_TS, this member can
                  only be
                  - <b> Hardware DMX </b>
                  - The enum value #AUI_DMX_ID_DEMUX0
                  - The enum value #AUI_DMX_ID_DEMUX1
                  - etc.

    @note   @b 3. When data_type is #AUI_AV_DATA_TYPE_RAM_TS, this member
                  - Is usually set to the enum value #AUI_DMX_ID_SW_DEMUX0
                  - Can be set to hardware DMX only when routing TS to common
                    interface
    */
    aui_dmx_id    dmx_id;

    /**
    Member to specify the type of data to be injected into AV Module as defined
    in the enum #aui_av_data_type
    */
    aui_av_data_type      data_type;

} aui_av_inject_settings, aui_av_inject_settings_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the packet
        information of the data to be injected in AV Module
        </div> @endhtmlonly

        Struct to specify the <b> Packet Information </b> of the data to be
        injected into AV Module

        @note   This struct is intended @a only for projects based on
                <b> Linux OS </b>.
*/
typedef struct aui_av_inject_packet_info {

    /**
    Member to specify the <b> channel to receive </b> the data to be injected
    in AV Module
    */
    aui_av_buffer_channel buffer_channel;

    /**
    Member to specify the information of the ES header

    @note  With reference to which Module/Device is using the buffer channel
           as defined in the enum #aui_av_buffer_channel_e, this member is
           - Applicable when setting #AUI_AV_BUFFER_CHANNEL_AUD_DECODER or
             #AUI_AV_BUFFER_CHANNEL_VID_DECODER
           - Ignored when setting #AUI_AV_BUFFER_CHANNEL_DMX
    */
    aui_av_packet buffer_info;

} aui_av_inject_packet_info, aui_av_inject_packet_info_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the DSC
        context
        </div> @endhtmlonly

        Struct to specify the DSC context in AV Module
**/
typedef struct aui_av_dsc_context {

    /**
    Member to specify the Audio ID of DSC Context
    **/
    aui_dsc_resource_id* p_audio_dsc_id;

    /**
    Member to specify the Video ID of DSC context
    **/
    aui_dsc_resource_id* p_video_dsc_id;

} aui_av_dsc_context;

/**
Enum to specify the all <b> different mode to inject the video data </b> into
AV Module

@note   This enum is intended @a only for projects based on <b> Linux OS </b>
*/
typedef enum aui_av_video_dec_mode {

    /**
    Value to specify the mode with which a whole video data frame needs to be
    injected into the AV Injector Module each time.
    */
    AUI_AV_VIDEO_DEC_MODE_FRAME = 5,

    /**
    Value to specify the mode with which either a fixed or another size of
    video data can be injected into the AV Injector Module continuously then
    the AV Injector Module will gather an entire frame to decode automatically
    */
    AUI_AV_VIDEO_DEC_MODE_STREAM = 6

} aui_av_video_dec_mode, aui_av_video_dec_mod_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the audio
        stream information for AV-Injector Module
        </div> @endhtmlonly

        Struct to specify the <b> Audio Stream Information </b> for AV-Injector
        Module

@note   This struct is intended @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_av_audio_info {

    /**
    Member to specify the <b> format of the audio stream </b> as defined in the
    enum #aui_deca_stream_type.
    */
    aui_deca_stream_type stream_type;

    /**
    Memner as a @a flag to specify the <b> audio stream channel type </b>, in
    particular the integer values can be
    - @b 1 = Mono
    - @b 2 = Stereo
    */
    int channels;

    /**
    Member to specify the <b> number of bits of information per sample </b>
    as @b bit-depth of the audio stream, which directly corresponds to the
    resolution of each sample
    */
    int bits_per_sample;

    /**
    Member to specify the <b> sample rate </b> of the audio stream
    (in @a Hz unit).

    @note  This member can be set to zero (0) as unknown value
    */
    unsigned long sample_rate;

    /**
    Member to specify the <b> extra data </b> of the audio stream, which are
    helpful for the audio decoding

    @note The extra data are extracted from the container, and its format is
          defined in the specification of the audio format
    */
    unsigned char *extradata;

    /**
    Member to specify the <b> size of the extra data </b> of the audio stream
    */
    unsigned long extradata_size;

    /**
    Member to specify the <b> bit rate </b> of the audio stream which is equals
    to \n\n

    #sample_rate X #bits_per_sample \n\n

    (in @a bps unit).
    */
    unsigned long bit_rate;

    /**
    Member as a @a flag to specify the <b> block alignment </b> of the @b PCM
    audio stream which is equals to \n\n

    (#channels X #bits_per_sample) / 8

    The different values of this flag are listed below:
    - @b 1 = 8-bit Mono
    - @b 2 = 16-bit Mono
    - @b 2 = 8-bit Stereo
    - @b 4 = 16-bit Stereo

    @note  If the stream type as per the enum value #AUI_DECA_STREAM_TYPE_BYE1
           is chosen, this member must be set correctly.
    */
    unsigned long block_align;

    /**
    Member as a @a flag to specify the <b> signedness </b> of the @b PCM audio
    data where, in particular,
    - @b 0 = Unsigned data
    - @b 1 = Signed data

    @note  This member is required @a only by PCM data
    */
    unsigned char sign_flag;

    /**
    Member as a @a flag to specify the <b> endian </b> of the @b PCM audio data
    where, in particular,
    - @b 0 = Little endian
    - @b 1 = Big endian

    @note  This member is required @a only by PCM data
    */
    unsigned char endian;

} aui_av_audio_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the video
        stream information for AV Injector Module
        </div> @endhtmlonly

        Struct to specify the <b> video stream information </b> for AV Injector
        Module

@note   This struct is intended @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_av_video_info {

    /**
    Member to specify the <b> format of the video stream </b> as defined in the
    enum #aui_decv_format
    */
    aui_decv_format stream_type;

    /**
    Member to specify the <b> video injection mode </b> of the video ES data as
    defined in the enum #aui_av_video_dec_mode.
    */
    aui_av_video_dec_mode decode_mode;

    /**
    Member to specify the <b> pictures width </b> of the video stream.
    */
    long pic_width;

    /**
    Member to specify the <b> pictures height </b> of the video stream.
    */
    long pic_height;

    /**
    Member to specify the <b> Sample Aspect Ratio Width </b> of the video stream
    */
    long sar_width;

    /**
    Member to specify the <b> Sample Aspect Ratio Height </b> of the video stream
    */
    long sar_height;

    /**
    Member to specify the <b> frame rate </b> (in @a fps*1000 unit) of the video
    stream
    */
    long frame_rate;

    /**
    Member to specify the @b extra-data of the video stream, which are helpful
    for video decoding

    @note The extra data are extracted from the container, and its format is
          defined in the specification of the video format
    */
    unsigned char *extradata;

    /**
    Member to specify the <b> size of the extra data </b> of the video stream
    */
    long extradata_size;

} aui_av_video_info;

// @coding

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video (AV) Module </b> to specify the information
       for TS
       </div> @endhtmlonly

       Struct to specify the information for TS
*/
typedef struct aui_av_info {

    /**
    Member to specify the <b> Video PID </b>
    */
    unsigned short ui_video_pid;

    /**
    Member to specify the <b> Audio PID </b>
    */
    unsigned short ui_audio_pid;

    /**
    Member to specify the <b> PCR PID </b>
    */
    unsigned short ui_pcr_pid;

    /**
    Member as a @a flag to indicate the configuration of DECA Module and setting
    of the Audio PID into DMX Module (if it is enabled), in particular the integer
    values can be
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned char b_audio_enable;

    /**
    Member as a @a flag to indicate whether perform or not the configuration of
    DECV Module and setting of the Video PID into DMX (if DMX is enabled),
    in particular
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned char b_video_enable;

    /**
    Member as a @a flag to indicate whether configure or not the DMX Module with
    audio, video and PCR PID, in particular
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned char b_dmx_enable;

    /**
    Member as a @a flag to indicate whether set or not the PCR PID into DMX Module
    (if it is enabled), in particular:
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned char b_pcr_enable;

    /**
    Member as a @a flag to indicate whether it is the first time to play a TS
    or not (i.e. stream not changed), in particular:
    - @b 0 = First time
    - @b 1 = Not the first time
    */
    unsigned char b_modify;

    /**
    Member to specify the <b> video stream format </b> as defined in the enum
    #aui_decv_format
    */
    aui_decv_format en_video_stream_type;

    /**
    Member to specify the the <b> audio stream format </b> as defined in the
    enum #aui_deca_stream_type
    */
    aui_deca_stream_type en_audio_stream_type;

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned char b_ttx_enable;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned char b_sub_enable;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned short ui_ttx_pid;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned short ui_sub_pid;

    /**
    Member to specify the <b> audio output data type </b> as defined in the
    enum #aui_en_snd_data_type.

    @warning  This member is not used any more, please use the function
              #aui_snd_out_data_type_set in SND Module to set the audio output
              data type
    */
    aui_en_snd_data_type en_spdif_type;

    /**
    Member to specify the <b> audio output interface type </b> as defined in
    the enum #aui_snd_out_type.

    @warning  This member is not used any more, please use the function
              #aui_snd_out_interface_type_set in SND Module to set the audio
              output interface type
    */
    aui_snd_out_type snd_out_type;

#endif

/// @endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

} aui_av_info, av_info;

// @endcoding

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video (AV) Module </b> to specify the handle of
       AUI Modules/Device used by AV Module
       </div> @endhtmlonly

       Struct to specify the handle of AUI Modules/Device used by AV Module
*/
typedef struct aui_attrAV {

    /**
    Member to specify the handle of @b DECV device
    */
    aui_hdl pv_hdl_decv;

    /**
    Member to specify the handle of @b DECA device
    */
    aui_hdl pv_hdl_deca;

    /**
    Member to specify the handle of @b DMX device
    */
    aui_hdl pv_hdl_dmx;

    /**
    Member to specify the handle of @b SND device

    @warning  This member is not used any more, please use SND Module to set
              the SND Device
    */
    aui_hdl pv_hdl_snd;

    /**
    Member to specify the handle of @b TSI device

    @note  This member is intended @a only for projects based on <b> Linux OS </b>
    */
    aui_hdl pv_hdl_tsi;

    /**
    Member to specify the handle of @b TSG device

    @note  This member is intended @a only for projects based on <b> Linux OS </b>
    */
    aui_hdl pv_hdl_tsg;

    /**
    Member to specify the handle of <b> DIS HD </b> device

    @note  This member is intended @a only for projects based on <b> Linux OS </b>
    */
    aui_hdl pv_hdl_dis_hd;

    /**
    Member to specify the handle of <b> DIS SD </b> device

    @note  This member is intended @a only for projects based on <b> Linux OS </b>
    */
    aui_hdl pv_hdl_dis_sd;

    /**
    Member to specify the pre-configuration of AV module considering the
    information for TS as defined in the struct #av_info
    */
    aui_av_info st_av_info;

    /**
    Member to specify the pre-configuration of AV module considering the setting
    of AV module as defined in the struct #aui_av_inject_settings

    @note  This member is intended @a only for projects based on <b> Linux OS </b>.
    */
    aui_av_inject_settings stream_type;

} aui_attrAV, *aui_pattrAV;

/**
Function pointer to specify the type of callback function registered with the
functions #aui_av_init and #aui_av_de_init and to be called during the
<b> Init/De-Init Stage </b> of AV Module.
*/
typedef void (*aui_funcAVInit) (

    void

    );

/**
Enum to specify the type of configurations to be performed before starting
AV-Module. In particular, this enum is used by the function #aui_av_set to p
erform a specific setting where
- The second parameter @b ul_item takes the item related to the specific
  setting to perform
- The third parameter @b pv_param takes the pointer as per the description
  of the specific setting to perform
*/
typedef enum aui_en_av_item_set {

    /**
    This value is to set the <b> Video PId </b> \n
    The parameter @b pv_param takes the pointer to an integer variable containing
    the Video PID.
    */
    AUI_AV_VIDEO_PID_SET = 0,

    /**
    This value is to set the <b> Audio PID </b> \n
    The parameter @b pv_param takes the pointer to an integer variable containing
    the Audio PID
    */
    AUI_AV_AUDIO_PID_SET,

    /**
    This value is to set the <b> PCR PID </b> \n
    The parameter @b pv_param takes the pointer to an integer variable containing
    the PCR PID
    */
    AUI_AV_PCR_PID_SET,

    /**
    This value is to set the <b> Video Format </b> \n
    The parameter @b pv_param takes the pointer to a video format defined in the
    enum #aui_decv_callback.
    */
    AUI_AV_VIDEO_TYPE_SET,

    /**
    This value is to set the <b> Audio Format </b> \n
    The parameter @b pv_param takes the pointer to a audio format defined in the
    enum #aui_audio_stream_type.
    */
    AUI_AV_AUDIO_TYPE_SET,

    /**
    This value is to set a @a flag to indicate whether it is the first time
    to <b> play a TS </b> or not (i.e. stream not changed).\n
    The parameter @b pv_param takes the pointer of an integer variable which
    values can be
    - @b 0 = First time
    - @b 1 = Not the first time
    */
    AUI_AV_MODIFY_SET,

    /**
    This value is to enable/disable the <b> DMX Module/Device </b> in AV Module.\n
    The parameter @b pv_param takes the pointer of an integer variable as a
    @a flag which values can be
    - @b 0 = Disable
    - @b 1 = enable
    */
    AUI_AV_DMX_ENABLE,

    /**
    This value is to enable/disable the @b Video in AV module. \n
    The parameter @b pv_param takes the pointer of an integer variable as a
    @a flag which values can be
    - @b 0 = Disable
    - @b 1 = Enable (i.e. configure video and set the video PID to DMX
             Module/Device)
    */
    AUI_AV_VIDEO_ENABLE,

    /**
    This value is to enable/disable the @b audio in AV module. \n
    The parameter @b pv_param takes the pointer of an integer variable
    as a @a flag which values can be
    - @b 0 = Disable
    - @b 1 = Enable (i.e. configure audio and set the audio PID to DMX
             Module/Device)
    */
    AUI_AV_AUDIO_ENABLE,

    /**
    This value is to set/unset the <b> PCR PID </b> to <b> DMX Module/
    Device </b> in AV module. \n
    The parameter @b pv_param takes the pointer of an integer variable
    as a @a flag which values can be
    - @b 0 = Disable
    - @b 1 = Enable
    */
    AUI_AV_PCR_ENABLE,

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_SPDIF_TYPE_SET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_TTX_ENABLE,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_SUB_ENABLE,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_TTX_PID_SET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_SUB_PID_SET

#endif

/// @endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

} aui_av_item_set;

/**
Enum to specify how to get the information of AV-Module. In particular, this
enum is used by the function #aui_av_get to get a specific information where:
- The second parameter @b ul_item takes the item related to the specific
  information to get
- The third parameter @b pv_param takes the pointer as per the description
  of the specific information to get
*/
typedef enum aui_en_av_item_get {

    /**
    This value is to get the <b> video PID </b>. \n
    The parameter @b pv_param takes the pointer to an integer variable
    containing the Video PID
    */
    AUI_AV_VIDEO_PID_GET = 0,

    /**
    This value is to get the <b> Audio PID </b>. \n
    The parameter @b pv_param takes the pointer to an integer variable
    containing the Audio PID
    */
    AUI_AV_AUDIO_PID_GET,

    /**
    This value is to get the <b> PCR PID </b>. \n
    The parameter @b pv_param takes the pointer to an integer variable
    containing the PCR PID
    */
    AUI_AV_PCR_PID_GET,

    /**
    This value is to get the <b> Video Format </b>. \n
    The parameter @b pv_param takes the pointer to a video format defined in
    the enum #aui_decv_callback.
    */
    AUI_AV_VIDEO_TYPE_GET,

    /**
    This value is to get the <b> Audio Format </b>. \n
    The parameter pv_param takes the pointer to a audio format defined in the
    enum #aui_audio_stream_type.
    */
    AUI_AV_AUDIO_TYPE_GET,

    /**
    This value is to get the @a flag which indicates whether it is the first
    time to play a TS or not (i.e. stream not changed). \n
    The parameter @b pv_param takes the pointer of an integer variable which
    values can be:
    - @b 0 = First time
    - @b 1 = Not the first time
    */
    AUI_AV_MODIFY_GET,

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                    START                                  */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_SPDIF_TYPE_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_TTX_PID_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_AV_SUB_PID_GET

#endif

/// @endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

} aui_av_item_get;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video (AV) Module </b> to specify the parameters
       to initialize AUI Modules used by AV-Module
       </div> @endhtmlonly

       Struct to specify the parameters to initialize AUI Modules used by AV
       Module

@note  This struct is intended @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_av_stream_info {

    /**
    Member to specify the initial information for playing TS as defined in the
    struct #av_info

    @note  This member is not applicable when inject ES then it @a must be empty
    */
    aui_av_info st_av_info;

    /**
    Member to specify the initial information for injecting Audio ES as defined
    in the struct #aui_av_audio_info
    */
    aui_av_audio_info *p_audio_parameters;

    /**
    Member to specify the initial information for injecting Video ES as defined
    in the struct #aui_av_video_info
    */
    aui_av_video_info *p_video_parameters;

    /**
    Member to specify the pre-configuration of AV module considering the setting
    of AV module as defined in the struct #aui_av_inject_settings
    */
    aui_av_inject_settings stream_type;

} aui_av_stream_info;

// @coding

typedef struct aui_decoder_buffer_status aui_av_buffer_status;

// @endcoding

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video (AV) Module </b> to specify the parameters
       for flushing AUI Modules used by AV-Module
       </div> @endhtmlonly

       Struct to specify the parameters for flushing AUI Modules used by AV-Module

@note  This struct is intended @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_av_flush_setting {

    /**
    Member to specify which channels has to be flushed as defined in the enum
    #aui_av_buffer_channel.
    */
    aui_av_buffer_channel buffer_channel;

    /**
    Member to specify the Current Video Playback Mode for injecting ES as defined
    in the enum #vdec_playback_mode.
    */
    aui_vdec_playback_mode playback_mode;

} aui_av_flush_setting;

/**
Enum to specify the trick mode of AV module.

@note @b 1. Trick mode is @a only applicable for local TS playback (i.e. when
            the data type to be injected into AV Module is set to the enum value
            #AUI_AV_DATA_TYPE_RAM_TS)

@note @b 2. This struct is intended @a only for projects based on <b> Linux OS </b>
*/
typedef enum aui_av_trick_mode {

    /**
    Value to specify that the playback is in normal mode
    (i.e. normal speed and forward)
    */
    AUI_AV_TRICK_MODE_NONE,

    /**
    Value to specify that the control of the playback speed and direction is
    performed by the user application
    */
    AUI_AV_TRICK_MODE_APP,

    /**
    Value to specify that the control of the playback speed is performed by the
    DECV Module/Device

    @note  At present this value is not supported
    */
    AUI_AV_TRICK_MODE_DECODER

} aui_av_trick_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the parameters
        for setting the trick mode of AV module
        </div> @endhtmlonly

        Struct to specify the parameters for setting the trick mode of AV module

@note   @b 1. A trick mode is @a only applicable for local TS playback
              (i.e. when the data type to be injected into AV Module is set to the
              enum value #AUI_AV_DATA_TYPE_RAM_TS)

@note   @b 2. This struct is intended @a only for projects based on <b> Linux
              OS </b>
*/
typedef struct aui_av_trickmode_params {

    /**
    Member to specify the trick mode of AV module
    */
    aui_av_trick_mode mode;

    /**
    Member to specify the speed of the video playback
    */
    aui_playback_speed speed;

    /**
    Member to specify the direction of the video playback
    */
    aui_playback_direction direction;

    /**
    Member to specify the bit rate (in @a bps unit) of the TS and used to estimate
    the member @b ul_chunk_size of the struct #aui_av_trickmode_status
    */
    unsigned long ul_bitrate;

} aui_av_trickmode_params;

/**
Enum to specify the next action to be done in trick mode of AV module.

@note   @b 1. If the playback speed is greater than #AUI_PLAYBACK_SPEED_8 or
              the playback direction is backward when the video decoder gets
              one I-frame, then the video decoder needs to get the next one.\n
              In this case #AUI_AV_TRICK_MODE_ACTION_NEW_FRAME will be set. \n
              In other case, just write data into the video decoder continuously
              then #AUI_AV_TRICK_MODE_ACTION_MORE_DATA will be set.

@note   @b 2. A trick mode is @a only applicable for local TS playback
              (i.e. when the data type to be injected into AV Module is set to the
              enum value #AUI_AV_DATA_TYPE_RAM_TS)

@note   @b 3. This struct is intended @a only for projects based on <b> Linux
              OS </b>
*/
typedef enum aui_av_trickmode_action {

    /**
    Value to specify that the user application needs to send the TS nearest to
    the next I-frame,

    @note   File seeking may be necessary
    */
    AUI_AV_TRICK_MODE_ACTION_NEW_FRAME,

    /**
    Value to specify that the user application needs to continue to send more data
    */
    AUI_AV_TRICK_MODE_ACTION_MORE_DATA

} aui_av_trickmode_action;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the status of
        the trick mode of AV module
        </div> @endhtmlonly

        Struct to specify the parameters for setting the trick mode of AV module

@note   @b 1. A trick mode is @a only applicable for local TS playback
              (i.e. when the data type to be injected into AV Module is set to the
              enum value #AUI_AV_DATA_TYPE_RAM_TS)

@note   @b 2. This struct is intended @a only for projects based on <b> Linux
              OS </b>
*/
typedef struct aui_av_trickmode_status {

    /**
    Member to specify the recommended size of data to be written by the function
    #aui_av_write based on the member @b ul_bitrate of the struct
    #aui_av_trickmode_params
    */
    unsigned long ul_chunk_size;

    /**
    Member to specify the next action to be done as defined in the enum
    #aui_av_trickmode_action
    */
    aui_av_trickmode_action action;

} aui_av_trickmode_status;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to open the AV Module and configure the desired
                attributes then get the related handle.

@warning        @b 1. This function can @a only be used in the <b> Pre-Run stage
                      </b> of the AV Module, in particular:
                      - Either after performing the initialization of the AV Module
                        by the function #aui_av_init for the first opening of the AV
                        Module,
                      - Or after closing the AV Module by the function #aui_av_close,
                        considering the initialization of the AV Module has previously
                        been performed by the function #aui_av_init

@warning        @b 2. Before calling this function, user needs to open and
                      configure the AUI Modules used in AV module
                      - Either using the APIs available in the other AUI Modules
                      - Or using the function #aui_av_init_attr, where this option is
                        @a only applicable in projects based on <b> Linux OS </b>

@param[in]      p_attrAV               = Pointer to a struct #aui_attrAV, which
                                         collects the desired attributes for
                                         the AV Module

@param[out]     ppv_handleAV           = #aui_hdl pointer to the handle of the
                                         AV Module just opened.

@return         @b AUI_RTN_SUCCESS     = AV Module opened successfully then
                                         user can configure or start AV Module
@return         @b Other_Values        = Opening of the AV Module failed for
                                         some reasons
*/
AUI_RTN_CODE aui_av_open (

    aui_attrAV *p_attrAV,

    aui_hdl *ppv_handleAV

    );

 /**
@brief          Function used to close the AV Module already opened by the function
                #aui_av_open then the related AV handle will be released
                (i.e. the related resources such as memory, device).

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the AV Module in pair with its opening by the function
                #aui_av_open. After closing the AV Module, user can:
                - Either perform the De-Initialization of the AV Module by the
                  function #aui_av_de_init
                - Or open again the AV Module by the function #aui_av_open

@param[in]      pv_handleAV             = #aui_hdl the handle of the AV Module
                                          already opened and to be closed.

@return         @b AUI_RTN_SUCCESS      = AV Module closed successfully
@return         @b Other_Values         = Closing of the AV Module faile
                                          for some reasons

@note           In projects based on <b> Linux OS </b>, the handle of AUI Modules
                opened by the function #aui_av_init_attr will be released as well
*/
AUI_RTN_CODE aui_av_close (

    aui_hdl pv_handleAV

    );

/**
@brief          Function used to start the AV Module already opened by the
                function #aui_av_open.\n
                After starting the AV Module, the stream will start to play.

@warning        Some configuration about Audio/Video/DMX have already been
                performed after opening the AV Module.All AUI Modules used
                in AV module can be configured by their own APIs if necessary
@warning        The function #aui_dmx_data_path_set must be called before this
                function, in order to set the correct DSC to be used for the
                stream decrypting.

@param[in]      pv_handleAV             = #aui_hdl handle of the AV Module
                                          already opened and to be started

@return         @b AUI_RTN_SUCCESS      = AV Module started successfully
@return         @b Other_Values         = Starting of the AV Module failed
                                          for some reasons
*/
AUI_RTN_CODE aui_av_start (

    aui_hdl pv_handleAV

    );

/**
@brief          Function used to stop the AV Module already opened and started
                by, respectively, the functions #aui_av_open and #aui_av_start.\n
                After stopping the AV Module, the stream will stop playing then
                user can:
                - Either close the AV Module by the function #aui_av_close if
                  no need to use the AV Module any more
                - Or start again the AV Module by the function #aui_av_start,
                  considering the supposed/suggested configurations of the AUI
                  Modules used by the AV Module have been performed again before
                  starting the AV Module, if user wants to play a new stream.

@param[in]      pv_handleAV             = #aui_hdl handle of the AV Module already
                                          opened and started then to be stopped

@return         @b AUI_RTN_SUCCESS      = AV Module stopped successfully.
@return         @b Other_Values         = Stopping of the AV Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_av_stop (

    aui_hdl pv_handleAV

    );

/**
@brief          Function used to pause the AV Module already opened and started
                by, respectively, the functions #aui_av_open and #aui_av_start.\n
                After pausing the AV Module, the last frame of the video will
                be freeze on the screen and the audio will pause, and two (2)
                actions can be performed as below:
                - Resuming the AV module by the function #aui_av_resume
                - Stopping the AV module by the function #aui_av_stop

@param[in]      pv_handleAV             = #aui_hdl handle of the AV Module already
                                          opened and started then to be paused

@return         @b AUI_RTN_SUCCESS      = AV Module paused successfully.
@return         @b Other_Values         = Pausing of the AV Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_av_pause (

    aui_hdl pv_handleAV

    );

/**
@brief          Function used to resume the AV Module already opened, started
                and paused by, respectively, the functions #aui_av_open,
                #aui_av_start and #aui_av_pause.\n
                After resuming the AV Module, the video and audio will play
                again and two (2) actions can be performed as below:
                - Pausing again by the function #aui_av_pause
                - Stopping the video by the function #aui_av_stop

@param[in]      pv_handleAV             = #aui_hdl handle of the AV Module already
                                          opened,started and paused then to be
                                          resumed

@return         @b AUI_RTN_SUCCESS      = AV Module resumed successfully
@return         @b Other_Values         = Resuming of the AV Module failed for
                                          some reasons.
*/
AUI_RTN_CODE aui_av_resume (

    aui_hdl pv_handleAV

    );

/**
@brief          Function used to perform the initialization of the AV Module
                before its opening by the function #aui_av_open.

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the AV Module.

@param[in]      fnAVInit                = Callback function used to initialize
                                          the AV Module, as per comment for the
                                          function pointer #aui_funcAVInit.

@return         @b AUI_RTN_SUCCESS      = AV Module initialized successfully,
@return         @b Other_Values         = Initializing of the AV Module failed
                                          for some reasons

@note           In projects based on <b> Linux OS </b>, after calling this function,
                user can open and configure the AUI Modules used in the AV module
                by the function #aui_av_init_attr
*/
AUI_RTN_CODE aui_av_init (

    aui_funcAVInit fnAVInit

    );

/**
@brief          Function used to open and configure the modules used in the AV
                Module before its opening by the function #aui_av_open.

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the AV Module.

@param[out]     p_attrAV                = Pointer to a struct #aui_attrAV, which
                                          collects the desired attributes for the
                                          AV Module.
                                          - @b Caution: It @a must be the same
                                            as the input parameter of the
                                            function #aui_av_open

@param[in]      p_av_stream_info        = Pointer to parameters for initializing
                                          AV Module
                                          - @b Caution: Some parameters will be
                                            stored into @b p_attrAV

@return         @b AUI_RTN_SUCCESS      = AV Module initialized successfully
@return         @b Other_Values         = Initializing of the AV Module failed
                                          for some reasons

@note           This function in @a only applicable in projects based on <b> Linux
                OS </b>.\n
                User can use this function to initialize the AUI modules to be
                used for playing except DSC Module with the specified or default
                parameters
*/
AUI_RTN_CODE aui_av_init_attr (

    aui_attrAV *p_attrAV,

    aui_av_stream_info *p_av_stream_info

    );

/**
@brief          Function used to perform the de-initialization of the AV Module
                after its closing by the function #aui_av_close.

@param[in]      fnAVDe_init             = Callback function used to de-initialize
                                          the AV Module, as per comment for the
                                          function pointer #aui_funcAVInit.

@return         @b AUI_RTN_SUCCESS      = AV Module de-initialized successfully.
@return         @b Other_Values         = De-initializing of the AV Module failed
                                          for some reasons
*/
AUI_RTN_CODE aui_av_de_init (

    aui_funcAVInit fnAVDe_init

    );

/**
@brief          Function used to perform a specific setting of the AV Module
                (as defined in the enum #aui_av_item_set) between its opening
                and starting by, respectively, the functions #aui_av_open and
                #aui_av_start.

@param[in]      pv_hdlAV                =  #aui_hdl handle of the AV Module
                                           already opened and to be managed to
                                           perform a specific setting
@param[in]      ul_item                 =  The item related to the specific
                                           setting of the AV Module to be performed,
                                           as defined in the enum #aui_av_item_set
@param[in]      *pv_param               =  The pointer as per the description of
                                           the specific setting of the AV Module
                                           to be performed, as defined in the enum
                                           #aui_av_item_set

@return         @b AUI_RTN_SUCCESS      =  Specific setting of the AV Module
                                           performed successfully
@return         @b Other_Values         =  Specific setting of the AV Module
                                           failed for some reasons
*/
AUI_RTN_CODE aui_av_set (

    aui_hdl pv_hdlAV,

    aui_av_item_set ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information of the AV Module
                (as defined in the enum #aui_av_item_get) after its opening
                and starting by, respectively, the functions #aui_av_open and
                #aui_av_start

@param[in]      pv_hdlAV                =  #aui_hdl handle of the AV Module
                                           already opened and to be managed
                                           to get a specific information,
                                           as defined in the enum #aui_av_item_get
@param[in]      ul_item                 =  The item related to the specific
                                           information of the AV Module to be
                                           gotten, as defined in the enum
                                           #aui_av_item_get

@param[out]     *pv_param               =  The pointer as per the description of
                                           the specific information of the AV
                                           Module to be gotten, as defined in
                                           the enum #aui_av_item_get

@return         @b AUI_RTN_SUCCESS      =  Getting of the specific information
                                           of the AV Module performed successfully
@return         @b Other_Values         =  Getting of the specific information
                                           of the AV Module failed for some reasons
*/
AUI_RTN_CODE aui_av_get (

    aui_hdl pv_hdlAV,

    aui_av_item_get ul_item,

    void *pv_param

    );

/**
@brief          Function used to flush the AV Module after its opening and
                starting by, respectively, the functions #aui_av_open and
                #aui_av_start.\n
                After using this function, the data in the buffer channel
                specified by the parameter @b setting is discarded and the
                specified channel is ready to receive new data

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module
                                          already opened and started then to be
                                          flushed.
@param[in]      p_setting               = The pointer to the flush parameters
                                          for AV Module, as defined in the struct
                                          #aui_av_flush_setting

@return         @b AUI_RTN_SUCCESS      = Flushing the AV Module performed
                                          successfully
@return         @b Other_Values         = Flushing the AV Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_av_flush (

    aui_hdl pv_hdlAV,

    aui_av_flush_setting *p_setting

    );

/**
@brief          Function used to inject TS/ES to the AV Module after its opening
                and starting by, respectively, the functions #aui_av_open and
                #aui_av_start

@warning        The application @b must always call the function #aui_av_buffer_status_get
                to make sure there is enough underlying free buffer space before
                calling this function which, however, will wait until there is free space
                for writing.

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module already
                                          opened and started then to be injected
@param[in]      p_packet_info           = The pointer to the packet information
                                          of the ES injection as defined in the
                                          struct #aui_av_inject_packet_info_t
                                          - @b Caution: For TS injection, it can
                                            be set as NULL
@param[in]      puc_buf                 = The pointer to the buffer containing
                                          the data to be injected, which
                                          - In case of TS injection, contains
                                            the TS packet including TS header
                                            and TS data
                                          - In case of ES injection, contains
                                            the ES data
@param[in]      ul_size                 = Size of the data in the buffer

@return         @b AUI_RTN_SUCCESS      = Injecting of the the TS/ES packet
                                          to the AV Module performed successfully
@return         @b Other_Values         = Injecting of the TS/ES packet to the AV
                                          Module failed for some reasons

@note           @b 1. When the type of data to be injected into AV Module is set
                      to the enum value #AUI_AV_DATA_TYPE_NIM_TS, this function will
                      return successfully without any effect

@note           @b 2. This function is @a only applicable in projects based on
                      <b> Linux OS </b>.
*/
AUI_RTN_CODE aui_av_write (

    aui_hdl pv_hdlAV,

    aui_av_inject_packet_info *p_packet_info,

    const unsigned char *puc_buf,

    unsigned long ul_size

    );

/**
@brief          Function used to set playback in trick mode into the AV Module
                after its opening and starting by, respectively, the functions
                #aui_av_open and #aui_av_start.

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module already
                                          opened and started then to be set for
                                          the trick mode
@param[in]      p_params                = The pointer to the trick mode parameters
                                          as defined in the struct #aui_av_trickmode_params

@return         @b AUI_RTN_SUCCESS      = Setting of the the trick mode into the
                                          AV Module performed successfully
@return         @b Other_Values         = Setting of the trick mode into the AV
                                          Module failed for some reasons.

@note           This function is @a only applicable in projects based on <b>
                Linux OS </b> and the type of data to be injected into AV Module
                is set to the enum value #AUI_AV_DATA_TYPE_RAM_TS
*/
AUI_RTN_CODE aui_av_trickmode_set (

    aui_hdl pv_hdlAV,

    aui_av_trickmode_params *p_params

    );

/**
@brief          Function used to get the status of the playback in trick mode of
                the AV Module after its setting by the function #aui_av_trickmode_set

@warning        After getting the status of playback in trick mode, user can use
                the function #aui_av_write to inject more data depending on the
                return value of this funtion. Please refer to the enum
                #aui_av_trickmode_action for more information

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module already
                                          opened and started then to be managed
                                          for getting the status of the playback
                                          in trick mode already set
@param[out]     p_status                = The pointer to the status of the playback
                                          in trick mode as defined in the enum
                                          #aui_av_trickmode_status

@return         @b AUI_RTN_SUCCESS      = Getting of the status of the playback
                                          in trick mode of the AV Module performed
                                          successfully
@return         @b Other_Values         = Getting of the status of the playback
                                          in trick mode of the AV Module failed
                                          for some reasons

@note           This function is @a only applicable in projects based on <b>
                Linux OS </b> and the type of data to be injected into AV Module
                is set to the enum value #AUI_AV_DATA_TYPE_RAM_TS
*/
AUI_RTN_CODE aui_av_trickmode_status_get (

    aui_hdl pv_hdlAV,

    aui_av_trickmode_status *p_status

    );

/**
@brief          Function used to get the status of the buffer used in the AV Module
                after its opening and starting by, respectively, the functions
                #aui_av_open and #aui_av_start.

@warning        The application @b must always call this function to make sure there
                is enough underlying free buffer space before calling the function
                #aui_av_write

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module already
                                          opened and started then to be managed
                                          for getting the buffer status
@param[in]      buffer_channel          = The specific buffer channel of which
                                          needs to know the status

@param[out]     p_status                = The pointer to the status of the specific
                                          buffer channel

@return         @b AUI_RTN_SUCCESS      = Getting of the status of specific buffer
                                          channel in the AV Module performed
                                          successfully
@return         @b Other_Values         = Getting of the status of specific buffer
                                          channel in the AV Module failed for
                                          some reasons

@note           This function is @a only applicable in projects based on <b>
                Linux OS </b>
*/
AUI_RTN_CODE aui_av_buffer_status_get (

    aui_hdl pv_hdlAV,

    aui_av_buffer_channel buffer_channel,

    aui_av_buffer_status *p_status

    );

/**
@brief          Function used to set the DSC context in AV Module

@note           The parameter @b p_dsc_context of this function will be used
                when the data channel is set to
                - #AUI_AV_BUFFER_CHANNEL_AUD_DECODER_FOR_ENCRYPTED_DATA
                - #AUI_AV_BUFFER_CHANNEL_VID_DECODER_FOR_ENCRYPTED_DATA

@note           as defined in the enum #aui_av_buffer_channel

@param[in]      pv_hdlAV                = #aui_hdl handle of the AV Module already
                                          opened and started
@param[in]      p_dsc_context           = The pointer to the DSC context in AV Module
                                          as defined in the struct #aui_av_dsc_context

@return         @b AUI_RTN_SUCCESS      = Setting of the DSC context in AV Module
                                          performed successfully
@return         @b Other_Values         = Setting of the DSC context in AV Module
                                          failed for some reasons

@note           This function is @a only applicable in projects based on
                <b> Linux OS </b>, and the type of data to be injected in AV Module
                is set to the enum value #AUI_AV_DATA_TYPE_RAM_ES
*/
AUI_RTN_CODE aui_av_dsc_context_set (

    aui_hdl pv_hdlAV,

    aui_av_dsc_context *p_dsc_context

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_av_data_type_e aui_av_data_type

#define aui_av_buffer_channel_t aui_av_buffer_channel

#define aui_av_inject_settings_s aui_av_inject_settings

#define aui_av_video_dec_mod_e aui_av_video_dec_mode

#define aui_av_audio_info_s aui_av_audio_info

#define aui_av_audio_info_t aui_av_audio_info

#define aui_av_video_info_s aui_av_video_info

#define aui_av_video_info_t aui_av_video_info

#define aui_av_flush_setting_s aui_av_flush_setting

#define aui_av_flush_setting_t aui_av_flush_setting

#define aui_av_trickmode_params_t aui_av_trickmode_params

#define aui_av_stream_info_t aui_av_stream_info

#endif

///@endcond

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


