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
Current Author:     Amu.Tu, Nick.Li
Last update:        2017.05.08
-->

@file   aui_deca.h

@brief  Audio Decoder (DECA) Module

        Audio Decoder (DECA) Module is used to process ES Data from De-Multiplexing
        (DMX) Module or memory
        When the data source is from DMX Module, that one will
        - Receive TS packets from
        - Transport Stream Switch Interface (TSI) Module, which gets data
          from either NET Interface (NIM) Module or Transport Stream
          Generator (TSG) Module
        - Local disk by injecting directly
        - Parse PES Data to ES data
        - Send the ES Data to DECA Module

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Audio Decoder (DECA) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DECA_H

#define _AUI_DECA_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List******************************/

/**
@warning    This macro is no longer supported then is @b deprecated
*/
#define AUI_MODULE_VERSION_NUM_DECA (0X00010000)

/**
Macro used to set @b 8kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_8KHZ_SAMPLES     1

/**
Macro used to set @b 16kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_16KHZ_SAMPLES    2

/**
Macro used to set @b 22kHz as <b> sample rate </b> of @b PCM data

*/
#define AUI_PCM_22KHZ_SAMPLES    3

/**
Macro used to set @b 24kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_24KHZ_SAMPLES    4

/**
Macro used to set @b 32kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_32KHZ_SAMPLES    5

/**
Macro used to set @b 44kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_44KHZ_SAMPLES    6

/**
Macro used to set @b 48kHz as <b> sample rate </b> of @b PCM data
*/

#define AUI_PCM_48KHZ_SAMPLES    7

/**
Macro used to set @b 64kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_64KHZ_SAMPLES    8

/**
Macro used to set @b 88kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_88KHZ_SAMPLES    9

/**
Macro used to set @b 96kHz as <b> sample rate </b> of @b PCM data
*/
#define AUI_PCM_96KHZ_SAMPLES    10

/**
Macro used to set @b 8bits for the <b> sample quantization </b> of @b PCM data
*/
#define AUI_PCM_8BIT_SAMPLES            1

/**
Macro used to set @b 16bits for the <b> sample quantization bits </b> of @b PCM data
*/
#define AUI_PCM_16BIT_SAMPLES           2

/**
Macro used to set @b 18bits for the <b> sample quantization bits </b> of @b PCM data
*/
#define AUI_PCM_18BIT_SAMPLES           3

/**
Macro used to set @b 20bits for the <b> sample quantization bits </b> of @b PCM data
*/
#define AUI_PCM_20BIT_SAMPLES           4

/**
Macro used to set @b 24bits for the <b> sample quantization bits </b> of @b PCM data
*/
#define AUI_PCM_24BIT_SAMPLES           5

/**
Macro used to set @b 32bits for the <b> sample quantization </b> of @b PCM data
*/
#define AUI_PCM_32BIT_SAMPLES           6

/**
Macro used to set the @b MSB (Most Significant Bit) as <b> justification type
</b> of @b PCM data
*/
#define AUI_PCM_SAMPLE_MSB_JUSTIFIED   0

/**
Macro used to set the @b LSB (Least Significant Bit) as <b> justification type
</b> of @b PCM data
*/
#define AUI_PCM_SAMPLE_LSB_JUSTIFIED   1

/**
Macro used to set <b> big-endian </b> format for the <b> data sequence </b> of
<b> multi-byte sample </b>
*/
#define AUI_PCM_SAMPLE_BIG_ENDIAN      0

/**
Macro used to set <b> little-endian </b> format for the <b> data sequence </b>
of <b> multi-byte sample </b>
*/
#define AUI_PCM_SAMPLE_LITTLE_ENDIAN   1

/**
Macro used to set @b Mono as the <b> channel type </b> of @b PCM data
*/
#define AUI_PCM_MONO                   0

/**
Macro used to set @b Stereo (with the @b right channel as the first one) as <b>
channel type </b> of @b PCM data
*/
#define AUI_PCM_STEREO_RIGHT_FIRST     1

/**
Macro used to set @b Stereo (with the @b left channel as the first one) as <b>
channel type </b> of @b PCM data
*/
#define AUI_PCM_STEREO_LEFT_FIRST      2

/**
Macro used to set <b> Multiple Channel </b> as <b> channel type </b> of
@b PCM data
*/
#define AUI_PCM_MULTI_CHANNEL          3

/**
Macro used to check whether @b 8kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_8KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 8kHz as sample rate of PCM data is
        actually supported.
*/
#define AUI_PCM_8KHZ_SAMPLE_MASK        (1 << AUI_PCM_8KHZ_SAMPLES)

/**
Macro used to check whether @b 16kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_16KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 16kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_16KHZ_SAMPLE_MASK       (1 << AUI_PCM_16KHZ_SAMPLES)

/**
Macro used to check whether @b 22.05kHz as <b> sample rate </b> of @b PCM data
is supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_22KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 22kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_22KHZ_SAMPLE_MASK       (1 << AUI_PCM_22KHZ_SAMPLES)

/**
Macro used to check whether @b 24kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_24KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 24kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_24KHZ_SAMPLE_MASK       (1 << AUI_PCM_24KHZ_SAMPLES)

/**
Macro used to check whether @b 32kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_32KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 32kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_32KHZ_SAMPLE_MASK       (1 << AUI_PCM_32KHZ_SAMPLES)

/**
Macro used to check whether @b 44.1kHz as <b> sample rate </b> of @b PCM data
is supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_44KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 44kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_44KHZ_SAMPLE_MASK       (1 << AUI_PCM_44KHZ_SAMPLES)

/**
Macro used to check whether @b 48kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_48KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 48kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_48KHZ_SAMPLE_MASK       (1 << AUI_PCM_48KHZ_SAMPLES)

/**
Macro used to check whether @b 64kHz as <b> sample rate </b> of @b PCM data
is supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_64KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 64kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_64KHZ_SAMPLE_MASK       (1 << AUI_PCM_64KHZ_SAMPLES)

/**
Macro used to check whether @b 88kHz as <b> sample rate </b> of @b PCM data
is supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_88KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 88kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_88KHZ_SAMPLE_MASK       (1 << AUI_PCM_88KHZ_SAMPLES)

/**
Macro used to check whether @b 96kHz as <b> sample rate </b> of @b PCM data is
supported or not

@note   This macro is a bitmask used on the member @b sampling_rates of the
        struct #aui_deca_pcm_cap \n
        The value of @b sampling_rates is given by the function #aui_deca_get
        with input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_96KHZ_SAMPLE_MASK & @b sampling_rates \n\n

        is @b TRUE (i.e. @b 1) then @b 96kHz as sample rate of PCM data is
        actually supported
*/
#define AUI_PCM_96KHZ_SAMPLE_MASK       (1 << AUI_PCM_96KHZ_SAMPLES)

/**
Macro used to check whether @b 8bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the struct
        #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_8BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 8bits for the sample quantization of PCM
        data is actually supported.
*/
#define AUI_PCM_8BIT_SAMPLE_MASK         (1 << AUI_PCM_8BIT_SAMPLES)

/**
Macro used to check whether @b 16bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the struct
        #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_16BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 16bits for the sample quantization of
        PCM data is actually supported.
*/
#define AUI_PCM_16BIT_SAMPLE_MASK        (1 << AUI_PCM_16BIT_SAMPLES)

/**
Macro used to check whether @b 18bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the
        struct #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_18BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 18bits for the sample quantization of
        PCM data is actually supported.
*/
#define AUI_PCM_18BIT_SAMPLE_MASK        (1 << AUI_PCM_18BIT_SAMPLES)

/**
Macro used to check whether @b 20bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the struct
        #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_20BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 20bits for the sample quantization of
        PCM data is actually supported.
*/
#define AUI_PCM_20BIT_SAMPLE_MASK        (1 << AUI_PCM_20BIT_SAMPLES)

/**
Macro used to check whether @b 24bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the struct
        #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_24BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 24bits for the sample quantization of
        PCM data is actually supported.
*/
#define AUI_PCM_24BIT_SAMPLE_MASK        (1 << AUI_PCM_24BIT_SAMPLES)

/**
Macro used to check whether @b 32bits for the <b> sample quantization </b> of
@b PCM data is supported or not

@note   This macro is a bitmask used on the member @b sample_size of the struct
        #aui_deca_pcm_cap \n
        The value of @b sample_size is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_32BIT_SAMPLE_MASK & @b sample_size \n\n

        is @b TRUE (i.e. @b 1) then @b 32bits for the sample quantization of
        PCM data is actually supported.
*/
#define AUI_PCM_32BIT_SAMPLE_MASK        (1 << AUI_PCM_32BIT_SAMPLES)

/**
Macro used to check whether <b> big-endian </b> format for the <b> data sequence
</b> of <b> multi-byte sample </b> is supported or not

@note   This macro is a bitmask used on the member @b endians of the struct
        #aui_deca_pcm_cap \n
        The value of @b endians is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_BIG_ENDIAN_MASK & @b endians \n\n

        is @b TRUE (i.e. @b 1) then @b big-endian format for the data sequence
        of multi-byte sample is actually supported.
*/
#define AUI_PCM_BIG_ENDIAN_MASK          (1 << AUI_PCM_SAMPLE_BIG_ENDIAN)

/**
Macro used to check whether <b> little-endian </b> format for the <b> data
sequence </b> of <b> multi-byte sample </b> is supported or not

@note   This macro is a bitmask used on the member @b endians of the struct
        #aui_deca_pcm_cap \n
        The value of @b endians is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_LITTLE_ENDIAN_MASK & @b endians \n\n

        is @b TRUE (i.e. @b 1) then @b little-endian format for the data
        sequence of multi-byte sample is actually supported
*/
#define AUI_PCM_LITTLE_ENDIAN_MASK       (1 << AUI_PCM_SAMPLE_LITTLE_ENDIAN)

/**
Macro used to check whether @b Mono as the <b> channel type </b> of @b PCM data
is supported or not

@note   This macro is a bitmask used on the member @b channels of the struct
        #aui_deca_pcm_cap \n
        The value of @b channels is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_MONO_MASK & @b channels \n\n

        is @b TRUE (i.e. @b 1) then @b MONO as channel type of PCM data is
        actually supported.
*/
#define AUI_PCM_MONO_MASK                        (1 << AUI_PCM_MONO)

/**
Macro used to check whether @b Stereo (with the @b right channel as the first
one) as <b> channel type </b> of @b PCM data is supported or not

@note   This macro is a bitmask used on the member @b channels of the struct
        #aui_deca_pcm_cap \n
        The value of @b channels is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_STEREO_RIGHT_FIRST & @b channels \n\n

        is @b TRUE (i.e. @b 1) then @b Stereo (with the @b right channel as the
        first one) as channel type of PCM data is supported.
*/
#define AUI_PCM_STEREO_RIGHT_FIRST_MASK          (1 << AUI_PCM_STEREO_RIGHT_FIRST)

/**
Macro used to check whether @b Stereo (with the @b left channel as the first
one) as <b> channel type </b> of @b PCM data is supported or not

@note   This macro is a bitmask used on the member @b channels of the struct
        #aui_deca_pcm_cap \n
        The value of @b channels is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_STEREO_LEFT_FIRST & @b channels \n\n

        is @b TRUE (i.e. @b 1) then @b Stereo (with the @b left channel as the
        first one) as channel type of PCM data is supported.

*/
#define AUI_PCM_STEREO_LEFT_FIRST_MASK           (1 << AUI_PCM_STEREO_LEFT_FIRST)

/**
Macro used to check whether <b> Multiple Channel </b> as <b> channel type </b>
of @b PCM data is supported or not

@note   This macro is a bitmask used on the member @b channels of the struct
        #aui_deca_pcm_cap \n
        The value of @b channels is given by the function #aui_deca_get with
        input parameter the value #AUI_DECA_PCM_CAP_GET of the enum
        #aui_deca_item_get \n
        If the expression \n\n

        #AUI_PCM_MULTI_CHANNEL_MASK & @b channels \n\n

        is @b TRUE (i.e. @b 1) then <b> Multiple Channel </b> as channel type
        of PCM data is supported.
*/
#define AUI_PCM_MULTI_CHANNEL_MASK               (1 << AUI_PCM_MULTI_CHANNEL)

/*******************************Global Type List*******************************/

/**
Function pointer to specify the type of callback function filled in the struct
#aui_st_deca_io_reg_callback_para and registered for various audio events as
present in the enum #aui_deca_cb_type

@note   The callback function receives @b p_user_data as user parameter which
        is given when registering, please refer to the struct
        #aui_st_deca_io_reg_callback_para for more information.\n
        The input parameters @b pv_param1 and @b pv_param2 are used in different
        callback notification. more information about them can be found in the
        enum #aui_deca_cb_type
*/
typedef void (*aui_deca_cb_func) (

    void *pv_param1,

    void *pv_param2,

    void *p_user_data

    );

/**
Enum to specify the different callback types for all possible audio events
*/
typedef enum aui_deca_cb_type {

    /**
    Value to specify the callback type for the event
    <b> New Audio frame decoded </b>

    @note   When calling the function #aui_deca_set with the value
            #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input parameter,
            @b pv_param1 and @b pv_param2 will get the following values:
            - @b pv_param1 = The value depends of the OS as below:
                             - In project based on <b> TDS OS </b>, the value is
                               the pointer to the struct
                               #aui_deca_io_reg_callback_para
                             - In project based on <b> Linux OS </b>, the value
                               is NULL
            - @b pv_param2 = NULL
    */
    AUI_DECA_CB_MONITOR_NEW_FRAME = 0,

    /**
    Value to specify the callback type for the event
    <b> DECA device started to work </b>

    @note   When calling the function #aui_deca_set with the value
            #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input parameter,
            @b pv_param1 and @b pv_param2 will get the following values:
            - @b pv_param1 = The value depends of the OS as below:
                             - In project based on <b> TDS OS </b>, the value
                               is the pointer to the struct
                               #aui_deca_io_reg_callback_para
                             - In project based on <b> Linux OS </b>, the value
                               is NULL
            - @b pv_param2 = NULL
    */
    AUI_DECA_CB_MONITOR_START,

    /**
    Value to specify the callback type for the event
    <b> DECA device stopped working </b>

    @note   When calling the function #aui_deca_set with the value
            #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input parameter,
            @b pv_param1 and @b pv_param2 will get the following values:
            - @b pv_param1 = The value depends of the OS as below:
                             - In project based on <b> TDS OS </b>, the value
                               is the pointer to the struct
                               #aui_deca_io_reg_callback_para
                             - In project based on <b> Linux OS </b>, the value
                               is NULL
            - @b pv_param2 = NULL
    */
    AUI_DECA_CB_MONITOR_STOP,

    /**
    Value to specify the callback type for the event
    <b> Decoding audio frame error occured </b>

    @note   When calling the function #aui_deca_set with the value
            #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input parameter,
            @b pv_param1 and @b pv_param2 will get the following values:
            - @b pv_param1 = The value depends of the OS as below:
                             - In project based on <b> TDS OS </b>, the value
                               is the pointer to the struct
                               #aui_deca_io_reg_callback_para
                             - In project based on <b> Linux OS </b>, the value
                               is NULL
            - @b pv_param2 = NULL
    */
    AUI_DECA_CB_MONITOR_DECODE_ERR,

    /**
    Value to specify the callback type for the event
    <b> Status of the audio decoder changed </b> from
    @a Underrun to @a Normal state

    @note   @b 1. @a Underrun means that there is no data for audio driver to
                  decode
    @note   @b 2. When calling the function #aui_deca_set with the value
                  #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input
                  parameter, @b pv_param1 and @b pv_param2 will get the
                  following values:
                  - @b pv_param1 = The value depends of the OS as below:
                                   - In project based on <b> TDS OS </b>, the
                                     value is the pointer to the struct
                                     #aui_deca_io_reg_callback_para
                                   - In project based on <b> Linux OS </b>, the
                                     value is NULL
                  - @b pv_param2 = NULL
    */
    AUI_DECA_CB_MONITOR_OTHER_ERR,

    /**
    Value to specify the callback type for the event <b> First frame decoded
    successfully </b> since DECA Module started to work

    @note   @b 1. When calling the function #aui_deca_set with the value
                  #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input
                  parameter, @b pv_param1 and @b pv_param2 will get the following
                  values:
                  - @b pv_param1 = NULL
                  - @b pv_param2 = NULL
    @note   @b 2. This callback type is only used in projects based on
                  <b> Linux OS </b>
    */
    AUI_DECA_CB_FIRST_FRAME,

    /**
    @warning    This callback type is no longer supported then is @a deprecated
    */
    AUI_DECA_CB_EWS = 20,

    /**
    Value to specify the callback type for the event
    <b> Status of DECA Module changed </b>

    @note   @b 1. When calling the function #aui_deca_set with the value
                  #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input
                  parameter, @b pv_param1 and @b pv_param2 will get the
                  following values:
                  - @b pv_param1 = The value is the pointer to the struct
                                   #aui_deca_io_reg_callback_para
                  - @b pv_param2 = NULL
    @note   @b 2. This callback type is @a only used in projects based on
                  <b> TDS OS </b>
    */
    AUI_DECA_CB_STATE_CHANGED,

    /**
    Value to specify the total number of callback type available in this enum
    */
    AUI_DECA_CB_MAX

} aui_deca_cb_type, aui_deca_cb_type_e;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify
        <em> Callback Type, Callback Function, User Data, Parameters </em>
        to be used
        </div> @endhtmlonly

        Struct to specify
        - A member as <em> Callback type </em>
        - A member as <em> Callback function </em>
        - A member a pointer to <em> user data </em> for the callback function
        - Other members as callback parameters

        This struct is used by the function #aui_deca_set with the value
        #AUI_DECA_REG_CB of the enum #aui_deca_item_set as input parameter
*/
typedef struct aui_st_deca_io_reg_callback_para {

    /**
    Member as <em> Callback Type </em> (i.e. as the enum #aui_deca_cb_type)
    */
    aui_deca_cb_type en_cb_type;

    /**
    Member as <em> Callback Function </em> (i.e. as the callback function
    #aui_deca_cb_func)
    */
    aui_deca_cb_func p_cb;

    /**
    Member as pointer to @b p_user_data for the callback function with
    the identifier @b p_cb
    */
    void *pv_param;

    /**
    Member to specify how often the callback type for the event
    <b> New Audio frame decoded </b>
    (i.e. defined with the value #AUI_DECA_CB_MONITOR_NEW_FRAME in the enum
    #aui_deca_cb_type) has to be called

    @note   It is recommended to initialize this value to 50 (in <em> times per
            second </em> unit)
    */
    unsigned long monitor_rate;

    /**
    Member as a @a flag used by the callback type for the event <b> DECA Module
    Status changed </b> (i.e. defined with the value #AUI_DECA_CB_STATE_CHANGED
    in the enum #aui_deca_cb_type) to notify the upper level about the new
    status, in particular
    - @b 0 = No data for the audio decoder
    - @b 1 = Decoding
    */
    unsigned long status;

    /**
    Member to specify the @b timeout (in @a millisecond (ms) unit) of no data
    for the audio decoder to rise the event <b> DECA Module Status changed </b>
    (i.e. defined with the value #AUI_DECA_CB_STATE_CHANGED in the enum
    #aui_deca_cb_type) after the decoding state occured

    @note   If user set the timeout to @b 0 then the audio driver will use
            @b 400ms as default value
    */
    unsigned long timeout;

} aui_deca_io_reg_callback_para;


#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio Decoder (DECA) Module </b> <em> deprecated </em>
       then ignore it
       </div> @endhtmlonly

@warning    This struct is no longer supported then is @a deprecated
*/
typedef struct aui_avsync_ctrl {

    unsigned char stc_id_valid;

    unsigned char pts_valid;

    unsigned char data_continue;

    unsigned char ctrlblk_valid;

    unsigned char instant_update;

    unsigned char vob_start;

    unsigned char reserve;

    unsigned char stc_id;

    unsigned long pts;

} aui_avsync_ctrl;

/**
Enum to specify all the available <b> Audio Sample Rate </b>
*/
typedef enum aui_deca_sample_rate {

    /**
    Value to specify an @b invalid audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_INVALID = 1,

    /**
    Value to specify @b 8kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_8,

    /**
    Value to specify @b 11.025kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_11,

    /**
    Value to specify @b 12kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_12,

    /**
    Value to specify @b 16kHz
    */
    AUI_DECA_STREAM_SAMPLE_RATE_16,

    /**
    Value to specify @b 22.05kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_22,

    /**
    Value to specify @b 24kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_24,

    /**
    Value to specify @b 32kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_32,

    /**
    Value to specify @b 44.1kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_44,

    /**
    Value to specify @b 48kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_48,

    /**
    Value to specify @b 64kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_64,

    /**
    Value to specify @b 88.2kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_88,

    /**
    Value to specify @b 96kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_96,

    /**
    Value to specify @b 128kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_128,

    /**
    Value to specify @b 176.4kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_176,

    /**
    Value to specify @b 192 kHz as audio sample rate
    */
    AUI_DECA_STREAM_SAMPLE_RATE_192,

    /**
    Value to specify the total number of audio sample rate available
    */
    AUI_DECA_STREAM_SAMPLE_LAST

} aui_deca_sample_rate;

/**
Enum to specify all the <b> Audio Sample Quantization Width (in bits) </b>
available
*/
typedef enum aui_deca_audio_quantization {
    /**
    Value to specify an @b invalid audio sample quantization width

    */
    AUI_DECA_AUDIO_QWLEN_INVALID = 1,

    /**
    Value to specify @b 8bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_8,

    /**
    Value to specify @b 12bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_12,

    /**
    Value to specify @b 16 bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_16,

    /**
    Value to specify @b 20 bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_20,

    /**
    Value to specify @b 24 bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_24,

    /**
    Value to specify @b 32 bits as audio sample quantization width
    */
    AUI_DECA_AUDIO_QWLEN_32,

    /**
    Value to specify the total number of audio sample quantization width
    available
    */
    AUI_DECA_AUDIO_QWLEN_LAST

} aui_deca_audio_quantization;

/**
@warning    This enum is no longer supported then is @a deprecated
*/
typedef enum aui_deca_stop_mode {

    AUI_DECA_STOP_IMM = 1,

    AUI_DECA_STOP_PTS,

    AUI_DECA_STOP_END

} aui_deca_stop_mode;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
       attributes of DECA device to inject data
       </div> @endhtmlonly

       Struct to specify the attributes of DECA device to inject data

@warning  This struct is no longer supported then is @a deprecated
*/
typedef struct aui_st_inject_deca {

  /**
  Member as an @a index to specify the SBM device
  */
  int i_sbm_idx;

  /**
  Member to specify the buffer size allocated by the CPU when it is injecting
  audio data into SEE DECA
  */
  unsigned long ul_buf_len;

  /**
  Member to specify the buffer address
  */
  char *ul_buf_base_addr;

  /**
  Member to specify the block size when reading one time
  */
  unsigned long ul_block_size;

  /**
  Member to specify the reserved size at the end of the buffer
  */
  unsigned long ul_reserve_len;

  //unsigned long ul_wrap_mode;

  //unsigned long ul_lock_mode;

} aui_inject_deca, *aui_p_inject_deca;

#endif

/**
Enum to specify all the audio decoder settings available to be performed by the
function #aui_deca_set

@note   When calling the function #aui_deca_set, its input parameter
        @b pv_param takes the pointer as per the description of the specifc
        setting to perform
*/
typedef enum aui_en_deca_item_set {

    /**
    Value used to set the <b> Audio Decode Type </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the enum
                  #aui_deca_stream_type
    @note   @b 2. The function #aui_deca_type_set can be used to perform the
                  same setting
    */
    AUI_DECA_DEOCDER_TYPE_SET=0,

    /**
    Value used to set the <b> Audio Synchronization Mode </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the enum
                  #aui_deca_sync_mode
    @note   @b 2. The function #aui_deca_sync_mode_set can be used to perform
                  the same setting
    */
    AUI_DECA_AUD_SYNC_MODE_SET,

    /**
    @warning    This setting is not used currently then user can ignore it
    */
    AUI_DECA_AUD_SYNC_LEVEL_SET,

    /**
    @warning    This setting is not used currently then user can ignore it
    */
    AUI_DECA_AUD_PID_SET,

    /**
    Value used to set the <b> bypassing of the bitstream </b>, which audio
    decoder will output raw data

    @note   The parameter @b pv_param takes the pointer to the enum
            #aui_deca_stream_type, in particular to the values
            - Either #AUI_DECA_STREAM_TYPE_EC3
            - Or #AUI_DECA_STREAM_TYPE_AC3

            in order to let the audio decoder output raw data
    */
    AUI_DECA_ADD_BS_SET,

    /**
    Value used to set the <b> writing of the ES data </b> into audio driver

    @note   @b 1. The parameter @b pv_param takes the pointer to the struct
                  #aui_cmd_deca_snd_data
    @note   @b 2. This value is @a only available for projects based on
                  <b> TDS OS </b>
    */
    AUI_DECA_INJECT_ES_DATA,

    /**
    Value used to set the <b> parameters to PCM decoder </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the struct
                  #aui_deca_pcm_param
    @note   @b 2. This value is @a only available for @b OPENTV projects based
                  on <b> TDS OS </b>
    */
    AUI_DECA_PCM_PARAM_SET,

    /**
    Value used to set the <b> registering of the callback function </b> to audio
    decoder as per the type defined in the enum #aui_deca_cb_type

    @note   The parameter @b pv_param takes the pointer to the enum
            #aui_deca_cb_type
    */
    AUI_DECA_REG_CB,

    /**
    Value used to set the <b> preparing of the audio track change </b>

    @note   The parameter @b pv_param is set as NULL
    */
    AUI_DECA_PREPARE_CHANGE_AUD_TRACK,

    /**
    Value used to set the <b> disabling of the bypass of the bistream </b> and
    remove the setting performed as per comment for the value
    #AUI_DECA_ADD_BS_SET of this enum

    @note   The parameter @b pv_param is set as NULL
    */
    AUI_DECA_EMPTY_BS_SET,

    /**
    Value to specify the total number of item in this enum
    */
    AUI_DECA_SET_CMD_LAST

} aui_deca_item_set;

/**
Enum to specify all the audio decoder settings available to be gotten by the
function #aui_deca_set

@note   When calling the function #aui_deca_get, the parameter @b pv_param
        takes the pointer as per the description of the specifc setting to get
*/
typedef enum aui_en_deca_item_get {

    /**
    Value used to get the <b> DECA decoding information </b>

    @note   The parameter @b pv_param takes the pointer to the struct
            #aui_st_deca_data_info
    */
    AUI_DECA_DATA_INFO_GET=0,

    /**
    Value used to get the <b> current DECA coding type </b>

    @note   The parameter @b pv_param takes the pointer to the enum
            #aui_deca_stream_type
    */
    AUI_DECA_DEOCDER_TYPE_GET,

    /**
    Value used to get the <b> current synchronization mode </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the enum
                  #aui_deca_sync_mode
    @note   @b 2. The function #aui_deca_sync_mode_get can be used to get the
                  same information
    */
    AUI_DECA_AUD_SYNC_MODE_GET,

    /**
    @warning    This value is no longer supported then is @a deprecated
    */
    AUI_DECA_AUD_SYNC_LEVEL_GET,

    /**
    @warning    This setting is no longer supported then is @a deprecated
    */
    AUI_DECA_AUD_PID_GET,

    /**
    @warning    This setting is no longer supported then is @a deprecated
    */
    AUI_DECA_BUF_DATA_PTR,

    /**
    Value used to get the <b> status of DECA Module </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to status of the
                  DECA Module
    @note   @b 2. The function #aui_deca_status_get can be used to get the same
                  information
    */
    AUI_DECA_DEOCDER_STATUS_GET,

    /**
    Value used to get the <b> parameters from PCM decoder </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the struct
                  #aui_deca_pcm_param
    @note   @b 2. This value is @a only available for projects based on
                  <b> TDS OS </b>
    */
    AUI_DECA_PCM_PARAM_GET,

    /**
    Value used to get the <b> capability of the PCM decoder </b>

    @note   @b 1. The parameter @b pv_param takes the pointer to the struct
                  #aui_deca_pcm_cap
    @note   @b 2. This value is @a only available for projects based on
                  <b> TDS OS </b>
    */
    AUI_DECA_PCM_CAP_GET,

    /**
    Value to specify the total number of item in this enum
    */
    AUI_DECA_GET_CMD_LAST

} aui_deca_item_get;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
        command parameters for sending data to the DECA buffer
        </div> @endhtmlonly

        Struct to specify the command parameters for sending data to the
        DECA buffer
*/
typedef struct aui_st_cmd_deca_snd_data {

    /**
    Member to specify the @b length of data to be sent
    */
    unsigned long ul_data_len;

    /**
    Member to specify the @b address of data to be sent
    */
    unsigned char *puc_data_addr;

    /**
    @warning    This member is @b deprecated then user can ignore it
    */
    aui_avsync_ctrl st_ctl_blk;

    /**
    Member to specify the @b length of data @a actually sent
    */
    unsigned long ul_real_send_data_len;

} aui_cmd_deca_snd_data, *aui_p_cmd_deca_snd_data;

/**
Enum to specify all the different <b> synchronization modes </b> of the audio
available to be configured
*/
typedef enum aui_en_deca_sync_mode {

    /**
    Value to specify that the audio is synchronized with the System Time Clock
    (STC) provided by the Program Clock Reference (PCR)
    */
    AUI_DECA_DEOCDER_SYNC=0,

    /**
    Value tos specify that the audio frame will free run according to the frame
    rate (in @a fps unit ) of the audio stream regardless of the System Time
    Clock (STC)

    @note   Please set the DECA to free run when the program has audio only
    */
    AUI_DECA_DEOCDER_ASYNC

} aui_deca_sync_mode;

/**
Enum to specify different @b status of the audio decoder

@note   To know the audio decoder status, please use the function #aui_deca_get
        with input parameter the value #AUI_DECA_DEOCDER_STATUS_GET of the enum
        #aui_deca_item_get
*/
typedef enum aui_en_deca_status {

    /**
    Value to specify that the audio decoder is <b> not ready </b> to decode
    */
    AUI_DECA_DEATTACH=0,

    /**
    Value to specify that the audio decoder is @b ready to decode.
    */
    AUI_DECA_ATTACH,

    /**
    Value to specify that the audio decoder has been @b stopped after its opening
    and starting.\n
    After stopping, the decoding stage can start again on demand
    */
    AUI_DECA_STOP,

    /**
    Value to specify that the audio decoder is in the <b> decoding stage </b>
    after its opening
    */
    AUI_DECA_RUN,

    /**
    Value to specify that the audio decoder has been @b paused after its
    opening and starting.\n
    After pausing, the decoding stage can be resumed on demand
    */
    AUI_DECA_PAUSE

} aui_deca_status;

/**
Enum to specify different <b> audio coding mode </b> when audio stream types is
set to the value #AUI_DECA_STREAM_TYPE_AC3 or #AUI_DECA_STREAM_TYPE_EC3

@note   To know the audio mode information, please use the function
        #aui_deca_data_info_get
*/
typedef enum aui_deca_ac3_acmode {

    /**
    Value to specify that the channel order is
    - @b CH1
    - @b CH2

    considering there are two mono channels in the AC3 stream
    */
    AUI_DECA_AC3_ACMODE_TWO_MONO_1_CH1_CH2,

    /**
    Value to specify that the channel order is
    - @c C

    considering there is only one channel in the AC3 stream
    */
    AUI_DECA_AC3_ACMODE_ONE_CENTER_1_0_C,

    /**
    Value to specify that the channel order is
    - @b L
    - @b R

    considering there are two channels (stereo) in the AC3 stream
    */
    AUI_DECA_AC3_ACMODE_TWO_CHANNEL_2_0_L_R,

    /**
    Value to specify that the channel order is
    - @b L
    - @b C
    - @b R
    */
    AUI_DECA_AC3_ACMODE_THREE_CHANNEL_3_0_L_C_R,

    /**
    Value to specify that the channel order is
    - @b L
    - @b R
    - @b S
    */
    AUI_DECA_AC3_ACMODE_THREE_CHANNEL_2_1_L_R_S,

    /**
    Value to specify that the channel order is
    - @b L
    - @b C
    - @b R
    - @b S
    */
    AUI_DECA_AC3_ACMODE_FOUR_CHANNEL_3_1_L_C_R_S,

    /**
    Value to specify that the channel order is
    - @b L
    - @b R
    - @b SL
    - @b SR
    */
    AUI_DECA_AC3_ACMODE_FOUR_CHANNEL_2_2_L_R_SL_SR,

    /**
    Value to specify that the channel order is
    - @b L
    - @b C
    - @b R
    - @b S
    - @b SR
    */
    AUI_DECA_AC3_ACMODE_FIVE_CHANNEL_3_2_L_C_R_SL_SR

} aui_deca_ac3_acmode;

/**
Enum to specify different <b> channel mode </b> when audio stream type is
set to the value #AUI_DECA_STREAM_TYPE_MPEG1 or #AUI_DECA_STREAM_TYPE_MPEG2

@note   To know the audio mode information, please use the function
        #aui_deca_data_info_get
*/
typedef enum aui_deca_mpeg_channel_mode {

    /**
    Value to specify that the audio channel mode is @b stereo
    */
    AUI_DECA_MPEG_CHANNEL_MODE_STEREO,

    /**
    Value to specify that the audio channel mode is <b> joint stereo </b>
    */
    AUI_DECA_MPEG_CHANNEL_MODE_JOINT_STEREO,

    /**
    Value to specify that the audio channel mode is <b> dual channel </b>
    */
    AUI_DECA_MPEG_CHANNEL_MODE_DUAL_CHANNEL,

    /**
    Value to specify that the audio channel mode is <b> single channel </b>
    */
    AUI_DECA_MPEG_CHANNEL_MODE_SINGLE_CHANNEL

} aui_deca_mpeg_channel_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
        attributes available to be configured
        </div> @endhtmlonly

        Struct to specify the attributes of DECA Module available to be
        configured
*/
typedef struct aui_st_attr_deca {

    /**
    Member as an @a Index for referring to different supported DECA devices

    @note   Currently, @a only one (1) DECA device is supported therefore that
            index can take @a only the value zero (0)
    */
    unsigned char uc_dev_idx;

    /**
    Member to specify the <b> Audio Stream Type </b>, as defined in the enum
    #aui_deca_stream_type
    */
    aui_deca_stream_type en_stream_aud_type;

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    Member to specify the <b> Total number of channels </b> available.
    */
    unsigned char uc_channel_num;

    /**
    Member to specify the <b> Audio Stream Sample Rate </b>, as defined in the
    enum #aui_deca_sample_rate
    */
    aui_deca_sample_rate en_sample_rate;

    /**
    Member to specify the <b> Sample Quantization Width  </b>, which can be
    found in the enum #aui_deca_audio_quantization
    */
    aui_deca_audio_quantization en_quan;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned long ul_pts;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    aui_deca_stop_mode ul_deca_stop_mode;

    /// @coding

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    unsigned long ul_info_struct;

    /// @endcoding

    /**
    Member to specify the <b> Audio Synchronization Mode </b>, as defined in
    the enum #aui_deca_sync_mode
    */
    aui_deca_sync_mode ul_sync_mode;

    /**
    @warning    This member is currently not used then user can ignore it
    */
    unsigned long ul_sync_level;

    /**
    Member to specify the <b> Audio Stream PID </b>
    */
    unsigned short us_aud_pid;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    aui_inject_deca inject_deca;

#endif

} aui_attr_deca, *aui_p_attr_deca;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify audio
        decoding information
        </div> @endhtmlonly

        Struct to specify audio decoding information
*/
typedef struct aui_st_deca_data_info {

    /**
    Member to specify the <b> Audio Decoding Type </b>, as defined in the enum
    #aui_deca_stream_type
    */
    unsigned char uc_deca_type;

    /**
    Member to specify the bit width of the output PCM, it is deprecated now
    */
    unsigned char uc_bit_width;

    /**
    Member to specify the sample rate of the output PCM
    */
    unsigned long ul_sample_rate;

    /**
    Member to specify the total number of <b> samples per audio frame </b>
    */
    unsigned long ul_sample_cnt;

    /**
    Member to specify the total number of <b> audio channels </b>
    */
    unsigned long ul_channel_cnt;

    /*
    Member to specify
    - Either the channel mode of MPEG audio
    - Or the audio coding mode of DD/DDPLUS audio
    */
    union {

        /**
        Sub-member to specify the channel mode of MPEG audio
        */
        aui_deca_mpeg_channel_mode channel_mode;

        /**
        Sub-member to specify the audio coding mode of DD/DDPLUS audio
        */
        aui_deca_ac3_acmode ac_mode;

    } audio_mode;

    /**
    Member to specify the total number of <b> decoded audio frames </b>
    */
    unsigned long ul_frame_cnt;

    /**
    Member to specify the number of <b> audio frames decoding errors </b>
    */
    unsigned long ul_decode_error_cnt;

    /**
    Member to specify the number of <b> dropped audio frames </b>
    */
    unsigned long ul_drop_cnt;

    /**
    Member to specify the total size of the @b buffer used for decoding audio
    data

    @note   @b This member is only used in projects based on <b> Linux OS </b>

    @note   @b When audio decoder is paused, this value will not change even if
               more data are written to the DMX
    */
    unsigned long ul_buffer_total_size;

    /**
    Member to specify the <b> valid size </b> of audio data in the @b buffer
    to be decoded

    @note   @b This member is only used in projects based on <b> Linux OS </b>

    @note   @b When audio decoder is paused, this value will not change even if
               more data are written to the DMX
    */
    unsigned long ul_buffer_valid_size;

    /**
    Member to specify the <b> free size </b> of the buffer used for decoding
    audio data

    @note   @b This member is only used in projects based on <b> Linux OS </b>

    @note   @b When audio decoder is paused, this value will not change even if
               more data are written to the DMX
    */
    unsigned long ul_buffer_free_size;

    /**
    Member to specify the PTS of the current played frame

    @note Available @a only for projects based on <b> Linux OS </b>
    */
    unsigned long ul_last_pts;

    /**
    @warning  This member is @a reserved to ALi R&D Dept. then user can
              ignore it
    */
    unsigned long ul_reserved;

} aui_deca_data_info, *aui_p_deca_data_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify
        information about injected audio data
        </div> @endhtmlonly

        Struct to specify information about injected audio data
*/
typedef struct aui_st_deca_inject_data {

    /**
    Member to specify the @b length of audio data to be injected to DECA Module
    */
    unsigned long data_in_len;

    /**
    Member to specify the @b address of audio data to be injected to DECA Module
    */
    unsigned char *puc_data_in;

    /**
    @warning    This member is no longer supported then is @a deprecated
    */
    aui_avsync_ctrl *p_ctrl_block;

    /**
    Member to specify the <b> actual length </b> of audio data injected to
    DECA Module
    */
    unsigned long data_in_len_real;

} aui_deca_inject_data;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
        attributes of a PCM decoder
        </div> @endhtmlonly

        Struct to specify the attributes of a PCM decoder available
        to be configured
*/
typedef struct aui_st_deca_pcm_param {

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long sampling_rate;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long sample_size;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long final_sample_size;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long sample_justification;

    /**
    Member as a @a flag to specify the <b> multi-byte sample endian mode </b>,
    in particular
    - @b 1 = Big-ndian
    - @b 0 = Little-endian
    */
    unsigned long endian;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long channels;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long channel_layout;

    /**
    Member as @a flag to specify the <b> PCM decoder sample sign </b>,
    in particular
    - @b 1 = Signed
    - @b 0 = Unsigned
    */
    unsigned long sign_flag;

    /**
    Member to specify the <b> frame length </b> (e.g. 1024, 1536)
    */
    unsigned long frame_len;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long reserved;

} aui_deca_pcm_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
        capability attributes of a PCM decoder
        </div> @endhtmlonly

        Struct to specify the capability attributes of a PCM decoder
*/
typedef struct aui_st_deca_pcm_cap {

    /**
    Member to specify the <b> sample rate format </b> supported by PCM decoder

    @note   This member is composed of all the Macros below:
            - #AUI_PCM_8KHZ_SAMPLE_MASK
            - #AUI_PCM_16KHZ_SAMPLE_MASK
            - #AUI_PCM_22KHZ_SAMPLE_MASK
            - #AUI_PCM_24KHZ_SAMPLE_MASK
            - #AUI_PCM_32KHZ_SAMPLE_MASK
            - #AUI_PCM_44KHZ_SAMPLE_MASK
            - #AUI_PCM_48KHZ_SAMPLE_MASK
            - #AUI_PCM_64KHZ_SAMPLE_MASK
            - #AUI_PCM_88KHZ_SAMPLE_MASK
            - #AUI_PCM_96KHZ_SAMPLE_MASK.
    */
    unsigned long sampling_rates;

    /**
    Member to specify the <b> sample size format </b> supported by the PCM decoder

    @note   This member is composed of all the Macros below:
            - #AUI_PCM_8BIT_SAMPLE_MASK
            - #AUI_PCM_16BIT_SAMPLE_MASK
            - #AUI_PCM_18BIT_SAMPLE_MASK
            - #AUI_PCM_20BIT_SAMPLE_MASK
            - #AUI_PCM_24BIT_SAMPLE_MASK
            - #AUI_PCM_32BIT_SAMPLE_MASK
    */
    unsigned long sample_sizes;

    /**
    Member to specify the <b> endians format </b> supported by the PCM decoder

    @note   This member is composed of all the Macros below:
            - #AUI_PCM_BIG_ENDIAN_MASK
            - #AUI_PCM_LITTLE_ENDIAN_MASK
    */
    unsigned long endians;

    /**
    Member to specify the <b> channel type </b> supported by the PCM decoder

    @note   This member is composed of all the Macros below:
            - #AUI_PCM_MONO_MASK
            - #AUI_PCM_STEREO_RIGHT_FIRST_MASK
            - #AUI_PCM_STEREO_LEFT_FIRST_MASK
            - #AUI_PCM_MULTI_CHANNEL_MASK
    */
    unsigned long channels;

    /**
    @warning    This member is @a reserved to ALi R&D Dept. for future use then
                user can ignore it
    */
    unsigned long reserved[2];

} aui_deca_pcm_cap;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Audio Decoder (DECA) Module </b> to specify the
        information of MPEG format data to be sent to Audio decoder to play
        </div> @endhtmlonly

        Struct to specify the information of MPEG format data sent to Audio
        decoder to play

@note This struct is used by the function #aui_deca_str_play
*/
typedef struct aui_deca_stream_play_param {

    /**
    Member to specify the @b address of MPEG format data to be sent
    */
    unsigned char *puc_src;

    /**
    Member to specify the @b length of MPEG format data to be sent
    */
    unsigned int len;

    /**
    Member to specify how many @b times MPEG format data should be played
    */
    unsigned int loop_cnt;
    
} aui_deca_stream_play_param;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@warning        This function is no longer supported then is @a deprecated
*/
AUI_RTN_CODE aui_deca_version_get (

    unsigned long* const pul_version

    );

/**
@brief          Function used to perform the initialization of the DECA Module,
                before its opening by the function #aui_deca_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the DECA Module

@param[in]      p_call_back_init        = Callback function used for the
                                          initialization of the DECA Module
@param[in]      pv_param                = The input parameter of the callback
                                          function @b p_call_back_init used for
                                          the initialization of the DECA Module

@return         @b AUI_RTN_SUCCESS      = Initializing of the DECA Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Initializing of the DECA Module
                                          failed for some reasons

@note           About the callback function #p_call_back_init as input parameter:
                - In projects based on <b> Linux OS </b>, it is suggested to set
                  as NULL
                - In projects based on <b> TDS OS </b>, it needs to perform some
                  audio device attachments and configurations.
*/
AUI_RTN_CODE aui_deca_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the DECA Module,
                after its closing by the function #aui_deca_close

@param[in]      p_call_back_init        = Callback function used to de-initialize
                                          the DECA Module
@param[in]      pv_param                = The input parameter of the callback
                                          function @b call_back_init used to
                                          de-initialize of the DECA Module

@return         @b AUI_RTN_SUCCESS      = De-initializing of the DECA Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = De-initializing of the DECA Module
                                          failed for some reasons

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested
                to @a only use this function before to go into standby mode
*/
AUI_RTN_CODE aui_deca_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to open the DECA Module and configure the desired
                attributes, then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the DECA Module, in particular:
                - Either after performing the initialization of the DECA Module
                  by the function #aui_deca_init for the first opening of the
                  DECA Module
                - Or after closing the DECA Module by the function
                  #aui_deca_close, considering the initialization of the DECA
                  Module has been performed previously by the function
                  #aui_deca_init

@param[in]      *p_attr_deca            = Pointer to a struct #aui_attr_deca,
                                          which collects the desired attributes
                                          for the DECA Module

@param[out]     *pp_hdl_deca            = #aui_hdl pointer to the handle of the
                                          DECA Module just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DECA Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_open (

    const aui_attr_deca *p_attr_deca,

    aui_hdl* const pp_hdl_deca

    );

/**
@brief          Function used to close the DECA Module already opened by the
                function #aui_deca_open, then the related handle will be
                released (i.e. the related resources such as memory, device)

@warning        This function can @a only be used in the <b> Post-Run Stage
                </b> of the DECA Module in pair with its the opening by the
                function #aui_deca_open.\n
                After closing the DECA Module, user can
                - Either perform the de-initialization of the DECA Module by
                  the function #aui_deca_de_init
                - Or open again the DECA Module by the function #aui_decv_open,
                  considering the initialization of the DECA Module has been
                  performed previously by the function #aui_deca_init

@param[in]      p_hdl_deca              = #aui_hdl pointer to the handle of the
                                          DECA Module already opened and to be
                                          closed

@return         @b AUI_RTN_SUCCESS      = Closing of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameters (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Closing of the DECA Module failed for
                                          same reasons

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested
                to @a only use this function before to go into standby mode
*/
AUI_RTN_CODE aui_deca_close (

    aui_hdl p_hdl_deca

    );

/**
@brief          Function used to start the DECA Module, already opened by the
                function #aui_deca_open

@warning        This function is particularly used after some supposed/suggested
                configurations, such as audio format by the function
                #aui_deca_type_set (which has already been performed after
                opening the DECA Module).\n
                After starting the DECA Module, the audio decoder can start to
                decode the incoming audio data

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened and to be started
@param[in]      p_attr_deca             = This parameter is not used and should
                                          be set as NULL

@return         @b AUI_RTN_SUCCESS      = Starting of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Starting of the DECA Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_start (

    aui_hdl p_hdl_deca,

    const aui_attr_deca *p_attr_deca

    );

/**
@brief          Function used to stop the DECA Module, already opened and started
                by respectively the functions #aui_deca_open and #aui_deca_start

@warning        After stopping the DECA Module, user can
                - Either close the DECA Module by the function #aui_deca_close
                  if no need to use the DECA Module anymore
                - Or start again the DECA Module by the function #aui_deca_start,
                  considering the supposed/suggested configurations of the DECA
                  Module have been performed again before starting the DECA
                  Module (if user wants to change the audio channel)

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already started then to be stopped
@param[in]      p_attr_deca             = This parameter is not used and should
                                          be set as NULL

@return         @b AUI_RTN_SUCCESS      = Stopping of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Stopping of the DECA Module failed
                                          for some reasons
*/
AUI_RTN_CODE aui_deca_stop (

    aui_hdl p_hdl_deca,

    const aui_attr_deca *p_attr_deca

    );

/**
@brief          Function used to pause the DECA Module, already opened and
                started by respectively the functions #aui_deca_open and
                #aui_deca_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already started then to be paused
@param[in]      p_attr_deca             = This parameter is not used and should
                                          be set as NULL

@return         @b AUI_RTN_SUCCESS      = Pausing of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Pausing of the DECA Module failed for
                                          some reasons

@note           @b 1. After pausing the DECA Module, the function #aui_deca_resume
                      should be called to resume the DECA Module

@note           @b 2. This function will only pause decoding, the sound output
                      does not stop immediately since some data are still going
                      out from the Sound Module. To pause sound output
                      immediately, please use the function  #aui_snd_pause
*/
AUI_RTN_CODE aui_deca_pause (

    aui_hdl p_hdl_deca,

    const aui_attr_deca *p_attr_deca

    );

/**
@brief          Function used to resume the DECA Module, already opened, started
                and paused by respectively the functions #aui_deca_open,
                #aui_decv_start and #aui_decv_pause

@warning        After resuming the DECA Module, the audio will be decoded again
                and two (2) actions can be performed as below:
                - Pausing the audio again by the function #aui_deca_pause
                - Stopping the audio by the function #aui_deca_stop

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened, as well as already
                                          started and paused then to be resumed
@param[in]      p_attr_deca             = This parameter is not used and should
                                          be set as NULL

@return         @b AUI_RTN_SUCCESS      = Resuming of the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Resuming of the DECA Module failed for
                                          some reasons

@note           The function #aui_deca_resume should be called to resume the
                DECA Module again when it is in pause status by calling
                #aui_deca_pause
*/
AUI_RTN_CODE aui_deca_resume (

    aui_hdl p_hdl_deca,

    const aui_attr_deca *p_attr_deca

    );

/**
@brief          Function used to perform a specific setting of the DECA Module,
                between its opening and starting by respectively the functions
                #aui_deca_open and #aui_deca_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened, and to be managed to
                                          perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting of the DECA Module to be
                                          performed, as defined in the enum
                                          #aui_deca_item_set
@param[in]      *pv_param               = The pointer as per the description
                                          of the specific setting of the DECA
                                          Module to be performed, as defined
                                          in the enum #aui_deca_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting of the DECA Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Specific setting of the DECA Module
                                          failed for some reasons
*/
AUI_RTN_CODE aui_deca_set (

    aui_hdl p_hdl_deca,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific setting of the DECA Module,
                after its opening and starting by respectively the functions
                #aui_deca_open and #aui_deca_start.

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened and to be managed to
                                          get a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting of the DECA Module to be
                                          gotten, as defined in the enum
                                          #aui_deca_item_get
@param[out]     *pv_param               = The pointer as per the description
                                          of the specific setting of the DECA
                                          Module to be gotten, as defined in
                                          the enum #aui_deca_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific setting of
                                          the DECA Module performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific setting of
                                          the DECA Module failed for some
                                          reasons
*/
AUI_RTN_CODE aui_deca_get (

    aui_hdl p_hdl_deca,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get the total number of suppoted audio
                decoding devices

@warning        At present, @a only one (1) audio decoding device can be
                supported

@param[out]     pul_deca_cnt            = Pointer to the total number of
                                          supported audio decoding devices

@return         @b AUI_RTN_SUCCESS      = Getting of the total number of
                                          supported audio decoding device
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The output parameter (i.e. [out])
                                          is invalid
@return         @b Other_Values         = Getting of the total number of
                                          supported audio decoding device
                                          failed for some reasons
*/
AUI_RTN_CODE aui_deca_dev_cnt_get (

    unsigned long* const pul_deca_cnt

    );

/**
@brief          Function used to get the audio format currently set to the DECA
                Module by the function #aui_deca_type_set

@param[in]      p_hdl_deca              = #aui_hdl handle of DECA Module already
                                          opened and to be checked for the audio
                                          format currently set

@param[out]     pul_deca_cur_type       = The audio format already set as the
                                          current one to the DECA Module, as
                                          defined in the enum
                                          #aui_deca_stream_type

@return         @b AUI_RTN_SUCCESS      = Checking of the audio format currently
                                          set to the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the audio format currently
                                          set to the DECA Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_type_get (

    aui_hdl p_hdl_deca,

    unsigned long* const pul_deca_cur_type

    );

/**
@brief          Function used to set the desired audio decoding format to the
                DECA Module, between its opening and starting by respectively
                the functions #aui_deca_open and #aui_deca_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened and to be set for the
                                          desired audio decoding format
@param[in]      ul_deca_cur_type        = Desired audio format to be set to the
                                          DECA Module, as defined in the enum
                                          #aui_en_dec_format

@return         @b AUI_RTN_SUCCESS      = Setting of the desired audio decoding
                                          format to the DECA Module performed
                                          succesfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Setting of the desired audio decoding
                                          format to the DECA Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_type_set (

    aui_hdl p_hdl_deca,

    unsigned long ul_deca_cur_type

    );

/**
@brief          Function used to get the synchronization mode currently set to
                the DECA Module by the function #aui_deca_sync_mode_set,
                between its opening and starting by respectively the functions
                #aui_deca_open and #aui_deca_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened and to be checked for
                                          the audio synchronization mode
                                          currently set
@param[in]      pul_deca_sync_mode      = The audio synchronization mode already
                                          set as the current one to the DECA
                                          Module, as defined in the enum
                                          #aui_deca_sync_mode

@return         @b AUI_RTN_SUCCESS      = Checking of the audio synchronization
                                          mode currently set to DECA Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Checking of the audio synchronization
                                          mode currently set to DECA Module
                                          failed for some reasons
*/
AUI_RTN_CODE aui_deca_sync_mode_get (

    aui_hdl p_hdl_deca,

    unsigned long* const pul_deca_sync_mode

    );

/**
@brief          Function used to set the desired synchronization mode to the
                DECA Module, between its opening and starting by respectively
                the functions #aui_deca_open and #aui_deca_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened and to be set for the
                                          desired synchronization mode
@param[in]      ul_deca_sync_mode       = Desired synchronization mode to be
                                          set to the DECA Module, as defined
                                          in the enum #aui_deca_sync_mode

@return         @b AUI_RTN_SUCCESS      = Setting of the desired synchronization
                                          mode to the DECA Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Setting of the desired synchronization
                                          mode to the DECA Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_sync_mode_set (

    aui_hdl p_hdl_deca,

    unsigned long ul_deca_sync_mode

    );

/**
@brief          Function used to get the current status of the DECA Module,
                after its opening by the functions #aui_deca_open

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened
@param[in]      pul_deca_status         = Pointer to the current status of DECA
                                          Module, as defined in the enum
                                          #aui_deca_status

@return         @b AUI_RTN_SUCCESS      = Getting of the current status of the
                                          DECA Module performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Getting of the status of the DECA
                                          Module failed for some reasons
*/
AUI_RTN_CODE aui_deca_status_get (

    aui_hdl p_hdl_deca,

    unsigned long* const pul_deca_status

    );

/**
@brief          Function used to start the tone (voice) of the DECA Module then
                DECA driver can output the mono channel voice with 48000 sample
                per second

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened
@param[in]      uc_level                = This parameter is no longer supported
                                          then is @a deprecated (user can set
                                          it as zero (0))
@param[in]      p_data                  = Address of the audio data sent to the
                                          audio driver
@param[in]      data_len                = Data length of the audio data sent to
                                          the audio driver

@return         @b AUI_RTN_SUCCESS      = Starting of the tone (voice) of the
                                          DECA Module performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Starting of the tone (voice) of the
                                          DECA Module failed for some reasons
*/
AUI_RTN_CODE aui_deca_bee_tone_start (

    aui_hdl p_hdl_deca,

    unsigned char uc_level,

    int *p_data,

    unsigned int data_len

    );

/**
@brief          Function used to stop the tone (voice) of the DECA Module
                started by the function #aui_deca_bee_tone_start

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened

@return         @b AUI_RTN_SUCCESS      = Stopping of the tone (voice) of the
                                          DECA Module is performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Stopping of the tone (voice) of the
                                          DECA Module failed for some reasons
*/
AUI_RTN_CODE aui_deca_bee_tone_stop (

    aui_hdl p_hdl_deca

    );

/**
@brief          Function used to play MPEG format data

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened
@param[in]      param                   = Pointer to a struct
                                          #aui_deca_stream_play_param, which
                                          specifies the information of MPEG
                                          format data to be sent to Audio decoder
                                          to play

@return         @b AUI_RTN_SUCCESS      = Playing of the MPEG format data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Playing of the MPEG format data failed
                                          for some reasons
*/
AUI_RTN_CODE aui_deca_str_play (

    aui_hdl p_hdl_deca,

    unsigned int param

    );

/**
@brief          Function used to set the interval (in @a millisecond (ms) unit)
                before every playing of the MPEG format data

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened
@param[in]      interval                = interval

@return         @b AUI_RTN_SUCCESS      = Setting of the interval performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Setting of the interval failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_set_beep_interval (

    aui_hdl p_hdl_deca,

    unsigned int interval

    );

/**
@brief          Function used to stop playing MPEG format data

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened

@return         @b AUI_RTN_SUCCESS      = Stop playing of the MPEG format data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e.[in]) is
                                          invalid
@return         @b Other_Values         = Stop playing of the MPEG format data
                                          failed for some reasons
*/
AUI_RTN_CODE aui_deca_str_stop (

    aui_hdl p_hdl_deca

    );

/**
@brief          Function used to get information from the DECA module

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA Module
                                          already opened

@param[out]     p_data_info             = Pointer to the struct #aui_deca_data_info
                                          which collects the current status information
                                          of the DECA module

@return         @b AUI_RTN_SUCCESS      = Getting of the information performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e.[in], [out]) is
                                          invalid
@return         @b Other_Values         = Getting of the information failed for
                                          some reasons
*/
AUI_RTN_CODE aui_deca_data_info_get (

    aui_hdl p_hdl_deca,

    aui_deca_data_info *p_data_info

    );

/**
@brief          Function used to enable/disable the output of the audio description

@note           The pid of the audio description must be set to DMX through the
                struct #aui_dmx_stream_pid before calling this function

@param[in]      p_hdl_deca              = #aui_hdl handle of the DECA device
                                          already opened
@param[in]      uc_enable               = Flag to set the output of the audio
                                          description, in particular:
                                          - @b 1 = Enabled (default value)
                                          - @b 0 = Disabled

@return         @b AUI_RTN_SUCCESS      = Setting of the output of the audio
                                          description performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Value          = Setting of the output of the audio
                                          description failed for some reasons
*/
AUI_RTN_CODE aui_deca_audio_desc_enable (

    aui_hdl p_hdl_deca,

    unsigned char uc_enable

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_audio_stream_type_em  aui_deca_stream_type

#define aui_audio_sample_rate_em  aui_deca_sample_rate

#define aui_audio_quantization_em aui_deca_audio_quantization

#define aui_audio_stop_mode_em    aui_deca_stop_mode

#define AUI_DECA_STREAM_TYPE_MPEG_AAC AUI_DECA_STREAM_TYPE_AAC_LATM

#define AUI_DECA_STREAM_TYPE_MPEG_ADTS_AAC AUI_DECA_STREAM_TYPE_AAC_ADTS

#define aui_avsync_ctrl_st aui_avsync_ctrl

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


