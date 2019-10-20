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
Current Author:     Alfa.Shang, Phill.Hsu
Last update:        2017.05.08
-->

@file   aui_mp.h

@brief  Media Player (MP) Module

        Media Player (MP) Module is used to play the stream from either external
        attachment or network.\n\n

        The available stream formats can be found with the enum #aui_en_dec_format,
        however the stream format @b AVS can not be supported by the MP Module
        presently.\n

        The stream from network is @a only available for projects based on <b>
        Linux OS </b>

        MP Module @b must cooperate with its own file system.\n
        To play the stream file, user needs to have the path of the stream file
        as input.

@note   For further details, please refer to ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Media Player Module"
        </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_MP_H

#define _AUI_MP_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Presently the structs contained in this header file are designed to support up
to one (1) MP device
*/
#define AUI_MP_DEV_CNT_MAX (1)

/*******************************Global Type List*******************************/

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Media Player (MP) Module </b> to specify the basic
        stream attributes
        </div> @endhtmlonly

        Struct to specify the basic stream attributes

@note   This struct is available @a only for projects based on <b> TDS OS </b>
*/
typedef struct aui_mp_video_info {

    /**
    Member as an @a array to specify the group of all <b> video decoder </b>
    currently supported by Media Player Module and for which ALi R&D Dept.
    can provide support on demand, i.e.
    - @b MP43
    - @b XIV1
    - @b XIV2
    - @b XVID
    - @b Unknown (i.e. unsupported video decoder)
    */
    char video_dec[10];

    /**
    Member as an @a array to specify the group of all <b> audio decoder </b>
    currently supported by Media Player Module and for which ALi R&D Dept.
    can provide support on demand, i.e.
    - @b PCM
    - @b MP3
    - @b AC3
    - @b DTS
    - @b Unknown (Unsupported audio decoder)
    */
    char audio_dec[10];

    /**
    Member to specify the <b> tracked audio stream ID </b> from the media stream

    @note   One media stream may have many audio stream ID
    */
    unsigned long audio_stream_num;

    /**
    Member to specify the <b> subtitle ID </b> from the media stream

    @note   One media stream may have many subtitle ID
    */
    unsigned long sub_stream_num;

    /**
    Member to specify the total number of @b frames from the media stream
    */
    unsigned long total_frame_num;

    /**
    Member to specify the total @b time for <b> playing one frame </b>
    */
    unsigned long frame_period;

    /**
    Member to specify the total @b time of the <b> media stream </b> under
    playing
    */
    unsigned long total_time;

    /**
    Member to specify the <b> video pictures width </b>

    @note   The maximum width supported by the video decoder is @b 720
    */
    int   width;

    /**
    Member to specify the <b> video picture height </b>

    @note   The maximum height supported by the video decoder is @b 576
    */
    int   height;

    /**
    Member to specify the <b> video bit rate </b> (in bps) of the media stream
    */
    unsigned long video_bitrate;

    /**
    Member to specify the <b> audio bit rate </b> (in @a bps unit ) of the media
    stream
    */
    unsigned long audio_bitrate;

    /**
    Member to specify the total number of <b> audio channels </b>
    */
    unsigned long audio_channel_num;

    /**
    Member to specify the size (in  @a byte unit) of the video file
    */
    long long file_size;

} aui_mp_video_info;

/**
Enum to specify the different <b> play speeds </b> of the media stream,
in particular
- Normal
- Fastrewind
- Slowrewind
- Slowforward
- Fastforward
- Pause

To set the desired play speed, please use the function #aui_mp_speed_set
*/
typedef enum aui_mp_speed {

    /**
    Value to specify the @b fastrewind speed is @b 24 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTREWIND_24 = 1,

    /**
    Value to specify the @b fastrewind speed is @b 16 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTREWIND_16,

    /**
    Value to specify the @b fastrewind speed is @b 8 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTREWIND_8,

    /**
    Value to specify the @b fastrewind speed is @b 4 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTREWIND_4,

    /**
    Value to specify the @b fastrewind speed is @b 2 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTREWIND_2,

    /**
    Value to specify the @b slowrewind speed is @b 2 times slower than the
    normal speed

    @note This value is available @a only for projects based on <b> TDS OS </b>,
          anyway is not supported currently
    */
    AUI_MP_SPEED_SLOWREWIND_2,

    /**
    Value to specify the @b slowrewind speed is @b 4 times slower than the
    normal speed

    @note   This value is available @a only for projects based on <b> TDS OS </b>,
            anyway is not supported currently
    */
    AUI_MP_SPEED_SLOWREWIND_4,

    /**
    Value to specify the @b slowrewind speed is @b 8 times slower than the
    normal speed

    @note   This value is available @a only for projects based on <b> TDS OS </b>,
            anyway is not supported currently
    */
    AUI_MP_SPEED_SLOWREWIND_8,

    /**
    Value to specify the @b slowforward speed is @b 2 times slower than the
    normal speed

    @note This enum value is available @a only for projects based on <b> TDS
          OS </b>
    */
    AUI_MP_SPEED_SLOWFORWARD_2,

    /**
    Value to specify the @b slowforward speed is @b 4 times slower than the
    normal speed

    @note This enum value is available @a only for projects based on <b> TDS
          OS </b>
    */
    AUI_MP_SPEED_SLOWFORWARD_4,

    /**
    Value to specify the @b slowforward speed is @b 8 times slower than the
    normal speed

    @note This enum value is available @a only for projects based on <b> TDS
          OS </b>
    */
    AUI_MP_SPEED_SLOWFORWARD_8,

    /**
    Value to specify the @b fastforward speed is @b 2 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTFORWARD_2,

    /**
    Value to specify the @b fastforward speed is @b 4 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTFORWARD_4,

    /**
    Value to specify the @b fastforward speed is @b 8 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTFORWARD_8,

    /**
    Value to specify the @b fastforward speed is @b 16 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTFORWARD_16,

    /**
    Value to specify the @b fastforward speed is @b 24 times faster than the
    normal speed
    */
    AUI_MP_SPEED_FASTFORWARD_24,

    /**
    Value to specify @b normal speed
    */
    AUI_MP_SPEED_1,

    /**
    Value to specify the @b pause
    */
    AUI_MP_SPEED_0

} aui_mp_speed;

/**
Function pointer to specify the type of callback function registered with the
functions #aui_mp_init and #aui_mp_de_init, and to be called during the
<b> Initialization/De-Initialization stage </b> of Media Player Module
*/
typedef void (*aui_func_mp_init) (

    void

    );

/**
Enum to specify different input items of the function #aui_mp_set to be used
to set some attributes for Media Player driver
*/
typedef enum aui_mp_item_set {

    /**
    Value to specify the <b> program index </b> needs to be set for Media
    Player driver (one media file @a only contains multiple programs)

    @note This enum is available @a only for projects based on <b> TDS OS </b>
    */
    AUI_MP_SET_VIDEO_ID,

    /**
    Value to specify the audio <b> track index </b> needs to be set for Media
    Player driver (one stream may contains many audio tracks)
    */
    AUI_MP_SET_AUDIO_ID,

    /**
    Value to specify the <b> subtitle track index </b> needs to be set for Media
    Player driver (one stream may contains many subtitle tracks)
    */
    AUI_MP_SET_SUBTITLE_ID,

    /**
    Value used to @b register a <b> File Descriptor Call Function </b>

    @note This value is available @a only for projects based on <b> TDS OS </b>
    */
    AUI_MP_SET_FD_CB_REG,

    /**
    Value used to @b un-register a <b> File Descriptor Call Function </b>

    @note   @b 1. The un-registering can be performed simply by setting the
                  File Descriptor Call Function to NULL.

    @note   @b 2. This value is available @a only for projects based on
                  <b> TDS OS </b>
    */
    AUI_MP_SET_FD_CB_UNREG

} aui_mp_item_set;

/*
This struct is @b deprecated then can be ignored
*/
typedef enum aui_mp_item_get {

    AUI_MP_GET_STREAM_INFO_SPEED,

    AUI_MP_GET_STREAM_INFO = 136

} aui_mp_item_get;


/**
Enum to specify all possible messages for Media Player Module callback
function

@note  All messages are sent to an application by the function
       #aui_mp_set_message_callback, then the application can perform the
       corresponding action
*/
typedef enum aui_mp_message {

    /**
    Value to specify the video file starts to be played
    */
    AUI_MP_PLAY_BEGIN,

    /**
    Value to specify the end of the video file
    */
    AUI_MP_PLAY_END,

    /**
    Value to specify the current video decoder type is not supported
    */
    AUI_MP_VIDEO_CODEC_NOT_SUPPORT,

    /**
    Value to specify the current audio decoder type is not supported
    */
    AUI_MP_AUDIO_CODEC_NOT_SUPPORT,

    /**
    Value to specify the current resolution is not supported
    */
    AUI_MP_RESOLUTION_NOT_SUPPORT,

    /**
    Value to specify the current framerate is not supported
    */
    AUI_MP_FRAMERATE_NOT_SUPPORT,

    /**
    Value to specify the memory to play the current video file is not enough
    then need more
    */
    AUI_MP_NO_MEMORY,

    /**
    Value to specify a video decoder error occurred
    */
    AUI_MP_DECODE_ERROR,

    /**
    Value to specify an unknown error occurred
    */
    AUI_MP_ERROR_UNKNOWN,

    /**
    Value to specify the percentage of the buffering for stream video data

    @note This message is available @a only for projects based on <b> Linux
          OS </b>
    */
    AUI_MP_BUFFERING,

    /**
    Value to specify an error from soup http connection is occurred

    @note  This message is available @a only for projects based on <b> Linux
           OS </b>
    */
    AUI_MP_ERROR_SOUPHTTP,

    /**
    Value to specify the frames capturing is finished and the returned path
    for the captured frames has been gotten

    @note This message is available @a only for projects based on <b> Linux
          OS </b>
    */
    AUI_MP_FRAME_CAPTURE,

    /**
    Value to specify the maximum number of messages available for Media Player
    Module callback function
    */
    AUI_MP_MESSAGE_MAX

} aui_mp_message;

/**
Enum to specify the available stream protocols

@note This enum is @a only for projects based on <b> Linux OS </b>
*/
typedef enum aui_mp_stream_protocol {

    /**
    Value to specify an unknown stream protocol

    @note  This value is the best choice when really the stream protocoll is
           unknown by the user
    */
    AUI_MP_STREAM_PROTOCOL_UNKNOW,

    /**
    Value to specify the stream protocol is live play
    */
    AUI_MP_STREAM_PROTOCOL_LIVE,

    /**
    Value to specify the stream protocol is @b VOD (Video of Demand)
    */
    AUI_MP_STREAM_PROTOCOL_VOD

} aui_mp_stream_protocol;

/**
Enum to specify the different types of stream information such as
- Audio
- Video
- Subtitle

where for every type user can
- Use the function #aui_mp_get_stream_info or #aui_mp_get_cur_stream_info to get
  the corresponding stream information
- Use the function #aui_mp_free_stream_info to free the buffer where the stream
  information are stored

@note This enum is @a only used for projects based on <b> Linux OS </b>
*/
typedef enum aui_mp_stream_info_type {

    /**
    Value to specify the stream information type related to @b audio
    */
    AUI_MP_STREAM_INFO_TYPE_AUDIO,

    /**
    Value to specify the stream information type related to @b subtitle
    */
    AUI_MP_STREAM_INFO_TYPE_SUBTITLE,

    /**
    Value to specify the stream information type related to @b video
    */
    AUI_MP_STREAM_INFO_TYPE_VIDEO,

    /**
    Value to specify the stream information type related to <b> media size </b>
    */
    AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE,

} aui_mp_stream_info_type;

/// @coding

/**
Function pointer to specify the @b type of callback function registered with
the function #aui_mp_set_message_callback to transfer the message notifications
defined in the enum #aui_mp_message

@note   To define MP callback, it is recommended using the function pointer
        @b aui_mp_message_callback instead of this one.
*/
typedef void (*pn_message_callback) (

    void

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b open a
media file, where the input paramete are explained below:
- @b pc_filename      = Handle of the input file
- @b puc_buffer       = Buffer
- @b ul_cache_length  = Cache lenght
- @b ul_offset        = Offset value

This funtion pointer returns the handle of the opened file

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef void* (*aui_pe_cache_open_fp) (

    const char *pc_filename,

    unsigned char *puc_buffer,

    unsigned int ul_cache_length,

    unsigned int ul_offset

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b read a
media file, where the input paramete are explained below:
- pv_buffer  = Buffer
- size       = Size of each data
- count      = Number of data (so the lenght data is <b> size x count </b>)
- pv_fp      = Handle of the input file

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef unsigned int (*aui_pe_cache_read_fp) (

    void *pv_buffer,

    unsigned int size,

    unsigned int count,

    void *pv_fp

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b seek
the media files, where the input paramete are explained below:
- pv_fp     = Handle of the input file.
- ll_offset = Offset (as the number of address locations added to a base address)
- from      = Seek mode, such as
              - @b SEEK_SET
              - @b SEEK_CUR
              - @b SEEK_END

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef int (*aui_pe_cache_seek_fp) (

    void *pv_fp,

    long long ll_offset,

  int from

  );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b check
whether the media file has reached the end

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef int (*aui_pe_cache_feof_fp) (

    void* pv_fp

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b get
the current position of the media file

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef long long (*aui_pe_cache_ftell_fp) (

    void *pv_fp

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b close
the media file with a proper filename

@note   This function pointer is available @a only for projects based on
        <b> TDS OS </b>
*/
typedef int (*aui_pe_cache_fclose_fp) (

    void *pv_fp

    );

/// @endcoding

/**
Function pointer to specify the type of callback function for the enum
#aui_mp_message, where the output parameter are explained below:
- msg           = The type of message from the player as defined in the enum
                  #aui_mp_message
- pv_data       = Data corresponding to a message, such as
                  - Capture path
                  - Weight and height
                  - etc.
- pv_user_data  = User data given from the the member @b user_data of the
                  struct #aui_attr_mp
*/
typedef void (*aui_mp_message_callback) (

    aui_mp_message msg,

    void *pv_data,

    void *pv_user_data

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Media Player (MP) Module </b> used to set a callback
        function
        </div> @endhtmlonly

        Struct used to set a callback function to Media Player Module

@note   This struct is available @a only for projects based on <b> TDS OS </b>
*/
typedef struct aui_pe_cache_cb_fp {

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b open a media
    file
    */
    aui_pe_cache_open_fp file_open;

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to to @b read a
    media file
    */
    aui_pe_cache_read_fp file_read;

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value parameter #AUI_MP_SET_FD_CB_REG to @b seek
    a media file
    */
    aui_pe_cache_seek_fp file_seek;

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b get the end of
    a media file
    */
    aui_pe_cache_feof_fp file_eof;

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b get the current
    position of a media file
    */
    aui_pe_cache_ftell_fp file_tell;

    /**
    Member to specify the type of callback function registered with the function
    #aui_mp_set by using the enum value #AUI_MP_SET_FD_CB_REG to @b close a media
    file
    */
    aui_pe_cache_fclose_fp file_close;

} aui_pe_cache_cb_fp;

/**
@brief    @htmlonly <div class="details">
          Struct of the <b> Media Player (MP) Module </b> to specify miscellaneous
          information about audio stream
          </div> @endhtmlonly

          Struct to specify miscellaneous information about audio stream

@note     This struct is @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_mp_audio_detail_info {

    /**
    Member to specify how many channels are available for the audio stream
    */
    unsigned int channels;

    /**
    Member to specify the bit-depth of the audio stream (in bits per sample unit)
    */
    unsigned int depth;

    /**
    Member to specify the sample rate of the audio stream
    */
    unsigned short samplerate;

    /**
    Member to specify the audio decoder format of the audio stream
    */
    aui_deca_stream_type audioCodecType;

} aui_mp_audio_detail_info;

/**
@brief    @htmlonly <div class="details">
          Struct of the <b> Media Player (MP) Module </b> to specify a set of
          audio stream track information
          </div> @endhtmlonly

          Struct to specify a set of audio stream track information

@note     This struct is @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_mp_audio_track_info {

    /**
    Member to specify the track index of the audio stream
    */
    unsigned int track_index;

    /**
    Member to specify the list of languages of the audio stream
    */
    char lang_code[5];

    /**
    Member to specify the audio detail information, such as
    - Depth
    - Sample Rate
    - etc.
    */
    aui_mp_audio_detail_info *audDetailInfo;

} aui_mp_audio_track_info;

/**
@brief    @htmlonly <div class="details">
          Struct of the <b> Media Player (MP) Module </b> to specify a set of
          subtitle stream information
          </div> @endhtmlonly

          Struct to specify a set of subtitle stream information

@note     This struct is @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_mp_subtitle_info {

    /**
    Member to specify the track index of subtitle stream
    */
    unsigned int track_index;

    /**
    Member to specify the the list of languages of the subtitle stream
    */
    char lang_code[5];


} aui_mp_subtitle_info;

/**
@brief    @htmlonly <div class="details">
          Struct of the <b> Media Player (MP) Module </b> to specify miscellaneous
          information about video stream
          </div> @endhtmlonly

          Struct to specify miscellaneous information about video stream

@note     This struct is @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_mp_video_track_info {

    /**
    Member to specify the video decoder format, which can refer to a value of
    the enum #aui_en_dec_format in header file #aui_decv.h
    */
    aui_decv_format vidCodecFmt;

    /**
    Member to specify the width of the video stream
    */
    unsigned short width;

    /**
    Member to specify the height of the video stream
    */
    unsigned short height;

    /**
    Member to specify the frame rate of the video stream
    */
    unsigned short framerate;

} aui_mp_video_track_info;

/**
@brief    @htmlonly <div class="details">
          Struct of the <b> Media Player (MP) Module </b> to specify the available
          stream type information
          </div> @endhtmlonly

          Struct to specify the available stream type information
          (i.e. audio, video, subtitle)

@note This struct is @a only for projects based on <b> Linux OS </b>
*/
typedef struct aui_mp_stream_info {

    /**
    Member to specify the total number of different stream type available
    (i.e. audio, video & subtitle)
    */
    unsigned int count;

    /**
    Member to specify the different type of stream information, as defined in
    the enum #eAUI_MP_STREAM_INFO_TYPE
    */
    aui_mp_stream_info_type type;

    /**
    Member as an union to specify the media size and information for
    - Audio
    - Subtitle
    - Video
    */
    union {
        /**
        Sub-members to specify the type of audio information, as defined in the
        struct #aui_mp_audio_track_info
        */
        aui_mp_audio_track_info *audio_track_info;

        /**
        Sub-member to specify the type of subtitle information, as defined in the
        struct #aui_mp_subtitle_info
        */
        aui_mp_subtitle_info *subtitle_info;

        /**
        Sub-member to specify the type of video information, as defined in the
        struct #aui_mp_video_track_info
        */
        aui_mp_video_track_info *video_track_info;

        /**
        Sub-member to specify the media size
        */
        long long mediaSize;

    } stream_info;

} aui_mp_stream_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Media Player (MP) Module </b> to specify the attributes
        to be configured
        </div> @endhtmlonly

        Struct to specify the attributes of Media Player Module available to be
        configured
*/
typedef struct aui_attr_mp {

    /**
    Member to specify the file path to be played
    */
    unsigned char uc_file_name[1024];

    /**
    Member as a @a flag to specify the <b> video zoom mode </b> to be used,
    in particular:
    - @b 0 = Full Mode
    - @b 1 = Preview Mode

    @note This attribute is available @a only for projects based on <b> TDS OS
        </b>
    */
    unsigned char b_is_preview;

    /**
    Member to specify the <b> stream protocol </b> to be used, as defined in the
    enum #aui_mp_stream_protocol, in particular
    - @b UNKNOW
    - @b LIVE
    - @b VOD

    @note This attribute is available @a only for projects based on <b> Linux
          OS </b>
    */
    aui_mp_stream_protocol stream_protocol;

    /**
    Member to specify the starting time to play a media file

    @note  @b 1. The starting time can be either the beginning of the media
                 file or re-start to play the media file after pausing it

    @note  @b 2. This member is available @a only for projects based on
                 <b> Linux OS </b>
    */
    int start_time;

    /**
    Member to specify the File Descriptor Callback Function

    @note This attribute is available @a only for projects based on <b> TDS OS
          </b>
    */
    aui_pe_cache_cb_fp aui_pe_cache_cb;

    /**
    Member to specify the callback function related to a Media Player message,
    as per comment of the function pointer #aui_mp_message_callback

    @note This attribute is available @a only for projects based on <b> Linux
          OS </b>
    */
    aui_mp_message_callback aui_mp_stream_cb;

    /**
    Member to specify the user data to be passed to the callback function related
    to a Media Player message mentioned in the member @b aui_mp_stream_cb of this
    struct
    */
    void *user_data;

} aui_attr_mp, *aui_p_attr_mp;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to @b open the MP Module and configure the
                desired attributes, then get the related handle

@note           This function can @a only be used in the <b> Pre-Run Stage </b>
                of the MPModule, in particular:
                 - Either after performing the initialization of the MP Module
                   by the function #aui_mp_init for the first opening of the
                   MP Module
                 - Or after closing the MP Module by the function #aui_mp_close,
                   considering the initialization of the MP Module has been
                   performed previously by the function #aui_mp_init

@param[in]      p_mp_attr            = Pointer to a struct #aui_attr_mp, which
                                       collects the desired attributes for
                                       the MP Module

@param[out]     p_handle_mp          = Pointer to the handle of the MP Module
                                       just opened

@return         @b AUI_RTN_SUCCESS   = MP Module opened successfully then
                                       user can start to configurate the
                                       MP module
@return         @b AUI_RTN_EINVAL    = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Opening of the MP Module failed
                                       for some reasons
*/
AUI_RTN_CODE aui_mp_open (

    aui_attr_mp *p_mp_attr,

    aui_hdl *p_handle_mp

    );

/**
@brief          Function used to @b close the MP Module, which is already opened
                by the function #aui_mp_open, then the related handle (i.e.
                the related resources such as memory, device) will be released.

@note           This function can @a only be used in the <b> Post-Run Stage </b>
                of the MP Module in pair with its the opening by the function
                #aui_mp_open. After closing the MP Module, user can
                 - Either perform the @b de-initialization of the MP Module
                   by the function #aui_mp_de_init
                 - Or <b> open again </b> the MP Module by the function
                   #aui_mp_open, considering the initialization of the MP
                   Module has been performed previously by the function
                   #aui_mp_init

@param[in]      *p_mp_attr           = Pointer to a struct #aui_attr_mp, which
                                       collects the desired attributes for the
                                       MP Module to be closed
                                       - @b Caution: For the MP Module, this value
                                          is suggested to be set as <b> "NULL" </b>
                                          because the struct #aui_attr_mp doesn't
                                          store any memory pointer in heap
@param[in]      *pp_handle_mp        = Pointer to the handle of the MP Module,
                                       which is already opened and to be closed

@return         @b AUI_RTN_SUCCESS   = MP Module closed successfully
@return         @b AUI_RTN_EINVAL    = Either one or both of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Closing of the MP Module failed for
                                       same reasons

@note           @b Only in projects based on TDS OS, it is suggested to only use
                this function before going into standby mode
*/
AUI_RTN_CODE aui_mp_close (

    aui_attr_mp *p_mp_attr,

    void **pp_handle_mp

    );

/**
@brief          Function used to @b start the MP Module, which is already opened
                by the function #aui_mp_open. In particular, this function is
                used after some supposed/suggested configurations, such as
                - Preview mode, by setting the attributes before open the MP module
                - Play speed, by setting #AUI_MP_SPEED_1 as default for start
                  playing the first time, and then the speed can be changed by the
                  function #aui_mp_speed_set
                After starting the MP Module, the MP file will be played.

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened and to be started

@return         @b AUI_RTN_SUCCESS   = MP Module started successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values      = Starting of the MP Module failed for
                                       some reasons
*/
AUI_RTN_CODE aui_mp_start (

    aui_hdl handle

    );

/**
@brief          Function used to @b stop the MP Module, which is already opened and
                started by respectively the functions #aui_mp_open and #aui_mp_start.\n
                After stopping the MP Module, user can @b close the MP Module by
                the function #aui_mp_close if no need to use the MP Module any more.

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened and to be stopped

@return         @b AUI_RTN_SUCCESS   = MP Module stopped successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values      = Stopping of the MP Module failed for
                                       some reasons
*/
AUI_RTN_CODE aui_mp_stop (

    aui_hdl handle

    );

/**
@brief          Function used to @b pause the MP Module, which is already opened
                and started by respectively the functions #aui_mp_open and
                #aui_mp_start.\n
                After pausing the MP Module, the last frame of the video will
                be frozen on the screen, and two (2) actions can be performed
                as below:
                - @b Resuming the playing video by the function #aui_mp_resume
                - @b Stopping the MP module by the function #aui_mp_stop

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened and started then to be
                                       paused

@return         @b AUI_RTN_SUCCESS   = MP Module paused successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values      = Pausing of the MP Module failed for some
                                       reasons
*/
AUI_RTN_CODE aui_mp_pause (

    aui_hdl handle

    );

/**
@brief          Function used to @b resume the MP Module, which is already opened,
                started and paused by respectively the functions #aui_mp_open,
                #aui_mp_start and #aui_mp_pause.\n
                After resuming the MP Module, the video will play again, and two
                (2) actions can be performed as below:
                - @b Pausing again by the function #aui_mp_pause
                - @b Stopping the MP module by the function #aui_mp_stop

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened, started and paused then
                                       to be resumed

@return         @b AUI_RTN_SUCCESS   = MP Module resumed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values      = Resuming of the MP Module failed for
                                       some reasons
*/
AUI_RTN_CODE aui_mp_resume (

    aui_hdl handle

    );

/**
@brief          Function used to perform the @b initialization of the MP Module,
                before its opening by the function #aui_mp_open.

@note           This function can be used @a only in the <b> Pre-Run Stage </b>
                of the MP Module

@param[in]      fn_mp_init           = Callback function used for the initialization
                                       of the MP Module, as per comment for the
                                       funtion pointer #aui_func_mp_init

@return         @b AUI_RTN_SUCCESS   = MP Module initialized successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values      = Initializing of the MP Module failed for
                                       some reasons

@note           About the callback function #fn_mp_init as input parameter of
                this function:
                - In projects based on Linux OS, it is suggested to set as NULL
                - In projects based on TDS OS, it needs to perform some video
                  decoder and play engine configurations. User can initialize
                  the MP module with the sample code #win_media_player_init
                  directly instead of #aui_mp_init, or using #aui_mp_init to
                  call the function just like #win_media_player_init.\n
                  Please refer to the sample code of the initialization for more
                  clarifications.
*/
AUI_RTN_CODE aui_mp_init (

    aui_func_mp_init fn_mp_init

    );

/**
@brief          Function used to perform the @b de-initialization of the MP Module,
                after its closing by the function #aui_mp_close

@param[in]      fn_mp_de_init        = Callback function used for the
                                       de-initialization of the MP Module, as
                                       per comment for the function pointer
                                       #aui_func_mp_init

@return         @b AUI_RTN_SUCCESS   = MP Module de-initialized successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is
                                       invalid
@return         @b Other_Values      = De-initializing of the MP Module failed
                                       for some reasons

@note           @b Only in projects based on TDS OS, it is suggested to only use
                this function before going into standby mode
*/
AUI_RTN_CODE aui_mp_de_init (

    aui_func_mp_init fn_mp_de_init

    );

/**
@brief          Function used to seek the special time for playing video with
                the MP Module

@param[in]      handle               = Pointer to the handle of the MP Module,
                                       already opened, started then to be managed
                                       for seeking
@param[in]      ul_time_in_ms        = The desired seek time ( in @a millisecond
                                       (ms) unit)

@return         @b AUI_RTN_SUCCESS   = Seeking of the special time for playing
                                       video performed successfully
@return         @b AUI_RTN_EINVAL    = Either one or both of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Seeking of the special time for playing
                                       video failed for some reasons
*/
AUI_RTN_CODE aui_mp_seek (

    aui_hdl handle,

    unsigned long ul_time_in_ms

    );

/**
@brief          Function used to set the speed for the video file

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      en_speed             = Desired play speed for the the media stream

@return         @b AUI_RTN_SUCCESS   = Speed set successfully
@return         @b AUI_RTN_EINVAL    = Either one or both of the input parameter
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Speed setting failed for some reasons

@note           About the audio, it will be changed to an abnormal speed,
                such as fastforward and backforward

*/
AUI_RTN_CODE aui_mp_speed_set (

    aui_hdl handle,

    aui_mp_speed en_speed

    );

/**
@brief          Function used to get the total time of the video file

@note           Comparing this function with #aui_mp_get_total_time, the
                difference is just the unit of the total time to be gotten,
                i.e. second instead of millisecond

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     pui_total_time       = Pointer to the total time of the video
                                       file (in @a second unit)

@return         @b AUI_RTN_SUCCESS   = Getting of the total time of the video
                                       file performed successfully

@return         @b AUI_RTN_EINVAL    = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid

@return         @b Other_Values      = Getting of the total time of the video
                                       file failed for some reasons
*/
AUI_RTN_CODE aui_mp_total_time_get (

    aui_hdl handle,

    unsigned int *pui_total_time

    );

/**
@brief          Function used to get the total time of the video file. with unit
                as millisecond

@note           Comparing this function with #aui_mp_total_time_get, the
                difference is just the unit of the total time to be gotten,
                i.e. millisecond instead of second

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     pui_total_time_in_ms = Pointer to the total time of the video
                                       file (in @a millisecond unit)

@return         @b AUI_RTN_SUCCESS   = Getting of the total time of the video
                                       file performed successfully
@return         @b AUI_RTN_EINVAL    = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Getting of the total time of the video
                                       file failed for some reasons
*/
AUI_RTN_CODE aui_mp_get_total_time (

    aui_hdl handle,

    unsigned int *pui_total_time_in_ms

    );

/**
@brief          Function used to get the current time of the video file

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     *pui_cur_time_in_ms  = Pointer to the current time of the video
                                       file (in @a millisecond unit)

@return         @b AUI_RTN_SUCCESS   = Getting of the current time of the video
                                       file performed successfully
@return         @b AUI_RTN_EINVAL    = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Getting of the current time of the video
                                       file failed for some reasons
*/
AUI_RTN_CODE aui_mp_get_cur_time (

    aui_hdl handle,

    unsigned int *pui_cur_time_in_ms

    );

/**
@brief          Function used to perform a specific setting of the MP Module,
                between its opening and starting by respectively the functions
                #aui_mp_open and #aui_mp_start

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened and to be managed to
                                       perform a specific setting
@param[in]      ul_item              = The item related to the specific setting
                                       of the MP Module to be performed,
                                       as defined in the enum #aui_mp_item_set
@param[in]      *pv_param            = The pointer as per the description of
                                       the specific setting of the MP Module
                                       to be performed, as defined in the enum
                                       #aui_mp_item_set

@return         @b AUI_RTN_SUCCESS   = Specific setting of the MP Module performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = Either one or all of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Specific setting of the MP Module failed
                                       for some reasons
*/
AUI_RTN_CODE aui_mp_set (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information of the MP Module,
                after its opening and starting by respectively the functions
                #aui_mp_open and #aui_mp_start

@param[in]      handle               =  Pointer to the handle of the MP Module,
                                        already opened and to be managed to get
                                        a specific information, as defined in
                                        the enum #aui_mp_item_get
@param[in]      ul_item              =  The item related to the specific
                                        information of the MP Module to be gotten,
                                        as defined in the enum #aui_mp_item_get

@param[out]     *pv_param            =  The pointer as per the description of the
                                        specific information of the MP Module to
                                        be gotten, as defined in the enum
                                        #aui_mp_item_get

@return         @b AUI_RTN_SUCCESS   =  Getting of the specific information of
                                        the MP Module performed successfully
@return         @b AUI_RTN_EINVAL    =  Either one or both of the parameters
                                        (i.e. [in], [out]) are invalid
@return         @b Other_Values      =  Getting of the specific information of
                                        the MP Module failed for some reasons
*/
AUI_RTN_CODE aui_mp_get (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

/// @coding

/**
@brief          Function used to get the download speed of http stream

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     pull_dl_Speed        = Pointer to the download speed of http
                                       stream

@return         @b AUI_RTN_SUCCESS   = Getting of the download speed of http
                                       stream performed successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Getting of the download speed of http
                                       stream failed for some reasons

@note  @b 1. This function is @a only available for http stream

@note  @b 2. This function is available @a only for projects based on
             <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_get_download_speed (

    aui_hdl handle,

    unsigned long long *pull_dl_Speed

    );

/// @endcoding

/**
@brief          Function used to check whether the stream is seekable or not

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     p_is_seekable        = Result:
                                       @b -1 = Haven't decided yet
                                       @b  0 = Stream not seekable
                                       @b  1 = Stream seekable

@return         @b AUI_RTN_SUCCESS   = Checking of the stream performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Checking of the stream failed for some
                                       reasons

@note  @b 1. Generally this function should always return @b AUI_RTN_SUCCESS

@note  @b 2. This function is available @a only for projects based on
             <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_is_seekable (

    aui_hdl handle,

    int *p_is_seekable

    );

/**
@brief          Function used to get the current play rate as play speed of the
                media stream

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[out]     p_speed              = Current speed of the media stream, as
                                       defined in the enum #aui_mp_speed

@return         @b AUI_RTN_SUCCESS   = Getting of the current play speed
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the parameter
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values      = Getting of the current play rate failed
                                       for some reasons

@note           This function is available @a only for projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_get_playrate (

    aui_hdl handle,

    aui_mp_speed *p_speed

    );

/**
@brief          Function used to get stream information, as defined in the enum
                #eAUI_MP_STREAM_INFO_TYPE

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      type                 = The type of stream information to ge gotten,
                                       as defined in the enum #aui_mp_stream_info_type

@param[out]     pp_info              = Pointer to the requested stream information,
                                       as defined in the struct #aui_mp_stream_info

@return         @b AUI_RTN_SUCCESS   = Getting of the stream information
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Either one or all of the parameter (i.e.
                                       [in], [out]) are invalid
@return         @b Other_Values      = Getting of the stream information failed
                                       for some reasons

@note  @b 1.  If user doesn't like to collect those stream information, user
              needs to call the function #aui_mp_free_stream_info to free the
              memory previously intended to store them.

@note  @b 2.  This function is available @a only for projects based on
              <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_get_stream_info (

    aui_hdl handle,

    aui_mp_stream_info_type type,

    aui_mp_stream_info **pp_info

    );

/**
@brief          Function used to get the current stream information

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened

@param[in]      type                 = The type of current stream information
                                       to be gotten

@param[out]     pp_info              = Pointer to the requested current stream
                                       information, as defined in the struct
                                       #aui_mp_stream_info

@return         @b AUI_RTN_SUCCESS   = Getting of the current stream information
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Either one or all of the parameter (i.e.
                                       [in], [out]) are invalid
@return         @b Other_Values      = Getting of the current stream information
                                       failed for some reasons

@note  @b 1. If user doesn't like to collect those stream information, user
             needs to call the function #aui_mp_free_stream_info to free the
             memory allocated to store them

@note  @b 2. This function is available @a only for projects based on
             <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_get_cur_stream_info (

    aui_hdl handle,

    aui_mp_stream_info_type type,

    aui_mp_stream_info **pp_info

    );

/**
@brief          Function used to free the memory allocated by the function
                #aui_mp_get_stream_info or #aui_mp_get_cur_stream_info to
                store the (current) stream information

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      p_info               = Pointer to the information which has
                                       been collected

@return         @b AUI_RTN_SUCCESS   = Freeing of the the memory storing the
                                       (current) stream information performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the input parameter
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Freeing of the memory storing the
                                       (current) stream information failed for
                                       some reasons

@note This function is available @a only for projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_free_stream_info (

    aui_hdl handle,

    aui_mp_stream_info *p_info

    );

/**
@brief          Function used to set the maximum buffering time in the buffering
                queue

@note           As example, if
                - The time is set as 20 seconds with high percentage
                - The boundary is set as 15 and 3 seconds with low percentage
                  respectively

                and when the buffered time is less than the lover bound, for
                example as 0.6s, then player will pause and will do buffering
                until the buffered time reaches the upper bound as 3s, and then
                the player will continue to play

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      time                 = Maximum buffering time in the buffering
                                       queue
                                       - @b Caution: It is recommended to use the
                                            default value as 20 seconds
@param[in]      low_percent          = Low percentage threshold of the time to
                                       start buffering
@param[in]      high_percent         = High percentage threshold of the time to
                                       finish buffering

@return         @b AUI_RTN_SUCCESS   = Setting of the maximum buffering time
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Either one or any of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Setting of the maximum buffering time
                                       failed for some reasons

@note:          @b 1.  This function @a must be called before calling the function
                       #aui_mp_start if user wants to set the buffering time
@note           @b 2.  This function is available @a only for projects based on
                       <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_set_buffering_time (

    aui_hdl handle,

    unsigned int time,

    unsigned int low_percent,

    unsigned int high_percent

    );

/**
@brief          Function used to set the buffering percentage for starting playback

@note           This function used for reducing the time to start playback and
                have better user experience.\n
                Example: Given a low percentage and a buffering time percentage
                set by the function #aui_mp_set_buffering_time as 30% to start
                playback, the player will start to playback when the buffering
                reaches 30% as minimum value to start the playback, and then it
                will back to the usual buffering scheme.

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      percent              = Percentage for starting playback
                                       - @b Caution: The default value is 100

@return         @b AUI_RTN_SUCCESS   = Setting of the buffering percentage for
                                       starting playback performed successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values      = Setting of the buffering percentage for
                                       starting playback failed for some reasons

@note           This function @a must be called before calling the function
                #aui_mp_start if user wants to set the buffering time

@note This function is available @a only for projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_set_start2play_percent (

    aui_hdl handle,

    int percent

    );

/**
@brief          Function used to change audio stream with a given track index

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      track_index          = A variable as a track index:
                                       - @b Invalid = Server just ignores the
                                                      command
                                       - @b      -1 = Server switches to the
                                                      next audio stream
                                                      circularly

                                       Player can get information about track
                                       index from the function #aui_mp_get_stream_info

@return         @b AUI_RTN_SUCCESS   = Changing of the audio stream performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the input parameters
                                       (i.e. param[in]) are invalid
@return         @b Other_Values      = Changing of the audio stream failed for
                                       some reasons

@note This function is available @a only for projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_change_audio (

    aui_hdl handle,

    int track_index

    );

/**
@brief          Function used to change subtitle stream with a given track index

@param[in]      handle               = Pointer to the handle of the MP Module
                                       already opened
@param[in]      track_index          = A variable as a track index:
                                        - @b Invalid = Server just ignores the
                                                       command
                                        - @b      -1 = Server switched to the
                                                       next subtitle stream
                                                       circularly

                                        Player can get information about track
                                        index from the function #aui_mp_get_stream_info

@return         @b AUI_RTN_SUCCESS   = Changing of the subtitle stream performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Either one or both of the input parameters
                                       (i.e. param[in]) are invalid
@return         @b Other_Values      = Changing of the subtitle stream failed for
                                       some reasons

@note This function is available @a only for projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_change_subtitle (

    aui_hdl handle,

    int track_index

    );

/**
@brief       Function used to get snapshot of video after opening the MP Module
             by the function #aui_mp_open

@note        After finishing the frame capture, a callback message returns with
             the stored path and height/width of the image by the enum value
             #AUI_MP_FRAME_CAPTURE, which message has format
             <b> /path/to/captured/image;h=xxxx,w=xxxx </b>.\n\n

             @b Example.\n
             For the return message as <b> /tmp/nmpvidcapture.yuv;h=1088,w=1920 </b>:
             - The stored path is <b> /tmp/nmpvidcapture.yuv </b>
             - The image is stored as @b YV12 format
             - The height of the captured image is @b 1088
             - The weight of the captured image is @b 1920

@note        User has to close the MP module by the function #aui_mp_close after
             getting the snapshots to avoid any undefined behavior

@param[in]   handle                  = Pointer to the handle of the MP Module
                                       already opened

@return      @b AUI_RTN_SUCCESS      = Getting of the snapshot performed
                                       successfully
@return      @b AUI_RTN_EINVAL       = The input parameter (i.e. param[in]) is
                                       invalid
@return      @b Other_Values         = Getting of the snapshot failed for some
                                       reasons

@note  This function is available @a only for projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_mp_get_snapshot (

    aui_hdl handle

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

/**
@brief       Function used to get the current volume value from the MP Module

@warning     This function is @a deprecated then can be ignored. please use
             the function #aui_snd_vol_get instead

@param[in]   pv_hdl_mp               = Pointer to the handle of the MP Module
                                       already opened

@param[out]  volume                  = Pointer to the current volume value

@return      @b AUI_RTN_SUCCESS      = Getting of the current volume value
                                       performed successfully
@return      @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                       invalid
@return      @b Other_Values         = Getting of the current volume value
                                       failed for some reasons
*/
AUI_RTN_CODE aui_mp_get_volume (

  void *pv_handle_mp,

  unsigned int *volume

  );

/**
@brief       Function used to set the volume value in the MP Module

@note        This function is @a deprecated then can be ignored, please use
             the function #aui_snd_vol_set instead

@param[in]   pv_hdl_mp               = Pointer to the handle of the MP Module
                                       already opened
@param[in]   volume                  = Pointer to the volume value to be set

@return      @b AUI_RTN_SUCCESS      = Setting of the volume value performed
                                       successfully
@return      @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                       invalid
@return      @b Other_Values         = Setting of the volume value failed for
                                       some reasons
*/
AUI_RTN_CODE aui_mp_set_volume (

    void *pv_handle_mp,

  int volume

  );

/**
@brief       Function used to get the total time of the video file

@note        This function is @a deprecated then can be ignored, please use
             the function #aui_mp_get_total_time instead

@param[in]   pv_handle_mp            = Pointer to the handle of the MP Module
                                       already opened

@param[out]  pui_total_time          = Pointer to the total time of the video
                                       file just gotten (in @a second unit)

@return      @b AUI_RTN_SUCCESS      = Getting of the total time of the video
                                       file performed successfully
@return      @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in], [out])
                                       is invalid
@return      @b Other_Values         = Getting of the total time of the video
                                       file failed for some reasons
*/
AUI_RTN_CODE aui_mp_total_time_get (

    void *pv_handle_mp,

  unsigned int *pui_total_time

  );

/**
@brief       Function used to get the current time of the video file

@note        This function is @a deprecated then can be ignored, please use
             the function #aui_mp_get_cur_time instead

@param[in]   pv_handle_mp            = Pointer to the handle of the MP Module
                                       already opened

@param[out]  pui_cur_time            = Pointer to the current time of the video
                                       file jsut gotten (in @a second unit)

@return      @b AUI_RTN_SUCCESS      = Getting of the current time of the video
                                       file performed successfully
@return      @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in], [out])
                                       is invalid
@return      @b Other_Values         = Getting of the current time of the  video
                                       file failed for some reasons
*/
AUI_RTN_CODE aui_mp_cur_time_get (

  void *pv_handle_mp,

  unsigned int *pui_cur_time

  );

/**
@warning  This function is @a deprecated then can be ignored
*/
AUI_RTN_CODE aui_mp_set_subtitle_display (

  void *pv_handle_mp,

  int onoff

  );

/**
@warning  This function is @a deprecated so user can ignor it
*/
AUI_RTN_CODE aui_mp_set_message_callback (

    aui_hdl handle,

    aui_mp_message msg,

    pn_message_callback func

    );

/*
@warning  This enum is @b deprecated then can be ignored
*/
typedef enum {

    AUI_CBT_NONE,

    /**
    Value to specify the response for the end of a media stream
    */
    AUI_CBT_FINISHED,

    /**
    Value to specify the response for changing state
    */
    AUI_CBT_STATE_CHANGE,

    /**
    Value to specify the response for buffering percentage
    */
    AUI_CBT_BUFFERING,

    /**
    Value to specify the warning for audio decoder doesn't support the audio type
    */
    AUI_CBT_WARN_UNSUPPORT_AUDIO,

     /**
    Value to specify the warning for video decoder doesn't support the video type
    */
    AUI_CBT_WARN_UNSUPPORT_VIDEO,

    /**
    Value to specify the warning for audio decoder decodes error
    */
    AUI_CBT_WARN_DECODE_ERR_AUDIO,

    /**
    Value to specify the warning for video decoder error
    */
    AUI_CBT_WARN_DECODE_ERR_VIDEO,

    /**
    Value to specify the player has reached the beginning of the stream in trick mode
    */
    AUI_CBT_WARN_TRICK_BOS,

    /**
    Value to specify the player has reached the end of the stream state in trick mode
    */
    AUI_CBT_WARN_TRICK_EOS,

    /**
    Value to specify the error from soup http connection
    */
    AUI_CBT_ERR_SOUPHTTP,

    /**
    Value to specify the player doesn't know the stream type
    */
    AUI_CBT_ERR_TYPE_NOT_FOUND,

    /**
    Value to specify the demultiplexing failed
    */
    AUI_CBT_ERR_DEMUX,

    /**
    Value to specify an undefined error from the player occurred
    */
    AUI_CBT_ERR_UNDEFINED,

    /**
    Value to mark the end of this enum
    */
    AUI_CBT_MAX

} AUI_CALLBACK_TYPE;

#define AUI_STREAM_PROTOCOL aui_mp_stream_protocol

#define AUI_MP_STREAM_CALLBACK aui_mp_message_callback

#define eAUI_MP_STREAM_INFO_TYPE aui_mp_stream_info_type

#define stAUI_MP_VIDEO_TRACK_INFO aui_mp_video_track_info

#define stAUI_MP_SUBTITLE_INFO aui_mp_subtitle_info

#define stAUI_MP_AUDIO_TRACK_INFO aui_mp_audio_track_info

#define stAUI_MP_AUDIO_DETAIL_INFO aui_mp_audio_detail_info

#define aui_en_play_speed aui_mp_speed

#define AUI_MP_ERROR_NONE AUI_MP_MESSAGE_MAX

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


