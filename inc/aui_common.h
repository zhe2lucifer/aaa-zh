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
Current ALi author: Davy.Wu
Last update:        2017.02.15
-->

@file   aui_common.h

@brief  Common Module

        <b> Common Module </b> contains useful information common with all the
        other AUI API Modules

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_COMMON_H
#define _AUI_COMMON_H

#ifdef __cplusplus

extern "C" {

#endif

/********************Included Header File List (General)**********************/

#include "aui_errno_sys.h"
#include "aui_errno_stb.h"
#include "aui_common_list.h"

#ifdef AUI_LINUX

/****************Included Header File List (for Linux OS only)****************/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#else

/****************Included Header File List (for TDS OS only)******************/

#include <string.h>

#endif

/**************Global Macro List (from syslog.h for TDS OS case)**************/

#ifdef AUI_TDS

#ifdef AUI_SLEEP
#undef AUI_SLEEP
#endif

/**
Macro to specify that the sleep function use TDS OS API (the time unit is millisecond (ms))
*/
#define AUI_SLEEP(x) aui_os_task_sleep(x)

#ifdef ITOA
#undef ITOA
#endif

/**
Macro to specify that ITOA use TDS OS API
*/
#define ITOA(str, val)    ali_itoa(str, val)

#ifdef ATOI
#undef ATOI
#endif

/**
Macro to specify that ATOI use TDS OS API
*/
#define ATOI(str)   ali_atoi(str)

#endif

#ifdef AUI_LINUX

/*********************Global Macro List (for Linux OS only)*******************/

#ifdef AUI_SLEEP
#undef AUI_SLEEP
#endif

/**
Macro to specify that the sleep function use Linux OS API (the time unit is millisecond (ms))
*/
#define AUI_SLEEP(x) usleep((x) * (1000))

#ifdef ITOA
#undef ITOA
#endif

/**
Macro to specify that ITOA use Linux OS standard API
*/
#define ITOA(str, val) itoa(str, val)

#ifdef ATOI
#undef ATOI
#endif

/**
Macro to specify that ATOI use Linux OS standard API
*/
#define ATOI(str) atoi(str)

#endif

#ifndef false
#define false (0)
#endif

#ifndef true
#define true (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef MALLOC
#define MALLOC malloc
#endif

#ifndef FREE
#define FREE free
#endif

#ifndef STRCPY
#define STRCPY strcpy
#endif

#ifndef STRLEN
#define STRLEN strlen
#endif

#ifndef STRCMP
#define STRCMP strcmp
#endif

#ifndef MEMCPY
#define MEMCPY memcpy
#endif

#ifndef MEMSET
#define MEMSET memset
#endif

#ifndef MEMCMP
#define MEMCMP memcmp
#endif

#ifndef MEMMOVE
#define MEMMOVE memmove
#endif

#define RED_STR "\033[22;31m"
#define GREEN_STR "\033[22;32m"
#define DEFAULT_STR "\033[0m"

#ifndef UNUSED

/**
Macro used to avoid warnings
*/
#define UNUSED(x) ((void)(x))
#endif

/**
Macro to specify the return value of the functions of an AUI API Module when
it is not necessary to print and/or log errors

@warning  This macro is deprecated
*/
#define AUI_RTN_VAL(x,y) (y)

/**
Macro used by all AUI API modules to return errors when is not necessary to
print and log errors

@warning  This macro is deprecated
*/
#define AUI_RTN(x,y) AUI_RTN_VAL(x,y);

/*
_RD_DEBUG option configuration:
 - For TDS, user can enable it in compiler_xxx.def
 - For Linux, user can enable it by selecting BR2_PACKAGE_ENABLE_AUI_DEBUG
*/
#ifdef _RD_DEBUG_

#ifdef AUI_LINUX

/**
Macro used in Linux OS to print some log messages.

@warning  This macro should not be used in the source code of AUI layer,
          please use the following macros instead:
          - AUI_ERR
          - AUI_WARN
          - AUI_INFO
          - AUI_DBG
*/
#define AUI_PRINTF  printf

#else

/**
Macro used in TDS OS to print some log messages.

@warning  This macro should not be used in the source code of AUI layer,
          please use the following macros instead:
          - AUI_ERR
          - AUI_WARN
          - AUI_INFO
          - AUI_DBG
*/
#define AUI_PRINTF  libc_printf

#endif

#else

#define AUI_PRINTF(...) do { } while(0)

#endif

/*******************************Global Type List*******************************/

/**
Macro to specify that the action must be taken immediately
*/
#define AUI_LOG_PRIO_ALERT   1

/**
Macro to specify critical conditions
*/
#define AUI_LOG_PRIO_CRIT    2

/**
Macro to specify error conditions
*/
#define AUI_LOG_PRIO_ERR     3

/**
Macro to specify warning conditions
*/
#define AUI_LOG_PRIO_WARNING 4

/**
Macro to specify a normal but significant condition
*/
#define AUI_LOG_PRIO_NOTICE  5

/**
Macro to specify an informational
*/
#define AUI_LOG_PRIO_INFO    6

/**
Macro to specify a debug-level message
*/
#define AUI_LOG_PRIO_DEBUG   7


/**
Macro used to generate an AUI Module ID as per file "aui_modules.def"
*/
#define AUI_MOD_DEF(x, level) AUI_MODULE_##x,

/**
Macro used when generating an AUI Module ID as per file "aui_modules.def"
in particular to expand SL_MOD(x) into an empty string
*/
#define SL_MOD(x)

/// @cond

/**
Enum to specify the AUI Module ID list as per the included file
<b> aui_modules.def </b> which is used for generating the values
*/
typedef enum aui_module_id {

    #include "aui_modules.def"

    /**
    Value to specify the end of this enum
    */
    AUI_MODULE_LAST

} aui_module_id;

/// @endcond

/*
Undefine SL_MOD Macro used after generating the AUI Module ID list
*/
#undef SL_MOD

/*
Undefine AUI_MOD_DEF Macro used after generating the AUI Module ID list
*/
#undef AUI_MOD_DEF

/**
Type of the return value of a function of an AUI API Module
*/
typedef unsigned long AUI_RTN_CODE;

/**
Pointer to the handle of an AUI API Module
*/
typedef void* aui_hdl;

/// @cond

/**
Pointer to the callback function used by the initialization/de-initialization
function of an AUI API Module
*/
typedef AUI_RTN_CODE (*p_fun_cb) (

    void *pv_param

    );

/// @endcond

/**
Enum to specify the available different <b> video formats </b> supported by
DECV Module
*/
typedef enum aui_decv_format {

    /**
    Value to specify the <b> ISO/IEC 13818-2 MPEG-2 </b> video format

    @note @b 1. When playing <b> TS Stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format is @a also supported by <b> AV-Injector Module
                </b> (@a only in <b> Linux OS</b>), please refer to the related
                header file
    */
    AUI_DECV_FORMAT_MPEG,

    /**
    Value to specify the <b> ITU-T H.264 </b> or <b> ISO/IEC 14496-10/MPEG-4 AVC
    </b> video format.

    @note @b 1. When playing <b> TS Stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format is @a also supported by <b> AV-Injector Module
                </b> (@a only in <b> Linux OS</b>), please refer to the related
                header file
    */
    AUI_DECV_FORMAT_AVC,

    /**
    Value to specify the <b> AVS+ </b> video format

    @note @b 1. When playing <b> TS stream </b>:
                - This video format can be set by the function
                  #aui_decv_decode_format_set
                - The function #aui_decv_decode_format_get can be used to check
                  if that video format is the current one

    @note @b 2. This video format can be used @a only in projects based on <b>
                Linux OS </b>
    */
    AUI_DECV_FORMAT_AVS,

    /**
    Value to specify the  <b> XVID </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_XVID,

    /**
    Value to specify the  <b> FLV1 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_FLV1,

    /**
    Value to specify the  <b> VP8 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_VP8,

    /**
    Value to specify the <b> WVC1 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_WVC1,

    /**
    Value to specify the  <b> WX3 </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_WX3,

    /**
    Value to specify the  <b> RMVB </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_RMVB,

    /**
    Value to specify the <b> MJPG </b> video format

    @note  This video format can be used @a only in <b> AV-Inject Module </b>
           (@a only in <b> Linux OS</b>), please refer to the related header file
    */
    AUI_DECV_FORMAT_MJPG,

    /**
    Value to specify the <b> HEVC (H.265) </b> video format
    */
    AUI_DECV_FORMAT_HEVC,

    /**
    Value to specify an invalid video format
    */
    AUI_DECV_FORMAT_INVALID,

    /**
    Value to specify the total number of video formats available in this enum
    */
    AUI_DECV_FORMAT_NUM

} aui_decv_format;

/**
Enum to specify the available different <b> audio formats </b> supported by
DECA Module
*/
typedef enum aui_deca_stream_type {

    /**
    Value to specify the <b> ISO/IEC 11172-3 MPEG-1 </b> audio format, which
    is used for live play and media player

    @note @b 1. When playing TS packets:
                - This audio format can be set by the function
                  #aui_deca_type_set
                - The function #aui_deca_type_get can be used to check if that
                  audio format is the current one

    @note @b 2. This audio format is also supported by AV-Injector Module
                (only in Linux OS), please refer to the header file
                @b aui_av_injecter.h
    */
    AUI_DECA_STREAM_TYPE_MPEG1,

    /**
    Value to specify the  <b> ISO/IEC 13813-3 MPEG-2 </b> audio format, which
    is used for live play and media player

    @note @b 1. When playing TS packets:
                - This audio format can be set by the function
                  #aui_deca_type_set
                - The function #aui_deca_type_get can be used to check if that
                  audio format is the current one

    @note @b 2. This audio format is also supported by AV-Injector Module
                (only in Linux OS), please refer to the header file
                @b aui_av_injecter.h
    */
    AUI_DECA_STREAM_TYPE_MPEG2,

    /**
    Value to specify the <b> ISO/IEC 14496-3 MPEG4-AAC </b> audio format
    (i.e. Advanced Audio Coding (AAC) with LOAS (Low Overhead Audio Stream)
    sync and LATM mux), which is used for live play and media player.

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one

    @warning If the AAC sync format (LATM/ADTS) is not set correctly,
             audio decode will probe the sync format and change to the correct
             sync format internally, but the first few seconds of audio data
             will be discarded.
    */
    AUI_DECA_STREAM_TYPE_AAC_LATM,

    /**
    Value to specify the <b> AC3 </b> audio format, which is used for live play
    and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    AUI_DECA_STREAM_TYPE_AC3,

    /**
    Value to specify the <b> DTS </b> audio format, which is used for live play
    and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    AUI_DECA_STREAM_TYPE_DTS,

    /**
    @warning  This value is deprecated
    */
    AUI_DECA_STREAM_TYPE_PPCM,

    /**
    @warning  This value is deprecated
    */
    AUI_DECA_STREAM_TYPE_LPCM_V,

    /**
    @warning  This value is deprecated
    */
    AUI_DECA_STREAM_TYPE_LPCM_A,

    /**
    @warning  This value is deprecated
    */
    AUI_DECA_STREAM_TYPE_PCM,

    /**
    Value to specify the <b> BYE1 </b> audio format, which is used for live
    play and media player

    @note When playing TS packets:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if that audio
            format is the current one
    */
    AUI_DECA_STREAM_TYPE_BYE1,

    /**
    Value to specify the <b> Real audio 8 </b> format, which is only used for
    media player
    */
    AUI_DECA_STREAM_TYPE_RA8,

    /**
    Value to specify the <b> MPEG Audio Layer 3 </b> format which is used for
    live play and media player

    @note When playing TS packets for live play:
          - This audio format can be set by the function #aui_deca_type_set
          - The function #aui_deca_type_get can be used to check if this audio
            format is the current one
    */
    AUI_DECA_STREAM_TYPE_MP3,

    /**
    Value to specify the <b> ISO/IEC 13818-7:2003 MPEG2-AAC </b> audio format
    (i.e. Advanced Audio Coding (AAC) with ADTS (Audio Data Transport Format) sync),
    which used for live play and media player.

    @warning If the AAC sync format (LATM/ADTS) is not set correctly,
             audio decode will probe the sync format and change to the correct
             sync format internally, but the first few seconds of audio data
             will be discarded.
    */
    AUI_DECA_STREAM_TYPE_AAC_ADTS,

    /**
    Value to specify the <b> OGG </b> audio format, which is @a only used
    for media player
    */
    AUI_DECA_STREAM_TYPE_OGG,

    /**
    Value to specify the <b> EC3 </b> audio format, which is used for live play
    and media player

    @note @b 1. When playing TS packets:
                - This audio format can be set by the function
                  #aui_deca_type_set
                - The function #aui_deca_type_get can be used to check if that
                  audio format is the current one

    @note @b 2. This audio format is also supported by AV-Injector Module (only
                in Linux OS), please refer to the header file @b aui_av_injecter.h
    */
    AUI_DECA_STREAM_TYPE_EC3,

    /**
    The scope of this value is the same as the value #AUI_DECA_STREAM_TYPE_MP3 of
    this enum
    */
    AUI_DECA_STREAM_TYPE_MP3_L3,

    /**
    Value to specify pure PCM data without packet header, which contains the
    below parameters:
    - How many channels are in PCM data
    - What is the sample rate of PCM data
    - What is the sample size of PCM data
    - Whether the PCM sample is big endian or not
    - Whether the PCM sample is signed or not

    @note   @b 1. This enum value is not used for live play
    @note   @b 2. User can send pure PCM data by using the function
                  #aui_deca_set with the enum value #AUI_DECA_INJECT_ES_DATA
                  as input parameter
    @note   @b 3. User needs to set the parameters which are usually in the PCM
                  header, as previously mentioned, by using the function #aui_deca_set
                  with the enum value #AUI_DECA_PCM_PARAM_SET as input parameter before
                  sending audio data to the driver
    @note   @b 4. The PCM pure data sent by user should align with 1536 bytes length
    */
    AUI_DECA_STREAM_TYPE_RAW_PCM,

    /**
    Value to specify the <b> BYE1 PRO </b> audio format, which is set to DECA
    Module for decoding in media player
    */
    AUI_DECA_STREAM_TYPE_BYE1PRO,

    /**
    Value to specify the <b> FLAC </b> audio format, which is set to DECA Module
    for decoding in media player
    */
    AUI_DECA_STREAM_TYPE_FLAC,

    /**
    Value to specify the <b> APE </b> audio format, which is set to DECA Module
    for decoding in media player

    @warning  This format is not supported by the DECA Module
    */
    AUI_DECA_STREAM_TYPE_APE,

    /**
    The scope of this value is the same as the value #AUI_DECA_STREAM_TYPE_MP3
    of this enum
    */
    AUI_DECA_STREAM_TYPE_MP3_2,

    /**
    Value to specify the <b> AMR </b> audio format, which is set to DECA Module
    for decoding
    */
    AUI_DECA_STREAM_TYPE_AMR,

    /**
    Value to specify the <b> ADPCM </b> audio format, which is set to DECA
    Module for decoding
    */
    AUI_DECA_STREAM_TYPE_ADPCM,

    /**
    Value to specify and invalid audio format
    */
    AUI_DECA_STREAM_TYPE_INVALID,

    /**
    Value to specify the total number of audio formats available
    */
    AUI_DECA_STREAM_TYPE_LAST

} aui_deca_stream_type;

/**
    Enum to specify the different type of user data which are passed to a
    proper callback function. In particular, the callback type is the one
    specified with the enum value #AUI_DECV_CB_USER_DATA_PARSED
*/
typedef enum aui_decv_user_data_type {

    /**
    This value is to specify that AUI will fiter based on the first
    user data type it detects.

    @note  The first parameter of the callback function is the pointer to an
           user data buffer which contains user data only,
           i.e. the extension information specified in the struct
           #aui_decv_user_data_ext_info are NOT included

    @warning Other types of user data will be discarded by the video decoder
    */
    AUI_DECV_USER_DATA_DEFAULT,
   
	
    /**
    This value is to specify that AUI will fiter based on the first
    user data type it detects.
	
    And such user data are @a appended with extension information defined in the
    struct #aui_decv_user_data_ext_info. The size field of user data contains the
    length of the user data and the length of the struct #aui_decv_user_data_ext_info

    @warning This data type filtering is not supported in AUI_DECV_CB_USER_DATA_PARSED callback,
             It is used for VBI filtering initiated by function #aui_vbi_cc_start	
    */
	AUI_DECV_USER_DATA_ANY,

    /**
    This value is to specify the user data type defined in ATSC A/53, Part 4:2009,
    "MPEG-2 Video System  Characteristics" 
  
    @warning This data type filtering is not supported in AUI_DECV_CB_USER_DATA_PARSED callback,
             It is used for VBI filtering initiated by function #aui_vbi_cc_start
    */
	AUI_DECV_USER_DATA_ATSC53,
	
    /**
    This value is to specify the SCTE-20 user data type.

    @warning This data type filtering is not supported in AUI_DECV_CB_USER_DATA_PARSED callback,
             It is used for VBI filtering initiated by function #aui_vbi_cc_start
    */
	AUI_DECV_USER_DATA_SCTE20,

    /**
    This value is to specify all user data in the video stream as user data type.
    And such user data are @a appended with extension information defined in the
    struct #aui_decv_user_data_ext_info. The size field of user data contains the
    length of the user data and the length of the struct #aui_decv_user_data_ext_info

    @warning The start address of the extension information is not 4 bytes aligned,
             please copy the content of extension information into the struct
             #aui_decv_user_data_ext_info in the heap, and accessing that struct
             instead.
             
    @warning This data type filtering is not supported the VBI filtering initiated
             by function #aui_vbi_cc_start    
    */
    AUI_DECV_USER_DATA_ALL
	

} aui_decv_user_data_type;


/**
Enum to specify which layers are available to be displayed

@attention  This enum is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef enum aui_dis_layer {

    /**
    Value to specify the <b> main picture </b> layer

    @note   This value is used for the video
    */
    AUI_DIS_LAYER_VIDEO,

    /**
    Value to specify the <b> auxiliary picture </b> layer
    */
    AUI_DIS_LAYER_AUXP,

    /**
    Value to specify the @b OSD0 layer
    */
    AUI_DIS_LAYER_OSD_0,

    /**
    Value to specify the @b OSD1 layer
    */
    AUI_DIS_LAYER_OSD_1

} aui_dis_layer;

/**
Enum to specify the different type of data to be injected into AV Module

@note This enum is intended @a only for projects based on <b> Linux OS </b>
*/
typedef enum aui_av_data_type {

    /**
    Value to specify that the data is a @b TS from @b broadcast by NET Interface
    (NIM) Module
    */
    AUI_AV_DATA_TYPE_NIM_TS = 0,

    /**
    Value to specify that the data is a <b> TS Stream </b> from <b> local disk
    </b> by
    - Direct Injection, for which the needed DMX ID is #AUI_DMX_ID_SW_DEMUX0
      as defined in the enum #aui_dmx_id
    - TSG Module, for which the needed DMX ID can be any value defined in the
      enum #aui_dmx_id except the value #AUI_DMX_ID_SW_DEMUX0
    */
    AUI_AV_DATA_TYPE_RAM_TS = 1,

    /**
    Value to specify that the data is an @b ES of audio/video by direct injection
    */
    AUI_AV_DATA_TYPE_ES = 2

} aui_av_data_type, aui_av_data_type_t;

/**
Enum to specify the different synchronization mode of video and audio
*/
typedef enum aui_stc_avsync_mode {

    /**
    Value to specify the synchronization of the video and audio with PCR
    */
    AUI_STC_AVSYNC_PCR,

    /**
    Value to specify the syncronization of the video with audio
    */
    AUI_STC_AVSYNC_AUDIO,

    /**
    Value to specify the Freerun mode
    */
    AUI_STC_AVSYNC_FREERUN

} aui_stc_avsync_mode;

/**
Enum to specify the different types of the IR Remote Control Protocols supported
by ALi STB software system

@note At present ALi STB software system @a only supports the following IR
      Remote Control Protocols:
       - @b NEC
       - @b RC5

@note However, user can also customize ALi STB Software System with more IR
      Remote Control Protocols.
*/
typedef enum aui_ir_protocol {

    /**
    This value indicates @b NEC IR Remote Control Protocol
    */
    AUI_IR_TYPE_NEC = 0,

    /**
    This value indicates @b LAB IR Remote Control Protocol

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_LAB,

    /**
    The value indicates @b 50560 IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_50560,

    /**
    The value indicates @b KF IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_KF,

    /**
    The value indicates @b LOGIC IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user.
    */
    AUI_IR_TYPE_LOGIC,

    /**
    The value indicates @b SRC IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_SRC,

    /**
    The value indicates @b NSE IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_NSE,

    /**
    The value indicates @b RC5 IR Remote Control Protocol.
    */
    AUI_IR_TYPE_RC5,

    /**
    The value indicates @b RC5_X IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_RC5_X,

    /**
    The value indicates @b RC6 IR Remote Control Protocol.

    @note At present that protocol is not supported by ALi STB Software System
          which, however, can be customized by user
    */
    AUI_IR_TYPE_RC6,

    /**
    The value indicates other undefined IR Remote Control Protocol in ALi STB
    Software System.
    */
    AUI_IR_TYPE_UNDEFINE

} aui_ir_protocol, aui_pan_ir_protocol_e;

/*******************************Global Type List*******************************/

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Common Module </b> to specify a Ring Buffer Handle
        </div> @endhtmlonly

        Struct to specify a <b> Ring Buffer Handle </b>
*/
typedef struct aui_st_ring_buf {

    /**
    Member to specify the <b> Start Address </b> of the Ring Buffer
    */
    unsigned char *pby_ring_buf;

    /**
    Member to specify the <b> Size </b> of the Ring Buffer
    */
    unsigned long ul_ring_buf_len;

    /**
    Member to specify the <b> Read Position </b> of the Ring Buffer
    */
    unsigned long ul_ring_buf_rd;

    /**
    Member to  specify the <b> Write Position </b> of the Ring Buffer
    */
    unsigned long ul_ring_buf_wt;

    /**
    Member to specify the <b> Semaphore ID </b> of the Ring Buffer
    */
    unsigned int sem_ring;

} aui_ring_buf, *aui_p_ring_buf;

/*****************************Global Function List*****************************/

/**
@brief          Function used to create and initialize the ring buffer

@param[in]      ul_buf_len          = Size of the ring buffer to be created

@param[out]     p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer
                                      just created

@return         @b AUI_RTN_SUCCESS  = Creating and Initializing of the ring
                                      buffer performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Creating and/or initializing of the ring
                                      buffer failed for some reason
*/
AUI_RTN_CODE aui_common_init_ring_buf (

    unsigned long ul_buf_len,

    aui_ring_buf *p_ring_buf

    );

/**
@brief          Function used to release the ring buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer to
                                      be released

@return         @b AUI_RTN_SUCCESS  = Releasing of the ring buffer performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Releasing of the ring buffer failed for
                                      some reasons
*/
AUI_RTN_CODE aui_common_un_init_ring_buf (

    aui_ring_buf *p_ring_buf

    );

/**
@brief          Function used to reset the ring buffer, i.e. empty the buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer to
                                      be reset

@return         @b AUI_RTN_SUCCESS  = Resetting of the ring buffer performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Resetting of the ring buffer failed for
                                      some reason
*/
AUI_RTN_CODE aui_common_rst_ring_buf (

    aui_ring_buf *p_ring_buf

    );

/**
@brief          Function used to get the length of data in the ring buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer of
                                      which getting the data length

@param[out]     pul_data_len        = Pointer to the length of data in the ring
                                      buffer

@return         @b AUI_RTN_SUCCESS  = Getting of the length of data in the ring
                                      buffer performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the length of data in the ring
                                      buffer failed for some reasons
*/
AUI_RTN_CODE aui_common_ring_buf_data_len (

    aui_ring_buf *p_ring_buf,

    unsigned long *pul_data_len

    );

/**
@brief          Function used to get the size of the ring buffer still available
                to contain further data

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer of
                                      which getting the size still available

@param[out]     pul_buf_remain      = Pointer to the size of the ring buffer
                                      still available just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the size of the ring buffer
                                      still available performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the size of the ring buffer
                                      still available failed for some reasons
*/
AUI_RTN_CODE aui_common_ring_buf_remain (

    aui_ring_buf *p_ring_buf,

    unsigned long *pul_buf_remain

    );

/**
@brief          Function use to get the start address of the ring buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer of
                                      which getting the start address

@param[out]     ppuc_buf_addr       = Pointer to the start address of the ring
                                      buffer just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the start address of the ring
                                      buffer performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the start address of the ring
                                      buffer failed for some reasons
*/
AUI_RTN_CODE aui_common_get_ring_buf_base_addr (

    aui_ring_buf *p_ring_buf,

    unsigned char **ppuc_buf_addr

    );

/**
@brief          Function used to read data from the ring buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer
                                      from which read data
@param[in]      ul_buf_len          = Length of data to be read from the ring
                                      buffer

@param[out]     pul_real_data_len   = The actual length data read from the ring
                                      buffer
@param[out]     puc_data_out          = Start address of data to be read from the
                                      ring buffer

@return         @b AUI_RTN_SUCCESS  = Reading of the data from the ring buffer
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Reading of the data from the ring buffer
                                      failed for some reasons
*/
AUI_RTN_CODE aui_common_ring_buf_rd (

    aui_ring_buf *p_ring_buf,

    unsigned long ul_buf_len,

    unsigned long *pul_real_data_len,

    unsigned char *puc_data_out

    );

/**
@brief          Function used to write data into the ring buffer

@param[in]      p_ring_buf          = Pointer to a struct #aui_ring_buf which
                                      specify the handle of the ring buffer
                                      from which reading data
@param[in]      ul_buf_len          = Length of data to be written in the
                                      ring buffer
@param[in]      puc_data_in         = Start address of data to be written into
                                      the ring buffer

@return         @b AUI_RTN_SUCCESS  = Writing of data into the ring buffer
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Writing of data into the ring buffer
                                      failed for some reasons
*/
AUI_RTN_CODE aui_common_ring_buf_wt (

    aui_ring_buf *p_ring_buf,

    unsigned long ul_buf_len,

    unsigned char *puc_data_in

    );

/*****************************Global Function List*****************************/

/**
@brief          Function used to get the handle of an AUI API device known the
                device index

@param[in]      dev_type            = Device type as defined in the enum
                                      #aui_module_id
@param[in]      ul_dev_idx          = Device index

@param[out]     p_aui_hdl           = Handle of the device just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the handle performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the handle failed for some
                                      reasons
*/
AUI_RTN_CODE aui_find_dev_by_idx (

    aui_module_id dev_type,

    unsigned long ul_dev_idx,

    aui_hdl *p_aui_hdl

    );

/**
@brief          Function used to set the log priority of a specific AUI Module

@param[in]      module              = AUI module ID, as defined in the enum
                                      #aui_module_id
@param[in]      prio                = Log priority

@return         @b AUI_RTN_SUCCESS  = Setting of the specific AUI Module's log
                                      priority performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Setting of the specific AUI Module's log
                                      priority failed for some reasons
*/
AUI_RTN_CODE aui_log_priority_set(

    int module,

    int prio

    );

/**
@brief          Function used to get the information of AUI package and related package

@param[out]     aui_package_info = The pointer to accept the string of aui
										package and it's related package's information

@param[in]     max_len     = The pointer's size
								       

@return         @b AUI_RTN_SUCCESS  = Get the information of aui's package successfully

@return         @b AUI_RTN_EINVAL   = Getging the information of aui's package is invalid. 
*/
AUI_RTN_CODE aui_log_package_info_get(
		char* aui_package_info,
		int max_len
    );


#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


