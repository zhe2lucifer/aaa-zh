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
Current ALi Author: Nick.Li, Amu.Tu
Last update:        2017.05.08
-->

@file       aui_dmx.h

@brief      De-Multiplexing (DMX) Module

            There are many kinds of data in STB system, such as
            - Video
            - Audio
            - Subtitle and section tables
            - etc.

            DMX Module is a kind of data filter that can filter the data by PID
            in TS. The source of the DMX data could be from
            - Front-end (NIM Module)
            - Local storage device which stores PVR data

            <b> DMX Module </b> can output the filtered data to the buffer
            allocated by an user applications or drivers.

@note       For further details, please refer to the ALi document
            <b><em> ALi_AUI_Porting_Guide_Modules.pdf - Chapter "De-Multiplexing
            (DMX) Module"
            </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DMX_H

#define _AUI_DMX_H

/**************************Included Header File List***************************/

#include "aui_common.h"

#include "aui_deca.h"

/******************************Global Macro List*******************************/

/**
@warning    This macro is no longer supported then is @a deprecated
*/
#define AUI_MODULE_VERSION_NUM_DMX  (0X00020200)

/**
Macro to specify an invalid service ID
*/
#define AUI_DMX_SERV_ID_INVALID     (0xffffffff)

/**
Macro to specify an invalid descramble algorithm
*/
#define AUI_DMX_DSC_ALGO_INVALID    (0xffffffff)

/**
Macro to specify the maximum length of the section mask
 */
#define AUI_MAX_SEC_MASK_LEN         (16)

/**
Macro to specify the maximum number of PIDs which can be set in the struct
#aui_dmx_stream_pid
*/
#define AUI_DMX_STREAM_PIDS_CNT_MAX  (16)

/**
Macro to specify a invalid PID value
*/
#define AUI_INVALID_PID     (0x1fff)

/**
Macro to specify the special PID for recording the whole broadcast TS at the
tuned frequency
*/
#define AUI_FULL_TS_PID     (0x2000)

/**
Macro to specify the maximun number of DMX devices
 */
#define AUI_DMX_DEV_CNT_MAX             (16)

/**
Macro to specify the maximun size of the DMX channel buffer
*/
#define AUI_DMX_CHANNEL_BUF_LEN (128*1024)

/**
Macro to specify the maximun size of the DMX filter buffer
 */
#define AUI_DMX_FILTER_BUF_LEN (32*1024)

/**
Macro to specify the maximum number of PIDs for a record channel
*/
#define AUI_DMX_REC_PID_LIST_MAX_LEN    32

/**
Macro to specify the maximum number of PIDs which can be caches in a DMX device
*/
#define AUI_CACHE_PID_MAX 32

/**
Macro to specify that the DMX stream has been decrypted succesfully
*/
#define AUI_DMX_STREAM_DECRYPT_SUCCESS  (1)

/**
Macro to specify that the DMX Stream has not been decrypted successfully
*/
#define AUI_DMX_STREAM_DECRYPT_FAIL  (0)

/*******************************Global Type List*******************************/

/**
Enum to specifies the <b> DMX device ID </b>

@note   As per the enum #aui_en_dmx_dev_type, there are two (2) kinds of DMX
        device
        - Hardware, where each one is an Intellectual Property (IP) Module
          in ALi chipset and the filter operation is executed by hardware
        - Software, where each one is a virtual device that simulates the filter
          operation by software (generally speaking, software DMX device is used
          for video/audio playback)
*/
typedef enum aui_dmx_id {

    /**
    Value to specify the <b> first hardware </b> DMX device
    */
    AUI_DMX_ID_DEMUX0 = 0,

    /**
    Value to specify the <b> second hardware </b> DMX device
    */
    AUI_DMX_ID_DEMUX1,

    /**
    Value to specify the <b> first software </b> DMX device
    */
    AUI_DMX_ID_SW_DEMUX0,

    /**
    Value to specify the <b> third hardware </b> DMX device
    */
    AUI_DMX_ID_DEMUX2,

    /**
    Value to specify the <b> fourth hardware </b> DMX device
    */
    AUI_DMX_ID_DEMUX3,

    /**
    Value to specify the <b> second software </b> DMX device.
    the second software dmx is only supported in Linux project
    */
    AUI_DMX_ID_SW_DEMUX1 = 64,

    /**
    Value to specify the total number of DMX device ID in this enum
    */
    AUI_DMX_NB_DEMUX

} aui_dmx_id, aui_dmx_id_t;

/**
Enum to specify the <b> type of DMX device </b>, i.e. @b hardware or @b software
*/
typedef enum aui_en_dmx_dev_type {

    /**
    Enum to specify an @b Hardware DMX device
    */
    AUI_DMX_DEV_TYPE_HARDWARE=0,

    /**
    Enum to specify an @b Software DMX device
    */
    AUI_DMX_DEV_TYPE_SOFTWARE,

    /**
    Value to specify the total number of type of DMX device in this enum
    */
    AUI_DMX_DEV_TYPE_LAST

} aui_dmx_dev_type, *aui_p_dmx_dev_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the DMX
        device capability
        </div> @endhtmlonly

        Struct to specify the DMX device capability
*/
typedef struct aui_st_dmx_dev_capability {

    /**
    Member to specify the DMX device ID as defined in the enum #aui_dmx_id
    */
    unsigned long ul_dev_idx;

    /**
    Member to specify the type of DMX device as defined in the enum
    #aui_en_dmx_dev_type
    */
    aui_dmx_dev_type dev_type;

} aui_dmx_dev_capability, *aui_p_dmx_dev_capability;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        attributes available to be configured for DMX Module
        </div> @endhtmlonly

        Struct to specifies the attributes available to be configured for
        DMX Module
*/
typedef struct aui_st_dmx_module_attr {

    /**
    Member as a @a flag to specify whether DMX module have been initialized
    or not, in particular
    - @b 0 = DMX Module has not been initialized
    - @b 1 = DMX Module has been initialized
    */
    unsigned long ul_init_flag;

    /**
    Member to specify the total number of DMX devices
    */
    unsigned long ul_dev_cnt;

    /**
    Member as an array of struct #aui_dmx_dev_capability to specify the
    capability of all DMX devices (which maximum number is defined by the
    macro #AUI_DMX_DEV_CNT_MAX)
    */
    aui_dmx_dev_capability dmx_capability[AUI_DMX_DEV_CNT_MAX+1];

} aui_dmx_module_attr, *aui_p_dmx_module_attr;

/**
Enum to specify the <b> data path </b> of a DMX device
*/
typedef enum aui_en_dmx_data_path_type {

    /**
    Value to specify the path of playing FTA stream, which means
    - The input data to DMX is clear
    - The output data from DMX is clear

    where clear means the transport stream data is not scrambled.
    */
    AUI_DMX_DATA_PATH_CLEAR_PLAY=0,

    /**
    Value to specify the path of recording FTA stream, which means
    - The input data to DMX is clear
    - The output data from DMX is clear
    */
    AUI_DMX_DATA_PATH_REC,

    /**
    Value to specify the path of playing encrypted stream, which means
    - The input AV data to DMX had been encrypted
    - DMX will send the encrypt AV data to DSC to be decrypted
    */
    AUI_DMX_DATA_PATH_DE_PLAY,

    /**
    Enum to specify the path of recording encrypted stream from FTA stream,
    which means
    - The input data to DMX is clear
    - DMX will send the clear data to DSC to be re-encrypted
    */
    AUI_DMX_DATA_PATH_EN_REC,

    /**
    Value to specify the path of playing FTA stream and recording encrypted
    stream from FTA stream, which means
    - The input data to DMX is clear
    - DMX will send the clear data to DSC to be re-encrypted
    */
    AUI_DMX_DATA_PATH_CLEAR_PLAY_EN_REC,

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_DATA_PATH_DE_PLAY_REC,

    /**
    Value to specify the path of playing encrypted stream and recording
    re-encrypted stream from encrypted stream. which means
    - The input data to DMX had been encrypted
    - DMX will send the encrypted data to DSC to be re-encrypted
    */
    AUI_DMX_DATA_PATH_DE_PLAY_EN_REC,

    /**
    Value to specify the path of recording re-encrypted stream from encrypted
    stream, which means
    - The input data to DMX had been encrypted
    - DMX will send the encrypt data to DSC to be re-encrypted
    */
    AUI_DMX_DATA_PATH_REEN_REC

} aui_dmx_data_path_type, *aui_p_dmx_data_path_type;

/**
Enum to specify the DSC handle type used by a DMX device

@note   The function #aui_dmx_data_path_set accepts DSC objects as input
        parameters.\n
        In a single process application, the input parameters will be the
        handle of a DSC ooject.\n
        In a multi-process application, the DSC object will be represented
        by a structure #aui_dmx_dsc_id, which therefore will be the input
        parameter.\n
        So this enum is used to tell that function whether the input parameter
        as DSC object will be an handle or #aui_dmx_dsc_id structure.
*/
typedef enum aui_dmx_data_path_dsc_type {

    /**
    Value to specify that the DSC handle type used by a DMX device is
    #aui_hdl handle of DSC module

    @note   That's the common case when DSC and DMX device are in the same
            process.
    */
    AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE,

    /**
    Value to specify that the DSC handle type used by the a DMX device is the
    pointer to a structure #aui_dmx_dsc_id.

    @note  That's the case when DSC and DMX device are in different processes.
           In fact, the structure #aui_dmx_dsc_id is used to specifiy the DSC
           resource configured in another process.
    */
    AUI_DMX_DATA_PATH_DSC_TYPE_ID,

    /**
    Value to specify that the DSC handle type used by a DMX device is the file
    descriptor to the DSC device

    @warning  That case is reserved to ALi R&D Dept., so user can ignore it.
    */
    AUI_DMX_DATA_PATH_DSC_TYPE_FD

} aui_dmx_data_path_dsc_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify
        the DSC resources identifier attached to the current DMX device
        </div> @endhtmlonly

        Struct to specify the DSC resource identifier attached to the current
        DMX device by the function #aui_dmx_data_path_set

@note   In multi-process environment, the DSC #aui_hdl handle cannot be passed
        from a process to another one so this structure is used to represent
        the DSC resources attached to the current DMX device. And this structure
        can be recongnized by the DMX module in another process.
*/
typedef struct aui_dmx_dsc_id {

    /**
    Member as an array of DSC identifiers
    **/
    unsigned char identifier[16];

} aui_dmx_dsc_id;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the data
        path of a DMX and DSC device </div> @endhtmlonly

        Struct to specify the data path of a DMX and DSC device

*/
typedef struct aui_st_dmx_data_path {

    /**
    Member to specify the data path of a DMX device, as defined in the enum
    #aui_dmx_data_path_type
    */
    aui_dmx_data_path_type data_path_type;

    /**
    Member to specify the DSC handle type used by a DMX device, as per enum
    #aui_dmx_data_path_dsc_type.

    @note   If @b dsc_type = #AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE, the members of
            the present structure
            - @b p_hdl_de_dev and @b p_hdl_en_dev should be set as #aui_hdl of
              DSC module
            - @b p_dsc_id @a must be set as NULL

    @note   If @b dsc_type = #AUI_DMX_DATA_PATH_DSC_TYPE_ID, the members of the
            present structure
            - @b p_dsc_id @a must be set as the pointer of the struct #aui_dmx_dsc_id
              (which is returned by the funtion #aui_dmx_dsc_id_get)
            - @b p_hdl_de_dev and @b p_hdl_en_dev should be set as NULL
    */
    aui_dmx_data_path_dsc_type  dsc_type;

    /**
    Member as pointer to the handle of DSC device already opened by the function
    #aui_dsc_open and to be used for decryption
    */
    void *p_hdl_de_dev;

    /**
    Member as pointer to the handle of DSC device already opened by the function
    #aui_dsc_open and to be used for encryption
    */
    void *p_hdl_en_dev;

    /**
    Member as pointer to a structure #aui_dmx_dsc_id which is to represent a
    DSC object.

    @note  This structure is returned by the function #aui_dmx_dsc_id_get
    */
    aui_dmx_dsc_id* p_dsc_id;

} aui_dmx_data_path, *aui_p_dmx_data_path;

/// @cond

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify
        the Program Clock Reference (PCR) parameters
        </div> @endhtmlonly

        Struct to specify the Program Clock Reference (PCR) parameters
*/
typedef struct aui_dmx_pcr_param {

    /**
    Member to specify the most significant bit (MSB) of the PCR value
    **/
    unsigned int pcr_base_1msb;

    /**
    Member to specify the 32 least significant bits of the PCR value
    **/
    unsigned int pcr_base_32lsb;

    /**
    Member to specify more precision for the PCR base, in particular the
    extension to a 9-bit field in the 27 MHz unit
    */
    unsigned int pcr_extension_9b;

    /**
    @attention  This member is not uses currently then user can ignore it
    */
    unsigned int reserved;

    /**
    Member as a function pointer to the callback function which will be
    activated when DMX device receives the PCR data
    */
    void (* get_pcr_cb)(struct aui_dmx_pcr_param *);

    /**
    Member as pointer to the user data passed to the callback function
    #get_pcr_cb mentioned in this struct
    */
    void *param;

    /**
    Member to specify the PCR PID
    */
    unsigned short pid;

} aui_dmx_pcr_param;

/// @endcond

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        Presentation Time Stamp (PTS) value
        </div> @endhtmlonly

        Struct to specify the Presentation Time Stamp (PTS) value which
        consists of 33 bits divided into two parts
*/
typedef struct aui_pts {

    /**
    Member to specify the first part of the PTS value as its most significant
    bit (MSB)
    **/
    unsigned int pts_1msb;

    /**
    Member to specify the second part of the PTS value as the 32 least
    significant bits (LSB)
    **/
    unsigned int pts_32lsb;

} aui_pts;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        attributes of DMX devices </div> @endhtmlonly

        Struct to specify the attributes of DMX devices
*/
typedef struct aui_st_attr_dmx {

    /**
    Member to specify the DMX device ID as defined in the enum #aui_dmx_id
    */
    unsigned char uc_dev_idx;

    /**
    Member as a struct #aui_dmx_data_path to specify the path of DMX and DSC
    device
    */
    aui_dmx_data_path dmx_data_path;

    /**
    Member as a struct #aui_dmx_pcr_param to specify the PCR parameters
    */
    aui_dmx_pcr_param  dmx_pcr_info;

    /**
    Member as a pointer used for registering the  PCR Service to get the PCR
    Value when receiving PCR in TS stream
    */
    void* reg_pcr_serv;

    /**
    Member used to specify the DECV handle which will be used to decode video
    data from the DMX

    @note This member is intended @a only for PiP, for normal play please set
          it to NULL
    */
    aui_hdl decv_handle;

} aui_attr_dmx, *aui_p_attr_dmx;

/**
Function pointer to specify the callback function registered by the function
#aui_dmx_set and used by an user application to send the requested user data
to the software DMX device for the the playback

@note   About the parameters:
        - @b p_priv         = Input parameter as the pointer to the private
                              user data
        - @b ul_length      = Input parameter as the length of the requested
                              TS data from an user application
        - @b ppuc_buffer    = Output parameter as the pointer to a TS buffer
                              which will be filled by an user application
        - @b pul_actlen     = Output parameter as the pointer to the actual
                              length of the TS buffer written by an user
                              application
        - @b puc_fastcopy   = Output parameter not used currently then user
                              can ignore it
*/
typedef void (*aui_dmx_playfeed_requestdata_callback) (

    void *p_priv,

    unsigned long ul_length,

    unsigned char **ppuc_buffer,

    unsigned long *pul_actlen,

    unsigned char *puc_fastcopy

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the TS
        playback callback function
        </div> @endhtmlonly

        Struct to specify the TS playback callback function
*/
typedef struct aui_dmx_playfeed_callback {

    /**
    Member to specify the callback function used by an user application to send
    the requested user data to the software DMX device for the the playback, as
    per comment of the function pointer #aui_dmx_playfeed_requestdata_callback

    @note   This member is used @a only in project based on <b> Linux OS </b>
    */
    aui_dmx_playfeed_requestdata_callback request_data;

} aui_dmx_playfeed_callback, aui_dmx_playfeed_callback_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the TS
        playback parameters
        </div> @endhtmlonly

        Struct to specify the TS playback parameters.

@note   This struct is used @a only in project based on <b> Linux OS </b>
*/
typedef struct aui_dmx_playback_param {

    /**
    @attention  This member is not used currently then user can ignore it
    */
    unsigned char is_radio;

    /**
    Member to specify the TS playback callback function as defined in the
    struct #aui_dmx_playfeed_callback
    */
    aui_dmx_playfeed_callback cb_feed;

    /**
    Member as the pointer to the private user data passed as input parameter
    of the callback function mentioned in the member @b cb_feed of this struct
    */
    void                    *priv;

    /**
    Member to specify the size of the TS data which has been written into
    DMX device
    */
    unsigned long write_size;

} aui_dmx_playback_param, aui_dmx_playback_param_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        parameters used when injecting TS data to DMX device
        </div> @endhtmlonly

        Struct to specify the parameters used when injecting TS data to DMX
        device (generally used in TS playback)
*/
typedef struct aui_dmx_write_playback_param {

    /**
    Member to specify the length of the data which will be injected into
    DMX device
    */
    unsigned long length;

    /**
    Member as a pointer to the data buffer which will be injected into
    DMX device
    */
    unsigned char *start_buf;

    /**
    @attention  This member is not used currently then user can ignore it
    */
    unsigned char fastcopy;

} aui_dmx_write_playback_param, aui_dmx_write_playback_param_t;

/**
Enum to specify the Audio & Video Synchronization Mode
*/
typedef enum aui_dmx_avsync_mode {

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_AVSYNC_NONE = 0,

    /**
    Value to specify the synchronization mode which will @a only be used in
    live play, and Audio & Video will be synchronized with PCR respectively
    */
    AUI_DMX_AVSYNC_LIVE,

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_AVSYNC_PLAYBACK,

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_AVSYNC_TSG_TIMESHIT,

    /**
    Value to specify the synchronization mode which will @a only be used in
    TS playback

    @note   In this case, Audio will free run, and Video will synchronize
            ith audio
    */
    AUI_DMX_AVSYNC_TSG_LIVE

} aui_dmx_avsync_mode;

/**
Enum to specify the TS data source type, which will be used in AV synchronization
*/
typedef enum aui_avsync_srctype {

    /**
    Value to specify that the TS data are from the Tuner
    */
    AUI_AVSYNC_FROM_TUNER,

    /**
    Value to specify that the TS data are from the Software DMX device
    */
    AUI_AVSYNC_FROM_SWDMX,

    /**
    Value to specify that the TS data are from the Hard Disk (HD
    */
    AUI_AVSYNC_FROM_HDD_MP,

    /**
    Value to specify that the TS data are from the Network
    */
    AUI_AVSYNC_FROM_NETWORK_MP,

} aui_avsync_srctype, aui_avsync_srctype_t;

/**
Enum to specify miscellaneous settings available to be set for DMX device

@note   This enum is used by the function #aui_dmx_set to perform a specific
        setting where:
        - The parameter @b ul_item takes the item related to the specific
          setting to be performed
        - The parameter @b pv_param takes the pointer as per the description
          of the specific setting to be performed
*/
typedef enum aui_en_dmx_item_set {

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_TYPE_SET=0,

    /**
    Value to specify the creating of a pipeline to play Video & Audio

    @note   The parameter @b pv_param takes the pointer to the PID info and the
            stream type as defined in the struct #aui_dmx_stream_pid
    */
    AUI_DMX_STREAM_CREATE_AV,

    /**
    Value to specify the creating of a pipeline to play Audio

    @note   The parameter @b pv_param takes the pointer to the PID info and the
            stream type as defined in the struct #aui_dmx_stream_pid
    */
    AUI_DMX_STREAM_CREATE_AUDIO,

    /**
    Value to specify the creating of a pipeline to play Video

    @note   The parameter @b pv_param takes the pointer to the PID info and the
            stream type as defined in the struct #aui_dmx_stream_pid
    */
    AUI_DMX_STREAM_CREATE_VIDEO,

    /**
    Value to specify the disabling of the Video & Audio playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_DISABLE,

    /**
    Value to specify the disabling of the Audio playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_DISABLE_AUDIO,

    /**
    Value to specify the disabling of the Video playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_DISABLE_VIDEO,

    /**
    Value to specify the enabling of the Video & Audio playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_ENABLE,

    /**
    Value to specify the enabling of the Audio playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_ENABLE_AUDIO,

    /**
    Value to specify the enabling of the Video playing

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_STREAM_ENABLE_VIDEO,

    /**
    Value to specify the setting of DMX device whether bypassing CSA module
    when demultiplexing TS stream

    @note   The parameter @b pv_param indicate whether bypassing CSA module
            and can take the values as below:
            - @b 1 = Bypass CSA module
            - @b 0 = Not bypass CSA module
    */
    AUI_DMX_BYPASS_CSA,

    /**
    Value to specify the setting of the Video & Audio synchronization mode

    @note   In this case, the parameter @b pv_param is not used then can be
            set to NULL
    */
    AUI_DMX_SET_AVSYNC_MODE,

    /**
    Value to specify the setting of the playback parameters

    @note   The parameter @b pv_param takes the pointer to the playback
            parameters as defined in the struct #aui_dmx_playback_param
    */
    AUI_DMX_SET_PLAYBACK_PARAM,

    /**
    Value to specify the setting of the parameters when injecting TS data to
    DMX device

    @note   The parameter @b pv_param takes the pointer to the data buffer and
            length as defined in the struct #aui_dmx_write_playback_param
    */
    AUI_DMX_SET_DIRECT_WRITE,

    /**
    Value to specify the changing of the audio stream PID when playing

    @note   The parameter @b pv_param takes the pointer to the PID and stream
            type as defined in the struct #aui_dmx_stream_pid
    */
    AUI_DMX_SET_CHANGE_AUD_STREM,

    /**
    Value to specify the setting of the record mode to be used @a only in FTA
    stream

    @note   The parameter @b pv_param takes the pointer to the record mode,
            in particular
            - @b 0 = FTA to FTA
            - @b 1 = FTA to Encrypt
    */
    AUI_DMX_SET_IO_REC_MODE,

    /**
    Value to specify the setting of the TS data source type, which will be used
    in on AV synchronization

    @note   The parameter @b pv_param takes the pointer to the TS data source
            type as defined in the struct #aui_avsync_srctype
    */
    AUI_DMX_SET_AVSYNC_SOURCE_TYPE,

    /**
    @warning  This value is not longer used for any project based on either
              Linux or TDS so user can ignore it

    Value to specify the setting of the fast channel change parameters

    @note   The parameter @b pv_param takes the pointer to the PID and DMX cache
            type as defined in the struct #aui_dmx_cache_param_t
    */
    AUI_DMX_SET_FAST_CHANNEL_CHANGE_PARAM,

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_SET_PLAYBACK_SPEED,

    /**
    Value to clear the count of discontinuous video/audio packets received
    by the DMX device.

    @note   In this case, the parameter @b pv_param is not used then can be
            set to @b NULL value.
    */
    AUI_DMX_SET_CLEAR_DISCONTINUE_PKG_CNT,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_SET_CMD_LAST

} aui_dmx_item_set;

/**
Enum used to get miscellaneous information from the DMX device.

@note   This enum is used by the function #aui_dmx_get to get a specific
        information where:
        - The parameter @b ul_item takes the item related to the specific
          information to get
        - The parameter @b pv_param takes the pointer as per the description of
          the specific information to get
*/
typedef enum aui_en_dmx_item_get {

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_TYPE_GET=0,

    /**
    Value to get the attributes of DMX device

    @note   The parameter @b pv_param takes the pointer to the attributes of
            DMX devices as defined in the struct #aui_attr_dmx
    */
    AUI_DMX_ATTR_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DMX_HANDLE_SIZE_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DMX_CHANNEL_HANDLE_SIZE_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DMX_FILTER_HANDLE_SIZE_GET,

    /**
    Value to get the total number of Audio and Video TS packets received by
    the DMX device

    @note   The parameter @b pv_param takes the pointer to number of
            packets received by the DMX device

    */
    AUI_DMX_RCV_PKG_CNT_GET,

    /**
    Value to get the number of packets of a specified PID stream received by
    the DMX device

    @note   @b 1. This value is @a only for Video and Audio PID

    @note   @b 2. The parameter @b pv_param takes the pointer to the number of
                  packets of a specified PID stream received by the DMX device
                  as defined in the struct #aui_st_dmx_get_ts_pkg_cnt_by_pid
    */
    AUI_DMX_RCV_TS_PKG_CNT_GET_BY_PID,

    /**
    Value to get the length of the DMX buffer that is still free.

    @note   @b 1. The parameter @b pv_param takes the pointer to the length of the
                  DMX buffer that is still free
    @note   @b 2. For <b> Hardware DMX </b>, the length is in <em> 188 bytes </em> unit
    @note   @b 3. For <b> Software DMX </b>, the length is in <em> 1 byte </em> unit
    */
    AUI_DMX_GET_FREE_BUF_LEN,

    /**
    Value to get the length (in @a bytes unit) of the data written to software
    DMX device by the function #aui_dmx_set, where the parameter @b ul_item
    takes the value #AUI_DMX_SET_DIRECT_WRITE of the enum #aui_en_dmx_item_set

    @note   @b 1. Every time after calling that function with the parameter
                  mentioned, the function #aui_dmx_get, where the parameter
                  @b ul_item takes this value, should be called to check/compare
                  the written and expected data length

    @note   @b 2. The parameter @b pv_param takes the pointer to the length of
                  the DMX buffer that has been written
    */
    AUI_DMX_GET_WRITE_BUF_LEN,

    /**
    Value to get the total length (in <em> 1 byte </em> unit) of the DMX buffer.

    @note   @b 1. The parameter @b pv_param takes the pointer to the length of the
                  DMX buffer
    @note   @b 2. This value is only used for <b> Software DMX </b>
    */
    AUI_DMX_GET_TOTAL_BUF_LEN,

    /**
    Value to get the record mode which is @a only used in FTA Stream, in
    particular
    - @b 0 = FTA to FTA
    - @b 1 = FTA to Encrypt

    @note   The parameter @b pv_param takes the pointer to the record mode
    */
    AUI_DMX_GET_IO_REC_MODE,

    /**
    Value to get the Presentation Time Stamp (PTS) value

    @note   The parameter @b pv_param takes the pointer to the PTS value as
            defined in the struct #aui_pts
    */
    AUI_DMX_GET_PTS,

    /**
    @warning    This value is no longer supported then is @a deprecated

    @note   For <b> ALi R&D </b> @a only:
            1. This value is to get the type of encrypted stream when
               it is playing.\n
            2. The parameter @b pv_param takes the pointer to the
               encrypted stream type, as defined in the enum
               #aui_dmx_stream_encrypt_type
    */
    AUI_DMX_GET_STREAM_ENCRYPT_INFO,

    /**
    @warning    This value is no longer supported then is @a deprecated

    @note       For <b> ALi R&D </b> @a only:
                1. This value is to get information related to the decrypted
                   stream when it is playing
                2. The parameter @b pv_param takes the pointer to the decrypted
                   stream information, as defined in the struct
                   #aui_dmx_stream_pid_decrypt_info
                3. Referring to the struct #aui_dmx_stream_pid_decrypt_info,
                   before getting the value of the members @b ul_result and
                   @b ul_info, it's necessary to configure the PID in the member
                   @b ul_pids[] which @a only supports two (2) index values as
                   below:
                   - @b ul_pids[0] = Video PID
                   - @b ul_pids[1] = Audio PID
    */
    AUI_DMX_GET_STREAM_PID_DECRYPT_INFO,

    /**
    Value to get the STC value

    @note   The parameter @b pv_param takes the pointer to the STC value
    */
    AUI_DMX_GET_CUR_STC,

    /**
    Value to get the total bit rate (in bit per second (bps) unit) of audio
    and video when playing the live channel

    @note   The parameter @b pv_param takes the pointer to total bit rate of
            audio and video when playing the live channel
    */
    AUI_DMX_GET_PROG_BITRATE,

    /**
    Value to get the number of discontinuous video and audio packets received
    by the DMX device.

    @note  The parameter @b pv_param takes the pointer to number of discontinuous
           video and audio packets received by the DMX device
    */
    AUI_DMX_GET_DISCONTINUE_PKG_CNT,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_GET_CMD_LAST

} aui_dmx_item_get;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to get the number
        of TS packets related to the specified PID
        </div> @endhtmlonly

        Struct to get the number of TS packets related to the specified PID
*/
typedef struct aui_st_dmx_get_ts_pkg_cnt_by_pid {

    /**
    Member to specify the PID of which getting the related number of TS packets
    */
    unsigned long ul_pid;

    /**
    Member to specify the number of TS packets related to the specified PID
    */
    unsigned long ul_ts_pkg_cnt;

} aui_dmx_get_ts_pkg_cnt_by_pid, *aui_p_st_dmx_get_ts_pkg_cnt_by_pid;

/**
Enum to specify the type of output data of the DMX device
*/
typedef enum aui_en_dmx_data_type {

    /**
    Value to specify the type <b> Section Data </b>
    */
    AUI_DMX_DATA_SECT=0,

    /**
    Value to specify the type <b> Raw Data </b> (which is also for the TS
    header). The original data of a single PID stream will be captured without
    being decrypted or re-encrypted. No DSC will be involved, the latency time
    for getting data will be very short.

    @warning  In this mode, DMX will call the callback function specified by
              the function pointer #aui_p_fun_data_up_wtCB immediately when a
              TS packet is filtered out.\n\n

              To be highlighted the callback function will be called too
              frequently and cause performance issues if the data rate of the
              filtering PID is too high.

    @note     This mode can be used to filter raw data of low bit rate streams
              like PAT, PMT. To record high bitrate streams, please use the
              mode #AUI_DMX_DATA_REC instead.

    @note     This mode can only capture the data of a single PID channel, it
              can not filter multiple PIDs even all the PIDs of the whole TP. \n
              With reference to the struct #aui_st_attr_dmx_channel
              - The member @b us_pid must not be the value of the macro #AUI_FULL_TS_PID
              - The member @b ul_pid_cnt must be 1
    */
    AUI_DMX_DATA_RAW,

    /**
    Value to specify the type <b> Packetized Elementary Stream (PES) </b>
    */
    AUI_DMX_DATA_PES,

    /**
    Value to specify the type <b> Elementary Stream (ES) </b>

    @warning  This value is not supported currently then user can ignore it
    */
    AUI_DMX_DATA_ES,

    /**
    Value to specify the type <b> Record Data </b>\n

    If the descrambler and scrambler are used to process the TS data (they are
    set by the function #aui_dmx_data_path_set), the recorded data will be
    encrypted or re-encrypted.\n

    If no scramber is set by the function #aui_dmx_data_path_set, the original
    data will be captured. The orignal data can be either FTA stream or encrypted
    stream.

    @warning  DMX will accumulate the captured data in the output buffer.
              The callback function specified by the function pointer
              #aui_p_fun_data_up_wtCB will not be called until the output buffer
              is full.\n\n

              If the application needs to get TS data as soon as possible, please
              consider to use the mode #AUI_DMX_DATA_RAW instead

    @warning  If the orignal data is encrypted, recorded data must be encrypted
              as well
    */
    AUI_DMX_DATA_REC,

    /**
    @attention  This value is not used currently then user can ignore it
    */
    AUI_DMX_DATA_PCR,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_DATA_LAST

} aui_dmx_data_type, *aui_p_dmx_data_type;

/**
Enum to specify the status of the DMX channel
*/
typedef enum aui_en_dmx_channel_status {

    /**
    Value to specify the status @b Stopped
    */
    AUI_DMX_CHANNEL_STOP=0,

    /**
    Value to specify the status @b Running
    */
    AUI_DMX_CHANNEL_RUN,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_CHANNEL_RUN_STATUS_LAST

} aui_dmx_channel_status, *aui_p_dmx_channel_status;

/**
Enum to specify the status of the DMX filter
*/
typedef enum aui_en_dmx_filter_status {

    /**
    Value to specify the status @b Stopped
    */
    AUI_DMX_FILTER_STOP=0,

    /**
    Value to specify the status @b Running
    */
    AUI_DMX_FILTER_RUN,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_FILTER_LAST

} aui_dmx_filter_status, *aui_p_dmx_filter_status;

/**
Enum to specify the type of the DMX Stream
*/
typedef enum aui_en_dmx_stream_type {

    /**
    Value to specify the type @b Video and @b Audio
    */
    AUI_DMX_STREAM_AV=0,

    /**
    Value to specify the type @b Audio
    */
    AUI_DMX_STREAM_AUDIO,

    /**
    Value to specify the type <b> Audio Description </b>
    */
    AUI_DMX_STREAM_AUDIO_DESC,

    /**
    Value to specify the type @b Video
    */
    AUI_DMX_STREAM_VIDEO,

    /**
    Value to specify the type @b PCR
    */
    AUI_DMX_STREAM_PCR,

    /**
    Value to specify the type @b Subtitle

    @warning  The subtitle stream type is no longer supported. the application
              must parse and show subtitle by itself
    */
    AUI_DMX_STREAM_SUBTITLE,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_STREAM_LAST

} aui_dmx_stream_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        Stream Type of a specified PID
        </div> @endhtmlonly

        Struct to specify the Stream Type of a specified PID
*/
typedef struct aui_dmx_stream_pid {

    /**
    @attention  This member is not used currently then user can ignore it
    */
    unsigned long ul_magic_num;

    /**
    Member to specify the number of PIDs which will be set into a DMX device
    when creating a Video and Audio Stream, as defined in the macro
    #AUI_DMX_STREAM_PIDS_CNT_MAX
    */
    unsigned long ul_pid_cnt;

    /**
    Member as an array of PIDs which will bet set into a DMX device when
    creating a Video and Audio Stream

    @note   The maximum number of PIDs as length of this array is denoted by
            the macro #AUI_DMX_STREAM_PIDS_CNT_MAX
    */
    unsigned short aus_pids_val[AUI_DMX_STREAM_PIDS_CNT_MAX+1];

    /**
    Member as an array of enum #aui_dmx_stream_type to specify the type of DMX
    Stream of each corresponding PID specified in the member @b aus_pids_val
    of this struct
    */
    aui_dmx_stream_type stream_types[AUI_DMX_STREAM_PIDS_CNT_MAX+1];

    /**
    Member used to specify the handle of DECV which will be used to decode video
    data from the DMX

    @note  This member is intended @a only for PiP, for normal play please set it
           to NULL

    @attention  This member is deprecated, please set the DECV handle when open DMX
    */
    aui_hdl decv_handle;

} aui_dmx_stream_pid, *aui_p_dmx_stream_pid;

/**
Enum to specify the type of Stream Encryption
*/
typedef enum aui_en_dmx_stream_encrypt_type {

    /**
    Value to specify that the stream is @b clear
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM = 0,

    /**
    Value to specify that the <b> Video Stream </b> has been encrypted in
    <b> TS Level </b>
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_VDE_TS_SCRBL = 0x01,

    /**
    Value to specify that the <b> Video Stream </b> has been encrypted in
    <b> PES Level </b>
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_VDE_PES_SCRBL = 0x02,

    /**
    Value to specify tha the <b> Audio Stream </b> has been encrypted in
    <b> TS Level </b>
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_AUD_TS_SCRBL = 0x04,

    /**
    Value to specify that the <b> Audio Stream </b> has been encrypted in
    <b> PES Level </b>
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_AUD_PES_SCRBL = 0x08,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_STREAM_ENCRYPT_TYPE_LAST

} aui_dmx_stream_encrypt_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        decrypted information related to a specific PID
        </div> @endhtmlonly

        Struct to specify the decrypted information related to a specific PID
*/
typedef struct aui_st_dmx_stream_pid_decrypt_info {

    /**
    Member as an array of PIDs of which specify the related decrypted information

    @note   The maximum number of PIDs as length of this array is denoted by the
            macro #AUI_DMX_STREAM_PIDS_CNT_MAX
    */
    unsigned short ul_pids[AUI_DMX_STREAM_PIDS_CNT_MAX];

    /**
    Member as a flag to indicate the decryption result, in particular
    - @b 1 = Decryption performed successfully
    - @b 0 = Decryption failed for some reasons
    */
    unsigned long ul_result;

    /**
    Member to specify the decrypted information as defined in enum
    #aui_en_dmx_stream_encrypt_type
    */
    unsigned long ul_info;

} aui_dmx_stream_pid_decrypt_info;

/**
Function pointer to specify the callback function registered by the function
#aui_dmx_reg_sect_call_back and used to get the section data

@note   About the parameters:
        - @b filter_handle             = Input parameter as #aui_hdl handle of
                                         the DMX filter already opened by the
                                         function #aui_dmx_filter_open
        - @b puc_section_data_buf_addr = Input parameter as a pointer to the
                                         start address of section data buffer.
                                         - @b Caution: This buffer contains a
                                              complete section table, it is not
                                              the buffer to TS data.
        - @b ul_section_data_len       = Input parameter as the length of the
                                         section data (the maximun value of a
                                         section data will be 4096 bytes)
        - @b pv_usr_data               = Input parameter as a pointer to the
                                         user private parameters
        - @b pv_reserved               = Input parameter not used currently
                                         then user can ignore it

@warning  @b 1. All the callback functions are fired in the same event loop
                thread, the application should do minimum jobs in the callback
                otherwise the application can not be notified by callbacks
                quickly enough

@warning  @b 2. In the DMX callback functions, the following operations are not
                allowed because a deadlock may occur (the application should
                always try to avoid deadlock in callbacks):
                - Create/Destroy DMX channels or DMX filters
                - Stop/Close the DMX device
                - Wait for another callback
*/
typedef long (*aui_p_fun_sectionCB) (

    aui_hdl filter_handle,

    unsigned char *puc_section_data_buf_addr,

    unsigned long ul_section_data_len,

    void *pv_usr_data,

    void *pv_reserved

    );

/**
Function pointer to specify the callback function registered by the function
#aui_dmx_reg_data_call_back and used by an user application to get the buffer
start address and buffer length for storing the targeted data

@note   About the parameters:
        - @b filter_handle      = Input parameter as #aui_hdl handle of the
                                  DMX filter already opened by the function
                                  #aui_dmx_filter_open
        - @b ul_req_size        = Input parameter as the size of the buffer
                                  requested by the driver for storing the
                                  targeted data
        - @b pp_req_buf         = Output parameter as a pointer to the buffer
                                  start address  for storing the targeted data
        - @b pul_req_buf_size   = Output parameter as the size of the buffer
                                  actually allocate by an user application for
                                  storing the targeted data
        - @b p_ctrl_blk         = Output parameter not used currently then user
                                  can ignore it

@warning  @b 1. All the callback functions are fired in the same event loop
                thread, the application should do minimum jobs in the callback
                otherwise the application can not be notified by callbacks
                quickly enough

@warning  @b 2. In the DMX callback functions, the following operations are not
                allowed because a deadlock may occur (the application should
                always try to avoid deadlock in callbacks):
                - Create/Destroy DMX channels or DMX filters
                - Stop/Close the DMX device
                - Wait for another callback
*/
typedef long (*aui_p_fun_data_req_wtCB) (

    aui_hdl filter_handle,

    unsigned long ul_req_size,

    void **pp_req_buf,

    unsigned long *pul_req_buf_size,

    aui_avsync_ctrl *p_ctrl_blk

    );

/**
Function pointer to specify the callback function registered by the function
#aui_dmx_reg_data_call_back and used to notify user application about the size
of data which have been written into the buffer previously allocated by the
user application

@note   About the parameter:
        - @b filter_handle = Input parameter as #aui_hdl handle of the DMX
                             filter already opened by the function
                             #aui_dmx_filter_open
        - @b ul_size       = Output parameter as the size of data which have
                             been written into the buffer previously allocated
                             by the user application

@warning  @b 1. All the callback functions are fired in the same event loop
                thread, the application should do minimum jobs in the callback
                otherwise the application can not be notified by callbacks
                quickly enough

@warning  @b 2. In the DMX callback functions, the following operations are not
                allowed because a deadlock may occur (the application should
                always try to avoid deadlock in callbacks):
                - Create/Destroy DMX channels or DMX filters
                - Stop/Close the DMX device
                - Wait for another callback
*/
typedef long (*aui_p_fun_data_up_wtCB) (

    aui_hdl filter_handle,

    unsigned long ul_size

    );

/**
Function pointer to specify the callback function registered by the function
#aui_dmx_reg_pes_call_back to get the PES data

@note   About the parameter:
        - @b filter_handle  = Input parameter as #aui_hdl handle of the DMX
                              filter already opened by the function
                              #aui_dmx_filter_open
        - @b puc_buf        = Input parameter as the start address of PES data
                              buffer (it is a complete PES packet)
        - @b ul_size        = Output parameter as the length of PES data
        - @b pv_usr_data    = Output parameter as a pointer to the user private
                              parameters
*/
typedef long (*aui_pes_data_callback) (

    aui_hdl filter_handle,

    unsigned char *puc_buf,

    unsigned long ul_size,

    void *pv_usr_data

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the DMX
        ring buffer status
        </div> @endhtmlonly

        Struct to specify the DMX ring buffer status

@note   This struct is @a only used in project based on <b> TDS OS </b>
*/
typedef struct aui_st_dmx_data_overflow_rpt {

    /**
    Member to specify the <b> right side </b> of the DMX ring buffer
    */
    unsigned long ringbuf_right;

    /**
    Member to specify the <b> length of the right side </b> of the ring buffer
    */
    unsigned long len_right;

    /**
    Member to specify the <b> left side </b> of the ring buffer
    */
    unsigned long ringbuf_left;

    /**
    Member to specify the <b> length of the left side </b> of the ring buffer
    */
    unsigned long len_left;

} aui_dmx_data_overflow_rpt, *aui_p_st_dmx_data_overflow_rpt, aui_st_dmx_data_overflow_rpt;

/**
Enum to specify the type of the DMX events
*/
typedef enum aui_en_dmx_event_type {

    /**
    Enum to specify that the DMX data buffer will overflow soon then it
    necessary to get the DMX data as soon as possible
    */
    AUI_DMX_EVENT_SYNC_GET_DATA_OVERFLOW=0,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_EVENT_LAST

} aui_dmx_event_type;

/**
Enum to specify the type of handle for a DMX device
*/
typedef enum aui_en_dmx_handle_type {

    /**
    Enum to specify the <b> DMX Channel </b> handle type
    */
    AUI_DMX_HANDLE_CHANNEL=0,

    /**
    Enum to specify the <b> DMX Filter </b> handle type
    */
    AUI_DMX_HANDLE_FILTER,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_HANDLE_LAST

} aui_dmx_handle_type;

/**
Function pointer to specify the callback function registered with the
function #aui_dmx_channel_open and used to report a specific event to a
DMX Device

@note   About the parameter:
        - @b p_user_hdl     = Input parameter as a pointer to the handle of
                              either DMX channel or DMX Filter
        - @b handle_type    = Input parameter as the handle type for a DMX
                              device as defined in the enum #aui_dmx_handle_type
        - @b event_id       = Input parameter as the event type of DMX as
                              defined in the enum #aui_dmx_event_type
        - @b pv_rptdata     = Input parameter as a pointer to the event data
        - @b pv_usr_param   = Input parameter as a pointer to the user private
                              parameters
*/
typedef long (*aui_dmx_event_report) (

    void *p_user_hdl,

    aui_dmx_handle_type handle_type,

    aui_dmx_event_type event_id,

    void *pv_rptdata,

    void *pv_usr_param

    );

/**
@attention  This function pointer is currently not used then user can ignore it
*/
typedef int (*aui_dmx_ch_headnotfiy) (

    void *pv_black,

    unsigned char *puc_head_buf,

    int lenth,

    void *p_usr_param

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> <em> deprecated </em>
        </div> @endhtmlonly

@warning    This struct is no longer supported then is @a deprecated
*/
typedef struct aui_st_dmx_ch_head_nty {

    unsigned int head_notify_ref_size;

    aui_dmx_ch_headnotfiy ch_head_notify_cb;

    void* ch_head_cb_param;

} aui_dmx_ch_head_nty, *aui_p_dmx_ch_head_nty, aui_st_dmx_ch_head_nty;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        attributes of DMX Channel
        </div> @endhtmlonly

        Struct to specify the attributes of DMX Channel
*/
typedef struct aui_st_attr_dmx_channel {

    /**
    Member to specify the @b PID to be set into DMX Channel
    */
    unsigned short us_pid;

    /**
    Member to specify the <b> total number of PIDs </b> to be set, considering
    that the maximum value is defined by the macro #AUI_DMX_REC_PID_LIST_MAX_LEN

    @note   This member is @a only valid when the member @b us_pid of this
            struct is set to the value defined by the macro #AUI_INVALID_PID,
            and the DMX output data type is the one as specified by the enum
            value #AUI_DMX_DATA_REC.\n
            Other data types support only single PID instead.
    */
    unsigned long ul_pid_cnt;

    /**
    Member as an <b> array of PIDs </b> to be set into DMX Channel

    @note   This member is @a only valid when the member @b us_pid of this
            struct is set to the value defined by the macro #AUI_INVALID_PID,
            and the DMX output data type is the one as specified by the enum
            value #AUI_DMX_DATA_REC.\n
            Other data types support only single PID instead.
    */
    unsigned short us_pid_list[AUI_DMX_REC_PID_LIST_MAX_LEN];

    /**
    Member to specify the @b status of the DMX Channel as defined in the enum
    #aui_dmx_channel_status
    */
    aui_dmx_channel_status dmx_channel_status;

    /**
    Member to specify the <b> type of output data </b> of a DMX device as
    defined in the enum #aui_dmx_data_type
    */
    aui_dmx_data_type dmx_data_type;

    /**
    Member as a flag to specify whether the PID stream filtered by the filters
    in the current DMX channel is scrambled or not, in particular
    - @b 1 = The PID stream is encrypted
    - @b 0 = The PID stream is not encrypted

    @attention  If the PID stream under filtering is encrypted (e.g. encrypted
                teletext stream, encrypted subtitle stream, etc.), the value of
                this member @a must be set as 1
    */
    unsigned int is_encrypted:1;

    /**
    Member to specify that DMX channel can get section data during a
    synchronization mode. In particular, the following functions are used to get
    section data:
    - #aui_dmx_channel_sync_get_section_data
    - #aui_dmx_channel_sync_get_section_data_ext
    */
    unsigned int dmx_channel_sec_sync_get_data_support:1;

    /**
    Member to specify that DMX channel can get PES data
    */
    unsigned int dmx_channel_pes_callback_support:1;

    /**
    @attention  This member is not currently used then user can ignore it
    */
    unsigned int reserved:28;

    /**
    Member to specify the number of auto resets of the buffer performed by a
    driver when an overflow occurs
    */
    unsigned int overflow_auto_rst_buf_cnt;

    /**
    Member to specify the number of overflows occurred
    */
    unsigned int overflow_rpt_cnt;

    /**
    Member to specify the callback function used to report a specific event to
    a DMX Device, as per comment for the function pointer #aui_dmx_event_report
    */
    aui_dmx_event_report  event_cb ;

    /**
    Member as a pointer to the parameter to be passed to the callback function
    mentioned in the member @b event_cb of this struct
    */
    void* event_cb_param;

} aui_attr_dmx_channel, *aui_p_attr_dmx_channel;

/**
Enum to specify miscellaneous settings for a DMX channel available to be
performed.\n

@note   This enum is used by the function #aui_dmx_set to perform a specific
        setting where:
        - The parameter @b ul_item takes the item related to the specific setting to
          perform
        - The parameter @b pv_param takes the pointer as per the description of the
          specific setting to perform
*/
typedef enum aui_en_dmx_channel_item_set {

    /**
    Value used to set the <b> DMX Channel PID </b>

    @note   The parameter @b pv_param takes the pointer to the PID to be set
            into DMX Channel
    */
    AUI_DMX_CHANNEL_PID_SET=0,

    /**
    @attention  This value is currently not used then user can ignore it
    */
    AUI_DMX_CHANNEL_SET_CHANNEL_HEAD_CB,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_CHANNEL_GET_CMD_LAST

} aui_dmx_channel_item_set;

/**
Enum to specify miscellaneous information available to be gotten from a DMX
channel

@note   This enum is used by the function #aui_dmx_channel_get to get a
        specific information where:
        - The parameter @b ul_item takes the item related to the specific information
          to get
        - The parameter @b pv_param takes the pointer as per the description of the
          specific information to get
*/
typedef enum aui_en_dmx_channel_item_get {

    /**
    Value used to get the <b> DMX Channel PID </b>

    @note   The parameter @b pv_param takes the pointer to the PID of DMX
            Channel
    */
    AUI_DMX_CHANNEL_PID_GET=0,

    /**
    Value used to get the DMX Channel attributes

    @note   The parameter @b pv_param takes the pointer to the DMX Channel
            attributes as defined in the struct #aui_attr_dmx_channel
    */
    AUI_DMX_CHANNEL_ATTR_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DMX_CHANNEL_GET_DMX_HDL,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_CHANNEL_SET_CMD_LAST

} aui_dmx_channel_item_get;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        attributes of DMX Filter </div> @endhtmlonly

        Struct to specify the attributes of DMX Filter
*/
typedef struct aui_st_attr_dmx_filter {

    /**
    Member to specify the <b> DMX Filter Status </b> as defined in the enum
    #aui_dmx_filter_status
    */
    aui_dmx_filter_status dmx_filter_status;

    /**
    Member to specify the <b> DMX Filter Mask </b> to be applied to the values
    specified in the member @b puc_val of this struct
    */
    unsigned char *puc_mask;

    /**
    Member to specify the values to be checked by the DMX Filter Mask specified
    in the member @b puc_mask of this struct
    */
    unsigned char *puc_val;

    /**
    Member to specify the <b> Reversed DMX Filter Mask </b>
    */
    unsigned char *puc_reverse;

    /**
    Member to specify the length of the DMX Filter Mask defined in the member
    @b puc_mask of this function
    */
    unsigned long ul_mask_val_len;

    /**
    Member as a @a flag to specify whether checking the CRC of the section
    data or not, in particular:
    - @b 0 = No check
    - @b 1 = Check
    */
    unsigned char uc_crc_check;

    /**
    Member as a @a flag to specify whether getting the section data continuously
    or get the section data only one time, in particular.
    - @b 1 = Get the section data continuously;
    - @b 0 = Get the section data only one time
    */
    unsigned char uc_continue_capture_flag;

    /**
    Member to specify the number of the captured sections
    */
    unsigned char uc_hit_sect_cnt;

    /**
    Member to specify the callback function to get the section data, as per
    comment for the function pointer #aui_p_fun_sectionCB
    */
    aui_p_fun_sectionCB p_fun_sectionCB ;

    /**
    Member to specify the callback function to get the buffer start address and
    buffer length for storing the targeted data, as per comment for the function
    pointer #aui_p_fun_data_req_wtCB
    */
    aui_p_fun_data_req_wtCB p_fun_data_req_wtCB ;

    /**
    Member to specify the callback function to notify an user application about
    the size of data which have been written into the buffer previously
    allocated by the user application, as per comment for the function pointer
    #aui_p_fun_data_up_wtCB
    */
    aui_p_fun_data_up_wtCB p_fun_data_up_wtCB ;

    /**
    Member to specify the callback function to get PES data, as per comment for
    the function pointer #aui_pes_data_callback
    */
    aui_pes_data_callback callback;

    /**
    Member as pointer to the parameter to be passed to a PES callback function
    specified in the member @b callback of this struct
    */
    void* callback_param;

    /**
    Member to specify that the DMX filter supports the getting of the section
    data in a synchronization mode
    */
    unsigned int dmx_fil_sec_data_sync_get_support :1;

    /**
    This value is currently not used then user can ignore it
    */
    unsigned int  reserv:29;

    /**
    Member as pointer to the user data to be passed to a section callback function
    specified in the member @b p_fun_sectionCB of this struct
    */
    void *usr_data;

    /**
    Member to specify the number of auto resets of the buffer performed by a
    driver when an overflow occurs
    */
    unsigned int overflow_auto_rst_buf_cnt;

    /**
    Member to specify the number of overflows occurred
    */
    unsigned int overflow_rpt_cnt;

    /**
    Member to specify the callback function to report a specific event to a DMX
    Filter, as per comment for the function pointer #aui_dmx_event_report
    */
    aui_dmx_event_report  event_cb ;

    /**
    Member as pointer to the parameter to be passed to the callback function
    specified in the member @b event_cb of this struct
    */
    void *event_cb_param;

} aui_attr_dmx_filter, *aui_p_attr_dmx_filter;

/**
@warning    This enum is no longer supported then is @a deprecated
*/
typedef enum aui_en_dmx_filter_item_set {

    AUI_DMX_FILTER_TYPE_SET=0,

    AUI_DMX_FILTER_MASKVAL_SET,

    AUI_DMX_FILTER_CONFIG,

    AUI_DMX_FILTER_GET_CMD_LAST

} aui_dmx_filter_item_set;

/**
Enum to specify miscellaneous information available to be gotten from DMX Filter

@note   This enum is used by the function #aui_dmx_channel_get to get a specific
        information where:
        - The parameter @b ul_item takes the item related to the specific information
          to get
        - The parameter @b pv_param takes the pointer as per the description of the
          specific information to get
*/
typedef enum aui_en_dmx_filter_item_get {

    /**
    This value is currently not used then user can ignore it
    */
    AUI_DMX_FILTER_TYPE_GET=0,

    /**
    Value to specify the DMX Channel Attributes

    @note   The parameter @b pv_param takes the pointer to the DMX Channel
            Attributes, as defined in the struct #aui_attr_dmx_filter
    */
    AUI_DMX_FILTER_ATTR_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DMX_FILTER_GET_CHANNEL_HDL,

    /**
    Value to specify the maximum number of items in this enum
    */
    AUI_DMX_FILTER_SET_CMD_LAST

} aui_dmx_filter_item_get;

/**
Enum to specify the <b> DMX Cache Type </b>

@note   This enum is used only in project based on <b> Linux OS </b>
*/
typedef enum aui_dmx_cache_type {

    /**
    Value to specify no cache in the DMX device
    */
    AUI_DMX_CACHE_NONE = 0,

    /**
    Value to specify the specific PID cached in the DMX device
    */
    AUI_DMX_CACHE_PID,

    /**
    Value to specify that the whole TP is cached in the DMX device
    */
    AUI_DMX_CACHE_TP

} aui_dmx_cache_type, aui_dmx_cache_type_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify
        the attributes of DMX Cache
        </div> @endhtmlonly

        Struct to specify the attributes of DMX Cache

@note   @b 1. This struct is generally used in fast channel change
@note   @b 2. This struct is used @a only in project based on <b> Linux OS </b>
*/
typedef struct aui_dmx_cache_param {

    /**
    Member to specify the DMX Cache Type, as defined in the enum
    #aui_dmx_cache_type
    */
    aui_dmx_cache_type type;

    /**
    Member to specify the number of PIDs which need to be cached in DMX device

    @note   This member is @a only used when the DMX Cache Type is set to the
            value @b AUI_DMX_CACHE_PID of the enum #aui_dmx_cache_type
    */
    unsigned int pid_list_len;

    /**
    Member to specify an array of PIDs to be cached in DMX device

    @note   This member is @a only used when the DMX Cache Type is set to the
            value @b AUI_DMX_CACHE_PID of the enum #aui_dmx_cache_type
    */
    short int pid_list[AUI_CACHE_PID_MAX];

} aui_dmx_cache_param, aui_dmx_cache_param_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> De-Multiplexing (DMX) Module </b> to specify the
        attributes for adding/deleting PIDs for TS recording dynamically
        </div> @endhtmlonly

        Struct to specify the attributes for adding/deleting PIDs for TS
        recording dynamically
*/
typedef struct aui_dmx_channel_pid_list {

    /**
    Member as pointer to the PID to be added/deleted
    */
    unsigned short  *pids;

    /**
    Member to specify the number of the PID to be added/deleted
    */
    unsigned int pid_cnt;

} aui_dmx_channel_pid_list;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C"

{

#endif

/**
@brief          Function used to get the the version number of DMX Module

@param[out]     pul_version             = Pointer to the version number of
                                          DMX module

@return         @b AUI_RTN_SUCCESS      = Getting of the version number of
                                          DMX Module performed successfully
@return         @b Other_Values         = Getting of the version number of DMX
                                          Module failed for some reasons
*/
AUI_RTN_CODE aui_dmx_version_get (

    unsigned long* const pul_version

    );

/**
@brief          Function used to perform the initialization of the DMX Module
                before its opening by the function #aui_dmx_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the DMX Module

@param[in]      p_call_back_init        = Callback function used to initialize
                                          the DMX Module, as per comment for
                                          the function pointer #p_fun_cb
@param[in]      pv_param                = Pointer to the input parameter of the
                                          callback function @b p_call_back_init

@return         @b AUI_RTN_SUCCESS      = Initializing of the DMX Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Initializing of the DMX Module failed
                                          for some reason
*/
AUI_RTN_CODE aui_dmx_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the DMX Module
                after its closing by the function #aui_dmx_close

@param[in]      p_call_back_init        = Callback function used to de-initialize
                                          the DMX Module, as per comment for the
                                          function pointer #p_fun_cb
@param[in]      pv_param                = Pointer to the input parameter of the
                                          callback function @b p_call_back_init
                                          - @b Caution: Usually it is set to
                                               @b NULL

@return         @b AUI_RTN_SUCCESS      = De-initializing of the DMX Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = De-initializing of the DMX Module
                                          failed for some reasons

@note           This function is used @a only in projects based on <b> TDS OS
                </b>, and it is suggested only to use before going into
                standby mode
*/
AUI_RTN_CODE aui_dmx_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to open a specific DMX device and configure the
                desired attributes then get the related handle

@param[in]      *p_attr_dmx             = Pointer to a struct #aui_attr_dmx,
                                          which collects the desired attributes
                                          for the DMX device

@param[out]     *p_hdl_dmx              = #aui_hdl pointer to the handle of
                                          the specific DMX device just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DMX device failed
                                          for some reasons

@note           Before calling this function, please call the function
                #aui_find_dev_by_idx to check whether the specific DMX device
                has been opened or not.\n
                If the specific DMX device had been opened then it is not suggest
                to call this function to open it again
*/
AUI_RTN_CODE aui_dmx_open (

    const aui_attr_dmx *p_attr_dmx,

    aui_hdl* const p_hdl_dmx

    );

/**
@brief          Function used to close the specific DMX device already opened
                by the function #aui_dmx_open then the related handle will be
                released (i.e. the related resources such as DMX channel,
                filter, device)

@param[in]      *hdl_dmx                = #aui_hdl pointer to the handle of the
                                          specific DMX device already opened and
                                          to be closed

@return         @b AUI_RTN_SUCCESS      = Closing of the specific DMX device
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameters (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Closing of the specific DMX device
                                          failed for same reasons

@note           This function can be used @a only in projects based on <b> TDS
                OS </b>, it is suggested to  only use this function before to
                go into standby mode
*/

AUI_RTN_CODE aui_dmx_close (

    aui_hdl hdl_dmx

    );

/**
@brief          Function used to get the number of supported DMX devices

@param[out]     pul_dmx_cnt             = Pointer to the number of supported
                                          DMX devices

@return         @b AUI_RTN_SUCCESS      = Getting of the number of the
                                          supported DMX devices performed
                                          successfully
@return         @b Other_Values         = Getting of the number of the
                                          supported DMX devices failed
                                          for some reasons

*/
AUI_RTN_CODE aui_dmx_dev_cnt_get (

    unsigned long* const pul_dmx_cnt

    );

/**
@brief          Function used to get information about DMX device capability

@param[in]      ul_dev_idx              = ID of the specific DMX device, as
                                          defined in the enum #aui_dmx_id

@param[out]     p_capability_info       = Pointer to a struct
                                          #aui_dmx_dev_capability, which
                                          contains information about DMX device
                                          capability

@return         @b AUI_RTN_SUCCESS      = Getting of the DMX device capability
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the DMX device capability
                                          failed for some reasons

*/
AUI_RTN_CODE aui_dmx_capability_get (

    unsigned long ul_dev_idx,

    aui_dmx_dev_capability* const p_capability_info

    );

/**
@brief          Function used to set the data path of a DMX device.\n
                The data path indicates whether the specific DMX device needs
                to send the TS data to DSC device for decrypting or encrypting

@attention      This function must be called after finishing to work with
                AUI PVR playback (i.e. after calling the function
                #aui_pvr_ply_close).\n
                AUI PVR playback may change current DMX data path.

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened
@param[in]      *p_dmx_data_path_info   = Pointer to a struct #aui_dmx_data_path
                                          which is to specify the data path of
                                          a DMX and DSC device

@return         @b AUI_RTN_SUCCESS      = Setting of the data path of a DMX
                                          device performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the data path of a DMX
                                          device failed for some reasons


*/
AUI_RTN_CODE aui_dmx_data_path_set (

    aui_hdl hdl_dmx,

    const aui_dmx_data_path *p_dmx_data_path_info

    );

/**
@brief          Function used to start a DMX device already opened by the
                function #aui_dmx_open

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and to be started
@param[in]      p_attr_dmx              = Pointer to a struct #aui_attr_dmx
                                          which collects the DMX device attributes
                                          - @b Caution: This parameter is
                                               @a reserved to ALi R&D Dept. then
                                               user can set it to NULL

@return         @b AUI_RTN_SUCCESS      = Starting of the DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = A least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Starting of the DMX device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_start (

    aui_hdl hdl_dmx,

    const aui_attr_dmx *p_attr_dmx

    );

/**
@brief          Function used to stop a DMX device already opened and started by,
                respectively, the functions #aui_dmx_open and #aui_dmx_start

@warning        After stopping the specific DMX device, user can call the
                function #aui_dmx_start to start it again

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and started
                                          then to be stopped
@param[in]      p_attr_dmx              = Pointer to a struct #aui_attr_dmx
                                          which collects the DMX device attributes
                                          - @b Caution: This parameter is
                                               @a reserved to ALi R&D Dept.
                                               then user can set it to NULL

@return         @b AUI_RTN_SUCCESS      = Stopping of the DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = A least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Stopping of the DMX device failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dmx_stop (

    aui_hdl hdl_dmx,

    const aui_attr_dmx *p_attr_dmx

    );

/**
@brief          Function used to pause a DMX device already opened and started by,
                respectively, the functions #aui_dmx_open and #aui_dmx_start

@warning        After pausing the specific DMX device, all channels and filters
                belonging to it will be paused then user can resume by the
                function #aui_dmx_resume

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and started
                                          then to be stopped.
@param[in]      p_attr_dmx              = Pointer to a struct #aui_attr_dmx
                                          which collects the DMX device attributes
                                          - @b Caution: This parameter is
                                               @a reserved to ALi R&D Dept. then
                                               user can set it to NULL

@return         @b AUI_RTN_SUCCESS      = Pausing of the DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = A least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Pausing of the DMX device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_pause (

    aui_hdl hdl_dmx,

    const aui_attr_dmx *p_attr_dmx

    );

/**
@brief          Function used to resume a DMX device already opened, started
                and paused by, respectively, the functions #aui_dmx_open,
                #aui_dmx_start and #aui_dmx_pause

@warning        After resuming the specific DMX device, all channels and
                filters belonging to it will be resumed

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened, started, paused
                                          then to be resumed
@param[in]      p_attr_dmx              = Pointer to a struct #aui_attr_dmx which
                                          collects the DMX device attributes
                                          - @b Caution: This parameter is
                                               @a reserved to ALi R&D Dept.
                                               then user can set it to NULL

@return         @b AUI_RTN_SUCCESS      = Resuming of the DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = A least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Resuming of the DMX device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_resume (

    aui_hdl hdl_dmx,

    const aui_attr_dmx *p_attr_dmx

    );

/**
@brief          Function used to perform a specific setting for a specific DMX
                device between its opening and starting by, respectively, the
                functions #aui_dmx_open and #aui_dmx_start

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and to be
                                          managed to perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting to be performed, as defined
                                          in the enum #aui_en_dmx_item_set
@param[in]      *pv_param               = The pointer as per the description
                                          of the specific setting to be
                                          performed, as defined in the enum
                                          #aui_en_dmx_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting for a DMX device
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Specific setting for a DMX device
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dmx_set (

    aui_hdl hdl_dmx,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get miscellaneous information from a DMX device
                after its opening and starting by, respectively, the functions
                #aui_dmx_open and #aui_dmx_start

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and to be managed
                                          to get miscellaneous information
@param[in]      ul_item                 = The item related to the specific
                                          information of the specific DMX device
                                          to be gotten, as defined in the enum
                                          #aui_en_dmx_item_get

@param[out]     *pv_param               = The pointer as per the description of
                                          the specific information of the
                                          specific DMX device to be gotten, as
                                          defined in the enum #aui_en_dmx_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific information of
                                          the specific DMX device performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific information of
                                          the specific DMX device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_get (

    aui_hdl hdl_dmx,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to open a DMX channel from a specific DMX device
                and configure the desired attributes then get the DMX channel
                handle

@param[in]      hdl_dmx                 = #aui_hdl handle of the specific DMX
                                          device already opened and to be managed
                                          to open a DMX channel from it
@param[in]      *p_attr_channel         = Pointer to a struct
                                          #aui_attr_dmx_channel, which collects
                                          the desired attributes for the DMX
                                          channel to be opened

@param[out]     *p_hdl_dmx_channel      = #aui_hdl pointer to the handle of the
                                          DMX channel just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the DMX channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DMX channel failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_channel_open (

    aui_hdl hdl_dmx,

    const aui_attr_dmx_channel *p_attr_channel,

    aui_hdl* const p_hdl_dmx_channel

    );

/**
@brief          Function used to close the specific DMX channel already opened
                by the function #aui_dmx_channel_open then the related handle
                and all filters belonging to it will be closed

@param[in]      *p_hdl_dmx_channel      = #aui_hdl pointer to the handle of the
                                          specific DMX channel already opened
                                          and to be closed

@return         @b AUI_RTN_SUCCESS      = Closing of the specific DMX channel
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Closing of the the specific DMX
                                          channel failed for same reasons
*/
AUI_RTN_CODE aui_dmx_channel_close (

    aui_hdl *p_hdl_dmx_channel

    );

/**
@brief          Function used to start the specific DMX channel already opened
                by the function #aui_dmx_channel_open

@param[in]      hdl_dmx_channel         = #aui_hdl handle of DMX Channel already
                                          opened and to be started
@param[in]      *p_attr_channel         = Pointer to a struct
                                          #aui_attr_dmx_channel, which collects
                                          the desired attributes for the DMX
                                          channel
                                          - @b Caution: User can change the
                                               attributes of the specific DMX
                                               channel by the parameter
                                               @b p_attr_channel, although the
                                               DMX channel's attribution has
                                               already been set by the function
                                               #aui_dmx_channel_open.\n
                                               If no change is necessary then
                                               user can set this parameter to
                                               @b NULL

@return         @b AUI_RTN_SUCCESS      = Starting of the specific DMX channel
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Starting of the the specific DMX
                                          channel failed for same reasons

@note           The parameter @b p_attr_channel can @a only be used in projects
                based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_dmx_channel_start (

    aui_hdl hdl_dmx_channel,

    const aui_attr_dmx_channel *p_attr_channel

    );

/**
@brief          Function used to stop the specific DMX channel already opened
                and started by, respectively, the functions #aui_dmx_channel_open
                and #aui_dmx_channel_start

@warning        After stopping the specific DMX channel, user can call the
                function #aui_dmx_filter_start to start it again.

@param[in]      hdl_dmx_channel         = #aui_hdl handle of the specific DMX
                                          channel already opened and started
                                          then to be stopped
@param[in]      *p_attr_channel         = Pointer to a struct
                                          #aui_attr_dmx_channel which the
                                          attributes for the DMCX Channel
                                          - @b Caution: This parameter is not
                                               used currently then user can
                                               set it to NULL

@return         @b AUI_RTN_SUCCESS      = Stopping of the te specific DMX
                                          channel stopped successfully
@return         @b AUI_RTN_EINVAL       = At Least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Stopping of the the specific DMX
                                          channel failed for some reasons
*/
AUI_RTN_CODE aui_dmx_channel_stop (

    aui_hdl hdl_dmx_channel,

    const aui_attr_dmx_channel *p_attr_channel

    );

/**
@brief          Function used to perform a specific setting for the specific
                DMX channel between its opening and starting by, respectively,
                the functions #aui_dmx_channel_open and #aui_dmx_channel_start

@param[in]      hdl_dmx_channel         = #aui_hdl handle of the specific DMX
                                          channel already opened and to be
                                          managed to perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting to be performed, as defined
                                          in the enum #aui_en_dmx_channel_item_set
@param[in]      *pv_param               = The pointer as per the description
                                          of the specific setting to be
                                          performed, as defined in the enum
                                          #aui_en_dmx_channel_item_get

@return         @b AUI_RTN_SUCCESS      = Specific setting for a DMX Channel
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Specific setting for a DMX Channel
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dmx_channel_set (

    aui_hdl hdl_dmx_channel,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get miscellaneous information from a DMX channel
                after its opening and starting by, respectively, the functions
                #aui_dmx_channel_open and #aui_dmx_channel_start

@param[in]      hdl_dmx_channel         = #aui_hdl handle of the specific DMX
                                          channel already opened and to be
                                          managed to get miscellaneous information
@param[in]      ul_item                 = The item related to the specific
                                          information of the specific DMX channel
                                          to be gotten, as defined in the enum
                                          #aui_en_dmx_channel_item_get

@param[out]     *pv_param               = The pointer as per the description of
                                          the specific information of the specific
                                          DMX channel to be gotten, as defined
                                          in the enum #aui_en_dmx_channel_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific information of
                                          the specific DMX channel performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific information
                                          of the specific DMX channel failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dmx_channel_get (

    aui_hdl hdl_dmx_channel,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to read data from the specific DMX channel after
                its opening, configuring and starting using, respectively, the
                functions #aui_dmx_channel_open, aui_dmx_channel_set and
                #aui_dmx_channel_start. This function synchronously reads and
                the requested amount of data within the specified timeout.

@attention           1. For asnychronous channel data read, see #aui_attr_dmx_channel
                used in #aui_channel_open to register the relevant callback functions to 
                receive channel data asynchronously. 
                     2. Read data type only for TS

@param[in]      *p_hdl_dmx_channel      = #aui_hdl pointer to the handle of the
                                          specific DMX channel already opened

@param[in]      p_uc_buffer             = buffer to store channel data

@param[in]      n_number_to_read        = Length (in @a byte unit) of the
                                          expected number of bytes to read

@param[out]     p_n_number_read         = Actual length (in @a byte unit) of
                                          data received from channel

@param[in]      n_timeout               = Maximum time for waiting for the
                                          expected number of bytes to read

@return         @b AUI_RTN_SUCCESS      = Operation performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Operation failed for some reasons
*/
AUI_RTN_CODE aui_dmx_channel_read(

    aui_hdl p_hdl_dmx_channel,

    unsigned char *p_uc_buffer,

    unsigned long n_number_to_read,

    unsigned long *p_n_number_read,

    int n_timeout

    );

/**
@brief          Function used to open a specific DMX filter from a specific DMX
                channel already opened by the function #aui_dmx_channel_open
                and configure the desired attributes then get the related handle

@param[in]      hdl_dmx_channel         = #aui_hdl handle of the specific DMX
                                          channel already opened and to be
                                          managed to open a DMX filter from it
@param[in]      *p_attr_filter          = Pointer to a struct #aui_attr_dmx_filter
                                          which collects the desired attributes
                                          for the DMX filter

@param[out]     *p_hdl_dmx_filter       = #aui_hdl pointer to the handle of the
                                          DMX filter just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the DMX filter performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DMX filter failed for
                                          some reasons

@note           All attributes of DMX filter can be set when calling this
                function, then user can change some of them by the functions
                - Either #aui_dmx_filter_mask_val_cfg,
                - Ot #aui_dmx_reg_sect_call_back
                - Or #aui_dmx_reg_data_call_back

                after the fucntion #aui_dmx_filter_open
*/
AUI_RTN_CODE aui_dmx_filter_open (

    aui_hdl hdl_dmx_channel,

    const aui_attr_dmx_filter *p_attr_filter,

    aui_hdl* const p_hdl_dmx_filter

    );

/**
@brief          Function used to close the specific DMX filter already opened
                by the function #aui_dmx_filter_open then the related handle
                will be released (i.e. the related resources such as memory,
                device)

@param[in]      *p_hdl_dmx_filter       = #aui_hdl pointer to the handle of the
                                          specific DMX filter already opened
                                          and to be closed

@return         @b AUI_RTN_SUCCESS      = Closing of the specific DMX filter
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Closing of the specific DMX filter
                                          failed for same reasons
*/
AUI_RTN_CODE aui_dmx_filter_close (

    aui_hdl *p_hdl_dmx_filter

    );

/**
@brief          Function used to start the specific DMX filter already opened
                by the function #aui_dmx_filter_open

@param[in]      hdl_dmx_filter          = #aui_hdl handle of the specific DMX
                                          filter already opened and to be started
@param[in]      *p_attr_filter          = Pointer to a struct #aui_attr_dmx_filter
                                          which collect the attributes for a
                                          DMX filter
                                          - @b Caution: This parameter is not
                                               used then user can set it to NULL

@return         @b AUI_RTN_SUCCESS      = Starting of the specific DMX filter
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Starting of the specific DMX filter
                                          failed for some reasons.
*/
AUI_RTN_CODE aui_dmx_filter_start (

    aui_hdl hdl_dmx_filter,

    const aui_attr_dmx_filter *p_attr_filter

    );

/**
@brief          Function used to stop the specific DMX filter already opened
                and started by, respectively, the functions #aui_dmx_channel_open
                and #aui_dmx_filter_open

@warning        After stopping the DMX filter, user can call the function
                #aui_dmx_filter_start to start it again

@param[in]      hdl_dmx_filter          = #aui_hdl handle of the specific DMX
                                          filter already opened and to be stopped
@param[in]      *p_attr_filter          = Pointer to a struct #aui_attr_dmx_filter
                                          which collect the attributes for a DMX
                                          filter
                                          - @b Caution: This parameter is not
                                               used used currently then user can
                                               ignore it

@return         @b AUI_RTN_SUCCESS      = Stopping of the specific DMX filter
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Stopping of the the specific DMX
                                          filter failed for some reasons
*/
AUI_RTN_CODE aui_dmx_filter_stop (

    aui_hdl hdl_dmx_filter,

    const aui_attr_dmx_filter *p_attr_filter

    );

/**
@brief          Function used to perform a specific setting for the specific
                DMX filter between its opening and starting by, respectively,
                the functions #aui_dmx_filter_open and #aui_dmx_filter_start

@param[in]      hdl_dmx_filter          = #aui_hdl handle of the specific DMX
                                          filter already opened and to be
                                          managed to perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting to be performed, as defined
                                          in the enum #aui_en_dmx_filter_item_set
@param[in]      *pv_param               = The pointer as per the description of
                                          the specific setting to be performed,
                                          as defined in the enum
                                          #aui_en_dmx_filter_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting for a DMX filter
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Specific setting for a DMX filter
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dmx_filter_set (

    aui_hdl hdl_dmx_filter,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get miscellaneous information from a DMX
                filter after its opening and starting by, respectively, the
                functions #aui_dmx_filter_open and #aui_dmx_filter_start

@param[in]      hdl_dmx_filter          = #aui_hdl handle of the specific DMX
                                          filter already opened and to be managed
                                          to get a specific information
@param[in]      ul_item                 = The item related to the specific
                                          information of the specific DMX filter
                                          to be gotten, as defined in the enum
                                          #aui_en_dmx_filter_item_get

@param[out]     *pv_param               = The pointer as per the description of
                                          the specific information of the
                                          specific DMX filter to be gotten,
                                          as defined in the enum
                                          #aui_en_dmx_filter_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific information
                                          of the specific DMX filter performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific information
                                          of the specific DMX filter failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dmx_filter_get (

    aui_hdl hdl_dmx_filter,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to configure the conditions for a specific DMX
                filter after its opening by the functions #aui_dmx_filter_open

@warning        Only the filtering of the section data needs to call this function

@param[in]      hdl_dmx_filter           = #aui_hdl handle of the specific DMX
                                           filter already opened and to be configured
@param[in]      puc_mask                 = Pointer to the mask of the specific
                                           DMX filter, as mentioned in the member
                                           @b puc_mask of the struct
                                           #aui_st_attr_dmx_filter
@param[in]      puc_reverse              = Pointer to the reversed mask of the
                                           specific DMX filter, as mentioned in
                                           the member @b puc_reverse of the
                                           struct #aui_st_attr_dmx_filter
@param[in]      puc_val                  = Pointer to the  value to be checked
                                           by the mask and reversed mask, as
                                           mentioned in the member @b puc_val
                                           of the struct #aui_st_attr_dmx_filter
@param[in]      ul_mask_val_len          = Length of the mask
@param[in]      uc_crc_check             = Flag to specify whether checking the
                                           CRC of the section data or not,
                                           in particular:
                                           - @b 0 = No check
                                           - @b 1 = Check
@param[in]      uc_continue_capture_flag = Flag to specify whether getting the
                                           section data only one time or
                                           continuously, in particular:
                                           - @b 1 = Get the section data
                                                    continuously
                                           - @b 0 = Get the section data only
                                                    one time

@return         @b AUI_RTN_SUCCESS       =  Setting of the DMX filter performed
                                            successfully
@return         @b AUI_RTN_EINVAL        =  At least one input parameter (i.e.
                                            [in], [out]) is invalid
@return         @b Other_Values          =  Setting of the DMX filter failed
                                            for some reasons

@note           Below some example of @b mask and <b> reversed mask </b>:

    mask example 1:
                    reverse:     0x00 0x00 0x00 0x00
                    mask:        0XFF 0X00 0X00 0X00
                    val:         0X55 0X00 0X00 0X00
                    src0(PASS):  0X55 0X00 0X00 0X00
                    src1(PASS):  0X55 0X12 0X34 0X56
                    src2(FAIL):  0X54 0X00 0X00 0X00
    mask example 2:
                    reverse:     0x00 0x00 0x00 0x00
                    mask:        0XFE 0X00 0X00 0X00
                    val:         0X55 0X00 0X00 0X00
                    src0(PASS):  0X55 0X00 0X00 0X00
                    src1(PASS):  0X55 0X12 0X34 0X56
                    src2(PASS):  0X54 0X00 0X00 0X00
    reverse mask example 1:
                    reverse:     0xFF 0x00 0x00 0x00
                    mask:        0XFF 0X00 0X00 0X00
                    val:         0X55 0X00 0X00 0X00
                    src0(FAIL):  0X55 0X00 0X00 0X00
                    src1(FAIL):  0X55 0X12 0X34 0X56
                    src2(PASS):  0X54 0X00 0X00 0X00
                    src3(PASS):  If the first byte is not equal to 0x55 then pass
    reverse mask example 2:
                    reverse:     0xFE 0x00 0x00 0x00
                    mask:        0XFF 0X00 0X00 0X00
                    val:         0X55 0X00 0X00 0X00
                    src0(FAIL):  0X55 0X00 0X00 0X00
                    src1(FAIL):  0X55 0X12 0X34 0X56
                    src2(FAIL):  0X54 0X00 0X00 0X00
                    src2(FAIL):  0X44 0X00 0X00 0X00
                    src2(PASS):  0X45 0X00 0X00 0X00
                    src2(PASS):  0X01 0X00 0X00 0X00

    About the reverse mask example 2:
    - Set the first byte of src = A then,
      if (A & 0XFE )!=(0X55 & 0XFE) and (A & 0X01 )==(0X55 & 0X01), PASS

    About all examples:
    - PASS = Can be captured by a callback function
    - FAIL = Can not be captured by a callback function then discard it
*/
AUI_RTN_CODE aui_dmx_filter_mask_val_cfg (

    aui_hdl hdl_dmx_filter,

    const unsigned char *puc_mask,

    const unsigned char *puc_val,

    const unsigned char *puc_reverse,

    unsigned long ul_mask_val_len,

    unsigned char uc_crc_check,

    unsigned char uc_continue_capture_flag

    );

/**
@brief          Function used to register the specific DMX filter callback
                function to be called when receiving the desired section data

@warning        This function is @a only used when the data type of the output
                of the DMX channel (to which the specific DMX filter belongs)
                is #AUI_DMX_DATA_SECT

@param[in]      hdl_dmx_filter           = #aui_hdl handle of the specific DMX
                                           filter already opened and to be
                                           managed to register the callback
                                           function
@param[in]      p_fun_sectionCB          = The callback function to be registered,
                                           as per comment for the function pointer
                                           #aui_p_fun_sectionCB

@return         @b AUI_RTN_SUCCESS       = Registering of the callback function
                                           to the specific DMX filter performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = A least one parameter (i.e. [in]) is
                                           invalid
@return         @b Other_Values          = Registering of the callback function
                                           to the specific DMX filter failed for
                                           some reasons
*/
AUI_RTN_CODE aui_dmx_reg_sect_call_back (

    aui_hdl hdl_dmx_filter,

    aui_p_fun_sectionCB p_fun_sectionCB

    );

/**
@brief          Function used to register the callback functions to the specific
                DMX filter to be called when receiving the desired Raw TS data

@warning        This function is used @a only when the data type of the output
                of the DMX channel (to which the specific DMX filter belongs)
                is #AUI_DMX_DATA_RAW

@param[in]      hdl_dmx_filter           = #aui_hdl handle of the specific DMX
                                           filter already opened and to be
                                           managed to register the callback
                                           function
@param[in]      p_fun_data_req_wtCB      = The callback function to be
                                           registered as per comment for the
                                           function pointer #aui_p_fun_sectionCB
@param[in]      p_fun_data_up_wtCB       = The callback function to be registered
                                           as per comment for the function
                                           pointer #aui_p_fun_data_req_wtCB

@return         @b AUI_RTN_SUCCESS       = Registering of the callback functions
                                           to the specific DMX filter performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = At least one input parameter (i.e.
                                           [in]) is invalid
@return         @b Other_Values          = Registering of the callback functions
                                           to the specific DMX filter failed for
                                           some reasons
*/
AUI_RTN_CODE aui_dmx_reg_data_call_back (

    aui_hdl hdl_dmx_filter,

    aui_p_fun_data_req_wtCB p_fun_data_req_wtCB,

    aui_p_fun_data_up_wtCB p_fun_data_up_wtCB

    );

/**
@brief          Function used to register the callback function to the specific
                DMX filter to be called when receiving the desired PES data

@warning        This function is @a only used when the data type of the output
                of the DMX channel (to which the specific DMX filter belongs)
                is #AUI_DMX_DATA_PES

@param[in]      hdl_dmx_filter           = #aui_hdl handle of the specific DMX
                                           filter already opened and to be
                                           managed to register the callback
                                           function
@param[in]      callback                 = The callback function to be registered
                                           as per comment for the function
                                           pointer #aui_pes_data_callback
@param[in]      pv_callback_param        = Pointer to the parameter of the
                                           callback function to be registered

@return         @b AUI_RTN_SUCCESS       = Registering of the callback function
                                           to the specific DMX filter performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = At least one input parameters (i.e.
                                           [in]) is invalid
@return         @b Other_Values          = Registering of the callback function
                                           to the specific DMX filter failed
                                           for some reasons

@note           This function can be used to filter PES packets that has an known
                packet length in the PES header. If the PES packet length is
                bigger than 64K bytes, then packet length field in the PES
                header will be zero, thus can not be filtered out by AUI DMX.
*/
AUI_RTN_CODE aui_dmx_reg_pes_call_back (

    aui_hdl hdl_dmx_filter,

    aui_pes_data_callback callback,

    void *pv_callback_param

    );

/**
@brief          Function used to get one complete synchronized section data from
                the corresponding buffer of DMX channel/filter

@param[in]      p_hdl_dmx_channel        = #aui_hdl handle of the specific DMX
                                           channel already opened
@param[in]      pst_hdl_dmx_filter       = #aui_hdl handle of the specific DMX
                                           filter already opened
@param[in]      req_size                 = Size of the complete synchronized
                                           section to be gotten
                                           - @b Caution: User @a must make sure
                                                this size is not shorter than
                                                the size of a complete synchronized
                                                section data
@param[in]      timeout_ms               = Timeout (in @a millisecond (ms) unit)
                                           to get one complete synchronized
                                           section data

@param[out]     *puc_buf                 = Pointer to the buffer used to save
                                           the complete synchronized section
                                           data from DMX channel/filter
@param[out]     *p_data_size             = Size of the section data already
                                           got from DMX channel/filter

@return         @b AUI_RTN_SUCCESS       = Getting of one complete synchronized
                                           section from the DMX channel/filter
                                           performed successfully
@return         @b AUI_RTN_EINVAL        = Either one or both of the input
                                           parameters (i.e. [in]) are invalid
@return         @b Other_Values          = Getting of one complete synchronized
                                           section from the DMX channel/filter
                                           failed for some reasons

@note           @b 1. To get a complete synchronized section data from the
                      corresponding DMX channel
                      - Set the member @b dmx_channel_sec_sync_get_data_support
                        of the struct #aui_st_attr_dmx_channel) to the value
                        @b 1 when opening the specific DMX channel by the
                        function #aui_dmx_channel_open
                      - Set the input parameter @b pst_hdl_dmx_filter of this
                        function to @b NULL
@note           @b 2. To get a complete synchronized section data from the
                      corresponding DMX filter
                      - Set the member @b dmx_fil_sec_data_sync_get_support of
                        the struct #aui_st_attr_dmx_filter to the value @b 1 when
                        opening the specific DMX filter by the function
                        #aui_dmx_filter_open
                      - Set the input parameter @b p_hdl_dmx_channel of this
                        function to NULL
@note           @b 3. After returning successfully, the DMX driver will continue
                      to receive one complete synchronized section data so if
                      user does not want to get it any more then just needs to
                      close or stop the corresponding DMX channel/filter
@note           @b 4. To avoid the DMX buffer overflow, user @a must call this
                      function as fast as possible
*/
AUI_RTN_CODE aui_dmx_channel_sync_get_section_data (

    aui_hdl p_hdl_dmx_channel,

    aui_hdl pst_hdl_dmx_filter,

    const unsigned int req_size,

    unsigned char* const puc_buf,

    unsigned int* const p_data_size,

    const unsigned int timeout_ms

    );

/**
@brief          Function used to get all complete synchronized section data from
                the corresponding buffer of DMX channel/filter

@warning        About the difference with the function
                #aui_dmx_channel_sync_get_section_data:
                - The function #aui_dmx_channel_sync_get_section_data can get
                  @a only one complete synchronized section data from the buffer
                  of DMX channel/filter
                - The function #aui_dmx_channel_sync_get_section_data_ext can
                  get @a all the complete synchronized section data from the
                  buffer of DMX channel/filter

@param[in]      p_hdl_dmx_channel        = #aui_hdl handle of the specific DMX
                                           channel already opened
@param[in]      pst_hdl_dmx_filter       = #aui_hdl handle of the specific DMX
                                           filter already opened
@param[in]      req_size                 = Size of the complete synchronized
                                           section to be gotten
                                           - @b Caution: User @a must make sure
                                                this size is not shorter than
                                                the the size of a complete
                                                synchronized section data
@param[in]      timeout_ms               = Timeout (in @a millisecond (ms) unit)
                                           to get one complete synchronized
                                           section data

@param[out]     *puc_buf                 = Pointer to the buffer used to save
                                           the complete synchronized section
                                           data from DMX channel/filter
@param[out]     *p_data_size             = Size of the section data already got
                                           from DMX channel/filter

@return         @b AUI_RTN_SUCCESS       = Getting of all complete synchronized
                                           section from the DMX channel/filter
                                           performed successfully
@return         @b AUI_RTN_EINVAL        = Either one or both of the input
                                           parameters (i.e. [in]) are invalid
@return         @b Other_Values          = Getting of all complete synchronized
                                           section from the DMX channel/filter
                                           failed for some reasons

@note           @b 1. To get all complete synchronized section data from the
                      corresponding DMX channel
                      - Set the member @b dmx_channel_sec_sync_get_data_support
                        of the struct #aui_st_attr_dmx_channel to the value @b 1
                        when opening the corresponding DMX channel by the function
                        #aui_dmx_channel_open
                      - Set the input parameter @b param pst_hdl_dmx_filter of
                        this function to @b NULL
@note           @b 2. To get all complete synchronized section data from a
                      corresponding DMX filter
                      - Set the member @b dmx_fil_sec_data_sync_get_support of
                        the struct #aui_st_attr_dmx_filter to the value @b 1
                        when opening the corresponding DMX filter by the function
                        #aui_dmx_filter_open
                      - Set the input parameter @b p_hdl_dmx_channel of this
                        function to @b NULL
@note           @b 3. After returning successfully, the DMX driver will continue
                      to receive all complete synchronized section data, so if
                      user do not want to get it any more then just need to
                      close or stop the corresponding DMX channel/filter
@note           @b 4. To avoid the DMX buffer overflow, user @a must call this
                      function as fast as possible
*/
AUI_RTN_CODE aui_dmx_channel_sync_get_section_data_ext (

    aui_hdl p_hdl_dmx_channel,

    aui_hdl pst_hdl_dmx_filter,

    const unsigned int req_size,

    unsigned char* const puc_buf,

    unsigned int* const p_data_size,

    const unsigned int timeout_ms

    );

/**
@brief          Function used to register a service to the specific DMX device
                to start getting the Program Clock Reference (PCR) data

@param[in]      hdl_dmx                  = #aui_hdl handle of the specific DMX
                                           device already opened
@param[in]      dmx_attr                 = Pointer to a struct #aui_attr_dmx,
                                           which collects the desired attributes
                                           for the DMX device

@return         @b AUI_RTN_SUCCESS       = Registering of the service to the
                                           specific DMX device performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = A least one parameter (i.e. [in])
                                           is invalid
@return         @b Other_Values          = Registering of the service to the
                                           specific DMX device failed for some
                                           reasons

@note           This function is used @a only in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_dmx_pcr_reg (

    aui_hdl hdl_dmx,

    aui_p_attr_dmx dmx_attr

    );

/**
@brief          Function used to de-register a service from the specific DMX
                device to stop getting the Program Clock Reference(PCR) data

@param[in]      hdl_dmx                  = #aui_hdl handle of the specific DMX
                                           device already opened
@param[in]      dmx_attr                 = Pointer to a struct #aui_attr_dmx,
                                           which collects the desired attributes
                                           for the DMX device

@return         @b AUI_RTN_SUCCESS       = De-registering of the service from
                                           the specific DMX device performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = At least one parameter (i.e. [in])
                                           is invalid
@return         @b Other_Values          = De-registering of the service from
                                           the specific DMX device failed for
                                           some reasons

@note           This function is used @a only in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_dmx_pcr_unreg (

    aui_hdl hdl_dmx,

    aui_p_attr_dmx dmx_attr

    );

/**
@brief          Function used to allow an user application to add more PIDs for
                TS recording after the DMX channel has been opened

@param[in]      hdl_dmx_channel          = #aui_hdl handle of the specific DMX
                                           channel already opened
@param[in]      p_channel_pid_list       = Pointer to a struct
                                           #aui_dmx_channel_pid_list which
                                           collects the list of PIDs to be added

@return         @b AUI_RTN_SUCCESS       = Adding of the PID list performed
                                           successfully
@return         @b AUI_RTN_EINVAL        = At least one parameter (i.e. [in])
                                           is invalid
@return         @b Others_Values         = Adding of the PID list failed for
                                           some reasons

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_dmx_channel_add_pid (

    aui_hdl hdl_dmx_channel,

    const aui_dmx_channel_pid_list *p_channel_pid_list

    );

/**
@brief          Function used to allow an user application to delete existing
                TS record PIDs after the DMX channel has been opened

@param[in]      hdl_dmx_channel           = #aui_hdl handle of the specific DMX
                                            channel already opened
@param[in]      p_channel_pid_list        = Pointer to a struct
                                            #aui_dmx_channel_pid_list which
                                            collects the list of PIDs to be
                                            deleted

@return         @b AUI_RTN_SUCCESS        = Deleting of the PID list performed
                                            successfully
@return         @b AUI_RTN_EINVAL         = At least one parameter (i.e. [in])
                                            is invalid
@return         @b Others_Values          = Deleting of the PID list failed for
                                            some reasons

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_dmx_channel_del_pid (

    aui_hdl hdl_dmx_channel,

    const aui_dmx_channel_pid_list *p_channel_pid_list

    );

/**
@brief          Function used to get the structure #aui_dmx_dsc_id which is to
                represent a DSC object for setting up DMX data path in another process.

@warning        This function can only be called after the data path of the current DMX
                device is set by the function #aui_dmx_data_path_set with parameter the
                DSC handle

@param[in]      hdl_dmx                   = #aui_hdl handle of the specific DMX
                                            device already opened and
                                            configured by #aui_dmx_data_path_set

@param[out]     p_dsc_id                  = Pointer to a struct #aui_dmx_dsc_id
                                            which collects the identifier of DSC
                                            resources

@return         @b AUI_RTN_SUCCESS        = Getting of the #aui_dmx_dsc_id structure
                                            performed successfully
@return         @b AUI_RTN_EINVAL         = At least one parameter (i.e. [in], [out])
                                            is invalid
@return         @b Others_Values          = Getting of the #aui_dmx_dsc_id structure
                                            failed for some reasons

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_dmx_dsc_id_get (

    aui_hdl hdl_dmx,

    aui_dmx_dsc_id *p_dsc_id

    );

 


/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_str_get_pcr_param aui_dmx_pcr_param

#define aui_get_pcr_param aui_dmx_pcr_param

#define aui_pts_t aui_pts

#define aui_avsync_mode aui_dmx_avsync_mode

#define aui_dmx_stream_pid_st aui_dmx_stream_pid

#define DMX_AUI_NO_CHACE AUI_DMX_CACHE_NONE

#define DMX_AUI_CHACE_PID AUI_DMX_CACHE_PID

#define DMX_AUI_CACHE_TP AUI_DMX_CACHE_TP

#define AUI_AVSYNC_FROM_TURNER AUI_AVSYNC_FROM_TUNER

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


