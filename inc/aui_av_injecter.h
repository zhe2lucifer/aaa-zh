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
Current ALi author: Wendy.He
Last update:        2017.01.16
-->

@file   aui_av_injecter.h

@brief  Audio/Video Injection (AV-Injection) Module

        <b> AV-Injection Module </b> is used to inject <b> audio/video ES data
        </b> into audio/video decoder for the upper layer, i.e. media player

@note   This module can be used @a only in projects based on <b> Linux OS </b>.\n
        For further details, please refer to ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "AV-Injection Module"
        </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_AV_INJECTER_H

#define _AUI_AV_INJECTER_H

/**************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List*******************************/

/**
Macro used by the struct #aui_av_packet when user cannot get the valid <b> PTS
(Presentation Time stamp) </b> or <b> DTS (Decompression Time stamp) </b>
*/
#define AUI_AV_NOPTS_VALUE  ((long long)(0x8000000000000000LL))

/*******************************Global Type List*******************************/

/**
Enum to specify different video data injection mode. The desired mode can be set
by the function #aui_video_decoder_open with the input parameter
#aui_video_decoder_init
*/
typedef enum aui_video_dec_mode {

    /**
    In this mode, an entire video frame of video data needs to be injected to
    video decoder each time.
    */
    AUI_VIDEO_DEC_MODE_FRAME = 5,

    /**
    In this mode, either a fixed or another size of video data can be injected
    into video decoder continuously then the video decoder will gather an entire
    frame to decode automatically
    */
    AUI_VIDEO_DEC_MODE_STREAM = 6

} aui_video_dec_mode;

/**
Enum used to specify the packet type to be injected
*/
typedef enum aui_av_packet_flag {

    /**
    Value to specify <b> ES Data </b> packet
    */
    AUI_AV_PACKET_ES_DATA,

    /**
    Value to specify <b> Extra Data </b> packet, which is a special packet
    containg only extra data for a codec

    @note  This value is like a flag required when the video resolution is
           changed, and it will notify the driver that the codec data must be
           parsed again
    */
    AUI_AV_PACKET_EXTRA_DATA,

    /**
    Value to specify <b> End of Stream </b> packet, which is to indicate that
    no further data will be injected to decoder after it.

    @note   This value is like a flag that an application needs to set to AUI
            in order to let the driver play any remaining thing in the buffer
            without waiting for futher data
    */
    AUI_AV_PACKET_EOS

} aui_av_packet_flag;

/**
Enum to specify the video playback mode of video decoder, and is used by the
function #aui_video_decoder_flush to implement video flushing
*/
typedef enum aui_vdec_playback_mode {

    /**
    This mode indicates the video is playing in normal forward speed
    */
    AUI_VDEC_PLAYBACK_MODE_NORMAL,

    /**
    This mode indicates the video is playing in fast forward speed

    @warning At the moment this mode is not supported by AV-Injection Module
    */
    AUI_VDEC_PLAYBACK_MODE_FF,

    /**
    This mode indicates the video is playing fast backward speed

    @warning At the moment this mode is not supported by AV-Injection Module
    */
    AUI_VDEC_PLAYBACK_MODE_FB

} aui_vdec_playback_mode;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video Injection (AV-Injection) Module </b>
       specify audio stream information
       </div> @endhtmlonly

       Struct to specify the information related to the audio stream, and is
       used by the function #aui_audio_decoder_open to initialize/configure
       the audio decoder
*/
typedef struct aui_audio_info_init {

    /**
    Member to specify the @b format of the audio stream. Please refer to the
    enum #aui_audio_stream_type_em for more information
    */
    aui_deca_stream_type codec_id;

    /**
    Member as @a flag to specify the <b> channel type </b> of the audio stream.
    The different integer values of this flag can be:
    - @b 1 = Mono
    - @b 2 = Stereo
    */
    int channels;

    /**
    Member to specify the <b> number of bits of information per sample </b> as
    @b bit-depth of the audio stream, and it directly corresponds to the
    resolution of each sample
    */
    int nb_bits_per_sample;

    /**
    Member to specify the <b> sample rate </b> of the audio stream
    (in @a Hz unit)

    @note  This member can be set to zero (0) as unknown value
    */
    unsigned long sample_rate;

    /**
    Member to specify the <b> extra data </b> of the audio stream, which are
    helpful for the audio decoding

    @note The extra data is extracted from the container, and its format is
          defined in the specification of the audio format.
    */
    unsigned char *extradata;

    /**
    Member to specify the <b> size of the extra data </b> of the audio stream
    */
    unsigned long extradata_size;

    /**
    Member to specify the <b> bit rate </b> of the audio stream which is equals
    to \n\n

    #sample_rate X #nb_bits_per_sample \n\n

    (in @a bps unit).
    */
    unsigned long bit_rate;

    /**
    Member as a @a flag to specify the <b> block alignment </b> of the @b PCM
    audio stream which is equals to \n\n

    (#channels X #nb_bits_per_sample) / 8 \n\n

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
    Member to specify the DECA handle for AV Injecter.

    @note @b 1. This handle should be set @a only when AV Injecter is used to
                support PCM mixing otherwise it should be set to NULL value.

    @note @b 2. The audio mixing format is PCM data with the following mandatory
                settings:
                - Bits per sample: 32
                - Channel: 2
                - Sample rate: 8192 ~ 96000
    */
    aui_hdl deca_handle;

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

} aui_audio_info_init;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
        specify the status of the audio decoder
        </div> @endhtmlonly

        Struct to specify the <b> status of the audio decoder </b>, and is used
        by the function #aui_audio_decoder_get_status to get the @a current one
*/
typedef struct aui_audio_decoder_status {

    /**
    Member to specify the <b> total size of the buffer </b> (in @a bytes unit)
    used to gather the <b> audio ES data </b>

    @note That buffer is the same mentioned in the struct #aui_decoder_buffer_status
          for audio decoder, please refer it for further information.
    */
    unsigned long buffer_size;

    /**
    Member to specify the <b> used size of the buffer </b> (in @a bytes unit)
    to gather audio ES data.
    */
    unsigned long buffer_used;

    /**
    Member to specify the <b> PTS (Presentation Time stamp) of the played audio
    frame </b> (in @b millisecond @b (ms) unit).
    */
    unsigned long last_pts;

    /**
    Member to specify the <b> total amount of the decoded audio frames </b>
    */
    unsigned long frames_decoded;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned long frames_played;

} aui_audio_decoder_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
       specify the status of the video decoder </div> @endhtmlonly

       Struct to specify the status of the video decoder, and is used by the
       function #aui_video_decoder_get_status to get the current one
*/
typedef struct aui_video_decoder_status {

    /**
    Member to specify the <b> total size of the buffer </b> (in @a bytes unit)
    used to gather the <b> video ES data </b>. There are two different cases:

    @note   Two (2) cases can be distinguished for the buffer, depending of
            which video data injection mode is running as per the enum
            #aui_video_dec_mode:
            - @b 1: When the video data injection mode is #AUI_VIDEO_DEC_MODE_FRAME,
                    the buffer is the same buffer mentioned in the struct
                    #aui_decoder_buffer_status for video decoder
            - @b 2: When the video data injection mode is #AUI_VIDEO_DEC_MODE_STREAM,
                    the buffer contains two (2) buffers:
                    - The VBV (Video Buffering Verifier) buffer
                    - The buffer that is the same buffer mentioned in the struct
                      #aui_decoder_buffer_status for video decoder
    */
    unsigned long buffer_size;

    /**
    Member to specify the <b> used size of the buffer </b> (in @a bytes unit)
    which is used to gather the video ES data
    */
    unsigned long buffer_used;

    /**
    Member to specify the <b> PTS (Presentation Time stamp) of the played video
    frame </b> (in @b millisecond @b (ms) unit)
    */
    unsigned long last_pts;

    /**
    Member to specify the <b> total amount of the decoded video frames </b>
    */
    unsigned long frames_decoded;

    /**
    Member to specify the <b> total amount of the displayed video frames </b>
    */
    unsigned long frames_displayed;

    /**
    Member to specify the <b> pictures width </b> of the current video stream
    */
    long width;

    /**
    Member to specify the <b> pictures height </b> of the current video stream
    */
    long height;

    /**
    Member to specify the <b> Sample Aspect Ratio Width </b> of the current
    video stream
    */
    long sar_width;

    /**
    Member to specify the <b> Sample Aspect Ratio Height </b> of the current
    video stream
    */
    long sar_height;

    /**
    Member to specify the frame rate (in @a fps*1000 unit) of the current
    video stream
    */
    long fps;

    /**
    Member as @a flag to specify which <b> scan mode </b> is running, in
    particular:
    - @b 0 = Progressive scanning
    - @b 1 = Interlaced scanning
    */
    unsigned char interlaced;

    /**
    Member to contain the <b> display layer </b> of the video decoder.\n
    */
    aui_dis_layer display_layer;

} aui_video_decoder_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
       specify the status of the buffer used to gather the audio/video ES data
       </div> @endhtmlonly

       Struct to specify the <b> status of the buffer </b> used to gather the
       ES data, and is used by the functions:
       - #aui_audio_decoder_get_buffer_status to get the status of the buffer
         related to <b> audio ES data </b>
       - #aui_video_decoder_get_buffer_status to get the status of the buffer
         related to <b> video ES data </b>

@note Checking that status is useful to control the <b> speed of injecting ES
      data </b>.
*/
typedef struct aui_decoder_buffer_status {

    /**
    Member to specify the <b> total size of the buffer </b> (in @a bytes unit)
    */
    unsigned long total_size;

    /**
    Member to specify the <b> valid size of the buffer </b> (in @a bytes unit)
    already injected
    */
    unsigned long valid_size;

    /**
    Member to specify the <b> free size of the buffer </b> (in @a bytes unit)
    that can still be injected
    */
    unsigned long free_size;

} aui_decoder_buffer_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
       specify video stream information
       </div> @endhtmlonly

       Struct to specify the information related to the video stream, and is
       used by the function #aui_video_decoder_open to initialize/configure
       the video decoder
*/
typedef struct aui_video_decoder_init {

    /**
    Member to specify the <b> format of the video stream </b>. Please refer
    to the enum #aui_decv_format for more information

    @note The video decoder can accept two (2) types of @b H.264 bitstream as
          per the two (2) cases below:
          1. Still referring to this struct, if
             - The value of the member @b codec_id is #AUI_DECV_FORMAT_AVC
             - And the value of the member @b extradata is @b NULL

             \n then the video decoder assumes that the inject data are in the
             <b> H.264 Annex.b </b> format, i.e. <b><em> Start Code </em> + NALU </b>

          2. Still referring to this struct, if
             - The value of the member @b codec_id is #AUI_DECV_FORMAT_AVC
             - And the value of the member @b extradata is <b> not NULL </b>
             - And the value of the member @b extradata_size <b> > 0 </b>

             \n then the  video decoder assumes that the inject data are in the
             <b> H.264 AVCC </b> format, i.e. <b><em> NALU size </em> + NALU </b>
    */
    aui_decv_format  codec_id;

    /**
    Member to specify the <b> injection mode </b> of the video ES data.
    Please refer to the enum #aui_video_dec_mode for more information.
    */
    aui_video_dec_mode decode_mode;

    /**
    Member to specify the <b> pictures width </b> of the video stream
    */
    long pic_width;

    /**
    Member to specify the <b> pictures height </b> of the video stream
    */
    long pic_height;

    /**
    Member to specify the <b> Sample Aspect Ratio Width </b> of the video
    stream
    */
    long sar_width;

    /**
    Member to specify the <b> Sample Aspect Ratio Height </b> of the video
    stream
    */
    long sar_height;

    /**
    Member to specify the <b> frame rate </b> (in @b fps*1000 unit) of the
    video stream
    */
    long frame_rate;

    /**
    Member to specify the @b extra_data of video stream, which are helpful for
    video decoding

    @note @b 1. The extra data is extracted from the container, and its format
                is defined in the specification of the video format

    @note @b 2. The video decoder can accept two (2) types of @b H.264 bitstream
                as per the two (2) cases below:
                1. Still referring to this struct,if
                   - The value of the member @b codec_id is #AUI_DECV_FORMAT_AVC
                   - And the value of the member @b extradata is @b NULL

                   \n then the video decoder assumes that the inject data are in the
                   <b> H.264 Annex.b </b> format, i.e. <b><em> Start Code </em> + NALU </b>

                2. Still referring to this struct,if
                   - The value of the member @b codec_id is #AUI_DECV_FORMAT_AVC
                   - And the value of the member @b extradata is <b> not NULL </b>
                   - And the value of the member @b extradata_size <b> > 0 </b>

                   \n then the  video decoder assumes that the inject data are in the
                   <b> H.264 AVCC format </b> format, i.e. <b><em> NALU size </em> + NALU </b>
    */
    unsigned char *extradata;

    /**
    Member to specify the <b> size of the extra data </b> of the video stream
    */
    long extradata_size;

    /**
    Member to specify the handle of DECV Device which will be used to decode
    video data by injecting.

    @note  This member is intended @a only for <b> Picture-in-Picture (PiP) </b>.
           \n For normal play, please set it to @b NULL.
    */
    aui_hdl decv_handle;

} aui_video_decoder_init;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
       specify sub-sample byte information
       </div> @endhtmlonly

       Struct to specify information related to sub-sample byte
*/
typedef struct aui_av_subsample_byte_info {

    /**
    Member to specify the bytes of clear data
    */
    unsigned short bytes_of_clear_data;

    /**
    Member to specify the bytes of encrypted data
    */
    unsigned long  bytes_of_encrypted_data;

} aui_av_subsample_byte_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video (AV) Module </b> to specify the
        information of sub-sample encryption data
        </div> @endhtmlonly

        Struct to specify the information of sub-sample encryption data.

@note   This struct is used when data buffer channel is set to
        #AUI_AV_BUFFER_CHANNEL_VID_DECODER_FOR_SUBSAMPLE_ENCRYPTED_DATA

@note   In sub-sample encryption mode, the sample is divided into one or more sub-samples.
        And each sub-sample may have an unencrypted part followed by an encrypted part.
*/
typedef struct aui_av_subsample_encryption_info {

    /**
    Member to specify the sub-samples count number
    */
    unsigned long ul_subsample_count;

    /**
    Member as a pointer to the sub-sample byte information,
    as defined in the structure #aui_av_subsample_byte_info
    */
    aui_av_subsample_byte_info *p_subsample_byte_info;

} aui_av_subsample_encryption_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio/Video Injection (AV-Injection) Module </b> to
        specify ES packet header information
        </div> @endhtmlonly

        Struct to specify the information related to the <b> ES packet header
        </b>, which is injected into the audio/video decoder before injecting
        audio/video ES data. and is used by the functions:
        - #aui_audio_decoder_write_header to inject ES packet header for audio
        - #aui_video_decoder_write_header to inject ES packet header for video
*/
typedef struct aui_av_packet {

    /**
    Member to specify the <b> PTS (Presentation Time Stamp) </b>
    (in @a millisecond @a (ms) unit).

    @note The value of this variable can be #AUI_AV_NOPTS_VALUE if the PTS is
          not contained in the audio/video stream.
     */
    long long pts;

    /**
    Member to specify the <b> DTS (Decompression Time Stamp) </b>
    (in @a millisecond @a (ms) unit).

    @note The value of this variable can be #AUI_AV_NOPTS_VALUE if the DTS is
          not contained in the audio/video stream.
    */
    long long dts;

    /**
    Member to specify the <b> size of an entire ES data </b> with audio/video
    frame
    */
    long size;

    /**
    Member to specify the flag of the current packet,
    as defined in the struct #aui_av_packet_flag
    */
    aui_av_packet_flag flag;

    /**
    Member to specify the length of IV.

    @note  It is used when the data are encrypted as below:
           - If <b> iv_length > 0 </b>, the descrambler will use the IV pointed
             by the member @b iv_ctr of the present structure as IV to decrypt
             the current writting data
           - If <b> iv_length = 0 </b>, the IV pointed by the member @b iv_ctr
             of the present structure will be ignored. Then the DSC will decide
             whether to update IV based on the @b ul_block_size of the structure
             #aui_dsc_process_attr. Please take a look at the structure
             #aui_dsc_process_attr for more information.
    */
    unsigned long  iv_length;

    /**
    Member to specify the length of IV

    @note  It is used when the data are encrypted
    */
    unsigned char* iv_ctr;

    /**
    Member to specify the sub-sample encryption information for encrypted ES data
    */
    aui_av_subsample_encryption_info subsampe_enc_info;

} aui_av_packet, aui_av_packet_t;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**********************Functions for Audio Decoder Only************************/

/**
@brief          Function used to open the audio decoder and configure the desired
                attributes then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run stage </b>
                of the AV-Injection Module. In particular, after opening the
                audio decoder user can
                - Either configure the AV synchronization by the function
                  #aui_audio_decoder_set_sync
                - Or inject ES packet header and ES data of audio into audio
                  decoder by, respectively, the functions
                  #aui_audio_decoder_write_header and
                  #aui_audio_decoder_write

@param[in]      p_audio_info        = Pointer to a struct #aui_audio_info_init,
                                      which collects audio information to
                                      initialize/configure the audio decoder

@param[out]     p_decoder_out       = #aui_hdl pointer to the handle of the
                                      audio decoder just opened

@return         @b AUI_RTN_SUCCESS  = Audio decoder is opened successfully then
                                      user can start to configure or inject ES
                                      packet header to the audio decoder
@return         @b Other_Values     = Opening of the audio decoder failed for
                                      some reasons
*/
AUI_RTN_CODE aui_audio_decoder_open (

    aui_audio_info_init *p_audio_info,

    aui_hdl *p_decoder_out

    );

/**
@brief          Function used to close the audio decoder already opened by the
                function #aui_audio_decoder_open then the related handle will
                be released (i.e. the related resources such as memory, device)

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be closed

@return         @b AUI_RTN_SUCCESS  = Audio decoder is closed successfully
@return         @b Other_Values     = Closing of the audio decoder failed
                                      for some reasons
*/
AUI_RTN_CODE aui_audio_decoder_close (

    aui_hdl decoder

    );

/**
@brief          Function used to pause/resume the audio decoder already opened
                by the function #aui_audio_decoder_open.

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be paused/resumed
@param[in]      pause               = Flag to indicate which functionality
                                      needs to be performed, in particular:
                                    - @b 0 = Pause
                                    - @b 1 = Resume

@return         @b AUI_RTN_SUCCESS  = Audio decoder is paused/resumed successfully
@return         @b Other_Values     = Pausing/Resuming of the audio decoder
                                      failed for some reasons
*/
AUI_RTN_CODE aui_audio_decoder_pause (

    aui_hdl decoder,

    const int pause

    );

/**
@brief          Function used to get the current status of the audio decoder
                already opened by the function #aui_audio_decoder_open

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be managed to get
                                      the current status

@param[out]     p_status            = Pointer to the struct #aui_audio_decoder_status
                                      which collect information related to the
                                      current status of the audio decoder

@return         @b AUI_RTN_SUCCESS  = Getting the current status of audio decoder
                                      performed successfully
@return         @b Other_Values     = Getting the current status of audio decoder
                                      failed for some reasons
*/
AUI_RTN_CODE aui_audio_decoder_get_status (

    aui_hdl decoder,

    aui_audio_decoder_status *p_status

    );

/**
@brief          Function used to set the desired synchronization mode of the
                audio decoder already opened by the function #aui_audio_decoder_open

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be managed to set
                                      the desired synchronization mode
@param[in]      enable              = Flag to indicate which synchronization
                                      mode needs to be set, in particular:
                                      - @b 0 = Free run mode then audio decoder
                                               plays audio frames with the sample
                                               rate
                                      - @b 1 = Enable synchronization,video decoder
                                               plays video frames with the STC (System
                                               Time Clock) provided by audio

@return         @b AUI_RTN_SUCCESS  = Setting the synchronization mode of audio
                                      decoder performed successfully
@return         @b Other_Values     = Setting the synchronization mode of audio
                                      decoder failed for some reasons

@note           In case of @a only audio, the video will be played in free run mode.
*/
AUI_RTN_CODE aui_audio_decoder_set_sync (

    aui_hdl decoder,

    const int enable

    );

/**
@brief          Function used to inject the ES packet header into audio decoder
                already opened by the function #aui_audio_decoder_open.\n
                After injecting the ES packet header, ES data can be injected
                into the audio decoder by the function #aui_audio_decoder_write.

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be managed to inject
                                      the ES packet header
@param[in]      *p_pkt_header       = Pointer to the struct #aui_av_packet which
                                      collect ES data header information

@return         @b AUI_RTN_SUCCESS  = Injecting the ES packet header into audio
                                      decoder performed successfully
@return         @b Other_Values     = Injecting the ES packet header into audio
                                      decoder failed for some reasons
*/
AUI_RTN_CODE aui_audio_decoder_write_header (

    aui_hdl decoder,

    const aui_av_packet *p_pkt_header

    );

/**
@brief          Function used to inject fragments of the ES data into audio
                decoder already opened by the function #aui_audio_decoder_open.\n
                After injecting the ES data, the next ES packet header can be
                injected into the audio decoder by the function
                #aui_audio_decoder_write_header

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be managed to inject
                                      the ES data
@param[in]      puc_buf             = Pointer to the ES data buffer
@param[in]      ul_size             = Size of data in the ES data buffer

@return         @b AUI_RTN_SUCCESS  = Injecting the ES data into audio decoder
                                      performed successfully
@return         @b Other_Values     = Injecting the ES data into audio decoder
                                      failed for some reasons
*/
AUI_RTN_CODE aui_audio_decoder_write (

    aui_hdl decoder,

    const unsigned char *puc_buf,

    const unsigned long ul_size

    );

/**
@brief          Function used to flush the audio decoder already opened by the
                function #aui_audio_decoder_open.\n
                After using that function the data buffering in audio decoder
                is discarded then new ES packet header and ES data can be
                injected into audio decoder by, respectively, the functions
                #aui_audio_decoder_write_header and #aui_audio_decoder_write.

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be flushed.

@return         @b AUI_RTN_SUCCESS  = Flushing the audio decoder performed
                                      successfully
@return         @b Other_Values     = Flushing the audio decoder failed for
                                      some reasons
*/
AUI_RTN_CODE aui_audio_decoder_flush (

    aui_hdl decoder

    );

/**
@brief          Function used to get the current status of the buffer used to
                gather audio ES data. This function can be used after opening
                the audio decoder by the function #aui_audio_decoder_open

@param[in]      decoder             = #aui_hdl handle of the audio decoder
                                      already opened and to be managed to get
                                      the status of the buffer used to gather
                                      audio ES data

@param[out]     p_buffer_status     = Pointer to the struct #aui_decoder_buffer_status
                                      which collect the status of the buffer
                                      used to gather audio ES data

@return         @b AUI_RTN_SUCCESS  = Getting the current status of the buffer
                                      used to gather audio ES data performed
                                      successfully
@return         @b Other_Values     = Getting the current status of the buffer
                                      used to gather audio ES data failed for
                                      some reasons.
*/
AUI_RTN_CODE aui_audio_decoder_get_buffer_status (

    aui_hdl decoder,

    aui_decoder_buffer_status *p_buffer_status

    );

/**********************Functions for Video Decoder Only************************/

/**
@brief          Function used to open the video decoder and configure the desired
                attributes then get the related handle.

@warning        This function can @a only be used in the <b> Pre-Run stage </b>
                of the AV-Injection Module. In particular, after opening the
                Video Decoder user can:
                - Either configure the AV synchronization by the function
                  #aui_video_decoder_set_sync.
                - Or register a callback function by the function #aui_decv_set
                  with parameter #AUI_DECV_SET_REG_CALLBACK
                - Or inject ES packet header and ES data of video into video
                  decoder by, respectively, the functions
                  #aui_video_decoder_write_header and #aui_video_decoder_write

@param[in]      p_video_info        = Pointer to a struct #aui_video_decoder_init
                                      which collects video information to
                                      initialize/configure the video decoder

@param[out]     p_decoder_out       = #aui_hdl pointer to the handle of the video
                                      decoder just opened

@return         @b AUI_RTN_SUCCESS  = Opening the video decoder performed
                                      successfully
@return         @b Other_Values     = Opening the video decoder failed for
                                      some reasons
 */
AUI_RTN_CODE aui_video_decoder_open (

    aui_video_decoder_init *p_video_info,

    aui_hdl *p_decoder_out

    );

/**
@brief          Function used to close the video decoder already opened by the
                function #aui_video_decoder_open then the related handle will
                be release (i.e. the related resources such as memory, device)

@param[in]      decoder             = #aui_hdl the handle of the video decoder
                                      already opened and to be closed

@return         @b AUI_RTN_SUCCESS  = Closing the video decoder performed
                                      successfully

@return         @b Other_Values     = Closing of the video decoder failed
                                      for some reasons
 */
AUI_RTN_CODE aui_video_decoder_close (

    aui_hdl decoder

    );

/**
@brief          Function used to get the current status of the video decoder
                already opened by the function #aui_video_decoder_open

@param[in]      decoder             = #aui_hdl handle of the video decoder
                                      already opened and to be managed to get
                                      the current status

@param[out]     p_status            = Pointer to the struct #aui_video_decoder_status
                                      which collect the current status of video
                                      decoder

@return         @b AUI_RTN_SUCCESS  = Getting the current status of video decoder
                                      performed successfully
@return         @b Other_Values     = Getting the current status of video decoder
                                      failed for some reasons
*/
AUI_RTN_CODE aui_video_decoder_get_status (

    aui_hdl decoder,

    aui_video_decoder_status *p_status

    );

/**
@brief          Function used to set the desired synchronization mode of video
                decoder already opened by the function #aui_video_decoder_open

@param[in]      decoder             = #aui_hdl handle of the video decoder already
                                      opened and to be managed to set the desired
                                      synchronization
@param[in]      enable              = Flag to indicate which synchronization mode
                                      needs to be set, in particular:
                                      - @b 0 = Free run mode then video decoder
                                               plays video frames with frame rate
                                      - @b 1 = Enable synchronization then video
                                               decoder plays video frames with the
                                               STC (System Time Clock) provided by
                                               audio

@return         @b AUI_RTN_SUCCESS  = Setting the synchronization mode of video
                                      decoder performed successfully
@return         @b Other_Values     = Setting the synchronization mode of video
                                      decoder failed for some reasons

@note           In case of @a only video, the video will be played in <b> Free
                Run Mode </b>
*/
AUI_RTN_CODE aui_video_decoder_set_sync (

    aui_hdl decoder,

    const int enable

    );

/**
@brief          Function used to pause/resume the video decoder already opened
                by the function #aui_video_decoder_open

@param[in]      decoder             = #aui_hdl the handle of the video decoder
                                      already opened and to be paused/resumed
@param[in]      pause               = Flag to indicate which functionality needs
                                      to be performed, in particular:
                                      - @b 0 = Pause
                                      - @b 1 = Resume

@return         @b AUI_RTN_SUCCESS  = Video decoder paused/resumed successfully
@return         @b Other_Values     = Pausing/Resuming the video decoder failed
                                      for some reasons
*/
AUI_RTN_CODE aui_video_decoder_pause (

    aui_hdl decoder,

    const int pause

    );

/**
@brief          Function used to inject the ES packet header into video decoder
                already opened by the function #aui_video_decoder_open \n
                After injecting the ES packet header, ES data can be injected
                into the video decoder by the function
                #aui_video_decoder_write_header

@param[in]      decoder             = #aui_hdl handle of the video decoder already
                                      opened and to be managed to inject the ES
                                      packet header
@param[in]      p_pkt_hdr           = Pointer to the ES data header, as per the
                                      struct #aui_av_packet

@return         @b AUI_RTN_SUCCESS  = Injecting the ES packet header to video
                                      decoder performed successfully
@return         @b Other_Values     = Injecting the ES packet header to video
                                      decoder failed for some reasons
*/
AUI_RTN_CODE aui_video_decoder_write_header (

    aui_hdl decoder,

    const aui_av_packet_t *p_pkt_hdr

    );

/**
@brief          Function used to inject fragments of the ES data into video
                decoder already opened by the function #aui_video_decoder_open.\n
                After injecting the ES data, the next ES packet header can be
                injected into the video decoder by the function
                #aui_video_decoder_write_header

@param[in]      decoder             = #aui_hdl handle of the video decoder already
                                      opened and to be managed to inject the ES
                                      data
@param[in]      puc_buf             = Pointer to the ES data buffer
@param[in]      ul_size             = Size of data in the ES data buffer

@return         @b AUI_RTN_SUCCESS  = Injecting the ES data into video decoder
                                      performed successfully
@return         @b Other_Values     = Injecting the ES data into video decoder
                                      failed for some reasons
 */
AUI_RTN_CODE aui_video_decoder_write (

    aui_hdl decoder,

    const unsigned char *puc_buf,

    const unsigned long ul_size

    );

/**
@brief          Function used to flush the video decoder already opened by the
                function #aui_video_decoder_open \n
                After using that function, the data buffering in video decoder
                is discarded then the new ES packet header and ES data can be
                injected into video decoder by, respectively, the functions
                #aui_video_decoder_write_header and #aui_video_decoder_write

@param[in]      decoder                 = #aui_hdl the handle of the video
                                          decoder already opened and to be
                                          flushed
@param[in]      vdec_playback_mode      = The current video playback mode of
                                          video decoder, as per the enum
                                          #aui_vdec_playback_mode

@return         @b AUI_RTN_SUCCESS      = Flushing the video decoder performed
                                          successfully
@return         @b Other_Values         = Flushing the video decoder failed for
                                          some reasons
*/
AUI_RTN_CODE aui_video_decoder_flush (

    aui_hdl decoder,

    const aui_vdec_playback_mode vdec_playback_mode

    );

/**
@brief          Function used to get the current status of the buffer used to
                gather video ES data This function can be used after opening
                the video decoder by the function #aui_video_decoder_open

@param[in]      decoder             = #aui_hdl handle of the video decoder
                                      already opened and to be managed to get
                                      the status of the buffer used to gather
                                      video ES data

@param[out]     p_buffer_status     = Pointer to the struct
                                      #aui_decoder_buffer_status which collect
                                      the status of the buffer used to gather
                                      video ES data

@return         @b AUI_RTN_SUCCESS  = Getting the current status of the buffer
                                      used to gather video ES data performed
                                      successfully
@return         @b Other_Values     = Getting the current status of the buffer
                                      used to gather video ES data failed for
                                      some reasons
*/
AUI_RTN_CODE aui_video_decoder_get_buffer_status (

    aui_hdl decoder,

    aui_decoder_buffer_status *p_buffer_status

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

/// @cond

#if !defined(DOXYGEN_SKIP_DEPRECATED_AUI_API) && !defined(DEPRECATE_OLD_CODECID_IN_AUI_API)

/**
@warning This definition is no longer supported then is @a deprecated
*/
typedef unsigned long aui_audio_codec_id;

/**
@warning This definition is no longer supported then is @a deprecated
*/
typedef aui_hdl aui_audio_decoder_id;

/**
@warning This definition is no longer supported then is @a deprecated
*/
typedef unsigned long aui_video_codec_id;

/**
@warning This definition is no longer supported then is @a deprecated
*/
typedef aui_hdl aui_video_decoder_id;

/**
Enum to specify an ID for different Codec available to be used
*/
enum CodecID {

    /**
    Value to specify that no codec is used
    */
    CODEC_ID_NONE,

    /**
    PCM Codec IDs
    START
    */

    CODEC_ID_PCM_S16LE = 0x10000,

    CODEC_ID_PCM_S16BE,

    CODEC_ID_PCM_U16LE,

    CODEC_ID_PCM_U16BE,

    CODEC_ID_PCM_S8,

    CODEC_ID_PCM_U8,

    CODEC_ID_PCM_MULAW,

    CODEC_ID_PCM_ALAW,

    CODEC_ID_PCM_S32LE,

    CODEC_ID_PCM_S32BE,

    CODEC_ID_PCM_U32LE,

    CODEC_ID_PCM_U32BE,

    CODEC_ID_PCM_S24LE,

    CODEC_ID_PCM_S24BE,

    CODEC_ID_PCM_U24LE,

    CODEC_ID_PCM_U24BE,

    CODEC_ID_PCM_S24DAUD,

    CODEC_ID_PCM_ZORK,

    CODEC_ID_PCM_S16LE_PLANAR,

    CODEC_ID_PCM_DVD,

    CODEC_ID_PCM_F32BE,

    CODEC_ID_PCM_F32LE,

    CODEC_ID_PCM_F64BE,

    CODEC_ID_PCM_F64LE,

    CODEC_ID_PCM_BLURAY,

    CODEC_ID_PCM_LXF,

    CODEC_ID_S302M,

    /**
    PCM Codec IDs
    END
    */

    /**
    ADPCM Codec IDs
    START
    */

    CODEC_ID_ADPCM_IMA_QT = 0x11000,

    CODEC_ID_ADPCM_IMA_WAV,

    CODEC_ID_ADPCM_IMA_DK3,

    CODEC_ID_ADPCM_IMA_DK4,

    CODEC_ID_ADPCM_IMA_WS,

    CODEC_ID_ADPCM_IMA_SMJPEG,

    CODEC_ID_ADPCM_MS,

    CODEC_ID_ADPCM_4XM,

    CODEC_ID_ADPCM_XA,

    CODEC_ID_ADPCM_ADX,

    CODEC_ID_ADPCM_EA,

    CODEC_ID_ADPCM_G726,

    CODEC_ID_ADPCM_CT,

    CODEC_ID_ADPCM_SWF,

    CODEC_ID_ADPCM_YAMAHA,

    CODEC_ID_ADPCM_SBPRO_4,

    CODEC_ID_ADPCM_SBPRO_3,

    CODEC_ID_ADPCM_SBPRO_2,

    CODEC_ID_ADPCM_THP,

    CODEC_ID_ADPCM_IMA_AMV,

    CODEC_ID_ADPCM_EA_R1,

    CODEC_ID_ADPCM_EA_R3,

    CODEC_ID_ADPCM_EA_R2,

    CODEC_ID_ADPCM_IMA_EA_SEAD,

    CODEC_ID_ADPCM_IMA_EA_EACS,

    CODEC_ID_ADPCM_EA_XAS,

    CODEC_ID_ADPCM_EA_MAXIS_XA,

    CODEC_ID_ADPCM_IMA_ISS,

    CODEC_ID_ADPCM_G722,

    /**
    ADPCM Codec IDs
    END
    */

    /**
    AMR Codec IDs
    START
    */

    CODEC_ID_AMR_NB = 0x12000,

    CODEC_ID_AMR_WB,

    /**
    AMR Codec IDs
    END
    */

    /**
    RealAudio Codec IDs
    START
    */

    CODEC_ID_RA_144 = 0x13000,

    CODEC_ID_RA_288,

    /**
    RealAudio Codec IDs
    END
    */

    /**
    DPCM Codec IDs
    START
    */

    CODEC_ID_ROQ_DPCM = 0x14000,

    CODEC_ID_INTERPLAY_DPCM,

    CODEC_ID_XAN_DPCM,

    CODEC_ID_SOL_DPCM,

    /**
    DPCM Codec IDs
    END
    */

    /**
    Audio Codec IDs
    START
    */

    CODEC_ID_MP2 = 0x15000,

    /**
    For MPEG Audio Layer 1, 2 or 3
    */
    CODEC_ID_MP3,

    /**
    For AAC ADTS
    */
    CODEC_ID_AAC,

    CODEC_ID_AC3,

    CODEC_ID_DTS,

    CODEC_ID_VORBIS,

    CODEC_ID_DVAUDIO,

    CODEC_ID_WMAV1,

    CODEC_ID_WMAV2,

    CODEC_ID_MACE3,

    CODEC_ID_MACE6,

    CODEC_ID_VMDAUDIO,

    CODEC_ID_SONIC,

    CODEC_ID_SONIC_LS,

    CODEC_ID_FLAC,

    CODEC_ID_MP3ADU,

    CODEC_ID_MP3ON4,

    CODEC_ID_SHORTEN,

    CODEC_ID_ALAC,

    CODEC_ID_WESTWOOD_SND1,

    /**
    As in Berlin toast format
    */
    CODEC_ID_GSM,

    CODEC_ID_QDM2,

    CODEC_ID_COOK,

    CODEC_ID_TRUESPEECH,

    CODEC_ID_TTA,

    CODEC_ID_SMACKAUDIO,

    CODEC_ID_QCELP,

    CODEC_ID_WAVPACK,

    CODEC_ID_DSICINAUDIO,

    CODEC_ID_IMC,

    CODEC_ID_MUSEPACK7,

    CODEC_ID_MLP,

    /**
    As found in WAV
    */
    CODEC_ID_GSM_MS,

    CODEC_ID_ATRAC3,

    CODEC_ID_VOXWARE,

    CODEC_ID_APE,

    CODEC_ID_NELLYMOSER,

    CODEC_ID_MUSEPACK8,

    CODEC_ID_SPEEX,

    CODEC_ID_WMAVOICE,

    CODEC_ID_WMAPRO,

    CODEC_ID_WMALOSSLESS,

    CODEC_ID_ATRAC3P,

    CODEC_ID_EAC3,

    CODEC_ID_SIPR,

    CODEC_ID_MP1,

    CODEC_ID_TWINVQ,

    CODEC_ID_TRUEHD,

    CODEC_ID_MP4ALS,

    CODEC_ID_ATRAC1,

    CODEC_ID_BINKAUDIO_RDFT,

    CODEC_ID_BINKAUDIO_DCT,

    /**
    For AAC LATM
    */
    CODEC_ID_AAC_LATM,

    CODEC_ID_QDMC,

    CODEC_ID_CELT,

    /**
    Audio Codec IDs
    END
    */

    /**
    DVB Subtitle Codec IDs
    START
    */

    CODEC_ID_DVD_SUBTITLE = 0x17000,

    CODEC_ID_DVB_SUBTITLE,

    /**
    DVB Subtitle Codec IDs
    END
    */

    /**
    Raw UTF-8 Text Codec IDs
    START
    */

    CODEC_ID_TEXT,

    CODEC_ID_XSUB,

    CODEC_ID_SSA,

    CODEC_ID_MOV_TEXT,

    CODEC_ID_HDMV_PGS_SUBTITLE,

    CODEC_ID_DVB_TELETEXT,

    CODEC_ID_SRT,

    CODEC_ID_MICRODVD,

    /**
    Raw UTF-8 Text Codec IDs
    END
    */

    /**
    IDs for other specific kind of Codec
    (generally used for attachments)
    */
    CODEC_ID_TTF = 0x18000,

    /**
    ID to specify an unknown codec (like #CODEC_ID_NONE)
    but lavf should attempt to identify it
    */
    CODEC_ID_PROBE = 0x19000,

    /**
    Fake ID to specify a Raw MPEG-2 TS Stream
    (only used by libavformat)
    */
    CODEC_ID_MPEG2TS = 0x20000,

    /**
    ID to specify a @a dummy Codec for Streams containing only metadata
    information
    */
    CODEC_ID_FFMETADATA = 0x21000

};

typedef enum CodecID CodecID_t;

/**
Enum to specify an ID for different video coding format
*/
enum aui_video_dec_id_e {

    /**
    H.264
    */
    AUI_VIDEO_DEC_ID_H264 = 0x10000,

    /**
    XVID
    */
    AUI_VIDEO_DEC_ID_XVID,

    /**
    MPEG2
    */
    AUI_VIDEO_DEC_ID_MPG2,

    /**
    FLV1
    */
    AUI_VIDEO_DEC_ID_FLV1,

    /**
    VP8
    */
    AUI_VIDEO_DEC_ID_VP8,

    /**
    WVC1
    */
    AUI_VIDEO_DEC_ID_WVC1,

    /**
    WMV3
    */
    AUI_VIDEO_DEC_ID_WMV3,

    /**
    WX3
    */
    AUI_VIDEO_DEC_ID_WX3,

    /**
    RMVB
    */
    AUI_VIDEO_DEC_ID_RMVB,

    /**
    MJPG
    */
    AUI_VIDEO_DEC_ID_MJPG,

    /**
    VC1
    */
    AUI_VIDEO_DEC_ID_VC1,

    /**
    XD
    */
    AUI_VIDEO_DEC_ID_XD,


    AUI_VIDEO_DEC_ID_LAST

};

typedef enum aui_video_dec_id_e aui_video_dec_id;

#define aui_video_dec_mod_e aui_video_dec_mode

#define aui_video_dec_mod aui_video_dec_mode

#define aui_av_packet_flag_e aui_av_packet_flag

// #define vdec_playback_mode aui_vdec_playback_mode

typedef aui_vdec_playback_mode vdec_playback_mode;

#define aui_audio_info_init_s aui_audio_info_init

#define aui_audio_decoder_status_s aui_audio_decoder_status

#define aui_video_decoder_status_s aui_video_decoder_status

#define aui_decoder_buffer_status_s aui_decoder_buffer_status

#define aui_video_decoder_init_s aui_video_decoder_init

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

